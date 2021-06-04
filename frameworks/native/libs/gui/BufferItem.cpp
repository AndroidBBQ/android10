/*
 * Copyright 2014 The Android Open Source Project
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

#include <gui/BufferItem.h>

#include <ui/Fence.h>
#include <ui/GraphicBuffer.h>

#include <system/window.h>

namespace android {

template<typename T>
static inline constexpr uint32_t low32(const T n) {
    return static_cast<uint32_t>(static_cast<uint64_t>(n));
}

template<typename T>
static inline constexpr uint32_t high32(const T n) {
    return static_cast<uint32_t>(static_cast<uint64_t>(n)>>32);
}

template<typename T>
static inline constexpr T to64(const uint32_t lo, const uint32_t hi) {
    return static_cast<T>(static_cast<uint64_t>(hi)<<32 | lo);
}

BufferItem::BufferItem() :
    mGraphicBuffer(nullptr),
    mFence(nullptr),
    mCrop(Rect::INVALID_RECT),
    mTransform(0),
    mScalingMode(NATIVE_WINDOW_SCALING_MODE_FREEZE),
    mTimestamp(0),
    mIsAutoTimestamp(false),
    mDataSpace(HAL_DATASPACE_UNKNOWN),
    mFrameNumber(0),
    mSlot(INVALID_BUFFER_SLOT),
    mIsDroppable(false),
    mAcquireCalled(false),
    mTransformToDisplayInverse(false),
    mSurfaceDamage(),
    mAutoRefresh(false),
    mQueuedBuffer(true),
    mIsStale(false),
    mApi(0) {
}

BufferItem::~BufferItem() {}

template <typename T>
static void addAligned(size_t& size, T /* value */) {
    size = FlattenableUtils::align<sizeof(T)>(size);
    size += sizeof(T);
}

size_t BufferItem::getPodSize() const {
    size_t size = 0;
    addAligned(size, mCrop);
    addAligned(size, mTransform);
    addAligned(size, mScalingMode);
    addAligned(size, low32(mTimestamp));
    addAligned(size, high32(mTimestamp));
    addAligned(size, mIsAutoTimestamp);
    addAligned(size, mDataSpace);
    addAligned(size, low32(mFrameNumber));
    addAligned(size, high32(mFrameNumber));
    addAligned(size, mSlot);
    addAligned(size, mIsDroppable);
    addAligned(size, mAcquireCalled);
    addAligned(size, mTransformToDisplayInverse);
    addAligned(size, mAutoRefresh);
    addAligned(size, mQueuedBuffer);
    addAligned(size, mIsStale);
    addAligned(size, mApi);
    return size;
}

size_t BufferItem::getFlattenedSize() const {
    size_t size = sizeof(uint32_t); // Flags
    if (mGraphicBuffer != nullptr) {
        size += mGraphicBuffer->getFlattenedSize();
        size = FlattenableUtils::align<4>(size);
    }
    if (mFence != nullptr) {
        size += mFence->getFlattenedSize();
        size = FlattenableUtils::align<4>(size);
    }
    size += mSurfaceDamage.getFlattenedSize();
    size += mHdrMetadata.getFlattenedSize();
    size = FlattenableUtils::align<8>(size);
    return size + getPodSize();
}

size_t BufferItem::getFdCount() const {
    size_t count = 0;
    if (mGraphicBuffer != nullptr) {
        count += mGraphicBuffer->getFdCount();
    }
    if (mFence != nullptr) {
        count += mFence->getFdCount();
    }
    return count;
}

template <typename T>
static void writeAligned(void*& buffer, size_t& size, T value) {
    size -= FlattenableUtils::align<alignof(T)>(buffer);
    FlattenableUtils::write(buffer, size, value);
}

status_t BufferItem::flatten(
        void*& buffer, size_t& size, int*& fds, size_t& count) const {

    // make sure we have enough space
    if (size < BufferItem::getFlattenedSize()) {
        return NO_MEMORY;
    }

    // content flags are stored first
    uint32_t& flags = *static_cast<uint32_t*>(buffer);

    // advance the pointer
    FlattenableUtils::advance(buffer, size, sizeof(uint32_t));

    flags = 0;
    if (mGraphicBuffer != nullptr) {
        status_t err = mGraphicBuffer->flatten(buffer, size, fds, count);
        if (err) return err;
        size -= FlattenableUtils::align<4>(buffer);
        flags |= 1;
    }
    if (mFence != nullptr) {
        status_t err = mFence->flatten(buffer, size, fds, count);
        if (err) return err;
        size -= FlattenableUtils::align<4>(buffer);
        flags |= 2;
    }

    status_t err = mSurfaceDamage.flatten(buffer, size);
    if (err) return err;
    FlattenableUtils::advance(buffer, size, mSurfaceDamage.getFlattenedSize());

    err = mHdrMetadata.flatten(buffer, size);
    if (err) return err;
    FlattenableUtils::advance(buffer, size, mHdrMetadata.getFlattenedSize());

    // Check we still have enough space
    if (size < getPodSize()) {
        return NO_MEMORY;
    }

    writeAligned(buffer, size, mCrop);
    writeAligned(buffer, size, mTransform);
    writeAligned(buffer, size, mScalingMode);
    writeAligned(buffer, size, low32(mTimestamp));
    writeAligned(buffer, size, high32(mTimestamp));
    writeAligned(buffer, size, mIsAutoTimestamp);
    writeAligned(buffer, size, mDataSpace);
    writeAligned(buffer, size, low32(mFrameNumber));
    writeAligned(buffer, size, high32(mFrameNumber));
    writeAligned(buffer, size, mSlot);
    writeAligned(buffer, size, mIsDroppable);
    writeAligned(buffer, size, mAcquireCalled);
    writeAligned(buffer, size, mTransformToDisplayInverse);
    writeAligned(buffer, size, mAutoRefresh);
    writeAligned(buffer, size, mQueuedBuffer);
    writeAligned(buffer, size, mIsStale);
    writeAligned(buffer, size, mApi);

    return NO_ERROR;
}

template <typename T>
static void readAligned(const void*& buffer, size_t& size, T& value) {
    size -= FlattenableUtils::align<alignof(T)>(buffer);
    FlattenableUtils::read(buffer, size, value);
}

status_t BufferItem::unflatten(
        void const*& buffer, size_t& size, int const*& fds, size_t& count) {

    if (size < sizeof(uint32_t)) {
        return NO_MEMORY;
    }

    uint32_t flags = 0;
    FlattenableUtils::read(buffer, size, flags);

    if (flags & 1) {
        mGraphicBuffer = new GraphicBuffer();
        status_t err = mGraphicBuffer->unflatten(buffer, size, fds, count);
        if (err) return err;
        size -= FlattenableUtils::align<4>(buffer);
    }

    if (flags & 2) {
        mFence = new Fence();
        status_t err = mFence->unflatten(buffer, size, fds, count);
        if (err) return err;
        size -= FlattenableUtils::align<4>(buffer);

        mFenceTime = std::make_shared<FenceTime>(mFence);
    }

    status_t err = mSurfaceDamage.unflatten(buffer, size);
    if (err) return err;
    FlattenableUtils::advance(buffer, size, mSurfaceDamage.getFlattenedSize());

    err = mHdrMetadata.unflatten(buffer, size);
    if (err) return err;
    FlattenableUtils::advance(buffer, size, mHdrMetadata.getFlattenedSize());

    // Check we still have enough space
    if (size < getPodSize()) {
        return NO_MEMORY;
    }

    uint32_t timestampLo = 0, timestampHi = 0;
    uint32_t frameNumberLo = 0, frameNumberHi = 0;

    readAligned(buffer, size, mCrop);
    readAligned(buffer, size, mTransform);
    readAligned(buffer, size, mScalingMode);
    readAligned(buffer, size, timestampLo);
    readAligned(buffer, size, timestampHi);
    mTimestamp = to64<int64_t>(timestampLo, timestampHi);
    readAligned(buffer, size, mIsAutoTimestamp);
    readAligned(buffer, size, mDataSpace);
    readAligned(buffer, size, frameNumberLo);
    readAligned(buffer, size, frameNumberHi);
    mFrameNumber = to64<uint64_t>(frameNumberLo, frameNumberHi);
    readAligned(buffer, size, mSlot);
    readAligned(buffer, size, mIsDroppable);
    readAligned(buffer, size, mAcquireCalled);
    readAligned(buffer, size, mTransformToDisplayInverse);
    readAligned(buffer, size, mAutoRefresh);
    readAligned(buffer, size, mQueuedBuffer);
    readAligned(buffer, size, mIsStale);
    readAligned(buffer, size, mApi);

    return NO_ERROR;
}

const char* BufferItem::scalingModeName(uint32_t scalingMode) {
    switch (scalingMode) {
        case NATIVE_WINDOW_SCALING_MODE_FREEZE: return "FREEZE";
        case NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW: return "SCALE_TO_WINDOW";
        case NATIVE_WINDOW_SCALING_MODE_SCALE_CROP: return "SCALE_CROP";
        default: return "Unknown";
    }
}

} // namespace android
