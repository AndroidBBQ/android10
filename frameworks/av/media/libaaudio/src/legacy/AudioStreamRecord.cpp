/*
 * Copyright 2016 The Android Open Source Project
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

#define LOG_TAG "AudioStreamRecord"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <stdint.h>

#include <aaudio/AAudio.h>
#include <audio_utils/primitives.h>
#include <media/AudioRecord.h>
#include <utils/String16.h>

#include "legacy/AudioStreamLegacy.h"
#include "legacy/AudioStreamRecord.h"
#include "utility/AudioClock.h"
#include "utility/FixedBlockWriter.h"

using namespace android;
using namespace aaudio;

AudioStreamRecord::AudioStreamRecord()
    : AudioStreamLegacy()
    , mFixedBlockWriter(*this)
{
}

AudioStreamRecord::~AudioStreamRecord()
{
    const aaudio_stream_state_t state = getState();
    bool bad = !(state == AAUDIO_STREAM_STATE_UNINITIALIZED || state == AAUDIO_STREAM_STATE_CLOSED);
    ALOGE_IF(bad, "stream not closed, in state %d", state);
}

aaudio_result_t AudioStreamRecord::open(const AudioStreamBuilder& builder)
{
    aaudio_result_t result = AAUDIO_OK;

    result = AudioStream::open(builder);
    if (result != AAUDIO_OK) {
        return result;
    }

    // Try to create an AudioRecord

    const aaudio_session_id_t requestedSessionId = builder.getSessionId();
    const audio_session_t sessionId = AAudioConvert_aaudioToAndroidSessionId(requestedSessionId);

    // TODO Support UNSPECIFIED in AudioRecord. For now, use stereo if unspecified.
    int32_t samplesPerFrame = (getSamplesPerFrame() == AAUDIO_UNSPECIFIED)
                              ? 2 : getSamplesPerFrame();
    audio_channel_mask_t channelMask = samplesPerFrame <= 2 ?
                               audio_channel_in_mask_from_count(samplesPerFrame) :
                               audio_channel_mask_for_index_assignment_from_count(samplesPerFrame);

    size_t frameCount = (builder.getBufferCapacity() == AAUDIO_UNSPECIFIED) ? 0
                        : builder.getBufferCapacity();


    audio_input_flags_t flags;
    aaudio_performance_mode_t perfMode = getPerformanceMode();
    switch (perfMode) {
        case AAUDIO_PERFORMANCE_MODE_LOW_LATENCY:
            // If the app asks for a sessionId then it means they want to use effects.
            // So don't use RAW flag.
            flags = (audio_input_flags_t) ((requestedSessionId == AAUDIO_SESSION_ID_NONE)
                    ? (AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_RAW)
                    : (AUDIO_INPUT_FLAG_FAST));
            break;

        case AAUDIO_PERFORMANCE_MODE_POWER_SAVING:
        case AAUDIO_PERFORMANCE_MODE_NONE:
        default:
            flags = AUDIO_INPUT_FLAG_NONE;
            break;
    }

    // Preserve behavior of API 26
    if (getFormat() == AUDIO_FORMAT_DEFAULT) {
        setFormat(AUDIO_FORMAT_PCM_FLOAT);
    }

    // Maybe change device format to get a FAST path.
    // AudioRecord does not support FAST mode for FLOAT data.
    // TODO AudioRecord should allow FLOAT data paths for FAST tracks.
    // So IF the user asks for low latency FLOAT
    // AND the sampleRate is likely to be compatible with FAST
    // THEN request I16 and convert to FLOAT when passing to user.
    // Note that hard coding 48000 Hz is not ideal because the sampleRate
    // for a FAST path might not be 48000 Hz.
    // It normally is but there is a chance that it is not.
    // And there is no reliable way to know that in advance.
    // Luckily the consequences of a wrong guess are minor.
    // We just may not get a FAST track.
    // But we wouldn't have anyway without this hack.
    constexpr int32_t kMostLikelySampleRateForFast = 48000;
    if (getFormat() == AUDIO_FORMAT_PCM_FLOAT
            && perfMode == AAUDIO_PERFORMANCE_MODE_LOW_LATENCY
            && (samplesPerFrame <= 2) // FAST only for mono and stereo
            && (getSampleRate() == kMostLikelySampleRateForFast
                || getSampleRate() == AAUDIO_UNSPECIFIED)) {
        setDeviceFormat(AUDIO_FORMAT_PCM_16_BIT);
    } else {
        setDeviceFormat(getFormat());
    }

    uint32_t notificationFrames = 0;

    // Setup the callback if there is one.
    AudioRecord::callback_t callback = nullptr;
    void *callbackData = nullptr;
    AudioRecord::transfer_type streamTransferType = AudioRecord::transfer_type::TRANSFER_SYNC;
    if (builder.getDataCallbackProc() != nullptr) {
        streamTransferType = AudioRecord::transfer_type::TRANSFER_CALLBACK;
        callback = getLegacyCallback();
        callbackData = this;
        notificationFrames = builder.getFramesPerDataCallback();
    }
    mCallbackBufferSize = builder.getFramesPerDataCallback();

    // Don't call mAudioRecord->setInputDevice() because it will be overwritten by set()!
    audio_port_handle_t selectedDeviceId = (getDeviceId() == AAUDIO_UNSPECIFIED)
                                           ? AUDIO_PORT_HANDLE_NONE
                                           : getDeviceId();

    const audio_content_type_t contentType =
            AAudioConvert_contentTypeToInternal(builder.getContentType());
    const audio_source_t source =
            AAudioConvert_inputPresetToAudioSource(builder.getInputPreset());

    const audio_attributes_t attributes = {
            .content_type = contentType,
            .usage = AUDIO_USAGE_UNKNOWN, // only used for output
            .source = source,
            .flags = AUDIO_FLAG_NONE, // Different than the AUDIO_INPUT_FLAGS
            .tags = ""
    };

    // ----------- open the AudioRecord ---------------------
    // Might retry, but never more than once.
    for (int i = 0; i < 2; i ++) {
        const audio_format_t requestedInternalFormat = getDeviceFormat();

        mAudioRecord = new AudioRecord(
                mOpPackageName // const String16& opPackageName TODO does not compile
        );
        mAudioRecord->set(
                AUDIO_SOURCE_DEFAULT, // ignored because we pass attributes below
                getSampleRate(),
                requestedInternalFormat,
                channelMask,
                frameCount,
                callback,
                callbackData,
                notificationFrames,
                false /*threadCanCallJava*/,
                sessionId,
                streamTransferType,
                flags,
                AUDIO_UID_INVALID, // DEFAULT uid
                -1,                // DEFAULT pid
                &attributes,
                selectedDeviceId
        );

        // Did we get a valid track?
        status_t status = mAudioRecord->initCheck();
        if (status != OK) {
            close();
            ALOGE("open(), initCheck() returned %d", status);
            return AAudioConvert_androidToAAudioResult(status);
        }

        // Check to see if it was worth hacking the deviceFormat.
        bool gotFastPath = (mAudioRecord->getFlags() & AUDIO_INPUT_FLAG_FAST)
                           == AUDIO_INPUT_FLAG_FAST;
        if (getFormat() != getDeviceFormat() && !gotFastPath) {
            // We tried to get a FAST path by switching the device format.
            // But it didn't work. So we might as well reopen using the same
            // format for device and for app.
            ALOGD("%s() used a different device format but no FAST path, reopen", __func__);
            mAudioRecord.clear();
            setDeviceFormat(getFormat());
        } else {
            break; // Keep the one we just opened.
        }
    }

    // Get the actual values from the AudioRecord.
    setSamplesPerFrame(mAudioRecord->channelCount());

    int32_t actualSampleRate = mAudioRecord->getSampleRate();
    ALOGW_IF(actualSampleRate != getSampleRate(),
             "open() sampleRate changed from %d to %d",
             getSampleRate(), actualSampleRate);
    setSampleRate(actualSampleRate);

    // We may need to pass the data through a block size adapter to guarantee constant size.
    if (mCallbackBufferSize != AAUDIO_UNSPECIFIED) {
        int callbackSizeBytes = getBytesPerFrame() * mCallbackBufferSize;
        mFixedBlockWriter.open(callbackSizeBytes);
        mBlockAdapter = &mFixedBlockWriter;
    } else {
        mBlockAdapter = nullptr;
    }

    // Allocate format conversion buffer if needed.
    if (getDeviceFormat() == AUDIO_FORMAT_PCM_16_BIT
        && getFormat() == AUDIO_FORMAT_PCM_FLOAT) {

        if (builder.getDataCallbackProc() != nullptr) {
            // If we have a callback then we need to convert the data into an internal float
            // array and then pass that entire array to the app.
            mFormatConversionBufferSizeInFrames =
                    (mCallbackBufferSize != AAUDIO_UNSPECIFIED)
                    ? mCallbackBufferSize : getFramesPerBurst();
            int32_t numSamples = mFormatConversionBufferSizeInFrames * getSamplesPerFrame();
            mFormatConversionBufferFloat = std::make_unique<float[]>(numSamples);
        } else {
            // If we don't have a callback then we will read into an internal short array
            // and then convert into the app float array in read().
            mFormatConversionBufferSizeInFrames = getFramesPerBurst();
            int32_t numSamples = mFormatConversionBufferSizeInFrames * getSamplesPerFrame();
            mFormatConversionBufferI16 = std::make_unique<int16_t[]>(numSamples);
        }
        ALOGD("%s() setup I16>FLOAT conversion buffer with %d frames",
              __func__, mFormatConversionBufferSizeInFrames);
    }

    // Update performance mode based on the actual stream.
    // For example, if the sample rate does not match native then you won't get a FAST track.
    audio_input_flags_t actualFlags = mAudioRecord->getFlags();
    aaudio_performance_mode_t actualPerformanceMode = AAUDIO_PERFORMANCE_MODE_NONE;
    // FIXME Some platforms do not advertise RAW mode for low latency inputs.
    if ((actualFlags & (AUDIO_INPUT_FLAG_FAST))
        == (AUDIO_INPUT_FLAG_FAST)) {
        actualPerformanceMode = AAUDIO_PERFORMANCE_MODE_LOW_LATENCY;
    }
    setPerformanceMode(actualPerformanceMode);

    setSharingMode(AAUDIO_SHARING_MODE_SHARED); // EXCLUSIVE mode not supported in legacy

    // Log warning if we did not get what we asked for.
    ALOGW_IF(actualFlags != flags,
             "open() flags changed from 0x%08X to 0x%08X",
             flags, actualFlags);
    ALOGW_IF(actualPerformanceMode != perfMode,
             "open() perfMode changed from %d to %d",
             perfMode, actualPerformanceMode);

    setState(AAUDIO_STREAM_STATE_OPEN);
    setDeviceId(mAudioRecord->getRoutedDeviceId());

    aaudio_session_id_t actualSessionId =
            (requestedSessionId == AAUDIO_SESSION_ID_NONE)
            ? AAUDIO_SESSION_ID_NONE
            : (aaudio_session_id_t) mAudioRecord->getSessionId();
    setSessionId(actualSessionId);

    mAudioRecord->addAudioDeviceCallback(mDeviceCallback);

    return AAUDIO_OK;
}

aaudio_result_t AudioStreamRecord::close()
{
    // TODO add close() or release() to AudioRecord API then call it from here
    if (getState() != AAUDIO_STREAM_STATE_CLOSED) {
        mAudioRecord->removeAudioDeviceCallback(mDeviceCallback);
        mAudioRecord.clear();
        setState(AAUDIO_STREAM_STATE_CLOSED);
    }
    mFixedBlockWriter.close();
    return AudioStream::close();
}

const void * AudioStreamRecord::maybeConvertDeviceData(const void *audioData, int32_t numFrames) {
    if (mFormatConversionBufferFloat.get() != nullptr) {
        LOG_ALWAYS_FATAL_IF(numFrames > mFormatConversionBufferSizeInFrames,
                            "%s() conversion size %d too large for buffer %d",
                            __func__, numFrames, mFormatConversionBufferSizeInFrames);

        int32_t numSamples = numFrames * getSamplesPerFrame();
        // Only conversion supported is I16 to FLOAT
        memcpy_to_float_from_i16(
                    mFormatConversionBufferFloat.get(),
                    (const int16_t *) audioData,
                    numSamples);
        return mFormatConversionBufferFloat.get();
    } else {
        return audioData;
    }
}

void AudioStreamRecord::processCallback(int event, void *info) {
    switch (event) {
        case AudioRecord::EVENT_MORE_DATA:
            processCallbackCommon(AAUDIO_CALLBACK_OPERATION_PROCESS_DATA, info);
            break;

            // Stream got rerouted so we disconnect.
        case AudioRecord::EVENT_NEW_IAUDIORECORD:
            processCallbackCommon(AAUDIO_CALLBACK_OPERATION_DISCONNECTED, info);
            break;

        default:
            break;
    }
    return;
}

aaudio_result_t AudioStreamRecord::requestStart()
{
    if (mAudioRecord.get() == nullptr) {
        return AAUDIO_ERROR_INVALID_STATE;
    }

    // Enable callback before starting AudioRecord to avoid shutting
    // down because of a race condition.
    mCallbackEnabled.store(true);
    mFramesWritten.reset32(); // service writes frames
    mTimestampPosition.reset32();
    status_t err = mAudioRecord->start(); // resets position to zero
    if (err != OK) {
        return AAudioConvert_androidToAAudioResult(err);
    } else {
        setState(AAUDIO_STREAM_STATE_STARTING);
    }
    return AAUDIO_OK;
}

aaudio_result_t AudioStreamRecord::requestStop() {
    if (mAudioRecord.get() == nullptr) {
        return AAUDIO_ERROR_INVALID_STATE;
    }
    setState(AAUDIO_STREAM_STATE_STOPPING);
    mFramesWritten.catchUpTo(getFramesRead());
    mTimestampPosition.catchUpTo(getFramesRead());
    mAudioRecord->stop();
    mCallbackEnabled.store(false);
    // Pass false to prevent errorCallback from being called after disconnect
    // when app has already requested a stop().
    return checkForDisconnectRequest(false);
}

aaudio_result_t AudioStreamRecord::updateStateMachine()
{
    aaudio_result_t result = AAUDIO_OK;
    aaudio_wrapping_frames_t position;
    status_t err;
    switch (getState()) {
    // TODO add better state visibility to AudioRecord
    case AAUDIO_STREAM_STATE_STARTING:
        // When starting, the position will begin at zero and then go positive.
        // The position can wrap but by that time the state will not be STARTING.
        err = mAudioRecord->getPosition(&position);
        if (err != OK) {
            result = AAudioConvert_androidToAAudioResult(err);
        } else if (position > 0) {
            setState(AAUDIO_STREAM_STATE_STARTED);
        }
        break;
    case AAUDIO_STREAM_STATE_STOPPING:
        if (mAudioRecord->stopped()) {
            setState(AAUDIO_STREAM_STATE_STOPPED);
        }
        break;
    default:
        break;
    }
    return result;
}

aaudio_result_t AudioStreamRecord::read(void *buffer,
                                      int32_t numFrames,
                                      int64_t timeoutNanoseconds)
{
    int32_t bytesPerDeviceFrame = getBytesPerDeviceFrame();
    int32_t numBytes;
    // This will detect out of range values for numFrames.
    aaudio_result_t result = AAudioConvert_framesToBytes(numFrames, bytesPerDeviceFrame, &numBytes);
    if (result != AAUDIO_OK) {
        return result;
    }

    if (getState() == AAUDIO_STREAM_STATE_DISCONNECTED) {
        return AAUDIO_ERROR_DISCONNECTED;
    }

    // TODO add timeout to AudioRecord
    bool blocking = (timeoutNanoseconds > 0);

    ssize_t bytesActuallyRead = 0;
    ssize_t totalBytesRead = 0;
    if (mFormatConversionBufferI16.get() != nullptr) {
        // Convert I16 data to float using an intermediate buffer.
        float *floatBuffer = (float *) buffer;
        int32_t framesLeft = numFrames;
        // Perform conversion using multiple read()s if necessary.
        while (framesLeft > 0) {
            // Read into short internal buffer.
            int32_t framesToRead = std::min(framesLeft, mFormatConversionBufferSizeInFrames);
            size_t bytesToRead = framesToRead * bytesPerDeviceFrame;
            bytesActuallyRead = mAudioRecord->read(mFormatConversionBufferI16.get(), bytesToRead, blocking);
            if (bytesActuallyRead <= 0) {
                break;
            }
            totalBytesRead += bytesActuallyRead;
            int32_t framesToConvert = bytesActuallyRead / bytesPerDeviceFrame;
            // Convert into app float buffer.
            size_t numSamples = framesToConvert * getSamplesPerFrame();
            memcpy_to_float_from_i16(
                    floatBuffer,
                    mFormatConversionBufferI16.get(),
                    numSamples);
            floatBuffer += numSamples;
            framesLeft -= framesToConvert;
        }
    } else {
        bytesActuallyRead = mAudioRecord->read(buffer, numBytes, blocking);
        totalBytesRead = bytesActuallyRead;
    }
    if (bytesActuallyRead == WOULD_BLOCK) {
        return 0;
    } else if (bytesActuallyRead < 0) {
        // In this context, a DEAD_OBJECT is more likely to be a disconnect notification due to
        // AudioRecord invalidation.
        if (bytesActuallyRead == DEAD_OBJECT) {
            setState(AAUDIO_STREAM_STATE_DISCONNECTED);
            return AAUDIO_ERROR_DISCONNECTED;
        }
        return AAudioConvert_androidToAAudioResult(bytesActuallyRead);
    }
    int32_t framesRead = (int32_t)(totalBytesRead / bytesPerDeviceFrame);
    incrementFramesRead(framesRead);

    result = updateStateMachine();
    if (result != AAUDIO_OK) {
        return result;
    }

    return (aaudio_result_t) framesRead;
}

aaudio_result_t AudioStreamRecord::setBufferSize(int32_t requestedFrames)
{
    return getBufferSize();
}

int32_t AudioStreamRecord::getBufferSize() const
{
    return getBufferCapacity(); // TODO implement in AudioRecord?
}

int32_t AudioStreamRecord::getBufferCapacity() const
{
    return static_cast<int32_t>(mAudioRecord->frameCount());
}

int32_t AudioStreamRecord::getXRunCount() const
{
    return 0; // TODO implement when AudioRecord supports it
}

int32_t AudioStreamRecord::getFramesPerBurst() const
{
    return static_cast<int32_t>(mAudioRecord->getNotificationPeriodInFrames());
}

aaudio_result_t AudioStreamRecord::getTimestamp(clockid_t clockId,
                                               int64_t *framePosition,
                                               int64_t *timeNanoseconds) {
    ExtendedTimestamp extendedTimestamp;
    if (getState() != AAUDIO_STREAM_STATE_STARTED) {
        return AAUDIO_ERROR_INVALID_STATE;
    }
    status_t status = mAudioRecord->getTimestamp(&extendedTimestamp);
    if (status == WOULD_BLOCK) {
        return AAUDIO_ERROR_INVALID_STATE;
    } else if (status != NO_ERROR) {
        return AAudioConvert_androidToAAudioResult(status);
    }
    return getBestTimestamp(clockId, framePosition, timeNanoseconds, &extendedTimestamp);
}

int64_t AudioStreamRecord::getFramesWritten() {
    aaudio_wrapping_frames_t position;
    status_t result;
    switch (getState()) {
        case AAUDIO_STREAM_STATE_STARTING:
        case AAUDIO_STREAM_STATE_STARTED:
            result = mAudioRecord->getPosition(&position);
            if (result == OK) {
                mFramesWritten.update32(position);
            }
            break;
        case AAUDIO_STREAM_STATE_STOPPING:
        default:
            break;
    }
    return AudioStreamLegacy::getFramesWritten();
}
