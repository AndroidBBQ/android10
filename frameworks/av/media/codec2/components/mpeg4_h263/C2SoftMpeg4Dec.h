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

#ifndef C2_SOFT_MPEG4_DEC_H_
#define C2_SOFT_MPEG4_DEC_H_

#include <SimpleC2Component.h>


struct tagvideoDecControls;

namespace android {

struct C2SoftMpeg4Dec : public SimpleC2Component {
    class IntfImpl;

    C2SoftMpeg4Dec(const char* name, c2_node_id_t id,
                   const std::shared_ptr<IntfImpl>& intfImpl);
    virtual ~C2SoftMpeg4Dec();

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
        kNumOutputBuffers = 2,
    };

    status_t initDecoder();
    c2_status_t ensureDecoderState(const std::shared_ptr<C2BlockPool> &pool);
    void finishWork(uint64_t index, const std::unique_ptr<C2Work> &work);
    bool handleResChange(const std::unique_ptr<C2Work> &work);

    std::shared_ptr<IntfImpl> mIntf;
    tagvideoDecControls *mDecHandle;
    std::shared_ptr<C2GraphicBlock> mOutBlock;
    uint8_t *mOutputBuffer[kNumOutputBuffers];
    size_t  mOutputBufferSize;

    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mNumSamplesOutput;

    bool mIsMpeg4;
    bool mInitialized;
    bool mFramesConfigured;
    bool mSignalledOutputEos;
    bool mSignalledError;

    C2_DO_NOT_COPY(C2SoftMpeg4Dec);
};

}  // namespace android

#endif  // C2_SOFT_MPEG4_DEC_H_
