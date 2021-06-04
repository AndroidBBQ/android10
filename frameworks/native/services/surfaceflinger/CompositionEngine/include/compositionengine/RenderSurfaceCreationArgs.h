/*
 * Copyright 2019 The Android Open Source Project
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

#include <cstdint>
#include <memory>

#include <compositionengine/DisplaySurface.h>
#include <utils/StrongPointer.h>

struct ANativeWindow;

namespace android {

namespace compositionengine {

class Display;

/**
 * A parameter object for creating RenderSurface instances
 */
struct RenderSurfaceCreationArgs {
    // The initial width of the surface
    int32_t displayWidth;

    // The initial height of the surface
    int32_t displayHeight;

    // The ANativeWindow for the buffer queue for this surface
    sp<ANativeWindow> nativeWindow;

    // The DisplaySurface for this surface
    sp<DisplaySurface> displaySurface;
};

/**
 * A helper for setting up a RenderSurfaceCreationArgs value in-line.
 * Prefer this builder over raw structure initialization.
 *
 * Instead of:
 *
 *   RenderSurfaceCreationArgs{1000, 1000, nativeWindow, displaySurface}
 *
 * Prefer:
 *
 *  RenderSurfaceCreationArgsBuilder().setDisplayWidth(1000).setDisplayHeight(1000)
 *      .setNativeWindow(nativeWindow).setDisplaySurface(displaySurface).Build();
 */
class RenderSurfaceCreationArgsBuilder {
public:
    RenderSurfaceCreationArgs build() { return std::move(mArgs); }

    RenderSurfaceCreationArgsBuilder& setDisplayWidth(int32_t displayWidth) {
        mArgs.displayWidth = displayWidth;
        return *this;
    }
    RenderSurfaceCreationArgsBuilder& setDisplayHeight(int32_t displayHeight) {
        mArgs.displayHeight = displayHeight;
        return *this;
    }
    RenderSurfaceCreationArgsBuilder& setNativeWindow(sp<ANativeWindow> nativeWindow) {
        mArgs.nativeWindow = nativeWindow;
        return *this;
    }
    RenderSurfaceCreationArgsBuilder& setDisplaySurface(sp<DisplaySurface> displaySurface) {
        mArgs.displaySurface = displaySurface;
        return *this;
    }

private:
    RenderSurfaceCreationArgs mArgs;
};

} // namespace compositionengine
} // namespace android
