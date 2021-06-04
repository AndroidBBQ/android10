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

#ifndef FIFO_FIFO_CONTROLLER_H
#define FIFO_FIFO_CONTROLLER_H

#include <stdint.h>
#include <atomic>

#include "FifoControllerBase.h"

namespace android {

/**
 * A FIFO with counters contained in the class.
 */
class FifoController : public FifoControllerBase
{
public:
    FifoController(fifo_frames_t bufferSize, fifo_frames_t threshold)
    : FifoControllerBase(bufferSize, threshold)
    , mReadCounter(0)
    , mWriteCounter(0)
    {}

    virtual ~FifoController() {}

    // TODO review use of memory barriers, probably incorrect
    virtual fifo_counter_t getReadCounter() override {
        return mReadCounter.load(std::memory_order_acquire);
    }
    virtual void setReadCounter(fifo_counter_t n) override {
        mReadCounter.store(n, std::memory_order_release);
    }
    virtual fifo_counter_t getWriteCounter() override {
        return mWriteCounter.load(std::memory_order_acquire);
    }
    virtual void setWriteCounter(fifo_counter_t n) override {
        mWriteCounter.store(n, std::memory_order_release);
    }

private:
    std::atomic<fifo_counter_t> mReadCounter;
    std::atomic<fifo_counter_t> mWriteCounter;
};

}  // android

#endif //FIFO_FIFO_CONTROLLER_H
