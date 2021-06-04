/*
**
** Copyright 2007, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

//#define LOG_NDEBUG 0
#define LOG_TAG "AudioTrack"

#include <inttypes.h>
#include <math.h>
#include <sys/resource.h>

#include <android-base/macros.h>
#include <audio_utils/clock.h>
#include <audio_utils/primitives.h>
#include <binder/IPCThreadState.h>
#include <media/AudioTrack.h>
#include <utils/Log.h>
#include <private/media/AudioTrackShared.h>
#include <processgroup/sched_policy.h>
#include <media/IAudioFlinger.h>
#include <media/IAudioPolicyService.h>
#include <media/AudioParameter.h>
#include <media/AudioResamplerPublic.h>
#include <media/AudioSystem.h>
#include <media/MediaAnalyticsItem.h>
#include <media/TypeConverter.h>

#define WAIT_PERIOD_MS                  10
#define WAIT_STREAM_END_TIMEOUT_SEC     120
static const int kMaxLoopCountNotifications = 32;

namespace android {
// ---------------------------------------------------------------------------

using media::VolumeShaper;

// TODO: Move to a separate .h

template <typename T>
static inline const T &min(const T &x, const T &y) {
    return x < y ? x : y;
}

template <typename T>
static inline const T &max(const T &x, const T &y) {
    return x > y ? x : y;
}

static inline nsecs_t framesToNanoseconds(ssize_t frames, uint32_t sampleRate, float speed)
{
    return ((double)frames * 1000000000) / ((double)sampleRate * speed);
}

static int64_t convertTimespecToUs(const struct timespec &tv)
{
    return tv.tv_sec * 1000000LL + tv.tv_nsec / 1000;
}

// TODO move to audio_utils.
static inline struct timespec convertNsToTimespec(int64_t ns) {
    struct timespec tv;
    tv.tv_sec = static_cast<time_t>(ns / NANOS_PER_SECOND);
    tv.tv_nsec = static_cast<long>(ns % NANOS_PER_SECOND);
    return tv;
}

// current monotonic time in microseconds.
static int64_t getNowUs()
{
    struct timespec tv;
    (void) clock_gettime(CLOCK_MONOTONIC, &tv);
    return convertTimespecToUs(tv);
}

// FIXME: we don't use the pitch setting in the time stretcher (not working);
// instead we emulate it using our sample rate converter.
static const bool kFixPitch = true; // enable pitch fix
static inline uint32_t adjustSampleRate(uint32_t sampleRate, float pitch)
{
    return kFixPitch ? (sampleRate * pitch + 0.5) : sampleRate;
}

static inline float adjustSpeed(float speed, float pitch)
{
    return kFixPitch ? speed / max(pitch, AUDIO_TIMESTRETCH_PITCH_MIN_DELTA) : speed;
}

static inline float adjustPitch(float pitch)
{
    return kFixPitch ? AUDIO_TIMESTRETCH_PITCH_NORMAL : pitch;
}

// static
status_t AudioTrack::getMinFrameCount(
        size_t* frameCount,
        audio_stream_type_t streamType,
        uint32_t sampleRate)
{
    if (frameCount == NULL) {
        return BAD_VALUE;
    }

    // FIXME handle in server, like createTrack_l(), possible missing info:
    //          audio_io_handle_t output
    //          audio_format_t format
    //          audio_channel_mask_t channelMask
    //          audio_output_flags_t flags (FAST)
    uint32_t afSampleRate;
    status_t status;
    status = AudioSystem::getOutputSamplingRate(&afSampleRate, streamType);
    if (status != NO_ERROR) {
        ALOGE("%s(): Unable to query output sample rate for stream type %d; status %d",
                __func__, streamType, status);
        return status;
    }
    size_t afFrameCount;
    status = AudioSystem::getOutputFrameCount(&afFrameCount, streamType);
    if (status != NO_ERROR) {
        ALOGE("%s(): Unable to query output frame count for stream type %d; status %d",
                __func__, streamType, status);
        return status;
    }
    uint32_t afLatency;
    status = AudioSystem::getOutputLatency(&afLatency, streamType);
    if (status != NO_ERROR) {
        ALOGE("%s(): Unable to query output latency for stream type %d; status %d",
                __func__, streamType, status);
        return status;
    }

    // When called from createTrack, speed is 1.0f (normal speed).
    // This is rechecked again on setting playback rate (TODO: on setting sample rate, too).
    *frameCount = AudioSystem::calculateMinFrameCount(afLatency, afFrameCount, afSampleRate,
                                              sampleRate, 1.0f /*, 0 notificationsPerBufferReq*/);

    // The formula above should always produce a non-zero value under normal circumstances:
    // AudioTrack.SAMPLE_RATE_HZ_MIN <= sampleRate <= AudioTrack.SAMPLE_RATE_HZ_MAX.
    // Return error in the unlikely event that it does not, as that's part of the API contract.
    if (*frameCount == 0) {
        ALOGE("%s(): failed for streamType %d, sampleRate %u",
                __func__, streamType, sampleRate);
        return BAD_VALUE;
    }
    ALOGV("%s(): getMinFrameCount=%zu: afFrameCount=%zu, afSampleRate=%u, afLatency=%u",
            __func__, *frameCount, afFrameCount, afSampleRate, afLatency);
    return NO_ERROR;
}

// static
bool AudioTrack::isDirectOutputSupported(const audio_config_base_t& config,
                                         const audio_attributes_t& attributes) {
    ALOGV("%s()", __FUNCTION__);
    const sp<IAudioPolicyService>& aps = AudioSystem::get_audio_policy_service();
    if (aps == 0) return false;
    return aps->isDirectOutputSupported(config, attributes);
}

// ---------------------------------------------------------------------------

void AudioTrack::MediaMetrics::gather(const AudioTrack *track)
{
    // only if we're in a good state...
    // XXX: shall we gather alternative info if failing?
    const status_t lstatus = track->initCheck();
    if (lstatus != NO_ERROR) {
        ALOGD("%s(): no metrics gathered, track status=%d", __func__, (int) lstatus);
        return;
    }

#define MM_PREFIX "android.media.audiotrack." // avoid cut-n-paste errors.

    // Java API 28 entries, do not change.
    mAnalyticsItem->setCString(MM_PREFIX "streamtype", toString(track->streamType()).c_str());
    mAnalyticsItem->setCString(MM_PREFIX "type",
            toString(track->mAttributes.content_type).c_str());
    mAnalyticsItem->setCString(MM_PREFIX "usage", toString(track->mAttributes.usage).c_str());

    // Non-API entries, these can change due to a Java string mistake.
    mAnalyticsItem->setInt32(MM_PREFIX "sampleRate", (int32_t)track->mSampleRate);
    mAnalyticsItem->setInt64(MM_PREFIX "channelMask", (int64_t)track->mChannelMask);
    // Non-API entries, these can change.
    mAnalyticsItem->setInt32(MM_PREFIX "portId", (int32_t)track->mPortId);
    mAnalyticsItem->setCString(MM_PREFIX "encoding", toString(track->mFormat).c_str());
    mAnalyticsItem->setInt32(MM_PREFIX "frameCount", (int32_t)track->mFrameCount);
    mAnalyticsItem->setCString(MM_PREFIX "attributes", toString(track->mAttributes).c_str());
}

// hand the user a snapshot of the metrics.
status_t AudioTrack::getMetrics(MediaAnalyticsItem * &item)
{
    mMediaMetrics.gather(this);
    MediaAnalyticsItem *tmp = mMediaMetrics.dup();
    if (tmp == nullptr) {
        return BAD_VALUE;
    }
    item = tmp;
    return NO_ERROR;
}

AudioTrack::AudioTrack()
    : mStatus(NO_INIT),
      mState(STATE_STOPPED),
      mPreviousPriority(ANDROID_PRIORITY_NORMAL),
      mPreviousSchedulingGroup(SP_DEFAULT),
      mPausedPosition(0),
      mSelectedDeviceId(AUDIO_PORT_HANDLE_NONE),
      mRoutedDeviceId(AUDIO_PORT_HANDLE_NONE)
{
    mAttributes.content_type = AUDIO_CONTENT_TYPE_UNKNOWN;
    mAttributes.usage = AUDIO_USAGE_UNKNOWN;
    mAttributes.flags = 0x0;
    strcpy(mAttributes.tags, "");
}

AudioTrack::AudioTrack(
        audio_stream_type_t streamType,
        uint32_t sampleRate,
        audio_format_t format,
        audio_channel_mask_t channelMask,
        size_t frameCount,
        audio_output_flags_t flags,
        callback_t cbf,
        void* user,
        int32_t notificationFrames,
        audio_session_t sessionId,
        transfer_type transferType,
        const audio_offload_info_t *offloadInfo,
        uid_t uid,
        pid_t pid,
        const audio_attributes_t* pAttributes,
        bool doNotReconnect,
        float maxRequiredSpeed,
        audio_port_handle_t selectedDeviceId)
    : mStatus(NO_INIT),
      mState(STATE_STOPPED),
      mPreviousPriority(ANDROID_PRIORITY_NORMAL),
      mPreviousSchedulingGroup(SP_DEFAULT),
      mPausedPosition(0)
{
    mAttributes = AUDIO_ATTRIBUTES_INITIALIZER;

    (void)set(streamType, sampleRate, format, channelMask,
            frameCount, flags, cbf, user, notificationFrames,
            0 /*sharedBuffer*/, false /*threadCanCallJava*/, sessionId, transferType,
            offloadInfo, uid, pid, pAttributes, doNotReconnect, maxRequiredSpeed, selectedDeviceId);
}

AudioTrack::AudioTrack(
        audio_stream_type_t streamType,
        uint32_t sampleRate,
        audio_format_t format,
        audio_channel_mask_t channelMask,
        const sp<IMemory>& sharedBuffer,
        audio_output_flags_t flags,
        callback_t cbf,
        void* user,
        int32_t notificationFrames,
        audio_session_t sessionId,
        transfer_type transferType,
        const audio_offload_info_t *offloadInfo,
        uid_t uid,
        pid_t pid,
        const audio_attributes_t* pAttributes,
        bool doNotReconnect,
        float maxRequiredSpeed)
    : mStatus(NO_INIT),
      mState(STATE_STOPPED),
      mPreviousPriority(ANDROID_PRIORITY_NORMAL),
      mPreviousSchedulingGroup(SP_DEFAULT),
      mPausedPosition(0),
      mSelectedDeviceId(AUDIO_PORT_HANDLE_NONE)
{
    mAttributes = AUDIO_ATTRIBUTES_INITIALIZER;

    (void)set(streamType, sampleRate, format, channelMask,
            0 /*frameCount*/, flags, cbf, user, notificationFrames,
            sharedBuffer, false /*threadCanCallJava*/, sessionId, transferType, offloadInfo,
            uid, pid, pAttributes, doNotReconnect, maxRequiredSpeed);
}

AudioTrack::~AudioTrack()
{
    // pull together the numbers, before we clean up our structures
    mMediaMetrics.gather(this);

    if (mStatus == NO_ERROR) {
        // Make sure that callback function exits in the case where
        // it is looping on buffer full condition in obtainBuffer().
        // Otherwise the callback thread will never exit.
        stop();
        if (mAudioTrackThread != 0) {
            mProxy->interrupt();
            mAudioTrackThread->requestExit();   // see comment in AudioTrack.h
            mAudioTrackThread->requestExitAndWait();
            mAudioTrackThread.clear();
        }
        // No lock here: worst case we remove a NULL callback which will be a nop
        if (mDeviceCallback != 0 && mOutput != AUDIO_IO_HANDLE_NONE) {
            AudioSystem::removeAudioDeviceCallback(this, mOutput, mPortId);
        }
        IInterface::asBinder(mAudioTrack)->unlinkToDeath(mDeathNotifier, this);
        mAudioTrack.clear();
        mCblkMemory.clear();
        mSharedBuffer.clear();
        IPCThreadState::self()->flushCommands();
        ALOGV("%s(%d), releasing session id %d from %d on behalf of %d",
                __func__, mPortId,
                mSessionId, IPCThreadState::self()->getCallingPid(), mClientPid);
        AudioSystem::releaseAudioSessionId(mSessionId, mClientPid);
    }
}

status_t AudioTrack::set(
        audio_stream_type_t streamType,
        uint32_t sampleRate,
        audio_format_t format,
        audio_channel_mask_t channelMask,
        size_t frameCount,
        audio_output_flags_t flags,
        callback_t cbf,
        void* user,
        int32_t notificationFrames,
        const sp<IMemory>& sharedBuffer,
        bool threadCanCallJava,
        audio_session_t sessionId,
        transfer_type transferType,
        const audio_offload_info_t *offloadInfo,
        uid_t uid,
        pid_t pid,
        const audio_attributes_t* pAttributes,
        bool doNotReconnect,
        float maxRequiredSpeed,
        audio_port_handle_t selectedDeviceId)
{
    status_t status;
    uint32_t channelCount;
    pid_t callingPid;
    pid_t myPid;

    // Note mPortId is not valid until the track is created, so omit mPortId in ALOG for set.
    ALOGV("%s(): streamType %d, sampleRate %u, format %#x, channelMask %#x, frameCount %zu, "
          "flags #%x, notificationFrames %d, sessionId %d, transferType %d, uid %d, pid %d",
          __func__,
          streamType, sampleRate, format, channelMask, frameCount, flags, notificationFrames,
          sessionId, transferType, uid, pid);

    mThreadCanCallJava = threadCanCallJava;
    mSelectedDeviceId = selectedDeviceId;
    mSessionId = sessionId;

    switch (transferType) {
    case TRANSFER_DEFAULT:
        if (sharedBuffer != 0) {
            transferType = TRANSFER_SHARED;
        } else if (cbf == NULL || threadCanCallJava) {
            transferType = TRANSFER_SYNC;
        } else {
            transferType = TRANSFER_CALLBACK;
        }
        break;
    case TRANSFER_CALLBACK:
    case TRANSFER_SYNC_NOTIF_CALLBACK:
        if (cbf == NULL || sharedBuffer != 0) {
            ALOGE("%s(): Transfer type %s but cbf == NULL || sharedBuffer != 0",
                    convertTransferToText(transferType), __func__);
            status = BAD_VALUE;
            goto exit;
        }
        break;
    case TRANSFER_OBTAIN:
    case TRANSFER_SYNC:
        if (sharedBuffer != 0) {
            ALOGE("%s(): Transfer type TRANSFER_OBTAIN but sharedBuffer != 0", __func__);
            status = BAD_VALUE;
            goto exit;
        }
        break;
    case TRANSFER_SHARED:
        if (sharedBuffer == 0) {
            ALOGE("%s(): Transfer type TRANSFER_SHARED but sharedBuffer == 0", __func__);
            status = BAD_VALUE;
            goto exit;
        }
        break;
    default:
        ALOGE("%s(): Invalid transfer type %d",
                __func__, transferType);
        status = BAD_VALUE;
        goto exit;
    }
    mSharedBuffer = sharedBuffer;
    mTransfer = transferType;
    mDoNotReconnect = doNotReconnect;

    ALOGV_IF(sharedBuffer != 0, "%s(): sharedBuffer: %p, size: %zu",
            __func__, sharedBuffer->pointer(), sharedBuffer->size());

    ALOGV("%s(): streamType %d frameCount %zu flags %04x",
            __func__, streamType, frameCount, flags);

    // invariant that mAudioTrack != 0 is true only after set() returns successfully
    if (mAudioTrack != 0) {
        ALOGE("%s(): Track already in use", __func__);
        status = INVALID_OPERATION;
        goto exit;
    }

    // handle default values first.
    if (streamType == AUDIO_STREAM_DEFAULT) {
        streamType = AUDIO_STREAM_MUSIC;
    }
    if (pAttributes == NULL) {
        if (uint32_t(streamType) >= AUDIO_STREAM_PUBLIC_CNT) {
            ALOGE("%s(): Invalid stream type %d", __func__, streamType);
            status = BAD_VALUE;
            goto exit;
        }
        mStreamType = streamType;

    } else {
        // stream type shouldn't be looked at, this track has audio attributes
        memcpy(&mAttributes, pAttributes, sizeof(audio_attributes_t));
        ALOGV("%s(): Building AudioTrack with attributes:"
                " usage=%d content=%d flags=0x%x tags=[%s]",
                __func__,
                 mAttributes.usage, mAttributes.content_type, mAttributes.flags, mAttributes.tags);
        mStreamType = AUDIO_STREAM_DEFAULT;
        audio_flags_to_audio_output_flags(mAttributes.flags, &flags);
    }

    // these below should probably come from the audioFlinger too...
    if (format == AUDIO_FORMAT_DEFAULT) {
        format = AUDIO_FORMAT_PCM_16_BIT;
    } else if (format == AUDIO_FORMAT_IEC61937) { // HDMI pass-through?
        mAttributes.flags |= AUDIO_OUTPUT_FLAG_IEC958_NONAUDIO;
    }

    // validate parameters
    if (!audio_is_valid_format(format)) {
        ALOGE("%s(): Invalid format %#x", __func__, format);
        status = BAD_VALUE;
        goto exit;
    }
    mFormat = format;

    if (!audio_is_output_channel(channelMask)) {
        ALOGE("%s(): Invalid channel mask %#x",  __func__, channelMask);
        status = BAD_VALUE;
        goto exit;
    }
    mChannelMask = channelMask;
    channelCount = audio_channel_count_from_out_mask(channelMask);
    mChannelCount = channelCount;

    // force direct flag if format is not linear PCM
    // or offload was requested
    if ((flags & AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD)
            || !audio_is_linear_pcm(format)) {
        ALOGV( (flags & AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD)
                    ? "%s(): Offload request, forcing to Direct Output"
                    : "%s(): Not linear PCM, forcing to Direct Output",
                    __func__);
        flags = (audio_output_flags_t)
                // FIXME why can't we allow direct AND fast?
                ((flags | AUDIO_OUTPUT_FLAG_DIRECT) & ~AUDIO_OUTPUT_FLAG_FAST);
    }

    // force direct flag if HW A/V sync requested
    if ((flags & AUDIO_OUTPUT_FLAG_HW_AV_SYNC) != 0) {
        flags = (audio_output_flags_t)(flags | AUDIO_OUTPUT_FLAG_DIRECT);
    }

    if (flags & AUDIO_OUTPUT_FLAG_DIRECT) {
        if (audio_has_proportional_frames(format)) {
            mFrameSize = channelCount * audio_bytes_per_sample(format);
        } else {
            mFrameSize = sizeof(uint8_t);
        }
    } else {
        ALOG_ASSERT(audio_has_proportional_frames(format));
        mFrameSize = channelCount * audio_bytes_per_sample(format);
        // createTrack will return an error if PCM format is not supported by server,
        // so no need to check for specific PCM formats here
    }

    // sampling rate must be specified for direct outputs
    if (sampleRate == 0 && (flags & AUDIO_OUTPUT_FLAG_DIRECT) != 0) {
        status = BAD_VALUE;
        goto exit;
    }
    mSampleRate = sampleRate;
    mOriginalSampleRate = sampleRate;
    mPlaybackRate = AUDIO_PLAYBACK_RATE_DEFAULT;
    // 1.0 <= mMaxRequiredSpeed <= AUDIO_TIMESTRETCH_SPEED_MAX
    mMaxRequiredSpeed = min(max(maxRequiredSpeed, 1.0f), AUDIO_TIMESTRETCH_SPEED_MAX);

    // Make copy of input parameter offloadInfo so that in the future:
    //  (a) createTrack_l doesn't need it as an input parameter
    //  (b) we can support re-creation of offloaded tracks
    if (offloadInfo != NULL) {
        mOffloadInfoCopy = *offloadInfo;
        mOffloadInfo = &mOffloadInfoCopy;
    } else {
        mOffloadInfo = NULL;
        memset(&mOffloadInfoCopy, 0, sizeof(audio_offload_info_t));
    }

    mVolume[AUDIO_INTERLEAVE_LEFT] = 1.0f;
    mVolume[AUDIO_INTERLEAVE_RIGHT] = 1.0f;
    mSendLevel = 0.0f;
    // mFrameCount is initialized in createTrack_l
    mReqFrameCount = frameCount;
    if (notificationFrames >= 0) {
        mNotificationFramesReq = notificationFrames;
        mNotificationsPerBufferReq = 0;
    } else {
        if (!(flags & AUDIO_OUTPUT_FLAG_FAST)) {
            ALOGE("%s(): notificationFrames=%d not permitted for non-fast track",
                    __func__, notificationFrames);
            status = BAD_VALUE;
            goto exit;
        }
        if (frameCount > 0) {
            ALOGE("%s(): notificationFrames=%d not permitted with non-zero frameCount=%zu",
                    __func__, notificationFrames, frameCount);
            status = BAD_VALUE;
            goto exit;
        }
        mNotificationFramesReq = 0;
        const uint32_t minNotificationsPerBuffer = 1;
        const uint32_t maxNotificationsPerBuffer = 8;
        mNotificationsPerBufferReq = min(maxNotificationsPerBuffer,
                max((uint32_t) -notificationFrames, minNotificationsPerBuffer));
        ALOGW_IF(mNotificationsPerBufferReq != (uint32_t) -notificationFrames,
                "%s(): notificationFrames=%d clamped to the range -%u to -%u",
                __func__,
                notificationFrames, minNotificationsPerBuffer, maxNotificationsPerBuffer);
    }
    mNotificationFramesAct = 0;
    callingPid = IPCThreadState::self()->getCallingPid();
    myPid = getpid();
    if (uid == AUDIO_UID_INVALID || (callingPid != myPid)) {
        mClientUid = IPCThreadState::self()->getCallingUid();
    } else {
        mClientUid = uid;
    }
    if (pid == -1 || (callingPid != myPid)) {
        mClientPid = callingPid;
    } else {
        mClientPid = pid;
    }
    mAuxEffectId = 0;
    mOrigFlags = mFlags = flags;
    mCbf = cbf;

    if (cbf != NULL) {
        mAudioTrackThread = new AudioTrackThread(*this);
        mAudioTrackThread->run("AudioTrack", ANDROID_PRIORITY_AUDIO, 0 /*stack*/);
        // thread begins in paused state, and will not reference us until start()
    }

    // create the IAudioTrack
    {
        AutoMutex lock(mLock);
        status = createTrack_l();
    }
    if (status != NO_ERROR) {
        if (mAudioTrackThread != 0) {
            mAudioTrackThread->requestExit();   // see comment in AudioTrack.h
            mAudioTrackThread->requestExitAndWait();
            mAudioTrackThread.clear();
        }
        goto exit;
    }

    mUserData = user;
    mLoopCount = 0;
    mLoopStart = 0;
    mLoopEnd = 0;
    mLoopCountNotified = 0;
    mMarkerPosition = 0;
    mMarkerReached = false;
    mNewPosition = 0;
    mUpdatePeriod = 0;
    mPosition = 0;
    mReleased = 0;
    mStartNs = 0;
    mStartFromZeroUs = 0;
    AudioSystem::acquireAudioSessionId(mSessionId, mClientPid);
    mSequence = 1;
    mObservedSequence = mSequence;
    mInUnderrun = false;
    mPreviousTimestampValid = false;
    mTimestampStartupGlitchReported = false;
    mTimestampRetrogradePositionReported = false;
    mTimestampRetrogradeTimeReported = false;
    mTimestampStallReported = false;
    mTimestampStaleTimeReported = false;
    mPreviousLocation = ExtendedTimestamp::LOCATION_INVALID;
    mStartTs.mPosition = 0;
    mUnderrunCountOffset = 0;
    mFramesWritten = 0;
    mFramesWrittenServerOffset = 0;
    mFramesWrittenAtRestore = -1; // -1 is a unique initializer.
    mVolumeHandler = new media::VolumeHandler();

exit:
    mStatus = status;
    return status;
}

// -------------------------------------------------------------------------

status_t AudioTrack::start()
{
    AutoMutex lock(mLock);
    ALOGV("%s(%d): prior state:%s", __func__, mPortId, stateToString(mState));

    if (mState == STATE_ACTIVE) {
        return INVALID_OPERATION;
    }

    mInUnderrun = true;

    State previousState = mState;
    if (previousState == STATE_PAUSED_STOPPING) {
        mState = STATE_STOPPING;
    } else {
        mState = STATE_ACTIVE;
    }
    (void) updateAndGetPosition_l();

    // save start timestamp
    if (isOffloadedOrDirect_l()) {
        if (getTimestamp_l(mStartTs) != OK) {
            mStartTs.mPosition = 0;
        }
    } else {
        if (getTimestamp_l(&mStartEts) != OK) {
            mStartEts.clear();
        }
    }
    mStartNs = systemTime(); // save this for timestamp adjustment after starting.
    if (previousState == STATE_STOPPED || previousState == STATE_FLUSHED) {
        // reset current position as seen by client to 0
        mPosition = 0;
        mPreviousTimestampValid = false;
        mTimestampStartupGlitchReported = false;
        mTimestampRetrogradePositionReported = false;
        mTimestampRetrogradeTimeReported = false;
        mTimestampStallReported = false;
        mTimestampStaleTimeReported = false;
        mPreviousLocation = ExtendedTimestamp::LOCATION_INVALID;

        if (!isOffloadedOrDirect_l()
                && mStartEts.mTimeNs[ExtendedTimestamp::LOCATION_SERVER] > 0) {
            // Server side has consumed something, but is it finished consuming?
            // It is possible since flush and stop are asynchronous that the server
            // is still active at this point.
            ALOGV("%s(%d): server read:%lld  cumulative flushed:%lld  client written:%lld",
                    __func__, mPortId,
                    (long long)(mFramesWrittenServerOffset
                            + mStartEts.mPosition[ExtendedTimestamp::LOCATION_SERVER]),
                    (long long)mStartEts.mFlushed,
                    (long long)mFramesWritten);
            // mStartEts is already adjusted by mFramesWrittenServerOffset, so we delta adjust.
            mFramesWrittenServerOffset -= mStartEts.mPosition[ExtendedTimestamp::LOCATION_SERVER];
        }
        mFramesWritten = 0;
        mProxy->clearTimestamp(); // need new server push for valid timestamp
        mMarkerReached = false;

        // For offloaded tracks, we don't know if the hardware counters are really zero here,
        // since the flush is asynchronous and stop may not fully drain.
        // We save the time when the track is started to later verify whether
        // the counters are realistic (i.e. start from zero after this time).
        mStartFromZeroUs = mStartNs / 1000;

        // force refresh of remaining frames by processAudioBuffer() as last
        // write before stop could be partial.
        mRefreshRemaining = true;

        // for static track, clear the old flags when starting from stopped state
        if (mSharedBuffer != 0) {
            android_atomic_and(
            ~(CBLK_LOOP_CYCLE | CBLK_LOOP_FINAL | CBLK_BUFFER_END),
            &mCblk->mFlags);
        }
    }
    mNewPosition = mPosition + mUpdatePeriod;
    int32_t flags = android_atomic_and(~(CBLK_STREAM_END_DONE | CBLK_DISABLED), &mCblk->mFlags);

    status_t status = NO_ERROR;
    if (!(flags & CBLK_INVALID)) {
        status = mAudioTrack->start();
        if (status == DEAD_OBJECT) {
            flags |= CBLK_INVALID;
        }
    }
    if (flags & CBLK_INVALID) {
        status = restoreTrack_l("start");
    }

    // resume or pause the callback thread as needed.
    sp<AudioTrackThread> t = mAudioTrackThread;
    if (status == NO_ERROR) {
        if (t != 0) {
            if (previousState == STATE_STOPPING) {
                mProxy->interrupt();
            } else {
                t->resume();
            }
        } else {
            mPreviousPriority = getpriority(PRIO_PROCESS, 0);
            get_sched_policy(0, &mPreviousSchedulingGroup);
            androidSetThreadPriority(0, ANDROID_PRIORITY_AUDIO);
        }

        // Start our local VolumeHandler for restoration purposes.
        mVolumeHandler->setStarted();
    } else {
        ALOGE("%s(%d): status %d", __func__, mPortId, status);
        mState = previousState;
        if (t != 0) {
            if (previousState != STATE_STOPPING) {
                t->pause();
            }
        } else {
            setpriority(PRIO_PROCESS, 0, mPreviousPriority);
            set_sched_policy(0, mPreviousSchedulingGroup);
        }
    }

    return status;
}

void AudioTrack::stop()
{
    AutoMutex lock(mLock);
    ALOGV("%s(%d): prior state:%s", __func__, mPortId, stateToString(mState));

    if (mState != STATE_ACTIVE && mState != STATE_PAUSED) {
        return;
    }

    if (isOffloaded_l()) {
        mState = STATE_STOPPING;
    } else {
        mState = STATE_STOPPED;
        ALOGD_IF(mSharedBuffer == nullptr,
                "%s(%d): called with %u frames delivered", __func__, mPortId, mReleased.value());
        mReleased = 0;
    }

    mProxy->stop(); // notify server not to read beyond current client position until start().
    mProxy->interrupt();
    mAudioTrack->stop();

    // Note: legacy handling - stop does not clear playback marker
    // and periodic update counter, but flush does for streaming tracks.

    if (mSharedBuffer != 0) {
        // clear buffer position and loop count.
        mStaticProxy->setBufferPositionAndLoop(0 /* position */,
                0 /* loopStart */, 0 /* loopEnd */, 0 /* loopCount */);
    }

    sp<AudioTrackThread> t = mAudioTrackThread;
    if (t != 0) {
        if (!isOffloaded_l()) {
            t->pause();
        } else if (mTransfer == TRANSFER_SYNC_NOTIF_CALLBACK) {
            // causes wake up of the playback thread, that will callback the client for
            // EVENT_STREAM_END in processAudioBuffer()
            t->wake();
        }
    } else {
        setpriority(PRIO_PROCESS, 0, mPreviousPriority);
        set_sched_policy(0, mPreviousSchedulingGroup);
    }
}

bool AudioTrack::stopped() const
{
    AutoMutex lock(mLock);
    return mState != STATE_ACTIVE;
}

void AudioTrack::flush()
{
    AutoMutex lock(mLock);
    ALOGV("%s(%d): prior state:%s", __func__, mPortId, stateToString(mState));

    if (mSharedBuffer != 0) {
        return;
    }
    if (mState == STATE_ACTIVE) {
        return;
    }
    flush_l();
}

void AudioTrack::flush_l()
{
    ALOG_ASSERT(mState != STATE_ACTIVE);

    // clear playback marker and periodic update counter
    mMarkerPosition = 0;
    mMarkerReached = false;
    mUpdatePeriod = 0;
    mRefreshRemaining = true;

    mState = STATE_FLUSHED;
    mReleased = 0;
    if (isOffloaded_l()) {
        mProxy->interrupt();
    }
    mProxy->flush();
    mAudioTrack->flush();
}

void AudioTrack::pause()
{
    AutoMutex lock(mLock);
    ALOGV("%s(%d): prior state:%s", __func__, mPortId, stateToString(mState));

    if (mState == STATE_ACTIVE) {
        mState = STATE_PAUSED;
    } else if (mState == STATE_STOPPING) {
        mState = STATE_PAUSED_STOPPING;
    } else {
        return;
    }
    mProxy->interrupt();
    mAudioTrack->pause();

    if (isOffloaded_l()) {
        if (mOutput != AUDIO_IO_HANDLE_NONE) {
            // An offload output can be re-used between two audio tracks having
            // the same configuration. A timestamp query for a paused track
            // while the other is running would return an incorrect time.
            // To fix this, cache the playback position on a pause() and return
            // this time when requested until the track is resumed.

            // OffloadThread sends HAL pause in its threadLoop. Time saved
            // here can be slightly off.

            // TODO: check return code for getRenderPosition.

            uint32_t halFrames;
            AudioSystem::getRenderPosition(mOutput, &halFrames, &mPausedPosition);
            ALOGV("%s(%d): for offload, cache current position %u",
                    __func__, mPortId, mPausedPosition);
        }
    }
}

status_t AudioTrack::setVolume(float left, float right)
{
    // This duplicates a test by AudioTrack JNI, but that is not the only caller
    if (isnanf(left) || left < GAIN_FLOAT_ZERO || left > GAIN_FLOAT_UNITY ||
            isnanf(right) || right < GAIN_FLOAT_ZERO || right > GAIN_FLOAT_UNITY) {
        return BAD_VALUE;
    }

    AutoMutex lock(mLock);
    mVolume[AUDIO_INTERLEAVE_LEFT] = left;
    mVolume[AUDIO_INTERLEAVE_RIGHT] = right;

    mProxy->setVolumeLR(gain_minifloat_pack(gain_from_float(left), gain_from_float(right)));

    if (isOffloaded_l()) {
        mAudioTrack->signal();
    }
    return NO_ERROR;
}

status_t AudioTrack::setVolume(float volume)
{
    return setVolume(volume, volume);
}

status_t AudioTrack::setAuxEffectSendLevel(float level)
{
    // This duplicates a test by AudioTrack JNI, but that is not the only caller
    if (isnanf(level) || level < GAIN_FLOAT_ZERO || level > GAIN_FLOAT_UNITY) {
        return BAD_VALUE;
    }

    AutoMutex lock(mLock);
    mSendLevel = level;
    mProxy->setSendLevel(level);

    return NO_ERROR;
}

void AudioTrack::getAuxEffectSendLevel(float* level) const
{
    if (level != NULL) {
        *level = mSendLevel;
    }
}

status_t AudioTrack::setSampleRate(uint32_t rate)
{
    AutoMutex lock(mLock);
    ALOGV("%s(%d): prior state:%s rate:%u", __func__, mPortId, stateToString(mState), rate);

    if (rate == mSampleRate) {
        return NO_ERROR;
    }
    if (isOffloadedOrDirect_l() || (mFlags & AUDIO_OUTPUT_FLAG_FAST)
            || (mChannelMask & AUDIO_CHANNEL_HAPTIC_ALL)) {
        return INVALID_OPERATION;
    }
    if (mOutput == AUDIO_IO_HANDLE_NONE) {
        return NO_INIT;
    }
    // NOTE: it is theoretically possible, but highly unlikely, that a device change
    // could mean a previously allowed sampling rate is no longer allowed.
    uint32_t afSamplingRate;
    if (AudioSystem::getSamplingRate(mOutput, &afSamplingRate) != NO_ERROR) {
        return NO_INIT;
    }
    // pitch is emulated by adjusting speed and sampleRate
    const uint32_t effectiveSampleRate = adjustSampleRate(rate, mPlaybackRate.mPitch);
    if (rate == 0 || effectiveSampleRate > afSamplingRate * AUDIO_RESAMPLER_DOWN_RATIO_MAX) {
        return BAD_VALUE;
    }
    // TODO: Should we also check if the buffer size is compatible?

    mSampleRate = rate;
    mProxy->setSampleRate(effectiveSampleRate);

    return NO_ERROR;
}

uint32_t AudioTrack::getSampleRate() const
{
    AutoMutex lock(mLock);

    // sample rate can be updated during playback by the offloaded decoder so we need to
    // query the HAL and update if needed.
// FIXME use Proxy return channel to update the rate from server and avoid polling here
    if (isOffloadedOrDirect_l()) {
        if (mOutput != AUDIO_IO_HANDLE_NONE) {
            uint32_t sampleRate = 0;
            status_t status = AudioSystem::getSamplingRate(mOutput, &sampleRate);
            if (status == NO_ERROR) {
                mSampleRate = sampleRate;
            }
        }
    }
    return mSampleRate;
}

uint32_t AudioTrack::getOriginalSampleRate() const
{
    return mOriginalSampleRate;
}

status_t AudioTrack::setPlaybackRate(const AudioPlaybackRate &playbackRate)
{
    AutoMutex lock(mLock);
    if (isAudioPlaybackRateEqual(playbackRate, mPlaybackRate)) {
        return NO_ERROR;
    }
    if (isOffloadedOrDirect_l()) {
        return INVALID_OPERATION;
    }
    if (mFlags & AUDIO_OUTPUT_FLAG_FAST) {
        return INVALID_OPERATION;
    }

    ALOGV("%s(%d): mSampleRate:%u  mSpeed:%f  mPitch:%f",
            __func__, mPortId, mSampleRate, playbackRate.mSpeed, playbackRate.mPitch);
    // pitch is emulated by adjusting speed and sampleRate
    const uint32_t effectiveRate = adjustSampleRate(mSampleRate, playbackRate.mPitch);
    const float effectiveSpeed = adjustSpeed(playbackRate.mSpeed, playbackRate.mPitch);
    const float effectivePitch = adjustPitch(playbackRate.mPitch);
    AudioPlaybackRate playbackRateTemp = playbackRate;
    playbackRateTemp.mSpeed = effectiveSpeed;
    playbackRateTemp.mPitch = effectivePitch;

    ALOGV("%s(%d) (effective) mSampleRate:%u  mSpeed:%f  mPitch:%f",
            __func__, mPortId, effectiveRate, effectiveSpeed, effectivePitch);

    if (!isAudioPlaybackRateValid(playbackRateTemp)) {
        ALOGW("%s(%d) (%f, %f) failed (effective rate out of bounds)",
                __func__, mPortId, playbackRate.mSpeed, playbackRate.mPitch);
        return BAD_VALUE;
    }
    // Check if the buffer size is compatible.
    if (!isSampleRateSpeedAllowed_l(effectiveRate, effectiveSpeed)) {
        ALOGW("%s(%d) (%f, %f) failed (buffer size)",
                __func__, mPortId, playbackRate.mSpeed, playbackRate.mPitch);
        return BAD_VALUE;
    }

    // Check resampler ratios are within bounds
    if ((uint64_t)effectiveRate > (uint64_t)mSampleRate *
            (uint64_t)AUDIO_RESAMPLER_DOWN_RATIO_MAX) {
        ALOGW("%s(%d) (%f, %f) failed. Resample rate exceeds max accepted value",
                __func__, mPortId, playbackRate.mSpeed, playbackRate.mPitch);
        return BAD_VALUE;
    }

    if ((uint64_t)effectiveRate * (uint64_t)AUDIO_RESAMPLER_UP_RATIO_MAX < (uint64_t)mSampleRate) {
        ALOGW("%s(%d) (%f, %f) failed. Resample rate below min accepted value",
                __func__, mPortId, playbackRate.mSpeed, playbackRate.mPitch);
        return BAD_VALUE;
    }
    mPlaybackRate = playbackRate;
    //set effective rates
    mProxy->setPlaybackRate(playbackRateTemp);
    mProxy->setSampleRate(effectiveRate); // FIXME: not quite "atomic" with setPlaybackRate
    return NO_ERROR;
}

const AudioPlaybackRate& AudioTrack::getPlaybackRate() const
{
    AutoMutex lock(mLock);
    return mPlaybackRate;
}

ssize_t AudioTrack::getBufferSizeInFrames()
{
    AutoMutex lock(mLock);
    if (mOutput == AUDIO_IO_HANDLE_NONE || mProxy.get() == 0) {
        return NO_INIT;
    }
    return (ssize_t) mProxy->getBufferSizeInFrames();
}

status_t AudioTrack::getBufferDurationInUs(int64_t *duration)
{
    if (duration == nullptr) {
        return BAD_VALUE;
    }
    AutoMutex lock(mLock);
    if (mOutput == AUDIO_IO_HANDLE_NONE || mProxy.get() == 0) {
        return NO_INIT;
    }
    ssize_t bufferSizeInFrames = (ssize_t) mProxy->getBufferSizeInFrames();
    if (bufferSizeInFrames < 0) {
        return (status_t)bufferSizeInFrames;
    }
    *duration = (int64_t)((double)bufferSizeInFrames * 1000000
            / ((double)mSampleRate * mPlaybackRate.mSpeed));
    return NO_ERROR;
}

ssize_t AudioTrack::setBufferSizeInFrames(size_t bufferSizeInFrames)
{
    AutoMutex lock(mLock);
    if (mOutput == AUDIO_IO_HANDLE_NONE || mProxy.get() == 0) {
        return NO_INIT;
    }
    // Reject if timed track or compressed audio.
    if (!audio_is_linear_pcm(mFormat)) {
        return INVALID_OPERATION;
    }
    return (ssize_t) mProxy->setBufferSizeInFrames((uint32_t) bufferSizeInFrames);
}

status_t AudioTrack::setLoop(uint32_t loopStart, uint32_t loopEnd, int loopCount)
{
    if (mSharedBuffer == 0 || isOffloadedOrDirect()) {
        return INVALID_OPERATION;
    }

    if (loopCount == 0) {
        ;
    } else if (loopCount >= -1 && loopStart < loopEnd && loopEnd <= mFrameCount &&
            loopEnd - loopStart >= MIN_LOOP) {
        ;
    } else {
        return BAD_VALUE;
    }

    AutoMutex lock(mLock);
    // See setPosition() regarding setting parameters such as loop points or position while active
    if (mState == STATE_ACTIVE) {
        return INVALID_OPERATION;
    }
    setLoop_l(loopStart, loopEnd, loopCount);
    return NO_ERROR;
}

void AudioTrack::setLoop_l(uint32_t loopStart, uint32_t loopEnd, int loopCount)
{
    // We do not update the periodic notification point.
    // mNewPosition = updateAndGetPosition_l() + mUpdatePeriod;
    mLoopCount = loopCount;
    mLoopEnd = loopEnd;
    mLoopStart = loopStart;
    mLoopCountNotified = loopCount;
    mStaticProxy->setLoop(loopStart, loopEnd, loopCount);

    // Waking the AudioTrackThread is not needed as this cannot be called when active.
}

status_t AudioTrack::setMarkerPosition(uint32_t marker)
{
    // The only purpose of setting marker position is to get a callback
    if (mCbf == NULL || isOffloadedOrDirect()) {
        return INVALID_OPERATION;
    }

    AutoMutex lock(mLock);
    mMarkerPosition = marker;
    mMarkerReached = false;

    sp<AudioTrackThread> t = mAudioTrackThread;
    if (t != 0) {
        t->wake();
    }
    return NO_ERROR;
}

status_t AudioTrack::getMarkerPosition(uint32_t *marker) const
{
    if (isOffloadedOrDirect()) {
        return INVALID_OPERATION;
    }
    if (marker == NULL) {
        return BAD_VALUE;
    }

    AutoMutex lock(mLock);
    mMarkerPosition.getValue(marker);

    return NO_ERROR;
}

status_t AudioTrack::setPositionUpdatePeriod(uint32_t updatePeriod)
{
    // The only purpose of setting position update period is to get a callback
    if (mCbf == NULL || isOffloadedOrDirect()) {
        return INVALID_OPERATION;
    }

    AutoMutex lock(mLock);
    mNewPosition = updateAndGetPosition_l() + updatePeriod;
    mUpdatePeriod = updatePeriod;

    sp<AudioTrackThread> t = mAudioTrackThread;
    if (t != 0) {
        t->wake();
    }
    return NO_ERROR;
}

status_t AudioTrack::getPositionUpdatePeriod(uint32_t *updatePeriod) const
{
    if (isOffloadedOrDirect()) {
        return INVALID_OPERATION;
    }
    if (updatePeriod == NULL) {
        return BAD_VALUE;
    }

    AutoMutex lock(mLock);
    *updatePeriod = mUpdatePeriod;

    return NO_ERROR;
}

status_t AudioTrack::setPosition(uint32_t position)
{
    if (mSharedBuffer == 0 || isOffloadedOrDirect()) {
        return INVALID_OPERATION;
    }
    if (position > mFrameCount) {
        return BAD_VALUE;
    }

    AutoMutex lock(mLock);
    // Currently we require that the player is inactive before setting parameters such as position
    // or loop points.  Otherwise, there could be a race condition: the application could read the
    // current position, compute a new position or loop parameters, and then set that position or
    // loop parameters but it would do the "wrong" thing since the position has continued to advance
    // in the mean time.  If we ever provide a sequencer in server, we could allow a way for the app
    // to specify how it wants to handle such scenarios.
    if (mState == STATE_ACTIVE) {
        return INVALID_OPERATION;
    }
    // After setting the position, use full update period before notification.
    mNewPosition = updateAndGetPosition_l() + mUpdatePeriod;
    mStaticProxy->setBufferPosition(position);

    // Waking the AudioTrackThread is not needed as this cannot be called when active.
    return NO_ERROR;
}

status_t AudioTrack::getPosition(uint32_t *position)
{
    if (position == NULL) {
        return BAD_VALUE;
    }

    AutoMutex lock(mLock);
    // FIXME: offloaded and direct tracks call into the HAL for render positions
    // for compressed/synced data; however, we use proxy position for pure linear pcm data
    // as we do not know the capability of the HAL for pcm position support and standby.
    // There may be some latency differences between the HAL position and the proxy position.
    if (isOffloadedOrDirect_l() && !isPurePcmData_l()) {
        uint32_t dspFrames = 0;

        if (isOffloaded_l() && ((mState == STATE_PAUSED) || (mState == STATE_PAUSED_STOPPING))) {
            ALOGV("%s(%d): called in paused state, return cached position %u",
                __func__, mPortId, mPausedPosition);
            *position = mPausedPosition;
            return NO_ERROR;
        }

        if (mOutput != AUDIO_IO_HANDLE_NONE) {
            uint32_t halFrames; // actually unused
            (void) AudioSystem::getRenderPosition(mOutput, &halFrames, &dspFrames);
            // FIXME: on getRenderPosition() error, we return OK with frame position 0.
        }
        // FIXME: dspFrames may not be zero in (mState == STATE_STOPPED || mState == STATE_FLUSHED)
        // due to hardware latency. We leave this behavior for now.
        *position = dspFrames;
    } else {
        if (mCblk->mFlags & CBLK_INVALID) {
            (void) restoreTrack_l("getPosition");
            // FIXME: for compatibility with the Java API we ignore the restoreTrack_l()
            // error here (e.g. DEAD_OBJECT) and return OK with the last recorded server position.
        }

        // IAudioTrack::stop() isn't synchronous; we don't know when presentation completes
        *position = (mState == STATE_STOPPED || mState == STATE_FLUSHED) ?
                0 : updateAndGetPosition_l().value();
    }
    return NO_ERROR;
}

status_t AudioTrack::getBufferPosition(uint32_t *position)
{
    if (mSharedBuffer == 0) {
        return INVALID_OPERATION;
    }
    if (position == NULL) {
        return BAD_VALUE;
    }

    AutoMutex lock(mLock);
    *position = mStaticProxy->getBufferPosition();
    return NO_ERROR;
}

status_t AudioTrack::reload()
{
    if (mSharedBuffer == 0 || isOffloadedOrDirect()) {
        return INVALID_OPERATION;
    }

    AutoMutex lock(mLock);
    // See setPosition() regarding setting parameters such as loop points or position while active
    if (mState == STATE_ACTIVE) {
        return INVALID_OPERATION;
    }
    mNewPosition = mUpdatePeriod;
    (void) updateAndGetPosition_l();
    mPosition = 0;
    mPreviousTimestampValid = false;
#if 0
    // The documentation is not clear on the behavior of reload() and the restoration
    // of loop count. Historically we have not restored loop count, start, end,
    // but it makes sense if one desires to repeat playing a particular sound.
    if (mLoopCount != 0) {
        mLoopCountNotified = mLoopCount;
        mStaticProxy->setLoop(mLoopStart, mLoopEnd, mLoopCount);
    }
#endif
    mStaticProxy->setBufferPosition(0);
    return NO_ERROR;
}

audio_io_handle_t AudioTrack::getOutput() const
{
    AutoMutex lock(mLock);
    return mOutput;
}

status_t AudioTrack::setOutputDevice(audio_port_handle_t deviceId) {
    AutoMutex lock(mLock);
    if (mSelectedDeviceId != deviceId) {
        mSelectedDeviceId = deviceId;
        if (mStatus == NO_ERROR) {
            android_atomic_or(CBLK_INVALID, &mCblk->mFlags);
            mProxy->interrupt();
        }
    }
    return NO_ERROR;
}

audio_port_handle_t AudioTrack::getOutputDevice() {
    AutoMutex lock(mLock);
    return mSelectedDeviceId;
}

// must be called with mLock held
void AudioTrack::updateRoutedDeviceId_l()
{
    // if the track is inactive, do not update actual device as the output stream maybe routed
    // to a device not relevant to this client because of other active use cases.
    if (mState != STATE_ACTIVE) {
        return;
    }
    if (mOutput != AUDIO_IO_HANDLE_NONE) {
        audio_port_handle_t deviceId = AudioSystem::getDeviceIdForIo(mOutput);
        if (deviceId != AUDIO_PORT_HANDLE_NONE) {
            mRoutedDeviceId = deviceId;
        }
    }
}

audio_port_handle_t AudioTrack::getRoutedDeviceId() {
    AutoMutex lock(mLock);
    updateRoutedDeviceId_l();
    return mRoutedDeviceId;
}

status_t AudioTrack::attachAuxEffect(int effectId)
{
    AutoMutex lock(mLock);
    status_t status = mAudioTrack->attachAuxEffect(effectId);
    if (status == NO_ERROR) {
        mAuxEffectId = effectId;
    }
    return status;
}

audio_stream_type_t AudioTrack::streamType() const
{
    if (mStreamType == AUDIO_STREAM_DEFAULT) {
        return AudioSystem::attributesToStreamType(mAttributes);
    }
    return mStreamType;
}

uint32_t AudioTrack::latency()
{
    AutoMutex lock(mLock);
    updateLatency_l();
    return mLatency;
}

// -------------------------------------------------------------------------

// must be called with mLock held
void AudioTrack::updateLatency_l()
{
    status_t status = AudioSystem::getLatency(mOutput, &mAfLatency);
    if (status != NO_ERROR) {
        ALOGW("%s(%d): getLatency(%d) failed status %d", __func__, mPortId, mOutput, status);
    } else {
        // FIXME don't believe this lie
        mLatency = mAfLatency + (1000LL * mFrameCount) / mSampleRate;
    }
}

// TODO Move this macro to a common header file for enum to string conversion in audio framework.
#define MEDIA_CASE_ENUM(name) case name: return #name
const char * AudioTrack::convertTransferToText(transfer_type transferType) {
    switch (transferType) {
        MEDIA_CASE_ENUM(TRANSFER_DEFAULT);
        MEDIA_CASE_ENUM(TRANSFER_CALLBACK);
        MEDIA_CASE_ENUM(TRANSFER_OBTAIN);
        MEDIA_CASE_ENUM(TRANSFER_SYNC);
        MEDIA_CASE_ENUM(TRANSFER_SHARED);
        MEDIA_CASE_ENUM(TRANSFER_SYNC_NOTIF_CALLBACK);
        default:
            return "UNRECOGNIZED";
    }
}

status_t AudioTrack::createTrack_l()
{
    status_t status;
    bool callbackAdded = false;

    const sp<IAudioFlinger>& audioFlinger = AudioSystem::get_audio_flinger();
    if (audioFlinger == 0) {
        ALOGE("%s(%d): Could not get audioflinger",
                __func__, mPortId);
        status = NO_INIT;
        goto exit;
    }

    {
    // mFlags (not mOrigFlags) is modified depending on whether fast request is accepted.
    // After fast request is denied, we will request again if IAudioTrack is re-created.
    // Client can only express a preference for FAST.  Server will perform additional tests.
    if (mFlags & AUDIO_OUTPUT_FLAG_FAST) {
        // either of these use cases:
        // use case 1: shared buffer
        bool sharedBuffer = mSharedBuffer != 0;
        bool transferAllowed =
            // use case 2: callback transfer mode
            (mTransfer == TRANSFER_CALLBACK) ||
            // use case 3: obtain/release mode
            (mTransfer == TRANSFER_OBTAIN) ||
            // use case 4: synchronous write
            ((mTransfer == TRANSFER_SYNC || mTransfer == TRANSFER_SYNC_NOTIF_CALLBACK)
                    && mThreadCanCallJava);

        bool fastAllowed = sharedBuffer || transferAllowed;
        if (!fastAllowed) {
            ALOGW("%s(%d): AUDIO_OUTPUT_FLAG_FAST denied by client,"
                  " not shared buffer and transfer = %s",
                  __func__, mPortId,
                  convertTransferToText(mTransfer));
            mFlags = (audio_output_flags_t) (mFlags & ~AUDIO_OUTPUT_FLAG_FAST);
        }
    }

    IAudioFlinger::CreateTrackInput input;
    if (mStreamType != AUDIO_STREAM_DEFAULT) {
        input.attr = AudioSystem::streamTypeToAttributes(mStreamType);
    } else {
        input.attr = mAttributes;
    }
    input.config = AUDIO_CONFIG_INITIALIZER;
    input.config.sample_rate = mSampleRate;
    input.config.channel_mask = mChannelMask;
    input.config.format = mFormat;
    input.config.offload_info = mOffloadInfoCopy;
    input.clientInfo.clientUid = mClientUid;
    input.clientInfo.clientPid = mClientPid;
    input.clientInfo.clientTid = -1;
    if (mFlags & AUDIO_OUTPUT_FLAG_FAST) {
        // It is currently meaningless to request SCHED_FIFO for a Java thread.  Even if the
        // application-level code follows all non-blocking design rules, the language runtime
        // doesn't also follow those rules, so the thread will not benefit overall.
        if (mAudioTrackThread != 0 && !mThreadCanCallJava) {
            input.clientInfo.clientTid = mAudioTrackThread->getTid();
        }
    }
    input.sharedBuffer = mSharedBuffer;
    input.notificationsPerBuffer = mNotificationsPerBufferReq;
    input.speed = 1.0;
    if (audio_has_proportional_frames(mFormat) && mSharedBuffer == 0 &&
            (mFlags & AUDIO_OUTPUT_FLAG_FAST) == 0) {
        input.speed  = !isPurePcmData_l() || isOffloadedOrDirect_l() ? 1.0f :
                        max(mMaxRequiredSpeed, mPlaybackRate.mSpeed);
    }
    input.flags = mFlags;
    input.frameCount = mReqFrameCount;
    input.notificationFrameCount = mNotificationFramesReq;
    input.selectedDeviceId = mSelectedDeviceId;
    input.sessionId = mSessionId;

    IAudioFlinger::CreateTrackOutput output;

    sp<IAudioTrack> track = audioFlinger->createTrack(input,
                                                      output,
                                                      &status);

    if (status != NO_ERROR || output.outputId == AUDIO_IO_HANDLE_NONE) {
        ALOGE("%s(%d): AudioFlinger could not create track, status: %d output %d",
                __func__, mPortId, status, output.outputId);
        if (status == NO_ERROR) {
            status = NO_INIT;
        }
        goto exit;
    }
    ALOG_ASSERT(track != 0);

    mFrameCount = output.frameCount;
    mNotificationFramesAct = (uint32_t)output.notificationFrameCount;
    mRoutedDeviceId = output.selectedDeviceId;
    mSessionId = output.sessionId;

    mSampleRate = output.sampleRate;
    if (mOriginalSampleRate == 0) {
        mOriginalSampleRate = mSampleRate;
    }

    mAfFrameCount = output.afFrameCount;
    mAfSampleRate = output.afSampleRate;
    mAfLatency = output.afLatencyMs;

    mLatency = mAfLatency + (1000LL * mFrameCount) / mSampleRate;

    // AudioFlinger now owns the reference to the I/O handle,
    // so we are no longer responsible for releasing it.

    // FIXME compare to AudioRecord
    sp<IMemory> iMem = track->getCblk();
    if (iMem == 0) {
        ALOGE("%s(%d): Could not get control block", __func__, mPortId);
        status = NO_INIT;
        goto exit;
    }
    void *iMemPointer = iMem->pointer();
    if (iMemPointer == NULL) {
        ALOGE("%s(%d): Could not get control block pointer", __func__, mPortId);
        status = NO_INIT;
        goto exit;
    }
    // invariant that mAudioTrack != 0 is true only after set() returns successfully
    if (mAudioTrack != 0) {
        IInterface::asBinder(mAudioTrack)->unlinkToDeath(mDeathNotifier, this);
        mDeathNotifier.clear();
    }
    mAudioTrack = track;
    mCblkMemory = iMem;
    IPCThreadState::self()->flushCommands();

    audio_track_cblk_t* cblk = static_cast<audio_track_cblk_t*>(iMemPointer);
    mCblk = cblk;

    mAwaitBoost = false;
    if (mFlags & AUDIO_OUTPUT_FLAG_FAST) {
        if (output.flags & AUDIO_OUTPUT_FLAG_FAST) {
            ALOGI("%s(%d): AUDIO_OUTPUT_FLAG_FAST successful; frameCount %zu -> %zu",
                  __func__, mPortId, mReqFrameCount, mFrameCount);
            if (!mThreadCanCallJava) {
                mAwaitBoost = true;
            }
        } else {
            ALOGW("%s(%d): AUDIO_OUTPUT_FLAG_FAST denied by server; frameCount %zu -> %zu",
                  __func__, mPortId, mReqFrameCount, mFrameCount);
        }
    }
    mFlags = output.flags;

    //mOutput != output includes the case where mOutput == AUDIO_IO_HANDLE_NONE for first creation
    if (mDeviceCallback != 0) {
        if (mOutput != AUDIO_IO_HANDLE_NONE) {
            AudioSystem::removeAudioDeviceCallback(this, mOutput, mPortId);
        }
        AudioSystem::addAudioDeviceCallback(this, output.outputId, output.portId);
        callbackAdded = true;
    }

    mPortId = output.portId;
    // We retain a copy of the I/O handle, but don't own the reference
    mOutput = output.outputId;
    mRefreshRemaining = true;

    // Starting address of buffers in shared memory.  If there is a shared buffer, buffers
    // is the value of pointer() for the shared buffer, otherwise buffers points
    // immediately after the control block.  This address is for the mapping within client
    // address space.  AudioFlinger::TrackBase::mBuffer is for the server address space.
    void* buffers;
    if (mSharedBuffer == 0) {
        buffers = cblk + 1;
    } else {
        buffers = mSharedBuffer->pointer();
        if (buffers == NULL) {
            ALOGE("%s(%d): Could not get buffer pointer", __func__, mPortId);
            status = NO_INIT;
            goto exit;
        }
    }

    mAudioTrack->attachAuxEffect(mAuxEffectId);

    // If IAudioTrack is re-created, don't let the requested frameCount
    // decrease.  This can confuse clients that cache frameCount().
    if (mFrameCount > mReqFrameCount) {
        mReqFrameCount = mFrameCount;
    }

    // reset server position to 0 as we have new cblk.
    mServer = 0;

    // update proxy
    if (mSharedBuffer == 0) {
        mStaticProxy.clear();
        mProxy = new AudioTrackClientProxy(cblk, buffers, mFrameCount, mFrameSize);
    } else {
        mStaticProxy = new StaticAudioTrackClientProxy(cblk, buffers, mFrameCount, mFrameSize);
        mProxy = mStaticProxy;
    }

    mProxy->setVolumeLR(gain_minifloat_pack(
            gain_from_float(mVolume[AUDIO_INTERLEAVE_LEFT]),
            gain_from_float(mVolume[AUDIO_INTERLEAVE_RIGHT])));

    mProxy->setSendLevel(mSendLevel);
    const uint32_t effectiveSampleRate = adjustSampleRate(mSampleRate, mPlaybackRate.mPitch);
    const float effectiveSpeed = adjustSpeed(mPlaybackRate.mSpeed, mPlaybackRate.mPitch);
    const float effectivePitch = adjustPitch(mPlaybackRate.mPitch);
    mProxy->setSampleRate(effectiveSampleRate);

    AudioPlaybackRate playbackRateTemp = mPlaybackRate;
    playbackRateTemp.mSpeed = effectiveSpeed;
    playbackRateTemp.mPitch = effectivePitch;
    mProxy->setPlaybackRate(playbackRateTemp);
    mProxy->setMinimum(mNotificationFramesAct);

    mDeathNotifier = new DeathNotifier(this);
    IInterface::asBinder(mAudioTrack)->linkToDeath(mDeathNotifier, this);

    }

exit:
    if (status != NO_ERROR && callbackAdded) {
        // note: mOutput is always valid is callbackAdded is true
        AudioSystem::removeAudioDeviceCallback(this, mOutput, mPortId);
    }

    mStatus = status;

    // sp<IAudioTrack> track destructor will cause releaseOutput() to be called by AudioFlinger
    return status;
}

status_t AudioTrack::obtainBuffer(Buffer* audioBuffer, int32_t waitCount, size_t *nonContig)
{
    if (audioBuffer == NULL) {
        if (nonContig != NULL) {
            *nonContig = 0;
        }
        return BAD_VALUE;
    }
    if (mTransfer != TRANSFER_OBTAIN) {
        audioBuffer->frameCount = 0;
        audioBuffer->size = 0;
        audioBuffer->raw = NULL;
        if (nonContig != NULL) {
            *nonContig = 0;
        }
        return INVALID_OPERATION;
    }

    const struct timespec *requested;
    struct timespec timeout;
    if (waitCount == -1) {
        requested = &ClientProxy::kForever;
    } else if (waitCount == 0) {
        requested = &ClientProxy::kNonBlocking;
    } else if (waitCount > 0) {
        time_t ms = WAIT_PERIOD_MS * (time_t) waitCount;
        timeout.tv_sec = ms / 1000;
        timeout.tv_nsec = (long) (ms % 1000) * 1000000;
        requested = &timeout;
    } else {
        ALOGE("%s(%d): invalid waitCount %d", __func__, mPortId, waitCount);
        requested = NULL;
    }
    return obtainBuffer(audioBuffer, requested, NULL /*elapsed*/, nonContig);
}

status_t AudioTrack::obtainBuffer(Buffer* audioBuffer, const struct timespec *requested,
        struct timespec *elapsed, size_t *nonContig)
{
    // previous and new IAudioTrack sequence numbers are used to detect track re-creation
    uint32_t oldSequence = 0;

    Proxy::Buffer buffer;
    status_t status = NO_ERROR;

    static const int32_t kMaxTries = 5;
    int32_t tryCounter = kMaxTries;

    do {
        // obtainBuffer() is called with mutex unlocked, so keep extra references to these fields to
        // keep them from going away if another thread re-creates the track during obtainBuffer()
        sp<AudioTrackClientProxy> proxy;
        sp<IMemory> iMem;

        {   // start of lock scope
            AutoMutex lock(mLock);

            uint32_t newSequence = mSequence;
            // did previous obtainBuffer() fail due to media server death or voluntary invalidation?
            if (status == DEAD_OBJECT) {
                // re-create track, unless someone else has already done so
                if (newSequence == oldSequence) {
                    status = restoreTrack_l("obtainBuffer");
                    if (status != NO_ERROR) {
                        buffer.mFrameCount = 0;
                        buffer.mRaw = NULL;
                        buffer.mNonContig = 0;
                        break;
                    }
                }
            }
            oldSequence = newSequence;

            if (status == NOT_ENOUGH_DATA) {
                restartIfDisabled();
            }

            // Keep the extra references
            proxy = mProxy;
            iMem = mCblkMemory;

            if (mState == STATE_STOPPING) {
                status = -EINTR;
                buffer.mFrameCount = 0;
                buffer.mRaw = NULL;
                buffer.mNonContig = 0;
                break;
            }

            // Non-blocking if track is stopped or paused
            if (mState != STATE_ACTIVE) {
                requested = &ClientProxy::kNonBlocking;
            }

        }   // end of lock scope

        buffer.mFrameCount = audioBuffer->frameCount;
        // FIXME starts the requested timeout and elapsed over from scratch
        status = proxy->obtainBuffer(&buffer, requested, elapsed);
    } while (((status == DEAD_OBJECT) || (status == NOT_ENOUGH_DATA)) && (tryCounter-- > 0));

    audioBuffer->frameCount = buffer.mFrameCount;
    audioBuffer->size = buffer.mFrameCount * mFrameSize;
    audioBuffer->raw = buffer.mRaw;
    audioBuffer->sequence = oldSequence;
    if (nonContig != NULL) {
        *nonContig = buffer.mNonContig;
    }
    return status;
}

void AudioTrack::releaseBuffer(const Buffer* audioBuffer)
{
    // FIXME add error checking on mode, by adding an internal version
    if (mTransfer == TRANSFER_SHARED) {
        return;
    }

    size_t stepCount = audioBuffer->size / mFrameSize;
    if (stepCount == 0) {
        return;
    }

    Proxy::Buffer buffer;
    buffer.mFrameCount = stepCount;
    buffer.mRaw = audioBuffer->raw;

    AutoMutex lock(mLock);
    if (audioBuffer->sequence != mSequence) {
        // This Buffer came from a different IAudioTrack instance, so ignore the releaseBuffer
        ALOGD("%s is no-op due to IAudioTrack sequence mismatch %u != %u",
                __func__, audioBuffer->sequence, mSequence);
        return;
    }
    mReleased += stepCount;
    mInUnderrun = false;
    mProxy->releaseBuffer(&buffer);

    // restart track if it was disabled by audioflinger due to previous underrun
    restartIfDisabled();
}

void AudioTrack::restartIfDisabled()
{
    int32_t flags = android_atomic_and(~CBLK_DISABLED, &mCblk->mFlags);
    if ((mState == STATE_ACTIVE) && (flags & CBLK_DISABLED)) {
        ALOGW("%s(%d): releaseBuffer() track %p disabled due to previous underrun, restarting",
                __func__, mPortId, this);
        // FIXME ignoring status
        mAudioTrack->start();
    }
}

// -------------------------------------------------------------------------

ssize_t AudioTrack::write(const void* buffer, size_t userSize, bool blocking)
{
    if (mTransfer != TRANSFER_SYNC && mTransfer != TRANSFER_SYNC_NOTIF_CALLBACK) {
        return INVALID_OPERATION;
    }

    if (isDirect()) {
        AutoMutex lock(mLock);
        int32_t flags = android_atomic_and(
                            ~(CBLK_UNDERRUN | CBLK_LOOP_CYCLE | CBLK_LOOP_FINAL | CBLK_BUFFER_END),
                            &mCblk->mFlags);
        if (flags & CBLK_INVALID) {
            return DEAD_OBJECT;
        }
    }

    if (ssize_t(userSize) < 0 || (buffer == NULL && userSize != 0)) {
        // Sanity-check: user is most-likely passing an error code, and it would
        // make the return value ambiguous (actualSize vs error).
        ALOGE("%s(%d): AudioTrack::write(buffer=%p, size=%zu (%zd)",
                __func__, mPortId, buffer, userSize, userSize);
        return BAD_VALUE;
    }

    size_t written = 0;
    Buffer audioBuffer;

    while (userSize >= mFrameSize) {
        audioBuffer.frameCount = userSize / mFrameSize;

        status_t err = obtainBuffer(&audioBuffer,
                blocking ? &ClientProxy::kForever : &ClientProxy::kNonBlocking);
        if (err < 0) {
            if (written > 0) {
                break;
            }
            if (err == TIMED_OUT || err == -EINTR) {
                err = WOULD_BLOCK;
            }
            return ssize_t(err);
        }

        size_t toWrite = audioBuffer.size;
        memcpy(audioBuffer.i8, buffer, toWrite);
        buffer = ((const char *) buffer) + toWrite;
        userSize -= toWrite;
        written += toWrite;

        releaseBuffer(&audioBuffer);
    }

    if (written > 0) {
        mFramesWritten += written / mFrameSize;

        if (mTransfer == TRANSFER_SYNC_NOTIF_CALLBACK) {
            const sp<AudioTrackThread> t = mAudioTrackThread;
            if (t != 0) {
                // causes wake up of the playback thread, that will callback the client for
                // more data (with EVENT_CAN_WRITE_MORE_DATA) in processAudioBuffer()
                t->wake();
            }
        }
    }

    return written;
}

// -------------------------------------------------------------------------

nsecs_t AudioTrack::processAudioBuffer()
{
    // Currently the AudioTrack thread is not created if there are no callbacks.
    // Would it ever make sense to run the thread, even without callbacks?
    // If so, then replace this by checks at each use for mCbf != NULL.
    LOG_ALWAYS_FATAL_IF(mCblk == NULL);

    mLock.lock();
    if (mAwaitBoost) {
        mAwaitBoost = false;
        mLock.unlock();
        static const int32_t kMaxTries = 5;
        int32_t tryCounter = kMaxTries;
        uint32_t pollUs = 10000;
        do {
            int policy = sched_getscheduler(0) & ~SCHED_RESET_ON_FORK;
            if (policy == SCHED_FIFO || policy == SCHED_RR) {
                break;
            }
            usleep(pollUs);
            pollUs <<= 1;
        } while (tryCounter-- > 0);
        if (tryCounter < 0) {
            ALOGE("%s(%d): did not receive expected priority boost on time",
                    __func__, mPortId);
        }
        // Run again immediately
        return 0;
    }

    // Can only reference mCblk while locked
    int32_t flags = android_atomic_and(
        ~(CBLK_UNDERRUN | CBLK_LOOP_CYCLE | CBLK_LOOP_FINAL | CBLK_BUFFER_END), &mCblk->mFlags);

    // Check for track invalidation
    if (flags & CBLK_INVALID) {
        // for offloaded tracks restoreTrack_l() will just update the sequence and clear
        // AudioSystem cache. We should not exit here but after calling the callback so
        // that the upper layers can recreate the track
        if (!isOffloadedOrDirect_l() || (mSequence == mObservedSequence)) {
            status_t status __unused = restoreTrack_l("processAudioBuffer");
            // FIXME unused status
            // after restoration, continue below to make sure that the loop and buffer events
            // are notified because they have been cleared from mCblk->mFlags above.
        }
    }

    bool waitStreamEnd = mState == STATE_STOPPING;
    bool active = mState == STATE_ACTIVE;

    // Manage underrun callback, must be done under lock to avoid race with releaseBuffer()
    bool newUnderrun = false;
    if (flags & CBLK_UNDERRUN) {
#if 0
        // Currently in shared buffer mode, when the server reaches the end of buffer,
        // the track stays active in continuous underrun state.  It's up to the application
        // to pause or stop the track, or set the position to a new offset within buffer.
        // This was some experimental code to auto-pause on underrun.   Keeping it here
        // in "if 0" so we can re-visit this if we add a real sequencer for shared memory content.
        if (mTransfer == TRANSFER_SHARED) {
            mState = STATE_PAUSED;
            active = false;
        }
#endif
        if (!mInUnderrun) {
            mInUnderrun = true;
            newUnderrun = true;
        }
    }

    // Get current position of server
    Modulo<uint32_t> position(updateAndGetPosition_l());

    // Manage marker callback
    bool markerReached = false;
    Modulo<uint32_t> markerPosition(mMarkerPosition);
    // uses 32 bit wraparound for comparison with position.
    if (!mMarkerReached && markerPosition.value() > 0 && position >= markerPosition) {
        mMarkerReached = markerReached = true;
    }

    // Determine number of new position callback(s) that will be needed, while locked
    size_t newPosCount = 0;
    Modulo<uint32_t> newPosition(mNewPosition);
    uint32_t updatePeriod = mUpdatePeriod;
    // FIXME fails for wraparound, need 64 bits
    if (updatePeriod > 0 && position >= newPosition) {
        newPosCount = ((position - newPosition).value() / updatePeriod) + 1;
        mNewPosition += updatePeriod * newPosCount;
    }

    // Cache other fields that will be needed soon
    uint32_t sampleRate = mSampleRate;
    float speed = mPlaybackRate.mSpeed;
    const uint32_t notificationFrames = mNotificationFramesAct;
    if (mRefreshRemaining) {
        mRefreshRemaining = false;
        mRemainingFrames = notificationFrames;
        mRetryOnPartialBuffer = false;
    }
    size_t misalignment = mProxy->getMisalignment();
    uint32_t sequence = mSequence;
    sp<AudioTrackClientProxy> proxy = mProxy;

    // Determine the number of new loop callback(s) that will be needed, while locked.
    int loopCountNotifications = 0;
    uint32_t loopPeriod = 0; // time in frames for next EVENT_LOOP_END or EVENT_BUFFER_END

    if (mLoopCount > 0) {
        int loopCount;
        size_t bufferPosition;
        mStaticProxy->getBufferPositionAndLoopCount(&bufferPosition, &loopCount);
        loopPeriod = ((loopCount > 0) ? mLoopEnd : mFrameCount) - bufferPosition;
        loopCountNotifications = min(mLoopCountNotified - loopCount, kMaxLoopCountNotifications);
        mLoopCountNotified = loopCount; // discard any excess notifications
    } else if (mLoopCount < 0) {
        // FIXME: We're not accurate with notification count and position with infinite looping
        // since loopCount from server side will always return -1 (we could decrement it).
        size_t bufferPosition = mStaticProxy->getBufferPosition();
        loopCountNotifications = int((flags & (CBLK_LOOP_CYCLE | CBLK_LOOP_FINAL)) != 0);
        loopPeriod = mLoopEnd - bufferPosition;
    } else if (/* mLoopCount == 0 && */ mSharedBuffer != 0) {
        size_t bufferPosition = mStaticProxy->getBufferPosition();
        loopPeriod = mFrameCount - bufferPosition;
    }

    // These fields don't need to be cached, because they are assigned only by set():
    //     mTransfer, mCbf, mUserData, mFormat, mFrameSize, mFlags
    // mFlags is also assigned by createTrack_l(), but not the bit we care about.

    mLock.unlock();

    // get anchor time to account for callbacks.
    const nsecs_t timeBeforeCallbacks = systemTime();

    if (waitStreamEnd) {
        // FIXME:  Instead of blocking in proxy->waitStreamEndDone(), Callback thread
        // should wait on proxy futex and handle CBLK_STREAM_END_DONE within this function
        // (and make sure we don't callback for more data while we're stopping).
        // This helps with position, marker notifications, and track invalidation.
        struct timespec timeout;
        timeout.tv_sec = WAIT_STREAM_END_TIMEOUT_SEC;
        timeout.tv_nsec = 0;

        status_t status = proxy->waitStreamEndDone(&timeout);
        switch (status) {
        case NO_ERROR:
        case DEAD_OBJECT:
        case TIMED_OUT:
            if (status != DEAD_OBJECT) {
                // for DEAD_OBJECT, we do not send a EVENT_STREAM_END after stop();
                // instead, the application should handle the EVENT_NEW_IAUDIOTRACK.
                mCbf(EVENT_STREAM_END, mUserData, NULL);
            }
            {
                AutoMutex lock(mLock);
                // The previously assigned value of waitStreamEnd is no longer valid,
                // since the mutex has been unlocked and either the callback handler
                // or another thread could have re-started the AudioTrack during that time.
                waitStreamEnd = mState == STATE_STOPPING;
                if (waitStreamEnd) {
                    mState = STATE_STOPPED;
                    mReleased = 0;
                }
            }
            if (waitStreamEnd && status != DEAD_OBJECT) {
               return NS_INACTIVE;
            }
            break;
        }
        return 0;
    }

    // perform callbacks while unlocked
    if (newUnderrun) {
        mCbf(EVENT_UNDERRUN, mUserData, NULL);
    }
    while (loopCountNotifications > 0) {
        mCbf(EVENT_LOOP_END, mUserData, NULL);
        --loopCountNotifications;
    }
    if (flags & CBLK_BUFFER_END) {
        mCbf(EVENT_BUFFER_END, mUserData, NULL);
    }
    if (markerReached) {
        mCbf(EVENT_MARKER, mUserData, &markerPosition);
    }
    while (newPosCount > 0) {
        size_t temp = newPosition.value(); // FIXME size_t != uint32_t
        mCbf(EVENT_NEW_POS, mUserData, &temp);
        newPosition += updatePeriod;
        newPosCount--;
    }

    if (mObservedSequence != sequence) {
        mObservedSequence = sequence;
        mCbf(EVENT_NEW_IAUDIOTRACK, mUserData, NULL);
        // for offloaded tracks, just wait for the upper layers to recreate the track
        if (isOffloadedOrDirect()) {
            return NS_INACTIVE;
        }
    }

    // if inactive, then don't run me again until re-started
    if (!active) {
        return NS_INACTIVE;
    }

    // Compute the estimated time until the next timed event (position, markers, loops)
    // FIXME only for non-compressed audio
    uint32_t minFrames = ~0;
    if (!markerReached && position < markerPosition) {
        minFrames = (markerPosition - position).value();
    }
    if (loopPeriod > 0 && loopPeriod < minFrames) {
        // loopPeriod is already adjusted for actual position.
        minFrames = loopPeriod;
    }
    if (updatePeriod > 0) {
        minFrames = min(minFrames, (newPosition - position).value());
    }

    // If > 0, poll periodically to recover from a stuck server.  A good value is 2.
    static const uint32_t kPoll = 0;
    if (kPoll > 0 && mTransfer == TRANSFER_CALLBACK && kPoll * notificationFrames < minFrames) {
        minFrames = kPoll * notificationFrames;
    }

    // This "fudge factor" avoids soaking CPU, and compensates for late progress by server
    static const nsecs_t kWaitPeriodNs = WAIT_PERIOD_MS * 1000000LL;
    const nsecs_t timeAfterCallbacks = systemTime();

    // Convert frame units to time units
    nsecs_t ns = NS_WHENEVER;
    if (minFrames != (uint32_t) ~0) {
        // AudioFlinger consumption of client data may be irregular when coming out of device
        // standby since the kernel buffers require filling. This is throttled to no more than 2x
        // the expected rate in the MixerThread. Hence, we reduce the estimated time to wait by one
        // half (but no more than half a second) to improve callback accuracy during these temporary
        // data surges.
        const nsecs_t estimatedNs = framesToNanoseconds(minFrames, sampleRate, speed);
        constexpr nsecs_t maxThrottleCompensationNs = 500000000LL;
        ns = estimatedNs - min(estimatedNs / 2, maxThrottleCompensationNs) + kWaitPeriodNs;
        ns -= (timeAfterCallbacks - timeBeforeCallbacks);  // account for callback time
        // TODO: Should we warn if the callback time is too long?
        if (ns < 0) ns = 0;
    }

    // If not supplying data by EVENT_MORE_DATA or EVENT_CAN_WRITE_MORE_DATA, then we're done
    if (mTransfer != TRANSFER_CALLBACK && mTransfer != TRANSFER_SYNC_NOTIF_CALLBACK) {
        return ns;
    }

    // EVENT_MORE_DATA callback handling.
    // Timing for linear pcm audio data formats can be derived directly from the
    // buffer fill level.
    // Timing for compressed data is not directly available from the buffer fill level,
    // rather indirectly from waiting for blocking mode callbacks or waiting for obtain()
    // to return a certain fill level.

    struct timespec timeout;
    const struct timespec *requested = &ClientProxy::kForever;
    if (ns != NS_WHENEVER) {
        timeout.tv_sec = ns / 1000000000LL;
        timeout.tv_nsec = ns % 1000000000LL;
        ALOGV("%s(%d): timeout %ld.%03d",
                __func__, mPortId, timeout.tv_sec, (int) timeout.tv_nsec / 1000000);
        requested = &timeout;
    }

    size_t writtenFrames = 0;
    while (mRemainingFrames > 0) {

        Buffer audioBuffer;
        audioBuffer.frameCount = mRemainingFrames;
        size_t nonContig;
        status_t err = obtainBuffer(&audioBuffer, requested, NULL, &nonContig);
        LOG_ALWAYS_FATAL_IF((err != NO_ERROR) != (audioBuffer.frameCount == 0),
                "%s(%d): obtainBuffer() err=%d frameCount=%zu",
                 __func__, mPortId, err, audioBuffer.frameCount);
        requested = &ClientProxy::kNonBlocking;
        size_t avail = audioBuffer.frameCount + nonContig;
        ALOGV("%s(%d): obtainBuffer(%u) returned %zu = %zu + %zu err %d",
                __func__, mPortId, mRemainingFrames, avail, audioBuffer.frameCount, nonContig, err);
        if (err != NO_ERROR) {
            if (err == TIMED_OUT || err == WOULD_BLOCK || err == -EINTR ||
                    (isOffloaded() && (err == DEAD_OBJECT))) {
                // FIXME bug 25195759
                return 1000000;
            }
            ALOGE("%s(%d): Error %d obtaining an audio buffer, giving up.",
                    __func__, mPortId, err);
            return NS_NEVER;
        }

        if (mRetryOnPartialBuffer && audio_has_proportional_frames(mFormat)) {
            mRetryOnPartialBuffer = false;
            if (avail < mRemainingFrames) {
                if (ns > 0) { // account for obtain time
                    const nsecs_t timeNow = systemTime();
                    ns = max((nsecs_t)0, ns - (timeNow - timeAfterCallbacks));
                }

                // delayNs is first computed by the additional frames required in the buffer.
                nsecs_t delayNs = framesToNanoseconds(
                        mRemainingFrames - avail, sampleRate, speed);

                // afNs is the AudioFlinger mixer period in ns.
                const nsecs_t afNs = framesToNanoseconds(mAfFrameCount, mAfSampleRate, speed);

                // If the AudioTrack is double buffered based on the AudioFlinger mixer period,
                // we may have a race if we wait based on the number of frames desired.
                // This is a possible issue with resampling and AAudio.
                //
                // The granularity of audioflinger processing is one mixer period; if
                // our wait time is less than one mixer period, wait at most half the period.
                if (delayNs < afNs) {
                    delayNs = std::min(delayNs, afNs / 2);
                }

                // adjust our ns wait by delayNs.
                if (ns < 0 /* NS_WHENEVER */ || delayNs < ns) {
                    ns = delayNs;
                }
                return ns;
            }
        }

        size_t reqSize = audioBuffer.size;
        if (mTransfer == TRANSFER_SYNC_NOTIF_CALLBACK) {
            // when notifying client it can write more data, pass the total size that can be
            // written in the next write() call, since it's not passed through the callback
            audioBuffer.size += nonContig;
        }
        mCbf(mTransfer == TRANSFER_CALLBACK ? EVENT_MORE_DATA : EVENT_CAN_WRITE_MORE_DATA,
                mUserData, &audioBuffer);
        size_t writtenSize = audioBuffer.size;

        // Sanity check on returned size
        if (ssize_t(writtenSize) < 0 || writtenSize > reqSize) {
            ALOGE("%s(%d): EVENT_MORE_DATA requested %zu bytes but callback returned %zd bytes",
                    __func__, mPortId, reqSize, ssize_t(writtenSize));
            return NS_NEVER;
        }

        if (writtenSize == 0) {
            if (mTransfer == TRANSFER_SYNC_NOTIF_CALLBACK) {
                // The callback EVENT_CAN_WRITE_MORE_DATA was processed in the JNI of
                // android.media.AudioTrack. The JNI is not using the callback to provide data,
                // it only signals to the Java client that it can provide more data, which
                // this track is read to accept now.
                // The playback thread will be awaken at the next ::write()
                return NS_WHENEVER;
            }
            // The callback is done filling buffers
            // Keep this thread going to handle timed events and
            // still try to get more data in intervals of WAIT_PERIOD_MS
            // but don't just loop and block the CPU, so wait

            // mCbf(EVENT_MORE_DATA, ...) might either
            // (1) Block until it can fill the buffer, returning 0 size on EOS.
            // (2) Block until it can fill the buffer, returning 0 data (silence) on EOS.
            // (3) Return 0 size when no data is available, does not wait for more data.
            //
            // (1) and (2) occurs with AudioPlayer/AwesomePlayer; (3) occurs with NuPlayer.
            // We try to compute the wait time to avoid a tight sleep-wait cycle,
            // especially for case (3).
            //
            // The decision to support (1) and (2) affect the sizing of mRemainingFrames
            // and this loop; whereas for case (3) we could simply check once with the full
            // buffer size and skip the loop entirely.

            nsecs_t myns;
            if (audio_has_proportional_frames(mFormat)) {
                // time to wait based on buffer occupancy
                const nsecs_t datans = mRemainingFrames <= avail ? 0 :
                        framesToNanoseconds(mRemainingFrames - avail, sampleRate, speed);
                // audio flinger thread buffer size (TODO: adjust for fast tracks)
                // FIXME: use mAfFrameCountHAL instead of mAfFrameCount below for fast tracks.
                const nsecs_t afns = framesToNanoseconds(mAfFrameCount, mAfSampleRate, speed);
                // add a half the AudioFlinger buffer time to avoid soaking CPU if datans is 0.
                myns = datans + (afns / 2);
            } else {
                // FIXME: This could ping quite a bit if the buffer isn't full.
                // Note that when mState is stopping we waitStreamEnd, so it never gets here.
                myns = kWaitPeriodNs;
            }
            if (ns > 0) { // account for obtain and callback time
                const nsecs_t timeNow = systemTime();
                ns = max((nsecs_t)0, ns - (timeNow - timeAfterCallbacks));
            }
            if (ns < 0 /* NS_WHENEVER */ || myns < ns) {
                ns = myns;
            }
            return ns;
        }

        size_t releasedFrames = writtenSize / mFrameSize;
        audioBuffer.frameCount = releasedFrames;
        mRemainingFrames -= releasedFrames;
        if (misalignment >= releasedFrames) {
            misalignment -= releasedFrames;
        } else {
            misalignment = 0;
        }

        releaseBuffer(&audioBuffer);
        writtenFrames += releasedFrames;

        // FIXME here is where we would repeat EVENT_MORE_DATA again on same advanced buffer
        // if callback doesn't like to accept the full chunk
        if (writtenSize < reqSize) {
            continue;
        }

        // There could be enough non-contiguous frames available to satisfy the remaining request
        if (mRemainingFrames <= nonContig) {
            continue;
        }

#if 0
        // This heuristic tries to collapse a series of EVENT_MORE_DATA that would total to a
        // sum <= notificationFrames.  It replaces that series by at most two EVENT_MORE_DATA
        // that total to a sum == notificationFrames.
        if (0 < misalignment && misalignment <= mRemainingFrames) {
            mRemainingFrames = misalignment;
            return ((double)mRemainingFrames * 1100000000) / ((double)sampleRate * speed);
        }
#endif

    }
    if (writtenFrames > 0) {
        AutoMutex lock(mLock);
        mFramesWritten += writtenFrames;
    }
    mRemainingFrames = notificationFrames;
    mRetryOnPartialBuffer = true;

    // A lot has transpired since ns was calculated, so run again immediately and re-calculate
    return 0;
}

status_t AudioTrack::restoreTrack_l(const char *from)
{
    ALOGW("%s(%d): dead IAudioTrack, %s, creating a new one from %s()",
            __func__, mPortId, isOffloadedOrDirect_l() ? "Offloaded or Direct" : "PCM", from);
    ++mSequence;

    // refresh the audio configuration cache in this process to make sure we get new
    // output parameters and new IAudioFlinger in createTrack_l()
    AudioSystem::clearAudioConfigCache();

    if (isOffloadedOrDirect_l() || mDoNotReconnect) {
        // FIXME re-creation of offloaded and direct tracks is not yet implemented;
        // reconsider enabling for linear PCM encodings when position can be preserved.
        return DEAD_OBJECT;
    }

    // Save so we can return count since creation.
    mUnderrunCountOffset = getUnderrunCount_l();

    // save the old static buffer position
    uint32_t staticPosition = 0;
    size_t bufferPosition = 0;
    int loopCount = 0;
    if (mStaticProxy != 0) {
        mStaticProxy->getBufferPositionAndLoopCount(&bufferPosition, &loopCount);
        staticPosition = mStaticProxy->getPosition().unsignedValue();
    }

    // See b/74409267. Connecting to a BT A2DP device supporting multiple codecs
    // causes a lot of churn on the service side, and it can reject starting
    // playback of a previously created track. May also apply to other cases.
    const int INITIAL_RETRIES = 3;
    int retries = INITIAL_RETRIES;
retry:
    if (retries < INITIAL_RETRIES) {
        // See the comment for clearAudioConfigCache at the start of the function.
        AudioSystem::clearAudioConfigCache();
    }
    mFlags = mOrigFlags;

    // If a new IAudioTrack is successfully created, createTrack_l() will modify the
    // following member variables: mAudioTrack, mCblkMemory and mCblk.
    // It will also delete the strong references on previous IAudioTrack and IMemory.
    // If a new IAudioTrack cannot be created, the previous (dead) instance will be left intact.
    status_t result = createTrack_l();

    if (result == NO_ERROR) {
        // take the frames that will be lost by track recreation into account in saved position
        // For streaming tracks, this is the amount we obtained from the user/client
        // (not the number actually consumed at the server - those are already lost).
        if (mStaticProxy == 0) {
            mPosition = mReleased;
        }
        // Continue playback from last known position and restore loop.
        if (mStaticProxy != 0) {
            if (loopCount != 0) {
                mStaticProxy->setBufferPositionAndLoop(bufferPosition,
                        mLoopStart, mLoopEnd, loopCount);
            } else {
                mStaticProxy->setBufferPosition(bufferPosition);
                if (bufferPosition == mFrameCount) {
                    ALOGD("%s(%d): restoring track at end of static buffer", __func__, mPortId);
                }
            }
        }
        // restore volume handler
        mVolumeHandler->forall([this](const VolumeShaper &shaper) -> VolumeShaper::Status {
            sp<VolumeShaper::Operation> operationToEnd =
                    new VolumeShaper::Operation(shaper.mOperation);
            // TODO: Ideally we would restore to the exact xOffset position
            // as returned by getVolumeShaperState(), but we don't have that
            // information when restoring at the client unless we periodically poll
            // the server or create shared memory state.
            //
            // For now, we simply advance to the end of the VolumeShaper effect
            // if it has been started.
            if (shaper.isStarted()) {
                operationToEnd->setNormalizedTime(1.f);
            }
            return mAudioTrack->applyVolumeShaper(shaper.mConfiguration, operationToEnd);
        });

        if (mState == STATE_ACTIVE) {
            result = mAudioTrack->start();
        }
        // server resets to zero so we offset
        mFramesWrittenServerOffset =
                mStaticProxy.get() != nullptr ? staticPosition : mFramesWritten;
        mFramesWrittenAtRestore = mFramesWrittenServerOffset;
    }
    if (result != NO_ERROR) {
        ALOGW("%s(%d): failed status %d, retries %d", __func__, mPortId, result, retries);
        if (--retries > 0) {
            // leave time for an eventual race condition to clear before retrying
            usleep(500000);
            goto retry;
        }
        // if no retries left, set invalid bit to force restoring at next occasion
        // and avoid inconsistent active state on client and server sides
        if (mCblk != nullptr) {
            android_atomic_or(CBLK_INVALID, &mCblk->mFlags);
        }
    }
    return result;
}

Modulo<uint32_t> AudioTrack::updateAndGetPosition_l()
{
    // This is the sole place to read server consumed frames
    Modulo<uint32_t> newServer(mProxy->getPosition());
    const int32_t delta = (newServer - mServer).signedValue();
    // TODO There is controversy about whether there can be "negative jitter" in server position.
    //      This should be investigated further, and if possible, it should be addressed.
    //      A more definite failure mode is infrequent polling by client.
    //      One could call (void)getPosition_l() in releaseBuffer(),
    //      so mReleased and mPosition are always lock-step as best possible.
    //      That should ensure delta never goes negative for infrequent polling
    //      unless the server has more than 2^31 frames in its buffer,
    //      in which case the use of uint32_t for these counters has bigger issues.
    ALOGE_IF(delta < 0,
            "%s(%d): detected illegal retrograde motion by the server: mServer advanced by %d",
            __func__, mPortId, delta);
    mServer = newServer;
    if (delta > 0) { // avoid retrograde
        mPosition += delta;
    }
    return mPosition;
}

bool AudioTrack::isSampleRateSpeedAllowed_l(uint32_t sampleRate, float speed)
{
    updateLatency_l();
    // applicable for mixing tracks only (not offloaded or direct)
    if (mStaticProxy != 0) {
        return true; // static tracks do not have issues with buffer sizing.
    }
    const size_t minFrameCount =
            AudioSystem::calculateMinFrameCount(mAfLatency, mAfFrameCount, mAfSampleRate,
                                            sampleRate, speed /*, 0 mNotificationsPerBufferReq*/);
    const bool allowed = mFrameCount >= minFrameCount;
    ALOGD_IF(!allowed,
            "%s(%d): denied "
            "mAfLatency:%u  mAfFrameCount:%zu  mAfSampleRate:%u  sampleRate:%u  speed:%f "
            "mFrameCount:%zu < minFrameCount:%zu",
            __func__, mPortId,
            mAfLatency, mAfFrameCount, mAfSampleRate, sampleRate, speed,
            mFrameCount, minFrameCount);
    return allowed;
}

status_t AudioTrack::setParameters(const String8& keyValuePairs)
{
    AutoMutex lock(mLock);
    return mAudioTrack->setParameters(keyValuePairs);
}

status_t AudioTrack::selectPresentation(int presentationId, int programId)
{
    AutoMutex lock(mLock);
    AudioParameter param = AudioParameter();
    param.addInt(String8(AudioParameter::keyPresentationId), presentationId);
    param.addInt(String8(AudioParameter::keyProgramId), programId);
    ALOGV("%s(%d): PresentationId/ProgramId[%s]",
            __func__, mPortId, param.toString().string());

    return mAudioTrack->setParameters(param.toString());
}

VolumeShaper::Status AudioTrack::applyVolumeShaper(
        const sp<VolumeShaper::Configuration>& configuration,
        const sp<VolumeShaper::Operation>& operation)
{
    AutoMutex lock(mLock);
    mVolumeHandler->setIdIfNecessary(configuration);
    VolumeShaper::Status status = mAudioTrack->applyVolumeShaper(configuration, operation);

    if (status == DEAD_OBJECT) {
        if (restoreTrack_l("applyVolumeShaper") == OK) {
            status = mAudioTrack->applyVolumeShaper(configuration, operation);
        }
    }
    if (status >= 0) {
        // save VolumeShaper for restore
        mVolumeHandler->applyVolumeShaper(configuration, operation);
        if (mState == STATE_ACTIVE || mState == STATE_STOPPING) {
            mVolumeHandler->setStarted();
        }
    } else {
        // warn only if not an expected restore failure.
        ALOGW_IF(!((isOffloadedOrDirect_l() || mDoNotReconnect) && status == DEAD_OBJECT),
                "%s(%d): applyVolumeShaper failed: %d", __func__, mPortId, status);
    }
    return status;
}

sp<VolumeShaper::State> AudioTrack::getVolumeShaperState(int id)
{
    AutoMutex lock(mLock);
    sp<VolumeShaper::State> state = mAudioTrack->getVolumeShaperState(id);
    if (state.get() == nullptr && (mCblk->mFlags & CBLK_INVALID) != 0) {
        if (restoreTrack_l("getVolumeShaperState") == OK) {
            state = mAudioTrack->getVolumeShaperState(id);
        }
    }
    return state;
}

status_t AudioTrack::getTimestamp(ExtendedTimestamp *timestamp)
{
    if (timestamp == nullptr) {
        return BAD_VALUE;
    }
    AutoMutex lock(mLock);
    return getTimestamp_l(timestamp);
}

status_t AudioTrack::getTimestamp_l(ExtendedTimestamp *timestamp)
{
    if (mCblk->mFlags & CBLK_INVALID) {
        const status_t status = restoreTrack_l("getTimestampExtended");
        if (status != OK) {
            // per getTimestamp() API doc in header, we return DEAD_OBJECT here,
            // recommending that the track be recreated.
            return DEAD_OBJECT;
        }
    }
    // check for offloaded/direct here in case restoring somehow changed those flags.
    if (isOffloadedOrDirect_l()) {
        return INVALID_OPERATION; // not supported
    }
    status_t status = mProxy->getTimestamp(timestamp);
    LOG_ALWAYS_FATAL_IF(status != OK, "%s(%d): status %d not allowed from proxy getTimestamp",
            __func__, mPortId, status);
    bool found = false;
    timestamp->mPosition[ExtendedTimestamp::LOCATION_CLIENT] = mFramesWritten;
    timestamp->mTimeNs[ExtendedTimestamp::LOCATION_CLIENT] = 0;
    // server side frame offset in case AudioTrack has been restored.
    for (int i = ExtendedTimestamp::LOCATION_SERVER;
            i < ExtendedTimestamp::LOCATION_MAX; ++i) {
        if (timestamp->mTimeNs[i] >= 0) {
            // apply server offset (frames flushed is ignored
            // so we don't report the jump when the flush occurs).
            timestamp->mPosition[i] += mFramesWrittenServerOffset;
            found = true;
        }
    }
    return found ? OK : WOULD_BLOCK;
}

status_t AudioTrack::getTimestamp(AudioTimestamp& timestamp)
{
    AutoMutex lock(mLock);
    return getTimestamp_l(timestamp);
}

status_t AudioTrack::getTimestamp_l(AudioTimestamp& timestamp)
{
    bool previousTimestampValid = mPreviousTimestampValid;
    // Set false here to cover all the error return cases.
    mPreviousTimestampValid = false;

    switch (mState) {
    case STATE_ACTIVE:
    case STATE_PAUSED:
        break; // handle below
    case STATE_FLUSHED:
    case STATE_STOPPED:
        return WOULD_BLOCK;
    case STATE_STOPPING:
    case STATE_PAUSED_STOPPING:
        if (!isOffloaded_l()) {
            return INVALID_OPERATION;
        }
        break; // offloaded tracks handled below
    default:
        LOG_ALWAYS_FATAL("%s(%d): Invalid mState in getTimestamp(): %d",
               __func__, mPortId, mState);
        break;
    }

    if (mCblk->mFlags & CBLK_INVALID) {
        const status_t status = restoreTrack_l("getTimestamp");
        if (status != OK) {
            // per getTimestamp() API doc in header, we return DEAD_OBJECT here,
            // recommending that the track be recreated.
            return DEAD_OBJECT;
        }
    }

    // The presented frame count must always lag behind the consumed frame count.
    // To avoid a race, read the presented frames first.  This ensures that presented <= consumed.

    status_t status;
    if (isOffloadedOrDirect_l()) {
        // use Binder to get timestamp
        status = mAudioTrack->getTimestamp(timestamp);
    } else {
        // read timestamp from shared memory
        ExtendedTimestamp ets;
        status = mProxy->getTimestamp(&ets);
        if (status == OK) {
            ExtendedTimestamp::Location location;
            status = ets.getBestTimestamp(&timestamp, &location);

            if (status == OK) {
                updateLatency_l();
                // It is possible that the best location has moved from the kernel to the server.
                // In this case we adjust the position from the previous computed latency.
                if (location == ExtendedTimestamp::LOCATION_SERVER) {
                    ALOGW_IF(mPreviousLocation == ExtendedTimestamp::LOCATION_KERNEL,
                            "%s(%d): location moved from kernel to server",
                            __func__, mPortId);
                    // check that the last kernel OK time info exists and the positions
                    // are valid (if they predate the current track, the positions may
                    // be zero or negative).
                    const int64_t frames =
                            (ets.mTimeNs[ExtendedTimestamp::LOCATION_SERVER_LASTKERNELOK] < 0 ||
                            ets.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL_LASTKERNELOK] < 0 ||
                            ets.mPosition[ExtendedTimestamp::LOCATION_SERVER_LASTKERNELOK] <= 0 ||
                            ets.mPosition[ExtendedTimestamp::LOCATION_KERNEL_LASTKERNELOK] <= 0)
                            ?
                            int64_t((double)mAfLatency * mSampleRate * mPlaybackRate.mSpeed
                                    / 1000)
                            :
                            (ets.mPosition[ExtendedTimestamp::LOCATION_SERVER_LASTKERNELOK]
                            - ets.mPosition[ExtendedTimestamp::LOCATION_KERNEL_LASTKERNELOK]);
                    ALOGV("%s(%d): frame adjustment:%lld  timestamp:%s",
                            __func__, mPortId, (long long)frames, ets.toString().c_str());
                    if (frames >= ets.mPosition[location]) {
                        timestamp.mPosition = 0;
                    } else {
                        timestamp.mPosition = (uint32_t)(ets.mPosition[location] - frames);
                    }
                } else if (location == ExtendedTimestamp::LOCATION_KERNEL) {
                    ALOGV_IF(mPreviousLocation == ExtendedTimestamp::LOCATION_SERVER,
                            "%s(%d): location moved from server to kernel",
                            __func__, mPortId);

                    if (ets.mPosition[ExtendedTimestamp::LOCATION_SERVER] ==
                            ets.mPosition[ExtendedTimestamp::LOCATION_KERNEL]) {
                        // In Q, we don't return errors as an invalid time
                        // but instead we leave the last kernel good timestamp alone.
                        //
                        // If server is identical to kernel, the device data pipeline is idle.
                        // A better start time is now.  The retrograde check ensures
                        // timestamp monotonicity.
                        const int64_t nowNs = systemTime();
                        if (!mTimestampStallReported) {
                            ALOGD("%s(%d): device stall time corrected using current time %lld",
                                    __func__, mPortId, (long long)nowNs);
                            mTimestampStallReported = true;
                        }
                        timestamp.mTime = convertNsToTimespec(nowNs);
                    }  else {
                        mTimestampStallReported = false;
                    }
                }

                // We update the timestamp time even when paused.
                if (mState == STATE_PAUSED /* not needed: STATE_PAUSED_STOPPING */) {
                    const int64_t now = systemTime();
                    const int64_t at = audio_utils_ns_from_timespec(&timestamp.mTime);
                    const int64_t lag =
                            (ets.mTimeNs[ExtendedTimestamp::LOCATION_SERVER_LASTKERNELOK] < 0 ||
                                ets.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL_LASTKERNELOK] < 0)
                            ? int64_t(mAfLatency * 1000000LL)
                            : (ets.mPosition[ExtendedTimestamp::LOCATION_SERVER_LASTKERNELOK]
                             - ets.mPosition[ExtendedTimestamp::LOCATION_KERNEL_LASTKERNELOK])
                             * NANOS_PER_SECOND / mSampleRate;
                    const int64_t limit = now - lag; // no earlier than this limit
                    if (at < limit) {
                        ALOGV("timestamp pause lag:%lld adjusting from %lld to %lld",
                                (long long)lag, (long long)at, (long long)limit);
                        timestamp.mTime = convertNsToTimespec(limit);
                    }
                }
                mPreviousLocation = location;
            } else {
                // right after AudioTrack is started, one may not find a timestamp
                ALOGV("%s(%d): getBestTimestamp did not find timestamp", __func__, mPortId);
            }
        }
        if (status == INVALID_OPERATION) {
            // INVALID_OPERATION occurs when no timestamp has been issued by the server;
            // other failures are signaled by a negative time.
            // If we come out of FLUSHED or STOPPED where the position is known
            // to be zero we convert this to WOULD_BLOCK (with the implicit meaning of
            // "zero" for NuPlayer).  We don't convert for track restoration as position
            // does not reset.
            ALOGV("%s(%d): timestamp server offset:%lld restore frames:%lld",
                    __func__, mPortId,
                    (long long)mFramesWrittenServerOffset, (long long)mFramesWrittenAtRestore);
            if (mFramesWrittenServerOffset != mFramesWrittenAtRestore) {
                status = WOULD_BLOCK;
            }
        }
    }
    if (status != NO_ERROR) {
        ALOGV_IF(status != WOULD_BLOCK, "%s(%d): getTimestamp error:%#x", __func__, mPortId, status);
        return status;
    }
    if (isOffloadedOrDirect_l()) {
        if (isOffloaded_l() && (mState == STATE_PAUSED || mState == STATE_PAUSED_STOPPING)) {
            // use cached paused position in case another offloaded track is running.
            timestamp.mPosition = mPausedPosition;
            clock_gettime(CLOCK_MONOTONIC, &timestamp.mTime);
            // TODO: adjust for delay
            return NO_ERROR;
        }

        // Check whether a pending flush or stop has completed, as those commands may
        // be asynchronous or return near finish or exhibit glitchy behavior.
        //
        // Originally this showed up as the first timestamp being a continuation of
        // the previous song under gapless playback.
        // However, we sometimes see zero timestamps, then a glitch of
        // the previous song's position, and then correct timestamps afterwards.
        if (mStartFromZeroUs != 0 && mSampleRate != 0) {
            static const int kTimeJitterUs = 100000; // 100 ms
            static const int k1SecUs = 1000000;

            const int64_t timeNow = getNowUs();

            if (timeNow < mStartFromZeroUs + k1SecUs) { // within first second of starting
                const int64_t timestampTimeUs = convertTimespecToUs(timestamp.mTime);
                if (timestampTimeUs < mStartFromZeroUs) {
                    return WOULD_BLOCK;  // stale timestamp time, occurs before start.
                }
                const int64_t deltaTimeUs = timestampTimeUs - mStartFromZeroUs;
                const int64_t deltaPositionByUs = (double)timestamp.mPosition * 1000000
                        / ((double)mSampleRate * mPlaybackRate.mSpeed);

                if (deltaPositionByUs > deltaTimeUs + kTimeJitterUs) {
                    // Verify that the counter can't count faster than the sample rate
                    // since the start time.  If greater, then that means we may have failed
                    // to completely flush or stop the previous playing track.
                    ALOGW_IF(!mTimestampStartupGlitchReported,
                            "%s(%d): startup glitch detected"
                            " deltaTimeUs(%lld) deltaPositionUs(%lld) tsmPosition(%u)",
                            __func__, mPortId,
                            (long long)deltaTimeUs, (long long)deltaPositionByUs,
                            timestamp.mPosition);
                    mTimestampStartupGlitchReported = true;
                    if (previousTimestampValid
                            && mPreviousTimestamp.mPosition == 0 /* should be true if valid */) {
                        timestamp = mPreviousTimestamp;
                        mPreviousTimestampValid = true;
                        return NO_ERROR;
                    }
                    return WOULD_BLOCK;
                }
                if (deltaPositionByUs != 0) {
                    mStartFromZeroUs = 0; // don't check again, we got valid nonzero position.
                }
            } else {
                mStartFromZeroUs = 0; // don't check again, start time expired.
            }
            mTimestampStartupGlitchReported = false;
        }
    } else {
        // Update the mapping between local consumed (mPosition) and server consumed (mServer)
        (void) updateAndGetPosition_l();
        // Server consumed (mServer) and presented both use the same server time base,
        // and server consumed is always >= presented.
        // The delta between these represents the number of frames in the buffer pipeline.
        // If this delta between these is greater than the client position, it means that
        // actually presented is still stuck at the starting line (figuratively speaking),
        // waiting for the first frame to go by.  So we can't report a valid timestamp yet.
        // Note: We explicitly use non-Modulo comparison here - potential wrap issue when
        // mPosition exceeds 32 bits.
        // TODO Remove when timestamp is updated to contain pipeline status info.
        const int32_t pipelineDepthInFrames = (mServer - timestamp.mPosition).signedValue();
        if (pipelineDepthInFrames > 0 /* should be true, but we check anyways */
                && (uint32_t)pipelineDepthInFrames > mPosition.value()) {
            return INVALID_OPERATION;
        }
        // Convert timestamp position from server time base to client time base.
        // TODO The following code should work OK now because timestamp.mPosition is 32-bit.
        // But if we change it to 64-bit then this could fail.
        // Use Modulo computation here.
        timestamp.mPosition = (mPosition - mServer + timestamp.mPosition).value();
        // Immediately after a call to getPosition_l(), mPosition and
        // mServer both represent the same frame position.  mPosition is
        // in client's point of view, and mServer is in server's point of
        // view.  So the difference between them is the "fudge factor"
        // between client and server views due to stop() and/or new
        // IAudioTrack.  And timestamp.mPosition is initially in server's
        // point of view, so we need to apply the same fudge factor to it.
    }

    // Prevent retrograde motion in timestamp.
    // This is sometimes caused by erratic reports of the available space in the ALSA drivers.
    if (status == NO_ERROR) {
        // Fix stale time when checking timestamp right after start().
        // The position is at the last reported location but the time can be stale
        // due to pause or standby or cold start latency.
        //
        // We keep advancing the time (but not the position) to ensure that the
        // stale value does not confuse the application.
        //
        // For offload compatibility, use a default lag value here.
        // Any time discrepancy between this update and the pause timestamp is handled
        // by the retrograde check afterwards.
        int64_t currentTimeNanos = audio_utils_ns_from_timespec(&timestamp.mTime);
        const int64_t lagNs = int64_t(mAfLatency * 1000000LL);
        const int64_t limitNs = mStartNs - lagNs;
        if (currentTimeNanos < limitNs) {
            if (!mTimestampStaleTimeReported) {
                ALOGD("%s(%d): stale timestamp time corrected, "
                        "currentTimeNanos: %lld < limitNs: %lld < mStartNs: %lld",
                        __func__, mPortId,
                        (long long)currentTimeNanos, (long long)limitNs, (long long)mStartNs);
                mTimestampStaleTimeReported = true;
            }
            timestamp.mTime = convertNsToTimespec(limitNs);
            currentTimeNanos = limitNs;
        } else {
            mTimestampStaleTimeReported = false;
        }

        // previousTimestampValid is set to false when starting after a stop or flush.
        if (previousTimestampValid) {
            const int64_t previousTimeNanos =
                    audio_utils_ns_from_timespec(&mPreviousTimestamp.mTime);

            // retrograde check
            if (currentTimeNanos < previousTimeNanos) {
                if (!mTimestampRetrogradeTimeReported) {
                    ALOGW("%s(%d): retrograde timestamp time corrected, %lld < %lld",
                            __func__, mPortId,
                            (long long)currentTimeNanos, (long long)previousTimeNanos);
                    mTimestampRetrogradeTimeReported = true;
                }
                timestamp.mTime = mPreviousTimestamp.mTime;
            } else {
                mTimestampRetrogradeTimeReported = false;
            }

            // Looking at signed delta will work even when the timestamps
            // are wrapping around.
            int32_t deltaPosition = (Modulo<uint32_t>(timestamp.mPosition)
                    - mPreviousTimestamp.mPosition).signedValue();
            if (deltaPosition < 0) {
                // Only report once per position instead of spamming the log.
                if (!mTimestampRetrogradePositionReported) {
                    ALOGW("%s(%d): retrograde timestamp position corrected, %d = %u - %u",
                            __func__, mPortId,
                            deltaPosition,
                            timestamp.mPosition,
                            mPreviousTimestamp.mPosition);
                    mTimestampRetrogradePositionReported = true;
                }
            } else {
                mTimestampRetrogradePositionReported = false;
            }
            if (deltaPosition < 0) {
                timestamp.mPosition = mPreviousTimestamp.mPosition;
                deltaPosition = 0;
            }
#if 0
            // Uncomment this to verify audio timestamp rate.
            const int64_t deltaTime =
                    audio_utils_ns_from_timespec(&timestamp.mTime) - previousTimeNanos;
            if (deltaTime != 0) {
                const int64_t computedSampleRate =
                        deltaPosition * (long long)NANOS_PER_SECOND / deltaTime;
                ALOGD("%s(%d): computedSampleRate:%u  sampleRate:%u",
                        __func__, mPortId,
                        (unsigned)computedSampleRate, mSampleRate);
            }
#endif
        }
        mPreviousTimestamp = timestamp;
        mPreviousTimestampValid = true;
    }

    return status;
}

String8 AudioTrack::getParameters(const String8& keys)
{
    audio_io_handle_t output = getOutput();
    if (output != AUDIO_IO_HANDLE_NONE) {
        return AudioSystem::getParameters(output, keys);
    } else {
        return String8::empty();
    }
}

bool AudioTrack::isOffloaded() const
{
    AutoMutex lock(mLock);
    return isOffloaded_l();
}

bool AudioTrack::isDirect() const
{
    AutoMutex lock(mLock);
    return isDirect_l();
}

bool AudioTrack::isOffloadedOrDirect() const
{
    AutoMutex lock(mLock);
    return isOffloadedOrDirect_l();
}


status_t AudioTrack::dump(int fd, const Vector<String16>& args __unused) const
{
    String8 result;

    result.append(" AudioTrack::dump\n");
    result.appendFormat("  id(%d) status(%d), state(%d), session Id(%d), flags(%#x)\n",
                        mPortId, mStatus, mState, mSessionId, mFlags);
    result.appendFormat("  stream type(%d), left - right volume(%f, %f)\n",
                        (mStreamType == AUDIO_STREAM_DEFAULT) ?
                            AudioSystem::attributesToStreamType(mAttributes) :
                            mStreamType,
                        mVolume[AUDIO_INTERLEAVE_LEFT], mVolume[AUDIO_INTERLEAVE_RIGHT]);
    result.appendFormat("  format(%#x), channel mask(%#x), channel count(%u)\n",
                  mFormat, mChannelMask, mChannelCount);
    result.appendFormat("  sample rate(%u), original sample rate(%u), speed(%f)\n",
                  mSampleRate, mOriginalSampleRate, mPlaybackRate.mSpeed);
    result.appendFormat("  frame count(%zu), req. frame count(%zu)\n",
                  mFrameCount, mReqFrameCount);
    result.appendFormat("  notif. frame count(%u), req. notif. frame count(%u),"
            " req. notif. per buff(%u)\n",
             mNotificationFramesAct, mNotificationFramesReq, mNotificationsPerBufferReq);
    result.appendFormat("  latency (%d), selected device Id(%d), routed device Id(%d)\n",
                        mLatency, mSelectedDeviceId, mRoutedDeviceId);
    result.appendFormat("  output(%d) AF latency (%u) AF frame count(%zu) AF SampleRate(%u)\n",
                        mOutput, mAfLatency, mAfFrameCount, mAfSampleRate);
    ::write(fd, result.string(), result.size());
    return NO_ERROR;
}

uint32_t AudioTrack::getUnderrunCount() const
{
    AutoMutex lock(mLock);
    return getUnderrunCount_l();
}

uint32_t AudioTrack::getUnderrunCount_l() const
{
    return mProxy->getUnderrunCount() + mUnderrunCountOffset;
}

uint32_t AudioTrack::getUnderrunFrames() const
{
    AutoMutex lock(mLock);
    return mProxy->getUnderrunFrames();
}

status_t AudioTrack::addAudioDeviceCallback(const sp<AudioSystem::AudioDeviceCallback>& callback)
{

    if (callback == 0) {
        ALOGW("%s(%d): adding NULL callback!", __func__, mPortId);
        return BAD_VALUE;
    }
    AutoMutex lock(mLock);
    if (mDeviceCallback.unsafe_get() == callback.get()) {
        ALOGW("%s(%d): adding same callback!", __func__, mPortId);
        return INVALID_OPERATION;
    }
    status_t status = NO_ERROR;
    if (mOutput != AUDIO_IO_HANDLE_NONE) {
        if (mDeviceCallback != 0) {
            ALOGW("%s(%d): callback already present!", __func__, mPortId);
            AudioSystem::removeAudioDeviceCallback(this, mOutput, mPortId);
        }
        status = AudioSystem::addAudioDeviceCallback(this, mOutput, mPortId);
    }
    mDeviceCallback = callback;
    return status;
}

status_t AudioTrack::removeAudioDeviceCallback(
        const sp<AudioSystem::AudioDeviceCallback>& callback)
{
    if (callback == 0) {
        ALOGW("%s(%d): removing NULL callback!", __func__, mPortId);
        return BAD_VALUE;
    }
    AutoMutex lock(mLock);
    if (mDeviceCallback.unsafe_get() != callback.get()) {
        ALOGW("%s removing different callback!", __FUNCTION__);
        return INVALID_OPERATION;
    }
    mDeviceCallback.clear();
    if (mOutput != AUDIO_IO_HANDLE_NONE) {
        AudioSystem::removeAudioDeviceCallback(this, mOutput, mPortId);
    }
    return NO_ERROR;
}


void AudioTrack::onAudioDeviceUpdate(audio_io_handle_t audioIo,
                                 audio_port_handle_t deviceId)
{
    sp<AudioSystem::AudioDeviceCallback> callback;
    {
        AutoMutex lock(mLock);
        if (audioIo != mOutput) {
            return;
        }
        callback = mDeviceCallback.promote();
        // only update device if the track is active as route changes due to other use cases are
        // irrelevant for this client
        if (mState == STATE_ACTIVE) {
            mRoutedDeviceId = deviceId;
        }
    }

    if (callback.get() != nullptr) {
        callback->onAudioDeviceUpdate(mOutput, mRoutedDeviceId);
    }
}

status_t AudioTrack::pendingDuration(int32_t *msec, ExtendedTimestamp::Location location)
{
    if (msec == nullptr ||
            (location != ExtendedTimestamp::LOCATION_SERVER
                    && location != ExtendedTimestamp::LOCATION_KERNEL)) {
        return BAD_VALUE;
    }
    AutoMutex lock(mLock);
    // inclusive of offloaded and direct tracks.
    //
    // It is possible, but not enabled, to allow duration computation for non-pcm
    // audio_has_proportional_frames() formats because currently they have
    // the drain rate equivalent to the pcm sample rate * framesize.
    if (!isPurePcmData_l()) {
        return INVALID_OPERATION;
    }
    ExtendedTimestamp ets;
    if (getTimestamp_l(&ets) == OK
            && ets.mTimeNs[location] > 0) {
        int64_t diff = ets.mPosition[ExtendedTimestamp::LOCATION_CLIENT]
                - ets.mPosition[location];
        if (diff < 0) {
            *msec = 0;
        } else {
            // ms is the playback time by frames
            int64_t ms = (int64_t)((double)diff * 1000 /
                    ((double)mSampleRate * mPlaybackRate.mSpeed));
            // clockdiff is the timestamp age (negative)
            int64_t clockdiff = (mState != STATE_ACTIVE) ? 0 :
                    ets.mTimeNs[location]
                    + ets.mTimebaseOffset[ExtendedTimestamp::TIMEBASE_MONOTONIC]
                    - systemTime(SYSTEM_TIME_MONOTONIC);

            //ALOGV("ms: %lld  clockdiff: %lld", (long long)ms, (long long)clockdiff);
            static const int NANOS_PER_MILLIS = 1000000;
            *msec = (int32_t)(ms + clockdiff / NANOS_PER_MILLIS);
        }
        return NO_ERROR;
    }
    if (location != ExtendedTimestamp::LOCATION_SERVER) {
        return INVALID_OPERATION; // LOCATION_KERNEL is not available
    }
    // use server position directly (offloaded and direct arrive here)
    updateAndGetPosition_l();
    int32_t diff = (Modulo<uint32_t>(mFramesWritten) - mPosition).signedValue();
    *msec = (diff <= 0) ? 0
            : (int32_t)((double)diff * 1000 / ((double)mSampleRate * mPlaybackRate.mSpeed));
    return NO_ERROR;
}

bool AudioTrack::hasStarted()
{
    AutoMutex lock(mLock);
    switch (mState) {
    case STATE_STOPPED:
        if (isOffloadedOrDirect_l()) {
            // check if we have started in the past to return true.
            return mStartFromZeroUs > 0;
        }
        // A normal audio track may still be draining, so
        // check if stream has ended.  This covers fasttrack position
        // instability and start/stop without any data written.
        if (mProxy->getStreamEndDone()) {
            return true;
        }
        FALLTHROUGH_INTENDED;
    case STATE_ACTIVE:
    case STATE_STOPPING:
        break;
    case STATE_PAUSED:
    case STATE_PAUSED_STOPPING:
    case STATE_FLUSHED:
        return false;  // we're not active
    default:
        LOG_ALWAYS_FATAL("%s(%d): Invalid mState in hasStarted(): %d", __func__, mPortId, mState);
        break;
    }

    // wait indicates whether we need to wait for a timestamp.
    // This is conservatively figured - if we encounter an unexpected error
    // then we will not wait.
    bool wait = false;
    if (isOffloadedOrDirect_l()) {
        AudioTimestamp ts;
        status_t status = getTimestamp_l(ts);
        if (status == WOULD_BLOCK) {
            wait = true;
        } else if (status == OK) {
            wait = (ts.mPosition == 0 || ts.mPosition == mStartTs.mPosition);
        }
        ALOGV("%s(%d): hasStarted wait:%d  ts:%u  start position:%lld",
                __func__, mPortId,
                (int)wait,
                ts.mPosition,
                (long long)mStartTs.mPosition);
    } else {
        int location = ExtendedTimestamp::LOCATION_SERVER; // for ALOG
        ExtendedTimestamp ets;
        status_t status = getTimestamp_l(&ets);
        if (status == WOULD_BLOCK) {  // no SERVER or KERNEL frame info in ets
            wait = true;
        } else if (status == OK) {
            for (location = ExtendedTimestamp::LOCATION_KERNEL;
                    location >= ExtendedTimestamp::LOCATION_SERVER; --location) {
                if (ets.mTimeNs[location] < 0 || mStartEts.mTimeNs[location] < 0) {
                    continue;
                }
                wait = ets.mPosition[location] == 0
                        || ets.mPosition[location] == mStartEts.mPosition[location];
                break;
            }
        }
        ALOGV("%s(%d): hasStarted wait:%d  ets:%lld  start position:%lld",
                __func__, mPortId,
                (int)wait,
                (long long)ets.mPosition[location],
                (long long)mStartEts.mPosition[location]);
    }
    return !wait;
}

// =========================================================================

void AudioTrack::DeathNotifier::binderDied(const wp<IBinder>& who __unused)
{
    sp<AudioTrack> audioTrack = mAudioTrack.promote();
    if (audioTrack != 0) {
        AutoMutex lock(audioTrack->mLock);
        audioTrack->mProxy->binderDied();
    }
}

// =========================================================================

AudioTrack::AudioTrackThread::AudioTrackThread(AudioTrack& receiver)
    : Thread(true /* bCanCallJava */)  // binder recursion on restoreTrack_l() may call Java.
    , mReceiver(receiver), mPaused(true), mPausedInt(false), mPausedNs(0LL),
      mIgnoreNextPausedInt(false)
{
}

AudioTrack::AudioTrackThread::~AudioTrackThread()
{
}

bool AudioTrack::AudioTrackThread::threadLoop()
{
    {
        AutoMutex _l(mMyLock);
        if (mPaused) {
            // TODO check return value and handle or log
            mMyCond.wait(mMyLock);
            // caller will check for exitPending()
            return true;
        }
        if (mIgnoreNextPausedInt) {
            mIgnoreNextPausedInt = false;
            mPausedInt = false;
        }
        if (mPausedInt) {
            // TODO use futex instead of condition, for event flag "or"
            if (mPausedNs > 0) {
                // TODO check return value and handle or log
                (void) mMyCond.waitRelative(mMyLock, mPausedNs);
            } else {
                // TODO check return value and handle or log
                mMyCond.wait(mMyLock);
            }
            mPausedInt = false;
            return true;
        }
    }
    if (exitPending()) {
        return false;
    }
    nsecs_t ns = mReceiver.processAudioBuffer();
    switch (ns) {
    case 0:
        return true;
    case NS_INACTIVE:
        pauseInternal();
        return true;
    case NS_NEVER:
        return false;
    case NS_WHENEVER:
        // Event driven: call wake() when callback notifications conditions change.
        ns = INT64_MAX;
        FALLTHROUGH_INTENDED;
    default:
        LOG_ALWAYS_FATAL_IF(ns < 0, "%s(%d): processAudioBuffer() returned %lld",
                __func__, mReceiver.mPortId, (long long)ns);
        pauseInternal(ns);
        return true;
    }
}

void AudioTrack::AudioTrackThread::requestExit()
{
    // must be in this order to avoid a race condition
    Thread::requestExit();
    resume();
}

void AudioTrack::AudioTrackThread::pause()
{
    AutoMutex _l(mMyLock);
    mPaused = true;
}

void AudioTrack::AudioTrackThread::resume()
{
    AutoMutex _l(mMyLock);
    mIgnoreNextPausedInt = true;
    if (mPaused || mPausedInt) {
        mPaused = false;
        mPausedInt = false;
        mMyCond.signal();
    }
}

void AudioTrack::AudioTrackThread::wake()
{
    AutoMutex _l(mMyLock);
    if (!mPaused) {
        // wake() might be called while servicing a callback - ignore the next
        // pause time and call processAudioBuffer.
        mIgnoreNextPausedInt = true;
        if (mPausedInt && mPausedNs > 0) {
            // audio track is active and internally paused with timeout.
            mPausedInt = false;
            mMyCond.signal();
        }
    }
}

void AudioTrack::AudioTrackThread::pauseInternal(nsecs_t ns)
{
    AutoMutex _l(mMyLock);
    mPausedInt = true;
    mPausedNs = ns;
}

} // namespace android
