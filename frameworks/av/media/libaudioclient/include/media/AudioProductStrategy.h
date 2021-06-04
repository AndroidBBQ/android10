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


#pragma once

#include <media/AudioCommonTypes.h>
#include <media/AudioAttributes.h>
#include <system/audio.h>
#include <system/audio_policy.h>
#include <binder/Parcelable.h>

namespace android {

class AudioProductStrategy : public Parcelable
{
public:
    AudioProductStrategy() {}
    AudioProductStrategy(const std::string &name, const std::vector<AudioAttributes> &attributes,
                         product_strategy_t id) :
        mName(name), mAudioAttributes(attributes), mId(id) {}

    const std::string &getName() const { return mName; }
    std::vector<AudioAttributes> getAudioAttributes() const { return mAudioAttributes; }
    product_strategy_t getId() const { return mId; }

    status_t readFromParcel(const Parcel *parcel) override;
    status_t writeToParcel(Parcel *parcel) const override;

    /**
     * @brief attributesMatches: checks if client attributes matches with a reference attributes
     * "matching" means the usage shall match if reference attributes has a defined usage, AND
     * content type shall match if reference attributes has a defined content type AND
     * flags shall match if reference attributes has defined flags AND
     * tags shall match if reference attributes has defined tags.
     * Reference attributes "default" shall not be considered as a "true" case. This convention
     * is used to identify the default strategy.
     * @param refAttributes to be considered
     * @param clientAttritubes to be considered
     * @return true if matching, false otherwise
     */
    static bool attributesMatches(const audio_attributes_t refAttributes,
                                  const audio_attributes_t clientAttritubes);
private:
    std::string mName;
    std::vector<AudioAttributes> mAudioAttributes;
    product_strategy_t mId;
};

using AudioProductStrategyVector = std::vector<AudioProductStrategy>;

} // namespace android

