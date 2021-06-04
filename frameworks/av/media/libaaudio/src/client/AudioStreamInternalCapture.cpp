/*
 * Copyright (C) 2017 The Android Open Source Project
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

#define LOG_TAG (mInService ? "AudioStreamInternalCapture_Service" \
                          : "AudioStreamInternalCapture_Client")
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <algorithm>
#include <audio_utils/primitives.h>
#include <aaudio/AAudio.h>

#include "client/AudioStreamInternalCapture.h"
#include "utility/AudioClock.h"

#define ATRACE_TAG ATRACE_TAG_AUDIO
#include <utils/Trace.h>

using android::WrappingBuffer;

using namespace aaudio;

AudioStreamInternalCapture::AudioStreamInternalCapture(AAudioServiceInterface  &serviceInterface,
                                                 bool inService)
    : AudioStreamInternal(serviceInterface, inService) {

}

AudioStreamInternalCapture::~AudioStreamInternalCapture() {}

void AudioStreamInternalCapture::advanceClientToMatchServerPosition() {
    int64_t readCounter = mAudioEndpoint.getDataReadCounter();
    int64_t writeCounter = mAudioEndpoint.getDataWriteCounter();

    // Bump offset so caller does not see the retrograde motion in getFramesRead().
    int64_t offset = readCounter - writeCounter;
    mFramesOffsetFromService += offset;
    ALOGD("advanceClientToMatchServerPosition() readN = %lld, writeN = %lld, offset = %lld",
          (long long)readCounter, (long long)writeCounter, (long long)mFramesOffsetFromService);

    // Force readCounter to match writeCounter.
    // This is because we cannot change the write counter in the hardware.
    mAudioEndpoint.setDataReadCounter(writeCounter);
}

// Write the data, block if needed and timeoutMillis > 0
aaudio_result_t AudioStreamInternalCapture::read(void *buffer, int32_t numFrames,
                                               int64_t timeoutNanoseconds)
{
    return processData(buffer, numFrames, timeoutNanoseconds);
}

// Read as much data as we can without blocking.
aaudio_result_t AudioStreamInternalCapture::processDataNow(void *buffer, int32_t numFrames,
                                                  int64_t currentNanoTime, int64_t *wakeTimePtr) {
    aaudio_result_t result = processCommands();
    if (result != AAUDIO_OK) {
        return result;
    }

    const char *traceName = "aaRdNow";
    ATRACE_BEGIN(traceName);

    if (mClockModel.isStarting()) {
        // Still haven't got any timestamps from server.
        // Keep waiting until we get some valid timestamps then start writing to the
        // current buffer position.
        ALOGD("processDataNow() wait for valid timestamps");
        // Sleep very briefly and hope we get a timestamp soon.
        *wakeTimePtr = currentNanoTime + (2000 * AAUDIO_NANOS_PER_MICROSECOND);
        ATRACE_END();
        return 0;
    }
    // If we have gotten this far then we have at least one timestamp from server.

    if (mAudioEndpoint.isFreeRunning()) {
        //ALOGD("AudioStreamInternalCapture::processDataNow() - update remote counter");
        // Update data queue based on the timing model.
        // Jitter in the DSP can cause late writes to the FIFO.
        // This might be caused by resampling.
        // We want to read the FIFO after the latest possible time
        // that the DSP could have written the data.
        int64_t estimatedRemoteCounter = mClockModel.convertLatestTimeToPosition(currentNanoTime);
        // TODO refactor, maybe use setRemoteCounter()
        mAudioEndpoint.setDataWriteCounter(estimatedRemoteCounter);
    }

    // This code assumes that we have already received valid timestamps.
    if (mNeedCatchUp.isRequested()) {
        // Catch an MMAP pointer that is already advancing.
        // This will avoid initial underruns caused by a slow cold start.
        advanceClientToMatchServerPosition();
        mNeedCatchUp.acknowledge();
    }

    // If the write index passed the read index then consider it an overrun.
    // For shared streams, the xRunCount is passed up from the service.
    if (mAudioEndpoint.isFreeRunning() && mAudioEndpoint.getEmptyFramesAvailable() < 0) {
        mXRunCount++;
        if (ATRACE_ENABLED()) {
            ATRACE_INT("aaOverRuns", mXRunCount);
        }
    }

    // Read some data from the buffer.
    //ALOGD("AudioStreamInternalCapture::processDataNow() - readNowWithConversion(%d)", numFrames);
    int32_t framesProcessed = readNowWithConversion(buffer, numFrames);
    //ALOGD("AudioStreamInternalCapture::processDataNow() - tried to read %d frames, read %d",
    //    numFrames, framesProcessed);
    if (ATRACE_ENABLED()) {
        ATRACE_INT("aaRead", framesProcessed);
    }

    // Calculate an ideal time to wake up.
    if (wakeTimePtr != nullptr && framesProcessed >= 0) {
        // By default wake up a few milliseconds from now.  // TODO review
        int64_t wakeTime = currentNanoTime + (1 * AAUDIO_NANOS_PER_MILLISECOND);
        aaudio_stream_state_t state = getState();
        //ALOGD("AudioStreamInternalCapture::processDataNow() - wakeTime based on %s",
        //      AAudio_convertStreamStateToText(state));
        switch (state) {
            case AAUDIO_STREAM_STATE_OPEN:
            case AAUDIO_STREAM_STATE_STARTING:
                break;
            case AAUDIO_STREAM_STATE_STARTED:
            {
                // When do we expect the next write burst to occur?

                // Calculate frame position based off of the readCounter because
                // the writeCounter might have just advanced in the background,
                // causing us to sleep until a later burst.
                int64_t nextPosition = mAudioEndpoint.getDataReadCounter() + mFramesPerBurst;
                wakeTime = mClockModel.convertPositionToLatestTime(nextPosition);
            }
                break;
            default:
                break;
        }
        *wakeTimePtr = wakeTime;

    }

    ATRACE_END();
    return framesProcessed;
}

aaudio_result_t AudioStreamInternalCapture::readNowWithConversion(void *buffer,
                                                                int32_t numFrames) {
    // ALOGD("readNowWithConversion(%p, %d)",
    //              buffer, numFrames);
    WrappingBuffer wrappingBuffer;
    uint8_t *destination = (uint8_t *) buffer;
    int32_t framesLeft = numFrames;

    mAudioEndpoint.getFullFramesAvailable(&wrappingBuffer);

    // Read data in one or two parts.
    for (int partIndex = 0; framesLeft > 0 && partIndex < WrappingBuffer::SIZE; partIndex++) {
        int32_t framesToProcess = framesLeft;
        const int32_t framesAvailable = wrappingBuffer.numFrames[partIndex];
        if (framesAvailable <= 0) break;

        if (framesToProcess > framesAvailable) {
            framesToProcess = framesAvailable;
        }

        const int32_t numBytes = getBytesPerFrame() * framesToProcess;
        const int32_t numSamples = framesToProcess * getSamplesPerFrame();

        const audio_format_t sourceFormat = getDeviceFormat();
        const audio_format_t destinationFormat = getFormat();
        // TODO factor this out into a utility function
        if (sourceFormat == destinationFormat) {
            memcpy(destination, wrappingBuffer.data[partIndex], numBytes);
        } else if (sourceFormat == AUDIO_FORMAT_PCM_16_BIT
                   && destinationFormat == AUDIO_FORMAT_PCM_FLOAT) {
            memcpy_to_float_from_i16(
                    (float *) destination,
                    (const int16_t *) wrappingBuffer.data[partIndex],
                    numSamples);
        } else if (sourceFormat == AUDIO_FORMAT_PCM_FLOAT
                   && destinationFormat == AUDIO_FORMAT_PCM_16_BIT) {
            memcpy_to_i16_from_float(
                    (int16_t *) destination,
                    (const float *) wrappingBuffer.data[partIndex],
                    numSamples);
        } else {
            ALOGE("%s() - Format conversion not supported! audio_format_t source = %u, dest = %u",
                __func__, sourceFormat, destinationFormat);
            return AAUDIO_ERROR_INVALID_FORMAT;
        }
        destination += numBytes;
        framesLeft -= framesToProcess;
    }

    int32_t framesProcessed = numFrames - framesLeft;
    mAudioEndpoint.advanceReadIndex(framesProcessed);

    //ALOGD("readNowWithConversion() returns %d", framesProcessed);
    return framesProcessed;
}

int64_t AudioStreamInternalCapture::getFramesWritten() {
    const int64_t framesWrittenHardware = isClockModelInControl()
            ? mClockModel.convertTimeToPosition(AudioClock::getNanoseconds())
            : mAudioEndpoint.getDataWriteCounter();
    // Add service offset and prevent retrograde motion.
    mLastFramesWritten = std::max(mLastFramesWritten,
                                  framesWrittenHardware + mFramesOffsetFromService);
    return mLastFramesWritten;
}

int64_t AudioStreamInternalCapture::getFramesRead() {
    int64_t frames = mAudioEndpoint.getDataReadCounter() + mFramesOffsetFromService;
    //ALOGD("getFramesRead() returns %lld", (long long)frames);
    return frames;
}

// Read data from the stream and pass it to the callback for processing.
void *AudioStreamInternalCapture::callbackLoop() {
    aaudio_result_t result = AAUDIO_OK;
    aaudio_data_callback_result_t callbackResult = AAUDIO_CALLBACK_RESULT_CONTINUE;
    if (!isDataCallbackSet()) return NULL;

    // result might be a frame count
    while (mCallbackEnabled.load() && isActive() && (result >= 0)) {

        // Read audio data from stream.
        int64_t timeoutNanos = calculateReasonableTimeout(mCallbackFrames);

        // This is a BLOCKING READ!
        result = read(mCallbackBuffer, mCallbackFrames, timeoutNanos);
        if ((result != mCallbackFrames)) {
            ALOGE("callbackLoop: read() returned %d", result);
            if (result >= 0) {
                // Only read some of the frames requested. Must have timed out.
                result = AAUDIO_ERROR_TIMEOUT;
            }
            maybeCallErrorCallback(result);
            break;
        }

        // Call application using the AAudio callback interface.
        callbackResult = maybeCallDataCallback(mCallbackBuffer, mCallbackFrames);

        if (callbackResult == AAUDIO_CALLBACK_RESULT_STOP) {
            ALOGD("%s(): callback returned AAUDIO_CALLBACK_RESULT_STOP", __func__);
            result = systemStopFromCallback();
            break;
        }
    }

    ALOGD("callbackLoop() exiting, result = %d, isActive() = %d",
          result, (int) isActive());
    return NULL;
}
