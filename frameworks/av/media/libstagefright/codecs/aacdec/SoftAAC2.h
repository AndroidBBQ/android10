/*
 * Copyright (C) 2012 The Android Open Source Project
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

#ifndef SOFT_AAC_2_H_
#define SOFT_AAC_2_H_

#include <media/stagefright/omx/SimpleSoftOMXComponent.h>

#include "aacdecoder_lib.h"
#include "DrcPresModeWrap.h"

namespace android {

struct SoftAAC2 : public SimpleSoftOMXComponent {
    SoftAAC2(const char *name,
            const OMX_CALLBACKTYPE *callbacks,
            OMX_PTR appData,
            OMX_COMPONENTTYPE **component);

protected:
    virtual ~SoftAAC2();

    virtual OMX_ERRORTYPE internalGetParameter(
            OMX_INDEXTYPE index, OMX_PTR params);

    virtual OMX_ERRORTYPE internalSetParameter(
            OMX_INDEXTYPE index, const OMX_PTR params);

    virtual void onQueueFilled(OMX_U32 portIndex);
    virtual void onPortFlushCompleted(OMX_U32 portIndex);
    virtual void onPortEnableCompleted(OMX_U32 portIndex, bool enabled);
    virtual void onReset();

private:
    enum {
        kNumInputBuffers        = 4,
        kNumOutputBuffers       = 4,
        kNumDelayBlocksMax      = 8,
    };

    HANDLE_AACDECODER mAACDecoder;
    CStreamInfo *mStreamInfo;
    bool mIsADTS;
    bool mIsFirst;
    size_t mInputBufferCount;
    size_t mOutputBufferCount;
    bool mSignalledError;
    OMX_BUFFERHEADERTYPE *mLastInHeader;
    Vector<int32_t> mBufferSizes;
    Vector<int32_t> mDecodedSizes;
    Vector<int64_t> mBufferTimestamps;

    CDrcPresModeWrapper mDrcWrap;

    enum {
        NONE,
        AWAITING_DISABLED,
        AWAITING_ENABLED
    } mOutputPortSettingsChange;

    void initPorts();
    status_t initDecoder();
    bool isConfigured() const;
    void drainDecoder();

//      delay compensation
    bool mEndOfInput;
    bool mEndOfOutput;
    int32_t mOutputDelayCompensated;
    int32_t mOutputDelayRingBufferSize;
    int16_t *mOutputDelayRingBuffer;
    int32_t mOutputDelayRingBufferWritePos;
    int32_t mOutputDelayRingBufferReadPos;
    int32_t mOutputDelayRingBufferFilled;
    bool outputDelayRingBufferPutSamples(INT_PCM *samples, int numSamples);
    int32_t outputDelayRingBufferGetSamples(INT_PCM *samples, int numSamples);
    int32_t outputDelayRingBufferSamplesAvailable();
    int32_t outputDelayRingBufferSpaceLeft();

    DISALLOW_EVIL_CONSTRUCTORS(SoftAAC2);
};

}  // namespace android

#endif  // SOFT_AAC_2_H_
