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

#include "egl_cache.h"

#include "../egl_impl.h"

#include "egl_display.h"

#include <private/EGL/cache.h>

#include <unistd.h>

#include <thread>

#include <log/log.h>

// Cache size limits.
static const size_t maxKeySize = 12 * 1024;
static const size_t maxValueSize = 64 * 1024;
static const size_t maxTotalSize = 2 * 1024 * 1024;

// The time in seconds to wait before saving newly inserted cache entries.
static const unsigned int deferredSaveDelay = 4;

// ----------------------------------------------------------------------------
namespace android {
// ----------------------------------------------------------------------------

#define BC_EXT_STR "EGL_ANDROID_blob_cache"

// called from android_view_ThreadedRenderer.cpp
void egl_set_cache_filename(const char* filename) {
    egl_cache_t::get()->setCacheFilename(filename);
}

//
// Callback functions passed to EGL.
//
static void setBlob(const void* key, EGLsizeiANDROID keySize,
        const void* value, EGLsizeiANDROID valueSize) {
    egl_cache_t::get()->setBlob(key, keySize, value, valueSize);
}

static EGLsizeiANDROID getBlob(const void* key, EGLsizeiANDROID keySize,
        void* value, EGLsizeiANDROID valueSize) {
    return egl_cache_t::get()->getBlob(key, keySize, value, valueSize);
}

//
// egl_cache_t definition
//
egl_cache_t::egl_cache_t() :
        mInitialized(false) {
}

egl_cache_t::~egl_cache_t() {
}

egl_cache_t egl_cache_t::sCache;

egl_cache_t* egl_cache_t::get() {
    return &sCache;
}

void egl_cache_t::initialize(egl_display_t *display) {
    std::lock_guard<std::mutex> lock(mMutex);

    egl_connection_t* const cnx = &gEGLImpl;
    if (cnx->dso && cnx->major >= 0 && cnx->minor >= 0) {
        const char* exts = display->disp.queryString.extensions;
        size_t bcExtLen = strlen(BC_EXT_STR);
        size_t extsLen = strlen(exts);
        bool equal = !strcmp(BC_EXT_STR, exts);
        bool atStart = !strncmp(BC_EXT_STR " ", exts, bcExtLen+1);
        bool atEnd = (bcExtLen+1) < extsLen &&
                !strcmp(" " BC_EXT_STR, exts + extsLen - (bcExtLen+1));
        bool inMiddle = strstr(exts, " " BC_EXT_STR " ") != nullptr;
        if (equal || atStart || atEnd || inMiddle) {
            PFNEGLSETBLOBCACHEFUNCSANDROIDPROC eglSetBlobCacheFuncsANDROID;
            eglSetBlobCacheFuncsANDROID =
                    reinterpret_cast<PFNEGLSETBLOBCACHEFUNCSANDROIDPROC>(
                            cnx->egl.eglGetProcAddress(
                                    "eglSetBlobCacheFuncsANDROID"));
            if (eglSetBlobCacheFuncsANDROID == nullptr) {
                ALOGE("EGL_ANDROID_blob_cache advertised, "
                        "but unable to get eglSetBlobCacheFuncsANDROID");
                return;
            }

            eglSetBlobCacheFuncsANDROID(display->disp.dpy,
                    android::setBlob, android::getBlob);
            EGLint err = cnx->egl.eglGetError();
            if (err != EGL_SUCCESS) {
                ALOGE("eglSetBlobCacheFuncsANDROID resulted in an error: "
                        "%#x", err);
            }
        }
    }

    mInitialized = true;
}

void egl_cache_t::terminate() {
    std::lock_guard<std::mutex> lock(mMutex);
    if (mBlobCache) {
        mBlobCache->writeToFile();
    }
    mBlobCache = nullptr;
}

void egl_cache_t::setBlob(const void* key, EGLsizeiANDROID keySize,
        const void* value, EGLsizeiANDROID valueSize) {
    std::lock_guard<std::mutex> lock(mMutex);

    if (keySize < 0 || valueSize < 0) {
        ALOGW("EGL_ANDROID_blob_cache set: negative sizes are not allowed");
        return;
    }

    if (mInitialized) {
        BlobCache* bc = getBlobCacheLocked();
        bc->set(key, keySize, value, valueSize);

        if (!mSavePending) {
            mSavePending = true;
            std::thread deferredSaveThread([this]() {
                sleep(deferredSaveDelay);
                std::lock_guard<std::mutex> lock(mMutex);
                if (mInitialized && mBlobCache) {
                    mBlobCache->writeToFile();
                }
                mSavePending = false;
            });
            deferredSaveThread.detach();
        }
    }
}

EGLsizeiANDROID egl_cache_t::getBlob(const void* key, EGLsizeiANDROID keySize,
        void* value, EGLsizeiANDROID valueSize) {
    std::lock_guard<std::mutex> lock(mMutex);

    if (keySize < 0 || valueSize < 0) {
        ALOGW("EGL_ANDROID_blob_cache set: negative sizes are not allowed");
        return 0;
    }

    if (mInitialized) {
        BlobCache* bc = getBlobCacheLocked();
        return bc->get(key, keySize, value, valueSize);
    }
    return 0;
}

void egl_cache_t::setCacheFilename(const char* filename) {
    std::lock_guard<std::mutex> lock(mMutex);
    mFilename = filename;
}

BlobCache* egl_cache_t::getBlobCacheLocked() {
    if (mBlobCache == nullptr) {
        mBlobCache.reset(new FileBlobCache(maxKeySize, maxValueSize, maxTotalSize, mFilename));
    }
    return mBlobCache.get();
}

// ----------------------------------------------------------------------------
}; // namespace android
// ----------------------------------------------------------------------------
