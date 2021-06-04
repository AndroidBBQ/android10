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
#define LOG_TAG "Codec2-ComponentStore"
#include <android-base/logging.h>

#include <codec2/hidl/1.0/ComponentStore.h>
#include <codec2/hidl/1.0/InputSurface.h>
#include <codec2/hidl/1.0/types.h>

#include <android-base/file.h>
#include <media/stagefright/bqhelper/GraphicBufferSource.h>
#include <utils/Errors.h>

#include <C2PlatformSupport.h>
#include <util/C2InterfaceHelper.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <ostream>
#include <sstream>

namespace android {
namespace hardware {
namespace media {
namespace c2 {
namespace V1_0 {
namespace utils {

using namespace ::android;
using ::android::GraphicBufferSource;
using namespace ::android::hardware::media::bufferpool::V2_0::implementation;

namespace /* unnamed */ {

struct StoreIntf : public ConfigurableC2Intf {
    StoreIntf(const std::shared_ptr<C2ComponentStore>& store)
          : ConfigurableC2Intf{store ? store->getName() : "", 0},
            mStore{store} {
    }

    virtual c2_status_t config(
            const std::vector<C2Param*> &params,
            c2_blocking_t mayBlock,
            std::vector<std::unique_ptr<C2SettingResult>> *const failures
            ) override {
        // Assume all params are blocking
        // TODO: Filter for supported params
        if (mayBlock == C2_DONT_BLOCK && params.size() != 0) {
            return C2_BLOCKING;
        }
        return mStore->config_sm(params, failures);
    }

    virtual c2_status_t query(
            const std::vector<C2Param::Index> &indices,
            c2_blocking_t mayBlock,
            std::vector<std::unique_ptr<C2Param>> *const params) const override {
        // Assume all params are blocking
        // TODO: Filter for supported params
        if (mayBlock == C2_DONT_BLOCK && indices.size() != 0) {
            return C2_BLOCKING;
        }
        return mStore->query_sm({}, indices, params);
    }

    virtual c2_status_t querySupportedParams(
            std::vector<std::shared_ptr<C2ParamDescriptor>> *const params
            ) const override {
        return mStore->querySupportedParams_nb(params);
    }

    virtual c2_status_t querySupportedValues(
            std::vector<C2FieldSupportedValuesQuery> &fields,
            c2_blocking_t mayBlock) const override {
        // Assume all params are blocking
        // TODO: Filter for supported params
        if (mayBlock == C2_DONT_BLOCK && fields.size() != 0) {
            return C2_BLOCKING;
        }
        return mStore->querySupportedValues_sm(fields);
    }

protected:
    std::shared_ptr<C2ComponentStore> mStore;
};

} // unnamed namespace

ComponentStore::ComponentStore(const std::shared_ptr<C2ComponentStore>& store)
      : mConfigurable{new CachedConfigurable(std::make_unique<StoreIntf>(store))},
        mStore{store} {

    std::shared_ptr<C2ComponentStore> platformStore = android::GetCodec2PlatformComponentStore();
    SetPreferredCodec2ComponentStore(store);

    // Retrieve struct descriptors
    mParamReflector = mStore->getParamReflector();

    // Retrieve supported parameters from store
    mInit = mConfigurable->init(this);
}

c2_status_t ComponentStore::status() const {
    return mInit;
}

c2_status_t ComponentStore::validateSupportedParams(
        const std::vector<std::shared_ptr<C2ParamDescriptor>>& params) {
    c2_status_t res = C2_OK;

    for (const std::shared_ptr<C2ParamDescriptor> &desc : params) {
        if (!desc) {
            // All descriptors should be valid
            res = res ? res : C2_BAD_VALUE;
            continue;
        }
        C2Param::CoreIndex coreIndex = desc->index().coreIndex();
        std::lock_guard<std::mutex> lock(mStructDescriptorsMutex);
        auto it = mStructDescriptors.find(coreIndex);
        if (it == mStructDescriptors.end()) {
            std::shared_ptr<C2StructDescriptor> structDesc =
                    mParamReflector->describe(coreIndex);
            if (!structDesc) {
                // All supported params must be described
                res = C2_BAD_INDEX;
            }
            mStructDescriptors.insert({ coreIndex, structDesc });
        }
    }
    return res;
}

// Methods from ::android::hardware::media::c2::V1_0::IComponentStore
Return<void> ComponentStore::createComponent(
        const hidl_string& name,
        const sp<IComponentListener>& listener,
        const sp<IClientManager>& pool,
        createComponent_cb _hidl_cb) {

    sp<Component> component;
    std::shared_ptr<C2Component> c2component;
    Status status = static_cast<Status>(
            mStore->createComponent(name, &c2component));

    if (status == Status::OK) {
        onInterfaceLoaded(c2component->intf());
        component = new Component(c2component, listener, this, pool);
        if (!component) {
            status = Status::CORRUPTED;
        } else {
            reportComponentBirth(component.get());
            if (component->status() != C2_OK) {
                status = static_cast<Status>(component->status());
            } else {
                component->initListener(component);
                if (component->status() != C2_OK) {
                    status = static_cast<Status>(component->status());
                }
            }
        }
    }
    _hidl_cb(status, component);
    return Void();
}

Return<void> ComponentStore::createInterface(
        const hidl_string& name,
        createInterface_cb _hidl_cb) {
    std::shared_ptr<C2ComponentInterface> c2interface;
    c2_status_t res = mStore->createInterface(name, &c2interface);
    sp<IComponentInterface> interface;
    if (res == C2_OK) {
        onInterfaceLoaded(c2interface);
        interface = new ComponentInterface(c2interface, this);
    }
    _hidl_cb(static_cast<Status>(res), interface);
    return Void();
}

Return<void> ComponentStore::listComponents(listComponents_cb _hidl_cb) {
    std::vector<std::shared_ptr<const C2Component::Traits>> c2traits =
            mStore->listComponents();
    hidl_vec<IComponentStore::ComponentTraits> traits(c2traits.size());
    size_t ix = 0;
    for (const std::shared_ptr<const C2Component::Traits> &c2trait : c2traits) {
        if (c2trait) {
            if (objcpy(&traits[ix], *c2trait)) {
                ++ix;
            } else {
                break;
            }
        }
    }
    traits.resize(ix);
    _hidl_cb(Status::OK, traits);
    return Void();
}

Return<void> ComponentStore::createInputSurface(createInputSurface_cb _hidl_cb) {
    sp<GraphicBufferSource> source = new GraphicBufferSource();
    if (source->initCheck() != OK) {
        _hidl_cb(Status::CORRUPTED, nullptr);
        return Void();
    }
    sp<InputSurface> inputSurface = new InputSurface(
            this,
            std::make_shared<C2ReflectorHelper>(),
            source->getHGraphicBufferProducer(),
            source);
    _hidl_cb(inputSurface ? Status::OK : Status::NO_MEMORY,
             inputSurface);
    return Void();
}

void ComponentStore::onInterfaceLoaded(const std::shared_ptr<C2ComponentInterface> &intf) {
    // invalidate unsupported struct descriptors if a new interface is loaded as it may have
    // exposed new descriptors
    std::lock_guard<std::mutex> lock(mStructDescriptorsMutex);
    if (!mLoadedInterfaces.count(intf->getName())) {
        mUnsupportedStructDescriptors.clear();
        mLoadedInterfaces.emplace(intf->getName());
    }
}

Return<void> ComponentStore::getStructDescriptors(
        const hidl_vec<uint32_t>& indices,
        getStructDescriptors_cb _hidl_cb) {
    hidl_vec<StructDescriptor> descriptors(indices.size());
    size_t dstIx = 0;
    Status res = Status::OK;
    for (size_t srcIx = 0; srcIx < indices.size(); ++srcIx) {
        std::lock_guard<std::mutex> lock(mStructDescriptorsMutex);
        const C2Param::CoreIndex coreIndex = C2Param::CoreIndex(indices[srcIx]).coreIndex();
        const auto item = mStructDescriptors.find(coreIndex);
        if (item == mStructDescriptors.end()) {
            // not in the cache, and not known to be unsupported, query local reflector
            if (!mUnsupportedStructDescriptors.count(coreIndex)) {
                std::shared_ptr<C2StructDescriptor> structDesc =
                    mParamReflector->describe(coreIndex);
                if (!structDesc) {
                    mUnsupportedStructDescriptors.emplace(coreIndex);
                } else {
                    mStructDescriptors.insert({ coreIndex, structDesc });
                    if (objcpy(&descriptors[dstIx], *structDesc)) {
                        ++dstIx;
                        continue;
                    }
                    res = Status::CORRUPTED;
                    break;
                }
            }
            res = Status::NOT_FOUND;
        } else if (item->second) {
            if (objcpy(&descriptors[dstIx], *item->second)) {
                ++dstIx;
                continue;
            }
            res = Status::CORRUPTED;
            break;
        } else {
            res = Status::NO_MEMORY;
            break;
        }
    }
    descriptors.resize(dstIx);
    _hidl_cb(res, descriptors);
    return Void();
}

Return<sp<IClientManager>> ComponentStore::getPoolClientManager() {
    return ClientManager::getInstance();
}

Return<Status> ComponentStore::copyBuffer(const Buffer& src, const Buffer& dst) {
    // TODO implement
    (void)src;
    (void)dst;
    return Status::OMITTED;
}

Return<sp<IConfigurable>> ComponentStore::getConfigurable() {
    return mConfigurable;
}

// Called from createComponent() after a successful creation of `component`.
void ComponentStore::reportComponentBirth(Component* component) {
    ComponentStatus componentStatus;
    componentStatus.c2Component = component->mComponent;
    componentStatus.birthTime = std::chrono::system_clock::now();

    std::lock_guard<std::mutex> lock(mComponentRosterMutex);
    mComponentRoster.emplace(component, componentStatus);
}

// Called from within the destructor of `component`. No virtual function calls
// are made on `component` here.
void ComponentStore::reportComponentDeath(Component* component) {
    std::lock_guard<std::mutex> lock(mComponentRosterMutex);
    mComponentRoster.erase(component);
}

// Dumps component traits.
std::ostream& ComponentStore::dump(
        std::ostream& out,
        const std::shared_ptr<const C2Component::Traits>& comp) {

    constexpr const char indent[] = "    ";

    out << indent << "name: " << comp->name << std::endl;
    out << indent << "domain: " << comp->domain << std::endl;
    out << indent << "kind: " << comp->kind << std::endl;
    out << indent << "rank: " << comp->rank << std::endl;
    out << indent << "mediaType: " << comp->mediaType << std::endl;
    out << indent << "aliases:";
    for (const auto& alias : comp->aliases) {
        out << ' ' << alias;
    }
    out << std::endl;

    return out;
}

// Dumps component status.
std::ostream& ComponentStore::dump(
        std::ostream& out,
        ComponentStatus& compStatus) {

    constexpr const char indent[] = "    ";

    // Print birth time.
    std::chrono::milliseconds ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                compStatus.birthTime.time_since_epoch());
    std::time_t birthTime = std::chrono::system_clock::to_time_t(
            compStatus.birthTime);
    std::tm tm = *std::localtime(&birthTime);
    out << indent << "Creation time: "
        << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count() % 1000
        << std::endl;

    // Print name and id.
    std::shared_ptr<C2ComponentInterface> intf = compStatus.c2Component->intf();
    if (!intf) {
        out << indent << "Unknown component -- null interface" << std::endl;
        return out;
    }
    out << indent << "Name: " << intf->getName() << std::endl;
    out << indent << "Id: " << intf->getId() << std::endl;

    return out;
}

// Dumps information when lshal is called.
Return<void> ComponentStore::debug(
        const hidl_handle& handle,
        const hidl_vec<hidl_string>& /* args */) {
    LOG(INFO) << "debug -- dumping...";
    const native_handle_t *h = handle.getNativeHandle();
    if (!h || h->numFds != 1) {
       LOG(ERROR) << "debug -- dumping failed -- "
               "invalid file descriptor to dump to";
       return Void();
    }
    std::ostringstream out;

    { // Populate "out".

        constexpr const char indent[] = "  ";

        // Show name.
        out << "Beginning of dump -- C2ComponentStore: "
                << mStore->getName() << std::endl << std::endl;

        // Retrieve the list of supported components.
        std::vector<std::shared_ptr<const C2Component::Traits>> traitsList =
                mStore->listComponents();

        // Dump the traits of supported components.
        out << indent << "Supported components:" << std::endl << std::endl;
        if (traitsList.size() == 0) {
            out << indent << indent << "NONE" << std::endl << std::endl;
        } else {
            for (const auto& traits : traitsList) {
                dump(out, traits) << std::endl;
            }
        }

        // Dump active components.
        {
            out << indent << "Active components:" << std::endl << std::endl;
            std::lock_guard<std::mutex> lock(mComponentRosterMutex);
            if (mComponentRoster.size() == 0) {
                out << indent << indent << "NONE" << std::endl << std::endl;
            } else {
                for (auto& pair : mComponentRoster) {
                    dump(out, pair.second) << std::endl;
                }
            }
        }

        out << "End of dump -- C2ComponentStore: "
                << mStore->getName() << std::endl;
    }

    if (!android::base::WriteStringToFd(out.str(), h->data[0])) {
        PLOG(WARNING) << "debug -- dumping failed -- write()";
    } else {
        LOG(INFO) << "debug -- dumping succeeded";
    }
    return Void();
}


}  // namespace utils
}  // namespace V1_0
}  // namespace c2
}  // namespace media
}  // namespace hardware
}  // namespace android

