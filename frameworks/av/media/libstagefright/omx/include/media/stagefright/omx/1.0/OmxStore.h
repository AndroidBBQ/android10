/*
 * Copyright 2017, The Android Open Source Project
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

#ifndef ANDROID_HARDWARE_MEDIA_OMX_V1_0_OMXSTORE_H
#define ANDROID_HARDWARE_MEDIA_OMX_V1_0_OMXSTORE_H

#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include <android/hardware/media/omx/1.0/IOmx.h>
#include <android/hardware/media/omx/1.0/IOmxStore.h>
#include <media/stagefright/xmlparser/MediaCodecsXmlParser.h>

namespace android {
namespace hardware {
namespace media {
namespace omx {
namespace V1_0 {
namespace implementation {

using ::android::hardware::media::omx::V1_0::IOmxStore;
using ::android::hardware::media::omx::V1_0::IOmx;
using ::android::hardware::media::omx::V1_0::Status;
using ::android::hidl::base::V1_0::IBase;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::android::wp;

struct OmxStore : public IOmxStore {
    OmxStore(
            const sp<IOmx> &omx = nullptr,
            const char* owner = "default",
            const std::vector<std::string> &searchDirs =
                MediaCodecsXmlParser::getDefaultSearchDirs(),
            const std::vector<std::string> &xmlFiles =
                MediaCodecsXmlParser::getDefaultXmlNames(),
            const char *xmlProfilingResultsPath =
                MediaCodecsXmlParser::defaultProfilingResultsXmlPath);

    virtual ~OmxStore();

    // Methods from IOmxStore
    Return<void> listServiceAttributes(listServiceAttributes_cb) override;
    Return<void> getNodePrefix(getNodePrefix_cb) override;
    Return<void> listRoles(listRoles_cb) override;
    Return<sp<IOmx>> getOmx(hidl_string const&) override;

protected:
    Status mParsingStatus;
    hidl_string mPrefix;
    hidl_vec<ServiceAttribute> mServiceAttributeList;
    hidl_vec<RoleInfo> mRoleList;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace omx
}  // namespace media
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_MEDIA_OMX_V1_0_OMXSTORE_H
