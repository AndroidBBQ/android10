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

#include <C2ParamInternal.h>
#include <util/C2Debug-interface.h>
#include <util/C2Debug-param.h>
#include <util/C2InterfaceUtils.h>

#include <iostream>

#include <android-base/stringprintf.h>

using android::base::StringPrintf;

/* -------------------------------- asString -------------------------------- */

const char *asString(c2_status_t i, const char *def) {
    switch (i) {
        case C2_OK:        return "OK";
        case C2_BAD_VALUE: return "BAD_VALUE";
        case C2_BAD_INDEX: return "BAD_INDEX";
        case C2_CANNOT_DO: return "CANNOT_DO";
        case C2_DUPLICATE: return "DUPLICATE";
        case C2_NOT_FOUND: return "NOT_FOUND";
        case C2_BAD_STATE: return "BAD_STATE";
        case C2_BLOCKING:  return "BLOCKING";
        case C2_CANCELED:  return "CANCELED";
        case C2_NO_MEMORY: return "NO_MEMORY";
        case C2_REFUSED:   return "REFUSED";
        case C2_TIMED_OUT: return "TIMED_OUT";
        case C2_OMITTED:   return "OMITTED";
        case C2_CORRUPTED: return "CORRUPTED";
        case C2_NO_INIT:   return "NO_INIT";
        default:           return def;
    }
}

const char *asString(C2FieldDescriptor::type_t i, const char *def) {
    switch (i) {
        case C2FieldDescriptor::BLOB:   return "u8";
        case C2FieldDescriptor::CNTR32: return "c32";
        case C2FieldDescriptor::CNTR64: return "c64";
        case C2FieldDescriptor::FLOAT:  return "fp";
        case C2FieldDescriptor::INT32:  return "i32";
        case C2FieldDescriptor::INT64:  return "i64";
        case C2FieldDescriptor::STRING: return "chr";
        case C2FieldDescriptor::UINT32: return "u32";
        case C2FieldDescriptor::UINT64: return "u64";
        default: return (i & C2FieldDescriptor::STRUCT_FLAG) ? "struct" : def;
    }
}

/* ------------------------------ C2ParamField ------------------------------ */

static std::string attribParamCoreIndex(const C2Param::CoreIndex &i) {
    return StringPrintf("%c%c%03x",
            i.isFlexible() ? 'F' : '-',
            i.isVendor() ? 'V' : '-',
            i.coreIndex());
}

static std::string attribParamIndex(
        const C2Param::Type &i, bool addStream, unsigned streamId) {
    std::string v = StringPrintf("%c%c",
            i.forInput() ? 'I' : i.forOutput() ? 'O' : '-',
            i.forStream() ? 'S' : i.forPort() ? 'P' : 'G');
    if (addStream) {
        if (i.forStream()) {
            v += StringPrintf("%02d", streamId);
        } else {
            v += "--";
        }
    }

    return v
            + StringPrintf("%c ",
                       i.kind() == C2Param::STRUCT  ? 'S' :
                       i.kind() == C2Param::INFO    ? 'i' :
                       i.kind() == C2Param::TUNING  ? 't' :
                       i.kind() == C2Param::SETTING ? 's' :
                       i.kind() == C2Param::NONE    ? '-' : '?')
            + attribParamCoreIndex(i);
}

std::ostream& operator<<(std::ostream& os, const C2Param::CoreIndex &i) {
    return os << "Param::CoreIndex(" << attribParamCoreIndex(i) << ")";
}

std::ostream& operator<<(std::ostream& os, const C2Param::Type &i) {
    return os << StringPrintf("Param::Type(%08x: ", i.type())
            << attribParamIndex(i, false, 0) << ")";
}

std::ostream& operator<<(std::ostream& os, const C2Param::Index &i) {
    return os << StringPrintf("Param::Index(%08x: ", (uint32_t)i)
            << attribParamIndex(i, true, i.stream()) << ")";
}

static std::string attribFieldId(const _C2FieldId &i) {
    return StringPrintf("Field(@%02x+%02x)",
            _C2ParamInspector::GetOffset(i),
            _C2ParamInspector::GetSize(i));
}


std::ostream& operator<<(std::ostream& os, const _C2FieldId &i) {
    return os << "<" << attribFieldId(i) << ">";
}


std::ostream& operator<<(std::ostream& os, const C2FieldDescriptor &i) {
    os << attribFieldId(_C2ParamInspector::GetField(i)) << " ";
    if (i.namedValues().size()) {
        os << "enum ";
    }
    return os << asString(i.type()) << " " << i.name()
            << StringPrintf("[%zu]", i.extent());
}


std::ostream& operator<<(std::ostream& os, const C2ParamField &i) {
    os << "<" << C2Param::Index(_C2ParamInspector::GetIndex(i))
            << StringPrintf("::Field(@%02x+%02x)>",
                            _C2ParamInspector::GetOffset(i),
                            _C2ParamInspector::GetSize(i));
    return os;
}


/* -------------------------- _C2FieldValueHelper -------------------------- */

std::ostream& _C2FieldValueHelper<char>::put(std::ostream &os, const C2Value::Primitive &p) {
    if (isprint(p.i32)) {
        return os << StringPrintf("'%c'", p.i32);
    } else {
        return os << StringPrintf("'\\x%02x'", (uint32_t)p.i32);
    }
}

std::ostream& _C2FieldValueHelper<uint8_t>::put(std::ostream &os, const C2Value::Primitive &p) {
    return os << StringPrintf("0x%02x", p.u32);
}

/* ---------------------- C2FieldSupportedValuesHelper ---------------------- */

template<typename T>
std::ostream& operator<<(std::ostream &os, const c2_cntr_t<T> &v) {
    return os << "ctr(" << v.peeku() << ")";
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const C2SupportedRange<T> &i) {
    os << "Range(";
    _C2FieldValueHelper<T>::put(os, i.min());
    os << "..";
    _C2FieldValueHelper<T>::put(os, i.max());
    os << " *= " << i.num() << " /= " << i.denom() << " += " << i.step() << ")";
    return os;
}
template std::ostream& operator<<(std::ostream& os, const C2SupportedRange<char> &i);
template std::ostream& operator<<(std::ostream& os, const C2SupportedRange<uint8_t> &i);
template std::ostream& operator<<(std::ostream& os, const C2SupportedRange<int32_t> &i);
template std::ostream& operator<<(std::ostream& os, const C2SupportedRange<uint32_t> &i);
//template std::ostream& operator<<(std::ostream& os, const C2SupportedRange<c2_cntr32_t> &i);
template std::ostream& operator<<(std::ostream& os, const C2SupportedRange<int64_t> &i);
template std::ostream& operator<<(std::ostream& os, const C2SupportedRange<uint64_t> &i);
//template std::ostream& operator<<(std::ostream& os, const C2SupportedRange<c2_cntr64_t> &i);
template std::ostream& operator<<(std::ostream& os, const C2SupportedRange<float> &i);

template<typename T>
std::ostream& operator<<(std::ostream& os, const C2SupportedFlags<T> &i) {
    os << "Flags[";
    if (!i.isEmpty()) {
        os << "min=";
        _C2FieldValueHelper<T>::put(os, i.min());
    }
    bool comma = false;
    for (const T &v : i.flags()) {
        if (comma) {
            os << ", ";
        }
        _C2FieldValueHelper<T>::put(os, v);
        comma = true;
    }
    os << "]";
    return os;
}
template std::ostream& operator<<(std::ostream& os, const C2SupportedFlags<char> &i);
template std::ostream& operator<<(std::ostream& os, const C2SupportedFlags<uint8_t> &i);
template std::ostream& operator<<(std::ostream& os, const C2SupportedFlags<int32_t> &i);
template std::ostream& operator<<(std::ostream& os, const C2SupportedFlags<uint32_t> &i);
//template std::ostream& operator<<(std::ostream& os, const C2SupportedFlags<c2_cntr32_t> &i);
template std::ostream& operator<<(std::ostream& os, const C2SupportedFlags<int64_t> &i);
template std::ostream& operator<<(std::ostream& os, const C2SupportedFlags<uint64_t> &i);
//template std::ostream& operator<<(std::ostream& os, const C2SupportedFlags<c2_cntr64_t> &i);
template std::ostream& operator<<(std::ostream& os, const C2SupportedFlags<float> &i);

template<typename T>
std::ostream& operator<<(std::ostream& os, const C2SupportedValueSet<T> &i) {
    os << "Values[";
    bool comma = false;
    for (const T &v : i.values()) {
        if (comma) {
            os << ", ";
        }
        _C2FieldValueHelper<T>::put(os, v);
        comma = true;
    }
    os << "]";
    return os;
}
template std::ostream& operator<<(std::ostream& os, const C2SupportedValueSet<char> &i);
template std::ostream& operator<<(std::ostream& os, const C2SupportedValueSet<uint8_t> &i);
template std::ostream& operator<<(std::ostream& os, const C2SupportedValueSet<int32_t> &i);
template std::ostream& operator<<(std::ostream& os, const C2SupportedValueSet<uint32_t> &i);
//template std::ostream& operator<<(std::ostream& os, const C2SupportedValueSet<c2_cntr32_t> &i);
template std::ostream& operator<<(std::ostream& os, const C2SupportedValueSet<int64_t> &i);
template std::ostream& operator<<(std::ostream& os, const C2SupportedValueSet<uint64_t> &i);
//template std::ostream& operator<<(std::ostream& os, const C2SupportedValueSet<c2_cntr64_t> &i);
template std::ostream& operator<<(std::ostream& os, const C2SupportedValueSet<float> &i);

template<typename T>
struct C2FieldSupportedValuesHelper<T>::Impl {
    Impl(const C2FieldSupportedValues &values);

private:
    typedef typename _C2FieldValueHelper<T>::ValueType ValueType;
    C2FieldSupportedValues::type_t _mType;
    C2SupportedRange<ValueType> _mRange;
    C2SupportedValueSet<ValueType> _mValues;
    C2SupportedFlags<ValueType> _mFlags;

public:
//    friend std::ostream& operator<< <T>(std::ostream& os, const C2FieldSupportedValuesHelper<T>::Impl &i);
//    friend std::ostream& operator<<(std::ostream& os, const Impl &i);
    std::ostream& streamOut(std::ostream& os) const;
};

template<typename T>
std::ostream& C2FieldSupportedValuesHelper<T>::Impl::streamOut(std::ostream& os) const {
    if (_mType == C2FieldSupportedValues::RANGE) {
        os << _mRange;
    } else if (_mType == C2FieldSupportedValues::VALUES) {
        os << _mValues;
    } else if (_mType == C2FieldSupportedValues::FLAGS) {
        os << _mFlags;
    } else {
        os << "Unknown FSV type: " << (uint32_t)_mType;
    }
    return os;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const C2FieldSupportedValuesHelper<T> &i) {
    return i._mImpl->streamOut(os);
}
template std::ostream& operator<<(std::ostream& os, const C2FieldSupportedValuesHelper<char> &i);
template std::ostream& operator<<(std::ostream& os, const C2FieldSupportedValuesHelper<uint8_t> &i);
template std::ostream& operator<<(std::ostream& os, const C2FieldSupportedValuesHelper<int32_t> &i);
template std::ostream& operator<<(std::ostream& os, const C2FieldSupportedValuesHelper<uint32_t> &i);
//template std::ostream& operator<<(std::ostream& os, const C2FieldSupportedValuesHelper<c2_cntr32_t> &i);
template std::ostream& operator<<(std::ostream& os, const C2FieldSupportedValuesHelper<int64_t> &i);
template std::ostream& operator<<(std::ostream& os, const C2FieldSupportedValuesHelper<uint64_t> &i);
//template std::ostream& operator<<(std::ostream& os, const C2FieldSupportedValuesHelper<c2_cntr64_t> &i);
template std::ostream& operator<<(std::ostream& os, const C2FieldSupportedValuesHelper<float> &i);

