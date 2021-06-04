/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef SOFT_VP9_ENCODER_H_

#define SOFT_VP9_ENCODER_H_

#include "SoftVPXEncoder.h"

#include <OMX_VideoExt.h>
#include <OMX_IndexExt.h>

#include "vpx/vpx_encoder.h"
#include "vpx/vpx_codec.h"
#include "vpx/vp8cx.h"

namespace android {

// Exposes a VP9 encoder as an OMX Component
//
// In addition to the base class settings, Only following encoder settings are
// available:
//    - tile rows
//    - tile columns
//    - frame parallel mode
struct SoftVP9Encoder : public SoftVPXEncoder {
    SoftVP9Encoder(const char *name,
                   const OMX_CALLBACKTYPE *callbacks,
                   OMX_PTR appData,
                   OMX_COMPONENTTYPE **component);

protected:
    // Returns current values for requested OMX
    // parameters
    virtual OMX_ERRORTYPE internalGetParameter(
            OMX_INDEXTYPE index, OMX_PTR param);

    // Validates, extracts and stores relevant OMX
    // parameters
    virtual OMX_ERRORTYPE internalSetParameter(
            OMX_INDEXTYPE index, const OMX_PTR param);

    // Populates |mCodecInterface| with codec specific settings.
    virtual void setCodecSpecificInterface();

    // Sets codec specific configuration.
    virtual void setCodecSpecificConfiguration();

    // Initializes codec specific encoder settings.
    virtual vpx_codec_err_t setCodecSpecificControls();

    // Gets vp9 specific parameters.
    OMX_ERRORTYPE internalGetVp9Params(
        OMX_VIDEO_PARAM_VP9TYPE* vp9Params);

    // Handles vp9 specific parameters.
    OMX_ERRORTYPE internalSetVp9Params(
        const OMX_VIDEO_PARAM_VP9TYPE* vp9Params);

private:
    // Encoder profile corresponding to OMX level parameter
    //
    // The inconsistency in the naming is caused by
    // OMX spec referring vpx profiles (g_profile)
    // as "levels" whereas using the name "profile" for
    // something else.
    OMX_VIDEO_VP9LEVELTYPE mLevel;

    int32_t mTileColumns;

    OMX_BOOL mFrameParallelDecoding;

    DISALLOW_EVIL_CONSTRUCTORS(SoftVP9Encoder);
};

}  // namespace android

#endif  // SOFT_VP9_ENCODER_H_
