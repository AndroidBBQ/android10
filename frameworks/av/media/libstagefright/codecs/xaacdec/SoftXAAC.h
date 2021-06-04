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

#ifndef SOFTXAAC_H_
#define SOFTXAAC_H_

#include <media/stagefright/omx/SimpleSoftOMXComponent.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ixheaacd_type_def.h"
#include "ixheaacd_error_standards.h"
#include "ixheaacd_error_handler.h"
#include "ixheaacd_apicmd_standards.h"
#include "ixheaacd_memory_standards.h"
#include "ixheaacd_aac_config.h"

#include "impd_apicmd_standards.h"
#include "impd_drc_config_params.h"

extern "C" IA_ERRORCODE ixheaacd_dec_api(pVOID p_ia_module_obj, WORD32 i_cmd, WORD32 i_idx,
                                         pVOID pv_value);
extern "C" IA_ERRORCODE ia_drc_dec_api(pVOID p_ia_module_obj, WORD32 i_cmd, WORD32 i_idx,
                                       pVOID pv_value);
extern "C" IA_ERRORCODE ixheaacd_get_config_param(pVOID p_ia_process_api_obj, pWORD32 pi_samp_freq,
                                                  pWORD32 pi_num_chan, pWORD32 pi_pcm_wd_sz,
                                                  pWORD32 pi_channel_mask);

namespace android {

struct SoftXAAC : public SimpleSoftOMXComponent {
    SoftXAAC(const char* name, const OMX_CALLBACKTYPE* callbacks, OMX_PTR appData,
             OMX_COMPONENTTYPE** component);

   protected:
    virtual ~SoftXAAC();

    virtual OMX_ERRORTYPE internalGetParameter(OMX_INDEXTYPE index, OMX_PTR params);

    virtual OMX_ERRORTYPE internalSetParameter(OMX_INDEXTYPE index, const OMX_PTR params);

    virtual void onQueueFilled(OMX_U32 portIndex);
    virtual void onPortFlushCompleted(OMX_U32 portIndex);
    virtual void onPortEnableCompleted(OMX_U32 portIndex, bool enabled);
    virtual void onReset();

   private:
    enum {
        kNumInputBuffers = 4,
        kNumOutputBuffers = 4,
        kNumDelayBlocksMax = 8,
    };

    bool mIsADTS;
    size_t mInputBufferCount;
    size_t mOutputBufferCount;
    bool mSignalledError;
    OMX_BUFFERHEADERTYPE* mLastInHeader;
    int64_t mPrevTimestamp;
    int64_t mCurrentTimestamp;
    uint32_t mBufSize;

    enum { NONE, AWAITING_DISABLED, AWAITING_ENABLED } mOutputPortSettingsChange;

    void initPorts();
    IA_ERRORCODE initDecoder();
    bool isConfigured() const;
    IA_ERRORCODE drainDecoder();
    IA_ERRORCODE initXAACDecoder();
    IA_ERRORCODE deInitXAACDecoder();
    IA_ERRORCODE initMPEGDDDrc();
    IA_ERRORCODE deInitMPEGDDDrc();
    IA_ERRORCODE configXAACDecoder(uint8_t* inBuffer, uint32_t inBufferLength);
    IA_ERRORCODE configMPEGDDrc();
    IA_ERRORCODE decodeXAACStream(uint8_t* inBuffer, uint32_t inBufferLength,
                                  int32_t* bytesConsumed, int32_t* outBytes);

    IA_ERRORCODE configflushDecode();
    IA_ERRORCODE getXAACStreamInfo();
    IA_ERRORCODE setXAACDRCInfo(int32_t drcCut, int32_t drcBoost, int32_t drcRefLevel,
                                int32_t drcHeavyCompression
#ifdef ENABLE_MPEG_D_DRC
                                ,
                                int32_t drEffectType
#endif
    );

    bool mEndOfInput;
    bool mEndOfOutput;

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
    bool mIsCodecInitialized;
    bool mIsCodecConfigFlushRequired;
    int8_t* mDrcInBuf;
    int8_t* mDrcOutBuf;
    int32_t mMpegDDRCPresent;
    int32_t mDRCFlag;
    Vector<void*> mMemoryVec;
    Vector<void*> mDrcMemoryVec;

    DISALLOW_EVIL_CONSTRUCTORS(SoftXAAC);
};

}  // namespace android

#endif  // SOFTXAAC_H_
