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

#define LOG_TAG "AudioVolumeGroup"

//#define LOG_NDEBUG 0

#include <utils/Log.h>
#include <binder/Parcel.h>

#include <media/AudioVolumeGroup.h>
#include <media/AudioAttributes.h>

namespace android {

status_t AudioVolumeGroup::readFromParcel(const Parcel *parcel)
{
    status_t ret = parcel->readUtf8FromUtf16(&mName);
    if (ret != NO_ERROR) {
        return ret;
    }
    mGroupId = static_cast<volume_group_t>(parcel->readInt32());
    size_t size = static_cast<size_t>(parcel->readInt32());
    for (size_t i = 0; i < size; i++) {
        AudioAttributes attribute;
        attribute.readFromParcel(parcel);
        if (ret != NO_ERROR) {
            mAudioAttributes.clear();
            return ret;
        }
        mAudioAttributes.push_back(attribute.getAttributes());
    }
    size = static_cast<size_t>(parcel->readInt32());
    for (size_t i = 0; i < size; i++) {
        audio_stream_type_t stream = static_cast<audio_stream_type_t>(parcel->readInt32());
        mStreams.push_back(stream);
    }
    return NO_ERROR;
}

status_t AudioVolumeGroup::writeToParcel(Parcel *parcel) const
{
    parcel->writeUtf8AsUtf16(mName);
    parcel->writeInt32(static_cast<int32_t>(mGroupId));
    size_t size = mAudioAttributes.size();
    size_t sizePosition = parcel->dataPosition();
    parcel->writeInt32(size);
    size_t finalSize = size;
    for (const auto &attributes : mAudioAttributes) {
        size_t position = parcel->dataPosition();
        AudioAttributes attribute(attributes);
        status_t ret = attribute.writeToParcel(parcel);
        if (ret != NO_ERROR) {
            parcel->setDataPosition(position);
            finalSize--;
        }
    }
    if (size != finalSize) {
        size_t position = parcel->dataPosition();
        parcel->setDataPosition(sizePosition);
        parcel->writeInt32(finalSize);
        parcel->setDataPosition(position);
    }
    parcel->writeInt32(mStreams.size());
    for (const auto &stream : mStreams) {
        parcel->writeInt32(static_cast<int32_t>(stream));
    }
    return NO_ERROR;
}

} // namespace android
