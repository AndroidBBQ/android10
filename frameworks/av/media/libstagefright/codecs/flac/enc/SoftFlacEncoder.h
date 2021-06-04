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

#ifndef SOFT_FLAC_ENC_H_

#define SOFT_FLAC_ENC_H_

#include <media/stagefright/omx/SimpleSoftOMXComponent.h>

#include "FLAC/stream_encoder.h"

namespace android {

struct SoftFlacEncoder : public SimpleSoftOMXComponent {
    SoftFlacEncoder(const char *name,
            const OMX_CALLBACKTYPE *callbacks,
            OMX_PTR appData,
            OMX_COMPONENTTYPE **component);

    virtual OMX_ERRORTYPE initCheck() const;

protected:
    virtual ~SoftFlacEncoder();

    virtual OMX_ERRORTYPE internalGetParameter(
            OMX_INDEXTYPE index, OMX_PTR params);

    virtual OMX_ERRORTYPE internalSetParameter(
            OMX_INDEXTYPE index, const OMX_PTR params);

    virtual void onQueueFilled(OMX_U32 portIndex);

private:
    const unsigned int kNumBuffers = 2;
    static constexpr unsigned int kMaxChannels = 2;
    static constexpr unsigned int kNumSamplesPerFrame = 1152;
    static constexpr unsigned int kMaxInputBufferSize =
            kNumSamplesPerFrame * kMaxChannels * sizeof(float);
    const unsigned int kMaxOutputBufferSize = 65536;    //TODO check if this can be reduced

    bool mSignalledError;

    OMX_U32 mNumChannels;
    OMX_U32 mSampleRate;
    OMX_U32 mCompressionLevel;
    OMX_NUMERICALDATATYPE mNumericalData = OMX_NumericalDataSigned;
    OMX_U32 mBitsPerSample = 16;

    // should the data received by the callback be written to the output port
    bool        mEncoderWriteData;
    bool        mEncoderReturnedEncodedData;
    bool        mSawInputEOS;
    bool        mSentOutputEOS;
    size_t      mEncoderReturnedNbBytes;
    OMX_TICKS  mCurrentInputTimeStamp;

    FLAC__StreamEncoder* mFlacStreamEncoder;

    void initPorts();

    OMX_ERRORTYPE configureEncoder();

    // FLAC encoder callbacks
    // maps to encoderEncodeFlac()
    static FLAC__StreamEncoderWriteStatus flacEncoderWriteCallback(
            const FLAC__StreamEncoder *encoder, const FLAC__byte buffer[],
            size_t bytes, unsigned samples, unsigned current_frame, void *client_data);

    FLAC__StreamEncoderWriteStatus onEncodedFlacAvailable(
                const FLAC__byte buffer[],
                size_t bytes, unsigned samples, unsigned current_frame);

    // FLAC takes samples aligned on 32bit boundaries, use this buffer for the conversion
    // before passing the input data to the encoder
    FLAC__int32* mInputBufferPcm32;

    unsigned mHeaderOffset;
    bool mHeaderComplete;
    bool mWroteHeader;
    char mHeader[128];

    DISALLOW_EVIL_CONSTRUCTORS(SoftFlacEncoder);
};

}  // namespace android

#endif  // SOFT_FLAC_ENC_H_

