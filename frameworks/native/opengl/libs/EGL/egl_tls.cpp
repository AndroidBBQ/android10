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

#include "egl_tls.h"

#include <stdlib.h>

#include <cutils/properties.h>
#include <log/log.h>
#include "CallStack.h"
#include "egl_platform_entries.h"

namespace android {

pthread_key_t egl_tls_t::sKey = TLS_KEY_NOT_INITIALIZED;
pthread_once_t egl_tls_t::sOnceKey = PTHREAD_ONCE_INIT;

egl_tls_t::egl_tls_t()
    : error(EGL_SUCCESS), ctx(nullptr), logCallWithNoContext(true) {
}

const char *egl_tls_t::egl_strerror(EGLint err) {
    switch (err) {
        case EGL_SUCCESS:               return "EGL_SUCCESS";
        case EGL_NOT_INITIALIZED:       return "EGL_NOT_INITIALIZED";
        case EGL_BAD_ACCESS:            return "EGL_BAD_ACCESS";
        case EGL_BAD_ALLOC:             return "EGL_BAD_ALLOC";
        case EGL_BAD_ATTRIBUTE:         return "EGL_BAD_ATTRIBUTE";
        case EGL_BAD_CONFIG:            return "EGL_BAD_CONFIG";
        case EGL_BAD_CONTEXT:           return "EGL_BAD_CONTEXT";
        case EGL_BAD_CURRENT_SURFACE:   return "EGL_BAD_CURRENT_SURFACE";
        case EGL_BAD_DISPLAY:           return "EGL_BAD_DISPLAY";
        case EGL_BAD_MATCH:             return "EGL_BAD_MATCH";
        case EGL_BAD_NATIVE_PIXMAP:     return "EGL_BAD_NATIVE_PIXMAP";
        case EGL_BAD_NATIVE_WINDOW:     return "EGL_BAD_NATIVE_WINDOW";
        case EGL_BAD_PARAMETER:         return "EGL_BAD_PARAMETER";
        case EGL_BAD_SURFACE:           return "EGL_BAD_SURFACE";
        case EGL_CONTEXT_LOST:          return "EGL_CONTEXT_LOST";
        default: return "UNKNOWN";
    }
}

void egl_tls_t::validateTLSKey()
{
    struct TlsKeyInitializer {
        static void create() { pthread_key_create(&sKey, destructTLSData); }
    };
    pthread_once(&sOnceKey, TlsKeyInitializer::create);
}

void egl_tls_t::destructTLSData(void* data) {
    egl_tls_t* tls = static_cast<egl_tls_t*>(data);
    if (!tls) return;

    // Several things in the call tree of eglReleaseThread expect to be able to get the current
    // thread state directly from TLS. That's a problem because Bionic has already cleared our
    // TLS pointer before calling this function (pthread_getspecific(sKey) will return nullptr).
    // Instead the data is passed as our parameter.
    //
    // Ideally we'd refactor this so we have thin wrappers that retrieve thread state from TLS and
    // then pass it as a parameter (or 'this' pointer) to functions that do the real work without
    // touching TLS. Then from here we could just call those implementation functions with the the
    // TLS data we just received as a parameter.
    //
    // But that's a fairly invasive refactoring, so to do this robustly in the short term we just
    // put the data *back* in TLS and call the top-level eglReleaseThread. It and it's call tree
    // will retrieve the value from TLS, and then finally clear the TLS data. Bionic explicitly
    // tolerates re-setting the value that it's currently trying to destruct (see
    // pthread_key_clean_all()). Even if we forgot to clear the restored TLS data, bionic would
    // call the destructor again, but eventually gives up and just leaks the data rather than
    // enter an infinite loop.
    pthread_setspecific(sKey, tls);
    eglReleaseThread();
    ALOGE_IF(pthread_getspecific(sKey) != nullptr,
             "EGL TLS data still exists after eglReleaseThread");
}

void egl_tls_t::setErrorEtcImpl(
        const char* caller, int line, EGLint error, bool quiet) {
    validateTLSKey();
    egl_tls_t* tls = getTLS();
    if (tls->error != error) {
        if (!quiet) {
            ALOGE("%s:%d error %x (%s)",
                    caller, line, error, egl_strerror(error));
            char value[PROPERTY_VALUE_MAX];
            property_get("debug.egl.callstack", value, "0");
            if (atoi(value)) {
                CallStack::log(LOG_TAG);
            }
        }
        tls->error = error;
    }
}

bool egl_tls_t::logNoContextCall() {
    egl_tls_t* tls = getTLS();
    if (tls->logCallWithNoContext) {
        tls->logCallWithNoContext = false;
        return true;
    }
    return false;

}

egl_tls_t* egl_tls_t::getTLS() {
    egl_tls_t* tls = (egl_tls_t*)pthread_getspecific(sKey);
    if (tls == nullptr) {
        tls = new egl_tls_t;
        pthread_setspecific(sKey, tls);
    }
    return tls;
}

void egl_tls_t::clearTLS() {
    if (sKey != TLS_KEY_NOT_INITIALIZED) {
        egl_tls_t* tls = (egl_tls_t*)pthread_getspecific(sKey);
        if (tls) {
            pthread_setspecific(sKey, nullptr);
            delete tls;
        }
    }
}

void egl_tls_t::clearError() {
    // This must clear the error from all the underlying EGL implementations as
    // well as the EGL wrapper layer.
    android::eglGetErrorImpl();
}

EGLint egl_tls_t::getError() {
    if (sKey == TLS_KEY_NOT_INITIALIZED) {
        return EGL_SUCCESS;
    }
    egl_tls_t* tls = (egl_tls_t*)pthread_getspecific(sKey);
    if (!tls) {
        return EGL_SUCCESS;
    }
    EGLint error = tls->error;
    tls->error = EGL_SUCCESS;
    return error;
}

void egl_tls_t::setContext(EGLContext ctx) {
    validateTLSKey();
    getTLS()->ctx = ctx;
}

EGLContext egl_tls_t::getContext() {
    if (sKey == TLS_KEY_NOT_INITIALIZED) {
        return EGL_NO_CONTEXT;
    }
    egl_tls_t* tls = (egl_tls_t *)pthread_getspecific(sKey);
    if (!tls) return EGL_NO_CONTEXT;
    return tls->ctx;
}


} // namespace android
