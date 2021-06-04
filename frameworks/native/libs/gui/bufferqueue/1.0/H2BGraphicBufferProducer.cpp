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

#define LOG_TAG "H2BGraphicBufferProducer"

#include <android-base/logging.h>

#include <gui/bufferqueue/1.0/H2BGraphicBufferProducer.h>
#include <gui/bufferqueue/1.0/B2HProducerListener.h>

#include <system/window.h>

namespace android {
namespace hardware {
namespace graphics {
namespace bufferqueue {
namespace V1_0 {
namespace utils {

using Status = HGraphicBufferProducer::Status;
using ::android::hardware::graphics::common::V1_0::Dataspace;
typedef ::android::hardware::media::V1_0::Rect HRect;
typedef ::android::hardware::media::V1_0::Region HRegion;

// Conversion functions

// native_handle_t helper functions.

/**
 * \brief Take an fd and create a native handle containing only the given fd.
 * The created handle will need to be deleted manually with
 * `native_handle_delete()`.
 *
 * \param[in] fd The source file descriptor (of type `int`).
 * \return The create `native_handle_t*` that contains the given \p fd. If the
 * supplied \p fd is negative, the created native handle will contain no file
 * descriptors.
 *
 * If the native handle cannot be created, the return value will be
 * `nullptr`.
 *
 * This function does not duplicate the file descriptor.
 */
inline native_handle_t* native_handle_create_from_fd(int fd) {
    if (fd < 0) {
        return native_handle_create(0, 0);
    }
    native_handle_t* nh = native_handle_create(1, 0);
    if (nh == nullptr) {
        return nullptr;
    }
    nh->data[0] = fd;
    return nh;
}

/**
 * \brief Extract a file descriptor from a native handle.
 *
 * \param[in] nh The source `native_handle_t*`.
 * \param[in] index The index of the file descriptor in \p nh to read from. This
 * input has the default value of `0`.
 * \return The `index`-th file descriptor in \p nh. If \p nh does not have
 * enough file descriptors, the returned value will be `-1`.
 *
 * This function does not duplicate the file descriptor.
 */
inline int native_handle_read_fd(native_handle_t const* nh, int index = 0) {
    return ((nh == nullptr) || (nh->numFds == 0) ||
            (nh->numFds <= index) || (index < 0)) ?
            -1 : nh->data[index];
}

/**
 * \brief Convert `Return<Status>` to `status_t`. This is for legacy binder
 * calls.
 *
 * \param[in] t The source `Return<Status>`.
 * \return The corresponding `status_t`.
 *
 * This function first check if \p t has a transport error. If it does, then the
 * return value is the transport error code. Otherwise, the return value is
 * converted from `Status` contained inside \p t.
 *
 * Note:
 * - This `Status` is omx-specific. It is defined in `types.hal`.
 * - The name of this function is not `convert`.
 */
// convert: Return<Status> -> status_t
inline status_t toStatusT(Return<Status> const& t) {
    if (t.isOk()) {
        return static_cast<status_t>(static_cast<Status>(t));
    } else if (t.isDeadObject()) {
        return DEAD_OBJECT;
    }
    return UNKNOWN_ERROR;
}

/**
 * \brief Convert `Return<void>` to `status_t`. This is for legacy binder calls.
 *
 * \param[in] t The source `Return<void>`.
 * \return The corresponding `status_t`.
 */
// convert: Return<void> -> status_t
inline status_t toStatusT(Return<void> const& t) {
    return t.isOk() ? OK : (t.isDeadObject() ? DEAD_OBJECT : UNKNOWN_ERROR);
}

/**
 * \brief Wrap `GraphicBuffer` in `AnwBuffer`.
 *
 * \param[out] t The wrapper of type `AnwBuffer`.
 * \param[in] l The source `GraphicBuffer`.
 */
// wrap: GraphicBuffer -> AnwBuffer
inline void wrapAs(AnwBuffer* t, GraphicBuffer const& l) {
    t->attr.width = l.getWidth();
    t->attr.height = l.getHeight();
    t->attr.stride = l.getStride();
    t->attr.format = static_cast<PixelFormat>(l.getPixelFormat());
    t->attr.layerCount = l.getLayerCount();
    t->attr.usage = uint32_t(l.getUsage());     // FIXME: need 64-bits usage version
    t->attr.id = l.getId();
    t->attr.generationNumber = l.getGenerationNumber();
    t->nativeHandle = hidl_handle(l.handle);
}

/**
 * \brief Convert `AnwBuffer` to `GraphicBuffer`.
 *
 * \param[out] l The destination `GraphicBuffer`.
 * \param[in] t The source `AnwBuffer`.
 *
 * This function will duplicate all file descriptors in \p t.
 */
// convert: AnwBuffer -> GraphicBuffer
// Ref: frameworks/native/libs/ui/GraphicBuffer.cpp: GraphicBuffer::flatten
inline bool convertTo(GraphicBuffer* l, AnwBuffer const& t) {
    native_handle_t* handle = t.nativeHandle == nullptr ?
            nullptr : native_handle_clone(t.nativeHandle);

    size_t const numInts = 12 +
            static_cast<size_t>(handle ? handle->numInts : 0);
    int32_t* ints = new int32_t[numInts];

    size_t numFds = static_cast<size_t>(handle ? handle->numFds : 0);
    int* fds = new int[numFds];

    ints[0] = 'GBFR';
    ints[1] = static_cast<int32_t>(t.attr.width);
    ints[2] = static_cast<int32_t>(t.attr.height);
    ints[3] = static_cast<int32_t>(t.attr.stride);
    ints[4] = static_cast<int32_t>(t.attr.format);
    ints[5] = static_cast<int32_t>(t.attr.layerCount);
    ints[6] = static_cast<int32_t>(t.attr.usage);
    ints[7] = static_cast<int32_t>(t.attr.id >> 32);
    ints[8] = static_cast<int32_t>(t.attr.id & 0xFFFFFFFF);
    ints[9] = static_cast<int32_t>(t.attr.generationNumber);
    ints[10] = 0;
    ints[11] = 0;
    if (handle) {
        ints[10] = static_cast<int32_t>(handle->numFds);
        ints[11] = static_cast<int32_t>(handle->numInts);
        int* intsStart = handle->data + handle->numFds;
        std::copy(handle->data, intsStart, fds);
        std::copy(intsStart, intsStart + handle->numInts, &ints[12]);
    }

    void const* constBuffer = static_cast<void const*>(ints);
    size_t size = numInts * sizeof(int32_t);
    int const* constFds = static_cast<int const*>(fds);
    status_t status = l->unflatten(constBuffer, size, constFds, numFds);

    delete [] fds;
    delete [] ints;
    native_handle_delete(handle);
    return status == NO_ERROR;
}

// Ref: frameworks/native/libs/ui/Fence.cpp

/**
 * \brief Return the size of the non-fd buffer required to flatten a fence.
 *
 * \param[in] fence The input fence of type `hidl_handle`.
 * \return The required size of the flat buffer.
 *
 * The current version of this function always returns 4, which is the number of
 * bytes required to store the number of file descriptors contained in the fd
 * part of the flat buffer.
 */
inline size_t getFenceFlattenedSize(hidl_handle const& /* fence */) {
    return 4;
};

/**
 * \brief Return the number of file descriptors contained in a fence.
 *
 * \param[in] fence The input fence of type `hidl_handle`.
 * \return `0` if \p fence does not contain a valid file descriptor, or `1`
 * otherwise.
 */
inline size_t getFenceFdCount(hidl_handle const& fence) {
    return native_handle_read_fd(fence) == -1 ? 0 : 1;
}

/**
 * \brief Unflatten `Fence` to `hidl_handle`.
 *
 * \param[out] fence The destination `hidl_handle`.
 * \param[out] nh The underlying native handle.
 * \param[in,out] buffer The pointer to the flat non-fd buffer.
 * \param[in,out] size The size of the flat non-fd buffer.
 * \param[in,out] fds The pointer to the flat fd buffer.
 * \param[in,out] numFds The size of the flat fd buffer.
 * \return `NO_ERROR` on success; other value on failure.
 *
 * If the return value is `NO_ERROR`, \p nh will point to a newly created
 * native handle, which needs to be deleted with `native_handle_delete()`
 * afterwards.
 */
inline status_t unflattenFence(hidl_handle* fence, native_handle_t** nh,
        void const*& buffer, size_t& size, int const*& fds, size_t& numFds) {
    if (size < 4) {
        return NO_MEMORY;
    }

    uint32_t numFdsInHandle;
    FlattenableUtils::read(buffer, size, numFdsInHandle);

    if (numFdsInHandle > 1) {
        return BAD_VALUE;
    }

    if (numFds < numFdsInHandle) {
        return NO_MEMORY;
    }

    if (numFdsInHandle) {
        *nh = native_handle_create_from_fd(*fds);
        if (*nh == nullptr) {
            return NO_MEMORY;
        }
        *fence = *nh;
        ++fds;
        --numFds;
    } else {
        *nh = nullptr;
        *fence = hidl_handle();
    }

    return NO_ERROR;
}

/**
 * \brief Flatten `hidl_handle` as `Fence`.
 *
 * \param[in] fence The source `hidl_handle`.
 * \param[in,out] buffer The pointer to the flat non-fd buffer.
 * \param[in,out] size The size of the flat non-fd buffer.
 * \param[in,out] fds The pointer to the flat fd buffer.
 * \param[in,out] numFds The size of the flat fd buffer.
 * \return `NO_ERROR` on success; other value on failure.
 */
inline status_t flattenFence(hidl_handle const& fence,
        void*& buffer, size_t& size, int*& fds, size_t& numFds) {
    if (size < getFenceFlattenedSize(fence) ||
            numFds < getFenceFdCount(fence)) {
        return NO_MEMORY;
    }
    // Cast to uint32_t since the size of a size_t can vary between 32- and
    // 64-bit processes
    FlattenableUtils::write(buffer, size,
            static_cast<uint32_t>(getFenceFdCount(fence)));
    int fd = native_handle_read_fd(fence);
    if (fd != -1) {
        *fds = fd;
        ++fds;
        --numFds;
    }
    return NO_ERROR;
}

/**
 * \brief Wrap `Fence` in `hidl_handle`.
 *
 * \param[out] t The wrapper of type `hidl_handle`.
 * \param[out] nh The native handle pointed to by \p t.
 * \param[in] l The source `Fence`.
 *
 * On success, \p nh will hold a newly created native handle, which must be
 * deleted manually with `native_handle_delete()` afterwards.
 */
// wrap: Fence -> hidl_handle
inline bool wrapAs(hidl_handle* t, native_handle_t** nh, Fence const& l) {
    size_t const baseSize = l.getFlattenedSize();
    std::unique_ptr<uint8_t[]> baseBuffer(
            new (std::nothrow) uint8_t[baseSize]);
    if (!baseBuffer) {
        return false;
    }

    size_t const baseNumFds = l.getFdCount();
    std::unique_ptr<int[]> baseFds(
            new (std::nothrow) int[baseNumFds]);
    if (!baseFds) {
        return false;
    }

    void* buffer = static_cast<void*>(baseBuffer.get());
    size_t size = baseSize;
    int* fds = static_cast<int*>(baseFds.get());
    size_t numFds = baseNumFds;
    if (l.flatten(buffer, size, fds, numFds) != NO_ERROR) {
        return false;
    }

    void const* constBuffer = static_cast<void const*>(baseBuffer.get());
    size = baseSize;
    int const* constFds = static_cast<int const*>(baseFds.get());
    numFds = baseNumFds;
    if (unflattenFence(t, nh, constBuffer, size, constFds, numFds)
            != NO_ERROR) {
        return false;
    }

    return true;
}

/**
 * \brief Convert `hidl_handle` to `Fence`.
 *
 * \param[out] l The destination `Fence`. `l` must not have been used
 * (`l->isValid()` must return `false`) before this function is called.
 * \param[in] t The source `hidl_handle`.
 *
 * If \p t contains a valid file descriptor, it will be duplicated.
 */
// convert: hidl_handle -> Fence
inline bool convertTo(Fence* l, hidl_handle const& t) {
    int fd = native_handle_read_fd(t);
    if (fd != -1) {
        fd = dup(fd);
        if (fd == -1) {
            return false;
        }
    }
    native_handle_t* nh = native_handle_create_from_fd(fd);
    if (nh == nullptr) {
        if (fd != -1) {
            close(fd);
        }
        return false;
    }

    size_t const baseSize = getFenceFlattenedSize(t);
    std::unique_ptr<uint8_t[]> baseBuffer(
            new (std::nothrow) uint8_t[baseSize]);
    if (!baseBuffer) {
        native_handle_delete(nh);
        return false;
    }

    size_t const baseNumFds = getFenceFdCount(t);
    std::unique_ptr<int[]> baseFds(
            new (std::nothrow) int[baseNumFds]);
    if (!baseFds) {
        native_handle_delete(nh);
        return false;
    }

    void* buffer = static_cast<void*>(baseBuffer.get());
    size_t size = baseSize;
    int* fds = static_cast<int*>(baseFds.get());
    size_t numFds = baseNumFds;
    if (flattenFence(hidl_handle(nh), buffer, size, fds, numFds) != NO_ERROR) {
        native_handle_delete(nh);
        return false;
    }
    native_handle_delete(nh);

    void const* constBuffer = static_cast<void const*>(baseBuffer.get());
    size = baseSize;
    int const* constFds = static_cast<int const*>(baseFds.get());
    numFds = baseNumFds;
    if (l->unflatten(constBuffer, size, constFds, numFds) != NO_ERROR) {
        return false;
    }

    return true;
}

// Ref: frameworks/native/libs/ui/Region.cpp

/**
 * \brief Unflatten `HRegion`.
 *
 * \param[out] t The destination `HRegion`.
 * \param[in,out] buffer The pointer to the flat buffer.
 * \param[in,out] size The size of the flat buffer.
 * \return `NO_ERROR` on success; other value on failure.
 */
inline status_t unflatten(HRegion* t, void const*& buffer, size_t& size) {
    if (size < sizeof(uint32_t)) {
        return NO_MEMORY;
    }

    uint32_t numRects = 0;
    FlattenableUtils::read(buffer, size, numRects);
    if (size < numRects * sizeof(HRect)) {
        return NO_MEMORY;
    }
    if (numRects > (UINT32_MAX / sizeof(HRect))) {
        return NO_MEMORY;
    }

    t->resize(numRects);
    for (size_t r = 0; r < numRects; ++r) {
        ::android::Rect rect(::android::Rect::EMPTY_RECT);
        status_t status = rect.unflatten(buffer, size);
        if (status != NO_ERROR) {
            return status;
        }
        FlattenableUtils::advance(buffer, size, sizeof(rect));
        (*t)[r] = HRect{
                static_cast<int32_t>(rect.left),
                static_cast<int32_t>(rect.top),
                static_cast<int32_t>(rect.right),
                static_cast<int32_t>(rect.bottom)};
    }
    return NO_ERROR;
}

// Ref: frameworks/native/libs/gui/IGraphicBufferProducer.cpp:
//      IGraphicBufferProducer::QueueBufferInput

/**
 * \brief Return a lower bound on the size of the buffer required to flatten
 * `HGraphicBufferProducer::QueueBufferInput`.
 *
 * \param[in] t The input `HGraphicBufferProducer::QueueBufferInput`.
 * \return A lower bound on the size of the flat buffer.
 */
constexpr size_t minFlattenedSize(
        HGraphicBufferProducer::QueueBufferInput const& /* t */) {
    return sizeof(int64_t) + // timestamp
            sizeof(int) + // isAutoTimestamp
            sizeof(android_dataspace) + // dataSpace
            sizeof(::android::Rect) + // crop
            sizeof(int) + // scalingMode
            sizeof(uint32_t) + // transform
            sizeof(uint32_t) + // stickyTransform
            sizeof(bool); // getFrameTimestamps
}

/**
 * \brief Unflatten `HGraphicBufferProducer::QueueBufferInput`.
 *
 * \param[out] t The destination `HGraphicBufferProducer::QueueBufferInput`.
 * \param[out] nh The underlying native handle for `t->fence`.
 * \param[in,out] buffer The pointer to the flat non-fd buffer.
 * \param[in,out] size The size of the flat non-fd buffer.
 * \param[in,out] fds The pointer to the flat fd buffer.
 * \param[in,out] numFds The size of the flat fd buffer.
 * \return `NO_ERROR` on success; other value on failure.
 *
 * If the return value is `NO_ERROR` and `t->fence` contains a valid file
 * descriptor, \p nh will be a newly created native handle holding that file
 * descriptor. \p nh needs to be deleted with `native_handle_delete()`
 * afterwards.
 */
inline status_t unflatten(
        HGraphicBufferProducer::QueueBufferInput* t, native_handle_t** nh,
        void const*& buffer, size_t& size, int const*& fds, size_t& numFds) {
    if (size < minFlattenedSize(*t)) {
        return NO_MEMORY;
    }

    FlattenableUtils::read(buffer, size, t->timestamp);
    int lIsAutoTimestamp;
    FlattenableUtils::read(buffer, size, lIsAutoTimestamp);
    t->isAutoTimestamp = static_cast<int32_t>(lIsAutoTimestamp);
    android_dataspace_t lDataSpace;
    FlattenableUtils::read(buffer, size, lDataSpace);
    t->dataSpace = static_cast<Dataspace>(lDataSpace);
    ::android::Rect lCrop;
    FlattenableUtils::read(buffer, size, lCrop);
    t->crop = HRect{
            static_cast<int32_t>(lCrop.left),
            static_cast<int32_t>(lCrop.top),
            static_cast<int32_t>(lCrop.right),
            static_cast<int32_t>(lCrop.bottom)};
    int lScalingMode;
    FlattenableUtils::read(buffer, size, lScalingMode);
    t->scalingMode = static_cast<int32_t>(lScalingMode);
    FlattenableUtils::read(buffer, size, t->transform);
    FlattenableUtils::read(buffer, size, t->stickyTransform);
    FlattenableUtils::read(buffer, size, t->getFrameTimestamps);

    status_t status = unflattenFence(&(t->fence), nh,
            buffer, size, fds, numFds);
    if (status != NO_ERROR) {
        return status;
    }
    return unflatten(&(t->surfaceDamage), buffer, size);
}

/**
 * \brief Wrap `IGraphicBufferProducer::QueueBufferInput` in
 * `HGraphicBufferProducer::QueueBufferInput`.
 *
 * \param[out] t The wrapper of type
 * `HGraphicBufferProducer::QueueBufferInput`.
 * \param[out] nh The underlying native handle for `t->fence`.
 * \param[in] l The source `IGraphicBufferProducer::QueueBufferInput`.
 *
 * If the return value is `true` and `t->fence` contains a valid file
 * descriptor, \p nh will be a newly created native handle holding that file
 * descriptor. \p nh needs to be deleted with `native_handle_delete()`
 * afterwards.
 */
inline bool wrapAs(
        HGraphicBufferProducer::QueueBufferInput* t,
        native_handle_t** nh,
        BGraphicBufferProducer::QueueBufferInput const& l) {

    size_t const baseSize = l.getFlattenedSize();
    std::unique_ptr<uint8_t[]> baseBuffer(
            new (std::nothrow) uint8_t[baseSize]);
    if (!baseBuffer) {
        return false;
    }

    size_t const baseNumFds = l.getFdCount();
    std::unique_ptr<int[]> baseFds(
            new (std::nothrow) int[baseNumFds]);
    if (!baseFds) {
        return false;
    }

    void* buffer = static_cast<void*>(baseBuffer.get());
    size_t size = baseSize;
    int* fds = baseFds.get();
    size_t numFds = baseNumFds;
    if (l.flatten(buffer, size, fds, numFds) != NO_ERROR) {
        return false;
    }

    void const* constBuffer = static_cast<void const*>(baseBuffer.get());
    size = baseSize;
    int const* constFds = static_cast<int const*>(baseFds.get());
    numFds = baseNumFds;
    if (unflatten(t, nh, constBuffer, size, constFds, numFds) != NO_ERROR) {
        return false;
    }

    return true;
}

// Ref: frameworks/native/libs/ui/FenceTime.cpp: FenceTime::Snapshot

/**
 * \brief Return the size of the non-fd buffer required to flatten
 * `FenceTimeSnapshot`.
 *
 * \param[in] t The input `FenceTimeSnapshot`.
 * \return The required size of the flat buffer.
 */
inline size_t getFlattenedSize(
        HGraphicBufferProducer::FenceTimeSnapshot const& t) {
    constexpr size_t min = sizeof(t.state);
    switch (t.state) {
        case HGraphicBufferProducer::FenceTimeSnapshot::State::EMPTY:
            return min;
        case HGraphicBufferProducer::FenceTimeSnapshot::State::FENCE:
            return min + getFenceFlattenedSize(t.fence);
        case HGraphicBufferProducer::FenceTimeSnapshot::State::SIGNAL_TIME:
            return min + sizeof(
                    ::android::FenceTime::Snapshot::signalTime);
    }
    return 0;
}

/**
 * \brief Return the number of file descriptors contained in
 * `FenceTimeSnapshot`.
 *
 * \param[in] t The input `FenceTimeSnapshot`.
 * \return The number of file descriptors contained in \p snapshot.
 */
inline size_t getFdCount(
        HGraphicBufferProducer::FenceTimeSnapshot const& t) {
    return t.state ==
            HGraphicBufferProducer::FenceTimeSnapshot::State::FENCE ?
            getFenceFdCount(t.fence) : 0;
}

/**
 * \brief Flatten `FenceTimeSnapshot`.
 *
 * \param[in] t The source `FenceTimeSnapshot`.
 * \param[out] nh The cloned native handle, if necessary.
 * \param[in,out] buffer The pointer to the flat non-fd buffer.
 * \param[in,out] size The size of the flat non-fd buffer.
 * \param[in,out] fds The pointer to the flat fd buffer.
 * \param[in,out] numFds The size of the flat fd buffer.
 * \return `NO_ERROR` on success; other value on failure.
 *
 * This function will duplicate the file descriptor in `t.fence` if `t.state ==
 * FENCE`, in which case \p nh will be returned.
 */
inline status_t flatten(HGraphicBufferProducer::FenceTimeSnapshot const& t,
        native_handle_t** nh,
        void*& buffer, size_t& size, int*& fds, size_t& numFds) {
    if (size < getFlattenedSize(t)) {
        return NO_MEMORY;
    }

    *nh = nullptr;
    switch (t.state) {
        case HGraphicBufferProducer::FenceTimeSnapshot::State::EMPTY:
            FlattenableUtils::write(buffer, size,
                    ::android::FenceTime::Snapshot::State::EMPTY);
            return NO_ERROR;
        case HGraphicBufferProducer::FenceTimeSnapshot::State::FENCE:
            FlattenableUtils::write(buffer, size,
                    ::android::FenceTime::Snapshot::State::FENCE);
            *nh = t.fence.getNativeHandle() == nullptr ?
                    nullptr : native_handle_clone(t.fence);
            return flattenFence(hidl_handle(*nh), buffer, size, fds, numFds);
        case HGraphicBufferProducer::FenceTimeSnapshot::State::SIGNAL_TIME:
            FlattenableUtils::write(buffer, size,
                    ::android::FenceTime::Snapshot::State::SIGNAL_TIME);
            FlattenableUtils::write(buffer, size, t.signalTimeNs);
            return NO_ERROR;
    }
    return NO_ERROR;
}

// Ref: frameworks/native/libs/gui/FrameTimestamps.cpp: FrameEventsDelta

/**
 * \brief Return a lower bound on the size of the non-fd buffer required to
 * flatten `FrameEventsDelta`.
 *
 * \param[in] t The input `FrameEventsDelta`.
 * \return A lower bound on the size of the flat buffer.
 */
constexpr size_t minFlattenedSize(
        HGraphicBufferProducer::FrameEventsDelta const& /* t */) {
    return sizeof(uint64_t) + // mFrameNumber
            sizeof(uint8_t) + // mIndex
            sizeof(uint8_t) + // mAddPostCompositeCalled
            sizeof(uint8_t) + // mAddRetireCalled
            sizeof(uint8_t) + // mAddReleaseCalled
            sizeof(nsecs_t) + // mPostedTime
            sizeof(nsecs_t) + // mRequestedPresentTime
            sizeof(nsecs_t) + // mLatchTime
            sizeof(nsecs_t) + // mFirstRefreshStartTime
            sizeof(nsecs_t); // mLastRefreshStartTime
}

/**
 * \brief Return the size of the non-fd buffer required to flatten
 * `FrameEventsDelta`.
 *
 * \param[in] t The input `FrameEventsDelta`.
 * \return The required size of the flat buffer.
 */
inline size_t getFlattenedSize(
        HGraphicBufferProducer::FrameEventsDelta const& t) {
    return minFlattenedSize(t) +
            getFlattenedSize(t.gpuCompositionDoneFence) +
            getFlattenedSize(t.displayPresentFence) +
            getFlattenedSize(t.displayRetireFence) +
            getFlattenedSize(t.releaseFence);
};

/**
 * \brief Return the number of file descriptors contained in
 * `FrameEventsDelta`.
 *
 * \param[in] t The input `FrameEventsDelta`.
 * \return The number of file descriptors contained in \p t.
 */
inline size_t getFdCount(
        HGraphicBufferProducer::FrameEventsDelta const& t) {
    return getFdCount(t.gpuCompositionDoneFence) +
            getFdCount(t.displayPresentFence) +
            getFdCount(t.displayRetireFence) +
            getFdCount(t.releaseFence);
};

/**
 * \brief Flatten `FrameEventsDelta`.
 *
 * \param[in] t The source `FrameEventsDelta`.
 * \param[out] nh The array of native handles that are cloned.
 * \param[in,out] buffer The pointer to the flat non-fd buffer.
 * \param[in,out] size The size of the flat non-fd buffer.
 * \param[in,out] fds The pointer to the flat fd buffer.
 * \param[in,out] numFds The size of the flat fd buffer.
 * \return `NO_ERROR` on success; other value on failure.
 *
 * On success, this function will duplicate file descriptors contained in \p t.
 * The cloned native handles will be stored in \p nh. These native handles will
 * need to be closed by the caller.
 */
// Ref: frameworks/native/libs/gui/FrameTimestamp.cpp:
//      FrameEventsDelta::flatten
inline status_t flatten(HGraphicBufferProducer::FrameEventsDelta const& t,
        std::vector<native_handle_t*>* nh,
        void*& buffer, size_t& size, int*& fds, size_t numFds) {
    // Check that t.index is within a valid range.
    if (t.index >= static_cast<uint32_t>(FrameEventHistory::MAX_FRAME_HISTORY)
            || t.index > std::numeric_limits<uint8_t>::max()) {
        return BAD_VALUE;
    }

    FlattenableUtils::write(buffer, size, t.frameNumber);

    // These are static_cast to uint8_t for alignment.
    FlattenableUtils::write(buffer, size, static_cast<uint8_t>(t.index));
    FlattenableUtils::write(
            buffer, size, static_cast<uint8_t>(t.addPostCompositeCalled));
    FlattenableUtils::write(
            buffer, size, static_cast<uint8_t>(t.addRetireCalled));
    FlattenableUtils::write(
            buffer, size, static_cast<uint8_t>(t.addReleaseCalled));

    FlattenableUtils::write(buffer, size, t.postedTimeNs);
    FlattenableUtils::write(buffer, size, t.requestedPresentTimeNs);
    FlattenableUtils::write(buffer, size, t.latchTimeNs);
    FlattenableUtils::write(buffer, size, t.firstRefreshStartTimeNs);
    FlattenableUtils::write(buffer, size, t.lastRefreshStartTimeNs);
    FlattenableUtils::write(buffer, size, t.dequeueReadyTime);

    // Fences
    HGraphicBufferProducer::FenceTimeSnapshot const* tSnapshot[4];
    tSnapshot[0] = &t.gpuCompositionDoneFence;
    tSnapshot[1] = &t.displayPresentFence;
    tSnapshot[2] = &t.displayRetireFence;
    tSnapshot[3] = &t.releaseFence;
    nh->resize(4);
    for (size_t snapshotIndex = 0; snapshotIndex < 4; ++snapshotIndex) {
        status_t status = flatten(
                *(tSnapshot[snapshotIndex]),
                &((*nh)[snapshotIndex]),
                buffer, size, fds, numFds);
        if (status != NO_ERROR) {
            while (snapshotIndex > 0) {
                --snapshotIndex;
                native_handle_close((*nh)[snapshotIndex]);
                native_handle_delete((*nh)[snapshotIndex]);
                (*nh)[snapshotIndex] = nullptr;
            }
            return status;
        }
    }
    return NO_ERROR;
}

// Ref: frameworks/native/libs/gui/FrameTimestamps.cpp: FrameEventHistoryDelta

/**
 * \brief Return the size of the non-fd buffer required to flatten
 * `HGraphicBufferProducer::FrameEventHistoryDelta`.
 *
 * \param[in] t The input `HGraphicBufferProducer::FrameEventHistoryDelta`.
 * \return The required size of the flat buffer.
 */
inline size_t getFlattenedSize(
        HGraphicBufferProducer::FrameEventHistoryDelta const& t) {
    size_t size = 4 + // mDeltas.size()
            sizeof(t.compositorTiming);
    for (size_t i = 0; i < t.deltas.size(); ++i) {
        size += getFlattenedSize(t.deltas[i]);
    }
    return size;
}

/**
 * \brief Return the number of file descriptors contained in
 * `HGraphicBufferProducer::FrameEventHistoryDelta`.
 *
 * \param[in] t The input `HGraphicBufferProducer::FrameEventHistoryDelta`.
 * \return The number of file descriptors contained in \p t.
 */
inline size_t getFdCount(
        HGraphicBufferProducer::FrameEventHistoryDelta const& t) {
    size_t numFds = 0;
    for (size_t i = 0; i < t.deltas.size(); ++i) {
        numFds += getFdCount(t.deltas[i]);
    }
    return numFds;
}

/**
 * \brief Flatten `FrameEventHistoryDelta`.
 *
 * \param[in] t The source `FrameEventHistoryDelta`.
 * \param[out] nh The array of arrays of cloned native handles.
 * \param[in,out] buffer The pointer to the flat non-fd buffer.
 * \param[in,out] size The size of the flat non-fd buffer.
 * \param[in,out] fds The pointer to the flat fd buffer.
 * \param[in,out] numFds The size of the flat fd buffer.
 * \return `NO_ERROR` on success; other value on failure.
 *
 * On success, this function will duplicate file descriptors contained in \p t.
 * The cloned native handles will be stored in \p nh. Before making the call, \p
 * nh should have enough space to store `n` pointers to arrays of native
 * handles, where `n` is the length of `t.deltas`, and each `nh[i]` should have
 * enough space to store `4` native handles.
 */
inline status_t flatten(
        HGraphicBufferProducer::FrameEventHistoryDelta const& t,
        std::vector<std::vector<native_handle_t*> >* nh,
        void*& buffer, size_t& size, int*& fds, size_t& numFds) {
    if (t.deltas.size() > ::android::FrameEventHistory::MAX_FRAME_HISTORY) {
        return BAD_VALUE;
    }
    if (size < getFlattenedSize(t)) {
        return NO_MEMORY;
    }

    FlattenableUtils::write(buffer, size, t.compositorTiming);

    FlattenableUtils::write(buffer, size, static_cast<uint32_t>(t.deltas.size()));
    nh->resize(t.deltas.size());
    for (size_t deltaIndex = 0; deltaIndex < t.deltas.size(); ++deltaIndex) {
        status_t status = flatten(
                t.deltas[deltaIndex], &((*nh)[deltaIndex]),
                buffer, size, fds, numFds);
        if (status != NO_ERROR) {
            while (deltaIndex > 0) {
                --deltaIndex;
                for (size_t snapshotIndex = 0;
                        snapshotIndex < 4; ++snapshotIndex) {
                    native_handle_close((*nh)[deltaIndex][snapshotIndex]);
                    native_handle_delete((*nh)[deltaIndex][snapshotIndex]);
                    (*nh)[deltaIndex][snapshotIndex] = nullptr;
                }
            }
            return status;
        }
    }
    return NO_ERROR;
}

/**
 * \brief Convert `HGraphicBufferProducer::FrameEventHistoryDelta` to
 * `::android::FrameEventHistoryDelta`.
 *
 * \param[out] l The destination `::android::FrameEventHistoryDelta`.
 * \param[in] t The source `HGraphicBufferProducer::FrameEventHistoryDelta`.
 *
 * This function will duplicate all file descriptors contained in \p t.
 */
inline bool convertTo(
        ::android::FrameEventHistoryDelta* l,
        HGraphicBufferProducer::FrameEventHistoryDelta const& t) {

    size_t const baseSize = getFlattenedSize(t);
    std::unique_ptr<uint8_t[]> baseBuffer(
            new (std::nothrow) uint8_t[baseSize]);
    if (!baseBuffer) {
        return false;
    }

    size_t const baseNumFds = getFdCount(t);
    std::unique_ptr<int[]> baseFds(
            new (std::nothrow) int[baseNumFds]);
    if (!baseFds) {
        return false;
    }

    void* buffer = static_cast<void*>(baseBuffer.get());
    size_t size = baseSize;
    int* fds = static_cast<int*>(baseFds.get());
    size_t numFds = baseNumFds;
    std::vector<std::vector<native_handle_t*> > nhAA;
    if (flatten(t, &nhAA, buffer, size, fds, numFds) != NO_ERROR) {
        return false;
    }

    void const* constBuffer = static_cast<void const*>(baseBuffer.get());
    size = baseSize;
    int const* constFds = static_cast<int const*>(baseFds.get());
    numFds = baseNumFds;
    if (l->unflatten(constBuffer, size, constFds, numFds) != NO_ERROR) {
        for (const auto& nhA : nhAA) {
            for (auto nh : nhA) {
                if (nh != nullptr) {
                    native_handle_close(nh);
                    native_handle_delete(nh);
                }
            }
        }
        return false;
    }

    for (const auto& nhA : nhAA) {
        for (const auto& nh : nhA) {
            if (nh != nullptr) {
                native_handle_delete(nh);
            }
        }
    }
    return true;
}

// Ref: frameworks/native/libs/gui/IGraphicBufferProducer.cpp:
//      IGraphicBufferProducer::QueueBufferOutput

/**
 * \brief Convert `HGraphicBufferProducer::QueueBufferOutput` to
 * `IGraphicBufferProducer::QueueBufferOutput`.
 *
 * \param[out] l The destination `IGraphicBufferProducer::QueueBufferOutput`.
 * \param[in] t The source `HGraphicBufferProducer::QueueBufferOutput`.
 *
 * This function will duplicate all file descriptors contained in \p t.
 */
// convert: HGraphicBufferProducer::QueueBufferOutput ->
// IGraphicBufferProducer::QueueBufferOutput
inline bool convertTo(
        BGraphicBufferProducer::QueueBufferOutput* l,
        HGraphicBufferProducer::QueueBufferOutput const& t) {
    if (!convertTo(&(l->frameTimestamps), t.frameTimestamps)) {
        return false;
    }
    l->width = t.width;
    l->height = t.height;
    l->transformHint = t.transformHint;
    l->numPendingBuffers = t.numPendingBuffers;
    l->nextFrameNumber = t.nextFrameNumber;
    l->bufferReplaced = t.bufferReplaced;
    return true;
}

/**
 * \brief Convert `IGraphicBufferProducer::DisconnectMode` to
 * `HGraphicBufferProducer::DisconnectMode`.
 *
 * \param[in] l The source `IGraphicBufferProducer::DisconnectMode`.
 * \return The corresponding `HGraphicBufferProducer::DisconnectMode`.
 */
inline HGraphicBufferProducer::DisconnectMode toHDisconnectMode(
        BGraphicBufferProducer::DisconnectMode l) {
    switch (l) {
        case BGraphicBufferProducer::DisconnectMode::Api:
            return HGraphicBufferProducer::DisconnectMode::API;
        case BGraphicBufferProducer::DisconnectMode::AllLocal:
            return HGraphicBufferProducer::DisconnectMode::ALL_LOCAL;
    }
    return HGraphicBufferProducer::DisconnectMode::API;
}

// H2BGraphicBufferProducer

status_t H2BGraphicBufferProducer::requestBuffer(int slot, sp<GraphicBuffer>* buf) {
    *buf = new GraphicBuffer();
    status_t fnStatus;
    status_t transStatus = toStatusT(mBase->requestBuffer(
            static_cast<int32_t>(slot),
            [&fnStatus, &buf] (Status status, AnwBuffer const& buffer) {
                fnStatus = toStatusT(status);
                if (!convertTo(buf->get(), buffer)) {
                    fnStatus = fnStatus == NO_ERROR ? BAD_VALUE : fnStatus;
                }
            }));
    return transStatus == NO_ERROR ? fnStatus : transStatus;
}

status_t H2BGraphicBufferProducer::setMaxDequeuedBufferCount(
        int maxDequeuedBuffers) {
    return toStatusT(mBase->setMaxDequeuedBufferCount(
            static_cast<int32_t>(maxDequeuedBuffers)));
}

status_t H2BGraphicBufferProducer::setAsyncMode(bool async) {
    return toStatusT(mBase->setAsyncMode(async));
}

// FIXME: usage bits truncated -- needs a 64-bits usage version
status_t H2BGraphicBufferProducer::dequeueBuffer(int* slot, sp<Fence>* fence, uint32_t w,
                                                 uint32_t h, ::android::PixelFormat format,
                                                 uint64_t usage, uint64_t* outBufferAge,
                                                 FrameEventHistoryDelta* outTimestamps) {
    *fence = new Fence();
    status_t fnStatus;
    status_t transStatus = toStatusT(mBase->dequeueBuffer(
            w, h, static_cast<PixelFormat>(format), uint32_t(usage),
            outTimestamps != nullptr,
            [&fnStatus, slot, fence, outTimestamps] (
                    Status status,
                    int32_t tSlot,
                    hidl_handle const& tFence,
                    HGraphicBufferProducer::FrameEventHistoryDelta const& tTs) {
                fnStatus = toStatusT(status);
                *slot = tSlot;
                if (!convertTo(fence->get(), tFence)) {
                    ALOGE("H2BGraphicBufferProducer::dequeueBuffer - "
                            "Invalid output fence");
                    fnStatus = fnStatus == NO_ERROR ? BAD_VALUE : fnStatus;
                }
                if (outTimestamps && !convertTo(outTimestamps, tTs)) {
                    ALOGE("H2BGraphicBufferProducer::dequeueBuffer - "
                            "Invalid output timestamps");
                    fnStatus = fnStatus == NO_ERROR ? BAD_VALUE : fnStatus;
                }
            }));
    if (outBufferAge) {
        // Since the HAL version doesn't return the buffer age, set it to 0:
        *outBufferAge = 0;
    }
    return transStatus == NO_ERROR ? fnStatus : transStatus;
}

status_t H2BGraphicBufferProducer::detachBuffer(int slot) {
    return toStatusT(mBase->detachBuffer(static_cast<int>(slot)));
}

status_t H2BGraphicBufferProducer::detachNextBuffer(
        sp<GraphicBuffer>* outBuffer, sp<Fence>* outFence) {
    *outBuffer = new GraphicBuffer();
    *outFence = new Fence();
    status_t fnStatus;
    status_t transStatus = toStatusT(mBase->detachNextBuffer(
            [&fnStatus, outBuffer, outFence] (
                    Status status,
                    AnwBuffer const& tBuffer,
                    hidl_handle const& tFence) {
                fnStatus = toStatusT(status);
                if (!convertTo(outFence->get(), tFence)) {
                    ALOGE("H2BGraphicBufferProducer::detachNextBuffer - "
                            "Invalid output fence");
                    fnStatus = fnStatus == NO_ERROR ? BAD_VALUE : fnStatus;
                }
                if (!convertTo(outBuffer->get(), tBuffer)) {
                    ALOGE("H2BGraphicBufferProducer::detachNextBuffer - "
                            "Invalid output buffer");
                    fnStatus = fnStatus == NO_ERROR ? BAD_VALUE : fnStatus;
                }
            }));
    return transStatus == NO_ERROR ? fnStatus : transStatus;
}

status_t H2BGraphicBufferProducer::attachBuffer(
        int* outSlot, const sp<GraphicBuffer>& buffer) {
    AnwBuffer tBuffer{};
    wrapAs(&tBuffer, *buffer);
    status_t fnStatus;
    status_t transStatus = toStatusT(mBase->attachBuffer(tBuffer,
            [&fnStatus, outSlot] (Status status, int32_t slot) {
                fnStatus = toStatusT(status);
                *outSlot = slot;
            }));
    return transStatus == NO_ERROR ? fnStatus : transStatus;
}

status_t H2BGraphicBufferProducer::queueBuffer(
        int slot,
        const QueueBufferInput& input,
        QueueBufferOutput* output) {
    HGraphicBufferProducer::QueueBufferInput tInput{};
    native_handle_t* nh;
    if (!wrapAs(&tInput, &nh, input)) {
        ALOGE("H2BGraphicBufferProducer::queueBuffer - "
                "Invalid input");
        return BAD_VALUE;
    }
    status_t fnStatus;
    status_t transStatus = toStatusT(mBase->queueBuffer(slot, tInput,
            [&fnStatus, output] (
                    Status status,
                    HGraphicBufferProducer::QueueBufferOutput const& tOutput) {
                fnStatus = toStatusT(status);
                if (!convertTo(output, tOutput)) {
                    ALOGE("H2BGraphicBufferProducer::queueBuffer - "
                            "Invalid output");
                    fnStatus = fnStatus == NO_ERROR ? BAD_VALUE : fnStatus;
                }
            }));
    native_handle_delete(nh);
    return transStatus == NO_ERROR ? fnStatus : transStatus;
}

status_t H2BGraphicBufferProducer::cancelBuffer(int slot, const sp<Fence>& fence) {
    hidl_handle tFence;
    native_handle_t* nh = nullptr;
    if ((fence == nullptr) || !wrapAs(&tFence, &nh, *fence)) {
        ALOGE("H2BGraphicBufferProducer::cancelBuffer - "
                "Invalid input fence");
        return BAD_VALUE;
    }

    status_t status = toStatusT(mBase->cancelBuffer(
            static_cast<int32_t>(slot), tFence));
    native_handle_delete(nh);
    return status;
}

int H2BGraphicBufferProducer::query(int what, int* value) {
    int result;
    status_t transStatus = toStatusT(mBase->query(
            static_cast<int32_t>(what),
            [&result, value] (int32_t tResult, int32_t tValue) {
                result = static_cast<int>(tResult);
                *value = static_cast<int>(tValue);
            }));
    return transStatus == NO_ERROR ? result : static_cast<int>(transStatus);
}

status_t H2BGraphicBufferProducer::connect(
        const sp<IProducerListener>& listener, int api,
        bool producerControlledByApp, QueueBufferOutput* output) {
    sp<HProducerListener> tListener = listener == nullptr ?
            nullptr : new B2HProducerListener(listener);
    status_t fnStatus;
    status_t transStatus = toStatusT(mBase->connect(
            tListener, static_cast<int32_t>(api), producerControlledByApp,
            [&fnStatus, output] (
                    Status status,
                    HGraphicBufferProducer::QueueBufferOutput const& tOutput) {
                fnStatus = toStatusT(status);
                if (!convertTo(output, tOutput)) {
                    ALOGE("H2BGraphicBufferProducer::connect - "
                            "Invalid output");
                    fnStatus = fnStatus == NO_ERROR ? BAD_VALUE : fnStatus;
                }
            }));
    return transStatus == NO_ERROR ? fnStatus : transStatus;
}

status_t H2BGraphicBufferProducer::disconnect(int api, DisconnectMode mode) {
    return toStatusT(mBase->disconnect(
            static_cast<int32_t>(api), toHDisconnectMode(mode)));
}

status_t H2BGraphicBufferProducer::setSidebandStream(
        const sp<NativeHandle>& stream) {
    return toStatusT(mBase->setSidebandStream(stream == nullptr ? nullptr : stream->handle()));
}

// FIXME: usage bits truncated -- needs a 64-bits usage version
void H2BGraphicBufferProducer::allocateBuffers(uint32_t width, uint32_t height,
        ::android::PixelFormat format, uint64_t usage) {
    mBase->allocateBuffers(
            width, height, static_cast<PixelFormat>(format), uint32_t(usage));
}

status_t H2BGraphicBufferProducer::allowAllocation(bool allow) {
    return toStatusT(mBase->allowAllocation(allow));
}

status_t H2BGraphicBufferProducer::setGenerationNumber(uint32_t generationNumber) {
    return toStatusT(mBase->setGenerationNumber(generationNumber));
}

String8 H2BGraphicBufferProducer::getConsumerName() const {
    String8 lName;
    mBase->getConsumerName([&lName] (hidl_string const& name) {
                lName = name.c_str();
            });
    return lName;
}

status_t H2BGraphicBufferProducer::setSharedBufferMode(bool sharedBufferMode) {
    return toStatusT(mBase->setSharedBufferMode(sharedBufferMode));
}

status_t H2BGraphicBufferProducer::setAutoRefresh(bool autoRefresh) {
    return toStatusT(mBase->setAutoRefresh(autoRefresh));
}

status_t H2BGraphicBufferProducer::setDequeueTimeout(nsecs_t timeout) {
    return toStatusT(mBase->setDequeueTimeout(static_cast<int64_t>(timeout)));
}

status_t H2BGraphicBufferProducer::getLastQueuedBuffer(
        sp<GraphicBuffer>* outBuffer,
        sp<Fence>* outFence,
        float outTransformMatrix[16]) {
    status_t fnStatus;
    status_t transStatus = toStatusT(mBase->getLastQueuedBuffer(
            [&fnStatus, outBuffer, outFence, &outTransformMatrix] (
                    Status status,
                    AnwBuffer const& buffer,
                    hidl_handle const& fence,
                    hidl_array<float, 16> const& transformMatrix) {
                fnStatus = toStatusT(status);
                *outBuffer = new GraphicBuffer();
                if (!convertTo(outBuffer->get(), buffer)) {
                    ALOGE("H2BGraphicBufferProducer::getLastQueuedBuffer - "
                            "Invalid output buffer");
                    fnStatus = fnStatus == NO_ERROR ? BAD_VALUE : fnStatus;
                }
                *outFence = new Fence();
                if (!convertTo(outFence->get(), fence)) {
                    ALOGE("H2BGraphicBufferProducer::getLastQueuedBuffer - "
                            "Invalid output fence");
                    fnStatus = fnStatus == NO_ERROR ? BAD_VALUE : fnStatus;
                }
                std::copy(transformMatrix.data(),
                        transformMatrix.data() + 16,
                        outTransformMatrix);
            }));
    return transStatus == NO_ERROR ? fnStatus : transStatus;
}

void H2BGraphicBufferProducer::getFrameTimestamps(FrameEventHistoryDelta* outDelta) {
    mBase->getFrameTimestamps([outDelta] (
            HGraphicBufferProducer::FrameEventHistoryDelta const& tDelta) {
                convertTo(outDelta, tDelta);
            });
}

status_t H2BGraphicBufferProducer::getUniqueId(uint64_t* outId) const {
    status_t fnStatus;
    status_t transStatus = toStatusT(mBase->getUniqueId(
            [&fnStatus, outId] (Status status, uint64_t id) {
                fnStatus = toStatusT(status);
                *outId = id;
            }));
    return transStatus == NO_ERROR ? fnStatus : transStatus;
}

status_t H2BGraphicBufferProducer::getConsumerUsage(uint64_t* outUsage) const {
    ALOGW("getConsumerUsage is not fully supported");
    int result;
    status_t transStatus = toStatusT(mBase->query(
            NATIVE_WINDOW_CONSUMER_USAGE_BITS,
            [&result, outUsage] (int32_t tResult, int32_t tValue) {
                result = static_cast<int>(tResult);
                *outUsage = static_cast<uint64_t>(tValue);
            }));
    return transStatus == NO_ERROR ? result : static_cast<int>(transStatus);
}

}  // namespace utils
}  // namespace V1_0
}  // namespace bufferqueue
}  // namespace graphics
}  // namespace hardware
}  // namespace android
