/*
 ** Copyright 2007, The Android Open Source Project
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

#ifndef ANDROID_GLES_CM_HOOKS_H
#define ANDROID_GLES_CM_HOOKS_H

#include <ctype.h>
#include <string.h>
#include <errno.h>

#include <pthread.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>

// set to 1 for debugging
#define USE_SLOW_BINDING    0

#undef NELEM
#define NELEM(x)            (sizeof(x)/sizeof(*(x)))

// maximum number of GL extensions that can be used simultaneously in
// a given process. this limitation exists because we need to have
// a static function for each extension and currently these static functions
// are generated at compile time.
#define MAX_NUMBER_OF_GL_EXTENSIONS 256


#include <bionic_tls.h>  /* special private C library header */

// ----------------------------------------------------------------------------
namespace android {
// ----------------------------------------------------------------------------

// GL / EGL hooks

#undef GL_ENTRY
#undef EGL_ENTRY
#define GL_ENTRY(_r, _api, ...) _r (*(_api))(__VA_ARGS__);
#define EGL_ENTRY(_r, _api, ...) _r (*(_api))(__VA_ARGS__);

struct platform_impl_t {
    #include "platform_entries.in"
};

struct egl_t {
    #include "EGL/egl_entries.in"
};

struct gl_hooks_t {
    struct gl_t {
        #include "entries.in"
    } gl;
    struct gl_ext_t {
        __eglMustCastToProperFunctionPointerType extensions[MAX_NUMBER_OF_GL_EXTENSIONS];
    } ext;
};
#undef GL_ENTRY
#undef EGL_ENTRY

EGLAPI void setGlThreadSpecific(gl_hooks_t const *value);

// We have a dedicated TLS slot in bionic
inline gl_hooks_t const * volatile * get_tls_hooks() {
    volatile void *tls_base = __get_tls();
    gl_hooks_t const * volatile * tls_hooks =
            reinterpret_cast<gl_hooks_t const * volatile *>(tls_base);
    return tls_hooks;
}

inline EGLAPI gl_hooks_t const* getGlThreadSpecific() {
    gl_hooks_t const * volatile * tls_hooks = get_tls_hooks();
    gl_hooks_t const* hooks = tls_hooks[TLS_SLOT_OPENGL_API];
    return hooks;
}

// ----------------------------------------------------------------------------
}; // namespace android
// ----------------------------------------------------------------------------

#endif /* ANDROID_GLES_CM_HOOKS_H */
