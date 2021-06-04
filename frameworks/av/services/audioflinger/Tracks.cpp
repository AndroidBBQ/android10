/*
**
** Copyright 2012, The Android Open Source Project
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


#define LOG_TAG "AudioFlinger"
//#define LOG_NDEBUG 0

#include "Configuration.h"
#include <linux/futex.h>
#include <math.h>
#include <sys/syscall.h>
#include <utils/Log.h>

#include <private/media/AudioTrackShared.h>

#include "AudioFlinger.h"

#include <media/nbaio/Pipe.h>
#include <media/nbaio/PipeReader.h>
#include <media/RecordBufferConverter.h>
#include <mediautils/ServiceUtilities.h>
#include <audio_utils/minifloat.h>

// ----------------------------------------------------------------------------

// Note: the following macro is used for extremely verbose logging message.  In
// order to run with ALOG_ASSERT turned on, we need to have LOG_NDEBUG set to
// 0; but one side effect of this is to turn all LOGV's as well.  Some messages
// are so verbose that we want to suppress them even when we have ALOG_ASSERT
// turned on.  Do not uncomment the #def below unless you really know what you
// are doing and want to see all of the extremely verbose messages.
//#define VERY_VERY_VERBOSE_LOGGING
#ifdef VERY_VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
#else
#define ALOGVV(a...) do { } while(0)
#endif

namespace android {

using media::VolumeShaper;
// ----------------------------------------------------------------------------
//      TrackBase
// ----------------------------------------------------------------------------
#undef LOG_TAG
#define LOG_TAG "AF::TrackBase"

static volatile int32_t nextTrackId = 55;

// TrackBase constructor must be called with AudioFlinger::mLock held
AudioFlinger::ThreadBase::TrackBase::TrackBase(
            ThreadBase *thread,
            const sp<Client>& client,
            const audio_attributes_t& attr,
            uint32_t sampleRate,
            audio_format_t format,
            audio_channel_mask_t channelMask,
            size_t frameCount,
            void *buffer,
            size_t bufferSize,
            audio_session_t sessionId,
            pid_t creatorPid,
            uid_t clientUid,
            bool isOut,
            alloc_type alloc,
            track_type type,
            audio_port_handle_t portId)
    :   RefBase(),
        mThread(thread),
        mClient(client),
        mCblk(NULL),
        // mBuffer, mBufferSize
        mState(IDLE),
        mAttr(attr),
        mSampleRate(sampleRate),
        mFormat(format),
        mChannelMask(channelMask),
        mChannelCount(isOut ?
                audio_channel_count_from_out_mask(channelMask) :
                audio_channel_count_from_in_mask(channelMask)),
        mFrameSize(audio_has_proportional_frames(format) ?
                mChannelCount * audio_bytes_per_sample(format) : sizeof(int8_t)),
        mFrameCount(frameCount),
        mSessionId(sessionId),
        mIsOut(isOut),
        mId(android_atomic_inc(&nextTrackId)),
        mTerminated(false),
        mType(type),
        mThreadIoHandle(thread ? thread->id() : AUDIO_IO_HANDLE_NONE),
        mPortId(portId),
        mIsInvalid(false),
        mCreatorPid(creatorPid)
{
    const uid_t callingUid = IPCThreadState::self()->getCallingUid();
    if (!isAudioServerOrMediaServerUid(callingUid) || clientUid == AUDIO_UID_INVALID) {
        ALOGW_IF(clientUid != AUDIO_UID_INVALID && clientUid != callingUid,
                "%s(%d): uid %d tried to pass itself off as %d",
                 __func__, mId, callingUid, clientUid);
        clientUid = callingUid;
    }
    // clientUid contains the uid of the app that is responsible for this track, so we can blame
    // battery usage on it.
    mUid = clientUid;

    // ALOGD("Creating track with %d buffers @ %d bytes", bufferCount, bufferSize);

    size_t minBufferSize = buffer == NULL ? roundup(frameCount) : frameCount;
    // check overflow when computing bufferSize due to multiplication by mFrameSize.
    if (minBufferSize < frameCount  // roundup rounds down for values above UINT_MAX / 2
            || mFrameSize == 0   // format needs to be correct
            || minBufferSize > SIZE_MAX / mFrameSize) {
        android_errorWriteLog(0x534e4554, "34749571");
        return;
    }
    minBufferSize *= mFrameSize;

    if (buffer == nullptr) {
        bufferSize = minBufferSize; // allocated here.
    } else if (minBufferSize > bufferSize) {
        android_errorWriteLog(0x534e4554, "38340117");
        return;
    }

    size_t size = sizeof(audio_track_cblk_t);
    if (buffer == NULL && alloc == ALLOC_CBLK) {
        // check overflow when computing allocation size for streaming tracks.
        if (size > SIZE_MAX - bufferSize) {
            android_errorWriteLog(0x534e4554, "34749571");
            return;
        }
        size += bufferSize;
    }

    if (client != 0) {
        mCblkMemory = client->heap()->allocate(size);
        if (mCblkMemory == 0 ||
                (mCblk = static_cast<audio_track_cblk_t *>(mCblkMemory->pointer())) == NULL) {
            ALOGE("%s(%d): not enough memory for AudioTrack size=%zu", __func__, mId, size);
            client->heap()->dump("AudioTrack");
            mCblkMemory.clear();
            return;
        }
    } else {
        mCblk = (audio_track_cblk_t *) malloc(size);
        if (mCblk == NULL) {
            ALOGE("%s(%d): not enough memory for AudioTrack size=%zu", __func__, mId, size);
            return;
        }
    }

    // construct the shared structure in-place.
    if (mCblk != NULL) {
        new(mCblk) audio_track_cblk_t();
        switch (alloc) {
        case ALLOC_READONLY: {
            const sp<MemoryDealer> roHeap(thread->readOnlyHeap());
            if (roHeap == 0 ||
                    (mBufferMemory = roHeap->allocate(bufferSize)) == 0 ||
                    (mBuffer = mBufferMemory->pointer()) == NULL) {
                ALOGE("%s(%d): not enough memory for read-only buffer size=%zu",
                        __func__, mId, bufferSize);
                if (roHeap != 0) {
                    roHeap->dump("buffer");
                }
                mCblkMemory.clear();
                mBufferMemory.clear();
                return;
            }
            memset(mBuffer, 0, bufferSize);
            } break;
        case ALLOC_PIPE:
            mBufferMemory = thread->pipeMemory();
            // mBuffer is the virtual address as seen from current process (mediaserver),
            // and should normally be coming from mBufferMemory->pointer().
            // However in this case the TrackBase does not reference the buffer directly.
            // It should references the buffer via the pipe.
            // Therefore, to detect incorrect usage of the buffer, we set mBuffer to NULL.
            mBuffer = NULL;
            bufferSize = 0;
            break;
        case ALLOC_CBLK:
            // clear all buffers
            if (buffer == NULL) {
                mBuffer = (char*)mCblk + sizeof(audio_track_cblk_t);
                memset(mBuffer, 0, bufferSize);
            } else {
                mBuffer = buffer;
#if 0
                mCblk->mFlags = CBLK_FORCEREADY;    // FIXME hack, need to fix the track ready logic
#endif
            }
            break;
        case ALLOC_LOCAL:
            mBuffer = calloc(1, bufferSize);
            break;
        case ALLOC_NONE:
            mBuffer = buffer;
            break;
        default:
            LOG_ALWAYS_FATAL("%s(%d): invalid allocation type: %d", __func__, mId, (int)alloc);
        }
        mBufferSize = bufferSize;

#ifdef TEE_SINK
        mTee.set(sampleRate, mChannelCount, format, NBAIO_Tee::TEE_FLAG_TRACK);
#endif

    }
}

status_t AudioFlinger::ThreadBase::TrackBase::initCheck() const
{
    status_t status;
    if (mType == TYPE_OUTPUT || mType == TYPE_PATCH) {
        status = cblk() != NULL ? NO_ERROR : NO_MEMORY;
    } else {
        status = getCblk() != 0 ? NO_ERROR : NO_MEMORY;
    }
    return status;
}

AudioFlinger::ThreadBase::TrackBase::~TrackBase()
{
    // delete the proxy before deleting the shared memory it refers to, to avoid dangling reference
    mServerProxy.clear();
    if (mCblk != NULL) {
        mCblk->~audio_track_cblk_t();   // destroy our shared-structure.
        if (mClient == 0) {
            free(mCblk);
        }
    }
    mCblkMemory.clear();    // free the shared memory before releasing the heap it belongs to
    if (mClient != 0) {
        // Client destructor must run with AudioFlinger client mutex locked
        Mutex::Autolock _l(mClient->audioFlinger()->mClientLock);
        // If the client's reference count drops to zero, the associated destructor
        // must run with AudioFlinger lock held. Thus the explicit clear() rather than
        // relying on the automatic clear() at end of scope.
        mClient.clear();
    }
    // flush the binder command buffer
    IPCThreadState::self()->flushCommands();
}

// AudioBufferProvider interface
// getNextBuffer() = 0;
// This implementation of releaseBuffer() is used by Track and RecordTrack
void AudioFlinger::ThreadBase::TrackBase::releaseBuffer(AudioBufferProvider::Buffer* buffer)
{
#ifdef TEE_SINK
    mTee.write(buffer->raw, buffer->frameCount);
#endif

    ServerProxy::Buffer buf;
    buf.mFrameCount = buffer->frameCount;
    buf.mRaw = buffer->raw;
    buffer->frameCount = 0;
    buffer->raw = NULL;
    mServerProxy->releaseBuffer(&buf);
}

status_t AudioFlinger::ThreadBase::TrackBase::setSyncEvent(const sp<SyncEvent>& event)
{
    mSyncEvents.add(event);
    return NO_ERROR;
}

AudioFlinger::ThreadBase::PatchTrackBase::PatchTrackBase(sp<ClientProxy> proxy,
                                                         const ThreadBase& thread,
                                                         const Timeout& timeout)
    : mProxy(proxy)
{
    if (timeout) {
        setPeerTimeout(*timeout);
    } else {
        // Double buffer mixer
        uint64_t mixBufferNs = ((uint64_t)2 * thread.frameCount() * 1000000000) /
                                              thread.sampleRate();
        setPeerTimeout(std::chrono::nanoseconds{mixBufferNs});
    }
}

void AudioFlinger::ThreadBase::PatchTrackBase::setPeerTimeout(std::chrono::nanoseconds timeout) {
    mPeerTimeout.tv_sec = timeout.count() / std::nano::den;
    mPeerTimeout.tv_nsec = timeout.count() % std::nano::den;
}


// ----------------------------------------------------------------------------
//      Playback
// ----------------------------------------------------------------------------
#undef LOG_TAG
#define LOG_TAG "AF::TrackHandle"

AudioFlinger::TrackHandle::TrackHandle(const sp<AudioFlinger::PlaybackThread::Track>& track)
    : BnAudioTrack(),
      mTrack(track)
{
}

AudioFlinger::TrackHandle::~TrackHandle() {
    // just stop the track on deletion, associated resources
    // will be freed from the main thread once all pending buffers have
    // been played. Unless it's not in the active track list, in which
    // case we free everything now...
    mTrack->destroy();
}

sp<IMemory> AudioFlinger::TrackHandle::getCblk() const {
    return mTrack->getCblk();
}

status_t AudioFlinger::TrackHandle::start() {
    return mTrack->start();
}

void AudioFlinger::TrackHandle::stop() {
    mTrack->stop();
}

void AudioFlinger::TrackHandle::flush() {
    mTrack->flush();
}

void AudioFlinger::TrackHandle::pause() {
    mTrack->pause();
}

status_t AudioFlinger::TrackHandle::attachAuxEffect(int EffectId)
{
    return mTrack->attachAuxEffect(EffectId);
}

status_t AudioFlinger::TrackHandle::setParameters(const String8& keyValuePairs) {
    return mTrack->setParameters(keyValuePairs);
}

status_t AudioFlinger::TrackHandle::selectPresentation(int presentationId, int programId) {
    return mTrack->selectPresentation(presentationId, programId);
}

VolumeShaper::Status AudioFlinger::TrackHandle::applyVolumeShaper(
        const sp<VolumeShaper::Configuration>& configuration,
        const sp<VolumeShaper::Operation>& operation) {
    return mTrack->applyVolumeShaper(configuration, operation);
}

sp<VolumeShaper::State> AudioFlinger::TrackHandle::getVolumeShaperState(int id) {
    return mTrack->getVolumeShaperState(id);
}

status_t AudioFlinger::TrackHandle::getTimestamp(AudioTimestamp& timestamp)
{
    return mTrack->getTimestamp(timestamp);
}


void AudioFlinger::TrackHandle::signal()
{
    return mTrack->signal();
}

status_t AudioFlinger::TrackHandle::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    return BnAudioTrack::onTransact(code, data, reply, flags);
}

// ----------------------------------------------------------------------------
//      AppOp for audio playback
// -------------------------------

// static
sp<AudioFlinger::PlaybackThread::OpPlayAudioMonitor>
AudioFlinger::PlaybackThread::OpPlayAudioMonitor::createIfNeeded(
            uid_t uid, const audio_attributes_t& attr, int id, audio_stream_type_t streamType)
{
    if (isServiceUid(uid)) {
        Vector <String16> packages;
        getPackagesForUid(uid, packages);
        if (packages.isEmpty()) {
            ALOGD("OpPlayAudio: not muting track:%d usage:%d for service UID %d",
                  id,
                  attr.usage,
                  uid);
            return nullptr;
        }
    }
    // stream type has been filtered by audio policy to indicate whether it can be muted
    if (streamType == AUDIO_STREAM_ENFORCED_AUDIBLE) {
        ALOGD("OpPlayAudio: not muting track:%d usage:%d ENFORCED_AUDIBLE", id, attr.usage);
        return nullptr;
    }
    if ((attr.flags & AUDIO_FLAG_BYPASS_INTERRUPTION_POLICY)
            == AUDIO_FLAG_BYPASS_INTERRUPTION_POLICY) {
        ALOGD("OpPlayAudio: not muting track:%d flags %#x have FLAG_BYPASS_INTERRUPTION_POLICY",
            id, attr.flags);
        return nullptr;
    }
    return new OpPlayAudioMonitor(uid, attr.usage, id);
}

AudioFlinger::PlaybackThread::OpPlayAudioMonitor::OpPlayAudioMonitor(
        uid_t uid, audio_usage_t usage, int id)
        : mHasOpPlayAudio(true), mUid(uid), mUsage((int32_t) usage), mId(id)
{
}

AudioFlinger::PlaybackThread::OpPlayAudioMonitor::~OpPlayAudioMonitor()
{
    if (mOpCallback != 0) {
        mAppOpsManager.stopWatchingMode(mOpCallback);
    }
    mOpCallback.clear();
}

void AudioFlinger::PlaybackThread::OpPlayAudioMonitor::onFirstRef()
{
    getPackagesForUid(mUid, mPackages);
    checkPlayAudioForUsage();
    if (!mPackages.isEmpty()) {
        mOpCallback = new PlayAudioOpCallback(this);
        mAppOpsManager.startWatchingMode(AppOpsManager::OP_PLAY_AUDIO, mPackages[0], mOpCallback);
    }
}

bool AudioFlinger::PlaybackThread::OpPlayAudioMonitor::hasOpPlayAudio() const {
    return mHasOpPlayAudio.load();
}

// Note this method is never called (and never to be) for audio server / patch record track
// - not called from constructor due to check on UID,
// - not called from PlayAudioOpCallback because the callback is not installed in this case
void AudioFlinger::PlaybackThread::OpPlayAudioMonitor::checkPlayAudioForUsage()
{
    if (mPackages.isEmpty()) {
        mHasOpPlayAudio.store(false);
    } else {
        bool hasIt = true;
        for (const String16& packageName : mPackages) {
            const int32_t mode = mAppOpsManager.checkAudioOpNoThrow(AppOpsManager::OP_PLAY_AUDIO,
                    mUsage, mUid, packageName);
            if (mode != AppOpsManager::MODE_ALLOWED) {
                hasIt = false;
                break;
            }
        }
        ALOGD("OpPlayAudio: track:%d usage:%d %smuted", mId, mUsage, hasIt ? "not " : "");
        mHasOpPlayAudio.store(hasIt);
    }
}

AudioFlinger::PlaybackThread::OpPlayAudioMonitor::PlayAudioOpCallback::PlayAudioOpCallback(
        const wp<OpPlayAudioMonitor>& monitor) : mMonitor(monitor)
{ }

void AudioFlinger::PlaybackThread::OpPlayAudioMonitor::PlayAudioOpCallback::opChanged(int32_t op,
            const String16& packageName) {
    // we only have uid, so we need to check all package names anyway
    UNUSED(packageName);
    if (op != AppOpsManager::OP_PLAY_AUDIO) {
        return;
    }
    sp<OpPlayAudioMonitor> monitor = mMonitor.promote();
    if (monitor != NULL) {
        monitor->checkPlayAudioForUsage();
    }
}

// static
void AudioFlinger::PlaybackThread::OpPlayAudioMonitor::getPackagesForUid(
    uid_t uid, Vector<String16>& packages)
{
    PermissionController permissionController;
    permissionController.getPackagesForUid(uid, packages);
}

// ----------------------------------------------------------------------------
#undef LOG_TAG
#define LOG_TAG "AF::Track"

// Track constructor must be called with AudioFlinger::mLock and ThreadBase::mLock held
AudioFlinger::PlaybackThread::Track::Track(
            PlaybackThread *thread,
            const sp<Client>& client,
            audio_stream_type_t streamType,
            const audio_attributes_t& attr,
            uint32_t sampleRate,
            audio_format_t format,
            audio_channel_mask_t channelMask,
            size_t frameCount,
            void *buffer,
            size_t bufferSize,
            const sp<IMemory>& sharedBuffer,
            audio_session_t sessionId,
            pid_t creatorPid,
            uid_t uid,
            audio_output_flags_t flags,
            track_type type,
            audio_port_handle_t portId,
            size_t frameCountToBeReady)
    :   TrackBase(thread, client, attr, sampleRate, format, channelMask, frameCount,
                  (sharedBuffer != 0) ? sharedBuffer->pointer() : buffer,
                  (sharedBuffer != 0) ? sharedBuffer->size() : bufferSize,
                  sessionId, creatorPid, uid, true /*isOut*/,
                  (type == TYPE_PATCH) ? ( buffer == NULL ? ALLOC_LOCAL : ALLOC_NONE) : ALLOC_CBLK,
                  type, portId),
    mFillingUpStatus(FS_INVALID),
    // mRetryCount initialized later when needed
    mSharedBuffer(sharedBuffer),
    mStreamType(streamType),
    mMainBuffer(thread->sinkBuffer()),
    mAuxBuffer(NULL),
    mAuxEffectId(0), mHasVolumeController(false),
    mPresentationCompleteFrames(0),
    mFrameMap(16 /* sink-frame-to-track-frame map memory */),
    mVolumeHandler(new media::VolumeHandler(sampleRate)),
    mOpPlayAudioMonitor(OpPlayAudioMonitor::createIfNeeded(uid, attr, id(), streamType)),
    // mSinkTimestamp
    mFrameCountToBeReady(frameCountToBeReady),
    mFastIndex(-1),
    mCachedVolume(1.0),
    /* The track might not play immediately after being active, similarly as if its volume was 0.
     * When the track starts playing, its volume will be computed. */
    mFinalVolume(0.f),
    mResumeToStopping(false),
    mFlushHwPending(false),
    mFlags(flags)
{
    // client == 0 implies sharedBuffer == 0
    ALOG_ASSERT(!(client == 0 && sharedBuffer != 0));

    ALOGV_IF(sharedBuffer != 0, "%s(%d): sharedBuffer: %p, size: %zu",
            __func__, mId, sharedBuffer->pointer(), sharedBuffer->size());

    if (mCblk == NULL) {
        return;
    }

    if (sharedBuffer == 0) {
        mAudioTrackServerProxy = new AudioTrackServerProxy(mCblk, mBuffer, frameCount,
                mFrameSize, !isExternalTrack(), sampleRate);
    } else {
        mAudioTrackServerProxy = new StaticAudioTrackServerProxy(mCblk, mBuffer, frameCount,
                mFrameSize);
    }
    mServerProxy = mAudioTrackServerProxy;

    if (!thread->isTrackAllowed_l(channelMask, format, sessionId, uid)) {
        ALOGE("%s(%d): no more tracks available", __func__, mId);
        return;
    }
    // only allocate a fast track index if we were able to allocate a normal track name
    if (flags & AUDIO_OUTPUT_FLAG_FAST) {
        // FIXME: Not calling framesReadyIsCalledByMultipleThreads() exposes a potential
        // race with setSyncEvent(). However, if we call it, we cannot properly start
        // static fast tracks (SoundPool) immediately after stopping.
        //mAudioTrackServerProxy->framesReadyIsCalledByMultipleThreads();
        ALOG_ASSERT(thread->mFastTrackAvailMask != 0);
        int i = __builtin_ctz(thread->mFastTrackAvailMask);
        ALOG_ASSERT(0 < i && i < (int)FastMixerState::sMaxFastTracks);
        // FIXME This is too eager.  We allocate a fast track index before the
        //       fast track becomes active.  Since fast tracks are a scarce resource,
        //       this means we are potentially denying other more important fast tracks from
        //       being created.  It would be better to allocate the index dynamically.
        mFastIndex = i;
        thread->mFastTrackAvailMask &= ~(1 << i);
    }

    mServerLatencySupported = thread->type() == ThreadBase::MIXER
            || thread->type() == ThreadBase::DUPLICATING;
#ifdef TEE_SINK
    mTee.setId(std::string("_") + std::to_string(mThreadIoHandle)
            + "_" + std::to_string(mId) + "_T");
#endif

    if (channelMask & AUDIO_CHANNEL_HAPTIC_ALL) {
        mAudioVibrationController = new AudioVibrationController(this);
        mExternalVibration = new os::ExternalVibration(
                mUid, "" /* pkg */, mAttr, mAudioVibrationController);
    }
}

AudioFlinger::PlaybackThread::Track::~Track()
{
    ALOGV("%s(%d)", __func__, mId);

    // The destructor would clear mSharedBuffer,
    // but it will not push the decremented reference count,
    // leaving the client's IMemory dangling indefinitely.
    // This prevents that leak.
    if (mSharedBuffer != 0) {
        mSharedBuffer.clear();
    }
}

status_t AudioFlinger::PlaybackThread::Track::initCheck() const
{
    status_t status = TrackBase::initCheck();
    if (status == NO_ERROR && mCblk == nullptr) {
        status = NO_MEMORY;
    }
    return status;
}

void AudioFlinger::PlaybackThread::Track::destroy()
{
    // NOTE: destroyTrack_l() can remove a strong reference to this Track
    // by removing it from mTracks vector, so there is a risk that this Tracks's
    // destructor is called. As the destructor needs to lock mLock,
    // we must acquire a strong reference on this Track before locking mLock
    // here so that the destructor is called only when exiting this function.
    // On the other hand, as long as Track::destroy() is only called by
    // TrackHandle destructor, the TrackHandle still holds a strong ref on
    // this Track with its member mTrack.
    sp<Track> keep(this);
    { // scope for mLock
        bool wasActive = false;
        sp<ThreadBase> thread = mThread.promote();
        if (thread != 0) {
            Mutex::Autolock _l(thread->mLock);
            PlaybackThread *playbackThread = (PlaybackThread *)thread.get();
            wasActive = playbackThread->destroyTrack_l(this);
        }
        if (isExternalTrack() && !wasActive) {
            AudioSystem::releaseOutput(mPortId);
        }
    }
    forEachTeePatchTrack([](auto patchTrack) { patchTrack->destroy(); });
}

void AudioFlinger::PlaybackThread::Track::appendDumpHeader(String8& result)
{
    result.appendFormat("Type     Id Active Client Session Port Id S  Flags "
                        "  Format Chn mask  SRate "
                        "ST Usg CT "
                        " G db  L dB  R dB  VS dB "
                        "  Server FrmCnt  FrmRdy F Underruns  Flushed"
                        "%s\n",
                        isServerLatencySupported() ? "   Latency" : "");
}

void AudioFlinger::PlaybackThread::Track::appendDump(String8& result, bool active)
{
    char trackType;
    switch (mType) {
    case TYPE_DEFAULT:
    case TYPE_OUTPUT:
        if (isStatic()) {
            trackType = 'S'; // static
        } else {
            trackType = ' '; // normal
        }
        break;
    case TYPE_PATCH:
        trackType = 'P';
        break;
    default:
        trackType = '?';
    }

    if (isFastTrack()) {
        result.appendFormat("F%d %c %6d", mFastIndex, trackType, mId);
    } else {
        result.appendFormat("   %c %6d", trackType, mId);
    }

    char nowInUnderrun;
    switch (mObservedUnderruns.mBitFields.mMostRecent) {
    case UNDERRUN_FULL:
        nowInUnderrun = ' ';
        break;
    case UNDERRUN_PARTIAL:
        nowInUnderrun = '<';
        break;
    case UNDERRUN_EMPTY:
        nowInUnderrun = '*';
        break;
    default:
        nowInUnderrun = '?';
        break;
    }

    char fillingStatus;
    switch (mFillingUpStatus) {
    case FS_INVALID:
        fillingStatus = 'I';
        break;
    case FS_FILLING:
        fillingStatus = 'f';
        break;
    case FS_FILLED:
        fillingStatus = 'F';
        break;
    case FS_ACTIVE:
        fillingStatus = 'A';
        break;
    default:
        fillingStatus = '?';
        break;
    }

    // clip framesReadySafe to max representation in dump
    const size_t framesReadySafe =
            std::min(mAudioTrackServerProxy->framesReadySafe(), (size_t)99999999);

    // obtain volumes
    const gain_minifloat_packed_t vlr = mAudioTrackServerProxy->getVolumeLR();
    const std::pair<float /* volume */, bool /* active */> vsVolume =
            mVolumeHandler->getLastVolume();

    // Our effective frame count is obtained by ServerProxy::getBufferSizeInFrames()
    // as it may be reduced by the application.
    const size_t bufferSizeInFrames = (size_t)mAudioTrackServerProxy->getBufferSizeInFrames();
    // Check whether the buffer size has been modified by the app.
    const char modifiedBufferChar = bufferSizeInFrames < mFrameCount
            ? 'r' /* buffer reduced */: bufferSizeInFrames > mFrameCount
                    ? 'e' /* error */ : ' ' /* identical */;

    result.appendFormat("%7s %6u %7u %7u %2s 0x%03X "
                        "%08X %08X %6u "
                        "%2u %3x %2x "
                        "%5.2g %5.2g %5.2g %5.2g%c "
                        "%08X %6zu%c %6zu %c %9u%c %7u",
            active ? "yes" : "no",
            (mClient == 0) ? getpid() : mClient->pid(),
            mSessionId,
            mPortId,
            getTrackStateString(),
            mCblk->mFlags,

            mFormat,
            mChannelMask,
            sampleRate(),

            mStreamType,
            mAttr.usage,
            mAttr.content_type,

            20.0 * log10(mFinalVolume),
            20.0 * log10(float_from_gain(gain_minifloat_unpack_left(vlr))),
            20.0 * log10(float_from_gain(gain_minifloat_unpack_right(vlr))),
            20.0 * log10(vsVolume.first), // VolumeShaper(s) total volume
            vsVolume.second ? 'A' : ' ',  // if any VolumeShapers active

            mCblk->mServer,
            bufferSizeInFrames,
            modifiedBufferChar,
            framesReadySafe,
            fillingStatus,
            mAudioTrackServerProxy->getUnderrunFrames(),
            nowInUnderrun,
            (unsigned)mAudioTrackServerProxy->framesFlushed() % 10000000
            );

    if (isServerLatencySupported()) {
        double latencyMs;
        bool fromTrack;
        if (getTrackLatencyMs(&latencyMs, &fromTrack) == OK) {
            // Show latency in msec, followed by 't' if from track timestamp (the most accurate)
            // or 'k' if estimated from kernel because track frames haven't been presented yet.
            result.appendFormat(" %7.2lf %c", latencyMs, fromTrack ? 't' : 'k');
        } else {
            result.appendFormat("%10s", mCblk->mServer != 0 ? "unavail" : "new");
        }
    }
    result.append("\n");
}

uint32_t AudioFlinger::PlaybackThread::Track::sampleRate() const {
    return mAudioTrackServerProxy->getSampleRate();
}

// AudioBufferProvider interface
status_t AudioFlinger::PlaybackThread::Track::getNextBuffer(AudioBufferProvider::Buffer* buffer)
{
    ServerProxy::Buffer buf;
    size_t desiredFrames = buffer->frameCount;
    buf.mFrameCount = desiredFrames;
    status_t status = mServerProxy->obtainBuffer(&buf);
    buffer->frameCount = buf.mFrameCount;
    buffer->raw = buf.mRaw;
    if (buf.mFrameCount == 0 && !isStopping() && !isStopped() && !isPaused()) {
        ALOGV("%s(%d): underrun,  framesReady(%zu) < framesDesired(%zd), state: %d",
                __func__, mId, buf.mFrameCount, desiredFrames, mState);
        mAudioTrackServerProxy->tallyUnderrunFrames(desiredFrames);
    } else {
        mAudioTrackServerProxy->tallyUnderrunFrames(0);
    }
    return status;
}

void AudioFlinger::PlaybackThread::Track::releaseBuffer(AudioBufferProvider::Buffer* buffer)
{
    interceptBuffer(*buffer);
    TrackBase::releaseBuffer(buffer);
}

// TODO: compensate for time shift between HW modules.
void AudioFlinger::PlaybackThread::Track::interceptBuffer(
        const AudioBufferProvider::Buffer& sourceBuffer) {
    auto start = std::chrono::steady_clock::now();
    const size_t frameCount = sourceBuffer.frameCount;
    if (frameCount == 0) {
        return;  // No audio to intercept.
        // Additionally PatchProxyBufferProvider::obtainBuffer (called by PathTrack::getNextBuffer)
        // does not allow 0 frame size request contrary to getNextBuffer
    }
    for (auto& teePatch : mTeePatches) {
        RecordThread::PatchRecord* patchRecord = teePatch.patchRecord.get();

        size_t framesWritten = writeFrames(patchRecord, sourceBuffer.i8, frameCount);
        // On buffer wrap, the buffer frame count will be less than requested,
        // when this happens a second buffer needs to be used to write the leftover audio
        size_t framesLeft = frameCount - framesWritten;
        if (framesWritten != 0 && framesLeft != 0) {
            framesWritten +=
                writeFrames(patchRecord, sourceBuffer.i8 + framesWritten * mFrameSize, framesLeft);
            framesLeft = frameCount - framesWritten;
        }
        ALOGW_IF(framesLeft != 0, "%s(%d) PatchRecord %d can not provide big enough "
                 "buffer %zu/%zu, dropping %zu frames", __func__, mId, patchRecord->mId,
                 framesWritten, frameCount, framesLeft);
    }
    auto spent = ceil<std::chrono::microseconds>(std::chrono::steady_clock::now() - start);
    using namespace std::chrono_literals;
    // Average is ~20us per track, this should virtually never be logged (Logging takes >200us)
    ALOGD_IF(spent > 500us, "%s: took %lldus to intercept %zu tracks", __func__,
             spent.count(), mTeePatches.size());
}

size_t AudioFlinger::PlaybackThread::Track::writeFrames(AudioBufferProvider* dest,
                                                        const void* src,
                                                        size_t frameCount) {
    AudioBufferProvider::Buffer patchBuffer;
    patchBuffer.frameCount = frameCount;
    auto status = dest->getNextBuffer(&patchBuffer);
    if (status != NO_ERROR) {
       ALOGW("%s PathRecord getNextBuffer failed with error %d: %s",
             __func__, status, strerror(-status));
       return 0;
    }
    ALOG_ASSERT(patchBuffer.frameCount <= frameCount);
    memcpy(patchBuffer.raw, src, patchBuffer.frameCount * mFrameSize);
    auto framesWritten = patchBuffer.frameCount;
    dest->releaseBuffer(&patchBuffer);
    return framesWritten;
}

// releaseBuffer() is not overridden

// ExtendedAudioBufferProvider interface

// framesReady() may return an approximation of the number of frames if called
// from a different thread than the one calling Proxy->obtainBuffer() and
// Proxy->releaseBuffer(). Also note there is no mutual exclusion in the
// AudioTrackServerProxy so be especially careful calling with FastTracks.
size_t AudioFlinger::PlaybackThread::Track::framesReady() const {
    if (mSharedBuffer != 0 && (isStopped() || isStopping())) {
        // Static tracks return zero frames immediately upon stopping (for FastTracks).
        // The remainder of the buffer is not drained.
        return 0;
    }
    return mAudioTrackServerProxy->framesReady();
}

int64_t AudioFlinger::PlaybackThread::Track::framesReleased() const
{
    return mAudioTrackServerProxy->framesReleased();
}

void AudioFlinger::PlaybackThread::Track::onTimestamp(const ExtendedTimestamp &timestamp)
{
    // This call comes from a FastTrack and should be kept lockless.
    // The server side frames are already translated to client frames.
    mAudioTrackServerProxy->setTimestamp(timestamp);

    // We do not set drained here, as FastTrack timestamp may not go to very last frame.

    // Compute latency.
    // TODO: Consider whether the server latency may be passed in by FastMixer
    // as a constant for all active FastTracks.
    const double latencyMs = timestamp.getOutputServerLatencyMs(sampleRate());
    mServerLatencyFromTrack.store(true);
    mServerLatencyMs.store(latencyMs);
}

// Don't call for fast tracks; the framesReady() could result in priority inversion
bool AudioFlinger::PlaybackThread::Track::isReady() const {
    if (mFillingUpStatus != FS_FILLING || isStopped() || isPausing()) {
        return true;
    }

    if (isStopping()) {
        if (framesReady() > 0) {
            mFillingUpStatus = FS_FILLED;
        }
        return true;
    }

    size_t bufferSizeInFrames = mServerProxy->getBufferSizeInFrames();
    size_t framesToBeReady = std::min(mFrameCountToBeReady, bufferSizeInFrames);

    if (framesReady() >= framesToBeReady || (mCblk->mFlags & CBLK_FORCEREADY)) {
        ALOGV("%s(%d): consider track ready with %zu/%zu, target was %zu)",
              __func__, mId, framesReady(), bufferSizeInFrames, framesToBeReady);
        mFillingUpStatus = FS_FILLED;
        android_atomic_and(~CBLK_FORCEREADY, &mCblk->mFlags);
        return true;
    }
    return false;
}

status_t AudioFlinger::PlaybackThread::Track::start(AudioSystem::sync_event_t event __unused,
                                                    audio_session_t triggerSession __unused)
{
    status_t status = NO_ERROR;
    ALOGV("%s(%d): calling pid %d session %d",
            __func__, mId, IPCThreadState::self()->getCallingPid(), mSessionId);

    sp<ThreadBase> thread = mThread.promote();
    if (thread != 0) {
        if (isOffloaded()) {
            Mutex::Autolock _laf(thread->mAudioFlinger->mLock);
            Mutex::Autolock _lth(thread->mLock);
            sp<EffectChain> ec = thread->getEffectChain_l(mSessionId);
            if (thread->mAudioFlinger->isNonOffloadableGlobalEffectEnabled_l() ||
                    (ec != 0 && ec->isNonOffloadableEnabled())) {
                invalidate();
                return PERMISSION_DENIED;
            }
        }
        Mutex::Autolock _lth(thread->mLock);
        track_state state = mState;
        // here the track could be either new, or restarted
        // in both cases "unstop" the track

        // initial state-stopping. next state-pausing.
        // What if resume is called ?

        if (state == PAUSED || state == PAUSING) {
            if (mResumeToStopping) {
                // happened we need to resume to STOPPING_1
                mState = TrackBase::STOPPING_1;
                ALOGV("%s(%d): PAUSED => STOPPING_1 on thread %d",
                        __func__, mId, (int)mThreadIoHandle);
            } else {
                mState = TrackBase::RESUMING;
                ALOGV("%s(%d): PAUSED => RESUMING on thread %d",
                        __func__,  mId, (int)mThreadIoHandle);
            }
        } else {
            mState = TrackBase::ACTIVE;
            ALOGV("%s(%d): ? => ACTIVE on thread %d",
                    __func__, mId, (int)mThreadIoHandle);
        }

        // states to reset position info for non-offloaded/direct tracks
        if (!isOffloaded() && !isDirect()
                && (state == IDLE || state == STOPPED || state == FLUSHED)) {
            mFrameMap.reset();
        }
        PlaybackThread *playbackThread = (PlaybackThread *)thread.get();
        if (isFastTrack()) {
            // refresh fast track underruns on start because that field is never cleared
            // by the fast mixer; furthermore, the same track can be recycled, i.e. start
            // after stop.
            mObservedUnderruns = playbackThread->getFastTrackUnderruns(mFastIndex);
        }
        status = playbackThread->addTrack_l(this);
        if (status == INVALID_OPERATION || status == PERMISSION_DENIED) {
            triggerEvents(AudioSystem::SYNC_EVENT_PRESENTATION_COMPLETE);
            //  restore previous state if start was rejected by policy manager
            if (status == PERMISSION_DENIED) {
                mState = state;
            }
        }

        if (status == NO_ERROR || status == ALREADY_EXISTS) {
            // for streaming tracks, remove the buffer read stop limit.
            mAudioTrackServerProxy->start();
        }

        // track was already in the active list, not a problem
        if (status == ALREADY_EXISTS) {
            status = NO_ERROR;
        } else {
            // Acknowledge any pending flush(), so that subsequent new data isn't discarded.
            // It is usually unsafe to access the server proxy from a binder thread.
            // But in this case we know the mixer thread (whether normal mixer or fast mixer)
            // isn't looking at this track yet:  we still hold the normal mixer thread lock,
            // and for fast tracks the track is not yet in the fast mixer thread's active set.
            // For static tracks, this is used to acknowledge change in position or loop.
            ServerProxy::Buffer buffer;
            buffer.mFrameCount = 1;
            (void) mAudioTrackServerProxy->obtainBuffer(&buffer, true /*ackFlush*/);
        }
    } else {
        status = BAD_VALUE;
    }
    if (status == NO_ERROR) {
        forEachTeePatchTrack([](auto patchTrack) { patchTrack->start(); });
    }
    return status;
}

void AudioFlinger::PlaybackThread::Track::stop()
{
    ALOGV("%s(%d): calling pid %d", __func__, mId, IPCThreadState::self()->getCallingPid());
    sp<ThreadBase> thread = mThread.promote();
    if (thread != 0) {
        Mutex::Autolock _l(thread->mLock);
        track_state state = mState;
        if (state == RESUMING || state == ACTIVE || state == PAUSING || state == PAUSED) {
            // If the track is not active (PAUSED and buffers full), flush buffers
            PlaybackThread *playbackThread = (PlaybackThread *)thread.get();
            if (playbackThread->mActiveTracks.indexOf(this) < 0) {
                reset();
                mState = STOPPED;
            } else if (!isFastTrack() && !isOffloaded() && !isDirect()) {
                mState = STOPPED;
            } else {
                // For fast tracks prepareTracks_l() will set state to STOPPING_2
                // presentation is complete
                // For an offloaded track this starts a drain and state will
                // move to STOPPING_2 when drain completes and then STOPPED
                mState = STOPPING_1;
                if (isOffloaded()) {
                    mRetryCount = PlaybackThread::kMaxTrackStopRetriesOffload;
                }
            }
            playbackThread->broadcast_l();
            ALOGV("%s(%d): not stopping/stopped => stopping/stopped on thread %d",
                    __func__, mId, (int)mThreadIoHandle);
        }
    }
    forEachTeePatchTrack([](auto patchTrack) { patchTrack->stop(); });
}

void AudioFlinger::PlaybackThread::Track::pause()
{
    ALOGV("%s(%d): calling pid %d", __func__, mId, IPCThreadState::self()->getCallingPid());
    sp<ThreadBase> thread = mThread.promote();
    if (thread != 0) {
        Mutex::Autolock _l(thread->mLock);
        PlaybackThread *playbackThread = (PlaybackThread *)thread.get();
        switch (mState) {
        case STOPPING_1:
        case STOPPING_2:
            if (!isOffloaded()) {
                /* nothing to do if track is not offloaded */
                break;
            }

            // Offloaded track was draining, we need to carry on draining when resumed
            mResumeToStopping = true;
            FALLTHROUGH_INTENDED;
        case ACTIVE:
        case RESUMING:
            mState = PAUSING;
            ALOGV("%s(%d): ACTIVE/RESUMING => PAUSING on thread %d",
                    __func__, mId, (int)mThreadIoHandle);
            playbackThread->broadcast_l();
            break;

        default:
            break;
        }
    }
    // Pausing the TeePatch to avoid a glitch on underrun, at the cost of buffered audio loss.
    forEachTeePatchTrack([](auto patchTrack) { patchTrack->pause(); });
}

void AudioFlinger::PlaybackThread::Track::flush()
{
    ALOGV("%s(%d)", __func__, mId);
    sp<ThreadBase> thread = mThread.promote();
    if (thread != 0) {
        Mutex::Autolock _l(thread->mLock);
        PlaybackThread *playbackThread = (PlaybackThread *)thread.get();

        // Flush the ring buffer now if the track is not active in the PlaybackThread.
        // Otherwise the flush would not be done until the track is resumed.
        // Requires FastTrack removal be BLOCK_UNTIL_ACKED
        if (playbackThread->mActiveTracks.indexOf(this) < 0) {
            (void)mServerProxy->flushBufferIfNeeded();
        }

        if (isOffloaded()) {
            // If offloaded we allow flush during any state except terminated
            // and keep the track active to avoid problems if user is seeking
            // rapidly and underlying hardware has a significant delay handling
            // a pause
            if (isTerminated()) {
                return;
            }

            ALOGV("%s(%d): offload flush", __func__, mId);
            reset();

            if (mState == STOPPING_1 || mState == STOPPING_2) {
                ALOGV("%s(%d): flushed in STOPPING_1 or 2 state, change state to ACTIVE",
                        __func__, mId);
                mState = ACTIVE;
            }

            mFlushHwPending = true;
            mResumeToStopping = false;
        } else {
            if (mState != STOPPING_1 && mState != STOPPING_2 && mState != STOPPED &&
                    mState != PAUSED && mState != PAUSING && mState != IDLE && mState != FLUSHED) {
                return;
            }
            // No point remaining in PAUSED state after a flush => go to
            // FLUSHED state
            mState = FLUSHED;
            // do not reset the track if it is still in the process of being stopped or paused.
            // this will be done by prepareTracks_l() when the track is stopped.
            // prepareTracks_l() will see mState == FLUSHED, then
            // remove from active track list, reset(), and trigger presentation complete
            if (isDirect()) {
                mFlushHwPending = true;
            }
            if (playbackThread->mActiveTracks.indexOf(this) < 0) {
                reset();
            }
        }
        // Prevent flush being lost if the track is flushed and then resumed
        // before mixer thread can run. This is important when offloading
        // because the hardware buffer could hold a large amount of audio
        playbackThread->broadcast_l();
    }
    // Flush the Tee to avoid on resume playing old data and glitching on the transition to new data
    forEachTeePatchTrack([](auto patchTrack) { patchTrack->flush(); });
}

// must be called with thread lock held
void AudioFlinger::PlaybackThread::Track::flushAck()
{
    if (!isOffloaded() && !isDirect())
        return;

    // Clear the client ring buffer so that the app can prime the buffer while paused.
    // Otherwise it might not get cleared until playback is resumed and obtainBuffer() is called.
    mServerProxy->flushBufferIfNeeded();

    mFlushHwPending = false;
}

void AudioFlinger::PlaybackThread::Track::reset()
{
    // Do not reset twice to avoid discarding data written just after a flush and before
    // the audioflinger thread detects the track is stopped.
    if (!mResetDone) {
        // Force underrun condition to avoid false underrun callback until first data is
        // written to buffer
        android_atomic_and(~CBLK_FORCEREADY, &mCblk->mFlags);
        mFillingUpStatus = FS_FILLING;
        mResetDone = true;
        if (mState == FLUSHED) {
            mState = IDLE;
        }
    }
}

status_t AudioFlinger::PlaybackThread::Track::setParameters(const String8& keyValuePairs)
{
    sp<ThreadBase> thread = mThread.promote();
    if (thread == 0) {
        ALOGE("%s(%d): thread is dead", __func__, mId);
        return FAILED_TRANSACTION;
    } else if ((thread->type() == ThreadBase::DIRECT) ||
                    (thread->type() == ThreadBase::OFFLOAD)) {
        return thread->setParameters(keyValuePairs);
    } else {
        return PERMISSION_DENIED;
    }
}

status_t AudioFlinger::PlaybackThread::Track::selectPresentation(int presentationId,
        int programId) {
    sp<ThreadBase> thread = mThread.promote();
    if (thread == 0) {
        ALOGE("thread is dead");
        return FAILED_TRANSACTION;
    } else if ((thread->type() == ThreadBase::DIRECT) || (thread->type() == ThreadBase::OFFLOAD)) {
        DirectOutputThread *directOutputThread = static_cast<DirectOutputThread*>(thread.get());
        return directOutputThread->selectPresentation(presentationId, programId);
    }
    return INVALID_OPERATION;
}

VolumeShaper::Status AudioFlinger::PlaybackThread::Track::applyVolumeShaper(
        const sp<VolumeShaper::Configuration>& configuration,
        const sp<VolumeShaper::Operation>& operation)
{
    sp<VolumeShaper::Configuration> newConfiguration;

    if (isOffloadedOrDirect()) {
        const VolumeShaper::Configuration::OptionFlag optionFlag
            = configuration->getOptionFlags();
        if ((optionFlag & VolumeShaper::Configuration::OPTION_FLAG_CLOCK_TIME) == 0) {
            ALOGW("%s(%d): %s tracks do not support frame counted VolumeShaper,"
                    " using clock time instead",
                    __func__, mId,
                    isOffloaded() ? "Offload" : "Direct");
            newConfiguration = new VolumeShaper::Configuration(*configuration);
            newConfiguration->setOptionFlags(
                VolumeShaper::Configuration::OptionFlag(optionFlag
                        | VolumeShaper::Configuration::OPTION_FLAG_CLOCK_TIME));
        }
    }

    VolumeShaper::Status status = mVolumeHandler->applyVolumeShaper(
            (newConfiguration.get() != nullptr ? newConfiguration : configuration), operation);

    if (isOffloadedOrDirect()) {
        // Signal thread to fetch new volume.
        sp<ThreadBase> thread = mThread.promote();
        if (thread != 0) {
            Mutex::Autolock _l(thread->mLock);
            thread->broadcast_l();
        }
    }
    return status;
}

sp<VolumeShaper::State> AudioFlinger::PlaybackThread::Track::getVolumeShaperState(int id)
{
    // Note: We don't check if Thread exists.

    // mVolumeHandler is thread safe.
    return mVolumeHandler->getVolumeShaperState(id);
}

void AudioFlinger::PlaybackThread::Track::setFinalVolume(float volume)
{
    if (mFinalVolume != volume) { // Compare to an epsilon if too many meaningless updates
        mFinalVolume = volume;
        setMetadataHasChanged();
    }
}

void AudioFlinger::PlaybackThread::Track::copyMetadataTo(MetadataInserter& backInserter) const
{
    *backInserter++ = {
            .usage = mAttr.usage,
            .content_type = mAttr.content_type,
            .gain = mFinalVolume,
    };
}

void AudioFlinger::PlaybackThread::Track::setTeePatches(TeePatches teePatches) {
    forEachTeePatchTrack([](auto patchTrack) { patchTrack->destroy(); });
    mTeePatches = std::move(teePatches);
}

status_t AudioFlinger::PlaybackThread::Track::getTimestamp(AudioTimestamp& timestamp)
{
    if (!isOffloaded() && !isDirect()) {
        return INVALID_OPERATION; // normal tracks handled through SSQ
    }
    sp<ThreadBase> thread = mThread.promote();
    if (thread == 0) {
        return INVALID_OPERATION;
    }

    Mutex::Autolock _l(thread->mLock);
    PlaybackThread *playbackThread = (PlaybackThread *)thread.get();
    return playbackThread->getTimestamp_l(timestamp);
}

status_t AudioFlinger::PlaybackThread::Track::attachAuxEffect(int EffectId)
{
    sp<ThreadBase> thread = mThread.promote();
    if (thread == nullptr) {
        return DEAD_OBJECT;
    }

    sp<PlaybackThread> dstThread = (PlaybackThread *)thread.get();
    sp<PlaybackThread> srcThread; // srcThread is initialized by call to moveAuxEffectToIo()
    sp<AudioFlinger> af = mClient->audioFlinger();
    status_t status = af->moveAuxEffectToIo(EffectId, dstThread, &srcThread);

    if (EffectId != 0 && status == NO_ERROR) {
        status = dstThread->attachAuxEffect(this, EffectId);
        if (status == NO_ERROR) {
            AudioSystem::moveEffectsToIo(std::vector<int>(EffectId), dstThread->id());
        }
    }

    if (status != NO_ERROR && srcThread != nullptr) {
        af->moveAuxEffectToIo(EffectId, srcThread, &dstThread);
    }
    return status;
}

void AudioFlinger::PlaybackThread::Track::setAuxBuffer(int EffectId, int32_t *buffer)
{
    mAuxEffectId = EffectId;
    mAuxBuffer = buffer;
}

bool AudioFlinger::PlaybackThread::Track::presentationComplete(
        int64_t framesWritten, size_t audioHalFrames)
{
    // TODO: improve this based on FrameMap if it exists, to ensure full drain.
    // This assists in proper timestamp computation as well as wakelock management.

    // a track is considered presented when the total number of frames written to audio HAL
    // corresponds to the number of frames written when presentationComplete() is called for the
    // first time (mPresentationCompleteFrames == 0) plus the buffer filling status at that time.
    // For an offloaded track the HAL+h/w delay is variable so a HAL drain() is used
    // to detect when all frames have been played. In this case framesWritten isn't
    // useful because it doesn't always reflect whether there is data in the h/w
    // buffers, particularly if a track has been paused and resumed during draining
    ALOGV("%s(%d): presentationComplete() mPresentationCompleteFrames %lld framesWritten %lld",
            __func__, mId,
            (long long)mPresentationCompleteFrames, (long long)framesWritten);
    if (mPresentationCompleteFrames == 0) {
        mPresentationCompleteFrames = framesWritten + audioHalFrames;
        ALOGV("%s(%d): presentationComplete() reset:"
                " mPresentationCompleteFrames %lld audioHalFrames %zu",
                __func__, mId,
                (long long)mPresentationCompleteFrames, audioHalFrames);
    }

    bool complete;
    if (isOffloaded()) {
        complete = true;
    } else if (isDirect() || isFastTrack()) { // these do not go through linear map
        complete = framesWritten >= (int64_t) mPresentationCompleteFrames;
    } else {  // Normal tracks, OutputTracks, and PatchTracks
        complete = framesWritten >= (int64_t) mPresentationCompleteFrames
                && mAudioTrackServerProxy->isDrained();
    }

    if (complete) {
        triggerEvents(AudioSystem::SYNC_EVENT_PRESENTATION_COMPLETE);
        mAudioTrackServerProxy->setStreamEndDone();
        return true;
    }
    return false;
}

void AudioFlinger::PlaybackThread::Track::triggerEvents(AudioSystem::sync_event_t type)
{
    for (size_t i = 0; i < mSyncEvents.size();) {
        if (mSyncEvents[i]->type() == type) {
            mSyncEvents[i]->trigger();
            mSyncEvents.removeAt(i);
        } else {
            ++i;
        }
    }
}

// implement VolumeBufferProvider interface

gain_minifloat_packed_t AudioFlinger::PlaybackThread::Track::getVolumeLR()
{
    // called by FastMixer, so not allowed to take any locks, block, or do I/O including logs
    ALOG_ASSERT(isFastTrack() && (mCblk != NULL));
    gain_minifloat_packed_t vlr = mAudioTrackServerProxy->getVolumeLR();
    float vl = float_from_gain(gain_minifloat_unpack_left(vlr));
    float vr = float_from_gain(gain_minifloat_unpack_right(vlr));
    // track volumes come from shared memory, so can't be trusted and must be clamped
    if (vl > GAIN_FLOAT_UNITY) {
        vl = GAIN_FLOAT_UNITY;
    }
    if (vr > GAIN_FLOAT_UNITY) {
        vr = GAIN_FLOAT_UNITY;
    }
    // now apply the cached master volume and stream type volume;
    // this is trusted but lacks any synchronization or barrier so may be stale
    float v = mCachedVolume;
    vl *= v;
    vr *= v;
    // re-combine into packed minifloat
    vlr = gain_minifloat_pack(gain_from_float(vl), gain_from_float(vr));
    // FIXME look at mute, pause, and stop flags
    return vlr;
}

status_t AudioFlinger::PlaybackThread::Track::setSyncEvent(const sp<SyncEvent>& event)
{
    if (isTerminated() || mState == PAUSED ||
            ((framesReady() == 0) && ((mSharedBuffer != 0) ||
                                      (mState == STOPPED)))) {
        ALOGW("%s(%d): in invalid state %d on session %d %s mode, framesReady %zu",
              __func__, mId,
              mState, mSessionId, (mSharedBuffer != 0) ? "static" : "stream", framesReady());
        event->cancel();
        return INVALID_OPERATION;
    }
    (void) TrackBase::setSyncEvent(event);
    return NO_ERROR;
}

void AudioFlinger::PlaybackThread::Track::invalidate()
{
    TrackBase::invalidate();
    signalClientFlag(CBLK_INVALID);
}

void AudioFlinger::PlaybackThread::Track::disable()
{
    // TODO(b/142394888): the filling status should also be reset to filling
    signalClientFlag(CBLK_DISABLED);
}

void AudioFlinger::PlaybackThread::Track::signalClientFlag(int32_t flag)
{
    // FIXME should use proxy, and needs work
    audio_track_cblk_t* cblk = mCblk;
    android_atomic_or(flag, &cblk->mFlags);
    android_atomic_release_store(0x40000000, &cblk->mFutex);
    // client is not in server, so FUTEX_WAKE is needed instead of FUTEX_WAKE_PRIVATE
    (void) syscall(__NR_futex, &cblk->mFutex, FUTEX_WAKE, INT_MAX);
}

void AudioFlinger::PlaybackThread::Track::signal()
{
    sp<ThreadBase> thread = mThread.promote();
    if (thread != 0) {
        PlaybackThread *t = (PlaybackThread *)thread.get();
        Mutex::Autolock _l(t->mLock);
        t->broadcast_l();
    }
}

//To be called with thread lock held
bool AudioFlinger::PlaybackThread::Track::isResumePending() {

    if (mState == RESUMING)
        return true;
    /* Resume is pending if track was stopping before pause was called */
    if (mState == STOPPING_1 &&
        mResumeToStopping)
        return true;

    return false;
}

//To be called with thread lock held
void AudioFlinger::PlaybackThread::Track::resumeAck() {


    if (mState == RESUMING)
        mState = ACTIVE;

    // Other possibility of  pending resume is stopping_1 state
    // Do not update the state from stopping as this prevents
    // drain being called.
    if (mState == STOPPING_1) {
        mResumeToStopping = false;
    }
}

//To be called with thread lock held
void AudioFlinger::PlaybackThread::Track::updateTrackFrameInfo(
        int64_t trackFramesReleased, int64_t sinkFramesWritten,
        uint32_t halSampleRate, const ExtendedTimestamp &timeStamp) {
   // Make the kernel frametime available.
    const FrameTime ft{
            timeStamp.mPosition[ExtendedTimestamp::LOCATION_KERNEL],
            timeStamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL]};
    // ALOGD("FrameTime: %lld %lld", (long long)ft.frames, (long long)ft.timeNs);
    mKernelFrameTime.store(ft);
    if (!audio_is_linear_pcm(mFormat)) {
        return;
    }

    //update frame map
    mFrameMap.push(trackFramesReleased, sinkFramesWritten);

    // adjust server times and set drained state.
    //
    // Our timestamps are only updated when the track is on the Thread active list.
    // We need to ensure that tracks are not removed before full drain.
    ExtendedTimestamp local = timeStamp;
    bool drained = true; // default assume drained, if no server info found
    bool checked = false;
    for (int i = ExtendedTimestamp::LOCATION_MAX - 1;
            i >= ExtendedTimestamp::LOCATION_SERVER; --i) {
        // Lookup the track frame corresponding to the sink frame position.
        if (local.mTimeNs[i] > 0) {
            local.mPosition[i] = mFrameMap.findX(local.mPosition[i]);
            // check drain state from the latest stage in the pipeline.
            if (!checked && i <= ExtendedTimestamp::LOCATION_KERNEL) {
                drained = local.mPosition[i] >= mAudioTrackServerProxy->framesReleased();
                checked = true;
            }
        }
    }

    mAudioTrackServerProxy->setDrained(drained);
    // Set correction for flushed frames that are not accounted for in released.
    local.mFlushed = mAudioTrackServerProxy->framesFlushed();
    mServerProxy->setTimestamp(local);

    // Compute latency info.
    const bool useTrackTimestamp = !drained;
    const double latencyMs = useTrackTimestamp
            ? local.getOutputServerLatencyMs(sampleRate())
            : timeStamp.getOutputServerLatencyMs(halSampleRate);

    mServerLatencyFromTrack.store(useTrackTimestamp);
    mServerLatencyMs.store(latencyMs);
}

binder::Status AudioFlinger::PlaybackThread::Track::AudioVibrationController::mute(
        /*out*/ bool *ret) {
    *ret = false;
    sp<ThreadBase> thread = mTrack->mThread.promote();
    if (thread != 0) {
        // Lock for updating mHapticPlaybackEnabled.
        Mutex::Autolock _l(thread->mLock);
        PlaybackThread *playbackThread = (PlaybackThread *)thread.get();
        if ((mTrack->channelMask() & AUDIO_CHANNEL_HAPTIC_ALL) != AUDIO_CHANNEL_NONE
                && playbackThread->mHapticChannelCount > 0) {
            mTrack->setHapticPlaybackEnabled(false);
            *ret = true;
        }
    }
    return binder::Status::ok();
}

binder::Status AudioFlinger::PlaybackThread::Track::AudioVibrationController::unmute(
        /*out*/ bool *ret) {
    *ret = false;
    sp<ThreadBase> thread = mTrack->mThread.promote();
    if (thread != 0) {
        // Lock for updating mHapticPlaybackEnabled.
        Mutex::Autolock _l(thread->mLock);
        PlaybackThread *playbackThread = (PlaybackThread *)thread.get();
        if ((mTrack->channelMask() & AUDIO_CHANNEL_HAPTIC_ALL) != AUDIO_CHANNEL_NONE
                && playbackThread->mHapticChannelCount > 0) {
            mTrack->setHapticPlaybackEnabled(true);
            *ret = true;
        }
    }
    return binder::Status::ok();
}

// ----------------------------------------------------------------------------
#undef LOG_TAG
#define LOG_TAG "AF::OutputTrack"

AudioFlinger::PlaybackThread::OutputTrack::OutputTrack(
            PlaybackThread *playbackThread,
            DuplicatingThread *sourceThread,
            uint32_t sampleRate,
            audio_format_t format,
            audio_channel_mask_t channelMask,
            size_t frameCount,
            uid_t uid)
    :   Track(playbackThread, NULL, AUDIO_STREAM_PATCH,
              audio_attributes_t{} /* currently unused for output track */,
              sampleRate, format, channelMask, frameCount,
              nullptr /* buffer */, (size_t)0 /* bufferSize */, nullptr /* sharedBuffer */,
              AUDIO_SESSION_NONE, getpid(), uid, AUDIO_OUTPUT_FLAG_NONE,
              TYPE_OUTPUT),
    mActive(false), mSourceThread(sourceThread)
{

    if (mCblk != NULL) {
        mOutBuffer.frameCount = 0;
        playbackThread->mTracks.add(this);
        ALOGV("%s(): mCblk %p, mBuffer %p, "
                "frameCount %zu, mChannelMask 0x%08x",
                __func__, mCblk, mBuffer,
                frameCount, mChannelMask);
        // since client and server are in the same process,
        // the buffer has the same virtual address on both sides
        mClientProxy = new AudioTrackClientProxy(mCblk, mBuffer, mFrameCount, mFrameSize,
                true /*clientInServer*/);
        mClientProxy->setVolumeLR(GAIN_MINIFLOAT_PACKED_UNITY);
        mClientProxy->setSendLevel(0.0);
        mClientProxy->setSampleRate(sampleRate);
    } else {
        ALOGW("%s(%d): Error creating output track on thread %d",
                __func__, mId, (int)mThreadIoHandle);
    }
}

AudioFlinger::PlaybackThread::OutputTrack::~OutputTrack()
{
    clearBufferQueue();
    // superclass destructor will now delete the server proxy and shared memory both refer to
}

status_t AudioFlinger::PlaybackThread::OutputTrack::start(AudioSystem::sync_event_t event,
                                                          audio_session_t triggerSession)
{
    status_t status = Track::start(event, triggerSession);
    if (status != NO_ERROR) {
        return status;
    }

    mActive = true;
    mRetryCount = 127;
    return status;
}

void AudioFlinger::PlaybackThread::OutputTrack::stop()
{
    Track::stop();
    clearBufferQueue();
    mOutBuffer.frameCount = 0;
    mActive = false;
}

ssize_t AudioFlinger::PlaybackThread::OutputTrack::write(void* data, uint32_t frames)
{
    Buffer *pInBuffer;
    Buffer inBuffer;
    bool outputBufferFull = false;
    inBuffer.frameCount = frames;
    inBuffer.raw = data;

    uint32_t waitTimeLeftMs = mSourceThread->waitTimeMs();

    if (!mActive && frames != 0) {
        (void) start();
    }

    while (waitTimeLeftMs) {
        // First write pending buffers, then new data
        if (mBufferQueue.size()) {
            pInBuffer = mBufferQueue.itemAt(0);
        } else {
            pInBuffer = &inBuffer;
        }

        if (pInBuffer->frameCount == 0) {
            break;
        }

        if (mOutBuffer.frameCount == 0) {
            mOutBuffer.frameCount = pInBuffer->frameCount;
            nsecs_t startTime = systemTime();
            status_t status = obtainBuffer(&mOutBuffer, waitTimeLeftMs);
            if (status != NO_ERROR && status != NOT_ENOUGH_DATA) {
                ALOGV("%s(%d): thread %d no more output buffers; status %d",
                        __func__, mId,
                        (int)mThreadIoHandle, status);
                outputBufferFull = true;
                break;
            }
            uint32_t waitTimeMs = (uint32_t)ns2ms(systemTime() - startTime);
            if (waitTimeLeftMs >= waitTimeMs) {
                waitTimeLeftMs -= waitTimeMs;
            } else {
                waitTimeLeftMs = 0;
            }
            if (status == NOT_ENOUGH_DATA) {
                restartIfDisabled();
                continue;
            }
        }

        uint32_t outFrames = pInBuffer->frameCount > mOutBuffer.frameCount ? mOutBuffer.frameCount :
                pInBuffer->frameCount;
        memcpy(mOutBuffer.raw, pInBuffer->raw, outFrames * mFrameSize);
        Proxy::Buffer buf;
        buf.mFrameCount = outFrames;
        buf.mRaw = NULL;
        mClientProxy->releaseBuffer(&buf);
        restartIfDisabled();
        pInBuffer->frameCount -= outFrames;
        pInBuffer->raw = (int8_t *)pInBuffer->raw + outFrames * mFrameSize;
        mOutBuffer.frameCount -= outFrames;
        mOutBuffer.raw = (int8_t *)mOutBuffer.raw + outFrames * mFrameSize;

        if (pInBuffer->frameCount == 0) {
            if (mBufferQueue.size()) {
                mBufferQueue.removeAt(0);
                free(pInBuffer->mBuffer);
                if (pInBuffer != &inBuffer) {
                    delete pInBuffer;
                }
                ALOGV("%s(%d): thread %d released overflow buffer %zu",
                        __func__, mId,
                        (int)mThreadIoHandle, mBufferQueue.size());
            } else {
                break;
            }
        }
    }

    // If we could not write all frames, allocate a buffer and queue it for next time.
    if (inBuffer.frameCount) {
        sp<ThreadBase> thread = mThread.promote();
        if (thread != 0 && !thread->standby()) {
            if (mBufferQueue.size() < kMaxOverFlowBuffers) {
                pInBuffer = new Buffer;
                pInBuffer->mBuffer = malloc(inBuffer.frameCount * mFrameSize);
                pInBuffer->frameCount = inBuffer.frameCount;
                pInBuffer->raw = pInBuffer->mBuffer;
                memcpy(pInBuffer->raw, inBuffer.raw, inBuffer.frameCount * mFrameSize);
                mBufferQueue.add(pInBuffer);
                ALOGV("%s(%d): thread %d adding overflow buffer %zu", __func__, mId,
                        (int)mThreadIoHandle, mBufferQueue.size());
                // audio data is consumed (stored locally); set frameCount to 0.
                inBuffer.frameCount = 0;
            } else {
                ALOGW("%s(%d): thread %d no more overflow buffers",
                        __func__, mId, (int)mThreadIoHandle);
                // TODO: return error for this.
            }
        }
    }

    // Calling write() with a 0 length buffer means that no more data will be written:
    // We rely on stop() to set the appropriate flags to allow the remaining frames to play out.
    if (frames == 0 && mBufferQueue.size() == 0 && mActive) {
        stop();
    }

    return frames - inBuffer.frameCount;  // number of frames consumed.
}

void AudioFlinger::PlaybackThread::OutputTrack::copyMetadataTo(MetadataInserter& backInserter) const
{
    std::lock_guard<std::mutex> lock(mTrackMetadatasMutex);
    backInserter = std::copy(mTrackMetadatas.begin(), mTrackMetadatas.end(), backInserter);
}

void AudioFlinger::PlaybackThread::OutputTrack::setMetadatas(const SourceMetadatas& metadatas) {
    {
        std::lock_guard<std::mutex> lock(mTrackMetadatasMutex);
        mTrackMetadatas = metadatas;
    }
    // No need to adjust metadata track volumes as OutputTrack volumes are always 0dBFS.
    setMetadataHasChanged();
}

status_t AudioFlinger::PlaybackThread::OutputTrack::obtainBuffer(
        AudioBufferProvider::Buffer* buffer, uint32_t waitTimeMs)
{
    ClientProxy::Buffer buf;
    buf.mFrameCount = buffer->frameCount;
    struct timespec timeout;
    timeout.tv_sec = waitTimeMs / 1000;
    timeout.tv_nsec = (int) (waitTimeMs % 1000) * 1000000;
    status_t status = mClientProxy->obtainBuffer(&buf, &timeout);
    buffer->frameCount = buf.mFrameCount;
    buffer->raw = buf.mRaw;
    return status;
}

void AudioFlinger::PlaybackThread::OutputTrack::clearBufferQueue()
{
    size_t size = mBufferQueue.size();

    for (size_t i = 0; i < size; i++) {
        Buffer *pBuffer = mBufferQueue.itemAt(i);
        free(pBuffer->mBuffer);
        delete pBuffer;
    }
    mBufferQueue.clear();
}

void AudioFlinger::PlaybackThread::OutputTrack::restartIfDisabled()
{
    int32_t flags = android_atomic_and(~CBLK_DISABLED, &mCblk->mFlags);
    if (mActive && (flags & CBLK_DISABLED)) {
        start();
    }
}

// ----------------------------------------------------------------------------
#undef LOG_TAG
#define LOG_TAG "AF::PatchTrack"

AudioFlinger::PlaybackThread::PatchTrack::PatchTrack(PlaybackThread *playbackThread,
                                                     audio_stream_type_t streamType,
                                                     uint32_t sampleRate,
                                                     audio_channel_mask_t channelMask,
                                                     audio_format_t format,
                                                     size_t frameCount,
                                                     void *buffer,
                                                     size_t bufferSize,
                                                     audio_output_flags_t flags,
                                                     const Timeout& timeout,
                                                     size_t frameCountToBeReady)
    :   Track(playbackThread, NULL, streamType,
              audio_attributes_t{} /* currently unused for patch track */,
              sampleRate, format, channelMask, frameCount,
              buffer, bufferSize, nullptr /* sharedBuffer */,
              AUDIO_SESSION_NONE, getpid(), AID_AUDIOSERVER, flags, TYPE_PATCH,
              AUDIO_PORT_HANDLE_NONE, frameCountToBeReady),
        PatchTrackBase(new ClientProxy(mCblk, mBuffer, frameCount, mFrameSize, true, true),
                       *playbackThread, timeout)
{
    ALOGV("%s(%d): sampleRate %d mPeerTimeout %d.%03d sec",
                                      __func__, mId, sampleRate,
                                      (int)mPeerTimeout.tv_sec,
                                      (int)(mPeerTimeout.tv_nsec / 1000000));
}

AudioFlinger::PlaybackThread::PatchTrack::~PatchTrack()
{
    ALOGV("%s(%d)", __func__, mId);
}

status_t AudioFlinger::PlaybackThread::PatchTrack::start(AudioSystem::sync_event_t event,
                                                         audio_session_t triggerSession)
{
    status_t status = Track::start(event, triggerSession);
    if (status != NO_ERROR) {
        return status;
    }
    android_atomic_and(~CBLK_DISABLED, &mCblk->mFlags);
    return status;
}

// AudioBufferProvider interface
status_t AudioFlinger::PlaybackThread::PatchTrack::getNextBuffer(
        AudioBufferProvider::Buffer* buffer)
{
    ALOG_ASSERT(mPeerProxy != 0, "%s(%d): called without peer proxy", __func__, mId);
    Proxy::Buffer buf;
    buf.mFrameCount = buffer->frameCount;
    status_t status = mPeerProxy->obtainBuffer(&buf, &mPeerTimeout);
    ALOGV_IF(status != NO_ERROR, "%s(%d): getNextBuffer status %d", __func__, mId, status);
    buffer->frameCount = buf.mFrameCount;
    if (buf.mFrameCount == 0) {
        return WOULD_BLOCK;
    }
    status = Track::getNextBuffer(buffer);
    return status;
}

void AudioFlinger::PlaybackThread::PatchTrack::releaseBuffer(AudioBufferProvider::Buffer* buffer)
{
    ALOG_ASSERT(mPeerProxy != 0, "%s(%d): called without peer proxy", __func__, mId);
    Proxy::Buffer buf;
    buf.mFrameCount = buffer->frameCount;
    buf.mRaw = buffer->raw;
    mPeerProxy->releaseBuffer(&buf);
    TrackBase::releaseBuffer(buffer);
}

status_t AudioFlinger::PlaybackThread::PatchTrack::obtainBuffer(Proxy::Buffer* buffer,
                                                                const struct timespec *timeOut)
{
    status_t status = NO_ERROR;
    static const int32_t kMaxTries = 5;
    int32_t tryCounter = kMaxTries;
    const size_t originalFrameCount = buffer->mFrameCount;
    do {
        if (status == NOT_ENOUGH_DATA) {
            restartIfDisabled();
            buffer->mFrameCount = originalFrameCount; // cleared on error, must be restored.
        }
        status = mProxy->obtainBuffer(buffer, timeOut);
    } while ((status == NOT_ENOUGH_DATA) && (tryCounter-- > 0));
    return status;
}

void AudioFlinger::PlaybackThread::PatchTrack::releaseBuffer(Proxy::Buffer* buffer)
{
    mProxy->releaseBuffer(buffer);
    restartIfDisabled();
}

void AudioFlinger::PlaybackThread::PatchTrack::restartIfDisabled()
{
    if (android_atomic_and(~CBLK_DISABLED, &mCblk->mFlags) & CBLK_DISABLED) {
        ALOGW("%s(%d): disabled due to previous underrun, restarting", __func__, mId);
        start();
    }
}

// ----------------------------------------------------------------------------
//      Record
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
//      AppOp for audio recording
// -------------------------------

#undef LOG_TAG
#define LOG_TAG "AF::OpRecordAudioMonitor"

// static
sp<AudioFlinger::RecordThread::OpRecordAudioMonitor>
AudioFlinger::RecordThread::OpRecordAudioMonitor::createIfNeeded(
            uid_t uid, const String16& opPackageName)
{
    if (isServiceUid(uid)) {
        ALOGV("not silencing record for service uid:%d pack:%s",
                uid, String8(opPackageName).string());
        return nullptr;
    }

    if (opPackageName.size() == 0) {
        Vector<String16> packages;
        // no package name, happens with SL ES clients
        // query package manager to find one
        PermissionController permissionController;
        permissionController.getPackagesForUid(uid, packages);
        if (packages.isEmpty()) {
            return nullptr;
        } else {
            ALOGV("using pack:%s for uid:%d", String8(packages[0]).string(), uid);
            return new OpRecordAudioMonitor(uid, packages[0]);
        }
    }

    return new OpRecordAudioMonitor(uid, opPackageName);
}

AudioFlinger::RecordThread::OpRecordAudioMonitor::OpRecordAudioMonitor(
        uid_t uid, const String16& opPackageName)
        : mHasOpRecordAudio(true), mUid(uid), mPackage(opPackageName)
{
}

AudioFlinger::RecordThread::OpRecordAudioMonitor::~OpRecordAudioMonitor()
{
    if (mOpCallback != 0) {
        mAppOpsManager.stopWatchingMode(mOpCallback);
    }
    mOpCallback.clear();
}

void AudioFlinger::RecordThread::OpRecordAudioMonitor::onFirstRef()
{
    checkRecordAudio();
    mOpCallback = new RecordAudioOpCallback(this);
    ALOGV("start watching OP_RECORD_AUDIO for pack:%s", String8(mPackage).string());
    mAppOpsManager.startWatchingMode(AppOpsManager::OP_RECORD_AUDIO, mPackage, mOpCallback);
}

bool AudioFlinger::RecordThread::OpRecordAudioMonitor::hasOpRecordAudio() const {
    return mHasOpRecordAudio.load();
}

// Called by RecordAudioOpCallback when OP_RECORD_AUDIO is updated in AppOp callback
// and in onFirstRef()
// Note this method is never called (and never to be) for audio server / root track
// due to the UID in createIfNeeded(). As a result for those record track, it's:
// - not called from constructor,
// - not called from RecordAudioOpCallback because the callback is not installed in this case
void AudioFlinger::RecordThread::OpRecordAudioMonitor::checkRecordAudio()
{
    const int32_t mode = mAppOpsManager.checkOp(AppOpsManager::OP_RECORD_AUDIO,
            mUid, mPackage);
    const bool hasIt =  (mode == AppOpsManager::MODE_ALLOWED);
    // verbose logging only log when appOp changed
    ALOGI_IF(hasIt != mHasOpRecordAudio.load(),
            "OP_RECORD_AUDIO missing, %ssilencing record uid%d pack:%s",
            hasIt ? "un" : "", mUid, String8(mPackage).string());
    mHasOpRecordAudio.store(hasIt);
}

AudioFlinger::RecordThread::OpRecordAudioMonitor::RecordAudioOpCallback::RecordAudioOpCallback(
        const wp<OpRecordAudioMonitor>& monitor) : mMonitor(monitor)
{ }

void AudioFlinger::RecordThread::OpRecordAudioMonitor::RecordAudioOpCallback::opChanged(int32_t op,
            const String16& packageName) {
    UNUSED(packageName);
    if (op != AppOpsManager::OP_RECORD_AUDIO) {
        return;
    }
    sp<OpRecordAudioMonitor> monitor = mMonitor.promote();
    if (monitor != NULL) {
        monitor->checkRecordAudio();
    }
}



#undef LOG_TAG
#define LOG_TAG "AF::RecordHandle"

AudioFlinger::RecordHandle::RecordHandle(
        const sp<AudioFlinger::RecordThread::RecordTrack>& recordTrack)
    : BnAudioRecord(),
    mRecordTrack(recordTrack)
{
}

AudioFlinger::RecordHandle::~RecordHandle() {
    stop_nonvirtual();
    mRecordTrack->destroy();
}

binder::Status AudioFlinger::RecordHandle::start(int /*AudioSystem::sync_event_t*/ event,
        int /*audio_session_t*/ triggerSession) {
    ALOGV("%s()", __func__);
    return binder::Status::fromStatusT(
        mRecordTrack->start((AudioSystem::sync_event_t)event, (audio_session_t) triggerSession));
}

binder::Status AudioFlinger::RecordHandle::stop() {
    stop_nonvirtual();
    return binder::Status::ok();
}

void AudioFlinger::RecordHandle::stop_nonvirtual() {
    ALOGV("%s()", __func__);
    mRecordTrack->stop();
}

binder::Status AudioFlinger::RecordHandle::getActiveMicrophones(
        std::vector<media::MicrophoneInfo>* activeMicrophones) {
    ALOGV("%s()", __func__);
    return binder::Status::fromStatusT(
            mRecordTrack->getActiveMicrophones(activeMicrophones));
}

binder::Status AudioFlinger::RecordHandle::setPreferredMicrophoneDirection(
        int /*audio_microphone_direction_t*/ direction) {
    ALOGV("%s()", __func__);
    return binder::Status::fromStatusT(mRecordTrack->setPreferredMicrophoneDirection(
            static_cast<audio_microphone_direction_t>(direction)));
}

binder::Status AudioFlinger::RecordHandle::setPreferredMicrophoneFieldDimension(float zoom) {
    ALOGV("%s()", __func__);
    return binder::Status::fromStatusT(mRecordTrack->setPreferredMicrophoneFieldDimension(zoom));
}

// ----------------------------------------------------------------------------
#undef LOG_TAG
#define LOG_TAG "AF::RecordTrack"

// RecordTrack constructor must be called with AudioFlinger::mLock and ThreadBase::mLock held
AudioFlinger::RecordThread::RecordTrack::RecordTrack(
            RecordThread *thread,
            const sp<Client>& client,
            const audio_attributes_t& attr,
            uint32_t sampleRate,
            audio_format_t format,
            audio_channel_mask_t channelMask,
            size_t frameCount,
            void *buffer,
            size_t bufferSize,
            audio_session_t sessionId,
            pid_t creatorPid,
            uid_t uid,
            audio_input_flags_t flags,
            track_type type,
            const String16& opPackageName,
            audio_port_handle_t portId)
    :   TrackBase(thread, client, attr, sampleRate, format,
                  channelMask, frameCount, buffer, bufferSize, sessionId,
                  creatorPid, uid, false /*isOut*/,
                  (type == TYPE_DEFAULT) ?
                          ((flags & AUDIO_INPUT_FLAG_FAST) ? ALLOC_PIPE : ALLOC_CBLK) :
                          ((buffer == NULL) ? ALLOC_LOCAL : ALLOC_NONE),
                  type, portId),
        mOverflow(false),
        mFramesToDrop(0),
        mResamplerBufferProvider(NULL), // initialize in case of early constructor exit
        mRecordBufferConverter(NULL),
        mFlags(flags),
        mSilenced(false),
        mOpRecordAudioMonitor(OpRecordAudioMonitor::createIfNeeded(uid, opPackageName))
{
    if (mCblk == NULL) {
        return;
    }

    if (!isDirect()) {
        mRecordBufferConverter = new RecordBufferConverter(
                thread->mChannelMask, thread->mFormat, thread->mSampleRate,
                channelMask, format, sampleRate);
        // Check if the RecordBufferConverter construction was successful.
        // If not, don't continue with construction.
        //
        // NOTE: It would be extremely rare that the record track cannot be created
        // for the current device, but a pending or future device change would make
        // the record track configuration valid.
        if (mRecordBufferConverter->initCheck() != NO_ERROR) {
            ALOGE("%s(%d): RecordTrack unable to create record buffer converter", __func__, mId);
            return;
        }
    }

    mServerProxy = new AudioRecordServerProxy(mCblk, mBuffer, frameCount,
            mFrameSize, !isExternalTrack());

    mResamplerBufferProvider = new ResamplerBufferProvider(this);

    if (flags & AUDIO_INPUT_FLAG_FAST) {
        ALOG_ASSERT(thread->mFastTrackAvail);
        thread->mFastTrackAvail = false;
    } else {
        // TODO: only Normal Record has timestamps (Fast Record does not).
        mServerLatencySupported = checkServerLatencySupported(mFormat, flags);
    }
#ifdef TEE_SINK
    mTee.setId(std::string("_") + std::to_string(mThreadIoHandle)
            + "_" + std::to_string(mId)
            + "_R");
#endif
}

AudioFlinger::RecordThread::RecordTrack::~RecordTrack()
{
    ALOGV("%s()", __func__);
    delete mRecordBufferConverter;
    delete mResamplerBufferProvider;
}

status_t AudioFlinger::RecordThread::RecordTrack::initCheck() const
{
    status_t status = TrackBase::initCheck();
    if (status == NO_ERROR && mServerProxy == 0) {
        status = BAD_VALUE;
    }
    return status;
}

// AudioBufferProvider interface
status_t AudioFlinger::RecordThread::RecordTrack::getNextBuffer(AudioBufferProvider::Buffer* buffer)
{
    ServerProxy::Buffer buf;
    buf.mFrameCount = buffer->frameCount;
    status_t status = mServerProxy->obtainBuffer(&buf);
    buffer->frameCount = buf.mFrameCount;
    buffer->raw = buf.mRaw;
    if (buf.mFrameCount == 0) {
        // FIXME also wake futex so that overrun is noticed more quickly
        (void) android_atomic_or(CBLK_OVERRUN, &mCblk->mFlags);
    }
    return status;
}

status_t AudioFlinger::RecordThread::RecordTrack::start(AudioSystem::sync_event_t event,
                                                        audio_session_t triggerSession)
{
    sp<ThreadBase> thread = mThread.promote();
    if (thread != 0) {
        RecordThread *recordThread = (RecordThread *)thread.get();
        return recordThread->start(this, event, triggerSession);
    } else {
        return BAD_VALUE;
    }
}

void AudioFlinger::RecordThread::RecordTrack::stop()
{
    sp<ThreadBase> thread = mThread.promote();
    if (thread != 0) {
        RecordThread *recordThread = (RecordThread *)thread.get();
        if (recordThread->stop(this) && isExternalTrack()) {
            AudioSystem::stopInput(mPortId);
        }
    }
}

void AudioFlinger::RecordThread::RecordTrack::destroy()
{
    // see comments at AudioFlinger::PlaybackThread::Track::destroy()
    sp<RecordTrack> keep(this);
    {
        track_state priorState = mState;
        sp<ThreadBase> thread = mThread.promote();
        if (thread != 0) {
            Mutex::Autolock _l(thread->mLock);
            RecordThread *recordThread = (RecordThread *) thread.get();
            priorState = mState;
            recordThread->destroyTrack_l(this); // move mState to STOPPED, terminate
        }
        // APM portid/client management done outside of lock.
        // NOTE: if thread doesn't exist, the input descriptor probably doesn't either.
        if (isExternalTrack()) {
            switch (priorState) {
            case ACTIVE:     // invalidated while still active
            case STARTING_2: // invalidated/start-aborted after startInput successfully called
            case PAUSING:    // invalidated while in the middle of stop() pausing (still active)
                AudioSystem::stopInput(mPortId);
                break;

            case STARTING_1: // invalidated/start-aborted and startInput not successful
            case PAUSED:     // OK, not active
            case IDLE:       // OK, not active
                break;

            case STOPPED:    // unexpected (destroyed)
            default:
                LOG_ALWAYS_FATAL("%s(%d): invalid prior state: %d", __func__, mId, priorState);
            }
            AudioSystem::releaseInput(mPortId);
        }
    }
}

void AudioFlinger::RecordThread::RecordTrack::invalidate()
{
    TrackBase::invalidate();
    // FIXME should use proxy, and needs work
    audio_track_cblk_t* cblk = mCblk;
    android_atomic_or(CBLK_INVALID, &cblk->mFlags);
    android_atomic_release_store(0x40000000, &cblk->mFutex);
    // client is not in server, so FUTEX_WAKE is needed instead of FUTEX_WAKE_PRIVATE
    (void) syscall(__NR_futex, &cblk->mFutex, FUTEX_WAKE, INT_MAX);
}


void AudioFlinger::RecordThread::RecordTrack::appendDumpHeader(String8& result)
{
    result.appendFormat("Active     Id Client Session Port Id  S  Flags  "
                        " Format Chn mask  SRate Source  "
                        " Server FrmCnt FrmRdy Sil%s\n",
                        isServerLatencySupported() ? "   Latency" : "");
}

void AudioFlinger::RecordThread::RecordTrack::appendDump(String8& result, bool active)
{
    result.appendFormat("%c%5s %6d %6u %7u %7u  %2s 0x%03X "
            "%08X %08X %6u %6X "
            "%08X %6zu %6zu %3c",
            isFastTrack() ? 'F' : ' ',
            active ? "yes" : "no",
            mId,
            (mClient == 0) ? getpid() : mClient->pid(),
            mSessionId,
            mPortId,
            getTrackStateString(),
            mCblk->mFlags,

            mFormat,
            mChannelMask,
            mSampleRate,
            mAttr.source,

            mCblk->mServer,
            mFrameCount,
            mServerProxy->framesReadySafe(),
            isSilenced() ? 's' : 'n'
            );
    if (isServerLatencySupported()) {
        double latencyMs;
        bool fromTrack;
        if (getTrackLatencyMs(&latencyMs, &fromTrack) == OK) {
            // Show latency in msec, followed by 't' if from track timestamp (the most accurate)
            // or 'k' if estimated from kernel (usually for debugging).
            result.appendFormat(" %7.2lf %c", latencyMs, fromTrack ? 't' : 'k');
        } else {
            result.appendFormat("%10s", mCblk->mServer != 0 ? "unavail" : "new");
        }
    }
    result.append("\n");
}

void AudioFlinger::RecordThread::RecordTrack::handleSyncStartEvent(const sp<SyncEvent>& event)
{
    if (event == mSyncStartEvent) {
        ssize_t framesToDrop = 0;
        sp<ThreadBase> threadBase = mThread.promote();
        if (threadBase != 0) {
            // TODO: use actual buffer filling status instead of 2 buffers when info is available
            // from audio HAL
            framesToDrop = threadBase->mFrameCount * 2;
        }
        mFramesToDrop = framesToDrop;
    }
}

void AudioFlinger::RecordThread::RecordTrack::clearSyncStartEvent()
{
    if (mSyncStartEvent != 0) {
        mSyncStartEvent->cancel();
        mSyncStartEvent.clear();
    }
    mFramesToDrop = 0;
}

void AudioFlinger::RecordThread::RecordTrack::updateTrackFrameInfo(
        int64_t trackFramesReleased, int64_t sourceFramesRead,
        uint32_t halSampleRate, const ExtendedTimestamp &timestamp)
{
   // Make the kernel frametime available.
    const FrameTime ft{
            timestamp.mPosition[ExtendedTimestamp::LOCATION_KERNEL],
            timestamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL]};
    // ALOGD("FrameTime: %lld %lld", (long long)ft.frames, (long long)ft.timeNs);
    mKernelFrameTime.store(ft);
    if (!audio_is_linear_pcm(mFormat)) {
        return;
    }

    ExtendedTimestamp local = timestamp;

    // Convert HAL frames to server-side track frames at track sample rate.
    // We use trackFramesReleased and sourceFramesRead as an anchor point.
    for (int i = ExtendedTimestamp::LOCATION_SERVER; i < ExtendedTimestamp::LOCATION_MAX; ++i) {
        if (local.mTimeNs[i] != 0) {
            const int64_t relativeServerFrames = local.mPosition[i] - sourceFramesRead;
            const int64_t relativeTrackFrames = relativeServerFrames
                    * mSampleRate / halSampleRate; // TODO: potential computation overflow
            local.mPosition[i] = relativeTrackFrames + trackFramesReleased;
        }
    }
    mServerProxy->setTimestamp(local);

    // Compute latency info.
    const bool useTrackTimestamp = true; // use track unless debugging.
    const double latencyMs = - (useTrackTimestamp
            ? local.getOutputServerLatencyMs(sampleRate())
            : timestamp.getOutputServerLatencyMs(halSampleRate));

    mServerLatencyFromTrack.store(useTrackTimestamp);
    mServerLatencyMs.store(latencyMs);
}

bool AudioFlinger::RecordThread::RecordTrack::isSilenced() const {
    if (mSilenced) {
        return true;
    }
    // The monitor is only created for record tracks that can be silenced.
    return mOpRecordAudioMonitor ? !mOpRecordAudioMonitor->hasOpRecordAudio() : false;
}

status_t AudioFlinger::RecordThread::RecordTrack::getActiveMicrophones(
        std::vector<media::MicrophoneInfo>* activeMicrophones)
{
    sp<ThreadBase> thread = mThread.promote();
    if (thread != 0) {
        RecordThread *recordThread = (RecordThread *)thread.get();
        return recordThread->getActiveMicrophones(activeMicrophones);
    } else {
        return BAD_VALUE;
    }
}

status_t AudioFlinger::RecordThread::RecordTrack::setPreferredMicrophoneDirection(
        audio_microphone_direction_t direction) {
    sp<ThreadBase> thread = mThread.promote();
    if (thread != 0) {
        RecordThread *recordThread = (RecordThread *)thread.get();
        return recordThread->setPreferredMicrophoneDirection(direction);
    } else {
        return BAD_VALUE;
    }
}

status_t AudioFlinger::RecordThread::RecordTrack::setPreferredMicrophoneFieldDimension(float zoom) {
    sp<ThreadBase> thread = mThread.promote();
    if (thread != 0) {
        RecordThread *recordThread = (RecordThread *)thread.get();
        return recordThread->setPreferredMicrophoneFieldDimension(zoom);
    } else {
        return BAD_VALUE;
    }
}

// ----------------------------------------------------------------------------
#undef LOG_TAG
#define LOG_TAG "AF::PatchRecord"

AudioFlinger::RecordThread::PatchRecord::PatchRecord(RecordThread *recordThread,
                                                     uint32_t sampleRate,
                                                     audio_channel_mask_t channelMask,
                                                     audio_format_t format,
                                                     size_t frameCount,
                                                     void *buffer,
                                                     size_t bufferSize,
                                                     audio_input_flags_t flags,
                                                     const Timeout& timeout)
    :   RecordTrack(recordThread, NULL,
                audio_attributes_t{} /* currently unused for patch track */,
                sampleRate, format, channelMask, frameCount,
                buffer, bufferSize, AUDIO_SESSION_NONE, getpid(), AID_AUDIOSERVER,
                flags, TYPE_PATCH, String16()),
        PatchTrackBase(new ClientProxy(mCblk, mBuffer, frameCount, mFrameSize, false, true),
                       *recordThread, timeout)
{
    ALOGV("%s(%d): sampleRate %d mPeerTimeout %d.%03d sec",
                                      __func__, mId, sampleRate,
                                      (int)mPeerTimeout.tv_sec,
                                      (int)(mPeerTimeout.tv_nsec / 1000000));
}

AudioFlinger::RecordThread::PatchRecord::~PatchRecord()
{
    ALOGV("%s(%d)", __func__, mId);
}

// AudioBufferProvider interface
status_t AudioFlinger::RecordThread::PatchRecord::getNextBuffer(
                                                  AudioBufferProvider::Buffer* buffer)
{
    ALOG_ASSERT(mPeerProxy != 0, "%s(%d): called without peer proxy", __func__, mId);
    Proxy::Buffer buf;
    buf.mFrameCount = buffer->frameCount;
    status_t status = mPeerProxy->obtainBuffer(&buf, &mPeerTimeout);
    ALOGV_IF(status != NO_ERROR,
             "%s(%d): mPeerProxy->obtainBuffer status %d", __func__, mId, status);
    buffer->frameCount = buf.mFrameCount;
    if (buf.mFrameCount == 0) {
        return WOULD_BLOCK;
    }
    status = RecordTrack::getNextBuffer(buffer);
    return status;
}

void AudioFlinger::RecordThread::PatchRecord::releaseBuffer(AudioBufferProvider::Buffer* buffer)
{
    ALOG_ASSERT(mPeerProxy != 0, "%s(%d): called without peer proxy", __func__, mId);
    Proxy::Buffer buf;
    buf.mFrameCount = buffer->frameCount;
    buf.mRaw = buffer->raw;
    mPeerProxy->releaseBuffer(&buf);
    TrackBase::releaseBuffer(buffer);
}

status_t AudioFlinger::RecordThread::PatchRecord::obtainBuffer(Proxy::Buffer* buffer,
                                                               const struct timespec *timeOut)
{
    return mProxy->obtainBuffer(buffer, timeOut);
}

void AudioFlinger::RecordThread::PatchRecord::releaseBuffer(Proxy::Buffer* buffer)
{
    mProxy->releaseBuffer(buffer);
}

// ----------------------------------------------------------------------------
#undef LOG_TAG
#define LOG_TAG "AF::MmapTrack"

AudioFlinger::MmapThread::MmapTrack::MmapTrack(ThreadBase *thread,
        const audio_attributes_t& attr,
        uint32_t sampleRate,
        audio_format_t format,
        audio_channel_mask_t channelMask,
        audio_session_t sessionId,
        bool isOut,
        uid_t uid,
        pid_t pid,
        pid_t creatorPid,
        audio_port_handle_t portId)
    :   TrackBase(thread, NULL, attr, sampleRate, format,
                  channelMask, (size_t)0 /* frameCount */,
                  nullptr /* buffer */, (size_t)0 /* bufferSize */,
                  sessionId, creatorPid, uid, isOut,
                  ALLOC_NONE,
                  TYPE_DEFAULT, portId),
        mPid(pid), mSilenced(false), mSilencedNotified(false)
{
}

AudioFlinger::MmapThread::MmapTrack::~MmapTrack()
{
}

status_t AudioFlinger::MmapThread::MmapTrack::initCheck() const
{
    return NO_ERROR;
}

status_t AudioFlinger::MmapThread::MmapTrack::start(AudioSystem::sync_event_t event __unused,
                                                    audio_session_t triggerSession __unused)
{
    return NO_ERROR;
}

void AudioFlinger::MmapThread::MmapTrack::stop()
{
}

// AudioBufferProvider interface
status_t AudioFlinger::MmapThread::MmapTrack::getNextBuffer(AudioBufferProvider::Buffer* buffer)
{
    buffer->frameCount = 0;
    buffer->raw = nullptr;
    return INVALID_OPERATION;
}

// ExtendedAudioBufferProvider interface
size_t AudioFlinger::MmapThread::MmapTrack::framesReady() const {
    return 0;
}

int64_t AudioFlinger::MmapThread::MmapTrack::framesReleased() const
{
    return 0;
}

void AudioFlinger::MmapThread::MmapTrack::onTimestamp(const ExtendedTimestamp &timestamp __unused)
{
}

void AudioFlinger::MmapThread::MmapTrack::appendDumpHeader(String8& result)
{
    result.appendFormat("Client Session Port Id  Format Chn mask  SRate Flags %s\n",
                        isOut() ? "Usg CT": "Source");
}

void AudioFlinger::MmapThread::MmapTrack::appendDump(String8& result, bool active __unused)
{
    result.appendFormat("%6u %7u %7u %08X %08X %6u 0x%03X ",
            mPid,
            mSessionId,
            mPortId,
            mFormat,
            mChannelMask,
            mSampleRate,
            mAttr.flags);
    if (isOut()) {
        result.appendFormat("%3x %2x", mAttr.usage, mAttr.content_type);
    } else {
        result.appendFormat("%6x", mAttr.source);
    }
    result.append("\n");
}

} // namespace android
