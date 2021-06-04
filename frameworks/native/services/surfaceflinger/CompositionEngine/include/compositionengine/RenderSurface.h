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
#include <vector>

#include <ui/Fence.h>
#include <ui/GraphicTypes.h>
#include <ui/Size.h>
#include <utils/Errors.h>
#include <utils/StrongPointer.h>

namespace android {

class GraphicBuffer;

namespace compositionengine {

/**
 * Encapsulates everything for composing to a render surface with RenderEngine
 */
class RenderSurface {
public:
    virtual ~RenderSurface();

    // Returns true if the render surface is valid. This is meant to be checked
    // post-construction and prior to use, as not everything is set up by the
    // constructor.
    virtual bool isValid() const = 0;

    // Performs one-time initialization of the render surface. This is meant
    // to be called after the validation check.
    virtual void initialize() = 0;

    // Returns the bounds of the surface
    virtual const ui::Size& getSize() const = 0;

    // Returns whether the surface is protected.
    virtual bool isProtected() const = 0;

    // Gets the latest fence to pass to the HWC to signal that the surface
    // buffer is done rendering
    virtual const sp<Fence>& getClientTargetAcquireFence() const = 0;

    // Sets the size of the surface
    virtual void setDisplaySize(const ui::Size&) = 0;

    // Sets the dataspace used for rendering the surface
    virtual void setBufferDataspace(ui::Dataspace) = 0;

    // Configures the protected rendering on the surface
    virtual void setProtected(bool useProtected) = 0;

    // Called to signal that rendering has started. 'mustRecompose' should be
    // true if the entire frame must be recomposed.
    virtual status_t beginFrame(bool mustRecompose) = 0;

    // Prepares the frame for rendering
    virtual status_t prepareFrame() = 0;

    // Allocates a buffer as scratch space for GPU composition
    virtual sp<GraphicBuffer> dequeueBuffer(base::unique_fd* bufferFence) = 0;

    // Queues the drawn buffer for consumption by HWC. readyFence is the fence
    // which will fire when the buffer is ready for consumption.
    virtual void queueBuffer(base::unique_fd&& readyFence) = 0;

    // Called after the HWC calls are made to present the display
    virtual void onPresentDisplayCompleted() = 0;

    // Called to set the viewport and projection state for rendering into this
    // surface
    virtual void setViewportAndProjection() = 0;

    // Called after the surface has been rendering to signal the surface should
    // be made ready for displaying
    virtual void flip() = 0;

    // Debugging - Dumps the state of the RenderSurface to a string
    virtual void dump(std::string& result) const = 0;

    // Debugging - gets the page flip count for the RenderSurface
    virtual std::uint32_t getPageFlipCount() const = 0;
};

} // namespace compositionengine
} // namespace android
