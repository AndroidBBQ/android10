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

//#define LOG_NDEBUG 0
#define LOG_TAG "C2SoftVp9Enc"
#include <utils/Log.h>
#include <utils/misc.h>

#include "C2SoftVp9Enc.h"

namespace android {

C2SoftVp9Enc::C2SoftVp9Enc(const char* name, c2_node_id_t id,
                           const std::shared_ptr<IntfImpl>& intfImpl)
    : C2SoftVpxEnc(name, id, intfImpl),
      mProfile(1),
      mLevel(0),
      mTileColumns(0),
      mFrameParallelDecoding(false) {
}

void C2SoftVp9Enc::setCodecSpecificInterface() {
    mCodecInterface = vpx_codec_vp9_cx();
}

void C2SoftVp9Enc::setCodecSpecificConfiguration() {
    switch (mProfile) {
        case 1:
            mCodecConfiguration->g_profile = 0;
            break;

        case 2:
            mCodecConfiguration->g_profile = 1;
            break;

        case 4:
            mCodecConfiguration->g_profile = 2;
            break;

        case 8:
            mCodecConfiguration->g_profile = 3;
            break;

        default:
            mCodecConfiguration->g_profile = 0;
    }
}

vpx_codec_err_t C2SoftVp9Enc::setCodecSpecificControls() {
    vpx_codec_err_t codecReturn = vpx_codec_control(
            mCodecContext, VP9E_SET_TILE_COLUMNS, mTileColumns);
    if (codecReturn != VPX_CODEC_OK) {
        ALOGE("Error setting VP9E_SET_TILE_COLUMNS to %d. vpx_codec_control() "
              "returned %d", mTileColumns, codecReturn);
        return codecReturn;
    }
    codecReturn = vpx_codec_control(
            mCodecContext, VP9E_SET_FRAME_PARALLEL_DECODING,
            mFrameParallelDecoding);
    if (codecReturn != VPX_CODEC_OK) {
        ALOGE("Error setting VP9E_SET_FRAME_PARALLEL_DECODING to %d."
              "vpx_codec_control() returned %d", mFrameParallelDecoding,
              codecReturn);
        return codecReturn;
    }
    codecReturn = vpx_codec_control(mCodecContext, VP9E_SET_ROW_MT, 1);
    if (codecReturn != VPX_CODEC_OK) {
        ALOGE("Error setting VP9E_SET_ROW_MT to 1. vpx_codec_control() "
              "returned %d", codecReturn);
        return codecReturn;
    }

    // For VP9, we always set CPU_USED to 8 (because the realtime default is 0
    // which is too slow).
    codecReturn = vpx_codec_control(mCodecContext, VP8E_SET_CPUUSED, 8);
    if (codecReturn != VPX_CODEC_OK) {
        ALOGE("Error setting VP8E_SET_CPUUSED to 8. vpx_codec_control() "
              "returned %d", codecReturn);
        return codecReturn;
    }
    return codecReturn;
}

class C2SoftVp9EncFactory : public C2ComponentFactory {
public:
    C2SoftVp9EncFactory()
        : mHelper(std::static_pointer_cast<C2ReflectorHelper>(
              GetCodec2PlatformComponentStore()->getParamReflector())) {}

    virtual c2_status_t createComponent(
            c2_node_id_t id,
            std::shared_ptr<C2Component>* const component,
            std::function<void(C2Component*)> deleter) override {
        *component = std::shared_ptr<C2Component>(
            new C2SoftVp9Enc(COMPONENT_NAME, id,
                             std::make_shared<C2SoftVpxEnc::IntfImpl>(mHelper)),
            deleter);
        return C2_OK;
    }

    virtual c2_status_t createInterface(
            c2_node_id_t id,
            std::shared_ptr<C2ComponentInterface>* const interface,
            std::function<void(C2ComponentInterface*)> deleter) override {
        *interface = std::shared_ptr<C2ComponentInterface>(
            new SimpleInterface<C2SoftVpxEnc::IntfImpl>(
                COMPONENT_NAME, id,
                std::make_shared<C2SoftVpxEnc::IntfImpl>(mHelper)),
            deleter);
        return C2_OK;
    }

    virtual ~C2SoftVp9EncFactory() override = default;

private:
    std::shared_ptr<C2ReflectorHelper> mHelper;
};

}  // namespace android

extern "C" ::C2ComponentFactory* CreateCodec2Factory() {
    ALOGV("in %s", __func__);
    return new ::android::C2SoftVp9EncFactory();
}

extern "C" void DestroyCodec2Factory(::C2ComponentFactory* factory) {
    ALOGV("in %s", __func__);
    delete factory;
}
