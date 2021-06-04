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

#include <gui/bufferqueue/1.0/H2BGraphicBufferProducer.h>
#include <media/omx/1.0/WOmx.h>
#include <media/omx/1.0/WOmxNode.h>
#include <media/omx/1.0/WOmxObserver.h>
#include <media/omx/1.0/WGraphicBufferSource.h>
#include <media/omx/1.0/Conversion.h>

namespace android {
namespace hardware {
namespace media {
namespace omx {
namespace V1_0 {
namespace utils {

using ::android::hardware::graphics::bufferqueue::V1_0::utils::
        H2BGraphicBufferProducer;
typedef ::android::hardware::graphics::bufferqueue::V1_0::IGraphicBufferProducer
        HGraphicBufferProducer;

// LWOmx
LWOmx::LWOmx(sp<IOmx> const& base) : mBase(base) {
}

status_t LWOmx::listNodes(List<IOMX::ComponentInfo>* list) {
    status_t fnStatus;
    status_t transStatus = toStatusT(mBase->listNodes(
            [&fnStatus, list](
                    Status status,
                    hidl_vec<IOmx::ComponentInfo> const& nodeList) {
                fnStatus = toStatusT(status);
                list->clear();
                for (size_t i = 0; i < nodeList.size(); ++i) {
                    auto newInfo = list->insert(
                            list->end(), IOMX::ComponentInfo());
                    convertTo(&*newInfo, nodeList[i]);
                }
            }));
    return transStatus == NO_ERROR ? fnStatus : transStatus;
}

status_t LWOmx::allocateNode(
        char const* name,
        sp<IOMXObserver> const& observer,
        sp<IOMXNode>* omxNode) {
    status_t fnStatus;
    status_t transStatus = toStatusT(mBase->allocateNode(
            name, new TWOmxObserver(observer),
            [&fnStatus, omxNode](Status status, sp<IOmxNode> const& node) {
                fnStatus = toStatusT(status);
                *omxNode = new LWOmxNode(node);
            }));
    return transStatus == NO_ERROR ? fnStatus : transStatus;
}

status_t LWOmx::createInputSurface(
        sp<::android::IGraphicBufferProducer>* bufferProducer,
        sp<::android::IGraphicBufferSource>* bufferSource) {
    status_t fnStatus;
    status_t transStatus = toStatusT(mBase->createInputSurface(
            [&fnStatus, bufferProducer, bufferSource] (
                    Status status,
                    sp<HGraphicBufferProducer> const& tProducer,
                    sp<IGraphicBufferSource> const& tSource) {
                fnStatus = toStatusT(status);
                *bufferProducer = new H2BGraphicBufferProducer(tProducer);
                *bufferSource = new LWGraphicBufferSource(tSource);
            }));
    return transStatus == NO_ERROR ? fnStatus : transStatus;
}

}  // namespace utils
}  // namespace V1_0
}  // namespace omx
}  // namespace media
}  // namespace hardware
}  // namespace android
