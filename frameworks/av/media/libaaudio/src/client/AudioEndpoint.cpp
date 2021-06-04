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

#define LOG_TAG "AudioEndpoint"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <cassert>
#include <aaudio/AAudio.h>

#include "AudioEndpointParcelable.h"
#include "AudioEndpoint.h"
#include "AAudioServiceMessage.h"

using namespace android;
using namespace aaudio;

#define RIDICULOUSLY_LARGE_BUFFER_CAPACITY   (256 * 1024)
#define RIDICULOUSLY_LARGE_FRAME_SIZE        4096

AudioEndpoint::AudioEndpoint()
    : mUpCommandQueue(nullptr)
    , mDataQueue(nullptr)
    , mFreeRunning(false)
    , mDataReadCounter(0)
    , mDataWriteCounter(0)
{
}

AudioEndpoint::~AudioEndpoint() {
    delete mDataQueue;
    delete mUpCommandQueue;
}

// TODO Consider moving to a method in RingBufferDescriptor
static aaudio_result_t AudioEndpoint_validateQueueDescriptor(const char *type,
                                                  const RingBufferDescriptor *descriptor) {
    if (descriptor == nullptr) {
        ALOGE("AudioEndpoint_validateQueueDescriptor() NULL descriptor");
        return AAUDIO_ERROR_NULL;
    }

    if (descriptor->capacityInFrames < 1
        || descriptor->capacityInFrames > RIDICULOUSLY_LARGE_BUFFER_CAPACITY) {
        ALOGE("AudioEndpoint_validateQueueDescriptor() bad capacityInFrames = %d",
              descriptor->capacityInFrames);
        return AAUDIO_ERROR_OUT_OF_RANGE;
    }

    // Reject extreme values to catch bugs and prevent numeric overflows.
    if (descriptor->bytesPerFrame < 1
        || descriptor->bytesPerFrame > RIDICULOUSLY_LARGE_FRAME_SIZE) {
        ALOGE("AudioEndpoint_validateQueueDescriptor() bad bytesPerFrame = %d",
              descriptor->bytesPerFrame);
        return AAUDIO_ERROR_OUT_OF_RANGE;
    }

    if (descriptor->dataAddress == nullptr) {
        ALOGE("AudioEndpoint_validateQueueDescriptor() NULL dataAddress");
        return AAUDIO_ERROR_NULL;
    }
    ALOGV("AudioEndpoint_validateQueueDescriptor %s, dataAddress at %p ====================",
          type,
          descriptor->dataAddress);
    ALOGV("AudioEndpoint_validateQueueDescriptor  readCounter at %p, writeCounter at %p",
          descriptor->readCounterAddress,
          descriptor->writeCounterAddress);

    // Try to READ from the data area.
    // This code will crash if the mmap failed.
    uint8_t value = descriptor->dataAddress[0];
    ALOGV("AudioEndpoint_validateQueueDescriptor() dataAddress[0] = %d, then try to write",
        (int) value);
    // Try to WRITE to the data area.
    descriptor->dataAddress[0] = value * 3;
    ALOGV("AudioEndpoint_validateQueueDescriptor() wrote successfully");

    if (descriptor->readCounterAddress) {
        fifo_counter_t counter = *descriptor->readCounterAddress;
        ALOGV("AudioEndpoint_validateQueueDescriptor() *readCounterAddress = %d, now write",
              (int) counter);
        *descriptor->readCounterAddress = counter;
        ALOGV("AudioEndpoint_validateQueueDescriptor() wrote readCounterAddress successfully");
    }

    if (descriptor->writeCounterAddress) {
        fifo_counter_t counter = *descriptor->writeCounterAddress;
        ALOGV("AudioEndpoint_validateQueueDescriptor() *writeCounterAddress = %d, now write",
              (int) counter);
        *descriptor->writeCounterAddress = counter;
        ALOGV("AudioEndpoint_validateQueueDescriptor() wrote writeCounterAddress successfully");
    }

    return AAUDIO_OK;
}

aaudio_result_t AudioEndpoint_validateDescriptor(const EndpointDescriptor *pEndpointDescriptor) {
    aaudio_result_t result = AudioEndpoint_validateQueueDescriptor("messages",
                                    &pEndpointDescriptor->upMessageQueueDescriptor);
    if (result == AAUDIO_OK) {
        result = AudioEndpoint_validateQueueDescriptor("data",
                                                &pEndpointDescriptor->dataQueueDescriptor);
    }
    return result;
}

aaudio_result_t AudioEndpoint::configure(const EndpointDescriptor *pEndpointDescriptor,
                                         aaudio_direction_t   direction)
{
    aaudio_result_t result = AudioEndpoint_validateDescriptor(pEndpointDescriptor);
    if (result != AAUDIO_OK) {
        return result;
    }

    // ============================ up message queue =============================
    const RingBufferDescriptor *descriptor = &pEndpointDescriptor->upMessageQueueDescriptor;
    if(descriptor->bytesPerFrame != sizeof(AAudioServiceMessage)) {
        ALOGE("configure() bytesPerFrame != sizeof(AAudioServiceMessage) = %d",
              descriptor->bytesPerFrame);
        return AAUDIO_ERROR_INTERNAL;
    }

    if(descriptor->readCounterAddress == nullptr || descriptor->writeCounterAddress == nullptr) {
        ALOGE("configure() NULL counter address");
        return AAUDIO_ERROR_NULL;
    }

    // Prevent memory leak and reuse.
    if(mUpCommandQueue != nullptr || mDataQueue != nullptr) {
        ALOGE("configure() endpoint already used");
        return AAUDIO_ERROR_INTERNAL;
    }

    mUpCommandQueue = new FifoBuffer(
            descriptor->bytesPerFrame,
            descriptor->capacityInFrames,
            descriptor->readCounterAddress,
            descriptor->writeCounterAddress,
            descriptor->dataAddress
    );

    // ============================ data queue =============================
    descriptor = &pEndpointDescriptor->dataQueueDescriptor;
    ALOGV("configure() data framesPerBurst = %d", descriptor->framesPerBurst);
    ALOGV("configure() data readCounterAddress = %p",
          descriptor->readCounterAddress);

    // An example of free running is when the other side is read or written by hardware DMA
    // or a DSP. It does not update its counter so we have to update it.
    int64_t *remoteCounter = (direction == AAUDIO_DIRECTION_OUTPUT)
                             ? descriptor->readCounterAddress // read by other side
                             : descriptor->writeCounterAddress; // written by other side
    mFreeRunning = (remoteCounter == nullptr);
    ALOGV("configure() mFreeRunning = %d", mFreeRunning ? 1 : 0);

    int64_t *readCounterAddress = (descriptor->readCounterAddress == nullptr)
                                  ? &mDataReadCounter
                                  : descriptor->readCounterAddress;
    int64_t *writeCounterAddress = (descriptor->writeCounterAddress == nullptr)
                                  ? &mDataWriteCounter
                                  : descriptor->writeCounterAddress;

    mDataQueue = new FifoBuffer(
            descriptor->bytesPerFrame,
            descriptor->capacityInFrames,
            readCounterAddress,
            writeCounterAddress,
            descriptor->dataAddress
    );
    uint32_t threshold = descriptor->capacityInFrames / 2;
    mDataQueue->setThreshold(threshold);
    return result;
}

aaudio_result_t AudioEndpoint::readUpCommand(AAudioServiceMessage *commandPtr)
{
    return mUpCommandQueue->read(commandPtr, 1);
}

int32_t AudioEndpoint::getEmptyFramesAvailable(WrappingBuffer *wrappingBuffer) {
    return mDataQueue->getEmptyRoomAvailable(wrappingBuffer);
}

int32_t AudioEndpoint::getEmptyFramesAvailable()
{
    return mDataQueue->getEmptyFramesAvailable();
}

int32_t AudioEndpoint::getFullFramesAvailable(WrappingBuffer *wrappingBuffer)
{
    return mDataQueue->getFullDataAvailable(wrappingBuffer);
}

int32_t AudioEndpoint::getFullFramesAvailable()
{
    return mDataQueue->getFullFramesAvailable();
}

void AudioEndpoint::advanceWriteIndex(int32_t deltaFrames) {
    mDataQueue->advanceWriteIndex(deltaFrames);
}

void AudioEndpoint::advanceReadIndex(int32_t deltaFrames) {
    mDataQueue->advanceReadIndex(deltaFrames);
}

void AudioEndpoint::setDataReadCounter(fifo_counter_t framesRead)
{
    mDataQueue->setReadCounter(framesRead);
}

fifo_counter_t AudioEndpoint::getDataReadCounter()
{
    return mDataQueue->getReadCounter();
}

void AudioEndpoint::setDataWriteCounter(fifo_counter_t framesRead)
{
    mDataQueue->setWriteCounter(framesRead);
}

fifo_counter_t AudioEndpoint::getDataWriteCounter()
{
    return mDataQueue->getWriteCounter();
}

int32_t AudioEndpoint::setBufferSizeInFrames(int32_t requestedFrames,
                                            int32_t *actualFrames)
{
    if (requestedFrames < ENDPOINT_DATA_QUEUE_SIZE_MIN) {
        requestedFrames = ENDPOINT_DATA_QUEUE_SIZE_MIN;
    }
    mDataQueue->setThreshold(requestedFrames);
    *actualFrames = mDataQueue->getThreshold();
    return AAUDIO_OK;
}

int32_t AudioEndpoint::getBufferSizeInFrames() const
{
    return mDataQueue->getThreshold();
}

int32_t AudioEndpoint::getBufferCapacityInFrames() const
{
    return (int32_t)mDataQueue->getBufferCapacityInFrames();
}

void AudioEndpoint::dump() const {
    ALOGD("data readCounter  = %lld", (long long) mDataQueue->getReadCounter());
    ALOGD("data writeCounter = %lld", (long long) mDataQueue->getWriteCounter());
}

void AudioEndpoint::eraseDataMemory() {
    mDataQueue->eraseMemory();
}
