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

#ifndef IMG_UTILS_TIFF_ENTRY
#define IMG_UTILS_TIFF_ENTRY

#include <img_utils/TiffWritable.h>
#include <img_utils/TiffHelpers.h>
#include <img_utils/EndianUtils.h>

#include <cutils/compiler.h>
#include <utils/String8.h>
#include <utils/Errors.h>
#include <stdint.h>

namespace android {
namespace img_utils {

#define COMPARE_DEF(op) \
inline bool operator op (const TiffEntry& entry) const;

/**
 * This class holds a single TIFF IFD entry.
 *
 * Subclasses are expected to support assignment and copying operations.
 */
class ANDROID_API TiffEntry : public TiffWritable {
    public:
        virtual ~TiffEntry();

        /**
         * Write the 12-byte IFD entry to the output. The given offset will be
         * set as the tag value if the size of the tag value exceeds the max
         * size for the TIFF Value field (4 bytes), and should be word aligned.
         *
         * Returns OK on success, or a negative error code on failure.
         */
        virtual status_t writeTagInfo(uint32_t offset, /*out*/EndianOutput* out) const = 0;

        /**
         * Get the count set for this entry. This corresponds to the TIFF Count
         * field.
         */
        virtual uint32_t getCount() const = 0;

        /**
         * Get the tag id set for this entry. This corresponds to the TIFF Tag
         * field.
         */
        virtual uint16_t getTag() const = 0;

        /**
         * Get the type set for this entry.  This corresponds to the TIFF Type
         * field.
         */
        virtual TagType getType() const = 0;

        /**
         * Get the defined endianness for this entry.  If this is defined,
         * the tag value will be written with the given byte order.
         */
        virtual Endianness getEndianness() const = 0;

        /**
         * Get the value for this entry.  This corresponds to the TIFF Value
         * field.
         *
         * Returns NULL if the value is NULL, or if the type used does not
         * match the type of this tag.
         */
        template<typename T>
        const T* getData() const;

        virtual String8 toString() const;

        /**
         * Force the type used here to be a valid TIFF type.
         *
         * Returns NULL if the given value is NULL, or if the type given does
         * not match the type of the value given.
         */
        template<typename T>
        static const T* forceValidType(TagType type, const T* value);

        virtual const void* getDataHelper() const = 0;

        COMPARE_DEF(>)
        COMPARE_DEF(<)

    protected:
        enum {
            MAX_PRINT_STRING_LENGTH = 256
        };
};

#define COMPARE(op) \
bool TiffEntry::operator op (const TiffEntry& entry) const { \
    return getComparableValue() op entry.getComparableValue(); \
}

COMPARE(>)
COMPARE(<)


template<typename T>
const T* TiffEntry::getData() const {
    const T* value = reinterpret_cast<const T*>(getDataHelper());
    return forceValidType<T>(getType(), value);
}

#undef COMPARE
#undef COMPARE_DEF

} /*namespace img_utils*/
} /*namespace android*/

#endif /*IMG_UTILS_TIFF_ENTRY*/
