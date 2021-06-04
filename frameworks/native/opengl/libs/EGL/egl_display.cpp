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

#define __STDC_LIMIT_MACROS 1
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "egl_display.h"

#include "../egl_impl.h"

#include <EGL/eglext_angle.h>
#include <private/EGL/display.h>

#include <cutils/properties.h>
#include "Loader.h"
#include "egl_angle_platform.h"
#include "egl_cache.h"
#include "egl_object.h"
#include "egl_tls.h"

#include <android/dlext.h>
#include <dlfcn.h>
#include <graphicsenv/GraphicsEnv.h>

#include <android/hardware/configstore/1.0/ISurfaceFlingerConfigs.h>
#include <configstore/Utils.h>

using namespace android::hardware::configstore;
using namespace android::hardware::configstore::V1_0;

// ----------------------------------------------------------------------------
namespace android {
// ----------------------------------------------------------------------------

static char const * const sVendorString     = "Android";
static char const* const sVersionString14 = "1.4 Android META-EGL";
static char const* const sVersionString15 = "1.5 Android META-EGL";
static char const * const sClientApiString  = "OpenGL_ES";

extern char const * const gBuiltinExtensionString;
extern char const * const gExtensionString;

extern void setGLHooksThreadSpecific(gl_hooks_t const *value);

// ----------------------------------------------------------------------------

bool findExtension(const char* exts, const char* name, size_t nameLen) {
    if (exts) {
        if (!nameLen) {
            nameLen = strlen(name);
        }
        for (const char* match = strstr(exts, name); match; match = strstr(match + nameLen, name)) {
            if (match[nameLen] == '\0' || match[nameLen] == ' ') {
                return true;
            }
        }
    }
    return false;
}

bool needsAndroidPEglMitigation() {
    static const int32_t vndk_version = property_get_int32("ro.vndk.version", -1);
    return vndk_version <= 28;
}

int egl_get_init_count(EGLDisplay dpy) {
    egl_display_t* eglDisplay = egl_display_t::get(dpy);
    return eglDisplay ? eglDisplay->getRefsCount() : 0;
}

egl_display_t egl_display_t::sDisplay[NUM_DISPLAYS];

egl_display_t::egl_display_t() :
    magic('_dpy'), finishOnSwap(false), traceGpuCompletion(false), refs(0), eglIsInitialized(false) {
}

egl_display_t::~egl_display_t() {
    magic = 0;
    egl_cache_t::get()->terminate();
}

egl_display_t* egl_display_t::get(EGLDisplay dpy) {
    if (uintptr_t(dpy) == 0) {
        return nullptr;
    }

    uintptr_t index = uintptr_t(dpy)-1U;
    if (index >= NUM_DISPLAYS || !sDisplay[index].isValid()) {
        return nullptr;
    }
    return &sDisplay[index];
}

void egl_display_t::addObject(egl_object_t* object) {
    std::lock_guard<std::mutex> _l(lock);
    objects.insert(object);
}

void egl_display_t::removeObject(egl_object_t* object) {
    std::lock_guard<std::mutex> _l(lock);
    objects.erase(object);
}

bool egl_display_t::getObject(egl_object_t* object) const {
    std::lock_guard<std::mutex> _l(lock);
    if (objects.find(object) != objects.end()) {
        if (object->getDisplay() == this) {
            object->incRef();
            return true;
        }
    }
    return false;
}

EGLDisplay egl_display_t::getFromNativeDisplay(EGLNativeDisplayType disp,
                                               const EGLAttrib* attrib_list) {
    if (uintptr_t(disp) >= NUM_DISPLAYS)
        return nullptr;

    return sDisplay[uintptr_t(disp)].getPlatformDisplay(disp, attrib_list);
}

static bool addAnglePlatformAttributes(egl_connection_t* const cnx,
                                       std::vector<EGLAttrib>& attrs) {
    intptr_t vendorEGL = (intptr_t)cnx->vendorEGL;

    attrs.reserve(4 * 2);

    attrs.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
    attrs.push_back(cnx->angleBackend);

    switch (cnx->angleBackend) {
        case EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE:
            ALOGV("%s: Requesting Vulkan ANGLE back-end", __FUNCTION__);
            char prop[PROPERTY_VALUE_MAX];
            property_get("debug.angle.validation", prop, "0");
            attrs.push_back(EGL_PLATFORM_ANGLE_DEBUG_LAYERS_ENABLED_ANGLE);
            attrs.push_back(atoi(prop));
            break;
        case EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE:
            ALOGV("%s: Requesting Default ANGLE back-end", __FUNCTION__);
            break;
        case EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE:
            ALOGV("%s: Requesting OpenGL ES ANGLE back-end", __FUNCTION__);
            // NOTE: This is only valid if the backend is OpenGL
            attrs.push_back(EGL_PLATFORM_ANGLE_EGL_HANDLE_ANGLE);
            attrs.push_back(vendorEGL);
            break;
        default:
            ALOGV("%s: Requesting Unknown (%d) ANGLE back-end", __FUNCTION__, cnx->angleBackend);
            break;
    }
    attrs.push_back(EGL_PLATFORM_ANGLE_CONTEXT_VIRTUALIZATION_ANGLE);
    attrs.push_back(EGL_FALSE);

    return true;
}

static EGLDisplay getPlatformDisplayAngle(EGLNativeDisplayType display, egl_connection_t* const cnx,
                                          const EGLAttrib* attrib_list, EGLint* error) {
    EGLDisplay dpy = EGL_NO_DISPLAY;
    *error = EGL_NONE;

    if (cnx->egl.eglGetPlatformDisplay) {
        std::vector<EGLAttrib> attrs;
        if (attrib_list) {
            for (const EGLAttrib* attr = attrib_list; *attr != EGL_NONE; attr += 2) {
                attrs.push_back(attr[0]);
                attrs.push_back(attr[1]);
            }
        }

        if (!addAnglePlatformAttributes(cnx, attrs)) {
            ALOGE("eglGetDisplay(%p) failed: Mismatch display request", display);
            *error = EGL_BAD_PARAMETER;
            return EGL_NO_DISPLAY;
        }
        attrs.push_back(EGL_NONE);

        dpy = cnx->egl.eglGetPlatformDisplay(EGL_PLATFORM_ANGLE_ANGLE,
                                             reinterpret_cast<void*>(EGL_DEFAULT_DISPLAY),
                                             attrs.data());
        if (dpy == EGL_NO_DISPLAY) {
            ALOGE("eglGetPlatformDisplay failed!");
        } else {
            if (!angle::initializeAnglePlatform(dpy)) {
                ALOGE("initializeAnglePlatform failed!");
            }
        }
    } else {
        ALOGE("eglGetDisplay(%p) failed: Unable to look up eglGetPlatformDisplay from ANGLE",
              display);
    }

    return dpy;
}

EGLDisplay egl_display_t::getPlatformDisplay(EGLNativeDisplayType display,
                                             const EGLAttrib* attrib_list) {
    std::lock_guard<std::mutex> _l(lock);
    ATRACE_CALL();

    // get our driver loader
    Loader& loader(Loader::getInstance());

    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso) {
        EGLDisplay dpy = EGL_NO_DISPLAY;

        if (cnx->useAngle) {
            EGLint error;
            dpy = getPlatformDisplayAngle(display, cnx, attrib_list, &error);
            if (error != EGL_NONE) {
                return setError(error, dpy);
            }
        }
        if (dpy == EGL_NO_DISPLAY) {
            // NOTE: eglGetPlatformDisplay with a empty attribute list
            // behaves the same as eglGetDisplay
            if (cnx->egl.eglGetPlatformDisplay) {
                dpy = cnx->egl.eglGetPlatformDisplay(EGL_PLATFORM_ANDROID_KHR, display,
                                                     attrib_list);
            }

            // It is possible that eglGetPlatformDisplay does not have a
            // working implementation for Android platform; in that case,
            // one last fallback to eglGetDisplay
            if(dpy == EGL_NO_DISPLAY) {
                if (attrib_list) {
                    ALOGW("getPlatformDisplay: unexpected attribute list, attributes ignored");
                }
                dpy = cnx->egl.eglGetDisplay(display);
            }
        }

        disp.dpy = dpy;
        if (dpy == EGL_NO_DISPLAY) {
            loader.close(cnx);
        }
    }

    return EGLDisplay(uintptr_t(display) + 1U);
}

EGLBoolean egl_display_t::initialize(EGLint *major, EGLint *minor) {

    { // scope for refLock
        std::unique_lock<std::mutex> _l(refLock);
        refs++;
        if (refs > 1) {
            // We don't know what to report until we know what the
            // driver supports. Make sure we are initialized before
            // returning the version info.
            while(!eglIsInitialized) {
                refCond.wait(_l);
            }
            egl_connection_t* const cnx = &gEGLImpl;

            // TODO: If device doesn't provide 1.4 or 1.5 then we'll be
            // changing the behavior from the past where we always advertise
            // version 1.4. May need to check that revision is valid
            // before using cnx->major & cnx->minor
            if (major != nullptr) *major = cnx->major;
            if (minor != nullptr) *minor = cnx->minor;
            return EGL_TRUE;
        }
        while(eglIsInitialized) {
            refCond.wait(_l);
        }
    }

    { // scope for lock
        std::lock_guard<std::mutex> _l(lock);

        setGLHooksThreadSpecific(&gHooksNoContext);

        // initialize each EGL and
        // build our own extension string first, based on the extension we know
        // and the extension supported by our client implementation

        egl_connection_t* const cnx = &gEGLImpl;
        cnx->major = -1;
        cnx->minor = -1;
        if (cnx->dso) {
            EGLDisplay idpy = disp.dpy;
            if (cnx->egl.eglInitialize(idpy, &cnx->major, &cnx->minor)) {
                //ALOGD("initialized dpy=%p, ver=%d.%d, cnx=%p",
                //        idpy, cnx->major, cnx->minor, cnx);

                // display is now initialized
                disp.state = egl_display_t::INITIALIZED;

                // get the query-strings for this display for each implementation
                disp.queryString.vendor = cnx->egl.eglQueryString(idpy,
                        EGL_VENDOR);
                disp.queryString.version = cnx->egl.eglQueryString(idpy,
                        EGL_VERSION);
                disp.queryString.extensions = cnx->egl.eglQueryString(idpy,
                        EGL_EXTENSIONS);
                disp.queryString.clientApi = cnx->egl.eglQueryString(idpy,
                        EGL_CLIENT_APIS);

            } else {
                ALOGW("eglInitialize(%p) failed (%s)", idpy,
                        egl_tls_t::egl_strerror(cnx->egl.eglGetError()));
            }
        }

        if (cnx->minor == 5) {
            // full list in egl_entries.in
            if (!cnx->egl.eglCreateImage ||
                !cnx->egl.eglDestroyImage ||
                !cnx->egl.eglGetPlatformDisplay ||
                !cnx->egl.eglCreatePlatformWindowSurface ||
                !cnx->egl.eglCreatePlatformPixmapSurface ||
                !cnx->egl.eglCreateSync ||
                !cnx->egl.eglDestroySync ||
                !cnx->egl.eglClientWaitSync ||
                !cnx->egl.eglGetSyncAttrib ||
                !cnx->egl.eglWaitSync) {
                ALOGE("Driver indicates EGL 1.5 support, but does not have "
                      "a critical API");
                cnx->minor = 4;
            }
        }

        // the query strings are per-display
        mVendorString = sVendorString;
        mVersionString.clear();
        cnx->driverVersion = EGL_MAKE_VERSION(1, 4, 0);
        mVersionString = sVersionString14;
        if ((cnx->major == 1) && (cnx->minor == 5)) {
            mVersionString = sVersionString15;
            cnx->driverVersion = EGL_MAKE_VERSION(1, 5, 0);
        }
        if (mVersionString.empty()) {
            ALOGW("Unexpected driver version: %d.%d, want 1.4 or 1.5", cnx->major, cnx->minor);
            mVersionString = sVersionString14;
        }
        mClientApiString = sClientApiString;

        mExtensionString = gBuiltinExtensionString;

        hasColorSpaceSupport = findExtension(disp.queryString.extensions, "EGL_KHR_gl_colorspace");

        // Note: CDD requires that devices supporting wide color and/or HDR color also support
        // the EGL_KHR_gl_colorspace extension.
        bool wideColorBoardConfig =
                getBool<ISurfaceFlingerConfigs, &ISurfaceFlingerConfigs::hasWideColorDisplay>(
                        false);

        // Add wide-color extensions if device can support wide-color
        if (wideColorBoardConfig && hasColorSpaceSupport) {
            mExtensionString.append(
                    "EGL_EXT_gl_colorspace_scrgb EGL_EXT_gl_colorspace_scrgb_linear "
                    "EGL_EXT_gl_colorspace_display_p3_linear EGL_EXT_gl_colorspace_display_p3 "
                    "EGL_EXT_gl_colorspace_display_p3_passthrough ");
        }

        bool hasHdrBoardConfig =
                getBool<ISurfaceFlingerConfigs, &ISurfaceFlingerConfigs::hasHDRDisplay>(false);

        if (hasHdrBoardConfig && hasColorSpaceSupport) {
            // hasHDRBoardConfig indicates the system is capable of supporting HDR content.
            // Typically that means there is an HDR capable display attached, but could be
            // support for attaching an HDR display. In either case, advertise support for
            // HDR color spaces.
            mExtensionString.append(
                    "EGL_EXT_gl_colorspace_bt2020_linear EGL_EXT_gl_colorspace_bt2020_pq ");
        }

        char const* start = gExtensionString;
        do {
            // length of the extension name
            size_t len = strcspn(start, " ");
            if (len) {
                // NOTE: we could avoid the copy if we had strnstr.
                const std::string ext(start, len);
                // Mitigation for Android P vendor partitions: Adreno 530 driver shipped on
                // some Android P vendor partitions this extension under the draft KHR name,
                // but during Khronos review it was decided to demote it to EXT.
                if (needsAndroidPEglMitigation() && ext == "EGL_EXT_image_gl_colorspace" &&
                    findExtension(disp.queryString.extensions, "EGL_KHR_image_gl_colorspace")) {
                    mExtensionString.append("EGL_EXT_image_gl_colorspace ");
                }
                if (findExtension(disp.queryString.extensions, ext.c_str(), len)) {
                    mExtensionString.append(ext + " ");
                }
                // advance to the next extension name, skipping the space.
                start += len;
                start += (*start == ' ') ? 1 : 0;
            }
        } while (*start != '\0');

        egl_cache_t::get()->initialize(this);

        char value[PROPERTY_VALUE_MAX];
        property_get("debug.egl.finish", value, "0");
        if (atoi(value)) {
            finishOnSwap = true;
        }

        property_get("debug.egl.traceGpuCompletion", value, "0");
        if (atoi(value)) {
            traceGpuCompletion = true;
        }

        // TODO: If device doesn't provide 1.4 or 1.5 then we'll be
        // changing the behavior from the past where we always advertise
        // version 1.4. May need to check that revision is valid
        // before using cnx->major & cnx->minor
        if (major != nullptr) *major = cnx->major;
        if (minor != nullptr) *minor = cnx->minor;
    }

    { // scope for refLock
        std::unique_lock<std::mutex> _l(refLock);
        eglIsInitialized = true;
        refCond.notify_all();
    }

    return EGL_TRUE;
}

EGLBoolean egl_display_t::terminate() {

    { // scope for refLock
        std::unique_lock<std::mutex> _rl(refLock);
        if (refs == 0) {
            /*
             * From the EGL spec (3.2):
             * "Termination of a display that has already been terminated,
             *  (...), is allowed, but the only effect of such a call is
             *  to return EGL_TRUE (...)
             */
            return EGL_TRUE;
        }

        // this is specific to Android, display termination is ref-counted.
        refs--;
        if (refs > 0) {
            return EGL_TRUE;
        }
    }

    EGLBoolean res = EGL_FALSE;

    { // scope for lock
        std::lock_guard<std::mutex> _l(lock);

        egl_connection_t* const cnx = &gEGLImpl;
        if (cnx->dso && disp.state == egl_display_t::INITIALIZED) {
            // If we're using ANGLE reset any custom DisplayPlatform
            if (cnx->useAngle) {
                angle::resetAnglePlatform(disp.dpy);
            }
            if (cnx->egl.eglTerminate(disp.dpy) == EGL_FALSE) {
                ALOGW("eglTerminate(%p) failed (%s)", disp.dpy,
                        egl_tls_t::egl_strerror(cnx->egl.eglGetError()));
            }
            // REVISIT: it's unclear what to do if eglTerminate() fails
            disp.state = egl_display_t::TERMINATED;
            res = EGL_TRUE;
        }

        // Reset the extension string since it will be regenerated if we get
        // reinitialized.
        mExtensionString.clear();

        // Mark all objects remaining in the list as terminated, unless
        // there are no reference to them, it which case, we're free to
        // delete them.
        size_t count = objects.size();
        ALOGW_IF(count, "eglTerminate() called w/ %zu objects remaining", count);
        for (auto o : objects) {
            o->destroy();
        }

        // this marks all object handles are "terminated"
        objects.clear();
    }

    { // scope for refLock
        std::unique_lock<std::mutex> _rl(refLock);
        eglIsInitialized = false;
        refCond.notify_all();
    }

    return res;
}

void egl_display_t::loseCurrent(egl_context_t * cur_c)
{
    if (cur_c) {
        egl_display_t* display = cur_c->getDisplay();
        if (display) {
            display->loseCurrentImpl(cur_c);
        }
    }
}

void egl_display_t::loseCurrentImpl(egl_context_t * cur_c)
{
    // by construction, these are either 0 or valid (possibly terminated)
    // it should be impossible for these to be invalid
    ContextRef _cur_c(cur_c);
    SurfaceRef _cur_r(cur_c ? get_surface(cur_c->read) : nullptr);
    SurfaceRef _cur_d(cur_c ? get_surface(cur_c->draw) : nullptr);

    { // scope for the lock
        std::lock_guard<std::mutex> _l(lock);
        cur_c->onLooseCurrent();

    }

    // This cannot be called with the lock held because it might end-up
    // calling back into EGL (in particular when a surface is destroyed
    // it calls ANativeWindow::disconnect
    _cur_c.release();
    _cur_r.release();
    _cur_d.release();
}

EGLBoolean egl_display_t::makeCurrent(egl_context_t* c, egl_context_t* cur_c,
        EGLSurface draw, EGLSurface read, EGLContext /*ctx*/,
        EGLSurface impl_draw, EGLSurface impl_read, EGLContext impl_ctx)
{
    EGLBoolean result;

    // by construction, these are either 0 or valid (possibly terminated)
    // it should be impossible for these to be invalid
    ContextRef _cur_c(cur_c);
    SurfaceRef _cur_r(cur_c ? get_surface(cur_c->read) : nullptr);
    SurfaceRef _cur_d(cur_c ? get_surface(cur_c->draw) : nullptr);

    { // scope for the lock
        std::lock_guard<std::mutex> _l(lock);
        if (c) {
            result = c->cnx->egl.eglMakeCurrent(
                    disp.dpy, impl_draw, impl_read, impl_ctx);
            if (result == EGL_TRUE) {
                c->onMakeCurrent(draw, read);
            }
        } else {
            result = cur_c->cnx->egl.eglMakeCurrent(
                    disp.dpy, impl_draw, impl_read, impl_ctx);
            if (result == EGL_TRUE) {
                cur_c->onLooseCurrent();
            }
        }
    }

    if (result == EGL_TRUE) {
        // This cannot be called with the lock held because it might end-up
        // calling back into EGL (in particular when a surface is destroyed
        // it calls ANativeWindow::disconnect
        _cur_c.release();
        _cur_r.release();
        _cur_d.release();
    }

    return result;
}

bool egl_display_t::haveExtension(const char* name, size_t nameLen) const {
    if (!nameLen) {
        nameLen = strlen(name);
    }
    return findExtension(mExtensionString.c_str(), name, nameLen);
}

// ----------------------------------------------------------------------------
}; // namespace android
// ----------------------------------------------------------------------------
