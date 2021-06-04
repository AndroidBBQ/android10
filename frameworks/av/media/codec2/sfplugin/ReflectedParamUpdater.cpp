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

//#define LOG_NDEBUG 0
#define LOG_TAG "ReflectedParamUpdater"
#include <utils/Log.h>

#include <iostream>
#include <set>
#include <sstream>

#include <C2Debug.h>
#include <C2ParamInternal.h>

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/foundation/hexdump.h>

#include "ReflectedParamUpdater.h"

namespace android {

std::string ReflectedParamUpdater::Dict::debugString(size_t indent_) const {
    std::string indent(indent_, ' ');
    std::stringstream s;
    s << "Dict {" << std::endl;

    for (const auto &it : *this) {
        s << indent << "  ";

        C2Value c2Value;
        int32_t int32Value;
        uint32_t uint32Value;
        int64_t int64Value;
        uint64_t uint64Value;
        float floatValue;
        sp<ABuffer> bufValue;
        AString strValue;
        if (it.second.find(&c2Value)) {
            switch (c2Value.type()) {
                case C2Value::INT32:
                    (void)c2Value.get(&int32Value);
                    s << "c2::i32 " << it.first << " = " << int32Value;
                    break;
                case C2Value::UINT32:
                    (void)c2Value.get(&uint32Value);
                    s << "c2::u32 " << it.first << " = " << uint32Value;
                    break;
                case C2Value::CNTR32:
                    // dump counter value as unsigned
                    (void)c2Value.get((c2_cntr32_t*)&uint32Value);
                    s << "c2::c32 " << it.first << " = " << uint32Value;
                    break;
                case C2Value::INT64:
                    (void)c2Value.get(&int64Value);
                    s << "c2::i64 " << it.first << " = " << int64Value;
                    break;
                case C2Value::UINT64:
                    (void)c2Value.get(&uint64Value);
                    s << "c2::u64 " << it.first << " = " << uint64Value;
                    break;
                case C2Value::CNTR64:
                    // dump counter value as unsigned
                    (void)c2Value.get((c2_cntr64_t*)&uint64Value);
                    s << "c2::c64 " << it.first << " = " << uint64Value;
                    break;
                case C2Value::FLOAT:
                    (void)c2Value.get(&floatValue);
                    s << "c2::float " << it.first << " = " << floatValue;
                    break;
                default:
                    // dump unsupported values for debugging, these should not be used
                    s << "c2::unsupported " << it.first;
            }
        } else if (it.second.find(&int32Value)) {
            s << "int32_t " << it.first << " = " << int32Value;
        } else if (it.second.find(&int64Value)) {
            s << "int64_t " << it.first << " = " << int64Value;
        } else if (it.second.find(&strValue)) {
            s << "string " << it.first << " = \"" << strValue.c_str() << "\"";
        } else if (it.second.find(&bufValue)) {
            s << "Buffer " << it.first << " = ";
            if (bufValue != nullptr && bufValue->data() != nullptr && bufValue->size() <= 64) {
                s << "{" << std::endl;
                AString tmp;
                hexdump(bufValue->data(), bufValue->size(), indent_ + 4, &tmp);
                s << tmp.c_str() << indent << "  }";
            } else {
                s << (void*)bufValue.get();
            }
        } else {
            // dump unsupported values for debugging, this should never happen.
            s << "unsupported " << it.first;
        }
        s << std::endl;
    }
    s << indent << "}";

    return s.str();
}

void ReflectedParamUpdater::addParamDesc(
        const std::shared_ptr<C2ParamReflector> &reflector,
        const std::vector<std::shared_ptr<C2ParamDescriptor>> &paramDescs) {
    for (const std::shared_ptr<C2ParamDescriptor> &desc : paramDescs) {
        std::unique_ptr<C2StructDescriptor> structDesc = reflector->describe(
                desc->index().coreIndex());
        if (structDesc == nullptr) {
            ALOGD("Could not describe %s", desc->name().c_str());
            continue;
        }
        addParamDesc(desc, *structDesc, reflector, true /* markVendor */);
    }

    // TEMP: also add vendor parameters as non-vendor
    for (const std::shared_ptr<C2ParamDescriptor> &desc : paramDescs) {
        if (!desc->index().isVendor()) {
            continue;
        }
        std::unique_ptr<C2StructDescriptor> structDesc = reflector->describe(
                desc->index().coreIndex());
        if (structDesc) {
            addParamDesc(desc, *structDesc, reflector, false /* markVendor */);
        }
    }
}

void ReflectedParamUpdater::addParamStructDesc(
        std::shared_ptr<C2ParamDescriptor> desc,
        C2String path,
        size_t offset,
        const C2StructDescriptor &structDesc,
        const std::shared_ptr<C2ParamReflector> &reflector) {
    for (auto it = structDesc.begin(); it != structDesc.end(); ++it) {
        C2String fieldName = path + "." + it->name();
        if (it->type() & C2FieldDescriptor::STRUCT_FLAG) {
            if (reflector == nullptr || it->extent() != 1) {
                ALOGD("ignored struct field %s", fieldName.c_str());
                continue;
            }
            std::unique_ptr<C2StructDescriptor> structDesc_ = reflector->describe(
                    C2Param::CoreIndex(it->type()).coreIndex());
            if (structDesc_ == nullptr) {
                ALOGD("Could not describe structure of %s", fieldName.c_str());
                continue;
            }
            addParamStructDesc(desc, fieldName, offset + _C2ParamInspector::GetOffset(*it),
                               *structDesc_, reflector);
            continue;
        }

        // verify extent and type
        switch (it->type()) {
            case C2FieldDescriptor::INT32:
            case C2FieldDescriptor::UINT32:
            case C2FieldDescriptor::CNTR32:
            case C2FieldDescriptor::INT64:
            case C2FieldDescriptor::UINT64:
            case C2FieldDescriptor::CNTR64:
            case C2FieldDescriptor::FLOAT:
                if (it->extent() != 1) {
                    ALOGD("extent() != 1 for single value type: %s", fieldName.c_str());
                    continue;
                }
                break;
            case C2FieldDescriptor::STRING:
            case C2FieldDescriptor::BLOB:
                break;

            default:
                ALOGD("Unrecognized type: %s", fieldName.c_str());
                continue;
        }

        ALOGV("%s registered", fieldName.c_str());
        // TODO: get the proper size by iterating through the fields.
        // only insert fields the very first time
        mMap.emplace(fieldName, FieldDesc {
            desc,
            std::make_unique<C2FieldDescriptor>(
                    it->type(), it->extent(), it->name(),
                    _C2ParamInspector::GetOffset(*it),
                    _C2ParamInspector::GetSize(*it)),
            offset,
        });
    }
}

void ReflectedParamUpdater::addParamDesc(
        std::shared_ptr<C2ParamDescriptor> desc, const C2StructDescriptor &structDesc,
        const std::shared_ptr<C2ParamReflector> &reflector, bool markVendor) {
    C2String paramName = desc->name();

    // Do not reflect requested parameters
    // TODO: split these once aliases are introduced into '.actual' and '.requested' and alias
    // the name to '.actual'.
    if (desc->index() & C2Param::CoreIndex::IS_REQUEST_FLAG) {
        return;
    }

    // prefix vendor parameters
    if (desc->index().isVendor() && markVendor) {
        paramName = "vendor." + paramName;
    }
    mParamNames.emplace(desc->index(), paramName);

    // also allow setting whole parameters in a binary fashion via ByteBuffer
    // this is opt-in for now
    auto it = mWholeParams.find(paramName);
    if (it != mWholeParams.end() && it->second.coreIndex() == desc->index().coreIndex()) {
        mMap.emplace(paramName, FieldDesc{ desc, nullptr, 0 /* offset */ });
        // don't add fields of whole parameters.
        return;
    }

    addParamStructDesc(desc, paramName, 0 /* offset */, structDesc, reflector);
}

void ReflectedParamUpdater::supportWholeParam(std::string name, C2Param::CoreIndex index) {
    mWholeParams.emplace(name, index);
}

std::string ReflectedParamUpdater::getParamName(C2Param::Index index) const {
    auto it = mParamNames.find(index);
    if (it != mParamNames.end()) {
        return it->second;
    }

    std::stringstream ret;
    ret << "<unknown " << index << ">";
    return ret.str();
}

void ReflectedParamUpdater::getParamIndicesFromMessage(
        const Dict &params,
        std::vector<C2Param::Index> *vec /* nonnull */) const {
    CHECK(vec != nullptr);
    vec->clear();
    std::set<C2Param::Index> indices;
    parseMessageAndDoWork(
            params,
            [&indices](const std::string &, const FieldDesc &desc, const void *, size_t) {
                indices.insert(desc.paramDesc->index());
            });
    for (const C2Param::Index &index : indices) {
        vec->push_back(index);
    }
}

void ReflectedParamUpdater::getParamIndicesForKeys(
        const std::vector<std::string> &keys,
        std::vector<C2Param::Index> *vec /* nonnull */) const {
    CHECK(vec != nullptr);
    vec->clear();
    std::set<C2Param::Index> indices;

    std::set<std::string> keyMap(keys.begin(), keys.end());

    ALOGV("in getParamIndicesForKeys with %zu keys and map of %zu entries",
            keyMap.size(), mMap.size());
    for (const std::pair<const std::string, FieldDesc> &kv : mMap) {
        const std::string &name = kv.first;
        const FieldDesc &desc = kv.second;
        ALOGV("count of %s is %zu", name.c_str(), keyMap.count(name));
        if (keyMap.count(name) > 0) {
            indices.insert(desc.paramDesc->index());
        }
    }

    for (const C2Param::Index &index : indices) {
        vec->push_back(index);
    }
}

void ReflectedParamUpdater::updateParamsFromMessage(
        const Dict &params,
        std::vector<std::unique_ptr<C2Param>> *vec /* nonnull */) const {
    CHECK(vec != nullptr);

    std::map<C2Param::Index, std::unique_ptr<C2Param>*> paramsMap;
    for (std::unique_ptr<C2Param> &param : *vec) {
        if (param && *param) {
            paramsMap[param->index()] = &param;
        }
    }

    parseMessageAndDoWork(
            params,
            [&paramsMap](const std::string &name, const FieldDesc &desc, const void *ptr, size_t size) {
                std::unique_ptr<C2Param> *param = nullptr;
                auto paramIt = paramsMap.find(desc.paramDesc->index());
                if (paramIt == paramsMap.end()) {
                    ALOGD("%s found, but param #%d isn't present to update",
                            name.c_str(), (int32_t)desc.paramDesc->index());
                    return;
                }
                param = paramIt->second;

                struct _C2Param : public C2Param {
                    using C2Param::C2Param;
                    _C2Param(uint32_t size, uint32_t index) : C2Param(size, index) { }
                };

                // we will handle whole param updates as part of a flexible param update using
                // a zero offset.
                size_t offset = 0;
                size_t minOffset = 0;

                // if this descriptor has a field, use the offset and size and ensure that offset
                // is not part of the header
                if (desc.fieldDesc) {
                    minOffset = sizeof(C2Param);
                    offset = sizeof(C2Param) + desc.offset
                            + _C2ParamInspector::GetOffset(*desc.fieldDesc);
                }

                // reallocate or trim flexible param (or whole param) as necessary
                if (!desc.fieldDesc /* whole param */ || desc.fieldDesc->extent() == 0) {
                    // reallocate param if more space is needed
                    if (param->get()->size() < offset + size) {
                        if (size > INT32_MAX - offset || offset < minOffset) {
                            // size too long or offset too early - abandon
                            return;
                        }
                        C2Param *newParam = (C2Param *)::operator new(offset + size);
                        new (newParam) _C2Param(offset + size, param->get()->index());
                        if (offset > sizeof(C2Param)) {
                            memcpy(newParam + 1, param->get() + 1, offset - sizeof(C2Param));
                        }
                        param->reset(newParam);
                    } else if (param->get()->size() > offset + size) {
                        // trim parameter size
                        _C2ParamInspector::TrimParam(param->get(), offset + size);
                    }
                } else if (desc.fieldDesc->type() == C2FieldDescriptor::BLOB) {
                    // zero fill blobs if updating with smaller blob
                    if (desc.fieldDesc->extent() > size) {
                        memset((uint8_t *)(param->get()) + offset + size, 0,
                               desc.fieldDesc->extent() - size);
                    }
                }

                memcpy((uint8_t *)(param->get()) + offset, ptr, size);
            });
}

void ReflectedParamUpdater::parseMessageAndDoWork(
        const Dict &params,
        std::function<void(const std::string &, const FieldDesc &, const void *, size_t)> work) const {
    for (const std::pair<const std::string, FieldDesc> &kv : mMap) {
        const std::string &name = kv.first;
        const FieldDesc &desc = kv.second;
        auto param = params.find(name);
        if (param == params.end()) {
            continue;
        }

        // handle whole parameters
        if (!desc.fieldDesc) {
            sp<ABuffer> tmp;
            if (param->second.find(&tmp) && tmp != nullptr) {
                C2Param *tmpAsParam = C2Param::From(tmp->data(), tmp->size());
                if (tmpAsParam && tmpAsParam->type().type() == desc.paramDesc->index().type()) {
                    work(name, desc, tmp->data(), tmp->size());
                } else {
                    ALOGD("Param blob does not match param for '%s' (%p, %x vs %x)",
                            name.c_str(), tmpAsParam, tmpAsParam ? tmpAsParam->type().type() : 0xDEADu,
                            desc.paramDesc->index().type());
                }
            }
            continue;
        }

        int32_t int32Value;
        int64_t int64Value;
        C2Value c2Value;

        C2FieldDescriptor::type_t fieldType = desc.fieldDesc->type();
        size_t fieldExtent = desc.fieldDesc->extent();
        switch (fieldType) {
            case C2FieldDescriptor::INT32:
                if ((param->second.find(&c2Value) && c2Value.get(&int32Value))
                        || param->second.find(&int32Value)) {
                    work(name, desc, &int32Value, sizeof(int32Value));
                }
                break;
            case C2FieldDescriptor::UINT32:
                if ((param->second.find(&c2Value) && c2Value.get((uint32_t*)&int32Value))
                        || param->second.find(&int32Value)) {
                    work(name, desc, &int32Value, sizeof(int32Value));
                }
                break;
            case C2FieldDescriptor::CNTR32:
                if ((param->second.find(&c2Value) && c2Value.get((c2_cntr32_t*)&int32Value))
                        || param->second.find(&int32Value)) {
                    work(name, desc, &int32Value, sizeof(int32Value));
                }
                break;
            case C2FieldDescriptor::INT64:
                if ((param->second.find(&c2Value) && c2Value.get(&int64Value))
                        || param->second.find(&int64Value)) {
                    work(name, desc, &int64Value, sizeof(int64Value));
                }
                break;
            case C2FieldDescriptor::UINT64:
                if ((param->second.find(&c2Value) && c2Value.get((uint64_t*)&int64Value))
                        || param->second.find(&int64Value)) {
                    work(name, desc, &int64Value, sizeof(int64Value));
                }
                break;
            case C2FieldDescriptor::CNTR64:
                if ((param->second.find(&c2Value) && c2Value.get((c2_cntr64_t*)&int64Value))
                        || param->second.find(&int64Value)) {
                    work(name, desc, &int64Value, sizeof(int64Value));
                }
                break;
            case C2FieldDescriptor::FLOAT: {
                float tmp;
                if (param->second.find(&c2Value) && c2Value.get(&tmp)) {
                    work(name, desc, &tmp, sizeof(tmp));
                }
                break;
            }
            case C2FieldDescriptor::STRING: {
                AString tmp;
                if (!param->second.find(&tmp)) {
                    break;
                }
                if (fieldExtent > 0 && tmp.size() >= fieldExtent) {
                    AString truncated(tmp, 0, fieldExtent - 1);
                    ALOGD("String value too long to fit: original \"%s\" truncated to \"%s\"",
                            tmp.c_str(), truncated.c_str());
                    tmp = truncated;
                }
                work(name, desc, tmp.c_str(), tmp.size() + 1);
                break;
            }

            case C2FieldDescriptor::BLOB: {
                sp<ABuffer> tmp;
                if (!param->second.find(&tmp) || tmp == nullptr) {
                    break;
                }

                if (fieldExtent > 0 && tmp->size() > fieldExtent) {
                    ALOGD("Blob value too long to fit. Truncating.");
                    tmp->setRange(tmp->offset(), fieldExtent);
                }
                work(name, desc, tmp->data(), tmp->size());
                break;
            }

            default:
                ALOGD("Unsupported data type for %s", name.c_str());
                break;
        }
    }
}

ReflectedParamUpdater::Dict
ReflectedParamUpdater::getParams(const std::vector<std::unique_ptr<C2Param>> &params_) const {
    std::vector<C2Param*> params;
    params.resize(params_.size());
    std::transform(params_.begin(), params_.end(), params.begin(),
                   [](const std::unique_ptr<C2Param>& p) -> C2Param* { return p.get(); });
    return getParams(params);
}

ReflectedParamUpdater::Dict
ReflectedParamUpdater::getParams(const std::vector<C2Param*> &params) const {
    Dict ret;

    // convert vector to map
    std::map<C2Param::Index, C2Param *> paramsMap;
    for (C2Param *param : params) {
        if (param != nullptr && *param) {
            paramsMap[param->index()] = param;
        }
    }

    for (const std::pair<const std::string, FieldDesc> &kv : mMap) {
        const std::string &name = kv.first;
        const FieldDesc &desc = kv.second;
        if (paramsMap.count(desc.paramDesc->index()) == 0) {
            continue;
        }
        C2Param *param = paramsMap[desc.paramDesc->index()];
        Value value;

        // handle whole params first
        if (!desc.fieldDesc) {
            sp<ABuffer> buf = ABuffer::CreateAsCopy(param, param->size());
            value.set(buf);
            ret.emplace(name, value);
            continue;
        }

        size_t offset = sizeof(C2Param) + desc.offset
                + _C2ParamInspector::GetOffset(*desc.fieldDesc);
        uint8_t *data = (uint8_t *)param + offset;
        C2FieldDescriptor::type_t fieldType = desc.fieldDesc->type();
        switch (fieldType) {
            case C2FieldDescriptor::STRING: {
                size_t length = desc.fieldDesc->extent();
                if (length == 0) {
                    length = param->size() - offset;
                }

                if (param->size() < length || param->size() - length < offset) {
                    ALOGD("param too small for string: length %zu size %zu offset %zu",
                            length, param->size(), offset);
                    break;
                }
                value.set(AString((char *)data, strnlen((char *)data, length)));
                break;
            }

            case C2FieldDescriptor::BLOB: {
                size_t length = desc.fieldDesc->extent();
                if (length == 0) {
                    length = param->size() - offset;
                }

                if (param->size() < length || param->size() - length < offset) {
                    ALOGD("param too small for blob: length %zu size %zu offset %zu",
                            length, param->size(), offset);
                    break;
                }

                sp<ABuffer> buf = ABuffer::CreateAsCopy(data, length);
                value.set(buf);
                break;
            }

            default: {
                size_t valueSize = C2Value::SizeFor((C2Value::type_t)fieldType);
                if (param->size() < valueSize || param->size() - valueSize < offset) {
                    ALOGD("param too small for c2value: size %zu offset %zu",
                            param->size(), offset);
                    break;
                }

                C2Value c2Value;
                switch (fieldType) {
                    case C2FieldDescriptor::INT32:  c2Value = *((int32_t *)data); break;
                    case C2FieldDescriptor::UINT32: c2Value = *((uint32_t *)data); break;
                    case C2FieldDescriptor::CNTR32: c2Value = *((c2_cntr32_t *)data); break;
                    case C2FieldDescriptor::INT64:  c2Value = *((int64_t *)data); break;
                    case C2FieldDescriptor::UINT64: c2Value = *((uint64_t *)data); break;
                    case C2FieldDescriptor::CNTR64: c2Value = *((c2_cntr64_t *)data); break;
                    case C2FieldDescriptor::FLOAT:  c2Value = *((float *)data); break;
                    default:
                        ALOGD("Unsupported data type for %s", name.c_str());
                        continue;
                }
                value.set(c2Value);
            }
        }
        ret.emplace(name, value);
    }
    return ret;
}

void ReflectedParamUpdater::clear() {
    mMap.clear();
}

}  // namespace android
