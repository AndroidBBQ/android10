/*
 * Copyright 2017 The Android Open Source Project
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

#ifndef ANDROID_UI_GRAPHICS_ENV_H
#define ANDROID_UI_GRAPHICS_ENV_H 1

#include <mutex>
#include <string>
#include <vector>

struct android_namespace_t;

namespace android {

struct NativeLoaderNamespace;

class GraphicsEnv {
public:
    enum Api {
        API_GL = 0,
        API_VK = 1,
    };

    enum Driver {
        NONE = 0,
        GL = 1,
        GL_UPDATED = 2,
        VULKAN = 3,
        VULKAN_UPDATED = 4,
        ANGLE = 5,
    };

    enum Stats {
        CPU_VULKAN_IN_USE = 0,
    };

private:
    struct GpuStats {
        std::string driverPackageName;
        std::string driverVersionName;
        uint64_t driverVersionCode;
        int64_t driverBuildTime;
        std::string appPackageName;
        int32_t vulkanVersion;
        Driver glDriverToLoad;
        Driver glDriverFallback;
        Driver vkDriverToLoad;
        Driver vkDriverFallback;
        bool glDriverToSend;
        bool vkDriverToSend;
        int64_t glDriverLoadingTime;
        int64_t vkDriverLoadingTime;

        GpuStats()
              : driverPackageName(""),
                driverVersionName(""),
                driverVersionCode(0),
                driverBuildTime(0),
                appPackageName(""),
                vulkanVersion(0),
                glDriverToLoad(Driver::NONE),
                glDriverFallback(Driver::NONE),
                vkDriverToLoad(Driver::NONE),
                vkDriverFallback(Driver::NONE),
                glDriverToSend(false),
                vkDriverToSend(false),
                glDriverLoadingTime(0),
                vkDriverLoadingTime(0) {}
    };

public:
    static GraphicsEnv& getInstance();

    // Check if the process is debuggable. It returns false except in any of the
    // following circumstances:
    // 1. ro.debuggable=1 (global debuggable enabled).
    // 2. android:debuggable="true" in the manifest for an individual app.
    // 3. An app which explicitly calls prctl(PR_SET_DUMPABLE, 1).
    // 4. GraphicsEnv calls prctl(PR_SET_DUMPABLE, 1) in the presence of
    //    <meta-data android:name="com.android.graphics.injectLayers.enable"
    //               android:value="true"/>
    //    in the application manifest.
    bool isDebuggable();

    // Set a search path for loading graphics drivers. The path is a list of
    // directories separated by ':'. A directory can be contained in a zip file
    // (drivers must be stored uncompressed and page aligned); such elements
    // in the search path must have a '!' after the zip filename, e.g.
    //     /data/app/com.example.driver/base.apk!/lib/arm64-v8a
    // Also set additional required sphal libraries to the linker for loading
    // graphics drivers. The string is a list of libraries separated by ':',
    // which is required by android_link_namespaces.
    void setDriverPathAndSphalLibraries(const std::string path, const std::string sphalLibraries);
    android_namespace_t* getDriverNamespace();
    void hintActivityLaunch();
    void setGpuStats(const std::string& driverPackageName, const std::string& driverVersionName,
                     uint64_t versionCode, int64_t driverBuildTime,
                     const std::string& appPackageName, const int32_t vulkanVersion);
    void setTargetStats(const Stats stats, const uint64_t value = 0);
    void setDriverToLoad(Driver driver);
    void setDriverLoaded(Api api, bool isDriverLoaded, int64_t driverLoadingTime);
    void sendGpuStatsLocked(Api api, bool isDriverLoaded, int64_t driverLoadingTime);

    bool shouldUseAngle(std::string appName);
    bool shouldUseAngle();
    // Set a search path for loading ANGLE libraries. The path is a list of
    // directories separated by ':'. A directory can be contained in a zip file
    // (libraries must be stored uncompressed and page aligned); such elements
    // in the search path must have a '!' after the zip filename, e.g.
    //     /system/app/ANGLEPrebuilt/ANGLEPrebuilt.apk!/lib/arm64-v8a
    void setAngleInfo(const std::string path, const std::string appName, std::string devOptIn,
                      const int rulesFd, const long rulesOffset, const long rulesLength);
    android_namespace_t* getAngleNamespace();
    std::string& getAngleAppName();

    void setLayerPaths(NativeLoaderNamespace* appNamespace, const std::string layerPaths);
    NativeLoaderNamespace* getAppNamespace();

    const std::string& getLayerPaths();

    void setDebugLayers(const std::string layers);
    void setDebugLayersGLES(const std::string layers);
    const std::string& getDebugLayers();
    const std::string& getDebugLayersGLES();

private:
    enum UseAngle { UNKNOWN, YES, NO };

    void* loadLibrary(std::string name);
    bool checkAngleRules(void* so);
    void updateUseAngle();
    bool linkDriverNamespaceLocked(android_namespace_t* vndkNamespace);

    GraphicsEnv() = default;
    std::string mDriverPath;
    std::string mSphalLibraries;
    std::mutex mStatsLock;
    GpuStats mGpuStats;
    std::string mAnglePath;
    std::string mAngleAppName;
    std::string mAngleDeveloperOptIn;
    std::vector<char> mRulesBuffer;
    UseAngle mUseAngle = UNKNOWN;
    std::string mDebugLayers;
    std::string mDebugLayersGLES;
    std::string mLayerPaths;
    std::mutex mNamespaceMutex;
    android_namespace_t* mDriverNamespace = nullptr;
    android_namespace_t* mAngleNamespace = nullptr;
    NativeLoaderNamespace* mAppNamespace = nullptr;
};

} // namespace android

#endif // ANDROID_UI_GRAPHICS_ENV_H
