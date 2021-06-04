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

#ifndef ANDROID_C2_SOFT_XAAC_DEC_H_
#define ANDROID_C2_SOFT_XAAC_DEC_H_
#include <utils/Vector.h>
#include <SimpleC2Component.h>

#include "ixheaacd_type_def.h"
#include "ixheaacd_error_standards.h"
#include "ixheaacd_error_handler.h"
#include "ixheaacd_apicmd_standards.h"
#include "ixheaacd_memory_standards.h"
#include "ixheaacd_aac_config.h"

#include "impd_apicmd_standards.h"
#include "impd_drc_config_params.h"

#define MAX_CHANNEL_COUNT           8  /* maximum number of audio channels that can be decoded */
#define MAX_NUM_BLOCKS              8  /* maximum number of audio blocks that can be decoded */

extern "C" IA_ERRORCODE ixheaacd_dec_api(pVOID p_ia_module_obj,
                        WORD32 i_cmd, WORD32 i_idx, pVOID pv_value);
extern "C" IA_ERRORCODE ia_drc_dec_api(pVOID p_ia_module_obj,
                        WORD32 i_cmd, WORD32 i_idx, pVOID pv_value);
extern "C"  IA_ERRORCODE ixheaacd_get_config_param(pVOID p_ia_process_api_obj,
                                       pWORD32 pi_samp_freq,
                                       pWORD32 pi_num_chan,
                                       pWORD32 pi_pcm_wd_sz,
                                       pWORD32 pi_channel_mask);

namespace android {

struct C2SoftXaacDec : public SimpleC2Component {
    class IntfImpl;

    C2SoftXaacDec(const char* name, c2_node_id_t id,
               const std::shared_ptr<IntfImpl>& intfImpl);
    virtual ~C2SoftXaacDec();

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
        kOutputDrainBufferSize      = 2048 * MAX_CHANNEL_COUNT * MAX_NUM_BLOCKS,
    };

    std::shared_ptr<IntfImpl> mIntf;
    void* mXheaacCodecHandle;
    void* mMpegDDrcHandle;
    uint32_t mInputBufferSize;
    uint32_t mOutputFrameLength;
    int8_t* mInputBuffer;
    int8_t* mOutputBuffer;
    int32_t mSampFreq;
    int32_t mNumChannels;
    int32_t mPcmWdSz;
    int32_t mChannelMask;
    int32_t mNumOutBytes;
    uint64_t mCurFrameIndex;
    uint64_t mCurTimestamp;
    bool mIsCodecInitialized;
    bool mIsCodecConfigFlushRequired;
    int8_t* mDrcInBuf;
    int8_t* mDrcOutBuf;
    int32_t mMpegDDRCPresent;
    int32_t mDRCFlag;

    Vector<void*> mMemoryVec;
    Vector<void*> mDrcMemoryVec;

    size_t mInputBufferCount __unused;
    size_t mOutputBufferCount __unused;
    bool mSignalledOutputEos;
    bool mSignalledError;
    char* mOutputDrainBuffer;
    uint32_t mOutputDrainBufferWritePos;

    IA_ERRORCODE initDecoder();
    IA_ERRORCODE setDrcParameter();
    IA_ERRORCODE configflushDecode();
    IA_ERRORCODE drainDecoder();
    void finishWork(const std::unique_ptr<C2Work>& work,
                    const std::shared_ptr<C2BlockPool>& pool);

    IA_ERRORCODE initXAACDrc();
    IA_ERRORCODE initXAACDecoder();
    IA_ERRORCODE deInitXAACDecoder();
    IA_ERRORCODE initMPEGDDDrc();
    IA_ERRORCODE deInitMPEGDDDrc();
    IA_ERRORCODE configXAACDecoder(uint8_t* inBuffer, uint32_t inBufferLength);
    int configMPEGDDrc();
    IA_ERRORCODE decodeXAACStream(uint8_t* inBuffer,
                         uint32_t inBufferLength,
                         int32_t* bytesConsumed,
                         int32_t* outBytes);
    IA_ERRORCODE getXAACStreamInfo();
    IA_ERRORCODE setXAACDRCInfo(int32_t drcCut, int32_t drcBoost,
                                int32_t drcRefLevel, int32_t drcHeavyCompression,
                                int32_t drEffectType);

    C2_DO_NOT_COPY(C2SoftXaacDec);
};

}  // namespace android

#endif  // C2_SOFT_XAAC_H_
