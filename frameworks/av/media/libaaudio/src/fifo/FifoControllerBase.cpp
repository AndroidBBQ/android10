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

#define LOG_TAG "FifoControllerBase"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <stdint.h>
#include "FifoControllerBase.h"

using namespace android;  // TODO just import names needed

FifoControllerBase::FifoControllerBase(fifo_frames_t capacity, fifo_frames_t threshold)
        : mCapacity(capacity)
        , mThreshold(threshold)
{
}

FifoControllerBase::~FifoControllerBase() {
}

fifo_frames_t FifoControllerBase::getFullFramesAvailable() {
    return (fifo_frames_t) (getWriteCounter() - getReadCounter());
}

fifo_frames_t FifoControllerBase::getReadIndex() {
    // % works with non-power of two sizes
    return (fifo_frames_t) ((uint64_t)getReadCounter() % mCapacity);
}

void FifoControllerBase::advanceReadIndex(fifo_frames_t numFrames) {
    setReadCounter(getReadCounter() + numFrames);
}

fifo_frames_t FifoControllerBase::getEmptyFramesAvailable() {
    return (int32_t)(mThreshold - getFullFramesAvailable());
}

fifo_frames_t FifoControllerBase::getWriteIndex() {
    // % works with non-power of two sizes
    return (fifo_frames_t) ((uint64_t)getWriteCounter() % mCapacity);
}

void FifoControllerBase::advanceWriteIndex(fifo_frames_t numFrames) {
    setWriteCounter(getWriteCounter() + numFrames);
}

void FifoControllerBase::setThreshold(fifo_frames_t threshold) {
    if (threshold > mCapacity) {
        threshold = mCapacity;
    } else if (threshold < 0) {
        threshold = 0;
    }
    mThreshold = threshold;
}
