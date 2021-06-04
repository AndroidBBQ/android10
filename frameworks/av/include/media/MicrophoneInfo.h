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

#ifndef ANDROID_MICROPHONE_INFO_H
#define ANDROID_MICROPHONE_INFO_H

#include <binder/Parcel.h>
#include <binder/Parcelable.h>
#include <system/audio.h>
#include <utils/String16.h>
#include <utils/Vector.h>

namespace android {
namespace media {

#define RETURN_IF_FAILED(calledOnce)                                     \
    {                                                                    \
        status_t returnStatus = calledOnce;                              \
        if (returnStatus) {                                              \
            ALOGE("Failed at %s:%d (%s)", __FILE__, __LINE__, __func__); \
            return returnStatus;                                         \
         }                                                               \
    }

class MicrophoneInfo : public Parcelable {
public:
    MicrophoneInfo() = default;
    MicrophoneInfo(const MicrophoneInfo& microphoneInfo) = default;
    MicrophoneInfo(audio_microphone_characteristic_t& characteristic) {
        mDeviceId = String16(&characteristic.device_id[0]);
        mPortId = characteristic.id;
        mType = characteristic.device;
        mAddress = String16(&characteristic.address[0]);
        mDeviceLocation = characteristic.location;
        mDeviceGroup = characteristic.group;
        mIndexInTheGroup = characteristic.index_in_the_group;
        mGeometricLocation.push_back(characteristic.geometric_location.x);
        mGeometricLocation.push_back(characteristic.geometric_location.y);
        mGeometricLocation.push_back(characteristic.geometric_location.z);
        mOrientation.push_back(characteristic.orientation.x);
        mOrientation.push_back(characteristic.orientation.y);
        mOrientation.push_back(characteristic.orientation.z);
        Vector<float> frequencies;
        Vector<float> responses;
        for (size_t i = 0; i < characteristic.num_frequency_responses; i++) {
            frequencies.push_back(characteristic.frequency_responses[0][i]);
            responses.push_back(characteristic.frequency_responses[1][i]);
        }
        mFrequencyResponses.push_back(frequencies);
        mFrequencyResponses.push_back(responses);
        for (size_t i = 0; i < AUDIO_CHANNEL_COUNT_MAX; i++) {
            mChannelMapping.push_back(characteristic.channel_mapping[i]);
        }
        mSensitivity = characteristic.sensitivity;
        mMaxSpl = characteristic.max_spl;
        mMinSpl = characteristic.min_spl;
        mDirectionality = characteristic.directionality;
    }

    virtual ~MicrophoneInfo() = default;

    virtual status_t writeToParcel(Parcel* parcel) const {
        RETURN_IF_FAILED(parcel->writeString16(mDeviceId));
        RETURN_IF_FAILED(parcel->writeInt32(mPortId));
        RETURN_IF_FAILED(parcel->writeUint32(mType));
        RETURN_IF_FAILED(parcel->writeString16(mAddress));
        RETURN_IF_FAILED(parcel->writeInt32(mDeviceLocation));
        RETURN_IF_FAILED(parcel->writeInt32(mDeviceGroup));
        RETURN_IF_FAILED(parcel->writeInt32(mIndexInTheGroup));
        RETURN_IF_FAILED(writeFloatVector(parcel, mGeometricLocation));
        RETURN_IF_FAILED(writeFloatVector(parcel, mOrientation));
        if (mFrequencyResponses.size() != 2) {
            return BAD_VALUE;
        }
        for (size_t i = 0; i < mFrequencyResponses.size(); i++) {
            RETURN_IF_FAILED(parcel->writeInt32(mFrequencyResponses[i].size()));
            RETURN_IF_FAILED(writeFloatVector(parcel, mFrequencyResponses[i]));
        }
        std::vector<int> channelMapping;
        for (size_t i = 0; i < mChannelMapping.size(); ++i) {
            channelMapping.push_back(mChannelMapping[i]);
        }
        RETURN_IF_FAILED(parcel->writeInt32Vector(channelMapping));
        RETURN_IF_FAILED(parcel->writeFloat(mSensitivity));
        RETURN_IF_FAILED(parcel->writeFloat(mMaxSpl));
        RETURN_IF_FAILED(parcel->writeFloat(mMinSpl));
        RETURN_IF_FAILED(parcel->writeInt32(mDirectionality));
        return OK;
    }

    virtual status_t readFromParcel(const Parcel* parcel) {
        RETURN_IF_FAILED(parcel->readString16(&mDeviceId));
        RETURN_IF_FAILED(parcel->readInt32(&mPortId));
        RETURN_IF_FAILED(parcel->readUint32(&mType));
        RETURN_IF_FAILED(parcel->readString16(&mAddress));
        RETURN_IF_FAILED(parcel->readInt32(&mDeviceLocation));
        RETURN_IF_FAILED(parcel->readInt32(&mDeviceGroup));
        RETURN_IF_FAILED(parcel->readInt32(&mIndexInTheGroup));
        RETURN_IF_FAILED(readFloatVector(parcel, &mGeometricLocation, 3));
        RETURN_IF_FAILED(readFloatVector(parcel, &mOrientation, 3));
        int32_t frequenciesNum;
        RETURN_IF_FAILED(parcel->readInt32(&frequenciesNum));
        Vector<float> frequencies;
        RETURN_IF_FAILED(readFloatVector(parcel, &frequencies, frequenciesNum));
        int32_t responsesNum;
        RETURN_IF_FAILED(parcel->readInt32(&responsesNum));
        Vector<float> responses;
        RETURN_IF_FAILED(readFloatVector(parcel, &responses, responsesNum));
        if (frequencies.size() != responses.size()) {
            return BAD_VALUE;
        }
        mFrequencyResponses.push_back(frequencies);
        mFrequencyResponses.push_back(responses);
        std::vector<int> channelMapping;
        status_t result = parcel->readInt32Vector(&channelMapping);
        if (result != OK) {
            return result;
        }
        if (channelMapping.size() != AUDIO_CHANNEL_COUNT_MAX) {
            return BAD_VALUE;
        }
        for (size_t i = 0; i < channelMapping.size(); i++) {
            mChannelMapping.push_back(channelMapping[i]);
        }
        RETURN_IF_FAILED(parcel->readFloat(&mSensitivity));
        RETURN_IF_FAILED(parcel->readFloat(&mMaxSpl));
        RETURN_IF_FAILED(parcel->readFloat(&mMinSpl));
        RETURN_IF_FAILED(parcel->readInt32(&mDirectionality));
        return OK;
    }

    String16 getDeviceId() const {
        return mDeviceId;
    }

    int getPortId() const {
        return mPortId;
    }

    unsigned int getType() const {
        return mType;
    }

    String16 getAddress() const {
        return mAddress;
    }

    int getDeviceLocation() const {
        return mDeviceLocation;
    }

    int getDeviceGroup() const {
        return mDeviceGroup;
    }

    int getIndexInTheGroup() const {
        return mIndexInTheGroup;
    }

    const Vector<float>& getGeometricLocation() const {
        return mGeometricLocation;
    }

    const Vector<float>& getOrientation() const {
        return mOrientation;
    }

    const Vector<Vector<float>>& getFrequencyResponses() const {
        return mFrequencyResponses;
    }

    const Vector<int>& getChannelMapping() const {
        return mChannelMapping;
    }

    float getSensitivity() const {
        return mSensitivity;
    }

    float getMaxSpl() const {
        return mMaxSpl;
    }

    float getMinSpl() const {
        return mMinSpl;
    }

    int getDirectionality() const {
        return mDirectionality;
    }

private:
    status_t readFloatVector(
            const Parcel* parcel, Vector<float> *vectorPtr, size_t defaultLength) {
        std::unique_ptr<std::vector<float>> v;
        status_t result = parcel->readFloatVector(&v);
        if (result != OK) return result;
        vectorPtr->clear();
        if (v.get() != nullptr) {
            for (const auto& iter : *v) {
                vectorPtr->push_back(iter);
            }
        } else {
            vectorPtr->resize(defaultLength);
        }
        return OK;
    }
    status_t writeFloatVector(Parcel* parcel, const Vector<float>& vector) const {
        std::vector<float> v;
        for (size_t i = 0; i < vector.size(); i++) {
            v.push_back(vector[i]);
        }
        return parcel->writeFloatVector(v);
    }

    String16 mDeviceId;
    int32_t mPortId;
    uint32_t mType;
    String16 mAddress;
    int32_t mDeviceLocation;
    int32_t mDeviceGroup;
    int32_t mIndexInTheGroup;
    Vector<float> mGeometricLocation;
    Vector<float> mOrientation;
    Vector<Vector<float>> mFrequencyResponses;
    Vector<int> mChannelMapping;
    float mSensitivity;
    float mMaxSpl;
    float mMinSpl;
    int32_t mDirectionality;
};

} // namespace media
} // namespace android

#endif
