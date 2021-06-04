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

#ifndef ANDROID_HARDWARE_MEDIA_OMX_V1_0_OMX_H
#define ANDROID_HARDWARE_MEDIA_OMX_V1_0_OMX_H

#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include <media/stagefright/xmlparser/MediaCodecsXmlParser.h>
#include <android/hardware/media/omx/1.0/IOmx.h>
#include <utils/KeyedVector.h>
#include <utils/Mutex.h>

namespace android {

struct OMXMaster;
struct OMXNodeInstance;

namespace hardware {
namespace media {
namespace omx {
namespace V1_0 {
namespace implementation {

using ::android::hardware::media::omx::V1_0::IOmx;
using ::android::hardware::media::omx::V1_0::IOmxNode;
using ::android::hardware::media::omx::V1_0::IOmxObserver;
using ::android::hardware::media::omx::V1_0::Status;
using ::android::hidl::base::V1_0::IBase;
using ::android::hardware::hidl_death_recipient;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::android::wp;

using ::android::OMXMaster;
using ::android::OMXNodeInstance;

struct Omx : public IOmx, public hidl_death_recipient {
    Omx();
    virtual ~Omx();

    // Methods from IOmx
    Return<void> listNodes(listNodes_cb _hidl_cb) override;
    Return<void> allocateNode(
            const hidl_string& name,
            const sp<IOmxObserver>& observer,
            allocateNode_cb _hidl_cb) override;
    Return<void> createInputSurface(createInputSurface_cb _hidl_cb) override;

    // Method from hidl_death_recipient
    void serviceDied(uint64_t cookie, const wp<IBase>& who) override;

    // Method for OMXNodeInstance
    status_t freeNode(sp<OMXNodeInstance> const& instance);

protected:
    OMXMaster* mMaster;
    Mutex mLock;
    KeyedVector<wp<IBase>, sp<OMXNodeInstance> > mLiveNodes;
    KeyedVector<OMXNodeInstance*, wp<IBase> > mNode2Observer;
    MediaCodecsXmlParser mParser;
};

extern "C" IOmx* HIDL_FETCH_IOmx(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace omx
}  // namespace media
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_MEDIA_OMX_V1_0_OMX_H
