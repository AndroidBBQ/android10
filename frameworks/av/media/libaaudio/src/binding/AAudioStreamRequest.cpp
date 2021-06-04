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

#define LOG_TAG "AAudioStreamRequest"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <stdint.h>

#include <sys/mman.h>
#include <binder/Parcel.h>
#include <binder/Parcelable.h>

#include <aaudio/AAudio.h>

#include "binding/AAudioStreamConfiguration.h"
#include "binding/AAudioStreamRequest.h"

using android::NO_ERROR;
using android::status_t;
using android::Parcel;
using android::Parcelable;

using namespace aaudio;

AAudioStreamRequest::AAudioStreamRequest()
    : mConfiguration()
    {}

AAudioStreamRequest::~AAudioStreamRequest() {}

status_t AAudioStreamRequest::writeToParcel(Parcel* parcel) const {
    status_t status = parcel->writeInt32((int32_t) mUserId);
    if (status != NO_ERROR) goto error;

    status = parcel->writeBool(mSharingModeMatchRequired);
    if (status != NO_ERROR) goto error;

    status = parcel->writeBool(mInService);
    if (status != NO_ERROR) goto error;

    status = mConfiguration.writeToParcel(parcel);
    if (status != NO_ERROR) goto error;

    return NO_ERROR;

error:
    ALOGE("writeToParcel(): write failed = %d", status);
    return status;
}

status_t AAudioStreamRequest::readFromParcel(const Parcel* parcel) {
    int32_t temp;
    status_t status = parcel->readInt32(&temp);
    if (status != NO_ERROR) goto error;
    mUserId = (uid_t) temp;

    status = parcel->readBool(&mSharingModeMatchRequired);
    if (status != NO_ERROR) goto error;

    status = parcel->readBool(&mInService);
    if (status != NO_ERROR) goto error;

    status = mConfiguration.readFromParcel(parcel);
    if (status != NO_ERROR) goto error;

    return NO_ERROR;

error:
    ALOGE("readFromParcel(): read failed = %d", status);
    return status;
}

aaudio_result_t AAudioStreamRequest::validate() const {
    return mConfiguration.validate();
}

void AAudioStreamRequest::dump() const {
    ALOGD("mUserId    = %d", mUserId);
    ALOGD("mProcessId = %d", mProcessId);
    ALOGD("mSharingModeMatchRequired = %d", mSharingModeMatchRequired);
    ALOGD("mInService = %d", mInService);
    mConfiguration.dump();
}
