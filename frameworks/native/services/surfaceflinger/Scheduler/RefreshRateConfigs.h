/*
 * Copyright 2019 The Android Open Source Project
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

#include <algorithm>
#include <numeric>

#include "android-base/stringprintf.h"

#include "DisplayHardware/HWComposer.h"
#include "Scheduler/SchedulerUtils.h"

namespace android {
namespace scheduler {

/**
 * This class is used to encapsulate configuration for refresh rates. It holds information
 * about available refresh rates on the device, and the mapping between the numbers and human
 * readable names.
 */
class RefreshRateConfigs {
public:
    // Enum to indicate which vsync rate to run at. Default is the old 60Hz, and performance
    // is the new 90Hz. Eventually we want to have a way for vendors to map these in the configs.
    enum class RefreshRateType { DEFAULT, PERFORMANCE };

    struct RefreshRate {
        // This config ID corresponds to the position of the config in the vector that is stored
        // on the device.
        int configId;
        // Human readable name of the refresh rate.
        std::string name;
        // Refresh rate in frames per second, rounded to the nearest integer.
        uint32_t fps = 0;
        // Vsync period in nanoseconds.
        nsecs_t vsyncPeriod;
        // Hwc config Id (returned from HWC2::Display::Config::getId())
        hwc2_config_t hwcId;
    };

    // Returns true if this device is doing refresh rate switching. This won't change at runtime.
    bool refreshRateSwitchingSupported() const { return mRefreshRateSwitchingSupported; }

    // Returns the refresh rate map. This map won't be modified at runtime, so it's safe to access
    // from multiple threads. This can only be called if refreshRateSwitching() returns true.
    // TODO(b/122916473): Get this information from configs prepared by vendors, instead of
    // baking them in.
    const std::map<RefreshRateType, RefreshRate>& getRefreshRateMap() const {
        LOG_ALWAYS_FATAL_IF(!mRefreshRateSwitchingSupported);
        return mRefreshRateMap;
    }

    const RefreshRate& getRefreshRateFromType(RefreshRateType type) const {
        if (!mRefreshRateSwitchingSupported) {
            return getCurrentRefreshRate().second;
        } else {
            auto refreshRate = mRefreshRateMap.find(type);
            LOG_ALWAYS_FATAL_IF(refreshRate == mRefreshRateMap.end());
            return refreshRate->second;
        }
    }

    std::pair<RefreshRateType, const RefreshRate&> getCurrentRefreshRate() const {
        int currentConfig = mCurrentConfig;
        if (mRefreshRateSwitchingSupported) {
            for (const auto& [type, refresh] : mRefreshRateMap) {
                if (refresh.configId == currentConfig) {
                    return {type, refresh};
                }
            }
            LOG_ALWAYS_FATAL();
        }
        return {RefreshRateType::DEFAULT, mRefreshRates[currentConfig]};
    }

    const RefreshRate& getRefreshRateFromConfigId(int configId) const {
        LOG_ALWAYS_FATAL_IF(configId >= mRefreshRates.size());
        return mRefreshRates[configId];
    }

    RefreshRateType getRefreshRateTypeFromHwcConfigId(hwc2_config_t hwcId) const {
        if (!mRefreshRateSwitchingSupported) return RefreshRateType::DEFAULT;

        for (const auto& [type, refreshRate] : mRefreshRateMap) {
            if (refreshRate.hwcId == hwcId) {
                return type;
            }
        }

        return RefreshRateType::DEFAULT;
    }

    void setCurrentConfig(int config) {
        LOG_ALWAYS_FATAL_IF(config >= mRefreshRates.size());
        mCurrentConfig = config;
    }

    struct InputConfig {
        hwc2_config_t hwcId = 0;
        nsecs_t vsyncPeriod = 0;
    };

    RefreshRateConfigs(bool refreshRateSwitching, const std::vector<InputConfig>& configs,
                       int currentConfig) {
        init(refreshRateSwitching, configs, currentConfig);
    }

    RefreshRateConfigs(bool refreshRateSwitching,
                       const std::vector<std::shared_ptr<const HWC2::Display::Config>>& configs,
                       int currentConfig) {
        std::vector<InputConfig> inputConfigs;
        for (const auto& config : configs) {
            inputConfigs.push_back({config->getId(), config->getVsyncPeriod()});
        }
        init(refreshRateSwitching, inputConfigs, currentConfig);
    }

private:
    void init(bool refreshRateSwitching, const std::vector<InputConfig>& configs,
              int currentConfig) {
        mRefreshRateSwitchingSupported = refreshRateSwitching;
        LOG_ALWAYS_FATAL_IF(configs.empty());
        LOG_ALWAYS_FATAL_IF(currentConfig >= configs.size());
        mCurrentConfig = currentConfig;

        auto buildRefreshRate = [&](int configId) -> RefreshRate {
            const nsecs_t vsyncPeriod = configs[configId].vsyncPeriod;
            const float fps = 1e9 / vsyncPeriod;
            return {configId, base::StringPrintf("%2.ffps", fps), static_cast<uint32_t>(fps),
                    vsyncPeriod, configs[configId].hwcId};
        };

        for (int i = 0; i < configs.size(); ++i) {
            mRefreshRates.push_back(buildRefreshRate(i));
        }

        if (!mRefreshRateSwitchingSupported) return;

        auto findDefaultAndPerfConfigs = [&]() -> std::optional<std::pair<int, int>> {
            if (configs.size() < 2) {
                return {};
            }

            std::vector<const RefreshRate*> sortedRefreshRates;
            for (const auto& refreshRate : mRefreshRates) {
                sortedRefreshRates.push_back(&refreshRate);
            }
            std::sort(sortedRefreshRates.begin(), sortedRefreshRates.end(),
                      [](const RefreshRate* refreshRate1, const RefreshRate* refreshRate2) {
                          return refreshRate1->vsyncPeriod > refreshRate2->vsyncPeriod;
                      });

            // When the configs are ordered by the resync rate, we assume that
            // the first one is DEFAULT and the second one is PERFORMANCE,
            // i.e. the higher rate.
            if (sortedRefreshRates[0]->vsyncPeriod == 0 ||
                sortedRefreshRates[1]->vsyncPeriod == 0) {
                return {};
            }

            return std::pair<int, int>(sortedRefreshRates[0]->configId,
                                       sortedRefreshRates[1]->configId);
        };

        auto defaultAndPerfConfigs = findDefaultAndPerfConfigs();
        if (!defaultAndPerfConfigs) {
            mRefreshRateSwitchingSupported = false;
            return;
        }

        mRefreshRateMap[RefreshRateType::DEFAULT] = mRefreshRates[defaultAndPerfConfigs->first];
        mRefreshRateMap[RefreshRateType::PERFORMANCE] =
                mRefreshRates[defaultAndPerfConfigs->second];
    }

    // Whether this device is doing refresh rate switching or not. This must not change after this
    // object is initialized.
    bool mRefreshRateSwitchingSupported;
    // The list of refresh rates, indexed by display config ID. This must not change after this
    // object is initialized.
    std::vector<RefreshRate> mRefreshRates;
    // The mapping of refresh rate type to RefreshRate. This must not change after this object is
    // initialized.
    std::map<RefreshRateType, RefreshRate> mRefreshRateMap;
    // The ID of the current config. This will change at runtime. This is set by SurfaceFlinger on
    // the main thread, and read by the Scheduler (and other objects) on other threads, so it's
    // atomic.
    std::atomic<int> mCurrentConfig;
};

} // namespace scheduler
} // namespace android
