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

//#define LOG_NDEBUG 0
#define LOG_TAG "Codec2Client"
#include <android-base/logging.h>

#include <codec2/hidl/client.h>

#include <deque>
#include <iterator>
#include <limits>
#include <map>
#include <mutex>
#include <sstream>
#include <thread>
#include <type_traits>
#include <vector>

#include <android-base/properties.h>
#include <bufferpool/ClientManager.h>
#include <cutils/native_handle.h>
#include <gui/bufferqueue/2.0/B2HGraphicBufferProducer.h>
#include <gui/bufferqueue/2.0/H2BGraphicBufferProducer.h>
#include <hidl/HidlSupport.h>

#include <android/hardware/media/bufferpool/2.0/IClientManager.h>
#include <android/hardware/media/c2/1.0/IComponent.h>
#include <android/hardware/media/c2/1.0/IComponentInterface.h>
#include <android/hardware/media/c2/1.0/IComponentListener.h>
#include <android/hardware/media/c2/1.0/IComponentStore.h>
#include <android/hardware/media/c2/1.0/IConfigurable.h>
#include <android/hidl/manager/1.2/IServiceManager.h>

#include <C2Debug.h>
#include <C2BufferPriv.h>
#include <C2PlatformSupport.h>

namespace android {

using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;
using ::android::hardware::Return;
using ::android::hardware::Void;

using namespace ::android::hardware::media::c2::V1_0;
using namespace ::android::hardware::media::c2::V1_0::utils;
using namespace ::android::hardware::media::bufferpool::V2_0;
using namespace ::android::hardware::media::bufferpool::V2_0::implementation;

using HGraphicBufferProducer1 = ::android::hardware::graphics::bufferqueue::
        V1_0::IGraphicBufferProducer;
using HGraphicBufferProducer2 = ::android::hardware::graphics::bufferqueue::
        V2_0::IGraphicBufferProducer;
using B2HGraphicBufferProducer2 = ::android::hardware::graphics::bufferqueue::
        V2_0::utils::B2HGraphicBufferProducer;
using H2BGraphicBufferProducer2 = ::android::hardware::graphics::bufferqueue::
        V2_0::utils::H2BGraphicBufferProducer;

namespace /* unnamed */ {

// c2_status_t value that corresponds to hwbinder transaction failure.
constexpr c2_status_t C2_TRANSACTION_FAILED = C2_CORRUPTED;

// Searches for a name in GetServiceNames() and returns the index found. If the
// name is not found, the returned index will be equal to
// GetServiceNames().size().
size_t getServiceIndex(char const* name) {
    std::vector<std::string> const& names = Codec2Client::GetServiceNames();
    size_t i = 0;
    for (; i < names.size(); ++i) {
        if (name == names[i]) {
            break;
        }
    }
    return i;
}

}  // unnamed namespace

// This class caches a Codec2Client object and its component traits. The client
// will be created the first time it is needed, and it can be refreshed if the
// service dies (by calling invalidate()). The first time listComponents() is
// called from the client, the result will be cached.
class Codec2Client::Cache {
    // Cached client
    std::shared_ptr<Codec2Client> mClient;
    mutable std::mutex mClientMutex;

    // Cached component traits
    std::vector<C2Component::Traits> mTraits;
    std::once_flag mTraitsInitializationFlag;

    // The index of the service. This is based on GetServiceNames().
    size_t mIndex;
    // Called by s() exactly once to initialize the cache. The index must be a
    // valid index into the vector returned by GetServiceNames(). Calling
    // init(index) will associate the cache to the service with name
    // GetServiceNames()[index].
    void init(size_t index) {
        mIndex = index;
    }

public:
    Cache() = default;

    // Initializes mClient if needed, then returns mClient.
    // If the service is unavailable but listed in the manifest, this function
    // will block indefinitely.
    std::shared_ptr<Codec2Client> getClient() {
        std::scoped_lock lock{mClientMutex};
        if (!mClient) {
            mClient = Codec2Client::_CreateFromIndex(mIndex);
        }
        CHECK(mClient) << "Failed to create Codec2Client to service \""
                       << GetServiceNames()[mIndex] << "\". (Index = "
                       << mIndex << ").";
        return mClient;
    }

    // Causes a subsequent call to getClient() to create a new client. This
    // function should be called after the service dies.
    //
    // Note: This function is called only by ForAllServices().
    void invalidate() {
        std::scoped_lock lock{mClientMutex};
        mClient = nullptr;
    }

    // Returns a list of traits for components supported by the service. This
    // list is cached.
    std::vector<C2Component::Traits> const& getTraits() {
        std::call_once(mTraitsInitializationFlag, [this]() {
            bool success{false};
            // Spin until _listComponents() is successful.
            while (true) {
                std::shared_ptr<Codec2Client> client = getClient();
                mTraits = client->_listComponents(&success);
                if (success) {
                    break;
                }
                using namespace std::chrono_literals;
                static constexpr auto kServiceRetryPeriod = 5s;
                LOG(INFO) << "Failed to retrieve component traits from service "
                             "\"" << GetServiceNames()[mIndex] << "\". "
                             "Retrying...";
                std::this_thread::sleep_for(kServiceRetryPeriod);
            }
        });
        return mTraits;
    }

    // List() returns the list of all caches.
    static std::vector<Cache>& List() {
        static std::vector<Cache> sCaches{[]() {
            size_t numServices = GetServiceNames().size();
            std::vector<Cache> caches(numServices);
            for (size_t i = 0; i < numServices; ++i) {
                caches[i].init(i);
            }
            return caches;
        }()};
        return sCaches;
    }
};

// Codec2ConfigurableClient

const C2String& Codec2ConfigurableClient::getName() const {
    return mName;
}

Codec2ConfigurableClient::Codec2ConfigurableClient(
        const sp<IConfigurable>& base)
      : mBase{base},
        mName{[base]() -> C2String {
                C2String outName;
                Return<void> transStatus = base->getName(
                        [&outName](const hidl_string& name) {
                            outName = name.c_str();
                        });
                return transStatus.isOk() ? outName : "";
            }()} {
}

c2_status_t Codec2ConfigurableClient::query(
        const std::vector<C2Param*> &stackParams,
        const std::vector<C2Param::Index> &heapParamIndices,
        c2_blocking_t mayBlock,
        std::vector<std::unique_ptr<C2Param>>* const heapParams) const {
    hidl_vec<ParamIndex> indices(
            stackParams.size() + heapParamIndices.size());
    size_t numIndices = 0;
    for (C2Param* const& stackParam : stackParams) {
        if (!stackParam) {
            LOG(WARNING) << "query -- null stack param encountered.";
            continue;
        }
        indices[numIndices++] = static_cast<ParamIndex>(stackParam->index());
    }
    size_t numStackIndices = numIndices;
    for (const C2Param::Index& index : heapParamIndices) {
        indices[numIndices++] =
                static_cast<ParamIndex>(static_cast<uint32_t>(index));
    }
    indices.resize(numIndices);
    if (heapParams) {
        heapParams->reserve(heapParams->size() + numIndices);
    }
    c2_status_t status;
    Return<void> transStatus = mBase->query(
            indices,
            mayBlock == C2_MAY_BLOCK,
            [&status, &numStackIndices, &stackParams, heapParams](
                    Status s, const Params& p) {
                status = static_cast<c2_status_t>(s);
                if (status != C2_OK && status != C2_BAD_INDEX) {
                    LOG(DEBUG) << "query -- call failed: "
                               << status << ".";
                    return;
                }
                std::vector<C2Param*> paramPointers;
                if (!parseParamsBlob(&paramPointers, p)) {
                    LOG(ERROR) << "query -- error while parsing params.";
                    status = C2_CORRUPTED;
                    return;
                }
                size_t i = 0;
                for (auto it = paramPointers.begin();
                        it != paramPointers.end(); ) {
                    C2Param* paramPointer = *it;
                    if (numStackIndices > 0) {
                        --numStackIndices;
                        if (!paramPointer) {
                            LOG(WARNING) << "query -- null stack param.";
                            ++it;
                            continue;
                        }
                        for (; i < stackParams.size() && !stackParams[i]; ) {
                            ++i;
                        }
                        if (i >= stackParams.size()) {
                            LOG(ERROR) << "query -- unexpected error.";
                            status = C2_CORRUPTED;
                            return;
                        }
                        if (stackParams[i]->index() != paramPointer->index()) {
                            LOG(WARNING) << "query -- param skipped: "
                                            "index = "
                                         << stackParams[i]->index() << ".";
                            stackParams[i++]->invalidate();
                            continue;
                        }
                        if (!stackParams[i++]->updateFrom(*paramPointer)) {
                            LOG(WARNING) << "query -- param update failed: "
                                            "index = "
                                         << paramPointer->index() << ".";
                        }
                    } else {
                        if (!paramPointer) {
                            LOG(WARNING) << "query -- null heap param.";
                            ++it;
                            continue;
                        }
                        if (!heapParams) {
                            LOG(WARNING) << "query -- "
                                            "unexpected extra stack param.";
                        } else {
                            heapParams->emplace_back(
                                    C2Param::Copy(*paramPointer));
                        }
                    }
                    ++it;
                }
            });
    if (!transStatus.isOk()) {
        LOG(ERROR) << "query -- transaction failed.";
        return C2_TRANSACTION_FAILED;
    }
    return status;
}

c2_status_t Codec2ConfigurableClient::config(
        const std::vector<C2Param*> &params,
        c2_blocking_t mayBlock,
        std::vector<std::unique_ptr<C2SettingResult>>* const failures) {
    Params hidlParams;
    if (!createParamsBlob(&hidlParams, params)) {
        LOG(ERROR) << "config -- bad input.";
        return C2_TRANSACTION_FAILED;
    }
    c2_status_t status;
    Return<void> transStatus = mBase->config(
            hidlParams,
            mayBlock == C2_MAY_BLOCK,
            [&status, &params, failures](
                    Status s,
                    const hidl_vec<SettingResult> f,
                    const Params& o) {
                status = static_cast<c2_status_t>(s);
                if (status != C2_OK && status != C2_BAD_INDEX) {
                    LOG(DEBUG) << "config -- call failed: "
                               << status << ".";
                }
                size_t i = failures->size();
                failures->resize(i + f.size());
                for (const SettingResult& sf : f) {
                    if (!objcpy(&(*failures)[i++], sf)) {
                        LOG(ERROR) << "config -- "
                                   << "invalid SettingResult returned.";
                        return;
                    }
                }
                if (!updateParamsFromBlob(params, o)) {
                    LOG(ERROR) << "config -- "
                               << "failed to parse returned params.";
                    status = C2_CORRUPTED;
                }
            });
    if (!transStatus.isOk()) {
        LOG(ERROR) << "config -- transaction failed.";
        return C2_TRANSACTION_FAILED;
    }
    return status;
}

c2_status_t Codec2ConfigurableClient::querySupportedParams(
        std::vector<std::shared_ptr<C2ParamDescriptor>>* const params) const {
    // TODO: Cache and query properly!
    c2_status_t status;
    Return<void> transStatus = mBase->querySupportedParams(
            std::numeric_limits<uint32_t>::min(),
            std::numeric_limits<uint32_t>::max(),
            [&status, params](
                    Status s,
                    const hidl_vec<ParamDescriptor>& p) {
                status = static_cast<c2_status_t>(s);
                if (status != C2_OK) {
                    LOG(DEBUG) << "querySupportedParams -- call failed: "
                               << status << ".";
                    return;
                }
                size_t i = params->size();
                params->resize(i + p.size());
                for (const ParamDescriptor& sp : p) {
                    if (!objcpy(&(*params)[i++], sp)) {
                        LOG(ERROR) << "querySupportedParams -- "
                                   << "invalid returned ParamDescriptor.";
                        return;
                    }
                }
            });
    if (!transStatus.isOk()) {
        LOG(ERROR) << "querySupportedParams -- transaction failed.";
        return C2_TRANSACTION_FAILED;
    }
    return status;
}

c2_status_t Codec2ConfigurableClient::querySupportedValues(
        std::vector<C2FieldSupportedValuesQuery>& fields,
        c2_blocking_t mayBlock) const {
    hidl_vec<FieldSupportedValuesQuery> inFields(fields.size());
    for (size_t i = 0; i < fields.size(); ++i) {
        if (!objcpy(&inFields[i], fields[i])) {
            LOG(ERROR) << "querySupportedValues -- bad input";
            return C2_TRANSACTION_FAILED;
        }
    }

    c2_status_t status;
    Return<void> transStatus = mBase->querySupportedValues(
            inFields,
            mayBlock == C2_MAY_BLOCK,
            [&status, &inFields, &fields](
                    Status s,
                    const hidl_vec<FieldSupportedValuesQueryResult>& r) {
                status = static_cast<c2_status_t>(s);
                if (status != C2_OK) {
                    LOG(DEBUG) << "querySupportedValues -- call failed: "
                               << status << ".";
                    return;
                }
                if (r.size() != fields.size()) {
                    LOG(ERROR) << "querySupportedValues -- "
                                  "input and output lists "
                                  "have different sizes.";
                    status = C2_CORRUPTED;
                    return;
                }
                for (size_t i = 0; i < fields.size(); ++i) {
                    if (!objcpy(&fields[i], inFields[i], r[i])) {
                        LOG(ERROR) << "querySupportedValues -- "
                                      "invalid returned value.";
                        status = C2_CORRUPTED;
                        return;
                    }
                }
            });
    if (!transStatus.isOk()) {
        LOG(ERROR) << "querySupportedValues -- transaction failed.";
        return C2_TRANSACTION_FAILED;
    }
    return status;
}

// Codec2Client::Component::HidlListener
struct Codec2Client::Component::HidlListener : public IComponentListener {
    std::weak_ptr<Component> component;
    std::weak_ptr<Listener> base;

    virtual Return<void> onWorkDone(const WorkBundle& workBundle) override {
        std::list<std::unique_ptr<C2Work>> workItems;
        if (!objcpy(&workItems, workBundle)) {
            LOG(DEBUG) << "onWorkDone -- received corrupted WorkBundle.";
            return Void();
        }
        // release input buffers potentially held by the component from queue
        std::shared_ptr<Codec2Client::Component> strongComponent =
                component.lock();
        if (strongComponent) {
            strongComponent->handleOnWorkDone(workItems);
        }
        if (std::shared_ptr<Codec2Client::Listener> listener = base.lock()) {
            listener->onWorkDone(component, workItems);
        } else {
            LOG(DEBUG) << "onWorkDone -- listener died.";
        }
        return Void();
    }

    virtual Return<void> onTripped(
            const hidl_vec<SettingResult>& settingResults) override {
        std::vector<std::shared_ptr<C2SettingResult>> c2SettingResults(
                settingResults.size());
        for (size_t i = 0; i < settingResults.size(); ++i) {
            std::unique_ptr<C2SettingResult> c2SettingResult;
            if (!objcpy(&c2SettingResult, settingResults[i])) {
                LOG(DEBUG) << "onTripped -- received corrupted SettingResult.";
                return Void();
            }
            c2SettingResults[i] = std::move(c2SettingResult);
        }
        if (std::shared_ptr<Codec2Client::Listener> listener = base.lock()) {
            listener->onTripped(component, c2SettingResults);
        } else {
            LOG(DEBUG) << "onTripped -- listener died.";
        }
        return Void();
    }

    virtual Return<void> onError(Status s, uint32_t errorCode) override {
        LOG(DEBUG) << "onError --"
                   << " status = " << s
                   << ", errorCode = " << errorCode
                   << ".";
        if (std::shared_ptr<Listener> listener = base.lock()) {
            listener->onError(component, s == Status::OK ?
                    errorCode : static_cast<c2_status_t>(s));
        } else {
            LOG(DEBUG) << "onError -- listener died.";
        }
        return Void();
    }

    virtual Return<void> onFramesRendered(
            const hidl_vec<RenderedFrame>& renderedFrames) override {
        std::shared_ptr<Listener> listener = base.lock();
        if (!listener) {
            LOG(DEBUG) << "onFramesRendered -- listener died.";
            return Void();
        }
        for (const RenderedFrame& renderedFrame : renderedFrames) {
            listener->onFrameRendered(
                    renderedFrame.bufferQueueId,
                    renderedFrame.slotId,
                    renderedFrame.timestampNs);
        }
        return Void();
    }

    virtual Return<void> onInputBuffersReleased(
            const hidl_vec<InputBuffer>& inputBuffers) override {
        std::shared_ptr<Listener> listener = base.lock();
        if (!listener) {
            LOG(DEBUG) << "onInputBuffersReleased -- listener died.";
            return Void();
        }
        for (const InputBuffer& inputBuffer : inputBuffers) {
            LOG(VERBOSE) << "onInputBuffersReleased --"
                            " received death notification of"
                            " input buffer:"
                            " frameIndex = " << inputBuffer.frameIndex
                         << ", bufferIndex = " << inputBuffer.arrayIndex
                         << ".";
            listener->onInputBufferDone(
                    inputBuffer.frameIndex, inputBuffer.arrayIndex);
        }
        return Void();
    }

};

// Codec2Client
Codec2Client::Codec2Client(const sp<IComponentStore>& base,
                           size_t serviceIndex)
      : Configurable{
            [base]() -> sp<IConfigurable> {
                Return<sp<IConfigurable>> transResult =
                        base->getConfigurable();
                return transResult.isOk() ?
                        static_cast<sp<IConfigurable>>(transResult) :
                        nullptr;
            }()
        },
        mBase{base},
        mServiceIndex{serviceIndex} {
    Return<sp<IClientManager>> transResult = base->getPoolClientManager();
    if (!transResult.isOk()) {
        LOG(ERROR) << "getPoolClientManager -- transaction failed.";
    } else {
        mHostPoolManager = static_cast<sp<IClientManager>>(transResult);
    }
}

sp<Codec2Client::Base> const& Codec2Client::getBase() const {
    return mBase;
}

std::string const& Codec2Client::getServiceName() const {
    return GetServiceNames()[mServiceIndex];
}

c2_status_t Codec2Client::createComponent(
        const C2String& name,
        const std::shared_ptr<Codec2Client::Listener>& listener,
        std::shared_ptr<Codec2Client::Component>* const component) {

    c2_status_t status;
    sp<Component::HidlListener> hidlListener = new Component::HidlListener{};
    hidlListener->base = listener;
    Return<void> transStatus = mBase->createComponent(
            name,
            hidlListener,
            ClientManager::getInstance(),
            [&status, component, hidlListener](
                    Status s,
                    const sp<IComponent>& c) {
                status = static_cast<c2_status_t>(s);
                if (status != C2_OK) {
                    return;
                }
                *component = std::make_shared<Codec2Client::Component>(c);
                hidlListener->component = *component;
            });
    if (!transStatus.isOk()) {
        LOG(ERROR) << "createComponent(" << name.c_str()
                   << ") -- transaction failed.";
        return C2_TRANSACTION_FAILED;
    } else if (status != C2_OK) {
        LOG(ERROR) << "createComponent(" << name.c_str()
                   << ") -- call failed: " << status << ".";
        return status;
    } else if (!*component) {
        LOG(ERROR) << "createComponent(" << name.c_str()
                   << ") -- null component.";
        return C2_CORRUPTED;
    }

    status = (*component)->setDeathListener(*component, listener);
    if (status != C2_OK) {
        LOG(ERROR) << "createComponent(" << name.c_str()
                   << ") -- failed to set up death listener: "
                   << status << ".";
    }

    (*component)->mBufferPoolSender.setReceiver(mHostPoolManager);
    return status;
}

c2_status_t Codec2Client::createInterface(
        const C2String& name,
        std::shared_ptr<Codec2Client::Interface>* const interface) {
    c2_status_t status;
    Return<void> transStatus = mBase->createInterface(
            name,
            [&status, interface](
                    Status s,
                    const sp<IComponentInterface>& i) {
                status = static_cast<c2_status_t>(s);
                if (status != C2_OK) {
                    return;
                }
                *interface = std::make_shared<Interface>(i);
            });
    if (!transStatus.isOk()) {
        LOG(ERROR) << "createInterface(" << name.c_str()
                   << ") -- transaction failed.";
        return C2_TRANSACTION_FAILED;
    } else if (status != C2_OK) {
        LOG(ERROR) << "createComponent(" << name.c_str()
                   << ") -- call failed: " << status << ".";
        return status;
    }

    return status;
}

c2_status_t Codec2Client::createInputSurface(
        std::shared_ptr<InputSurface>* const inputSurface) {
    c2_status_t status;
    Return<void> transStatus = mBase->createInputSurface(
            [&status, inputSurface](
                    Status s,
                    const sp<IInputSurface>& i) {
                status = static_cast<c2_status_t>(s);
                if (status != C2_OK) {
                    return;
                }
                *inputSurface = std::make_shared<InputSurface>(i);
            });
    if (!transStatus.isOk()) {
        LOG(ERROR) << "createInputSurface -- transaction failed.";
        return C2_TRANSACTION_FAILED;
    } else if (status != C2_OK) {
        LOG(DEBUG) << "createInputSurface -- call failed: "
                   << status << ".";
    }
    return status;
}

std::vector<C2Component::Traits> const& Codec2Client::listComponents() const {
    return Cache::List()[mServiceIndex].getTraits();
}

std::vector<C2Component::Traits> Codec2Client::_listComponents(
        bool* success) const {
    std::vector<C2Component::Traits> traits;
    std::string const& serviceName = getServiceName();
    Return<void> transStatus = mBase->listComponents(
            [&traits, &serviceName](Status s,
                   const hidl_vec<IComponentStore::ComponentTraits>& t) {
                if (s != Status::OK) {
                    LOG(DEBUG) << "_listComponents -- call failed: "
                               << static_cast<c2_status_t>(s) << ".";
                    return;
                }
                traits.resize(t.size());
                for (size_t i = 0; i < t.size(); ++i) {
                    if (!objcpy(&traits[i], t[i])) {
                        LOG(ERROR) << "_listComponents -- corrupted output.";
                        return;
                    }
                    traits[i].owner = serviceName;
                }
            });
    if (!transStatus.isOk()) {
        LOG(ERROR) << "_listComponents -- transaction failed.";
        *success = false;
    } else {
        *success = true;
    }
    return traits;
}

c2_status_t Codec2Client::copyBuffer(
        const std::shared_ptr<C2Buffer>& src,
        const std::shared_ptr<C2Buffer>& dst) {
    // TODO: Implement?
    (void)src;
    (void)dst;
    LOG(ERROR) << "copyBuffer not implemented";
    return C2_OMITTED;
}

std::shared_ptr<C2ParamReflector>
        Codec2Client::getParamReflector() {
    // TODO: this is not meant to be exposed as C2ParamReflector on the client side; instead, it
    // should reflect the HAL API.
    struct SimpleParamReflector : public C2ParamReflector {
        virtual std::unique_ptr<C2StructDescriptor> describe(C2Param::CoreIndex coreIndex) const {
            hidl_vec<ParamIndex> indices(1);
            indices[0] = static_cast<ParamIndex>(coreIndex.coreIndex());
            std::unique_ptr<C2StructDescriptor> descriptor;
            Return<void> transStatus = mBase->getStructDescriptors(
                    indices,
                    [&descriptor](
                            Status s,
                            const hidl_vec<StructDescriptor>& sd) {
                        c2_status_t status = static_cast<c2_status_t>(s);
                        if (status != C2_OK) {
                            LOG(DEBUG) << "SimpleParamReflector -- "
                                          "getStructDescriptors() failed: "
                                       << status << ".";
                            descriptor.reset();
                            return;
                        }
                        if (sd.size() != 1) {
                            LOG(DEBUG) << "SimpleParamReflector -- "
                                          "getStructDescriptors() "
                                          "returned vector of size "
                                       << sd.size() << ". "
                                          "It should be 1.";
                            descriptor.reset();
                            return;
                        }
                        if (!objcpy(&descriptor, sd[0])) {
                            LOG(DEBUG) << "SimpleParamReflector -- "
                                          "getStructDescriptors() returned "
                                          "corrupted data.";
                            descriptor.reset();
                            return;
                        }
                    });
            return descriptor;
        }

        SimpleParamReflector(sp<Base> base)
            : mBase(base) { }

        sp<Base> mBase;
    };

    return std::make_shared<SimpleParamReflector>(mBase);
};

std::vector<std::string> const& Codec2Client::GetServiceNames() {
    static std::vector<std::string> sServiceNames{[]() {
        using ::android::hardware::media::c2::V1_0::IComponentStore;
        using ::android::hidl::manager::V1_2::IServiceManager;

        while (true) {
            sp<IServiceManager> serviceManager = IServiceManager::getService();
            CHECK(serviceManager) << "Hardware service manager is not running.";

            // There are three categories of services based on names.
            std::vector<std::string> defaultNames; // Prefixed with "default"
            std::vector<std::string> vendorNames;  // Prefixed with "vendor"
            std::vector<std::string> otherNames;   // Others
            Return<void> transResult;
            transResult = serviceManager->listManifestByInterface(
                    IComponentStore::descriptor,
                    [&defaultNames, &vendorNames, &otherNames](
                            hidl_vec<hidl_string> const& instanceNames) {
                        for (hidl_string const& instanceName : instanceNames) {
                            char const* name = instanceName.c_str();
                            if (strncmp(name, "default", 7) == 0) {
                                defaultNames.emplace_back(name);
                            } else if (strncmp(name, "vendor", 6) == 0) {
                                vendorNames.emplace_back(name);
                            } else {
                                otherNames.emplace_back(name);
                            }
                        }
                    });
            if (transResult.isOk()) {
                // Sort service names in each category.
                std::sort(defaultNames.begin(), defaultNames.end());
                std::sort(vendorNames.begin(), vendorNames.end());
                std::sort(otherNames.begin(), otherNames.end());

                // Concatenate the three lists in this order: default, vendor,
                // other.
                std::vector<std::string>& names = defaultNames;
                names.reserve(names.size() + vendorNames.size() + otherNames.size());
                names.insert(names.end(),
                             std::make_move_iterator(vendorNames.begin()),
                             std::make_move_iterator(vendorNames.end()));
                names.insert(names.end(),
                             std::make_move_iterator(otherNames.begin()),
                             std::make_move_iterator(otherNames.end()));

                // Summarize to logcat.
                if (names.empty()) {
                    LOG(INFO) << "No Codec2 services declared in the manifest.";
                } else {
                    std::stringstream stringOutput;
                    stringOutput << "Available Codec2 services:";
                    for (std::string const& name : names) {
                        stringOutput << " \"" << name << "\"";
                    }
                    LOG(INFO) << stringOutput.str();
                }

                return names;
            }
            LOG(ERROR) << "Could not retrieve the list of service instances of "
                       << IComponentStore::descriptor
                       << ". Retrying...";
        }
    }()};
    return sServiceNames;
}

std::shared_ptr<Codec2Client> Codec2Client::CreateFromService(
        const char* name) {
    size_t index = getServiceIndex(name);
    return index == GetServiceNames().size() ?
            nullptr : _CreateFromIndex(index);
}

std::vector<std::shared_ptr<Codec2Client>> Codec2Client::
        CreateFromAllServices() {
    std::vector<std::shared_ptr<Codec2Client>> clients(
            GetServiceNames().size());
    for (size_t i = GetServiceNames().size(); i > 0; ) {
        --i;
        clients[i] = _CreateFromIndex(i);
    }
    return clients;
}

std::shared_ptr<Codec2Client> Codec2Client::_CreateFromIndex(size_t index) {
    std::string const& name = GetServiceNames()[index];
    LOG(INFO) << "Creating a Codec2 client to service \"" << name << "\"";
    sp<Base> baseStore = Base::getService(name);
    CHECK(baseStore) << "Codec2 service \"" << name << "\""
                        " inaccessible for unknown reasons.";
    LOG(INFO) << "Client to Codec2 service \"" << name << "\" created";
    return std::make_shared<Codec2Client>(baseStore, index);
}

c2_status_t Codec2Client::ForAllServices(
        const std::string &key,
        size_t numberOfAttempts,
        std::function<c2_status_t(const std::shared_ptr<Codec2Client>&)>
            predicate) {
    c2_status_t status = C2_NO_INIT;  // no IComponentStores present

    // Cache the mapping key -> index of Codec2Client in Cache::List().
    static std::mutex key2IndexMutex;
    static std::map<std::string, size_t> key2Index;

    // By default try all stores. However, try the last known client first. If
    // the last known client fails, retry once. We do this by pushing the last
    // known client in front of the list of all clients.
    std::deque<size_t> indices;
    for (size_t index = Cache::List().size(); index > 0; ) {
        indices.push_front(--index);
    }

    bool wasMapped = false;
    {
        std::scoped_lock lock{key2IndexMutex};
        auto it = key2Index.find(key);
        if (it != key2Index.end()) {
            indices.push_front(it->second);
            wasMapped = true;
        }
    }

    for (size_t index : indices) {
        Cache& cache = Cache::List()[index];
        for (size_t tries = numberOfAttempts; tries > 0; --tries) {
            std::shared_ptr<Codec2Client> client{cache.getClient()};
            status = predicate(client);
            if (status == C2_OK) {
                std::scoped_lock lock{key2IndexMutex};
                key2Index[key] = index; // update last known client index
                return C2_OK;
            } else if (status == C2_TRANSACTION_FAILED) {
                LOG(WARNING) << "\"" << key << "\" failed for service \""
                             << client->getName()
                             << "\" due to transaction failure. "
                             << "(Service may have crashed.)"
                             << (tries > 1 ? " Retrying..." : "");
                cache.invalidate();
                continue;
            }
            if (wasMapped) {
                LOG(INFO) << "\"" << key << "\" became invalid in service \""
                          << client->getName() << "\". Retrying...";
                wasMapped = false;
            }
            break;
        }
    }
    return status; // return the last status from a valid client
}

std::shared_ptr<Codec2Client::Component>
        Codec2Client::CreateComponentByName(
        const char* componentName,
        const std::shared_ptr<Listener>& listener,
        std::shared_ptr<Codec2Client>* owner,
        size_t numberOfAttempts) {
    std::string key{"create:"};
    key.append(componentName);
    std::shared_ptr<Component> component;
    c2_status_t status = ForAllServices(
            key,
            numberOfAttempts,
            [owner, &component, componentName, &listener](
                    const std::shared_ptr<Codec2Client> &client)
                        -> c2_status_t {
                c2_status_t status = client->createComponent(componentName,
                                                             listener,
                                                             &component);
                if (status == C2_OK) {
                    if (owner) {
                        *owner = client;
                    }
                } else if (status != C2_NOT_FOUND) {
                    LOG(DEBUG) << "IComponentStore("
                                   << client->getServiceName()
                               << ")::createComponent(\"" << componentName
                               << "\") returned status = "
                               << status << ".";
                }
                return status;
            });
    if (status != C2_OK) {
        LOG(DEBUG) << "Failed to create component \"" << componentName
                   << "\" from all known services. "
                      "Last returned status = " << status << ".";
    }
    return component;
}

std::shared_ptr<Codec2Client::Interface>
        Codec2Client::CreateInterfaceByName(
        const char* interfaceName,
        std::shared_ptr<Codec2Client>* owner,
        size_t numberOfAttempts) {
    std::string key{"create:"};
    key.append(interfaceName);
    std::shared_ptr<Interface> interface;
    c2_status_t status = ForAllServices(
            key,
            numberOfAttempts,
            [owner, &interface, interfaceName](
                    const std::shared_ptr<Codec2Client> &client)
                        -> c2_status_t {
                c2_status_t status = client->createInterface(interfaceName,
                                                             &interface);
                if (status == C2_OK) {
                    if (owner) {
                        *owner = client;
                    }
                } else if (status != C2_NOT_FOUND) {
                    LOG(DEBUG) << "IComponentStore("
                                   << client->getServiceName()
                               << ")::createInterface(\"" << interfaceName
                               << "\") returned status = "
                               << status << ".";
                }
                return status;
            });
    if (status != C2_OK) {
        LOG(DEBUG) << "Failed to create interface \"" << interfaceName
                   << "\" from all known services. "
                      "Last returned status = " << status << ".";
    }
    return interface;
}

std::vector<C2Component::Traits> const& Codec2Client::ListComponents() {
    static std::vector<C2Component::Traits> sList{[]() {
        std::vector<C2Component::Traits> list;
        for (Cache& cache : Cache::List()) {
            std::vector<C2Component::Traits> const& traits = cache.getTraits();
            list.insert(list.end(), traits.begin(), traits.end());
        }
        return list;
    }()};
    return sList;
}

std::shared_ptr<Codec2Client::InputSurface> Codec2Client::CreateInputSurface(
        char const* serviceName) {
    int32_t inputSurfaceSetting = ::android::base::GetIntProperty(
            "debug.stagefright.c2inputsurface", int32_t(0));
    if (inputSurfaceSetting <= 0) {
        return nullptr;
    }
    size_t index = GetServiceNames().size();
    if (serviceName) {
        index = getServiceIndex(serviceName);
        if (index == GetServiceNames().size()) {
            LOG(DEBUG) << "CreateInputSurface -- invalid service name: \""
                       << serviceName << "\"";
        }
    }

    std::shared_ptr<Codec2Client::InputSurface> inputSurface;
    if (index != GetServiceNames().size()) {
        std::shared_ptr<Codec2Client> client = Cache::List()[index].getClient();
        if (client->createInputSurface(&inputSurface) == C2_OK) {
            return inputSurface;
        }
    }
    LOG(INFO) << "CreateInputSurface -- attempting to create an input surface "
                 "from all services...";
    for (Cache& cache : Cache::List()) {
        std::shared_ptr<Codec2Client> client = cache.getClient();
        if (client->createInputSurface(&inputSurface) == C2_OK) {
            LOG(INFO) << "CreateInputSurface -- input surface obtained from "
                         "service \"" << client->getServiceName() << "\"";
            return inputSurface;
        }
    }
    LOG(WARNING) << "CreateInputSurface -- failed to create an input surface "
                    "from all services";
    return nullptr;
}

// Codec2Client::Listener

Codec2Client::Listener::~Listener() {
}

// Codec2Client::Interface
Codec2Client::Interface::Interface(const sp<Base>& base)
      : Configurable{
            [base]() -> sp<IConfigurable> {
                Return<sp<IConfigurable>> transResult =
                        base->getConfigurable();
                return transResult.isOk() ?
                        static_cast<sp<IConfigurable>>(transResult) :
                        nullptr;
            }()
        },
        mBase{base} {
}

// Codec2Client::Component
Codec2Client::Component::Component(const sp<Base>& base)
      : Configurable{
            [base]() -> sp<IConfigurable> {
                Return<sp<IComponentInterface>> transResult1 =
                        base->getInterface();
                if (!transResult1.isOk()) {
                    return nullptr;
                }
                Return<sp<IConfigurable>> transResult2 =
                        static_cast<sp<IComponentInterface>>(transResult1)->
                        getConfigurable();
                return transResult2.isOk() ?
                        static_cast<sp<IConfigurable>>(transResult2) :
                        nullptr;
            }()
        },
        mBase{base},
        mBufferPoolSender{nullptr} {
}

Codec2Client::Component::~Component() {
}

c2_status_t Codec2Client::Component::createBlockPool(
        C2Allocator::id_t id,
        C2BlockPool::local_id_t* blockPoolId,
        std::shared_ptr<Codec2Client::Configurable>* configurable) {
    c2_status_t status;
    Return<void> transStatus = mBase->createBlockPool(
            static_cast<uint32_t>(id),
            [&status, blockPoolId, configurable](
                    Status s,
                    uint64_t pId,
                    const sp<IConfigurable>& c) {
                status = static_cast<c2_status_t>(s);
                configurable->reset();
                if (status != C2_OK) {
                    LOG(DEBUG) << "createBlockPool -- call failed: "
                               << status << ".";
                    return;
                }
                *blockPoolId = static_cast<C2BlockPool::local_id_t>(pId);
                *configurable = std::make_shared<Configurable>(c);
            });
    if (!transStatus.isOk()) {
        LOG(ERROR) << "createBlockPool -- transaction failed.";
        return C2_TRANSACTION_FAILED;
    }
    return status;
}

c2_status_t Codec2Client::Component::destroyBlockPool(
        C2BlockPool::local_id_t localId) {
    Return<Status> transResult = mBase->destroyBlockPool(
            static_cast<uint64_t>(localId));
    if (!transResult.isOk()) {
        LOG(ERROR) << "destroyBlockPool -- transaction failed.";
        return C2_TRANSACTION_FAILED;
    }
    return static_cast<c2_status_t>(static_cast<Status>(transResult));
}

void Codec2Client::Component::handleOnWorkDone(
        const std::list<std::unique_ptr<C2Work>> &workItems) {
    // Output bufferqueue-based blocks' lifetime management
    mOutputBufferQueue.holdBufferQueueBlocks(workItems);
}

c2_status_t Codec2Client::Component::queue(
        std::list<std::unique_ptr<C2Work>>* const items) {
    WorkBundle workBundle;
    if (!objcpy(&workBundle, *items, &mBufferPoolSender)) {
        LOG(ERROR) << "queue -- bad input.";
        return C2_TRANSACTION_FAILED;
    }
    Return<Status> transStatus = mBase->queue(workBundle);
    if (!transStatus.isOk()) {
        LOG(ERROR) << "queue -- transaction failed.";
        return C2_TRANSACTION_FAILED;
    }
    c2_status_t status =
            static_cast<c2_status_t>(static_cast<Status>(transStatus));
    if (status != C2_OK) {
        LOG(DEBUG) << "queue -- call failed: " << status << ".";
    }
    return status;
}

c2_status_t Codec2Client::Component::flush(
        C2Component::flush_mode_t mode,
        std::list<std::unique_ptr<C2Work>>* const flushedWork) {
    (void)mode; // Flush mode isn't supported in HIDL yet.
    c2_status_t status;
    Return<void> transStatus = mBase->flush(
            [&status, flushedWork](
                    Status s, const WorkBundle& wb) {
                status = static_cast<c2_status_t>(s);
                if (status != C2_OK) {
                    LOG(DEBUG) << "flush -- call failed: " << status << ".";
                    return;
                }
                if (!objcpy(flushedWork, wb)) {
                    status = C2_CORRUPTED;
                } else {
                    status = C2_OK;
                }
            });
    if (!transStatus.isOk()) {
        LOG(ERROR) << "flush -- transaction failed.";
        return C2_TRANSACTION_FAILED;
    }

    // Indices of flushed work items.
    std::vector<uint64_t> flushedIndices;
    for (const std::unique_ptr<C2Work> &work : *flushedWork) {
        if (work) {
            if (work->worklets.empty()
                    || !work->worklets.back()
                    || (work->worklets.back()->output.flags &
                        C2FrameData::FLAG_INCOMPLETE) == 0) {
                // input is complete
                flushedIndices.emplace_back(
                        work->input.ordinal.frameIndex.peeku());
            }
        }
    }

    // Output bufferqueue-based blocks' lifetime management
    mOutputBufferQueue.holdBufferQueueBlocks(*flushedWork);

    return status;
}

c2_status_t Codec2Client::Component::drain(C2Component::drain_mode_t mode) {
    Return<Status> transStatus = mBase->drain(
            mode == C2Component::DRAIN_COMPONENT_WITH_EOS);
    if (!transStatus.isOk()) {
        LOG(ERROR) << "drain -- transaction failed.";
        return C2_TRANSACTION_FAILED;
    }
    c2_status_t status =
            static_cast<c2_status_t>(static_cast<Status>(transStatus));
    if (status != C2_OK) {
        LOG(DEBUG) << "drain -- call failed: " << status << ".";
    }
    return status;
}

c2_status_t Codec2Client::Component::start() {
    Return<Status> transStatus = mBase->start();
    if (!transStatus.isOk()) {
        LOG(ERROR) << "start -- transaction failed.";
        return C2_TRANSACTION_FAILED;
    }
    c2_status_t status =
            static_cast<c2_status_t>(static_cast<Status>(transStatus));
    if (status != C2_OK) {
        LOG(DEBUG) << "start -- call failed: " << status << ".";
    }
    return status;
}

c2_status_t Codec2Client::Component::stop() {
    Return<Status> transStatus = mBase->stop();
    if (!transStatus.isOk()) {
        LOG(ERROR) << "stop -- transaction failed.";
        return C2_TRANSACTION_FAILED;
    }
    c2_status_t status =
            static_cast<c2_status_t>(static_cast<Status>(transStatus));
    if (status != C2_OK) {
        LOG(DEBUG) << "stop -- call failed: " << status << ".";
    }
    return status;
}

c2_status_t Codec2Client::Component::reset() {
    Return<Status> transStatus = mBase->reset();
    if (!transStatus.isOk()) {
        LOG(ERROR) << "reset -- transaction failed.";
        return C2_TRANSACTION_FAILED;
    }
    c2_status_t status =
            static_cast<c2_status_t>(static_cast<Status>(transStatus));
    if (status != C2_OK) {
        LOG(DEBUG) << "reset -- call failed: " << status << ".";
    }
    return status;
}

c2_status_t Codec2Client::Component::release() {
    Return<Status> transStatus = mBase->release();
    if (!transStatus.isOk()) {
        LOG(ERROR) << "release -- transaction failed.";
        return C2_TRANSACTION_FAILED;
    }
    c2_status_t status =
            static_cast<c2_status_t>(static_cast<Status>(transStatus));
    if (status != C2_OK) {
        LOG(DEBUG) << "release -- call failed: " << status << ".";
    }
    return status;
}

c2_status_t Codec2Client::Component::setOutputSurface(
        C2BlockPool::local_id_t blockPoolId,
        const sp<IGraphicBufferProducer>& surface,
        uint32_t generation) {
    uint64_t bqId = 0;
    sp<IGraphicBufferProducer> nullIgbp;
    sp<HGraphicBufferProducer2> nullHgbp;

    sp<HGraphicBufferProducer2> igbp = surface ?
            surface->getHalInterface<HGraphicBufferProducer2>() : nullHgbp;
    if (surface && !igbp) {
        igbp = new B2HGraphicBufferProducer2(surface);
    }

    if (!surface) {
        mOutputBufferQueue.configure(nullIgbp, generation, 0);
    } else if (surface->getUniqueId(&bqId) != OK) {
        LOG(ERROR) << "setOutputSurface -- "
                   "cannot obtain bufferqueue id.";
        bqId = 0;
        mOutputBufferQueue.configure(nullIgbp, generation, 0);
    } else {
        mOutputBufferQueue.configure(surface, generation, bqId);
    }
    ALOGD("generation remote change %u", generation);

    Return<Status> transStatus = mBase->setOutputSurface(
            static_cast<uint64_t>(blockPoolId),
            bqId == 0 ? nullHgbp : igbp);
    if (!transStatus.isOk()) {
        LOG(ERROR) << "setOutputSurface -- transaction failed.";
        return C2_TRANSACTION_FAILED;
    }
    c2_status_t status =
            static_cast<c2_status_t>(static_cast<Status>(transStatus));
    if (status != C2_OK) {
        LOG(DEBUG) << "setOutputSurface -- call failed: " << status << ".";
    }
    return status;
}

status_t Codec2Client::Component::queueToOutputSurface(
        const C2ConstGraphicBlock& block,
        const QueueBufferInput& input,
        QueueBufferOutput* output) {
    return mOutputBufferQueue.outputBuffer(block, input, output);
}

c2_status_t Codec2Client::Component::connectToInputSurface(
        const std::shared_ptr<InputSurface>& inputSurface,
        std::shared_ptr<InputSurfaceConnection>* connection) {
    c2_status_t status;
    Return<void> transStatus = mBase->connectToInputSurface(
            inputSurface->mBase,
            [&status, connection](
                    Status s, const sp<IInputSurfaceConnection>& c) {
                status = static_cast<c2_status_t>(s);
                if (status != C2_OK) {
                    LOG(DEBUG) << "connectToInputSurface -- call failed: "
                               << status << ".";
                    return;
                }
                *connection = std::make_shared<InputSurfaceConnection>(c);
            });
    if (!transStatus.isOk()) {
        LOG(ERROR) << "connectToInputSurface -- transaction failed";
        return C2_TRANSACTION_FAILED;
    }
    return status;
}

c2_status_t Codec2Client::Component::connectToOmxInputSurface(
        const sp<HGraphicBufferProducer1>& producer,
        const sp<HGraphicBufferSource>& source,
        std::shared_ptr<InputSurfaceConnection>* connection) {
    c2_status_t status;
    Return<void> transStatus = mBase->connectToOmxInputSurface(
            producer, source,
            [&status, connection](
                    Status s, const sp<IInputSurfaceConnection>& c) {
                status = static_cast<c2_status_t>(s);
                if (status != C2_OK) {
                    LOG(DEBUG) << "connectToOmxInputSurface -- call failed: "
                               << status << ".";
                    return;
                }
                *connection = std::make_shared<InputSurfaceConnection>(c);
            });
    if (!transStatus.isOk()) {
        LOG(ERROR) << "connectToOmxInputSurface -- transaction failed.";
        return C2_TRANSACTION_FAILED;
    }
    return status;
}

c2_status_t Codec2Client::Component::disconnectFromInputSurface() {
    Return<Status> transStatus = mBase->disconnectFromInputSurface();
    if (!transStatus.isOk()) {
        LOG(ERROR) << "disconnectToInputSurface -- transaction failed.";
        return C2_TRANSACTION_FAILED;
    }
    c2_status_t status =
            static_cast<c2_status_t>(static_cast<Status>(transStatus));
    if (status != C2_OK) {
        LOG(DEBUG) << "disconnectFromInputSurface -- call failed: "
                   << status << ".";
    }
    return status;
}

c2_status_t Codec2Client::Component::setDeathListener(
        const std::shared_ptr<Component>& component,
        const std::shared_ptr<Listener>& listener) {

    struct HidlDeathRecipient : public hardware::hidl_death_recipient {
        std::weak_ptr<Component> component;
        std::weak_ptr<Listener> base;

        virtual void serviceDied(
                uint64_t /* cookie */,
                const wp<::android::hidl::base::V1_0::IBase>& /* who */
                ) override {
            if (std::shared_ptr<Codec2Client::Listener> listener = base.lock()) {
                listener->onDeath(component);
            } else {
                LOG(DEBUG) << "onDeath -- listener died.";
            }
        }
    };

    sp<HidlDeathRecipient> deathRecipient = new HidlDeathRecipient();
    deathRecipient->base = listener;
    deathRecipient->component = component;

    component->mDeathRecipient = deathRecipient;
    Return<bool> transResult = component->mBase->linkToDeath(
            component->mDeathRecipient, 0);
    if (!transResult.isOk()) {
        LOG(ERROR) << "setDeathListener -- linkToDeath() transaction failed.";
        return C2_TRANSACTION_FAILED;
    }
    if (!static_cast<bool>(transResult)) {
        LOG(DEBUG) << "setDeathListener -- linkToDeath() call failed.";
        return C2_CORRUPTED;
    }
    return C2_OK;
}

// Codec2Client::InputSurface
Codec2Client::InputSurface::InputSurface(const sp<IInputSurface>& base)
      : Configurable{
            [base]() -> sp<IConfigurable> {
                Return<sp<IConfigurable>> transResult =
                        base->getConfigurable();
                return transResult.isOk() ?
                        static_cast<sp<IConfigurable>>(transResult) :
                        nullptr;
            }()
        },
        mBase{base},
        mGraphicBufferProducer{new
            H2BGraphicBufferProducer2([base]() -> sp<HGraphicBufferProducer2> {
                Return<sp<HGraphicBufferProducer2>> transResult =
                        base->getGraphicBufferProducer();
                return transResult.isOk() ?
                        static_cast<sp<HGraphicBufferProducer2>>(transResult) :
                        nullptr;
            }())} {
}

sp<IGraphicBufferProducer>
        Codec2Client::InputSurface::getGraphicBufferProducer() const {
    return mGraphicBufferProducer;
}

sp<IInputSurface> Codec2Client::InputSurface::getHalInterface() const {
    return mBase;
}

// Codec2Client::InputSurfaceConnection
Codec2Client::InputSurfaceConnection::InputSurfaceConnection(
        const sp<IInputSurfaceConnection>& base)
      : Configurable{
            [base]() -> sp<IConfigurable> {
                Return<sp<IConfigurable>> transResult =
                        base->getConfigurable();
                return transResult.isOk() ?
                        static_cast<sp<IConfigurable>>(transResult) :
                        nullptr;
            }()
        },
        mBase{base} {
}

c2_status_t Codec2Client::InputSurfaceConnection::disconnect() {
    Return<Status> transResult = mBase->disconnect();
    return static_cast<c2_status_t>(static_cast<Status>(transResult));
}

}  // namespace android

