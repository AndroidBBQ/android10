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

#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "GLImage.h"

#include <vector>

#include <gui/DebugEGLImageTracker.h>
#include <log/log.h>
#include <utils/Trace.h>
#include "GLESRenderEngine.h"
#include "GLExtensions.h"

namespace android {
namespace renderengine {
namespace gl {

static std::vector<EGLint> buildAttributeList(bool isProtected) {
    std::vector<EGLint> attrs;
    attrs.reserve(16);

    attrs.push_back(EGL_IMAGE_PRESERVED_KHR);
    attrs.push_back(EGL_TRUE);

    if (isProtected && GLExtensions::getInstance().hasProtectedContent()) {
        attrs.push_back(EGL_PROTECTED_CONTENT_EXT);
        attrs.push_back(EGL_TRUE);
    }

    attrs.push_back(EGL_NONE);

    return attrs;
}

GLImage::GLImage(const GLESRenderEngine& engine) : mEGLDisplay(engine.getEGLDisplay()) {}

GLImage::~GLImage() {
    setNativeWindowBuffer(nullptr, false);
}

bool GLImage::setNativeWindowBuffer(ANativeWindowBuffer* buffer, bool isProtected) {
    ATRACE_CALL();
    if (mEGLImage != EGL_NO_IMAGE_KHR) {
        if (!eglDestroyImageKHR(mEGLDisplay, mEGLImage)) {
            ALOGE("failed to destroy image: %#x", eglGetError());
        }
        DEBUG_EGL_IMAGE_TRACKER_DESTROY();
        mEGLImage = EGL_NO_IMAGE_KHR;
    }

    if (buffer) {
        std::vector<EGLint> attrs = buildAttributeList(isProtected);
        mEGLImage = eglCreateImageKHR(mEGLDisplay, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
                                      static_cast<EGLClientBuffer>(buffer), attrs.data());
        if (mEGLImage == EGL_NO_IMAGE_KHR) {
            ALOGE("failed to create EGLImage: %#x", eglGetError());
            return false;
        }
        DEBUG_EGL_IMAGE_TRACKER_CREATE();
        mProtected = isProtected;
    }

    return true;
}

} // namespace gl
} // namespace renderengine
} // namespace android
