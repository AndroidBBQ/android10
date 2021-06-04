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

#include "VersionedInterfaces.h"

#include "Callbacks.h"
#include "ExecutionBurstController.h"
#include "Tracing.h"
#include "Utils.h"

#include <android-base/logging.h>
#include <android-base/scopeguard.h>
#include <android-base/thread_annotations.h>
#include <functional>
#include <type_traits>

namespace android {
namespace nn {

// anonymous namespace
namespace {

using HidlToken = hidl_array<uint8_t, static_cast<uint32_t>(Constant::BYTE_SIZE_OF_CACHE_TOKEN)>;

const Timing kBadTiming = {.timeOnDevice = UINT64_MAX, .timeInDriver = UINT64_MAX};

void sendFailureMessage(const sp<IPreparedModelCallback>& cb) {
    cb->notify(ErrorStatus::GENERAL_FAILURE, nullptr);
}

void sendFailureMessage(const sp<PreparedModelCallback>& cb) {
    sendFailureMessage(static_cast<sp<IPreparedModelCallback>>(cb));
}

void sendFailureMessage(const sp<IExecutionCallback>& cb) {
    cb->notify(ErrorStatus::GENERAL_FAILURE);
}

void sendFailureMessage(const sp<ExecutionCallback>& cb) {
    sendFailureMessage(static_cast<sp<IExecutionCallback>>(cb));
}

// This class is thread safe
template <typename ICallback>
class DeathHandler : public hardware::hidl_death_recipient {
   public:
    void serviceDied(uint64_t /*cookie*/, const wp<hidl::base::V1_0::IBase>& /*who*/) override {
        LOG(ERROR) << "DeathHandler::serviceDied -- service unexpectedly died!";
        std::lock_guard<std::mutex> hold(mMutex);
        std::for_each(mCallbacks.begin(), mCallbacks.end(),
                      [](const auto& cb) { sendFailureMessage(cb); });
    }

    [[nodiscard]] base::ScopeGuard<std::function<void()>> protectCallback(
            const sp<ICallback>& callback) {
        registerCallback(callback);
        return ::android::base::make_scope_guard(
                [this, callback] { unregisterCallback(callback); });
    }

    private : void registerCallback(const sp<ICallback>& callback) {
        std::lock_guard<std::mutex> hold(mMutex);
        mCallbacks.push_back(callback);
    }

    void unregisterCallback(const sp<ICallback>& callback) {
        std::lock_guard<std::mutex> hold(mMutex);
        mCallbacks.erase(std::remove(mCallbacks.begin(), mCallbacks.end(), callback),
                         mCallbacks.end());
    }

    std::mutex mMutex;
    std::vector<sp<ICallback>> mCallbacks GUARDED_BY(mMutex);
};

}  // anonymous namespace

class IDeviceDeathHandler : public DeathHandler<IPreparedModelCallback> {};
class IPreparedModelDeathHandler : public DeathHandler<IExecutionCallback> {};

static std::shared_ptr<VersionedIPreparedModel> makeVersionedIPreparedModel(
        sp<V1_0::IPreparedModel> preparedModel) {
    // verify input
    if (!preparedModel) {
        LOG(ERROR) << "makeVersionedIPreparedModel -- passed invalid preparedModel object.";
        return nullptr;
    }

    // create death handler object
    sp<IPreparedModelDeathHandler> deathHandler = new (std::nothrow) IPreparedModelDeathHandler();
    if (!deathHandler) {
        LOG(ERROR) << "makeVersionedIPreparedModel -- Failed to create IPreparedModelDeathHandler.";
        return nullptr;
    }

    // linkToDeath registers a callback that will be invoked on service death to
    // proactively handle service crashes. If the linkToDeath call fails,
    // asynchronous calls are susceptible to hangs if the service crashes before
    // providing the response.
    const Return<bool> ret = preparedModel->linkToDeath(deathHandler, 0);
    if (!ret.isOk() || ret != true) {
        LOG(ERROR) << "makeVersionedIPreparedModel -- Failed to register a death recipient for the "
                      "IPreparedModel object.";
        return nullptr;
    }

    // return a valid VersionedIPreparedModel object
    return std::make_shared<VersionedIPreparedModel>(std::move(preparedModel),
                                                     std::move(deathHandler));
}

VersionedIPreparedModel::VersionedIPreparedModel(sp<V1_0::IPreparedModel> preparedModel,
                                                 sp<IPreparedModelDeathHandler> deathHandler)
    : mPreparedModelV1_0(std::move(preparedModel)),
      mPreparedModelV1_2(V1_2::IPreparedModel::castFrom(mPreparedModelV1_0).withDefault(nullptr)),
      mDeathHandler(std::move(deathHandler)) {}

VersionedIPreparedModel::~VersionedIPreparedModel() {
    // It is safe to ignore any errors resulting from this unlinkToDeath call
    // because the VersionedIPreparedModel object is already being destroyed and
    // its underlying IPreparedModel object is no longer being used by the NN
    // runtime.
    mPreparedModelV1_0->unlinkToDeath(mDeathHandler).isOk();
}

ErrorStatus VersionedIPreparedModel::execute(const Request& request, MeasureTiming measure,
                                             const sp<ExecutionCallback>& callback) {
    const auto scoped = mDeathHandler->protectCallback(callback);

    if (mPreparedModelV1_2 != nullptr) {
        Return<ErrorStatus> ret = mPreparedModelV1_2->execute_1_2(request, measure, callback);
        if (!ret.isOk()) {
            sendFailureMessage(callback);
            LOG(ERROR) << "execute_1_2 failure: " << ret.description();
            return ErrorStatus::GENERAL_FAILURE;
        }
        if (ret != ErrorStatus::NONE) {
            sendFailureMessage(callback);
            LOG(ERROR) << "execute_1_2 returned " << toString(static_cast<ErrorStatus>(ret));
            return static_cast<ErrorStatus>(ret);
        }
        callback->wait();
        return static_cast<ErrorStatus>(ret);
    } else if (mPreparedModelV1_0 != nullptr) {
        Return<ErrorStatus> ret = mPreparedModelV1_0->execute(request, callback);
        if (!ret.isOk()) {
            sendFailureMessage(callback);
            LOG(ERROR) << "execute failure: " << ret.description();
            return ErrorStatus::GENERAL_FAILURE;
        }
        if (ret != ErrorStatus::NONE) {
            sendFailureMessage(callback);
            LOG(ERROR) << "execute returned " << toString(static_cast<ErrorStatus>(ret));
            return static_cast<ErrorStatus>(ret);
        }
        callback->wait();
        return static_cast<ErrorStatus>(ret);
    } else {
        sendFailureMessage(callback);
        LOG(ERROR) << "execute called with no preparedModel";
        return ErrorStatus::GENERAL_FAILURE;
    }
}

std::tuple<ErrorStatus, hidl_vec<OutputShape>, Timing>
VersionedIPreparedModel::executeSynchronously(const Request& request, MeasureTiming measure) {
    const std::tuple<ErrorStatus, hidl_vec<OutputShape>, Timing> kFailure = {
            ErrorStatus::GENERAL_FAILURE, {}, kBadTiming};

    if (mPreparedModelV1_2 != nullptr) {
        std::tuple<ErrorStatus, hidl_vec<OutputShape>, Timing> result;
        Return<void> ret = mPreparedModelV1_2->executeSynchronously(
                request, measure,
                [&result](ErrorStatus error, const hidl_vec<OutputShape>& outputShapes,
                          const Timing& timing) {
                    result = std::make_tuple(error, outputShapes, timing);
                });
        if (!ret.isOk()) {
            LOG(ERROR) << "executeSynchronously failure: " << ret.description();
            return kFailure;
        }
        return result;
    } else {
        // Simulate synchronous execution.
        sp<ExecutionCallback> callback = new ExecutionCallback();
        ErrorStatus ret = execute(request, measure, callback);
        if (ret != ErrorStatus::NONE) {
            return {ret, {}, kBadTiming};
        }
        callback->wait();
        // callback->getOutputShapes() will always return an empty hidl vector.
        // callback->getTiming() will always return values indicating no measurement.
        return {callback->getStatus(), callback->getOutputShapes(), callback->getTiming()};
    }
}

std::shared_ptr<ExecutionBurstController> VersionedIPreparedModel::configureExecutionBurst(
        bool blocking) const {
    if (mPreparedModelV1_2 != nullptr) {
        return ExecutionBurstController::create(mPreparedModelV1_2, blocking);
    } else {
        return nullptr;
    }
}

bool VersionedIPreparedModel::operator==(nullptr_t) const {
    return mPreparedModelV1_0 == nullptr;
}

bool VersionedIPreparedModel::operator!=(nullptr_t) const {
    return mPreparedModelV1_0 != nullptr;
}

std::shared_ptr<VersionedIDevice> VersionedIDevice::create(std::string serviceName,
                                                           sp<V1_0::IDevice> device) {
    auto core = Core::create(std::move(device));
    if (!core.has_value()) {
        LOG(ERROR) << "VersionedIDevice::create -- Failed to create Core.";
        return nullptr;
    }

    // return a valid VersionedIDevice object
    return std::make_shared<VersionedIDevice>(std::move(serviceName), std::move(core.value()));
}

VersionedIDevice::VersionedIDevice(std::string serviceName, Core core)
    : mServiceName(std::move(serviceName)), mCore(std::move(core)) {}

std::optional<VersionedIDevice::Core> VersionedIDevice::Core::create(sp<V1_0::IDevice> device) {
    // verify input
    if (!device) {
        LOG(ERROR) << "VersionedIDevice::Core::create -- passed invalid device object.";
        return {};
    }

    // create death handler object
    sp<IDeviceDeathHandler> deathHandler = new (std::nothrow) IDeviceDeathHandler();
    if (!deathHandler) {
        LOG(ERROR) << "VersionedIDevice::Core::create -- Failed to create IDeviceDeathHandler.";
        return {};
    }

    // linkToDeath registers a callback that will be invoked on service death to
    // proactively handle service crashes. If the linkToDeath call fails,
    // asynchronous calls are susceptible to hangs if the service crashes before
    // providing the response.
    const Return<bool> ret = device->linkToDeath(deathHandler, 0);
    if (!ret.isOk() || ret != true) {
        LOG(ERROR)
                << "VersionedIDevice::Core::create -- Failed to register a death recipient for the "
                   "IDevice object.";
        return {};
    }

    // return a valid Core object
    return Core(std::move(device), std::move(deathHandler));
}

// HIDL guarantees all V1_1 interfaces inherit from their corresponding V1_0 interfaces.
VersionedIDevice::Core::Core(sp<V1_0::IDevice> device, sp<IDeviceDeathHandler> deathHandler)
    : mDeviceV1_0(std::move(device)),
      mDeviceV1_1(V1_1::IDevice::castFrom(mDeviceV1_0).withDefault(nullptr)),
      mDeviceV1_2(V1_2::IDevice::castFrom(mDeviceV1_0).withDefault(nullptr)),
      mDeathHandler(std::move(deathHandler)) {}

VersionedIDevice::Core::~Core() {
    if (mDeathHandler != nullptr) {
        CHECK(mDeviceV1_0 != nullptr);
        // It is safe to ignore any errors resulting from this unlinkToDeath call
        // because the VersionedIDevice::Core object is already being destroyed and
        // its underlying IDevice object is no longer being used by the NN runtime.
        mDeviceV1_0->unlinkToDeath(mDeathHandler).isOk();
    }
}

VersionedIDevice::Core::Core(Core&& other) noexcept
    : mDeviceV1_0(std::move(other.mDeviceV1_0)),
      mDeviceV1_1(std::move(other.mDeviceV1_1)),
      mDeviceV1_2(std::move(other.mDeviceV1_2)),
      mDeathHandler(std::move(other.mDeathHandler)) {
    other.mDeathHandler = nullptr;
}

VersionedIDevice::Core& VersionedIDevice::Core::operator=(Core&& other) noexcept {
    if (this != &other) {
        mDeviceV1_0 = std::move(other.mDeviceV1_0);
        mDeviceV1_1 = std::move(other.mDeviceV1_1);
        mDeviceV1_2 = std::move(other.mDeviceV1_2);
        mDeathHandler = std::move(other.mDeathHandler);
        other.mDeathHandler = nullptr;
    }
    return *this;
}

template <typename T_IDevice>
std::pair<sp<T_IDevice>, sp<IDeviceDeathHandler>> VersionedIDevice::Core::getDeviceAndDeathHandler()
        const {
    return {getDevice<T_IDevice>(), mDeathHandler};
}

template <typename T_IDevice, typename T_Callback>
Return<ErrorStatus> callProtected(
        const char* context, const std::function<Return<ErrorStatus>(const sp<T_IDevice>&)>& fn,
        const sp<T_IDevice>& device, const sp<T_Callback>& callback,
        const sp<IDeviceDeathHandler>& deathHandler) {
    const auto scoped = deathHandler->protectCallback(callback);
    Return<ErrorStatus> ret = fn(device);
    // Suppose there was a transport error.  We have the following cases:
    // 1. Either not due to a dead device, or due to a device that was
    //    already dead at the time of the call to protectCallback().  In
    //    this case, the callback was never signalled.
    // 2. Due to a device that died after the call to protectCallback() but
    //    before fn() completed.  In this case, the callback was (or will
    //    be) signalled by the deathHandler.
    // Furthermore, what if there was no transport error, but the ErrorStatus is
    // other than NONE?  We'll conservatively signal the callback anyway, just in
    // case the driver was sloppy and failed to do so.
    if (!ret.isOk() || ret != ErrorStatus::NONE) {
        // What if the deathHandler has signalled or will signal the callback?
        // This is fine -- we're permitted to signal multiple times; and we're
        // sending the same signal that the deathHandler does.
        //
        // What if the driver signalled the callback?  Then this signal is
        // ignored.

        if (ret.isOk()) {
            LOG(ERROR) << context << " returned " << toString(static_cast<ErrorStatus>(ret));
        } else {
            LOG(ERROR) << context << " failure: " << ret.description();
        }
        sendFailureMessage(callback);
    }
    callback->wait();
    return ret;
}
template <typename T_Return, typename T_IDevice>
Return<T_Return> callProtected(const char*,
                               const std::function<Return<T_Return>(const sp<T_IDevice>&)>& fn,
                               const sp<T_IDevice>& device, const std::nullptr_t&,
                               const sp<IDeviceDeathHandler>&) {
    return fn(device);
}

template <typename T_Return, typename T_IDevice, typename T_Callback>
Return<T_Return> VersionedIDevice::recoverable(
        const char* context, const std::function<Return<T_Return>(const sp<T_IDevice>&)>& fn,
        const T_Callback& callback) const EXCLUDES(mMutex) {
    CHECK_EQ(callback == nullptr, (std::is_same_v<T_Callback, std::nullptr_t>));

    sp<T_IDevice> device;
    sp<IDeviceDeathHandler> deathHandler;
    std::tie(device, deathHandler) = getDeviceAndDeathHandler<T_IDevice>();

    Return<T_Return> ret = callProtected(context, fn, device, callback, deathHandler);

    if (ret.isDeadObject()) {
        {
            std::unique_lock lock(mMutex);
            // It's possible that another device has already done the recovery.
            // It's harmless but wasteful for us to do so in this case.
            auto pingReturn = mCore.getDevice<T_IDevice>()->ping();
            if (pingReturn.isDeadObject()) {
                VLOG(DRIVER) << "VersionedIDevice::recoverable(" << context << ") -- Recovering "
                             << mServiceName;
                sp<V1_0::IDevice> recoveredDevice = V1_0::IDevice::tryGetService(mServiceName);
                if (recoveredDevice == nullptr) {
                    VLOG(DRIVER) << "VersionedIDevice::recoverable got a null IDEVICE for "
                                 << mServiceName;
                    return ret;
                }

                auto core = Core::create(std::move(recoveredDevice));
                if (!core.has_value()) {
                    LOG(ERROR) << "VersionedIDevice::recoverable -- Failed to create Core.";
                    return ret;
                }

                mCore = std::move(core.value());
            } else {
                VLOG(DRIVER) << "VersionedIDevice::recoverable(" << context
                             << ") -- Someone else recovered " << mServiceName;
                // Might still have a transport error, which we need to check
                // before pingReturn goes out of scope.
                (void)pingReturn.isOk();
            }
            std::tie(device, deathHandler) = mCore.getDeviceAndDeathHandler<T_IDevice>();
        }
        ret = callProtected(context, fn, device, callback, deathHandler);
        // It's possible that the device died again, but we're only going to
        // attempt recovery once per call to recoverable().
    }
    return ret;
}

std::pair<ErrorStatus, Capabilities> VersionedIDevice::getCapabilities() {
    const std::pair<ErrorStatus, Capabilities> kFailure = {ErrorStatus::GENERAL_FAILURE, {}};
    std::pair<ErrorStatus, Capabilities> result;

    if (getDevice<V1_2::IDevice>() != nullptr) {
        NNTRACE_FULL(NNTRACE_LAYER_IPC, NNTRACE_PHASE_INITIALIZATION, "getCapabilities_1_2");
        Return<void> ret = recoverable<void, V1_2::IDevice>(
                __FUNCTION__, [&result](const sp<V1_2::IDevice>& device) {
                    return device->getCapabilities_1_2(
                            [&result](ErrorStatus error, const Capabilities& capabilities) {
                                result = std::make_pair(error, capabilities);
                            });
                });
        if (!ret.isOk()) {
            LOG(ERROR) << "getCapabilities_1_2 failure: " << ret.description();
            return {ErrorStatus::GENERAL_FAILURE, {}};
        }
    } else if (getDevice<V1_1::IDevice>() != nullptr) {
        NNTRACE_FULL(NNTRACE_LAYER_IPC, NNTRACE_PHASE_INITIALIZATION, "getCapabilities_1_1");
        Return<void> ret = recoverable<void, V1_1::IDevice>(
                __FUNCTION__, [&result](const sp<V1_1::IDevice>& device) {
                    return device->getCapabilities_1_1(
                            [&result](ErrorStatus error, const V1_1::Capabilities& capabilities) {
                                // Time taken to convert capabilities is trivial
                                result = std::make_pair(error, convertToV1_2(capabilities));
                            });
                });
        if (!ret.isOk()) {
            LOG(ERROR) << "getCapabilities_1_1 failure: " << ret.description();
            return kFailure;
        }
    } else if (getDevice<V1_0::IDevice>() != nullptr) {
        NNTRACE_FULL(NNTRACE_LAYER_IPC, NNTRACE_PHASE_INITIALIZATION, "getCapabilities");
        Return<void> ret = recoverable<void, V1_0::IDevice>(
                __FUNCTION__, [&result](const sp<V1_0::IDevice>& device) {
                    return device->getCapabilities(
                            [&result](ErrorStatus error, const V1_0::Capabilities& capabilities) {
                                // Time taken to convert capabilities is trivial
                                result = std::make_pair(error, convertToV1_2(capabilities));
                            });
                });
        if (!ret.isOk()) {
            LOG(ERROR) << "getCapabilities failure: " << ret.description();
            return kFailure;
        }
    } else {
        LOG(ERROR) << "Device not available!";
        return {ErrorStatus::DEVICE_UNAVAILABLE, {}};
    }

    return result;
}

std::pair<ErrorStatus, hidl_vec<Extension>> VersionedIDevice::getSupportedExtensions() {
    const std::pair<ErrorStatus, hidl_vec<Extension>> kFailure = {ErrorStatus::GENERAL_FAILURE, {}};
    NNTRACE_FULL(NNTRACE_LAYER_IPC, NNTRACE_PHASE_COMPILATION, "getSupportedExtensions");
    if (getDevice<V1_2::IDevice>() != nullptr) {
        std::pair<ErrorStatus, hidl_vec<Extension>> result;
        Return<void> ret = recoverable<void, V1_2::IDevice>(
                __FUNCTION__, [&result](const sp<V1_2::IDevice>& device) {
                    return device->getSupportedExtensions(
                            [&result](ErrorStatus error, const hidl_vec<Extension>& extensions) {
                                result = std::make_pair(error, extensions);
                            });
                });
        if (!ret.isOk()) {
            LOG(ERROR) << "getSupportedExtensions failure: " << ret.description();
            return kFailure;
        }
        return result;
    } else if (getDevice<V1_0::IDevice>() != nullptr) {
        return {ErrorStatus::NONE, {/* No extensions. */}};
    } else {
        LOG(ERROR) << "Device not available!";
        return {ErrorStatus::DEVICE_UNAVAILABLE, {}};
    }
}

std::pair<ErrorStatus, hidl_vec<bool>> VersionedIDevice::getSupportedOperations(
        const Model& model, IModelSlicer* slicer) {
    const std::pair<ErrorStatus, hidl_vec<bool>> kFailure = {ErrorStatus::GENERAL_FAILURE, {}};
    std::pair<ErrorStatus, hidl_vec<bool>> result;

    auto noneSupported = [&model] {
        hidl_vec<bool> supported(model.operations.size());
        std::fill(supported.begin(), supported.end(), false);
        return std::make_pair(ErrorStatus::NONE, std::move(supported));
    };

    auto remappedResult = [&model](const std::pair<ErrorStatus, hidl_vec<bool>>& result,
                                   const std::function<uint32_t(uint32_t)>&
                                           submodelOperationIndexToModelOperationIndex) {
        const ErrorStatus status = result.first;
        const hidl_vec<bool>& supported = result.second;
        hidl_vec<bool> remappedSupported(model.operations.size());
        std::fill(remappedSupported.begin(), remappedSupported.end(), false);
        for (size_t i = 0; i < supported.size(); ++i) {
            if (supported[i]) {
                remappedSupported[submodelOperationIndexToModelOperationIndex(i)] = true;
            }
        }
        return std::make_pair(status, std::move(remappedSupported));
    };

    if (getDevice<V1_2::IDevice>() != nullptr) {
        NNTRACE_FULL(NNTRACE_LAYER_IPC, NNTRACE_PHASE_COMPILATION, "getSupportedOperations_1_2");
        Return<void> ret = recoverable<void, V1_2::IDevice>(
                __FUNCTION__, [&model, &result](const sp<V1_2::IDevice>& device) {
                    return device->getSupportedOperations_1_2(
                            model, [&result](ErrorStatus error, const hidl_vec<bool>& supported) {
                                result = std::make_pair(error, supported);
                            });
                });
        if (!ret.isOk()) {
            LOG(ERROR) << "getSupportedOperations_1_2 failure: " << ret.description();
            return kFailure;
        }
        return result;
    }

    if (getDevice<V1_1::IDevice>() != nullptr) {
        const bool compliant = compliantWithV1_1(model);
        if (compliant || slicer) {
            V1_1::Model model11;
            std::function<uint32_t(uint32_t)> submodelOperationIndexToModelOperationIndex;
            if (compliant) {
                model11 = convertToV1_1(model);
            } else {
                const auto slice11 = slicer->getSliceV1_1();
                if (!slice11.has_value()) {
                    return noneSupported();
                }
                std::tie(model11, submodelOperationIndexToModelOperationIndex) = *slice11;
            }
            NNTRACE_FULL(NNTRACE_LAYER_IPC, NNTRACE_PHASE_COMPILATION,
                         "getSupportedOperations_1_1");
            Return<void> ret = recoverable<void, V1_1::IDevice>(
                    __FUNCTION__, [&model11, &result](const sp<V1_1::IDevice>& device) {
                        return device->getSupportedOperations_1_1(
                                model11,
                                [&result](ErrorStatus error, const hidl_vec<bool>& supported) {
                                    result = std::make_pair(error, supported);
                                });
                    });
            if (!ret.isOk()) {
                LOG(ERROR) << "getSupportedOperations_1_1 failure: " << ret.description();
                return kFailure;
            }
            if (!compliant) {
                return remappedResult(result, submodelOperationIndexToModelOperationIndex);
            }
        }
        return result;
    }

    if (getDevice<V1_0::IDevice>() != nullptr) {
        const bool compliant = compliantWithV1_0(model);
        if (compliant || slicer) {
            V1_0::Model model10;
            std::function<uint32_t(uint32_t)> submodelOperationIndexToModelOperationIndex;
            if (compliant) {
                model10 = convertToV1_0(model);
            } else {
                const auto slice10 = slicer->getSliceV1_0();
                if (!slice10.has_value()) {
                    return noneSupported();
                }
                std::tie(model10, submodelOperationIndexToModelOperationIndex) = *slice10;
            }
            NNTRACE_FULL(NNTRACE_LAYER_IPC, NNTRACE_PHASE_COMPILATION, "getSupportedOperations");
            Return<void> ret = recoverable<void, V1_0::IDevice>(
                    __FUNCTION__, [&model10, &result](const sp<V1_0::IDevice>& device) {
                        return device->getSupportedOperations(
                                model10,
                                [&result](ErrorStatus error, const hidl_vec<bool>& supported) {
                                    result = std::make_pair(error, supported);
                                });
                    });
            if (!ret.isOk()) {
                LOG(ERROR) << "getSupportedOperations failure: " << ret.description();
                return kFailure;
            }
            if (!compliant) {
                return remappedResult(result, submodelOperationIndexToModelOperationIndex);
            }
        }
        return result;
    }

    return kFailure;
}

std::pair<ErrorStatus, std::shared_ptr<VersionedIPreparedModel>> VersionedIDevice::prepareModel(
        const Model& model, ExecutionPreference preference, const hidl_vec<hidl_handle>& modelCache,
        const hidl_vec<hidl_handle>& dataCache, const HidlToken& token) {
    const std::pair<ErrorStatus, std::shared_ptr<VersionedIPreparedModel>> kFailure = {
            ErrorStatus::GENERAL_FAILURE, nullptr};

    const sp<PreparedModelCallback> callback = new (std::nothrow) PreparedModelCallback();
    if (callback == nullptr) {
        LOG(ERROR) << "prepareModel failed to create callback object";
        return kFailure;
    }

    // If 1.2 device, try preparing model
    if (getDevice<V1_2::IDevice>() != nullptr) {
        const Return<ErrorStatus> ret = recoverable<ErrorStatus, V1_2::IDevice>(
                __FUNCTION__,
                [&model, &preference, &modelCache, &dataCache, &token,
                 &callback](const sp<V1_2::IDevice>& device) {
                    return device->prepareModel_1_2(model, preference, modelCache, dataCache, token,
                                                    callback);
                },
                callback);
        if (!ret.isOk()) {
            LOG(ERROR) << "prepareModel_1_2 failure: " << ret.description();
            return kFailure;
        }
        if (ret != ErrorStatus::NONE) {
            LOG(ERROR) << "prepareModel_1_2 returned " << toString(static_cast<ErrorStatus>(ret));
            return kFailure;
        }
        callback->wait();
        return {callback->getStatus(), makeVersionedIPreparedModel(callback->getPreparedModel())};
    }

    // If 1.1 device, try preparing model (requires conversion)
    if (getDevice<V1_1::IDevice>() != nullptr) {
        bool compliant = false;
        V1_1::Model model11;
        {
            // Attribute time spent in model inspection and conversion to
            // Runtime, as the time may be substantial (0.03ms for mobilenet,
            // but could be larger for other models).
            NNTRACE_FULL_SUBTRACT(NNTRACE_LAYER_RUNTIME, NNTRACE_PHASE_COMPILATION,
                                  "VersionedIDevice::prepareModel_1_1");
            compliant = compliantWithV1_1(model);
            if (compliant) {
                model11 = convertToV1_1(model);  // copy is elided
            }
        }
        if (compliant) {
            const Return<ErrorStatus> ret = recoverable<ErrorStatus, V1_1::IDevice>(
                    __FUNCTION__,
                    [&model11, &preference, &callback](const sp<V1_1::IDevice>& device) {
                        return device->prepareModel_1_1(model11, preference, callback);
                    },
                    callback);
            if (!ret.isOk()) {
                LOG(ERROR) << "prepareModel_1_1 failure: " << ret.description();
                return kFailure;
            }
            if (ret != ErrorStatus::NONE) {
                LOG(ERROR) << "prepareModel_1_1 returned "
                           << toString(static_cast<ErrorStatus>(ret));
                return kFailure;
            }
            callback->wait();
            return {callback->getStatus(),
                    makeVersionedIPreparedModel(callback->getPreparedModel())};
        }

        LOG(ERROR) << "Could not handle prepareModel_1_1!";
        return kFailure;
    }

    // If 1.0 device, try preparing model (requires conversion)
    if (getDevice<V1_0::IDevice>() != nullptr) {
        bool compliant = false;
        V1_0::Model model10;
        {
            // Attribute time spent in model inspection and conversion to
            // Runtime, as the time may be substantial (0.03ms for mobilenet,
            // but could be larger for other models).
            NNTRACE_FULL_SUBTRACT(NNTRACE_LAYER_RUNTIME, NNTRACE_PHASE_COMPILATION,
                                  "VersionedIDevice::prepareModel");
            compliant = compliantWithV1_0(model);
            if (compliant) {
                model10 = convertToV1_0(model);  // copy is elided
            }
        }
        if (compliant) {
            const Return<ErrorStatus> ret = recoverable<ErrorStatus, V1_0::IDevice>(
                    __FUNCTION__,
                    [&model10, &callback](const sp<V1_0::IDevice>& device) {
                        return device->prepareModel(model10, callback);
                    },
                    callback);
            if (!ret.isOk()) {
                LOG(ERROR) << "prepareModel failure: " << ret.description();
                return kFailure;
            }
            if (ret != ErrorStatus::NONE) {
                LOG(ERROR) << "prepareModel returned " << toString(static_cast<ErrorStatus>(ret));
                return kFailure;
            }
            callback->wait();
            return {callback->getStatus(),
                    makeVersionedIPreparedModel(callback->getPreparedModel())};
        }

        LOG(ERROR) << "Could not handle prepareModel!";
        return kFailure;
    }

    // Return error because there is no valid device
    LOG(ERROR) << "prepareModel called with no device";
    return kFailure;
}

std::pair<ErrorStatus, std::shared_ptr<VersionedIPreparedModel>>
VersionedIDevice::prepareModelFromCache(const hidl_vec<hidl_handle>& modelCache,
                                        const hidl_vec<hidl_handle>& dataCache,
                                        const HidlToken& token) {
    const std::pair<ErrorStatus, std::shared_ptr<VersionedIPreparedModel>> kFailure = {
            ErrorStatus::GENERAL_FAILURE, nullptr};

    const sp<PreparedModelCallback> callback = new (std::nothrow) PreparedModelCallback();
    if (callback == nullptr) {
        LOG(ERROR) << "prepareModelFromCache failed to create callback object";
        return kFailure;
    }

    if (getDevice<V1_2::IDevice>() != nullptr) {
        const Return<ErrorStatus> ret = recoverable<ErrorStatus, V1_2::IDevice>(
                __FUNCTION__,
                [&modelCache, &dataCache, &token, &callback](const sp<V1_2::IDevice>& device) {
                    return device->prepareModelFromCache(modelCache, dataCache, token, callback);
                },
                callback);
        if (!ret.isOk()) {
            LOG(ERROR) << "prepareModelFromCache failure: " << ret.description();
            return kFailure;
        }
        if (ret != ErrorStatus::NONE) {
            LOG(ERROR) << "prepareModelFromCache returned "
                       << toString(static_cast<ErrorStatus>(ret));
            return kFailure;
        }
        callback->wait();
        return {callback->getStatus(), makeVersionedIPreparedModel(callback->getPreparedModel())};
    }

    if (getDevice<V1_1::IDevice>() != nullptr || getDevice<V1_0::IDevice>() != nullptr) {
        LOG(ERROR) << "prepareModelFromCache called on V1_1 or V1_0 device";
        return kFailure;
    }

    LOG(ERROR) << "prepareModelFromCache called with no device";
    return kFailure;
}

DeviceStatus VersionedIDevice::getStatus() {
    if (getDevice<V1_0::IDevice>() == nullptr) {
        LOG(ERROR) << "Device not available!";
        return DeviceStatus::UNKNOWN;
    }

    Return<DeviceStatus> ret = recoverable<DeviceStatus, V1_0::IDevice>(
            __FUNCTION__, [](const sp<V1_0::IDevice>& device) { return device->getStatus(); });

    if (!ret.isOk()) {
        LOG(ERROR) << "getStatus failure: " << ret.description();
        return DeviceStatus::UNKNOWN;
    }
    return static_cast<DeviceStatus>(ret);
}

int64_t VersionedIDevice::getFeatureLevel() {
    constexpr int64_t kFailure = -1;

    if (getDevice<V1_2::IDevice>() != nullptr) {
        return __ANDROID_API_Q__;
    } else if (getDevice<V1_1::IDevice>() != nullptr) {
        return __ANDROID_API_P__;
    } else if (getDevice<V1_0::IDevice>() != nullptr) {
        return __ANDROID_API_O_MR1__;
    } else {
        LOG(ERROR) << "Device not available!";
        return kFailure;
    }
}

int32_t VersionedIDevice::getType() const {
    constexpr int32_t kFailure = -1;
    std::pair<ErrorStatus, DeviceType> result;

    if (getDevice<V1_2::IDevice>() != nullptr) {
        Return<void> ret = recoverable<void, V1_2::IDevice>(
                __FUNCTION__, [&result](const sp<V1_2::IDevice>& device) {
                    return device->getType([&result](ErrorStatus error, DeviceType deviceType) {
                        result = std::make_pair(error, deviceType);
                    });
                });
        if (!ret.isOk()) {
            LOG(ERROR) << "getType failure: " << ret.description();
            return kFailure;
        }
        return static_cast<int32_t>(result.second);
    } else {
        LOG(INFO) << "Unknown NNAPI device type.";
        return ANEURALNETWORKS_DEVICE_UNKNOWN;
    }
}

std::pair<ErrorStatus, hidl_string> VersionedIDevice::getVersionString() {
    const std::pair<ErrorStatus, hidl_string> kFailure = {ErrorStatus::GENERAL_FAILURE, ""};
    std::pair<ErrorStatus, hidl_string> result;

    if (getDevice<V1_2::IDevice>() != nullptr) {
        Return<void> ret = recoverable<void, V1_2::IDevice>(
                __FUNCTION__, [&result](const sp<V1_2::IDevice>& device) {
                    return device->getVersionString(
                            [&result](ErrorStatus error, const hidl_string& version) {
                                result = std::make_pair(error, version);
                            });
                });
        if (!ret.isOk()) {
            LOG(ERROR) << "getVersion failure: " << ret.description();
            return kFailure;
        }
        return result;
    } else if (getDevice<V1_1::IDevice>() != nullptr || getDevice<V1_0::IDevice>() != nullptr) {
        return {ErrorStatus::NONE, "UNKNOWN"};
    } else {
        LOG(ERROR) << "Could not handle getVersionString";
        return kFailure;
    }
}

std::tuple<ErrorStatus, uint32_t, uint32_t> VersionedIDevice::getNumberOfCacheFilesNeeded() {
    constexpr std::tuple<ErrorStatus, uint32_t, uint32_t> kFailure = {ErrorStatus::GENERAL_FAILURE,
                                                                      0, 0};
    std::tuple<ErrorStatus, uint32_t, uint32_t> result;

    if (getDevice<V1_2::IDevice>() != nullptr) {
        Return<void> ret = recoverable<void, V1_2::IDevice>(
                __FUNCTION__, [&result](const sp<V1_2::IDevice>& device) {
                    return device->getNumberOfCacheFilesNeeded([&result](ErrorStatus error,
                                                                         uint32_t numModelCache,
                                                                         uint32_t numDataCache) {
                        result = {error, numModelCache, numDataCache};
                    });
                });
        if (!ret.isOk()) {
            LOG(ERROR) << "getNumberOfCacheFilesNeeded failure: " << ret.description();
            return kFailure;
        }
        return result;
    } else if (getDevice<V1_1::IDevice>() != nullptr || getDevice<V1_0::IDevice>() != nullptr) {
        return {ErrorStatus::NONE, 0, 0};
    } else {
        LOG(ERROR) << "Could not handle getNumberOfCacheFilesNeeded";
        return kFailure;
    }
}

bool VersionedIDevice::operator==(nullptr_t) const {
    return getDevice<V1_0::IDevice>() == nullptr;
}

bool VersionedIDevice::operator!=(nullptr_t) const {
    return getDevice<V1_0::IDevice>() != nullptr;
}

}  // namespace nn
}  // namespace android
