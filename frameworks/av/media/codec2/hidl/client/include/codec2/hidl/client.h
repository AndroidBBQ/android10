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

#ifndef CODEC2_HIDL_CLIENT_H
#define CODEC2_HIDL_CLIENT_H

#include <gui/IGraphicBufferProducer.h>
#include <codec2/hidl/1.0/ClientBlockHelper.h>
#include <C2PlatformSupport.h>
#include <C2Component.h>
#include <C2Buffer.h>
#include <C2Param.h>
#include <C2.h>

#include <hidl/HidlSupport.h>
#include <utils/StrongPointer.h>

#include <functional>
#include <map>
#include <memory>
#include <mutex>

/**
 * This file contains minimal interfaces for the framework to access Codec2.0.
 *
 * Codec2Client is the main class that contains the following inner classes:
 * - Listener
 * - Configurable
 * - Interface
 * - Component
 *
 * Classes in Codec2Client, interfaces in Codec2.0, and  HIDL interfaces are
 * related as follows:
 * - Codec2Client <==> C2ComponentStore <==> IComponentStore
 * - Codec2Client::Listener <==> C2Component::Listener <==> IComponentListener
 * - Codec2Client::Configurable <==> [No equivalent] <==> IConfigurable
 * - Codec2Client::Interface <==> C2ComponentInterface <==> IComponentInterface
 * - Codec2Client::Component <==> C2Component <==> IComponent
 *
 * The entry point is Codec2Client::CreateFromService(), which creates a
 * Codec2Client object. From Codec2Client, Interface and Component objects can
 * be created by calling createComponent() and createInterface().
 *
 * createComponent() takes a Listener object, which must be implemented by the
 * user.
 *
 * At the present, createBlockPool() is the only method that yields a
 * Configurable object. Note, however, that Interface, Component and
 * Codec2Client are all subclasses of Configurable.
 */

// Forward declaration of relevant HIDL interfaces

namespace android::hardware::media::c2::V1_0 {
struct IConfigurable;
struct IComponent;
struct IComponentInterface;
struct IComponentStore;
struct IInputSink;
struct IInputSurface;
struct IInputSurfaceConnection;
}  // namespace android::hardware::media::c2::V1_0

namespace android::hardware::media::bufferpool::V2_0 {
struct IClientManager;
}  // namespace android::hardware::media::bufferpool::V2_0

namespace android::hardware::graphics::bufferqueue::V1_0 {
struct IGraphicBufferProducer;
}  // android::hardware::graphics::bufferqueue::V1_0

namespace android::hardware::media::omx::V1_0 {
struct IGraphicBufferSource;
}  // namespace android::hardware::media::omx::V1_0

namespace android {

// This class is supposed to be called Codec2Client::Configurable, but forward
// declaration of an inner class is not possible.
struct Codec2ConfigurableClient {

    typedef ::android::hardware::media::c2::V1_0::IConfigurable Base;

    const C2String& getName() const;

    c2_status_t query(
            const std::vector<C2Param*>& stackParams,
            const std::vector<C2Param::Index> &heapParamIndices,
            c2_blocking_t mayBlock,
            std::vector<std::unique_ptr<C2Param>>* const heapParams) const;

    c2_status_t config(
            const std::vector<C2Param*> &params,
            c2_blocking_t mayBlock,
            std::vector<std::unique_ptr<C2SettingResult>>* const failures);

    c2_status_t querySupportedParams(
            std::vector<std::shared_ptr<C2ParamDescriptor>>* const params
            ) const;

    c2_status_t querySupportedValues(
            std::vector<C2FieldSupportedValuesQuery>& fields,
            c2_blocking_t mayBlock) const;

    // base cannot be null.
    Codec2ConfigurableClient(const sp<Base>& base);

protected:
    sp<Base> mBase;
    C2String mName;

    friend struct Codec2Client;
};

struct Codec2Client : public Codec2ConfigurableClient {

    typedef ::android::hardware::media::c2::V1_0::IComponentStore Base;

    struct Listener;

    typedef Codec2ConfigurableClient Configurable;

    struct Component;

    struct Interface;

    struct InputSurface;

    struct InputSurfaceConnection;

    typedef Codec2Client Store;

    sp<Base> const& getBase() const;

    std::string const& getServiceName() const;

    c2_status_t createComponent(
            C2String const& name,
            std::shared_ptr<Listener> const& listener,
            std::shared_ptr<Component>* const component);

    c2_status_t createInterface(
            C2String const& name,
            std::shared_ptr<Interface>* const interface);

    c2_status_t createInputSurface(
            std::shared_ptr<InputSurface>* const inputSurface);

    std::vector<C2Component::Traits> const& listComponents() const;

    c2_status_t copyBuffer(
            std::shared_ptr<C2Buffer> const& src,
            std::shared_ptr<C2Buffer> const& dst);

    std::shared_ptr<C2ParamReflector> getParamReflector();

    // Returns the list of IComponentStore service names that are available on
    // the device. This list is specified at the build time in manifest files.
    // Note: A software service will have "_software" as a suffix.
    static std::vector<std::string> const& GetServiceNames();

    // Create a service with a given service name.
    static std::shared_ptr<Codec2Client> CreateFromService(char const* name);

    // Get clients to all services.
    static std::vector<std::shared_ptr<Codec2Client>> CreateFromAllServices();

    // Try to create a component with a given name from all known
    // IComponentStore services. numberOfAttempts determines the number of times
    // to retry the HIDL call if the transaction fails.
    static std::shared_ptr<Component> CreateComponentByName(
            char const* componentName,
            std::shared_ptr<Listener> const& listener,
            std::shared_ptr<Codec2Client>* owner = nullptr,
            size_t numberOfAttempts = 10);

    // Try to create a component interface with a given name from all known
    // IComponentStore services. numberOfAttempts determines the number of times
    // to retry the HIDL call if the transaction fails.
    static std::shared_ptr<Interface> CreateInterfaceByName(
            char const* interfaceName,
            std::shared_ptr<Codec2Client>* owner = nullptr,
            size_t numberOfAttempts = 10);

    // List traits from all known IComponentStore services.
    static std::vector<C2Component::Traits> const& ListComponents();

    // Create an input surface.
    static std::shared_ptr<InputSurface> CreateInputSurface(
            char const* serviceName = nullptr);

    // base cannot be null.
    Codec2Client(sp<Base> const& base, size_t serviceIndex);

protected:
    sp<Base> mBase;

    // Finds the first store where the predicate returns C2_OK and returns the
    // last predicate result. The predicate will be tried on all stores. The
    // function will return C2_OK the first time the predicate returns C2_OK,
    // or it will return the value from the last time that predicate is tried.
    // (The latter case corresponds to a failure on every store.) The order of
    // the stores to try is the same as the return value of GetServiceNames().
    //
    // key is used to remember the last store with which the predicate last
    // succeeded. If the last successful store is cached, it will be tried
    // first before all the stores are tried. Note that the last successful
    // store will be tried twice---first before all the stores, and another time
    // with all the stores.
    //
    // If an attempt to evaluate the predicate results in a transaction failure,
    // repeated attempts will be made until the predicate returns without a
    // transaction failure or numberOfAttempts attempts have been made.
    static c2_status_t ForAllServices(
            const std::string& key,
            size_t numberOfAttempts,
            std::function<c2_status_t(std::shared_ptr<Codec2Client> const&)>
                predicate);

    size_t mServiceIndex;
    mutable std::vector<C2Component::Traits> mTraitsList;

    sp<::android::hardware::media::bufferpool::V2_0::IClientManager>
            mHostPoolManager;

    static std::shared_ptr<Codec2Client> _CreateFromIndex(size_t index);

    std::vector<C2Component::Traits> _listComponents(bool* success) const;

    class Cache;
};

struct Codec2Client::Interface : public Codec2Client::Configurable {

    typedef ::android::hardware::media::c2::V1_0::IComponentInterface Base;

    Interface(const sp<Base>& base);

protected:
    sp<Base> mBase;
};

struct Codec2Client::Listener {

    // This is called when the component produces some output.
    virtual void onWorkDone(
            const std::weak_ptr<Component>& comp,
            std::list<std::unique_ptr<C2Work>>& workItems) = 0;

    // This is called when the component goes into a tripped state.
    virtual void onTripped(
            const std::weak_ptr<Component>& comp,
            const std::vector<std::shared_ptr<C2SettingResult>>& settingResults
            ) = 0;

    // This is called when the component encounters an error.
    virtual void onError(
            const std::weak_ptr<Component>& comp,
            uint32_t errorCode) = 0;

    // This is called when the process that hosts the component shuts down
    // unexpectedly.
    virtual void onDeath(
            const std::weak_ptr<Component>& comp) = 0;

    // This is called when an input buffer is no longer in use by the codec.
    // Input buffers that have been returned by onWorkDone() or flush() will not
    // trigger a call to this function.
    virtual void onInputBufferDone(
            uint64_t frameIndex, size_t arrayIndex) = 0;

    // This is called when the component becomes aware of a frame being
    // rendered.
    virtual void onFrameRendered(
            uint64_t bufferQueueId,
            int32_t slotId,
            int64_t timestampNs) = 0;

    virtual ~Listener();

};

struct Codec2Client::Component : public Codec2Client::Configurable {

    typedef ::android::hardware::media::c2::V1_0::IComponent Base;

    c2_status_t createBlockPool(
            C2Allocator::id_t id,
            C2BlockPool::local_id_t* blockPoolId,
            std::shared_ptr<Configurable>* configurable);

    c2_status_t destroyBlockPool(
            C2BlockPool::local_id_t localId);

    c2_status_t queue(
            std::list<std::unique_ptr<C2Work>>* const items);

    c2_status_t flush(
            C2Component::flush_mode_t mode,
            std::list<std::unique_ptr<C2Work>>* const flushedWork);

    c2_status_t drain(C2Component::drain_mode_t mode);

    c2_status_t start();

    c2_status_t stop();

    c2_status_t reset();

    c2_status_t release();

    typedef ::android::
            IGraphicBufferProducer IGraphicBufferProducer;
    typedef IGraphicBufferProducer::
            QueueBufferInput QueueBufferInput;
    typedef IGraphicBufferProducer::
            QueueBufferOutput QueueBufferOutput;

    typedef ::android::hardware::graphics::bufferqueue::V1_0::
            IGraphicBufferProducer HGraphicBufferProducer1;
    typedef ::android::hardware::graphics::bufferqueue::V2_0::
            IGraphicBufferProducer HGraphicBufferProducer2;
    typedef ::android::hardware::media::omx::V1_0::
            IGraphicBufferSource HGraphicBufferSource;

    // Set the output surface to be used with a blockpool previously created by
    // createBlockPool().
    c2_status_t setOutputSurface(
            C2BlockPool::local_id_t blockPoolId,
            const sp<IGraphicBufferProducer>& surface,
            uint32_t generation);

    // Extract a slot number from of the block, then call
    // IGraphicBufferProducer::queueBuffer().
    //
    // If the output surface has not been set, NO_INIT will be returned.
    //
    // If the block does not come from a bufferqueue-based blockpool,
    // attachBuffer() will be called, followed by queueBuffer().
    //
    // If the block has a bqId that does not match the id of the output surface,
    // DEAD_OBJECT will be returned.
    //
    // If the call to queueBuffer() is successful but the block cannot be
    // associated to the output surface for automatic cancellation upon
    // destruction, UNKNOWN_ERROR will be returned.
    //
    // Otherwise, the return value from queueBuffer() will be returned.
    status_t queueToOutputSurface(
            const C2ConstGraphicBlock& block,
            const QueueBufferInput& input,
            QueueBufferOutput* output);

    // Connect to a given InputSurface.
    c2_status_t connectToInputSurface(
            const std::shared_ptr<InputSurface>& inputSurface,
            std::shared_ptr<InputSurfaceConnection>* connection);

    c2_status_t connectToOmxInputSurface(
            const sp<HGraphicBufferProducer1>& producer,
            const sp<HGraphicBufferSource>& source,
            std::shared_ptr<InputSurfaceConnection>* connection);

    c2_status_t disconnectFromInputSurface();

    // base cannot be null.
    Component(const sp<Base>& base);

    ~Component();

protected:
    sp<Base> mBase;

    ::android::hardware::media::c2::V1_0::utils::DefaultBufferPoolSender
            mBufferPoolSender;

    ::android::hardware::media::c2::V1_0::utils::OutputBufferQueue
            mOutputBufferQueue;

    static c2_status_t setDeathListener(
            const std::shared_ptr<Component>& component,
            const std::shared_ptr<Listener>& listener);
    sp<::android::hardware::hidl_death_recipient> mDeathRecipient;

    friend struct Codec2Client;

    struct HidlListener;
    void handleOnWorkDone(const std::list<std::unique_ptr<C2Work>> &workItems);

};

struct Codec2Client::InputSurface : public Codec2Client::Configurable {
public:
    typedef ::android::hardware::media::c2::V1_0::IInputSurface Base;

    typedef ::android::hardware::media::c2::V1_0::IInputSurfaceConnection
            ConnectionBase;

    typedef Codec2Client::InputSurfaceConnection Connection;

    typedef ::android::IGraphicBufferProducer IGraphicBufferProducer;

    sp<IGraphicBufferProducer> getGraphicBufferProducer() const;

    // Return the underlying IInputSurface.
    sp<Base> getHalInterface() const;

    // base cannot be null.
    InputSurface(const sp<Base>& base);

protected:
    sp<Base> mBase;

    sp<IGraphicBufferProducer> mGraphicBufferProducer;

    friend struct Codec2Client;
    friend struct Component;
};

struct Codec2Client::InputSurfaceConnection : public Codec2Client::Configurable {

    typedef ::android::hardware::media::c2::V1_0::IInputSurfaceConnection Base;

    c2_status_t disconnect();

    // base cannot be null.
    InputSurfaceConnection(const sp<Base>& base);

protected:
    sp<Base> mBase;

    friend struct Codec2Client::InputSurface;
};

}  // namespace android

#endif  // CODEC2_HIDL_CLIENT_H

