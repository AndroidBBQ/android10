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

#ifndef AAUDIO_AAUDIO_MIXER_H
#define AAUDIO_AAUDIO_MIXER_H

#include <stdint.h>

#include <aaudio/AAudio.h>
#include <fifo/FifoBuffer.h>

class AAudioMixer {
public:
    AAudioMixer() {}
    ~AAudioMixer();

    void allocate(int32_t samplesPerFrame, int32_t framesPerBurst);

    void clear();

    /**
     * Mix from this FIFO
     * @param streamIndex for marking stream variables in systrace
     * @param fifo to read from
     * @param allowUnderflow if true then allow mixer to advance read index past the write index
     * @return frames read from this stream
     */
    int32_t mix(int streamIndex, android::FifoBuffer *fifo, bool allowUnderflow);

    float *getOutputBuffer();

    int32_t getFramesPerBurst() const { return mFramesPerBurst; }

private:
    void mixPart(float *destination, float *source, int32_t numFrames);

    float   *mOutputBuffer = nullptr;
    int32_t  mSamplesPerFrame = 0;
    int32_t  mFramesPerBurst = 0;
    int32_t  mBufferSizeInBytes = 0;
};

#endif //AAUDIO_AAUDIO_MIXER_H
