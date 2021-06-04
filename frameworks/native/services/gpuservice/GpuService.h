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

#ifndef ANDROID_GPUSERVICE_H
#define ANDROID_GPUSERVICE_H

#include <binder/IInterface.h>
#include <cutils/compiler.h>
#include <graphicsenv/GpuStatsInfo.h>
#include <graphicsenv/IGpuService.h>
#include <serviceutils/PriorityDumper.h>

#include <mutex>
#include <vector>

namespace android {

class GpuStats;

class GpuService : public BnGpuService, public PriorityDumper {
public:
    static const char* const SERVICE_NAME ANDROID_API;

    GpuService() ANDROID_API;

protected:
    status_t shellCommand(int in, int out, int err, std::vector<String16>& args) override;

private:
    /*
     * IGpuService interface
     */
    void setGpuStats(const std::string& driverPackageName, const std::string& driverVersionName,
                     uint64_t driverVersionCode, int64_t driverBuildTime,
                     const std::string& appPackageName, const int32_t vulkanVersion,
                     GraphicsEnv::Driver driver, bool isDriverLoaded,
                     int64_t driverLoadingTime) override;
    status_t getGpuStatsGlobalInfo(std::vector<GpuStatsGlobalInfo>* outStats) const override;
    status_t getGpuStatsAppInfo(std::vector<GpuStatsAppInfo>* outStats) const override;
    void setTargetStats(const std::string& appPackageName, const uint64_t driverVersionCode,
                        const GraphicsEnv::Stats stats, const uint64_t value) override;

    /*
     * IBinder interface
     */
    status_t dump(int fd, const Vector<String16>& args) override { return priorityDump(fd, args); }

    /*
     * Debugging & dumpsys
     */
    status_t dumpCritical(int fd, const Vector<String16>& /*args*/, bool asProto) override {
        return doDump(fd, Vector<String16>(), asProto);
    }

    status_t dumpAll(int fd, const Vector<String16>& args, bool asProto) override {
        return doDump(fd, args, asProto);
    }

    status_t doDump(int fd, const Vector<String16>& args, bool asProto);

    /*
     * Attributes
     */
    std::unique_ptr<GpuStats> mGpuStats;
};

} // namespace android

#endif // ANDROID_GPUSERVICE_H
