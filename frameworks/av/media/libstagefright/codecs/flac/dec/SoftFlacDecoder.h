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

#ifndef SOFT_FLAC_DECODER_H
#define SOFT_FLAC_DECODER_H

#include "FLACDecoder.h"
#include <media/stagefright/omx/SimpleSoftOMXComponent.h>

namespace android {

struct SoftFlacDecoder : public SimpleSoftOMXComponent {
    SoftFlacDecoder(const char *name,
            const OMX_CALLBACKTYPE *callbacks,
            OMX_PTR appData,
            OMX_COMPONENTTYPE **component);

    virtual OMX_ERRORTYPE initCheck() const override;

protected:
    virtual ~SoftFlacDecoder();

    virtual OMX_ERRORTYPE internalGetParameter(
            OMX_INDEXTYPE index, OMX_PTR params) override;

    virtual OMX_ERRORTYPE internalSetParameter(
            OMX_INDEXTYPE index, const OMX_PTR params) override;

    virtual void onQueueFilled(OMX_U32 portIndex);
    virtual void onPortFlushCompleted(OMX_U32 portIndex) override;
    virtual void onPortEnableCompleted(OMX_U32 portIndex, bool enabled) override;
    virtual void onReset() override;

private:
    static constexpr unsigned int kNumSamplesPerFrame = 2048; // adjusted based on stream.

    enum {
        kNumInputBuffers   = 4,
        kNumOutputBuffers  = 4,
    };
    OMX_NUMERICALDATATYPE mNumericalData = OMX_NumericalDataSigned;
    OMX_U32 mBitsPerSample = 16;

    FLACDecoder *mFLACDecoder;
    FLAC__StreamMetadata_StreamInfo mStreamInfo;
    size_t mInputBufferCount;
    bool mHasStreamInfo;
    bool mSignalledError;
    bool mSawInputEOS;
    bool mFinishedDecoder;

    enum {
        NONE,
        AWAITING_DISABLED,
        AWAITING_ENABLED
    } mOutputPortSettingsChange;

    void initPorts();
    void initDecoder();
    bool isConfigured() const;
    void drainDecoder();

    DISALLOW_EVIL_CONSTRUCTORS(SoftFlacDecoder);
};

}  // namespace android

#endif  // SOFT_FLAC_DECODER_H
