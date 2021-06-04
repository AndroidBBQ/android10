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

#ifndef CODEC2_HIDL_V1_0_UTILS_COMPONENT_H
#define CODEC2_HIDL_V1_0_UTILS_COMPONENT_H

#include <codec2/hidl/1.0/ComponentInterface.h>
#include <codec2/hidl/1.0/Configurable.h>
#include <codec2/hidl/1.0/types.h>

#include <android/hardware/media/bufferpool/2.0/IClientManager.h>
#include <android/hardware/media/c2/1.0/IComponent.h>
#include <android/hardware/media/c2/1.0/IComponentInterface.h>
#include <android/hardware/media/c2/1.0/IComponentListener.h>
#include <android/hardware/media/c2/1.0/IComponentStore.h>
#include <android/hardware/media/c2/1.0/IInputSink.h>
#include <hidl/Status.h>
#include <hwbinder/IBinder.h>

#include <C2Component.h>
#include <C2Buffer.h>
#include <C2.h>

#include <map>
#include <memory>
#include <mutex>

namespace android {
namespace hardware {
namespace media {
namespace c2 {
namespace V1_0 {
namespace utils {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::IBinder;
using ::android::sp;
using ::android::wp;

struct ComponentStore;

struct Component : public IComponent,
                   public std::enable_shared_from_this<Component> {
    Component(
            const std::shared_ptr<C2Component>&,
            const sp<IComponentListener>& listener,
            const sp<ComponentStore>& store,
            const sp<::android::hardware::media::bufferpool::V2_0::
                IClientManager>& clientPoolManager);
    c2_status_t status() const;

    typedef ::android::hardware::graphics::bufferqueue::V1_0::
            IGraphicBufferProducer HGraphicBufferProducer1;
    typedef ::android::hardware::graphics::bufferqueue::V2_0::
            IGraphicBufferProducer HGraphicBufferProducer2;

    // Methods from IComponent follow.
    virtual Return<Status> queue(const WorkBundle& workBundle) override;
    virtual Return<void> flush(flush_cb _hidl_cb) override;
    virtual Return<Status> drain(bool withEos) override;
    virtual Return<Status> setOutputSurface(
            uint64_t blockPoolId,
            const sp<HGraphicBufferProducer2>& surface) override;
    virtual Return<void> connectToInputSurface(
            const sp<IInputSurface>& inputSurface,
            connectToInputSurface_cb _hidl_cb) override;
    virtual Return<void> connectToOmxInputSurface(
            const sp<HGraphicBufferProducer1>& producer,
            const sp<::android::hardware::media::omx::V1_0::
            IGraphicBufferSource>& source,
            connectToOmxInputSurface_cb _hidl_cb) override;
    virtual Return<Status> disconnectFromInputSurface() override;
    virtual Return<void> createBlockPool(
            uint32_t allocatorId,
            createBlockPool_cb _hidl_cb) override;
    virtual Return<Status> destroyBlockPool(uint64_t blockPoolId) override;
    virtual Return<Status> start() override;
    virtual Return<Status> stop() override;
    virtual Return<Status> reset() override;
    virtual Return<Status> release() override;
    virtual Return<sp<IComponentInterface>> getInterface() override;
    virtual Return<sp<IInputSink>> asInputSink() override;

    // Returns a C2Component associated to the given sink if the sink is indeed
    // a local component. Returns nullptr otherwise.
    //
    // This function is used by InputSurface::connect().
    static std::shared_ptr<C2Component> findLocalComponent(
            const sp<IInputSink>& sink);

protected:
    c2_status_t mInit;
    std::shared_ptr<C2Component> mComponent;
    sp<ComponentInterface> mInterface;
    sp<IComponentListener> mListener;
    sp<ComponentStore> mStore;
    ::android::hardware::media::c2::V1_0::utils::DefaultBufferPoolSender
            mBufferPoolSender;

    struct Sink;
    std::mutex mSinkMutex;
    sp<Sink> mSink;

    std::mutex mBlockPoolsMutex;
    // This map keeps C2BlockPool objects that are created by createBlockPool()
    // alive. These C2BlockPool objects can be deleted by calling
    // destroyBlockPool(), reset() or release(), or by destroying the component.
    std::map<uint64_t, std::shared_ptr<C2BlockPool>> mBlockPools;

    void initListener(const sp<Component>& self);

    virtual ~Component() override;

    friend struct ComponentStore;

    struct Listener;
};

}  // namespace utils
}  // namespace V1_0
}  // namespace c2
}  // namespace media
}  // namespace hardware
}  // namespace android

#endif  // CODEC2_HIDL_V1_0_UTILS_COMPONENT_H
