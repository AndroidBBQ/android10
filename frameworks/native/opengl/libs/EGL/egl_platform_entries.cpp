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

#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "egl_platform_entries.h"

#include <ctype.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#include <hardware/gralloc1.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_angle.h>

#include <android/hardware_buffer.h>
#include <android-base/strings.h>
#include <graphicsenv/GraphicsEnv.h>
#include <private/android/AHardwareBufferHelpers.h>

#include <cutils/compiler.h>
#include <cutils/properties.h>
#include <log/log.h>

#include <condition_variable>
#include <deque>
#include <mutex>
#include <unordered_map>
#include <string>
#include <thread>

#include "../egl_impl.h"

#include "egl_display.h"
#include "egl_object.h"
#include "egl_layers.h"
#include "egl_tls.h"
#include "egl_trace.h"

using namespace android;

// ----------------------------------------------------------------------------

namespace android {

using nsecs_t = int64_t;

struct extension_map_t {
    const char* name;
    __eglMustCastToProperFunctionPointerType address;
};

/*
 * This is the list of EGL extensions exposed to applications.
 *
 * Some of them (gBuiltinExtensionString) are implemented entirely in this EGL
 * wrapper and are always available.
 *
 * The rest (gExtensionString) depend on support in the EGL driver, and are
 * only available if the driver supports them. However, some of these must be
 * supported because they are used by the Android system itself; these are
 * listed as mandatory below and are required by the CDD. The system *assumes*
 * the mandatory extensions are present and may not function properly if some
 * are missing.
 *
 * NOTE: Both strings MUST have a single space as the last character.
 */

extern char const * const gBuiltinExtensionString;
extern char const * const gExtensionString;

// clang-format off
// Extensions implemented by the EGL wrapper.
char const * const gBuiltinExtensionString =
        "EGL_KHR_get_all_proc_addresses "
        "EGL_ANDROID_presentation_time "
        "EGL_KHR_swap_buffers_with_damage "
        "EGL_ANDROID_get_native_client_buffer "
        "EGL_ANDROID_front_buffer_auto_refresh "
        "EGL_ANDROID_get_frame_timestamps "
        "EGL_EXT_surface_SMPTE2086_metadata "
        "EGL_EXT_surface_CTA861_3_metadata "
        ;

// Whitelist of extensions exposed to applications if implemented in the vendor driver.
char const * const gExtensionString  =
        "EGL_KHR_image "                        // mandatory
        "EGL_KHR_image_base "                   // mandatory
        "EGL_EXT_image_gl_colorspace "
        "EGL_KHR_image_pixmap "
        "EGL_KHR_lock_surface "
        "EGL_KHR_gl_colorspace "
        "EGL_KHR_gl_texture_2D_image "
        "EGL_KHR_gl_texture_3D_image "
        "EGL_KHR_gl_texture_cubemap_image "
        "EGL_KHR_gl_renderbuffer_image "
        "EGL_KHR_reusable_sync "
        "EGL_KHR_fence_sync "
        "EGL_KHR_create_context "
        "EGL_KHR_config_attribs "
        "EGL_KHR_surfaceless_context "
        "EGL_KHR_stream "
        "EGL_KHR_stream_fifo "
        "EGL_KHR_stream_producer_eglsurface "
        "EGL_KHR_stream_consumer_gltexture "
        "EGL_KHR_stream_cross_process_fd "
        "EGL_EXT_create_context_robustness "
        "EGL_NV_system_time "
        "EGL_ANDROID_image_native_buffer "      // mandatory
        "EGL_KHR_wait_sync "                    // strongly recommended
        "EGL_ANDROID_recordable "               // mandatory
        "EGL_KHR_partial_update "               // strongly recommended
        "EGL_EXT_pixel_format_float "
        "EGL_EXT_buffer_age "                   // strongly recommended with partial_update
        "EGL_KHR_create_context_no_error "
        "EGL_KHR_mutable_render_buffer "
        "EGL_EXT_yuv_surface "
        "EGL_EXT_protected_content "
        "EGL_IMG_context_priority "
        "EGL_KHR_no_config_context "
        ;

char const * const gClientExtensionString =
        "EGL_EXT_client_extensions "
        "EGL_KHR_platform_android "
        "EGL_ANGLE_platform_angle "
        "EGL_ANDROID_GLES_layers";
// clang-format on

// extensions not exposed to applications but used by the ANDROID system
//      "EGL_ANDROID_blob_cache "               // strongly recommended
//      "EGL_IMG_hibernate_process "            // optional
//      "EGL_ANDROID_native_fence_sync "        // strongly recommended
//      "EGL_ANDROID_framebuffer_target "       // mandatory for HWC 1.1

/*
 * EGL Extensions entry-points exposed to 3rd party applications
 * (keep in sync with gExtensionString above)
 *
 */
static const extension_map_t sExtensionMap[] = {
    // EGL_KHR_lock_surface
    { "eglLockSurfaceKHR",
            (__eglMustCastToProperFunctionPointerType)&eglLockSurfaceKHR },
    { "eglUnlockSurfaceKHR",
            (__eglMustCastToProperFunctionPointerType)&eglUnlockSurfaceKHR },

    // EGL_KHR_image, EGL_KHR_image_base
    { "eglCreateImageKHR",
            (__eglMustCastToProperFunctionPointerType)&eglCreateImageKHR },
    { "eglDestroyImageKHR",
            (__eglMustCastToProperFunctionPointerType)&eglDestroyImageKHR },

    // EGL_KHR_reusable_sync, EGL_KHR_fence_sync
    { "eglCreateSyncKHR",
            (__eglMustCastToProperFunctionPointerType)&eglCreateSyncKHR },
    { "eglDestroySyncKHR",
            (__eglMustCastToProperFunctionPointerType)&eglDestroySyncKHR },
    { "eglClientWaitSyncKHR",
            (__eglMustCastToProperFunctionPointerType)&eglClientWaitSyncKHR },
    { "eglSignalSyncKHR",
            (__eglMustCastToProperFunctionPointerType)&eglSignalSyncKHR },
    { "eglGetSyncAttribKHR",
            (__eglMustCastToProperFunctionPointerType)&eglGetSyncAttribKHR },

    // EGL_NV_system_time
    { "eglGetSystemTimeFrequencyNV",
            (__eglMustCastToProperFunctionPointerType)&eglGetSystemTimeFrequencyNV },
    { "eglGetSystemTimeNV",
            (__eglMustCastToProperFunctionPointerType)&eglGetSystemTimeNV },

    // EGL_KHR_wait_sync
    { "eglWaitSyncKHR",
            (__eglMustCastToProperFunctionPointerType)&eglWaitSyncKHR },

    // EGL_ANDROID_presentation_time
    { "eglPresentationTimeANDROID",
            (__eglMustCastToProperFunctionPointerType)&eglPresentationTimeANDROID },

    // EGL_KHR_swap_buffers_with_damage
    { "eglSwapBuffersWithDamageKHR",
            (__eglMustCastToProperFunctionPointerType)&eglSwapBuffersWithDamageKHR },

    // EGL_ANDROID_get_native_client_buffer
    { "eglGetNativeClientBufferANDROID",
            (__eglMustCastToProperFunctionPointerType)&eglGetNativeClientBufferANDROID },

    // EGL_KHR_partial_update
    { "eglSetDamageRegionKHR",
            (__eglMustCastToProperFunctionPointerType)&eglSetDamageRegionKHR },

    { "eglCreateStreamKHR",
            (__eglMustCastToProperFunctionPointerType)&eglCreateStreamKHR },
    { "eglDestroyStreamKHR",
            (__eglMustCastToProperFunctionPointerType)&eglDestroyStreamKHR },
    { "eglStreamAttribKHR",
            (__eglMustCastToProperFunctionPointerType)&eglStreamAttribKHR },
    { "eglQueryStreamKHR",
            (__eglMustCastToProperFunctionPointerType)&eglQueryStreamKHR },
    { "eglQueryStreamu64KHR",
            (__eglMustCastToProperFunctionPointerType)&eglQueryStreamu64KHR },
    { "eglQueryStreamTimeKHR",
            (__eglMustCastToProperFunctionPointerType)&eglQueryStreamTimeKHR },
    { "eglCreateStreamProducerSurfaceKHR",
            (__eglMustCastToProperFunctionPointerType)&eglCreateStreamProducerSurfaceKHR },
    { "eglStreamConsumerGLTextureExternalKHR",
            (__eglMustCastToProperFunctionPointerType)&eglStreamConsumerGLTextureExternalKHR },
    { "eglStreamConsumerAcquireKHR",
            (__eglMustCastToProperFunctionPointerType)&eglStreamConsumerAcquireKHR },
    { "eglStreamConsumerReleaseKHR",
            (__eglMustCastToProperFunctionPointerType)&eglStreamConsumerReleaseKHR },
    { "eglGetStreamFileDescriptorKHR",
            (__eglMustCastToProperFunctionPointerType)&eglGetStreamFileDescriptorKHR },
    { "eglCreateStreamFromFileDescriptorKHR",
            (__eglMustCastToProperFunctionPointerType)&eglCreateStreamFromFileDescriptorKHR },

    // EGL_ANDROID_get_frame_timestamps
    { "eglGetNextFrameIdANDROID",
            (__eglMustCastToProperFunctionPointerType)&eglGetNextFrameIdANDROID },
    { "eglGetCompositorTimingANDROID",
            (__eglMustCastToProperFunctionPointerType)&eglGetCompositorTimingANDROID },
    { "eglGetCompositorTimingSupportedANDROID",
            (__eglMustCastToProperFunctionPointerType)&eglGetCompositorTimingSupportedANDROID },
    { "eglGetFrameTimestampsANDROID",
            (__eglMustCastToProperFunctionPointerType)&eglGetFrameTimestampsANDROID },
    { "eglGetFrameTimestampSupportedANDROID",
            (__eglMustCastToProperFunctionPointerType)&eglGetFrameTimestampSupportedANDROID },

    // EGL_ANDROID_native_fence_sync
    { "eglDupNativeFenceFDANDROID",
            (__eglMustCastToProperFunctionPointerType)&eglDupNativeFenceFDANDROID },
};

/*
 * These extensions entry-points should not be exposed to applications.
 * They're used internally by the Android EGL layer.
 */
#define FILTER_EXTENSIONS(procname) \
        (!strcmp((procname), "eglSetBlobCacheFuncsANDROID") ||    \
         !strcmp((procname), "eglHibernateProcessIMG")      ||    \
         !strcmp((procname), "eglAwakenProcessIMG"))

// accesses protected by sExtensionMapMutex
static std::unordered_map<std::string, __eglMustCastToProperFunctionPointerType> sGLExtensionMap;
static std::unordered_map<std::string, int> sGLExtensionSlotMap;

static int sGLExtensionSlot = 0;
static pthread_mutex_t sExtensionMapMutex = PTHREAD_MUTEX_INITIALIZER;

static void(*findProcAddress(const char* name,
        const extension_map_t* map, size_t n))() {
    for (uint32_t i=0 ; i<n ; i++) {
        if (!strcmp(name, map[i].name)) {
            return map[i].address;
        }
    }
    return nullptr;
}

// ----------------------------------------------------------------------------

extern void setGLHooksThreadSpecific(gl_hooks_t const *value);
extern EGLBoolean egl_init_drivers();
extern const __eglMustCastToProperFunctionPointerType gExtensionForwarders[MAX_NUMBER_OF_GL_EXTENSIONS];
extern gl_hooks_t gHooksTrace;

// ----------------------------------------------------------------------------

static inline EGLContext getContext() { return egl_tls_t::getContext(); }

// ----------------------------------------------------------------------------

static EGLDisplay eglGetPlatformDisplayTmpl(EGLenum platform, EGLNativeDisplayType display,
                                            const EGLAttrib* attrib_list) {
    if (platform != EGL_PLATFORM_ANDROID_KHR) {
        return setError(EGL_BAD_PARAMETER, EGL_NO_DISPLAY);
    }

    uintptr_t index = reinterpret_cast<uintptr_t>(display);
    if (index >= NUM_DISPLAYS) {
        return setError(EGL_BAD_PARAMETER, EGL_NO_DISPLAY);
    }

    EGLDisplay dpy = egl_display_t::getFromNativeDisplay(display, attrib_list);
    return dpy;
}

EGLDisplay eglGetDisplayImpl(EGLNativeDisplayType display) {
    return eglGetPlatformDisplayTmpl(EGL_PLATFORM_ANDROID_KHR, display, nullptr);
}

EGLDisplay eglGetPlatformDisplayImpl(EGLenum platform, void* native_display,
                                     const EGLAttrib* attrib_list) {
    return eglGetPlatformDisplayTmpl(platform, static_cast<EGLNativeDisplayType>(native_display),
                                     attrib_list);
}

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

EGLBoolean eglInitializeImpl(EGLDisplay dpy, EGLint *major, EGLint *minor)
{
    egl_display_ptr dp = get_display(dpy);
    if (!dp) return setError(EGL_BAD_DISPLAY, (EGLBoolean)EGL_FALSE);

    EGLBoolean res = dp->initialize(major, minor);

    return res;
}

EGLBoolean eglTerminateImpl(EGLDisplay dpy)
{
    // NOTE: don't unload the drivers b/c some APIs can be called
    // after eglTerminate() has been called. eglTerminate() only
    // terminates an EGLDisplay, not a EGL itself.

    egl_display_ptr dp = get_display(dpy);
    if (!dp) return setError(EGL_BAD_DISPLAY, (EGLBoolean)EGL_FALSE);

    EGLBoolean res = dp->terminate();

    return res;
}

// ----------------------------------------------------------------------------
// configuration
// ----------------------------------------------------------------------------

EGLBoolean eglGetConfigsImpl(EGLDisplay dpy,
                             EGLConfig *configs,
                             EGLint config_size, EGLint *num_config)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    if (num_config==nullptr) {
        return setError(EGL_BAD_PARAMETER, (EGLBoolean)EGL_FALSE);
    }

    EGLBoolean res = EGL_FALSE;
    *num_config = 0;

    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso) {
        res = cnx->egl.eglGetConfigs(
                dp->disp.dpy, configs, config_size, num_config);
    }

    return res;
}

EGLBoolean eglChooseConfigImpl( EGLDisplay dpy, const EGLint *attrib_list,
                                EGLConfig *configs, EGLint config_size,
                                EGLint *num_config)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    if (num_config==nullptr) {
        return setError(EGL_BAD_PARAMETER, (EGLBoolean)EGL_FALSE);
    }

    EGLBoolean res = EGL_FALSE;
    *num_config = 0;

    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso) {
        if (attrib_list) {
            char value[PROPERTY_VALUE_MAX];
            property_get("debug.egl.force_msaa", value, "false");

            if (!strcmp(value, "true")) {
                size_t attribCount = 0;
                EGLint attrib = attrib_list[0];

                // Only enable MSAA if the context is OpenGL ES 2.0 and
                // if no caveat is requested
                const EGLint *attribRendererable = nullptr;
                const EGLint *attribCaveat = nullptr;

                // Count the number of attributes and look for
                // EGL_RENDERABLE_TYPE and EGL_CONFIG_CAVEAT
                while (attrib != EGL_NONE) {
                    attrib = attrib_list[attribCount];
                    switch (attrib) {
                        case EGL_RENDERABLE_TYPE:
                            attribRendererable = &attrib_list[attribCount];
                            break;
                        case EGL_CONFIG_CAVEAT:
                            attribCaveat = &attrib_list[attribCount];
                            break;
                        default:
                            break;
                    }
                    attribCount++;
                }

                if (attribRendererable && attribRendererable[1] == EGL_OPENGL_ES2_BIT &&
                        (!attribCaveat || attribCaveat[1] != EGL_NONE)) {

                    // Insert 2 extra attributes to force-enable MSAA 4x
                    EGLint aaAttribs[attribCount + 4];
                    aaAttribs[0] = EGL_SAMPLE_BUFFERS;
                    aaAttribs[1] = 1;
                    aaAttribs[2] = EGL_SAMPLES;
                    aaAttribs[3] = 4;

                    memcpy(&aaAttribs[4], attrib_list, attribCount * sizeof(EGLint));

                    EGLint numConfigAA;
                    EGLBoolean resAA = cnx->egl.eglChooseConfig(
                            dp->disp.dpy, aaAttribs, configs, config_size, &numConfigAA);

                    if (resAA == EGL_TRUE && numConfigAA > 0) {
                        ALOGD("Enabling MSAA 4x");
                        *num_config = numConfigAA;
                        return resAA;
                    }
                }
            }
        }

        res = cnx->egl.eglChooseConfig(
                dp->disp.dpy, attrib_list, configs, config_size, num_config);
    }
    return res;
}

EGLBoolean eglGetConfigAttribImpl(EGLDisplay dpy, EGLConfig config,
        EGLint attribute, EGLint *value)
{
    egl_connection_t* cnx = nullptr;
    const egl_display_ptr dp = validate_display_connection(dpy, cnx);
    if (!dp) return EGL_FALSE;

    return cnx->egl.eglGetConfigAttrib(
            dp->disp.dpy, config, attribute, value);
}

// ----------------------------------------------------------------------------
// surfaces
// ----------------------------------------------------------------------------

// Translates EGL color spaces to Android data spaces.
static android_dataspace dataSpaceFromEGLColorSpace(EGLint colorspace) {
    if (colorspace == EGL_GL_COLORSPACE_LINEAR_KHR) {
        return HAL_DATASPACE_UNKNOWN;
    } else if (colorspace == EGL_GL_COLORSPACE_SRGB_KHR) {
        return HAL_DATASPACE_V0_SRGB;
    } else if (colorspace == EGL_GL_COLORSPACE_DISPLAY_P3_EXT) {
        return HAL_DATASPACE_DISPLAY_P3;
    } else if (colorspace == EGL_GL_COLORSPACE_DISPLAY_P3_LINEAR_EXT) {
        return HAL_DATASPACE_DISPLAY_P3_LINEAR;
    } else if (colorspace == EGL_GL_COLORSPACE_DISPLAY_P3_PASSTHROUGH_EXT) {
        return HAL_DATASPACE_DISPLAY_P3;
    } else if (colorspace == EGL_GL_COLORSPACE_SCRGB_EXT) {
        return HAL_DATASPACE_V0_SCRGB;
    } else if (colorspace == EGL_GL_COLORSPACE_SCRGB_LINEAR_EXT) {
        return HAL_DATASPACE_V0_SCRGB_LINEAR;
    } else if (colorspace == EGL_GL_COLORSPACE_BT2020_LINEAR_EXT) {
        return HAL_DATASPACE_BT2020_LINEAR;
    } else if (colorspace == EGL_GL_COLORSPACE_BT2020_PQ_EXT) {
        return HAL_DATASPACE_BT2020_PQ;
    }
    return HAL_DATASPACE_UNKNOWN;
}

// Get the colorspace value that should be reported from queries. When the colorspace
// is unknown (no attribute passed), default to reporting LINEAR.
static EGLint getReportedColorSpace(EGLint colorspace) {
    return colorspace == EGL_UNKNOWN ? EGL_GL_COLORSPACE_LINEAR_KHR : colorspace;
}

// Returns a list of color spaces understood by the vendor EGL driver.
static std::vector<EGLint> getDriverColorSpaces(egl_display_ptr dp) {
    std::vector<EGLint> colorSpaces;

    // sRGB and linear are always supported when color space support is present.
    colorSpaces.push_back(EGL_GL_COLORSPACE_SRGB_KHR);
    colorSpaces.push_back(EGL_GL_COLORSPACE_LINEAR_KHR);

    if (findExtension(dp->disp.queryString.extensions, "EGL_EXT_gl_colorspace_display_p3")) {
        colorSpaces.push_back(EGL_GL_COLORSPACE_DISPLAY_P3_EXT);
    }
    if (findExtension(dp->disp.queryString.extensions, "EGL_EXT_gl_colorspace_scrgb")) {
        colorSpaces.push_back(EGL_GL_COLORSPACE_SCRGB_EXT);
    }
    if (findExtension(dp->disp.queryString.extensions, "EGL_EXT_gl_colorspace_scrgb_linear")) {
        colorSpaces.push_back(EGL_GL_COLORSPACE_SCRGB_LINEAR_EXT);
    }
    if (findExtension(dp->disp.queryString.extensions, "EGL_EXT_gl_colorspace_bt2020_linear")) {
        colorSpaces.push_back(EGL_GL_COLORSPACE_BT2020_LINEAR_EXT);
    }
    if (findExtension(dp->disp.queryString.extensions, "EGL_EXT_gl_colorspace_bt2020_pq")) {
        colorSpaces.push_back(EGL_GL_COLORSPACE_BT2020_PQ_EXT);
    }
    if (findExtension(dp->disp.queryString.extensions, "EGL_EXT_gl_colorspace_display_p3_linear")) {
        colorSpaces.push_back(EGL_GL_COLORSPACE_DISPLAY_P3_LINEAR_EXT);
    }
    if (findExtension(dp->disp.queryString.extensions, "EGL_EXT_gl_colorspace_display_p3_passthrough")) {
        colorSpaces.push_back(EGL_GL_COLORSPACE_DISPLAY_P3_PASSTHROUGH_EXT);
    }
    return colorSpaces;
}

// Cleans up color space related parameters that the driver does not understand.
// If there is no color space attribute in attrib_list, colorSpace is left
// unmodified.
template <typename AttrType>
static EGLBoolean processAttributes(egl_display_ptr dp, ANativeWindow* window,
                                    const AttrType* attrib_list, EGLint* colorSpace,
                                    std::vector<AttrType>* strippedAttribList) {
    for (const AttrType* attr = attrib_list; attr && attr[0] != EGL_NONE; attr += 2) {
        bool copyAttribute = true;
        if (attr[0] == EGL_GL_COLORSPACE_KHR) {
            switch (attr[1]) {
                case EGL_GL_COLORSPACE_LINEAR_KHR:
                case EGL_GL_COLORSPACE_SRGB_KHR:
                case EGL_GL_COLORSPACE_DISPLAY_P3_EXT:
                case EGL_GL_COLORSPACE_DISPLAY_P3_PASSTHROUGH_EXT:
                case EGL_GL_COLORSPACE_SCRGB_LINEAR_EXT:
                case EGL_GL_COLORSPACE_SCRGB_EXT:
                case EGL_GL_COLORSPACE_BT2020_LINEAR_EXT:
                case EGL_GL_COLORSPACE_BT2020_PQ_EXT:
                case EGL_GL_COLORSPACE_DISPLAY_P3_LINEAR_EXT:
                    // Fail immediately if the driver doesn't have color space support at all.
                    if (!dp->hasColorSpaceSupport) return setError(EGL_BAD_ATTRIBUTE, EGL_FALSE);
                    break;
                default:
                    // BAD_ATTRIBUTE if attr is not any of the EGL_GL_COLORSPACE_*
                    return setError(EGL_BAD_ATTRIBUTE, EGL_FALSE);
            }
            *colorSpace = static_cast<EGLint>(attr[1]);

            // Strip the attribute if the driver doesn't understand it.
            copyAttribute = false;
            std::vector<EGLint> driverColorSpaces = getDriverColorSpaces(dp);
            for (auto driverColorSpace : driverColorSpaces) {
                if (static_cast<EGLint>(attr[1]) == driverColorSpace) {
                    copyAttribute = true;
                    break;
                }
            }

            // If the driver doesn't understand it, we should map sRGB-encoded P3 to
            // sRGB rather than just dropping the colorspace on the floor.
            // For this format, the driver is expected to apply the sRGB
            // transfer function during framebuffer operations.
            if (!copyAttribute && attr[1] == EGL_GL_COLORSPACE_DISPLAY_P3_EXT) {
                strippedAttribList->push_back(attr[0]);
                strippedAttribList->push_back(EGL_GL_COLORSPACE_SRGB_KHR);
            }
        }
        if (copyAttribute) {
            strippedAttribList->push_back(attr[0]);
            strippedAttribList->push_back(attr[1]);
        }
    }
    // Terminate the attribute list.
    strippedAttribList->push_back(EGL_NONE);

    // If the passed color space has wide color gamut, check whether the target native window
    // supports wide color.
    const bool colorSpaceIsNarrow = *colorSpace == EGL_GL_COLORSPACE_SRGB_KHR ||
            *colorSpace == EGL_GL_COLORSPACE_LINEAR_KHR || *colorSpace == EGL_UNKNOWN;
    if (window && !colorSpaceIsNarrow) {
        bool windowSupportsWideColor = true;
        // Ordinarily we'd put a call to native_window_get_wide_color_support
        // at the beginning of the function so that we'll have the
        // result when needed elsewhere in the function.
        // However, because eglCreateWindowSurface is called by SurfaceFlinger and
        // SurfaceFlinger is required to answer the call below we would
        // end up in a deadlock situation. By moving the call to only happen
        // if the application has specifically asked for wide-color we avoid
        // the deadlock with SurfaceFlinger since it will not ask for a
        // wide-color surface.
        int err = native_window_get_wide_color_support(window, &windowSupportsWideColor);

        if (err) {
            ALOGE("processAttributes: invalid window (win=%p) "
                  "failed (%#x) (already connected to another API?)",
                  window, err);
            return setError(EGL_BAD_NATIVE_WINDOW, EGL_FALSE);
        }
        if (!windowSupportsWideColor) {
            // Application has asked for a wide-color colorspace but
            // wide-color support isn't available on the display the window is on.
            return setError(EGL_BAD_MATCH, EGL_FALSE);
        }
    }
    return true;
}

// Note: This only works for existing GLenum's that are all 32bits.
// If you have 64bit attributes (e.g. pointers) you shouldn't be calling this.
void convertAttribs(const EGLAttrib* attribList, std::vector<EGLint>& newList) {
    for (const EGLAttrib* attr = attribList; attr && attr[0] != EGL_NONE; attr += 2) {
        newList.push_back(static_cast<EGLint>(attr[0]));
        newList.push_back(static_cast<EGLint>(attr[1]));
    }
    newList.push_back(EGL_NONE);
}

// Gets the native pixel format corrsponding to the passed EGLConfig.
void getNativePixelFormat(EGLDisplay dpy, egl_connection_t* cnx, EGLConfig config,
                          android_pixel_format* format) {
    // Set the native window's buffers format to match what this config requests.
    // Whether to use sRGB gamma is not part of the EGLconfig, but is part
    // of our native format. So if sRGB gamma is requested, we have to
    // modify the EGLconfig's format before setting the native window's
    // format.

    EGLint componentType = EGL_COLOR_COMPONENT_TYPE_FIXED_EXT;
    cnx->egl.eglGetConfigAttrib(dpy, config, EGL_COLOR_COMPONENT_TYPE_EXT, &componentType);

    EGLint a = 0;
    EGLint r, g, b;
    r = g = b = 0;
    cnx->egl.eglGetConfigAttrib(dpy, config, EGL_RED_SIZE, &r);
    cnx->egl.eglGetConfigAttrib(dpy, config, EGL_GREEN_SIZE, &g);
    cnx->egl.eglGetConfigAttrib(dpy, config, EGL_BLUE_SIZE, &b);
    cnx->egl.eglGetConfigAttrib(dpy, config, EGL_ALPHA_SIZE, &a);
    EGLint colorDepth = r + g + b;

    // Today, the driver only understands sRGB and linear on 888X
    // formats. Strip other colorspaces from the attribute list and
    // only use them to set the dataspace via
    // native_window_set_buffers_dataspace
    // if pixel format is RGBX 8888
    //    TBD: Can test for future extensions that indicate that driver
    //    handles requested color space and we can let it through.
    //    allow SRGB and LINEAR. All others need to be stripped.
    // else if 565, 4444
    //    TBD: Can we assume these are supported if 8888 is?
    // else if FP16 or 1010102
    //    strip colorspace from attribs.
    // endif
    if (a == 0) {
        if (colorDepth <= 16) {
            *format = HAL_PIXEL_FORMAT_RGB_565;
        } else {
            if (componentType == EGL_COLOR_COMPONENT_TYPE_FIXED_EXT) {
                if (colorDepth > 24) {
                    *format = HAL_PIXEL_FORMAT_RGBA_1010102;
                } else {
                    *format = HAL_PIXEL_FORMAT_RGBX_8888;
                }
            } else {
                *format = HAL_PIXEL_FORMAT_RGBA_FP16;
            }
        }
    } else {
        if (componentType == EGL_COLOR_COMPONENT_TYPE_FIXED_EXT) {
            if (colorDepth > 24) {
                *format = HAL_PIXEL_FORMAT_RGBA_1010102;
            } else {
                *format = HAL_PIXEL_FORMAT_RGBA_8888;
            }
        } else {
            *format = HAL_PIXEL_FORMAT_RGBA_FP16;
        }
    }
}

EGLBoolean sendSurfaceMetadata(egl_surface_t* s) {
    android_smpte2086_metadata smpteMetadata;
    if (s->getSmpte2086Metadata(smpteMetadata)) {
        int err =
                native_window_set_buffers_smpte2086_metadata(s->getNativeWindow(), &smpteMetadata);
        s->resetSmpte2086Metadata();
        if (err != 0) {
            ALOGE("error setting native window smpte2086 metadata: %s (%d)", strerror(-err), err);
            return EGL_FALSE;
        }
    }
    android_cta861_3_metadata cta8613Metadata;
    if (s->getCta8613Metadata(cta8613Metadata)) {
        int err =
                native_window_set_buffers_cta861_3_metadata(s->getNativeWindow(), &cta8613Metadata);
        s->resetCta8613Metadata();
        if (err != 0) {
            ALOGE("error setting native window CTS 861.3 metadata: %s (%d)", strerror(-err), err);
            return EGL_FALSE;
        }
    }
    return EGL_TRUE;
}

template <typename AttrType, typename CreateFuncType>
EGLSurface eglCreateWindowSurfaceTmpl(egl_display_ptr dp, egl_connection_t* cnx, EGLConfig config,
                                      ANativeWindow* window, const AttrType* attrib_list,
                                      CreateFuncType createWindowSurfaceFunc) {
    const AttrType* origAttribList = attrib_list;

    if (!window) {
        return setError(EGL_BAD_NATIVE_WINDOW, EGL_NO_SURFACE);
    }

    int value = 0;
    window->query(window, NATIVE_WINDOW_IS_VALID, &value);
    if (!value) {
        return setError(EGL_BAD_NATIVE_WINDOW, EGL_NO_SURFACE);
    }

    // NOTE: When using Vulkan backend, the Vulkan runtime makes all the
    // native_window_* calls, so don't do them here.
    if (cnx->angleBackend != EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE) {
        int result = native_window_api_connect(window, NATIVE_WINDOW_API_EGL);
        if (result < 0) {
            ALOGE("eglCreateWindowSurface: native_window_api_connect (win=%p) "
                  "failed (%#x) (already connected to another API?)",
                  window, result);
            return setError(EGL_BAD_ALLOC, EGL_NO_SURFACE);
        }
    }

    EGLDisplay iDpy = dp->disp.dpy;
    android_pixel_format format;
    getNativePixelFormat(iDpy, cnx, config, &format);

    // now select correct colorspace and dataspace based on user's attribute list
    EGLint colorSpace = EGL_UNKNOWN;
    std::vector<AttrType> strippedAttribList;
    if (!processAttributes<AttrType>(dp, window, attrib_list, &colorSpace, &strippedAttribList)) {
        ALOGE("error invalid colorspace: %d", colorSpace);
        if (cnx->angleBackend != EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE) {
            native_window_api_disconnect(window, NATIVE_WINDOW_API_EGL);
        }
        return EGL_NO_SURFACE;
    }
    attrib_list = strippedAttribList.data();

    if (cnx->angleBackend != EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE) {
        int err = native_window_set_buffers_format(window, format);
        if (err != 0) {
            ALOGE("error setting native window pixel format: %s (%d)", strerror(-err), err);
            native_window_api_disconnect(window, NATIVE_WINDOW_API_EGL);
            return setError(EGL_BAD_NATIVE_WINDOW, EGL_NO_SURFACE);
        }

        android_dataspace dataSpace = dataSpaceFromEGLColorSpace(colorSpace);
        // Set dataSpace even if it could be HAL_DATASPACE_UNKNOWN.
        // HAL_DATASPACE_UNKNOWN is the default value, but it may have changed
        // at this point.
        err = native_window_set_buffers_data_space(window, dataSpace);
        if (err != 0) {
            ALOGE("error setting native window pixel dataSpace: %s (%d)", strerror(-err), err);
            native_window_api_disconnect(window, NATIVE_WINDOW_API_EGL);
            return setError(EGL_BAD_NATIVE_WINDOW, EGL_NO_SURFACE);
        }
    }

    // the EGL spec requires that a new EGLSurface default to swap interval
    // 1, so explicitly set that on the window here.
    window->setSwapInterval(window, 1);

    EGLSurface surface = createWindowSurfaceFunc(iDpy, config, window, attrib_list);
    if (surface != EGL_NO_SURFACE) {
        egl_surface_t* s = new egl_surface_t(dp.get(), config, window, surface,
                                             getReportedColorSpace(colorSpace), cnx);
        return s;
    }

    // EGLSurface creation failed
    if (cnx->angleBackend != EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE) {
        native_window_set_buffers_format(window, 0);
        native_window_api_disconnect(window, NATIVE_WINDOW_API_EGL);
    }
    return EGL_NO_SURFACE;
}

typedef EGLSurface(EGLAPIENTRYP PFNEGLCREATEWINDOWSURFACEPROC)(EGLDisplay dpy, EGLConfig config,
                                                               NativeWindowType window,
                                                               const EGLint* attrib_list);
typedef EGLSurface(EGLAPIENTRYP PFNEGLCREATEPLATFORMWINDOWSURFACEPROC)(
        EGLDisplay dpy, EGLConfig config, void* native_window, const EGLAttrib* attrib_list);

EGLSurface eglCreateWindowSurfaceImpl(EGLDisplay dpy, EGLConfig config, NativeWindowType window,
                                      const EGLint* attrib_list) {
    egl_connection_t* cnx = NULL;
    egl_display_ptr dp = validate_display_connection(dpy, cnx);
    if (dp) {
        return eglCreateWindowSurfaceTmpl<
                EGLint, PFNEGLCREATEWINDOWSURFACEPROC>(dp, cnx, config, window, attrib_list,
                                                       cnx->egl.eglCreateWindowSurface);
    }
    return EGL_NO_SURFACE;
}

EGLSurface eglCreatePlatformWindowSurfaceImpl(EGLDisplay dpy, EGLConfig config, void* native_window,
                                              const EGLAttrib* attrib_list) {
    egl_connection_t* cnx = NULL;
    egl_display_ptr dp = validate_display_connection(dpy, cnx);
    if (dp) {
        if (cnx->driverVersion >= EGL_MAKE_VERSION(1, 5, 0)) {
            if (cnx->egl.eglCreatePlatformWindowSurface) {
                return eglCreateWindowSurfaceTmpl<EGLAttrib, PFNEGLCREATEPLATFORMWINDOWSURFACEPROC>(
                        dp, cnx, config, static_cast<ANativeWindow*>(native_window), attrib_list,
                        cnx->egl.eglCreatePlatformWindowSurface);
            }
            // driver doesn't support native function, return EGL_BAD_DISPLAY
            ALOGE("Driver indicates EGL 1.5 support, but does not have "
                  "eglCreatePlatformWindowSurface");
            return setError(EGL_BAD_DISPLAY, EGL_NO_SURFACE);
        }

        std::vector<EGLint> convertedAttribs;
        convertAttribs(attrib_list, convertedAttribs);
        if (cnx->egl.eglCreatePlatformWindowSurfaceEXT) {
            return eglCreateWindowSurfaceTmpl<EGLint, PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC>(
                    dp, cnx, config, static_cast<ANativeWindow*>(native_window),
                    convertedAttribs.data(), cnx->egl.eglCreatePlatformWindowSurfaceEXT);
        } else {
            return eglCreateWindowSurfaceTmpl<
                    EGLint, PFNEGLCREATEWINDOWSURFACEPROC>(dp, cnx, config,
                                                           static_cast<ANativeWindow*>(
                                                                   native_window),
                                                           convertedAttribs.data(),
                                                           cnx->egl.eglCreateWindowSurface);
        }
    }
    return EGL_NO_SURFACE;
}

EGLSurface eglCreatePlatformPixmapSurfaceImpl(EGLDisplay dpy, EGLConfig /*config*/,
                                              void* /*native_pixmap*/,
                                              const EGLAttrib* /*attrib_list*/) {
    // Per EGL_KHR_platform_android:
    // It is not valid to call eglCreatePlatformPixmapSurface with a <dpy> that
    // belongs to the Android platform. Any such call fails and generates
    // an EGL_BAD_PARAMETER error.

    egl_connection_t* cnx = NULL;
    egl_display_ptr dp = validate_display_connection(dpy, cnx);
    if (dp) {
        return setError(EGL_BAD_PARAMETER, EGL_NO_SURFACE);
    }
    return EGL_NO_SURFACE;
}

EGLSurface eglCreatePixmapSurfaceImpl(EGLDisplay dpy, EGLConfig /*config*/,
                                      NativePixmapType /*pixmap*/, const EGLint* /*attrib_list*/) {
    egl_connection_t* cnx = nullptr;
    egl_display_ptr dp = validate_display_connection(dpy, cnx);
    if (dp) {
        return setError(EGL_BAD_PARAMETER, EGL_NO_SURFACE);
    }
    return EGL_NO_SURFACE;
}

EGLSurface eglCreatePbufferSurfaceImpl(EGLDisplay dpy, EGLConfig config,
                                       const EGLint* attrib_list) {
    egl_connection_t* cnx = nullptr;
    egl_display_ptr dp = validate_display_connection(dpy, cnx);
    if (dp) {
        EGLDisplay iDpy = dp->disp.dpy;
        android_pixel_format format;
        getNativePixelFormat(iDpy, cnx, config, &format);

        // Select correct colorspace based on user's attribute list
        EGLint colorSpace = EGL_UNKNOWN;
        std::vector<EGLint> strippedAttribList;
        if (!processAttributes(dp, nullptr, attrib_list, &colorSpace, &strippedAttribList)) {
            ALOGE("error invalid colorspace: %d", colorSpace);
            return EGL_NO_SURFACE;
        }
        attrib_list = strippedAttribList.data();

        EGLSurface surface = cnx->egl.eglCreatePbufferSurface(dp->disp.dpy, config, attrib_list);
        if (surface != EGL_NO_SURFACE) {
            egl_surface_t* s = new egl_surface_t(dp.get(), config, nullptr, surface,
                                                 getReportedColorSpace(colorSpace), cnx);
            return s;
        }
    }
    return EGL_NO_SURFACE;
}

EGLBoolean eglDestroySurfaceImpl(EGLDisplay dpy, EGLSurface surface) {
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    SurfaceRef _s(dp.get(), surface);
    if (!_s.get()) return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);

    egl_surface_t* const s = get_surface(surface);
    EGLBoolean result = s->cnx->egl.eglDestroySurface(dp->disp.dpy, s->surface);
    if (result == EGL_TRUE) {
        _s.terminate();
    }
    return result;
}

EGLBoolean eglQuerySurfaceImpl(EGLDisplay dpy, EGLSurface surface, EGLint attribute,
                               EGLint* value) {
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    SurfaceRef _s(dp.get(), surface);
    if (!_s.get()) return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);

    egl_surface_t const* const s = get_surface(surface);
    if (s->getColorSpaceAttribute(attribute, value)) {
        return EGL_TRUE;
    } else if (s->getSmpte2086Attribute(attribute, value)) {
        return EGL_TRUE;
    } else if (s->getCta8613Attribute(attribute, value)) {
        return EGL_TRUE;
    }
    return s->cnx->egl.eglQuerySurface(dp->disp.dpy, s->surface, attribute, value);
}

void EGLAPI eglBeginFrameImpl(EGLDisplay dpy, EGLSurface surface) {
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) {
        return;
    }

    SurfaceRef _s(dp.get(), surface);
    if (!_s.get()) {
        setError(EGL_BAD_SURFACE, EGL_FALSE);
    }
}

// ----------------------------------------------------------------------------
// Contexts
// ----------------------------------------------------------------------------

EGLContext eglCreateContextImpl(EGLDisplay dpy, EGLConfig config,
                                EGLContext share_list, const EGLint *attrib_list)
{
    egl_connection_t* cnx = nullptr;
    const egl_display_ptr dp = validate_display_connection(dpy, cnx);
    if (dp) {
        if (share_list != EGL_NO_CONTEXT) {
            if (!ContextRef(dp.get(), share_list).get()) {
                return setError(EGL_BAD_CONTEXT, EGL_NO_CONTEXT);
            }
            egl_context_t* const c = get_context(share_list);
            share_list = c->context;
        }
        // b/111083885 - If we are presenting EGL 1.4 interface to apps
        // error out on robust access attributes that are invalid
        // in EGL 1.4 as the driver may be fine with them but dEQP expects
        // tests to fail according to spec.
        if (attrib_list && (cnx->driverVersion < EGL_MAKE_VERSION(1, 5, 0))) {
            const EGLint* attrib_ptr = attrib_list;
            while (*attrib_ptr != EGL_NONE) {
                GLint attr = *attrib_ptr++;
                GLint value = *attrib_ptr++;
                if (attr == EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR) {
                    // We are GL ES context with EGL 1.4, this is an invalid
                    // attribute
                    return setError(EGL_BAD_ATTRIBUTE, EGL_NO_CONTEXT);
                }
            };
        }
        EGLContext context = cnx->egl.eglCreateContext(
                dp->disp.dpy, config, share_list, attrib_list);
        if (context != EGL_NO_CONTEXT) {
            // figure out if it's a GLESv1 or GLESv2
            int version = 0;
            if (attrib_list) {
                while (*attrib_list != EGL_NONE) {
                    GLint attr = *attrib_list++;
                    GLint value = *attrib_list++;
                    if (attr == EGL_CONTEXT_CLIENT_VERSION) {
                        if (value == 1) {
                            version = egl_connection_t::GLESv1_INDEX;
                        } else if (value == 2 || value == 3) {
                            version = egl_connection_t::GLESv2_INDEX;
                        }
                    }
                };
            }
            egl_context_t* c = new egl_context_t(dpy, context, config, cnx,
                    version);
            return c;
        }
    }
    return EGL_NO_CONTEXT;
}

EGLBoolean eglDestroyContextImpl(EGLDisplay dpy, EGLContext ctx)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp)
        return EGL_FALSE;

    ContextRef _c(dp.get(), ctx);
    if (!_c.get())
        return setError(EGL_BAD_CONTEXT, (EGLBoolean)EGL_FALSE);

    egl_context_t * const c = get_context(ctx);
    EGLBoolean result = c->cnx->egl.eglDestroyContext(dp->disp.dpy, c->context);
    if (result == EGL_TRUE) {
        _c.terminate();
    }
    return result;
}

EGLBoolean eglMakeCurrentImpl(  EGLDisplay dpy, EGLSurface draw,
                                EGLSurface read, EGLContext ctx)
{
    egl_display_ptr dp = validate_display(dpy);
    if (!dp) return setError(EGL_BAD_DISPLAY, (EGLBoolean)EGL_FALSE);

    // If ctx is not EGL_NO_CONTEXT, read is not EGL_NO_SURFACE, or draw is not
    // EGL_NO_SURFACE, then an EGL_NOT_INITIALIZED error is generated if dpy is
    // a valid but uninitialized display.
    if ( (ctx != EGL_NO_CONTEXT) || (read != EGL_NO_SURFACE) ||
         (draw != EGL_NO_SURFACE) ) {
        if (!dp->isReady()) return setError(EGL_NOT_INITIALIZED, (EGLBoolean)EGL_FALSE);
    }

    // get a reference to the object passed in
    ContextRef _c(dp.get(), ctx);
    SurfaceRef _d(dp.get(), draw);
    SurfaceRef _r(dp.get(), read);

    // validate the context (if not EGL_NO_CONTEXT)
    if ((ctx != EGL_NO_CONTEXT) && !_c.get()) {
        // EGL_NO_CONTEXT is valid
        return setError(EGL_BAD_CONTEXT, (EGLBoolean)EGL_FALSE);
    }

    // these are the underlying implementation's object
    EGLContext impl_ctx  = EGL_NO_CONTEXT;
    EGLSurface impl_draw = EGL_NO_SURFACE;
    EGLSurface impl_read = EGL_NO_SURFACE;

    // these are our objects structs passed in
    egl_context_t       * c = nullptr;
    egl_surface_t const * d = nullptr;
    egl_surface_t const * r = nullptr;

    // these are the current objects structs
    egl_context_t * cur_c = get_context(getContext());

    if (ctx != EGL_NO_CONTEXT) {
        c = get_context(ctx);
        impl_ctx = c->context;
    } else {
        // no context given, use the implementation of the current context
        if (draw != EGL_NO_SURFACE || read != EGL_NO_SURFACE) {
            // calling eglMakeCurrent( ..., !=0, !=0, EGL_NO_CONTEXT);
            return setError(EGL_BAD_MATCH, (EGLBoolean)EGL_FALSE);
        }
        if (cur_c == nullptr) {
            // no current context
            // not an error, there is just no current context.
            return EGL_TRUE;
        }
    }

    // retrieve the underlying implementation's draw EGLSurface
    if (draw != EGL_NO_SURFACE) {
        if (!_d.get()) return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);
        d = get_surface(draw);
        impl_draw = d->surface;
    }

    // retrieve the underlying implementation's read EGLSurface
    if (read != EGL_NO_SURFACE) {
        if (!_r.get()) return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);
        r = get_surface(read);
        impl_read = r->surface;
    }


    EGLBoolean result = dp->makeCurrent(c, cur_c,
            draw, read, ctx,
            impl_draw, impl_read, impl_ctx);

    if (result == EGL_TRUE) {
        if (c) {
            setGLHooksThreadSpecific(c->cnx->hooks[c->version]);
            egl_tls_t::setContext(ctx);
            _c.acquire();
            _r.acquire();
            _d.acquire();
        } else {
            setGLHooksThreadSpecific(&gHooksNoContext);
            egl_tls_t::setContext(EGL_NO_CONTEXT);
        }
    } else {
        // this will ALOGE the error
        egl_connection_t* const cnx = &gEGLImpl;
        result = setError(cnx->egl.eglGetError(), (EGLBoolean)EGL_FALSE);
    }
    return result;
}

EGLBoolean eglQueryContextImpl( EGLDisplay dpy, EGLContext ctx,
                                EGLint attribute, EGLint *value)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    ContextRef _c(dp.get(), ctx);
    if (!_c.get()) return setError(EGL_BAD_CONTEXT, (EGLBoolean)EGL_FALSE);

    egl_context_t * const c = get_context(ctx);
    return c->cnx->egl.eglQueryContext(
            dp->disp.dpy, c->context, attribute, value);

}

EGLContext eglGetCurrentContextImpl(void)
{
    // could be called before eglInitialize(), but we wouldn't have a context
    // then, and this function would correctly return EGL_NO_CONTEXT.
    EGLContext ctx = getContext();
    return ctx;
}

EGLSurface eglGetCurrentSurfaceImpl(EGLint readdraw)
{
    // could be called before eglInitialize(), but we wouldn't have a context
    // then, and this function would correctly return EGL_NO_SURFACE.

    EGLContext ctx = getContext();
    if (ctx) {
        egl_context_t const * const c = get_context(ctx);
        if (!c) return setError(EGL_BAD_CONTEXT, EGL_NO_SURFACE);
        switch (readdraw) {
            case EGL_READ: return c->read;
            case EGL_DRAW: return c->draw;
            default: return setError(EGL_BAD_PARAMETER, EGL_NO_SURFACE);
        }
    }
    return EGL_NO_SURFACE;
}

EGLDisplay eglGetCurrentDisplayImpl(void)
{
    // could be called before eglInitialize(), but we wouldn't have a context
    // then, and this function would correctly return EGL_NO_DISPLAY.

    EGLContext ctx = getContext();
    if (ctx) {
        egl_context_t const * const c = get_context(ctx);
        if (!c) return setError(EGL_BAD_CONTEXT, EGL_NO_SURFACE);
        return c->dpy;
    }
    return EGL_NO_DISPLAY;
}

EGLBoolean eglWaitGLImpl(void)
{
    egl_connection_t* const cnx = &gEGLImpl;
    if (!cnx->dso)
        return setError(EGL_BAD_CONTEXT, (EGLBoolean)EGL_FALSE);

    return cnx->egl.eglWaitGL();
}

EGLBoolean eglWaitNativeImpl(EGLint engine)
{
    egl_connection_t* const cnx = &gEGLImpl;
    if (!cnx->dso)
        return setError(EGL_BAD_CONTEXT, (EGLBoolean)EGL_FALSE);

    return cnx->egl.eglWaitNative(engine);
}

EGLint eglGetErrorImpl(void)
{
    EGLint err = EGL_SUCCESS;
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso) {
        err = cnx->egl.eglGetError();
    }
    if (err == EGL_SUCCESS) {
        err = egl_tls_t::getError();
    }
    return err;
}

static __eglMustCastToProperFunctionPointerType findBuiltinWrapper(
        const char* procname) {
    const egl_connection_t* cnx = &gEGLImpl;
    void* proc = nullptr;

    proc = dlsym(cnx->libEgl, procname);
    if (proc) return (__eglMustCastToProperFunctionPointerType)proc;

    proc = dlsym(cnx->libGles2, procname);
    if (proc) return (__eglMustCastToProperFunctionPointerType)proc;

    proc = dlsym(cnx->libGles1, procname);
    if (proc) return (__eglMustCastToProperFunctionPointerType)proc;

    return nullptr;
}

__eglMustCastToProperFunctionPointerType eglGetProcAddressImpl(const char *procname)
{
    if (FILTER_EXTENSIONS(procname)) {
        return nullptr;
    }

    __eglMustCastToProperFunctionPointerType addr;
    addr = findProcAddress(procname, sExtensionMap, NELEM(sExtensionMap));
    if (addr) return addr;

    addr = findBuiltinWrapper(procname);
    if (addr) return addr;

    // this protects accesses to sGLExtensionMap, sGLExtensionSlot, and sGLExtensionSlotMap
    pthread_mutex_lock(&sExtensionMapMutex);

    /*
     * Since eglGetProcAddress() is not associated to anything, it needs
     * to return a function pointer that "works" regardless of what
     * the current context is.
     *
     * For this reason, we return a "forwarder", a small stub that takes
     * care of calling the function associated with the context
     * currently bound.
     *
     * We first look for extensions we've already resolved, if we're seeing
     * this extension for the first time, we go through all our
     * implementations and call eglGetProcAddress() and record the
     * result in the appropriate implementation hooks and return the
     * address of the forwarder corresponding to that hook set.
     *
     */

    const std::string name(procname);
    auto& extensionMap = sGLExtensionMap;
    auto& extensionSlotMap = sGLExtensionSlotMap;
    egl_connection_t* const cnx = &gEGLImpl;
    LayerLoader& layer_loader(LayerLoader::getInstance());

    // See if we've already looked up this extension
    auto pos = extensionMap.find(name);
    addr = (pos != extensionMap.end()) ? pos->second : nullptr;

    if (!addr) {
        // This is the first time we've looked this function up
        // Ensure we have room to track it
        const int slot = sGLExtensionSlot;
        if (slot < MAX_NUMBER_OF_GL_EXTENSIONS) {

            if (cnx->dso && cnx->egl.eglGetProcAddress) {

                // Extensions are independent of the bound context
                addr = cnx->egl.eglGetProcAddress(procname);
                if (addr) {

                    // purposefully track the bottom of the stack in extensionMap
                    extensionMap[name] = addr;

                    // Apply layers
                    addr = layer_loader.ApplyLayers(procname, addr);

                    // Track the top most entry point return the extension forwarder
                    cnx->hooks[egl_connection_t::GLESv1_INDEX]->ext.extensions[slot] =
                    cnx->hooks[egl_connection_t::GLESv2_INDEX]->ext.extensions[slot] = addr;
                    addr = gExtensionForwarders[slot];

                    // Remember the slot for this extension
                    extensionSlotMap[name] = slot;

                    // Increment the global extension index
                    sGLExtensionSlot++;
                }
            }
        } else {
            // The extension forwarder has a fixed number of slots
            ALOGE("no more slots for eglGetProcAddress(\"%s\")", procname);
        }

    } else {
        // We tracked an address, so we've seen this func before
        // Look up the slot for this extension
        auto slot_pos = extensionSlotMap.find(name);
        int ext_slot = (slot_pos != extensionSlotMap.end()) ? slot_pos->second : -1;
        if (ext_slot < 0) {
            // Something has gone wrong, this should not happen
            ALOGE("No extension slot found for %s", procname);
            return nullptr;
        }

        // We tracked the bottom of the stack, so re-apply layers since
        // more layers might have been enabled
        addr = layer_loader.ApplyLayers(procname, addr);

        // Track the top most entry point and return the extension forwarder
        cnx->hooks[egl_connection_t::GLESv1_INDEX]->ext.extensions[ext_slot] =
        cnx->hooks[egl_connection_t::GLESv2_INDEX]->ext.extensions[ext_slot] = addr;
        addr = gExtensionForwarders[ext_slot];
    }

    pthread_mutex_unlock(&sExtensionMapMutex);
    return addr;
}

class FrameCompletionThread {
public:

    static void queueSync(EGLSyncKHR sync) {
        static FrameCompletionThread thread;

        char name[64];

        std::lock_guard<std::mutex> lock(thread.mMutex);
        snprintf(name, sizeof(name), "kicked off frame %u", (unsigned int)thread.mFramesQueued);
        ATRACE_NAME(name);

        thread.mQueue.push_back(sync);
        thread.mCondition.notify_one();
        thread.mFramesQueued++;
        ATRACE_INT("GPU Frames Outstanding", int32_t(thread.mQueue.size()));
    }

private:

    FrameCompletionThread() : mFramesQueued(0), mFramesCompleted(0) {
        std::thread thread(&FrameCompletionThread::loop, this);
        thread.detach();
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    void loop() {
        while (true) {
            threadLoop();
        }
    }
#pragma clang diagnostic pop

    void threadLoop() {
        EGLSyncKHR sync;
        uint32_t frameNum;
        {
            std::unique_lock<std::mutex> lock(mMutex);
            while (mQueue.empty()) {
                mCondition.wait(lock);
            }
            sync = mQueue[0];
            frameNum = mFramesCompleted;
        }
        EGLDisplay dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        {
            char name[64];
            snprintf(name, sizeof(name), "waiting for frame %u", (unsigned int)frameNum);
            ATRACE_NAME(name);

            EGLint result = eglClientWaitSyncKHR(dpy, sync, 0, EGL_FOREVER_KHR);
            if (result == EGL_FALSE) {
                ALOGE("FrameCompletion: error waiting for fence: %#x", eglGetError());
            } else if (result == EGL_TIMEOUT_EXPIRED_KHR) {
                ALOGE("FrameCompletion: timeout waiting for fence");
            }
            eglDestroySyncKHR(dpy, sync);
        }
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mQueue.pop_front();
            mFramesCompleted++;
            ATRACE_INT("GPU Frames Outstanding", int32_t(mQueue.size()));
        }
    }

    uint32_t mFramesQueued;
    uint32_t mFramesCompleted;
    std::deque<EGLSyncKHR> mQueue;
    std::condition_variable mCondition;
    std::mutex mMutex;
};

EGLBoolean eglSwapBuffersWithDamageKHRImpl(EGLDisplay dpy, EGLSurface draw,
        EGLint *rects, EGLint n_rects)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    SurfaceRef _s(dp.get(), draw);
    if (!_s.get())
        return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);

    egl_surface_t* const s = get_surface(draw);

    if (CC_UNLIKELY(dp->traceGpuCompletion)) {
        EGLSyncKHR sync = eglCreateSyncKHR(dpy, EGL_SYNC_FENCE_KHR, nullptr);
        if (sync != EGL_NO_SYNC_KHR) {
            FrameCompletionThread::queueSync(sync);
        }
    }

    if (CC_UNLIKELY(dp->finishOnSwap)) {
        uint32_t pixel;
        egl_context_t * const c = get_context( egl_tls_t::getContext() );
        if (c) {
            // glReadPixels() ensures that the frame is complete
            s->cnx->hooks[c->version]->gl.glReadPixels(0,0,1,1,
                    GL_RGBA,GL_UNSIGNED_BYTE,&pixel);
        }
    }

    if (s->cnx->angleBackend != EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE) {
        if (!sendSurfaceMetadata(s)) {
            native_window_api_disconnect(s->getNativeWindow(), NATIVE_WINDOW_API_EGL);
            return setError(EGL_BAD_NATIVE_WINDOW, (EGLBoolean)EGL_FALSE);
        }
    }

    if (n_rects == 0) {
        return s->cnx->egl.eglSwapBuffers(dp->disp.dpy, s->surface);
    }

    std::vector<android_native_rect_t> androidRects((size_t)n_rects);
    for (int r = 0; r < n_rects; ++r) {
        int offset = r * 4;
        int x = rects[offset];
        int y = rects[offset + 1];
        int width = rects[offset + 2];
        int height = rects[offset + 3];
        android_native_rect_t androidRect;
        androidRect.left = x;
        androidRect.top = y + height;
        androidRect.right = x + width;
        androidRect.bottom = y;
        androidRects.push_back(androidRect);
    }
    if (s->cnx->angleBackend != EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE) {
        native_window_set_surface_damage(s->getNativeWindow(), androidRects.data(),
                                         androidRects.size());
    }

    if (s->cnx->egl.eglSwapBuffersWithDamageKHR) {
        return s->cnx->egl.eglSwapBuffersWithDamageKHR(dp->disp.dpy, s->surface,
                rects, n_rects);
    } else {
        return s->cnx->egl.eglSwapBuffers(dp->disp.dpy, s->surface);
    }
}

EGLBoolean eglSwapBuffersImpl(EGLDisplay dpy, EGLSurface surface)
{
    return eglSwapBuffersWithDamageKHRImpl(dpy, surface, nullptr, 0);
}

EGLBoolean eglCopyBuffersImpl(  EGLDisplay dpy, EGLSurface surface,
                                NativePixmapType target)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    SurfaceRef _s(dp.get(), surface);
    if (!_s.get())
        return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);

    egl_surface_t const * const s = get_surface(surface);
    return s->cnx->egl.eglCopyBuffers(dp->disp.dpy, s->surface, target);
}

const char* eglQueryStringImpl(EGLDisplay dpy, EGLint name)
{
    if (dpy == EGL_NO_DISPLAY && name == EGL_EXTENSIONS) {
        // Return list of client extensions
        return gClientExtensionString;
    }

    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return (const char *) nullptr;

    switch (name) {
        case EGL_VENDOR:
            return dp->getVendorString();
        case EGL_VERSION:
            return dp->getVersionString();
        case EGL_EXTENSIONS:
            return dp->getExtensionString();
        case EGL_CLIENT_APIS:
            return dp->getClientApiString();
        default:
            break;
    }
    return setError(EGL_BAD_PARAMETER, (const char *)nullptr);
}

EGLAPI const char* eglQueryStringImplementationANDROIDImpl(EGLDisplay dpy, EGLint name)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return (const char *) nullptr;

    switch (name) {
        case EGL_VENDOR:
            return dp->disp.queryString.vendor;
        case EGL_VERSION:
            return dp->disp.queryString.version;
        case EGL_EXTENSIONS:
            return dp->disp.queryString.extensions;
        case EGL_CLIENT_APIS:
            return dp->disp.queryString.clientApi;
        default:
            break;
    }
    return setError(EGL_BAD_PARAMETER, (const char *)nullptr);
}

// ----------------------------------------------------------------------------
// EGL 1.1
// ----------------------------------------------------------------------------

EGLBoolean eglSurfaceAttribImpl(
        EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    SurfaceRef _s(dp.get(), surface);
    if (!_s.get())
        return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);

    egl_surface_t * const s = get_surface(surface);

    if (attribute == EGL_FRONT_BUFFER_AUTO_REFRESH_ANDROID) {
        if (!s->getNativeWindow()) {
            setError(EGL_BAD_SURFACE, EGL_FALSE);
        }
        int err = native_window_set_auto_refresh(s->getNativeWindow(), value != 0);
        return (err == 0) ? EGL_TRUE : setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);
    }

    if (attribute == EGL_TIMESTAMPS_ANDROID) {
        if (!s->getNativeWindow()) {
            return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);
        }
        int err = native_window_enable_frame_timestamps(s->getNativeWindow(), value != 0);
        return (err == 0) ? EGL_TRUE : setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);
    }

    if (s->setSmpte2086Attribute(attribute, value)) {
        return EGL_TRUE;
    } else if (s->setCta8613Attribute(attribute, value)) {
        return EGL_TRUE;
    } else if (s->cnx->egl.eglSurfaceAttrib) {
        return s->cnx->egl.eglSurfaceAttrib(
                dp->disp.dpy, s->surface, attribute, value);
    }
    return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);
}

EGLBoolean eglBindTexImageImpl(
        EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    SurfaceRef _s(dp.get(), surface);
    if (!_s.get())
        return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);

    egl_surface_t const * const s = get_surface(surface);
    if (s->cnx->egl.eglBindTexImage) {
        return s->cnx->egl.eglBindTexImage(
                dp->disp.dpy, s->surface, buffer);
    }
    return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);
}

EGLBoolean eglReleaseTexImageImpl(
        EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    SurfaceRef _s(dp.get(), surface);
    if (!_s.get())
        return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);

    egl_surface_t const * const s = get_surface(surface);
    if (s->cnx->egl.eglReleaseTexImage) {
        return s->cnx->egl.eglReleaseTexImage(
                dp->disp.dpy, s->surface, buffer);
    }
    return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);
}

EGLBoolean eglSwapIntervalImpl(EGLDisplay dpy, EGLint interval)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    EGLBoolean res = EGL_TRUE;
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && cnx->egl.eglSwapInterval) {
        res = cnx->egl.eglSwapInterval(dp->disp.dpy, interval);
    }

    return res;
}


// ----------------------------------------------------------------------------
// EGL 1.2
// ----------------------------------------------------------------------------

EGLBoolean eglWaitClientImpl(void)
{
    egl_connection_t* const cnx = &gEGLImpl;
    if (!cnx->dso)
        return setError(EGL_BAD_CONTEXT, (EGLBoolean)EGL_FALSE);

    EGLBoolean res;
    if (cnx->egl.eglWaitClient) {
        res = cnx->egl.eglWaitClient();
    } else {
        res = cnx->egl.eglWaitGL();
    }
    return res;
}

EGLBoolean eglBindAPIImpl(EGLenum api)
{
    // bind this API on all EGLs
    EGLBoolean res = EGL_TRUE;
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && cnx->egl.eglBindAPI) {
        res = cnx->egl.eglBindAPI(api);
    }
    return res;
}

EGLenum eglQueryAPIImpl(void)
{
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && cnx->egl.eglQueryAPI) {
        return cnx->egl.eglQueryAPI();
    }

    // or, it can only be OpenGL ES
    return EGL_OPENGL_ES_API;
}

EGLBoolean eglReleaseThreadImpl(void)
{
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && cnx->egl.eglReleaseThread) {
        cnx->egl.eglReleaseThread();
    }

    // If there is context bound to the thread, release it
    egl_display_t::loseCurrent(get_context(getContext()));

    egl_tls_t::clearTLS();
    return EGL_TRUE;
}

EGLSurface eglCreatePbufferFromClientBufferImpl(
          EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer,
          EGLConfig config, const EGLint *attrib_list)
{
    egl_connection_t* cnx = nullptr;
    const egl_display_ptr dp = validate_display_connection(dpy, cnx);
    if (!dp) return EGL_FALSE;
    if (cnx->egl.eglCreatePbufferFromClientBuffer) {
        return cnx->egl.eglCreatePbufferFromClientBuffer(
                dp->disp.dpy, buftype, buffer, config, attrib_list);
    }
    return setError(EGL_BAD_CONFIG, EGL_NO_SURFACE);
}

// ----------------------------------------------------------------------------
// EGL_EGLEXT_VERSION 3
// ----------------------------------------------------------------------------

EGLBoolean eglLockSurfaceKHRImpl(EGLDisplay dpy, EGLSurface surface,
        const EGLint *attrib_list)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    SurfaceRef _s(dp.get(), surface);
    if (!_s.get())
        return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);

    egl_surface_t const * const s = get_surface(surface);
    if (s->cnx->egl.eglLockSurfaceKHR) {
        return s->cnx->egl.eglLockSurfaceKHR(
                dp->disp.dpy, s->surface, attrib_list);
    }
    return setError(EGL_BAD_DISPLAY, (EGLBoolean)EGL_FALSE);
}

EGLBoolean eglUnlockSurfaceKHRImpl(EGLDisplay dpy, EGLSurface surface)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    SurfaceRef _s(dp.get(), surface);
    if (!_s.get())
        return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);

    egl_surface_t const * const s = get_surface(surface);
    if (s->cnx->egl.eglUnlockSurfaceKHR) {
        return s->cnx->egl.eglUnlockSurfaceKHR(dp->disp.dpy, s->surface);
    }
    return setError(EGL_BAD_DISPLAY, (EGLBoolean)EGL_FALSE);
}

// Note: EGLImageKHR and EGLImage are the same thing so no need
// to templatize that.
template <typename AttrType, typename FuncType>
EGLImageKHR eglCreateImageTmpl(EGLDisplay dpy, EGLContext ctx, EGLenum target,
                               EGLClientBuffer buffer, const AttrType* attrib_list,
                               FuncType eglCreateImageFunc) {
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_NO_IMAGE_KHR;

    std::vector<AttrType> strippedAttribs;
    if (needsAndroidPEglMitigation()) {
        // Mitigation for Android P vendor partitions: eglImageCreateKHR should accept
        // EGL_GL_COLORSPACE_LINEAR_KHR, EGL_GL_COLORSPACE_SRGB_KHR and
        // EGL_GL_COLORSPACE_DEFAULT_EXT if EGL_EXT_image_gl_colorspace is supported,
        // but some drivers don't like the DEFAULT value and generate an error.
        for (const AttrType *attr = attrib_list; attr && attr[0] != EGL_NONE; attr += 2) {
            if (attr[0] == EGL_GL_COLORSPACE_KHR &&
                dp->haveExtension("EGL_EXT_image_gl_colorspace")) {
                if (attr[1] != EGL_GL_COLORSPACE_LINEAR_KHR &&
                    attr[1] != EGL_GL_COLORSPACE_SRGB_KHR) {
                    continue;
                }
            }
            strippedAttribs.push_back(attr[0]);
            strippedAttribs.push_back(attr[1]);
        }
        strippedAttribs.push_back(EGL_NONE);
    }

    ContextRef _c(dp.get(), ctx);
    egl_context_t* const c = _c.get();

    EGLImageKHR result = EGL_NO_IMAGE_KHR;
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && eglCreateImageFunc) {
        result = eglCreateImageFunc(dp->disp.dpy, c ? c->context : EGL_NO_CONTEXT, target, buffer,
                                    needsAndroidPEglMitigation() ? strippedAttribs.data() : attrib_list);
    }
    return result;
}

typedef EGLImage(EGLAPIENTRYP PFNEGLCREATEIMAGE)(EGLDisplay dpy, EGLContext ctx, EGLenum target,
                                                 EGLClientBuffer buffer,
                                                 const EGLAttrib* attrib_list);

EGLImageKHR eglCreateImageKHRImpl(EGLDisplay dpy, EGLContext ctx, EGLenum target,
                                  EGLClientBuffer buffer, const EGLint* attrib_list) {
    return eglCreateImageTmpl<EGLint, PFNEGLCREATEIMAGEKHRPROC>(dpy, ctx, target, buffer,
                                                                attrib_list,
                                                                gEGLImpl.egl.eglCreateImageKHR);
}

EGLImage eglCreateImageImpl(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer,
                            const EGLAttrib* attrib_list) {
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->driverVersion >= EGL_MAKE_VERSION(1, 5, 0)) {
        if (cnx->egl.eglCreateImage) {
            return eglCreateImageTmpl<EGLAttrib, PFNEGLCREATEIMAGE>(dpy, ctx, target, buffer,
                                                                    attrib_list,
                                                                    cnx->egl.eglCreateImage);
        }
        // driver doesn't support native function, return EGL_BAD_DISPLAY
        ALOGE("Driver indicates EGL 1.5 support, but does not have eglCreateImage");
        return setError(EGL_BAD_DISPLAY, EGL_NO_IMAGE);
    }

    std::vector<EGLint> convertedAttribs;
    convertAttribs(attrib_list, convertedAttribs);
    return eglCreateImageTmpl<EGLint, PFNEGLCREATEIMAGEKHRPROC>(dpy, ctx, target, buffer,
                                                                convertedAttribs.data(),
                                                                gEGLImpl.egl.eglCreateImageKHR);
}

EGLBoolean eglDestroyImageTmpl(EGLDisplay dpy, EGLImageKHR img,
                               PFNEGLDESTROYIMAGEKHRPROC destroyImageFunc) {
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    EGLBoolean result = EGL_FALSE;
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && destroyImageFunc) {
        result = destroyImageFunc(dp->disp.dpy, img);
    }
    return result;
}

EGLBoolean eglDestroyImageKHRImpl(EGLDisplay dpy, EGLImageKHR img) {
    return eglDestroyImageTmpl(dpy, img, gEGLImpl.egl.eglDestroyImageKHR);
}

EGLBoolean eglDestroyImageImpl(EGLDisplay dpy, EGLImageKHR img) {
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->driverVersion >= EGL_MAKE_VERSION(1, 5, 0)) {
        if (cnx->egl.eglDestroyImage) {
            return eglDestroyImageTmpl(dpy, img, gEGLImpl.egl.eglDestroyImage);
        }
        // driver doesn't support native function, return EGL_BAD_DISPLAY
        ALOGE("Driver indicates EGL 1.5 support, but does not have eglDestroyImage");
        return setError(EGL_BAD_DISPLAY, EGL_FALSE);
    }

    return eglDestroyImageTmpl(dpy, img, gEGLImpl.egl.eglDestroyImageKHR);
}

// ----------------------------------------------------------------------------
// EGL_EGLEXT_VERSION 5
// ----------------------------------------------------------------------------

// NOTE: EGLSyncKHR and EGLSync are identical, no need to templatize
template <typename AttrType, typename FuncType>
EGLSyncKHR eglCreateSyncTmpl(EGLDisplay dpy, EGLenum type, const AttrType* attrib_list,
                             FuncType eglCreateSyncFunc) {
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_NO_SYNC_KHR;

    egl_connection_t* const cnx = &gEGLImpl;
    EGLSyncKHR result = EGL_NO_SYNC_KHR;
    if (cnx->dso && eglCreateSyncFunc) {
        result = eglCreateSyncFunc(dp->disp.dpy, type, attrib_list);
    }
    return result;
}

typedef EGLSurface(EGLAPIENTRYP PFNEGLCREATESYNC)(EGLDisplay dpy, EGLenum type,
                                                  const EGLAttrib* attrib_list);

EGLSyncKHR eglCreateSyncKHRImpl(EGLDisplay dpy, EGLenum type, const EGLint* attrib_list) {
    return eglCreateSyncTmpl<EGLint, PFNEGLCREATESYNCKHRPROC>(dpy, type, attrib_list,
                                                              gEGLImpl.egl.eglCreateSyncKHR);
}

EGLSync eglCreateSyncImpl(EGLDisplay dpy, EGLenum type, const EGLAttrib* attrib_list) {
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->driverVersion >= EGL_MAKE_VERSION(1, 5, 0)) {
        if (cnx->egl.eglCreateSync) {
            return eglCreateSyncTmpl<EGLAttrib, PFNEGLCREATESYNC>(dpy, type, attrib_list,
                                                                  cnx->egl.eglCreateSync);
        }
        // driver doesn't support native function, return EGL_BAD_DISPLAY
        ALOGE("Driver indicates EGL 1.5 support, but does not have eglCreateSync");
        return setError(EGL_BAD_DISPLAY, EGL_NO_SYNC);
    }

    std::vector<EGLint> convertedAttribs;
    convertAttribs(attrib_list, convertedAttribs);
    return eglCreateSyncTmpl<EGLint, PFNEGLCREATESYNCKHRPROC>(dpy, type, convertedAttribs.data(),
                                                              cnx->egl.eglCreateSyncKHR);
}

EGLBoolean eglDestroySyncTmpl(EGLDisplay dpy, EGLSyncKHR sync,
                              PFNEGLDESTROYSYNCKHRPROC eglDestroySyncFunc) {
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    EGLBoolean result = EGL_FALSE;
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && eglDestroySyncFunc) {
        result = eglDestroySyncFunc(dp->disp.dpy, sync);
    }
    return result;
}

EGLBoolean eglDestroySyncKHRImpl(EGLDisplay dpy, EGLSyncKHR sync) {
    return eglDestroySyncTmpl(dpy, sync, gEGLImpl.egl.eglDestroySyncKHR);
}

EGLBoolean eglDestroySyncImpl(EGLDisplay dpy, EGLSyncKHR sync) {
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->driverVersion >= EGL_MAKE_VERSION(1, 5, 0)) {
        if (cnx->egl.eglDestroySync) {
            return eglDestroySyncTmpl(dpy, sync, cnx->egl.eglDestroySync);
        }
        ALOGE("Driver indicates EGL 1.5 support, but does not have eglDestroySync");
        return setError(EGL_BAD_DISPLAY, EGL_FALSE);
    }

    return eglDestroySyncTmpl(dpy, sync, cnx->egl.eglDestroySyncKHR);
}

EGLBoolean eglSignalSyncKHRImpl(EGLDisplay dpy, EGLSyncKHR sync, EGLenum mode) {
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    EGLBoolean result = EGL_FALSE;
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && gEGLImpl.egl.eglSignalSyncKHR) {
        result = gEGLImpl.egl.eglSignalSyncKHR(dp->disp.dpy, sync, mode);
    }
    return result;
}

EGLint eglClientWaitSyncTmpl(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout,
                             PFNEGLCLIENTWAITSYNCKHRPROC eglClientWaitSyncFunc) {
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    EGLint result = EGL_FALSE;
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && eglClientWaitSyncFunc) {
        result = eglClientWaitSyncFunc(dp->disp.dpy, sync, flags, timeout);
    }
    return result;
}

EGLint eglClientWaitSyncKHRImpl(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout) {
    egl_connection_t* const cnx = &gEGLImpl;
    return eglClientWaitSyncTmpl(dpy, sync, flags, timeout, cnx->egl.eglClientWaitSyncKHR);
}

EGLint eglClientWaitSyncImpl(EGLDisplay dpy, EGLSync sync, EGLint flags, EGLTimeKHR timeout) {
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->driverVersion >= EGL_MAKE_VERSION(1, 5, 0)) {
        if (cnx->egl.eglClientWaitSync) {
            return eglClientWaitSyncTmpl(dpy, sync, flags, timeout, cnx->egl.eglClientWaitSync);
        }
        ALOGE("Driver indicates EGL 1.5 support, but does not have eglClientWaitSync");
        return setError(EGL_BAD_DISPLAY, (EGLint)EGL_FALSE);
    }

    return eglClientWaitSyncTmpl(dpy, sync, flags, timeout, cnx->egl.eglClientWaitSyncKHR);
}

template <typename AttrType, typename FuncType>
EGLBoolean eglGetSyncAttribTmpl(EGLDisplay dpy, EGLSyncKHR sync, EGLint attribute, AttrType* value,
                                FuncType eglGetSyncAttribFunc) {
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    EGLBoolean result = EGL_FALSE;
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && eglGetSyncAttribFunc) {
        result = eglGetSyncAttribFunc(dp->disp.dpy, sync, attribute, value);
    }
    return result;
}

typedef EGLBoolean(EGLAPIENTRYP PFNEGLGETSYNCATTRIB)(EGLDisplay dpy, EGLSync sync, EGLint attribute,
                                                     EGLAttrib* value);

EGLBoolean eglGetSyncAttribImpl(EGLDisplay dpy, EGLSync sync, EGLint attribute, EGLAttrib* value) {
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->driverVersion >= EGL_MAKE_VERSION(1, 5, 0)) {
        if (cnx->egl.eglGetSyncAttrib) {
            return eglGetSyncAttribTmpl<EGLAttrib, PFNEGLGETSYNCATTRIB>(dpy, sync, attribute, value,
                                                                        cnx->egl.eglGetSyncAttrib);
        }
        ALOGE("Driver indicates EGL 1.5 support, but does not have eglGetSyncAttrib");
        return setError(EGL_BAD_DISPLAY, (EGLint)EGL_FALSE);
    }

    // Fallback to KHR, ask for EGLint attribute and cast back to EGLAttrib
    EGLint attribValue;
    EGLBoolean ret =
            eglGetSyncAttribTmpl<EGLint, PFNEGLGETSYNCATTRIBKHRPROC>(dpy, sync, attribute,
                                                                     &attribValue,
                                                                     gEGLImpl.egl
                                                                             .eglGetSyncAttribKHR);
    if (ret) {
        *value = static_cast<EGLAttrib>(attribValue);
    }
    return ret;
}

EGLBoolean eglGetSyncAttribKHRImpl(EGLDisplay dpy, EGLSyncKHR sync, EGLint attribute,
                                   EGLint* value) {
    return eglGetSyncAttribTmpl<EGLint, PFNEGLGETSYNCATTRIBKHRPROC>(dpy, sync, attribute, value,
                                                                    gEGLImpl.egl
                                                                            .eglGetSyncAttribKHR);
}

EGLStreamKHR eglCreateStreamKHRImpl(EGLDisplay dpy, const EGLint *attrib_list)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_NO_STREAM_KHR;

    EGLStreamKHR result = EGL_NO_STREAM_KHR;
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && cnx->egl.eglCreateStreamKHR) {
        result = cnx->egl.eglCreateStreamKHR(
                dp->disp.dpy, attrib_list);
    }
    return result;
}

EGLBoolean eglDestroyStreamKHRImpl(EGLDisplay dpy, EGLStreamKHR stream)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    EGLBoolean result = EGL_FALSE;
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && cnx->egl.eglDestroyStreamKHR) {
        result = cnx->egl.eglDestroyStreamKHR(
                dp->disp.dpy, stream);
    }
    return result;
}

EGLBoolean eglStreamAttribKHRImpl(EGLDisplay dpy, EGLStreamKHR stream,
        EGLenum attribute, EGLint value)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    EGLBoolean result = EGL_FALSE;
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && cnx->egl.eglStreamAttribKHR) {
        result = cnx->egl.eglStreamAttribKHR(
                dp->disp.dpy, stream, attribute, value);
    }
    return result;
}

EGLBoolean eglQueryStreamKHRImpl(EGLDisplay dpy, EGLStreamKHR stream,
        EGLenum attribute, EGLint *value)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    EGLBoolean result = EGL_FALSE;
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && cnx->egl.eglQueryStreamKHR) {
        result = cnx->egl.eglQueryStreamKHR(
                dp->disp.dpy, stream, attribute, value);
    }
    return result;
}

EGLBoolean eglQueryStreamu64KHRImpl(EGLDisplay dpy, EGLStreamKHR stream,
        EGLenum attribute, EGLuint64KHR *value)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    EGLBoolean result = EGL_FALSE;
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && cnx->egl.eglQueryStreamu64KHR) {
        result = cnx->egl.eglQueryStreamu64KHR(
                dp->disp.dpy, stream, attribute, value);
    }
    return result;
}

EGLBoolean eglQueryStreamTimeKHRImpl(EGLDisplay dpy, EGLStreamKHR stream,
        EGLenum attribute, EGLTimeKHR *value)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    EGLBoolean result = EGL_FALSE;
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && cnx->egl.eglQueryStreamTimeKHR) {
        result = cnx->egl.eglQueryStreamTimeKHR(
                dp->disp.dpy, stream, attribute, value);
    }
    return result;
}

EGLSurface eglCreateStreamProducerSurfaceKHRImpl(EGLDisplay dpy, EGLConfig config,
        EGLStreamKHR stream, const EGLint *attrib_list)
{
    egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_NO_SURFACE;

    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && cnx->egl.eglCreateStreamProducerSurfaceKHR) {
        EGLSurface surface = cnx->egl.eglCreateStreamProducerSurfaceKHR(
                dp->disp.dpy, config, stream, attrib_list);
        if (surface != EGL_NO_SURFACE) {
            egl_surface_t* s = new egl_surface_t(dp.get(), config, nullptr, surface,
                                                 EGL_GL_COLORSPACE_LINEAR_KHR, cnx);
            return s;
        }
    }
    return EGL_NO_SURFACE;
}

EGLBoolean eglStreamConsumerGLTextureExternalKHRImpl(EGLDisplay dpy,
        EGLStreamKHR stream)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    EGLBoolean result = EGL_FALSE;
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && cnx->egl.eglStreamConsumerGLTextureExternalKHR) {
        result = cnx->egl.eglStreamConsumerGLTextureExternalKHR(
                dp->disp.dpy, stream);
    }
    return result;
}

EGLBoolean eglStreamConsumerAcquireKHRImpl(EGLDisplay dpy,
        EGLStreamKHR stream)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    EGLBoolean result = EGL_FALSE;
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && cnx->egl.eglStreamConsumerAcquireKHR) {
        result = cnx->egl.eglStreamConsumerAcquireKHR(
                dp->disp.dpy, stream);
    }
    return result;
}

EGLBoolean eglStreamConsumerReleaseKHRImpl(EGLDisplay dpy,
        EGLStreamKHR stream)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;

    EGLBoolean result = EGL_FALSE;
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && cnx->egl.eglStreamConsumerReleaseKHR) {
        result = cnx->egl.eglStreamConsumerReleaseKHR(
                dp->disp.dpy, stream);
    }
    return result;
}

EGLNativeFileDescriptorKHR eglGetStreamFileDescriptorKHRImpl(
        EGLDisplay dpy, EGLStreamKHR stream)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_NO_FILE_DESCRIPTOR_KHR;

    EGLNativeFileDescriptorKHR result = EGL_NO_FILE_DESCRIPTOR_KHR;
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && cnx->egl.eglGetStreamFileDescriptorKHR) {
        result = cnx->egl.eglGetStreamFileDescriptorKHR(
                dp->disp.dpy, stream);
    }
    return result;
}

EGLStreamKHR eglCreateStreamFromFileDescriptorKHRImpl(
        EGLDisplay dpy, EGLNativeFileDescriptorKHR file_descriptor)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_NO_STREAM_KHR;

    EGLStreamKHR result = EGL_NO_STREAM_KHR;
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && cnx->egl.eglCreateStreamFromFileDescriptorKHR) {
        result = cnx->egl.eglCreateStreamFromFileDescriptorKHR(
                dp->disp.dpy, file_descriptor);
    }
    return result;
}

// ----------------------------------------------------------------------------
// EGL_EGLEXT_VERSION 15
// ----------------------------------------------------------------------------

// Need to template function type because return type is different
template <typename ReturnType, typename FuncType>
ReturnType eglWaitSyncTmpl(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags,
                           FuncType eglWaitSyncFunc) {
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_FALSE;
    ReturnType result = EGL_FALSE;
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && eglWaitSyncFunc) {
        result = eglWaitSyncFunc(dp->disp.dpy, sync, flags);
    }
    return result;
}

typedef EGLBoolean(EGLAPIENTRYP PFNEGLWAITSYNC)(EGLDisplay dpy, EGLSync sync, EGLint flags);

EGLint eglWaitSyncKHRImpl(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags) {
    egl_connection_t* const cnx = &gEGLImpl;
    return eglWaitSyncTmpl<EGLint, PFNEGLWAITSYNCKHRPROC>(dpy, sync, flags,
                                                          cnx->egl.eglWaitSyncKHR);
}

EGLBoolean eglWaitSyncImpl(EGLDisplay dpy, EGLSync sync, EGLint flags) {
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->driverVersion >= EGL_MAKE_VERSION(1, 5, 0)) {
        if (cnx->egl.eglWaitSync) {
            return eglWaitSyncTmpl<EGLBoolean, PFNEGLWAITSYNC>(dpy, sync, flags,
                                                               cnx->egl.eglWaitSync);
        }
        return setError(EGL_BAD_DISPLAY, (EGLint)EGL_FALSE);
    }

    return static_cast<EGLBoolean>(
            eglWaitSyncTmpl<EGLint, PFNEGLWAITSYNCKHRPROC>(dpy, sync, flags,
                                                           cnx->egl.eglWaitSyncKHR));
}

// ----------------------------------------------------------------------------
// ANDROID extensions
// ----------------------------------------------------------------------------

EGLint eglDupNativeFenceFDANDROIDImpl(EGLDisplay dpy, EGLSyncKHR sync)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) return EGL_NO_NATIVE_FENCE_FD_ANDROID;

    EGLint result = EGL_NO_NATIVE_FENCE_FD_ANDROID;
    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && cnx->egl.eglDupNativeFenceFDANDROID) {
        result = cnx->egl.eglDupNativeFenceFDANDROID(dp->disp.dpy, sync);
    }
    return result;
}

EGLBoolean eglPresentationTimeANDROIDImpl(EGLDisplay dpy, EGLSurface surface,
        EGLnsecsANDROID time)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) {
        return EGL_FALSE;
    }

    SurfaceRef _s(dp.get(), surface);
    if (!_s.get()) {
        setError(EGL_BAD_SURFACE, EGL_FALSE);
        return EGL_FALSE;
    }

    egl_surface_t const * const s = get_surface(surface);
    native_window_set_buffers_timestamp(s->getNativeWindow(), time);

    return EGL_TRUE;
}

EGLClientBuffer eglGetNativeClientBufferANDROIDImpl(const AHardwareBuffer *buffer) {
    // AHardwareBuffer_to_ANativeWindowBuffer is a platform-only symbol and thus
    // this function cannot be implemented when this libEGL is built for
    // vendors.
#ifndef __ANDROID_VNDK__
    if (!buffer) return setError(EGL_BAD_PARAMETER, (EGLClientBuffer) nullptr);
    return const_cast<ANativeWindowBuffer *>(AHardwareBuffer_to_ANativeWindowBuffer(buffer));
#else
    return setError(EGL_BAD_PARAMETER, (EGLClientBuffer) nullptr);
#endif
}

// ----------------------------------------------------------------------------
// NVIDIA extensions
// ----------------------------------------------------------------------------
EGLuint64NV eglGetSystemTimeFrequencyNVImpl()
{
    EGLuint64NV ret = 0;
    egl_connection_t* const cnx = &gEGLImpl;

    if (cnx->dso && cnx->egl.eglGetSystemTimeFrequencyNV) {
        return cnx->egl.eglGetSystemTimeFrequencyNV();
    }

    return setErrorQuiet(EGL_BAD_DISPLAY, (EGLuint64NV)0);
}

EGLuint64NV eglGetSystemTimeNVImpl()
{
    EGLuint64NV ret = 0;
    egl_connection_t* const cnx = &gEGLImpl;

    if (cnx->dso && cnx->egl.eglGetSystemTimeNV) {
        return cnx->egl.eglGetSystemTimeNV();
    }

    return setErrorQuiet(EGL_BAD_DISPLAY, (EGLuint64NV)0);
}

// ----------------------------------------------------------------------------
// Partial update extension
// ----------------------------------------------------------------------------
EGLBoolean eglSetDamageRegionKHRImpl(EGLDisplay dpy, EGLSurface surface,
        EGLint *rects, EGLint n_rects)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) {
        setError(EGL_BAD_DISPLAY, EGL_FALSE);
        return EGL_FALSE;
    }

    SurfaceRef _s(dp.get(), surface);
    if (!_s.get()) {
        setError(EGL_BAD_SURFACE, EGL_FALSE);
        return EGL_FALSE;
    }

    egl_surface_t const * const s = get_surface(surface);
    if (s->cnx->egl.eglSetDamageRegionKHR) {
        return s->cnx->egl.eglSetDamageRegionKHR(dp->disp.dpy, s->surface,
                rects, n_rects);
    }

    return EGL_FALSE;
}

EGLBoolean eglGetNextFrameIdANDROIDImpl(EGLDisplay dpy, EGLSurface surface,
            EGLuint64KHR *frameId) {
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) {
        return setError(EGL_BAD_DISPLAY, (EGLBoolean)EGL_FALSE);
    }

    SurfaceRef _s(dp.get(), surface);
    if (!_s.get()) {
        return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);
    }

    egl_surface_t const * const s = get_surface(surface);

    if (!s->getNativeWindow()) {
        return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);
    }

    uint64_t nextFrameId = 0;
    int ret = native_window_get_next_frame_id(s->getNativeWindow(), &nextFrameId);

    if (ret != 0) {
        // This should not happen. Return an error that is not in the spec
        // so it's obvious something is very wrong.
        ALOGE("eglGetNextFrameId: Unexpected error.");
        return setError(EGL_NOT_INITIALIZED, (EGLBoolean)EGL_FALSE);
    }

    *frameId = nextFrameId;
    return EGL_TRUE;
}

EGLBoolean eglGetCompositorTimingANDROIDImpl(EGLDisplay dpy, EGLSurface surface,
        EGLint numTimestamps, const EGLint *names, EGLnsecsANDROID *values)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) {
        return setError(EGL_BAD_DISPLAY, (EGLBoolean)EGL_FALSE);
    }

    SurfaceRef _s(dp.get(), surface);
    if (!_s.get()) {
        return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);
    }

    egl_surface_t const * const s = get_surface(surface);

    if (!s->getNativeWindow()) {
        return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);
    }

    nsecs_t* compositeDeadline = nullptr;
    nsecs_t* compositeInterval = nullptr;
    nsecs_t* compositeToPresentLatency = nullptr;

    for (int i = 0; i < numTimestamps; i++) {
        switch (names[i]) {
            case EGL_COMPOSITE_DEADLINE_ANDROID:
                compositeDeadline = &values[i];
                break;
            case EGL_COMPOSITE_INTERVAL_ANDROID:
                compositeInterval = &values[i];
                break;
            case EGL_COMPOSITE_TO_PRESENT_LATENCY_ANDROID:
                compositeToPresentLatency = &values[i];
                break;
            default:
                return setError(EGL_BAD_PARAMETER, (EGLBoolean)EGL_FALSE);
        }
    }

    int ret = native_window_get_compositor_timing(s->getNativeWindow(),
            compositeDeadline, compositeInterval, compositeToPresentLatency);

    switch (ret) {
      case 0:
        return EGL_TRUE;
      case -ENOSYS:
        return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);
      default:
        // This should not happen. Return an error that is not in the spec
        // so it's obvious something is very wrong.
        ALOGE("eglGetCompositorTiming: Unexpected error.");
        return setError(EGL_NOT_INITIALIZED, (EGLBoolean)EGL_FALSE);
    }
}

EGLBoolean eglGetCompositorTimingSupportedANDROIDImpl(
        EGLDisplay dpy, EGLSurface surface, EGLint name)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) {
        return setError(EGL_BAD_DISPLAY, (EGLBoolean)EGL_FALSE);
    }

    SurfaceRef _s(dp.get(), surface);
    if (!_s.get()) {
        return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);
    }

    egl_surface_t const * const s = get_surface(surface);

    ANativeWindow* window = s->getNativeWindow();
    if (!window) {
        return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);
    }

    switch (name) {
        case EGL_COMPOSITE_DEADLINE_ANDROID:
        case EGL_COMPOSITE_INTERVAL_ANDROID:
        case EGL_COMPOSITE_TO_PRESENT_LATENCY_ANDROID:
            return EGL_TRUE;
        default:
            return EGL_FALSE;
    }
}

EGLBoolean eglGetFrameTimestampsANDROIDImpl(EGLDisplay dpy, EGLSurface surface,
        EGLuint64KHR frameId, EGLint numTimestamps, const EGLint *timestamps,
        EGLnsecsANDROID *values)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) {
        return setError(EGL_BAD_DISPLAY, (EGLBoolean)EGL_FALSE);
    }

    SurfaceRef _s(dp.get(), surface);
    if (!_s.get()) {
        return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);
    }

    egl_surface_t const * const s = get_surface(surface);

    if (!s->getNativeWindow()) {
        return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);
    }

    nsecs_t* requestedPresentTime = nullptr;
    nsecs_t* acquireTime = nullptr;
    nsecs_t* latchTime = nullptr;
    nsecs_t* firstRefreshStartTime = nullptr;
    nsecs_t* gpuCompositionDoneTime = nullptr;
    nsecs_t* lastRefreshStartTime = nullptr;
    nsecs_t* displayPresentTime = nullptr;
    nsecs_t* dequeueReadyTime = nullptr;
    nsecs_t* releaseTime = nullptr;

    for (int i = 0; i < numTimestamps; i++) {
        switch (timestamps[i]) {
            case EGL_REQUESTED_PRESENT_TIME_ANDROID:
                requestedPresentTime = &values[i];
                break;
            case EGL_RENDERING_COMPLETE_TIME_ANDROID:
                acquireTime = &values[i];
                break;
            case EGL_COMPOSITION_LATCH_TIME_ANDROID:
                latchTime = &values[i];
                break;
            case EGL_FIRST_COMPOSITION_START_TIME_ANDROID:
                firstRefreshStartTime = &values[i];
                break;
            case EGL_LAST_COMPOSITION_START_TIME_ANDROID:
                lastRefreshStartTime = &values[i];
                break;
            case EGL_FIRST_COMPOSITION_GPU_FINISHED_TIME_ANDROID:
                gpuCompositionDoneTime = &values[i];
                break;
            case EGL_DISPLAY_PRESENT_TIME_ANDROID:
                displayPresentTime = &values[i];
                break;
            case EGL_DEQUEUE_READY_TIME_ANDROID:
                dequeueReadyTime = &values[i];
                break;
            case EGL_READS_DONE_TIME_ANDROID:
                releaseTime = &values[i];
                break;
            default:
                return setError(EGL_BAD_PARAMETER, (EGLBoolean)EGL_FALSE);
        }
    }

    int ret = native_window_get_frame_timestamps(s->getNativeWindow(), frameId,
            requestedPresentTime, acquireTime, latchTime, firstRefreshStartTime,
            lastRefreshStartTime, gpuCompositionDoneTime, displayPresentTime,
            dequeueReadyTime, releaseTime);

    switch (ret) {
        case 0:
            return EGL_TRUE;
        case -ENOENT:
            return setError(EGL_BAD_ACCESS, (EGLBoolean)EGL_FALSE);
        case -ENOSYS:
            return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);
        case -EINVAL:
            return setError(EGL_BAD_PARAMETER, (EGLBoolean)EGL_FALSE);
        default:
            // This should not happen. Return an error that is not in the spec
            // so it's obvious something is very wrong.
            ALOGE("eglGetFrameTimestamps: Unexpected error.");
            return setError(EGL_NOT_INITIALIZED, (EGLBoolean)EGL_FALSE);
    }
}

EGLBoolean eglGetFrameTimestampSupportedANDROIDImpl(
        EGLDisplay dpy, EGLSurface surface, EGLint timestamp)
{
    const egl_display_ptr dp = validate_display(dpy);
    if (!dp) {
        return setError(EGL_BAD_DISPLAY, (EGLBoolean)EGL_FALSE);
    }

    SurfaceRef _s(dp.get(), surface);
    if (!_s.get()) {
        return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);
    }

    egl_surface_t const * const s = get_surface(surface);

    ANativeWindow* window = s->getNativeWindow();
    if (!window) {
        return setError(EGL_BAD_SURFACE, (EGLBoolean)EGL_FALSE);
    }

    switch (timestamp) {
        case EGL_COMPOSITE_DEADLINE_ANDROID:
        case EGL_COMPOSITE_INTERVAL_ANDROID:
        case EGL_COMPOSITE_TO_PRESENT_LATENCY_ANDROID:
        case EGL_REQUESTED_PRESENT_TIME_ANDROID:
        case EGL_RENDERING_COMPLETE_TIME_ANDROID:
        case EGL_COMPOSITION_LATCH_TIME_ANDROID:
        case EGL_FIRST_COMPOSITION_START_TIME_ANDROID:
        case EGL_LAST_COMPOSITION_START_TIME_ANDROID:
        case EGL_FIRST_COMPOSITION_GPU_FINISHED_TIME_ANDROID:
        case EGL_DEQUEUE_READY_TIME_ANDROID:
        case EGL_READS_DONE_TIME_ANDROID:
            return EGL_TRUE;
        case EGL_DISPLAY_PRESENT_TIME_ANDROID: {
            int value = 0;
            window->query(window,
                    NATIVE_WINDOW_FRAME_TIMESTAMPS_SUPPORTS_PRESENT, &value);
            return value == 0 ? EGL_FALSE : EGL_TRUE;
        }
        default:
            return EGL_FALSE;
    }
}

const GLubyte * glGetStringImpl(GLenum name) {
    const GLubyte * ret = egl_get_string_for_current_context(name);
    if (ret == NULL) {
        gl_hooks_t::gl_t const * const _c = &getGlThreadSpecific()->gl;
        if(_c) ret = _c->glGetString(name);
    }
    return ret;
}

const GLubyte * glGetStringiImpl(GLenum name, GLuint index) {
    const GLubyte * ret = egl_get_string_for_current_context(name, index);
    if (ret == NULL) {
        gl_hooks_t::gl_t const * const _c = &getGlThreadSpecific()->gl;
        if(_c) ret = _c->glGetStringi(name, index);
    }
    return ret;
}

void glGetBooleanvImpl(GLenum pname, GLboolean * data) {
    if (pname == GL_NUM_EXTENSIONS) {
        int num_exts = egl_get_num_extensions_for_current_context();
        if (num_exts >= 0) {
            *data = num_exts > 0 ? GL_TRUE : GL_FALSE;
            return;
        }
    }

    gl_hooks_t::gl_t const * const _c = &getGlThreadSpecific()->gl;
    if (_c) _c->glGetBooleanv(pname, data);
}

void glGetFloatvImpl(GLenum pname, GLfloat * data) {
    if (pname == GL_NUM_EXTENSIONS) {
        int num_exts = egl_get_num_extensions_for_current_context();
        if (num_exts >= 0) {
            *data = (GLfloat)num_exts;
            return;
        }
    }

    gl_hooks_t::gl_t const * const _c = &getGlThreadSpecific()->gl;
    if (_c) _c->glGetFloatv(pname, data);
}

void glGetIntegervImpl(GLenum pname, GLint * data) {
    if (pname == GL_NUM_EXTENSIONS) {
        int num_exts = egl_get_num_extensions_for_current_context();
        if (num_exts >= 0) {
            *data = (GLint)num_exts;
            return;
        }
    }

    gl_hooks_t::gl_t const * const _c = &getGlThreadSpecific()->gl;
    if (_c) _c->glGetIntegerv(pname, data);
}

void glGetInteger64vImpl(GLenum pname, GLint64 * data) {
    if (pname == GL_NUM_EXTENSIONS) {
        int num_exts = egl_get_num_extensions_for_current_context();
        if (num_exts >= 0) {
            *data = (GLint64)num_exts;
            return;
        }
    }

    gl_hooks_t::gl_t const * const _c = &getGlThreadSpecific()->gl;
    if (_c) _c->glGetInteger64v(pname, data);
}

struct implementation_map_t {
    const char* name;
    EGLFuncPointer address;
};

static const implementation_map_t sPlatformImplMap[] = {
        // clang-format off
    { "eglGetDisplay", (EGLFuncPointer)&eglGetDisplayImpl },
    { "eglGetPlatformDisplay", (EGLFuncPointer)&eglGetPlatformDisplayImpl },
    { "eglInitialize", (EGLFuncPointer)&eglInitializeImpl },
    { "eglTerminate", (EGLFuncPointer)&eglTerminateImpl },
    { "eglGetConfigs", (EGLFuncPointer)&eglGetConfigsImpl },
    { "eglChooseConfig", (EGLFuncPointer)&eglChooseConfigImpl },
    { "eglGetConfigAttrib", (EGLFuncPointer)&eglGetConfigAttribImpl },
    { "eglCreateWindowSurface", (EGLFuncPointer)&eglCreateWindowSurfaceImpl },
    { "eglCreatePixmapSurface", (EGLFuncPointer)&eglCreatePixmapSurfaceImpl },
    { "eglCreatePlatformWindowSurface", (EGLFuncPointer)&eglCreatePlatformWindowSurfaceImpl },
    { "eglCreatePlatformPixmapSurface", (EGLFuncPointer)&eglCreatePlatformPixmapSurfaceImpl },
    { "eglCreatePbufferSurface", (EGLFuncPointer)&eglCreatePbufferSurfaceImpl },
    { "eglDestroySurface", (EGLFuncPointer)&eglDestroySurfaceImpl },
    { "eglQuerySurface", (EGLFuncPointer)&eglQuerySurfaceImpl },
    { "eglBeginFrame", (EGLFuncPointer)&eglBeginFrameImpl },
    { "eglCreateContext", (EGLFuncPointer)&eglCreateContextImpl },
    { "eglDestroyContext", (EGLFuncPointer)&eglDestroyContextImpl },
    { "eglMakeCurrent", (EGLFuncPointer)&eglMakeCurrentImpl },
    { "eglQueryContext", (EGLFuncPointer)&eglQueryContextImpl },
    { "eglGetCurrentContext", (EGLFuncPointer)&eglGetCurrentContextImpl },
    { "eglGetCurrentSurface", (EGLFuncPointer)&eglGetCurrentSurfaceImpl },
    { "eglGetCurrentDisplay", (EGLFuncPointer)&eglGetCurrentDisplayImpl },
    { "eglWaitGL", (EGLFuncPointer)&eglWaitGLImpl },
    { "eglWaitNative", (EGLFuncPointer)&eglWaitNativeImpl },
    { "eglGetError", (EGLFuncPointer)&eglGetErrorImpl },
    { "eglSwapBuffersWithDamageKHR", (EGLFuncPointer)&eglSwapBuffersWithDamageKHRImpl },
    { "eglGetProcAddress", (EGLFuncPointer)&eglGetProcAddressImpl },
    { "eglSwapBuffers", (EGLFuncPointer)&eglSwapBuffersImpl },
    { "eglCopyBuffers", (EGLFuncPointer)&eglCopyBuffersImpl },
    { "eglQueryString", (EGLFuncPointer)&eglQueryStringImpl },
    { "eglQueryStringImplementationANDROID", (EGLFuncPointer)&eglQueryStringImplementationANDROIDImpl },
    { "eglSurfaceAttrib", (EGLFuncPointer)&eglSurfaceAttribImpl },
    { "eglBindTexImage", (EGLFuncPointer)&eglBindTexImageImpl },
    { "eglReleaseTexImage", (EGLFuncPointer)&eglReleaseTexImageImpl },
    { "eglSwapInterval", (EGLFuncPointer)&eglSwapIntervalImpl },
    { "eglWaitClient", (EGLFuncPointer)&eglWaitClientImpl },
    { "eglBindAPI", (EGLFuncPointer)&eglBindAPIImpl },
    { "eglQueryAPI", (EGLFuncPointer)&eglQueryAPIImpl },
    { "eglReleaseThread", (EGLFuncPointer)&eglReleaseThreadImpl },
    { "eglCreatePbufferFromClientBuffer", (EGLFuncPointer)&eglCreatePbufferFromClientBufferImpl },
    { "eglLockSurfaceKHR", (EGLFuncPointer)&eglLockSurfaceKHRImpl },
    { "eglUnlockSurfaceKHR", (EGLFuncPointer)&eglUnlockSurfaceKHRImpl },
    { "eglCreateImageKHR", (EGLFuncPointer)&eglCreateImageKHRImpl },
    { "eglDestroyImageKHR", (EGLFuncPointer)&eglDestroyImageKHRImpl },
    { "eglCreateImage", (EGLFuncPointer)&eglCreateImageImpl },
    { "eglDestroyImage", (EGLFuncPointer)&eglDestroyImageImpl },
    { "eglCreateSync", (EGLFuncPointer)&eglCreateSyncImpl },
    { "eglDestroySync", (EGLFuncPointer)&eglDestroySyncImpl },
    { "eglClientWaitSync", (EGLFuncPointer)&eglClientWaitSyncImpl },
    { "eglGetSyncAttrib", (EGLFuncPointer)&eglGetSyncAttribImpl },
    { "eglCreateSyncKHR", (EGLFuncPointer)&eglCreateSyncKHRImpl },
    { "eglDestroySyncKHR", (EGLFuncPointer)&eglDestroySyncKHRImpl },
    { "eglSignalSyncKHR", (EGLFuncPointer)&eglSignalSyncKHRImpl },
    { "eglClientWaitSyncKHR", (EGLFuncPointer)&eglClientWaitSyncKHRImpl },
    { "eglGetSyncAttribKHR", (EGLFuncPointer)&eglGetSyncAttribKHRImpl },
    { "eglCreateStreamKHR", (EGLFuncPointer)&eglCreateStreamKHRImpl },
    { "eglDestroyStreamKHR", (EGLFuncPointer)&eglDestroyStreamKHRImpl },
    { "eglStreamAttribKHR", (EGLFuncPointer)&eglStreamAttribKHRImpl },
    { "eglQueryStreamKHR", (EGLFuncPointer)&eglQueryStreamKHRImpl },
    { "eglQueryStreamu64KHR", (EGLFuncPointer)&eglQueryStreamu64KHRImpl },
    { "eglQueryStreamTimeKHR", (EGLFuncPointer)&eglQueryStreamTimeKHRImpl },
    { "eglCreateStreamProducerSurfaceKHR", (EGLFuncPointer)&eglCreateStreamProducerSurfaceKHRImpl },
    { "eglStreamConsumerGLTextureExternalKHR", (EGLFuncPointer)&eglStreamConsumerGLTextureExternalKHRImpl },
    { "eglStreamConsumerAcquireKHR", (EGLFuncPointer)&eglStreamConsumerAcquireKHRImpl },
    { "eglStreamConsumerReleaseKHR", (EGLFuncPointer)&eglStreamConsumerReleaseKHRImpl },
    { "eglGetStreamFileDescriptorKHR", (EGLFuncPointer)&eglGetStreamFileDescriptorKHRImpl },
    { "eglCreateStreamFromFileDescriptorKHR", (EGLFuncPointer)&eglCreateStreamFromFileDescriptorKHRImpl },
    { "eglWaitSync", (EGLFuncPointer)&eglWaitSyncImpl },
    { "eglWaitSyncKHR", (EGLFuncPointer)&eglWaitSyncKHRImpl },
    { "eglDupNativeFenceFDANDROID", (EGLFuncPointer)&eglDupNativeFenceFDANDROIDImpl },
    { "eglPresentationTimeANDROID", (EGLFuncPointer)&eglPresentationTimeANDROIDImpl },
    { "eglGetNativeClientBufferANDROID", (EGLFuncPointer)&eglGetNativeClientBufferANDROIDImpl },
    { "eglGetSystemTimeFrequencyNV", (EGLFuncPointer)&eglGetSystemTimeFrequencyNVImpl },
    { "eglGetSystemTimeNV", (EGLFuncPointer)&eglGetSystemTimeNVImpl },
    { "eglSetDamageRegionKHR", (EGLFuncPointer)&eglSetDamageRegionKHRImpl },
    { "eglGetNextFrameIdANDROID", (EGLFuncPointer)&eglGetNextFrameIdANDROIDImpl },
    { "eglGetCompositorTimingANDROID", (EGLFuncPointer)&eglGetCompositorTimingANDROIDImpl },
    { "eglGetCompositorTimingSupportedANDROID", (EGLFuncPointer)&eglGetCompositorTimingSupportedANDROIDImpl },
    { "eglGetFrameTimestampsANDROID", (EGLFuncPointer)&eglGetFrameTimestampsANDROIDImpl },
    { "eglGetFrameTimestampSupportedANDROID", (EGLFuncPointer)&eglGetFrameTimestampSupportedANDROIDImpl },
    { "glGetString", (EGLFuncPointer)&glGetStringImpl },
    { "glGetStringi", (EGLFuncPointer)&glGetStringiImpl },
    { "glGetBooleanv", (EGLFuncPointer)&glGetBooleanvImpl },
    { "glGetFloatv", (EGLFuncPointer)&glGetFloatvImpl },
    { "glGetIntegerv", (EGLFuncPointer)&glGetIntegervImpl },
    { "glGetInteger64v", (EGLFuncPointer)&glGetInteger64vImpl },
        // clang-format on
};

EGLFuncPointer FindPlatformImplAddr(const char* name)
{
    static const bool DEBUG = false;

    if (name == nullptr) {
        ALOGV("FindPlatformImplAddr called with null name");
        return nullptr;
    }

    for (int i = 0; i < NELEM(sPlatformImplMap); i++) {
        if (sPlatformImplMap[i].name == nullptr) {
            ALOGV("FindPlatformImplAddr found nullptr for sPlatformImplMap[%i].name (%s)", i, name);
            return nullptr;
        }
        if (!strcmp(name, sPlatformImplMap[i].name)) {
            ALOGV("FindPlatformImplAddr found %llu for sPlatformImplMap[%i].address (%s)", (unsigned long long)sPlatformImplMap[i].address, i, name);
            return sPlatformImplMap[i].address;
        }
    }

    ALOGV("FindPlatformImplAddr did not find an entry for %s", name);
    return nullptr;
}
} // namespace android
