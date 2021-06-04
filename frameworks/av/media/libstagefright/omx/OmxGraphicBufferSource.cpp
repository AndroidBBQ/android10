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

#include <inttypes.h>

#define LOG_TAG "OmxGraphicBufferSource"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <media/stagefright/bqhelper/ComponentWrapper.h>
#include <media/stagefright/bqhelper/GraphicBufferSource.h>
#include <media/stagefright/omx/OmxGraphicBufferSource.h>

namespace android {

namespace {

class OmxComponentWrapper : public ComponentWrapper {
public:
    explicit OmxComponentWrapper(const sp<IOmxNodeWrapper> &node)
        : mOmxNode(node) {}
    virtual ~OmxComponentWrapper() = default;

    status_t submitBuffer(
            int32_t bufferId, const sp<GraphicBuffer> &buffer,
            int64_t timestamp, int fenceFd) override {
        return mOmxNode->emptyBuffer(
                bufferId, OMX_BUFFERFLAG_ENDOFFRAME, buffer, timestamp, fenceFd);
    }

    status_t submitEos(int32_t bufferId) override {
        return mOmxNode->emptyBuffer(bufferId, OMX_BUFFERFLAG_ENDOFFRAME | OMX_BUFFERFLAG_EOS);
    }

    void dispatchDataSpaceChanged(
            int32_t dataSpace, int32_t aspects, int32_t pixelFormat) override {
        mOmxNode->dispatchDataSpaceChanged(dataSpace, aspects, pixelFormat);
    }

private:
    sp<IOmxNodeWrapper> mOmxNode;

    DISALLOW_EVIL_CONSTRUCTORS(OmxComponentWrapper);
};

}  // namespace

Status OmxGraphicBufferSource::onOmxExecuting() {
    return start();
}

Status OmxGraphicBufferSource::onOmxIdle() {
    return stop();
}

Status OmxGraphicBufferSource::onOmxLoaded(){
    return release();
}

status_t OmxGraphicBufferSource::configure(
        const sp<IOmxNodeWrapper>& omxNode,
        int32_t dataSpace,
        int32_t bufferCount,
        uint32_t frameWidth,
        uint32_t frameHeight,
        uint32_t consumerUsage) {
    if (omxNode == NULL) {
        return BAD_VALUE;
    }

    return GraphicBufferSource::configure(
            new OmxComponentWrapper(omxNode), dataSpace, bufferCount,
            frameWidth, frameHeight, consumerUsage);
}

}  // namespace android
