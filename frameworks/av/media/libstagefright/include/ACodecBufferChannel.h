/*
 * Copyright 2016, The Android Open Source Project
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

#ifndef A_BUFFER_CHANNEL_H_

#define A_BUFFER_CHANNEL_H_

#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include <media/openmax/OMX_Types.h>
#include <media/stagefright/CodecBase.h>
#include <media/ICrypto.h>
#include <media/IOMX.h>

namespace android {
namespace hardware {
class HidlMemory;
};
using hardware::HidlMemory;

/**
 * BufferChannelBase implementation for ACodec.
 */
class ACodecBufferChannel : public BufferChannelBase {
public:
    struct BufferAndId {
        sp<MediaCodecBuffer> mBuffer;
        IOMX::buffer_id mBufferId;
    };

    struct BufferInfo {
        BufferInfo(
                const sp<MediaCodecBuffer> &buffer,
                IOMX::buffer_id bufferId,
                const sp<IMemory> &sharedEncryptedBuffer);

        BufferInfo() = delete;

        // Buffer facing MediaCodec and its clients.
        const sp<MediaCodecBuffer> mClientBuffer;
        // Buffer facing CodecBase.
        const sp<MediaCodecBuffer> mCodecBuffer;
        // OMX buffer ID.
        const IOMX::buffer_id mBufferId;
        // Encrypted buffer in case of secure input.
        const sp<IMemory> mSharedEncryptedBuffer;
    };

    ACodecBufferChannel(
            const sp<AMessage> &inputBufferFilled, const sp<AMessage> &outputBufferDrained);
    virtual ~ACodecBufferChannel();

    // BufferChannelBase interface
    virtual status_t queueInputBuffer(const sp<MediaCodecBuffer> &buffer) override;
    virtual status_t queueSecureInputBuffer(
            const sp<MediaCodecBuffer> &buffer,
            bool secure,
            const uint8_t *key,
            const uint8_t *iv,
            CryptoPlugin::Mode mode,
            CryptoPlugin::Pattern pattern,
            const CryptoPlugin::SubSample *subSamples,
            size_t numSubSamples,
            AString *errorDetailMsg) override;
    virtual status_t renderOutputBuffer(
            const sp<MediaCodecBuffer> &buffer, int64_t timestampNs) override;
    virtual status_t discardBuffer(const sp<MediaCodecBuffer> &buffer) override;
    virtual void getInputBufferArray(Vector<sp<MediaCodecBuffer>> *array) override;
    virtual void getOutputBufferArray(Vector<sp<MediaCodecBuffer>> *array) override;

    // Methods below are interface for ACodec to use.

    /**
     * Set input buffer array.
     *
     * @param array     Newly allocated buffers. Empty if buffers are
     *                  deallocated.
     */
    void setInputBufferArray(const std::vector<BufferAndId> &array);
    /**
     * Set output buffer array.
     *
     * @param array     Newly allocated buffers. Empty if buffers are
     *                  deallocated.
     */
    void setOutputBufferArray(const std::vector<BufferAndId> &array);
    /**
     * Request MediaCodec to fill the specified input buffer.
     *
     * @param bufferId  ID of the buffer, assigned by underlying component.
     */
    void fillThisBuffer(IOMX::buffer_id bufferID);
    /**
     * Request MediaCodec to drain the specified output buffer.
     *
     * @param bufferId  ID of the buffer, assigned by underlying component.
     * @param omxFlags  flags associated with this buffer (e.g. EOS).
     */
    void drainThisBuffer(IOMX::buffer_id bufferID, OMX_U32 omxFlags);

private:
    const sp<AMessage> mInputBufferFilled;
    const sp<AMessage> mOutputBufferDrained;

    sp<MemoryDealer> mDealer;
    sp<IMemory> mDecryptDestination;
    int32_t mHeapSeqNum;
    sp<HidlMemory> mHidlMemory;

    // These should only be accessed via std::atomic_* functions.
    //
    // Note on thread safety: since the vector and BufferInfo are const, it's
    // safe to read them at any thread once the shared_ptr object is atomically
    // obtained. Inside BufferInfo, mBufferId and mSharedEncryptedBuffer are
    // immutable objects. We write internal states of mClient/CodecBuffer when
    // the caller has given up the reference, so that access is also safe.
    std::shared_ptr<const std::vector<const BufferInfo>> mInputBuffers;
    std::shared_ptr<const std::vector<const BufferInfo>> mOutputBuffers;

    sp<MemoryDealer> makeMemoryDealer(size_t heapSize);

    bool hasCryptoOrDescrambler() {
        return mCrypto != NULL || mDescrambler != NULL;
    }
};

}  // namespace android

#endif  // A_BUFFER_CHANNEL_H_
