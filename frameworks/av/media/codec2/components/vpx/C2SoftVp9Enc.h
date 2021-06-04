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

#ifndef ANDROID_C2_SOFT_VP9_ENC_H__
#define ANDROID_C2_SOFT_VP9_ENC_H__

#include "C2SoftVpxEnc.h"

namespace android {

// Exposes vp9 encoder as a c2 Component
//
// In addition to the base class settings, Only following encoder settings are
// available:
//    - tile rows
//    - tile columns
//    - frame parallel mode
struct C2SoftVp9Enc : public C2SoftVpxEnc {
    C2SoftVp9Enc(const char* name, c2_node_id_t id,
                 const std::shared_ptr<IntfImpl>& intfImpl);

 protected:
    // Populates |mCodecInterface| with codec specific settings.
    virtual void setCodecSpecificInterface();

    // Sets codec specific configuration.
    virtual void setCodecSpecificConfiguration();

    // Initializes codec specific encoder settings.
    virtual vpx_codec_err_t setCodecSpecificControls();

 private:
    // C2 Profile & Level parameter
    int32_t mProfile;
    int32_t mLevel __unused;

    int32_t mTileColumns;

    bool mFrameParallelDecoding;

    C2_DO_NOT_COPY(C2SoftVp9Enc);
};

}  // namespace android

#endif  // ANDROID_C2_SOFT_VP9_ENC_H__
