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
#define LOG_TAG "Codec2Buffer"
#include <utils/Log.h>

#include <hidlmemory/FrameworkUtils.h>
#include <media/hardware/HardwareAPI.h>
#include <media/stagefright/MediaCodecConstants.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AUtils.h>
#include <nativebase/nativebase.h>
#include <ui/Fence.h>

#include <C2AllocatorGralloc.h>
#include <C2BlockInternal.h>
#include <C2Debug.h>

#include "Codec2Buffer.h"

namespace android {

// Codec2Buffer

bool Codec2Buffer::canCopyLinear(const std::shared_ptr<C2Buffer> &buffer) const {
    if (const_cast<Codec2Buffer *>(this)->base() == nullptr) {
        return false;
    }
    if (!buffer) {
        // Nothing to copy, so we can copy by doing nothing.
        return true;
    }
    if (buffer->data().type() != C2BufferData::LINEAR) {
        return false;
    }
    if (buffer->data().linearBlocks().size() == 0u) {
        // Nothing to copy, so we can copy by doing nothing.
        return true;
    } else if (buffer->data().linearBlocks().size() > 1u) {
        // We don't know how to copy more than one blocks.
        return false;
    }
    if (buffer->data().linearBlocks()[0].size() > capacity()) {
        // It won't fit.
        return false;
    }
    return true;
}

bool Codec2Buffer::copyLinear(const std::shared_ptr<C2Buffer> &buffer) {
    // We assume that all canCopyLinear() checks passed.
    if (!buffer || buffer->data().linearBlocks().size() == 0u
            || buffer->data().linearBlocks()[0].size() == 0u) {
        setRange(0, 0);
        return true;
    }
    C2ReadView view = buffer->data().linearBlocks()[0].map().get();
    if (view.error() != C2_OK) {
        ALOGD("Error while mapping: %d", view.error());
        return false;
    }
    if (view.capacity() > capacity()) {
        ALOGD("C2ConstLinearBlock lied --- it actually doesn't fit: view(%u) > this(%zu)",
                view.capacity(), capacity());
        return false;
    }
    memcpy(base(), view.data(), view.capacity());
    setRange(0, view.capacity());
    return true;
}

void Codec2Buffer::setImageData(const sp<ABuffer> &imageData) {
    mImageData = imageData;
}

// LocalLinearBuffer

bool LocalLinearBuffer::canCopy(const std::shared_ptr<C2Buffer> &buffer) const {
    return canCopyLinear(buffer);
}

bool LocalLinearBuffer::copy(const std::shared_ptr<C2Buffer> &buffer) {
    return copyLinear(buffer);
}

// DummyContainerBuffer

static uint8_t sDummyByte[1] = { 0 };

DummyContainerBuffer::DummyContainerBuffer(
        const sp<AMessage> &format, const std::shared_ptr<C2Buffer> &buffer)
    : Codec2Buffer(format, new ABuffer(sDummyByte, 1)),
      mBufferRef(buffer) {
    setRange(0, buffer ? 1 : 0);
}

std::shared_ptr<C2Buffer> DummyContainerBuffer::asC2Buffer() {
    return std::move(mBufferRef);
}

bool DummyContainerBuffer::canCopy(const std::shared_ptr<C2Buffer> &) const {
    return !mBufferRef;
}

bool DummyContainerBuffer::copy(const std::shared_ptr<C2Buffer> &buffer) {
    mBufferRef = buffer;
    setRange(0, mBufferRef ? 1 : 0);
    return true;
}

// LinearBlockBuffer

// static
sp<LinearBlockBuffer> LinearBlockBuffer::Allocate(
        const sp<AMessage> &format, const std::shared_ptr<C2LinearBlock> &block) {
    C2WriteView writeView(block->map().get());
    if (writeView.error() != C2_OK) {
        return nullptr;
    }
    return new LinearBlockBuffer(format, std::move(writeView), block);
}

std::shared_ptr<C2Buffer> LinearBlockBuffer::asC2Buffer() {
    return C2Buffer::CreateLinearBuffer(mBlock->share(offset(), size(), C2Fence()));
}

bool LinearBlockBuffer::canCopy(const std::shared_ptr<C2Buffer> &buffer) const {
    return canCopyLinear(buffer);
}

bool LinearBlockBuffer::copy(const std::shared_ptr<C2Buffer> &buffer) {
    return copyLinear(buffer);
}

LinearBlockBuffer::LinearBlockBuffer(
        const sp<AMessage> &format,
        C2WriteView&& writeView,
        const std::shared_ptr<C2LinearBlock> &block)
    : Codec2Buffer(format, new ABuffer(writeView.data(), writeView.size())),
      mWriteView(writeView),
      mBlock(block) {
}

// ConstLinearBlockBuffer

// static
sp<ConstLinearBlockBuffer> ConstLinearBlockBuffer::Allocate(
        const sp<AMessage> &format, const std::shared_ptr<C2Buffer> &buffer) {
    if (!buffer
            || buffer->data().type() != C2BufferData::LINEAR
            || buffer->data().linearBlocks().size() != 1u) {
        return nullptr;
    }
    C2ReadView readView(buffer->data().linearBlocks()[0].map().get());
    if (readView.error() != C2_OK) {
        return nullptr;
    }
    return new ConstLinearBlockBuffer(format, std::move(readView), buffer);
}

ConstLinearBlockBuffer::ConstLinearBlockBuffer(
        const sp<AMessage> &format,
        C2ReadView&& readView,
        const std::shared_ptr<C2Buffer> &buffer)
    : Codec2Buffer(format, new ABuffer(
            // NOTE: ABuffer only takes non-const pointer but this data is
            //       supposed to be read-only.
            const_cast<uint8_t *>(readView.data()), readView.capacity())),
      mReadView(readView),
      mBufferRef(buffer) {
}

std::shared_ptr<C2Buffer> ConstLinearBlockBuffer::asC2Buffer() {
    return std::move(mBufferRef);
}

// GraphicView2MediaImageConverter

namespace {

class GraphicView2MediaImageConverter {
public:
    /**
     * Creates a C2GraphicView <=> MediaImage converter
     *
     * \param view C2GraphicView object
     * \param colorFormat desired SDK color format for the MediaImage (if this is a flexible format,
     *        an attempt is made to simply represent the graphic view as a flexible SDK format
     *        without a memcpy)
     * \param copy whether the converter is used for copy or not
     */
    GraphicView2MediaImageConverter(
            const C2GraphicView &view, int32_t colorFormat, bool copy)
        : mInitCheck(NO_INIT),
          mView(view),
          mWidth(view.width()),
          mHeight(view.height()),
          mColorFormat(colorFormat),
          mAllocatedDepth(0),
          mBackBufferSize(0),
          mMediaImage(new ABuffer(sizeof(MediaImage2))) {
        if (view.error() != C2_OK) {
            ALOGD("Converter: view.error() = %d", view.error());
            mInitCheck = BAD_VALUE;
            return;
        }
        MediaImage2 *mediaImage = (MediaImage2 *)mMediaImage->base();
        const C2PlanarLayout &layout = view.layout();
        if (layout.numPlanes == 0) {
            ALOGD("Converter: 0 planes");
            mInitCheck = BAD_VALUE;
            return;
        }
        memset(mediaImage, 0, sizeof(*mediaImage));
        mAllocatedDepth = layout.planes[0].allocatedDepth;
        uint32_t bitDepth = layout.planes[0].bitDepth;

        // align width and height to support subsampling cleanly
        uint32_t stride = align(view.crop().width, 2) * divUp(layout.planes[0].allocatedDepth, 8u);
        uint32_t vStride = align(view.crop().height, 2);

        switch (layout.type) {
            case C2PlanarLayout::TYPE_YUV:
                mediaImage->mType = MediaImage2::MEDIA_IMAGE_TYPE_YUV;
                if (layout.numPlanes != 3) {
                    ALOGD("Converter: %d planes for YUV layout", layout.numPlanes);
                    mInitCheck = BAD_VALUE;
                    return;
                }
                if (layout.planes[0].channel != C2PlaneInfo::CHANNEL_Y
                        || layout.planes[1].channel != C2PlaneInfo::CHANNEL_CB
                        || layout.planes[2].channel != C2PlaneInfo::CHANNEL_CR
                        || layout.planes[0].colSampling != 1
                        || layout.planes[0].rowSampling != 1
                        || layout.planes[1].colSampling != 2
                        || layout.planes[1].rowSampling != 2
                        || layout.planes[2].colSampling != 2
                        || layout.planes[2].rowSampling != 2) {
                    ALOGD("Converter: not YUV420 for YUV layout");
                    mInitCheck = BAD_VALUE;
                    return;
                }
                switch (mColorFormat) {
                    case COLOR_FormatYUV420Flexible:
                        if (!copy) {
                            // try to map directly. check if the planes are near one another
                            const uint8_t *minPtr = mView.data()[0];
                            const uint8_t *maxPtr = mView.data()[0];
                            int32_t planeSize = 0;
                            for (uint32_t i = 0; i < layout.numPlanes; ++i) {
                                const C2PlaneInfo &plane = layout.planes[i];
                                ssize_t minOffset = plane.minOffset(mWidth, mHeight);
                                ssize_t maxOffset = plane.maxOffset(mWidth, mHeight);
                                if (minPtr > mView.data()[i] + minOffset) {
                                    minPtr = mView.data()[i] + minOffset;
                                }
                                if (maxPtr < mView.data()[i] + maxOffset) {
                                    maxPtr = mView.data()[i] + maxOffset;
                                }
                                planeSize += std::abs(plane.rowInc) * align(mHeight, 64)
                                        / plane.rowSampling / plane.colSampling
                                        * divUp(mAllocatedDepth, 8u);
                            }

                            if ((maxPtr - minPtr + 1) <= planeSize) {
                                // FIXME: this is risky as reading/writing data out of bound results
                                //        in an undefined behavior, but gralloc does assume a
                                //        contiguous mapping
                                for (uint32_t i = 0; i < layout.numPlanes; ++i) {
                                    const C2PlaneInfo &plane = layout.planes[i];
                                    mediaImage->mPlane[i].mOffset = mView.data()[i] - minPtr;
                                    mediaImage->mPlane[i].mColInc = plane.colInc;
                                    mediaImage->mPlane[i].mRowInc = plane.rowInc;
                                    mediaImage->mPlane[i].mHorizSubsampling = plane.colSampling;
                                    mediaImage->mPlane[i].mVertSubsampling = plane.rowSampling;
                                }
                                mWrapped = new ABuffer(const_cast<uint8_t *>(minPtr),
                                                       maxPtr - minPtr + 1);
                                break;
                            }
                        }
                        [[fallthrough]];

                    case COLOR_FormatYUV420Planar:
                    case COLOR_FormatYUV420PackedPlanar:
                        mediaImage->mPlane[mediaImage->Y].mOffset = 0;
                        mediaImage->mPlane[mediaImage->Y].mColInc = 1;
                        mediaImage->mPlane[mediaImage->Y].mRowInc = stride;
                        mediaImage->mPlane[mediaImage->Y].mHorizSubsampling = 1;
                        mediaImage->mPlane[mediaImage->Y].mVertSubsampling = 1;

                        mediaImage->mPlane[mediaImage->U].mOffset = stride * vStride;
                        mediaImage->mPlane[mediaImage->U].mColInc = 1;
                        mediaImage->mPlane[mediaImage->U].mRowInc = stride / 2;
                        mediaImage->mPlane[mediaImage->U].mHorizSubsampling = 2;
                        mediaImage->mPlane[mediaImage->U].mVertSubsampling = 2;

                        mediaImage->mPlane[mediaImage->V].mOffset = stride * vStride * 5 / 4;
                        mediaImage->mPlane[mediaImage->V].mColInc = 1;
                        mediaImage->mPlane[mediaImage->V].mRowInc = stride / 2;
                        mediaImage->mPlane[mediaImage->V].mHorizSubsampling = 2;
                        mediaImage->mPlane[mediaImage->V].mVertSubsampling = 2;
                        break;

                    case COLOR_FormatYUV420SemiPlanar:
                    case COLOR_FormatYUV420PackedSemiPlanar:
                        mediaImage->mPlane[mediaImage->Y].mOffset = 0;
                        mediaImage->mPlane[mediaImage->Y].mColInc = 1;
                        mediaImage->mPlane[mediaImage->Y].mRowInc = stride;
                        mediaImage->mPlane[mediaImage->Y].mHorizSubsampling = 1;
                        mediaImage->mPlane[mediaImage->Y].mVertSubsampling = 1;

                        mediaImage->mPlane[mediaImage->U].mOffset = stride * vStride;
                        mediaImage->mPlane[mediaImage->U].mColInc = 2;
                        mediaImage->mPlane[mediaImage->U].mRowInc = stride;
                        mediaImage->mPlane[mediaImage->U].mHorizSubsampling = 2;
                        mediaImage->mPlane[mediaImage->U].mVertSubsampling = 2;

                        mediaImage->mPlane[mediaImage->V].mOffset = stride * vStride + 1;
                        mediaImage->mPlane[mediaImage->V].mColInc = 2;
                        mediaImage->mPlane[mediaImage->V].mRowInc = stride;
                        mediaImage->mPlane[mediaImage->V].mHorizSubsampling = 2;
                        mediaImage->mPlane[mediaImage->V].mVertSubsampling = 2;
                        break;

                    default:
                        ALOGD("Converter: incompactible color format (%d) for YUV layout", mColorFormat);
                        mInitCheck = BAD_VALUE;
                        return;
                }
                break;
            case C2PlanarLayout::TYPE_YUVA:
                mediaImage->mType = MediaImage2::MEDIA_IMAGE_TYPE_YUVA;
                // We don't have an SDK YUVA format
                ALOGD("Converter: incompactible color format (%d) for YUVA layout", mColorFormat);
                mInitCheck = BAD_VALUE;
                return;
            case C2PlanarLayout::TYPE_RGB:
                mediaImage->mType = MediaImage2::MEDIA_IMAGE_TYPE_RGB;
                switch (mColorFormat) {
                    // TODO media image
                    case COLOR_FormatRGBFlexible:
                    case COLOR_Format24bitBGR888:
                    case COLOR_Format24bitRGB888:
                        break;
                    default:
                        ALOGD("Converter: incompactible color format (%d) for RGB layout", mColorFormat);
                        mInitCheck = BAD_VALUE;
                        return;
                }
                if (layout.numPlanes != 3) {
                    ALOGD("Converter: %d planes for RGB layout", layout.numPlanes);
                    mInitCheck = BAD_VALUE;
                    return;
                }
                break;
            case C2PlanarLayout::TYPE_RGBA:
                mediaImage->mType = MediaImage2::MEDIA_IMAGE_TYPE_RGBA;
                switch (mColorFormat) {
                    // TODO media image
                    case COLOR_FormatRGBAFlexible:
                    case COLOR_Format32bitABGR8888:
                    case COLOR_Format32bitARGB8888:
                    case COLOR_Format32bitBGRA8888:
                        break;
                    default:
                        ALOGD("Incompactible color format (%d) for RGBA layout", mColorFormat);
                        mInitCheck = BAD_VALUE;
                        return;
                }
                if (layout.numPlanes != 4) {
                    ALOGD("Converter: %d planes for RGBA layout", layout.numPlanes);
                    mInitCheck = BAD_VALUE;
                    return;
                }
                break;
            default:
                mediaImage->mType = MediaImage2::MEDIA_IMAGE_TYPE_UNKNOWN;
                ALOGD("Unknown layout");
                mInitCheck = BAD_VALUE;
                return;
        }
        mediaImage->mNumPlanes = layout.numPlanes;
        mediaImage->mWidth = view.crop().width;
        mediaImage->mHeight = view.crop().height;
        mediaImage->mBitDepth = bitDepth;
        mediaImage->mBitDepthAllocated = mAllocatedDepth;

        uint32_t bufferSize = 0;
        for (uint32_t i = 0; i < layout.numPlanes; ++i) {
            const C2PlaneInfo &plane = layout.planes[i];
            if (plane.allocatedDepth < plane.bitDepth
                    || plane.rightShift != plane.allocatedDepth - plane.bitDepth) {
                ALOGD("rightShift value of %u unsupported", plane.rightShift);
                mInitCheck = BAD_VALUE;
                return;
            }
            if (plane.allocatedDepth > 8 && plane.endianness != C2PlaneInfo::NATIVE) {
                ALOGD("endianness value of %u unsupported", plane.endianness);
                mInitCheck = BAD_VALUE;
                return;
            }
            if (plane.allocatedDepth != mAllocatedDepth || plane.bitDepth != bitDepth) {
                ALOGV("different allocatedDepth/bitDepth per plane unsupported");
                mInitCheck = BAD_VALUE;
                return;
            }
            bufferSize += stride * vStride
                    / plane.rowSampling / plane.colSampling;
        }

        mBackBufferSize = bufferSize;
        mInitCheck = OK;
    }

    status_t initCheck() const { return mInitCheck; }

    uint32_t backBufferSize() const { return mBackBufferSize; }

    /**
     * Wrap C2GraphicView using a MediaImage2. Note that if not wrapped, the content is not mapped
     * in this function --- the caller should use CopyGraphicView2MediaImage() function to copy the
     * data into a backing buffer explicitly.
     *
     * \return media buffer. This is null if wrapping failed.
     */
    sp<ABuffer> wrap() const {
        if (mBackBuffer == nullptr) {
            return mWrapped;
        }
        return nullptr;
    }

    bool setBackBuffer(const sp<ABuffer> &backBuffer) {
        if (backBuffer == nullptr) {
            return false;
        }
        if (backBuffer->capacity() < mBackBufferSize) {
            return false;
        }
        backBuffer->setRange(0, mBackBufferSize);
        mBackBuffer = backBuffer;
        return true;
    }

    /**
     * Copy C2GraphicView to MediaImage2.
     */
    status_t copyToMediaImage() {
        if (mInitCheck != OK) {
            return mInitCheck;
        }
        return ImageCopy(mBackBuffer->base(), getMediaImage(), mView);
    }

    const sp<ABuffer> &imageData() const { return mMediaImage; }

private:
    status_t mInitCheck;

    const C2GraphicView mView;
    uint32_t mWidth;
    uint32_t mHeight;
    int32_t mColorFormat;  ///< SDK color format for MediaImage
    sp<ABuffer> mWrapped;  ///< wrapped buffer (if we can map C2Buffer to an ABuffer)
    uint32_t mAllocatedDepth;
    uint32_t mBackBufferSize;
    sp<ABuffer> mMediaImage;
    std::function<sp<ABuffer>(size_t)> mAlloc;

    sp<ABuffer> mBackBuffer;    ///< backing buffer if we have to copy C2Buffer <=> ABuffer

    MediaImage2 *getMediaImage() {
        return (MediaImage2 *)mMediaImage->base();
    }
};

}  // namespace

// GraphicBlockBuffer

// static
sp<GraphicBlockBuffer> GraphicBlockBuffer::Allocate(
        const sp<AMessage> &format,
        const std::shared_ptr<C2GraphicBlock> &block,
        std::function<sp<ABuffer>(size_t)> alloc) {
    C2GraphicView view(block->map().get());
    if (view.error() != C2_OK) {
        ALOGD("C2GraphicBlock::map failed: %d", view.error());
        return nullptr;
    }

    int32_t colorFormat = COLOR_FormatYUV420Flexible;
    (void)format->findInt32("color-format", &colorFormat);

    GraphicView2MediaImageConverter converter(view, colorFormat, false /* copy */);
    if (converter.initCheck() != OK) {
        ALOGD("Converter init failed: %d", converter.initCheck());
        return nullptr;
    }
    bool wrapped = true;
    sp<ABuffer> buffer = converter.wrap();
    if (buffer == nullptr) {
        buffer = alloc(converter.backBufferSize());
        if (!converter.setBackBuffer(buffer)) {
            ALOGD("Converter failed to set back buffer");
            return nullptr;
        }
        wrapped = false;
    }
    return new GraphicBlockBuffer(
            format,
            buffer,
            std::move(view),
            block,
            converter.imageData(),
            wrapped);
}

GraphicBlockBuffer::GraphicBlockBuffer(
        const sp<AMessage> &format,
        const sp<ABuffer> &buffer,
        C2GraphicView &&view,
        const std::shared_ptr<C2GraphicBlock> &block,
        const sp<ABuffer> &imageData,
        bool wrapped)
    : Codec2Buffer(format, buffer),
      mView(view),
      mBlock(block),
      mWrapped(wrapped) {
    setImageData(imageData);
}

std::shared_ptr<C2Buffer> GraphicBlockBuffer::asC2Buffer() {
    uint32_t width = mView.width();
    uint32_t height = mView.height();
    if (!mWrapped) {
        (void)ImageCopy(mView, base(), imageData());
    }
    return C2Buffer::CreateGraphicBuffer(
            mBlock->share(C2Rect(width, height), C2Fence()));
}

// GraphicMetadataBuffer
GraphicMetadataBuffer::GraphicMetadataBuffer(
        const sp<AMessage> &format,
        const std::shared_ptr<C2Allocator> &alloc)
    : Codec2Buffer(format, new ABuffer(sizeof(VideoNativeMetadata))),
      mAlloc(alloc) {
    ((VideoNativeMetadata *)base())->pBuffer = nullptr;
}

std::shared_ptr<C2Buffer> GraphicMetadataBuffer::asC2Buffer() {
#ifndef __LP64__
    VideoNativeMetadata *meta = (VideoNativeMetadata *)base();
    ANativeWindowBuffer *buffer = (ANativeWindowBuffer *)meta->pBuffer;
    if (buffer == nullptr) {
        ALOGD("VideoNativeMetadata contains null buffer");
        return nullptr;
    }

    ALOGV("VideoNativeMetadata: %dx%d", buffer->width, buffer->height);
    C2Handle *handle = WrapNativeCodec2GrallocHandle(
            buffer->handle,
            buffer->width,
            buffer->height,
            buffer->format,
            buffer->usage,
            buffer->stride);
    std::shared_ptr<C2GraphicAllocation> alloc;
    c2_status_t err = mAlloc->priorGraphicAllocation(handle, &alloc);
    if (err != C2_OK) {
        ALOGD("Failed to wrap VideoNativeMetadata into C2GraphicAllocation");
        return nullptr;
    }
    std::shared_ptr<C2GraphicBlock> block = _C2BlockFactory::CreateGraphicBlock(alloc);

    meta->pBuffer = 0;
    // TODO: wrap this in C2Fence so that the component can wait when it
    //       actually starts processing.
    if (meta->nFenceFd >= 0) {
        sp<Fence> fence(new Fence(meta->nFenceFd));
        fence->waitForever(LOG_TAG);
    }
    return C2Buffer::CreateGraphicBuffer(
            block->share(C2Rect(buffer->width, buffer->height), C2Fence()));
#else
    ALOGE("GraphicMetadataBuffer does not work on 64-bit arch");
    return nullptr;
#endif
}

// ConstGraphicBlockBuffer

// static
sp<ConstGraphicBlockBuffer> ConstGraphicBlockBuffer::Allocate(
        const sp<AMessage> &format,
        const std::shared_ptr<C2Buffer> &buffer,
        std::function<sp<ABuffer>(size_t)> alloc) {
    if (!buffer
            || buffer->data().type() != C2BufferData::GRAPHIC
            || buffer->data().graphicBlocks().size() != 1u) {
        ALOGD("C2Buffer precond fail");
        return nullptr;
    }
    std::unique_ptr<const C2GraphicView> view(std::make_unique<const C2GraphicView>(
            buffer->data().graphicBlocks()[0].map().get()));
    std::unique_ptr<const C2GraphicView> holder;

    int32_t colorFormat = COLOR_FormatYUV420Flexible;
    (void)format->findInt32("color-format", &colorFormat);

    GraphicView2MediaImageConverter converter(*view, colorFormat, false /* copy */);
    if (converter.initCheck() != OK) {
        ALOGD("Converter init failed: %d", converter.initCheck());
        return nullptr;
    }
    bool wrapped = true;
    sp<ABuffer> aBuffer = converter.wrap();
    if (aBuffer == nullptr) {
        aBuffer = alloc(converter.backBufferSize());
        if (!converter.setBackBuffer(aBuffer)) {
            ALOGD("Converter failed to set back buffer");
            return nullptr;
        }
        wrapped = false;
        converter.copyToMediaImage();
        // We don't need the view.
        holder = std::move(view);
    }
    return new ConstGraphicBlockBuffer(
            format,
            aBuffer,
            std::move(view),
            buffer,
            converter.imageData(),
            wrapped);
}

// static
sp<ConstGraphicBlockBuffer> ConstGraphicBlockBuffer::AllocateEmpty(
        const sp<AMessage> &format,
        std::function<sp<ABuffer>(size_t)> alloc) {
    int32_t width, height;
    if (!format->findInt32("width", &width)
            || !format->findInt32("height", &height)) {
        ALOGD("format had no width / height");
        return nullptr;
    }
    // NOTE: we currently only support YUV420 formats for byte-buffer mode.
    sp<ABuffer> aBuffer(alloc(align(width, 16) * align(height, 16) * 3 / 2));
    return new ConstGraphicBlockBuffer(
            format,
            aBuffer,
            nullptr,
            nullptr,
            nullptr,
            false);
}

ConstGraphicBlockBuffer::ConstGraphicBlockBuffer(
        const sp<AMessage> &format,
        const sp<ABuffer> &aBuffer,
        std::unique_ptr<const C2GraphicView> &&view,
        const std::shared_ptr<C2Buffer> &buffer,
        const sp<ABuffer> &imageData,
        bool wrapped)
    : Codec2Buffer(format, aBuffer),
      mView(std::move(view)),
      mBufferRef(buffer),
      mWrapped(wrapped) {
    setImageData(imageData);
}

std::shared_ptr<C2Buffer> ConstGraphicBlockBuffer::asC2Buffer() {
    mView.reset();
    return std::move(mBufferRef);
}

bool ConstGraphicBlockBuffer::canCopy(const std::shared_ptr<C2Buffer> &buffer) const {
    if (mWrapped || mBufferRef) {
        ALOGD("ConstGraphicBlockBuffer::canCopy: %swrapped ; buffer ref %s",
                mWrapped ? "" : "not ", mBufferRef ? "exists" : "doesn't exist");
        return false;
    }
    if (!buffer) {
        // Nothing to copy, so we can copy by doing nothing.
        return true;
    }
    if (buffer->data().type() != C2BufferData::GRAPHIC) {
        ALOGD("ConstGraphicBlockBuffer::canCopy: buffer precondition unsatisfied");
        return false;
    }
    if (buffer->data().graphicBlocks().size() == 0) {
        return true;
    } else if (buffer->data().graphicBlocks().size() != 1u) {
        ALOGD("ConstGraphicBlockBuffer::canCopy: too many blocks");
        return false;
    }

    int32_t colorFormat = COLOR_FormatYUV420Flexible;
    // FIXME: format() is not const, but we cannot change it, so do a const cast here
    const_cast<ConstGraphicBlockBuffer *>(this)->format()->findInt32("color-format", &colorFormat);

    GraphicView2MediaImageConverter converter(
            buffer->data().graphicBlocks()[0].map().get(), colorFormat, true /* copy */);
    if (converter.initCheck() != OK) {
        ALOGD("ConstGraphicBlockBuffer::canCopy: converter init failed: %d", converter.initCheck());
        return false;
    }
    if (converter.backBufferSize() > capacity()) {
        ALOGD("ConstGraphicBlockBuffer::canCopy: insufficient capacity: req %u has %zu",
                converter.backBufferSize(), capacity());
        return false;
    }
    return true;
}

bool ConstGraphicBlockBuffer::copy(const std::shared_ptr<C2Buffer> &buffer) {
    if (!buffer || buffer->data().graphicBlocks().size() == 0) {
        setRange(0, 0);
        return true;
    }
    int32_t colorFormat = COLOR_FormatYUV420Flexible;
    format()->findInt32("color-format", &colorFormat);

    GraphicView2MediaImageConverter converter(
            buffer->data().graphicBlocks()[0].map().get(), colorFormat, true /* copy */);
    if (converter.initCheck() != OK) {
        ALOGD("ConstGraphicBlockBuffer::copy: converter init failed: %d", converter.initCheck());
        return false;
    }
    sp<ABuffer> aBuffer = new ABuffer(base(), capacity());
    if (!converter.setBackBuffer(aBuffer)) {
        ALOGD("ConstGraphicBlockBuffer::copy: set back buffer failed");
        return false;
    }
    setRange(0, aBuffer->size());  // align size info
    converter.copyToMediaImage();
    setImageData(converter.imageData());
    mBufferRef = buffer;
    return true;
}

// EncryptedLinearBlockBuffer

EncryptedLinearBlockBuffer::EncryptedLinearBlockBuffer(
        const sp<AMessage> &format,
        const std::shared_ptr<C2LinearBlock> &block,
        const sp<IMemory> &memory,
        int32_t heapSeqNum)
    : Codec2Buffer(format, new ABuffer(memory->pointer(), memory->size())),
      mBlock(block),
      mMemory(memory),
      mHeapSeqNum(heapSeqNum) {
}

std::shared_ptr<C2Buffer> EncryptedLinearBlockBuffer::asC2Buffer() {
    return C2Buffer::CreateLinearBuffer(mBlock->share(offset(), size(), C2Fence()));
}

void EncryptedLinearBlockBuffer::fillSourceBuffer(
        ICrypto::SourceBuffer *source) {
    source->mSharedMemory = mMemory;
    source->mHeapSeqNum = mHeapSeqNum;
}

void EncryptedLinearBlockBuffer::fillSourceBuffer(
        hardware::cas::native::V1_0::SharedBuffer *source) {
    ssize_t offset;
    size_t size;

    mHidlMemory = hardware::fromHeap(mMemory->getMemory(&offset, &size));
    source->heapBase = *mHidlMemory;
    source->offset = offset;
    source->size = size;
}

bool EncryptedLinearBlockBuffer::copyDecryptedContent(
        const sp<IMemory> &decrypted, size_t length) {
    C2WriteView view = mBlock->map().get();
    if (view.error() != C2_OK) {
        return false;
    }
    if (view.size() < length) {
        return false;
    }
    memcpy(view.data(), decrypted->pointer(), length);
    return true;
}

bool EncryptedLinearBlockBuffer::copyDecryptedContentFromMemory(size_t length) {
    return copyDecryptedContent(mMemory, length);
}

native_handle_t *EncryptedLinearBlockBuffer::handle() const {
    return const_cast<native_handle_t *>(mBlock->handle());
}

}  // namespace android
