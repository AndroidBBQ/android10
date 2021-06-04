/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef ANDROID_HARDWARE_CAMERA2_SESSIONCONFIGURATION_H
#define ANDROID_HARDWARE_CAMERA2_SESSIONCONFIGURATION_H

#include <binder/Parcelable.h>

namespace android {

namespace hardware {
namespace camera2 {
namespace params {

class OutputConfiguration;

class SessionConfiguration : public android::Parcelable {
public:

    const std::vector<OutputConfiguration>& getOutputConfigurations() const {
        return mOutputStreams;
    }

    int getInputWidth() const { return mInputWidth; }
    int getInputHeight() const { return mInputHeight; }
    int getInputFormat() const { return mInputFormat; }
    int getOperatingMode() const { return mOperatingMode; }

    virtual status_t writeToParcel(android::Parcel* parcel) const override;
    virtual status_t readFromParcel(const android::Parcel* parcel) override;

    SessionConfiguration() :
            mInputWidth(0),
            mInputHeight(0),
            mInputFormat(-1),
            mOperatingMode(-1) {}

    SessionConfiguration(const android::Parcel& parcel) {
        readFromParcel(&parcel);
    }

    SessionConfiguration(int inputWidth, int inputHeight, int inputFormat, int operatingMode) :
        mInputWidth(inputWidth), mInputHeight(inputHeight), mInputFormat(inputFormat),
        mOperatingMode(operatingMode) {}

    bool operator == (const SessionConfiguration& other) const {
        return (outputsEqual(other) &&
                mInputWidth == other.mInputWidth &&
                mInputHeight == other.mInputHeight &&
                mInputFormat == other.mInputFormat &&
                mOperatingMode == other.mOperatingMode);
    }

    bool operator != (const SessionConfiguration& other) const {
        return !(*this == other);
    }

    bool operator < (const SessionConfiguration& other) const {
        if (*this == other) return false;

        if (mInputWidth != other.mInputWidth) {
            return mInputWidth < other.mInputWidth;
        }

        if (mInputHeight != other.mInputHeight) {
            return mInputHeight < other.mInputHeight;
        }

        if (mInputFormat != other.mInputFormat) {
            return mInputFormat < other.mInputFormat;
        }

        if (mOperatingMode != other.mOperatingMode) {
            return mOperatingMode < other.mOperatingMode;
        }

        return outputsLessThan(other);
    }

    bool operator > (const SessionConfiguration& other) const {
        return (*this != other && !(*this < other));
    }

    bool outputsEqual(const SessionConfiguration& other) const;
    bool outputsLessThan(const SessionConfiguration& other) const;
    void addOutputConfiguration(const OutputConfiguration &config) {
        mOutputStreams.push_back(config);
    }

private:

    std::vector<OutputConfiguration> mOutputStreams;
    int                              mInputWidth, mInputHeight, mInputFormat, mOperatingMode;
};
} // namespace params
} // namespace camera2
} // namespace hardware

using hardware::camera2::params::SessionConfiguration;

}; // namespace android

#endif
