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

#define LOG_TAG "RingBufferParcelable"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <stdint.h>

#include <binder/Parcelable.h>
#include <utility/AAudioUtilities.h>

#include "binding/AAudioServiceDefinitions.h"
#include "binding/SharedRegionParcelable.h"
#include "binding/RingBufferParcelable.h"

using namespace aaudio;

RingBufferParcelable::RingBufferParcelable() {}
RingBufferParcelable::~RingBufferParcelable() {}

// TODO This assumes that all three use the same SharedMemoryParcelable
void RingBufferParcelable::setupMemory(int32_t sharedMemoryIndex,
                 int32_t dataMemoryOffset,
                 int32_t dataSizeInBytes,
                 int32_t readCounterOffset,
                 int32_t writeCounterOffset,
                 int32_t counterSizeBytes) {
    mReadCounterParcelable.setup(sharedMemoryIndex, readCounterOffset, counterSizeBytes);
    mWriteCounterParcelable.setup(sharedMemoryIndex, writeCounterOffset, counterSizeBytes);
    mDataParcelable.setup(sharedMemoryIndex, dataMemoryOffset, dataSizeInBytes);
}

void RingBufferParcelable::setupMemory(int32_t sharedMemoryIndex,
                 int32_t dataMemoryOffset,
                 int32_t dataSizeInBytes) {
    mReadCounterParcelable.setup(sharedMemoryIndex, 0, 0);
    mWriteCounterParcelable.setup(sharedMemoryIndex, 0, 0);
    mDataParcelable.setup(sharedMemoryIndex, dataMemoryOffset, dataSizeInBytes);
}

int32_t RingBufferParcelable::getBytesPerFrame() {
    return mBytesPerFrame;
}

void RingBufferParcelable::setBytesPerFrame(int32_t bytesPerFrame) {
    mBytesPerFrame = bytesPerFrame;
}

int32_t RingBufferParcelable::getFramesPerBurst() {
    return mFramesPerBurst;
}

void RingBufferParcelable::setFramesPerBurst(int32_t framesPerBurst) {
    mFramesPerBurst = framesPerBurst;
}

int32_t RingBufferParcelable::getCapacityInFrames() {
    return mCapacityInFrames;
}

void RingBufferParcelable::setCapacityInFrames(int32_t capacityInFrames) {
    mCapacityInFrames = capacityInFrames;
}

/**
 * The read and write must be symmetric.
 */
status_t RingBufferParcelable::writeToParcel(Parcel* parcel) const {
    status_t status = AAudioConvert_aaudioToAndroidStatus(validate());
    if (status != NO_ERROR) goto error;

    status = parcel->writeInt32(mCapacityInFrames);
    if (status != NO_ERROR) goto error;
    if (mCapacityInFrames > 0) {
        status = parcel->writeInt32(mBytesPerFrame);
        if (status != NO_ERROR) goto error;
        status = parcel->writeInt32(mFramesPerBurst);
        if (status != NO_ERROR) goto error;
        status = parcel->writeInt32(mFlags);
        if (status != NO_ERROR) goto error;
        status = mReadCounterParcelable.writeToParcel(parcel);
        if (status != NO_ERROR) goto error;
        status = mWriteCounterParcelable.writeToParcel(parcel);
        if (status != NO_ERROR) goto error;
        status = mDataParcelable.writeToParcel(parcel);
        if (status != NO_ERROR) goto error;
    }
    return NO_ERROR;
error:
    ALOGE("%s returning %d", __func__, status);
    return status;
}

status_t RingBufferParcelable::readFromParcel(const Parcel* parcel) {
    status_t status = parcel->readInt32(&mCapacityInFrames);
    if (status != NO_ERROR) goto error;
    if (mCapacityInFrames > 0) {
        status = parcel->readInt32(&mBytesPerFrame);
        if (status != NO_ERROR) goto error;
        status = parcel->readInt32(&mFramesPerBurst);
        if (status != NO_ERROR) goto error;
        status = parcel->readInt32((int32_t *)&mFlags);
        if (status != NO_ERROR) goto error;
        status = mReadCounterParcelable.readFromParcel(parcel);
        if (status != NO_ERROR) goto error;
        status = mWriteCounterParcelable.readFromParcel(parcel);
        if (status != NO_ERROR) goto error;
        status = mDataParcelable.readFromParcel(parcel);
        if (status != NO_ERROR) goto error;
    }
    return AAudioConvert_aaudioToAndroidStatus(validate());
error:
    ALOGE("%s returning %d", __func__, status);
    return status;
}

aaudio_result_t RingBufferParcelable::resolve(SharedMemoryParcelable *memoryParcels, RingBufferDescriptor *descriptor) {
    aaudio_result_t result;

    result = mReadCounterParcelable.resolve(memoryParcels,
                                            (void **) &descriptor->readCounterAddress);
    if (result != AAUDIO_OK) {
        return result;
    }

    result = mWriteCounterParcelable.resolve(memoryParcels,
                                             (void **) &descriptor->writeCounterAddress);
    if (result != AAUDIO_OK) {
        return result;
    }

    result = mDataParcelable.resolve(memoryParcels, (void **) &descriptor->dataAddress);
    if (result != AAUDIO_OK) {
        return result;
    }

    descriptor->bytesPerFrame = mBytesPerFrame;
    descriptor->framesPerBurst = mFramesPerBurst;
    descriptor->capacityInFrames = mCapacityInFrames;
    descriptor->flags = mFlags;
    return AAUDIO_OK;
}

aaudio_result_t RingBufferParcelable::validate() const {
    if (mCapacityInFrames < 0 || mCapacityInFrames >= 32 * 1024) {
        ALOGE("invalid mCapacityInFrames = %d", mCapacityInFrames);
        return AAUDIO_ERROR_INTERNAL;
    }
    if (mBytesPerFrame < 0 || mBytesPerFrame >= 256) {
        ALOGE("invalid mBytesPerFrame = %d", mBytesPerFrame);
        return AAUDIO_ERROR_INTERNAL;
    }
    if (mFramesPerBurst < 0 || mFramesPerBurst >= 16 * 1024) {
        ALOGE("invalid mFramesPerBurst = %d", mFramesPerBurst);
        return AAUDIO_ERROR_INTERNAL;
    }
    return AAUDIO_OK;
}


void RingBufferParcelable::dump() {
    ALOGD("mCapacityInFrames = %d ---------", mCapacityInFrames);
    if (mCapacityInFrames > 0) {
        ALOGD("mBytesPerFrame = %d", mBytesPerFrame);
        ALOGD("mFramesPerBurst = %d", mFramesPerBurst);
        ALOGD("mFlags = %u", mFlags);
        mReadCounterParcelable.dump();
        mWriteCounterParcelable.dump();
        mDataParcelable.dump();
    }
}
