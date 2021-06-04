/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef ANDROID_ML_NN_RUNTIME_COMPILATION_BUILDER_H
#define ANDROID_ML_NN_RUNTIME_COMPILATION_BUILDER_H

#include "ExecutionPlan.h"
#include "NeuralNetworks.h"

#include <memory>
#include <vector>

namespace android {
namespace nn {

class BurstBuilder;
class Device;
class ExecutionBuilder;
class ModelBuilder;

class CompilationBuilder {
public:
    friend class ExecutionBuilder;  // TODO remove this

    // explicitDeviceList is true if the list of devices was provided explicitly
    // via the ANeuralNetworksModel_createForDevices API (which has certain
    // special semantics) and false otherwise.
    CompilationBuilder(const ModelBuilder* model,
                       const std::vector<std::shared_ptr<Device>>& devices,
                       bool explicitDeviceList = false);

    int setPreference(int32_t preference);

    int setPartitioning(uint32_t partitioning);

    int setCaching(const std::string& cacheDir, const uint8_t* token);

    int finish();

    int createExecution(ExecutionBuilder** execution);

    int createBurst(BurstBuilder** burst);

    const ExecutionPlan& forTest_getExecutionPlan() const { return mPlan; }

private:
    const ModelBuilder* mModel;

    ExecutionPlan mPlan;

    // Whether the application prefers to go fast or use low power for this execution.
    int32_t mPreference = ANEURALNETWORKS_PREFER_FAST_SINGLE_ANSWER;

    // See class DeviceManager.  When CompilationBuilder is
    // instantiated, we capture partitioning from DeviceManager; but
    // we can override this later.
    uint32_t mPartitioning;

    // Once the compilation has been finished, we should not allow further
    // modifications to the compilation.
    bool mFinished = false;

    // The set of devices that the partitioning algorithm operates on when
    // finish() is called.
    std::vector<std::shared_ptr<Device>> mDevices;

    // mExplicitDeviceList is true if the list of devices was provided
    // explicitly via the ANeuralNetworksModel_createForDevices API (which has
    // certain special semantics) and false otherwise.
    bool mExplicitDeviceList;

    // Compilation caching information.
    std::string mCacheDir;
    uint8_t mToken[ANEURALNETWORKS_BYTE_SIZE_OF_CACHE_TOKEN];
    bool mIsCacheInfoProvided = false;
};

} // namespace nn
} // namespace android

#endif // ANDROID_ML_NN_RUNTIME_COMPILATION_BUILDER_H
