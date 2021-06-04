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

#include <system/audio.h>
#include <Volume.h>
#include <utils/Errors.h>
#include <utils/String8.h>
#include <vector>

namespace android {

class IVolumeCurves
{
public:
    virtual ~IVolumeCurves() = default;

    virtual void clearCurrentVolumeIndex() = 0;
    virtual void addCurrentVolumeIndex(audio_devices_t device, int index) = 0;
    virtual bool canBeMuted() const = 0;
    virtual int getVolumeIndexMin() const = 0;
    virtual int getVolumeIndex(audio_devices_t device) const = 0;
    virtual int getVolumeIndexMax() const = 0;
    virtual float volIndexToDb(device_category device, int indexInUi) const = 0;
    virtual bool hasVolumeIndexForDevice(audio_devices_t device) const = 0;
    virtual status_t initVolume(int indexMin, int indexMax) = 0;
    virtual std::vector<audio_attributes_t> getAttributes() const = 0;
    virtual std::vector<audio_stream_type_t> getStreamTypes() const = 0;
    virtual void dump(String8 *dst, int spaces = 0, bool curvePoints = false) const = 0;
};

} // namespace android
