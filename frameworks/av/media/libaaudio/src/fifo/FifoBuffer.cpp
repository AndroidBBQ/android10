/*
 * Copyright 2015 The Android Open Source Project
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

#include <cstring>
#include <unistd.h>


#define LOG_TAG "FifoBuffer"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <algorithm>
#include <memory>

#include "FifoControllerBase.h"
#include "FifoController.h"
#include "FifoControllerIndirect.h"
#include "FifoBuffer.h"

using android::FifoBuffer;
using android::fifo_frames_t;

FifoBuffer::FifoBuffer(int32_t bytesPerFrame, fifo_frames_t capacityInFrames)
        : mBytesPerFrame(bytesPerFrame)
{
    mFifo = std::make_unique<FifoController>(capacityInFrames, capacityInFrames);
    // allocate buffer
    int32_t bytesPerBuffer = bytesPerFrame * capacityInFrames;
    mStorage = new uint8_t[bytesPerBuffer];
    mStorageOwned = true;
    ALOGV("%s() capacityInFrames = %d, bytesPerFrame = %d",
          __func__, capacityInFrames, bytesPerFrame);
}

FifoBuffer::FifoBuffer( int32_t   bytesPerFrame,
                        fifo_frames_t   capacityInFrames,
                        fifo_counter_t *  readIndexAddress,
                        fifo_counter_t *  writeIndexAddress,
                        void *  dataStorageAddress
                        )
        : mBytesPerFrame(bytesPerFrame)
        , mStorage(static_cast<uint8_t *>(dataStorageAddress))
{
    mFifo = std::make_unique<FifoControllerIndirect>(capacityInFrames,
                                       capacityInFrames,
                                       readIndexAddress,
                                       writeIndexAddress);
    mStorageOwned = false;
}

FifoBuffer::~FifoBuffer() {
    if (mStorageOwned) {
        delete[] mStorage;
    }
}

int32_t FifoBuffer::convertFramesToBytes(fifo_frames_t frames) {
    return frames * mBytesPerFrame;
}

void FifoBuffer::fillWrappingBuffer(WrappingBuffer *wrappingBuffer,
                                    int32_t framesAvailable,
                                    int32_t startIndex) {
    wrappingBuffer->data[1] = nullptr;
    wrappingBuffer->numFrames[1] = 0;
    if (framesAvailable > 0) {
        fifo_frames_t capacity = mFifo->getCapacity();
        uint8_t *source = &mStorage[convertFramesToBytes(startIndex)];
        // Does the available data cross the end of the FIFO?
        if ((startIndex + framesAvailable) > capacity) {
            wrappingBuffer->data[0] = source;
            fifo_frames_t firstFrames = capacity - startIndex;
            wrappingBuffer->numFrames[0] = firstFrames;
            wrappingBuffer->data[1] = &mStorage[0];
            wrappingBuffer->numFrames[1] = framesAvailable - firstFrames;
        } else {
            wrappingBuffer->data[0] = source;
            wrappingBuffer->numFrames[0] = framesAvailable;
        }
    } else {
        wrappingBuffer->data[0] = nullptr;
        wrappingBuffer->numFrames[0] = 0;
    }
}

fifo_frames_t FifoBuffer::getFullDataAvailable(WrappingBuffer *wrappingBuffer) {
    // The FIFO might be overfull so clip to capacity.
    fifo_frames_t framesAvailable = std::min(mFifo->getFullFramesAvailable(),
                                             mFifo->getCapacity());
    fifo_frames_t startIndex = mFifo->getReadIndex();
    fillWrappingBuffer(wrappingBuffer, framesAvailable, startIndex);
    return framesAvailable;
}

fifo_frames_t FifoBuffer::getEmptyRoomAvailable(WrappingBuffer *wrappingBuffer) {
    // The FIFO might have underrun so clip to capacity.
    fifo_frames_t framesAvailable = std::min(mFifo->getEmptyFramesAvailable(),
                                             mFifo->getCapacity());
    fifo_frames_t startIndex = mFifo->getWriteIndex();
    fillWrappingBuffer(wrappingBuffer, framesAvailable, startIndex);
    return framesAvailable;
}

fifo_frames_t FifoBuffer::read(void *buffer, fifo_frames_t numFrames) {
    WrappingBuffer wrappingBuffer;
    uint8_t *destination = (uint8_t *) buffer;
    fifo_frames_t framesLeft = numFrames;

    getFullDataAvailable(&wrappingBuffer);

    // Read data in one or two parts.
    int partIndex = 0;
    while (framesLeft > 0 && partIndex < WrappingBuffer::SIZE) {
        fifo_frames_t framesToRead = framesLeft;
        fifo_frames_t framesAvailable = wrappingBuffer.numFrames[partIndex];
        if (framesAvailable > 0) {
            if (framesToRead > framesAvailable) {
                framesToRead = framesAvailable;
            }
            int32_t numBytes = convertFramesToBytes(framesToRead);
            memcpy(destination, wrappingBuffer.data[partIndex], numBytes);

            destination += numBytes;
            framesLeft -= framesToRead;
        } else {
            break;
        }
        partIndex++;
    }
    fifo_frames_t framesRead = numFrames - framesLeft;
    mFifo->advanceReadIndex(framesRead);
    return framesRead;
}

fifo_frames_t FifoBuffer::write(const void *buffer, fifo_frames_t numFrames) {
    WrappingBuffer wrappingBuffer;
    uint8_t *source = (uint8_t *) buffer;
    fifo_frames_t framesLeft = numFrames;

    getEmptyRoomAvailable(&wrappingBuffer);

    // Read data in one or two parts.
    int partIndex = 0;
    while (framesLeft > 0 && partIndex < WrappingBuffer::SIZE) {
        fifo_frames_t framesToWrite = framesLeft;
        fifo_frames_t framesAvailable = wrappingBuffer.numFrames[partIndex];
        if (framesAvailable > 0) {
            if (framesToWrite > framesAvailable) {
                framesToWrite = framesAvailable;
            }
            int32_t numBytes = convertFramesToBytes(framesToWrite);
            memcpy(wrappingBuffer.data[partIndex], source, numBytes);

            source += numBytes;
            framesLeft -= framesToWrite;
        } else {
            break;
        }
        partIndex++;
    }
    fifo_frames_t framesWritten = numFrames - framesLeft;
    mFifo->advanceWriteIndex(framesWritten);
    return framesWritten;
}

fifo_frames_t FifoBuffer::getThreshold() {
    return mFifo->getThreshold();
}

void FifoBuffer::setThreshold(fifo_frames_t threshold) {
    mFifo->setThreshold(threshold);
}

fifo_frames_t FifoBuffer::getBufferCapacityInFrames() {
    return mFifo->getCapacity();
}

void FifoBuffer::eraseMemory() {
    int32_t numBytes = convertFramesToBytes(getBufferCapacityInFrames());
    if (numBytes > 0) {
        memset(mStorage, 0, (size_t) numBytes);
    }
}
