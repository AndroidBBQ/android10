/*
 * Copyright 2016 The Android Open Source Project
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

#ifndef ANDROID_SURFACEREPLAYER_BUFFERQUEUESCHEDULER_H
#define ANDROID_SURFACEREPLAYER_BUFFERQUEUESCHEDULER_H

#include "Color.h"
#include "Event.h"

#include <gui/SurfaceControl.h>

#include <utils/StrongPointer.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>

namespace android {

auto constexpr LAYER_ALPHA = 190;

struct Dimensions {
    Dimensions() = default;
    Dimensions(int w, int h) : width(w), height(h) {}

    int width = 0;
    int height = 0;
};

struct BufferEvent {
    BufferEvent() = default;
    BufferEvent(std::shared_ptr<Event> e, Dimensions d) : event(e), dimensions(d) {}

    std::shared_ptr<Event> event;
    Dimensions dimensions;
};

class BufferQueueScheduler {
  public:
    BufferQueueScheduler(const sp<SurfaceControl>& surfaceControl, const HSV& color, int id);

    void startScheduling();
    void addEvent(const BufferEvent&);
    void stopScheduling();

    void setSurfaceControl(const sp<SurfaceControl>& surfaceControl, const HSV& color);

  private:
    void bufferUpdate(const Dimensions& dimensions);

    // Lock and fill the surface, block until the event is signaled by the main loop,
    // then unlock and post the buffer.
    void fillSurface(const std::shared_ptr<Event>& event);

    sp<SurfaceControl> mSurfaceControl;
    HSV mColor;
    const int mSurfaceId;

    bool mContinueScheduling;

    std::queue<BufferEvent> mBufferEvents;
    std::mutex mMutex;
    std::condition_variable mCondition;
};

}  // namespace android
#endif
