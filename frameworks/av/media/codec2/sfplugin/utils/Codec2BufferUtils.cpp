/*
 * Copyright 2018, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "Codec2BufferUtils"
#include <utils/Log.h>

#include <libyuv.h>

#include <list>
#include <mutex>

#include <media/hardware/HardwareAPI.h>
#include <media/stagefright/foundation/AUtils.h>

#include <C2Debug.h>

#include "Codec2BufferUtils.h"

namespace android {

namespace {

/**
 * A flippable, optimizable memcpy. Constructs such as (from ? src : dst) do not work as the results are
 * always const.
 */
template<bool ToA, size_t S>
struct MemCopier {
    template<typename A, typename B>
    inline static void copy(A *a, const B *b, size_t size) {
        __builtin_memcpy(a, b, size);
    }
};

template<size_t S>
struct MemCopier<false, S> {
    template<typename A, typename B>
    inline static void copy(const A *a, B *b, size_t size) {
        MemCopier<true, S>::copy(b, a, size);
    }
};

/**
 * Copies between a MediaImage and a graphic view.
 *
 * \param ToMediaImage whether to copy to (or from) the MediaImage
 * \param view graphic view (could be ConstGraphicView or GraphicView depending on direction)
 * \param img MediaImage data
 * \param imgBase base of MediaImage (could be const uint8_t* or uint8_t* depending on direction)
 */
template<bool ToMediaImage, typename View, typename ImagePixel>
static status_t _ImageCopy(View &view, const MediaImage2 *img, ImagePixel *imgBase) {
    // TODO: more efficient copying --- e.g. copy interleaved planes together, etc.
    const C2PlanarLayout &layout = view.layout();
    const size_t bpp = divUp(img->mBitDepthAllocated, 8u);

    for (uint32_t i = 0; i < layout.numPlanes; ++i) {
        typename std::conditional<ToMediaImage, uint8_t, const uint8_t>::type *imgRow =
            imgBase + img->mPlane[i].mOffset;
        typename std::conditional<ToMediaImage, const uint8_t, uint8_t>::type *viewRow =
            viewRow = view.data()[i];
        const C2PlaneInfo &plane = layout.planes[i];
        if (plane.colSampling != img->mPlane[i].mHorizSubsampling
                || plane.rowSampling != img->mPlane[i].mVertSubsampling
                || plane.allocatedDepth != img->mBitDepthAllocated
                || plane.allocatedDepth < plane.bitDepth
                // MediaImage only supports MSB values
                || plane.rightShift != plane.allocatedDepth - plane.bitDepth
                || (bpp > 1 && plane.endianness != plane.NATIVE)) {
            return BAD_VALUE;
        }

        uint32_t planeW = img->mWidth / plane.colSampling;
        uint32_t planeH = img->mHeight / plane.rowSampling;

        bool canCopyByRow = (plane.colInc == 1) && (img->mPlane[i].mColInc == 1);
        bool canCopyByPlane = canCopyByRow && (plane.rowInc == img->mPlane[i].mRowInc);
        if (canCopyByPlane) {
            MemCopier<ToMediaImage, 0>::copy(imgRow, viewRow, plane.rowInc * planeH);
        } else if (canCopyByRow) {
            for (uint32_t row = 0; row < planeH; ++row) {
                MemCopier<ToMediaImage, 0>::copy(
                        imgRow, viewRow, std::min(plane.rowInc, img->mPlane[i].mRowInc));
                imgRow += img->mPlane[i].mRowInc;
                viewRow += plane.rowInc;
            }
        } else {
            for (uint32_t row = 0; row < planeH; ++row) {
                decltype(imgRow) imgPtr = imgRow;
                decltype(viewRow) viewPtr = viewRow;
                for (uint32_t col = 0; col < planeW; ++col) {
                    MemCopier<ToMediaImage, 0>::copy(imgPtr, viewPtr, bpp);
                    imgPtr += img->mPlane[i].mColInc;
                    viewPtr += plane.colInc;
                }
                imgRow += img->mPlane[i].mRowInc;
                viewRow += plane.rowInc;
            }
        }
    }
    return OK;
}

}  // namespace

status_t ImageCopy(uint8_t *imgBase, const MediaImage2 *img, const C2GraphicView &view) {
    if (view.crop().width != img->mWidth || view.crop().height != img->mHeight) {
        return BAD_VALUE;
    }
    if ((IsNV12(view) && IsI420(img)) || (IsI420(view) && IsNV12(img))) {
        // Take shortcuts to use libyuv functions between NV12 and I420 conversion.
        const uint8_t* src_y = view.data()[0];
        const uint8_t* src_u = view.data()[1];
        const uint8_t* src_v = view.data()[2];
        int32_t src_stride_y = view.layout().planes[0].rowInc;
        int32_t src_stride_u = view.layout().planes[1].rowInc;
        int32_t src_stride_v = view.layout().planes[2].rowInc;
        uint8_t* dst_y = imgBase + img->mPlane[0].mOffset;
        uint8_t* dst_u = imgBase + img->mPlane[1].mOffset;
        uint8_t* dst_v = imgBase + img->mPlane[2].mOffset;
        int32_t dst_stride_y = img->mPlane[0].mRowInc;
        int32_t dst_stride_u = img->mPlane[1].mRowInc;
        int32_t dst_stride_v = img->mPlane[2].mRowInc;
        if (IsNV12(view) && IsI420(img)) {
            if (!libyuv::NV12ToI420(src_y, src_stride_y, src_u, src_stride_u, dst_y, dst_stride_y,
                                    dst_u, dst_stride_u, dst_v, dst_stride_v, view.crop().width,
                                    view.crop().height)) {
                return OK;
            }
        } else {
            if (!libyuv::I420ToNV12(src_y, src_stride_y, src_u, src_stride_u, src_v, src_stride_v,
                                    dst_y, dst_stride_y, dst_u, dst_stride_u, view.crop().width,
                                    view.crop().height)) {
                return OK;
            }
        }
    }
    return _ImageCopy<true>(view, img, imgBase);
}

status_t ImageCopy(C2GraphicView &view, const uint8_t *imgBase, const MediaImage2 *img) {
    if (view.crop().width != img->mWidth || view.crop().height != img->mHeight) {
        return BAD_VALUE;
    }
    if ((IsNV12(img) && IsI420(view)) || (IsI420(img) && IsNV12(view))) {
        // Take shortcuts to use libyuv functions between NV12 and I420 conversion.
        const uint8_t* src_y = imgBase + img->mPlane[0].mOffset;
        const uint8_t* src_u = imgBase + img->mPlane[1].mOffset;
        const uint8_t* src_v = imgBase + img->mPlane[2].mOffset;
        int32_t src_stride_y = img->mPlane[0].mRowInc;
        int32_t src_stride_u = img->mPlane[1].mRowInc;
        int32_t src_stride_v = img->mPlane[2].mRowInc;
        uint8_t* dst_y = view.data()[0];
        uint8_t* dst_u = view.data()[1];
        uint8_t* dst_v = view.data()[2];
        int32_t dst_stride_y = view.layout().planes[0].rowInc;
        int32_t dst_stride_u = view.layout().planes[1].rowInc;
        int32_t dst_stride_v = view.layout().planes[2].rowInc;
        if (IsNV12(img) && IsI420(view)) {
            if (!libyuv::NV12ToI420(src_y, src_stride_y, src_u, src_stride_u, dst_y, dst_stride_y,
                                    dst_u, dst_stride_u, dst_v, dst_stride_v, view.width(),
                                    view.height())) {
                return OK;
            }
        } else {
            if (!libyuv::I420ToNV12(src_y, src_stride_y, src_u, src_stride_u, src_v, src_stride_v,
                                    dst_y, dst_stride_y, dst_u, dst_stride_u, view.width(),
                                    view.height())) {
                return OK;
            }
        }
    }
    return _ImageCopy<false>(view, img, imgBase);
}

bool IsYUV420(const C2GraphicView &view) {
    const C2PlanarLayout &layout = view.layout();
    return (layout.numPlanes == 3
            && layout.type == C2PlanarLayout::TYPE_YUV
            && layout.planes[layout.PLANE_Y].channel == C2PlaneInfo::CHANNEL_Y
            && layout.planes[layout.PLANE_Y].allocatedDepth == 8
            && layout.planes[layout.PLANE_Y].bitDepth == 8
            && layout.planes[layout.PLANE_Y].rightShift == 0
            && layout.planes[layout.PLANE_Y].colSampling == 1
            && layout.planes[layout.PLANE_Y].rowSampling == 1
            && layout.planes[layout.PLANE_U].channel == C2PlaneInfo::CHANNEL_CB
            && layout.planes[layout.PLANE_U].allocatedDepth == 8
            && layout.planes[layout.PLANE_U].bitDepth == 8
            && layout.planes[layout.PLANE_U].rightShift == 0
            && layout.planes[layout.PLANE_U].colSampling == 2
            && layout.planes[layout.PLANE_U].rowSampling == 2
            && layout.planes[layout.PLANE_V].channel == C2PlaneInfo::CHANNEL_CR
            && layout.planes[layout.PLANE_V].allocatedDepth == 8
            && layout.planes[layout.PLANE_V].bitDepth == 8
            && layout.planes[layout.PLANE_V].rightShift == 0
            && layout.planes[layout.PLANE_V].colSampling == 2
            && layout.planes[layout.PLANE_V].rowSampling == 2);
}

bool IsNV12(const C2GraphicView &view) {
    if (!IsYUV420(view)) {
        return false;
    }
    const C2PlanarLayout &layout = view.layout();
    return (layout.rootPlanes == 2
            && layout.planes[layout.PLANE_U].colInc == 2
            && layout.planes[layout.PLANE_U].rootIx == layout.PLANE_U
            && layout.planes[layout.PLANE_U].offset == 0
            && layout.planes[layout.PLANE_V].colInc == 2
            && layout.planes[layout.PLANE_V].rootIx == layout.PLANE_U
            && layout.planes[layout.PLANE_V].offset == 1);
}

bool IsI420(const C2GraphicView &view) {
    if (!IsYUV420(view)) {
        return false;
    }
    const C2PlanarLayout &layout = view.layout();
    return (layout.rootPlanes == 3
            && layout.planes[layout.PLANE_U].colInc == 1
            && layout.planes[layout.PLANE_U].rootIx == layout.PLANE_U
            && layout.planes[layout.PLANE_U].offset == 0
            && layout.planes[layout.PLANE_V].colInc == 1
            && layout.planes[layout.PLANE_V].rootIx == layout.PLANE_V
            && layout.planes[layout.PLANE_V].offset == 0);
}

bool IsYUV420(const MediaImage2 *img) {
    return (img->mType == MediaImage2::MEDIA_IMAGE_TYPE_YUV
            && img->mNumPlanes == 3
            && img->mBitDepth == 8
            && img->mBitDepthAllocated == 8
            && img->mPlane[0].mHorizSubsampling == 1
            && img->mPlane[0].mVertSubsampling == 1
            && img->mPlane[1].mHorizSubsampling == 2
            && img->mPlane[1].mVertSubsampling == 2
            && img->mPlane[2].mHorizSubsampling == 2
            && img->mPlane[2].mVertSubsampling == 2);
}

bool IsNV12(const MediaImage2 *img) {
    if (!IsYUV420(img)) {
        return false;
    }
    return (img->mPlane[1].mColInc == 2
            && img->mPlane[2].mColInc == 2
            && (img->mPlane[2].mOffset - img->mPlane[1].mOffset == 1));
}

bool IsI420(const MediaImage2 *img) {
    if (!IsYUV420(img)) {
        return false;
    }
    return (img->mPlane[1].mColInc == 1
            && img->mPlane[2].mColInc == 1
            && img->mPlane[2].mOffset > img->mPlane[1].mOffset);
}

MediaImage2 CreateYUV420PlanarMediaImage2(
        uint32_t width, uint32_t height, uint32_t stride, uint32_t vstride) {
    return MediaImage2 {
        .mType = MediaImage2::MEDIA_IMAGE_TYPE_YUV,
        .mNumPlanes = 3,
        .mWidth = width,
        .mHeight = height,
        .mBitDepth = 8,
        .mBitDepthAllocated = 8,
        .mPlane = {
            {
                .mOffset = 0,
                .mColInc = 1,
                .mRowInc = (int32_t)stride,
                .mHorizSubsampling = 1,
                .mVertSubsampling = 1,
            },
            {
                .mOffset = stride * vstride,
                .mColInc = 1,
                .mRowInc = (int32_t)stride / 2,
                .mHorizSubsampling = 2,
                .mVertSubsampling = 2,
            },
            {
                .mOffset = stride * vstride * 5 / 4,
                .mColInc = 1,
                .mRowInc = (int32_t)stride / 2,
                .mHorizSubsampling = 2,
                .mVertSubsampling = 2,
            }
        },
    };
}

MediaImage2 CreateYUV420SemiPlanarMediaImage2(
        uint32_t width, uint32_t height, uint32_t stride, uint32_t vstride) {
    return MediaImage2 {
        .mType = MediaImage2::MEDIA_IMAGE_TYPE_YUV,
        .mNumPlanes = 3,
        .mWidth = width,
        .mHeight = height,
        .mBitDepth = 8,
        .mBitDepthAllocated = 8,
        .mPlane = {
            {
                .mOffset = 0,
                .mColInc = 1,
                .mRowInc = (int32_t)stride,
                .mHorizSubsampling = 1,
                .mVertSubsampling = 1,
            },
            {
                .mOffset = stride * vstride,
                .mColInc = 2,
                .mRowInc = (int32_t)stride,
                .mHorizSubsampling = 2,
                .mVertSubsampling = 2,
            },
            {
                .mOffset = stride * vstride + 1,
                .mColInc = 2,
                .mRowInc = (int32_t)stride,
                .mHorizSubsampling = 2,
                .mVertSubsampling = 2,
            }
        },
    };
}

status_t ConvertRGBToPlanarYUV(
        uint8_t *dstY, size_t dstStride, size_t dstVStride, size_t bufferSize,
        const C2GraphicView &src) {
    CHECK(dstY != nullptr);
    CHECK((src.width() & 1) == 0);
    CHECK((src.height() & 1) == 0);

    if (dstStride * dstVStride * 3 / 2 > bufferSize) {
        ALOGD("conversion buffer is too small for converting from RGB to YUV");
        return NO_MEMORY;
    }

    uint8_t *dstU = dstY + dstStride * dstVStride;
    uint8_t *dstV = dstU + (dstStride >> 1) * (dstVStride >> 1);

    const C2PlanarLayout &layout = src.layout();
    const uint8_t *pRed   = src.data()[C2PlanarLayout::PLANE_R];
    const uint8_t *pGreen = src.data()[C2PlanarLayout::PLANE_G];
    const uint8_t *pBlue  = src.data()[C2PlanarLayout::PLANE_B];

#define CLIP3(x,y,z) (((z) < (x)) ? (x) : (((z) > (y)) ? (y) : (z)))
    for (size_t y = 0; y < src.height(); ++y) {
        for (size_t x = 0; x < src.width(); ++x) {
            uint8_t red = *pRed;
            uint8_t green = *pGreen;
            uint8_t blue = *pBlue;

            // using ITU-R BT.601 conversion matrix
            unsigned luma =
                CLIP3(0, (((red * 66 + green * 129 + blue * 25) >> 8) + 16), 255);

            dstY[x] = luma;

            if ((x & 1) == 0 && (y & 1) == 0) {
                unsigned U =
                    CLIP3(0, (((-red * 38 - green * 74 + blue * 112) >> 8) + 128), 255);

                unsigned V =
                    CLIP3(0, (((red * 112 - green * 94 - blue * 18) >> 8) + 128), 255);

                dstU[x >> 1] = U;
                dstV[x >> 1] = V;
            }
            pRed   += layout.planes[C2PlanarLayout::PLANE_R].colInc;
            pGreen += layout.planes[C2PlanarLayout::PLANE_G].colInc;
            pBlue  += layout.planes[C2PlanarLayout::PLANE_B].colInc;
        }

        if ((y & 1) == 0) {
            dstU += dstStride >> 1;
            dstV += dstStride >> 1;
        }

        pRed   -= layout.planes[C2PlanarLayout::PLANE_R].colInc * src.width();
        pGreen -= layout.planes[C2PlanarLayout::PLANE_G].colInc * src.width();
        pBlue  -= layout.planes[C2PlanarLayout::PLANE_B].colInc * src.width();
        pRed   += layout.planes[C2PlanarLayout::PLANE_R].rowInc;
        pGreen += layout.planes[C2PlanarLayout::PLANE_G].rowInc;
        pBlue  += layout.planes[C2PlanarLayout::PLANE_B].rowInc;

        dstY += dstStride;
    }
    return OK;
}

namespace {

/**
 * A block of raw allocated memory.
 */
struct MemoryBlockPoolBlock {
    MemoryBlockPoolBlock(size_t size)
        : mData(new uint8_t[size]), mSize(mData ? size : 0) { }

    ~MemoryBlockPoolBlock() {
        delete[] mData;
    }

    const uint8_t *data() const {
        return mData;
    }

    size_t size() const {
        return mSize;
    }

    C2_DO_NOT_COPY(MemoryBlockPoolBlock);

private:
    uint8_t *mData;
    size_t mSize;
};

/**
 * A simple raw memory block pool implementation.
 */
struct MemoryBlockPoolImpl {
    void release(std::list<MemoryBlockPoolBlock>::const_iterator block) {
        std::lock_guard<std::mutex> lock(mMutex);
        // return block to free blocks if it is the current size; otherwise, discard
        if (block->size() == mCurrentSize) {
            mFreeBlocks.splice(mFreeBlocks.begin(), mBlocksInUse, block);
        } else {
            mBlocksInUse.erase(block);
        }
    }

    std::list<MemoryBlockPoolBlock>::const_iterator fetch(size_t size) {
        std::lock_guard<std::mutex> lock(mMutex);
        mFreeBlocks.remove_if([size](const MemoryBlockPoolBlock &block) -> bool {
            return block.size() != size;
        });
        mCurrentSize = size;
        if (mFreeBlocks.empty()) {
            mBlocksInUse.emplace_front(size);
        } else {
            mBlocksInUse.splice(mBlocksInUse.begin(), mFreeBlocks, mFreeBlocks.begin());
        }
        return mBlocksInUse.begin();
    }

    MemoryBlockPoolImpl() = default;

    C2_DO_NOT_COPY(MemoryBlockPoolImpl);

private:
    std::mutex mMutex;
    std::list<MemoryBlockPoolBlock> mFreeBlocks;
    std::list<MemoryBlockPoolBlock> mBlocksInUse;
    size_t mCurrentSize;
};

} // namespace

struct MemoryBlockPool::Impl : MemoryBlockPoolImpl {
};

struct MemoryBlock::Impl {
    Impl(std::list<MemoryBlockPoolBlock>::const_iterator block,
         std::shared_ptr<MemoryBlockPoolImpl> pool)
        : mBlock(block), mPool(pool) {
    }

    ~Impl() {
        mPool->release(mBlock);
    }

    const uint8_t *data() const {
        return mBlock->data();
    }

    size_t size() const {
        return mBlock->size();
    }

private:
    std::list<MemoryBlockPoolBlock>::const_iterator mBlock;
    std::shared_ptr<MemoryBlockPoolImpl> mPool;
};

MemoryBlock MemoryBlockPool::fetch(size_t size) {
    std::list<MemoryBlockPoolBlock>::const_iterator poolBlock = mImpl->fetch(size);
    return MemoryBlock(std::make_shared<MemoryBlock::Impl>(
            poolBlock, std::static_pointer_cast<MemoryBlockPoolImpl>(mImpl)));
}

MemoryBlockPool::MemoryBlockPool()
    : mImpl(std::make_shared<MemoryBlockPool::Impl>()) {
}

MemoryBlock::MemoryBlock(std::shared_ptr<MemoryBlock::Impl> impl)
    : mImpl(impl) {
}

MemoryBlock::MemoryBlock() = default;

MemoryBlock::~MemoryBlock() = default;

const uint8_t* MemoryBlock::data() const {
    return mImpl ? mImpl->data() : nullptr;
}

size_t MemoryBlock::size() const {
    return mImpl ? mImpl->size() : 0;
}

MemoryBlock MemoryBlock::Allocate(size_t size) {
    return MemoryBlockPool().fetch(size);
}

}  // namespace android
