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

#ifndef CODEC2_HIDL_V1_0_UTILS_CONFIGURABLE_H
#define CODEC2_HIDL_V1_0_UTILS_CONFIGURABLE_H

#include <android/hardware/media/c2/1.0/IConfigurable.h>
#include <hidl/Status.h>

#include <C2Component.h>
#include <C2Param.h>
#include <C2.h>

#include <memory>

namespace android {
namespace hardware {
namespace media {
namespace c2 {
namespace V1_0 {
namespace utils {

using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct ComponentStore;

/**
 * Codec2 objects of different types may have different querying and configuring
 * functions, but across the Treble boundary, they share the same HIDL
 * interface, IConfigurable.
 *
 * ConfigurableC2Intf is an abstract class that a Codec2 object can implement to
 * easily expose an IConfigurable instance. See CachedConfigurable below.
 */
struct ConfigurableC2Intf {
    C2String getName() const { return mName; }
    uint32_t getId() const { return mId; }
    /** C2ComponentInterface::query_vb sans stack params */
    virtual c2_status_t query(
            const std::vector<C2Param::Index> &indices,
            c2_blocking_t mayBlock,
            std::vector<std::unique_ptr<C2Param>>* const params) const = 0;
    /** C2ComponentInterface::config_vb */
    virtual c2_status_t config(
            const std::vector<C2Param*> &params,
            c2_blocking_t mayBlock,
            std::vector<std::unique_ptr<C2SettingResult>>* const failures) = 0;
    /** C2ComponentInterface::querySupportedParams_nb */
    virtual c2_status_t querySupportedParams(
            std::vector<std::shared_ptr<C2ParamDescriptor>>* const params) const = 0;
    /** C2ComponentInterface::querySupportedParams_nb */
    virtual c2_status_t querySupportedValues(
            std::vector<C2FieldSupportedValuesQuery>& fields, c2_blocking_t mayBlock) const = 0;

    virtual ~ConfigurableC2Intf() = default;

    ConfigurableC2Intf(const C2String& name, uint32_t id)
          : mName{name}, mId{id} {}

protected:
    C2String mName; /* cached component name */
    uint32_t mId;
};

/**
 * Implementation of the IConfigurable interface that supports caching of
 * supported parameters from a supplied ComponentStore.
 *
 * CachedConfigurable essentially converts a ConfigurableC2Intf into HIDL's
 * IConfigurable. A Codec2 object generally implements ConfigurableC2Intf and
 * passes the implementation to the constructor of CachedConfigurable.
 *
 * Note that caching happens
 */
struct CachedConfigurable : public IConfigurable {
    CachedConfigurable(std::unique_ptr<ConfigurableC2Intf>&& intf);

    c2_status_t init(ComponentStore* store);

    // Methods from ::android::hardware::media::c2::V1_0::IConfigurable

    virtual Return<uint32_t> getId() override;

    virtual Return<void> getName(getName_cb _hidl_cb) override;

    virtual Return<void> query(
            const hidl_vec<uint32_t>& indices,
            bool mayBlock,
            query_cb _hidl_cb) override;

    virtual Return<void> config(
            const hidl_vec<uint8_t>& inParams,
            bool mayBlock,
            config_cb _hidl_cb) override;

    virtual Return<void> querySupportedParams(
            uint32_t start,
            uint32_t count,
            querySupportedParams_cb _hidl_cb) override;

    virtual Return<void> querySupportedValues(
            const hidl_vec<FieldSupportedValuesQuery>& inFields,
            bool mayBlock,
            querySupportedValues_cb _hidl_cb) override;

protected:
    // Common Codec2.0 interface wrapper
    std::unique_ptr<ConfigurableC2Intf> mIntf;

    // Cached supported params
    std::vector<std::shared_ptr<C2ParamDescriptor>> mSupportedParams;
};

}  // namespace utils
}  // namespace V1_0
}  // namespace c2
}  // namespace media
}  // namespace hardware
}  // namespace android

#endif  // CODEC2_HIDL_V1_0_UTILS_CONFIGURABLE_H

