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

#define LOG_TAG "APM::AudioPolicyEngine/VolumeGroup"
//#define LOG_NDEBUG 0

#include "VolumeGroup.h"
#include <media/TypeConverter.h>
#include <utils/String8.h>
#include <cstdint>
#include <string>

#include <log/log.h>


namespace android {

//
// VolumeGroup implementation
//
VolumeGroup::VolumeGroup(const std::string &name, int indexMin, int indexMax) :
    mName(name), mId(static_cast<volume_group_t>(HandleGenerator<uint32_t>::getNextHandle())),
    mGroupVolumeCurves(VolumeCurves(indexMin, indexMax))
{
}

void VolumeGroup::dump(String8 *dst, int spaces) const
{
    dst->appendFormat("\n%*s-%s (id: %d)\n", spaces, "", mName.c_str(), mId);
    mGroupVolumeCurves.dump(dst, spaces + 2, true);
    mGroupVolumeCurves.dump(dst, spaces + 2, false);
    dst->appendFormat("\n");
}

void VolumeGroup::add(const sp<VolumeCurve> &curve)
{
    mGroupVolumeCurves.add(curve);
}

void VolumeGroup::addSupportedAttributes(const audio_attributes_t &attr)
{
    mGroupVolumeCurves.addAttributes(attr);
}

void VolumeGroup::addSupportedStream(audio_stream_type_t stream)
{
    mGroupVolumeCurves.addStreamType(stream);
}

//
// VolumeGroupMap implementation
//
void VolumeGroupMap::dump(String8 *dst, int spaces) const
{
    dst->appendFormat("\n%*sVolume Groups dump:", spaces, "");
    for (const auto &iter : *this) {
        iter.second->dump(dst, spaces + 2);
    }
}

} // namespace android

