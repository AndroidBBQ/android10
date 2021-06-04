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

#undef LOG_TAG
#define LOG_TAG "PowerAdvisor"

#include <cinttypes>

#include <utils/Log.h>
#include <utils/Mutex.h>

#include "PowerAdvisor.h"

namespace android {
namespace Hwc2 {

PowerAdvisor::~PowerAdvisor() = default;

namespace impl {

namespace V1_0 = android::hardware::power::V1_0;
using V1_3::PowerHint;

PowerAdvisor::~PowerAdvisor() = default;

PowerAdvisor::PowerAdvisor() = default;

void PowerAdvisor::setExpensiveRenderingExpected(DisplayId displayId, bool expected) {
    if (expected) {
        mExpensiveDisplays.insert(displayId);
    } else {
        mExpensiveDisplays.erase(displayId);
    }

    const bool expectsExpensiveRendering = !mExpensiveDisplays.empty();
    if (mNotifiedExpensiveRendering != expectsExpensiveRendering) {
        const sp<V1_3::IPower> powerHal = getPowerHal();
        if (powerHal == nullptr) {
            return;
        }
        auto ret = powerHal->powerHintAsync_1_3(PowerHint::EXPENSIVE_RENDERING,
                                                expectsExpensiveRendering);
        // If Power HAL 1.3 was available previously but now fails,
        // it may restart, so attempt to reconnect next time
        if (!ret.isOk()) {
            mReconnectPowerHal = true;
            return;
        }
        mNotifiedExpensiveRendering = expectsExpensiveRendering;
    }
}

sp<V1_3::IPower> PowerAdvisor::getPowerHal() {
    static sp<V1_3::IPower> sPowerHal_1_3 = nullptr;
    static bool sHasPowerHal_1_3 = true;

    if (mReconnectPowerHal) {
        sPowerHal_1_3 = nullptr;
        mReconnectPowerHal = false;
    }

    // Power HAL 1.3 is not guaranteed to be available, thus we need to query
    // Power HAL 1.0 first and try to cast it to Power HAL 1.3.
    // Power HAL 1.0 is always available, thus if we fail to query it, it means
    // Power HAL is not available temporarily and we should retry later. However,
    // if Power HAL 1.0 is available and we can't cast it to Power HAL 1.3,
    // it means Power HAL 1.3 is not available at all, so we should stop trying.
    if (sHasPowerHal_1_3 && sPowerHal_1_3 == nullptr) {
        sp<V1_0::IPower> powerHal_1_0 = V1_0::IPower::getService();
        if (powerHal_1_0 != nullptr) {
            // Try to cast to Power HAL 1.3
            sPowerHal_1_3 =  V1_3::IPower::castFrom(powerHal_1_0);
            if (sPowerHal_1_3 == nullptr) {
                ALOGW("No Power HAL 1.3 service in system");
                sHasPowerHal_1_3 = false;
            } else {
                ALOGI("Loaded Power HAL 1.3 service");
            }
        }
    }
    return sPowerHal_1_3;
}

} // namespace impl
} // namespace Hwc2
} // namespace android
