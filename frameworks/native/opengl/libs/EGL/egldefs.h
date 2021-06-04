/*
 ** Copyright 2011, The Android Open Source Project
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

#ifndef ANDROID_EGLDEFS_H
#define ANDROID_EGLDEFS_H

#include "../hooks.h"
#include "egl_platform_entries.h"

#include <log/log.h>

#define VERSION_MAJOR 1
#define VERSION_MINOR 4
#define EGL_MAKE_VERSION(major, minor, patch) (((major) << 22) | ((minor) << 12) | (patch))

// ----------------------------------------------------------------------------
namespace android {
// ----------------------------------------------------------------------------

//  EGLDisplay are global, not attached to a given thread
const unsigned int NUM_DISPLAYS = 1;

// ----------------------------------------------------------------------------

extern char const * const platform_names[];

// clang-format off
struct egl_connection_t {
    enum {
        GLESv1_INDEX = 0,
        GLESv2_INDEX = 1
    };

    inline egl_connection_t() : dso(nullptr),
                                libEgl(nullptr),
                                libGles1(nullptr),
                                libGles2(nullptr),
                                systemDriverUnloaded(false) {

        char const* const* entries = platform_names;
        EGLFuncPointer* curr = reinterpret_cast<EGLFuncPointer*>(&platform);
        while (*entries) {
            const char* name = *entries;
            EGLFuncPointer f = FindPlatformImplAddr(name);

            if (f == nullptr) {
                // If no entry found, update the lookup table: sPlatformImplMap
                ALOGE("No entry found in platform lookup table for %s", name);
            }

            *curr++ = f;
            entries++;
        }
    }

    void *              dso;
    gl_hooks_t *        hooks[2];
    EGLint              major;
    EGLint              minor;
    EGLint              driverVersion;
    egl_t               egl;

    // Functions implemented or redirected by platform libraries
    platform_impl_t     platform;

    void*               libEgl;
    void*               libGles1;
    void*               libGles2;

    bool                systemDriverUnloaded;
    bool                shouldUseAngle; // Should we attempt to load ANGLE
    bool                angleDecided;   // Have we tried to load ANGLE
    bool                useAngle;       // Was ANGLE successfully loaded
    EGLint              angleBackend;
    void*               vendorEGL;
};
// clang-format on

// ----------------------------------------------------------------------------

extern gl_hooks_t gHooks[2];
extern gl_hooks_t gHooksNoContext;
extern pthread_key_t gGLWrapperKey;
extern "C" void gl_unimplemented();
extern "C" void gl_noop();

extern char const * const gl_names[];
extern char const * const gl_names_1[];
extern char const * const egl_names[];

extern egl_connection_t gEGLImpl;

// ----------------------------------------------------------------------------
}; // namespace android
// ----------------------------------------------------------------------------

#endif /* ANDROID_EGLDEFS_H */
