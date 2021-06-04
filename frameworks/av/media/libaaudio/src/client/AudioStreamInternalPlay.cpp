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

#define LOG_TAG (mInService ? "AudioStreamInternalPlay_Service" \
                          : "AudioStreamInternalPlay_Client")
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#define ATRACE_TAG ATRACE_TAG_AUDIO

#include <utils/Trace.h>

#include "client/AudioStreamInternalPlay.h"
#include "utility/AudioClock.h"

using android::WrappingBuffer;

using namespace aaudio;

AudioStreamInternalPlay::AudioStreamInternalPlay(AAudioServiceInterface  &serviceInterface,
                                                       bool inService)
        : AudioStreamInternal(serviceInterface, inService) {

}

AudioStreamInternalPlay::~AudioStreamInternalPlay() {}

constexpr int kRampMSec = 10; // time to apply a change in volume

aaudio_result_t AudioStreamInternalPlay::open(const AudioStreamBuilder &builder) {
    aaudio_result_t result = AudioStreamInternal::open(builder);
    if (result == AAUDIO_OK) {
        result = mFlowGraph.configure(getFormat(),
                             getSamplesPerFrame(),
                             getDeviceFormat(),
                             getDeviceChannelCount());

        if (result != AAUDIO_OK) {
            close();
        }
        // Sample rate is constrained to common values by now and should not overflow.
        int32_t numFrames = kRampMSec * getSampleRate() / AAUDIO_MILLIS_PER_SECOND;
        mFlowGraph.setRampLengthInFrames(numFrames);
    }
    return result;
}

// This must be called under mStreamLock.
aaudio_result_t AudioStreamInternalPlay::requestPause()
{
    aaudio_result_t result = stopCallback();
    if (result != AAUDIO_OK) {
        return result;
    }
    if (mServiceStreamHandle == AAUDIO_HANDLE_INVALID) {
        ALOGW("%s() mServiceStreamHandle invalid", __func__);
        return AAUDIO_ERROR_INVALID_STATE;
    }

    mClockModel.stop(AudioClock::getNanoseconds());
    setState(AAUDIO_STREAM_STATE_PAUSING);
    mAtomicInternalTimestamp.clear();
    return mServiceInterface.pauseStream(mServiceStreamHandle);
}

aaudio_result_t AudioStreamInternalPlay::requestFlush() {
    if (mServiceStreamHandle == AAUDIO_HANDLE_INVALID) {
        ALOGW("%s() mServiceStreamHandle invalid", __func__);
        return AAUDIO_ERROR_INVALID_STATE;
    }

    setState(AAUDIO_STREAM_STATE_FLUSHING);
    return mServiceInterface.flushStream(mServiceStreamHandle);
}

void AudioStreamInternalPlay::advanceClientToMatchServerPosition() {
    int64_t readCounter = mAudioEndpoint.getDataReadCounter();
    int64_t writeCounter = mAudioEndpoint.getDataWriteCounter();

    // Bump offset so caller does not see the retrograde motion in getFramesRead().
    int64_t offset = writeCounter - readCounter;
    mFramesOffsetFromService += offset;
    ALOGV("%s() readN = %lld, writeN = %lld, offset = %lld", __func__,
          (long long)readCounter, (long long)writeCounter, (long long)mFramesOffsetFromService);

    // Force writeCounter to match readCounter.
    // This is because we cannot change the read counter in the hardware.
    mAudioEndpoint.setDataWriteCounter(readCounter);
}

void AudioStreamInternalPlay::onFlushFromServer() {
    advanceClientToMatchServerPosition();
}

// Write the data, block if needed and timeoutMillis > 0
aaudio_result_t AudioStreamInternalPlay::write(const void *buffer, int32_t numFrames,
                                               int64_t timeoutNanoseconds) {
    return processData((void *)buffer, numFrames, timeoutNanoseconds);
}

// Write as much data as we can without blocking.
aaudio_result_t AudioStreamInternalPlay::processDataNow(void *buffer, int32_t numFrames,
                                              int64_t currentNanoTime, int64_t *wakeTimePtr) {
    aaudio_result_t result = processCommands();
    if (result != AAUDIO_OK) {
        return result;
    }

    const char *traceName = "aaWrNow";
    ATRACE_BEGIN(traceName);

    if (mClockModel.isStarting()) {
        // Still haven't got any timestamps from server.
        // Keep waiting until we get some valid timestamps then start writing to the
        // current buffer position.
        ALOGV("%s() wait for valid timestamps", __func__);
        // Sleep very briefly and hope we get a timestamp soon.
        *wakeTimePtr = currentNanoTime + (2000 * AAUDIO_NANOS_PER_MICROSECOND);
        ATRACE_END();
        return 0;
    }
    // If we have gotten this far then we have at least one timestamp from server.

    // If a DMA channel or DSP is reading the other end then we have to update the readCounter.
    if (mAudioEndpoint.isFreeRunning()) {
        // Update data queue based on the timing model.
        int64_t estimatedReadCounter = mClockModel.convertTimeToPosition(currentNanoTime);
        // ALOGD("AudioStreamInternal::processDataNow() - estimatedReadCounter = %d", (int)estimatedReadCounter);
        mAudioEndpoint.setDataReadCounter(estimatedReadCounter);
    }

    if (mNeedCatchUp.isRequested()) {
        // Catch an MMAP pointer that is already advancing.
        // This will avoid initial underruns caused by a slow cold start.
        advanceClientToMatchServerPosition();
        mNeedCatchUp.acknowledge();
    }

    // If the read index passed the write index then consider it an underrun.
    // For shared streams, the xRunCount is passed up from the service.
    if (mAudioEndpoint.isFreeRunning() && mAudioEndpoint.getFullFramesAvailable() < 0) {
        mXRunCount++;
        if (ATRACE_ENABLED()) {
            ATRACE_INT("aaUnderRuns", mXRunCount);
        }
    }

    // Write some data to the buffer.
    //ALOGD("AudioStreamInternal::processDataNow() - writeNowWithConversion(%d)", numFrames);
    int32_t framesWritten = writeNowWithConversion(buffer, numFrames);
    //ALOGD("AudioStreamInternal::processDataNow() - tried to write %d frames, wrote %d",
    //    numFrames, framesWritten);
    if (ATRACE_ENABLED()) {
        ATRACE_INT("aaWrote", framesWritten);
    }

    // Calculate an ideal time to wake up.
    if (wakeTimePtr != nullptr && framesWritten >= 0) {
        // By default wake up a few milliseconds from now.  // TODO review
        int64_t wakeTime = currentNanoTime + (1 * AAUDIO_NANOS_PER_MILLISECOND);
        aaudio_stream_state_t state = getState();
        //ALOGD("AudioStreamInternal::processDataNow() - wakeTime based on %s",
        //      AAudio_convertStreamStateToText(state));
        switch (state) {
            case AAUDIO_STREAM_STATE_OPEN:
            case AAUDIO_STREAM_STATE_STARTING:
                if (framesWritten != 0) {
                    // Don't wait to write more data. Just prime the buffer.
                    wakeTime = currentNanoTime;
                }
                break;
            case AAUDIO_STREAM_STATE_STARTED:
            {
                // When do we expect the next read burst to occur?

                // Calculate frame position based off of the writeCounter because
                // the readCounter might have just advanced in the background,
                // causing us to sleep until a later burst.
                int64_t nextPosition = mAudioEndpoint.getDataWriteCounter() + mFramesPerBurst
                        - mAudioEndpoint.getBufferSizeInFrames();
                wakeTime = mClockModel.convertPositionToTime(nextPosition);
            }
                break;
            default:
                break;
        }
        *wakeTimePtr = wakeTime;

    }

    ATRACE_END();
    return framesWritten;
}


aaudio_result_t AudioStreamInternalPlay::writeNowWithConversion(const void *buffer,
                                                            int32_t numFrames) {
    WrappingBuffer wrappingBuffer;
    uint8_t *byteBuffer = (uint8_t *) buffer;
    int32_t framesLeft = numFrames;

    mAudioEndpoint.getEmptyFramesAvailable(&wrappingBuffer);

    // Write data in one or two parts.
    int partIndex = 0;
    while (framesLeft > 0 && partIndex < WrappingBuffer::SIZE) {
        int32_t framesToWrite = framesLeft;
        int32_t framesAvailable = wrappingBuffer.numFrames[partIndex];
        if (framesAvailable > 0) {
            if (framesToWrite > framesAvailable) {
                framesToWrite = framesAvailable;
            }

            int32_t numBytes = getBytesPerFrame() * framesToWrite;

            mFlowGraph.process((void *)byteBuffer,
                               wrappingBuffer.data[partIndex],
                               framesToWrite);

            byteBuffer += numBytes;
            framesLeft -= framesToWrite;
        } else {
            break;
        }
        partIndex++;
    }
    int32_t framesWritten = numFrames - framesLeft;
    mAudioEndpoint.advanceWriteIndex(framesWritten);

    return framesWritten;
}

int64_t AudioStreamInternalPlay::getFramesRead() {
    const int64_t framesReadHardware = isClockModelInControl()
            ? mClockModel.convertTimeToPosition(AudioClock::getNanoseconds())
            : mAudioEndpoint.getDataReadCounter();
    // Add service offset and prevent retrograde motion.
    mLastFramesRead = std::max(mLastFramesRead, framesReadHardware + mFramesOffsetFromService);
    return mLastFramesRead;
}

int64_t AudioStreamInternalPlay::getFramesWritten() {
    const int64_t framesWritten = mAudioEndpoint.getDataWriteCounter()
                               + mFramesOffsetFromService;
    return framesWritten;
}


// Render audio in the application callback and then write the data to the stream.
void *AudioStreamInternalPlay::callbackLoop() {
    ALOGD("%s() entering >>>>>>>>>>>>>>>", __func__);
    aaudio_result_t result = AAUDIO_OK;
    aaudio_data_callback_result_t callbackResult = AAUDIO_CALLBACK_RESULT_CONTINUE;
    if (!isDataCallbackSet()) return NULL;
    int64_t timeoutNanos = calculateReasonableTimeout(mCallbackFrames);

    // result might be a frame count
    while (mCallbackEnabled.load() && isActive() && (result >= 0)) {
        // Call application using the AAudio callback interface.
        callbackResult = maybeCallDataCallback(mCallbackBuffer, mCallbackFrames);

        if (callbackResult == AAUDIO_CALLBACK_RESULT_CONTINUE) {
            // Write audio data to stream. This is a BLOCKING WRITE!
            result = write(mCallbackBuffer, mCallbackFrames, timeoutNanos);
            if ((result != mCallbackFrames)) {
                if (result >= 0) {
                    // Only wrote some of the frames requested. Must have timed out.
                    result = AAUDIO_ERROR_TIMEOUT;
                }
                maybeCallErrorCallback(result);
                break;
            }
        } else if (callbackResult == AAUDIO_CALLBACK_RESULT_STOP) {
            ALOGD("%s(): callback returned AAUDIO_CALLBACK_RESULT_STOP", __func__);
            result = systemStopFromCallback();
            break;
        }
    }

    ALOGD("%s() exiting, result = %d, isActive() = %d <<<<<<<<<<<<<<",
          __func__, result, (int) isActive());
    return NULL;
}

//------------------------------------------------------------------------------
// Implementation of PlayerBase
status_t AudioStreamInternalPlay::doSetVolume() {
    float combinedVolume = mStreamVolume * getDuckAndMuteVolume();
    ALOGD("%s() mStreamVolume * duckAndMuteVolume = %f * %f = %f",
          __func__, mStreamVolume, getDuckAndMuteVolume(), combinedVolume);
    mFlowGraph.setTargetVolume(combinedVolume);
    return android::NO_ERROR;
}
