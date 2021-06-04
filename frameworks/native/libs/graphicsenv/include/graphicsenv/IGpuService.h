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

#include <vector>

#include <binder/IInterface.h>
#include <cutils/compiler.h>
#include <graphicsenv/GpuStatsInfo.h>
#include <graphicsenv/GraphicsEnv.h>

namespace android {

/*
 * This class defines the Binder IPC interface for GPU-related queries and
 * control.
 */
class IGpuService : public IInterface {
public:
    DECLARE_META_INTERFACE(GpuService)

    // set GPU stats from GraphicsEnvironment.
    virtual void setGpuStats(const std::string& driverPackageName,
                             const std::string& driverVersionName, uint64_t driverVersionCode,
                             int64_t driverBuildTime, const std::string& appPackageName,
                             const int32_t vulkanVersion, GraphicsEnv::Driver driver,
                             bool isDriverLoaded, int64_t driverLoadingTime) = 0;

    // set target stats.
    virtual void setTargetStats(const std::string& appPackageName, const uint64_t driverVersionCode,
                                const GraphicsEnv::Stats stats, const uint64_t value = 0) = 0;

    // get GPU global stats from GpuStats module.
    virtual status_t getGpuStatsGlobalInfo(std::vector<GpuStatsGlobalInfo>* outStats) const = 0;

    // get GPU app stats from GpuStats module.
    virtual status_t getGpuStatsAppInfo(std::vector<GpuStatsAppInfo>* outStats) const = 0;
};

class BnGpuService : public BnInterface<IGpuService> {
public:
    enum IGpuServiceTag {
        SET_GPU_STATS = IBinder::FIRST_CALL_TRANSACTION,
        GET_GPU_STATS_GLOBAL_INFO,
        GET_GPU_STATS_APP_INFO,
        SET_TARGET_STATS,
        // Always append new enum to the end.
    };

    status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply,
                        uint32_t flags = 0) override;

protected:
    virtual status_t shellCommand(int in, int out, int err, std::vector<String16>& args) = 0;
};

} // namespace android
