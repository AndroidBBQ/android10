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

#include <memory>

#include <utils/StrongPointer.h>

struct ANativeWindow;

namespace android {

class IGraphicBufferProducer;

namespace surfaceflinger {

// A thin interface to abstract creating instances of Surface (gui/Surface.h) to
// use as a NativeWindow.
class NativeWindowSurface {
public:
    virtual ~NativeWindowSurface();

    // Gets the NativeWindow to use for the surface.
    virtual sp<ANativeWindow> getNativeWindow() const = 0;

    // Indicates that the surface should allocate its buffers now.
    virtual void preallocateBuffers() = 0;
};

namespace impl {

std::unique_ptr<NativeWindowSurface> createNativeWindowSurface(const sp<IGraphicBufferProducer>&);

} // namespace impl
} // namespace surfaceflinger
} // namespace android
