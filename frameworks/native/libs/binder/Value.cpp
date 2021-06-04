/*
 * Copyright (C) 2015 The Android Open Source Project
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

#define LOG_TAG "Value"

#include <binder/Value.h>

#include <limits>

#include <binder/IBinder.h>
#include <binder/Parcel.h>
#include <binder/Map.h>
#include <private/binder/ParcelValTypes.h>
#include <log/log.h>
#include <utils/Errors.h>

using android::BAD_TYPE;
using android::BAD_VALUE;
using android::NO_ERROR;
using android::UNEXPECTED_NULL;
using android::Parcel;
using android::sp;
using android::status_t;
using std::map;
using std::set;
using std::vector;
using android::binder::Value;
using android::IBinder;
using android::os::PersistableBundle;
using namespace android::binder;

// ====================================================================

#define RETURN_IF_FAILED(calledOnce)                                     \
    do {                                                                 \
        status_t returnStatus = calledOnce;                              \
        if (returnStatus) {                                              \
            ALOGE("Failed at %s:%d (%s)", __FILE__, __LINE__, __func__); \
            return returnStatus;                                         \
         }                                                               \
    } while(false)

// ====================================================================

/* These `internal_type_ptr()` functions allow this
 * class to work without C++ RTTI support. This technique
 * only works properly when called directly from this file,
 * but that is OK because that is the only place we will
 * be calling them from. */
template<class T> const void* internal_type_ptr()
{
    static const T *marker;
    return (void*)&marker;
}

/* Allows the type to be specified by the argument
 * instead of inside angle brackets. */
template<class T> const void* internal_type_ptr(const T&)
{
    return internal_type_ptr<T>();
}

// ====================================================================

namespace android {

namespace binder {

class Value::ContentBase {
public:
    virtual ~ContentBase() = default;
    virtual const void* type_ptr() const = 0;
    virtual ContentBase * clone() const = 0;
    virtual bool operator==(const ContentBase& rhs) const = 0;

#ifdef LIBBINDER_VALUE_SUPPORTS_TYPE_INFO
    virtual const std::type_info &type() const = 0;
#endif

    template<typename T> bool get(T* out) const;
};

/* This is the actual class that holds the value. */
template<typename T> class Value::Content : public Value::ContentBase {
public:
    Content() = default;
    explicit Content(const T & value) : mValue(value) { }

    virtual ~Content() = default;

#ifdef LIBBINDER_VALUE_SUPPORTS_TYPE_INFO
    virtual const std::type_info &type() const override
    {
        return typeid(T);
    }
#endif

    virtual const void* type_ptr() const override
    {
        return internal_type_ptr<T>();
    }

    virtual ContentBase * clone() const override
    {
        return new Content(mValue);
    };

    virtual bool operator==(const ContentBase& rhs) const override
    {
        if (type_ptr() != rhs.type_ptr()) {
            return false;
        }
        return mValue == static_cast<const Content<T>* >(&rhs)->mValue;
    }

    T mValue;
};

template<typename T> bool Value::ContentBase::get(T* out) const
{
    if (internal_type_ptr(*out) != type_ptr())
    {
        return false;
    }

    *out = static_cast<const Content<T>*>(this)->mValue;

    return true;
}

// ====================================================================

Value::Value() : mContent(nullptr)
{
}

Value::Value(const Value& value)
    : mContent(value.mContent ? value.mContent->clone() : nullptr)
{
}

Value::~Value()
{
    delete mContent;
}

bool Value::operator==(const Value& rhs) const
{
    const Value& lhs(*this);

    if (lhs.empty() && rhs.empty()) {
        return true;
    }

    if ( (lhs.mContent == nullptr)
      || (rhs.mContent == nullptr)
    ) {
        return false;
    }

    return *lhs.mContent == *rhs.mContent;
}

Value& Value::swap(Value &rhs)
{
    std::swap(mContent, rhs.mContent);
    return *this;
}

Value& Value::operator=(const Value& rhs)
{
    if (this != &rhs) {
        delete mContent;
        mContent = rhs.mContent
            ? rhs.mContent->clone()
            : nullptr;
    }
    return *this;
}

bool Value::empty() const
{
    return mContent == nullptr;
}

void Value::clear()
{
    delete mContent;
    mContent = nullptr;
}

int32_t Value::parcelType() const
{
    const void* t_info(mContent ? mContent->type_ptr() : nullptr);

    if (t_info == internal_type_ptr<bool>()) return VAL_BOOLEAN;
    if (t_info == internal_type_ptr<uint8_t>()) return VAL_BYTE;
    if (t_info == internal_type_ptr<int32_t>()) return VAL_INTEGER;
    if (t_info == internal_type_ptr<int64_t>()) return VAL_LONG;
    if (t_info == internal_type_ptr<double>()) return VAL_DOUBLE;
    if (t_info == internal_type_ptr<String16>()) return VAL_STRING;

    if (t_info == internal_type_ptr<vector<bool>>()) return VAL_BOOLEANARRAY;
    if (t_info == internal_type_ptr<vector<uint8_t>>()) return VAL_BYTEARRAY;
    if (t_info == internal_type_ptr<vector<int32_t>>()) return VAL_INTARRAY;
    if (t_info == internal_type_ptr<vector<int64_t>>()) return VAL_LONGARRAY;
    if (t_info == internal_type_ptr<vector<double>>()) return VAL_DOUBLEARRAY;
    if (t_info == internal_type_ptr<vector<String16>>()) return VAL_STRINGARRAY;

    if (t_info == internal_type_ptr<Map>()) return VAL_MAP;
    if (t_info == internal_type_ptr<PersistableBundle>()) return VAL_PERSISTABLEBUNDLE;

    return VAL_NULL;
}

#ifdef LIBBINDER_VALUE_SUPPORTS_TYPE_INFO
const std::type_info& Value::type() const
{
    return mContent != nullptr
        ? mContent->type()
        : typeid(void);
}
#endif // ifdef LIBBINDER_VALUE_SUPPORTS_TYPE_INFO

#define DEF_TYPE_ACCESSORS(T, TYPENAME)                      \
    bool Value::is ## TYPENAME() const                       \
    {                                                        \
        return mContent                                      \
            ? internal_type_ptr<T>() == mContent->type_ptr() \
            : false;                                         \
    }                                                        \
    bool Value::get ## TYPENAME(T* out) const                \
    {                                                        \
        return mContent                                      \
            ? mContent->get(out)                             \
            : false;                                         \
    }                                                        \
    void Value::put ## TYPENAME(const T& in)                 \
    {                                                        \
        *this = in;                                          \
    }                                                        \
    Value& Value::operator=(const T& rhs)                    \
    {                                                        \
        delete mContent;                                     \
        mContent = new Content< T >(rhs);                    \
        return *this;                                        \
    }                                                        \
    Value::Value(const T& value)                             \
        : mContent(new Content< T >(value))                  \
    { }

DEF_TYPE_ACCESSORS(bool, Boolean)
DEF_TYPE_ACCESSORS(int8_t, Byte)
DEF_TYPE_ACCESSORS(int32_t, Int)
DEF_TYPE_ACCESSORS(int64_t, Long)
DEF_TYPE_ACCESSORS(double, Double)
DEF_TYPE_ACCESSORS(String16, String)

DEF_TYPE_ACCESSORS(std::vector<bool>, BooleanVector)
DEF_TYPE_ACCESSORS(std::vector<uint8_t>, ByteVector)
DEF_TYPE_ACCESSORS(std::vector<int32_t>, IntVector)
DEF_TYPE_ACCESSORS(std::vector<int64_t>, LongVector)
DEF_TYPE_ACCESSORS(std::vector<double>, DoubleVector)
DEF_TYPE_ACCESSORS(std::vector<String16>, StringVector)

DEF_TYPE_ACCESSORS(::android::binder::Map, Map)
DEF_TYPE_ACCESSORS(PersistableBundle, PersistableBundle)

bool Value::getString(String8* out) const
{
    String16 val;
    bool ret = getString(&val);
    if (ret) {
        *out = String8(val);
    }
    return ret;
}

bool Value::getString(::std::string* out) const
{
    String8 val;
    bool ret = getString(&val);
    if (ret) {
        *out = val.string();
    }
    return ret;
}

status_t Value::writeToParcel(Parcel* parcel) const
{
    // This implementation needs to be kept in sync with the writeValue
    // implementation in frameworks/base/core/java/android/os/Parcel.java

#define BEGIN_HANDLE_WRITE()                                                                      \
    do {                                                                                          \
        const void* t_info(mContent?mContent->type_ptr():nullptr);                                \
        if (false) { }
#define HANDLE_WRITE_TYPE(T, TYPEVAL, TYPEMETHOD)                                                 \
    else if (t_info == internal_type_ptr<T>()) {                                                  \
        RETURN_IF_FAILED(parcel->writeInt32(TYPEVAL));                                            \
        RETURN_IF_FAILED(parcel->TYPEMETHOD(static_cast<const Content<T>*>(mContent)->mValue));   \
    }
#define HANDLE_WRITE_PARCELABLE(T, TYPEVAL)                                                       \
    else if (t_info == internal_type_ptr<T>()) {                                                  \
        RETURN_IF_FAILED(parcel->writeInt32(TYPEVAL));                                            \
        RETURN_IF_FAILED(static_cast<const Content<T>*>(mContent)->mValue.writeToParcel(parcel)); \
    }
#define END_HANDLE_WRITE()                                                                        \
        else {                                                                                    \
            ALOGE("writeToParcel: Type not supported");                                           \
            return BAD_TYPE;                                                                      \
        }                                                                                         \
    } while (false);

    BEGIN_HANDLE_WRITE()

    HANDLE_WRITE_TYPE(bool,     VAL_BOOLEAN, writeBool)
    HANDLE_WRITE_TYPE(int8_t,   VAL_BYTE,    writeByte)
    HANDLE_WRITE_TYPE(int8_t,   VAL_BYTE,    writeByte)
    HANDLE_WRITE_TYPE(int32_t,  VAL_INTEGER, writeInt32)
    HANDLE_WRITE_TYPE(int64_t,  VAL_LONG,    writeInt64)
    HANDLE_WRITE_TYPE(double,   VAL_DOUBLE,  writeDouble)
    HANDLE_WRITE_TYPE(String16, VAL_STRING,  writeString16)

    HANDLE_WRITE_TYPE(vector<bool>,     VAL_BOOLEANARRAY, writeBoolVector)
    HANDLE_WRITE_TYPE(vector<uint8_t>,  VAL_BYTEARRAY,    writeByteVector)
    HANDLE_WRITE_TYPE(vector<int8_t>,   VAL_BYTEARRAY,    writeByteVector)
    HANDLE_WRITE_TYPE(vector<int32_t>,  VAL_INTARRAY,     writeInt32Vector)
    HANDLE_WRITE_TYPE(vector<int64_t>,  VAL_LONGARRAY,    writeInt64Vector)
    HANDLE_WRITE_TYPE(vector<double>,   VAL_DOUBLEARRAY,  writeDoubleVector)
    HANDLE_WRITE_TYPE(vector<String16>, VAL_STRINGARRAY,  writeString16Vector)

    HANDLE_WRITE_PARCELABLE(PersistableBundle, VAL_PERSISTABLEBUNDLE)

    END_HANDLE_WRITE()

    return NO_ERROR;

#undef BEGIN_HANDLE_WRITE
#undef HANDLE_WRITE_TYPE
#undef HANDLE_WRITE_PARCELABLE
#undef END_HANDLE_WRITE
}

status_t Value::readFromParcel(const Parcel* parcel)
{
    // This implementation needs to be kept in sync with the readValue
    // implementation in frameworks/base/core/java/android/os/Parcel.javai

#define BEGIN_HANDLE_READ()                                                                      \
    switch(value_type) {                                                                         \
        default:                                                                                 \
            ALOGE("readFromParcel: Parcel type %d is not supported", value_type);                \
            return BAD_TYPE;
#define HANDLE_READ_TYPE(T, TYPEVAL, TYPEMETHOD)                                                 \
        case TYPEVAL:                                                                            \
            mContent = new Content<T>();                                                         \
            RETURN_IF_FAILED(parcel->TYPEMETHOD(&static_cast<Content<T>*>(mContent)->mValue));   \
            break;
#define HANDLE_READ_PARCELABLE(T, TYPEVAL)                                                       \
        case TYPEVAL:                                                                            \
            mContent = new Content<T>();                                                         \
            RETURN_IF_FAILED(static_cast<Content<T>*>(mContent)->mValue.readFromParcel(parcel)); \
            break;
#define END_HANDLE_READ()                                                                        \
    }

    int32_t value_type = VAL_NULL;

    delete mContent;
    mContent = nullptr;

    RETURN_IF_FAILED(parcel->readInt32(&value_type));

    BEGIN_HANDLE_READ()

    HANDLE_READ_TYPE(bool,     VAL_BOOLEAN, readBool)
    HANDLE_READ_TYPE(int8_t,   VAL_BYTE,    readByte)
    HANDLE_READ_TYPE(int32_t,  VAL_INTEGER, readInt32)
    HANDLE_READ_TYPE(int64_t,  VAL_LONG,    readInt64)
    HANDLE_READ_TYPE(double,   VAL_DOUBLE,  readDouble)
    HANDLE_READ_TYPE(String16, VAL_STRING,  readString16)

    HANDLE_READ_TYPE(vector<bool>,     VAL_BOOLEANARRAY, readBoolVector)
    HANDLE_READ_TYPE(vector<uint8_t>,  VAL_BYTEARRAY,    readByteVector)
    HANDLE_READ_TYPE(vector<int32_t>,  VAL_INTARRAY,     readInt32Vector)
    HANDLE_READ_TYPE(vector<int64_t>,  VAL_LONGARRAY,    readInt64Vector)
    HANDLE_READ_TYPE(vector<double>,   VAL_DOUBLEARRAY,  readDoubleVector)
    HANDLE_READ_TYPE(vector<String16>, VAL_STRINGARRAY,  readString16Vector)

    HANDLE_READ_PARCELABLE(PersistableBundle, VAL_PERSISTABLEBUNDLE)

    END_HANDLE_READ()

    return NO_ERROR;

#undef BEGIN_HANDLE_READ
#undef HANDLE_READ_TYPE
#undef HANDLE_READ_PARCELABLE
#undef END_HANDLE_READ
}

}  // namespace binder

}  // namespace android

/* vim: set ts=4 sw=4 tw=0 et :*/
