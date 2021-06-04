/*
 * Copyright 2017, The Android Open Source Project
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

#ifndef CODEC2_BUFFER_H_

#define CODEC2_BUFFER_H_

#include <C2Buffer.h>

#include <android/hardware/cas/native/1.0/types.h>
#include <binder/IMemory.h>
#include <media/hardware/VideoAPI.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/MediaCodecBuffer.h>
#include <media/ICrypto.h>

namespace android {

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

class Codec2Buffer : public MediaCodecBuffer {
public:
    using MediaCodecBuffer::MediaCodecBuffer;
    ~Codec2Buffer() override = default;

    /**
     * \return  C2Buffer object represents this buffer.
     */
    virtual std::shared_ptr<C2Buffer> asC2Buffer() = 0;

    /**
     * Test if we can copy the content of |buffer| into this object.
     *
     * \param   buffer  C2Buffer object to copy.
     * \return  true    if the content of buffer can be copied over to this buffer
     *          false   otherwise.
     */
    virtual bool canCopy(const std::shared_ptr<C2Buffer> &buffer) const {
        (void)buffer;
        return false;
    }

    /**
     * Copy the content of |buffer| into this object. This method assumes that
     * canCopy() check already passed.
     *
     * \param   buffer  C2Buffer object to copy.
     * \return  true    if successful
     *          false   otherwise.
     */
    virtual bool copy(const std::shared_ptr<C2Buffer> &buffer) {
        (void)buffer;
        return false;
    }

    sp<ABuffer> getImageData() const { return mImageData; }

protected:
    /**
     * canCopy() implementation for linear buffers.
     */
    bool canCopyLinear(const std::shared_ptr<C2Buffer> &buffer) const;

    /**
     * copy() implementation for linear buffers.
     */
    bool copyLinear(const std::shared_ptr<C2Buffer> &buffer);

    /**
     * sets MediaImage data for flexible graphic buffers
     */
    void setImageData(const sp<ABuffer> &imageData);

    sp<ABuffer> mImageData;
};

/**
 * MediaCodecBuffer implementation on top of local linear buffer. This cannot
 * cross process boundary so asC2Buffer() returns only nullptr.
 */
class LocalLinearBuffer : public Codec2Buffer {
public:
    using Codec2Buffer::Codec2Buffer;

    std::shared_ptr<C2Buffer> asC2Buffer() override { return nullptr; }
    bool canCopy(const std::shared_ptr<C2Buffer> &buffer) const override;
    bool copy(const std::shared_ptr<C2Buffer> &buffer) override;
};

/**
 * MediaCodecBuffer implementation to be used only as a dummy wrapper around a
 * C2Buffer object.
 */
class DummyContainerBuffer : public Codec2Buffer {
public:
    DummyContainerBuffer(
            const sp<AMessage> &format,
            const std::shared_ptr<C2Buffer> &buffer = nullptr);

    std::shared_ptr<C2Buffer> asC2Buffer() override;
    bool canCopy(const std::shared_ptr<C2Buffer> &buffer) const override;
    bool copy(const std::shared_ptr<C2Buffer> &buffer) override;

private:
    std::shared_ptr<C2Buffer> mBufferRef;
};

/**
 * MediaCodecBuffer implementation wraps around C2LinearBlock.
 */
class LinearBlockBuffer : public Codec2Buffer {
public:
    /**
     * Allocate a new LinearBufferBlock wrapping around C2LinearBlock object.
     *
     * \param   format  mandatory buffer format for MediaCodecBuffer
     * \param   block   C2LinearBlock object to wrap around.
     * \return          LinearBlockBuffer object with writable mapping.
     *                  nullptr if unsuccessful.
     */
    static sp<LinearBlockBuffer> Allocate(
            const sp<AMessage> &format, const std::shared_ptr<C2LinearBlock> &block);

    virtual ~LinearBlockBuffer() = default;

    std::shared_ptr<C2Buffer> asC2Buffer() override;
    bool canCopy(const std::shared_ptr<C2Buffer> &buffer) const override;
    bool copy(const std::shared_ptr<C2Buffer> &buffer) override;

private:
    LinearBlockBuffer(
            const sp<AMessage> &format,
            C2WriteView &&writeView,
            const std::shared_ptr<C2LinearBlock> &block);
    LinearBlockBuffer() = delete;

    C2WriteView mWriteView;
    std::shared_ptr<C2LinearBlock> mBlock;
};

/**
 * MediaCodecBuffer implementation wraps around C2ConstLinearBlock.
 */
class ConstLinearBlockBuffer : public Codec2Buffer {
public:
    /**
     * Allocate a new ConstLinearBlockBuffer wrapping around C2Buffer object.
     *
     * \param   format  mandatory buffer format for MediaCodecBuffer
     * \param   buffer  linear C2Buffer object to wrap around.
     * \return          ConstLinearBlockBuffer object with readable mapping.
     *                  nullptr if unsuccessful.
     */
    static sp<ConstLinearBlockBuffer> Allocate(
            const sp<AMessage> &format, const std::shared_ptr<C2Buffer> &buffer);

    virtual ~ConstLinearBlockBuffer() = default;

    std::shared_ptr<C2Buffer> asC2Buffer() override;

private:
    ConstLinearBlockBuffer(
            const sp<AMessage> &format,
            C2ReadView &&readView,
            const std::shared_ptr<C2Buffer> &buffer);
    ConstLinearBlockBuffer() = delete;

    C2ReadView mReadView;
    std::shared_ptr<C2Buffer> mBufferRef;
};

/**
 * MediaCodecBuffer implementation wraps around C2GraphicBlock.
 *
 * This object exposes the underlying bits via accessor APIs and "image-data"
 * metadata, created automatically at allocation time.
 */
class GraphicBlockBuffer : public Codec2Buffer {
public:
    /**
     * Allocate a new GraphicBlockBuffer wrapping around C2GraphicBlock object.
     * If |block| is not in good color formats, it allocates YV12 local buffer
     * and copies the content over at asC2Buffer().
     *
     * \param   format  mandatory buffer format for MediaCodecBuffer
     * \param   block   C2GraphicBlock object to wrap around.
     * \param   alloc   a function to allocate backing ABuffer if needed.
     * \return          GraphicBlockBuffer object with writable mapping.
     *                  nullptr if unsuccessful.
     */
    static sp<GraphicBlockBuffer> Allocate(
            const sp<AMessage> &format,
            const std::shared_ptr<C2GraphicBlock> &block,
            std::function<sp<ABuffer>(size_t)> alloc);

    virtual ~GraphicBlockBuffer() = default;

    std::shared_ptr<C2Buffer> asC2Buffer() override;

private:
    GraphicBlockBuffer(
            const sp<AMessage> &format,
            const sp<ABuffer> &buffer,
            C2GraphicView &&view,
            const std::shared_ptr<C2GraphicBlock> &block,
            const sp<ABuffer> &imageData,
            bool wrapped);
    GraphicBlockBuffer() = delete;

    inline MediaImage2 *imageData() { return (MediaImage2 *)mImageData->data(); }

    C2GraphicView mView;
    std::shared_ptr<C2GraphicBlock> mBlock;
    const bool mWrapped;
};

/**
 * MediaCodecBuffer implementation wraps around VideoNativeMetadata.
 */
class GraphicMetadataBuffer : public Codec2Buffer {
public:
    /**
     * Construct a new GraphicMetadataBuffer with local linear buffer for
     * VideoNativeMetadata.
     *
     * \param   format      mandatory buffer format for MediaCodecBuffer
     */
    GraphicMetadataBuffer(
            const sp<AMessage> &format, const std::shared_ptr<C2Allocator> &alloc);
    virtual ~GraphicMetadataBuffer() = default;

    std::shared_ptr<C2Buffer> asC2Buffer() override;

private:
    GraphicMetadataBuffer() = delete;

    std::shared_ptr<C2Allocator> mAlloc;
};

/**
 * MediaCodecBuffer implementation wraps around graphic C2Buffer object.
 *
 * This object exposes the underlying bits via accessor APIs and "image-data"
 * metadata, created automatically at allocation time.
 */
class ConstGraphicBlockBuffer : public Codec2Buffer {
public:
    /**
     * Allocate a new ConstGraphicBlockBuffer wrapping around C2Buffer object.
     * If |buffer| is not in good color formats, it allocates YV12 local buffer
     * and copies the content of |buffer| over to expose.
     *
     * \param   format  mandatory buffer format for MediaCodecBuffer
     * \param   buffer  graphic C2Buffer object to wrap around.
     * \param   alloc   a function to allocate backing ABuffer if needed.
     * \return          ConstGraphicBlockBuffer object with readable mapping.
     *                  nullptr if unsuccessful.
     */
    static sp<ConstGraphicBlockBuffer> Allocate(
            const sp<AMessage> &format,
            const std::shared_ptr<C2Buffer> &buffer,
            std::function<sp<ABuffer>(size_t)> alloc);

    /**
     * Allocate a new ConstGraphicBlockBuffer which allocates YV12 local buffer
     * and copies the content of |buffer| over to expose.
     *
     * \param   format  mandatory buffer format for MediaCodecBuffer
     * \param   alloc   a function to allocate backing ABuffer if needed.
     * \return          ConstGraphicBlockBuffer object with no wrapping buffer.
     */
    static sp<ConstGraphicBlockBuffer> AllocateEmpty(
            const sp<AMessage> &format,
            std::function<sp<ABuffer>(size_t)> alloc);

    virtual ~ConstGraphicBlockBuffer() = default;

    std::shared_ptr<C2Buffer> asC2Buffer() override;
    bool canCopy(const std::shared_ptr<C2Buffer> &buffer) const override;
    bool copy(const std::shared_ptr<C2Buffer> &buffer) override;

private:
    ConstGraphicBlockBuffer(
            const sp<AMessage> &format,
            const sp<ABuffer> &aBuffer,
            std::unique_ptr<const C2GraphicView> &&view,
            const std::shared_ptr<C2Buffer> &buffer,
            const sp<ABuffer> &imageData,
            bool wrapped);
    ConstGraphicBlockBuffer() = delete;

    sp<ABuffer> mImageData;
    std::unique_ptr<const C2GraphicView> mView;
    std::shared_ptr<C2Buffer> mBufferRef;
    const bool mWrapped;
};

/**
 * MediaCodecBuffer implementation wraps around C2LinearBlock for component
 * and IMemory for client. Underlying C2LinearBlock won't be mapped for secure
 * usecases..
 */
class EncryptedLinearBlockBuffer : public Codec2Buffer {
public:
    /**
     * Construct a new EncryptedLinearBufferBlock wrapping around C2LinearBlock
     * object and writable IMemory region.
     *
     * \param   format      mandatory buffer format for MediaCodecBuffer
     * \param   block       C2LinearBlock object to wrap around.
     * \param   memory      IMemory object to store encrypted content.
     * \param   heapSeqNum  Heap sequence number from ICrypto; -1 if N/A
     */
    EncryptedLinearBlockBuffer(
            const sp<AMessage> &format,
            const std::shared_ptr<C2LinearBlock> &block,
            const sp<IMemory> &memory,
            int32_t heapSeqNum = -1);
    EncryptedLinearBlockBuffer() = delete;

    virtual ~EncryptedLinearBlockBuffer() = default;

    std::shared_ptr<C2Buffer> asC2Buffer() override;

    /**
     * Fill the source buffer structure with appropriate value based on
     * internal IMemory object.
     *
     * \param source  source buffer structure to fill.
     */
    void fillSourceBuffer(ICrypto::SourceBuffer *source);
    void fillSourceBuffer(
            hardware::cas::native::V1_0::SharedBuffer *source);

    /**
     * Copy the content of |decrypted| into C2LinearBlock inside. This shall
     * only be called in non-secure usecases.
     *
     * \param   decrypted   decrypted content to copy from.
     * \param   length      length of the content
     * \return  true        if successful
     *          false       otherwise.
     */
    bool copyDecryptedContent(const sp<IMemory> &decrypted, size_t length);

    /**
     * Copy the content of internal IMemory object into C2LinearBlock inside.
     * This shall only be called in non-secure usecases.
     *
     * \param   length      length of the content
     * \return  true        if successful
     *          false       otherwise.
     */
    bool copyDecryptedContentFromMemory(size_t length);

    /**
     * Return native handle of secure buffer understood by ICrypto.
     *
     * \return secure buffer handle
     */
    native_handle_t *handle() const;

private:

    std::shared_ptr<C2LinearBlock> mBlock;
    sp<IMemory> mMemory;
    sp<hardware::HidlMemory> mHidlMemory;
    int32_t mHeapSeqNum;
};

}  // namespace android

#endif  // CODEC2_BUFFER_H_
