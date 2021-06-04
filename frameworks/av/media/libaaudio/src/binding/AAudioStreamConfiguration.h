/*
 * Copyright 2016 The Android Open Source Project
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

#ifndef ANDROID_BINDING_AAUDIO_STREAM_CONFIGURATION_H
#define ANDROID_BINDING_AAUDIO_STREAM_CONFIGURATION_H

#include <stdint.h>

#include <aaudio/AAudio.h>
#include <binder/Parcel.h>
#include <binder/Parcelable.h>
#include "core/AAudioStreamParameters.h"

using android::status_t;
using android::Parcel;
using android::Parcelable;

namespace aaudio {

class AAudioStreamConfiguration : public AAudioStreamParameters, public Parcelable {
public:
    AAudioStreamConfiguration();
    virtual ~AAudioStreamConfiguration();

    virtual status_t writeToParcel(Parcel* parcel) const override;

    virtual status_t readFromParcel(const Parcel* parcel) override;
};

} /* namespace aaudio */

#endif //ANDROID_BINDING_AAUDIO_STREAM_CONFIGURATION_H
