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

#ifndef ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V1_0_CONVERSION_H_
#define ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V1_0_CONVERSION_H_

#include <vector>
#include <list>

#include <unistd.h>

#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include <binder/Binder.h>
#include <binder/Status.h>
#include <ui/FenceTime.h>
#include <cutils/native_handle.h>
#include <gui/IGraphicBufferProducer.h>

#include <android/hardware/graphics/bufferqueue/1.0/IProducerListener.h>

namespace android {
namespace conversion {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_handle;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::android::status_t;

using ::android::String8;

using ::android::hardware::media::V1_0::Rect;
using ::android::hardware::media::V1_0::Region;

using ::android::hardware::graphics::common::V1_0::Dataspace;

using ::android::hardware::graphics::common::V1_0::PixelFormat;

using ::android::hardware::media::V1_0::AnwBuffer;
using ::android::GraphicBuffer;

typedef ::android::hardware::graphics::bufferqueue::V1_0::IGraphicBufferProducer
        HGraphicBufferProducer;
typedef ::android::IGraphicBufferProducer
        BGraphicBufferProducer;

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
native_handle_t* native_handle_create_from_fd(int fd);

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
int native_handle_read_fd(native_handle_t const* nh, int index = 0);

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
 * \brief Convert `Return<void>` to `binder::Status`.
 *
 * \param[in] t The source `Return<void>`.
 * \return The corresponding `binder::Status`.
 */
// convert: Return<void> -> ::android::binder::Status
::android::binder::Status toBinderStatus(Return<void> const& t);

/**
 * \brief Convert `Return<void>` to `status_t`. This is for legacy binder calls.
 *
 * \param[in] t The source `Return<void>`.
 * \return The corresponding `status_t`.
 */
// convert: Return<void> -> status_t
status_t toStatusT(Return<void> const& t);

/**
 * \brief Wrap `native_handle_t*` in `hidl_handle`.
 *
 * \param[in] nh The source `native_handle_t*`.
 * \return The `hidl_handle` that points to \p nh.
 */
// wrap: native_handle_t* -> hidl_handle
hidl_handle inHidlHandle(native_handle_t const* nh);

/**
 * \brief Convert `int32_t` to `Dataspace`.
 *
 * \param[in] l The source `int32_t`.
 * \result The corresponding `Dataspace`.
 */
// convert: int32_t -> Dataspace
Dataspace toHardwareDataspace(int32_t l);

/**
 * \brief Convert `Dataspace` to `int32_t`.
 *
 * \param[in] t The source `Dataspace`.
 * \result The corresponding `int32_t`.
 */
// convert: Dataspace -> int32_t
int32_t toRawDataspace(Dataspace const& t);

/**
 * \brief Wrap an opaque buffer inside a `hidl_vec<uint8_t>`.
 *
 * \param[in] l The pointer to the beginning of the opaque buffer.
 * \param[in] size The size of the buffer.
 * \return A `hidl_vec<uint8_t>` that points to the buffer.
 */
// wrap: void*, size_t -> hidl_vec<uint8_t>
hidl_vec<uint8_t> inHidlBytes(void const* l, size_t size);

/**
 * \brief Create a `hidl_vec<uint8_t>` that is a copy of an opaque buffer.
 *
 * \param[in] l The pointer to the beginning of the opaque buffer.
 * \param[in] size The size of the buffer.
 * \return A `hidl_vec<uint8_t>` that is a copy of the input buffer.
 */
// convert: void*, size_t -> hidl_vec<uint8_t>
hidl_vec<uint8_t> toHidlBytes(void const* l, size_t size);

/**
 * \brief Wrap `GraphicBuffer` in `AnwBuffer`.
 *
 * \param[out] t The wrapper of type `AnwBuffer`.
 * \param[in] l The source `GraphicBuffer`.
 */
// wrap: GraphicBuffer -> AnwBuffer
void wrapAs(AnwBuffer* t, GraphicBuffer const& l);

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
bool convertTo(GraphicBuffer* l, AnwBuffer const& t);

/**
 * Conversion functions for types outside media
 * ============================================
 *
 * Some objects in libui and libgui that were made to go through binder calls do
 * not expose ways to read or write their fields to the public. To pass an
 * object of this kind through the HIDL boundary, translation functions need to
 * work around the access restriction by using the publicly available
 * `flatten()` and `unflatten()` functions.
 *
 * All `flatten()` and `unflatten()` overloads follow the same convention as
 * follows:
 *
 *     status_t flatten(ObjectType const& object,
 *                      [OtherType const& other, ...]
 *                      void*& buffer, size_t& size,
 *                      int*& fds, size_t& numFds)
 *
 *     status_t unflatten(ObjectType* object,
 *                        [OtherType* other, ...,]
 *                        void*& buffer, size_t& size,
 *                        int*& fds, size_t& numFds)
 *
 * The number of `other` parameters varies depending on the `ObjectType`. For
 * example, in the process of unflattening an object that contains
 * `hidl_handle`, `other` is needed to hold `native_handle_t` objects that will
 * be created.
 *
 * The last four parameters always work the same way in all overloads of
 * `flatten()` and `unflatten()`:
 * - For `flatten()`, `buffer` is the pointer to the non-fd buffer to be filled,
 *   `size` is the size (in bytes) of the non-fd buffer pointed to by `buffer`,
 *   `fds` is the pointer to the fd buffer to be filled, and `numFds` is the
 *   size (in ints) of the fd buffer pointed to by `fds`.
 * - For `unflatten()`, `buffer` is the pointer to the non-fd buffer to be read
 *   from, `size` is the size (in bytes) of the non-fd buffer pointed to by
 *   `buffer`, `fds` is the pointer to the fd buffer to be read from, and
 *   `numFds` is the size (in ints) of the fd buffer pointed to by `fds`.
 * - After a successful call to `flatten()` or `unflatten()`, `buffer` and `fds`
 *   will be advanced, while `size` and `numFds` will be decreased to reflect
 *   how much storage/data of the two buffers (fd and non-fd) have been used.
 * - After an unsuccessful call, the values of `buffer`, `size`, `fds` and
 *   `numFds` are invalid.
 *
 * The return value of a successful `flatten()` or `unflatten()` call will be
 * `OK` (also aliased as `NO_ERROR`). Any other values indicate a failure.
 *
 * For each object type that supports flattening, there will be two accompanying
 * functions: `getFlattenedSize()` and `getFdCount()`. `getFlattenedSize()` will
 * return the size of the non-fd buffer that the object will need for
 * flattening. `getFdCount()` will return the size of the fd buffer that the
 * object will need for flattening.
 *
 * The set of these four functions, `getFlattenedSize()`, `getFdCount()`,
 * `flatten()` and `unflatten()`, are similar to functions of the same name in
 * the abstract class `Flattenable`. The only difference is that functions in
 * this file are not member functions of the object type. For example, we write
 *
 *     flatten(x, buffer, size, fds, numFds)
 *
 * instead of
 *
 *     x.flatten(buffer, size, fds, numFds)
 *
 * because we cannot modify the type of `x`.
 *
 * There is one exception to the naming convention: `hidl_handle` that
 * represents a fence. The four functions for this "Fence" type have the word
 * "Fence" attched to their names because the object type, which is
 * `hidl_handle`, does not carry the special meaning that the object itself can
 * only contain zero or one file descriptor.
 */

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
size_t getFenceFlattenedSize(hidl_handle const& fence);

/**
 * \brief Return the number of file descriptors contained in a fence.
 *
 * \param[in] fence The input fence of type `hidl_handle`.
 * \return `0` if \p fence does not contain a valid file descriptor, or `1`
 * otherwise.
 */
size_t getFenceFdCount(hidl_handle const& fence);

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
status_t unflattenFence(hidl_handle* fence, native_handle_t** nh,
        void const*& buffer, size_t& size, int const*& fds, size_t& numFds);

/**
 * \brief Flatten `hidl_handle` as `Fence`.
 *
 * \param[in] t The source `hidl_handle`.
 * \param[in,out] buffer The pointer to the flat non-fd buffer.
 * \param[in,out] size The size of the flat non-fd buffer.
 * \param[in,out] fds The pointer to the flat fd buffer.
 * \param[in,out] numFds The size of the flat fd buffer.
 * \return `NO_ERROR` on success; other value on failure.
 */
status_t flattenFence(hidl_handle const& fence,
        void*& buffer, size_t& size, int*& fds, size_t& numFds);

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
bool wrapAs(hidl_handle* t, native_handle_t** nh, Fence const& l);

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
bool convertTo(Fence* l, hidl_handle const& t);

// Ref: frameworks/native/libs/ui/FenceTime.cpp: FenceTime::Snapshot

/**
 * \brief Return the size of the non-fd buffer required to flatten
 * `FenceTimeSnapshot`.
 *
 * \param[in] t The input `FenceTimeSnapshot`.
 * \return The required size of the flat buffer.
 */
size_t getFlattenedSize(HGraphicBufferProducer::FenceTimeSnapshot const& t);

/**
 * \brief Return the number of file descriptors contained in
 * `FenceTimeSnapshot`.
 *
 * \param[in] t The input `FenceTimeSnapshot`.
 * \return The number of file descriptors contained in \p snapshot.
 */
size_t getFdCount(HGraphicBufferProducer::FenceTimeSnapshot const& t);

/**
 * \brief Flatten `FenceTimeSnapshot`.
 *
 * \param[in] t The source `FenceTimeSnapshot`.
 * \param[in,out] buffer The pointer to the flat non-fd buffer.
 * \param[in,out] size The size of the flat non-fd buffer.
 * \param[in,out] fds The pointer to the flat fd buffer.
 * \param[in,out] numFds The size of the flat fd buffer.
 * \return `NO_ERROR` on success; other value on failure.
 *
 * This function will duplicate the file descriptor in `t.fence` if `t.state ==
 * FENCE`.
 */
status_t flatten(HGraphicBufferProducer::FenceTimeSnapshot const& t,
        void*& buffer, size_t& size, int*& fds, size_t& numFds);

/**
 * \brief Unflatten `FenceTimeSnapshot`.
 *
 * \param[out] t The destination `FenceTimeSnapshot`.
 * \param[out] nh The underlying native handle.
 * \param[in,out] buffer The pointer to the flat non-fd buffer.
 * \param[in,out] size The size of the flat non-fd buffer.
 * \param[in,out] fds The pointer to the flat fd buffer.
 * \param[in,out] numFds The size of the flat fd buffer.
 * \return `NO_ERROR` on success; other value on failure.
 *
 * If the return value is `NO_ERROR` and the constructed snapshot contains a
 * file descriptor, \p nh will be created to hold that file descriptor. In this
 * case, \p nh needs to be deleted with `native_handle_delete()` afterwards.
 */
status_t unflatten(
        HGraphicBufferProducer::FenceTimeSnapshot* t, native_handle_t** nh,
        void const*& buffer, size_t& size, int const*& fds, size_t& numFds);

// Ref: frameworks/native/libs/gui/FrameTimestamps.cpp: FrameEventsDelta

/**
 * \brief Return the size of the non-fd buffer required to flatten
 * `FrameEventsDelta`.
 *
 * \param[in] t The input `FrameEventsDelta`.
 * \return The required size of the flat buffer.
 */
size_t getFlattenedSize(HGraphicBufferProducer::FrameEventsDelta const& t);

/**
 * \brief Return the number of file descriptors contained in
 * `FrameEventsDelta`.
 *
 * \param[in] t The input `FrameEventsDelta`.
 * \return The number of file descriptors contained in \p t.
 */
size_t getFdCount(HGraphicBufferProducer::FrameEventsDelta const& t);

/**
 * \brief Unflatten `FrameEventsDelta`.
 *
 * \param[out] t The destination `FrameEventsDelta`.
 * \param[out] nh The underlying array of native handles.
 * \param[in,out] buffer The pointer to the flat non-fd buffer.
 * \param[in,out] size The size of the flat non-fd buffer.
 * \param[in,out] fds The pointer to the flat fd buffer.
 * \param[in,out] numFds The size of the flat fd buffer.
 * \return `NO_ERROR` on success; other value on failure.
 *
 * If the return value is `NO_ERROR`, \p nh will have length 4, and it will be
 * populated with `nullptr` or newly created handles. Each non-null slot in \p
 * nh will need to be deleted manually with `native_handle_delete()`.
 */
status_t unflatten(HGraphicBufferProducer::FrameEventsDelta* t,
        std::vector<native_handle_t*>* nh,
        void const*& buffer, size_t& size, int const*& fds, size_t& numFds);

/**
 * \brief Flatten `FrameEventsDelta`.
 *
 * \param[in] t The source `FrameEventsDelta`.
 * \param[in,out] buffer The pointer to the flat non-fd buffer.
 * \param[in,out] size The size of the flat non-fd buffer.
 * \param[in,out] fds The pointer to the flat fd buffer.
 * \param[in,out] numFds The size of the flat fd buffer.
 * \return `NO_ERROR` on success; other value on failure.
 *
 * This function will duplicate file descriptors contained in \p t.
 */
// Ref: frameworks/native/libs/gui/FrameTimestamp.cpp:
//      FrameEventsDelta::flatten
status_t flatten(HGraphicBufferProducer::FrameEventsDelta const& t,
        void*& buffer, size_t& size, int*& fds, size_t numFds);

// Ref: frameworks/native/libs/gui/FrameTimestamps.cpp: FrameEventHistoryDelta

/**
 * \brief Return the size of the non-fd buffer required to flatten
 * `HGraphicBufferProducer::FrameEventHistoryDelta`.
 *
 * \param[in] t The input `HGraphicBufferProducer::FrameEventHistoryDelta`.
 * \return The required size of the flat buffer.
 */
size_t getFlattenedSize(
        HGraphicBufferProducer::FrameEventHistoryDelta const& t);

/**
 * \brief Return the number of file descriptors contained in
 * `HGraphicBufferProducer::FrameEventHistoryDelta`.
 *
 * \param[in] t The input `HGraphicBufferProducer::FrameEventHistoryDelta`.
 * \return The number of file descriptors contained in \p t.
 */
size_t getFdCount(
        HGraphicBufferProducer::FrameEventHistoryDelta const& t);

/**
 * \brief Unflatten `FrameEventHistoryDelta`.
 *
 * \param[out] t The destination `FrameEventHistoryDelta`.
 * \param[out] nh The underlying array of arrays of native handles.
 * \param[in,out] buffer The pointer to the flat non-fd buffer.
 * \param[in,out] size The size of the flat non-fd buffer.
 * \param[in,out] fds The pointer to the flat fd buffer.
 * \param[in,out] numFds The size of the flat fd buffer.
 * \return `NO_ERROR` on success; other value on failure.
 *
 * If the return value is `NO_ERROR`, \p nh will be populated with `nullptr` or
 * newly created handles. The second dimension of \p nh will be 4. Each non-null
 * slot in \p nh will need to be deleted manually with `native_handle_delete()`.
 */
status_t unflatten(
        HGraphicBufferProducer::FrameEventHistoryDelta* t,
        std::vector<std::vector<native_handle_t*> >* nh,
        void const*& buffer, size_t& size, int const*& fds, size_t& numFds);

/**
 * \brief Flatten `FrameEventHistoryDelta`.
 *
 * \param[in] t The source `FrameEventHistoryDelta`.
 * \param[in,out] buffer The pointer to the flat non-fd buffer.
 * \param[in,out] size The size of the flat non-fd buffer.
 * \param[in,out] fds The pointer to the flat fd buffer.
 * \param[in,out] numFds The size of the flat fd buffer.
 * \return `NO_ERROR` on success; other value on failure.
 *
 * This function will duplicate file descriptors contained in \p t.
 */
status_t flatten(
        HGraphicBufferProducer::FrameEventHistoryDelta const& t,
        void*& buffer, size_t& size, int*& fds, size_t& numFds);

/**
 * \brief Wrap `::android::FrameEventHistoryData` in
 * `HGraphicBufferProducer::FrameEventHistoryDelta`.
 *
 * \param[out] t The wrapper of type
 * `HGraphicBufferProducer::FrameEventHistoryDelta`.
 * \param[out] nh The array of array of native handles that are referred to by
 * members of \p t.
 * \param[in] l The source `::android::FrameEventHistoryDelta`.
 *
 * On success, each member of \p nh will be either `nullptr` or a newly created
 * native handle. All the non-`nullptr` elements must be deleted individually
 * with `native_handle_delete()`.
 */
bool wrapAs(HGraphicBufferProducer::FrameEventHistoryDelta* t,
        std::vector<std::vector<native_handle_t*> >* nh,
        ::android::FrameEventHistoryDelta const& l);

/**
 * \brief Convert `HGraphicBufferProducer::FrameEventHistoryDelta` to
 * `::android::FrameEventHistoryDelta`.
 *
 * \param[out] l The destination `::android::FrameEventHistoryDelta`.
 * \param[in] t The source `HGraphicBufferProducer::FrameEventHistoryDelta`.
 *
 * This function will duplicate all file descriptors contained in \p t.
 */
bool convertTo(
        ::android::FrameEventHistoryDelta* l,
        HGraphicBufferProducer::FrameEventHistoryDelta const& t);

// Ref: frameworks/native/libs/ui/Region.cpp

/**
 * \brief Return the size of the buffer required to flatten `Region`.
 *
 * \param[in] t The input `Region`.
 * \return The required size of the flat buffer.
 */
size_t getFlattenedSize(Region const& t);

/**
 * \brief Unflatten `Region`.
 *
 * \param[out] t The destination `Region`.
 * \param[in,out] buffer The pointer to the flat buffer.
 * \param[in,out] size The size of the flat buffer.
 * \return `NO_ERROR` on success; other value on failure.
 */
status_t unflatten(Region* t, void const*& buffer, size_t& size);

/**
 * \brief Flatten `Region`.
 *
 * \param[in] t The source `Region`.
 * \param[in,out] buffer The pointer to the flat buffer.
 * \param[in,out] size The size of the flat buffer.
 * \return `NO_ERROR` on success; other value on failure.
 */
status_t flatten(Region const& t, void*& buffer, size_t& size);

/**
 * \brief Convert `::android::Region` to `Region`.
 *
 * \param[out] t The destination `Region`.
 * \param[in] l The source `::android::Region`.
 */
// convert: ::android::Region -> Region
bool convertTo(Region* t, ::android::Region const& l);

/**
 * \brief Convert `Region` to `::android::Region`.
 *
 * \param[out] l The destination `::android::Region`.
 * \param[in] t The source `Region`.
 */
// convert: Region -> ::android::Region
bool convertTo(::android::Region* l, Region const& t);

// Ref: frameworks/native/libs/gui/BGraphicBufferProducer.cpp:
//      BGraphicBufferProducer::QueueBufferInput

/**
 * \brief Return the size of the buffer required to flatten
 * `HGraphicBufferProducer::QueueBufferInput`.
 *
 * \param[in] t The input `HGraphicBufferProducer::QueueBufferInput`.
 * \return The required size of the flat buffer.
 */
size_t getFlattenedSize(HGraphicBufferProducer::QueueBufferInput const& t);

/**
 * \brief Return the number of file descriptors contained in
 * `HGraphicBufferProducer::QueueBufferInput`.
 *
 * \param[in] t The input `HGraphicBufferProducer::QueueBufferInput`.
 * \return The number of file descriptors contained in \p t.
 */
size_t getFdCount(
        HGraphicBufferProducer::QueueBufferInput const& t);
/**
 * \brief Flatten `HGraphicBufferProducer::QueueBufferInput`.
 *
 * \param[in] t The source `HGraphicBufferProducer::QueueBufferInput`.
 * \param[out] nh The native handle cloned from `t.fence`.
 * \param[in,out] buffer The pointer to the flat non-fd buffer.
 * \param[in,out] size The size of the flat non-fd buffer.
 * \param[in,out] fds The pointer to the flat fd buffer.
 * \param[in,out] numFds The size of the flat fd buffer.
 * \return `NO_ERROR` on success; other value on failure.
 *
 * This function will duplicate the file descriptor in `t.fence`. */
status_t flatten(HGraphicBufferProducer::QueueBufferInput const& t,
        native_handle_t** nh,
        void*& buffer, size_t& size, int*& fds, size_t& numFds);

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
status_t unflatten(
        HGraphicBufferProducer::QueueBufferInput* t, native_handle_t** nh,
        void const*& buffer, size_t& size, int const*& fds, size_t& numFds);

/**
 * \brief Wrap `BGraphicBufferProducer::QueueBufferInput` in
 * `HGraphicBufferProducer::QueueBufferInput`.
 *
 * \param[out] t The wrapper of type
 * `HGraphicBufferProducer::QueueBufferInput`.
 * \param[out] nh The underlying native handle for `t->fence`.
 * \param[in] l The source `BGraphicBufferProducer::QueueBufferInput`.
 *
 * If the return value is `true` and `t->fence` contains a valid file
 * descriptor, \p nh will be a newly created native handle holding that file
 * descriptor. \p nh needs to be deleted with `native_handle_delete()`
 * afterwards.
 */
bool wrapAs(
        HGraphicBufferProducer::QueueBufferInput* t,
        native_handle_t** nh,
        BGraphicBufferProducer::QueueBufferInput const& l);

/**
 * \brief Convert `HGraphicBufferProducer::QueueBufferInput` to
 * `BGraphicBufferProducer::QueueBufferInput`.
 *
 * \param[out] l The destination `BGraphicBufferProducer::QueueBufferInput`.
 * \param[in] t The source `HGraphicBufferProducer::QueueBufferInput`.
 *
 * If `t.fence` has a valid file descriptor, it will be duplicated.
 */
bool convertTo(
        BGraphicBufferProducer::QueueBufferInput* l,
        HGraphicBufferProducer::QueueBufferInput const& t);

// Ref: frameworks/native/libs/gui/BGraphicBufferProducer.cpp:
//      BGraphicBufferProducer::QueueBufferOutput

/**
 * \brief Wrap `BGraphicBufferProducer::QueueBufferOutput` in
 * `HGraphicBufferProducer::QueueBufferOutput`.
 *
 * \param[out] t The wrapper of type
 * `HGraphicBufferProducer::QueueBufferOutput`.
 * \param[out] nh The array of array of native handles that are referred to by
 * members of \p t.
 * \param[in] l The source `BGraphicBufferProducer::QueueBufferOutput`.
 *
 * On success, each member of \p nh will be either `nullptr` or a newly created
 * native handle. All the non-`nullptr` elements must be deleted individually
 * with `native_handle_delete()`.
 */
// wrap: BGraphicBufferProducer::QueueBufferOutput ->
// HGraphicBufferProducer::QueueBufferOutput
bool wrapAs(HGraphicBufferProducer::QueueBufferOutput* t,
        std::vector<std::vector<native_handle_t*> >* nh,
        BGraphicBufferProducer::QueueBufferOutput const& l);

/**
 * \brief Convert `HGraphicBufferProducer::QueueBufferOutput` to
 * `BGraphicBufferProducer::QueueBufferOutput`.
 *
 * \param[out] l The destination `BGraphicBufferProducer::QueueBufferOutput`.
 * \param[in] t The source `HGraphicBufferProducer::QueueBufferOutput`.
 *
 * This function will duplicate all file descriptors contained in \p t.
 */
// convert: HGraphicBufferProducer::QueueBufferOutput ->
// BGraphicBufferProducer::QueueBufferOutput
bool convertTo(
        BGraphicBufferProducer::QueueBufferOutput* l,
        HGraphicBufferProducer::QueueBufferOutput const& t);

/**
 * \brief Convert `BGraphicBufferProducer::DisconnectMode` to
 * `HGraphicBufferProducer::DisconnectMode`.
 *
 * \param[in] l The source `BGraphicBufferProducer::DisconnectMode`.
 * \return The corresponding `HGraphicBufferProducer::DisconnectMode`.
 */
HGraphicBufferProducer::DisconnectMode toHidlDisconnectMode(
        BGraphicBufferProducer::DisconnectMode l);

/**
 * \brief Convert `HGraphicBufferProducer::DisconnectMode` to
 * `BGraphicBufferProducer::DisconnectMode`.
 *
 * \param[in] l The source `HGraphicBufferProducer::DisconnectMode`.
 * \return The corresponding `BGraphicBufferProducer::DisconnectMode`.
 */
BGraphicBufferProducer::DisconnectMode toGuiDisconnectMode(
        HGraphicBufferProducer::DisconnectMode t);

}  // namespace conversion
}  // namespace android

#endif  // ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V1_0_CONVERSION_H_
