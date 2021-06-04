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

#ifndef ANDROID_ML_NN_SAMPLE_DRIVER_SAMPLE_DRIVER_H
#define ANDROID_ML_NN_SAMPLE_DRIVER_SAMPLE_DRIVER_H

#include "CpuExecutor.h"
#include "HalInterfaces.h"
#include "NeuralNetworks.h"

#include <string>

namespace android {
namespace nn {
namespace sample_driver {

using ::android::hardware::MQDescriptorSync;
using HidlToken = hidl_array<uint8_t, ANEURALNETWORKS_BYTE_SIZE_OF_CACHE_TOKEN>;

// Base class used to create sample drivers for the NN HAL.  This class
// provides some implementation of the more common functions.
//
// Since these drivers simulate hardware, they must run the computations
// on the CPU.  An actual driver would not do that.
class SampleDriver : public IDevice {
   public:
    SampleDriver(const char* name,
                 const IOperationResolver* operationResolver = BuiltinOperationResolver::get())
        : mName(name), mOperationResolver(operationResolver) {
        android::nn::initVLogMask();
    }
    ~SampleDriver() override {}
    Return<void> getCapabilities(getCapabilities_cb cb) override;
    Return<void> getCapabilities_1_1(getCapabilities_1_1_cb cb) override;
    Return<void> getVersionString(getVersionString_cb cb) override;
    Return<void> getType(getType_cb cb) override;
    Return<void> getSupportedExtensions(getSupportedExtensions_cb) override;
    Return<void> getSupportedOperations(const V1_0::Model& model,
                                        getSupportedOperations_cb cb) override;
    Return<void> getSupportedOperations_1_1(const V1_1::Model& model,
                                            getSupportedOperations_1_1_cb cb) override;
    Return<void> getNumberOfCacheFilesNeeded(getNumberOfCacheFilesNeeded_cb cb) override;
    Return<ErrorStatus> prepareModel(const V1_0::Model& model,
                                     const sp<V1_0::IPreparedModelCallback>& callback) override;
    Return<ErrorStatus> prepareModel_1_1(const V1_1::Model& model, ExecutionPreference preference,
                                         const sp<V1_0::IPreparedModelCallback>& callback) override;
    Return<ErrorStatus> prepareModel_1_2(const V1_2::Model& model, ExecutionPreference preference,
                                         const hidl_vec<hidl_handle>& modelCache,
                                         const hidl_vec<hidl_handle>& dataCache,
                                         const HidlToken& token,
                                         const sp<V1_2::IPreparedModelCallback>& callback) override;
    Return<ErrorStatus> prepareModelFromCache(
            const hidl_vec<hidl_handle>& modelCache, const hidl_vec<hidl_handle>& dataCache,
            const HidlToken& token, const sp<V1_2::IPreparedModelCallback>& callback) override;
    Return<DeviceStatus> getStatus() override;

    // Starts and runs the driver service.  Typically called from main().
    // This will return only once the service shuts down.
    int run();

    CpuExecutor getExecutor() const { return CpuExecutor(mOperationResolver); }

   protected:
    std::string mName;
    const IOperationResolver* mOperationResolver;
};

class SamplePreparedModel : public IPreparedModel {
   public:
    SamplePreparedModel(const Model& model, const SampleDriver* driver)
        : mModel(model), mDriver(driver) {}
    ~SamplePreparedModel() override {}
    bool initialize();
    Return<ErrorStatus> execute(const Request& request,
                                const sp<V1_0::IExecutionCallback>& callback) override;
    Return<ErrorStatus> execute_1_2(const Request& request, MeasureTiming measure,
                                    const sp<V1_2::IExecutionCallback>& callback) override;
    Return<void> executeSynchronously(const Request& request, MeasureTiming measure,
                                      executeSynchronously_cb cb) override;
    Return<void> configureExecutionBurst(
            const sp<V1_2::IBurstCallback>& callback,
            const MQDescriptorSync<V1_2::FmqRequestDatum>& requestChannel,
            const MQDescriptorSync<V1_2::FmqResultDatum>& resultChannel,
            configureExecutionBurst_cb cb) override;

   private:
    Model mModel;
    const SampleDriver* mDriver;
    std::vector<RunTimePoolInfo> mPoolInfos;
};

}  // namespace sample_driver
}  // namespace nn
}  // namespace android

#endif  // ANDROID_ML_NN_SAMPLE_DRIVER_SAMPLE_DRIVER_H
