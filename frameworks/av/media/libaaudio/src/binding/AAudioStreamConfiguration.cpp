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

#define LOG_TAG "AAudioStreamConfiguration"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <stdint.h>

#include <sys/mman.h>
#include <aaudio/AAudio.h>

#include <binder/Parcel.h>
#include <binder/Parcelable.h>

#include "binding/AAudioStreamConfiguration.h"

using android::NO_ERROR;
using android::status_t;
using android::Parcel;
using android::Parcelable;

using namespace aaudio;

AAudioStreamConfiguration::AAudioStreamConfiguration() {}
AAudioStreamConfiguration::~AAudioStreamConfiguration() {}

status_t AAudioStreamConfiguration::writeToParcel(Parcel* parcel) const {
    status_t status;

    status = parcel->writeInt32(getDeviceId());
    if (status != NO_ERROR) goto error;
    status = parcel->writeInt32(getSampleRate());
    if (status != NO_ERROR) goto error;
    status = parcel->writeInt32(getSamplesPerFrame());
    if (status != NO_ERROR) goto error;
    status = parcel->writeInt32((int32_t) getSharingMode());
    if (status != NO_ERROR) goto error;
    status = parcel->writeInt32((int32_t) getFormat());
    if (status != NO_ERROR) goto error;

    status = parcel->writeInt32((int32_t) getDirection());
    if (status != NO_ERROR) goto error;
    status = parcel->writeInt32(getBufferCapacity());
    if (status != NO_ERROR) goto error;
    status = parcel->writeInt32((int32_t) getUsage());
    if (status != NO_ERROR) goto error;
    status = parcel->writeInt32((int32_t) getContentType());
    if (status != NO_ERROR) goto error;
    status = parcel->writeInt32((int32_t) getInputPreset());
    if (status != NO_ERROR) goto error;
    status = parcel->writeInt32((int32_t) getAllowedCapturePolicy());
    if (status != NO_ERROR) goto error;
    status = parcel->writeInt32(getSessionId());
    if (status != NO_ERROR) goto error;
    return NO_ERROR;
error:
    ALOGE("%s(): write failed = %d", __func__, status);
    return status;
}

status_t AAudioStreamConfiguration::readFromParcel(const Parcel* parcel) {
    int32_t value;
    status_t status = parcel->readInt32(&value);
    if (status != NO_ERROR) goto error;
    setDeviceId(value);
    status = parcel->readInt32(&value);
    if (status != NO_ERROR) goto error;
    setSampleRate(value);
    status = parcel->readInt32(&value);
    if (status != NO_ERROR) goto error;
    setSamplesPerFrame(value);
    status = parcel->readInt32(&value);
    if (status != NO_ERROR) goto error;
    setSharingMode((aaudio_sharing_mode_t) value);
    status = parcel->readInt32(&value);
    if (status != NO_ERROR) goto error;
    setFormat((audio_format_t) value);

    status = parcel->readInt32(&value);
    if (status != NO_ERROR) goto error;
    setDirection((aaudio_direction_t) value);
    status = parcel->readInt32(&value);
    if (status != NO_ERROR) goto error;
    setBufferCapacity(value);
    status = parcel->readInt32(&value);
    if (status != NO_ERROR) goto error;
    setUsage((aaudio_usage_t) value);
    status = parcel->readInt32(&value);
    if (status != NO_ERROR) goto error;
    setContentType((aaudio_content_type_t) value);
    status = parcel->readInt32(&value);
    if (status != NO_ERROR) goto error;
    setInputPreset((aaudio_input_preset_t) value);
    status = parcel->readInt32(&value);
    if (status != NO_ERROR) goto error;
    setAllowedCapturePolicy((aaudio_allowed_capture_policy_t) value);
    status = parcel->readInt32(&value);
    if (status != NO_ERROR) goto error;
    setSessionId(value);

    return NO_ERROR;
error:
    ALOGE("%s(): read failed = %d", __func__, status);
    return status;
}
