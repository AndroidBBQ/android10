/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef _HEIF_DECODER_API_
#define _HEIF_DECODER_API_

#include <memory>

/*
 * The output color pixel format of heif decoder.
 */
typedef enum {
    kHeifColorFormat_RGB565     = 0,
    kHeifColorFormat_RGBA_8888  = 1,
    kHeifColorFormat_BGRA_8888  = 2,
} HeifColorFormat;

/*
 * The color spaces encoded in the heif image.
 */
typedef enum {
    kHeifEncodedColor_RGB = 0,
    kHeifEncodedColor_YUV = 1,
    kHeifEncodedColor_CMYK = 2,
} HeifEncodedColor;

/*
 * Represents a color converted (RGB-based) video frame
 */
struct HeifFrameInfo
{
    HeifFrameInfo() :
        mWidth(0), mHeight(0), mRotationAngle(0), mBytesPerPixel(0),
        mIccSize(0), mIccData(nullptr) {}

    // update the frame info, will make a copy of |iccData| internally
    void set(uint32_t width, uint32_t height, int32_t rotation, uint32_t bpp,
            uint32_t iccSize, uint8_t* iccData) {
        mWidth = width;
        mHeight = height;
        mRotationAngle = rotation;
        mBytesPerPixel = bpp;

        if (mIccData != nullptr) {
            mIccData.reset(nullptr);
        }
        mIccSize = iccSize;
        if (iccSize > 0) {
            mIccData.reset(new uint8_t[iccSize]);
            if (mIccData.get() != nullptr) {
                memcpy(mIccData.get(), iccData, iccSize);
            } else {
                mIccSize = 0;
            }
        }
    }

    // Intentional public access modifiers:
    uint32_t mWidth;
    uint32_t mHeight;
    int32_t  mRotationAngle;           // Rotation angle, clockwise, should be multiple of 90
    uint32_t mBytesPerPixel;           // Number of bytes for one pixel
    uint32_t mIccSize;                 // Number of bytes in mIccData
    std::unique_ptr<uint8_t[]> mIccData; // Actual ICC data, memory is owned by this structure
};

/*
 * Abstract interface to provide data to HeifDecoder.
 */
struct HeifStream {
    HeifStream() {}

    virtual ~HeifStream() {}

    /*
     * Reads or skips size number of bytes. return the number of bytes actually
     * read or skipped.
     * If |buffer| == NULL, skip size bytes, return how many were skipped.
     * If |buffer| != NULL, copy size bytes into buffer, return how many were copied.
     */
    virtual size_t read(void* buffer, size_t size) = 0;

    /*
     * Rewinds to the beginning of the stream. Returns true if the stream is known
     * to be at the beginning after this call returns.
     */
    virtual bool rewind() = 0;

    /*
     * Seeks to an absolute position in the stream. If this cannot be done, returns false.
     * If an attempt is made to seek past the end of the stream, the position will be set
     * to the end of the stream.
     */
    virtual bool seek(size_t /*position*/) = 0;

    /** Returns true if this stream can report its total length. */
    virtual bool hasLength() const = 0;

    /** Returns the total length of the stream. If this cannot be done, returns 0. */
    virtual size_t getLength() const = 0;

private:
    HeifStream(const HeifFrameInfo&) = delete;
    HeifStream& operator=(const HeifFrameInfo&) = delete;
};

/*
 * Abstract interface to decode heif images from a HeifStream data source.
 */
struct HeifDecoder {
    HeifDecoder() {}

    virtual ~HeifDecoder() {}

    /*
     * Returns true if it successfully sets outColor to the encoded color,
     * and false otherwise.
     */
    virtual bool getEncodedColor(HeifEncodedColor* outColor) const = 0;

    /*
     * Returns true if it successfully sets the output color format to color,
     * and false otherwise.
     */
    virtual bool setOutputColor(HeifColorFormat color) = 0;

    /*
     * Returns true if it successfully initialize heif decoder with source,
     * and false otherwise. |frameInfo| will be filled with information of
     * the primary picture upon success and unmodified upon failure.
     * Takes ownership of |stream| regardless of result.
     */
    virtual bool init(HeifStream* stream, HeifFrameInfo* frameInfo) = 0;

    /*
     * Decode the picture internally, returning whether it succeeded. |frameInfo|
     * will be filled with information of the primary picture upon success and
     * unmodified upon failure.
     *
     * After this succeeded, getScanline can be called to read the scanlines
     * that were decoded.
     */
    virtual bool decode(HeifFrameInfo* frameInfo) = 0;

    /*
     * Read the next scanline (in top-down order), returns true upon success
     * and false otherwise.
     */
    virtual bool getScanline(uint8_t* dst) = 0;

    /*
     * Skip the next |count| scanlines, returns true upon success and
     * false otherwise.
     */
    virtual size_t skipScanlines(size_t count) = 0;

private:
    HeifDecoder(const HeifFrameInfo&) = delete;
    HeifDecoder& operator=(const HeifFrameInfo&) = delete;
};

/*
 * Creates a HeifDecoder. Returns a HeifDecoder instance upon success, or NULL
 * if the creation has failed.
 */
HeifDecoder* createHeifDecoder();

#endif // _HEIF_DECODER_API_
