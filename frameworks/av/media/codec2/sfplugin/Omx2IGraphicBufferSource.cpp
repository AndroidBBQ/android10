/*
 * Copyright 2019 The Android Open Source Project
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

#ifdef __LP64__
#define OMX_ANDROID_COMPILE_AS_32BIT_ON_64BIT_PLATFORMS
#endif

//#define LOG_NDEBUG 0
#define LOG_TAG "Omx2IGraphicBufferSource"
#include <android-base/logging.h>

#include "Omx2IGraphicBufferSource.h"

#include <android/BnOMXBufferSource.h>
#include <media/OMXBuffer.h>
#include <media/stagefright/omx/OMXUtils.h>

#include <OMX_Component.h>
#include <OMX_Index.h>
#include <OMX_IndexExt.h>

namespace android {

namespace /* unnamed */ {

// OmxGraphicBufferSource -> IOMXBufferSource

struct OmxGbs2IOmxBs : public BnOMXBufferSource {
    sp<OmxGraphicBufferSource> mBase;
    OmxGbs2IOmxBs(sp<OmxGraphicBufferSource> const& base) : mBase{base} {}
    BnStatus onOmxExecuting() override {
        return mBase->onOmxExecuting();
    }
    BnStatus onOmxIdle() override {
        return mBase->onOmxIdle();
    }
    BnStatus onOmxLoaded() override {
        return mBase->onOmxLoaded();
    }
    BnStatus onInputBufferAdded(int32_t bufferId) override {
        return mBase->onInputBufferAdded(bufferId);
    }
    BnStatus onInputBufferEmptied(
            int32_t bufferId,
            OMXFenceParcelable const& fenceParcel) override {
        return mBase->onInputBufferEmptied(bufferId, fenceParcel.get());
    }
};

struct OmxNodeWrapper : public IOmxNodeWrapper {
    sp<IOMXNode> mBase;
    OmxNodeWrapper(sp<IOMXNode> const& base) : mBase{base} {}
    status_t emptyBuffer(
            int32_t bufferId, uint32_t flags,
            const sp<GraphicBuffer> &buffer,
            int64_t timestamp, int fenceFd) override {
        return mBase->emptyBuffer(bufferId, buffer, flags, timestamp, fenceFd);
    }
    void dispatchDataSpaceChanged(
            int32_t dataSpace, int32_t aspects, int32_t pixelFormat) override {
        omx_message msg{};
        msg.type = omx_message::EVENT;
        msg.fenceFd = -1;
        msg.u.event_data.event = OMX_EventDataSpaceChanged;
        msg.u.event_data.data1 = dataSpace;
        msg.u.event_data.data2 = aspects;
        msg.u.event_data.data3 = pixelFormat;
        mBase->dispatchMessage(msg);
    }
};

} // unnamed namespace

// Omx2IGraphicBufferSource
Omx2IGraphicBufferSource::Omx2IGraphicBufferSource(
        sp<OmxGraphicBufferSource> const& base)
      : mBase{base},
        mOMXBufferSource{new OmxGbs2IOmxBs(base)} {
}

BnStatus Omx2IGraphicBufferSource::setSuspend(
        bool suspend, int64_t timeUs) {
    return BnStatus::fromStatusT(mBase->setSuspend(suspend, timeUs));
}

BnStatus Omx2IGraphicBufferSource::setRepeatPreviousFrameDelayUs(
        int64_t repeatAfterUs) {
    return BnStatus::fromStatusT(mBase->setRepeatPreviousFrameDelayUs(repeatAfterUs));
}

BnStatus Omx2IGraphicBufferSource::setMaxFps(float maxFps) {
    return BnStatus::fromStatusT(mBase->setMaxFps(maxFps));
}

BnStatus Omx2IGraphicBufferSource::setTimeLapseConfig(
        double fps, double captureFps) {
    return BnStatus::fromStatusT(mBase->setTimeLapseConfig(fps, captureFps));
}

BnStatus Omx2IGraphicBufferSource::setStartTimeUs(
        int64_t startTimeUs) {
    return BnStatus::fromStatusT(mBase->setStartTimeUs(startTimeUs));
}

BnStatus Omx2IGraphicBufferSource::setStopTimeUs(
        int64_t stopTimeUs) {
    return BnStatus::fromStatusT(mBase->setStopTimeUs(stopTimeUs));
}

BnStatus Omx2IGraphicBufferSource::getStopTimeOffsetUs(
        int64_t *stopTimeOffsetUs) {
    return BnStatus::fromStatusT(mBase->getStopTimeOffsetUs(stopTimeOffsetUs));
}

BnStatus Omx2IGraphicBufferSource::setColorAspects(
        int32_t aspects) {
    return BnStatus::fromStatusT(mBase->setColorAspects(aspects));
}

BnStatus Omx2IGraphicBufferSource::setTimeOffsetUs(
        int64_t timeOffsetsUs) {
    return BnStatus::fromStatusT(mBase->setTimeOffsetUs(timeOffsetsUs));
}

BnStatus Omx2IGraphicBufferSource::signalEndOfInputStream() {
    return BnStatus::fromStatusT(mBase->signalEndOfInputStream());
}

BnStatus Omx2IGraphicBufferSource::configure(
        const sp<IOMXNode>& omxNode, int32_t dataSpace) {
    if (omxNode == NULL) {
        return BnStatus::fromServiceSpecificError(BAD_VALUE);
    }

    // Do setInputSurface() first, the node will try to enable metadata
    // mode on input, and does necessary error checking. If this fails,
    // we can't use this input surface on the node.
    status_t err = omxNode->setInputSurface(mOMXBufferSource);
    if (err != NO_ERROR) {
        ALOGE("Unable to set input surface: %d", err);
        return BnStatus::fromServiceSpecificError(err);
    }

    uint32_t consumerUsage;
    if (omxNode->getParameter(
            (OMX_INDEXTYPE)OMX_IndexParamConsumerUsageBits,
            &consumerUsage, sizeof(consumerUsage)) != OK) {
        consumerUsage = 0;
    }

    OMX_PARAM_PORTDEFINITIONTYPE def;
    InitOMXParams(&def);
    def.nPortIndex = 0; // kPortIndexInput

    err = omxNode->getParameter(
            OMX_IndexParamPortDefinition, &def, sizeof(def));
    if (err != NO_ERROR) {
        ALOGE("Failed to get port definition: %d", err);
        return BnStatus::fromServiceSpecificError(UNKNOWN_ERROR);
    }

    return BnStatus::fromStatusT(mBase->configure(
            new OmxNodeWrapper(omxNode),
            dataSpace,
            def.nBufferCountActual,
            def.format.video.nFrameWidth,
            def.format.video.nFrameHeight,
            consumerUsage));
}

} // namespace android

