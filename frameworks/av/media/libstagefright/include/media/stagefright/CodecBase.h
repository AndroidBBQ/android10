/*
 * Copyright (C) 2014 The Android Open Source Project
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

#ifndef CODEC_BASE_H_

#define CODEC_BASE_H_

#include <list>
#include <memory>

#include <stdint.h>

#define STRINGIFY_ENUMS

#include <media/hardware/CryptoAPI.h>
#include <media/hardware/HardwareAPI.h>
#include <media/MediaCodecInfo.h>
#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/ColorUtils.h>
#include <media/stagefright/MediaErrors.h>
#include <system/graphics.h>
#include <utils/NativeHandle.h>

namespace android {
class BufferChannelBase;
struct BufferProducerWrapper;
class MediaCodecBuffer;
struct PersistentSurface;
struct RenderedFrameInfo;
class Surface;
struct ICrypto;
namespace hardware {
namespace cas {
namespace native {
namespace V1_0 {
struct IDescrambler;
}}}}
using hardware::cas::native::V1_0::IDescrambler;

struct CodecBase : public AHandler, /* static */ ColorUtils {
    /**
     * This interface defines events firing from CodecBase back to MediaCodec.
     * All methods must not block.
     */
    class CodecCallback {
    public:
        virtual ~CodecCallback() = default;

        /**
         * Notify MediaCodec for seeing an output EOS.
         *
         * @param err the underlying cause of the EOS. If the value is neither
         *            OK nor ERROR_END_OF_STREAM, the EOS is declared
         *            prematurely for that error.
         */
        virtual void onEos(status_t err) = 0;
        /**
         * Notify MediaCodec that start operation is complete.
         */
        virtual void onStartCompleted() = 0;
        /**
         * Notify MediaCodec that stop operation is complete.
         */
        virtual void onStopCompleted() = 0;
        /**
         * Notify MediaCodec that release operation is complete.
         */
        virtual void onReleaseCompleted() = 0;
        /**
         * Notify MediaCodec that flush operation is complete.
         */
        virtual void onFlushCompleted() = 0;
        /**
         * Notify MediaCodec that an error is occurred.
         *
         * @param err         an error code for the occurred error.
         * @param actionCode  an action code for severity of the error.
         */
        virtual void onError(status_t err, enum ActionCode actionCode) = 0;
        /**
         * Notify MediaCodec that the underlying component is allocated.
         *
         * @param componentName the unique name of the component specified in
         *                      MediaCodecList.
         */
        virtual void onComponentAllocated(const char *componentName) = 0;
        /**
         * Notify MediaCodec that the underlying component is configured.
         *
         * @param inputFormat   an input format at configure time.
         * @param outputFormat  an output format at configure time.
         */
        virtual void onComponentConfigured(
                const sp<AMessage> &inputFormat, const sp<AMessage> &outputFormat) = 0;
        /**
         * Notify MediaCodec that the input surface is created.
         *
         * @param inputFormat   an input format at surface creation. Formats
         *                      could change from the previous state as a result
         *                      of creating a surface.
         * @param outputFormat  an output format at surface creation.
         * @param inputSurface  the created surface.
         */
        virtual void onInputSurfaceCreated(
                const sp<AMessage> &inputFormat,
                const sp<AMessage> &outputFormat,
                const sp<BufferProducerWrapper> &inputSurface) = 0;
        /**
         * Notify MediaCodec that the input surface creation is failed.
         *
         * @param err an error code of the cause.
         */
        virtual void onInputSurfaceCreationFailed(status_t err) = 0;
        /**
         * Notify MediaCodec that the component accepted the provided input
         * surface.
         *
         * @param inputFormat   an input format at surface assignment. Formats
         *                      could change from the previous state as a result
         *                      of assigning a surface.
         * @param outputFormat  an output format at surface assignment.
         */
        virtual void onInputSurfaceAccepted(
                const sp<AMessage> &inputFormat,
                const sp<AMessage> &outputFormat) = 0;
        /**
         * Notify MediaCodec that the component declined the provided input
         * surface.
         *
         * @param err an error code of the cause.
         */
        virtual void onInputSurfaceDeclined(status_t err) = 0;
        /**
         * Noitfy MediaCodec that the requested input EOS is sent to the input
         * surface.
         *
         * @param err an error code returned from the surface. If there is no
         *            input surface, the value is INVALID_OPERATION.
         */
        virtual void onSignaledInputEOS(status_t err) = 0;
        /**
         * Notify MediaCodec that output frames are rendered with information on
         * those frames.
         *
         * @param done  a list of rendered frames.
         */
        virtual void onOutputFramesRendered(const std::list<RenderedFrameInfo> &done) = 0;
        /**
         * Notify MediaCodec that output buffers are changed.
         */
        virtual void onOutputBuffersChanged() = 0;
    };

    /**
     * This interface defines events firing from BufferChannelBase back to MediaCodec.
     * All methods must not block.
     */
    class BufferCallback {
    public:
        virtual ~BufferCallback() = default;

        /**
         * Notify MediaCodec that an input buffer is available with given index.
         * When BufferChannelBase::getInputBufferArray() is not called,
         * BufferChannelBase may report different buffers with the same index if
         * MediaCodec already queued/discarded the buffer. After calling
         * BufferChannelBase::getInputBufferArray(), the buffer and index match the
         * returned array.
         */
        virtual void onInputBufferAvailable(
                size_t index, const sp<MediaCodecBuffer> &buffer) = 0;
        /**
         * Notify MediaCodec that an output buffer is available with given index.
         * When BufferChannelBase::getOutputBufferArray() is not called,
         * BufferChannelBase may report different buffers with the same index if
         * MediaCodec already queued/discarded the buffer. After calling
         * BufferChannelBase::getOutputBufferArray(), the buffer and index match the
         * returned array.
         */
        virtual void onOutputBufferAvailable(
                size_t index, const sp<MediaCodecBuffer> &buffer) = 0;
    };
    enum {
        kMaxCodecBufferSize = 8192 * 4096 * 4, // 8K RGBA
    };

    inline void setCallback(std::unique_ptr<CodecCallback> &&callback) {
        mCallback = std::move(callback);
    }
    virtual std::shared_ptr<BufferChannelBase> getBufferChannel() = 0;

    virtual void initiateAllocateComponent(const sp<AMessage> &msg) = 0;
    virtual void initiateConfigureComponent(const sp<AMessage> &msg) = 0;
    virtual void initiateCreateInputSurface() = 0;
    virtual void initiateSetInputSurface(
            const sp<PersistentSurface> &surface) = 0;
    virtual void initiateStart() = 0;
    virtual void initiateShutdown(bool keepComponentAllocated = false) = 0;

    // require an explicit message handler
    virtual void onMessageReceived(const sp<AMessage> &msg) = 0;

    virtual status_t setSurface(const sp<Surface>& /*surface*/) { return INVALID_OPERATION; }

    virtual void signalFlush() = 0;
    virtual void signalResume() = 0;

    virtual void signalRequestIDRFrame() = 0;
    virtual void signalSetParameters(const sp<AMessage> &msg) = 0;
    virtual void signalEndOfInputStream() = 0;

    typedef CodecBase *(*CreateCodecFunc)(void);
    typedef PersistentSurface *(*CreateInputSurfaceFunc)(void);

protected:
    CodecBase() = default;
    virtual ~CodecBase() = default;

    std::unique_ptr<CodecCallback> mCallback;

private:
    DISALLOW_EVIL_CONSTRUCTORS(CodecBase);
};

/**
 * A channel between MediaCodec and CodecBase object which manages buffer
 * passing. Only MediaCodec is expected to call these methods, and
 * underlying CodecBase implementation should define its own interface
 * separately for itself.
 *
 * Concurrency assumptions:
 *
 * 1) Clients may access the object at multiple threads concurrently.
 * 2) All methods do not call underlying CodecBase object while holding a lock.
 * 3) Code inside critical section executes within 1ms.
 */
class BufferChannelBase {
public:
    virtual ~BufferChannelBase() = default;

    inline void setCallback(std::unique_ptr<CodecBase::BufferCallback> &&callback) {
        mCallback = std::move(callback);
    }

    void setCrypto(const sp<ICrypto> &crypto);

    void setDescrambler(const sp<IDescrambler> &descrambler);

    /**
     * Queue an input buffer into the buffer channel.
     *
     * @return    OK if successful;
     *            -ENOENT if the buffer is not known (TODO: this should be
     *            handled gracefully in the future, here and below).
     */
    virtual status_t queueInputBuffer(const sp<MediaCodecBuffer> &buffer) = 0;
    /**
     * Queue a secure input buffer into the buffer channel.
     *
     * @return    OK if successful;
     *            -ENOENT if the buffer is not known;
     *            -ENOSYS if mCrypto is not set so that decryption is not
     *            possible;
     *            other errors if decryption failed.
     */
    virtual status_t queueSecureInputBuffer(
            const sp<MediaCodecBuffer> &buffer,
            bool secure,
            const uint8_t *key,
            const uint8_t *iv,
            CryptoPlugin::Mode mode,
            CryptoPlugin::Pattern pattern,
            const CryptoPlugin::SubSample *subSamples,
            size_t numSubSamples,
            AString *errorDetailMsg) = 0;
    /**
     * Request buffer rendering at specified time.
     *
     * @param     timestampNs   nanosecond timestamp for rendering time.
     * @return    OK if successful;
     *            -ENOENT if the buffer is not known.
     */
    virtual status_t renderOutputBuffer(
            const sp<MediaCodecBuffer> &buffer, int64_t timestampNs) = 0;
    /**
     * Discard a buffer to the underlying CodecBase object.
     *
     * TODO: remove once this operation can be handled by just clearing the
     * reference.
     *
     * @return    OK if successful;
     *            -ENOENT if the buffer is not known.
     */
    virtual status_t discardBuffer(const sp<MediaCodecBuffer> &buffer) = 0;
    /**
     * Clear and fill array with input buffers.
     */
    virtual void getInputBufferArray(Vector<sp<MediaCodecBuffer>> *array) = 0;
    /**
     * Clear and fill array with output buffers.
     */
    virtual void getOutputBufferArray(Vector<sp<MediaCodecBuffer>> *array) = 0;

protected:
    std::unique_ptr<CodecBase::BufferCallback> mCallback;
    sp<ICrypto> mCrypto;
    sp<IDescrambler> mDescrambler;
};

}  // namespace android

#endif  // CODEC_BASE_H_
