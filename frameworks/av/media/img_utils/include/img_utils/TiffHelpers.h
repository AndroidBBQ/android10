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

#ifndef IMG_UTILS_TIFF_HELPERS_H
#define IMG_UTILS_TIFF_HELPERS_H

#include <stdint.h>

namespace android {
namespace img_utils {

const uint8_t ZERO_WORD[] = {0, 0, 0, 0};

#define BAIL_ON_FAIL(x, flag) \
    if (((flag) = (x)) != OK) return flag;

#define BYTES_TILL_WORD(index) \
    ((TIFF_WORD_SIZE - ((index) % TIFF_WORD_SIZE)) % TIFF_WORD_SIZE)

#define WORD_ALIGN(count) \
    count += BYTES_TILL_WORD(count);

#define ZERO_TILL_WORD(output, index, ret) \
    { \
        size_t remaining = BYTES_TILL_WORD(index); \
        if (remaining > 0) { \
            BAIL_ON_FAIL((output)->write(ZERO_WORD, 0, remaining), ret); \
        } \
    }

/**
 * Basic TIFF header constants.
 */
enum {
    BAD_OFFSET = 0,
    TIFF_WORD_SIZE = 4, // Size in bytes
    IFD_HEADER_SIZE = 2, // Size in bytes
    IFD_FOOTER_SIZE = 4, // Size in bytes
    TIFF_ENTRY_SIZE = 12, // Size in bytes
    MAX_IFD_ENTRIES = UINT16_MAX,
    FILE_HEADER_SIZE = 8, // Size in bytes
    ENDIAN_MARKER_SIZE = 2, // Size in bytes
    TIFF_MARKER_SIZE = 2, // Size in bytes
    OFFSET_MARKER_SIZE = 4, // Size in bytes
    TIFF_FILE_MARKER = 42,
    BIG_ENDIAN_MARKER = 0x4D4Du,
    LITTLE_ENDIAN_MARKER = 0x4949u
};

/**
 * Constants for the TIFF tag types.
 */
enum TagType {
    UNKNOWN_TAGTYPE = 0,
    BYTE=1,
    ASCII,
    SHORT,
    LONG,
    RATIONAL,
    SBYTE,
    UNDEFINED,
    SSHORT,
    SLONG,
    SRATIONAL,
    FLOAT,
    DOUBLE
};

/**
 * Sizes of the TIFF entry fields (in bytes).
 */
enum {
    TAG_SIZE = 2,
    TYPE_SIZE = 2,
    COUNT_SIZE = 4,
    OFFSET_SIZE = 4
};

/**
 * Convenience IFD id constants.
 */
enum {
    IFD_0 = 0,
    RAW_IFD,
    PROFILE_IFD,
    PREVIEW_IFD
};

inline size_t getTypeSize(TagType type) {
    switch(type) {
        case UNDEFINED:
        case ASCII:
        case BYTE:
        case SBYTE:
            return 1;
        case SHORT:
        case SSHORT:
            return 2;
        case LONG:
        case SLONG:
        case FLOAT:
            return 4;
        case RATIONAL:
        case SRATIONAL:
        case DOUBLE:
            return 8;
        default:
            return 0;
    }
}

inline uint32_t calculateIfdSize(size_t numberOfEntries) {
    return IFD_HEADER_SIZE + IFD_FOOTER_SIZE + TIFF_ENTRY_SIZE * numberOfEntries;
}

} /*namespace img_utils*/
} /*namespace android*/

#endif /*IMG_UTILS_TIFF_HELPERS_H*/
