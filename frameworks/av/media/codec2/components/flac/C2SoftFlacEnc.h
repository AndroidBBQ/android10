/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef ANDROID_C2_SOFT_FLAC_ENC_H_
#define ANDROID_C2_SOFT_FLAC_ENC_H_

#include <SimpleC2Component.h>

#include "FLAC/stream_encoder.h"

#define FLAC_COMPRESSION_LEVEL_MIN     0
#define FLAC_COMPRESSION_LEVEL_DEFAULT 5
#define FLAC_COMPRESSION_LEVEL_MAX     8

#define FLAC_HEADER_SIZE               128

#define MIN(a, b)                      (((a) < (b)) ? (a) : (b))

namespace android {

class C2SoftFlacEnc : public SimpleC2Component {
public:
    class IntfImpl;

    C2SoftFlacEnc(const char *name, c2_node_id_t id, const std::shared_ptr<IntfImpl> &intfImpl);
    virtual ~C2SoftFlacEnc();

    // From SimpleC2Component
    c2_status_t onInit() override;
    c2_status_t onStop() override;
    void onReset() override;
    void onRelease() override;
    c2_status_t onFlush_sm() override;
    void process(
            const std::unique_ptr<C2Work> &work,
            const std::shared_ptr<C2BlockPool> &pool) override;
    c2_status_t drain(
            uint32_t drainMode,
            const std::shared_ptr<C2BlockPool> &pool) override;

private:
    status_t configureEncoder();
    static FLAC__StreamEncoderWriteStatus flacEncoderWriteCallback(
            const FLAC__StreamEncoder *encoder, const FLAC__byte buffer[],
            size_t bytes, unsigned samples, unsigned current_frame,
            void *client_data);
    FLAC__StreamEncoderWriteStatus onEncodedFlacAvailable(
            const FLAC__byte buffer[], size_t bytes, unsigned samples,
            unsigned current_frame);

    std::shared_ptr<IntfImpl> mIntf;
    const unsigned int kInBlockSize = 1152;
    const unsigned int kMaxNumChannels = 2;
    FLAC__StreamEncoder* mFlacStreamEncoder;
    FLAC__int32* mInputBufferPcm32;
    std::shared_ptr<C2LinearBlock> mOutputBlock;
    bool mSignalledError;
    bool mSignalledOutputEos;
    uint32_t mBlockSize;
    bool mIsFirstFrame;
    uint64_t mAnchorTimeStamp;
    uint64_t mProcessedSamples;
    // should the data received by the callback be written to the output port
    bool mEncoderWriteData;
    size_t mEncoderReturnedNbBytes;
    unsigned mHeaderOffset;
    bool mWroteHeader;
    char mHeader[FLAC_HEADER_SIZE];

    C2_DO_NOT_COPY(C2SoftFlacEnc);
};

}  // namespace android

#endif  // ANDROID_C2_SOFT_FLAC_ENC_H_
