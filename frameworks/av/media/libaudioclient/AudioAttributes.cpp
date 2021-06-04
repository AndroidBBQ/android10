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

#define LOG_TAG "AudioAttributes"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <binder/Parcel.h>

#include <media/AudioAttributes.h>

namespace android {

status_t AudioAttributes::readFromParcel(const Parcel *parcel)
{
    status_t ret = NO_ERROR;
    mAttributes.content_type = static_cast<audio_content_type_t>(parcel->readInt32());
    mAttributes.usage = static_cast<audio_usage_t>(parcel->readInt32());
    mAttributes.source = static_cast<audio_source_t>(parcel->readInt32());
    mAttributes.flags = static_cast<audio_flags_mask_t>(parcel->readInt32());
    const bool hasFlattenedTag = (parcel->readInt32() == 1);
    if (hasFlattenedTag) {
        std::string tags;
        ret = parcel->readUtf8FromUtf16(&tags);
        if (ret != NO_ERROR) {
            return ret;
        }
        std::strncpy(mAttributes.tags, tags.c_str(), AUDIO_ATTRIBUTES_TAGS_MAX_SIZE - 1);
    } else {
        strcpy(mAttributes.tags, "");
    }
    mStreamType = static_cast<audio_stream_type_t>(parcel->readInt32());
    mGroupId = static_cast<volume_group_t>(parcel->readUint32());
    return NO_ERROR;
}

status_t AudioAttributes::writeToParcel(Parcel *parcel) const
{
    parcel->writeInt32(static_cast<int32_t>(mAttributes.content_type));
    parcel->writeInt32(static_cast<int32_t>(mAttributes.usage));
    parcel->writeInt32(static_cast<int32_t>(mAttributes.source));
    parcel->writeInt32(static_cast<int32_t>(mAttributes.flags));
    if (strlen(mAttributes.tags) == 0) {
        parcel->writeInt32(0);
    } else {
        parcel->writeInt32(1);
        parcel->writeUtf8AsUtf16(mAttributes.tags);
    }
    parcel->writeInt32(static_cast<int32_t>(mStreamType));
    parcel->writeUint32(static_cast<uint32_t>(mGroupId));
    return NO_ERROR;
}

} // namespace android
