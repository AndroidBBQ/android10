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

//#define LOG_NDEBUG 0
#define LOG_TAG "Codec2-Configurable"
#include <android-base/logging.h>

#include <codec2/hidl/1.0/Configurable.h>
#include <codec2/hidl/1.0/ComponentStore.h>
#include <codec2/hidl/1.0/types.h>

#include <C2ParamInternal.h>

namespace android {
namespace hardware {
namespace media {
namespace c2 {
namespace V1_0 {
namespace utils {

using namespace ::android;

CachedConfigurable::CachedConfigurable(
        std::unique_ptr<ConfigurableC2Intf>&& intf)
      : mIntf{std::move(intf)} {
}

c2_status_t CachedConfigurable::init(ComponentStore* store) {
    // Retrieve supported parameters from store
    c2_status_t init = mIntf->querySupportedParams(&mSupportedParams);
    c2_status_t validate = store->validateSupportedParams(mSupportedParams);
    return init == C2_OK ? C2_OK : validate;
}

// Methods from ::android::hardware::media::c2::V1_0::IConfigurable follow.
Return<uint32_t> CachedConfigurable::getId() {
    return mIntf->getId();
}

Return<void> CachedConfigurable::getName(getName_cb _hidl_cb) {
    _hidl_cb(mIntf->getName());
    return Void();
}

Return<void> CachedConfigurable::query(
        const hidl_vec<uint32_t>& indices,
        bool mayBlock,
        query_cb _hidl_cb) {
    typedef C2Param::Index Index;
    std::vector<Index> c2heapParamIndices(
            (Index*)indices.data(),
            (Index*)indices.data() + indices.size());
    std::vector<std::unique_ptr<C2Param>> c2heapParams;
    c2_status_t c2res = mIntf->query(
            c2heapParamIndices,
            mayBlock ? C2_MAY_BLOCK : C2_DONT_BLOCK,
            &c2heapParams);

    hidl_vec<uint8_t> params;
    if (!createParamsBlob(&params, c2heapParams)) {
        LOG(WARNING) << "query -- invalid output params.";
    }
    _hidl_cb(static_cast<Status>(c2res), params);
    return Void();
}

Return<void> CachedConfigurable::config(
        const hidl_vec<uint8_t>& inParams,
        bool mayBlock,
        config_cb _hidl_cb) {
    // inParams is not writable, so create a copy as config modifies the parameters
    hidl_vec<uint8_t> inParamsCopy = inParams;
    std::vector<C2Param*> c2params;
    if (!parseParamsBlob(&c2params, inParamsCopy)) {
        LOG(WARNING) << "config -- invalid input params.";
        _hidl_cb(Status::CORRUPTED,
                hidl_vec<SettingResult>(),
                hidl_vec<uint8_t>());
        return Void();
    }
    // TODO: check if blob was invalid
    std::vector<std::unique_ptr<C2SettingResult>> c2failures;
    c2_status_t c2res = mIntf->config(
            c2params,
            mayBlock ? C2_MAY_BLOCK : C2_DONT_BLOCK,
            &c2failures);
    hidl_vec<SettingResult> failures(c2failures.size());
    {
        size_t ix = 0;
        for (const std::unique_ptr<C2SettingResult>& c2result : c2failures) {
            if (c2result) {
                if (objcpy(&failures[ix], *c2result)) {
                    ++ix;
                } else {
                    LOG(DEBUG) << "config -- invalid setting results.";
                    break;
                }
            }
        }
        failures.resize(ix);
    }
    hidl_vec<uint8_t> outParams;
    if (!createParamsBlob(&outParams, c2params)) {
        LOG(DEBUG) << "config -- invalid output params.";
    }
    _hidl_cb((Status)c2res, failures, outParams);
    return Void();
}

Return<void> CachedConfigurable::querySupportedParams(
        uint32_t start,
        uint32_t count,
        querySupportedParams_cb _hidl_cb) {
    C2LinearRange request = C2LinearCapacity(mSupportedParams.size()).range(
            start, count);
    hidl_vec<ParamDescriptor> params(request.size());
    Status res = Status::OK;
    size_t dstIx = 0;
    for (size_t srcIx = request.offset(); srcIx < request.endOffset(); ++srcIx) {
        if (mSupportedParams[srcIx]) {
            if (objcpy(&params[dstIx], *mSupportedParams[srcIx])) {
                ++dstIx;
            } else {
                res = Status::CORRUPTED;
                LOG(WARNING) << "querySupportedParams -- invalid output params.";
                break;
            }
        } else {
            res = Status::BAD_INDEX;
        }
    }
    params.resize(dstIx);
    _hidl_cb(res, params);
    return Void();
}

Return<void> CachedConfigurable::querySupportedValues(
        const hidl_vec<FieldSupportedValuesQuery>& inFields,
        bool mayBlock,
        querySupportedValues_cb _hidl_cb) {
    std::vector<C2FieldSupportedValuesQuery> c2fields;
    {
        // C2FieldSupportedValuesQuery objects are restricted in that some
        // members are const.
        // C2ParamField - required for its constructor - has no constructors
        // from fields. Use C2ParamInspector.
        for (const FieldSupportedValuesQuery &query : inFields) {
            c2fields.emplace_back(_C2ParamInspector::CreateParamField(
                    query.field.index,
                    query.field.fieldId.offset,
                    query.field.fieldId.size),
                    query.type == FieldSupportedValuesQuery::Type::POSSIBLE ?
                    C2FieldSupportedValuesQuery::POSSIBLE :
                    C2FieldSupportedValuesQuery::CURRENT);
        }
    }
    c2_status_t c2res = mIntf->querySupportedValues(
            c2fields,
            mayBlock ? C2_MAY_BLOCK : C2_DONT_BLOCK);
    hidl_vec<FieldSupportedValuesQueryResult> outFields(inFields.size());
    size_t dstIx = 0;
    for (const C2FieldSupportedValuesQuery &result : c2fields) {
        if (objcpy(&outFields[dstIx], result)) {
            ++dstIx;
        } else {
            outFields.resize(dstIx);
            c2res = C2_CORRUPTED;
            LOG(WARNING) << "querySupportedValues -- invalid output params.";
            break;
        }
    }
    _hidl_cb((Status)c2res, outFields);
    return Void();
}

}  // namespace utils
}  // namespace V1_0
}  // namespace c2
}  // namespace media
}  // namespace hardware
}  // namespace android

