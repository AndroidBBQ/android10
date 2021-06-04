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

#include "NativeWindowSurface.h"

#include <gui/IGraphicBufferProducer.h>
#include <gui/Surface.h>

namespace android::surfaceflinger {

NativeWindowSurface::~NativeWindowSurface() = default;

namespace impl {

std::unique_ptr<surfaceflinger::NativeWindowSurface> createNativeWindowSurface(
        const sp<IGraphicBufferProducer>& producer) {
    class NativeWindowSurface final : public surfaceflinger::NativeWindowSurface {
    public:
        explicit NativeWindowSurface(const sp<IGraphicBufferProducer>& producer)
              : mSurface(new Surface(producer, /* controlledByApp */ false)) {}

        ~NativeWindowSurface() override = default;

        sp<ANativeWindow> getNativeWindow() const override { return mSurface; }

        void preallocateBuffers() override { mSurface->allocateBuffers(); }

    private:
        sp<Surface> mSurface;
    };

    return std::make_unique<NativeWindowSurface>(producer);
}

} // namespace impl
} // namespace android::surfaceflinger
