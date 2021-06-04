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

#ifndef ANDROID_HARDWARE_MEDIA_OMX_V1_0_IMPL_CONVERSION_H
#define ANDROID_HARDWARE_MEDIA_OMX_V1_0_IMPL_CONVERSION_H

#include <vector>
#include <list>

#include <cinttypes>
#include <unistd.h>

#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <hidlmemory/mapping.h>

#include <binder/Binder.h>
#include <binder/Status.h>
#include <ui/BufferQueueDefs.h>
#include <ui/FenceTime.h>
#include <cutils/native_handle.h>

#include <media/OMXFenceParcelable.h>
#include <media/OMXBuffer.h>
#include <media/omx/1.0/Conversion.h>
#include <media/hardware/VideoAPI.h>
#include <media/stagefright/MediaErrors.h>

#include <android/hidl/memory/1.0/IMemory.h>
#include <android/hardware/graphics/bufferqueue/1.0/IProducerListener.h>
#include <android/hardware/media/omx/1.0/types.h>
#include <android/hardware/media/omx/1.0/IOmx.h>
#include <android/hardware/media/omx/1.0/IOmxNode.h>
#include <android/hardware/media/omx/1.0/IOmxBufferSource.h>
#include <android/hardware/media/omx/1.0/IGraphicBufferSource.h>
#include <android/hardware/media/omx/1.0/IOmxObserver.h>

#include <android/IGraphicBufferSource.h>
#include <android/IOMXBufferSource.h>

namespace android {
namespace hardware {
namespace media {
namespace omx {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_handle;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

using ::android::String8;
using ::android::OMXFenceParcelable;

using ::android::hardware::media::omx::V1_0::Message;
using ::android::omx_message;

using ::android::hardware::media::omx::V1_0::ColorAspects;
using ::android::hardware::media::V1_0::Rect;
using ::android::hardware::media::V1_0::Region;

using ::android::hardware::graphics::common::V1_0::Dataspace;

using ::android::hardware::graphics::common::V1_0::PixelFormat;

using ::android::OMXBuffer;

using ::android::hardware::media::V1_0::AnwBuffer;
using ::android::GraphicBuffer;

using ::android::hardware::media::omx::V1_0::IOmx;
using ::android::IOMX;

using ::android::hardware::media::omx::V1_0::IOmxNode;
using ::android::IOMXNode;

using ::android::hardware::media::omx::V1_0::IOmxObserver;
using ::android::IOMXObserver;

using ::android::hardware::media::omx::V1_0::IOmxBufferSource;
using ::android::IOMXBufferSource;

typedef ::android::hardware::graphics::bufferqueue::V1_0::IGraphicBufferProducer
        HGraphicBufferProducer;

// Use the conversion functions from libmedia_omx so that we don't need libgui
using namespace ::android::hardware::media::omx::V1_0::utils;

/**
 * Conversion functions
 * ====================
 *
 * There are two main directions of conversion:
 * - `inTargetType(...)`: Create a wrapper whose lifetime depends on the
 *   input. The wrapper has type `TargetType`.
 * - `toTargetType(...)`: Create a standalone object of type `TargetType` that
 *   corresponds to the input. The lifetime of the output does not depend on the
 *   lifetime of the input.
 * - `wrapIn(TargetType*, ...)`: Same as `inTargetType()`, but for `TargetType`
 *   that cannot be copied and/or moved efficiently, or when there are multiple
 *   output arguments.
 * - `convertTo(TargetType*, ...)`: Same as `toTargetType()`, but for
 *   `TargetType` that cannot be copied and/or moved efficiently, or when there
 *   are multiple output arguments.
 *
 * `wrapIn()` and `convertTo()` functions will take output arguments before
 * input arguments. Some of these functions might return a value to indicate
 * success or error.
 *
 * In converting or wrapping something as a Treble type that contains a
 * `hidl_handle`, `native_handle_t*` will need to be created and returned as
 * an additional output argument, hence only `wrapIn()` or `convertTo()` would
 * be available. The caller must call `native_handle_delete()` to deallocate the
 * returned native handle when it is no longer needed.
 *
 * For types that contain file descriptors, `inTargetType()` and `wrapAs()` do
 * not perform duplication of file descriptors, while `toTargetType()` and
 * `convertTo()` do.
 */

/**
 * \brief Convert `status_t` to `Status`.
 *
 * \param[in] l The source `status_t`.
 * \return The corresponding `Status`.
 */
// convert: status_t -> Status
inline Status toStatus(status_t l) {
    switch (l) {
    case NO_ERROR:
    case NAME_NOT_FOUND:
    case WOULD_BLOCK:
    case NO_MEMORY:
    case ALREADY_EXISTS:
    case NO_INIT:
    case BAD_VALUE:
    case DEAD_OBJECT:
    case INVALID_OPERATION:
    case TIMED_OUT:
    case ERROR_UNSUPPORTED:
    case UNKNOWN_ERROR:
    case BufferQueueDefs::RELEASE_ALL_BUFFERS:
    case BufferQueueDefs::BUFFER_NEEDS_REALLOCATION:
        return static_cast<Status>(l);
    case NOT_ENOUGH_DATA:
        return Status::BUFFER_NEEDS_REALLOCATION;
    default:
        ALOGW("Unrecognized status value: %" PRId32, static_cast<int32_t>(l));
        return static_cast<Status>(l);
    }
}

/**
 * \brief Wrap an `omx_message` and construct the corresponding `Message`.
 *
 * \param[out] t The wrapper of type `Message`.
 * \param[out] nh The native_handle_t referred to by `t->fence`.
 * \param[in] l The source `omx_message`.
 * \return `true` if the wrapping is successful; `false` otherwise.
 *
 * Upon success, \p nh will be created to hold the file descriptor stored in
 * `l.fenceFd`, and `t->fence` will point to \p nh. \p nh will need to be
 * destroyed manually by `native_handle_delete()` when \p t is no longer needed.
 *
 * Upon failure, \p nh will not be created and will not need to be deleted. \p t
 * will be invalid.
 */
// wrap, omx_message -> Message, native_handle_t*
inline bool wrapAs(Message* t, native_handle_t** nh, omx_message const& l) {
    *nh = native_handle_create_from_fd(l.fenceFd);
    if (!*nh) {
        return false;
    }
    t->fence = *nh;
    switch (l.type) {
        case omx_message::EVENT:
            t->type = Message::Type::EVENT;
            t->data.eventData.event = uint32_t(l.u.event_data.event);
            t->data.eventData.data1 = l.u.event_data.data1;
            t->data.eventData.data2 = l.u.event_data.data2;
            t->data.eventData.data3 = l.u.event_data.data3;
            t->data.eventData.data4 = l.u.event_data.data4;
            break;
        case omx_message::EMPTY_BUFFER_DONE:
            t->type = Message::Type::EMPTY_BUFFER_DONE;
            t->data.bufferData.buffer = l.u.buffer_data.buffer;
            break;
        case omx_message::FILL_BUFFER_DONE:
            t->type = Message::Type::FILL_BUFFER_DONE;
            t->data.extendedBufferData.buffer = l.u.extended_buffer_data.buffer;
            t->data.extendedBufferData.rangeOffset =
                    l.u.extended_buffer_data.range_offset;
            t->data.extendedBufferData.rangeLength =
                    l.u.extended_buffer_data.range_length;
            t->data.extendedBufferData.flags = l.u.extended_buffer_data.flags;
            t->data.extendedBufferData.timestampUs =
                    l.u.extended_buffer_data.timestamp;
            break;
        case omx_message::FRAME_RENDERED:
            t->type = Message::Type::FRAME_RENDERED;
            t->data.renderData.timestampUs = l.u.render_data.timestamp;
            t->data.renderData.systemTimeNs = l.u.render_data.nanoTime;
            break;
        default:
            native_handle_delete(*nh);
            return false;
    }
    return true;
}

/**
 * \brief Wrap a `Message` inside an `omx_message`.
 *
 * \param[out] l The wrapper of type `omx_message`.
 * \param[in] t The source `Message`.
 * \return `true` if the wrapping is successful; `false` otherwise.
 */
// wrap: Message -> omx_message
inline bool wrapAs(omx_message* l, Message const& t) {
    l->fenceFd = native_handle_read_fd(t.fence);
    switch (t.type) {
        case Message::Type::EVENT:
            l->type = omx_message::EVENT;
            l->u.event_data.event = OMX_EVENTTYPE(t.data.eventData.event);
            l->u.event_data.data1 = t.data.eventData.data1;
            l->u.event_data.data2 = t.data.eventData.data2;
            l->u.event_data.data3 = t.data.eventData.data3;
            l->u.event_data.data4 = t.data.eventData.data4;
            break;
        case Message::Type::EMPTY_BUFFER_DONE:
            l->type = omx_message::EMPTY_BUFFER_DONE;
            l->u.buffer_data.buffer = t.data.bufferData.buffer;
            break;
        case Message::Type::FILL_BUFFER_DONE:
            l->type = omx_message::FILL_BUFFER_DONE;
            l->u.extended_buffer_data.buffer = t.data.extendedBufferData.buffer;
            l->u.extended_buffer_data.range_offset =
                    t.data.extendedBufferData.rangeOffset;
            l->u.extended_buffer_data.range_length =
                    t.data.extendedBufferData.rangeLength;
            l->u.extended_buffer_data.flags = t.data.extendedBufferData.flags;
            l->u.extended_buffer_data.timestamp =
                    t.data.extendedBufferData.timestampUs;
            break;
        case Message::Type::FRAME_RENDERED:
            l->type = omx_message::FRAME_RENDERED;
            l->u.render_data.timestamp = t.data.renderData.timestampUs;
            l->u.render_data.nanoTime = t.data.renderData.systemTimeNs;
            break;
        default:
            return false;
    }
    return true;
}

/**
 * \brief Similar to `wrapTo(omx_message*, Message const&)`, but the output will
 * have an extended lifetime.
 *
 * \param[out] l The output `omx_message`.
 * \param[in] t The source `Message`.
 * \return `true` if the conversion is successful; `false` otherwise.
 *
 * This function calls `wrapto()`, then attempts to duplicate the file
 * descriptor for the fence if it is not `-1`. If duplication fails, `false`
 * will be returned.
 */
// convert: Message -> omx_message
inline bool convertTo(omx_message* l, Message const& t) {
    if (!wrapAs(l, t)) {
        return false;
    }
    if (l->fenceFd == -1) {
        return true;
    }
    l->fenceFd = dup(l->fenceFd);
    return l->fenceFd != -1;
}

/**
 * \brief Wrap an `OMXFenceParcelable` inside a `hidl_handle`.
 *
 * \param[out] t The wrapper of type `hidl_handle`.
 * \param[out] nh The native handle created to hold the file descriptor inside
 * \p l.
 * \param[in] l The source `OMXFenceParcelable`, which essentially contains one
 * file descriptor.
 * \return `true` if \p t and \p nh are successfully created to wrap around \p
 * l; `false` otherwise.
 *
 * On success, \p nh needs to be deleted by the caller with
 * `native_handle_delete()` after \p t and \p nh are no longer needed.
 *
 * On failure, \p nh will not need to be deleted, and \p t will hold an invalid
 * value.
 */
// wrap: OMXFenceParcelable -> hidl_handle, native_handle_t*
inline bool wrapAs(hidl_handle* t, native_handle_t** nh,
        OMXFenceParcelable const& l) {
    *nh = native_handle_create_from_fd(l.get());
    if (!*nh) {
        return false;
    }
    *t = *nh;
    return true;
}

/**
 * \brief Wrap a `hidl_handle` inside an `OMXFenceParcelable`.
 *
 * \param[out] l The wrapper of type `OMXFenceParcelable`.
 * \param[in] t The source `hidl_handle`.
 */
// wrap: hidl_handle -> OMXFenceParcelable
inline void wrapAs(OMXFenceParcelable* l, hidl_handle const& t) {
    l->mFenceFd = native_handle_read_fd(t);
}

/**
 * \brief Convert a `hidl_handle` to `OMXFenceParcelable`. If `hidl_handle`
 * contains file descriptors, the first file descriptor will be duplicated and
 * stored in the output `OMXFenceParcelable`.
 *
 * \param[out] l The output `OMXFenceParcelable`.
 * \param[in] t The input `hidl_handle`.
 * \return `false` if \p t contains a valid file descriptor but duplication
 * fails; `true` otherwise.
 */
// convert: hidl_handle -> OMXFenceParcelable
inline bool convertTo(OMXFenceParcelable* l, hidl_handle const& t) {
    int fd = native_handle_read_fd(t);
    if (fd != -1) {
        fd = dup(fd);
        if (fd == -1) {
            return false;
        }
    }
    l->mFenceFd = fd;
    return true;
}

/**
 * \brief Convert `::android::ColorAspects` to `ColorAspects`.
 *
 * \param[in] l The source `::android::ColorAspects`.
 * \return The corresponding `ColorAspects`.
 */
// convert: ::android::ColorAspects -> ColorAspects
inline ColorAspects toHardwareColorAspects(::android::ColorAspects const& l) {
    return ColorAspects{
            static_cast<ColorAspects::Range>(l.mRange),
            static_cast<ColorAspects::Primaries>(l.mPrimaries),
            static_cast<ColorAspects::Transfer>(l.mTransfer),
            static_cast<ColorAspects::MatrixCoeffs>(l.mMatrixCoeffs)};
}

/**
 * \brief Convert `int32_t` to `ColorAspects`.
 *
 * \param[in] l The source `int32_t`.
 * \return The corresponding `ColorAspects`.
 */
// convert: int32_t -> ColorAspects
inline ColorAspects toHardwareColorAspects(int32_t l) {
    return ColorAspects{
            static_cast<ColorAspects::Range>((l >> 24) & 0xFF),
            static_cast<ColorAspects::Primaries>((l >> 16) & 0xFF),
            static_cast<ColorAspects::Transfer>(l & 0xFF),
            static_cast<ColorAspects::MatrixCoeffs>((l >> 8) & 0xFF)};
}

/**
 * \brief Convert `ColorAspects` to `::android::ColorAspects`.
 *
 * \param[in] t The source `ColorAspects`.
 * \return The corresponding `::android::ColorAspects`.
 */
// convert: ColorAspects -> ::android::ColorAspects
inline int32_t toCompactColorAspects(ColorAspects const& t) {
    return static_cast<int32_t>(
            (static_cast<uint32_t>(t.range) << 24) |
            (static_cast<uint32_t>(t.primaries) << 16) |
            (static_cast<uint32_t>(t.transfer)) |
            (static_cast<uint32_t>(t.matrixCoeffs) << 8));
}

/**
 * \brief Wrap `GraphicBuffer` in `CodecBuffer`.
 *
 * \param[out] t The wrapper of type `CodecBuffer`.
 * \param[in] l The source `GraphicBuffer`.
 */
// wrap: OMXBuffer -> CodecBuffer
inline CodecBuffer *wrapAs(CodecBuffer *t, sp<GraphicBuffer> const& graphicBuffer) {
    t->sharedMemory = hidl_memory();
    t->nativeHandle = hidl_handle();
    t->type = CodecBuffer::Type::ANW_BUFFER;
    if (graphicBuffer == nullptr) {
        t->attr.anwBuffer.width = 0;
        t->attr.anwBuffer.height = 0;
        t->attr.anwBuffer.stride = 0;
        t->attr.anwBuffer.format = static_cast<PixelFormat>(1);
        t->attr.anwBuffer.layerCount = 0;
        t->attr.anwBuffer.usage = 0;
        return t;
    }
    t->attr.anwBuffer.width = graphicBuffer->getWidth();
    t->attr.anwBuffer.height = graphicBuffer->getHeight();
    t->attr.anwBuffer.stride = graphicBuffer->getStride();
    t->attr.anwBuffer.format = static_cast<PixelFormat>(
            graphicBuffer->getPixelFormat());
    t->attr.anwBuffer.layerCount = graphicBuffer->getLayerCount();
    t->attr.anwBuffer.usage = graphicBuffer->getUsage();
    t->nativeHandle = graphicBuffer->handle;
    return t;
}

/**
 * \brief Wrap `OMXBuffer` in `CodecBuffer`.
 *
 * \param[out] t The wrapper of type `CodecBuffer`.
 * \param[in] l The source `OMXBuffer`.
 * \return `true` if the wrapping is successful; `false` otherwise.
 */
// wrap: OMXBuffer -> CodecBuffer
inline bool wrapAs(CodecBuffer* t, OMXBuffer const& l) {
    t->sharedMemory = hidl_memory();
    t->nativeHandle = hidl_handle();
    switch (l.mBufferType) {
        case OMXBuffer::kBufferTypeInvalid: {
            t->type = CodecBuffer::Type::INVALID;
            return true;
        }
        case OMXBuffer::kBufferTypePreset: {
            t->type = CodecBuffer::Type::PRESET;
            t->attr.preset.rangeLength = static_cast<uint32_t>(l.mRangeLength);
            t->attr.preset.rangeOffset = static_cast<uint32_t>(l.mRangeOffset);
            return true;
        }
        case OMXBuffer::kBufferTypeHidlMemory: {
            t->type = CodecBuffer::Type::SHARED_MEM;
            t->sharedMemory = l.mHidlMemory;
            return true;
        }
        case OMXBuffer::kBufferTypeSharedMem: {
            // This is not supported.
            return false;
        }
        case OMXBuffer::kBufferTypeANWBuffer: {
            wrapAs(t, l.mGraphicBuffer);
            return true;
        }
        case OMXBuffer::kBufferTypeNativeHandle: {
            t->type = CodecBuffer::Type::NATIVE_HANDLE;
            t->nativeHandle = l.mNativeHandle->handle();
            return true;
        }
    }
    return false;
}

/**
 * \brief Convert `CodecBuffer` to `OMXBuffer`.
 *
 * \param[out] l The destination `OMXBuffer`.
 * \param[in] t The source `CodecBuffer`.
 * \return `true` if successful; `false` otherwise.
 */
// convert: CodecBuffer -> OMXBuffer
inline bool convertTo(OMXBuffer* l, CodecBuffer const& t) {
    switch (t.type) {
        case CodecBuffer::Type::INVALID: {
            *l = OMXBuffer();
            return true;
        }
        case CodecBuffer::Type::PRESET: {
            *l = OMXBuffer(
                    t.attr.preset.rangeOffset,
                    t.attr.preset.rangeLength);
            return true;
        }
        case CodecBuffer::Type::SHARED_MEM: {
            *l = OMXBuffer(t.sharedMemory);
            return true;
        }
        case CodecBuffer::Type::ANW_BUFFER: {
            if (t.nativeHandle.getNativeHandle() == nullptr) {
                *l = OMXBuffer(sp<GraphicBuffer>(nullptr));
                return true;
            }
            AnwBuffer anwBuffer;
            anwBuffer.nativeHandle = t.nativeHandle;
            anwBuffer.attr = t.attr.anwBuffer;
            sp<GraphicBuffer> graphicBuffer = new GraphicBuffer();
            if (!::android::hardware::media::omx::V1_0::utils::convertTo(
                    graphicBuffer.get(), anwBuffer)) {
                return false;
            }
            *l = OMXBuffer(graphicBuffer);
            return true;
        }
        case CodecBuffer::Type::NATIVE_HANDLE: {
            *l = OMXBuffer(NativeHandle::create(
                    native_handle_clone(t.nativeHandle), true));
            return true;
        }
    }
    return false;
}

/**
 * \brief Convert `IOMX::ComponentInfo` to `IOmx::ComponentInfo`.
 *
 * \param[out] t The destination `IOmx::ComponentInfo`.
 * \param[in] l The source `IOMX::ComponentInfo`.
 */
// convert: IOMX::ComponentInfo -> IOmx::ComponentInfo
inline bool convertTo(IOmx::ComponentInfo* t, IOMX::ComponentInfo const& l) {
    t->mName = l.mName.string();
    t->mRoles.resize(l.mRoles.size());
    size_t i = 0;
    for (auto& role : l.mRoles) {
        t->mRoles[i++] = role.string();
    }
    return true;
}

/**
 * \brief Convert `IOmx::ComponentInfo` to `IOMX::ComponentInfo`.
 *
 * \param[out] l The destination `IOMX::ComponentInfo`.
 * \param[in] t The source `IOmx::ComponentInfo`.
 */
// convert: IOmx::ComponentInfo -> IOMX::ComponentInfo
inline bool convertTo(IOMX::ComponentInfo* l, IOmx::ComponentInfo const& t) {
    l->mName = t.mName.c_str();
    l->mRoles.clear();
    for (size_t i = 0; i < t.mRoles.size(); ++i) {
        l->mRoles.push_back(String8(t.mRoles[i].c_str()));
    }
    return true;
}

/**
 * \brief Convert `OMX_BOOL` to `bool`.
 *
 * \param[in] l The source `OMX_BOOL`.
 * \return The destination `bool`.
 */
// convert: OMX_BOOL -> bool
inline bool toRawBool(OMX_BOOL l) {
    return l == OMX_FALSE ? false : true;
}

/**
 * \brief Convert `bool` to `OMX_BOOL`.
 *
 * \param[in] t The source `bool`.
 * \return The destination `OMX_BOOL`.
 */
// convert: bool -> OMX_BOOL
inline OMX_BOOL toEnumBool(bool t) {
    return t ? OMX_TRUE : OMX_FALSE;
}

/**
 * \brief Convert `OMX_COMMANDTYPE` to `uint32_t`.
 *
 * \param[in] l The source `OMX_COMMANDTYPE`.
 * \return The underlying value of type `uint32_t`.
 *
 * `OMX_COMMANDTYPE` is an enum type whose underlying type is `uint32_t`.
 */
// convert: OMX_COMMANDTYPE -> uint32_t
inline uint32_t toRawCommandType(OMX_COMMANDTYPE l) {
    return static_cast<uint32_t>(l);
}

/**
 * \brief Convert `uint32_t` to `OMX_COMMANDTYPE`.
 *
 * \param[in] t The source `uint32_t`.
 * \return The corresponding enum value of type `OMX_COMMANDTYPE`.
 *
 * `OMX_COMMANDTYPE` is an enum type whose underlying type is `uint32_t`.
 */
// convert: uint32_t -> OMX_COMMANDTYPE
inline OMX_COMMANDTYPE toEnumCommandType(uint32_t t) {
    return static_cast<OMX_COMMANDTYPE>(t);
}

/**
 * \brief Convert `OMX_INDEXTYPE` to `uint32_t`.
 *
 * \param[in] l The source `OMX_INDEXTYPE`.
 * \return The underlying value of type `uint32_t`.
 *
 * `OMX_INDEXTYPE` is an enum type whose underlying type is `uint32_t`.
 */
// convert: OMX_INDEXTYPE -> uint32_t
inline uint32_t toRawIndexType(OMX_INDEXTYPE l) {
    return static_cast<uint32_t>(l);
}

/**
 * \brief Convert `uint32_t` to `OMX_INDEXTYPE`.
 *
 * \param[in] t The source `uint32_t`.
 * \return The corresponding enum value of type `OMX_INDEXTYPE`.
 *
 * `OMX_INDEXTYPE` is an enum type whose underlying type is `uint32_t`.
 */
// convert: uint32_t -> OMX_INDEXTYPE
inline OMX_INDEXTYPE toEnumIndexType(uint32_t t) {
    return static_cast<OMX_INDEXTYPE>(t);
}

/**
 * \brief Convert `IOMX::PortMode` to `PortMode`.
 *
 * \param[in] l The source `IOMX::PortMode`.
 * \return The destination `PortMode`.
 */
// convert: IOMX::PortMode -> PortMode
inline PortMode toHardwarePortMode(IOMX::PortMode l) {
    return static_cast<PortMode>(l);
}

/**
 * \brief Convert `PortMode` to `IOMX::PortMode`.
 *
 * \param[in] t The source `PortMode`.
 * \return The destination `IOMX::PortMode`.
 */
// convert: PortMode -> IOMX::PortMode
inline IOMX::PortMode toIOMXPortMode(PortMode t) {
    return static_cast<IOMX::PortMode>(t);
}

/**
 * \brief Convert `OMX_TICKS` to `uint64_t`.
 *
 * \param[in] l The source `OMX_TICKS`.
 * \return The destination `uint64_t`.
 */
// convert: OMX_TICKS -> uint64_t
inline uint64_t toRawTicks(OMX_TICKS l) {
#ifndef OMX_SKIP64BIT
    return static_cast<uint64_t>(l);
#else
    return static_cast<uint64_t>(l.nLowPart) |
            static_cast<uint64_t>(l.nHighPart << 32);
#endif
}

/**
 * \brief Convert `uint64_t` to `OMX_TICKS`.
 *
 * \param[in] l The source `uint64_t`.
 * \return The destination `OMX_TICKS`.
 */
// convert: uint64_t -> OMX_TICKS
inline OMX_TICKS toOMXTicks(uint64_t t) {
#ifndef OMX_SKIP64BIT
    return static_cast<OMX_TICKS>(t);
#else
    return OMX_TICKS{
            static_cast<uint32_t>(t & 0xFFFFFFFF),
            static_cast<uint32_t>(t >> 32)};
#endif
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace omx
}  // namespace media
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_MEDIA_OMX_V1_0_IMPL_CONVERSION_H
