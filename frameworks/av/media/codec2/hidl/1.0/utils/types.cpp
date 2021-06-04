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
#define LOG_TAG "Codec2-types"
#include <android-base/logging.h>

#include <codec2/hidl/1.0/types.h>
#include <media/stagefright/foundation/AUtils.h>

#include <C2AllocatorIon.h>
#include <C2AllocatorGralloc.h>
#include <C2BlockInternal.h>
#include <C2Buffer.h>
#include <C2Component.h>
#include <C2Param.h>
#include <C2ParamInternal.h>
#include <C2PlatformSupport.h>
#include <C2Work.h>
#include <util/C2ParamUtils.h>

#include <algorithm>
#include <functional>
#include <iomanip>
#include <unordered_map>

namespace android {
namespace hardware {
namespace media {
namespace c2 {
namespace V1_0 {
namespace utils {

using ::android::hardware::Return;
using ::android::hardware::media::bufferpool::BufferPoolData;
using ::android::hardware::media::bufferpool::V2_0::BufferStatusMessage;
using ::android::hardware::media::bufferpool::V2_0::ResultStatus;
using ::android::hardware::media::bufferpool::V2_0::implementation::
        ClientManager;
using ::android::hardware::media::bufferpool::V2_0::implementation::
        TransactionId;

const char* asString(Status status, const char* def) {
    return asString(static_cast<c2_status_t>(status), def);
}

namespace /* unnamed */ {

template <typename EnumClass>
typename std::underlying_type<EnumClass>::type underlying_value(
        EnumClass x) {
    return static_cast<typename std::underlying_type<EnumClass>::type>(x);
}

template <typename Common, typename DstVector, typename SrcVector>
void copyVector(DstVector* d, const SrcVector& s) {
    static_assert(sizeof(Common) == sizeof(decltype((*d)[0])),
            "DstVector's component size does not match Common");
    static_assert(sizeof(Common) == sizeof(decltype(s[0])),
            "SrcVector's component size does not match Common");
    d->resize(s.size());
    std::copy(
            reinterpret_cast<const Common*>(&s[0]),
            reinterpret_cast<const Common*>(&s[0] + s.size()),
            reinterpret_cast<Common*>(&(*d)[0]));
}

// C2ParamField -> ParamField
bool objcpy(ParamField *d, const C2ParamField &s) {
    d->index = static_cast<ParamIndex>(_C2ParamInspector::GetIndex(s));
    d->fieldId.offset = static_cast<uint32_t>(_C2ParamInspector::GetOffset(s));
    d->fieldId.size = static_cast<uint32_t>(_C2ParamInspector::GetSize(s));
    return true;
}

struct C2ParamFieldBuilder : public C2ParamField {
    C2ParamFieldBuilder() : C2ParamField(
            static_cast<C2Param::Index>(static_cast<uint32_t>(0)), 0, 0) {
    }
    // ParamField -> C2ParamField
    C2ParamFieldBuilder(const ParamField& s) : C2ParamField(
            static_cast<C2Param::Index>(static_cast<uint32_t>(s.index)),
            static_cast<uint32_t>(s.fieldId.offset),
            static_cast<uint32_t>(s.fieldId.size)) {
    }
};

// C2WorkOrdinalStruct -> WorkOrdinal
bool objcpy(WorkOrdinal *d, const C2WorkOrdinalStruct &s) {
    d->frameIndex = static_cast<uint64_t>(s.frameIndex.peeku());
    d->timestampUs = static_cast<uint64_t>(s.timestamp.peeku());
    d->customOrdinal = static_cast<uint64_t>(s.customOrdinal.peeku());
    return true;
}

// WorkOrdinal -> C2WorkOrdinalStruct
bool objcpy(C2WorkOrdinalStruct *d, const WorkOrdinal &s) {
    d->frameIndex = c2_cntr64_t(s.frameIndex);
    d->timestamp = c2_cntr64_t(s.timestampUs);
    d->customOrdinal = c2_cntr64_t(s.customOrdinal);
    return true;
}

// C2FieldSupportedValues::range's type -> ValueRange
bool objcpy(
        ValueRange* d,
        const decltype(C2FieldSupportedValues::range)& s) {
    d->min = static_cast<PrimitiveValue>(s.min.u64);
    d->max = static_cast<PrimitiveValue>(s.max.u64);
    d->step = static_cast<PrimitiveValue>(s.step.u64);
    d->num = static_cast<PrimitiveValue>(s.num.u64);
    d->denom = static_cast<PrimitiveValue>(s.denom.u64);
    return true;
}

// C2FieldSupportedValues -> FieldSupportedValues
bool objcpy(FieldSupportedValues *d, const C2FieldSupportedValues &s) {
    switch (s.type) {
    case C2FieldSupportedValues::EMPTY: {
            d->empty(::android::hidl::safe_union::V1_0::Monostate{});
            break;
        }
    case C2FieldSupportedValues::RANGE: {
            ValueRange range{};
            if (!objcpy(&range, s.range)) {
                LOG(ERROR) << "Invalid C2FieldSupportedValues::range.";
                d->range(range);
                return false;
            }
            d->range(range);
            break;
        }
    case C2FieldSupportedValues::VALUES: {
            hidl_vec<PrimitiveValue> values;
            copyVector<uint64_t>(&values, s.values);
            d->values(values);
            break;
        }
    case C2FieldSupportedValues::FLAGS: {
            hidl_vec<PrimitiveValue> flags;
            copyVector<uint64_t>(&flags, s.values);
            d->flags(flags);
            break;
        }
    default:
        LOG(DEBUG) << "Unrecognized C2FieldSupportedValues::type_t "
                   << "with underlying value " << underlying_value(s.type)
                   << ".";
        return false;
    }
    return true;
}

// ValueRange -> C2FieldSupportedValues::range's type
bool objcpy(
        decltype(C2FieldSupportedValues::range)* d,
        const ValueRange& s) {
    d->min.u64 = static_cast<uint64_t>(s.min);
    d->max.u64 = static_cast<uint64_t>(s.max);
    d->step.u64 = static_cast<uint64_t>(s.step);
    d->num.u64 = static_cast<uint64_t>(s.num);
    d->denom.u64 = static_cast<uint64_t>(s.denom);
    return true;
}

// FieldSupportedValues -> C2FieldSupportedValues
bool objcpy(C2FieldSupportedValues *d, const FieldSupportedValues &s) {
    switch (s.getDiscriminator()) {
    case FieldSupportedValues::hidl_discriminator::empty: {
            d->type = C2FieldSupportedValues::EMPTY;
            break;
        }
    case FieldSupportedValues::hidl_discriminator::range: {
            d->type = C2FieldSupportedValues::RANGE;
            if (!objcpy(&d->range, s.range())) {
                LOG(ERROR) << "Invalid FieldSupportedValues::range.";
                return false;
            }
            d->values.resize(0);
            break;
        }
    case FieldSupportedValues::hidl_discriminator::values: {
            d->type = C2FieldSupportedValues::VALUES;
            copyVector<uint64_t>(&d->values, s.values());
            break;
        }
    case FieldSupportedValues::hidl_discriminator::flags: {
            d->type = C2FieldSupportedValues::FLAGS;
            copyVector<uint64_t>(&d->values, s.flags());
            break;
        }
    default:
        LOG(WARNING) << "Unrecognized FieldSupportedValues::getDiscriminator()";
        return false;
    }
    return true;
}

} // unnamed namespace

// C2FieldSupportedValuesQuery -> FieldSupportedValuesQuery
bool objcpy(
        FieldSupportedValuesQuery* d,
        const C2FieldSupportedValuesQuery& s) {
    if (!objcpy(&d->field, s.field())) {
        LOG(ERROR) << "Invalid C2FieldSupportedValuesQuery::field.";
        return false;
    }
    switch (s.type()) {
    case C2FieldSupportedValuesQuery::POSSIBLE:
        d->type = FieldSupportedValuesQuery::Type::POSSIBLE;
        break;
    case C2FieldSupportedValuesQuery::CURRENT:
        d->type = FieldSupportedValuesQuery::Type::CURRENT;
        break;
    default:
        LOG(DEBUG) << "Unrecognized C2FieldSupportedValuesQuery::type_t "
                   << "with underlying value " << underlying_value(s.type())
                   << ".";
        d->type = static_cast<FieldSupportedValuesQuery::Type>(s.type());
    }
    return true;
}

// FieldSupportedValuesQuery -> C2FieldSupportedValuesQuery
bool objcpy(
        C2FieldSupportedValuesQuery* d,
        const FieldSupportedValuesQuery& s) {
    C2FieldSupportedValuesQuery::type_t dType;
    switch (s.type) {
    case FieldSupportedValuesQuery::Type::POSSIBLE:
        dType = C2FieldSupportedValuesQuery::POSSIBLE;
        break;
    case FieldSupportedValuesQuery::Type::CURRENT:
        dType = C2FieldSupportedValuesQuery::CURRENT;
        break;
    default:
        LOG(DEBUG) << "Unrecognized FieldSupportedValuesQuery::Type "
                   << "with underlying value " << underlying_value(s.type)
                   << ".";
        dType = static_cast<C2FieldSupportedValuesQuery::type_t>(s.type);
    }
    *d = C2FieldSupportedValuesQuery(C2ParamFieldBuilder(s.field), dType);
    return true;
}

// C2FieldSupportedValuesQuery -> FieldSupportedValuesQueryResult
bool objcpy(
        FieldSupportedValuesQueryResult* d,
        const C2FieldSupportedValuesQuery& s) {
    d->status = static_cast<Status>(s.status);
    return objcpy(&d->values, s.values);
}

// FieldSupportedValuesQuery, FieldSupportedValuesQueryResult ->
// C2FieldSupportedValuesQuery
bool objcpy(
        C2FieldSupportedValuesQuery* d,
        const FieldSupportedValuesQuery& sq,
        const FieldSupportedValuesQueryResult& sr) {
    if (!objcpy(d, sq)) {
        LOG(ERROR) << "Invalid FieldSupportedValuesQuery.";
        return false;
    }
    d->status = static_cast<c2_status_t>(sr.status);
    if (!objcpy(&d->values, sr.values)) {
        LOG(ERROR) << "Invalid FieldSupportedValuesQueryResult::values.";
        return false;
    }
    return true;
}

// C2Component::Traits -> IComponentStore::ComponentTraits
bool objcpy(
        IComponentStore::ComponentTraits *d,
        const C2Component::Traits &s) {
    d->name = s.name;

    switch (s.domain) {
    case C2Component::DOMAIN_VIDEO:
        d->domain = IComponentStore::ComponentTraits::Domain::VIDEO;
        break;
    case C2Component::DOMAIN_AUDIO:
        d->domain = IComponentStore::ComponentTraits::Domain::AUDIO;
        break;
    case C2Component::DOMAIN_IMAGE:
        d->domain = IComponentStore::ComponentTraits::Domain::IMAGE;
        break;
    case C2Component::DOMAIN_OTHER:
        d->domain = IComponentStore::ComponentTraits::Domain::OTHER;
        break;
    default:
        LOG(DEBUG) << "Unrecognized C2Component::domain_t "
                   << "with underlying value " << underlying_value(s.domain)
                   << ".";
        d->domain = static_cast<IComponentStore::ComponentTraits::Domain>(
                s.domain);
    }

    switch (s.kind) {
    case C2Component::KIND_DECODER:
        d->kind = IComponentStore::ComponentTraits::Kind::DECODER;
        break;
    case C2Component::KIND_ENCODER:
        d->kind = IComponentStore::ComponentTraits::Kind::ENCODER;
        break;
    case C2Component::KIND_OTHER:
        d->kind = IComponentStore::ComponentTraits::Kind::OTHER;
        break;
    default:
        LOG(DEBUG) << "Unrecognized C2Component::kind_t "
                   << "with underlying value " << underlying_value(s.kind)
                   << ".";
        d->kind = static_cast<IComponentStore::ComponentTraits::Kind>(
                s.kind);
    }

    d->rank = static_cast<uint32_t>(s.rank);

    d->mediaType = s.mediaType;

    d->aliases.resize(s.aliases.size());
    for (size_t ix = s.aliases.size(); ix > 0; ) {
        --ix;
        d->aliases[ix] = s.aliases[ix];
    }
    return true;
}

// ComponentTraits -> C2Component::Traits, std::unique_ptr<std::vector<std::string>>
bool objcpy(
        C2Component::Traits* d,
        const IComponentStore::ComponentTraits& s) {
    d->name = s.name.c_str();

    switch (s.domain) {
    case IComponentStore::ComponentTraits::Domain::VIDEO:
        d->domain = C2Component::DOMAIN_VIDEO;
        break;
    case IComponentStore::ComponentTraits::Domain::AUDIO:
        d->domain = C2Component::DOMAIN_AUDIO;
        break;
    case IComponentStore::ComponentTraits::Domain::IMAGE:
        d->domain = C2Component::DOMAIN_IMAGE;
        break;
    case IComponentStore::ComponentTraits::Domain::OTHER:
        d->domain = C2Component::DOMAIN_OTHER;
        break;
    default:
        LOG(DEBUG) << "Unrecognized ComponentTraits::Domain "
                   << "with underlying value " << underlying_value(s.domain)
                   << ".";
        d->domain = static_cast<C2Component::domain_t>(s.domain);
    }

    switch (s.kind) {
    case IComponentStore::ComponentTraits::Kind::DECODER:
        d->kind = C2Component::KIND_DECODER;
        break;
    case IComponentStore::ComponentTraits::Kind::ENCODER:
        d->kind = C2Component::KIND_ENCODER;
        break;
    case IComponentStore::ComponentTraits::Kind::OTHER:
        d->kind = C2Component::KIND_OTHER;
        break;
    default:
        LOG(DEBUG) << "Unrecognized ComponentTraits::Kind "
                   << "with underlying value " << underlying_value(s.kind)
                   << ".";
        d->kind = static_cast<C2Component::kind_t>(s.kind);
    }

    d->rank = static_cast<C2Component::rank_t>(s.rank);
    d->mediaType = s.mediaType.c_str();
    d->aliases.resize(s.aliases.size());
    for (size_t i = 0; i < s.aliases.size(); ++i) {
        d->aliases[i] = s.aliases[i];
    }
    return true;
}

namespace /* unnamed */ {

// C2ParamFieldValues -> ParamFieldValues
bool objcpy(ParamFieldValues *d, const C2ParamFieldValues &s) {
    if (!objcpy(&d->paramOrField, s.paramOrField)) {
        LOG(ERROR) << "Invalid C2ParamFieldValues::paramOrField.";
        return false;
    }
    if (s.values) {
        d->values.resize(1);
        if (!objcpy(&d->values[0], *s.values)) {
            LOG(ERROR) << "Invalid C2ParamFieldValues::values.";
            return false;
        }
        return true;
    }
    d->values.resize(0);
    return true;
}

// ParamFieldValues -> C2ParamFieldValues
bool objcpy(C2ParamFieldValues *d, const ParamFieldValues &s) {
    d->paramOrField = C2ParamFieldBuilder(s.paramOrField);
    if (s.values.size() == 1) {
        d->values = std::make_unique<C2FieldSupportedValues>();
        if (!objcpy(d->values.get(), s.values[0])) {
            LOG(ERROR) << "Invalid ParamFieldValues::values.";
            return false;
        }
        return true;
    } else if (s.values.size() == 0) {
        d->values.reset();
        return true;
    }
    LOG(ERROR) << "Invalid ParamFieldValues: "
                  "Two or more FieldSupportedValues objects exist in "
                  "ParamFieldValues. "
                  "Only zero or one is allowed.";
    return false;
}

} // unnamed namespace

// C2SettingResult -> SettingResult
bool objcpy(SettingResult *d, const C2SettingResult &s) {
    switch (s.failure) {
    case C2SettingResult::BAD_TYPE:
        d->failure = SettingResult::Failure::BAD_TYPE;
        break;
    case C2SettingResult::BAD_PORT:
        d->failure = SettingResult::Failure::BAD_PORT;
        break;
    case C2SettingResult::BAD_INDEX:
        d->failure = SettingResult::Failure::BAD_INDEX;
        break;
    case C2SettingResult::READ_ONLY:
        d->failure = SettingResult::Failure::READ_ONLY;
        break;
    case C2SettingResult::MISMATCH:
        d->failure = SettingResult::Failure::MISMATCH;
        break;
    case C2SettingResult::BAD_VALUE:
        d->failure = SettingResult::Failure::BAD_VALUE;
        break;
    case C2SettingResult::CONFLICT:
        d->failure = SettingResult::Failure::CONFLICT;
        break;
    case C2SettingResult::UNSUPPORTED:
        d->failure = SettingResult::Failure::UNSUPPORTED;
        break;
    case C2SettingResult::INFO_BAD_VALUE:
        d->failure = SettingResult::Failure::INFO_BAD_VALUE;
        break;
    case C2SettingResult::INFO_CONFLICT:
        d->failure = SettingResult::Failure::INFO_CONFLICT;
        break;
    default:
        LOG(DEBUG) << "Unrecognized C2SettingResult::Failure "
                   << "with underlying value " << underlying_value(s.failure)
                   << ".";
        d->failure = static_cast<SettingResult::Failure>(s.failure);
    }
    if (!objcpy(&d->field, s.field)) {
        LOG(ERROR) << "Invalid C2SettingResult::field.";
        return false;
    }
    d->conflicts.resize(s.conflicts.size());
    size_t i = 0;
    for (const C2ParamFieldValues& sConflict : s.conflicts) {
        ParamFieldValues &dConflict = d->conflicts[i++];
        if (!objcpy(&dConflict, sConflict)) {
            LOG(ERROR) << "Invalid C2SettingResult::conflicts["
                       << i - 1 << "].";
            return false;
        }
    }
    return true;
}

// SettingResult -> std::unique_ptr<C2SettingResult>
bool objcpy(std::unique_ptr<C2SettingResult> *d, const SettingResult &s) {
    *d = std::unique_ptr<C2SettingResult>(new C2SettingResult {
            .field = C2ParamFieldValues(C2ParamFieldBuilder()) });
    if (!*d) {
        LOG(ERROR) << "No memory for C2SettingResult.";
        return false;
    }

    // failure
    switch (s.failure) {
    case SettingResult::Failure::BAD_TYPE:
        (*d)->failure = C2SettingResult::BAD_TYPE;
        break;
    case SettingResult::Failure::BAD_PORT:
        (*d)->failure = C2SettingResult::BAD_PORT;
        break;
    case SettingResult::Failure::BAD_INDEX:
        (*d)->failure = C2SettingResult::BAD_INDEX;
        break;
    case SettingResult::Failure::READ_ONLY:
        (*d)->failure = C2SettingResult::READ_ONLY;
        break;
    case SettingResult::Failure::MISMATCH:
        (*d)->failure = C2SettingResult::MISMATCH;
        break;
    case SettingResult::Failure::BAD_VALUE:
        (*d)->failure = C2SettingResult::BAD_VALUE;
        break;
    case SettingResult::Failure::CONFLICT:
        (*d)->failure = C2SettingResult::CONFLICT;
        break;
    case SettingResult::Failure::UNSUPPORTED:
        (*d)->failure = C2SettingResult::UNSUPPORTED;
        break;
    case SettingResult::Failure::INFO_BAD_VALUE:
        (*d)->failure = C2SettingResult::INFO_BAD_VALUE;
        break;
    case SettingResult::Failure::INFO_CONFLICT:
        (*d)->failure = C2SettingResult::INFO_CONFLICT;
        break;
    default:
        LOG(DEBUG) << "Unrecognized SettingResult::Failure "
                   << "with underlying value " << underlying_value(s.failure)
                   << ".";
        (*d)->failure = static_cast<C2SettingResult::Failure>(s.failure);
    }

    // field
    if (!objcpy(&(*d)->field, s.field)) {
        LOG(ERROR) << "Invalid SettingResult::field.";
        return false;
    }

    // conflicts
    (*d)->conflicts.clear();
    (*d)->conflicts.reserve(s.conflicts.size());
    for (const ParamFieldValues& sConflict : s.conflicts) {
        (*d)->conflicts.emplace_back(
                C2ParamFieldValues{ C2ParamFieldBuilder(), nullptr });
        if (!objcpy(&(*d)->conflicts.back(), sConflict)) {
            LOG(ERROR) << "Invalid SettingResult::conflicts.";
            return false;
        }
    }
    return true;
}

// C2ParamDescriptor -> ParamDescriptor
bool objcpy(ParamDescriptor *d, const C2ParamDescriptor &s) {
    d->index = static_cast<ParamIndex>(s.index());
    d->attrib = static_cast<hidl_bitfield<ParamDescriptor::Attrib>>(
            _C2ParamInspector::GetAttrib(s));
    d->name = s.name();
    copyVector<uint32_t>(&d->dependencies, s.dependencies());
    return true;
}

// ParamDescriptor -> C2ParamDescriptor
bool objcpy(std::shared_ptr<C2ParamDescriptor> *d, const ParamDescriptor &s) {
    std::vector<C2Param::Index> dDependencies;
    dDependencies.reserve(s.dependencies.size());
    for (const ParamIndex& sDependency : s.dependencies) {
        dDependencies.emplace_back(static_cast<uint32_t>(sDependency));
    }
    *d = std::make_shared<C2ParamDescriptor>(
            C2Param::Index(static_cast<uint32_t>(s.index)),
            static_cast<C2ParamDescriptor::attrib_t>(s.attrib),
            C2String(s.name.c_str()),
            std::move(dDependencies));
    return true;
}

// C2StructDescriptor -> StructDescriptor
bool objcpy(StructDescriptor *d, const C2StructDescriptor &s) {
    d->type = static_cast<ParamIndex>(s.coreIndex().coreIndex());
    d->fields.resize(s.numFields());
    size_t i = 0;
    for (const auto& sField : s) {
        FieldDescriptor& dField = d->fields[i++];
        dField.fieldId.offset = static_cast<uint32_t>(
                _C2ParamInspector::GetOffset(sField));
        dField.fieldId.size = static_cast<uint32_t>(
                _C2ParamInspector::GetSize(sField));
        dField.type = static_cast<hidl_bitfield<FieldDescriptor::Type>>(
                sField.type());
        dField.extent = static_cast<uint32_t>(sField.extent());
        dField.name = static_cast<hidl_string>(sField.name());
        const auto& sNamedValues = sField.namedValues();
        dField.namedValues.resize(sNamedValues.size());
        size_t j = 0;
        for (const auto& sNamedValue : sNamedValues) {
            FieldDescriptor::NamedValue& dNamedValue = dField.namedValues[j++];
            dNamedValue.name = static_cast<hidl_string>(sNamedValue.first);
            dNamedValue.value = static_cast<PrimitiveValue>(
                    sNamedValue.second.u64);
        }
    }
    return true;
}

// StructDescriptor -> C2StructDescriptor
bool objcpy(std::unique_ptr<C2StructDescriptor> *d, const StructDescriptor &s) {
    C2Param::CoreIndex dIndex = C2Param::CoreIndex(static_cast<uint32_t>(s.type));
    std::vector<C2FieldDescriptor> dFields;
    dFields.reserve(s.fields.size());
    for (const auto &sField : s.fields) {
        C2FieldDescriptor dField = {
            static_cast<uint32_t>(sField.type),
            sField.extent,
            sField.name,
            sField.fieldId.offset,
            sField.fieldId.size };
        C2FieldDescriptor::NamedValuesType namedValues;
        namedValues.reserve(sField.namedValues.size());
        for (const auto& sNamedValue : sField.namedValues) {
            namedValues.emplace_back(
                sNamedValue.name,
                C2Value::Primitive(static_cast<uint64_t>(sNamedValue.value)));
        }
        _C2ParamInspector::AddNamedValues(dField, std::move(namedValues));
        dFields.emplace_back(dField);
    }
    *d = std::make_unique<C2StructDescriptor>(
            _C2ParamInspector::CreateStructDescriptor(dIndex, std::move(dFields)));
    return true;
}

namespace /* unnamed */ {

// Find or add a hidl BaseBlock object from a given C2Handle* to a list and an
// associated map.
// Note: The handle is not cloned.
bool _addBaseBlock(
        uint32_t* index,
        const C2Handle* handle,
        std::list<BaseBlock>* baseBlocks,
        std::map<const void*, uint32_t>* baseBlockIndices) {
    if (!handle) {
        LOG(ERROR) << "addBaseBlock called on a null C2Handle.";
        return false;
    }
    auto it = baseBlockIndices->find(handle);
    if (it != baseBlockIndices->end()) {
        *index = it->second;
    } else {
        *index = baseBlocks->size();
        baseBlockIndices->emplace(handle, *index);
        baseBlocks->emplace_back();

        BaseBlock &dBaseBlock = baseBlocks->back();
        // This does not clone the handle.
        dBaseBlock.nativeBlock(
                reinterpret_cast<const native_handle_t*>(handle));

    }
    return true;
}

// Find or add a hidl BaseBlock object from a given BufferPoolData to a list and
// an associated map.
bool _addBaseBlock(
        uint32_t* index,
        const std::shared_ptr<BufferPoolData> bpData,
        BufferPoolSender* bufferPoolSender,
        std::list<BaseBlock>* baseBlocks,
        std::map<const void*, uint32_t>* baseBlockIndices) {
    if (!bpData) {
        LOG(ERROR) << "addBaseBlock called on a null BufferPoolData.";
        return false;
    }
    auto it = baseBlockIndices->find(bpData.get());
    if (it != baseBlockIndices->end()) {
        *index = it->second;
    } else {
        *index = baseBlocks->size();
        baseBlockIndices->emplace(bpData.get(), *index);
        baseBlocks->emplace_back();

        BaseBlock &dBaseBlock = baseBlocks->back();

        if (bufferPoolSender) {
            BufferStatusMessage pooledBlock;
            ResultStatus bpStatus = bufferPoolSender->send(
                    bpData,
                    &pooledBlock);

            if (bpStatus != ResultStatus::OK) {
                LOG(ERROR) << "Failed to send buffer with BufferPool. Error: "
                           << static_cast<int32_t>(bpStatus)
                           << ".";
                return false;
            }
            dBaseBlock.pooledBlock(pooledBlock);
        }
    }
    return true;
}

bool addBaseBlock(
        uint32_t* index,
        const C2Handle* handle,
        const std::shared_ptr<const _C2BlockPoolData>& blockPoolData,
        BufferPoolSender* bufferPoolSender,
        std::list<BaseBlock>* baseBlocks,
        std::map<const void*, uint32_t>* baseBlockIndices) {
    if (!blockPoolData) {
        // No BufferPoolData ==> NATIVE block.
        return _addBaseBlock(
                index, handle,
                baseBlocks, baseBlockIndices);
    }
    switch (blockPoolData->getType()) {
    case _C2BlockPoolData::TYPE_BUFFERPOOL: {
            // BufferPoolData
            std::shared_ptr<BufferPoolData> bpData;
            if (!_C2BlockFactory::GetBufferPoolData(blockPoolData, &bpData)
                    || !bpData) {
                LOG(ERROR) << "BufferPoolData unavailable in a block.";
                return false;
            }
            return _addBaseBlock(
                    index, bpData,
                    bufferPoolSender, baseBlocks, baseBlockIndices);
        }
    case _C2BlockPoolData::TYPE_BUFFERQUEUE:
        uint32_t gen;
        uint64_t bqId;
        int32_t bqSlot;
        // Update handle if migration happened.
        if (_C2BlockFactory::GetBufferQueueData(
                blockPoolData, &gen, &bqId, &bqSlot)) {
            android::MigrateNativeCodec2GrallocHandle(
                    const_cast<native_handle_t*>(handle), gen, bqId, bqSlot);
        }
        return _addBaseBlock(
                index, handle,
                baseBlocks, baseBlockIndices);
    default:
        LOG(ERROR) << "Unknown C2BlockPoolData type.";
        return false;
    }
}

// C2Fence -> hidl_handle
// Note: File descriptors are not duplicated. The original file descriptor must
// not be closed before the transaction is complete.
bool objcpy(hidl_handle* d, const C2Fence& s) {
    (void)s; // TODO: implement s.fd()
    int fenceFd = -1;
    d->setTo(nullptr);
    if (fenceFd >= 0) {
        native_handle_t *handle = native_handle_create(1, 0);
        if (!handle) {
            LOG(ERROR) << "Failed to create a native handle.";
            return false;
        }
        handle->data[0] = fenceFd;
        d->setTo(handle, true /* owns */);
    }
    return true;
}

// C2ConstLinearBlock -> Block
// Note: Native handles are not duplicated. The original handles must not be
// closed before the transaction is complete.
bool objcpy(Block* d, const C2ConstLinearBlock& s,
        BufferPoolSender* bufferPoolSender,
        std::list<BaseBlock>* baseBlocks,
        std::map<const void*, uint32_t>* baseBlockIndices) {
    std::shared_ptr<const _C2BlockPoolData> bpData =
            _C2BlockFactory::GetLinearBlockPoolData(s);
    if (!addBaseBlock(&d->index, s.handle(), bpData,
            bufferPoolSender, baseBlocks, baseBlockIndices)) {
        LOG(ERROR) << "Invalid block data in C2ConstLinearBlock.";
        return false;
    }

    // Create the metadata.
    C2Hidl_RangeInfo dRangeInfo;
    dRangeInfo.offset = static_cast<uint32_t>(s.offset());
    dRangeInfo.length = static_cast<uint32_t>(s.size());
    if (!createParamsBlob(&d->meta, std::vector<C2Param*>{ &dRangeInfo })) {
        LOG(ERROR) << "Invalid range info in C2ConstLinearBlock.";
        return false;
    }

    // Copy the fence
    if (!objcpy(&d->fence, s.fence())) {
        LOG(ERROR) << "Invalid C2ConstLinearBlock::fence.";
        return false;
    }
    return true;
}

// C2ConstGraphicBlock -> Block
// Note: Native handles are not duplicated. The original handles must not be
// closed before the transaction is complete.
bool objcpy(Block* d, const C2ConstGraphicBlock& s,
        BufferPoolSender* bufferPoolSender,
        std::list<BaseBlock>* baseBlocks,
        std::map<const void*, uint32_t>* baseBlockIndices) {
    std::shared_ptr<const _C2BlockPoolData> bpData =
            _C2BlockFactory::GetGraphicBlockPoolData(s);
    if (!addBaseBlock(&d->index, s.handle(), bpData,
            bufferPoolSender, baseBlocks, baseBlockIndices)) {
        LOG(ERROR) << "Invalid block data in C2ConstGraphicBlock.";
        return false;
    }

    // Create the metadata.
    C2Hidl_RectInfo dRectInfo;
    C2Rect sRect = s.crop();
    dRectInfo.left = static_cast<uint32_t>(sRect.left);
    dRectInfo.top = static_cast<uint32_t>(sRect.top);
    dRectInfo.width = static_cast<uint32_t>(sRect.width);
    dRectInfo.height = static_cast<uint32_t>(sRect.height);
    if (!createParamsBlob(&d->meta, std::vector<C2Param*>{ &dRectInfo })) {
        LOG(ERROR) << "Invalid rect info in C2ConstGraphicBlock.";
        return false;
    }

    // Copy the fence
    if (!objcpy(&d->fence, s.fence())) {
        LOG(ERROR) << "Invalid C2ConstGraphicBlock::fence.";
        return false;
    }
    return true;
}

// C2BufferData -> Buffer
// This function only fills in d->blocks.
bool objcpy(Buffer* d, const C2BufferData& s,
        BufferPoolSender* bufferPoolSender,
        std::list<BaseBlock>* baseBlocks,
        std::map<const void*, uint32_t>* baseBlockIndices) {
    d->blocks.resize(
            s.linearBlocks().size() +
            s.graphicBlocks().size());
    size_t i = 0;
    for (const C2ConstLinearBlock& linearBlock : s.linearBlocks()) {
        Block& dBlock = d->blocks[i++];
        if (!objcpy(
                &dBlock, linearBlock,
                bufferPoolSender, baseBlocks, baseBlockIndices)) {
            LOG(ERROR) << "Invalid C2BufferData::linearBlocks. "
                       << "(Destination index = " << i - 1 << ".)";
            return false;
        }
    }
    for (const C2ConstGraphicBlock& graphicBlock : s.graphicBlocks()) {
        Block& dBlock = d->blocks[i++];
        if (!objcpy(
                &dBlock, graphicBlock,
                bufferPoolSender, baseBlocks, baseBlockIndices)) {
            LOG(ERROR) << "Invalid C2BufferData::graphicBlocks. "
                       << "(Destination index = " << i - 1 << ".)";
            return false;
        }
    }
    return true;
}

// C2Buffer -> Buffer
bool objcpy(Buffer* d, const C2Buffer& s,
        BufferPoolSender* bufferPoolSender,
        std::list<BaseBlock>* baseBlocks,
        std::map<const void*, uint32_t>* baseBlockIndices) {
    if (!createParamsBlob(&d->info, s.info())) {
        LOG(ERROR) << "Invalid C2Buffer::info.";
        return false;
    }
    if (!objcpy(d, s.data(), bufferPoolSender, baseBlocks, baseBlockIndices)) {
        LOG(ERROR) << "Invalid C2Buffer::data.";
        return false;
    }
    return true;
}

// C2InfoBuffer -> InfoBuffer
bool objcpy(InfoBuffer* d, const C2InfoBuffer& s,
        BufferPoolSender* bufferPoolSender,
        std::list<BaseBlock>* baseBlocks,
        std::map<const void*, uint32_t>* baseBlockIndices) {
    // TODO: C2InfoBuffer is not implemented.
    (void)d;
    (void)s;
    (void)bufferPoolSender;
    (void)baseBlocks;
    (void)baseBlockIndices;
    LOG(INFO) << "InfoBuffer not implemented.";
    return true;
}

// C2FrameData -> FrameData
bool objcpy(FrameData* d, const C2FrameData& s,
        BufferPoolSender* bufferPoolSender,
        std::list<BaseBlock>* baseBlocks,
        std::map<const void*, uint32_t>* baseBlockIndices) {
    d->flags = static_cast<hidl_bitfield<FrameData::Flags>>(s.flags);
    if (!objcpy(&d->ordinal, s.ordinal)) {
        LOG(ERROR) << "Invalid C2FrameData::ordinal.";
        return false;
    }

    d->buffers.resize(s.buffers.size());
    size_t i = 0;
    for (const std::shared_ptr<C2Buffer>& sBuffer : s.buffers) {
        Buffer& dBuffer = d->buffers[i++];
        if (!sBuffer) {
            // A null (pointer to) C2Buffer corresponds to a Buffer with empty
            // info and blocks.
            dBuffer.info.resize(0);
            dBuffer.blocks.resize(0);
            continue;
        }
        if (!objcpy(
                &dBuffer, *sBuffer,
                bufferPoolSender, baseBlocks, baseBlockIndices)) {
            LOG(ERROR) << "Invalid C2FrameData::buffers["
                       << i - 1 << "].";
            return false;
        }
    }

    if (!createParamsBlob(&d->configUpdate, s.configUpdate)) {
        LOG(ERROR) << "Invalid C2FrameData::configUpdate.";
        return false;
    }

    d->infoBuffers.resize(s.infoBuffers.size());
    i = 0;
    for (const std::shared_ptr<C2InfoBuffer>& sInfoBuffer : s.infoBuffers) {
        InfoBuffer& dInfoBuffer = d->infoBuffers[i++];
        if (!sInfoBuffer) {
            LOG(ERROR) << "Null C2FrameData::infoBuffers["
                       << i - 1 << "].";
            return false;
        }
        if (!objcpy(&dInfoBuffer, *sInfoBuffer,
                bufferPoolSender, baseBlocks, baseBlockIndices)) {
            LOG(ERROR) << "Invalid C2FrameData::infoBuffers["
                       << i - 1 << "].";
            return false;
        }
    }

    return true;
}

} // unnamed namespace

// DefaultBufferPoolSender's implementation

DefaultBufferPoolSender::DefaultBufferPoolSender(
        const sp<IClientManager>& receiverManager,
        std::chrono::steady_clock::duration refreshInterval)
    : mReceiverManager(receiverManager),
      mSourceConnectionId(0),
      mLastSent(std::chrono::steady_clock::now()),
      mRefreshInterval(refreshInterval) {
}

void DefaultBufferPoolSender::setReceiver(
        const sp<IClientManager>& receiverManager,
        std::chrono::steady_clock::duration refreshInterval) {
    std::lock_guard<std::mutex> lock(mMutex);
    if (mReceiverManager != receiverManager) {
        mReceiverManager = receiverManager;
    }
    mRefreshInterval = refreshInterval;
}

ResultStatus DefaultBufferPoolSender::send(
        const std::shared_ptr<BufferPoolData>& bpData,
        BufferStatusMessage* bpMessage) {
    if (!mReceiverManager) {
        LOG(ERROR) << "No access to receiver's BufferPool.";
        return ResultStatus::NOT_FOUND;
    }
    ResultStatus rs;
    std::lock_guard<std::mutex> lock(mMutex);
    if (!mSenderManager) {
        mSenderManager = ClientManager::getInstance();
        if (!mSenderManager) {
            LOG(ERROR) << "Failed to retrieve local BufferPool ClientManager.";
            return ResultStatus::CRITICAL_ERROR;
        }
    }
    int64_t connectionId = bpData->mConnectionId;
    std::chrono::steady_clock::time_point now =
            std::chrono::steady_clock::now();
    std::chrono::steady_clock::duration interval = now - mLastSent;
    if (mSourceConnectionId == 0 ||
            mSourceConnectionId != connectionId ||
            interval > mRefreshInterval) {
        // Initialize the bufferpool connection.
        mSourceConnectionId = connectionId;
        if (mSourceConnectionId == 0) {
            return ResultStatus::CRITICAL_ERROR;
        }

        int64_t receiverConnectionId;
        rs = mSenderManager->registerSender(mReceiverManager,
                                            connectionId,
                                            &receiverConnectionId);
        if ((rs != ResultStatus::OK) && (rs != ResultStatus::ALREADY_EXISTS)) {
            LOG(WARNING) << "registerSender -- returned error: "
                         << static_cast<int32_t>(rs)
                         << ".";
            return rs;
        } else {
            mReceiverConnectionId = receiverConnectionId;
        }
    }

    uint64_t transactionId;
    int64_t timestampUs;
    rs = mSenderManager->postSend(
            mReceiverConnectionId, bpData, &transactionId, &timestampUs);
    if (rs != ResultStatus::OK) {
        LOG(ERROR) << "ClientManager::postSend -- returned error: "
                   << static_cast<int32_t>(rs)
                   << ".";
        return rs;
    }
    if (!bpMessage) {
        LOG(ERROR) << "Null output parameter for BufferStatusMessage.";
        return ResultStatus::CRITICAL_ERROR;
    }
    bpMessage->connectionId = mReceiverConnectionId;
    bpMessage->bufferId = bpData->mId;
    bpMessage->transactionId = transactionId;
    bpMessage->timestampUs = timestampUs;
    mLastSent = now;
    return rs;
}

// std::list<std::unique_ptr<C2Work>> -> WorkBundle
bool objcpy(
        WorkBundle* d,
        const std::list<std::unique_ptr<C2Work>>& s,
        BufferPoolSender* bufferPoolSender) {
    // baseBlocks holds a list of BaseBlock objects that Blocks can refer to.
    std::list<BaseBlock> baseBlocks;

    // baseBlockIndices maps a raw pointer to native_handle_t or BufferPoolData
    // inside baseBlocks to the corresponding index into baseBlocks. The keys
    // (pointers) are used to identify blocks that have the same "base block" in
    // s, a list of C2Work objects. Because baseBlocks will be copied into a
    // hidl_vec eventually, the values of baseBlockIndices are zero-based
    // integer indices instead of list iterators.
    //
    // Note that the pointers can be raw because baseBlockIndices has a shorter
    // lifespan than all of base blocks.
    std::map<const void*, uint32_t> baseBlockIndices;

    d->works.resize(s.size());
    size_t i = 0;
    for (const std::unique_ptr<C2Work>& sWork : s) {
        Work &dWork = d->works[i++];
        if (!sWork) {
            LOG(WARNING) << "Null C2Work encountered.";
            continue;
        }

        // chain info is not in use currently.

        // input
        if (!objcpy(&dWork.input, sWork->input,
                bufferPoolSender, &baseBlocks, &baseBlockIndices)) {
            LOG(ERROR) << "Invalid C2Work::input.";
            return false;
        }

        // worklets
        if (sWork->worklets.size() == 0) {
            LOG(DEBUG) << "Work with no worklets.";
        } else {
            // Parcel the worklets.
            hidl_vec<Worklet> &dWorklets = dWork.worklets;
            dWorklets.resize(sWork->worklets.size());
            size_t j = 0;
            for (const std::unique_ptr<C2Worklet>& sWorklet : sWork->worklets)
            {
                if (!sWorklet) {
                    LOG(WARNING) << "Null C2Work::worklets["
                                 << j << "].";
                    continue;
                }
                Worklet &dWorklet = dWorklets[j++];

                // component id
                dWorklet.componentId = static_cast<uint32_t>(
                        sWorklet->component);

                // tunings
                if (!createParamsBlob(&dWorklet.tunings, sWorklet->tunings)) {
                    LOG(ERROR) << "Invalid C2Work::worklets["
                               << j - 1 << "]->tunings.";
                    return false;
                }

                // failures
                dWorklet.failures.resize(sWorklet->failures.size());
                size_t k = 0;
                for (const std::unique_ptr<C2SettingResult>& sFailure :
                        sWorklet->failures) {
                    if (!sFailure) {
                        LOG(WARNING) << "Null C2Work::worklets["
                                     << j - 1 << "]->failures["
                                     << k << "].";
                        continue;
                    }
                    if (!objcpy(&dWorklet.failures[k++], *sFailure)) {
                        LOG(ERROR) << "Invalid C2Work::worklets["
                                   << j - 1 << "]->failures["
                                   << k - 1 << "].";
                        return false;
                    }
                }

                // output
                if (!objcpy(&dWorklet.output, sWorklet->output,
                        bufferPoolSender, &baseBlocks, &baseBlockIndices)) {
                    LOG(ERROR) << "Invalid C2Work::worklets["
                               << j - 1 << "]->output.";
                    return false;
                }
            }
        }

        // worklets processed
        dWork.workletsProcessed = sWork->workletsProcessed;

        // result
        dWork.result = static_cast<Status>(sWork->result);
    }

    // Copy std::list<BaseBlock> to hidl_vec<BaseBlock>.
    {
        d->baseBlocks.resize(baseBlocks.size());
        size_t i = 0;
        for (const BaseBlock& baseBlock : baseBlocks) {
            d->baseBlocks[i++] = baseBlock;
        }
    }

    return true;
}

namespace /* unnamed */ {

struct C2BaseBlock {
    enum type_t {
        LINEAR,
        GRAPHIC,
    };
    type_t type;
    std::shared_ptr<C2LinearBlock> linear;
    std::shared_ptr<C2GraphicBlock> graphic;
};

// hidl_handle -> C2Fence
// Note: File descriptors are not duplicated. The original file descriptor must
// not be closed before the transaction is complete.
bool objcpy(C2Fence* d, const hidl_handle& s) {
    // TODO: Implement.
    (void)s;
    *d = C2Fence();
    return true;
}

// C2LinearBlock, vector<C2Param*>, C2Fence -> C2Buffer
bool createLinearBuffer(
        std::shared_ptr<C2Buffer>* buffer,
        const std::shared_ptr<C2LinearBlock>& block,
        const std::vector<C2Param*>& meta,
        const C2Fence& fence) {
    // Check the block meta. It should have exactly 1 C2Info:
    // C2Hidl_RangeInfo.
    if ((meta.size() != 1) || !meta[0]) {
        LOG(ERROR) << "Invalid C2LinearBlock::meta.";
        return false;
    }
    if (meta[0]->size() != sizeof(C2Hidl_RangeInfo)) {
        LOG(ERROR) << "Invalid range info in C2LinearBlock.";
        return false;
    }
    C2Hidl_RangeInfo *rangeInfo =
            reinterpret_cast<C2Hidl_RangeInfo*>(meta[0]);

    // Create C2Buffer from C2LinearBlock.
    *buffer = C2Buffer::CreateLinearBuffer(block->share(
            rangeInfo->offset, rangeInfo->length,
            fence));
    if (!(*buffer)) {
        LOG(ERROR) << "CreateLinearBuffer failed.";
        return false;
    }
    return true;
}

// C2GraphicBlock, vector<C2Param*>, C2Fence -> C2Buffer
bool createGraphicBuffer(
        std::shared_ptr<C2Buffer>* buffer,
        const std::shared_ptr<C2GraphicBlock>& block,
        const std::vector<C2Param*>& meta,
        const C2Fence& fence) {
    // Check the block meta. It should have exactly 1 C2Info:
    // C2Hidl_RectInfo.
    if ((meta.size() != 1) || !meta[0]) {
        LOG(ERROR) << "Invalid C2GraphicBlock::meta.";
        return false;
    }
    if (meta[0]->size() != sizeof(C2Hidl_RectInfo)) {
        LOG(ERROR) << "Invalid rect info in C2GraphicBlock.";
        return false;
    }
    C2Hidl_RectInfo *rectInfo =
            reinterpret_cast<C2Hidl_RectInfo*>(meta[0]);

    // Create C2Buffer from C2GraphicBlock.
    *buffer = C2Buffer::CreateGraphicBuffer(block->share(
            C2Rect(rectInfo->width, rectInfo->height).
            at(rectInfo->left, rectInfo->top),
            fence));
    if (!(*buffer)) {
        LOG(ERROR) << "CreateGraphicBuffer failed.";
        return false;
    }
    return true;
}

// Buffer -> C2Buffer
// Note: The native handles will be cloned.
bool objcpy(std::shared_ptr<C2Buffer>* d, const Buffer& s,
        const std::vector<C2BaseBlock>& baseBlocks) {
    *d = nullptr;

    // Currently, a non-null C2Buffer must contain exactly 1 block.
    if (s.blocks.size() == 0) {
        return true;
    } else if (s.blocks.size() != 1) {
        LOG(ERROR) << "Invalid Buffer: "
                      "Currently, a C2Buffer must contain exactly 1 block.";
        return false;
    }

    const Block &sBlock = s.blocks[0];
    if (sBlock.index >= baseBlocks.size()) {
        LOG(ERROR) << "Invalid Buffer::blocks[0].index: "
                      "Array index out of range.";
        return false;
    }
    const C2BaseBlock &baseBlock = baseBlocks[sBlock.index];

    // Parse meta.
    std::vector<C2Param*> sBlockMeta;
    if (!parseParamsBlob(&sBlockMeta, sBlock.meta)) {
        LOG(ERROR) << "Invalid Buffer::blocks[0].meta.";
        return false;
    }

    // Copy fence.
    C2Fence dFence;
    if (!objcpy(&dFence, sBlock.fence)) {
        LOG(ERROR) << "Invalid Buffer::blocks[0].fence.";
        return false;
    }

    // Construct a block.
    switch (baseBlock.type) {
    case C2BaseBlock::LINEAR:
        if (!createLinearBuffer(d, baseBlock.linear, sBlockMeta, dFence)) {
            LOG(ERROR) << "Invalid C2BaseBlock::linear.";
            return false;
        }
        break;
    case C2BaseBlock::GRAPHIC:
        if (!createGraphicBuffer(d, baseBlock.graphic, sBlockMeta, dFence)) {
            LOG(ERROR) << "Invalid C2BaseBlock::graphic.";
            return false;
        }
        break;
    default:
        LOG(ERROR) << "Invalid C2BaseBlock::type.";
        return false;
    }

    // Parse info
    std::vector<C2Param*> params;
    if (!parseParamsBlob(&params, s.info)) {
        LOG(ERROR) << "Invalid Buffer::info.";
        return false;
    }
    for (C2Param* param : params) {
        if (param == nullptr) {
            LOG(ERROR) << "Null param in Buffer::info.";
            return false;
        }
        std::shared_ptr<C2Param> c2param{
                C2Param::Copy(*param).release()};
        if (!c2param) {
            LOG(ERROR) << "Invalid param in Buffer::info.";
            return false;
        }
        c2_status_t status =
                (*d)->setInfo(std::static_pointer_cast<C2Info>(c2param));
        if (status != C2_OK) {
            LOG(ERROR) << "C2Buffer::setInfo failed.";
            return false;
        }
    }

    return true;
}

// FrameData -> C2FrameData
bool objcpy(C2FrameData* d, const FrameData& s,
        const std::vector<C2BaseBlock>& baseBlocks) {
    d->flags = static_cast<C2FrameData::flags_t>(s.flags);
    if (!objcpy(&d->ordinal, s.ordinal)) {
        LOG(ERROR) << "Invalid FrameData::ordinal.";
        return false;
    }
    d->buffers.clear();
    d->buffers.reserve(s.buffers.size());
    for (const Buffer& sBuffer : s.buffers) {
        std::shared_ptr<C2Buffer> dBuffer;
        if (!objcpy(&dBuffer, sBuffer, baseBlocks)) {
            LOG(ERROR) << "Invalid FrameData::buffers.";
            return false;
        }
        d->buffers.emplace_back(dBuffer);
    }

    std::vector<C2Param*> params;
    if (!parseParamsBlob(&params, s.configUpdate)) {
        LOG(ERROR) << "Invalid FrameData::configUpdate.";
        return false;
    }
    d->configUpdate.clear();
    for (C2Param* param : params) {
        d->configUpdate.emplace_back(C2Param::Copy(*param));
        if (!d->configUpdate.back()) {
            LOG(ERROR) << "Unexpected error while parsing "
                          "FrameData::configUpdate.";
            return false;
        }
    }

    // TODO: Implement this once C2InfoBuffer has constructors.
    d->infoBuffers.clear();
    return true;
}

// BaseBlock -> C2BaseBlock
bool objcpy(C2BaseBlock* d, const BaseBlock& s) {
    switch (s.getDiscriminator()) {
    case BaseBlock::hidl_discriminator::nativeBlock: {
            native_handle_t* sHandle =
                    native_handle_clone(s.nativeBlock());
            if (sHandle == nullptr) {
                LOG(ERROR) << "Null BaseBlock::nativeBlock.";
                return false;
            }
            const C2Handle *sC2Handle =
                    reinterpret_cast<const C2Handle*>(sHandle);

            d->linear = _C2BlockFactory::CreateLinearBlock(sC2Handle);
            if (d->linear) {
                d->type = C2BaseBlock::LINEAR;
                return true;
            }

            d->graphic = _C2BlockFactory::CreateGraphicBlock(sC2Handle);
            if (d->graphic) {
                d->type = C2BaseBlock::GRAPHIC;
                return true;
            }

            LOG(ERROR) << "Unknown handle type in BaseBlock::nativeBlock.";
            if (sHandle) {
                native_handle_close(sHandle);
                native_handle_delete(sHandle);
            }
            return false;
        }
    case BaseBlock::hidl_discriminator::pooledBlock: {
            const BufferStatusMessage &bpMessage =
                    s.pooledBlock();
            sp<ClientManager> bp = ClientManager::getInstance();
            std::shared_ptr<BufferPoolData> bpData;
            native_handle_t *cHandle;
            ResultStatus bpStatus = bp->receive(
                    bpMessage.connectionId,
                    bpMessage.transactionId,
                    bpMessage.bufferId,
                    bpMessage.timestampUs,
                    &cHandle,
                    &bpData);
            if (bpStatus != ResultStatus::OK) {
                LOG(ERROR) << "Failed to receive buffer from bufferpool -- "
                           << "resultStatus = " << underlying_value(bpStatus)
                           << ".";
                return false;
            } else if (!bpData) {
                LOG(ERROR) << "No data in bufferpool transaction.";
                return false;
            }

            d->linear = _C2BlockFactory::CreateLinearBlock(cHandle, bpData);
            if (d->linear) {
                d->type = C2BaseBlock::LINEAR;
                return true;
            }

            d->graphic = _C2BlockFactory::CreateGraphicBlock(cHandle, bpData);
            if (d->graphic) {
                d->type = C2BaseBlock::GRAPHIC;
                return true;
            }
            if (cHandle) {
                // Though we got cloned handle, creating block failed.
                native_handle_close(cHandle);
                native_handle_delete(cHandle);
            }

            LOG(ERROR) << "Unknown handle type in BaseBlock::pooledBlock.";
            return false;
        }
    default:
        LOG(ERROR) << "Unrecognized BaseBlock's discriminator with "
                   << "underlying value "
                   << underlying_value(s.getDiscriminator()) << ".";
        return false;
    }
}

} // unnamed namespace

// WorkBundle -> std::list<std::unique_ptr<C2Work>>
bool objcpy(std::list<std::unique_ptr<C2Work>>* d, const WorkBundle& s) {
    // Convert BaseBlocks to C2BaseBlocks.
    std::vector<C2BaseBlock> dBaseBlocks(s.baseBlocks.size());
    for (size_t i = 0; i < s.baseBlocks.size(); ++i) {
        if (!objcpy(&dBaseBlocks[i], s.baseBlocks[i])) {
            LOG(ERROR) << "Invalid WorkBundle::baseBlocks["
                       << i << "].";
            return false;
        }
    }

    d->clear();
    for (const Work& sWork : s.works) {
        d->emplace_back(std::make_unique<C2Work>());
        C2Work& dWork = *d->back();

        // chain info is not in use currently.

        // input
        if (!objcpy(&dWork.input, sWork.input, dBaseBlocks)) {
            LOG(ERROR) << "Invalid Work::input.";
            return false;
        }

        // worklet(s)
        dWork.worklets.clear();
        for (const Worklet& sWorklet : sWork.worklets) {
            std::unique_ptr<C2Worklet> dWorklet = std::make_unique<C2Worklet>();

            // component id
            dWorklet->component = static_cast<c2_node_id_t>(
                    sWorklet.componentId);

            // tunings
            if (!copyParamsFromBlob(&dWorklet->tunings, sWorklet.tunings)) {
                LOG(ERROR) << "Invalid Worklet::tunings";
                return false;
            }

            // failures
            dWorklet->failures.clear();
            dWorklet->failures.reserve(sWorklet.failures.size());
            for (const SettingResult& sFailure : sWorklet.failures) {
                std::unique_ptr<C2SettingResult> dFailure;
                if (!objcpy(&dFailure, sFailure)) {
                    LOG(ERROR) << "Invalid Worklet::failures.";
                    return false;
                }
                dWorklet->failures.emplace_back(std::move(dFailure));
            }

            // output
            if (!objcpy(&dWorklet->output, sWorklet.output, dBaseBlocks)) {
                LOG(ERROR) << "Invalid Worklet::output.";
                return false;
            }

            dWork.worklets.emplace_back(std::move(dWorklet));
        }

        // workletsProcessed
        dWork.workletsProcessed = sWork.workletsProcessed;

        // result
        dWork.result = static_cast<c2_status_t>(sWork.result);
    }

    return true;
}

constexpr size_t PARAMS_ALIGNMENT = 8;  // 64-bit alignment
static_assert(PARAMS_ALIGNMENT % alignof(C2Param) == 0, "C2Param alignment mismatch");
static_assert(PARAMS_ALIGNMENT % alignof(C2Info) == 0, "C2Param alignment mismatch");
static_assert(PARAMS_ALIGNMENT % alignof(C2Tuning) == 0, "C2Param alignment mismatch");

// Params -> std::vector<C2Param*>
bool parseParamsBlob(std::vector<C2Param*> *params, const hidl_vec<uint8_t> &blob) {
    // assuming blob is const here
    size_t size = blob.size();
    size_t ix = 0;
    const uint8_t *data = blob.data();
    C2Param *p = nullptr;

    do {
        p = C2ParamUtils::ParseFirst(data + ix, size - ix);
        if (p) {
            params->emplace_back(p);
            ix += p->size();
            ix = align(ix, PARAMS_ALIGNMENT);
        }
    } while (p);

    if (ix != size) {
        LOG(ERROR) << "parseParamsBlob -- inconsistent sizes.";
        return false;
    }
    return true;
}

namespace /* unnamed */ {

/**
 * Concatenates a list of C2Params into a params blob. T is a container type
 * whose member type is compatible with C2Param*.
 *
 * \param[out] blob target blob
 * \param[in] params parameters to concatenate
 * \retval C2_OK if the blob was successfully created
 * \retval C2_BAD_VALUE if the blob was not successful created (this only
 *         happens if the parameters were not const)
 */
template <typename T>
bool _createParamsBlob(hidl_vec<uint8_t> *blob, const T &params) {
    // assuming the parameter values are const
    size_t size = 0;
    for (const auto &p : params) {
        if (!p) {
            continue;
        }
        size += p->size();
        size = align(size, PARAMS_ALIGNMENT);
    }
    blob->resize(size);
    size_t ix = 0;
    for (const auto &p : params) {
        if (!p) {
            continue;
        }
        // NEVER overwrite even if param values (e.g. size) changed
        size_t paramSize = std::min(p->size(), size - ix);
        std::copy(
                reinterpret_cast<const uint8_t*>(&*p),
                reinterpret_cast<const uint8_t*>(&*p) + paramSize,
                &(*blob)[ix]);
        ix += paramSize;
        ix = align(ix, PARAMS_ALIGNMENT);
    }
    blob->resize(ix);
    if (ix != size) {
        LOG(ERROR) << "createParamsBlob -- inconsistent sizes.";
        return false;
    }
    return true;
}

/**
 * Parses a params blob and create a vector of new T objects that contain copies
 * of the params in the blob. T is C2Param or its compatible derived class.
 *
 * \param[out] params the resulting vector
 * \param[in] blob parameter blob to parse
 * \retval C2_OK if the full blob was parsed and params was constructed
 * \retval C2_BAD_VALUE otherwise
 */
template <typename T>
bool _copyParamsFromBlob(
        std::vector<std::unique_ptr<T>>* params,
        Params blob) {

    std::vector<C2Param*> paramPointers;
    if (!parseParamsBlob(&paramPointers, blob)) {
        LOG(ERROR) << "copyParamsFromBlob -- failed to parse.";
        return false;
    }

    params->resize(paramPointers.size());
    size_t i = 0;
    for (C2Param* const& paramPointer : paramPointers) {
        if (!paramPointer) {
            LOG(ERROR) << "copyParamsFromBlob -- null paramPointer.";
            return false;
        }
        (*params)[i++].reset(reinterpret_cast<T*>(
                C2Param::Copy(*paramPointer).release()));
    }
    return true;
}

} // unnamed namespace

// std::vector<const C2Param*> -> Params
bool createParamsBlob(
        hidl_vec<uint8_t> *blob,
        const std::vector<const C2Param*> &params) {
    return _createParamsBlob(blob, params);
}

// std::vector<C2Param*> -> Params
bool createParamsBlob(
        hidl_vec<uint8_t> *blob,
        const std::vector<C2Param*> &params) {
    return _createParamsBlob(blob, params);
}

// std::vector<std::unique_ptr<C2Param>> -> Params
bool createParamsBlob(
        hidl_vec<uint8_t> *blob,
        const std::vector<std::unique_ptr<C2Param>> &params) {
    return _createParamsBlob(blob, params);
}

// std::vector<std::unique_ptr<C2Tuning>> -> Params
bool createParamsBlob(
        hidl_vec<uint8_t> *blob,
        const std::vector<std::unique_ptr<C2Tuning>> &params) {
    return _createParamsBlob(blob, params);
}

// std::vector<std::shared_ptr<const C2Info>> -> Params
bool createParamsBlob(
        hidl_vec<uint8_t> *blob,
        const std::vector<std::shared_ptr<const C2Info>> &params) {
    return _createParamsBlob(blob, params);
}

// Params -> std::vector<std::unique_ptr<C2Param>>
bool copyParamsFromBlob(
        std::vector<std::unique_ptr<C2Param>>* params,
        Params blob) {
    return _copyParamsFromBlob(params, blob);
}

// Params -> std::vector<std::unique_ptr<C2Tuning>>
bool copyParamsFromBlob(
        std::vector<std::unique_ptr<C2Tuning>>* params,
        Params blob) {
    return _copyParamsFromBlob(params, blob);
}

// Params -> update std::vector<std::unique_ptr<C2Param>>
bool updateParamsFromBlob(
        const std::vector<C2Param*>& params,
        const Params& blob) {
    std::unordered_map<uint32_t, C2Param*> index2param;
    for (C2Param* const& param : params) {
        if (!param) {
            LOG(ERROR) << "updateParamsFromBlob -- null output param.";
            return false;
        }
        if (index2param.find(param->index()) == index2param.end()) {
            index2param.emplace(param->index(), param);
        }
    }

    std::vector<C2Param*> paramPointers;
    if (!parseParamsBlob(&paramPointers, blob)) {
        LOG(ERROR) << "updateParamsFromBlob -- failed to parse.";
        return false;
    }

    for (C2Param* const& paramPointer : paramPointers) {
        if (!paramPointer) {
            LOG(ERROR) << "updateParamsFromBlob -- null input param.";
            return false;
        }
        decltype(index2param)::iterator i = index2param.find(
                paramPointer->index());
        if (i == index2param.end()) {
            LOG(DEBUG) << "updateParamsFromBlob -- index "
                       << paramPointer->index() << " not found. Skipping...";
            continue;
        }
        if (!i->second->updateFrom(*paramPointer)) {
            LOG(ERROR) << "updateParamsFromBlob -- size mismatch: "
                       << params.size() << " vs " << paramPointer->size()
                       << " (index = " << i->first << ").";
            return false;
        }
    }
    return true;
}

// Convert BufferPool ResultStatus to c2_status_t.
c2_status_t toC2Status(ResultStatus rs) {
    switch (rs) {
    case ResultStatus::OK:
        return C2_OK;
    case ResultStatus::NO_MEMORY:
        return C2_NO_MEMORY;
    case ResultStatus::ALREADY_EXISTS:
        return C2_DUPLICATE;
    case ResultStatus::NOT_FOUND:
        return C2_NOT_FOUND;
    case ResultStatus::CRITICAL_ERROR:
        return C2_CORRUPTED;
    default:
        LOG(WARNING) << "Unrecognized BufferPool ResultStatus: "
                     << static_cast<int32_t>(rs) << ".";
        return C2_CORRUPTED;
    }
}

namespace /* unnamed */ {

template <typename BlockProcessor>
void forEachBlock(C2FrameData& frameData,
                  BlockProcessor process) {
    for (const std::shared_ptr<C2Buffer>& buffer : frameData.buffers) {
        if (buffer) {
            for (const C2ConstGraphicBlock& block :
                    buffer->data().graphicBlocks()) {
                process(block);
            }
        }
    }
}

template <typename BlockProcessor>
void forEachBlock(const std::list<std::unique_ptr<C2Work>>& workList,
                  BlockProcessor process,
                  bool processInput, bool processOutput) {
    for (const std::unique_ptr<C2Work>& work : workList) {
        if (!work) {
            continue;
        }
        if (processInput) {
            forEachBlock(work->input, process);
        }
        if (processOutput) {
            for (const std::unique_ptr<C2Worklet>& worklet : work->worklets) {
                if (worklet) {
                    forEachBlock(worklet->output,
                                 process);
                }
            }
        }
    }
}

} // unnamed namespace

bool beginTransferBufferQueueBlock(const C2ConstGraphicBlock& block) {
    std::shared_ptr<_C2BlockPoolData> data =
            _C2BlockFactory::GetGraphicBlockPoolData(block);
    if (data && _C2BlockFactory::GetBufferQueueData(data)) {
        _C2BlockFactory::BeginTransferBlockToClient(data);
        return true;
    }
    return false;
}

void beginTransferBufferQueueBlocks(
        const std::list<std::unique_ptr<C2Work>>& workList,
        bool processInput, bool processOutput) {
    forEachBlock(workList, beginTransferBufferQueueBlock,
                 processInput, processOutput);
}

bool endTransferBufferQueueBlock(
        const C2ConstGraphicBlock& block,
        bool transfer) {
    std::shared_ptr<_C2BlockPoolData> data =
            _C2BlockFactory::GetGraphicBlockPoolData(block);
    if (data && _C2BlockFactory::GetBufferQueueData(data)) {
        _C2BlockFactory::EndTransferBlockToClient(data, transfer);
        return true;
    }
    return false;
}

void endTransferBufferQueueBlocks(
        const std::list<std::unique_ptr<C2Work>>& workList,
        bool transfer,
        bool processInput, bool processOutput) {
    forEachBlock(workList,
                 std::bind(endTransferBufferQueueBlock,
                           std::placeholders::_1, transfer),
                 processInput, processOutput);
}

bool displayBufferQueueBlock(const C2ConstGraphicBlock& block) {
    std::shared_ptr<_C2BlockPoolData> data =
            _C2BlockFactory::GetGraphicBlockPoolData(block);
    if (data && _C2BlockFactory::GetBufferQueueData(data)) {
        _C2BlockFactory::DisplayBlockToBufferQueue(data);
        return true;
    }
    return false;
}

}  // namespace utils
}  // namespace V1_0
}  // namespace c2
}  // namespace media
}  // namespace hardware
}  // namespace android

