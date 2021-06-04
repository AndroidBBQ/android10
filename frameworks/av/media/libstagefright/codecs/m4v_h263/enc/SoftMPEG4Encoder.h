/*
 * Copyright (C) 2012 The Android Open Source Project
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

#ifndef SOFT_MPEG4_ENCODER_H_
#define SOFT_MPEG4_ENCODER_H_

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/omx/SoftVideoEncoderOMXComponent.h>
#include "mp4enc_api.h"


namespace android {

struct CodecProfileLevel;

struct SoftMPEG4Encoder : public SoftVideoEncoderOMXComponent {
    SoftMPEG4Encoder(
            const char *name,
            const char *componentRole,
            OMX_VIDEO_CODINGTYPE codingType,
            const char *mime,
            const CodecProfileLevel *profileLevels,
            size_t numProfileLevels,
            const OMX_CALLBACKTYPE *callbacks,
            OMX_PTR appData,
            OMX_COMPONENTTYPE **component);

    // Override SimpleSoftOMXComponent methods
    virtual OMX_ERRORTYPE internalGetParameter(
            OMX_INDEXTYPE index, OMX_PTR params);

    virtual OMX_ERRORTYPE internalSetParameter(
            OMX_INDEXTYPE index, const OMX_PTR params);

    virtual void onQueueFilled(OMX_U32 portIndex);

    virtual void onReset();

protected:
    virtual ~SoftMPEG4Encoder();

private:
    enum {
        kNumBuffers = 2,
    };

    // OMX input buffer's timestamp and flags
    typedef struct {
        int64_t mTimeUs;
        int32_t mFlags;
    } InputBufferInfo;

    MP4EncodingMode mEncodeMode;
    int32_t  mKeyFrameInterval; // 1: all I-frames, <0: infinite

    int64_t  mNumInputFrames;
    bool     mStarted;
    bool     mSawInputEOS;
    bool     mSignalledError;

    tagvideoEncControls   *mHandle;
    tagvideoEncOptions    *mEncParams;
    uint8_t               *mInputFrameData;
    Vector<InputBufferInfo> mInputBufferInfoVec;

    OMX_ERRORTYPE initEncParams();
    OMX_ERRORTYPE initEncoder();
    OMX_ERRORTYPE releaseEncoder();

    DISALLOW_EVIL_CONSTRUCTORS(SoftMPEG4Encoder);
};

}  // namespace android

#endif  // SOFT_MPEG4_ENCODER_H_
