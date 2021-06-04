/*
 * Copyright 2018 The Android Open Source Project
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

#ifndef C2_SOFT_MPEG4_ENC_H__
#define C2_SOFT_MPEG4_ENC_H__

#include <map>

#include <Codec2BufferUtils.h>
#include <SimpleC2Component.h>

#include "mp4enc_api.h"

namespace android {

struct C2SoftMpeg4Enc : public SimpleC2Component {
    class IntfImpl;

    C2SoftMpeg4Enc(const char* name, c2_node_id_t id,
                   const std::shared_ptr<IntfImpl>& intfImpl);

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

protected:

    virtual ~C2SoftMpeg4Enc();

private:
    std::shared_ptr<IntfImpl> mIntf;

    tagvideoEncControls *mHandle;
    tagvideoEncOptions  *mEncParams;

    bool     mStarted;
    bool     mSignalledOutputEos;
    bool     mSignalledError;

    uint32_t mOutBufferSize;
    // configurations used by component in process
    // (TODO: keep this in intf but make them internal only)
    std::shared_ptr<C2StreamPictureSizeInfo::input> mSize;
    std::shared_ptr<C2StreamFrameRateInfo::output> mFrameRate;
    std::shared_ptr<C2StreamBitrateInfo::output> mBitrate;

    int64_t  mNumInputFrames;
    MP4EncodingMode mEncodeMode;

    MemoryBlockPool mConversionBuffers;
    std::map<void *, MemoryBlock> mConversionBuffersInUse;

    c2_status_t initEncParams();
    c2_status_t initEncoder();

    C2_DO_NOT_COPY(C2SoftMpeg4Enc);
};

}  // namespace android

#endif  // C2_SOFT_MPEG4_ENC_H__
