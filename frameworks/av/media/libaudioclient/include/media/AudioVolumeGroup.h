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

#include <media/AudioProductStrategy.h>
#include <system/audio.h>
#include <system/audio_policy.h>
#include <binder/Parcelable.h>

namespace android {

class AudioVolumeGroup : public Parcelable
{
public:
    AudioVolumeGroup() {}
    AudioVolumeGroup(const std::string &name,
                     volume_group_t group,
                     const AttributesVector &attributes,
                     const StreamTypeVector &streams) :
        mName(name), mGroupId(group), mAudioAttributes(attributes), mStreams(streams) {}

    const std::string &getName() const { return mName; }
    volume_group_t getId() const { return mGroupId; }
    AttributesVector getAudioAttributes() const { return mAudioAttributes; }
    StreamTypeVector getStreamTypes() const { return mStreams; }

    status_t readFromParcel(const Parcel *parcel) override;
    status_t writeToParcel(Parcel *parcel) const override;

private:
    std::string mName;
    volume_group_t mGroupId = VOLUME_GROUP_NONE;
    AttributesVector mAudioAttributes;
    StreamTypeVector mStreams;
};

using AudioVolumeGroupVector = std::vector<AudioVolumeGroup>;

} // namespace android
