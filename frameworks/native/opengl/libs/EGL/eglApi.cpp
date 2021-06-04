/*
 ** Copyright 2018, The Android Open Source Project
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

#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "../egl_impl.h"

#include "egl_layers.h"
#include "egl_platform_entries.h"
#include "egl_tls.h"
#include "egl_trace.h"

using namespace android;

namespace android {

extern EGLBoolean egl_init_drivers();

} // namespace android

static inline void clearError() {
    egl_tls_t::clearError();
}

EGLDisplay eglGetDisplay(EGLNativeDisplayType display) {
    ATRACE_CALL();
    clearError();

    if (egl_init_drivers() == EGL_FALSE) {
        return setError(EGL_BAD_PARAMETER, EGL_NO_DISPLAY);
    }

    // Call down the chain, which usually points directly to the impl
    // but may also be routed through layers
    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglGetDisplay(display);
}

EGLDisplay eglGetPlatformDisplay(EGLenum platform, EGLNativeDisplayType display,
                                 const EGLAttrib* attrib_list) {
    ATRACE_CALL();
    clearError();

    if (egl_init_drivers() == EGL_FALSE) {
        return setError(EGL_BAD_PARAMETER, EGL_NO_DISPLAY);
    }

    // Call down the chain, which usually points directly to the impl
    // but may also be routed through layers
    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglGetPlatformDisplay(platform, display, attrib_list);
}

EGLBoolean eglInitialize(EGLDisplay dpy, EGLint* major, EGLint* minor) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglInitialize(dpy, major, minor);
}

EGLBoolean eglTerminate(EGLDisplay dpy) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglTerminate(dpy);
}

EGLBoolean eglGetConfigs(EGLDisplay dpy, EGLConfig* configs, EGLint config_size,
                         EGLint* num_config) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglGetConfigs(dpy, configs, config_size, num_config);
}

EGLBoolean eglChooseConfig(EGLDisplay dpy, const EGLint* attrib_list, EGLConfig* configs,
                           EGLint config_size, EGLint* num_config) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglChooseConfig(dpy, attrib_list, configs, config_size, num_config);
}

EGLBoolean eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint* value) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglGetConfigAttrib(dpy, config, attribute, value);
}

EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, NativeWindowType window,
                                  const EGLint* attrib_list) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglCreateWindowSurface(dpy, config, window, attrib_list);
}

EGLSurface eglCreatePlatformWindowSurface(EGLDisplay dpy, EGLConfig config, void* native_window,
                                          const EGLAttrib* attrib_list) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglCreatePlatformWindowSurface(dpy, config, native_window, attrib_list);
}

EGLSurface eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config, NativePixmapType pixmap,
                                  const EGLint* attrib_list) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglCreatePixmapSurface(dpy, config, pixmap, attrib_list);
}

EGLSurface eglCreatePlatformPixmapSurface(EGLDisplay dpy, EGLConfig config, void* native_pixmap,
                                          const EGLAttrib* attrib_list) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglCreatePlatformPixmapSurface(dpy, config, native_pixmap, attrib_list);
}

EGLSurface eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint* attrib_list) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglCreatePbufferSurface(dpy, config, attrib_list);
}

EGLBoolean eglDestroySurface(EGLDisplay dpy, EGLSurface surface) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglDestroySurface(dpy, surface);
}

EGLBoolean eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint* value) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglQuerySurface(dpy, surface, attribute, value);
}

void EGLAPI eglBeginFrame(EGLDisplay dpy, EGLSurface surface) {
    ATRACE_CALL();
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    cnx->platform.eglBeginFrame(dpy, surface);
}

EGLContext eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_list,
                            const EGLint* attrib_list) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglCreateContext(dpy, config, share_list, attrib_list);
}

EGLBoolean eglDestroyContext(EGLDisplay dpy, EGLContext ctx) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglDestroyContext(dpy, ctx);
}

EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglMakeCurrent(dpy, draw, read, ctx);
}

EGLBoolean eglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint* value) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglQueryContext(dpy, ctx, attribute, value);
}

EGLContext eglGetCurrentContext(void) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglGetCurrentContext();
}

EGLSurface eglGetCurrentSurface(EGLint readdraw) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglGetCurrentSurface(readdraw);
}

EGLDisplay eglGetCurrentDisplay(void) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglGetCurrentDisplay();
}

EGLBoolean eglWaitGL(void) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglWaitGL();
}

EGLBoolean eglWaitNative(EGLint engine) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglWaitNative(engine);
}

EGLint eglGetError(void) {
    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglGetError();
}

__eglMustCastToProperFunctionPointerType eglGetProcAddress(const char* procname) {
    // eglGetProcAddress() could be the very first function called
    // in which case we must make sure we've initialized ourselves, this
    // happens the first time egl_get_display() is called.

    clearError();

    if (egl_init_drivers() == EGL_FALSE) {
        setError(EGL_BAD_PARAMETER, NULL);
        return nullptr;
    }

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglGetProcAddress(procname);
}

EGLBoolean eglSwapBuffersWithDamageKHR(EGLDisplay dpy, EGLSurface draw, EGLint* rects,
                                       EGLint n_rects) {
    ATRACE_CALL();
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglSwapBuffersWithDamageKHR(dpy, draw, rects, n_rects);
}

EGLBoolean eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    ATRACE_CALL();
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglSwapBuffers(dpy, surface);
}

EGLBoolean eglCopyBuffers(EGLDisplay dpy, EGLSurface surface, NativePixmapType target) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglCopyBuffers(dpy, surface, target);
}

const char* eglQueryString(EGLDisplay dpy, EGLint name) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglQueryString(dpy, name);
}

extern "C" EGLAPI const char* eglQueryStringImplementationANDROID(EGLDisplay dpy, EGLint name) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglQueryStringImplementationANDROID(dpy, name);
}

EGLBoolean eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglSurfaceAttrib(dpy, surface, attribute, value);
}

EGLBoolean eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglBindTexImage(dpy, surface, buffer);
}

EGLBoolean eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglReleaseTexImage(dpy, surface, buffer);
}

EGLBoolean eglSwapInterval(EGLDisplay dpy, EGLint interval) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglSwapInterval(dpy, interval);
}

EGLBoolean eglWaitClient(void) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglWaitClient();
}

EGLBoolean eglBindAPI(EGLenum api) {
    clearError();

    if (egl_init_drivers() == EGL_FALSE) {
        return setError(EGL_BAD_PARAMETER, (EGLBoolean)EGL_FALSE);
    }

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglBindAPI(api);
}

EGLenum eglQueryAPI(void) {
    clearError();

    if (egl_init_drivers() == EGL_FALSE) {
        return setError(EGL_BAD_PARAMETER, (EGLBoolean)EGL_FALSE);
    }

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglQueryAPI();
}

EGLBoolean eglReleaseThread(void) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglReleaseThread();
}

EGLSurface eglCreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer,
                                            EGLConfig config, const EGLint* attrib_list) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglCreatePbufferFromClientBuffer(dpy, buftype, buffer, config,
                                                          attrib_list);
}

EGLBoolean eglLockSurfaceKHR(EGLDisplay dpy, EGLSurface surface, const EGLint* attrib_list) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglLockSurfaceKHR(dpy, surface, attrib_list);
}

EGLBoolean eglUnlockSurfaceKHR(EGLDisplay dpy, EGLSurface surface) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglUnlockSurfaceKHR(dpy, surface);
}

EGLImageKHR eglCreateImageKHR(EGLDisplay dpy, EGLContext ctx, EGLenum target,
                              EGLClientBuffer buffer, const EGLint* attrib_list) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglCreateImageKHR(dpy, ctx, target, buffer, attrib_list);
}

EGLImage eglCreateImage(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer,
                        const EGLAttrib* attrib_list) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglCreateImage(dpy, ctx, target, buffer, attrib_list);
}

EGLBoolean eglDestroyImageKHR(EGLDisplay dpy, EGLImageKHR img) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglDestroyImageKHR(dpy, img);
}

EGLBoolean eglDestroyImage(EGLDisplay dpy, EGLImageKHR img) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglDestroyImage(dpy, img);
}

// ----------------------------------------------------------------------------
// EGL_EGLEXT_VERSION 5
// ----------------------------------------------------------------------------

EGLSyncKHR eglCreateSync(EGLDisplay dpy, EGLenum type, const EGLAttrib* attrib_list) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglCreateSync(dpy, type, attrib_list);
}

EGLSyncKHR eglCreateSyncKHR(EGLDisplay dpy, EGLenum type, const EGLint* attrib_list) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglCreateSyncKHR(dpy, type, attrib_list);
}

EGLBoolean eglDestroySync(EGLDisplay dpy, EGLSyncKHR sync) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglDestroySync(dpy, sync);
}

EGLBoolean eglDestroySyncKHR(EGLDisplay dpy, EGLSyncKHR sync) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglDestroySyncKHR(dpy, sync);
}

EGLBoolean eglSignalSyncKHR(EGLDisplay dpy, EGLSyncKHR sync, EGLenum mode) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglSignalSyncKHR(dpy, sync, mode);
}

EGLint eglClientWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags, EGLTimeKHR timeout) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglClientWaitSyncKHR(dpy, sync, flags, timeout);
}

EGLint eglClientWaitSyncKHR(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglClientWaitSyncKHR(dpy, sync, flags, timeout);
}

EGLBoolean eglGetSyncAttrib(EGLDisplay dpy, EGLSync sync, EGLint attribute, EGLAttrib* value) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglGetSyncAttrib(dpy, sync, attribute, value);
}

EGLBoolean eglGetSyncAttribKHR(EGLDisplay dpy, EGLSyncKHR sync, EGLint attribute, EGLint* value) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglGetSyncAttribKHR(dpy, sync, attribute, value);
}

EGLStreamKHR eglCreateStreamKHR(EGLDisplay dpy, const EGLint* attrib_list) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglCreateStreamKHR(dpy, attrib_list);
}

EGLBoolean eglDestroyStreamKHR(EGLDisplay dpy, EGLStreamKHR stream) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglDestroyStreamKHR(dpy, stream);
}

EGLBoolean eglStreamAttribKHR(EGLDisplay dpy, EGLStreamKHR stream, EGLenum attribute,
                              EGLint value) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglStreamAttribKHR(dpy, stream, attribute, value);
}

EGLBoolean eglQueryStreamKHR(EGLDisplay dpy, EGLStreamKHR stream, EGLenum attribute,
                             EGLint* value) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglQueryStreamKHR(dpy, stream, attribute, value);
}

EGLBoolean eglQueryStreamu64KHR(EGLDisplay dpy, EGLStreamKHR stream, EGLenum attribute,
                                EGLuint64KHR* value) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglQueryStreamu64KHR(dpy, stream, attribute, value);
}

EGLBoolean eglQueryStreamTimeKHR(EGLDisplay dpy, EGLStreamKHR stream, EGLenum attribute,
                                 EGLTimeKHR* value) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglQueryStreamTimeKHR(dpy, stream, attribute, value);
}

EGLSurface eglCreateStreamProducerSurfaceKHR(EGLDisplay dpy, EGLConfig config, EGLStreamKHR stream,
                                             const EGLint* attrib_list) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglCreateStreamProducerSurfaceKHR(dpy, config, stream, attrib_list);
}

EGLBoolean eglStreamConsumerGLTextureExternalKHR(EGLDisplay dpy, EGLStreamKHR stream) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglStreamConsumerGLTextureExternalKHR(dpy, stream);
}

EGLBoolean eglStreamConsumerAcquireKHR(EGLDisplay dpy, EGLStreamKHR stream) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglStreamConsumerAcquireKHR(dpy, stream);
}

EGLBoolean eglStreamConsumerReleaseKHR(EGLDisplay dpy, EGLStreamKHR stream) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglStreamConsumerReleaseKHR(dpy, stream);
}

EGLNativeFileDescriptorKHR eglGetStreamFileDescriptorKHR(EGLDisplay dpy, EGLStreamKHR stream) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglGetStreamFileDescriptorKHR(dpy, stream);
}

EGLStreamKHR eglCreateStreamFromFileDescriptorKHR(EGLDisplay dpy,
                                                  EGLNativeFileDescriptorKHR file_descriptor) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglCreateStreamFromFileDescriptorKHR(dpy, file_descriptor);
}

EGLint eglWaitSyncKHR(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags) {
    clearError();
    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglWaitSyncKHR(dpy, sync, flags);
}

EGLBoolean eglWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags) {
    clearError();
    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglWaitSync(dpy, sync, flags);
}

EGLint eglDupNativeFenceFDANDROID(EGLDisplay dpy, EGLSyncKHR sync) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglDupNativeFenceFDANDROID(dpy, sync);
}

EGLBoolean eglPresentationTimeANDROID(EGLDisplay dpy, EGLSurface surface, EGLnsecsANDROID time) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglPresentationTimeANDROID(dpy, surface, time);
}

EGLClientBuffer eglGetNativeClientBufferANDROID(const AHardwareBuffer* buffer) {
    clearError();
    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglGetNativeClientBufferANDROID(buffer);
}

EGLuint64NV eglGetSystemTimeFrequencyNV() {
    clearError();

    if (egl_init_drivers() == EGL_FALSE) {
        return setError(EGL_BAD_PARAMETER, (EGLuint64NV)EGL_FALSE);
    }

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglGetSystemTimeFrequencyNV();
}

EGLuint64NV eglGetSystemTimeNV() {
    clearError();

    if (egl_init_drivers() == EGL_FALSE) {
        return setError(EGL_BAD_PARAMETER, (EGLuint64NV)EGL_FALSE);
    }

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglGetSystemTimeNV();
}

EGLBoolean eglSetDamageRegionKHR(EGLDisplay dpy, EGLSurface surface, EGLint* rects,
                                 EGLint n_rects) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglSetDamageRegionKHR(dpy, surface, rects, n_rects);
}

EGLBoolean eglGetNextFrameIdANDROID(EGLDisplay dpy, EGLSurface surface, EGLuint64KHR* frameId) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglGetNextFrameIdANDROID(dpy, surface, frameId);
}

EGLBoolean eglGetCompositorTimingANDROID(EGLDisplay dpy, EGLSurface surface, EGLint numTimestamps,
                                         const EGLint* names, EGLnsecsANDROID* values) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglGetCompositorTimingANDROID(dpy, surface, numTimestamps, names, values);
}

EGLBoolean eglGetCompositorTimingSupportedANDROID(EGLDisplay dpy, EGLSurface surface, EGLint name) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglGetCompositorTimingSupportedANDROID(dpy, surface, name);
}

EGLBoolean eglGetFrameTimestampsANDROID(EGLDisplay dpy, EGLSurface surface, EGLuint64KHR frameId,
                                        EGLint numTimestamps, const EGLint* timestamps,
                                        EGLnsecsANDROID* values) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglGetFrameTimestampsANDROID(dpy, surface, frameId, numTimestamps,
                                                      timestamps, values);
}

EGLBoolean eglGetFrameTimestampSupportedANDROID(EGLDisplay dpy, EGLSurface surface,
                                                EGLint timestamp) {
    clearError();

    egl_connection_t* const cnx = &gEGLImpl;
    return cnx->platform.eglGetFrameTimestampSupportedANDROID(dpy, surface, timestamp);
}
