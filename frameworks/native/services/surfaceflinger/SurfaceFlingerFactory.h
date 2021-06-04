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

#pragma once

#include <cinttypes>
#include <functional>
#include <memory>
#include <string>

#include <cutils/compiler.h>
#include <utils/StrongPointer.h>

namespace android {

typedef int32_t PixelFormat;

class BufferQueueLayer;
class BufferStateLayer;
class ColorLayer;
class ContainerLayer;
class DisplayDevice;
class DispSync;
class EventControlThread;
class GraphicBuffer;
class HWComposer;
class IGraphicBufferConsumer;
class IGraphicBufferProducer;
class MessageQueue;
class Scheduler;
class StartPropertySetThread;
class SurfaceFlinger;
class SurfaceInterceptor;
class TimeStats;

struct DisplayDeviceCreationArgs;
struct LayerCreationArgs;

namespace compositionengine {
class CompositionEngine;
} // namespace compositionengine

namespace scheduler {
class PhaseOffsets;
} // namespace scheduler
namespace surfaceflinger {

class NativeWindowSurface;

// The interface that SurfaceFlinger uses to create all of the implementations
// of each interface.
class Factory {
public:
    virtual std::unique_ptr<DispSync> createDispSync(const char* name, bool hasSyncFramework,
                                                     int64_t dispSyncPresentTimeOffset) = 0;
    virtual std::unique_ptr<EventControlThread> createEventControlThread(
            std::function<void(bool)> setVSyncEnabled) = 0;
    virtual std::unique_ptr<HWComposer> createHWComposer(const std::string& serviceName) = 0;
    virtual std::unique_ptr<MessageQueue> createMessageQueue() = 0;
    virtual std::unique_ptr<scheduler::PhaseOffsets> createPhaseOffsets() = 0;
    virtual std::unique_ptr<Scheduler> createScheduler(
            std::function<void(bool)> callback,
            const scheduler::RefreshRateConfigs& refreshRateConfig) = 0;
    virtual std::unique_ptr<SurfaceInterceptor> createSurfaceInterceptor(SurfaceFlinger*) = 0;

    virtual sp<StartPropertySetThread> createStartPropertySetThread(
            bool timestampPropertyValue) = 0;
    virtual sp<DisplayDevice> createDisplayDevice(DisplayDeviceCreationArgs&&) = 0;
    virtual sp<GraphicBuffer> createGraphicBuffer(uint32_t width, uint32_t height,
                                                  PixelFormat format, uint32_t layerCount,
                                                  uint64_t usage, std::string requestorName) = 0;
    virtual void createBufferQueue(sp<IGraphicBufferProducer>* outProducer,
                                   sp<IGraphicBufferConsumer>* outConsumer,
                                   bool consumerIsSurfaceFlinger) = 0;
    virtual std::unique_ptr<surfaceflinger::NativeWindowSurface> createNativeWindowSurface(
            const sp<IGraphicBufferProducer>&) = 0;

    virtual std::unique_ptr<compositionengine::CompositionEngine> createCompositionEngine() = 0;

    virtual sp<BufferQueueLayer> createBufferQueueLayer(const LayerCreationArgs& args) = 0;
    virtual sp<BufferStateLayer> createBufferStateLayer(const LayerCreationArgs& args) = 0;
    virtual sp<ColorLayer> createColorLayer(const LayerCreationArgs& args) = 0;
    virtual sp<ContainerLayer> createContainerLayer(const LayerCreationArgs& args) = 0;

    virtual std::shared_ptr<TimeStats> createTimeStats() = 0;

protected:
    ~Factory() = default;
};

ANDROID_API sp<SurfaceFlinger> createSurfaceFlinger();

} // namespace surfaceflinger
} // namespace android
