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

//#define LOG_NDEBUG 0
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <EGL/Loader.h>

#include <string>

#include <dirent.h>
#include <dlfcn.h>

#include <android/dlext.h>
#include <cutils/properties.h>
#include <log/log.h>
#include <utils/Timers.h>

#ifndef __ANDROID_VNDK__
#include <graphicsenv/GraphicsEnv.h>
#endif
#include <vndksupport/linker.h>

#include "egl_platform_entries.h"
#include "egl_trace.h"
#include "egldefs.h"
#include <EGL/eglext_angle.h>

namespace android {

/*
 * EGL userspace drivers must be provided either:
 * - as a single library:
 *      /vendor/lib/egl/libGLES.so
 *
 * - as separate libraries:
 *      /vendor/lib/egl/libEGL.so
 *      /vendor/lib/egl/libGLESv1_CM.so
 *      /vendor/lib/egl/libGLESv2.so
 *
 * The software renderer for the emulator must be provided as a single
 * library at:
 *
 *      /system/lib/egl/libGLES_android.so
 *
 *
 * For backward compatibility and to facilitate the transition to
 * this new naming scheme, the loader will additionally look for:
 *
 *      /{vendor|system}/lib/egl/lib{GLES | [EGL|GLESv1_CM|GLESv2]}_*.so
 *
 */

Loader& Loader::getInstance() {
    static Loader loader;
    return loader;
}

static void* do_dlopen(const char* path, int mode) {
    ATRACE_CALL();
    return dlopen(path, mode);
}

static void* do_android_dlopen_ext(const char* path, int mode, const android_dlextinfo* info) {
    ATRACE_CALL();
    return android_dlopen_ext(path, mode, info);
}

static void* do_android_load_sphal_library(const char* path, int mode) {
    ATRACE_CALL();
    return android_load_sphal_library(path, mode);
}

static int do_android_unload_sphal_library(void* dso) {
    ATRACE_CALL();
    return android_unload_sphal_library(dso);
}

Loader::driver_t::driver_t(void* gles)
{
    dso[0] = gles;
    for (size_t i=1 ; i<NELEM(dso) ; i++)
        dso[i] = nullptr;
}

Loader::driver_t::~driver_t()
{
    for (size_t i=0 ; i<NELEM(dso) ; i++) {
        if (dso[i]) {
            dlclose(dso[i]);
            dso[i] = nullptr;
        }
    }
}

int Loader::driver_t::set(void* hnd, int32_t api)
{
    switch (api) {
        case EGL:
            dso[0] = hnd;
            break;
        case GLESv1_CM:
            dso[1] = hnd;
            break;
        case GLESv2:
            dso[2] = hnd;
            break;
        default:
            return -EOVERFLOW;
    }
    return 0;
}

Loader::Loader()
    : getProcAddress(nullptr)
{
}

Loader::~Loader() {
}

static void* load_wrapper(const char* path) {
    void* so = do_dlopen(path, RTLD_NOW | RTLD_LOCAL);
    ALOGE_IF(!so, "dlopen(\"%s\") failed: %s", path, dlerror());
    return so;
}

#ifndef EGL_WRAPPER_DIR
#if defined(__LP64__)
#define EGL_WRAPPER_DIR "/system/lib64"
#else
#define EGL_WRAPPER_DIR "/system/lib"
#endif
#endif

static void setEmulatorGlesValue(void) {
    char prop[PROPERTY_VALUE_MAX];
    property_get("ro.kernel.qemu", prop, "0");
    if (atoi(prop) != 1) return;

    property_get("ro.kernel.qemu.gles",prop,"0");
    if (atoi(prop) == 1) {
        ALOGD("Emulator has host GPU support, qemu.gles is set to 1.");
        property_set("qemu.gles", "1");
        return;
    }

    // for now, checking the following
    // directory is good enough for emulator system images
    const char* vendor_lib_path =
#if defined(__LP64__)
        "/vendor/lib64/egl";
#else
        "/vendor/lib/egl";
#endif

    const bool has_vendor_lib = (access(vendor_lib_path, R_OK) == 0);
    if (has_vendor_lib) {
        ALOGD("Emulator has vendor provided software renderer, qemu.gles is set to 2.");
        property_set("qemu.gles", "2");
    } else {
        ALOGD("Emulator without GPU support detected. "
              "Fallback to legacy software renderer, qemu.gles is set to 0.");
        property_set("qemu.gles", "0");
    }
}

static const char* DRIVER_SUFFIX_PROPERTY = "ro.hardware.egl";

static const char* HAL_SUBNAME_KEY_PROPERTIES[2] = {
    DRIVER_SUFFIX_PROPERTY,
    "ro.board.platform",
};

static bool should_unload_system_driver(egl_connection_t* cnx) {
    // Return false if the system driver has been unloaded once.
    if (cnx->systemDriverUnloaded) {
        return false;
    }

    // Return true if Angle namespace is set.
    android_namespace_t* ns = android::GraphicsEnv::getInstance().getAngleNamespace();
    if (ns) {
        return true;
    }

#ifndef __ANDROID_VNDK__
    // Return true if updated driver namespace is set.
    ns = android::GraphicsEnv::getInstance().getDriverNamespace();
    if (ns) {
        return true;
    }
#endif

    return false;
}

static void uninit_api(char const* const* api, __eglMustCastToProperFunctionPointerType* curr) {
    while (*api) {
        *curr++ = nullptr;
        api++;
    }
}

void Loader::unload_system_driver(egl_connection_t* cnx) {
    ATRACE_CALL();

    uninit_api(gl_names,
               (__eglMustCastToProperFunctionPointerType*)&cnx
                       ->hooks[egl_connection_t::GLESv2_INDEX]
                       ->gl);
    uninit_api(gl_names,
               (__eglMustCastToProperFunctionPointerType*)&cnx
                       ->hooks[egl_connection_t::GLESv1_INDEX]
                       ->gl);
    uninit_api(egl_names, (__eglMustCastToProperFunctionPointerType*)&cnx->egl);

    if (cnx->dso) {
        ALOGD("Unload system gl driver.");
        driver_t* hnd = (driver_t*)cnx->dso;
        if (hnd->dso[2]) {
            do_android_unload_sphal_library(hnd->dso[2]);
        }
        if (hnd->dso[1]) {
            do_android_unload_sphal_library(hnd->dso[1]);
        }
        if (hnd->dso[0]) {
            do_android_unload_sphal_library(hnd->dso[0]);
        }
        cnx->dso = nullptr;
    }

    cnx->systemDriverUnloaded = true;
}

void* Loader::open(egl_connection_t* cnx)
{
    ATRACE_CALL();
    const nsecs_t openTime = systemTime();

    if (should_unload_system_driver(cnx)) {
        unload_system_driver(cnx);
    }

    // If a driver has been loaded, return the driver directly.
    if (cnx->dso) {
        return cnx->dso;
    }

    setEmulatorGlesValue();

    // Check if we should use ANGLE early, so loading each driver doesn't require repeated queries.
    if (android::GraphicsEnv::getInstance().shouldUseAngle()) {
        cnx->shouldUseAngle = true;
    } else {
        cnx->shouldUseAngle = false;
    }

    // Firstly, try to load ANGLE driver.
    driver_t* hnd = attempt_to_load_angle(cnx);
    if (!hnd) {
        // Secondly, try to load from driver apk.
        hnd = attempt_to_load_updated_driver(cnx);
    }

    bool failToLoadFromDriverSuffixProperty = false;
    if (!hnd) {
        // Finally, try to load system driver, start by searching for the library name appended by
        // the system properties of the GLES userspace driver in both locations.
        // i.e.:
        //      libGLES_${prop}.so, or:
        //      libEGL_${prop}.so, libGLESv1_CM_${prop}.so, libGLESv2_${prop}.so
        char prop[PROPERTY_VALUE_MAX + 1];
        for (auto key : HAL_SUBNAME_KEY_PROPERTIES) {
            if (property_get(key, prop, nullptr) <= 0) {
                continue;
            }
            hnd = attempt_to_load_system_driver(cnx, prop, true);
            if (hnd) {
                break;
            } else if (strcmp(key, DRIVER_SUFFIX_PROPERTY) == 0) {
                failToLoadFromDriverSuffixProperty = true;
            }
        }
    }

    if (!hnd) {
        // Can't find graphics driver by appending system properties, now search for the exact name
        // without any suffix of the GLES userspace driver in both locations.
        // i.e.:
        //      libGLES.so, or:
        //      libEGL.so, libGLESv1_CM.so, libGLESv2.so
        hnd = attempt_to_load_system_driver(cnx, nullptr, true);
    }

    if (!hnd && !failToLoadFromDriverSuffixProperty) {
        hnd = attempt_to_load_system_driver(cnx, nullptr, false);
    }

    if (!hnd) {
        android::GraphicsEnv::getInstance().setDriverLoaded(android::GraphicsEnv::Api::API_GL,
                                                            false, systemTime() - openTime);
    }

    LOG_ALWAYS_FATAL_IF(!hnd,
                        "couldn't find an OpenGL ES implementation, make sure you set %s or %s",
                        HAL_SUBNAME_KEY_PROPERTIES[0], HAL_SUBNAME_KEY_PROPERTIES[1]);

    if (!cnx->libEgl) {
        cnx->libEgl = load_wrapper(EGL_WRAPPER_DIR "/libEGL.so");
    }
    if (!cnx->libGles1) {
        cnx->libGles1 = load_wrapper(EGL_WRAPPER_DIR "/libGLESv1_CM.so");
    }
    if (!cnx->libGles2) {
        cnx->libGles2 = load_wrapper(EGL_WRAPPER_DIR "/libGLESv2.so");
    }

    if (!cnx->libEgl || !cnx->libGles2 || !cnx->libGles1) {
        android::GraphicsEnv::getInstance().setDriverLoaded(android::GraphicsEnv::Api::API_GL,
                                                            false, systemTime() - openTime);
    }

    LOG_ALWAYS_FATAL_IF(!cnx->libEgl,
            "couldn't load system EGL wrapper libraries");

    LOG_ALWAYS_FATAL_IF(!cnx->libGles2 || !cnx->libGles1,
            "couldn't load system OpenGL ES wrapper libraries");

    android::GraphicsEnv::getInstance().setDriverLoaded(android::GraphicsEnv::Api::API_GL, true,
                                                        systemTime() - openTime);

    return (void*)hnd;
}

void Loader::close(egl_connection_t* cnx)
{
    driver_t* hnd = (driver_t*) cnx->dso;
    delete hnd;
    cnx->dso = nullptr;

    cnx->shouldUseAngle = false;
    cnx->angleDecided = false;
    cnx->useAngle = false;

    if (cnx->vendorEGL) {
        dlclose(cnx->vendorEGL);
        cnx->vendorEGL = nullptr;
    }
}

void Loader::init_api(void* dso,
        char const * const * api,
        char const * const * ref_api,
        __eglMustCastToProperFunctionPointerType* curr,
        getProcAddressType getProcAddress)
{
    ATRACE_CALL();

    const ssize_t SIZE = 256;
    char scrap[SIZE];
    while (*api) {
        char const * name = *api;
        if (ref_api) {
            char const * ref_name = *ref_api;
            if (std::strcmp(name, ref_name) != 0) {
                *curr++ = nullptr;
                ref_api++;
                continue;
            }
        }

        __eglMustCastToProperFunctionPointerType f =
            (__eglMustCastToProperFunctionPointerType)dlsym(dso, name);
        if (f == nullptr) {
            // couldn't find the entry-point, use eglGetProcAddress()
            f = getProcAddress(name);
        }
        if (f == nullptr) {
            // Try without the OES postfix
            ssize_t index = ssize_t(strlen(name)) - 3;
            if ((index>0 && (index<SIZE-1)) && (!strcmp(name+index, "OES"))) {
                strncpy(scrap, name, index);
                scrap[index] = 0;
                f = (__eglMustCastToProperFunctionPointerType)dlsym(dso, scrap);
                //ALOGD_IF(f, "found <%s> instead", scrap);
            }
        }
        if (f == nullptr) {
            // Try with the OES postfix
            ssize_t index = ssize_t(strlen(name)) - 3;
            if (index>0 && strcmp(name+index, "OES")) {
                snprintf(scrap, SIZE, "%sOES", name);
                f = (__eglMustCastToProperFunctionPointerType)dlsym(dso, scrap);
                //ALOGD_IF(f, "found <%s> instead", scrap);
            }
        }
        if (f == nullptr) {
            //ALOGD("%s", name);
            f = (__eglMustCastToProperFunctionPointerType)gl_unimplemented;

            /*
             * GL_EXT_debug_label is special, we always report it as
             * supported, it's handled by GLES_trace. If GLES_trace is not
             * enabled, then these are no-ops.
             */
            if (!strcmp(name, "glInsertEventMarkerEXT")) {
                f = (__eglMustCastToProperFunctionPointerType)gl_noop;
            } else if (!strcmp(name, "glPushGroupMarkerEXT")) {
                f = (__eglMustCastToProperFunctionPointerType)gl_noop;
            } else if (!strcmp(name, "glPopGroupMarkerEXT")) {
                f = (__eglMustCastToProperFunctionPointerType)gl_noop;
            }
        }
        *curr++ = f;
        api++;
        if (ref_api) ref_api++;
    }
}

static void* load_system_driver(const char* kind, const char* suffix, const bool exact) {
    ATRACE_CALL();
    class MatchFile {
    public:
        static std::string find(const char* libraryName, const bool exact) {
            const char* const searchPaths[] = {
#if defined(__LP64__)
                    "/vendor/lib64/egl",
                    "/system/lib64/egl"
#else
                    "/vendor/lib/egl",
                    "/system/lib/egl"
#endif
            };

            for (auto dir : searchPaths) {
                std::string absolutePath;
                if (find(absolutePath, libraryName, dir, exact)) {
                    return absolutePath;
                }
            }

            // Driver not found. gah.
            return std::string();
        }
    private:
        static bool find(std::string& result,
                const std::string& pattern, const char* const search, bool exact) {
            if (exact) {
                std::string absolutePath = std::string(search) + "/" + pattern + ".so";
                if (!access(absolutePath.c_str(), R_OK)) {
                    result = absolutePath;
                    return true;
                }
                return false;
            }

            DIR* d = opendir(search);
            if (d != nullptr) {
                struct dirent* e;
                while ((e = readdir(d)) != nullptr) {
                    if (e->d_type == DT_DIR) {
                        continue;
                    }
                    if (!strcmp(e->d_name, "libGLES_android.so")) {
                        // always skip the software renderer
                        continue;
                    }
                    if (strstr(e->d_name, pattern.c_str()) == e->d_name) {
                        if (!strcmp(e->d_name + strlen(e->d_name) - 3, ".so")) {
                            result = std::string(search) + "/" + e->d_name;
                            closedir(d);
                            return true;
                        }
                    }
                }
                closedir(d);
            }
            return false;
        }
    };

    std::string libraryName = std::string("lib") + kind;
    if (suffix) {
        libraryName += std::string("_") + suffix;
    } else if (!exact) {
        // Deprecated: we look for files that match
        //      libGLES_*.so, or:
        //      libEGL_*.so, libGLESv1_CM_*.so, libGLESv2_*.so
        libraryName += std::string("_");
    }
    std::string absolutePath = MatchFile::find(libraryName.c_str(), exact);
    if (absolutePath.empty()) {
        // this happens often, we don't want to log an error
        return nullptr;
    }
    const char* const driver_absolute_path = absolutePath.c_str();

    // Try to load drivers from the 'sphal' namespace, if it exist. Fall back to
    // the original routine when the namespace does not exist.
    // See /system/core/rootdir/etc/ld.config.txt for the configuration of the
    // sphal namespace.
    void* dso = do_android_load_sphal_library(driver_absolute_path,
                                              RTLD_NOW | RTLD_LOCAL);
    if (dso == nullptr) {
        const char* err = dlerror();
        ALOGE("load_driver(%s): %s", driver_absolute_path, err ? err : "unknown");
        return nullptr;
    }

    ALOGD("loaded %s", driver_absolute_path);

    return dso;
}

static void* load_angle_from_namespace(const char* kind, android_namespace_t* ns) {
    const android_dlextinfo dlextinfo = {
            .flags = ANDROID_DLEXT_USE_NAMESPACE,
            .library_namespace = ns,
    };

    std::string name = std::string("lib") + kind + "_angle.so";

    void* so = do_android_dlopen_ext(name.c_str(), RTLD_LOCAL | RTLD_NOW, &dlextinfo);

    if (so) {
        ALOGD("dlopen_ext from APK (%s) success at %p", name.c_str(), so);
        return so;
    } else {
        ALOGE("dlopen_ext(\"%s\") failed: %s", name.c_str(), dlerror());
    }

    return nullptr;
}

static void* load_angle(const char* kind, android_namespace_t* ns, egl_connection_t* cnx) {
    void* so = nullptr;

    if ((cnx->shouldUseAngle) || android::GraphicsEnv::getInstance().shouldUseAngle()) {
        so = load_angle_from_namespace(kind, ns);
        cnx->shouldUseAngle = true;
    } else {
        cnx->shouldUseAngle = false;
    }

    if (so) {
        ALOGV("Loaded ANGLE %s library for '%s' (instead of native)", kind,
            android::GraphicsEnv::getInstance().getAngleAppName().c_str());
        cnx->useAngle = true;

        char prop[PROPERTY_VALUE_MAX];

        property_get("debug.hwui.renderer", prop, "UNSET");
        ALOGV("Skia's renderer set to %s", prop);

        EGLint angleBackendDefault = EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE;
        property_get("debug.angle.backend", prop, "0");
        switch (atoi(prop)) {
            case 1:
                ALOGV("%s: Requesting OpenGLES back-end", __FUNCTION__);
                angleBackendDefault = EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE;
                break;
            case 2:
                ALOGV("%s: Requesting Vulkan back-end", __FUNCTION__);
                angleBackendDefault = EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE;
                break;
            default:
                break;
        }

        cnx->angleBackend = angleBackendDefault;
        if (!cnx->vendorEGL && (cnx->angleBackend == EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE)) {
            // Find and load vendor libEGL for ANGLE's GL back-end to use.
            char prop[PROPERTY_VALUE_MAX + 1];
            for (auto key : HAL_SUBNAME_KEY_PROPERTIES) {
                if (property_get(key, prop, nullptr) <= 0) {
                    continue;
                }
                void* dso = load_system_driver("EGL", prop, true);
                if (dso) {
                    cnx->vendorEGL = dso;
                    break;
                }
            }
            if (!cnx->vendorEGL) {
                cnx->vendorEGL = load_system_driver("EGL", nullptr, true);
            }
        }
    } else {
        ALOGV("Loaded native %s library for '%s' (instead of ANGLE)", kind,
            android::GraphicsEnv::getInstance().getAngleAppName().c_str());
        cnx->useAngle = false;
    }
    cnx->angleDecided = true;

    return so;
}

static void* load_updated_driver(const char* kind, android_namespace_t* ns) {
    ATRACE_CALL();
    const android_dlextinfo dlextinfo = {
        .flags = ANDROID_DLEXT_USE_NAMESPACE,
        .library_namespace = ns,
    };
    void* so = nullptr;
    char prop[PROPERTY_VALUE_MAX + 1];
    for (auto key : HAL_SUBNAME_KEY_PROPERTIES) {
        if (property_get(key, prop, nullptr) <= 0) {
            continue;
        }
        std::string name = std::string("lib") + kind + "_" + prop + ".so";
        so = do_android_dlopen_ext(name.c_str(), RTLD_LOCAL | RTLD_NOW, &dlextinfo);
        if (so) {
            return so;
        }
    }
    return nullptr;
}

Loader::driver_t* Loader::attempt_to_load_angle(egl_connection_t* cnx) {
    ATRACE_CALL();
    android_namespace_t* ns = android::GraphicsEnv::getInstance().getAngleNamespace();
    if (!ns) {
        return nullptr;
    }

    android::GraphicsEnv::getInstance().setDriverToLoad(android::GraphicsEnv::Driver::ANGLE);
    driver_t* hnd = nullptr;

    // ANGLE doesn't ship with GLES library, and thus we skip GLES driver.
    void* dso = load_angle("EGL", ns, cnx);
    if (dso) {
        initialize_api(dso, cnx, EGL);
        hnd = new driver_t(dso);

        dso = load_angle("GLESv1_CM", ns, cnx);
        initialize_api(dso, cnx, GLESv1_CM);
        hnd->set(dso, GLESv1_CM);

        dso = load_angle("GLESv2", ns, cnx);
        initialize_api(dso, cnx, GLESv2);
        hnd->set(dso, GLESv2);
    }
    return hnd;
}

Loader::driver_t* Loader::attempt_to_load_updated_driver(egl_connection_t* cnx) {
    ATRACE_CALL();
#ifndef __ANDROID_VNDK__
    android_namespace_t* ns = android::GraphicsEnv::getInstance().getDriverNamespace();
    if (!ns) {
        return nullptr;
    }

    ALOGD("Load updated gl driver.");
    android::GraphicsEnv::getInstance().setDriverToLoad(android::GraphicsEnv::Driver::GL_UPDATED);
    driver_t* hnd = nullptr;
    void* dso = load_updated_driver("GLES", ns);
    if (dso) {
        initialize_api(dso, cnx, EGL | GLESv1_CM | GLESv2);
        hnd = new driver_t(dso);
        return hnd;
    }

    dso = load_updated_driver("EGL", ns);
    if (dso) {
        initialize_api(dso, cnx, EGL);
        hnd = new driver_t(dso);

        dso = load_updated_driver("GLESv1_CM", ns);
        initialize_api(dso, cnx, GLESv1_CM);
        hnd->set(dso, GLESv1_CM);

        dso = load_updated_driver("GLESv2", ns);
        initialize_api(dso, cnx, GLESv2);
        hnd->set(dso, GLESv2);
    }
    return hnd;
#else
    return nullptr;
#endif
}

Loader::driver_t* Loader::attempt_to_load_system_driver(egl_connection_t* cnx, const char* suffix,
                                                        const bool exact) {
    ATRACE_CALL();
    android::GraphicsEnv::getInstance().setDriverToLoad(android::GraphicsEnv::Driver::GL);
    driver_t* hnd = nullptr;
    void* dso = load_system_driver("GLES", suffix, exact);
    if (dso) {
        initialize_api(dso, cnx, EGL | GLESv1_CM | GLESv2);
        hnd = new driver_t(dso);
        return hnd;
    }
    dso = load_system_driver("EGL", suffix, exact);
    if (dso) {
        initialize_api(dso, cnx, EGL);
        hnd = new driver_t(dso);

        dso = load_system_driver("GLESv1_CM", suffix, exact);
        initialize_api(dso, cnx, GLESv1_CM);
        hnd->set(dso, GLESv1_CM);

        dso = load_system_driver("GLESv2", suffix, exact);
        initialize_api(dso, cnx, GLESv2);
        hnd->set(dso, GLESv2);
    }
    return hnd;
}

void Loader::initialize_api(void* dso, egl_connection_t* cnx, uint32_t mask) {
    if (mask & EGL) {
        getProcAddress = (getProcAddressType)dlsym(dso, "eglGetProcAddress");

        ALOGE_IF(!getProcAddress,
                "can't find eglGetProcAddress() in EGL driver library");

        egl_t* egl = &cnx->egl;
        __eglMustCastToProperFunctionPointerType* curr =
            (__eglMustCastToProperFunctionPointerType*)egl;
        char const * const * api = egl_names;
        while (*api) {
            char const * name = *api;
            __eglMustCastToProperFunctionPointerType f =
                (__eglMustCastToProperFunctionPointerType)dlsym(dso, name);
            if (f == nullptr) {
                // couldn't find the entry-point, use eglGetProcAddress()
                f = getProcAddress(name);
                if (f == nullptr) {
                    f = (__eglMustCastToProperFunctionPointerType)nullptr;
                }
            }
            *curr++ = f;
            api++;
        }
    }

    if (mask & GLESv1_CM) {
        init_api(dso, gl_names_1, gl_names,
            (__eglMustCastToProperFunctionPointerType*)
                &cnx->hooks[egl_connection_t::GLESv1_INDEX]->gl,
            getProcAddress);
    }

    if (mask & GLESv2) {
        init_api(dso, gl_names, nullptr,
            (__eglMustCastToProperFunctionPointerType*)
                &cnx->hooks[egl_connection_t::GLESv2_INDEX]->gl,
            getProcAddress);
    }
}

} // namespace android
