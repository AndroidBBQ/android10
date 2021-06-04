/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef SHCIRCULARBUFFER_H
#define SHCIRCULARBUFFER_H

#include <log/log.h>
#include <vector>

template <class T>
class SHCircularBuffer {

public:
    SHCircularBuffer() : mReadIndex(0), mWriteIndex(0), mReadAvailable(0) {
    }

    explicit SHCircularBuffer(size_t maxSize) {
        resize(maxSize);
    }
    void resize(size_t maxSize) {
        mBuffer.resize(maxSize);
        mReadIndex = 0;
        mWriteIndex = 0;
        mReadAvailable = 0;
    }
    inline void write(T value) {
        if (availableToWrite()) {
            mBuffer[mWriteIndex++] = value;
            if (mWriteIndex >= getSize()) {
                mWriteIndex = 0;
            }
            mReadAvailable++;
        } else {
            ALOGE("Error: SHCircularBuffer no space to write. allocated size %zu ", getSize());
        }
    }
    inline T read() {
        T value = T();
        if (availableToRead()) {
            value = mBuffer[mReadIndex++];
            if (mReadIndex >= getSize()) {
                mReadIndex = 0;
            }
            mReadAvailable--;
        } else {
            ALOGW("Warning: SHCircularBuffer no data available to read. Default value returned");
        }
        return value;
    }
    inline size_t availableToRead() const {
        return mReadAvailable;
    }
    inline size_t availableToWrite() const {
        return getSize() - mReadAvailable;
    }
    inline size_t getSize() const {
        return mBuffer.size();
    }

private:
    std::vector<T> mBuffer;
    size_t mReadIndex;
    size_t mWriteIndex;
    size_t mReadAvailable;
};


#endif //SHCIRCULARBUFFER_H
