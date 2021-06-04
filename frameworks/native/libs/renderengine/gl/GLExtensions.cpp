/*
 * Copyright (C) 2010 The Android Open Source Project
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

#include "GLExtensions.h"

#include <string>
#include <unordered_set>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

ANDROID_SINGLETON_STATIC_INSTANCE(android::renderengine::gl::GLExtensions)

namespace android {
namespace renderengine {
namespace gl {

namespace {

class ExtensionSet {
public:
    ExtensionSet(const char* extensions) {
        char const* curr = extensions;
        char const* head = curr;
        do {
            head = strchr(curr, ' ');
            size_t len = head ? head - curr : strlen(curr);
            if (len > 0) {
                mExtensions.emplace(curr, len);
            }
            curr = head + 1;
        } while (head);
    }

    bool hasExtension(const char* extension) const { return mExtensions.count(extension) > 0; }

private:
    std::unordered_set<std::string> mExtensions;
};

} // anonymous namespace

void GLExtensions::initWithGLStrings(GLubyte const* vendor, GLubyte const* renderer,
                                     GLubyte const* version, GLubyte const* extensions) {
    mVendor = (char const*)vendor;
    mRenderer = (char const*)renderer;
    mVersion = (char const*)version;
    mExtensions = (char const*)extensions;

    ExtensionSet extensionSet(mExtensions.c_str());
    if (extensionSet.hasExtension("GL_EXT_protected_textures")) {
        mHasProtectedTexture = true;
    }
}

char const* GLExtensions::getVendor() const {
    return mVendor.string();
}

char const* GLExtensions::getRenderer() const {
    return mRenderer.string();
}

char const* GLExtensions::getVersion() const {
    return mVersion.string();
}

char const* GLExtensions::getExtensions() const {
    return mExtensions.string();
}

void GLExtensions::initWithEGLStrings(char const* eglVersion, char const* eglExtensions) {
    mEGLVersion = eglVersion;
    mEGLExtensions = eglExtensions;

    ExtensionSet extensionSet(eglExtensions);

    // EGL_ANDROIDX_no_config_context is an experimental extension with no
    // written specification. It will be replaced by something more formal.
    // SurfaceFlinger is using it to allow a single EGLContext to render to
    // both a 16-bit primary display framebuffer and a 32-bit virtual display
    // framebuffer.
    //
    // EGL_KHR_no_config_context is official extension to allow creating a
    // context that works with any surface of a display.
    if (extensionSet.hasExtension("EGL_ANDROIDX_no_config_context") ||
        extensionSet.hasExtension("EGL_KHR_no_config_context")) {
        mHasNoConfigContext = true;
    }

    if (extensionSet.hasExtension("EGL_ANDROID_native_fence_sync")) {
        mHasNativeFenceSync = true;
    }
    if (extensionSet.hasExtension("EGL_KHR_fence_sync")) {
        mHasFenceSync = true;
    }
    if (extensionSet.hasExtension("EGL_KHR_wait_sync")) {
        mHasWaitSync = true;
    }
    if (extensionSet.hasExtension("EGL_EXT_protected_content")) {
        mHasProtectedContent = true;
    }
    if (extensionSet.hasExtension("EGL_IMG_context_priority")) {
        mHasContextPriority = true;
    }
    if (extensionSet.hasExtension("EGL_KHR_surfaceless_context")) {
        mHasSurfacelessContext = true;
    }
}

char const* GLExtensions::getEGLVersion() const {
    return mEGLVersion.string();
}

char const* GLExtensions::getEGLExtensions() const {
    return mEGLExtensions.string();
}

} // namespace gl
} // namespace renderengine
} // namespace android
