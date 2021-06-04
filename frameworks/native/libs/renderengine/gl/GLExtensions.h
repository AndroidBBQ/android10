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

#ifndef ANDROID_SF_GLEXTENSION_H
#define ANDROID_SF_GLEXTENSION_H

#include <stdint.h>
#include <sys/types.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <utils/Singleton.h>
#include <utils/String8.h>

namespace android {
namespace renderengine {
namespace gl {

class GLExtensions : public Singleton<GLExtensions> {
public:
    bool hasNoConfigContext() const { return mHasNoConfigContext; }
    bool hasNativeFenceSync() const { return mHasNativeFenceSync; }
    bool hasFenceSync() const { return mHasFenceSync; }
    bool hasWaitSync() const { return mHasWaitSync; }
    bool hasProtectedContent() const { return mHasProtectedContent; }
    bool hasContextPriority() const { return mHasContextPriority; }
    bool hasSurfacelessContext() const { return mHasSurfacelessContext; }
    bool hasProtectedTexture() const { return mHasProtectedTexture; }

    void initWithGLStrings(GLubyte const* vendor, GLubyte const* renderer, GLubyte const* version,
                           GLubyte const* extensions);
    char const* getVendor() const;
    char const* getRenderer() const;
    char const* getVersion() const;
    char const* getExtensions() const;

    void initWithEGLStrings(char const* eglVersion, char const* eglExtensions);
    char const* getEGLVersion() const;
    char const* getEGLExtensions() const;

protected:
    GLExtensions() = default;

private:
    friend class Singleton<GLExtensions>;

    bool mHasNoConfigContext = false;
    bool mHasNativeFenceSync = false;
    bool mHasFenceSync = false;
    bool mHasWaitSync = false;
    bool mHasProtectedContent = false;
    bool mHasContextPriority = false;
    bool mHasSurfacelessContext = false;
    bool mHasProtectedTexture = false;

    String8 mVendor;
    String8 mRenderer;
    String8 mVersion;
    String8 mExtensions;
    String8 mEGLVersion;
    String8 mEGLExtensions;

    GLExtensions(const GLExtensions&);
    GLExtensions& operator=(const GLExtensions&);
};

} // namespace gl
} // namespace renderengine
} // namespace android

#endif // ANDROID_SF_GLEXTENSION_H
