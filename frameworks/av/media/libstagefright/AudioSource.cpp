/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <inttypes.h>
#include <stdlib.h>

//#define LOG_NDEBUG 0
#define LOG_TAG "AudioSource"
#include <utils/Log.h>

#include <media/AudioRecord.h>
#include <media/stagefright/AudioSource.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <cutils/properties.h>

namespace android {

static void AudioRecordCallbackFunction(int event, void *user, void *info) {
    AudioSource *source = (AudioSource *) user;
    switch (event) {
        case AudioRecord::EVENT_MORE_DATA: {
            source->dataCallback(*((AudioRecord::Buffer *) info));
            break;
        }
        case AudioRecord::EVENT_OVERRUN: {
            ALOGW("AudioRecord reported overrun!");
            break;
        }
        default:
            // does nothing
            break;
    }
}

AudioSource::AudioSource(
        audio_source_t inputSource, const String16 &opPackageName,
        uint32_t sampleRate, uint32_t channelCount, uint32_t outSampleRate,
        uid_t uid, pid_t pid, audio_port_handle_t selectedDeviceId,
        audio_microphone_direction_t selectedMicDirection,
        float selectedMicFieldDimension)
    : mStarted(false),
      mSampleRate(sampleRate),
      mOutSampleRate(outSampleRate > 0 ? outSampleRate : sampleRate),
      mTrackMaxAmplitude(false),
      mStartTimeUs(0),
      mStopSystemTimeUs(-1),
      mLastFrameTimestampUs(0),
      mMaxAmplitude(0),
      mPrevSampleTimeUs(0),
      mInitialReadTimeUs(0),
      mNumFramesReceived(0),
      mNumFramesSkipped(0),
      mNumFramesLost(0),
      mNumClientOwnedBuffers(0),
      mNoMoreFramesToRead(false) {
    ALOGV("sampleRate: %u, outSampleRate: %u, channelCount: %u",
            sampleRate, outSampleRate, channelCount);
    CHECK(channelCount == 1 || channelCount == 2);
    CHECK(sampleRate > 0);

    size_t minFrameCount;
    status_t status = AudioRecord::getMinFrameCount(&minFrameCount,
                                           sampleRate,
                                           AUDIO_FORMAT_PCM_16_BIT,
                                           audio_channel_in_mask_from_count(channelCount));
    if (status == OK) {
        // make sure that the AudioRecord callback never returns more than the maximum
        // buffer size
        uint32_t frameCount = kMaxBufferSize / sizeof(int16_t) / channelCount;

        // make sure that the AudioRecord total buffer size is large enough
        size_t bufCount = 2;
        while ((bufCount * frameCount) < minFrameCount) {
            bufCount++;
        }

        mRecord = new AudioRecord(
                    inputSource, sampleRate, AUDIO_FORMAT_PCM_16_BIT,
                    audio_channel_in_mask_from_count(channelCount),
                    opPackageName,
                    (size_t) (bufCount * frameCount),
                    AudioRecordCallbackFunction,
                    this,
                    frameCount /*notificationFrames*/,
                    AUDIO_SESSION_ALLOCATE,
                    AudioRecord::TRANSFER_DEFAULT,
                    AUDIO_INPUT_FLAG_NONE,
                    uid,
                    pid,
                    NULL /*pAttributes*/,
                    selectedDeviceId,
                    selectedMicDirection,
                    selectedMicFieldDimension);
        mInitCheck = mRecord->initCheck();
        if (mInitCheck != OK) {
            mRecord.clear();
        }
    } else {
        mInitCheck = status;
    }
}

AudioSource::~AudioSource() {
    if (mStarted) {
        reset();
    }
}

status_t AudioSource::initCheck() const {
    return mInitCheck;
}

status_t AudioSource::start(MetaData *params) {
    Mutex::Autolock autoLock(mLock);
    if (mStarted) {
        return UNKNOWN_ERROR;
    }

    if (mInitCheck != OK) {
        return NO_INIT;
    }

    mTrackMaxAmplitude = false;
    mMaxAmplitude = 0;
    mInitialReadTimeUs = 0;
    mStartTimeUs = 0;
    int64_t startTimeUs;
    if (params && params->findInt64(kKeyTime, &startTimeUs)) {
        mStartTimeUs = startTimeUs;
    }
    status_t err = mRecord->start();
    if (err == OK) {
        mStarted = true;
    } else {
        mRecord.clear();
    }


    return err;
}

void AudioSource::releaseQueuedFrames_l() {
    ALOGV("releaseQueuedFrames_l");
    List<MediaBuffer *>::iterator it;
    while (!mBuffersReceived.empty()) {
        it = mBuffersReceived.begin();
        (*it)->release();
        mBuffersReceived.erase(it);
    }
}

void AudioSource::waitOutstandingEncodingFrames_l() {
    ALOGV("waitOutstandingEncodingFrames_l: %" PRId64, mNumClientOwnedBuffers);
    while (mNumClientOwnedBuffers > 0) {
        mFrameEncodingCompletionCondition.wait(mLock);
    }
}

status_t AudioSource::reset() {
    Mutex::Autolock autoLock(mLock);
    if (!mStarted) {
        return UNKNOWN_ERROR;
    }

    if (mInitCheck != OK) {
        return NO_INIT;
    }

    mStarted = false;
    mStopSystemTimeUs = -1;
    mNoMoreFramesToRead = false;
    mFrameAvailableCondition.signal();

    mRecord->stop();
    waitOutstandingEncodingFrames_l();
    releaseQueuedFrames_l();

    return OK;
}

sp<MetaData> AudioSource::getFormat() {
    Mutex::Autolock autoLock(mLock);
    if (mInitCheck != OK) {
        return 0;
    }

    sp<MetaData> meta = new MetaData;
    meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_RAW);
    meta->setInt32(kKeySampleRate, mSampleRate);
    meta->setInt32(kKeyChannelCount, mRecord->channelCount());
    meta->setInt32(kKeyMaxInputSize, kMaxBufferSize);
    meta->setInt32(kKeyPcmEncoding, kAudioEncodingPcm16bit);

    return meta;
}

void AudioSource::rampVolume(
        int32_t startFrame, int32_t rampDurationFrames,
        uint8_t *data,   size_t bytes) {

    const int32_t kShift = 14;
    int32_t fixedMultiplier = (startFrame << kShift) / rampDurationFrames;
    const int32_t nChannels = mRecord->channelCount();
    int32_t stopFrame = startFrame + bytes / sizeof(int16_t);
    int16_t *frame = (int16_t *) data;
    if (stopFrame > rampDurationFrames) {
        stopFrame = rampDurationFrames;
    }

    while (startFrame < stopFrame) {
        if (nChannels == 1) {  // mono
            frame[0] = (frame[0] * fixedMultiplier) >> kShift;
            ++frame;
            ++startFrame;
        } else {               // stereo
            frame[0] = (frame[0] * fixedMultiplier) >> kShift;
            frame[1] = (frame[1] * fixedMultiplier) >> kShift;
            frame += 2;
            startFrame += 2;
        }

        // Update the multiplier every 4 frames
        if ((startFrame & 3) == 0) {
            fixedMultiplier = (startFrame << kShift) / rampDurationFrames;
        }
    }
}

status_t AudioSource::read(
        MediaBufferBase **out, const ReadOptions * /* options */) {
    Mutex::Autolock autoLock(mLock);
    *out = NULL;

    if (mInitCheck != OK) {
        return NO_INIT;
    }

    while (mStarted && mBuffersReceived.empty()) {
        mFrameAvailableCondition.wait(mLock);
        if (mNoMoreFramesToRead) {
            return OK;
        }
    }
    if (!mStarted) {
        return OK;
    }
    MediaBuffer *buffer = *mBuffersReceived.begin();
    mBuffersReceived.erase(mBuffersReceived.begin());
    ++mNumClientOwnedBuffers;
    buffer->setObserver(this);
    buffer->add_ref();

    // Mute/suppress the recording sound
    int64_t timeUs;
    CHECK(buffer->meta_data().findInt64(kKeyTime, &timeUs));
    int64_t elapsedTimeUs = timeUs - mStartTimeUs;
    if (elapsedTimeUs < kAutoRampStartUs) {
        memset((uint8_t *) buffer->data(), 0, buffer->range_length());
    } else if (elapsedTimeUs < kAutoRampStartUs + kAutoRampDurationUs) {
        int32_t autoRampDurationFrames =
                    ((int64_t)kAutoRampDurationUs * mSampleRate + 500000LL) / 1000000LL; //Need type casting

        int32_t autoRampStartFrames =
                    ((int64_t)kAutoRampStartUs * mSampleRate + 500000LL) / 1000000LL; //Need type casting

        int32_t nFrames = mNumFramesReceived - autoRampStartFrames;
        rampVolume(nFrames, autoRampDurationFrames,
                (uint8_t *) buffer->data(), buffer->range_length());
    }

    // Track the max recording signal amplitude.
    if (mTrackMaxAmplitude) {
        trackMaxAmplitude(
            (int16_t *) buffer->data(), buffer->range_length() >> 1);
    }

    if (mSampleRate != mOutSampleRate) {
            timeUs *= (int64_t)mSampleRate / (int64_t)mOutSampleRate;
            buffer->meta_data().setInt64(kKeyTime, timeUs);
    }

    *out = buffer;
    return OK;
}

status_t AudioSource::setStopTimeUs(int64_t stopTimeUs) {
    Mutex::Autolock autoLock(mLock);
    ALOGV("Set stoptime: %lld us", (long long)stopTimeUs);

    if (stopTimeUs < -1) {
        ALOGE("Invalid stop time %lld us", (long long)stopTimeUs);
        return BAD_VALUE;
    } else if (stopTimeUs == -1) {
        ALOGI("reset stopTime to be -1");
    }

    mStopSystemTimeUs = stopTimeUs;
    return OK;
}

void AudioSource::signalBufferReturned(MediaBufferBase *buffer) {
    ALOGV("signalBufferReturned: %p", buffer->data());
    Mutex::Autolock autoLock(mLock);
    --mNumClientOwnedBuffers;
    buffer->setObserver(0);
    buffer->release();
    mFrameEncodingCompletionCondition.signal();
    return;
}

status_t AudioSource::dataCallback(const AudioRecord::Buffer& audioBuffer) {
    int64_t timeUs, position, timeNs;
    ExtendedTimestamp ts;
    ExtendedTimestamp::Location location;
    const int32_t usPerSec = 1000000;

    if (mRecord->getTimestamp(&ts) == OK &&
            ts.getBestTimestamp(&position, &timeNs, ExtendedTimestamp::TIMEBASE_MONOTONIC,
            &location) == OK) {
        // Use audio timestamp.
        timeUs = timeNs / 1000 -
                (position - mNumFramesSkipped -
                mNumFramesReceived + mNumFramesLost) * usPerSec / mSampleRate;
    } else {
        // This should not happen in normal case.
        ALOGW("Failed to get audio timestamp, fallback to use systemclock");
        timeUs = systemTime() / 1000LL;
        // Estimate the real sampling time of the 1st sample in this buffer
        // from AudioRecord's latency. (Apply this adjustment first so that
        // the start time logic is not affected.)
        timeUs -= mRecord->latency() * 1000LL;
    }

    ALOGV("dataCallbackTimestamp: %" PRId64 " us", timeUs);
    Mutex::Autolock autoLock(mLock);
    if (!mStarted) {
        ALOGW("Spurious callback from AudioRecord. Drop the audio data.");
        return OK;
    }

    const size_t bufferSize = audioBuffer.size;

    // Drop retrieved and previously lost audio data.
    if (mNumFramesReceived == 0 && timeUs < mStartTimeUs) {
        (void) mRecord->getInputFramesLost();
        int64_t receievedFrames = bufferSize / mRecord->frameSize();
        ALOGV("Drop audio data(%" PRId64 " frames) at %" PRId64 "/%" PRId64 " us",
                receievedFrames, timeUs, mStartTimeUs);
        mNumFramesSkipped += receievedFrames;
        return OK;
    }

    if (mStopSystemTimeUs != -1 && timeUs >= mStopSystemTimeUs) {
        ALOGV("Drop Audio frame at %lld  stop time: %lld us",
                (long long)timeUs, (long long)mStopSystemTimeUs);
        mNoMoreFramesToRead = true;
        mFrameAvailableCondition.signal();
        return OK;
    }

    if (mNumFramesReceived == 0 && mPrevSampleTimeUs == 0) {
        mInitialReadTimeUs = timeUs;
        // Initial delay
        if (mStartTimeUs > 0) {
            mStartTimeUs = timeUs - mStartTimeUs;
        }
        mPrevSampleTimeUs = mStartTimeUs;
    }
    mLastFrameTimestampUs = timeUs;

    uint64_t numLostBytes = 0; // AudioRecord::getInputFramesLost() returns uint32_t
    if (mNumFramesReceived > 0) {  // Ignore earlier frame lost
        // getInputFramesLost() returns the number of lost frames.
        // Convert number of frames lost to number of bytes lost.
        numLostBytes = (uint64_t)mRecord->getInputFramesLost() * mRecord->frameSize();
    }

    CHECK_EQ(numLostBytes & 1, 0u);
    CHECK_EQ(audioBuffer.size & 1, 0u);
    if (numLostBytes > 0) {
        // Loss of audio frames should happen rarely; thus the LOGW should
        // not cause a logging spam
        ALOGW("Lost audio record data: %" PRIu64 " bytes", numLostBytes);
    }

    while (numLostBytes > 0) {
        uint64_t bufferSize = numLostBytes;
        if (numLostBytes > kMaxBufferSize) {
            numLostBytes -= kMaxBufferSize;
            bufferSize = kMaxBufferSize;
        } else {
            numLostBytes = 0;
        }
        MediaBuffer *lostAudioBuffer = new MediaBuffer(bufferSize);
        memset(lostAudioBuffer->data(), 0, bufferSize);
        lostAudioBuffer->set_range(0, bufferSize);
        mNumFramesLost += bufferSize / mRecord->frameSize();
        queueInputBuffer_l(lostAudioBuffer, timeUs);
    }

    if (audioBuffer.size == 0) {
        ALOGW("Nothing is available from AudioRecord callback buffer");
        return OK;
    }

    MediaBuffer *buffer = new MediaBuffer(bufferSize);
    memcpy((uint8_t *) buffer->data(),
            audioBuffer.i16, audioBuffer.size);
    buffer->set_range(0, bufferSize);
    queueInputBuffer_l(buffer, timeUs);
    return OK;
}

void AudioSource::queueInputBuffer_l(MediaBuffer *buffer, int64_t timeUs) {
    const size_t bufferSize = buffer->range_length();
    const size_t frameSize = mRecord->frameSize();
    if (mNumFramesReceived == 0) {
        buffer->meta_data().setInt64(kKeyAnchorTime, mStartTimeUs);
    }
    mNumFramesReceived += bufferSize / frameSize;
    const int64_t timestampUs =
                mStartTimeUs +
                    ((1000000LL * mNumFramesReceived) +
                        (mSampleRate >> 1)) / mSampleRate;
    buffer->meta_data().setInt64(kKeyTime, mPrevSampleTimeUs);
    buffer->meta_data().setInt64(kKeyDriftTime, timeUs - mInitialReadTimeUs);
    mPrevSampleTimeUs = timestampUs;
    mBuffersReceived.push_back(buffer);
    mFrameAvailableCondition.signal();
}

void AudioSource::trackMaxAmplitude(int16_t *data, int nSamples) {
    for (int i = nSamples; i > 0; --i) {
        int16_t value = *data++;
        if (value < 0) {
            value = -value;
        }
        if (mMaxAmplitude < value) {
            mMaxAmplitude = value;
        }
    }
}

int16_t AudioSource::getMaxAmplitude() {
    // First call activates the tracking.
    if (!mTrackMaxAmplitude) {
        mTrackMaxAmplitude = true;
    }
    int16_t value = mMaxAmplitude;
    mMaxAmplitude = 0;
    ALOGV("max amplitude since last call: %d", value);
    return value;
}

status_t AudioSource::setInputDevice(audio_port_handle_t deviceId) {
    if (mRecord != 0) {
        return mRecord->setInputDevice(deviceId);
    }
    return NO_INIT;
}

status_t AudioSource::getRoutedDeviceId(audio_port_handle_t* deviceId) {
    if (mRecord != 0) {
        *deviceId = mRecord->getRoutedDeviceId();
        return NO_ERROR;
    }
    return NO_INIT;
}

status_t AudioSource::addAudioDeviceCallback(
        const sp<AudioSystem::AudioDeviceCallback>& callback) {
    if (mRecord != 0) {
        return mRecord->addAudioDeviceCallback(callback);
    }
    return NO_INIT;
}

status_t AudioSource::removeAudioDeviceCallback(
        const sp<AudioSystem::AudioDeviceCallback>& callback) {
    if (mRecord != 0) {
        return mRecord->removeAudioDeviceCallback(callback);
    }
    return NO_INIT;
}

status_t AudioSource::getActiveMicrophones(
        std::vector<media::MicrophoneInfo>* activeMicrophones) {
    if (mRecord != 0) {
        return mRecord->getActiveMicrophones(activeMicrophones);
    }
    return NO_INIT;
}

status_t AudioSource::setPreferredMicrophoneDirection(audio_microphone_direction_t direction) {
    ALOGV("setPreferredMicrophoneDirection(%d)", direction);
    if (mRecord != 0) {
        return mRecord->setPreferredMicrophoneDirection(direction);
    }
    return NO_INIT;
}

status_t AudioSource::setPreferredMicrophoneFieldDimension(float zoom) {
    ALOGV("setPreferredMicrophoneFieldDimension(%f)", zoom);
    if (mRecord != 0) {
        return mRecord->setPreferredMicrophoneFieldDimension(zoom);
    }
    return NO_INIT;
}

status_t AudioSource::getPortId(audio_port_handle_t *portId) const {
    if (mRecord != 0) {
        *portId = mRecord->getPortId();
        return NO_ERROR;
    }
    return NO_INIT;
}
}  // namespace android
