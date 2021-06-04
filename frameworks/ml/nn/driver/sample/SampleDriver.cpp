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

#define LOG_TAG "SampleDriver"

#include "SampleDriver.h"

#include "CpuExecutor.h"
#include "ExecutionBurstServer.h"
#include "HalInterfaces.h"
#include "Tracing.h"
#include "ValidateHal.h"

#include <android-base/logging.h>
#include <hidl/LegacySupport.h>
#include <chrono>
#include <optional>
#include <thread>

namespace android {
namespace nn {
namespace sample_driver {

namespace {

using time_point = std::chrono::steady_clock::time_point;

auto now() {
    return std::chrono::steady_clock::now();
};

auto microsecondsDuration(decltype(now()) end, decltype(now()) start) {
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
};

}  // namespace

static const Timing kNoTiming = {.timeOnDevice = UINT64_MAX, .timeInDriver = UINT64_MAX};

Return<void> SampleDriver::getCapabilities(getCapabilities_cb cb) {
    NNTRACE_FULL(NNTRACE_LAYER_DRIVER, NNTRACE_PHASE_INITIALIZATION,
                 "SampleDriver::getCapabilities");
    return getCapabilities_1_2([&](ErrorStatus error, const V1_2::Capabilities& capabilities) {
        // TODO(dgross): Do we need to check compliantWithV1_0(capabilities)?
        cb(error, convertToV1_0(capabilities));
    });
}

Return<void> SampleDriver::getCapabilities_1_1(getCapabilities_1_1_cb cb) {
    NNTRACE_FULL(NNTRACE_LAYER_DRIVER, NNTRACE_PHASE_INITIALIZATION,
                 "SampleDriver::getCapabilities_1_1");
    return getCapabilities_1_2([&](ErrorStatus error, const V1_2::Capabilities& capabilities) {
        // TODO(dgross): Do we need to check compliantWithV1_1(capabilities)?
        cb(error, convertToV1_1(capabilities));
    });
}

Return<void> SampleDriver::getVersionString(getVersionString_cb cb) {
    NNTRACE_FULL(NNTRACE_LAYER_DRIVER, NNTRACE_PHASE_INITIALIZATION,
                 "SampleDriver::getVersionString");
    cb(ErrorStatus::NONE, "JUST_AN_EXAMPLE");
    return Void();
}

Return<void> SampleDriver::getType(getType_cb cb) {
    NNTRACE_FULL(NNTRACE_LAYER_DRIVER, NNTRACE_PHASE_INITIALIZATION, "SampleDriver::getType");
    cb(ErrorStatus::NONE, V1_2::DeviceType::CPU);
    return Void();
}

Return<void> SampleDriver::getSupportedExtensions(getSupportedExtensions_cb cb) {
    NNTRACE_FULL(NNTRACE_LAYER_DRIVER, NNTRACE_PHASE_INITIALIZATION,
                 "SampleDriver::getSupportedExtensions");
    cb(ErrorStatus::NONE, {/* No extensions. */});
    return Void();
}

Return<void> SampleDriver::getSupportedOperations(const V1_0::Model& model,
                                                  getSupportedOperations_cb cb) {
    NNTRACE_FULL(NNTRACE_LAYER_DRIVER, NNTRACE_PHASE_COMPILATION,
                 "SampleDriver::getSupportedOperations");
    if (!validateModel(model)) {
        VLOG(DRIVER) << "getSupportedOperations";
        std::vector<bool> supported;
        cb(ErrorStatus::INVALID_ARGUMENT, supported);
        return Void();
    }
    return getSupportedOperations_1_2(convertToV1_2(model), cb);
}

Return<void> SampleDriver::getSupportedOperations_1_1(const V1_1::Model& model,
                                                      getSupportedOperations_1_1_cb cb) {
    NNTRACE_FULL(NNTRACE_LAYER_DRIVER, NNTRACE_PHASE_COMPILATION,
                 "SampleDriver::getSupportedOperations_1_1");
    if (!validateModel(model)) {
        VLOG(DRIVER) << "getSupportedOperations_1_1";
        std::vector<bool> supported;
        cb(ErrorStatus::INVALID_ARGUMENT, supported);
        return Void();
    }
    return getSupportedOperations_1_2(convertToV1_2(model), cb);
}

Return<void> SampleDriver::getNumberOfCacheFilesNeeded(getNumberOfCacheFilesNeeded_cb cb) {
    NNTRACE_FULL(NNTRACE_LAYER_DRIVER, NNTRACE_PHASE_INITIALIZATION,
                 "SampleDriver::getNumberOfCacheFilesNeeded");
    // Set both numbers to be 0 for cache not supported.
    cb(ErrorStatus::NONE, /*numModelCache=*/0, /*numDataCache=*/0);
    return Void();
}

static void notify(const sp<V1_0::IPreparedModelCallback>& callback, const ErrorStatus& status,
                   const sp<SamplePreparedModel>& preparedModel) {
    callback->notify(status, preparedModel);
}

static void notify(const sp<V1_2::IPreparedModelCallback>& callback, const ErrorStatus& status,
                   const sp<SamplePreparedModel>& preparedModel) {
    callback->notify_1_2(status, preparedModel);
}

template <typename T_Model, typename T_IPreparedModelCallback>
Return<ErrorStatus> prepareModelBase(const T_Model& model, const SampleDriver* driver,
                                     ExecutionPreference preference,
                                     const sp<T_IPreparedModelCallback>& callback) {
    if (callback.get() == nullptr) {
        LOG(ERROR) << "invalid callback passed to prepareModelBase";
        return ErrorStatus::INVALID_ARGUMENT;
    }
    if (VLOG_IS_ON(DRIVER)) {
        VLOG(DRIVER) << "prepareModelBase";
        logModelToInfo(model);
    }
    if (!validateModel(model) || !validateExecutionPreference(preference)) {
        notify(callback, ErrorStatus::INVALID_ARGUMENT, nullptr);
        return ErrorStatus::INVALID_ARGUMENT;
    }

    // TODO: make asynchronous later
    sp<SamplePreparedModel> preparedModel = new SamplePreparedModel(convertToV1_2(model), driver);
    if (!preparedModel->initialize()) {
        notify(callback, ErrorStatus::INVALID_ARGUMENT, nullptr);
        return ErrorStatus::INVALID_ARGUMENT;
    }
    notify(callback, ErrorStatus::NONE, preparedModel);
    return ErrorStatus::NONE;
}

Return<ErrorStatus> SampleDriver::prepareModel(const V1_0::Model& model,
                                               const sp<V1_0::IPreparedModelCallback>& callback) {
    NNTRACE_FULL(NNTRACE_LAYER_DRIVER, NNTRACE_PHASE_COMPILATION, "SampleDriver::prepareModel");
    return prepareModelBase(model, this, ExecutionPreference::FAST_SINGLE_ANSWER, callback);
}

Return<ErrorStatus> SampleDriver::prepareModel_1_1(
        const V1_1::Model& model, ExecutionPreference preference,
        const sp<V1_0::IPreparedModelCallback>& callback) {
    NNTRACE_FULL(NNTRACE_LAYER_DRIVER, NNTRACE_PHASE_COMPILATION, "SampleDriver::prepareModel_1_1");
    return prepareModelBase(model, this, preference, callback);
}

Return<ErrorStatus> SampleDriver::prepareModel_1_2(
        const V1_2::Model& model, ExecutionPreference preference, const hidl_vec<hidl_handle>&,
        const hidl_vec<hidl_handle>&, const HidlToken&,
        const sp<V1_2::IPreparedModelCallback>& callback) {
    NNTRACE_FULL(NNTRACE_LAYER_DRIVER, NNTRACE_PHASE_COMPILATION, "SampleDriver::prepareModel_1_2");
    return prepareModelBase(model, this, preference, callback);
}

Return<ErrorStatus> SampleDriver::prepareModelFromCache(
        const hidl_vec<hidl_handle>&, const hidl_vec<hidl_handle>&, const HidlToken&,
        const sp<V1_2::IPreparedModelCallback>& callback) {
    NNTRACE_FULL(NNTRACE_LAYER_DRIVER, NNTRACE_PHASE_COMPILATION,
                 "SampleDriver::prepareModelFromCache");
    callback->notify_1_2(ErrorStatus::GENERAL_FAILURE, nullptr);
    return ErrorStatus::GENERAL_FAILURE;
}

Return<DeviceStatus> SampleDriver::getStatus() {
    NNTRACE_FULL(NNTRACE_LAYER_DRIVER, NNTRACE_PHASE_UNSPECIFIED,
                 "SampleDriver::getStatus");
    VLOG(DRIVER) << "getStatus()";
    return DeviceStatus::AVAILABLE;
}

int SampleDriver::run() {
    android::hardware::configureRpcThreadpool(4, true);
    if (registerAsService(mName) != android::OK) {
        LOG(ERROR) << "Could not register service";
        return 1;
    }
    android::hardware::joinRpcThreadpool();
    LOG(ERROR) << "Service exited!";
    return 1;
}

bool SamplePreparedModel::initialize() {
    return setRunTimePoolInfosFromHidlMemories(&mPoolInfos, mModel.pools);
}

static Return<void> notify(const sp<V1_0::IExecutionCallback>& callback, const ErrorStatus& status,
                           const hidl_vec<OutputShape>&, Timing) {
    return callback->notify(status);
}

static Return<void> notify(const sp<V1_2::IExecutionCallback>& callback, const ErrorStatus& status,
                           const hidl_vec<OutputShape>& outputShapes, Timing timing) {
    return callback->notify_1_2(status, outputShapes, timing);
}

// TODO(xusongw): Let callback notify actual output shape once dynamic output shape
//                is supported in CpuExecutor.
template <typename T_IExecutionCallback>
void asyncExecute(const Request& request, MeasureTiming measure, time_point driverStart,
                  const Model& model, const SampleDriver& driver,
                  const std::vector<RunTimePoolInfo>& poolInfos,
                  const sp<T_IExecutionCallback>& callback) {
    NNTRACE_FULL(NNTRACE_LAYER_DRIVER, NNTRACE_PHASE_INPUTS_AND_OUTPUTS,
                 "SampleDriver::asyncExecute");
    std::vector<RunTimePoolInfo> requestPoolInfos;
    if (!setRunTimePoolInfosFromHidlMemories(&requestPoolInfos, request.pools)) {
        notify(callback, ErrorStatus::GENERAL_FAILURE, {}, kNoTiming);
        return;
    }

    NNTRACE_FULL_SWITCH(NNTRACE_LAYER_DRIVER, NNTRACE_PHASE_EXECUTION,
                        "SampleDriver::asyncExecute");
    CpuExecutor executor = driver.getExecutor();
    time_point driverEnd, deviceStart, deviceEnd;
    if (measure == MeasureTiming::YES) deviceStart = now();
    int n = executor.run(model, request, poolInfos, requestPoolInfos);
    if (measure == MeasureTiming::YES) deviceEnd = now();
    VLOG(DRIVER) << "executor.run returned " << n;
    ErrorStatus executionStatus = convertResultCodeToErrorStatus(n);
    hidl_vec<OutputShape> outputShapes = executor.getOutputShapes();
    Return<void> returned;
    if (measure == MeasureTiming::YES && executionStatus == ErrorStatus::NONE) {
        driverEnd = now();
        Timing timing = {.timeOnDevice = uint64_t(microsecondsDuration(deviceEnd, deviceStart)),
                         .timeInDriver = uint64_t(microsecondsDuration(driverEnd, driverStart))};
        VLOG(DRIVER) << "SampleDriver::asyncExecute timing = " << toString(timing);
        returned = notify(callback, executionStatus, outputShapes, timing);
    } else {
        returned = notify(callback, executionStatus, outputShapes, kNoTiming);
    }
    if (!returned.isOk()) {
        LOG(ERROR) << " hidl callback failed to return properly: " << returned.description();
    }
}

template <typename T_IExecutionCallback>
Return<ErrorStatus> executeBase(const Request& request, MeasureTiming measure, const Model& model,
                                const SampleDriver& driver,
                                const std::vector<RunTimePoolInfo>& poolInfos,
                                const sp<T_IExecutionCallback>& callback) {
    NNTRACE_FULL(NNTRACE_LAYER_DRIVER, NNTRACE_PHASE_EXECUTION, "SampleDriver::executeBase");
    VLOG(DRIVER) << "executeBase(" << SHOW_IF_DEBUG(toString(request)) << ")";

    time_point driverStart;
    if (measure == MeasureTiming::YES) driverStart = now();

    if (callback.get() == nullptr) {
        LOG(ERROR) << "invalid callback passed to executeBase";
        return ErrorStatus::INVALID_ARGUMENT;
    }
    if (!validateRequest(request, model)) {
        notify(callback, ErrorStatus::INVALID_ARGUMENT, {}, kNoTiming);
        return ErrorStatus::INVALID_ARGUMENT;
    }

    // This thread is intentionally detached because the sample driver service
    // is expected to live forever.
    std::thread([&model, &driver, &poolInfos, request, measure, driverStart, callback] {
        asyncExecute(request, measure, driverStart, model, driver, poolInfos, callback);
    })
            .detach();

    return ErrorStatus::NONE;
}

Return<ErrorStatus> SamplePreparedModel::execute(const Request& request,
                                                 const sp<V1_0::IExecutionCallback>& callback) {
    return executeBase(request, MeasureTiming::NO, mModel, *mDriver, mPoolInfos, callback);
}

Return<ErrorStatus> SamplePreparedModel::execute_1_2(const Request& request, MeasureTiming measure,
                                                     const sp<V1_2::IExecutionCallback>& callback) {
    return executeBase(request, measure, mModel, *mDriver, mPoolInfos, callback);
}

Return<void> SamplePreparedModel::executeSynchronously(const Request& request,
                                                       MeasureTiming measure,
                                                       executeSynchronously_cb cb) {
    NNTRACE_FULL(NNTRACE_LAYER_DRIVER, NNTRACE_PHASE_EXECUTION,
                 "SampleDriver::executeSynchronously");
    VLOG(DRIVER) << "executeSynchronously(" << SHOW_IF_DEBUG(toString(request)) << ")";

    time_point driverStart, driverEnd, deviceStart, deviceEnd;
    if (measure == MeasureTiming::YES) driverStart = now();

    if (!validateRequest(request, mModel)) {
        cb(ErrorStatus::INVALID_ARGUMENT, {}, kNoTiming);
        return Void();
    }

    NNTRACE_FULL_SWITCH(NNTRACE_LAYER_DRIVER, NNTRACE_PHASE_INPUTS_AND_OUTPUTS,
                        "SampleDriver::executeSynchronously");
    std::vector<RunTimePoolInfo> requestPoolInfos;
    if (!setRunTimePoolInfosFromHidlMemories(&requestPoolInfos, request.pools)) {
        cb(ErrorStatus::GENERAL_FAILURE, {}, kNoTiming);
        return Void();
    }

    NNTRACE_FULL_SWITCH(NNTRACE_LAYER_DRIVER, NNTRACE_PHASE_EXECUTION,
                        "SampleDriver::executeSynchronously");
    CpuExecutor executor = mDriver->getExecutor();
    if (measure == MeasureTiming::YES) deviceStart = now();
    int n = executor.run(mModel, request, mPoolInfos, requestPoolInfos);
    if (measure == MeasureTiming::YES) deviceEnd = now();
    VLOG(DRIVER) << "executor.run returned " << n;
    ErrorStatus executionStatus = convertResultCodeToErrorStatus(n);
    hidl_vec<OutputShape> outputShapes = executor.getOutputShapes();
    if (measure == MeasureTiming::YES && executionStatus == ErrorStatus::NONE) {
        driverEnd = now();
        Timing timing = {.timeOnDevice = uint64_t(microsecondsDuration(deviceEnd, deviceStart)),
                         .timeInDriver = uint64_t(microsecondsDuration(driverEnd, driverStart))};
        VLOG(DRIVER) << "executeSynchronously timing = " << toString(timing);
        cb(executionStatus, outputShapes, timing);
    } else {
        cb(executionStatus, outputShapes, kNoTiming);
    }
    return Void();
}

// BurstExecutorWithCache maps hidl_memory when it is first seen, and preserves
// the mapping until either (1) the memory is freed in the runtime, or (2) the
// burst object is destroyed. This allows for subsequent executions operating on
// pools that have been used before to reuse the mapping instead of mapping and
// unmapping the memory on each execution.
class BurstExecutorWithCache : public ExecutionBurstServer::IBurstExecutorWithCache {
   public:
    BurstExecutorWithCache(const Model& model, const SampleDriver* driver,
                           const std::vector<RunTimePoolInfo>& poolInfos)
        : mModel(model), mDriver(driver), mModelPoolInfos(poolInfos) {}

    bool isCacheEntryPresent(int32_t slot) const override {
        const auto it = mMemoryCache.find(slot);
        return (it != mMemoryCache.end()) && it->second.has_value();
    }

    void addCacheEntry(const hidl_memory& memory, int32_t slot) override {
        mMemoryCache[slot] = RunTimePoolInfo::createFromHidlMemory(memory);
    }

    void removeCacheEntry(int32_t slot) override { mMemoryCache.erase(slot); }

    std::tuple<ErrorStatus, hidl_vec<OutputShape>, Timing> execute(
            const Request& request, const std::vector<int32_t>& slots,
            MeasureTiming measure) override {
        NNTRACE_FULL(NNTRACE_LAYER_DRIVER, NNTRACE_PHASE_EXECUTION,
                     "BurstExecutorWithCache::execute");

        time_point driverStart, driverEnd, deviceStart, deviceEnd;
        if (measure == MeasureTiming::YES) driverStart = now();

        // ensure all relevant pools are valid
        if (!std::all_of(slots.begin(), slots.end(),
                         [this](int32_t slot) { return isCacheEntryPresent(slot); })) {
            return {ErrorStatus::INVALID_ARGUMENT, {}, kNoTiming};
        }

        // finish the request object (for validation)
        hidl_vec<hidl_memory> pools(slots.size());
        std::transform(slots.begin(), slots.end(), pools.begin(),
                       [this](int32_t slot) { return mMemoryCache[slot]->getHidlMemory(); });
        Request fullRequest = request;
        fullRequest.pools = std::move(pools);

        // validate request object against the model
        if (!validateRequest(fullRequest, mModel)) {
            return {ErrorStatus::INVALID_ARGUMENT, {}, kNoTiming};
        }

        // select relevant entries from cache
        std::vector<RunTimePoolInfo> requestPoolInfos;
        requestPoolInfos.reserve(slots.size());
        std::transform(slots.begin(), slots.end(), std::back_inserter(requestPoolInfos),
                       [this](int32_t slot) { return *mMemoryCache[slot]; });

        // execution
        CpuExecutor executor = mDriver->getExecutor();
        if (measure == MeasureTiming::YES) deviceStart = now();
        int n = executor.run(mModel, request, mModelPoolInfos, requestPoolInfos);
        if (measure == MeasureTiming::YES) deviceEnd = now();
        VLOG(DRIVER) << "executor.run returned " << n;
        ErrorStatus executionStatus = convertResultCodeToErrorStatus(n);
        hidl_vec<OutputShape> outputShapes = executor.getOutputShapes();
        if (measure == MeasureTiming::YES && executionStatus == ErrorStatus::NONE) {
            driverEnd = now();
            Timing timing = {
                    .timeOnDevice = uint64_t(microsecondsDuration(deviceEnd, deviceStart)),
                    .timeInDriver = uint64_t(microsecondsDuration(driverEnd, driverStart))};
            VLOG(DRIVER) << "BurstExecutorWithCache::execute timing = " << toString(timing);
            return std::make_tuple(executionStatus, outputShapes, timing);
        } else {
            return std::make_tuple(executionStatus, outputShapes, kNoTiming);
        }
    }

   private:
    const Model mModel;
    const SampleDriver* const mDriver;
    const std::vector<RunTimePoolInfo> mModelPoolInfos;
    std::map<int32_t, std::optional<RunTimePoolInfo>> mMemoryCache;  // cached requestPoolInfos
};

Return<void> SamplePreparedModel::configureExecutionBurst(
        const sp<V1_2::IBurstCallback>& callback,
        const MQDescriptorSync<V1_2::FmqRequestDatum>& requestChannel,
        const MQDescriptorSync<V1_2::FmqResultDatum>& resultChannel,
        configureExecutionBurst_cb cb) {
    NNTRACE_FULL(NNTRACE_LAYER_DRIVER, NNTRACE_PHASE_EXECUTION,
                 "SampleDriver::configureExecutionBurst");

    // Alternatively, the burst could be configured via:
    // const sp<V1_2::IBurstContext> burst =
    //         ExecutionBurstServer::create(callback, requestChannel,
    //                                      resultChannel, this);
    //
    // However, this alternative representation does not include a memory map
    // caching optimization, and adds overhead.
    const std::shared_ptr<BurstExecutorWithCache> executorWithCache =
            std::make_shared<BurstExecutorWithCache>(mModel, mDriver, mPoolInfos);
    const sp<V1_2::IBurstContext> burst = ExecutionBurstServer::create(
            callback, requestChannel, resultChannel, executorWithCache);

    if (burst == nullptr) {
        cb(ErrorStatus::GENERAL_FAILURE, {});
    } else {
        cb(ErrorStatus::NONE, burst);
    }

    return Void();
}

} // namespace sample_driver
} // namespace nn
} // namespace android
