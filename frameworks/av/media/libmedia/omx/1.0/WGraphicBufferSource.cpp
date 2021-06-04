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

#include <media/omx/1.0/WGraphicBufferSource.h>
#include <media/omx/1.0/WOmxNode.h>
#include <media/omx/1.0/Conversion.h>

namespace android {
namespace hardware {
namespace media {
namespace omx {
namespace V1_0 {
namespace utils {

// LWGraphicBufferSource
LWGraphicBufferSource::LWGraphicBufferSource(
        sp<TGraphicBufferSource> const& base) : mBase(base) {
}

BnStatus LWGraphicBufferSource::configure(
        const sp<IOMXNode>& omxNode, int32_t dataSpace) {
    sp<IOmxNode> hOmxNode = omxNode->getHalInterface<IOmxNode>();
    return toBinderStatus(mBase->configure(
            hOmxNode == nullptr ? new TWOmxNode(omxNode) : hOmxNode,
            toHardwareDataspace(dataSpace)));
}

BnStatus LWGraphicBufferSource::setSuspend(
        bool suspend, int64_t timeUs) {
    return toBinderStatus(mBase->setSuspend(suspend, timeUs));
}

BnStatus LWGraphicBufferSource::setRepeatPreviousFrameDelayUs(
        int64_t repeatAfterUs) {
    return toBinderStatus(mBase->setRepeatPreviousFrameDelayUs(repeatAfterUs));
}

BnStatus LWGraphicBufferSource::setMaxFps(float maxFps) {
    return toBinderStatus(mBase->setMaxFps(maxFps));
}

BnStatus LWGraphicBufferSource::setTimeLapseConfig(
        double fps, double captureFps) {
    return toBinderStatus(mBase->setTimeLapseConfig(fps, captureFps));
}

BnStatus LWGraphicBufferSource::setStartTimeUs(
        int64_t startTimeUs) {
    return toBinderStatus(mBase->setStartTimeUs(startTimeUs));
}

BnStatus LWGraphicBufferSource::setStopTimeUs(
        int64_t stopTimeUs) {
    return toBinderStatus(mBase->setStopTimeUs(stopTimeUs));
}

BnStatus LWGraphicBufferSource::getStopTimeOffsetUs(
        int64_t *stopTimeOffsetUs) {
    return toBinderStatus(mBase->getStopTimeOffsetUs(
            [stopTimeOffsetUs](auto, auto offsetUs) {
                *stopTimeOffsetUs = offsetUs;
            }));
}

BnStatus LWGraphicBufferSource::setColorAspects(
        int32_t aspects) {
    return toBinderStatus(mBase->setColorAspects(
            toHardwareColorAspects(aspects)));
}

BnStatus LWGraphicBufferSource::setTimeOffsetUs(
        int64_t timeOffsetsUs) {
    return toBinderStatus(mBase->setTimeOffsetUs(timeOffsetsUs));
}

BnStatus LWGraphicBufferSource::signalEndOfInputStream() {
    return toBinderStatus(mBase->signalEndOfInputStream());
}

}  // namespace utils
}  // namespace V1_0
}  // namespace omx
}  // namespace media
}  // namespace hardware
}  // namespace android
