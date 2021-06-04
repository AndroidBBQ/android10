/*
**
** Copyright 2018, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "SessionConfiguration"
//#define LOG_NDEBUG 0

#include <utils/Log.h>

#include <camera/camera2/SessionConfiguration.h>
#include <camera/camera2/OutputConfiguration.h>
#include <binder/Parcel.h>

namespace android {

status_t SessionConfiguration::readFromParcel(const android::Parcel* parcel) {
    status_t err = OK;
    int operatingMode = 0;

    if (parcel == nullptr) return BAD_VALUE;

    if ((err = parcel->readInt32(&operatingMode)) != OK) {
        ALOGE("%s: Failed to read operating mode from parcel", __FUNCTION__);
        return err;
    }

    int inputWidth = 0;
    if ((err = parcel->readInt32(&inputWidth)) != OK) {
        ALOGE("%s: Failed to read input width from parcel", __FUNCTION__);
        return err;
    }

    int inputHeight = 0;
    if ((err = parcel->readInt32(&inputHeight)) != OK) {
        ALOGE("%s: Failed to read input height from parcel", __FUNCTION__);
        return err;
    }

    int inputFormat = -1;
    if ((err = parcel->readInt32(&inputFormat)) != OK) {
        ALOGE("%s: Failed to read input format from parcel", __FUNCTION__);
        return err;
    }

    std::vector<OutputConfiguration> outputStreams;
    if ((err = parcel->readParcelableVector(&outputStreams)) != OK) {
        ALOGE("%s: Failed to read output configurations from parcel", __FUNCTION__);
        return err;
    }

    mOperatingMode = operatingMode;
    mInputWidth = inputWidth;
    mInputHeight = inputHeight;
    mInputFormat = inputFormat;
    for (auto& stream : outputStreams) {
        mOutputStreams.push_back(stream);
    }


    return err;
}

status_t SessionConfiguration::writeToParcel(android::Parcel* parcel) const {

    if (parcel == nullptr) return BAD_VALUE;
    status_t err = OK;

    err = parcel->writeInt32(mOperatingMode);
    if (err != OK) return err;

    err = parcel->writeInt32(mInputWidth);
    if (err != OK) return err;

    err = parcel->writeInt32(mInputHeight);
    if (err != OK) return err;

    err = parcel->writeInt32(mInputFormat);
    if (err != OK) return err;

    err = parcel->writeParcelableVector(mOutputStreams);
    if (err != OK) return err;

    return OK;
}

bool SessionConfiguration::outputsEqual(const SessionConfiguration& other) const {
    const std::vector<OutputConfiguration>& otherOutputStreams =
            other.getOutputConfigurations();

    if (mOutputStreams.size() !=  otherOutputStreams.size()) {
        return false;
    }

    for (size_t i = 0; i < mOutputStreams.size(); i++) {
        if (mOutputStreams[i] != otherOutputStreams[i]) {
            return false;
        }
    }

    return true;
}

bool SessionConfiguration::outputsLessThan(const SessionConfiguration& other) const {
    const std::vector<OutputConfiguration>& otherOutputStreams =
            other.getOutputConfigurations();

    if (mOutputStreams.size() !=  otherOutputStreams.size()) {
        return mOutputStreams.size() < otherOutputStreams.size();
    }

    for (size_t i = 0; i < mOutputStreams.size(); i++) {
        if (mOutputStreams[i] != otherOutputStreams[i]) {
            return mOutputStreams[i] < otherOutputStreams[i];
        }
    }

    return false;
}

}; // namespace android
