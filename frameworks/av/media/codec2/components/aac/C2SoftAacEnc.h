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

#ifndef ANDROID_C2_SOFT_AAC_ENC_H_
#define ANDROID_C2_SOFT_AAC_ENC_H_

#include <atomic>

#include <SimpleC2Component.h>

#include "aacenc_lib.h"

namespace android {

class C2SoftAacEnc : public SimpleC2Component {
public:
    class IntfImpl;

    C2SoftAacEnc(const char *name, c2_node_id_t id, const std::shared_ptr<IntfImpl> &intfImpl);
    virtual ~C2SoftAacEnc();

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
    std::shared_ptr<IntfImpl> mIntf;

    HANDLE_AACENCODER mAACEncoder;

    UINT mNumBytesPerInputFrame;
    UINT mOutBufferSize;

    bool mSentCodecSpecificData;
    bool mInputTimeSet;
    size_t mInputSize;
    c2_cntr64_t mNextFrameTimestampUs;

    bool mSignalledError;
    std::atomic_uint64_t mOutIndex;

    // We support max 6 channels
    uint8_t mRemainder[6 * sizeof(int16_t)];
    size_t mRemainderLen;

    status_t initEncoder();

    status_t setAudioParams();

    C2_DO_NOT_COPY(C2SoftAacEnc);
};

}  // namespace android

#endif  // ANDROID_C2_SOFT_AAC_ENC_H_
