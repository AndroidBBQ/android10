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

#ifndef FIFO_FIFO_CONTROLLER_BASE_H
#define FIFO_FIFO_CONTROLLER_BASE_H

#include <stdint.h>

namespace android {

typedef int64_t fifo_counter_t;
typedef int32_t fifo_frames_t;

/**
 * Manage the read/write indices of a circular buffer.
 *
 * The caller is responsible for reading and writing the actual data.
 * Note that the span of available frames may not be contiguous. They
 * may wrap around from the end to the beginning of the buffer. In that
 * case the data must be read or written in at least two blocks of frames.
 *
 */
class FifoControllerBase {

public:
    /**
     * Constructor for FifoControllerBase
     * @param capacity Total size of the circular buffer in frames.
     * @param threshold Number of frames to fill. Must be less than capacity.
     */
    FifoControllerBase(fifo_frames_t capacity, fifo_frames_t threshold);

    virtual ~FifoControllerBase();

    // Abstract methods to be implemented in subclasses.
    /**
     * @return Counter used by the reader of the FIFO.
     */
    virtual fifo_counter_t getReadCounter() = 0;

    /**
     * This is normally only used internally.
     * @param count Number of frames that have been read.
     */
    virtual void setReadCounter(fifo_counter_t count) = 0;

    /**
     * @return Counter used by the reader of the FIFO.
     */
    virtual fifo_counter_t getWriteCounter() = 0;

    /**
     * This is normally only used internally.
     * @param count Number of frames that have been read.
     */
    virtual void setWriteCounter(fifo_counter_t count) = 0;

    /**
     * This may be negative if an unthrottled reader has read beyond the available data.
     * @return number of valid frames available to read. Never read more than this.
     */
    fifo_frames_t getFullFramesAvailable();

    /**
     * The index in a circular buffer of the next frame to read.
     */
    fifo_frames_t getReadIndex();

    /**
     * @param numFrames number of frames to advance the read index
     */
    void advanceReadIndex(fifo_frames_t numFrames);

    /**
     * @return number of frames that can be written. Never write more than this.
     */
    fifo_frames_t getEmptyFramesAvailable();

    /**
     * The index in a circular buffer of the next frame to write.
     */
    fifo_frames_t getWriteIndex();

    /**
     * @param numFrames number of frames to advance the write index
     */
    void advanceWriteIndex(fifo_frames_t numFrames);

    /**
     * You can request that the buffer not be filled above a maximum
     * number of frames.
     *
     * The threshold will be clipped between zero and the buffer capacity.
     *
     * @param threshold effective size of the buffer
     */
    void setThreshold(fifo_frames_t threshold);

    fifo_frames_t getThreshold() const {
        return mThreshold;
    }

    fifo_frames_t getCapacity() const {
        return mCapacity;
    }


private:
    fifo_frames_t mCapacity;
    fifo_frames_t mThreshold;
};

}  // android

#endif // FIFO_FIFO_CONTROLLER_BASE_H
