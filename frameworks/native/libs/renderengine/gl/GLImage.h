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

#include <cstdint>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <android-base/macros.h>
#include <renderengine/Image.h>

struct ANativeWindowBuffer;

namespace android {
namespace renderengine {
namespace gl {

class GLESRenderEngine;

class GLImage : public renderengine::Image {
public:
    explicit GLImage(const GLESRenderEngine& engine);
    ~GLImage() override;

    bool setNativeWindowBuffer(ANativeWindowBuffer* buffer, bool isProtected) override;

    EGLImageKHR getEGLImage() const { return mEGLImage; }
    bool isProtected() const { return mProtected; }

private:
    EGLDisplay mEGLDisplay;
    EGLImageKHR mEGLImage = EGL_NO_IMAGE_KHR;
    bool mProtected = false;

    DISALLOW_COPY_AND_ASSIGN(GLImage);
};

} // namespace gl
} // namespace renderengine
} // namespace android
