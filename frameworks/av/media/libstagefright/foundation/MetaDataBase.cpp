/*
 * Copyright (C) 2009 The Android Open Source Project
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
#define LOG_TAG "MetaDataBase"
#include <inttypes.h>
#include <binder/Parcel.h>
#include <utils/KeyedVector.h>
#include <utils/Log.h>

#include <stdlib.h>
#include <string.h>

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/MetaDataBase.h>

namespace android {

struct MetaDataBase::typed_data {
    typed_data();
    ~typed_data();

    typed_data(const MetaDataBase::typed_data &);
    typed_data &operator=(const MetaDataBase::typed_data &);

    void clear();
    void setData(uint32_t type, const void *data, size_t size);
    void getData(uint32_t *type, const void **data, size_t *size) const;
    // may include hexdump of binary data if verbose=true
    String8 asString(bool verbose) const;

private:
    uint32_t mType;
    size_t mSize;

    union {
        void *ext_data;
        float reservoir;
    } u;

    bool usesReservoir() const {
        return mSize <= sizeof(u.reservoir);
    }

    void *allocateStorage(size_t size);
    void freeStorage();

    void *storage() {
        return usesReservoir() ? &u.reservoir : u.ext_data;
    }

    const void *storage() const {
        return usesReservoir() ? &u.reservoir : u.ext_data;
    }
};

struct MetaDataBase::Rect {
    int32_t mLeft, mTop, mRight, mBottom;
};


struct MetaDataBase::MetaDataInternal {
    KeyedVector<uint32_t, MetaDataBase::typed_data> mItems;
};


MetaDataBase::MetaDataBase()
    : mInternalData(new MetaDataInternal()) {
}

MetaDataBase::MetaDataBase(const MetaDataBase &from)
    : mInternalData(new MetaDataInternal()) {
    mInternalData->mItems = from.mInternalData->mItems;
}

MetaDataBase& MetaDataBase::operator = (const MetaDataBase &rhs) {
    this->mInternalData->mItems = rhs.mInternalData->mItems;
    return *this;
}

MetaDataBase::~MetaDataBase() {
    clear();
    delete mInternalData;
}

void MetaDataBase::clear() {
    mInternalData->mItems.clear();
}

bool MetaDataBase::remove(uint32_t key) {
    ssize_t i = mInternalData->mItems.indexOfKey(key);

    if (i < 0) {
        return false;
    }

    mInternalData->mItems.removeItemsAt(i);

    return true;
}

bool MetaDataBase::setCString(uint32_t key, const char *value) {
    return setData(key, TYPE_C_STRING, value, strlen(value) + 1);
}

bool MetaDataBase::setInt32(uint32_t key, int32_t value) {
    return setData(key, TYPE_INT32, &value, sizeof(value));
}

bool MetaDataBase::setInt64(uint32_t key, int64_t value) {
    return setData(key, TYPE_INT64, &value, sizeof(value));
}

bool MetaDataBase::setFloat(uint32_t key, float value) {
    return setData(key, TYPE_FLOAT, &value, sizeof(value));
}

bool MetaDataBase::setPointer(uint32_t key, void *value) {
    return setData(key, TYPE_POINTER, &value, sizeof(value));
}

bool MetaDataBase::setRect(
        uint32_t key,
        int32_t left, int32_t top,
        int32_t right, int32_t bottom) {
    Rect r;
    r.mLeft = left;
    r.mTop = top;
    r.mRight = right;
    r.mBottom = bottom;

    return setData(key, TYPE_RECT, &r, sizeof(r));
}

/**
 * Note that the returned pointer becomes invalid when additional metadata is set.
 */
bool MetaDataBase::findCString(uint32_t key, const char **value) const {
    uint32_t type;
    const void *data;
    size_t size;
    if (!findData(key, &type, &data, &size) || type != TYPE_C_STRING) {
        return false;
    }

    *value = (const char *)data;

    return true;
}

bool MetaDataBase::findInt32(uint32_t key, int32_t *value) const {
    uint32_t type = 0;
    const void *data;
    size_t size;
    if (!findData(key, &type, &data, &size) || type != TYPE_INT32) {
        return false;
    }

    CHECK_EQ(size, sizeof(*value));

    *value = *(int32_t *)data;

    return true;
}

bool MetaDataBase::findInt64(uint32_t key, int64_t *value) const {
    uint32_t type = 0;
    const void *data;
    size_t size;
    if (!findData(key, &type, &data, &size) || type != TYPE_INT64) {
        return false;
    }

    CHECK_EQ(size, sizeof(*value));

    *value = *(int64_t *)data;

    return true;
}

bool MetaDataBase::findFloat(uint32_t key, float *value) const {
    uint32_t type = 0;
    const void *data;
    size_t size;
    if (!findData(key, &type, &data, &size) || type != TYPE_FLOAT) {
        return false;
    }

    CHECK_EQ(size, sizeof(*value));

    *value = *(float *)data;

    return true;
}

bool MetaDataBase::findPointer(uint32_t key, void **value) const {
    uint32_t type = 0;
    const void *data;
    size_t size;
    if (!findData(key, &type, &data, &size) || type != TYPE_POINTER) {
        return false;
    }

    CHECK_EQ(size, sizeof(*value));

    *value = *(void **)data;

    return true;
}

bool MetaDataBase::findRect(
        uint32_t key,
        int32_t *left, int32_t *top,
        int32_t *right, int32_t *bottom) const {
    uint32_t type = 0;
    const void *data;
    size_t size;
    if (!findData(key, &type, &data, &size) || type != TYPE_RECT) {
        return false;
    }

    CHECK_EQ(size, sizeof(Rect));

    const Rect *r = (const Rect *)data;
    *left = r->mLeft;
    *top = r->mTop;
    *right = r->mRight;
    *bottom = r->mBottom;

    return true;
}

bool MetaDataBase::setData(
        uint32_t key, uint32_t type, const void *data, size_t size) {
    bool overwrote_existing = true;

    ssize_t i = mInternalData->mItems.indexOfKey(key);
    if (i < 0) {
        typed_data item;
        i = mInternalData->mItems.add(key, item);

        overwrote_existing = false;
    }

    typed_data &item = mInternalData->mItems.editValueAt(i);

    item.setData(type, data, size);

    return overwrote_existing;
}

bool MetaDataBase::findData(uint32_t key, uint32_t *type,
                        const void **data, size_t *size) const {
    ssize_t i = mInternalData->mItems.indexOfKey(key);

    if (i < 0) {
        return false;
    }

    const typed_data &item = mInternalData->mItems.valueAt(i);

    item.getData(type, data, size);

    return true;
}

bool MetaDataBase::hasData(uint32_t key) const {
    ssize_t i = mInternalData->mItems.indexOfKey(key);

    if (i < 0) {
        return false;
    }

    return true;
}

MetaDataBase::typed_data::typed_data()
    : mType(0),
      mSize(0) {
}

MetaDataBase::typed_data::~typed_data() {
    clear();
}

MetaDataBase::typed_data::typed_data(const typed_data &from)
    : mType(from.mType),
      mSize(0) {

    void *dst = allocateStorage(from.mSize);
    if (dst) {
        memcpy(dst, from.storage(), mSize);
    }
}

MetaDataBase::typed_data &MetaDataBase::typed_data::operator=(
        const MetaDataBase::typed_data &from) {
    if (this != &from) {
        clear();
        mType = from.mType;
        void *dst = allocateStorage(from.mSize);
        if (dst) {
            memcpy(dst, from.storage(), mSize);
        }
    }

    return *this;
}

void MetaDataBase::typed_data::clear() {
    freeStorage();

    mType = 0;
}

void MetaDataBase::typed_data::setData(
        uint32_t type, const void *data, size_t size) {
    clear();

    mType = type;

    void *dst = allocateStorage(size);
    if (dst) {
        memcpy(dst, data, size);
    }
}

void MetaDataBase::typed_data::getData(
        uint32_t *type, const void **data, size_t *size) const {
    *type = mType;
    *size = mSize;
    *data = storage();
}

void *MetaDataBase::typed_data::allocateStorage(size_t size) {
    mSize = size;

    if (usesReservoir()) {
        return &u.reservoir;
    }

    u.ext_data = malloc(mSize);
    if (u.ext_data == NULL) {
        ALOGE("Couldn't allocate %zu bytes for item", size);
        mSize = 0;
    }
    return u.ext_data;
}

void MetaDataBase::typed_data::freeStorage() {
    if (!usesReservoir()) {
        if (u.ext_data) {
            free(u.ext_data);
            u.ext_data = NULL;
        }
    }

    mSize = 0;
}

String8 MetaDataBase::typed_data::asString(bool verbose) const {
    String8 out;
    const void *data = storage();
    switch(mType) {
        case TYPE_NONE:
            out = String8::format("no type, size %zu)", mSize);
            break;
        case TYPE_C_STRING:
            out = String8::format("(char*) %s", (const char *)data);
            break;
        case TYPE_INT32:
            out = String8::format("(int32_t) %d", *(int32_t *)data);
            break;
        case TYPE_INT64:
            out = String8::format("(int64_t) %" PRId64, *(int64_t *)data);
            break;
        case TYPE_FLOAT:
            out = String8::format("(float) %f", *(float *)data);
            break;
        case TYPE_POINTER:
            out = String8::format("(void*) %p", *(void **)data);
            break;
        case TYPE_RECT:
        {
            const Rect *r = (const Rect *)data;
            out = String8::format("Rect(%d, %d, %d, %d)",
                                  r->mLeft, r->mTop, r->mRight, r->mBottom);
            break;
        }

        default:
            out = String8::format("(unknown type %d, size %zu)", mType, mSize);
            if (verbose && mSize <= 48) { // if it's less than three lines of hex data, dump it
                AString foo;
                hexdump(data, mSize, 0, &foo);
                out.append("\n");
                out.append(foo.c_str());
            }
            break;
    }
    return out;
}

static void MakeFourCCString(uint32_t x, char *s) {
    s[0] = x >> 24;
    s[1] = (x >> 16) & 0xff;
    s[2] = (x >> 8) & 0xff;
    s[3] = x & 0xff;
    s[4] = '\0';
}

String8 MetaDataBase::toString() const {
    String8 s;
    for (int i = mInternalData->mItems.size(); --i >= 0;) {
        int32_t key = mInternalData->mItems.keyAt(i);
        char cc[5];
        MakeFourCCString(key, cc);
        const typed_data &item = mInternalData->mItems.valueAt(i);
        s.appendFormat("%s: %s", cc, item.asString(false).string());
        if (i != 0) {
            s.append(", ");
        }
    }
    return s;
}

void MetaDataBase::dumpToLog() const {
    for (int i = mInternalData->mItems.size(); --i >= 0;) {
        int32_t key = mInternalData->mItems.keyAt(i);
        char cc[5];
        MakeFourCCString(key, cc);
        const typed_data &item = mInternalData->mItems.valueAt(i);
        ALOGI("%s: %s", cc, item.asString(true /* verbose */).string());
    }
}

status_t MetaDataBase::writeToParcel(Parcel &parcel) {
    status_t ret;
    size_t numItems = mInternalData->mItems.size();
    ret = parcel.writeUint32(uint32_t(numItems));
    if (ret) {
        return ret;
    }
    for (size_t i = 0; i < numItems; i++) {
        int32_t key = mInternalData->mItems.keyAt(i);
        const typed_data &item = mInternalData->mItems.valueAt(i);
        uint32_t type;
        const void *data;
        size_t size;
        item.getData(&type, &data, &size);
        ret = parcel.writeInt32(key);
        if (ret) {
            return ret;
        }
        ret = parcel.writeUint32(type);
        if (ret) {
            return ret;
        }
        if (type == TYPE_NONE) {
            android::Parcel::WritableBlob blob;
            ret = parcel.writeUint32(static_cast<uint32_t>(size));
            if (ret) {
                return ret;
            }
            ret = parcel.writeBlob(size, false, &blob);
            if (ret) {
                return ret;
            }
            memcpy(blob.data(), data, size);
            blob.release();
        } else {
            ret = parcel.writeByteArray(size, (uint8_t*)data);
            if (ret) {
                return ret;
            }
        }
    }
    return OK;
}

status_t MetaDataBase::updateFromParcel(const Parcel &parcel) {
    uint32_t numItems;
    if (parcel.readUint32(&numItems) == OK) {

        for (size_t i = 0; i < numItems; i++) {
            int32_t key;
            uint32_t type;
            uint32_t size;
            status_t ret = parcel.readInt32(&key);
            ret |= parcel.readUint32(&type);
            ret |= parcel.readUint32(&size);
            if (ret != OK) {
                break;
            }
            // copy data from Blob, which may be inline in Parcel storage,
            // then advance position
            if (type == TYPE_NONE) {
                android::Parcel::ReadableBlob blob;
                ret = parcel.readBlob(size, &blob);
                if (ret != OK) {
                    break;
                }
                setData(key, type, blob.data(), size);
                blob.release();
            } else {
                // copy data directly from Parcel storage, then advance position
                setData(key, type, parcel.readInplace(size), size);
            }
         }

        return OK;
    }
    ALOGW("no metadata in parcel");
    return UNKNOWN_ERROR;
}

}  // namespace android

