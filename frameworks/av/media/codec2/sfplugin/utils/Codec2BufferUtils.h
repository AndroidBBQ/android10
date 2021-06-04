/*
 * Copyright 2018, The Android Open Source Project
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

#ifndef CODEC2_BUFFER_UTILS_H_
#define CODEC2_BUFFER_UTILS_H_

#include <C2Buffer.h>
#include <C2ParamDef.h>

#include <media/hardware/VideoAPI.h>
#include <utils/Errors.h>

namespace android {

/**
 * Converts an RGB view to planar YUV 420 media image.
 *
 * \param dstY       pointer to media image buffer
 * \param dstStride  stride in bytes
 * \param dstVStride vertical stride in pixels
 * \param bufferSize media image buffer size
 * \param src source image
 *
 * \retval NO_MEMORY media image is too small
 * \retval OK on success
 */
status_t ConvertRGBToPlanarYUV(
        uint8_t *dstY, size_t dstStride, size_t dstVStride, size_t bufferSize,
        const C2GraphicView &src);

/**
 * Returns a planar YUV 420 8-bit media image descriptor.
 *
 * \param width width of image in pixels
 * \param height height of image in pixels
 * \param stride stride of image in pixels
 * \param vstride vertical stride of image in pixels
 */
MediaImage2 CreateYUV420PlanarMediaImage2(
        uint32_t width, uint32_t height, uint32_t stride, uint32_t vstride);

/**
 * Returns a semiplanar YUV 420 8-bit media image descriptor.
 *
 * \param width width of image in pixels
 * \param height height of image in pixels
 * \param stride stride of image in pixels
 * \param vstride vertical stride of image in pixels
 */
MediaImage2 CreateYUV420SemiPlanarMediaImage2(
        uint32_t width, uint32_t height, uint32_t stride, uint32_t vstride);

/**
 * Copies a graphic view into a media image.
 *
 * \param imgBase base of MediaImage
 * \param img MediaImage data
 * \param view graphic view
 *
 * \return OK on success
 */
status_t ImageCopy(uint8_t *imgBase, const MediaImage2 *img, const C2GraphicView &view);

/**
 * Copies a media image into a graphic view.
 *
 * \param view graphic view
 * \param imgBase base of MediaImage
 * \param img MediaImage data
 *
 * \return OK on success
 */
status_t ImageCopy(C2GraphicView &view, const uint8_t *imgBase, const MediaImage2 *img);

/**
 * Returns true iff a view has a YUV 420 888 layout.
 */
bool IsYUV420(const C2GraphicView &view);

/**
 * Returns true iff a view has a NV12 layout.
 */
bool IsNV12(const C2GraphicView &view);

/**
 * Returns true iff a view has a I420 layout.
 */
bool IsI420(const C2GraphicView &view);

/**
 * Returns true iff a MediaImage2 has a YUV 420 888 layout.
 */
bool IsYUV420(const MediaImage2 *img);

/**
 * Returns true iff a MediaImage2 has a NV12 layout.
 */
bool IsNV12(const MediaImage2 *img);

/**
 * Returns true iff a MediaImage2 has a I420 layout.
 */
bool IsI420(const MediaImage2 *img);

/**
 * A raw memory block to use for internal buffers.
 *
 * TODO: replace this with C2LinearBlocks from a private C2BlockPool
 */
struct MemoryBlock : public C2MemoryBlock<uint8_t> {
    virtual const uint8_t* data() const override;
    virtual size_t size() const override;

    inline uint8_t *data() {
        return const_cast<uint8_t*>(const_cast<const MemoryBlock*>(this)->data());
    }

    // allocates an unmanaged block (not in a pool)
    static MemoryBlock Allocate(size_t);

    // memory block with no actual memory (size is 0, data is null)
    MemoryBlock();

    struct Impl;
    MemoryBlock(std::shared_ptr<Impl> impl);
    virtual ~MemoryBlock();

private:
    std::shared_ptr<Impl> mImpl;
};

/**
 * A raw memory mini-pool.
 */
struct MemoryBlockPool {
    /**
     * Fetches a block with a given size.
     *
     * \param size size in bytes
     */
    MemoryBlock fetch(size_t size);

    MemoryBlockPool();
    ~MemoryBlockPool() = default;

private:
    struct Impl;
    std::shared_ptr<Impl> mImpl;
};

} // namespace android

#endif  // CODEC2_BUFFER_UTILS_H_
