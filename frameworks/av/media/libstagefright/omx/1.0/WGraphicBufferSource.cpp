/*
 * Copyright 2016, The Android Open Source Project
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
#define LOG_TAG "TWGraphicBufferSource"

#include <media/stagefright/omx/1.0/WGraphicBufferSource.h>
#include <media/stagefright/omx/1.0/WOmxNode.h>
#include <media/stagefright/omx/1.0/Conversion.h>
#include <media/stagefright/omx/OMXUtils.h>
#include <android/hardware/media/omx/1.0/IOmxBufferSource.h>
#include <android/hardware/media/omx/1.0/IOmxNode.h>
#include <media/openmax/OMX_Component.h>
#include <media/openmax/OMX_IndexExt.h>

namespace android {
namespace hardware {
namespace media {
namespace omx {
namespace V1_0 {
namespace implementation {

static const OMX_U32 kPortIndexInput = 0;

struct TWGraphicBufferSource::TWOmxNodeWrapper : public IOmxNodeWrapper {
    sp<IOmxNode> mOmxNode;

    TWOmxNodeWrapper(const sp<IOmxNode> &omxNode): mOmxNode(omxNode) {
    }

    virtual status_t emptyBuffer(
            int32_t bufferId, uint32_t flags,
            const sp<GraphicBuffer> &buffer,
            int64_t timestamp, int fenceFd) override {
        CodecBuffer tBuffer;
        native_handle_t* fenceNh = native_handle_create_from_fd(fenceFd);
        status_t err = toStatusT(mOmxNode->emptyBuffer(
              bufferId,
              *wrapAs(&tBuffer, buffer),
              flags,
              toRawTicks(timestamp),
              fenceNh));
        native_handle_close(fenceNh);
        native_handle_delete(fenceNh);
        return err;
    }

    virtual void dispatchDataSpaceChanged(
            int32_t dataSpace, int32_t aspects, int32_t pixelFormat) override {
        Message tMsg;
        tMsg.type = Message::Type::EVENT;
        tMsg.fence = native_handle_create(0, 0);
        tMsg.data.eventData.event = uint32_t(OMX_EventDataSpaceChanged);
        tMsg.data.eventData.data1 = dataSpace;
        tMsg.data.eventData.data2 = aspects;
        tMsg.data.eventData.data3 = pixelFormat;
        if (!mOmxNode->dispatchMessage(tMsg).isOk()) {
            ALOGE("TWOmxNodeWrapper failed to dispatch message "
                    "OMX_EventDataSpaceChanged: "
                    "dataSpace = %ld, aspects = %ld, pixelFormat = %ld",
                    static_cast<long>(dataSpace),
                    static_cast<long>(aspects),
                    static_cast<long>(pixelFormat));
        }
    }
};

struct TWGraphicBufferSource::TWOmxBufferSource : public IOmxBufferSource {
    sp<OmxGraphicBufferSource> mSource;

    TWOmxBufferSource(const sp<OmxGraphicBufferSource> &source): mSource(source) {
    }

    Return<void> onOmxExecuting() override {
        mSource->onOmxExecuting();
        return Void();
    }

    Return<void> onOmxIdle() override {
        mSource->onOmxIdle();
        return Void();
    }

    Return<void> onOmxLoaded() override {
        mSource->onOmxLoaded();
        return Void();
    }

    Return<void> onInputBufferAdded(uint32_t bufferId) override {
        mSource->onInputBufferAdded(static_cast<int32_t>(bufferId));
        return Void();
    }

    Return<void> onInputBufferEmptied(
            uint32_t bufferId, hidl_handle const& tFence) override {
        mSource->onInputBufferEmptied(
                static_cast<int32_t>(bufferId),
                native_handle_read_fd(tFence));
        return Void();
    }
};

// TWGraphicBufferSource
TWGraphicBufferSource::TWGraphicBufferSource(
        sp<OmxGraphicBufferSource> const& base) :
    mBase(base),
    mOmxBufferSource(new TWOmxBufferSource(base)) {
}

Return<Status> TWGraphicBufferSource::configure(
        const sp<IOmxNode>& omxNode, Dataspace dataspace) {
    if (omxNode == NULL) {
        return toStatus(BAD_VALUE);
    }

    // Do setInputSurface() first, the node will try to enable metadata
    // mode on input, and does necessary error checking. If this fails,
    // we can't use this input surface on the node.
    Return<Status> err(omxNode->setInputSurface(mOmxBufferSource));
    status_t fnStatus = toStatusT(err);
    if (fnStatus != NO_ERROR) {
        ALOGE("Unable to set input surface: %d", fnStatus);
        return err;
    }

    // use consumer usage bits queried from encoder, but always add
    // HW_VIDEO_ENCODER for backward compatibility.
    uint32_t  consumerUsage;
    void *_params = &consumerUsage;
    uint8_t *params = static_cast<uint8_t*>(_params);
    fnStatus = UNKNOWN_ERROR;
    IOmxNode::getParameter_cb _hidl_cb(
            [&fnStatus, &params](Status status, hidl_vec<uint8_t> const& outParams) {
                fnStatus = toStatusT(status);
                std::copy(
                        outParams.data(),
                        outParams.data() + outParams.size(),
                        params);
            });
    auto transStatus = omxNode->getParameter(
            static_cast<uint32_t>(OMX_IndexParamConsumerUsageBits),
            inHidlBytes(&consumerUsage, sizeof(consumerUsage)),
            _hidl_cb);
    if (!transStatus.isOk()) {
        return toStatus(FAILED_TRANSACTION);
    }
    if (fnStatus != OK) {
        consumerUsage = 0;
    }

    OMX_PARAM_PORTDEFINITIONTYPE def;
    InitOMXParams(&def);
    def.nPortIndex = kPortIndexInput;

    _params = &def;
    params = static_cast<uint8_t*>(_params);
    transStatus = omxNode->getParameter(
            static_cast<uint32_t>(OMX_IndexParamPortDefinition),
            inHidlBytes(&def, sizeof(def)),
            _hidl_cb);
    if (!transStatus.isOk()) {
        return toStatus(FAILED_TRANSACTION);
    }
    if (fnStatus != NO_ERROR) {
        ALOGE("Failed to get port definition: %d", fnStatus);
        return toStatus(fnStatus);
    }


    return toStatus(mBase->configure(
            new TWOmxNodeWrapper(omxNode),
            toRawDataspace(dataspace),
            def.nBufferCountActual,
            def.format.video.nFrameWidth,
            def.format.video.nFrameHeight,
            consumerUsage));
}

Return<Status> TWGraphicBufferSource::setSuspend(
        bool suspend, int64_t timeUs) {
    return toStatus(mBase->setSuspend(suspend, timeUs));
}

Return<Status> TWGraphicBufferSource::setRepeatPreviousFrameDelayUs(
        int64_t repeatAfterUs) {
    return toStatus(mBase->setRepeatPreviousFrameDelayUs(repeatAfterUs));
}

Return<Status> TWGraphicBufferSource::setMaxFps(float maxFps) {
    return toStatus(mBase->setMaxFps(maxFps));
}

Return<Status> TWGraphicBufferSource::setTimeLapseConfig(
        double fps, double captureFps) {
    return toStatus(mBase->setTimeLapseConfig(fps, captureFps));
}

Return<Status> TWGraphicBufferSource::setStartTimeUs(int64_t startTimeUs) {
    return toStatus(mBase->setStartTimeUs(startTimeUs));
}

Return<Status> TWGraphicBufferSource::setStopTimeUs(int64_t stopTimeUs) {
    return toStatus(mBase->setStopTimeUs(stopTimeUs));
}

Return<void> TWGraphicBufferSource::getStopTimeOffsetUs(
        getStopTimeOffsetUs_cb _hidl_cb) {
    status_t status;
    int64_t stopTimeOffsetUs;
    status = mBase->getStopTimeOffsetUs(&stopTimeOffsetUs);
    _hidl_cb(toStatus(status), stopTimeOffsetUs);
    return Void();
}

Return<Status> TWGraphicBufferSource::setColorAspects(
        const ColorAspects& aspects) {
    return toStatus(mBase->setColorAspects(toCompactColorAspects(aspects)));
}

Return<Status> TWGraphicBufferSource::setTimeOffsetUs(int64_t timeOffsetUs) {
    return toStatus(mBase->setTimeOffsetUs(timeOffsetUs));
}

Return<Status> TWGraphicBufferSource::signalEndOfInputStream() {
    return toStatus(mBase->signalEndOfInputStream());
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace omx
}  // namespace media
}  // namespace hardware
}  // namespace android
