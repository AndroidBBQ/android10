/*
 * Copyright 2016 The Android Open Source Project
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

#ifndef FIFO_FIFO_CONTROLLER_INDIRECT_H
#define FIFO_FIFO_CONTROLLER_INDIRECT_H

#include <stdint.h>
#include <atomic>

#include "FifoControllerBase.h"

namespace android {

/**
 * A FifoControllerBase with counters external to the class.
 *
 * The actual copunters may be stored in separate regions of shared memory
 * with different access rights.
 */
class FifoControllerIndirect : public FifoControllerBase {

public:
    FifoControllerIndirect(fifo_frames_t capacity,
                           fifo_frames_t threshold,
                           fifo_counter_t * readCounterAddress,
                           fifo_counter_t * writeCounterAddress)
        : FifoControllerBase(capacity, threshold)
        , mReadCounterAddress((std::atomic<fifo_counter_t> *) readCounterAddress)
        , mWriteCounterAddress((std::atomic<fifo_counter_t> *) writeCounterAddress)
    {
        setReadCounter(0);
        setWriteCounter(0);
    }
    virtual ~FifoControllerIndirect() {};

    // TODO review use of memory barriers, probably incorrect
    virtual fifo_counter_t getReadCounter() override {
        return mReadCounterAddress->load(std::memory_order_acquire);
    }

    virtual void setReadCounter(fifo_counter_t count) override {
        mReadCounterAddress->store(count, std::memory_order_release);
    }

    virtual fifo_counter_t getWriteCounter() override {
        return mWriteCounterAddress->load(std::memory_order_acquire);
    }

    virtual void setWriteCounter(fifo_counter_t count) override {
        mWriteCounterAddress->store(count, std::memory_order_release);
    }

private:
    std::atomic<fifo_counter_t> * mReadCounterAddress;
    std::atomic<fifo_counter_t> * mWriteCounterAddress;
};

}  // android

#endif //FIFO_FIFO_CONTROLLER_INDIRECT_H
