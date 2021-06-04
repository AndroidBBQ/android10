/*
 * Copyright 2018 The Android Open Source Project
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

#define HWC2_INCLUDE_STRINGIFICATION
#define HWC2_USE_CPP11
#include <hardware/hwcomposer2.h>
#undef HWC2_INCLUDE_STRINGIFICATION
#undef HWC2_USE_CPP11

#include <unordered_set>

#include <android/hardware/power/1.3/IPower.h>
#include <utils/StrongPointer.h>

#include "DisplayIdentification.h"

namespace android {
namespace Hwc2 {

class PowerAdvisor {
public:
    virtual ~PowerAdvisor();

    virtual void setExpensiveRenderingExpected(DisplayId displayId, bool expected) = 0;
};

namespace impl {

namespace V1_3 = android::hardware::power::V1_3;

// PowerAdvisor is a wrapper around IPower HAL which takes into account the
// full state of the system when sending out power hints to things like the GPU.
class PowerAdvisor final : public Hwc2::PowerAdvisor {
public:
    PowerAdvisor();
    ~PowerAdvisor() override;

    void setExpensiveRenderingExpected(DisplayId displayId, bool expected) override;

private:
    sp<V1_3::IPower> getPowerHal();

    std::unordered_set<DisplayId> mExpensiveDisplays;
    bool mNotifiedExpensiveRendering = false;
    bool mReconnectPowerHal = false;
};

} // namespace impl
} // namespace Hwc2
} // namespace android
