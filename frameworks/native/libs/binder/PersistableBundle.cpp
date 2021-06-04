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

#define LOG_TAG "PersistableBundle"

#include <binder/PersistableBundle.h>
#include <private/binder/ParcelValTypes.h>

#include <limits>

#include <binder/IBinder.h>
#include <binder/Parcel.h>
#include <log/log.h>
#include <utils/Errors.h>

using android::BAD_TYPE;
using android::BAD_VALUE;
using android::NO_ERROR;
using android::Parcel;
using android::sp;
using android::status_t;
using android::UNEXPECTED_NULL;
using std::map;
using std::set;
using std::vector;
using namespace ::android::binder;

enum {
    // Keep them in sync with BUNDLE_MAGIC* in frameworks/base/core/java/android/os/BaseBundle.java.
    BUNDLE_MAGIC = 0x4C444E42,
    BUNDLE_MAGIC_NATIVE = 0x4C444E44,
};

namespace {
template <typename T>
bool getValue(const android::String16& key, T* out, const map<android::String16, T>& map) {
    const auto& it = map.find(key);
    if (it == map.end()) return false;
    *out = it->second;
    return true;
}

template <typename T>
set<android::String16> getKeys(const map<android::String16, T>& map) {
    if (map.empty()) return set<android::String16>();
    set<android::String16> keys;
    for (const auto& key_value_pair : map) {
        keys.emplace(key_value_pair.first);
    }
    return keys;
}
}  // namespace

namespace android {

namespace os {

#define RETURN_IF_FAILED(calledOnce)                                     \
    {                                                                    \
        status_t returnStatus = calledOnce;                              \
        if (returnStatus) {                                              \
            ALOGE("Failed at %s:%d (%s)", __FILE__, __LINE__, __func__); \
            return returnStatus;                                         \
         }                                                               \
    }

#define RETURN_IF_ENTRY_ERASED(map, key)                                 \
    {                                                                    \
        size_t num_erased = (map).erase(key);                            \
        if (num_erased) {                                                \
            ALOGE("Failed at %s:%d (%s)", __FILE__, __LINE__, __func__); \
            return num_erased;                                           \
         }                                                               \
    }

status_t PersistableBundle::writeToParcel(Parcel* parcel) const {
    /*
     * Keep implementation in sync with writeToParcelInner() in
     * frameworks/base/core/java/android/os/BaseBundle.java.
     */

    // Special case for empty bundles.
    if (empty()) {
        RETURN_IF_FAILED(parcel->writeInt32(0));
        return NO_ERROR;
    }

    size_t length_pos = parcel->dataPosition();
    RETURN_IF_FAILED(parcel->writeInt32(1));  // dummy, will hold length
    RETURN_IF_FAILED(parcel->writeInt32(BUNDLE_MAGIC_NATIVE));

    size_t start_pos = parcel->dataPosition();
    RETURN_IF_FAILED(writeToParcelInner(parcel));
    size_t end_pos = parcel->dataPosition();

    // Backpatch length. This length value includes the length header.
    parcel->setDataPosition(length_pos);
    size_t length = end_pos - start_pos;
    if (length > std::numeric_limits<int32_t>::max()) {
        ALOGE("Parcel length (%zu) too large to store in 32-bit signed int", length);
        return BAD_VALUE;
    }
    RETURN_IF_FAILED(parcel->writeInt32(static_cast<int32_t>(length)));
    parcel->setDataPosition(end_pos);
    return NO_ERROR;
}

status_t PersistableBundle::readFromParcel(const Parcel* parcel) {
    /*
     * Keep implementation in sync with readFromParcelInner() in
     * frameworks/base/core/java/android/os/BaseBundle.java.
     */
    int32_t length = parcel->readInt32();
    if (length < 0) {
        ALOGE("Bad length in parcel: %d", length);
        return UNEXPECTED_NULL;
    }

    return readFromParcelInner(parcel, static_cast<size_t>(length));
}

bool PersistableBundle::empty() const {
    return size() == 0u;
}

size_t PersistableBundle::size() const {
    return (mBoolMap.size() +
            mIntMap.size() +
            mLongMap.size() +
            mDoubleMap.size() +
            mStringMap.size() +
            mBoolVectorMap.size() +
            mIntVectorMap.size() +
            mLongVectorMap.size() +
            mDoubleVectorMap.size() +
            mStringVectorMap.size() +
            mPersistableBundleMap.size());
}

size_t PersistableBundle::erase(const String16& key) {
    RETURN_IF_ENTRY_ERASED(mBoolMap, key);
    RETURN_IF_ENTRY_ERASED(mIntMap, key);
    RETURN_IF_ENTRY_ERASED(mLongMap, key);
    RETURN_IF_ENTRY_ERASED(mDoubleMap, key);
    RETURN_IF_ENTRY_ERASED(mStringMap, key);
    RETURN_IF_ENTRY_ERASED(mBoolVectorMap, key);
    RETURN_IF_ENTRY_ERASED(mIntVectorMap, key);
    RETURN_IF_ENTRY_ERASED(mLongVectorMap, key);
    RETURN_IF_ENTRY_ERASED(mDoubleVectorMap, key);
    RETURN_IF_ENTRY_ERASED(mStringVectorMap, key);
    return mPersistableBundleMap.erase(key);
}

void PersistableBundle::putBoolean(const String16& key, bool value) {
    erase(key);
    mBoolMap[key] = value;
}

void PersistableBundle::putInt(const String16& key, int32_t value) {
    erase(key);
    mIntMap[key] = value;
}

void PersistableBundle::putLong(const String16& key, int64_t value) {
    erase(key);
    mLongMap[key] = value;
}

void PersistableBundle::putDouble(const String16& key, double value) {
    erase(key);
    mDoubleMap[key] = value;
}

void PersistableBundle::putString(const String16& key, const String16& value) {
    erase(key);
    mStringMap[key] = value;
}

void PersistableBundle::putBooleanVector(const String16& key, const vector<bool>& value) {
    erase(key);
    mBoolVectorMap[key] = value;
}

void PersistableBundle::putIntVector(const String16& key, const vector<int32_t>& value) {
    erase(key);
    mIntVectorMap[key] = value;
}

void PersistableBundle::putLongVector(const String16& key, const vector<int64_t>& value) {
    erase(key);
    mLongVectorMap[key] = value;
}

void PersistableBundle::putDoubleVector(const String16& key, const vector<double>& value) {
    erase(key);
    mDoubleVectorMap[key] = value;
}

void PersistableBundle::putStringVector(const String16& key, const vector<String16>& value) {
    erase(key);
    mStringVectorMap[key] = value;
}

void PersistableBundle::putPersistableBundle(const String16& key, const PersistableBundle& value) {
    erase(key);
    mPersistableBundleMap[key] = value;
}

bool PersistableBundle::getBoolean(const String16& key, bool* out) const {
    return getValue(key, out, mBoolMap);
}

bool PersistableBundle::getInt(const String16& key, int32_t* out) const {
    return getValue(key, out, mIntMap);
}

bool PersistableBundle::getLong(const String16& key, int64_t* out) const {
    return getValue(key, out, mLongMap);
}

bool PersistableBundle::getDouble(const String16& key, double* out) const {
    return getValue(key, out, mDoubleMap);
}

bool PersistableBundle::getString(const String16& key, String16* out) const {
    return getValue(key, out, mStringMap);
}

bool PersistableBundle::getBooleanVector(const String16& key, vector<bool>* out) const {
    return getValue(key, out, mBoolVectorMap);
}

bool PersistableBundle::getIntVector(const String16& key, vector<int32_t>* out) const {
    return getValue(key, out, mIntVectorMap);
}

bool PersistableBundle::getLongVector(const String16& key, vector<int64_t>* out) const {
    return getValue(key, out, mLongVectorMap);
}

bool PersistableBundle::getDoubleVector(const String16& key, vector<double>* out) const {
    return getValue(key, out, mDoubleVectorMap);
}

bool PersistableBundle::getStringVector(const String16& key, vector<String16>* out) const {
    return getValue(key, out, mStringVectorMap);
}

bool PersistableBundle::getPersistableBundle(const String16& key, PersistableBundle* out) const {
    return getValue(key, out, mPersistableBundleMap);
}

set<String16> PersistableBundle::getBooleanKeys() const {
    return getKeys(mBoolMap);
}

set<String16> PersistableBundle::getIntKeys() const {
    return getKeys(mIntMap);
}

set<String16> PersistableBundle::getLongKeys() const {
    return getKeys(mLongMap);
}

set<String16> PersistableBundle::getDoubleKeys() const {
    return getKeys(mDoubleMap);
}

set<String16> PersistableBundle::getStringKeys() const {
    return getKeys(mStringMap);
}

set<String16> PersistableBundle::getBooleanVectorKeys() const {
    return getKeys(mBoolVectorMap);
}

set<String16> PersistableBundle::getIntVectorKeys() const {
    return getKeys(mIntVectorMap);
}

set<String16> PersistableBundle::getLongVectorKeys() const {
    return getKeys(mLongVectorMap);
}

set<String16> PersistableBundle::getDoubleVectorKeys() const {
    return getKeys(mDoubleVectorMap);
}

set<String16> PersistableBundle::getStringVectorKeys() const {
    return getKeys(mStringVectorMap);
}

set<String16> PersistableBundle::getPersistableBundleKeys() const {
    return getKeys(mPersistableBundleMap);
}

status_t PersistableBundle::writeToParcelInner(Parcel* parcel) const {
    /*
     * To keep this implementation in sync with writeArrayMapInternal() in
     * frameworks/base/core/java/android/os/Parcel.java, the number of key
     * value pairs must be written into the parcel before writing the key-value
     * pairs themselves.
     */
    size_t num_entries = size();
    if (num_entries > std::numeric_limits<int32_t>::max()) {
        ALOGE("The size of this PersistableBundle (%zu) too large to store in 32-bit signed int",
              num_entries);
        return BAD_VALUE;
    }
    RETURN_IF_FAILED(parcel->writeInt32(static_cast<int32_t>(num_entries)));

    for (const auto& key_val_pair : mBoolMap) {
        RETURN_IF_FAILED(parcel->writeString16(key_val_pair.first));
        RETURN_IF_FAILED(parcel->writeInt32(VAL_BOOLEAN));
        RETURN_IF_FAILED(parcel->writeBool(key_val_pair.second));
    }
    for (const auto& key_val_pair : mIntMap) {
        RETURN_IF_FAILED(parcel->writeString16(key_val_pair.first));
        RETURN_IF_FAILED(parcel->writeInt32(VAL_INTEGER));
        RETURN_IF_FAILED(parcel->writeInt32(key_val_pair.second));
    }
    for (const auto& key_val_pair : mLongMap) {
        RETURN_IF_FAILED(parcel->writeString16(key_val_pair.first));
        RETURN_IF_FAILED(parcel->writeInt32(VAL_LONG));
        RETURN_IF_FAILED(parcel->writeInt64(key_val_pair.second));
    }
    for (const auto& key_val_pair : mDoubleMap) {
        RETURN_IF_FAILED(parcel->writeString16(key_val_pair.first));
        RETURN_IF_FAILED(parcel->writeInt32(VAL_DOUBLE));
        RETURN_IF_FAILED(parcel->writeDouble(key_val_pair.second));
    }
    for (const auto& key_val_pair : mStringMap) {
        RETURN_IF_FAILED(parcel->writeString16(key_val_pair.first));
        RETURN_IF_FAILED(parcel->writeInt32(VAL_STRING));
        RETURN_IF_FAILED(parcel->writeString16(key_val_pair.second));
    }
    for (const auto& key_val_pair : mBoolVectorMap) {
        RETURN_IF_FAILED(parcel->writeString16(key_val_pair.first));
        RETURN_IF_FAILED(parcel->writeInt32(VAL_BOOLEANARRAY));
        RETURN_IF_FAILED(parcel->writeBoolVector(key_val_pair.second));
    }
    for (const auto& key_val_pair : mIntVectorMap) {
        RETURN_IF_FAILED(parcel->writeString16(key_val_pair.first));
        RETURN_IF_FAILED(parcel->writeInt32(VAL_INTARRAY));
        RETURN_IF_FAILED(parcel->writeInt32Vector(key_val_pair.second));
    }
    for (const auto& key_val_pair : mLongVectorMap) {
        RETURN_IF_FAILED(parcel->writeString16(key_val_pair.first));
        RETURN_IF_FAILED(parcel->writeInt32(VAL_LONGARRAY));
        RETURN_IF_FAILED(parcel->writeInt64Vector(key_val_pair.second));
    }
    for (const auto& key_val_pair : mDoubleVectorMap) {
        RETURN_IF_FAILED(parcel->writeString16(key_val_pair.first));
        RETURN_IF_FAILED(parcel->writeInt32(VAL_DOUBLEARRAY));
        RETURN_IF_FAILED(parcel->writeDoubleVector(key_val_pair.second));
    }
    for (const auto& key_val_pair : mStringVectorMap) {
        RETURN_IF_FAILED(parcel->writeString16(key_val_pair.first));
        RETURN_IF_FAILED(parcel->writeInt32(VAL_STRINGARRAY));
        RETURN_IF_FAILED(parcel->writeString16Vector(key_val_pair.second));
    }
    for (const auto& key_val_pair : mPersistableBundleMap) {
        RETURN_IF_FAILED(parcel->writeString16(key_val_pair.first));
        RETURN_IF_FAILED(parcel->writeInt32(VAL_PERSISTABLEBUNDLE));
        RETURN_IF_FAILED(key_val_pair.second.writeToParcel(parcel));
    }
    return NO_ERROR;
}

status_t PersistableBundle::readFromParcelInner(const Parcel* parcel, size_t length) {
    /*
     * Note: we don't actually use length for anything other than an empty PersistableBundle
     * check, since we do not actually need to copy in an entire Parcel, unlike in the Java
     * implementation.
     */
    if (length == 0) {
        // Empty PersistableBundle or end of data.
        return NO_ERROR;
    }

    int32_t magic;
    RETURN_IF_FAILED(parcel->readInt32(&magic));
    if (magic != BUNDLE_MAGIC && magic != BUNDLE_MAGIC_NATIVE) {
        ALOGE("Bad magic number for PersistableBundle: 0x%08x", magic);
        return BAD_VALUE;
    }

    /*
     * To keep this implementation in sync with unparcel() in
     * frameworks/base/core/java/android/os/BaseBundle.java, the number of
     * key-value pairs must be read from the parcel before reading the key-value
     * pairs themselves.
     */
    int32_t num_entries;
    RETURN_IF_FAILED(parcel->readInt32(&num_entries));

    for (; num_entries > 0; --num_entries) {
        String16 key;
        int32_t value_type;
        RETURN_IF_FAILED(parcel->readString16(&key));
        RETURN_IF_FAILED(parcel->readInt32(&value_type));

        /*
         * We assume that both the C++ and Java APIs ensure that all keys in a PersistableBundle
         * are unique.
         */
        switch (value_type) {
            case VAL_STRING: {
                RETURN_IF_FAILED(parcel->readString16(&mStringMap[key]));
                break;
            }
            case VAL_INTEGER: {
                RETURN_IF_FAILED(parcel->readInt32(&mIntMap[key]));
                break;
            }
            case VAL_LONG: {
                RETURN_IF_FAILED(parcel->readInt64(&mLongMap[key]));
                break;
            }
            case VAL_DOUBLE: {
                RETURN_IF_FAILED(parcel->readDouble(&mDoubleMap[key]));
                break;
            }
            case VAL_BOOLEAN: {
                RETURN_IF_FAILED(parcel->readBool(&mBoolMap[key]));
                break;
            }
            case VAL_STRINGARRAY: {
                RETURN_IF_FAILED(parcel->readString16Vector(&mStringVectorMap[key]));
                break;
            }
            case VAL_INTARRAY: {
                RETURN_IF_FAILED(parcel->readInt32Vector(&mIntVectorMap[key]));
                break;
            }
            case VAL_LONGARRAY: {
                RETURN_IF_FAILED(parcel->readInt64Vector(&mLongVectorMap[key]));
                break;
            }
            case VAL_BOOLEANARRAY: {
                RETURN_IF_FAILED(parcel->readBoolVector(&mBoolVectorMap[key]));
                break;
            }
            case VAL_PERSISTABLEBUNDLE: {
                RETURN_IF_FAILED(mPersistableBundleMap[key].readFromParcel(parcel));
                break;
            }
            case VAL_DOUBLEARRAY: {
                RETURN_IF_FAILED(parcel->readDoubleVector(&mDoubleVectorMap[key]));
                break;
            }
            default: {
                ALOGE("Unrecognized type: %d", value_type);
                return BAD_TYPE;
                break;
            }
        }
    }

    return NO_ERROR;
}

}  // namespace os

}  // namespace android
