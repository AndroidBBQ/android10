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

#define LOG_TAG "AAudioMixer"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#define ATRACE_TAG ATRACE_TAG_AUDIO

#include <cstring>
#include <utils/Trace.h>

#include "AAudioMixer.h"

#ifndef AAUDIO_MIXER_ATRACE_ENABLED
#define AAUDIO_MIXER_ATRACE_ENABLED    1
#endif

using android::WrappingBuffer;
using android::FifoBuffer;
using android::fifo_frames_t;

AAudioMixer::~AAudioMixer() {
    delete[] mOutputBuffer;
}

void AAudioMixer::allocate(int32_t samplesPerFrame, int32_t framesPerBurst) {
    mSamplesPerFrame = samplesPerFrame;
    mFramesPerBurst = framesPerBurst;
    int32_t samplesPerBuffer = samplesPerFrame * framesPerBurst;
    mOutputBuffer = new float[samplesPerBuffer];
    mBufferSizeInBytes = samplesPerBuffer * sizeof(float);
}

void AAudioMixer::clear() {
    memset(mOutputBuffer, 0, mBufferSizeInBytes);
}

int32_t AAudioMixer::mix(int streamIndex, FifoBuffer *fifo, bool allowUnderflow) {
    WrappingBuffer wrappingBuffer;
    float *destination = mOutputBuffer;

#if AAUDIO_MIXER_ATRACE_ENABLED
    ATRACE_BEGIN("aaMix");
#endif /* AAUDIO_MIXER_ATRACE_ENABLED */

    // Gather the data from the client. May be in two parts.
    fifo_frames_t fullFrames = fifo->getFullDataAvailable(&wrappingBuffer);
#if AAUDIO_MIXER_ATRACE_ENABLED
    if (ATRACE_ENABLED()) {
        char rdyText[] = "aaMixRdy#";
        char letter = 'A' + (streamIndex % 26);
        rdyText[sizeof(rdyText) - 2] = letter;
        ATRACE_INT(rdyText, fullFrames);
    }
#else /* MIXER_ATRACE_ENABLED */
    (void) trackIndex;
#endif /* AAUDIO_MIXER_ATRACE_ENABLED */

    // If allowUnderflow then always advance by one burst even if we do not have the data.
    // Otherwise the stream timing will drift whenever there is an underflow.
    // This actual underflow can then be detected by the client for XRun counting.
    //
    // Generally, allowUnderflow will be false when stopping a stream and we want to
    // use up whatever data is in the queue.
    fifo_frames_t framesDesired = mFramesPerBurst;
    if (!allowUnderflow && fullFrames < framesDesired) {
        framesDesired = fullFrames; // just use what is available then stop
    }

    // Mix data in one or two parts.
    int partIndex = 0;
    int32_t framesLeft = framesDesired;
    while (framesLeft > 0 && partIndex < WrappingBuffer::SIZE) {
        fifo_frames_t framesToMixFromPart = framesLeft;
        fifo_frames_t framesAvailableFromPart = wrappingBuffer.numFrames[partIndex];
        if (framesAvailableFromPart > 0) {
            if (framesToMixFromPart > framesAvailableFromPart) {
                framesToMixFromPart = framesAvailableFromPart;
            }
            mixPart(destination, (float *)wrappingBuffer.data[partIndex],
                    framesToMixFromPart);

            destination += framesToMixFromPart * mSamplesPerFrame;
            framesLeft -= framesToMixFromPart;
        }
        partIndex++;
    }
    fifo->advanceReadIndex(framesDesired);

#if AAUDIO_MIXER_ATRACE_ENABLED
    ATRACE_END();
#endif /* AAUDIO_MIXER_ATRACE_ENABLED */

    return (framesDesired - framesLeft); // framesRead
}

void AAudioMixer::mixPart(float *destination, float *source, int32_t numFrames) {
    int32_t numSamples = numFrames * mSamplesPerFrame;
    // TODO maybe optimize using SIMD
    for (int sampleIndex = 0; sampleIndex < numSamples; sampleIndex++) {
        *destination++ += *source++;
    }
}

float *AAudioMixer::getOutputBuffer() {
    return mOutputBuffer;
}
