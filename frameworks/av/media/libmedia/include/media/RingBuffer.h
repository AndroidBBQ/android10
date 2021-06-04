/*
 * Copyright (C) 2015 The Android Open Source Project
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


#ifndef ANDROID_SERVICE_UTILS_RING_BUFFER_H
#define ANDROID_SERVICE_UTILS_RING_BUFFER_H

#include <utils/Log.h>
#include <cutils/compiler.h>

#include <iterator>
#include <utility>
#include <vector>

namespace android {

/**
 * A RingBuffer class that maintains an array of objects that can grow up to a certain size.
 * Elements added to the RingBuffer are inserted in the logical front of the buffer, and
 * invalidate all current iterators for that RingBuffer object.
 */
template <class T>
class RingBuffer final {
public:

    /**
     * Construct a RingBuffer that can grow up to the given length.
     */
    explicit RingBuffer(size_t length);

    /**
     * Forward iterator to this class.  Implements an std:forward_iterator.
     */
    class iterator : public std::iterator<std::forward_iterator_tag, T> {
    public:
        iterator(T* ptr, size_t size, size_t pos, size_t ctr);

        iterator& operator++();

        iterator operator++(int);

        bool operator==(const iterator& rhs);

        bool operator!=(const iterator& rhs);

        T& operator*();

        T* operator->();

    private:
        T* mPtr;
        size_t mSize;
        size_t mPos;
        size_t mCtr;
    };

    /**
     * Constant forward iterator to this class.  Implements an std:forward_iterator.
     */
    class const_iterator : public std::iterator<std::forward_iterator_tag, T> {
    public:
        const_iterator(const T* ptr, size_t size, size_t pos, size_t ctr);

        const_iterator& operator++();

        const_iterator operator++(int);

        bool operator==(const const_iterator& rhs);

        bool operator!=(const const_iterator& rhs);

        const T& operator*();

        const T* operator->();

    private:
        const T* mPtr;
        size_t mSize;
        size_t mPos;
        size_t mCtr;
    };

    /**
     * Adds item to the front of this RingBuffer.  If the RingBuffer is at its maximum length,
     * this will result in the last element being replaced (this is done using the element's
     * assignment operator).
     *
     * All current iterators are invalidated.
     */
    void add(const T& item);

    /**
     * Moves item to the front of this RingBuffer.  Following a call to this, item should no
     * longer be used.  If the RingBuffer is at its maximum length, this will result in the
     * last element being replaced (this is done using the element's assignment operator).
     *
     * All current iterators are invalidated.
     */
    void add(T&& item);

    /**
     * Construct item in-place in the front of this RingBuffer using the given arguments.  If
     * the RingBuffer is at its maximum length, this will result in the last element being
     * replaced (this is done using the element's assignment operator).
     *
     * All current iterators are invalidated.
     */
    template <class... Args>
    void emplace(Args&&... args);

    /**
     * Get an iterator to the front of this RingBuffer.
     */
    iterator begin();

    /**
     * Get an iterator to the end of this RingBuffer.
     */
    iterator end();

    /**
     * Get a const_iterator to the front of this RingBuffer.
     */
    const_iterator begin() const;

    /**
     * Get a const_iterator to the end of this RingBuffer.
     */
    const_iterator end() const;

    /**
     * Return a reference to the element at a given index.  If the index is out of range for
     * this ringbuffer, [0, size), the behavior for this is undefined.
     */
    T& operator[](size_t index);

    /**
     * Return a const reference to the element at a given index.  If the index is out of range
     * for this ringbuffer, [0, size), the behavior for this is undefined.
     */
    const T& operator[](size_t index) const;

    /**
     * Return the current size of this RingBuffer.
     */
    size_t size() const;

    /**
     * Remove all elements from this RingBuffer and set the size to 0.
     */
    void clear();

private:
    size_t mFrontIdx;
    size_t mMaxBufferSize;
    std::vector<T> mBuffer;
}; // class RingBuffer


template <class T>
RingBuffer<T>::RingBuffer(size_t length) : mFrontIdx{0}, mMaxBufferSize{length} {}

template <class T>
RingBuffer<T>::iterator::iterator(T* ptr, size_t size, size_t pos, size_t ctr) :
        mPtr{ptr}, mSize{size}, mPos{pos}, mCtr{ctr} {}

template <class T>
typename RingBuffer<T>::iterator& RingBuffer<T>::iterator::operator++() {
    ++mCtr;

    if (CC_UNLIKELY(mCtr == mSize)) {
        mPos = mSize;
        return *this;
    }

    mPos = ((CC_UNLIKELY(mPos == 0)) ? mSize - 1 : mPos - 1);
    return *this;
}

template <class T>
typename RingBuffer<T>::iterator RingBuffer<T>::iterator::operator++(int) {
    iterator tmp{mPtr, mSize, mPos, mCtr};
    ++(*this);
    return tmp;
}

template <class T>
bool RingBuffer<T>::iterator::operator==(const iterator& rhs) {
    return (mPtr + mPos) == (rhs.mPtr + rhs.mPos);
}

template <class T>
bool RingBuffer<T>::iterator::operator!=(const iterator& rhs) {
    return (mPtr + mPos) != (rhs.mPtr + rhs.mPos);
}

template <class T>
T& RingBuffer<T>::iterator::operator*() {
    return *(mPtr + mPos);
}

template <class T>
T* RingBuffer<T>::iterator::operator->() {
    return mPtr + mPos;
}

template <class T>
RingBuffer<T>::const_iterator::const_iterator(const T* ptr, size_t size, size_t pos, size_t ctr) :
        mPtr{ptr}, mSize{size}, mPos{pos}, mCtr{ctr} {}

template <class T>
typename RingBuffer<T>::const_iterator& RingBuffer<T>::const_iterator::operator++() {
    ++mCtr;

    if (CC_UNLIKELY(mCtr == mSize)) {
        mPos = mSize;
        return *this;
    }

    mPos = ((CC_UNLIKELY(mPos == 0)) ? mSize - 1 : mPos - 1);
    return *this;
}

template <class T>
typename RingBuffer<T>::const_iterator RingBuffer<T>::const_iterator::operator++(int) {
    const_iterator tmp{mPtr, mSize, mPos, mCtr};
    ++(*this);
    return tmp;
}

template <class T>
bool RingBuffer<T>::const_iterator::operator==(const const_iterator& rhs) {
    return (mPtr + mPos) == (rhs.mPtr + rhs.mPos);
}

template <class T>
bool RingBuffer<T>::const_iterator::operator!=(const const_iterator& rhs) {
    return (mPtr + mPos) != (rhs.mPtr + rhs.mPos);
}

template <class T>
const T& RingBuffer<T>::const_iterator::operator*() {
    return *(mPtr + mPos);
}

template <class T>
const T* RingBuffer<T>::const_iterator::operator->() {
    return mPtr + mPos;
}

template <class T>
void RingBuffer<T>::add(const T& item) {
    if (mBuffer.size() < mMaxBufferSize) {
        mBuffer.push_back(item);
        mFrontIdx = ((mFrontIdx + 1) % mMaxBufferSize);
        return;
    }

    mBuffer[mFrontIdx] = item;
    mFrontIdx = ((mFrontIdx + 1) % mMaxBufferSize);
}

template <class T>
void RingBuffer<T>::add(T&& item) {
    if (mBuffer.size() != mMaxBufferSize) {
        mBuffer.push_back(std::forward<T>(item));
        mFrontIdx = ((mFrontIdx + 1) % mMaxBufferSize);
        return;
    }

    // Only works for types with move assignment operator
    mBuffer[mFrontIdx] = std::forward<T>(item);
    mFrontIdx = ((mFrontIdx + 1) % mMaxBufferSize);
}

template <class T>
template <class... Args>
void RingBuffer<T>::emplace(Args&&... args) {
    if (mBuffer.size() != mMaxBufferSize) {
        mBuffer.emplace_back(std::forward<Args>(args)...);
        mFrontIdx = ((mFrontIdx + 1) % mMaxBufferSize);
        return;
    }

    // Only works for types with move assignment operator
    mBuffer[mFrontIdx] = T(std::forward<Args>(args)...);
    mFrontIdx = ((mFrontIdx + 1) % mMaxBufferSize);
}

template <class T>
typename RingBuffer<T>::iterator RingBuffer<T>::begin() {
    size_t tmp = (mBuffer.size() == 0) ? 0 : mBuffer.size() - 1;
    return iterator(mBuffer.data(), mBuffer.size(), (mFrontIdx == 0) ? tmp : mFrontIdx - 1, 0);
}

template <class T>
typename RingBuffer<T>::iterator RingBuffer<T>::end() {
    size_t s = mBuffer.size();
    return iterator(mBuffer.data(), s, s, s);
}

template <class T>
typename RingBuffer<T>::const_iterator RingBuffer<T>::begin() const {
    size_t tmp = (mBuffer.size() == 0) ? 0 : mBuffer.size() - 1;
    return const_iterator(mBuffer.data(), mBuffer.size(),
            (mFrontIdx == 0) ? tmp : mFrontIdx - 1, 0);
}

template <class T>
typename RingBuffer<T>::const_iterator RingBuffer<T>::end() const {
    size_t s = mBuffer.size();
    return const_iterator(mBuffer.data(), s, s, s);
}

template <class T>
T& RingBuffer<T>::operator[](size_t index) {
    LOG_ALWAYS_FATAL_IF(index >= mBuffer.size(), "Index %zu out of bounds, size is %zu.",
            index, mBuffer.size());
    size_t pos = (index >= mFrontIdx) ?
            mBuffer.size() - 1 - (index - mFrontIdx) : mFrontIdx - 1 - index;
    return mBuffer[pos];
}

template <class T>
const T& RingBuffer<T>::operator[](size_t index) const {
    LOG_ALWAYS_FATAL_IF(index >= mBuffer.size(), "Index %zu out of bounds, size is %zu.",
            index, mBuffer.size());
    size_t pos = (index >= mFrontIdx) ?
            mBuffer.size() - 1 - (index - mFrontIdx) : mFrontIdx - 1 - index;
    return mBuffer[pos];
}

template <class T>
size_t RingBuffer<T>::size() const {
    return mBuffer.size();
}

template <class T>
void RingBuffer<T>::clear() {
    mBuffer.clear();
    mFrontIdx = 0;
}

}; // namespace android

#endif // ANDROID_SERVICE_UTILS_RING_BUFFER_H


