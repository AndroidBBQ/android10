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

#define LOG_TAG "AudioEndpointParcelable"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <stdint.h>

#include <binder/Parcel.h>
#include <binder/Parcelable.h>
#include <utility/AAudioUtilities.h>

#include "binding/AAudioServiceDefinitions.h"
#include "binding/RingBufferParcelable.h"
#include "binding/AudioEndpointParcelable.h"

using android::base::unique_fd;
using android::NO_ERROR;
using android::status_t;
using android::Parcel;
using android::Parcelable;

using namespace aaudio;

/**
 * Container for information about the message queues plus
 * general stream information needed by AAudio clients.
 * It contains no addresses, just sizes, offsets and file descriptors for
 * shared memory that can be passed through Binder.
 */
AudioEndpointParcelable::AudioEndpointParcelable() {}

AudioEndpointParcelable::~AudioEndpointParcelable() {}

/**
 * Add the file descriptor to the table.
 * @return index in table or negative error
 */
int32_t AudioEndpointParcelable::addFileDescriptor(const unique_fd& fd,
                                                   int32_t sizeInBytes) {
    if (mNumSharedMemories >= MAX_SHARED_MEMORIES) {
        return AAUDIO_ERROR_OUT_OF_RANGE;
    }
    int32_t index = mNumSharedMemories++;
    mSharedMemories[index].setup(fd, sizeInBytes);
    return index;
}

/**
 * The read and write must be symmetric.
 */
status_t AudioEndpointParcelable::writeToParcel(Parcel* parcel) const {
    status_t status = AAudioConvert_aaudioToAndroidStatus(validate());
    if (status != NO_ERROR) goto error;

    status = parcel->writeInt32(mNumSharedMemories);
    if (status != NO_ERROR) goto error;

    for (int i = 0; i < mNumSharedMemories; i++) {
        status = mSharedMemories[i].writeToParcel(parcel);
        if (status != NO_ERROR) goto error;
    }
    status = mUpMessageQueueParcelable.writeToParcel(parcel);
    if (status != NO_ERROR) goto error;
    status = mDownMessageQueueParcelable.writeToParcel(parcel);
    if (status != NO_ERROR) goto error;
    status = mUpDataQueueParcelable.writeToParcel(parcel);
    if (status != NO_ERROR) goto error;
    status = mDownDataQueueParcelable.writeToParcel(parcel);
    if (status != NO_ERROR) goto error;

    return NO_ERROR;

error:
    ALOGE("%s returning %d", __func__, status);
    return status;
}

status_t AudioEndpointParcelable::readFromParcel(const Parcel* parcel) {
    status_t status = parcel->readInt32(&mNumSharedMemories);
    if (status != NO_ERROR) goto error;

    for (int i = 0; i < mNumSharedMemories; i++) {
        mSharedMemories[i].readFromParcel(parcel);
        if (status != NO_ERROR) goto error;
    }
    status = mUpMessageQueueParcelable.readFromParcel(parcel);
    if (status != NO_ERROR) goto error;
    status = mDownMessageQueueParcelable.readFromParcel(parcel);
    if (status != NO_ERROR) goto error;
    status = mUpDataQueueParcelable.readFromParcel(parcel);
    if (status != NO_ERROR) goto error;
    status = mDownDataQueueParcelable.readFromParcel(parcel);
    if (status != NO_ERROR) goto error;

    return AAudioConvert_aaudioToAndroidStatus(validate());

error:
    ALOGE("%s returning %d", __func__, status);
    return status;
}

aaudio_result_t AudioEndpointParcelable::resolve(EndpointDescriptor *descriptor) {
    aaudio_result_t result = mUpMessageQueueParcelable.resolve(mSharedMemories,
                                                           &descriptor->upMessageQueueDescriptor);
    if (result != AAUDIO_OK) return result;
    result = mDownMessageQueueParcelable.resolve(mSharedMemories,
                                        &descriptor->downMessageQueueDescriptor);
    if (result != AAUDIO_OK) return result;

    result = mDownDataQueueParcelable.resolve(mSharedMemories,
                                              &descriptor->dataQueueDescriptor);
    return result;
}

aaudio_result_t AudioEndpointParcelable::close() {
    int err = 0;
    for (int i = 0; i < mNumSharedMemories; i++) {
        int lastErr = mSharedMemories[i].close();
        if (lastErr < 0) err = lastErr;
    }
    return AAudioConvert_androidToAAudioResult(err);
}

aaudio_result_t AudioEndpointParcelable::validate() const {
    if (mNumSharedMemories < 0 || mNumSharedMemories >= MAX_SHARED_MEMORIES) {
        ALOGE("invalid mNumSharedMemories = %d", mNumSharedMemories);
        return AAUDIO_ERROR_INTERNAL;
    }
    return AAUDIO_OK;
}

void AudioEndpointParcelable::dump() {
    ALOGD("======================================= BEGIN");
    ALOGD("mNumSharedMemories = %d", mNumSharedMemories);
    for (int i = 0; i < mNumSharedMemories; i++) {
        mSharedMemories[i].dump();
    }
    ALOGD("mUpMessageQueueParcelable =========");
    mUpMessageQueueParcelable.dump();
    ALOGD("mDownMessageQueueParcelable =======");
    mDownMessageQueueParcelable.dump();
    ALOGD("mUpDataQueueParcelable ============");
    mUpDataQueueParcelable.dump();
    ALOGD("mDownDataQueueParcelable ==========");
    mDownDataQueueParcelable.dump();
    ALOGD("======================================= END");
}

