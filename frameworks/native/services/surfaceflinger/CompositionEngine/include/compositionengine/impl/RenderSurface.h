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

#include <memory>

#include <android-base/unique_fd.h>
#include <compositionengine/RenderSurface.h>
#include <utils/StrongPointer.h>

struct ANativeWindow;

namespace android {

namespace compositionengine {

class CompositionEngine;
class Display;
class DisplaySurface;

struct RenderSurfaceCreationArgs;

namespace impl {

class RenderSurface : public compositionengine::RenderSurface {
public:
    RenderSurface(const CompositionEngine&, compositionengine::Display&,
                  compositionengine::RenderSurfaceCreationArgs&&);
    ~RenderSurface() override;

    bool isValid() const override;
    void initialize() override;
    const ui::Size& getSize() const override;
    bool isProtected() const override { return mProtected; }

    const sp<Fence>& getClientTargetAcquireFence() const override;
    void setBufferDataspace(ui::Dataspace) override;
    void setDisplaySize(const ui::Size&) override;
    void setProtected(bool useProtected) override;
    status_t beginFrame(bool mustRecompose) override;
    status_t prepareFrame() override;
    sp<GraphicBuffer> dequeueBuffer(base::unique_fd* bufferFence) override;
    void queueBuffer(base::unique_fd&& readyFence) override;
    void onPresentDisplayCompleted() override;
    void setViewportAndProjection() override;
    void flip() override;

    // Debugging
    void dump(std::string& result) const override;
    std::uint32_t getPageFlipCount() const override;

    // Testing
    void setPageFlipCountForTest(std::uint32_t);
    void setSizeForTest(const ui::Size&);
    sp<GraphicBuffer>& mutableGraphicBufferForTest();
    base::unique_fd& mutableBufferReadyForTest();

private:
    const compositionengine::CompositionEngine& mCompositionEngine;
    const compositionengine::Display& mDisplay;

    // ANativeWindow being rendered into
    const sp<ANativeWindow> mNativeWindow;
    // Current buffer being rendered into
    sp<GraphicBuffer> mGraphicBuffer;
    const sp<DisplaySurface> mDisplaySurface;
    ui::Size mSize;
    bool mProtected{false};
    std::uint32_t mPageFlipCount{0};
};

std::unique_ptr<compositionengine::RenderSurface> createRenderSurface(
        const compositionengine::CompositionEngine&, compositionengine::Display&,
        compositionengine::RenderSurfaceCreationArgs&&);

} // namespace impl
} // namespace compositionengine
} // namespace android
