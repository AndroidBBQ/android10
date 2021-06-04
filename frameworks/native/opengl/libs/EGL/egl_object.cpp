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

#include "egl_object.h"

#include <sstream>


// ----------------------------------------------------------------------------
namespace android {
// ----------------------------------------------------------------------------

egl_object_t::egl_object_t(egl_display_t* disp) :
    display(disp), count(1) {
    // NOTE: this does an implicit incRef
    display->addObject(this);
}

egl_object_t::~egl_object_t() {
}

void egl_object_t::terminate() {
    // this marks the object as "terminated"
    display->removeObject(this);
    if (decRef() == 1) {
        // shouldn't happen because this is called from LocalRef
        ALOGE("egl_object_t::terminate() removed the last reference!");
    }
}

void egl_object_t::destroy() {
    if (decRef() == 1) {
        delete this;
    }
}

bool egl_object_t::get(egl_display_t const* display, egl_object_t* object) {
    // used by LocalRef, this does an incRef() atomically with
    // checking that the object is valid.
    return display->getObject(object);
}

// ----------------------------------------------------------------------------

egl_surface_t::egl_surface_t(egl_display_t* dpy, EGLConfig config, EGLNativeWindowType win,
                             EGLSurface surface, EGLint colorSpace, egl_connection_t const* cnx)
      : egl_object_t(dpy),
        surface(surface),
        config(config),
        win(win),
        cnx(cnx),
        connected(true),
        colorSpace(colorSpace),
        egl_smpte2086_dirty(false),
        egl_cta861_3_dirty(false) {
    egl_smpte2086_metadata.displayPrimaryRed = { EGL_DONT_CARE, EGL_DONT_CARE };
    egl_smpte2086_metadata.displayPrimaryGreen = { EGL_DONT_CARE, EGL_DONT_CARE };
    egl_smpte2086_metadata.displayPrimaryBlue = { EGL_DONT_CARE, EGL_DONT_CARE };
    egl_smpte2086_metadata.whitePoint = { EGL_DONT_CARE, EGL_DONT_CARE };
    egl_smpte2086_metadata.maxLuminance = EGL_DONT_CARE;
    egl_smpte2086_metadata.minLuminance = EGL_DONT_CARE;
    egl_cta861_3_metadata.maxFrameAverageLightLevel = EGL_DONT_CARE;
    egl_cta861_3_metadata.maxContentLightLevel = EGL_DONT_CARE;

    if (win) {
        win->incStrong(this);
    }
}

egl_surface_t::~egl_surface_t() {
    if (win != nullptr) {
        disconnect();
        win->decStrong(this);
    }
}

void egl_surface_t::disconnect() {
    if (win != nullptr && connected) {
        native_window_set_buffers_format(win, 0);
        if (native_window_api_disconnect(win, NATIVE_WINDOW_API_EGL)) {
            ALOGW("EGLNativeWindowType %p disconnect failed", win);
        }
        connected = false;
    }
}

EGLBoolean egl_surface_t::setSmpte2086Attribute(EGLint attribute, EGLint value) {
    switch (attribute) {
        case EGL_SMPTE2086_DISPLAY_PRIMARY_RX_EXT:
            egl_smpte2086_metadata.displayPrimaryRed.x = value;
            egl_smpte2086_dirty = true;
            return EGL_TRUE;
        case EGL_SMPTE2086_DISPLAY_PRIMARY_RY_EXT:
            egl_smpte2086_metadata.displayPrimaryRed.y = value;
            egl_smpte2086_dirty = true;
            return EGL_TRUE;
        case EGL_SMPTE2086_DISPLAY_PRIMARY_GX_EXT:
            egl_smpte2086_metadata.displayPrimaryGreen.x = value;
            egl_smpte2086_dirty = true;
            return EGL_TRUE;
        case EGL_SMPTE2086_DISPLAY_PRIMARY_GY_EXT:
            egl_smpte2086_metadata.displayPrimaryGreen.y = value;
            egl_smpte2086_dirty = true;
            return EGL_TRUE;
        case EGL_SMPTE2086_DISPLAY_PRIMARY_BX_EXT:
            egl_smpte2086_metadata.displayPrimaryBlue.x = value;
            egl_smpte2086_dirty = true;
            return EGL_TRUE;
        case EGL_SMPTE2086_DISPLAY_PRIMARY_BY_EXT:
            egl_smpte2086_metadata.displayPrimaryBlue.y = value;
            egl_smpte2086_dirty = true;
            return EGL_TRUE;
        case EGL_SMPTE2086_WHITE_POINT_X_EXT:
            egl_smpte2086_metadata.whitePoint.x = value;
            egl_smpte2086_dirty = true;
            return EGL_TRUE;
        case EGL_SMPTE2086_WHITE_POINT_Y_EXT:
            egl_smpte2086_metadata.whitePoint.y = value;
            egl_smpte2086_dirty = true;
            return EGL_TRUE;
        case EGL_SMPTE2086_MAX_LUMINANCE_EXT:
            egl_smpte2086_metadata.maxLuminance = value;
            egl_smpte2086_dirty = true;
            return EGL_TRUE;
        case EGL_SMPTE2086_MIN_LUMINANCE_EXT:
            egl_smpte2086_metadata.minLuminance = value;
            egl_smpte2086_dirty = true;
            return EGL_TRUE;
    }
    return EGL_FALSE;
}

EGLBoolean egl_surface_t::setCta8613Attribute(EGLint attribute, EGLint value) {
    switch (attribute) {
        case EGL_CTA861_3_MAX_CONTENT_LIGHT_LEVEL_EXT:
            egl_cta861_3_metadata.maxContentLightLevel = value;
            egl_cta861_3_dirty = true;
            return EGL_TRUE;
        case EGL_CTA861_3_MAX_FRAME_AVERAGE_LEVEL_EXT:
            egl_cta861_3_metadata.maxFrameAverageLightLevel = value;
            egl_cta861_3_dirty = true;
            return EGL_TRUE;
    }
    return EGL_FALSE;
}

EGLBoolean egl_surface_t::getSmpte2086Metadata(android_smpte2086_metadata& metadata) const {
    if (!egl_smpte2086_dirty) return EGL_FALSE;
    if (egl_smpte2086_metadata.displayPrimaryRed.x == EGL_DONT_CARE ||
        egl_smpte2086_metadata.displayPrimaryRed.y == EGL_DONT_CARE ||
        egl_smpte2086_metadata.displayPrimaryGreen.x == EGL_DONT_CARE ||
        egl_smpte2086_metadata.displayPrimaryGreen.y == EGL_DONT_CARE ||
        egl_smpte2086_metadata.displayPrimaryBlue.x == EGL_DONT_CARE ||
        egl_smpte2086_metadata.displayPrimaryBlue.y == EGL_DONT_CARE ||
        egl_smpte2086_metadata.whitePoint.x == EGL_DONT_CARE ||
        egl_smpte2086_metadata.whitePoint.y == EGL_DONT_CARE ||
        egl_smpte2086_metadata.maxLuminance == EGL_DONT_CARE ||
        egl_smpte2086_metadata.minLuminance == EGL_DONT_CARE) {
        ALOGW("egl_surface_t: incomplete SMPTE 2086 metadata!");
        return EGL_FALSE;
    }

    metadata.displayPrimaryRed.x = static_cast<float>(egl_smpte2086_metadata.displayPrimaryRed.x) / EGL_METADATA_SCALING_EXT;
    metadata.displayPrimaryRed.y = static_cast<float>(egl_smpte2086_metadata.displayPrimaryRed.y) / EGL_METADATA_SCALING_EXT;
    metadata.displayPrimaryGreen.x = static_cast<float>(egl_smpte2086_metadata.displayPrimaryGreen.x) / EGL_METADATA_SCALING_EXT;
    metadata.displayPrimaryGreen.y = static_cast<float>(egl_smpte2086_metadata.displayPrimaryGreen.y) / EGL_METADATA_SCALING_EXT;
    metadata.displayPrimaryBlue.x = static_cast<float>(egl_smpte2086_metadata.displayPrimaryBlue.x) / EGL_METADATA_SCALING_EXT;
    metadata.displayPrimaryBlue.y = static_cast<float>(egl_smpte2086_metadata.displayPrimaryBlue.y) / EGL_METADATA_SCALING_EXT;
    metadata.whitePoint.x = static_cast<float>(egl_smpte2086_metadata.whitePoint.x) / EGL_METADATA_SCALING_EXT;
    metadata.whitePoint.y = static_cast<float>(egl_smpte2086_metadata.whitePoint.y) / EGL_METADATA_SCALING_EXT;
    metadata.maxLuminance = static_cast<float>(egl_smpte2086_metadata.maxLuminance) / EGL_METADATA_SCALING_EXT;
    metadata.minLuminance = static_cast<float>(egl_smpte2086_metadata.minLuminance) / EGL_METADATA_SCALING_EXT;

    return EGL_TRUE;
}

EGLBoolean egl_surface_t::getCta8613Metadata(android_cta861_3_metadata& metadata) const {
    if (!egl_cta861_3_dirty) return EGL_FALSE;

    if (egl_cta861_3_metadata.maxContentLightLevel == EGL_DONT_CARE ||
        egl_cta861_3_metadata.maxFrameAverageLightLevel == EGL_DONT_CARE) {
        ALOGW("egl_surface_t: incomplete CTA861.3 metadata!");
        return EGL_FALSE;
    }

    metadata.maxContentLightLevel = static_cast<float>(egl_cta861_3_metadata.maxContentLightLevel) / EGL_METADATA_SCALING_EXT;
    metadata.maxFrameAverageLightLevel = static_cast<float>(egl_cta861_3_metadata.maxFrameAverageLightLevel) / EGL_METADATA_SCALING_EXT;

    return EGL_TRUE;
}


EGLBoolean egl_surface_t::getColorSpaceAttribute(EGLint attribute, EGLint* value) const {
    if (attribute == EGL_GL_COLORSPACE_KHR) {
        *value = colorSpace;
        return EGL_TRUE;
    }
    return EGL_FALSE;
}

EGLBoolean egl_surface_t::getSmpte2086Attribute(EGLint attribute, EGLint *value) const {
    switch (attribute) {
        case EGL_SMPTE2086_DISPLAY_PRIMARY_RX_EXT:
            *value = egl_smpte2086_metadata.displayPrimaryRed.x;
            return EGL_TRUE;
            break;
        case EGL_SMPTE2086_DISPLAY_PRIMARY_RY_EXT:
            *value = egl_smpte2086_metadata.displayPrimaryRed.y;
            return EGL_TRUE;
            break;
        case EGL_SMPTE2086_DISPLAY_PRIMARY_GX_EXT:
            *value = egl_smpte2086_metadata.displayPrimaryGreen.x;
            return EGL_TRUE;
            break;
        case EGL_SMPTE2086_DISPLAY_PRIMARY_GY_EXT:
            *value = egl_smpte2086_metadata.displayPrimaryGreen.y;
            return EGL_TRUE;
            break;
        case EGL_SMPTE2086_DISPLAY_PRIMARY_BX_EXT:
            *value = egl_smpte2086_metadata.displayPrimaryBlue.x;
            return EGL_TRUE;
            break;
        case EGL_SMPTE2086_DISPLAY_PRIMARY_BY_EXT:
            *value = egl_smpte2086_metadata.displayPrimaryBlue.y;
            return EGL_TRUE;
            break;
        case EGL_SMPTE2086_WHITE_POINT_X_EXT:
            *value = egl_smpte2086_metadata.whitePoint.x;
            return EGL_TRUE;
            break;
        case EGL_SMPTE2086_WHITE_POINT_Y_EXT:
            *value = egl_smpte2086_metadata.whitePoint.y;
            return EGL_TRUE;
            break;
        case EGL_SMPTE2086_MAX_LUMINANCE_EXT:
            *value = egl_smpte2086_metadata.maxLuminance;
            return EGL_TRUE;
            break;
        case EGL_SMPTE2086_MIN_LUMINANCE_EXT:
            *value = egl_smpte2086_metadata.minLuminance;
            return EGL_TRUE;
            break;
    }
    return EGL_FALSE;
}

EGLBoolean egl_surface_t::getCta8613Attribute(EGLint attribute, EGLint *value) const {
    switch (attribute) {
        case EGL_CTA861_3_MAX_CONTENT_LIGHT_LEVEL_EXT:
            *value = egl_cta861_3_metadata.maxContentLightLevel;
            return EGL_TRUE;
            break;
        case EGL_CTA861_3_MAX_FRAME_AVERAGE_LEVEL_EXT:
            *value = egl_cta861_3_metadata.maxFrameAverageLightLevel;
            return EGL_TRUE;
            break;
    }
    return EGL_FALSE;
}

void egl_surface_t::terminate() {
    disconnect();
    egl_object_t::terminate();
}

// ----------------------------------------------------------------------------

egl_context_t::egl_context_t(EGLDisplay dpy, EGLContext context, EGLConfig config,
        egl_connection_t const* cnx, int version) :
    egl_object_t(get_display_nowake(dpy)), dpy(dpy), context(context),
            config(config), read(nullptr), draw(nullptr), cnx(cnx), version(version) {
}

void egl_context_t::onLooseCurrent() {
    read = nullptr;
    draw = nullptr;
}

void egl_context_t::onMakeCurrent(EGLSurface draw, EGLSurface read) {
    this->read = read;
    this->draw = draw;

    /*
     * Here we cache the GL_EXTENSIONS string for this context and we
     * add the extensions always handled by the wrapper
     */

    if (gl_extensions.empty()) {
        // call the implementation's glGetString(GL_EXTENSIONS)
        const char* exts = (const char *)gEGLImpl.hooks[version]->gl.glGetString(GL_EXTENSIONS);

        // If this context is sharing with another context, and the other context was reset
        // e.g. due to robustness failure, this context might also be reset and glGetString can
        // return NULL.
        if (exts) {
            gl_extensions = exts;
            if (gl_extensions.find("GL_EXT_debug_marker") == std::string::npos) {
                gl_extensions.insert(0, "GL_EXT_debug_marker ");
            }

            // tokenize the supported extensions for the glGetStringi() wrapper
            std::stringstream ss;
            std::string str;
            ss << gl_extensions;
            while (ss >> str) {
                tokenized_gl_extensions.push_back(str);
            }
        }
    }
}

// ----------------------------------------------------------------------------
}; // namespace android
// ----------------------------------------------------------------------------
