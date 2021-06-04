/*
 * Copyright 2013 The Android Open Source Project
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

#include <renderengine/RenderEngine.h>

#include <cutils/properties.h>
#include <log/log.h>
#include <private/gui/SyncFeatures.h>
#include "gl/GLESRenderEngine.h"

namespace android {
namespace renderengine {

std::unique_ptr<impl::RenderEngine> RenderEngine::create(int hwcFormat, uint32_t featureFlags,
                                                         uint32_t imageCacheSize) {
    char prop[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_DEBUG_RENDERENGINE_BACKEND, prop, "gles");
    if (strcmp(prop, "gles") == 0) {
        ALOGD("RenderEngine GLES Backend");
        return renderengine::gl::GLESRenderEngine::create(hwcFormat, featureFlags, imageCacheSize);
    }
    ALOGE("UNKNOWN BackendType: %s, create GLES RenderEngine.", prop);
    return renderengine::gl::GLESRenderEngine::create(hwcFormat, featureFlags, imageCacheSize);
}

RenderEngine::~RenderEngine() = default;

namespace impl {

RenderEngine::RenderEngine(uint32_t featureFlags) : mFeatureFlags(featureFlags) {}

RenderEngine::~RenderEngine() = default;

bool RenderEngine::useNativeFenceSync() const {
    return SyncFeatures::getInstance().useNativeFenceSync();
}

bool RenderEngine::useWaitSync() const {
    return SyncFeatures::getInstance().useWaitSync();
}

} // namespace impl
} // namespace renderengine
} // namespace android
