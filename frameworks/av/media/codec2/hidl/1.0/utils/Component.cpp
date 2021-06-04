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
#define LOG_TAG "Codec2-Component"
#include <android-base/logging.h>

#include <codec2/hidl/1.0/Component.h>
#include <codec2/hidl/1.0/ComponentStore.h>
#include <codec2/hidl/1.0/InputBufferManager.h>

#include <hidl/HidlBinderSupport.h>
#include <utils/Timers.h>

#include <C2BqBufferPriv.h>
#include <C2Debug.h>
#include <C2PlatformSupport.h>

#include <chrono>
#include <thread>

namespace android {
namespace hardware {
namespace media {
namespace c2 {
namespace V1_0 {
namespace utils {

using namespace ::android;

// ComponentListener wrapper
struct Component::Listener : public C2Component::Listener {

    Listener(const sp<Component>& component) :
        mComponent(component),
        mListener(component->mListener) {
    }

    virtual void onError_nb(
            std::weak_ptr<C2Component> /* c2component */,
            uint32_t errorCode) override {
        sp<IComponentListener> listener = mListener.promote();
        if (listener) {
            Return<void> transStatus = listener->onError(Status::OK, errorCode);
            if (!transStatus.isOk()) {
                LOG(ERROR) << "Component::Listener::onError_nb -- "
                           << "transaction failed.";
            }
        }
    }

    virtual void onTripped_nb(
            std::weak_ptr<C2Component> /* c2component */,
            std::vector<std::shared_ptr<C2SettingResult>> c2settingResult
            ) override {
        sp<IComponentListener> listener = mListener.promote();
        if (listener) {
            hidl_vec<SettingResult> settingResults(c2settingResult.size());
            size_t ix = 0;
            for (const std::shared_ptr<C2SettingResult> &c2result :
                    c2settingResult) {
                if (c2result) {
                    if (!objcpy(&settingResults[ix++], *c2result)) {
                        break;
                    }
                }
            }
            settingResults.resize(ix);
            Return<void> transStatus = listener->onTripped(settingResults);
            if (!transStatus.isOk()) {
                LOG(ERROR) << "Component::Listener::onTripped_nb -- "
                           << "transaction failed.";
            }
        }
    }

    virtual void onWorkDone_nb(
            std::weak_ptr<C2Component> /* c2component */,
            std::list<std::unique_ptr<C2Work>> c2workItems) override {
        for (const std::unique_ptr<C2Work>& work : c2workItems) {
            if (work) {
                if (work->worklets.empty()
                        || !work->worklets.back()
                        || (work->worklets.back()->output.flags &
                            C2FrameData::FLAG_INCOMPLETE) == 0) {
                    InputBufferManager::
                            unregisterFrameData(mListener, work->input);
                }
            }
        }

        sp<IComponentListener> listener = mListener.promote();
        if (listener) {
            WorkBundle workBundle;

            sp<Component> strongComponent = mComponent.promote();
            beginTransferBufferQueueBlocks(c2workItems, true);
            if (!objcpy(&workBundle, c2workItems, strongComponent ?
                    &strongComponent->mBufferPoolSender : nullptr)) {
                LOG(ERROR) << "Component::Listener::onWorkDone_nb -- "
                           << "received corrupted work items.";
                endTransferBufferQueueBlocks(c2workItems, false, true);
                return;
            }
            Return<void> transStatus = listener->onWorkDone(workBundle);
            if (!transStatus.isOk()) {
                LOG(ERROR) << "Component::Listener::onWorkDone_nb -- "
                           << "transaction failed.";
                endTransferBufferQueueBlocks(c2workItems, false, true);
                return;
            }
            endTransferBufferQueueBlocks(c2workItems, true, true);
        }
    }

protected:
    wp<Component> mComponent;
    wp<IComponentListener> mListener;
};

// Component::Sink
struct Component::Sink : public IInputSink {
    std::shared_ptr<Component> mComponent;
    sp<IConfigurable> mConfigurable;

    virtual Return<Status> queue(const WorkBundle& workBundle) override {
        return mComponent->queue(workBundle);
    }

    virtual Return<sp<IConfigurable>> getConfigurable() override {
        return mConfigurable;
    }

    Sink(const std::shared_ptr<Component>& component);
    virtual ~Sink() override;

    // Process-wide map: Component::Sink -> C2Component.
    static std::mutex sSink2ComponentMutex;
    static std::map<IInputSink*, std::weak_ptr<C2Component>> sSink2Component;

    static std::shared_ptr<C2Component> findLocalComponent(
            const sp<IInputSink>& sink);
};

std::mutex
        Component::Sink::sSink2ComponentMutex{};
std::map<IInputSink*, std::weak_ptr<C2Component>>
        Component::Sink::sSink2Component{};

Component::Sink::Sink(const std::shared_ptr<Component>& component)
        : mComponent{component},
          mConfigurable{[&component]() -> sp<IConfigurable> {
              Return<sp<IComponentInterface>> ret1 = component->getInterface();
              if (!ret1.isOk()) {
                  LOG(ERROR) << "Sink::Sink -- component's transaction failed.";
                  return nullptr;
              }
              Return<sp<IConfigurable>> ret2 =
                      static_cast<sp<IComponentInterface>>(ret1)->
                      getConfigurable();
              if (!ret2.isOk()) {
                  LOG(ERROR) << "Sink::Sink -- interface's transaction failed.";
                  return nullptr;
              }
              return static_cast<sp<IConfigurable>>(ret2);
          }()} {
    std::lock_guard<std::mutex> lock(sSink2ComponentMutex);
    sSink2Component.emplace(this, component->mComponent);
}

Component::Sink::~Sink() {
    std::lock_guard<std::mutex> lock(sSink2ComponentMutex);
    sSink2Component.erase(this);
}

std::shared_ptr<C2Component> Component::Sink::findLocalComponent(
        const sp<IInputSink>& sink) {
    std::lock_guard<std::mutex> lock(sSink2ComponentMutex);
    auto i = sSink2Component.find(sink.get());
    if (i == sSink2Component.end()) {
        return nullptr;
    }
    return i->second.lock();
}

// Component
Component::Component(
        const std::shared_ptr<C2Component>& component,
        const sp<IComponentListener>& listener,
        const sp<ComponentStore>& store,
        const sp<::android::hardware::media::bufferpool::V2_0::
        IClientManager>& clientPoolManager)
      : mComponent{component},
        mInterface{new ComponentInterface(component->intf(), store.get())},
        mListener{listener},
        mStore{store},
        mBufferPoolSender{clientPoolManager} {
    // Retrieve supported parameters from store
    // TODO: We could cache this per component/interface type
    mInit = mInterface->status();
}

c2_status_t Component::status() const {
    return mInit;
}

// Methods from ::android::hardware::media::c2::V1_0::IComponent
Return<Status> Component::queue(const WorkBundle& workBundle) {
    std::list<std::unique_ptr<C2Work>> c2works;

    if (!objcpy(&c2works, workBundle)) {
        return Status::CORRUPTED;
    }

    // Register input buffers.
    for (const std::unique_ptr<C2Work>& work : c2works) {
        if (work) {
            InputBufferManager::
                    registerFrameData(mListener, work->input);
        }
    }

    return static_cast<Status>(mComponent->queue_nb(&c2works));
}

Return<void> Component::flush(flush_cb _hidl_cb) {
    std::list<std::unique_ptr<C2Work>> c2flushedWorks;
    c2_status_t c2res = mComponent->flush_sm(
            C2Component::FLUSH_COMPONENT,
            &c2flushedWorks);

    // Unregister input buffers.
    for (const std::unique_ptr<C2Work>& work : c2flushedWorks) {
        if (work) {
            if (work->worklets.empty()
                    || !work->worklets.back()
                    || (work->worklets.back()->output.flags &
                        C2FrameData::FLAG_INCOMPLETE) == 0) {
                InputBufferManager::
                        unregisterFrameData(mListener, work->input);
            }
        }
    }

    WorkBundle flushedWorkBundle;
    Status res = static_cast<Status>(c2res);
    beginTransferBufferQueueBlocks(c2flushedWorks, true);
    if (c2res == C2_OK) {
        if (!objcpy(&flushedWorkBundle, c2flushedWorks, &mBufferPoolSender)) {
            res = Status::CORRUPTED;
        }
    }
    _hidl_cb(res, flushedWorkBundle);
    endTransferBufferQueueBlocks(c2flushedWorks, true, true);
    return Void();
}

Return<Status> Component::drain(bool withEos) {
    return static_cast<Status>(mComponent->drain_nb(withEos ?
            C2Component::DRAIN_COMPONENT_WITH_EOS :
            C2Component::DRAIN_COMPONENT_NO_EOS));
}

Return<Status> Component::setOutputSurface(
        uint64_t blockPoolId,
        const sp<HGraphicBufferProducer2>& surface) {
    std::shared_ptr<C2BlockPool> pool;
    GetCodec2BlockPool(blockPoolId, mComponent, &pool);
    if (pool && pool->getAllocatorId() == C2PlatformAllocatorStore::BUFFERQUEUE) {
        std::shared_ptr<C2BufferQueueBlockPool> bqPool =
                std::static_pointer_cast<C2BufferQueueBlockPool>(pool);
        C2BufferQueueBlockPool::OnRenderCallback cb =
            [this](uint64_t producer, int32_t slot, int64_t nsecs) {
                // TODO: batch this
                hidl_vec<IComponentListener::RenderedFrame> rendered;
                rendered.resize(1);
                rendered[0] = { producer, slot, nsecs };
                (void)mListener->onFramesRendered(rendered).isOk();
        };
        if (bqPool) {
            bqPool->setRenderCallback(cb);
            bqPool->configureProducer(surface);
        }
    }
    return Status::OK;
}

Return<void> Component::connectToInputSurface(
        const sp<IInputSurface>& inputSurface,
        connectToInputSurface_cb _hidl_cb) {
    Status status;
    sp<IInputSurfaceConnection> connection;
    auto transStatus = inputSurface->connect(
            asInputSink(),
            [&status, &connection](
                    Status s, const sp<IInputSurfaceConnection>& c) {
                status = s;
                connection = c;
            }
        );
    _hidl_cb(status, connection);
    return Void();
}

Return<void> Component::connectToOmxInputSurface(
        const sp<HGraphicBufferProducer1>& producer,
        const sp<::android::hardware::media::omx::V1_0::
        IGraphicBufferSource>& source,
        connectToOmxInputSurface_cb _hidl_cb) {
    (void)producer;
    (void)source;
    (void)_hidl_cb;
    return Void();
}

Return<Status> Component::disconnectFromInputSurface() {
    // TODO implement
    return Status::OK;
}

namespace /* unnamed */ {

struct BlockPoolIntf : public ConfigurableC2Intf {
    BlockPoolIntf(const std::shared_ptr<C2BlockPool>& pool)
          : ConfigurableC2Intf{
                "C2BlockPool:" +
                    (pool ? std::to_string(pool->getLocalId()) : "null"),
                0},
            mPool{pool} {
    }

    virtual c2_status_t config(
            const std::vector<C2Param*>& params,
            c2_blocking_t mayBlock,
            std::vector<std::unique_ptr<C2SettingResult>>* const failures
            ) override {
        (void)params;
        (void)mayBlock;
        (void)failures;
        return C2_OK;
    }

    virtual c2_status_t query(
            const std::vector<C2Param::Index>& indices,
            c2_blocking_t mayBlock,
            std::vector<std::unique_ptr<C2Param>>* const params
            ) const override {
        (void)indices;
        (void)mayBlock;
        (void)params;
        return C2_OK;
    }

    virtual c2_status_t querySupportedParams(
            std::vector<std::shared_ptr<C2ParamDescriptor>>* const params
            ) const override {
        (void)params;
        return C2_OK;
    }

    virtual c2_status_t querySupportedValues(
            std::vector<C2FieldSupportedValuesQuery>& fields,
            c2_blocking_t mayBlock) const override {
        (void)fields;
        (void)mayBlock;
        return C2_OK;
    }

protected:
    std::shared_ptr<C2BlockPool> mPool;
};

} // unnamed namespace

Return<void> Component::createBlockPool(
        uint32_t allocatorId,
        createBlockPool_cb _hidl_cb) {
    std::shared_ptr<C2BlockPool> blockPool;
    c2_status_t status = CreateCodec2BlockPool(
            static_cast<C2PlatformAllocatorStore::id_t>(allocatorId),
            mComponent,
            &blockPool);
    if (status != C2_OK) {
        blockPool = nullptr;
    }
    if (blockPool) {
        mBlockPoolsMutex.lock();
        mBlockPools.emplace(blockPool->getLocalId(), blockPool);
        mBlockPoolsMutex.unlock();
    } else if (status == C2_OK) {
        status = C2_CORRUPTED;
    }

    _hidl_cb(static_cast<Status>(status),
            blockPool ? blockPool->getLocalId() : 0,
            new CachedConfigurable(
            std::make_unique<BlockPoolIntf>(blockPool)));
    return Void();
}

Return<Status> Component::destroyBlockPool(uint64_t blockPoolId) {
    std::lock_guard<std::mutex> lock(mBlockPoolsMutex);
    return mBlockPools.erase(blockPoolId) == 1 ?
            Status::OK : Status::CORRUPTED;
}

Return<Status> Component::start() {
    return static_cast<Status>(mComponent->start());
}

Return<Status> Component::stop() {
    InputBufferManager::unregisterFrameData(mListener);
    return static_cast<Status>(mComponent->stop());
}

Return<Status> Component::reset() {
    Status status = static_cast<Status>(mComponent->reset());
    {
        std::lock_guard<std::mutex> lock(mBlockPoolsMutex);
        mBlockPools.clear();
    }
    InputBufferManager::unregisterFrameData(mListener);
    return status;
}

Return<Status> Component::release() {
    Status status = static_cast<Status>(mComponent->release());
    {
        std::lock_guard<std::mutex> lock(mBlockPoolsMutex);
        mBlockPools.clear();
    }
    InputBufferManager::unregisterFrameData(mListener);
    return status;
}

Return<sp<IComponentInterface>> Component::getInterface() {
    return sp<IComponentInterface>(mInterface);
}

Return<sp<IInputSink>> Component::asInputSink() {
    std::lock_guard<std::mutex> lock(mSinkMutex);
    if (!mSink) {
        mSink = new Sink(shared_from_this());
    }
    return {mSink};
}

std::shared_ptr<C2Component> Component::findLocalComponent(
        const sp<IInputSink>& sink) {
    return Component::Sink::findLocalComponent(sink);
}

void Component::initListener(const sp<Component>& self) {
    std::shared_ptr<C2Component::Listener> c2listener =
            std::make_shared<Listener>(self);
    c2_status_t res = mComponent->setListener_vb(c2listener, C2_DONT_BLOCK);
    if (res != C2_OK) {
        mInit = res;
    }
}

Component::~Component() {
    InputBufferManager::unregisterFrameData(mListener);
    mStore->reportComponentDeath(this);
}

}  // namespace utils
}  // namespace V1_0
}  // namespace c2
}  // namespace media
}  // namespace hardware
}  // namespace android

