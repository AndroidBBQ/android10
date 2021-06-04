/*
 * Copyright (C) 2011 The Android Open Source Project
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

#ifndef SOFT_VPX_H_

#define SOFT_VPX_H_

#include <media/stagefright/omx/SoftVideoDecoderOMXComponent.h>

#include "vpx/vpx_decoder.h"
#include "vpx/vpx_codec.h"
#include "vpx/vp8dx.h"

namespace android {

struct ABuffer;

struct SoftVPX : public SoftVideoDecoderOMXComponent {
    SoftVPX(const char *name,
            const char *componentRole,
            OMX_VIDEO_CODINGTYPE codingType,
            const OMX_CALLBACKTYPE *callbacks,
            OMX_PTR appData,
            OMX_COMPONENTTYPE **component);

protected:
    virtual ~SoftVPX();

    virtual void onQueueFilled(OMX_U32 portIndex);
    virtual void onPortFlushCompleted(OMX_U32 portIndex);
    virtual void onReset();
    virtual bool supportDescribeHdrStaticInfo();
    virtual bool supportDescribeHdr10PlusInfo();

private:
    enum {
        kNumBuffers = 10
    };

    enum {
        MODE_VP8,
        MODE_VP9
    } mMode;

    enum {
        INPUT_DATA_AVAILABLE,  // VPX component is ready to decode data.
        INPUT_EOS_SEEN,        // VPX component saw EOS and is flushing On2 decoder.
        OUTPUT_FRAMES_FLUSHED  // VPX component finished flushing On2 decoder.
    } mEOSStatus;

    void *mCtx;
    bool mFrameParallelMode;  // Frame parallel is only supported by VP9 decoder.
    struct PrivInfo {
        OMX_TICKS mTimeStamp;
        sp<ABuffer> mHdr10PlusInfo;
    };
    PrivInfo mPrivInfo[kNumBuffers];
    uint8_t mTimeStampIdx;
    vpx_image_t *mImg;

    status_t initDecoder();
    status_t destroyDecoder();
    bool outputBuffers(bool flushDecoder, bool display, bool eos, bool *portWillReset);
    bool outputBufferSafe(OMX_BUFFERHEADERTYPE *outHeader);

    DISALLOW_EVIL_CONSTRUCTORS(SoftVPX);
};

}  // namespace android

#endif  // SOFT_VPX_H_
