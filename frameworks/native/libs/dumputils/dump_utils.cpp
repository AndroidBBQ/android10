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
#include <set>

#include <android-base/file.h>
#include <android-base/properties.h>
#include <android-base/stringprintf.h>
#include <android-base/strings.h>
#include <android/hidl/manager/1.0/IServiceManager.h>
#include <dumputils/dump_utils.h>
#include <log/log.h>

/* list of native processes to include in the native dumps */
// This matches the /proc/pid/exe link instead of /proc/pid/cmdline.
static const char* native_processes_to_dump[] = {
        "/system/bin/audioserver",
        "/system/bin/cameraserver",
        "/system/bin/drmserver",
        "/system/bin/mediadrmserver",
        "/system/bin/mediaextractor", // media.extractor
        "/system/bin/mediametrics", // media.metrics
        "/system/bin/mediaserver",
        "/system/bin/netd",
        "/system/bin/vold",
        "/system/bin/sdcard",
        "/system/bin/statsd",
        "/system/bin/surfaceflinger",
        "/system/bin/vehicle_network_service",
        "/vendor/bin/hw/android.hardware.media.omx@1.0-service", // media.codec
        "/apex/com.android.media.swcodec/bin/mediaswcodec", // media.swcodec
        NULL,
};

/* list of hal interface to dump containing process during native dumps */
static const char* hal_interfaces_to_dump[] {
        "android.hardware.audio@2.0::IDevicesFactory",
        "android.hardware.audio@4.0::IDevicesFactory",
        "android.hardware.audio@5.0::IDevicesFactory",
        "android.hardware.biometrics.face@1.0::IBiometricsFace",
        "android.hardware.bluetooth@1.0::IBluetoothHci",
        "android.hardware.camera.provider@2.4::ICameraProvider",
        "android.hardware.drm@1.0::IDrmFactory",
        "android.hardware.graphics.allocator@2.0::IAllocator",
        "android.hardware.graphics.composer@2.1::IComposer",
        "android.hardware.health@2.0::IHealth",
        "android.hardware.media.c2@1.0::IComponentStore",
        "android.hardware.media.omx@1.0::IOmx",
        "android.hardware.media.omx@1.0::IOmxStore",
        "android.hardware.power@1.3::IPower",
        "android.hardware.power.stats@1.0::IPowerStats",
        "android.hardware.sensors@1.0::ISensors",
        "android.hardware.thermal@2.0::IThermal",
        "android.hardware.vr@1.0::IVr",
        "android.hardware.automotive.audiocontrol@1.0::IAudioControl",
        "android.hardware.automotive.vehicle@2.0::IVehicle",
        "android.hardware.automotive.evs@1.0::IEvsCamera",
        NULL,
};

/* list of extra hal interfaces to dump containing process during native dumps */
// This is filled when dumpstate is called.
static std::set<const std::string> extra_hal_interfaces_to_dump;

static void read_extra_hals_to_dump_from_property() {
    // extra hals to dump are already filled
    if (extra_hal_interfaces_to_dump.size() > 0) {
        return;
    }
    std::string value = android::base::GetProperty("ro.dump.hals.extra", "");
    std::vector<std::string> tokens = android::base::Split(value, ",");
    for (const auto &token : tokens) {
        std::string trimmed_token = android::base::Trim(token);
        if (trimmed_token.length() == 0) {
            continue;
        }
        extra_hal_interfaces_to_dump.insert(trimmed_token);
    }
}

// check if interface is included in either default hal list or extra hal list
bool should_dump_hal_interface(const std::string& interface) {
    for (const char** i = hal_interfaces_to_dump; *i; i++) {
        if (interface == *i) {
            return true;
        }
    }
    return extra_hal_interfaces_to_dump.find(interface) != extra_hal_interfaces_to_dump.end();
}

bool should_dump_native_traces(const char* path) {
    for (const char** p = native_processes_to_dump; *p; p++) {
        if (!strcmp(*p, path)) {
            return true;
        }
    }
    return false;
}

std::set<int> get_interesting_hal_pids() {
    using android::hidl::manager::V1_0::IServiceManager;
    using android::sp;
    using android::hardware::Return;

    sp<IServiceManager> manager = IServiceManager::getService();
    std::set<int> pids;

    read_extra_hals_to_dump_from_property();

    Return<void> ret = manager->debugDump([&](auto& hals) {
        for (const auto &info : hals) {
            if (info.pid == static_cast<int>(IServiceManager::PidConstant::NO_PID)) {
                continue;
            }

            if (!should_dump_hal_interface(info.interfaceName)) {
                continue;
            }

            pids.insert(info.pid);
        }
    });

    if (!ret.isOk()) {
        ALOGE("Could not get list of HAL PIDs: %s\n", ret.description().c_str());
    }

    return pids; // whether it was okay or not
}

bool IsZygote(int pid) {
    std::string cmdline;
    if (!android::base::ReadFileToString(android::base::StringPrintf("/proc/%d/cmdline", pid),
                                         &cmdline)) {
        return true;
    }

    // cmdline has embedded nulls; only consider argv[0].
    cmdline = std::string(cmdline.c_str());

    return cmdline == "zygote" || cmdline == "zygote64" || cmdline == "usap32" ||
            cmdline == "usap64";
}
