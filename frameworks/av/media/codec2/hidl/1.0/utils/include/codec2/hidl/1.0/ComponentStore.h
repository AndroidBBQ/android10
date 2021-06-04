/*
 * Copyright 2018 The Android Open Source Project
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

#ifndef CODEC2_HIDL_V1_0_UTILS_COMPONENTSTORE_H
#define CODEC2_HIDL_V1_0_UTILS_COMPONENTSTORE_H

#include <codec2/hidl/1.0/Component.h>
#include <codec2/hidl/1.0/ComponentInterface.h>
#include <codec2/hidl/1.0/Configurable.h>

#include <android/hardware/media/bufferpool/2.0/IClientManager.h>
#include <android/hardware/media/c2/1.0/IComponentStore.h>
#include <hidl/Status.h>

#include <C2Component.h>
#include <C2Param.h>
#include <C2.h>

#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <vector>

namespace android {
namespace hardware {
namespace media {
namespace c2 {
namespace V1_0 {
namespace utils {

using ::android::hardware::media::bufferpool::V2_0::IClientManager;

using ::android::hardware::hidl_handle;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct ComponentStore : public IComponentStore {
    ComponentStore(const std::shared_ptr<C2ComponentStore>& store);
    virtual ~ComponentStore() = default;

    /**
     * Returns the status of the construction of this object.
     */
    c2_status_t status() const;

    /**
     * This function is called by CachedConfigurable::init() to validate
     * supported parameters.
     */
    c2_status_t validateSupportedParams(
            const std::vector<std::shared_ptr<C2ParamDescriptor>>& params);

    // Methods from ::android::hardware::media::c2::V1_0::IComponentStore.
    virtual Return<void> createComponent(
            const hidl_string& name,
            const sp<IComponentListener>& listener,
            const sp<IClientManager>& pool,
            createComponent_cb _hidl_cb) override;
    virtual Return<void> createInterface(
            const hidl_string& name,
            createInterface_cb _hidl_cb) override;
    virtual Return<void> listComponents(listComponents_cb _hidl_cb) override;
    virtual Return<void> createInputSurface(
            createInputSurface_cb _hidl_cb) override;
    virtual Return<void> getStructDescriptors(
            const hidl_vec<uint32_t>& indices,
            getStructDescriptors_cb _hidl_cb) override;
    virtual Return<sp<IClientManager>> getPoolClientManager() override;
    virtual Return<Status> copyBuffer(
            const Buffer& src,
            const Buffer& dst) override;
    virtual Return<sp<IConfigurable>> getConfigurable() override;

    /**
     * Dumps information when lshal is called.
     */
    virtual Return<void> debug(
            const hidl_handle& handle,
            const hidl_vec<hidl_string>& args) override;

protected:
    sp<CachedConfigurable> mConfigurable;

    // Does bookkeeping for an interface that has been loaded.
    void onInterfaceLoaded(const std::shared_ptr<C2ComponentInterface> &intf);

    c2_status_t mInit;
    std::shared_ptr<C2ComponentStore> mStore;
    std::shared_ptr<C2ParamReflector> mParamReflector;

    std::map<C2Param::CoreIndex, std::shared_ptr<C2StructDescriptor>> mStructDescriptors;
    std::set<C2Param::CoreIndex> mUnsupportedStructDescriptors;
    std::set<C2String> mLoadedInterfaces;
    mutable std::mutex mStructDescriptorsMutex;

    // ComponentStore keeps track of live Components.

    struct ComponentStatus {
        std::shared_ptr<C2Component> c2Component;
        std::chrono::system_clock::time_point birthTime;
    };

    mutable std::mutex mComponentRosterMutex;
    std::map<Component*, ComponentStatus> mComponentRoster;

    // Called whenever Component is created.
    void reportComponentBirth(Component* component);
    // Called only from the destructor of Component.
    void reportComponentDeath(Component* component);

    friend Component;

    // Helper functions for dumping.

    std::ostream& dump(
            std::ostream& out,
            const std::shared_ptr<const C2Component::Traits>& comp);

    std::ostream& dump(
            std::ostream& out,
            ComponentStatus& compStatus);

};

}  // namespace utils
}  // namespace V1_0
}  // namespace c2
}  // namespace media
}  // namespace hardware
}  // namespace android

#endif  // CODEC2_HIDL_V1_0_UTILS_COMPONENTSTORE_H
