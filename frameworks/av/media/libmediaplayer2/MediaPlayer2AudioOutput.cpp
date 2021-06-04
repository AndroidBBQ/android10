/*
**
** Copyright 2018, The Android Open Source Project
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
#define LOG_TAG "MediaPlayer2AudioOutput"
#include <mediaplayer2/MediaPlayer2AudioOutput.h>

#include <cutils/properties.h> // for property_get
#include <utils/Log.h>

#include <media/stagefright/foundation/ADebug.h>

namespace {

const float kMaxRequiredSpeed = 8.0f; // for PCM tracks allow up to 8x speedup.

} // anonymous namespace

namespace android {

// TODO: Find real cause of Audio/Video delay in PV framework and remove this workaround
/* static */ int MediaPlayer2AudioOutput::mMinBufferCount = 4;
/* static */ bool MediaPlayer2AudioOutput::mIsOnEmulator = false;

status_t MediaPlayer2AudioOutput::dump(int fd, const Vector<String16>& args) const {
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;

    result.append(" MediaPlayer2AudioOutput\n");
    snprintf(buffer, 255, "  volume(%f)\n", mVolume);
    result.append(buffer);
    snprintf(buffer, 255, "  msec per frame(%f), latency (%d)\n",
            mMsecsPerFrame, (mJAudioTrack != nullptr) ? mJAudioTrack->latency() : -1);
    result.append(buffer);
    snprintf(buffer, 255, "  aux effect id(%d), send level (%f)\n",
            mAuxEffectId, mSendLevel);
    result.append(buffer);

    ::write(fd, result.string(), result.size());
    if (mJAudioTrack != nullptr) {
        mJAudioTrack->dump(fd, args);
    }
    return NO_ERROR;
}

MediaPlayer2AudioOutput::MediaPlayer2AudioOutput(int32_t sessionId, uid_t uid, int pid,
        const jobject attributes)
    : mCallback(nullptr),
      mCallbackCookie(nullptr),
      mCallbackData(nullptr),
      mVolume(1.0),
      mPlaybackRate(AUDIO_PLAYBACK_RATE_DEFAULT),
      mSampleRateHz(0),
      mMsecsPerFrame(0),
      mFrameSize(0),
      mSessionId(sessionId),
      mUid(uid),
      mPid(pid),
      mSendLevel(0.0),
      mAuxEffectId(0),
      mFlags(AUDIO_OUTPUT_FLAG_NONE) {
    ALOGV("MediaPlayer2AudioOutput(%d)", sessionId);

    if (attributes != nullptr) {
        mAttributes = new JObjectHolder(attributes);
    }

    setMinBufferCount();
    mRoutingDelegates.clear();
}

MediaPlayer2AudioOutput::~MediaPlayer2AudioOutput() {
    close();
    delete mCallbackData;
}

//static
void MediaPlayer2AudioOutput::setMinBufferCount() {
    char value[PROPERTY_VALUE_MAX];
    if (property_get("ro.kernel.qemu", value, 0)) {
        mIsOnEmulator = true;
        mMinBufferCount = 12;  // to prevent systematic buffer underrun for emulator
    }
}

// static
bool MediaPlayer2AudioOutput::isOnEmulator() {
    setMinBufferCount();  // benign race wrt other threads
    return mIsOnEmulator;
}

// static
int MediaPlayer2AudioOutput::getMinBufferCount() {
    setMinBufferCount();  // benign race wrt other threads
    return mMinBufferCount;
}

ssize_t MediaPlayer2AudioOutput::bufferSize() const {
    Mutex::Autolock lock(mLock);
    if (mJAudioTrack == nullptr) {
        return NO_INIT;
    }
    return mJAudioTrack->frameCount() * mFrameSize;
}

ssize_t MediaPlayer2AudioOutput::frameCount() const {
    Mutex::Autolock lock(mLock);
    if (mJAudioTrack == nullptr) {
        return NO_INIT;
    }
    return mJAudioTrack->frameCount();
}

ssize_t MediaPlayer2AudioOutput::channelCount() const {
    Mutex::Autolock lock(mLock);
    if (mJAudioTrack == nullptr) {
        return NO_INIT;
    }
    return mJAudioTrack->channelCount();
}

ssize_t MediaPlayer2AudioOutput::frameSize() const {
    Mutex::Autolock lock(mLock);
    if (mJAudioTrack == nullptr) {
        return NO_INIT;
    }
    return mFrameSize;
}

uint32_t MediaPlayer2AudioOutput::latency () const {
    Mutex::Autolock lock(mLock);
    if (mJAudioTrack == nullptr) {
        return 0;
    }
    return mJAudioTrack->latency();
}

float MediaPlayer2AudioOutput::msecsPerFrame() const {
    Mutex::Autolock lock(mLock);
    return mMsecsPerFrame;
}

status_t MediaPlayer2AudioOutput::getPosition(uint32_t *position) const {
    Mutex::Autolock lock(mLock);
    if (mJAudioTrack == nullptr) {
        return NO_INIT;
    }
    return mJAudioTrack->getPosition(position);
}

status_t MediaPlayer2AudioOutput::getTimestamp(AudioTimestamp &ts) const {
    Mutex::Autolock lock(mLock);
    if (mJAudioTrack == nullptr) {
        return NO_INIT;
    }
    return mJAudioTrack->getTimestamp(ts);
}

// TODO: Remove unnecessary calls to getPlayedOutDurationUs()
// as it acquires locks and may query the audio driver.
//
// Some calls could conceivably retrieve extrapolated data instead of
// accessing getTimestamp() or getPosition() every time a data buffer with
// a media time is received.
//
// Calculate duration of played samples if played at normal rate (i.e., 1.0).
int64_t MediaPlayer2AudioOutput::getPlayedOutDurationUs(int64_t nowUs) const {
    Mutex::Autolock lock(mLock);
    if (mJAudioTrack == nullptr || mSampleRateHz == 0) {
        return 0;
    }

    uint32_t numFramesPlayed;
    int64_t numFramesPlayedAtUs;
    AudioTimestamp ts;

    status_t res = mJAudioTrack->getTimestamp(ts);

    if (res == OK) {                 // case 1: mixing audio tracks and offloaded tracks.
        numFramesPlayed = ts.mPosition;
        numFramesPlayedAtUs = ts.mTime.tv_sec * 1000000LL + ts.mTime.tv_nsec / 1000;
        //ALOGD("getTimestamp: OK %d %lld", numFramesPlayed, (long long)numFramesPlayedAtUs);
    } else {                         // case 2: transitory state on start of a new track
                                     // case 3: transitory at new track or audio fast tracks.
        numFramesPlayed = 0;
        numFramesPlayedAtUs = nowUs;
        //ALOGD("getTimestamp: WOULD_BLOCK %d %lld",
        //        numFramesPlayed, (long long)numFramesPlayedAtUs);
    }

    // CHECK_EQ(numFramesPlayed & (1 << 31), 0);  // can't be negative until 12.4 hrs, test
    // TODO: remove the (int32_t) casting below as it may overflow at 12.4 hours.
    int64_t durationUs = (int64_t)((int32_t)numFramesPlayed * 1000000LL / mSampleRateHz)
            + nowUs - numFramesPlayedAtUs;
    if (durationUs < 0) {
        // Occurs when numFramesPlayed position is very small and the following:
        // (1) In case 1, the time nowUs is computed before getTimestamp() is called and
        //     numFramesPlayedAtUs is greater than nowUs by time more than numFramesPlayed.
        // (2) In case 3, using getPosition and adding mAudioSink->latency() to
        //     numFramesPlayedAtUs, by a time amount greater than numFramesPlayed.
        //
        // Both of these are transitory conditions.
        ALOGV("getPlayedOutDurationUs: negative duration %lld set to zero", (long long)durationUs);
        durationUs = 0;
    }
    ALOGV("getPlayedOutDurationUs(%lld) nowUs(%lld) frames(%u) framesAt(%lld)",
            (long long)durationUs, (long long)nowUs,
            numFramesPlayed, (long long)numFramesPlayedAtUs);
    return durationUs;
}

status_t MediaPlayer2AudioOutput::getFramesWritten(uint32_t *frameswritten) const {
    Mutex::Autolock lock(mLock);
    if (mJAudioTrack == nullptr) {
        return NO_INIT;
    }
    ExtendedTimestamp ets;
    status_t status = mJAudioTrack->getTimestamp(&ets);
    if (status == OK || status == WOULD_BLOCK) {
        *frameswritten = (uint32_t)ets.mPosition[ExtendedTimestamp::LOCATION_CLIENT];
    }
    return status;
}

void MediaPlayer2AudioOutput::setAudioAttributes(const jobject attributes) {
    Mutex::Autolock lock(mLock);
    mAttributes = (attributes == nullptr) ? nullptr : new JObjectHolder(attributes);
}

audio_stream_type_t MediaPlayer2AudioOutput::getAudioStreamType() const {
    ALOGV("getAudioStreamType");
    Mutex::Autolock lock(mLock);
    if (mJAudioTrack == nullptr) {
        return AUDIO_STREAM_DEFAULT;
    }
    return mJAudioTrack->getAudioStreamType();
}

void MediaPlayer2AudioOutput::close_l() {
    mJAudioTrack.clear();
}

status_t MediaPlayer2AudioOutput::open(
        uint32_t sampleRate, int channelCount, audio_channel_mask_t channelMask,
        audio_format_t format,
        AudioCallback cb, void *cookie,
        audio_output_flags_t flags,
        const audio_offload_info_t *offloadInfo,
        uint32_t suggestedFrameCount) {
    ALOGV("open(%u, %d, 0x%x, 0x%x, %d 0x%x)", sampleRate, channelCount, channelMask,
                format, mSessionId, flags);

    // offloading is only supported in callback mode for now.
    // offloadInfo must be present if offload flag is set
    if (((flags & AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD) != 0) &&
            ((cb == nullptr) || (offloadInfo == nullptr))) {
        return BAD_VALUE;
    }

    // compute frame count for the AudioTrack internal buffer
    const size_t frameCount =
           ((flags & AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD) != 0) ? 0 : suggestedFrameCount;

    if (channelMask == CHANNEL_MASK_USE_CHANNEL_ORDER) {
        channelMask = audio_channel_out_mask_from_count(channelCount);
        if (0 == channelMask) {
            ALOGE("open() error, can\'t derive mask for %d audio channels", channelCount);
            return NO_INIT;
        }
    }

    Mutex::Autolock lock(mLock);
    mCallback = cb;
    mCallbackCookie = cookie;

    sp<JAudioTrack> jT;
    CallbackData *newcbd = nullptr;

    ALOGV("creating new JAudioTrack");

    if (mCallback != nullptr) {
        newcbd = new CallbackData(this);
        jT = new JAudioTrack(
                 sampleRate,
                 format,
                 channelMask,
                 CallbackWrapper,
                 newcbd,
                 frameCount,
                 mSessionId,
                 mAttributes != nullptr ? mAttributes->getJObject() : nullptr,
                 1.0f);  // default value for maxRequiredSpeed
    } else {
        // TODO: Due to buffer memory concerns, we use a max target playback speed
        // based on mPlaybackRate at the time of open (instead of kMaxRequiredSpeed),
        // also clamping the target speed to 1.0 <= targetSpeed <= kMaxRequiredSpeed.
        const float targetSpeed =
                std::min(std::max(mPlaybackRate.mSpeed, 1.0f), kMaxRequiredSpeed);
        ALOGW_IF(targetSpeed != mPlaybackRate.mSpeed,
                "track target speed:%f clamped from playback speed:%f",
                targetSpeed, mPlaybackRate.mSpeed);
        jT = new JAudioTrack(
                 sampleRate,
                 format,
                 channelMask,
                 nullptr,
                 nullptr,
                 frameCount,
                 mSessionId,
                 mAttributes != nullptr ? mAttributes->getJObject() : nullptr,
                 targetSpeed);
    }

    if (jT == 0) {
        ALOGE("Unable to create audio track");
        delete newcbd;
        // t goes out of scope, so reference count drops to zero
        return NO_INIT;
    }

    CHECK((jT != nullptr) && ((mCallback == nullptr) || (newcbd != nullptr)));

    mCallbackData = newcbd;
    ALOGV("setVolume");
    jT->setVolume(mVolume);

    mSampleRateHz = sampleRate;
    mFlags = flags;
    mMsecsPerFrame = 1E3f / (mPlaybackRate.mSpeed * sampleRate);
    mFrameSize = jT->frameSize();
    mJAudioTrack = jT;

    return updateTrack_l();
}

status_t MediaPlayer2AudioOutput::updateTrack_l() {
    if (mJAudioTrack == nullptr) {
        return NO_ERROR;
    }

    status_t res = NO_ERROR;
    // Note some output devices may give us a direct track even though we don't specify it.
    // Example: Line application b/17459982.
    if ((mJAudioTrack->getFlags()
            & (AUDIO_OUTPUT_FLAG_COMPRESS_OFFLOAD | AUDIO_OUTPUT_FLAG_DIRECT)) == 0) {
        res = mJAudioTrack->setPlaybackRate(mPlaybackRate);
        if (res == NO_ERROR) {
            mJAudioTrack->setAuxEffectSendLevel(mSendLevel);
            res = mJAudioTrack->attachAuxEffect(mAuxEffectId);
        }
    }
    if (mPreferredDevice != nullptr) {
        mJAudioTrack->setPreferredDevice(mPreferredDevice->getJObject());
    }

    mJAudioTrack->registerRoutingDelegates(mRoutingDelegates);

    ALOGV("updateTrack_l() DONE status %d", res);
    return res;
}

status_t MediaPlayer2AudioOutput::start() {
    ALOGV("start");
    Mutex::Autolock lock(mLock);
    if (mCallbackData != nullptr) {
        mCallbackData->endTrackSwitch();
    }
    if (mJAudioTrack != nullptr) {
        mJAudioTrack->setVolume(mVolume);
        mJAudioTrack->setAuxEffectSendLevel(mSendLevel);
        status_t status = mJAudioTrack->start();
        return status;
    }
    return NO_INIT;
}

ssize_t MediaPlayer2AudioOutput::write(const void* buffer, size_t size, bool blocking) {
    Mutex::Autolock lock(mLock);
    LOG_ALWAYS_FATAL_IF(mCallback != nullptr, "Don't call write if supplying a callback.");

    //ALOGV("write(%p, %u)", buffer, size);
    if (mJAudioTrack != nullptr) {
        return mJAudioTrack->write(buffer, size, blocking);
    }
    return NO_INIT;
}

void MediaPlayer2AudioOutput::stop() {
    ALOGV("stop");
    Mutex::Autolock lock(mLock);
    if (mJAudioTrack != nullptr) {
        mJAudioTrack->stop();
    }
}

void MediaPlayer2AudioOutput::flush() {
    ALOGV("flush");
    Mutex::Autolock lock(mLock);
    if (mJAudioTrack != nullptr) {
        mJAudioTrack->flush();
    }
}

void MediaPlayer2AudioOutput::pause() {
    ALOGV("pause");
    Mutex::Autolock lock(mLock);
    if (mJAudioTrack != nullptr) {
        mJAudioTrack->pause();
    }
}

void MediaPlayer2AudioOutput::close() {
    ALOGV("close");
    sp<JAudioTrack> track;
    {
        Mutex::Autolock lock(mLock);
        track = mJAudioTrack;
        close_l(); // clears mJAudioTrack
    }
    // destruction of the track occurs outside of mutex.
}

void MediaPlayer2AudioOutput::setVolume(float volume) {
    ALOGV("setVolume(%f)", volume);
    Mutex::Autolock lock(mLock);
    mVolume = volume;
    if (mJAudioTrack != nullptr) {
        mJAudioTrack->setVolume(volume);
    }
}

status_t MediaPlayer2AudioOutput::setPlaybackRate(const AudioPlaybackRate &rate) {
    ALOGV("setPlaybackRate(%f %f %d %d)",
                rate.mSpeed, rate.mPitch, rate.mFallbackMode, rate.mStretchMode);
    Mutex::Autolock lock(mLock);
    if (mJAudioTrack == 0) {
        // remember rate so that we can set it when the track is opened
        mPlaybackRate = rate;
        return OK;
    }
    status_t res = mJAudioTrack->setPlaybackRate(rate);
    if (res != NO_ERROR) {
        return res;
    }
    // rate.mSpeed is always greater than 0 if setPlaybackRate succeeded
    CHECK_GT(rate.mSpeed, 0.f);
    mPlaybackRate = rate;
    if (mSampleRateHz != 0) {
        mMsecsPerFrame = 1E3f / (rate.mSpeed * mSampleRateHz);
    }
    return res;
}

status_t MediaPlayer2AudioOutput::getPlaybackRate(AudioPlaybackRate *rate) {
    ALOGV("getPlaybackRate");
    Mutex::Autolock lock(mLock);
    if (mJAudioTrack == 0) {
        return NO_INIT;
    }
    *rate = mJAudioTrack->getPlaybackRate();
    return NO_ERROR;
}

status_t MediaPlayer2AudioOutput::setAuxEffectSendLevel(float level) {
    ALOGV("setAuxEffectSendLevel(%f)", level);
    Mutex::Autolock lock(mLock);
    mSendLevel = level;
    if (mJAudioTrack != nullptr) {
        return mJAudioTrack->setAuxEffectSendLevel(level);
    }
    return NO_ERROR;
}

status_t MediaPlayer2AudioOutput::attachAuxEffect(int effectId) {
    ALOGV("attachAuxEffect(%d)", effectId);
    Mutex::Autolock lock(mLock);
    mAuxEffectId = effectId;
    if (mJAudioTrack != nullptr) {
        return mJAudioTrack->attachAuxEffect(effectId);
    }
    return NO_ERROR;
}

status_t MediaPlayer2AudioOutput::setPreferredDevice(jobject device) {
    ALOGV("setPreferredDevice");
    Mutex::Autolock lock(mLock);
    status_t ret = NO_ERROR;
    if (mJAudioTrack != nullptr) {
        ret = mJAudioTrack->setPreferredDevice(device);
    }
    if (ret == NO_ERROR) {
        mPreferredDevice = new JObjectHolder(device);
    }
    return ret;
}

jobject MediaPlayer2AudioOutput::getRoutedDevice() {
    ALOGV("getRoutedDevice");
    Mutex::Autolock lock(mLock);
    if (mJAudioTrack != nullptr) {
        return mJAudioTrack->getRoutedDevice();
    }
    return nullptr;
}

status_t MediaPlayer2AudioOutput::addAudioDeviceCallback(jobject jRoutingDelegate) {
    ALOGV("addAudioDeviceCallback");
    Mutex::Autolock lock(mLock);
    jobject listener = JAudioTrack::getListener(jRoutingDelegate);
    if (JAudioTrack::findByKey(mRoutingDelegates, listener) == nullptr) {
        sp<JObjectHolder> listenerHolder = new JObjectHolder(listener);
        jobject handler = JAudioTrack::getHandler(jRoutingDelegate);
        sp<JObjectHolder> routingDelegateHolder = new JObjectHolder(jRoutingDelegate);

        mRoutingDelegates.push_back(std::pair<sp<JObjectHolder>, sp<JObjectHolder>>(
                listenerHolder, routingDelegateHolder));

        if (mJAudioTrack != nullptr) {
            return mJAudioTrack->addAudioDeviceCallback(
                    routingDelegateHolder->getJObject(), handler);
        }
    }
    return NO_ERROR;
}

status_t MediaPlayer2AudioOutput::removeAudioDeviceCallback(jobject listener) {
    ALOGV("removeAudioDeviceCallback");
    Mutex::Autolock lock(mLock);
    jobject routingDelegate = nullptr;
    if ((routingDelegate = JAudioTrack::findByKey(mRoutingDelegates, listener)) != nullptr) {
        if (mJAudioTrack != nullptr) {
            mJAudioTrack->removeAudioDeviceCallback(routingDelegate);
        }
        JAudioTrack::eraseByKey(mRoutingDelegates, listener);
    }
    return NO_ERROR;
}

// static
void MediaPlayer2AudioOutput::CallbackWrapper(
        int event, void *cookie, void *info) {
    //ALOGV("callbackwrapper");
    CallbackData *data = (CallbackData*)cookie;
    // lock to ensure we aren't caught in the middle of a track switch.
    data->lock();
    MediaPlayer2AudioOutput *me = data->getOutput();
    JAudioTrack::Buffer *buffer = (JAudioTrack::Buffer *)info;
    if (me == nullptr) {
        // no output set, likely because the track was scheduled to be reused
        // by another player, but the format turned out to be incompatible.
        data->unlock();
        if (buffer != nullptr) {
            buffer->mSize = 0;
        }
        return;
    }

    switch(event) {
    case JAudioTrack::EVENT_MORE_DATA: {
        size_t actualSize = (*me->mCallback)(
                me, buffer->mData, buffer->mSize, me->mCallbackCookie,
                CB_EVENT_FILL_BUFFER);

        // Log when no data is returned from the callback.
        // (1) We may have no data (especially with network streaming sources).
        // (2) We may have reached the EOS and the audio track is not stopped yet.
        // Note that AwesomePlayer/AudioPlayer will only return zero size when it reaches the EOS.
        // NuPlayer2Renderer will return zero when it doesn't have data (it doesn't block to fill).
        //
        // This is a benign busy-wait, with the next data request generated 10 ms or more later;
        // nevertheless for power reasons, we don't want to see too many of these.

        ALOGV_IF(actualSize == 0 && buffer->mSize > 0, "callbackwrapper: empty buffer returned");

        buffer->mSize = actualSize;
        } break;

    case JAudioTrack::EVENT_STREAM_END:
        // currently only occurs for offloaded callbacks
        ALOGV("callbackwrapper: deliver EVENT_STREAM_END");
        (*me->mCallback)(me, nullptr /* buffer */, 0 /* size */,
                me->mCallbackCookie, CB_EVENT_STREAM_END);
        break;

    case JAudioTrack::EVENT_NEW_IAUDIOTRACK :
        ALOGV("callbackwrapper: deliver EVENT_TEAR_DOWN");
        (*me->mCallback)(me,  nullptr /* buffer */, 0 /* size */,
                me->mCallbackCookie, CB_EVENT_TEAR_DOWN);
        break;

    case JAudioTrack::EVENT_UNDERRUN:
        // This occurs when there is no data available, typically
        // when there is a failure to supply data to the AudioTrack.  It can also
        // occur in non-offloaded mode when the audio device comes out of standby.
        //
        // If an AudioTrack underruns it outputs silence. Since this happens suddenly
        // it may sound like an audible pop or glitch.
        //
        // The underrun event is sent once per track underrun; the condition is reset
        // when more data is sent to the AudioTrack.
        ALOGD("callbackwrapper: EVENT_UNDERRUN (discarded)");
        break;

    default:
        ALOGE("received unknown event type: %d inside CallbackWrapper !", event);
    }

    data->unlock();
}

int32_t MediaPlayer2AudioOutput::getSessionId() const {
    Mutex::Autolock lock(mLock);
    return mSessionId;
}

void MediaPlayer2AudioOutput::setSessionId(const int32_t sessionId) {
    Mutex::Autolock lock(mLock);
    mSessionId = sessionId;
}

uint32_t MediaPlayer2AudioOutput::getSampleRate() const {
    Mutex::Autolock lock(mLock);
    if (mJAudioTrack == 0) {
        return 0;
    }
    return mJAudioTrack->getSampleRate();
}

int64_t MediaPlayer2AudioOutput::getBufferDurationInUs() const {
    Mutex::Autolock lock(mLock);
    if (mJAudioTrack == 0) {
        return 0;
    }
    int64_t duration;
    if (mJAudioTrack->getBufferDurationInUs(&duration) != OK) {
        return 0;
    }
    return duration;
}

} // namespace android
