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

#include <compositionengine/impl/CompositionEngine.h>
#include <ui/GraphicBuffer.h>

#include "BufferQueueLayer.h"
#include "BufferStateLayer.h"
#include "ColorLayer.h"
#include "ContainerLayer.h"
#include "DisplayDevice.h"
#include "Layer.h"
#include "NativeWindowSurface.h"
#include "StartPropertySetThread.h"
#include "SurfaceFlinger.h"
#include "SurfaceFlingerFactory.h"
#include "SurfaceInterceptor.h"

#include "DisplayHardware/ComposerHal.h"
#include "Scheduler/DispSync.h"
#include "Scheduler/EventControlThread.h"
#include "Scheduler/MessageQueue.h"
#include "Scheduler/PhaseOffsets.h"
#include "Scheduler/Scheduler.h"
#include "TimeStats/TimeStats.h"

namespace android::surfaceflinger {

sp<SurfaceFlinger> createSurfaceFlinger() {
    class Factory final : public surfaceflinger::Factory {
    public:
        Factory() = default;
        ~Factory() = default;

        std::unique_ptr<DispSync> createDispSync(const char* name, bool hasSyncFramework,
                                                 int64_t dispSyncPresentTimeOffset) override {
            // Note: We create a local temporary with the real DispSync implementation
            // type temporarily so we can initialize it with the configured values,
            // before storing it for more generic use using the interface type.
            auto primaryDispSync = std::make_unique<android::impl::DispSync>(name);
            primaryDispSync->init(hasSyncFramework, dispSyncPresentTimeOffset);
            return primaryDispSync;
        }

        std::unique_ptr<EventControlThread> createEventControlThread(
                std::function<void(bool)> setVSyncEnabled) override {
            return std::make_unique<android::impl::EventControlThread>(setVSyncEnabled);
        }

        std::unique_ptr<HWComposer> createHWComposer(const std::string& serviceName) override {
            return std::make_unique<android::impl::HWComposer>(
                    std::make_unique<Hwc2::impl::Composer>(serviceName));
        }

        std::unique_ptr<MessageQueue> createMessageQueue() override {
            return std::make_unique<android::impl::MessageQueue>();
        }

        std::unique_ptr<scheduler::PhaseOffsets> createPhaseOffsets() override {
            return std::make_unique<scheduler::impl::PhaseOffsets>();
        }

        std::unique_ptr<Scheduler> createScheduler(
                std::function<void(bool)> callback,
                const scheduler::RefreshRateConfigs& refreshRateConfig) override {
            return std::make_unique<Scheduler>(callback, refreshRateConfig);
        }

        std::unique_ptr<SurfaceInterceptor> createSurfaceInterceptor(
                SurfaceFlinger* flinger) override {
            return std::make_unique<android::impl::SurfaceInterceptor>(flinger);
        }

        sp<StartPropertySetThread> createStartPropertySetThread(
                bool timestampPropertyValue) override {
            return new StartPropertySetThread(timestampPropertyValue);
        }

        sp<DisplayDevice> createDisplayDevice(DisplayDeviceCreationArgs&& creationArgs) override {
            return new DisplayDevice(std::move(creationArgs));
        }

        sp<GraphicBuffer> createGraphicBuffer(uint32_t width, uint32_t height, PixelFormat format,
                                              uint32_t layerCount, uint64_t usage,
                                              std::string requestorName) override {
            return new GraphicBuffer(width, height, format, layerCount, usage, requestorName);
        }

        void createBufferQueue(sp<IGraphicBufferProducer>* outProducer,
                               sp<IGraphicBufferConsumer>* outConsumer,
                               bool consumerIsSurfaceFlinger) override {
            BufferQueue::createBufferQueue(outProducer, outConsumer, consumerIsSurfaceFlinger);
        }

        std::unique_ptr<surfaceflinger::NativeWindowSurface> createNativeWindowSurface(
                const sp<IGraphicBufferProducer>& producer) override {
            return surfaceflinger::impl::createNativeWindowSurface(producer);
        }

        std::unique_ptr<compositionengine::CompositionEngine> createCompositionEngine() override {
            return compositionengine::impl::createCompositionEngine();
        }

        sp<ContainerLayer> createContainerLayer(const LayerCreationArgs& args) override {
            return new ContainerLayer(args);
        }

        sp<BufferQueueLayer> createBufferQueueLayer(const LayerCreationArgs& args) override {
            return new BufferQueueLayer(args);
        }

        sp<BufferStateLayer> createBufferStateLayer(const LayerCreationArgs& args) override {
            return new BufferStateLayer(args);
        }

        sp<ColorLayer> createColorLayer(const LayerCreationArgs& args) override {
            return new ColorLayer(args);
        }

        std::shared_ptr<TimeStats> createTimeStats() override {
            return std::make_shared<android::impl::TimeStats>();
        }
    };
    static Factory factory;

    return new SurfaceFlinger(factory);
}

} // namespace android::surfaceflinger
