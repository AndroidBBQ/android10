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

#ifndef CODEC2_HIDL_V1_0_UTILS_COMPONENT_INTERFACE_H
#define CODEC2_HIDL_V1_0_UTILS_COMPONENT_INTERFACE_H

#include <codec2/hidl/1.0/Configurable.h>
#include <codec2/hidl/1.0/types.h>

#include <android/hardware/media/c2/1.0/IComponentInterface.h>
#include <hidl/Status.h>

#include <C2Component.h>
#include <C2Buffer.h>
#include <C2.h>

#include <memory>

namespace android {
namespace hardware {
namespace media {
namespace c2 {
namespace V1_0 {
namespace utils {

using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct ComponentStore;

struct ComponentInterface : public IComponentInterface {
    ComponentInterface(
            const std::shared_ptr<C2ComponentInterface>& interface,
            ComponentStore* store);
    c2_status_t status() const;
    virtual Return<sp<IConfigurable>> getConfigurable() override;

protected:
    std::shared_ptr<C2ComponentInterface> mInterface;
    sp<CachedConfigurable> mConfigurable;
    c2_status_t mInit;
};


}  // namespace utils
}  // namespace V1_0
}  // namespace c2
}  // namespace media
}  // namespace hardware
}  // namespace android

#endif  // CODEC2_HIDL_V1_0_UTILS_COMPONENT_INTERFACE_H
