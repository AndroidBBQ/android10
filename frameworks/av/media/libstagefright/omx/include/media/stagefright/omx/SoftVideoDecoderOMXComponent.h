/*
 * Copyright (C) 2013 The Android Open Source Project
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

#ifndef SOFT_VIDEO_DECODER_OMX_COMPONENT_H_

#define SOFT_VIDEO_DECODER_OMX_COMPONENT_H_

#include "SimpleSoftOMXComponent.h"

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/AHandlerReflector.h>
#include <media/stagefright/foundation/ColorUtils.h>
#include <media/openmax/OMX_Core.h>
#include <media/openmax/OMX_Video.h>
#include <media/openmax/OMX_VideoExt.h>

#include <media/hardware/HardwareAPI.h>

#include <utils/RefBase.h>
#include <utils/threads.h>
#include <utils/Vector.h>
#include <utils/List.h>

namespace android {

struct SoftVideoDecoderOMXComponent : public SimpleSoftOMXComponent {
    SoftVideoDecoderOMXComponent(
            const char *name,
            const char *componentRole,
            OMX_VIDEO_CODINGTYPE codingType,
            const CodecProfileLevel *profileLevels,
            size_t numProfileLevels,
            int32_t width,
            int32_t height,
            const OMX_CALLBACKTYPE *callbacks,
            OMX_PTR appData,
            OMX_COMPONENTTYPE **component);

protected:
    enum {
        kDescribeColorAspectsIndex = kPrepareForAdaptivePlaybackIndex + 1,
        kDescribeHdrStaticInfoIndex = kPrepareForAdaptivePlaybackIndex + 2,
        kDescribeHdr10PlusInfoIndex = kPrepareForAdaptivePlaybackIndex + 3,
    };

    enum {
        kNotSupported,
        kPreferBitstream,
        kPreferContainer,
    };

    virtual void onPortEnableCompleted(OMX_U32 portIndex, bool enabled);
    virtual void onReset();

    virtual OMX_ERRORTYPE internalGetParameter(
            OMX_INDEXTYPE index, OMX_PTR params);

    virtual OMX_ERRORTYPE internalSetParameter(
            OMX_INDEXTYPE index, const OMX_PTR params);

    virtual OMX_ERRORTYPE getConfig(
            OMX_INDEXTYPE index, OMX_PTR params);

    virtual OMX_ERRORTYPE internalSetConfig(
            OMX_INDEXTYPE index, const OMX_PTR params, bool *frameConfig);

    virtual OMX_ERRORTYPE getExtensionIndex(
            const char *name, OMX_INDEXTYPE *index);

    virtual bool supportsDescribeColorAspects();

    virtual int getColorAspectPreference();

    virtual bool supportDescribeHdrStaticInfo();

    virtual bool supportDescribeHdr10PlusInfo();

    // This function sets both minimum buffer count and actual buffer count of
    // input port to be |numInputBuffers|. It will also set both minimum buffer
    // count and actual buffer count of output port to be |numOutputBuffers|.
    void initPorts(OMX_U32 numInputBuffers,
            OMX_U32 inputBufferSize,
            OMX_U32 numOutputBuffers,
            const char *mimeType,
            OMX_U32 minCompressionRatio = 1u);

    // This function sets input port's minimum buffer count to |numMinInputBuffers|,
    // sets input port's actual buffer count to |numInputBuffers|, sets output port's
    // minimum buffer count to |numMinOutputBuffers| and sets output port's actual buffer
    // count to be |numOutputBuffers|.
    void initPorts(OMX_U32 numMinInputBuffers,
            OMX_U32 numInputBuffers,
            OMX_U32 inputBufferSize,
            OMX_U32 numMinOutputBuffers,
            OMX_U32 numOutputBuffers,
            const char *mimeType,
            OMX_U32 minCompressionRatio = 1u);

    virtual void updatePortDefinitions(bool updateCrop = true, bool updateInputSize = false);

    uint32_t outputBufferWidth();
    uint32_t outputBufferHeight();

    enum CropSettingsMode {
        kCropUnSet = 0,
        kCropSet,
        kCropChanged,
    };

    // This function will handle several port change events which include
    // size changed, crop changed, stride changed and coloraspects changed.
    // It will trigger OMX_EventPortSettingsChanged event if necessary.
    void handlePortSettingsChange(
            bool *portWillReset, uint32_t width, uint32_t height,
            OMX_COLOR_FORMATTYPE outputFormat = OMX_COLOR_FormatYUV420Planar,
            CropSettingsMode cropSettingsMode = kCropUnSet,
            bool fakeStride = false);

    void copyYV12FrameToOutputBuffer(
            uint8_t *dst, const uint8_t *srcY, const uint8_t *srcU, const uint8_t *srcV,
            size_t srcYStride, size_t srcUStride, size_t srcVStride);

    enum {
        kInputPortIndex  = 0,
        kOutputPortIndex = 1,
        kMaxPortIndex = 1,
    };

    bool mIsAdaptive;
    uint32_t mAdaptiveMaxWidth, mAdaptiveMaxHeight;
    uint32_t mWidth, mHeight;
    uint32_t mCropLeft, mCropTop, mCropWidth, mCropHeight;
    OMX_COLOR_FORMATTYPE mOutputFormat;
    HDRStaticInfo mHdrStaticInfo;
    enum {
        NONE,
        AWAITING_DISABLED,
        AWAITING_ENABLED
    } mOutputPortSettingsChange;

    bool mUpdateColorAspects;

    Mutex mColorAspectsLock;
    // color aspects passed from the framework.
    ColorAspects mDefaultColorAspects;
    // color aspects parsed from the bitstream.
    ColorAspects mBitstreamColorAspects;
    // final color aspects after combining the above two aspects.
    ColorAspects mFinalColorAspects;

    bool colorAspectsDiffer(const ColorAspects &a, const ColorAspects &b);

    // This functions takes two color aspects and updates the mFinalColorAspects
    // based on |preferredAspects|.
    void updateFinalColorAspects(
            const ColorAspects &otherAspects, const ColorAspects &preferredAspects);

    // This function will update the mFinalColorAspects based on codec preference.
    status_t handleColorAspectsChange();

    // Helper function to dump the ColorAspects.
    void dumpColorAspects(const ColorAspects &colorAspects);

    sp<ABuffer> dequeueInputFrameConfig();
    void queueOutputFrameConfig(const sp<ABuffer> &info);

private:
    uint32_t mMinInputBufferSize;
    uint32_t mMinCompressionRatio;

    const char *mComponentRole;
    OMX_VIDEO_CODINGTYPE mCodingType;
    const CodecProfileLevel *mProfileLevels;
    size_t mNumProfileLevels;
    typedef List<sp<ABuffer> > Hdr10PlusInfoList;
    Hdr10PlusInfoList mHdr10PlusInputs;
    Hdr10PlusInfoList mHdr10PlusOutputs;

    DISALLOW_EVIL_CONSTRUCTORS(SoftVideoDecoderOMXComponent);
};

}  // namespace android

#endif  // SOFT_VIDEO_DECODER_OMX_COMPONENT_H_
