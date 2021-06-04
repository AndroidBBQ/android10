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

#include <img_utils/TiffIfd.h>
#include <img_utils/TiffHelpers.h>
#include <img_utils/TiffEntry.h>

#include <utils/Errors.h>
#include <utils/StrongPointer.h>
#include <utils/Vector.h>

namespace android {
namespace img_utils {

TiffEntry::~TiffEntry() {}

/**
 * Specialize for each valid type, including sub-IFDs.
 *
 * Values with types other than the ones given here should not compile.
 */

template<>
const sp<TiffIfd>* TiffEntry::forceValidType<sp<TiffIfd> >(TagType type, const sp<TiffIfd>* value) {
    if (type == LONG) {
        return value;
    }
    ALOGE("%s: Value of type 'ifd' is not valid for tag with TIFF type %d.",
            __FUNCTION__, type);
    return NULL;
}

template<>
const uint8_t* TiffEntry::forceValidType<uint8_t>(TagType type, const uint8_t* value) {
    if (type == BYTE || type == ASCII || type == UNDEFINED) {
        return value;
    }
    ALOGE("%s: Value of type 'uint8_t' is not valid for tag with TIFF type %d.",
            __FUNCTION__, type);
    return NULL;
}

template<>
const int8_t* TiffEntry::forceValidType<int8_t>(TagType type, const int8_t* value) {
    if (type == SBYTE || type == ASCII || type == UNDEFINED) {
        return value;
    }
    ALOGE("%s: Value of type 'int8_t' is not valid for tag with TIFF type %d.",
            __FUNCTION__, type);
    return NULL;
}

template<>
const uint16_t* TiffEntry::forceValidType<uint16_t>(TagType type, const uint16_t* value) {
    if (type == SHORT) {
        return value;
    }
    ALOGE("%s: Value of type 'uint16_t' is not valid for tag with TIFF type %d.",
            __FUNCTION__, type);
    return NULL;
}

template<>
const int16_t* TiffEntry::forceValidType<int16_t>(TagType type, const int16_t* value) {
    if (type == SSHORT) {
        return value;
    }
    ALOGE("%s: Value of type 'int16_t' is not valid for tag with TIFF type %d.",
            __FUNCTION__, type);
    return NULL;
}

template<>
const uint32_t* TiffEntry::forceValidType<uint32_t>(TagType type, const uint32_t* value) {
    if (type == LONG || type == RATIONAL) {
        return value;
    }
    ALOGE("%s: Value of type 'uint32_t' is not valid for tag with TIFF type %d.",
            __FUNCTION__, type);
    return NULL;
}

template<>
const int32_t* TiffEntry::forceValidType<int32_t>(TagType type, const int32_t* value) {
    if (type == SLONG || type == SRATIONAL) {
        return value;
    }
    ALOGE("%s: Value of type 'int32_t' is not valid for tag with TIFF type %d.",
            __FUNCTION__, type);
    return NULL;
}

template<>
const double* TiffEntry::forceValidType<double>(TagType type, const double* value) {
    if (type == DOUBLE) {
        return value;
    }
    ALOGE("%s: Value of type 'double' is not valid for tag with TIFF type %d.",
            __FUNCTION__, type);
    return NULL;
}

template<>
const float* TiffEntry::forceValidType<float>(TagType type, const float* value) {
    if (type == FLOAT) {
        return value;
    }
    ALOGE("%s: Value of type 'float' is not valid for tag with TIFF type %d.",
            __FUNCTION__, type);
    return NULL;
}

String8 TiffEntry::toString() const {
    String8 output;
    uint32_t count = getCount();
    output.appendFormat("[id: %x, type: %d, count: %u, value: '", getTag(), getType(), count);

    size_t cappedCount = count;
    if (count > MAX_PRINT_STRING_LENGTH) {
        cappedCount = MAX_PRINT_STRING_LENGTH;
    }

    TagType type = getType();
    switch (type) {
        case UNDEFINED:
        case BYTE: {
            const uint8_t* typed_data = getData<uint8_t>();
            for (size_t i = 0; i < cappedCount; ++i) {
                output.appendFormat("%u ", typed_data[i]);
            }
            break;
        }
        case ASCII: {
            const char* typed_data = reinterpret_cast<const char*>(getData<uint8_t>());
            size_t len = count;
            if (count > MAX_PRINT_STRING_LENGTH) {
                 len = MAX_PRINT_STRING_LENGTH;
            }
            output.append(typed_data, len);
            break;
        }
        case SHORT: {
            const uint16_t* typed_data = getData<uint16_t>();
            for (size_t i = 0; i < cappedCount; ++i) {
                output.appendFormat("%u ", typed_data[i]);
            }
            break;
        }
        case LONG: {
            const uint32_t* typed_data = getData<uint32_t>();
            for (size_t i = 0; i < cappedCount; ++i) {
                output.appendFormat("%u ", typed_data[i]);
            }
            break;
        }
        case RATIONAL: {
            const uint32_t* typed_data = getData<uint32_t>();
            cappedCount <<= 1;
            for (size_t i = 0; i < cappedCount; i+=2) {
                output.appendFormat("%u/%u ", typed_data[i], typed_data[i + 1]);
            }
            break;
        }
        case SBYTE: {
            const int8_t* typed_data = getData<int8_t>();
            for (size_t i = 0; i < cappedCount; ++i) {
                output.appendFormat("%d ", typed_data[i]);
            }
            break;
        }
        case SSHORT: {
            const int16_t* typed_data = getData<int16_t>();
            for (size_t i = 0; i < cappedCount; ++i) {
                output.appendFormat("%d ", typed_data[i]);
            }
            break;
        }
        case SLONG: {
            const int32_t* typed_data = getData<int32_t>();
            for (size_t i = 0; i < cappedCount; ++i) {
                output.appendFormat("%d ", typed_data[i]);
            }
            break;
        }
        case SRATIONAL: {
            const int32_t* typed_data = getData<int32_t>();
            cappedCount <<= 1;
            for (size_t i = 0; i < cappedCount; i+=2) {
                output.appendFormat("%d/%d ", typed_data[i], typed_data[i + 1]);
            }
            break;
        }
        case FLOAT: {
            const float* typed_data = getData<float>();
            for (size_t i = 0; i < cappedCount; ++i) {
                output.appendFormat("%f ", typed_data[i]);
            }
            break;
        }
        case DOUBLE: {
            const double* typed_data = getData<double>();
            for (size_t i = 0; i < cappedCount; ++i) {
                output.appendFormat("%f ", typed_data[i]);
            }
            break;
        }
        default: {
            output.append("unknown type ");
            break;
        }
    }

    if (count > MAX_PRINT_STRING_LENGTH) {
        output.append("...");
    }
    output.append("']");
    return output;
}

} /*namespace img_utils*/
} /*namespace android*/
