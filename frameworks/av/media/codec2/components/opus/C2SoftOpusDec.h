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

#ifndef ANDROID_C2_SOFT_OPUS_DEC_H_
#define ANDROID_C2_SOFT_OPUS_DEC_H_

#include <SimpleC2Component.h>


struct OpusMSDecoder;

namespace android {

struct C2SoftOpusDec : public SimpleC2Component {
    class IntfImpl;

    C2SoftOpusDec(const char *name, c2_node_id_t id,
               const std::shared_ptr<IntfImpl> &intfImpl);
    virtual ~C2SoftOpusDec();

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
    enum {
        kMaxNumSamplesPerBuffer = 960 * 6
    };

    std::shared_ptr<IntfImpl> mIntf;
    OpusMSDecoder *mDecoder;
    OpusHeader mHeader;

    int64_t mCodecDelay;
    int64_t mSeekPreRoll;
    int64_t mSamplesToDiscard;
    size_t mInputBufferCount;
    bool mSignalledError;
    bool mSignalledOutputEos;

    status_t initDecoder();

    C2_DO_NOT_COPY(C2SoftOpusDec);
};

}  // namespace android

#endif  // ANDROID_C2_SOFT_OPUS_DEC_H_
