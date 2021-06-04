/* 
 ** Copyright 2009, The Android Open Source Project
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

#ifndef ANDROID_EGL_LOADER_H
#define ANDROID_EGL_LOADER_H

#include <stdint.h>

#include <EGL/egl.h>

// ----------------------------------------------------------------------------
namespace android {
// ----------------------------------------------------------------------------

struct egl_connection_t;

class Loader {
    typedef __eglMustCastToProperFunctionPointerType (* getProcAddressType)(const char*);

    enum {
        EGL         = 0x01,
        GLESv1_CM   = 0x02,
        GLESv2      = 0x04,
        PLATFORM    = 0x08
    };
    struct driver_t {
        explicit driver_t(void* gles);
        ~driver_t();
        // returns -errno
        int set(void* hnd, int32_t api);
        void* dso[3];
    };

    getProcAddressType getProcAddress;

public:
    static Loader& getInstance();
    ~Loader();

    void* open(egl_connection_t* cnx);
    void close(egl_connection_t* cnx);

private:
    Loader();
    driver_t* attempt_to_load_angle(egl_connection_t* cnx);
    driver_t* attempt_to_load_updated_driver(egl_connection_t* cnx);
    driver_t* attempt_to_load_system_driver(egl_connection_t* cnx, const char* suffix, const bool exact);
    void unload_system_driver(egl_connection_t* cnx);
    void initialize_api(void* dso, egl_connection_t* cnx, uint32_t mask);

    static __attribute__((noinline))
    void init_api(void* dso,
            char const * const * api,
            char const * const * ref_api,
            __eglMustCastToProperFunctionPointerType* curr,
            getProcAddressType getProcAddress);
};

// ----------------------------------------------------------------------------
}; // namespace android
// ----------------------------------------------------------------------------

#endif /* ANDROID_EGL_LOADER_H */
