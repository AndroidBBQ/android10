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

#ifndef ANDROID_C2_SOFT_VPX_DEC_H_
#define ANDROID_C2_SOFT_VPX_DEC_H_

#include <SimpleC2Component.h>


#include "vpx/vpx_decoder.h"
#include "vpx/vp8dx.h"

namespace android {

struct C2SoftVpxDec : public SimpleC2Component {
    class IntfImpl;

    C2SoftVpxDec(const char* name, c2_node_id_t id,
              const std::shared_ptr<IntfImpl>& intfImpl);
    virtual ~C2SoftVpxDec();

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
        MODE_VP8,
        MODE_VP9,
    } mMode;

    struct ConversionQueue;

    class ConverterThread : public Thread {
    public:
        explicit ConverterThread(
                const std::shared_ptr<Mutexed<ConversionQueue>> &queue);
        ~ConverterThread() override = default;
        bool threadLoop() override;

    private:
        std::shared_ptr<Mutexed<ConversionQueue>> mQueue;
    };

    std::shared_ptr<IntfImpl> mIntf;
    vpx_codec_ctx_t *mCodecCtx;
    bool mFrameParallelMode;  // Frame parallel is only supported by VP9 decoder.

    uint32_t mWidth;
    uint32_t mHeight;
    bool mSignalledOutputEos;
    bool mSignalledError;

    int mCoreCount;
    struct ConversionQueue {
        std::list<std::function<void()>> entries;
        Condition cond;
        size_t numPending{0u};
    };
    std::shared_ptr<Mutexed<ConversionQueue>> mQueue;
    std::vector<sp<ConverterThread>> mConverterThreads;

    status_t initDecoder();
    status_t destroyDecoder();
    void finishWork(uint64_t index, const std::unique_ptr<C2Work> &work,
                    const std::shared_ptr<C2GraphicBlock> &block);
    status_t outputBuffer(
            const std::shared_ptr<C2BlockPool> &pool,
            const std::unique_ptr<C2Work> &work);
    c2_status_t drainInternal(
            uint32_t drainMode,
            const std::shared_ptr<C2BlockPool> &pool,
            const std::unique_ptr<C2Work> &work);

    C2_DO_NOT_COPY(C2SoftVpxDec);
};

}  // namespace android

#endif  // ANDROID_C2_SOFT_VPX_DEC_H_
