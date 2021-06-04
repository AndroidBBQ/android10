/*
 * Copyright 2014 The Android Open Source Project
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

#ifndef IMG_UTILS_TIFF_ENTRY_IMPL
#define IMG_UTILS_TIFF_ENTRY_IMPL

#include <img_utils/TiffIfd.h>
#include <img_utils/TiffEntry.h>
#include <img_utils/TiffHelpers.h>
#include <img_utils/Output.h>
#include <img_utils/EndianUtils.h>

#include <utils/Log.h>
#include <utils/Errors.h>
#include <utils/Vector.h>
#include <utils/StrongPointer.h>
#include <stdint.h>

namespace android {
namespace img_utils {

template<typename T>
class TiffEntryImpl : public TiffEntry {
    public:
        TiffEntryImpl(uint16_t tag, TagType type, uint32_t count, Endianness end, const T* data);
        virtual ~TiffEntryImpl();

        status_t writeData(uint32_t offset, /*out*/EndianOutput* out) const;
        status_t writeTagInfo(uint32_t offset, /*out*/EndianOutput* out) const;

        uint32_t getCount() const;
        uint16_t getTag() const;
        TagType getType() const;
        Endianness getEndianness() const;
        size_t getSize() const;
        uint32_t getComparableValue() const;

    protected:
        const void* getDataHelper() const;
        uint32_t getActualSize() const;

        uint16_t mTag;
        uint16_t mType;
        uint32_t mCount;
        Endianness mEnd;
        Vector<T> mData;

};

template<typename T>
TiffEntryImpl<T>::TiffEntryImpl(uint16_t tag, TagType type, uint32_t count, Endianness end,
        const T* data)
        : mTag(tag), mType(static_cast<uint16_t>(type)), mCount(count), mEnd(end) {
    count = (type == RATIONAL || type == SRATIONAL) ? count * 2 : count;
    ssize_t index = mData.appendArray(data, count);
    LOG_ALWAYS_FATAL_IF(index < 0, "%s: Could not allocate vector for data.", __FUNCTION__);
}

template<typename T>
TiffEntryImpl<T>::~TiffEntryImpl() {}

template<typename T>
uint32_t TiffEntryImpl<T>::getCount() const {
    return mCount;
}

template<typename T>
uint16_t TiffEntryImpl<T>::getTag() const {
    return mTag;
}

template<typename T>
TagType TiffEntryImpl<T>::getType() const {
    return static_cast<TagType>(mType);
}

template<typename T>
const void* TiffEntryImpl<T>::getDataHelper() const {
    return reinterpret_cast<const void*>(mData.array());
}

template<typename T>
size_t TiffEntryImpl<T>::getSize() const {
    uint32_t total = getActualSize();
    WORD_ALIGN(total)
    return (total <= OFFSET_SIZE) ? 0 : total;
}

template<typename T>
uint32_t TiffEntryImpl<T>::getActualSize() const {
    uint32_t total = sizeof(T) * mCount;
    if (getType() == RATIONAL || getType() == SRATIONAL) {
        // 2 ints stored for each rational, multiply by 2
        total <<= 1;
    }
    return total;
}

template<typename T>
Endianness TiffEntryImpl<T>::getEndianness() const {
    return mEnd;
}

template<typename T>
uint32_t TiffEntryImpl<T>::getComparableValue() const {
    return mTag;
}

template<typename T>
status_t TiffEntryImpl<T>::writeTagInfo(uint32_t offset, /*out*/EndianOutput* out) const {
    assert((offset % TIFF_WORD_SIZE) == 0);
    status_t ret = OK;
    BAIL_ON_FAIL(out->write(&mTag, 0, 1), ret);
    BAIL_ON_FAIL(out->write(&mType, 0, 1), ret);
    BAIL_ON_FAIL(out->write(&mCount, 0, 1), ret);

    uint32_t dataSize = getActualSize();
    if (dataSize > OFFSET_SIZE) {
        BAIL_ON_FAIL(out->write(&offset, 0, 1), ret);
    } else {
        uint32_t count = mCount;
        if (getType() == RATIONAL || getType() == SRATIONAL) {
            /**
             * Rationals are stored as an array of ints.  Each
             * rational is represented by 2 ints.  To recover the
             * size of the array here, multiply the count by 2.
             */
            count <<= 1;
        }
        BAIL_ON_FAIL(out->write(mData.array(), 0, count), ret);
        ZERO_TILL_WORD(out, dataSize, ret);
    }
    return ret;
}

template<typename T>
status_t TiffEntryImpl<T>::writeData(uint32_t /*offset*/, EndianOutput* out) const {
    status_t ret = OK;

    // Some tags have fixed-endian value output
    Endianness tmp = UNDEFINED_ENDIAN;
    if (mEnd != UNDEFINED_ENDIAN) {
        tmp = out->getEndianness();
        out->setEndianness(mEnd);
    }

    uint32_t count = mCount;
    if (getType() == RATIONAL || getType() == SRATIONAL) {
        /**
         * Rationals are stored as an array of ints.  Each
         * rational is represented by 2 ints.  To recover the
         * size of the array here, multiply the count by 2.
         */
        count <<= 1;
    }

    BAIL_ON_FAIL(out->write(mData.array(), 0, count), ret);

    if (mEnd != UNDEFINED_ENDIAN) {
        out->setEndianness(tmp);
    }

    // Write to next word alignment
    ZERO_TILL_WORD(out, sizeof(T) * count, ret);
    return ret;
}

template<>
inline status_t TiffEntryImpl<sp<TiffIfd> >::writeTagInfo(uint32_t offset,
        /*out*/EndianOutput* out) const {
    assert((offset % TIFF_WORD_SIZE) == 0);
    status_t ret = OK;
    BAIL_ON_FAIL(out->write(&mTag, 0, 1), ret);
    BAIL_ON_FAIL(out->write(&mType, 0, 1), ret);
    BAIL_ON_FAIL(out->write(&mCount, 0, 1), ret);

    BAIL_ON_FAIL(out->write(&offset, 0, 1), ret);
    return ret;
}

template<>
inline uint32_t TiffEntryImpl<sp<TiffIfd> >::getActualSize() const {
    uint32_t total = 0;
    for (size_t i = 0; i < mData.size(); ++i) {
        total += mData[i]->getSize();
    }
    return total;
}

template<>
inline status_t TiffEntryImpl<sp<TiffIfd> >::writeData(uint32_t offset, EndianOutput* out) const {
    status_t ret = OK;
    for (uint32_t i = 0; i < mCount; ++i) {
        BAIL_ON_FAIL(mData[i]->writeData(offset, out), ret);
        offset += mData[i]->getSize();
    }
    return ret;
}

} /*namespace img_utils*/
} /*namespace android*/

#endif /*IMG_UTILS_TIFF_ENTRY_IMPL*/


