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

#ifndef ANDROID_EGL_DISPLAY_H
#define ANDROID_EGL_DISPLAY_H


#include <stdint.h>
#include <stddef.h>

#include <condition_variable>
#include <mutex>
#include <string>
#include <unordered_set>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <cutils/compiler.h>

#include "egldefs.h"
#include "../hooks.h"

// ----------------------------------------------------------------------------
namespace android {
// ----------------------------------------------------------------------------

class egl_object_t;
class egl_context_t;
struct egl_connection_t;

bool findExtension(const char* exts, const char* name, size_t nameLen = 0);
bool needsAndroidPEglMitigation();

// ----------------------------------------------------------------------------

class EGLAPI egl_display_t { // marked as EGLAPI for testing purposes
    static egl_display_t sDisplay[NUM_DISPLAYS];
    EGLDisplay getDisplay(EGLNativeDisplayType display);
    EGLDisplay getPlatformDisplay(EGLNativeDisplayType display, const EGLAttrib* attrib_list);
    void loseCurrentImpl(egl_context_t * cur_c);

public:
    enum {
        NOT_INITIALIZED = 0,
        INITIALIZED     = 1,
        TERMINATED      = 2
    };

    egl_display_t();
    ~egl_display_t();

    EGLBoolean initialize(EGLint *major, EGLint *minor);
    EGLBoolean terminate();

    // add object to this display's list
    void addObject(egl_object_t* object);
    // remove object from this display's list
    void removeObject(egl_object_t* object);
    // add reference to this object. returns true if this is a valid object.
    bool getObject(egl_object_t* object) const;

    static egl_display_t* get(EGLDisplay dpy);
    static EGLDisplay getFromNativeDisplay(EGLNativeDisplayType disp, const EGLAttrib* attrib_list);

    EGLBoolean makeCurrent(egl_context_t* c, egl_context_t* cur_c,
            EGLSurface draw, EGLSurface read, EGLContext ctx,
            EGLSurface impl_draw, EGLSurface impl_read, EGLContext impl_ctx);
    static void loseCurrent(egl_context_t * cur_c);

    inline bool isReady() const { return (refs > 0); }
    inline bool isValid() const { return magic == '_dpy'; }
    inline bool isAlive() const { return isValid(); }

    char const * getVendorString() const { return mVendorString.c_str(); }
    char const * getVersionString() const { return mVersionString.c_str(); }
    char const * getClientApiString() const { return mClientApiString.c_str(); }
    char const * getExtensionString() const { return mExtensionString.c_str(); }

    bool haveExtension(const char* name, size_t nameLen = 0) const;

    inline uint32_t getRefsCount() const { return refs; }

    struct strings_t {
        char const * vendor;
        char const * version;
        char const * clientApi;
        char const * extensions;
    };

    struct DisplayImpl {
        DisplayImpl() : dpy(EGL_NO_DISPLAY), state(NOT_INITIALIZED) { }
        EGLDisplay  dpy;
        EGLint      state;
        strings_t   queryString;
    };

private:
    uint32_t        magic;

public:
    DisplayImpl     disp;
    bool    finishOnSwap;       // property: debug.egl.finish
    bool    traceGpuCompletion; // property: debug.egl.traceGpuCompletion
    bool    hasColorSpaceSupport;

private:
    friend class egl_display_ptr;

            uint32_t                    refs;
            bool                        eglIsInitialized;
    mutable std::mutex                  lock;
    mutable std::mutex                  refLock;
    mutable std::condition_variable     refCond;
            std::unordered_set<egl_object_t*> objects;
            std::string mVendorString;
            std::string mVersionString;
            std::string mClientApiString;
            std::string mExtensionString;
};

// ----------------------------------------------------------------------------

// An egl_display_ptr is a kind of smart pointer for egl_display_t objects.
// It doesn't refcount the egl_display_t, but does ensure that the underlying
// EGL implementation is "awake" (not hibernating) and ready for use as long
// as the egl_display_ptr exists.
class egl_display_ptr {
public:
    explicit egl_display_ptr(egl_display_t* dpy): mDpy(dpy) {}

    // We only really need a C++11 move constructor, not a copy constructor.
    // A move constructor would save an enter()/leave() pair on every EGL API
    // call. But enabling -std=c++0x causes lots of errors elsewhere, so I
    // can't use a move constructor until those are cleaned up.
    //
    // egl_display_ptr(egl_display_ptr&& other) {
    //     mDpy = other.mDpy;
    //     other.mDpy = NULL;
    // }
    //
    egl_display_ptr(const egl_display_ptr& other): mDpy(other.mDpy) {}

    ~egl_display_ptr() {}

    const egl_display_t* operator->() const { return mDpy; }
          egl_display_t* operator->()       { return mDpy; }

    const egl_display_t* get() const { return mDpy; }
          egl_display_t* get()       { return mDpy; }

    operator bool() const { return mDpy != nullptr; }

private:
    egl_display_t* mDpy;

    // non-assignable
    egl_display_ptr& operator=(const egl_display_ptr&);
};

// ----------------------------------------------------------------------------

inline egl_display_ptr get_display(EGLDisplay dpy) {
    return egl_display_ptr(egl_display_t::get(dpy));
}

// Does not ensure EGL is unhibernated. Use with caution: calls into the
// underlying EGL implementation are not safe.
inline egl_display_t* get_display_nowake(EGLDisplay dpy) {
    return egl_display_t::get(dpy);
}

// ----------------------------------------------------------------------------

egl_display_ptr validate_display(EGLDisplay dpy);
egl_display_ptr validate_display_connection(EGLDisplay dpy,
        egl_connection_t*& cnx);
EGLBoolean validate_display_context(EGLDisplay dpy, EGLContext ctx);
EGLBoolean validate_display_surface(EGLDisplay dpy, EGLSurface surface);

// ----------------------------------------------------------------------------
}; // namespace android
// ----------------------------------------------------------------------------

#endif // ANDROID_EGL_DISPLAY_H
