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

#ifndef IMG_UTILS_ENDIAN_UTILS
#define IMG_UTILS_ENDIAN_UTILS

#include <img_utils/Output.h>

#include <cutils/compiler.h>
#include <utils/Errors.h>
#include <stdint.h>
#include <endian.h>
#include <assert.h>

namespace android {
namespace img_utils {

/**
 * Endianness types supported.
 */
enum ANDROID_API Endianness {
    UNDEFINED_ENDIAN, // Default endianness will be used.
    BIG,
    LITTLE
};

/**
 * Convert from the native device endianness to big endian.
 */
template<typename T>
T convertToBigEndian(T in);

/**
 * Convert from the native device endianness to little endian.
 */
template<typename T>
T convertToLittleEndian(T in);

/**
 * A utility class for writing to an Output with the given endianness.
 */
class ANDROID_API EndianOutput : public Output {
    public:
        /**
         * Wrap the given Output.  Calling write methods will result in
         * writes to this output.
         */
        explicit EndianOutput(Output* out, Endianness end=LITTLE);

        virtual ~EndianOutput();

        /**
         * Call open on the wrapped output.
         */
        virtual status_t open();

        /**
         * Call close on the wrapped output.
         */
        virtual status_t close();

        /**
         * Set the endianness to use when writing.
         */
        virtual void setEndianness(Endianness end);

        /**
         * Get the currently configured endianness.
         */
        virtual Endianness getEndianness() const;

        /**
         * Get the current number of bytes written by this EndianOutput.
         */
        virtual uint32_t getCurrentOffset() const;


        // TODO: switch write methods to uint32_t instead of size_t,
        // the max size of a TIFF files is bounded

        /**
         * The following methods will write elements from given input buffer to the output.
         * Count elements in the buffer will be written with the endianness set for this
         * EndianOutput.  If the given offset is greater than zero, that many elements will
         * be skipped in the buffer before writing.
         *
         * Returns OK on success, or a negative error code.
         */
        virtual status_t write(const uint8_t* buf, size_t offset, size_t count);

        virtual status_t write(const int8_t* buf, size_t offset, size_t count);

        virtual status_t write(const uint16_t* buf, size_t offset, size_t count);

        virtual status_t write(const int16_t* buf, size_t offset, size_t count);

        virtual status_t write(const uint32_t* buf, size_t offset, size_t count);

        virtual status_t write(const int32_t* buf, size_t offset, size_t count);

        virtual status_t write(const uint64_t* buf, size_t offset, size_t count);

        virtual status_t write(const int64_t* buf, size_t offset, size_t count);

        virtual status_t write(const float* buf, size_t offset, size_t count);

        virtual status_t write(const double* buf, size_t offset, size_t count);

    protected:
        template<typename T>
        inline status_t writeHelper(const T* buf, size_t offset, size_t count);

        uint32_t mOffset;
        Output* mOutput;
        Endianness mEndian;
};

template<typename T>
inline status_t EndianOutput::writeHelper(const T* buf, size_t offset, size_t count) {
    assert(offset <= count);
    status_t res = OK;
    size_t size = sizeof(T);
    switch(mEndian) {
        case BIG: {
            for (size_t i = offset; i < count; ++i) {
                T tmp = convertToBigEndian<T>(buf[offset + i]);
                if ((res = mOutput->write(reinterpret_cast<uint8_t*>(&tmp), 0, size))
                        != OK) {
                    return res;
                }
                mOffset += size;
            }
            break;
        }
        case LITTLE: {
            for (size_t i = offset; i < count; ++i) {
                T tmp = convertToLittleEndian<T>(buf[offset + i]);
                if ((res = mOutput->write(reinterpret_cast<uint8_t*>(&tmp), 0, size))
                        != OK) {
                    return res;
                }
                mOffset += size;
            }
            break;
        }
        default: {
            return BAD_VALUE;
        }
    }
    return res;
}

template<>
inline uint8_t convertToBigEndian(uint8_t in) {
    return in;
}

template<>
inline int8_t convertToBigEndian(int8_t in) {
    return in;
}

template<>
inline uint16_t convertToBigEndian(uint16_t in) {
    return htobe16(in);
}

template<>
inline int16_t convertToBigEndian(int16_t in) {
    return htobe16(in);
}

template<>
inline uint32_t convertToBigEndian(uint32_t in) {
    return htobe32(in);
}

template<>
inline int32_t convertToBigEndian(int32_t in) {
    return htobe32(in);
}

template<>
inline uint64_t convertToBigEndian(uint64_t in) {
    return htobe64(in);
}

template<>
inline int64_t convertToBigEndian(int64_t in) {
    return htobe64(in);
}

template<>
inline uint8_t convertToLittleEndian(uint8_t in) {
    return in;
}

template<>
inline int8_t convertToLittleEndian(int8_t in) {
    return in;
}

template<>
inline uint16_t convertToLittleEndian(uint16_t in) {
    return htole16(in);
}

template<>
inline int16_t convertToLittleEndian(int16_t in) {
    return htole16(in);
}

template<>
inline uint32_t convertToLittleEndian(uint32_t in) {
    return htole32(in);
}

template<>
inline int32_t convertToLittleEndian(int32_t in) {
    return htole32(in);
}

template<>
inline uint64_t convertToLittleEndian(uint64_t in) {
    return htole64(in);
}

template<>
inline int64_t convertToLittleEndian(int64_t in) {
    return htole64(in);
}

} /*namespace img_utils*/
} /*namespace android*/

#endif /*IMG_UTILS_ENDIAN_UTILS*/

