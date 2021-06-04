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

#define LOG_TAG "AudioProductStrategy"
//#define LOG_NDEBUG 0
#include <utils/Log.h>
#include <media/AudioProductStrategy.h>
#include <media/AudioAttributes.h>
#include <media/AudioSystem.h>

namespace android {

status_t AudioProductStrategy::readFromParcel(const Parcel *parcel)
{
    mId = static_cast<product_strategy_t>(parcel->readInt32());
    status_t ret = parcel->readUtf8FromUtf16(&mName);
    if (ret != NO_ERROR) {
        return ret;
    }
    size_t size = static_cast<size_t>(parcel->readInt32());
    for (size_t i = 0; i < size; i++) {
        AudioAttributes attribute;
        ret = attribute.readFromParcel(parcel);
        if (ret != NO_ERROR) {
            mAudioAttributes.clear();
            return ret;
        }
        mAudioAttributes.push_back(attribute);
    }
    return NO_ERROR;
}

status_t AudioProductStrategy::writeToParcel(Parcel *parcel) const
{
    parcel->writeInt32(static_cast<int32_t>(mId));
    parcel->writeUtf8AsUtf16(mName);
    size_t size = mAudioAttributes.size();
    size_t sizePosition = parcel->dataPosition();
    parcel->writeInt32(size);
    size_t finalSize = size;

    for (size_t i = 0; i < size; i++) {
        size_t position = parcel->dataPosition();
        AudioAttributes attribute(mAudioAttributes[i]);
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
    return NO_ERROR;
}

bool AudioProductStrategy::attributesMatches(const audio_attributes_t refAttributes,
                                        const audio_attributes_t clientAttritubes)
{
    if (refAttributes == AUDIO_ATTRIBUTES_INITIALIZER) {
        // The default product strategy is the strategy that holds default attributes by convention.
        // All attributes that fail to match will follow the default strategy for routing.
        // Choosing the default must be done as a fallback, the attributes match shall not
        // select the default.
        return false;
    }
    return ((refAttributes.usage == AUDIO_USAGE_UNKNOWN) ||
            (clientAttritubes.usage == refAttributes.usage)) &&
            ((refAttributes.content_type == AUDIO_CONTENT_TYPE_UNKNOWN) ||
             (clientAttritubes.content_type == refAttributes.content_type)) &&
            ((refAttributes.flags == AUDIO_FLAG_NONE) ||
             (clientAttritubes.flags != AUDIO_FLAG_NONE &&
            (clientAttritubes.flags & refAttributes.flags) == refAttributes.flags)) &&
            ((strlen(refAttributes.tags) == 0) ||
             (std::strcmp(clientAttritubes.tags, refAttributes.tags) == 0));
}

} // namespace android
