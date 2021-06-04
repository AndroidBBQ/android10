/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef C2BUFFER_H_
#define C2BUFFER_H_

#include <C2.h>
#include <C2BufferBase.h>
#include <C2Param.h> // for C2Info

#include <memory>
#include <vector>

#ifdef __ANDROID__
#include <android-C2Buffer.h>
#else

typedef void* C2Handle;

#endif

/// \defgroup buffer Buffers
/// @{

/// \defgroup buffer_sync Synchronization
/// @{

/**
 * Synchronization is accomplished using event and fence objects.
 *
 * These are cross-process extensions of promise/future infrastructure.
 * Events are analogous to std::promise<void>, whereas fences are to std::shared_future<void>.
 *
 * Fences and events are shareable/copyable.
 *
 * Fences are used in two scenarios, and all copied instances refer to the same event.
 * \todo do events need to be copyable or should they be unique?
 *
 * acquire sync fence object: signaled when it is safe for the component or client to access
 * (the contents of) an object.
 *
 * release sync fence object: \todo
 *
 * Fences can be backed by hardware. Hardware fences are guaranteed to signal NO MATTER WHAT within
 * a short (platform specific) amount of time; this guarantee is usually less than 15 msecs.
 */

/**
 * Fence object used by components and the framework.
 *
 * Implements the waiting for an event, analogous to a 'future'.
 *
 * To be implemented by vendors if using HW fences.
 */
class C2Fence {
public:
    /**
     * Waits for a fence to be signaled with a timeout.
     *
     * \todo a mechanism to cancel a wait - for now the only way to do this is to abandon the
     * event, but fences are shared so canceling a wait will cancel all waits.
     *
     * \param timeoutNs           the maximum time to wait in nsecs
     *
     * \retval C2_OK            the fence has been signaled
     * \retval C2_TIMED_OUT     the fence has not been signaled within the timeout
     * \retval C2_BAD_STATE     the fence has been abandoned without being signaled (it will never
     *                          be signaled)
     * \retval C2_REFUSED       no permission to wait for the fence (unexpected - system)
     * \retval C2_CORRUPTED     some unknown error prevented waiting for the fence (unexpected)
     */
    c2_status_t wait(c2_nsecs_t timeoutNs);

    /**
     * Used to check if this fence is valid (if there is a chance for it to be signaled.)
     * A fence becomes invalid if the controling event is destroyed without it signaling the fence.
     *
     * \return whether this fence is valid
     */
    bool valid() const;

    /**
     * Used to check if this fence has been signaled (is ready).
     *
     * \return whether this fence has been signaled
     */
    bool ready() const;

    /**
     * Returns a file descriptor that can be used to wait for this fence in a select system call.
     * \note The returned file descriptor, if valid, must be closed by the caller.
     *
     * This can be used in e.g. poll() system calls. This file becomes readable (POLLIN) when the
     * fence is signaled, and bad (POLLERR) if the fence is abandoned.
     *
     * \return a file descriptor representing this fence (with ownership), or -1 if the fence
     * has already been signaled (\todo or abandoned).
     *
     * \todo this must be compatible with fences used by gralloc
     */
    int fd() const;

    /**
     * Returns whether this fence is a hardware-backed fence.
     * \return whether this is a hardware fence
     */
    bool isHW() const;

    /**
     * Null-fence. A fence that has fired.
     */
    constexpr C2Fence() : mImpl(nullptr) { }

private:
    class Impl;
    std::shared_ptr<Impl> mImpl;
    C2Fence(std::shared_ptr<Impl> impl);
    friend struct _C2FenceFactory;
};

/**
 * Event object used by components and the framework.
 *
 * Implements the signaling of an event, analogous to a 'promise'.
 *
 * Hardware backed events do not go through this object, and must be exposed directly as fences
 * by vendors.
 */
class C2Event {
public:
    /**
     * Returns a fence for this event.
     */
    C2Fence fence() const;

    /**
     * Signals (all) associated fence(s).
     * This has no effect no effect if the event was already signaled or abandoned.
     *
     * \retval C2_OK            the fence(s) were successfully signaled
     * \retval C2_BAD_STATE     the fence(s) have already been abandoned or merged (caller error)
     * \retval C2_DUPLICATE     the fence(s) have already been signaled (caller error)
     * \retval C2_REFUSED       no permission to signal the fence (unexpected - system)
     * \retval C2_CORRUPTED     some unknown error prevented signaling the fence(s) (unexpected)
     */
    c2_status_t fire();

    /**
     * Trigger this event from the merging of the supplied fences. This means that it will be
     * abandoned if any of these fences have been abandoned, and it will be fired if all of these
     * fences have been signaled.
     *
     * \retval C2_OK            the merging was successfully done
     * \retval C2_NO_MEMORY     not enough memory to perform the merging
     * \retval C2_DUPLICATE     the fence have already been merged (caller error)
     * \retval C2_BAD_STATE     the fence have already been signaled or abandoned (caller error)
     * \retval C2_REFUSED       no permission to merge the fence (unexpected - system)
     * \retval C2_CORRUPTED     some unknown error prevented merging the fence(s) (unexpected)
     */
    c2_status_t merge(std::vector<C2Fence> fences);

    /**
     * Abandons the event and any associated fence(s).
     * \note Call this to explicitly abandon an event before it is destructed to avoid a warning.
     *
     * This has no effect no effect if the event was already signaled or abandoned.
     *
     * \retval C2_OK            the fence(s) were successfully signaled
     * \retval C2_BAD_STATE     the fence(s) have already been signaled or merged (caller error)
     * \retval C2_DUPLICATE     the fence(s) have already been abandoned (caller error)
     * \retval C2_REFUSED       no permission to abandon the fence (unexpected - system)
     * \retval C2_CORRUPTED     some unknown error prevented signaling the fence(s) (unexpected)
     */
    c2_status_t abandon();

private:
    class Impl;
    std::shared_ptr<Impl> mImpl;
};

/// \addtogroup buf_internal Internal
/// @{

/**
 * Interface for objects that encapsulate an updatable status value.
 */
struct _C2InnateStatus {
    inline c2_status_t status() const { return mStatus; }

protected:
    _C2InnateStatus(c2_status_t status) : mStatus(status) { }

    c2_status_t mStatus; // this status is updatable by the object
};

/// @}

/**
 * This is a utility template for objects protected by an acquire fence, so that errors during
 * acquiring the object are propagated to the object itself.
 */
template<typename T>
class C2Acquirable : public C2Fence {
public:
    /**
     * Acquires the object protected by an acquire fence. Any errors during the mapping will be
     * passed to the object.
     *
     * \return acquired object potentially invalidated if waiting for the fence failed.
     */
    T get() {
        // TODO:
        // wait();
        return mT;
    }

protected:
    C2Acquirable(c2_status_t error, C2Fence fence, T t) : C2Fence(fence), mInitialError(error), mT(t) { }

private:
    c2_status_t mInitialError;
    T mT; // TODO: move instead of copy
};

/// @}

/// \defgroup linear Linear Data Blocks
/// @{

/**************************************************************************************************
  LINEAR ASPECTS, BLOCKS AND VIEWS
**************************************************************************************************/

/**
 * Basic segment math support.
 */
struct C2Segment {
    uint32_t offset;
    uint32_t size;

    inline constexpr C2Segment(uint32_t offset_, uint32_t size_)
        : offset(offset_),
          size(size_) {
    }

    inline constexpr bool isEmpty() const {
        return size == 0;
    }

    inline constexpr bool isValid() const {
        return offset <= ~size;
    }

    inline constexpr operator bool() const {
        return isValid() && !isEmpty();
    }

    inline constexpr bool operator!() const {
        return !bool(*this);
    }

    C2_ALLOW_OVERFLOW
    inline constexpr bool contains(const C2Segment &other) const {
        if (!isValid() || !other.isValid()) {
            return false;
        } else {
            return offset <= other.offset
                    && offset + size >= other.offset + other.size;
        }
    }

    inline constexpr bool operator==(const C2Segment &other) const {
        if (!isValid()) {
            return !other.isValid();
        } else {
            return offset == other.offset && size == other.size;
        }
    }

    inline constexpr bool operator!=(const C2Segment &other) const {
        return !operator==(other);
    }

    inline constexpr bool operator>=(const C2Segment &other) const {
        return contains(other);
    }

    inline constexpr bool operator>(const C2Segment &other) const {
        return contains(other) && !operator==(other);
    }

    inline constexpr bool operator<=(const C2Segment &other) const {
        return other.contains(*this);
    }

    inline constexpr bool operator<(const C2Segment &other) const {
        return other.contains(*this) && !operator==(other);
    }

    C2_ALLOW_OVERFLOW
    inline constexpr uint32_t end() const {
        return offset + size;
    }

    C2_ALLOW_OVERFLOW
    inline constexpr C2Segment intersect(const C2Segment &other) const {
        return C2Segment(c2_max(offset, other.offset),
                         c2_min(end(), other.end()) - c2_max(offset, other.offset));
    }

    /** clamps end to offset if it overflows */
    inline constexpr C2Segment normalize() const {
        return C2Segment(offset, c2_max(offset, end()) - offset);
    }

    /** clamps end to max if it overflows */
    inline constexpr C2Segment saturate() const {
        return C2Segment(offset, c2_min(size, ~offset));
    }

};

/**
 * Common aspect for all objects that have a linear capacity.
 */
class _C2LinearCapacityAspect {
/// \name Linear capacity interface
/// @{
public:
    inline constexpr uint32_t capacity() const { return mCapacity; }

    inline constexpr operator C2Segment() const {
        return C2Segment(0, mCapacity);
    }

protected:

#if UINTPTR_MAX == 0xffffffff
    static_assert(sizeof(size_t) == sizeof(uint32_t), "size_t is too big");
#else
    static_assert(sizeof(size_t) > sizeof(uint32_t), "size_t is too small");
    // explicitly disable construction from size_t
    inline explicit _C2LinearCapacityAspect(size_t capacity) = delete;
#endif

    inline explicit constexpr _C2LinearCapacityAspect(uint32_t capacity)
      : mCapacity(capacity) { }

    inline explicit constexpr _C2LinearCapacityAspect(const _C2LinearCapacityAspect *parent)
        : mCapacity(parent == nullptr ? 0 : parent->capacity()) { }

private:
    uint32_t mCapacity;
/// @}
};

/**
 * Aspect for objects that have a linear range inside a linear capacity.
 *
 * This class is copiable.
 */
class _C2LinearRangeAspect : public _C2LinearCapacityAspect {
/// \name Linear range interface
/// @{
public:
    inline constexpr uint32_t offset() const { return mOffset; }
    inline constexpr uint32_t endOffset() const { return mOffset + mSize; }
    inline constexpr uint32_t size() const { return mSize; }

    inline constexpr operator C2Segment() const {
        return C2Segment(mOffset, mSize);
    }

private:
    // subrange of capacity [0, capacity] & [size, size + offset]
    inline constexpr _C2LinearRangeAspect(uint32_t capacity_, size_t offset, size_t size)
        : _C2LinearCapacityAspect(capacity_),
          mOffset(c2_min(offset, capacity())),
          mSize(c2_min(size, capacity() - mOffset)) {
    }

protected:
    // copy constructor (no error check)
    inline constexpr _C2LinearRangeAspect(const _C2LinearRangeAspect &other)
        : _C2LinearCapacityAspect(other.capacity()),
          mOffset(other.offset()),
          mSize(other.size()) {
    }

    // parent capacity range [0, capacity]
    inline constexpr explicit _C2LinearRangeAspect(const _C2LinearCapacityAspect *parent)
        : _C2LinearCapacityAspect(parent),
          mOffset(0),
          mSize(capacity()) {
    }

    // subrange of parent capacity [0, capacity] & [size, size + offset]
    inline constexpr _C2LinearRangeAspect(const _C2LinearCapacityAspect *parent, size_t offset, size_t size)
        : _C2LinearCapacityAspect(parent),
          mOffset(c2_min(offset, capacity())),
          mSize(c2_min(size, capacity() - mOffset)) {
    }

    // subsection of the parent's and [offset, offset + size] ranges
    inline constexpr _C2LinearRangeAspect(const _C2LinearRangeAspect *parent, size_t offset, size_t size)
        : _C2LinearCapacityAspect(parent),
          mOffset(c2_min(c2_max(offset, parent == nullptr ? 0 : parent->offset()), capacity())),
          mSize(std::min(c2_min(size, parent == nullptr ? 0 : parent->size()), capacity() - mOffset)) {
    }

public:
    inline constexpr _C2LinearRangeAspect childRange(size_t offset, size_t size) const {
        return _C2LinearRangeAspect(
            mSize,
            c2_min(c2_max(offset, mOffset), capacity()) - mOffset,
            c2_min(c2_min(size, mSize), capacity() - c2_min(c2_max(offset, mOffset), capacity())));
    }

    friend class _C2EditableLinearRangeAspect;
    // invariants 0 <= mOffset <= mOffset + mSize <= capacity()
    uint32_t mOffset;
    uint32_t mSize;
/// @}
};

/**
 * Utility class for safe range calculations using size_t-s.
 */
class C2LinearRange : public _C2LinearRangeAspect {
public:
    inline constexpr C2LinearRange(const _C2LinearCapacityAspect &parent, size_t offset, size_t size)
        : _C2LinearRangeAspect(&parent, offset, size) { }

    inline constexpr C2LinearRange(const _C2LinearRangeAspect &parent, size_t offset, size_t size)
        : _C2LinearRangeAspect(&parent, offset, size) { }

    inline constexpr C2LinearRange intersect(size_t offset, size_t size) const {
        return C2LinearRange(*this, offset, size);
    }
};

/**
 * Utility class for simple and safe capacity and range construction.
 */
class C2LinearCapacity : public _C2LinearCapacityAspect {
public:
    inline constexpr explicit C2LinearCapacity(size_t capacity)
        : _C2LinearCapacityAspect(c2_min(capacity, std::numeric_limits<uint32_t>::max())) { }

    inline constexpr C2LinearRange range(size_t offset, size_t size) const {
        return C2LinearRange(*this, offset, size);
    }
};

/**
 * Aspect for objects that have an editable linear range.
 *
 * This class is copiable.
 */
class _C2EditableLinearRangeAspect : public _C2LinearRangeAspect {
    using _C2LinearRangeAspect::_C2LinearRangeAspect;

public:
/// \name Editable linear range interface
/// @{

    /**
     * Sets the offset to |offset|, while trying to keep the end of the buffer unchanged (e.g.
     * size will grow if offset is decreased, and may shrink if offset is increased.) Returns
     * true if successful, which is equivalent to if 0 <= |offset| <= capacity().
     *
     * Note: setting offset and size will yield different result depending on the order of the
     * operations. Always set offset first to ensure proper size.
     */
    inline bool setOffset(uint32_t offset) {
        if (offset > capacity()) {
            return false;
        }

        if (offset > mOffset + mSize) {
            mSize = 0;
        } else {
            mSize = mOffset + mSize - offset;
        }
        mOffset = offset;
        return true;
    }

    /**
     * Sets the size to |size|. Returns true if successful, which is equivalent to
     * if 0 <= |size| <= capacity() - offset().
     *
     * Note: setting offset and size will yield different result depending on the order of the
     * operations. Always set offset first to ensure proper size.
     */
    inline bool setSize(uint32_t size) {
        if (size > capacity() - mOffset) {
            return false;
        } else {
            mSize = size;
            return true;
        }
    }

    /**
     * Sets the offset to |offset| with best effort. Same as setOffset() except that offset will
     * be clamped to the buffer capacity.
     *
     * Note: setting offset and size (even using best effort) will yield different result depending
     * on the order of the operations. Always set offset first to ensure proper size.
     */
    inline void setOffset_be(uint32_t offset) {
        (void)setOffset(c2_min(offset, capacity()));
    }

    /**
     * Sets the size to |size| with best effort. Same as setSize() except that the selected region
     * will be clamped to the buffer capacity (e.g. size is clamped to [0, capacity() - offset()]).
     *
     * Note: setting offset and size (even using best effort) will yield different result depending
     * on the order of the operations. Always set offset first to ensure proper size.
     */
    inline void setSize_be(uint32_t size) {
        mSize = c2_min(size, capacity() - mOffset);
    }
/// @}
};

/**************************************************************************************************
  ALLOCATIONS
**************************************************************************************************/

/// \ingroup allocator Allocation and memory placement
/// @{

class C2LinearAllocation;
class C2GraphicAllocation;

/**
 *  Allocators are used by the framework to allocate memory (allocations) for buffers. They can
 *  support either 1D or 2D allocations.
 *
 *  \note In theory they could support both, but in practice, we will use only one or the other.
 *
 *  Never constructed on stack.
 *
 *  Allocators are provided by vendors.
 */
class C2Allocator {
public:
    /**
     * Allocator ID type.
     */
    typedef uint32_t id_t;
    enum : id_t {
        BAD_ID = 0xBADD, // invalid allocator ID
    };

    /**
     * Allocation types. This is a bitmask and is used in C2Allocator::Info
     * to list the supported allocation types of an allocator.
     */
    enum type_t : uint32_t {
        LINEAR  = 1 << 0, //
        GRAPHIC = 1 << 1,
    };

    /**
     * Information about an allocator.
     *
     * Allocators don't have a query API so all queriable information is stored here.
     */
    struct Traits {
        C2String name;              ///< allocator name
        id_t id;                    ///< allocator ID
        type_t supportedTypes;      ///< supported allocation types
        C2MemoryUsage minimumUsage; ///< usage that is minimally required for allocations
        C2MemoryUsage maximumUsage; ///< usage that is maximally allowed for allocations
    };

    /**
     * Returns the unique name of this allocator.
     *
     * This method MUST be "non-blocking" and return within 1ms.
     *
     * \return the name of this allocator.
     * \retval an empty string if there was not enough memory to allocate the actual name.
     */
    virtual C2String getName() const = 0;

    /**
     * Returns a unique ID for this allocator. This ID is used to get this allocator from the
     * allocator store, and to identify this allocator across all processes.
     *
     * This method MUST be "non-blocking" and return within 1ms.
     *
     * \return a unique ID for this allocator.
     */
    virtual id_t getId() const = 0;

    /**
     * Returns the allocator traits.
     *
     * This method MUST be "non-blocking" and return within 1ms.
     *
     * Allocators don't have a full-fledged query API, only this method.
     *
     * \return allocator information
     */
    virtual std::shared_ptr<const Traits> getTraits() const = 0;

    /**
     * Allocates a 1D allocation of given |capacity| and |usage|. If successful, the allocation is
     * stored in |allocation|. Otherwise, |allocation| is set to 'nullptr'.
     *
     * \param capacity      the size of requested allocation (the allocation could be slightly
     *                      larger, e.g. to account for any system-required alignment)
     * \param usage         the memory usage info for the requested allocation. \note that the
     *                      returned allocation may be later used/mapped with different usage.
     *                      The allocator should layout the buffer to be optimized for this usage,
     *                      but must support any usage. One exception: protected buffers can
     *                      only be used in a protected scenario.
     * \param allocation    pointer to where the allocation shall be stored on success. nullptr
     *                      will be stored here on failure
     *
     * \retval C2_OK        the allocation was successful
     * \retval C2_NO_MEMORY not enough memory to complete the allocation
     * \retval C2_TIMED_OUT the allocation timed out
     * \retval C2_REFUSED   no permission to complete the allocation
     * \retval C2_BAD_VALUE capacity or usage are not supported (invalid) (caller error)
     * \retval C2_OMITTED   this allocator does not support 1D allocations
     * \retval C2_CORRUPTED some unknown, unrecoverable error occured during allocation (unexpected)
     */
    virtual c2_status_t newLinearAllocation(
            uint32_t capacity __unused, C2MemoryUsage usage __unused,
            std::shared_ptr<C2LinearAllocation> *allocation /* nonnull */) {
        *allocation = nullptr;
        return C2_OMITTED;
    }

    /**
     * (Re)creates a 1D allocation from a native |handle|. If successful, the allocation is stored
     * in |allocation|. Otherwise, |allocation| is set to 'nullptr'.
     *
     * \param handle      the handle for the existing allocation. On success, the allocation will
     *                    take ownership of |handle|.
     * \param allocation  pointer to where the allocation shall be stored on success. nullptr
     *                    will be stored here on failure
     *
     * \retval C2_OK        the allocation was recreated successfully
     * \retval C2_NO_MEMORY not enough memory to recreate the allocation
     * \retval C2_TIMED_OUT the recreation timed out (unexpected)
     * \retval C2_REFUSED   no permission to recreate the allocation
     * \retval C2_BAD_VALUE invalid handle (caller error)
     * \retval C2_OMITTED   this allocator does not support 1D allocations
     * \retval C2_CORRUPTED some unknown, unrecoverable error occured during allocation (unexpected)
     */
    virtual c2_status_t priorLinearAllocation(
            const C2Handle *handle __unused,
            std::shared_ptr<C2LinearAllocation> *allocation /* nonnull */) {
        *allocation = nullptr;
        return C2_OMITTED;
    }

    /**
     * Allocates a 2D allocation of given |width|, |height|, |format| and |usage|. If successful,
     * the allocation is stored in |allocation|. Otherwise, |allocation| is set to 'nullptr'.
     *
     * \param width         the width of requested allocation (the allocation could be slightly
     *                      larger, e.g. to account for any system-required alignment)
     * \param height        the height of requested allocation (the allocation could be slightly
     *                      larger, e.g. to account for any system-required alignment)
     * \param format        the pixel format of requested allocation. This could be a vendor
     *                      specific format.
     * \param usage         the memory usage info for the requested allocation. \note that the
     *                      returned allocation may be later used/mapped with different usage.
     *                      The allocator should layout the buffer to be optimized for this usage,
     *                      but must support any usage. One exception: protected buffers can
     *                      only be used in a protected scenario.
     * \param allocation    pointer to where the allocation shall be stored on success. nullptr
     *                      will be stored here on failure
     *
     * \retval C2_OK        the allocation was successful
     * \retval C2_NO_MEMORY not enough memory to complete the allocation
     * \retval C2_TIMED_OUT the allocation timed out
     * \retval C2_REFUSED   no permission to complete the allocation
     * \retval C2_BAD_VALUE width, height, format or usage are not supported (invalid) (caller error)
     * \retval C2_OMITTED   this allocator does not support 2D allocations
     * \retval C2_CORRUPTED some unknown, unrecoverable error occured during allocation (unexpected)
     */
    virtual c2_status_t newGraphicAllocation(
            uint32_t width __unused, uint32_t height __unused, uint32_t format __unused,
            C2MemoryUsage usage __unused,
            std::shared_ptr<C2GraphicAllocation> *allocation /* nonnull */) {
        *allocation = nullptr;
        return C2_OMITTED;
    }

    /**
     * (Re)creates a 2D allocation from a native handle.  If successful, the allocation is stored
     * in |allocation|. Otherwise, |allocation| is set to 'nullptr'.
     *
     * \param handle      the handle for the existing allocation. On success, the allocation will
     *                    take ownership of |handle|.
     * \param allocation  pointer to where the allocation shall be stored on success. nullptr
     *                    will be stored here on failure
     *
     * \retval C2_OK        the allocation was recreated successfully
     * \retval C2_NO_MEMORY not enough memory to recreate the allocation
     * \retval C2_TIMED_OUT the recreation timed out (unexpected)
     * \retval C2_REFUSED   no permission to recreate the allocation
     * \retval C2_BAD_VALUE invalid handle (caller error)
     * \retval C2_OMITTED   this allocator does not support 2D allocations
     * \retval C2_CORRUPTED some unknown, unrecoverable error occured during recreation (unexpected)
     */
    virtual c2_status_t priorGraphicAllocation(
            const C2Handle *handle __unused,
            std::shared_ptr<C2GraphicAllocation> *allocation /* nonnull */) {
        *allocation = nullptr;
        return C2_OMITTED;
    }

    virtual ~C2Allocator() = default;
protected:
    C2Allocator() = default;
};

/**
 * \ingroup linear allocator
 * 1D allocation interface.
 */
class C2LinearAllocation : public _C2LinearCapacityAspect {
public:
    /**
     * Maps a portion of an allocation starting from |offset| with |size| into local process memory.
     * Stores the starting address into |addr|, or NULL if the operation was unsuccessful.
     * |fence| will contain an acquire sync fence object. If it is already
     * safe to access the buffer contents, then it will contain an empty (already fired) fence.
     *
     * \param offset        starting position of the portion to be mapped (this does not have to
     *                      be page aligned)
     * \param size          size of the portion to be mapped (this does not have to be page
     *                      aligned)
     * \param usage         the desired usage. \todo this must be kSoftwareRead and/or
     *                      kSoftwareWrite.
     * \param fence         a pointer to a fence object if an async mapping is requested. If
     *                      not-null, and acquire fence will be stored here on success, or empty
     *                      fence on failure. If null, the mapping will be synchronous.
     * \param addr          a pointer to where the starting address of the mapped portion will be
     *                      stored. On failure, nullptr will be stored here.
     *
     * \todo Only one portion can be mapped at the same time - this is true for gralloc, but there
     *       is no need for this for 1D buffers.
     * \todo Do we need to support sync operation as we could just wait for the fence?
     *
     * \retval C2_OK        the operation was successful
     * \retval C2_REFUSED   no permission to map the portion
     * \retval C2_TIMED_OUT the operation timed out
     * \retval C2_DUPLICATE if the allocation is already mapped.
     * \retval C2_NO_MEMORY not enough memory to complete the operation
     * \retval C2_BAD_VALUE the parameters (offset/size) are invalid or outside the allocation, or
     *                      the usage flags are invalid (caller error)
     * \retval C2_CORRUPTED some unknown error prevented the operation from completing (unexpected)
     */
    virtual c2_status_t map(
            size_t offset, size_t size, C2MemoryUsage usage, C2Fence *fence /* nullable */,
            void **addr /* nonnull */) = 0;

    /**
     * Unmaps a portion of an allocation at |addr| with |size|. These must be parameters previously
     * passed to and returned by |map|; otherwise, this operation is a no-op.
     *
     * \param addr          starting address of the mapped region
     * \param size          size of the mapped region
     * \param fence         a pointer to a fence object if an async unmapping is requested. If
     *                      not-null, a release fence will be stored here on success, or empty fence
     *                      on failure. This fence signals when the original allocation contains
     *                      all changes that happened to the mapped region. If null, the unmapping
     *                      will be synchronous.
     *
     * \retval C2_OK        the operation was successful
     * \retval C2_TIMED_OUT the operation timed out
     * \retval C2_NOT_FOUND if the allocation was not mapped previously.
     * \retval C2_BAD_VALUE the parameters (addr/size) do not correspond to previously mapped
     *                      regions (caller error)
     * \retval C2_CORRUPTED some unknown error prevented the operation from completing (unexpected)
     * \retval C2_REFUSED   no permission to unmap the portion (unexpected - system)
     */
    virtual c2_status_t unmap(void *addr, size_t size, C2Fence *fence /* nullable */) = 0;

    /**
     * Returns the allocator ID for this allocation. This is useful to put the handle into context.
     */
    virtual C2Allocator::id_t getAllocatorId() const = 0;

    /**
     * Returns a pointer to the allocation handle.
     */
    virtual const C2Handle *handle() const = 0;

    /**
     * Returns true if this is the same allocation as |other|.
     */
    virtual bool equals(const std::shared_ptr<C2LinearAllocation> &other) const = 0;

protected:
    // \todo should we limit allocation directly?
    C2LinearAllocation(size_t capacity) : _C2LinearCapacityAspect(c2_min(capacity, UINT32_MAX)) {}
    virtual ~C2LinearAllocation() = default;
};

class C2CircularBlock;
class C2LinearBlock;
class C2GraphicBlock;

/**
 *  Block pools are used by components to obtain output buffers in an efficient way. They can
 *  support either linear (1D), circular (1D) or graphic (2D) blocks.
 *
 *  Block pools decouple the recycling of memory/allocations from the components. They are meant to
 *  be an opaque service (there are no public APIs other than obtaining blocks) provided by the
 *  platform. Block pools are also meant to decouple allocations from memory used by buffers. This
 *  is accomplished by allowing pools to allot multiple memory 'blocks' on a single allocation. As
 *  their name suggest, block pools maintain a pool of memory blocks. When a component asks for
 *  a memory block, pools will try to return a free memory block already in the pool. If no such
 *  block exists, they will allocate memory using the backing allocator and allot a block on that
 *  allocation. When blocks are no longer used in the system, they are recycled back to the block
 *  pool and are available as free blocks.
 *
 *  Never constructed on stack.
 */
class C2BlockPool {
public:
    /**
     * Block pool ID type.
     */
    typedef uint64_t local_id_t;

    enum : local_id_t {
        BASIC_LINEAR = 0,  ///< ID of basic (unoptimized) block pool for fetching 1D blocks
        BASIC_GRAPHIC = 1, ///< ID of basic (unoptimized) block pool for fetching 2D blocks
        PLATFORM_START = 0x10,
    };

    /**
     * Returns the ID for this block pool. This ID is used to get this block pool from the platform.
     * It is only valid in the current process.
     *
     * This method MUST be "non-blocking" and return within 1ms.
     *
     * \return a local ID for this block pool.
     */
    virtual local_id_t getLocalId() const = 0;

    /**
     * Returns the ID of the backing allocator of this block pool.
     *
     * This method MUST be "non-blocking" and return within 1ms.
     *
     * \return the ID of the backing allocator of this block pool.
     */
    virtual C2Allocator::id_t getAllocatorId() const = 0;

    /**
     * Obtains a linear writeable block of given |capacity| and |usage|. If successful, the
     * block is stored in |block|. Otherwise, |block| is set to 'nullptr'.
     *
     * \param capacity the size of requested block.
     * \param usage    the memory usage info for the requested block. Returned blocks will be
     *                 optimized for this usage, but may be used with any usage. One exception:
     *                 protected blocks/buffers can only be used in a protected scenario.
     * \param block    pointer to where the obtained block shall be stored on success. nullptr will
     *                 be stored here on failure
     *
     * \retval C2_OK        the operation was successful
     * \retval C2_NO_MEMORY not enough memory to complete any required allocation
     * \retval C2_TIMED_OUT the operation timed out
     * \retval C2_BLOCKING  the operation is blocked
     * \retval C2_REFUSED   no permission to complete any required allocation
     * \retval C2_BAD_VALUE capacity or usage are not supported (invalid) (caller error)
     * \retval C2_OMITTED   this pool does not support linear blocks
     * \retval C2_CORRUPTED some unknown, unrecoverable error occured during operation (unexpected)
     */
    virtual c2_status_t fetchLinearBlock(
            uint32_t capacity __unused, C2MemoryUsage usage __unused,
            std::shared_ptr<C2LinearBlock> *block /* nonnull */) {
        *block = nullptr;
        return C2_OMITTED;
    }

    /**
     * Obtains a circular writeable block of given |capacity| and |usage|. If successful, the
     * block is stored in |block|. Otherwise, |block| is set to 'nullptr'.
     *
     * \param capacity the size of requested circular block. (note: the size of the obtained
     *                 block could be slightly larger, e.g. to accommodate any system-required
     *                 alignment)
     * \param usage    the memory usage info for the requested block. Returned blocks will be
     *                 optimized for this usage, but may be used with any usage. One exception:
     *                 protected blocks/buffers can only be used in a protected scenario.
     * \param block    pointer to where the obtained block shall be stored on success. nullptr
     *                 will be stored here on failure
     *
     * \retval C2_OK        the operation was successful
     * \retval C2_NO_MEMORY not enough memory to complete any required allocation
     * \retval C2_TIMED_OUT the operation timed out
     * \retval C2_BLOCKING  the operation is blocked
     * \retval C2_REFUSED   no permission to complete any required allocation
     * \retval C2_BAD_VALUE capacity or usage are not supported (invalid) (caller error)
     * \retval C2_OMITTED   this pool does not support circular blocks
     * \retval C2_CORRUPTED some unknown, unrecoverable error occured during operation (unexpected)
     */
    virtual c2_status_t fetchCircularBlock(
            uint32_t capacity __unused, C2MemoryUsage usage __unused,
            std::shared_ptr<C2CircularBlock> *block /* nonnull */) {
        *block = nullptr;
        return C2_OMITTED;
    }

    /**
     * Obtains a 2D graphic block of given |width|, |height|, |format| and |usage|. If successful,
     * the block is stored in |block|. Otherwise, |block| is set to 'nullptr'.
     *
     * \param width  the width of requested block (the obtained block could be slightly larger, e.g.
     *               to accommodate any system-required alignment)
     * \param height the height of requested block (the obtained block could be slightly larger,
     *               e.g. to accommodate any system-required alignment)
     * \param format the pixel format of requested block. This could be a vendor specific format.
     * \param usage  the memory usage info for the requested block. Returned blocks will be
     *               optimized for this usage, but may be used with any usage. One exception:
     *               protected blocks/buffers can only be used in a protected scenario.
     * \param block  pointer to where the obtained block shall be stored on success. nullptr
     *               will be stored here on failure
     *
     * \retval C2_OK        the operation was successful
     * \retval C2_NO_MEMORY not enough memory to complete any required allocation
     * \retval C2_TIMED_OUT the operation timed out
     * \retval C2_BLOCKING  the operation is blocked
     * \retval C2_REFUSED   no permission to complete any required allocation
     * \retval C2_BAD_VALUE width, height, format or usage are not supported (invalid) (caller
     *                      error)
     * \retval C2_OMITTED   this pool does not support 2D blocks
     * \retval C2_CORRUPTED some unknown, unrecoverable error occured during operation (unexpected)
     */
    virtual c2_status_t fetchGraphicBlock(
            uint32_t width __unused, uint32_t height __unused, uint32_t format __unused,
            C2MemoryUsage usage __unused,
            std::shared_ptr<C2GraphicBlock> *block /* nonnull */) {
        *block = nullptr;
        return C2_OMITTED;
    }

    virtual ~C2BlockPool() = default;
protected:
    C2BlockPool() = default;
};

/// @}

// ================================================================================================
//  BLOCKS
// ================================================================================================

/**
 * Blocks are sections of allocations. They can be either 1D or 2D.
 */

class C2LinearAllocation;

/**
 * A 1D block.
 *
 * \note capacity() is not meaningful for users of blocks; instead size() is the capacity of the
 * usable portion. Use and offset() and size() if accessing the block directly through its handle
 * to represent the allotted range of the underlying allocation to this block.
 */
class C2Block1D : public _C2LinearRangeAspect {
public:
    /**
     * Returns the underlying handle for this allocation.
     *
     * \note that the block and its block pool has shared ownership of the handle
     *       and if all references to the block are released, the underlying block
     *       allocation may get reused even if a client keeps a clone of this handle.
     */
    const C2Handle *handle() const;

    /**
     * Returns the allocator's ID that created the underlying allocation for this block. This
     * provides the context for understanding the handle.
     */
    C2Allocator::id_t getAllocatorId() const;

protected:
    class Impl;
    /** construct a block. */
    C2Block1D(std::shared_ptr<Impl> impl, const _C2LinearRangeAspect &range);

    friend struct _C2BlockFactory;
    std::shared_ptr<Impl> mImpl;
};

/**
 * Read view provides read-only access for a linear memory segment.
 *
 * This class is copiable.
 */
class C2ReadView : public _C2LinearCapacityAspect {
public:
    /**
     * \return pointer to the start of the block or nullptr on error.
     *         This pointer is only valid during the lifetime of this view or until it is released.
     */
    const uint8_t *data() const;

    /**
     * Returns a portion of this view.
     *
     * \param offset  the start offset of the portion. \note This is clamped to the capacity of this
     *              view.
     * \param size    the size of the portion. \note This is clamped to the remaining data from offset.
     *
     * \return a read view containing a portion of this view
     */
    C2ReadView subView(size_t offset, size_t size) const;

    /**
     * \return error during the creation/mapping of this view.
     */
    c2_status_t error() const;

    /**
     * Releases this view. This sets error to C2_NO_INIT.
     */
    //void release();

protected:
    class Impl;
    C2ReadView(std::shared_ptr<Impl> impl, uint32_t offset, uint32_t size);
    explicit C2ReadView(c2_status_t error);

private:
    friend struct _C2BlockFactory;
    std::shared_ptr<Impl> mImpl;
    uint32_t mOffset; /**< offset into the linear block backing this read view */
};

/**
 * Write view provides read/write access for a linear memory segment.
 *
 * This class is copiable. \todo movable only?
 */
class C2WriteView : public _C2EditableLinearRangeAspect {
public:
    /**
     * Start of the block.
     *
     * \return pointer to the start of the block or nullptr on error.
     *         This pointer is only valid during the lifetime of this view or until it is released.
     */
    uint8_t *base();

    /**
     * \return pointer to the block at the current offset or nullptr on error.
     *         This pointer is only valid during the lifetime of this view or until it is released.
     */
    uint8_t *data();

    /**
     * \return error during the creation/mapping of this view.
     */
    c2_status_t error() const;

    /**
     * Releases this view. This sets error to C2_NO_INIT.
     */
    //void release();

protected:
    class Impl;
    C2WriteView(std::shared_ptr<Impl> impl);
    explicit C2WriteView(c2_status_t error);

private:
    friend struct _C2BlockFactory;
    std::shared_ptr<Impl> mImpl;
};

/**
 * A constant (read-only) linear block (portion of an allocation) with an acquire fence.
 * Blocks are unmapped when created, and can be mapped into a read view on demand.
 *
 * This class is copiable and contains a reference to the allocation that it is based on.
 */
class C2ConstLinearBlock : public C2Block1D {
public:
    /**
     * Maps this block into memory and returns a read view for it.
     *
     * \return a read view for this block.
     */
    C2Acquirable<C2ReadView> map() const;

    /**
     * Returns a portion of this block.
     *
     * \param offset  the start offset of the portion. \note This is clamped to the capacity of this
     *              block.
     * \param size    the size of the portion. \note This is clamped to the remaining data from offset.
     *
     * \return a constant linear block containing a portion of this block
     */
    C2ConstLinearBlock subBlock(size_t offset, size_t size) const;

    /**
     * Returns the acquire fence for this block.
     *
     * \return a fence that must be waited on before reading the block.
     */
    C2Fence fence() const { return mFence; }

protected:
    C2ConstLinearBlock(std::shared_ptr<Impl> impl, const _C2LinearRangeAspect &range, C2Fence mFence);

private:
    friend struct _C2BlockFactory;
    C2Fence mFence;
};

/**
 * Linear block is a writeable 1D block. Once written, it can be shared in whole or in parts with
 * consumers/readers as read-only const linear block(s).
 */
class C2LinearBlock : public C2Block1D {
public:
    /**
     * Maps this block into memory and returns a write view for it.
     *
     * \return a write view for this block.
     */
    C2Acquirable<C2WriteView> map();

    /**
     * Creates a read-only const linear block for a portion of this block; optionally protected
     * by an acquire fence. There are two ways to use this:
     *
     * 1) share ready block after writing data into the block. In this case no fence shall be
     *    supplied, and the block shall not be modified after calling this method.
     * 2) share block metadata before actually (finishing) writing the data into the block. In
     *    this case a fence must be supplied that will be triggered when the data is written.
     *    The block shall be modified only until firing the event for the fence.
     */
    C2ConstLinearBlock share(size_t offset, size_t size, C2Fence fence);

protected:
    C2LinearBlock(std::shared_ptr<Impl> impl, const _C2LinearRangeAspect &range);

    friend struct _C2BlockFactory;
};

/// @}

/**************************************************************************************************
  CIRCULAR BLOCKS AND VIEWS
**************************************************************************************************/

/// \defgroup circular Circular buffer support
/// @{

/**
 * Circular blocks can be used to share data between a writer and a reader (and/or other consumers)-
 * in a memory-efficient way by reusing a section of memory. Circular blocks are a bit more complex
 * than single reader/single writer schemes to facilitate block-based consuming of data.
 *
 * They can operate in two modes:
 *
 * 1) one writer that creates blocks to be consumed (this model can be used by components)
 *
 * 2) one writer that writes continuously, and one reader that can creates blocks to be consumed
 *    by further recipients (this model is used by the framework, and cannot be used by components.)
 *
 * Circular blocks have four segments with running pointers:
 *  - reserved: data reserved and available for the writer
 *  - committed: data committed by the writer and available to the reader (if present)
 *  - used: data used by consumers (if present)
 *  - available: unused data available to be reserved
 */
class C2CircularBlock : public C2Block1D {
    // TODO: add methods

private:
    size_t mReserved __unused;   // end of reserved section
    size_t mCommitted __unused;  // end of committed section
    size_t mUsed __unused;       // end of used section
    size_t mFree __unused;       // end of free section
};

class _C2CircularBlockSegment : public _C2LinearCapacityAspect {
public:
    /**
     * Returns the available size for this segment.
     *
     * \return currently available size for this segment
     */
    size_t available() const;

    /**
     * Reserve some space for this segment from its current start.
     *
     * \param size    desired space in bytes
     * \param fence   a pointer to an acquire fence. If non-null, the reservation is asynchronous and
     *              a fence will be stored here that will be signaled when the reservation is
     *              complete. If null, the reservation is synchronous.
     *
     * \retval C2_OK            the space was successfully reserved
     * \retval C2_NO_MEMORY     the space requested cannot be reserved
     * \retval C2_TIMED_OUT     the reservation timed out \todo when?
     * \retval C2_CORRUPTED     some unknown error prevented reserving space. (unexpected)
     */
    c2_status_t reserve(size_t size, C2Fence *fence /* nullable */);

    /**
     * Abandons a portion of this segment. This will move to the beginning of this segment.
     *
     * \note This methods is only allowed if this segment is producing blocks.
     *
     * \param size    number of bytes to abandon
     *
     * \retval C2_OK            the data was successfully abandoned
     * \retval C2_TIMED_OUT     the operation timed out (unexpected)
     * \retval C2_CORRUPTED     some unknown error prevented abandoning the data (unexpected)
     */
    c2_status_t abandon(size_t size);

    /**
     * Share a portion as block(s) with consumers (these are moved to the used section).
     *
     * \note This methods is only allowed if this segment is producing blocks.
     * \note Share does not move the beginning of the segment. (\todo add abandon/offset?)
     *
     * \param size    number of bytes to share
     * \param fence   fence to be used for the section
     * \param blocks  vector where the blocks of the section are appended to
     *
     * \retval C2_OK            the portion was successfully shared
     * \retval C2_NO_MEMORY     not enough memory to share the portion
     * \retval C2_TIMED_OUT     the operation timed out (unexpected)
     * \retval C2_CORRUPTED     some unknown error prevented sharing the data (unexpected)
     */
    c2_status_t share(size_t size, C2Fence fence, std::vector<C2ConstLinearBlock> &blocks);

    /**
     * Returns the beginning offset of this segment from the start of this circular block.
     *
     * @return beginning offset
     */
    size_t begin();

    /**
     * Returns the end offset of this segment from the start of this circular block.
     *
     * @return end offset
     */
    size_t end();
};

/**
 * A circular write-view is a dynamic mapped view for a segment of a circular block. Care must be
 * taken when using this view so that only the section owned by the segment is modified.
 */
class C2CircularWriteView : public _C2LinearCapacityAspect {
public:
    /**
     * Start of the circular block.
     * \note the segment does not own this pointer.
     *
     * \return pointer to the start of the circular block or nullptr on error.
     */
    uint8_t *base();

    /**
     * \return error during the creation/mapping of this view.
     */
    c2_status_t error() const;
};

/**
 * The writer of a circular buffer.
 *
 * Can commit data to a reader (not supported for components) OR share data blocks directly with a
 * consumer.
 *
 * If a component supports outputting data into circular buffers, it must allocate a circular
 * block and use a circular writer.
 */
class C2CircularWriter : public _C2CircularBlockSegment {
public:
    /**
     * Commits a portion of this segment to the next segment. This moves the beginning of the
     * segment.
     *
     * \param size    number of bytes to commit to the next segment
     * \param fence   fence used for the commit (the fence must signal before the data is committed)
     */
    c2_status_t commit(size_t size, C2Fence fence);

    /**
     * Maps this block into memory and returns a write view for it.
     *
     * \return a write view for this block.
     */
    C2Acquirable<C2CircularWriteView> map();
};

/// @}

/// \defgroup graphic Graphic Data Blocks
/// @{

/**
 * C2Rect: rectangle type with non-negative coordinates.
 *
 * \note This struct has public fields without getters/setters. All methods are inline.
 */
struct C2Rect {
// public:
    uint32_t width;
    uint32_t height;
    uint32_t left;
    uint32_t top;

    constexpr inline C2Rect()
        : C2Rect(0, 0, 0, 0) { }

    constexpr inline C2Rect(uint32_t width_, uint32_t height_)
        : C2Rect(width_, height_, 0, 0) { }

    constexpr C2Rect inline at(uint32_t left_, uint32_t top_) const {
        return C2Rect(width, height, left_, top_);
    }

    // utility methods

    inline constexpr bool isEmpty() const {
        return width == 0 || height == 0;
    }

    inline constexpr bool isValid() const {
        return left <= ~width && top <= ~height;
    }

    inline constexpr operator bool() const {
        return isValid() && !isEmpty();
    }

    inline constexpr bool operator!() const {
        return !bool(*this);
    }

    C2_ALLOW_OVERFLOW
    inline constexpr bool contains(const C2Rect &other) const {
        if (!isValid() || !other.isValid()) {
            return false;
        } else {
            return left <= other.left && top <= other.top
                    && left + width >= other.left + other.width
                    && top + height >= other.top + other.height;
        }
    }

    inline constexpr bool operator==(const C2Rect &other) const {
        if (!isValid()) {
            return !other.isValid();
        } else {
            return left == other.left && top == other.top
                    && width == other.width && height == other.height;
        }
    }

    inline constexpr bool operator!=(const C2Rect &other) const {
        return !operator==(other);
    }

    inline constexpr bool operator>=(const C2Rect &other) const {
        return contains(other);
    }

    inline constexpr bool operator>(const C2Rect &other) const {
        return contains(other) && !operator==(other);
    }

    inline constexpr bool operator<=(const C2Rect &other) const {
        return other.contains(*this);
    }

    inline constexpr bool operator<(const C2Rect &other) const {
        return other.contains(*this) && !operator==(other);
    }

    C2_ALLOW_OVERFLOW
    inline constexpr uint32_t right() const {
        return left + width;
    }

    C2_ALLOW_OVERFLOW
    inline constexpr uint32_t bottom() const {
        return top + height;
    }

    C2_ALLOW_OVERFLOW
    inline constexpr C2Rect intersect(const C2Rect &other) const {
        return C2Rect(c2_min(right(), other.right()) - c2_max(left, other.left),
                      c2_min(bottom(), other.bottom()) - c2_max(top, other.top),
                      c2_max(left, other.left),
                      c2_max(top, other.top));
    }

    /** clamps right and bottom to top, left if they overflow */
    inline constexpr C2Rect normalize() const {
        return C2Rect(c2_max(left, right()) - left, c2_max(top, bottom()) - top, left, top);
    }

private:
    /// note: potentially unusual argument order
    constexpr inline C2Rect(uint32_t width_, uint32_t height_, uint32_t left_, uint32_t top_)
        : width(width_),
          height(height_),
          left(left_),
          top(top_) { }
};

/**
 * Interface for objects that have a width and height (planar capacity).
 */
class _C2PlanarCapacityAspect {
/// \name Planar capacity interface
/// @{
public:
    inline constexpr uint32_t width() const { return _mWidth; }
    inline constexpr uint32_t height() const { return _mHeight; }

    inline constexpr operator C2Rect() const {
        return C2Rect(_mWidth, _mHeight);
    }

protected:
    inline constexpr _C2PlanarCapacityAspect(uint32_t width, uint32_t height)
      : _mWidth(width), _mHeight(height) { }

    inline explicit constexpr _C2PlanarCapacityAspect(const _C2PlanarCapacityAspect *parent)
        : _mWidth(parent == nullptr ? 0 : parent->width()),
          _mHeight(parent == nullptr ? 0 : parent->height()) { }

private:
    uint32_t _mWidth;
    uint32_t _mHeight;
/// @}
};

/**
 * C2PlaneInfo: information on the layout of a singe flexible plane.
 *
 * Public fields without getters/setters.
 */
struct C2PlaneInfo {
//public:
    enum channel_t : uint32_t {
        CHANNEL_Y,  ///< luma
        CHANNEL_R,  ///< red
        CHANNEL_G,  ///< green
        CHANNEL_B,  ///< blue
        CHANNEL_A,  ///< alpha
        CHANNEL_CR, ///< Cr
        CHANNEL_CB, ///< Cb
    } channel;

    int32_t colInc;       ///< column increment in bytes. may be negative
    int32_t rowInc;       ///< row increment in bytes. may be negative

    uint32_t colSampling; ///< subsampling compared to width (must be a power of 2)
    uint32_t rowSampling; ///< subsampling compared to height (must be a power of 2)

    uint32_t allocatedDepth; ///< size of each sample (must be a multiple of 8)
    uint32_t bitDepth;       ///< significant bits per sample
    /**
     * the right shift of the significant bits in the sample. E.g. if a 10-bit significant
     * value is laid out in a 16-bit allocation aligned to LSB (values 0-1023), rightShift
     * would be 0 as the 16-bit value read from the sample does not need to be right shifted
     * and can be used as is (after applying a 10-bit mask of 0x3FF).
     *
     * +--------+--------+
     * |      VV|VVVVVVVV|
     * +--------+--------+
     *  15     8 7      0
     *
     * If the value is laid out aligned to MSB, rightShift would be 6, as the value read
     * from the allocated sample must be right-shifted by 6 to get the actual sample value.
     *
     * +--------+--------+
     * |VVVVVVVV|VV      |
     * +--------+--------+
     *  15     8 7      0
     */
    uint32_t rightShift;

    enum endianness_t : uint32_t {
        NATIVE,
        LITTLE_END, // LITTLE_ENDIAN is reserved macro
        BIG_END,    // BIG_ENDIAN is a reserved macro
    } endianness; ///< endianness of the samples

    /**
     * The following two fields define the relation between multiple planes. If multiple planes are
     * interleaved, they share a root plane (whichever plane's start address is the lowest), and
     * |offset| is the offset of this plane inside the root plane (in bytes). |rootIx| is the index
     * of the root plane. If a plane is independent, rootIx is its index and offset is 0.
     */
    uint32_t rootIx; ///< index of the root plane
    uint32_t offset; ///< offset of this plane inside of the root plane

    inline constexpr ssize_t minOffset(uint32_t width, uint32_t height) const {
        ssize_t offs = 0;
        if (width > 0 && colInc < 0) {
            offs += colInc * (ssize_t)(width - 1);
        }
        if (height > 0 && rowInc < 0) {
            offs += rowInc * (ssize_t)(height - 1);
        }
        return offs;
    }

    inline constexpr ssize_t maxOffset(uint32_t width, uint32_t height) const {
        ssize_t offs = (allocatedDepth + 7) >> 3;
        if (width > 0 && colInc > 0) {
            offs += colInc * (ssize_t)(width - 1);
        }
        if (height > 0 && rowInc > 0) {
            offs += rowInc * (ssize_t)(height - 1);
        }
        return offs;
    }
} C2_PACK;

struct C2PlanarLayout {
//public:
    enum type_t : uint32_t {
        TYPE_UNKNOWN = 0,
        TYPE_YUV = 0x100,   ///< YUV image with 3 planes
        TYPE_YUVA,          ///< YUVA image with 4 planes
        TYPE_RGB,           ///< RGB image with 3 planes
        TYPE_RGBA,          ///< RBGA image with 4 planes
    };

    type_t type;                    // image type
    uint32_t numPlanes;             // number of component planes
    uint32_t rootPlanes;            // number of layout planes (root planes)

    enum plane_index_t : uint32_t {
        PLANE_Y = 0,
        PLANE_U = 1,
        PLANE_V = 2,
        PLANE_R = 0,
        PLANE_G = 1,
        PLANE_B = 2,
        PLANE_A = 3,
        MAX_NUM_PLANES = 4,
    };

    C2PlaneInfo planes[MAX_NUM_PLANES];
};

/**
 * Aspect for objects that have a planar section (crop rectangle).
 *
 * This class is copiable.
 */
class _C2PlanarSectionAspect : public _C2PlanarCapacityAspect {
/// \name Planar section interface
/// @{
private:
    inline constexpr _C2PlanarSectionAspect(uint32_t width, uint32_t height, const C2Rect &crop)
        : _C2PlanarCapacityAspect(width, height),
          mCrop(C2Rect(std::min(width - std::min(crop.left, width), crop.width),
                       std::min(height - std::min(crop.top, height), crop.height)).at(
                               std::min(crop.left, width),
                               std::min(crop.height, height))) {
    }

public:
    // crop can be an empty rect, does not have to line up with subsampling
    // NOTE: we do not support floating-point crop
    inline constexpr C2Rect crop() const { return mCrop; }

    /**
     * Returns a child planar section for |crop|, where the capacity represents this section.
     */
    inline constexpr _C2PlanarSectionAspect childSection(const C2Rect &crop) const {
        return _C2PlanarSectionAspect(
                mCrop.width, mCrop.height,
                // crop and translate |crop| rect
                C2Rect(c2_min(mCrop.right() - c2_clamp(mCrop.left, crop.left, mCrop.right()),
                              crop.width),
                       c2_min(mCrop.bottom() - c2_clamp(mCrop.top, crop.top, mCrop.bottom()),
                              crop.height))
                .at(c2_clamp(mCrop.left, crop.left, mCrop.right()) - mCrop.left,
                    c2_clamp(mCrop.top, crop.top, mCrop.bottom()) - mCrop.top));
    }

protected:
    inline constexpr _C2PlanarSectionAspect(const _C2PlanarCapacityAspect *parent)
        : _C2PlanarCapacityAspect(parent), mCrop(width(), height()) {}

    inline constexpr _C2PlanarSectionAspect(const _C2PlanarCapacityAspect *parent, const C2Rect &crop)
        : _C2PlanarCapacityAspect(parent),
          mCrop(parent == nullptr ? C2Rect() : ((C2Rect)*parent).intersect(crop).normalize()) { }

    inline constexpr _C2PlanarSectionAspect(const _C2PlanarSectionAspect *parent, const C2Rect &crop)
        : _C2PlanarCapacityAspect(parent),
          mCrop(parent == nullptr ? C2Rect() : parent->crop().intersect(crop).normalize()) { }

private:
    friend class _C2EditablePlanarSectionAspect;
    C2Rect mCrop;
/// @}
};

/**
 * Aspect for objects that have an editable planar section (crop rectangle).
 *
 * This class is copiable.
 */
class _C2EditablePlanarSectionAspect : public _C2PlanarSectionAspect {
/// \name Planar section interface
/// @{
    using _C2PlanarSectionAspect::_C2PlanarSectionAspect;

public:
    // crop can be an empty rect, does not have to line up with subsampling
    // NOTE: we do not support floating-point crop
    inline constexpr C2Rect crop() const { return mCrop; }

    /**
     *  Sets crop to crop intersected with [(0,0) .. (width, height)]
     */
    inline void setCrop_be(const C2Rect &crop) {
        mCrop.left = std::min(width(), crop.left);
        mCrop.top = std::min(height(), crop.top);
        // It's guaranteed that mCrop.left <= width() && mCrop.top <= height()
        mCrop.width = std::min(width() - mCrop.left, crop.width);
        mCrop.height = std::min(height() - mCrop.top, crop.height);
    }

    /**
     * If crop is within the dimensions of this object, it sets crop to it.
     *
     * \return true iff crop is within the dimensions of this object
     */
    inline bool setCrop(const C2Rect &crop) {
        if (width() < crop.width || height() < crop.height
                || width() - crop.width < crop.left || height() - crop.height < crop.top) {
            return false;
        }
        mCrop = crop;
        return true;
    }
/// @}
};

/**
 * Utility class for safe range calculations using size_t-s.
 */
class C2PlanarSection : public _C2PlanarSectionAspect {
public:
    inline constexpr C2PlanarSection(const _C2PlanarCapacityAspect &parent, const C2Rect &crop)
        : _C2PlanarSectionAspect(&parent, crop) { }

    inline constexpr C2PlanarSection(const _C2PlanarSectionAspect &parent, const C2Rect &crop)
        : _C2PlanarSectionAspect(&parent, crop) { }

    inline constexpr C2PlanarSection intersect(const C2Rect &crop) const {
        return C2PlanarSection(*this, crop);
    }
};

/**
 * Utility class for simple and safe planar capacity and section construction.
 */
class C2PlanarCapacity : public _C2PlanarCapacityAspect {
public:
    inline constexpr explicit C2PlanarCapacity(size_t width, size_t height)
        : _C2PlanarCapacityAspect(c2_min(width, std::numeric_limits<uint32_t>::max()),
                                  c2_min(height, std::numeric_limits<uint32_t>::max())) { }

    inline constexpr C2PlanarSection section(const C2Rect &crop) const {
        return C2PlanarSection(*this, crop);
    }
};


/**
 * \ingroup graphic allocator
 * 2D allocation interface.
 */
class C2GraphicAllocation : public _C2PlanarCapacityAspect {
public:
    /**
     * Maps a rectangular section (as defined by |rect|) of a 2D allocation into local process
     * memory for flexible access. On success, it fills out |layout| with the plane specifications
     * and fills the |addr| array with pointers to the first byte of the top-left pixel of each
     * plane used. Otherwise, it leaves |layout| and |addr| untouched. |fence| will contain
     * an acquire sync fence object. If it is already safe to access the
     * buffer contents, then it will be an empty (already fired) fence.
     *
     * Safe regions for the pointer addresses returned can be gotten via C2LayoutInfo.minOffset()/
     * maxOffset().
     *
     * \param rect          section to be mapped (this does not have to be aligned)
     * \param usage         the desired usage. \todo this must be kSoftwareRead and/or
     *                      kSoftwareWrite.
     * \param fence         a pointer to a fence object if an async mapping is requested. If
     *                      not-null, and acquire fence will be stored here on success, or empty
     *                      fence on failure. If null, the mapping will be synchronous.
     * \param layout        a pointer to where the mapped planes' descriptors will be
     *                      stored. On failure, nullptr will be stored here.
     * \param addr          pointer to an array with at least C2PlanarLayout::MAX_NUM_PLANES
     *                      elements. Only layout.numPlanes elements will be modified on success.
     *
     * \retval C2_OK        the operation was successful
     * \retval C2_REFUSED   no permission to map the section
     * \retval C2_DUPLICATE there is already a mapped region and this allocation cannot support
     *                      multi-mapping (caller error)
     * \retval C2_TIMED_OUT the operation timed out
     * \retval C2_NO_MEMORY not enough memory to complete the operation
     * \retval C2_BAD_VALUE the parameters (rect) are invalid or outside the allocation, or the
     *                      usage flags are invalid (caller error)
     * \retval C2_CORRUPTED some unknown error prevented the operation from completing (unexpected)

     */
    virtual c2_status_t map(
            C2Rect rect, C2MemoryUsage usage, C2Fence *fence,
            C2PlanarLayout *layout /* nonnull */, uint8_t **addr /* nonnull */) = 0;

    /**
     * Unmaps a section of an allocation at |addr| with |rect|. These must be parameters previously
     * passed to and returned by |map|; otherwise, this operation is a no-op.
     *
     * \param addr          pointer to an array with at least C2PlanarLayout::MAX_NUM_PLANES
     *                      elements containing the starting addresses of the mapped layers
     * \param rect          boundaries of the mapped section
     * \param fence         a pointer to a fence object if an async unmapping is requested. If
     *                      not-null, a release fence will be stored here on success, or empty fence
     *                      on failure. This fence signals when the original allocation contains
     *                      all changes that happened to the mapped section. If null, the unmapping
     *                      will be synchronous.
     *
     * \retval C2_OK        the operation was successful
     * \retval C2_TIMED_OUT the operation timed out
     * \retval C2_NOT_FOUND there is no such mapped region (caller error)
     * \retval C2_CORRUPTED some unknown error prevented the operation from completing (unexpected)
     * \retval C2_REFUSED   no permission to unmap the section (unexpected - system)
     */
    virtual c2_status_t unmap(
            uint8_t **addr /* nonnull */, C2Rect rect, C2Fence *fence /* nullable */) = 0;

    /**
     * Returns the allocator ID for this allocation. This is useful to put the handle into context.
     */
    virtual C2Allocator::id_t getAllocatorId() const = 0;

    /**
     * Returns a pointer to the allocation handle.
     */
    virtual const C2Handle *handle() const = 0;

    /**
     * Returns true if this is the same allocation as |other|.
     */
    virtual bool equals(const std::shared_ptr<const C2GraphicAllocation> &other) const = 0;

protected:
    using _C2PlanarCapacityAspect::_C2PlanarCapacityAspect;
    virtual ~C2GraphicAllocation() = default;
};

class C2GraphicAllocation;

/**
 * A 2D block.
 *
 * \note width()/height() is not meaningful for users of blocks; instead, crop().width() and
 * crop().height() is the capacity of the usable portion. Use and crop() if accessing the block
 * directly through its handle to represent the allotted region of the underlying allocation to this
 * block.
 */
class C2Block2D : public _C2PlanarSectionAspect {
public:
    /**
     * Returns the underlying handle for this allocation.
     *
     * \note that the block and its block pool has shared ownership of the handle
     *       and if all references to the block are released, the underlying block
     *       allocation may get reused even if a client keeps a clone of this handle.
     */
    const C2Handle *handle() const;

    /**
     * Returns the allocator's ID that created the underlying allocation for this block. This
     * provides the context for understanding the handle.
     */
    C2Allocator::id_t getAllocatorId() const;

protected:
    class Impl;
    C2Block2D(std::shared_ptr<Impl> impl, const _C2PlanarSectionAspect &section);

    friend struct _C2BlockFactory;
    std::shared_ptr<Impl> mImpl;
};

/**
 * Graphic view provides read or read-write access for a graphic block.
 *
 * This class is copiable.
 *
 * \note Due to the subsampling of graphic buffers, a read view must still contain a crop rectangle
 * to ensure subsampling is followed. This results in nearly identical interface between read and
 * write views, so C2GraphicView can encompass both of them.
 */
class C2GraphicView : public _C2EditablePlanarSectionAspect {
public:
    /**
     * \return array of pointers (of layout().numPlanes elements) to the start of the planes or
     * nullptr on error. Regardless of crop rect, they always point to the top-left corner of each
     * plane. Access outside of the crop rect results in an undefined behavior.
     */
    const uint8_t *const *data() const;

    /**
     * \return array of pointers (of layout().numPlanes elements) to the start of the planes or
     * nullptr on error. Regardless of crop rect, they always point to the top-left corner of each
     * plane. Access outside of the crop rect results in an undefined behavior.
     */
    uint8_t *const *data();

    /**
     * \return layout of the graphic block to interpret the returned data.
     */
    const C2PlanarLayout layout() const;

    /**
     * Returns a section of this view.
     *
     * \param rect    the dimension of the section. \note This is clamped to the crop of this view.
     *
     * \return a read view containing the requested section of this view
     */
    const C2GraphicView subView(const C2Rect &rect) const;
    C2GraphicView subView(const C2Rect &rect);

    /**
     * \return error during the creation/mapping of this view.
     */
    c2_status_t error() const;

protected:
    class Impl;
    C2GraphicView(std::shared_ptr<Impl> impl, const _C2PlanarSectionAspect &section);
    explicit C2GraphicView(c2_status_t error);

private:
    friend struct _C2BlockFactory;
    std::shared_ptr<Impl> mImpl;
};

/**
 * A constant (read-only) graphic block (portion of an allocation) with an acquire fence.
 * Blocks are unmapped when created, and can be mapped into a read view on demand.
 *
 * This class is copiable and contains a reference to the allocation that it is based on.
 */
class C2ConstGraphicBlock : public C2Block2D {
public:
    /**
     * Maps this block into memory and returns a read view for it.
     *
     * \return a read view for this block.
     */
    C2Acquirable<const C2GraphicView> map() const;

    /**
     * Returns a section of this block.
     *
     * \param rect    the coordinates of the section. \note This is clamped to the crop rectangle of
     *              this block.
     *
     * \return a constant graphic block containing a portion of this block
     */
    C2ConstGraphicBlock subBlock(const C2Rect &rect) const;

    /**
     * Returns the acquire fence for this block.
     *
     * \return a fence that must be waited on before reading the block.
     */
    C2Fence fence() const { return mFence; }

protected:
    C2ConstGraphicBlock(
            std::shared_ptr<Impl> impl, const _C2PlanarSectionAspect &section, C2Fence fence);

private:
    friend struct _C2BlockFactory;
    C2Fence mFence;
};

/**
 * Graphic block is a writeable 2D block. Once written, it can be shared in whole or in part with
 * consumers/readers as read-only const graphic block.
 */
class C2GraphicBlock : public C2Block2D {
public:
    /**
     * Maps this block into memory and returns a write view for it.
     *
     * \return a write view for this block.
     */
    C2Acquirable<C2GraphicView> map();

    /**
     * Creates a read-only const linear block for a portion of this block; optionally protected
     * by an acquire fence. There are two ways to use this:
     *
     * 1) share ready block after writing data into the block. In this case no fence shall be
     *    supplied, and the block shall not be modified after calling this method.
     * 2) share block metadata before actually (finishing) writing the data into the block. In
     *    this case a fence must be supplied that will be triggered when the data is written.
     *    The block shall be modified only until firing the event for the fence.
     */
    C2ConstGraphicBlock share(const C2Rect &crop, C2Fence fence);

protected:
    C2GraphicBlock(std::shared_ptr<Impl> impl, const _C2PlanarSectionAspect &section);

    friend struct _C2BlockFactory;
};

/// @}

/// \defgroup buffer_onj Buffer objects
/// @{

// ================================================================================================
//  BUFFERS
// ================================================================================================

/// \todo: Do we still need this?
///
// There are 2 kinds of buffers: linear or graphic. Linear buffers can contain a single block, or
// a list of blocks (LINEAR_CHUNKS). Support for list of blocks is optional, and can allow consuming
// data from circular buffers or scattered data sources without extra memcpy. Currently, list of
// graphic blocks is not supported.

class C2LinearBuffer;   // read-write buffer
class C2GraphicBuffer;  // read-write buffer
class C2LinearChunksBuffer;

/**
 * C2BufferData: the main, non-meta data of a buffer. A buffer can contain either linear blocks
 * or graphic blocks, and can contain either a single block or multiple blocks. This is determined
 * by its type.
 */
class C2BufferData {
public:
    /**
     *  The type of buffer data.
     */
    enum type_t : uint32_t {
        INVALID,            ///< invalid buffer type. Do not use.
        LINEAR,             ///< the buffer contains a single linear block
        LINEAR_CHUNKS,      ///< the buffer contains one or more linear blocks
        GRAPHIC,            ///< the buffer contains a single graphic block
        GRAPHIC_CHUNKS,     ///< the buffer contains one of more graphic blocks
    };

    /**
     * Gets the type of this buffer (data).
     * \return the type of this buffer data.
     */
    type_t type() const;

    /**
     * Gets the linear blocks of this buffer.
     * \return a constant list of const linear blocks of this buffer.
     * \retval empty list if this buffer does not contain linear block(s).
     */
    const std::vector<C2ConstLinearBlock> linearBlocks() const;

    /**
     * Gets the graphic blocks of this buffer.
     * \return a constant list of const graphic blocks of this buffer.
     * \retval empty list if this buffer does not contain graphic block(s).
     */
    const std::vector<C2ConstGraphicBlock> graphicBlocks() const;

private:
    class Impl;
    std::shared_ptr<Impl> mImpl;

protected:
    // no public constructor
    explicit C2BufferData(const std::vector<C2ConstLinearBlock> &blocks);
    explicit C2BufferData(const std::vector<C2ConstGraphicBlock> &blocks);
};

/**
 * C2Buffer: buffer base class. These are always used as shared_ptrs. Though the underlying buffer
 * objects (native buffers, ion buffers, or dmabufs) are reference-counted by the system,
 * C2Buffers hold only a single reference.
 *
 * These objects cannot be used on the stack.
 */
class C2Buffer {
public:
    /**
     * Gets the buffer's data.
     *
     * \return the buffer's data.
     */
    const C2BufferData data() const;

    ///@name Pre-destroy notification handling
    ///@{

    /**
     * Register for notification just prior to the destruction of this object.
     */
    typedef void (*OnDestroyNotify) (const C2Buffer *buf, void *arg);

    /**
     * Registers for a pre-destroy notification. This is called just prior to the destruction of
     * this buffer (when this buffer is no longer valid.)
     *
     * \param onDestroyNotify   the notification callback
     * \param arg               an arbitrary parameter passed to the callback
     *
     * \retval C2_OK        the registration was successful.
     * \retval C2_DUPLICATE a notification was already registered for this callback and argument
     * \retval C2_NO_MEMORY not enough memory to register for this callback
     * \retval C2_CORRUPTED an unknown error prevented the registration (unexpected)
     */
    c2_status_t registerOnDestroyNotify(OnDestroyNotify onDestroyNotify, void *arg = nullptr);

    /**
     * Unregisters a previously registered pre-destroy notification.
     *
     * \param onDestroyNotify   the notification callback
     * \param arg               an arbitrary parameter passed to the callback
     *
     * \retval C2_OK        the unregistration was successful.
     * \retval C2_NOT_FOUND the notification was not found
     * \retval C2_CORRUPTED an unknown error prevented the registration (unexpected)
     */
    c2_status_t unregisterOnDestroyNotify(OnDestroyNotify onDestroyNotify, void *arg = nullptr);

    ///@}

    virtual ~C2Buffer() = default;

    ///@name Buffer-specific arbitrary metadata handling
    ///@{

    /**
     * Gets the list of metadata associated with this buffer.
     *
     * \return a constant list of info objects associated with this buffer.
     */
    const std::vector<std::shared_ptr<const C2Info>> info() const;

    /**
     * Attaches (or updates) an (existing) metadata for this buffer.
     * If the metadata is stream specific, the stream information will be reset.
     *
     * \param info Metadata to update
     *
     * \retval C2_OK        the metadata was successfully attached/updated.
     * \retval C2_NO_MEMORY not enough memory to attach the metadata (this return value is not
     *                      used if the same kind of metadata is already attached to the buffer).
     */
    c2_status_t setInfo(const std::shared_ptr<C2Info> &info);

    /**
     * Checks if there is a certain type of metadata attached to this buffer.
     *
     * \param index the parameter type of the metadata
     *
     * \return true iff there is a metadata with the parameter type attached to this buffer.
     */
    bool hasInfo(C2Param::Type index) const;

    /**
     * Checks if there is a certain type of metadata attached to this buffer, and returns a
     * shared pointer to it if there is. Returns an empty shared pointer object (nullptr) if there
     * is not.
     *
     * \param index the parameter type of the metadata
     *
     * \return shared pointer to the metadata.
     */
    std::shared_ptr<const C2Info> getInfo(C2Param::Type index) const;

    /**
     * Removes a metadata from the buffer.
     */
    std::shared_ptr<C2Info> removeInfo(C2Param::Type index);
    ///@}

    /**
     * Creates a buffer containing a single linear block.
     *
     * \param block the content of the buffer.
     *
     * \return shared pointer to the created buffer.
     */
    static std::shared_ptr<C2Buffer> CreateLinearBuffer(const C2ConstLinearBlock &block);

    /**
     * Creates a buffer containing a single graphic block.
     *
     * \param block the content of the buffer.
     *
     * \return shared pointer to the created buffer.
     */
    static std::shared_ptr<C2Buffer> CreateGraphicBuffer(const C2ConstGraphicBlock &block);

protected:
    // no public constructor
    explicit C2Buffer(const std::vector<C2ConstLinearBlock> &blocks);
    explicit C2Buffer(const std::vector<C2ConstGraphicBlock> &blocks);

private:
    class Impl;
    std::shared_ptr<Impl> mImpl;
};

/**
 * An extension of C2Info objects that can contain arbitrary buffer data.
 *
 * \note This object is not describable and contains opaque data.
 */
class C2InfoBuffer {
public:
    /**
     * Gets the index of this info object.
     *
     * \return the parameter index.
     */
    const C2Param::Index index() const;

    /**
     * Gets the buffer's data.
     *
     * \return the buffer's data.
     */
    const C2BufferData data() const;
};

/// @}

/// @}

#endif  // C2BUFFER_H_
