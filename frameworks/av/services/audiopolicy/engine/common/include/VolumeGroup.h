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

#include <AudioPolicyManagerInterface.h>
#include <VolumeCurve.h>
#include <system/audio.h>
#include <utils/RefBase.h>
#include <HandleGenerator.h>
#include <string>
#include <vector>
#include <map>
#include <utils/Errors.h>

namespace android {

class VolumeGroup : public virtual RefBase, private HandleGenerator<uint32_t>
{
public:
    VolumeGroup(const std::string &name, int indexMin, int indexMax);
    std::string getName() const { return mName; }
    volume_group_t getId() const { return mId; }

    void add(const sp<VolumeCurve> &curve);

    VolumeCurves *getVolumeCurves() { return &mGroupVolumeCurves; }

    void addSupportedAttributes(const audio_attributes_t &attr);
    AttributesVector getSupportedAttributes() const { return mGroupVolumeCurves.getAttributes(); }

    void addSupportedStream(audio_stream_type_t stream);
    StreamTypeVector getStreamTypes() const { return mGroupVolumeCurves.getStreamTypes(); }

    void dump(String8 *dst, int spaces = 0) const;

private:
    const std::string mName;
    const volume_group_t mId;
    VolumeCurves mGroupVolumeCurves;
};

class VolumeGroupMap : public std::map<volume_group_t, sp<VolumeGroup> >
{
public:
    void dump(String8 *dst, int spaces = 0) const;
};

} // namespace android
