/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "GLConsumer"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
//#define LOG_NDEBUG 0

#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES

#include <inttypes.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <cutils/compiler.h>

#include <hardware/hardware.h>

#include <math/mat4.h>

#include <gui/BufferItem.h>
#include <gui/DebugEGLImageTracker.h>
#include <gui/GLConsumer.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>

#include <private/gui/ComposerService.h>
#include <private/gui/SyncFeatures.h>

#include <utils/Log.h>
#include <utils/String8.h>
#include <utils/Trace.h>

extern "C" EGLAPI const char* eglQueryStringImplementationANDROID(EGLDisplay dpy, EGLint name);
#define PROT_CONTENT_EXT_STR "EGL_EXT_protected_content"
#define EGL_PROTECTED_CONTENT_EXT 0x32C0

namespace android {

// Macros for including the GLConsumer name in log messages
#define GLC_LOGV(x, ...) ALOGV("[%s] " x, mName.string(), ##__VA_ARGS__)
#define GLC_LOGD(x, ...) ALOGD("[%s] " x, mName.string(), ##__VA_ARGS__)
//#define GLC_LOGI(x, ...) ALOGI("[%s] " x, mName.string(), ##__VA_ARGS__)
#define GLC_LOGW(x, ...) ALOGW("[%s] " x, mName.string(), ##__VA_ARGS__)
#define GLC_LOGE(x, ...) ALOGE("[%s] " x, mName.string(), ##__VA_ARGS__)

static const struct {
    uint32_t width, height;
    char const* bits;
} kDebugData = { 15, 12,
    "_______________"
    "_______________"
    "_____XX_XX_____"
    "__X_X_____X_X__"
    "__X_XXXXXXX_X__"
    "__XXXXXXXXXXX__"
    "___XX_XXX_XX___"
    "____XXXXXXX____"
    "_____X___X_____"
    "____X_____X____"
    "_______________"
    "_______________"
};

static const mat4 mtxIdentity;

Mutex GLConsumer::sStaticInitLock;
sp<GraphicBuffer> GLConsumer::sReleasedTexImageBuffer;

static bool hasEglProtectedContentImpl() {
    EGLDisplay dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    const char* exts = eglQueryString(dpy, EGL_EXTENSIONS);
    size_t cropExtLen = strlen(PROT_CONTENT_EXT_STR);
    size_t extsLen = strlen(exts);
    bool equal = !strcmp(PROT_CONTENT_EXT_STR, exts);
    bool atStart = !strncmp(PROT_CONTENT_EXT_STR " ", exts, cropExtLen+1);
    bool atEnd = (cropExtLen+1) < extsLen &&
            !strcmp(" " PROT_CONTENT_EXT_STR, exts + extsLen - (cropExtLen+1));
    bool inMiddle = strstr(exts, " " PROT_CONTENT_EXT_STR " ");
    return equal || atStart || atEnd || inMiddle;
}

static bool hasEglProtectedContent() {
    // Only compute whether the extension is present once the first time this
    // function is called.
    static bool hasIt = hasEglProtectedContentImpl();
    return hasIt;
}

GLConsumer::GLConsumer(const sp<IGraphicBufferConsumer>& bq, uint32_t tex,
        uint32_t texTarget, bool useFenceSync, bool isControlledByApp) :
    ConsumerBase(bq, isControlledByApp),
    mCurrentCrop(Rect::EMPTY_RECT),
    mCurrentTransform(0),
    mCurrentScalingMode(NATIVE_WINDOW_SCALING_MODE_FREEZE),
    mCurrentFence(Fence::NO_FENCE),
    mCurrentTimestamp(0),
    mCurrentDataSpace(HAL_DATASPACE_UNKNOWN),
    mCurrentFrameNumber(0),
    mDefaultWidth(1),
    mDefaultHeight(1),
    mFilteringEnabled(true),
    mTexName(tex),
    mUseFenceSync(useFenceSync),
    mTexTarget(texTarget),
    mEglDisplay(EGL_NO_DISPLAY),
    mEglContext(EGL_NO_CONTEXT),
    mCurrentTexture(BufferQueue::INVALID_BUFFER_SLOT),
    mAttached(true)
{
    GLC_LOGV("GLConsumer");

    memcpy(mCurrentTransformMatrix, mtxIdentity.asArray(),
            sizeof(mCurrentTransformMatrix));

    mConsumer->setConsumerUsageBits(DEFAULT_USAGE_FLAGS);
}

GLConsumer::GLConsumer(const sp<IGraphicBufferConsumer>& bq, uint32_t texTarget,
        bool useFenceSync, bool isControlledByApp) :
    ConsumerBase(bq, isControlledByApp),
    mCurrentCrop(Rect::EMPTY_RECT),
    mCurrentTransform(0),
    mCurrentScalingMode(NATIVE_WINDOW_SCALING_MODE_FREEZE),
    mCurrentFence(Fence::NO_FENCE),
    mCurrentTimestamp(0),
    mCurrentDataSpace(HAL_DATASPACE_UNKNOWN),
    mCurrentFrameNumber(0),
    mDefaultWidth(1),
    mDefaultHeight(1),
    mFilteringEnabled(true),
    mTexName(0),
    mUseFenceSync(useFenceSync),
    mTexTarget(texTarget),
    mEglDisplay(EGL_NO_DISPLAY),
    mEglContext(EGL_NO_CONTEXT),
    mCurrentTexture(BufferQueue::INVALID_BUFFER_SLOT),
    mAttached(false)
{
    GLC_LOGV("GLConsumer");

    memcpy(mCurrentTransformMatrix, mtxIdentity.asArray(),
            sizeof(mCurrentTransformMatrix));

    mConsumer->setConsumerUsageBits(DEFAULT_USAGE_FLAGS);
}

status_t GLConsumer::setDefaultBufferSize(uint32_t w, uint32_t h)
{
    Mutex::Autolock lock(mMutex);
    if (mAbandoned) {
        GLC_LOGE("setDefaultBufferSize: GLConsumer is abandoned!");
        return NO_INIT;
    }
    mDefaultWidth = w;
    mDefaultHeight = h;
    return mConsumer->setDefaultBufferSize(w, h);
}

status_t GLConsumer::updateTexImage() {
    ATRACE_CALL();
    GLC_LOGV("updateTexImage");
    Mutex::Autolock lock(mMutex);

    if (mAbandoned) {
        GLC_LOGE("updateTexImage: GLConsumer is abandoned!");
        return NO_INIT;
    }

    // Make sure the EGL state is the same as in previous calls.
    status_t err = checkAndUpdateEglStateLocked();
    if (err != NO_ERROR) {
        return err;
    }

    BufferItem item;

    // Acquire the next buffer.
    // In asynchronous mode the list is guaranteed to be one buffer
    // deep, while in synchronous mode we use the oldest buffer.
    err = acquireBufferLocked(&item, 0);
    if (err != NO_ERROR) {
        if (err == BufferQueue::NO_BUFFER_AVAILABLE) {
            // We always bind the texture even if we don't update its contents.
            GLC_LOGV("updateTexImage: no buffers were available");
            glBindTexture(mTexTarget, mTexName);
            err = NO_ERROR;
        } else {
            GLC_LOGE("updateTexImage: acquire failed: %s (%d)",
                strerror(-err), err);
        }
        return err;
    }

    // Release the previous buffer.
    err = updateAndReleaseLocked(item);
    if (err != NO_ERROR) {
        // We always bind the texture.
        glBindTexture(mTexTarget, mTexName);
        return err;
    }

    // Bind the new buffer to the GL texture, and wait until it's ready.
    return bindTextureImageLocked();
}


status_t GLConsumer::releaseTexImage() {
    ATRACE_CALL();
    GLC_LOGV("releaseTexImage");
    Mutex::Autolock lock(mMutex);

    if (mAbandoned) {
        GLC_LOGE("releaseTexImage: GLConsumer is abandoned!");
        return NO_INIT;
    }

    // Make sure the EGL state is the same as in previous calls.
    status_t err = NO_ERROR;

    if (mAttached) {
        err = checkAndUpdateEglStateLocked(true);
        if (err != NO_ERROR) {
            return err;
        }
    } else {
        // if we're detached, no need to validate EGL's state -- we won't use it.
    }

    // Update the GLConsumer state.
    int buf = mCurrentTexture;
    if (buf != BufferQueue::INVALID_BUFFER_SLOT) {

        GLC_LOGV("releaseTexImage: (slot=%d, mAttached=%d)", buf, mAttached);

        if (mAttached) {
            // Do whatever sync ops we need to do before releasing the slot.
            err = syncForReleaseLocked(mEglDisplay);
            if (err != NO_ERROR) {
                GLC_LOGE("syncForReleaseLocked failed (slot=%d), err=%d", buf, err);
                return err;
            }
        } else {
            // if we're detached, we just use the fence that was created in detachFromContext()
            // so... basically, nothing more to do here.
        }

        err = releaseBufferLocked(buf, mSlots[buf].mGraphicBuffer, mEglDisplay, EGL_NO_SYNC_KHR);
        if (err < NO_ERROR) {
            GLC_LOGE("releaseTexImage: failed to release buffer: %s (%d)",
                    strerror(-err), err);
            return err;
        }

        if (mReleasedTexImage == nullptr) {
            mReleasedTexImage = new EglImage(getDebugTexImageBuffer());
        }

        mCurrentTexture = BufferQueue::INVALID_BUFFER_SLOT;
        mCurrentTextureImage = mReleasedTexImage;
        mCurrentCrop.makeInvalid();
        mCurrentTransform = 0;
        mCurrentTimestamp = 0;
        mCurrentDataSpace = HAL_DATASPACE_UNKNOWN;
        mCurrentFence = Fence::NO_FENCE;
        mCurrentFenceTime = FenceTime::NO_FENCE;

        if (mAttached) {
            // This binds a dummy buffer (mReleasedTexImage).
            status_t result = bindTextureImageLocked();
            if (result != NO_ERROR) {
                return result;
            }
        } else {
            // detached, don't touch the texture (and we may not even have an
            // EGLDisplay here.
        }
    }

    return NO_ERROR;
}

sp<GraphicBuffer> GLConsumer::getDebugTexImageBuffer() {
    Mutex::Autolock _l(sStaticInitLock);
    if (CC_UNLIKELY(sReleasedTexImageBuffer == nullptr)) {
        // The first time, create the debug texture in case the application
        // continues to use it.
        sp<GraphicBuffer> buffer = new GraphicBuffer(
                kDebugData.width, kDebugData.height, PIXEL_FORMAT_RGBA_8888,
                GraphicBuffer::USAGE_SW_WRITE_RARELY,
                "[GLConsumer debug texture]");
        uint32_t* bits;
        buffer->lock(GraphicBuffer::USAGE_SW_WRITE_RARELY, reinterpret_cast<void**>(&bits));
        uint32_t stride = buffer->getStride();
        uint32_t height = buffer->getHeight();
        memset(bits, 0, stride * height * 4);
        for (uint32_t y = 0; y < kDebugData.height; y++) {
            for (uint32_t x = 0; x < kDebugData.width; x++) {
                bits[x] = (kDebugData.bits[y + kDebugData.width + x] == 'X') ?
                    0xFF000000 : 0xFFFFFFFF;
            }
            bits += stride;
        }
        buffer->unlock();
        sReleasedTexImageBuffer = buffer;
    }
    return sReleasedTexImageBuffer;
}

status_t GLConsumer::acquireBufferLocked(BufferItem *item,
        nsecs_t presentWhen, uint64_t maxFrameNumber) {
    status_t err = ConsumerBase::acquireBufferLocked(item, presentWhen,
            maxFrameNumber);
    if (err != NO_ERROR) {
        return err;
    }

    // If item->mGraphicBuffer is not null, this buffer has not been acquired
    // before, so any prior EglImage created is using a stale buffer. This
    // replaces any old EglImage with a new one (using the new buffer).
    if (item->mGraphicBuffer != nullptr) {
        int slot = item->mSlot;
        mEglSlots[slot].mEglImage = new EglImage(item->mGraphicBuffer);
    }

    return NO_ERROR;
}

status_t GLConsumer::releaseBufferLocked(int buf,
        sp<GraphicBuffer> graphicBuffer,
        EGLDisplay display, EGLSyncKHR eglFence) {
    // release the buffer if it hasn't already been discarded by the
    // BufferQueue. This can happen, for example, when the producer of this
    // buffer has reallocated the original buffer slot after this buffer
    // was acquired.
    status_t err = ConsumerBase::releaseBufferLocked(
            buf, graphicBuffer, display, eglFence);
    mEglSlots[buf].mEglFence = EGL_NO_SYNC_KHR;
    return err;
}

status_t GLConsumer::updateAndReleaseLocked(const BufferItem& item,
        PendingRelease* pendingRelease)
{
    status_t err = NO_ERROR;

    int slot = item.mSlot;

    if (!mAttached) {
        GLC_LOGE("updateAndRelease: GLConsumer is not attached to an OpenGL "
                "ES context");
        releaseBufferLocked(slot, mSlots[slot].mGraphicBuffer,
                mEglDisplay, EGL_NO_SYNC_KHR);
        return INVALID_OPERATION;
    }

    // Confirm state.
    err = checkAndUpdateEglStateLocked();
    if (err != NO_ERROR) {
        releaseBufferLocked(slot, mSlots[slot].mGraphicBuffer,
                mEglDisplay, EGL_NO_SYNC_KHR);
        return err;
    }

    // Ensure we have a valid EglImageKHR for the slot, creating an EglImage
    // if nessessary, for the gralloc buffer currently in the slot in
    // ConsumerBase.
    // We may have to do this even when item.mGraphicBuffer == NULL (which
    // means the buffer was previously acquired).
    err = mEglSlots[slot].mEglImage->createIfNeeded(mEglDisplay);
    if (err != NO_ERROR) {
        GLC_LOGW("updateAndRelease: unable to createImage on display=%p slot=%d",
                mEglDisplay, slot);
        releaseBufferLocked(slot, mSlots[slot].mGraphicBuffer,
                mEglDisplay, EGL_NO_SYNC_KHR);
        return UNKNOWN_ERROR;
    }

    // Do whatever sync ops we need to do before releasing the old slot.
    if (slot != mCurrentTexture) {
        err = syncForReleaseLocked(mEglDisplay);
        if (err != NO_ERROR) {
            // Release the buffer we just acquired.  It's not safe to
            // release the old buffer, so instead we just drop the new frame.
            // As we are still under lock since acquireBuffer, it is safe to
            // release by slot.
            releaseBufferLocked(slot, mSlots[slot].mGraphicBuffer,
                    mEglDisplay, EGL_NO_SYNC_KHR);
            return err;
        }
    }

    GLC_LOGV("updateAndRelease: (slot=%d buf=%p) -> (slot=%d buf=%p)",
            mCurrentTexture, mCurrentTextureImage != nullptr ?
                    mCurrentTextureImage->graphicBufferHandle() : nullptr,
            slot, mSlots[slot].mGraphicBuffer->handle);

    // Hang onto the pointer so that it isn't freed in the call to
    // releaseBufferLocked() if we're in shared buffer mode and both buffers are
    // the same.
    sp<EglImage> nextTextureImage = mEglSlots[slot].mEglImage;

    // release old buffer
    if (mCurrentTexture != BufferQueue::INVALID_BUFFER_SLOT) {
        if (pendingRelease == nullptr) {
            status_t status = releaseBufferLocked(
                    mCurrentTexture, mCurrentTextureImage->graphicBuffer(),
                    mEglDisplay, mEglSlots[mCurrentTexture].mEglFence);
            if (status < NO_ERROR) {
                GLC_LOGE("updateAndRelease: failed to release buffer: %s (%d)",
                        strerror(-status), status);
                err = status;
                // keep going, with error raised [?]
            }
        } else {
            pendingRelease->currentTexture = mCurrentTexture;
            pendingRelease->graphicBuffer =
                    mCurrentTextureImage->graphicBuffer();
            pendingRelease->display = mEglDisplay;
            pendingRelease->fence = mEglSlots[mCurrentTexture].mEglFence;
            pendingRelease->isPending = true;
        }
    }

    // Update the GLConsumer state.
    mCurrentTexture = slot;
    mCurrentTextureImage = nextTextureImage;
    mCurrentCrop = item.mCrop;
    mCurrentTransform = item.mTransform;
    mCurrentScalingMode = item.mScalingMode;
    mCurrentTimestamp = item.mTimestamp;
    mCurrentDataSpace = item.mDataSpace;
    mCurrentFence = item.mFence;
    mCurrentFenceTime = item.mFenceTime;
    mCurrentFrameNumber = item.mFrameNumber;

    computeCurrentTransformMatrixLocked();

    return err;
}

status_t GLConsumer::bindTextureImageLocked() {
    if (mEglDisplay == EGL_NO_DISPLAY) {
        ALOGE("bindTextureImage: invalid display");
        return INVALID_OPERATION;
    }

    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        GLC_LOGW("bindTextureImage: clearing GL error: %#04x", error);
    }

    glBindTexture(mTexTarget, mTexName);
    if (mCurrentTexture == BufferQueue::INVALID_BUFFER_SLOT &&
            mCurrentTextureImage == nullptr) {
        GLC_LOGE("bindTextureImage: no currently-bound texture");
        return NO_INIT;
    }

    status_t err = mCurrentTextureImage->createIfNeeded(mEglDisplay);
    if (err != NO_ERROR) {
        GLC_LOGW("bindTextureImage: can't create image on display=%p slot=%d",
                mEglDisplay, mCurrentTexture);
        return UNKNOWN_ERROR;
    }
    mCurrentTextureImage->bindToTextureTarget(mTexTarget);

    // In the rare case that the display is terminated and then initialized
    // again, we can't detect that the display changed (it didn't), but the
    // image is invalid. In this case, repeat the exact same steps while
    // forcing the creation of a new image.
    if ((error = glGetError()) != GL_NO_ERROR) {
        glBindTexture(mTexTarget, mTexName);
        status_t result = mCurrentTextureImage->createIfNeeded(mEglDisplay, true);
        if (result != NO_ERROR) {
            GLC_LOGW("bindTextureImage: can't create image on display=%p slot=%d",
                    mEglDisplay, mCurrentTexture);
            return UNKNOWN_ERROR;
        }
        mCurrentTextureImage->bindToTextureTarget(mTexTarget);
        if ((error = glGetError()) != GL_NO_ERROR) {
            GLC_LOGE("bindTextureImage: error binding external image: %#04x", error);
            return UNKNOWN_ERROR;
        }
    }

    // Wait for the new buffer to be ready.
    return doGLFenceWaitLocked();
}

status_t GLConsumer::checkAndUpdateEglStateLocked(bool contextCheck) {
    EGLDisplay dpy = eglGetCurrentDisplay();
    EGLContext ctx = eglGetCurrentContext();

    if (!contextCheck) {
        // if this is the first time we're called, mEglDisplay/mEglContext have
        // never been set, so don't error out (below).
        if (mEglDisplay == EGL_NO_DISPLAY) {
            mEglDisplay = dpy;
        }
        if (mEglContext == EGL_NO_CONTEXT) {
            mEglContext = ctx;
        }
    }

    if (mEglDisplay != dpy || dpy == EGL_NO_DISPLAY) {
        GLC_LOGE("checkAndUpdateEglState: invalid current EGLDisplay");
        return INVALID_OPERATION;
    }

    if (mEglContext != ctx || ctx == EGL_NO_CONTEXT) {
        GLC_LOGE("checkAndUpdateEglState: invalid current EGLContext");
        return INVALID_OPERATION;
    }

    mEglDisplay = dpy;
    mEglContext = ctx;
    return NO_ERROR;
}

void GLConsumer::setReleaseFence(const sp<Fence>& fence) {
    if (fence->isValid() &&
            mCurrentTexture != BufferQueue::INVALID_BUFFER_SLOT) {
        status_t err = addReleaseFence(mCurrentTexture,
                mCurrentTextureImage->graphicBuffer(), fence);
        if (err != OK) {
            GLC_LOGE("setReleaseFence: failed to add the fence: %s (%d)",
                    strerror(-err), err);
        }
    }
}

status_t GLConsumer::detachFromContext() {
    ATRACE_CALL();
    GLC_LOGV("detachFromContext");
    Mutex::Autolock lock(mMutex);

    if (mAbandoned) {
        GLC_LOGE("detachFromContext: abandoned GLConsumer");
        return NO_INIT;
    }

    if (!mAttached) {
        GLC_LOGE("detachFromContext: GLConsumer is not attached to a "
                "context");
        return INVALID_OPERATION;
    }

    EGLDisplay dpy = eglGetCurrentDisplay();
    EGLContext ctx = eglGetCurrentContext();

    if (mEglDisplay != dpy && mEglDisplay != EGL_NO_DISPLAY) {
        GLC_LOGE("detachFromContext: invalid current EGLDisplay");
        return INVALID_OPERATION;
    }

    if (mEglContext != ctx && mEglContext != EGL_NO_CONTEXT) {
        GLC_LOGE("detachFromContext: invalid current EGLContext");
        return INVALID_OPERATION;
    }

    if (dpy != EGL_NO_DISPLAY && ctx != EGL_NO_CONTEXT) {
        status_t err = syncForReleaseLocked(dpy);
        if (err != OK) {
            return err;
        }

        glDeleteTextures(1, &mTexName);
    }

    mEglDisplay = EGL_NO_DISPLAY;
    mEglContext = EGL_NO_CONTEXT;
    mAttached = false;

    return OK;
}

status_t GLConsumer::attachToContext(uint32_t tex) {
    ATRACE_CALL();
    GLC_LOGV("attachToContext");
    Mutex::Autolock lock(mMutex);

    if (mAbandoned) {
        GLC_LOGE("attachToContext: abandoned GLConsumer");
        return NO_INIT;
    }

    if (mAttached) {
        GLC_LOGE("attachToContext: GLConsumer is already attached to a "
                "context");
        return INVALID_OPERATION;
    }

    EGLDisplay dpy = eglGetCurrentDisplay();
    EGLContext ctx = eglGetCurrentContext();

    if (dpy == EGL_NO_DISPLAY) {
        GLC_LOGE("attachToContext: invalid current EGLDisplay");
        return INVALID_OPERATION;
    }

    if (ctx == EGL_NO_CONTEXT) {
        GLC_LOGE("attachToContext: invalid current EGLContext");
        return INVALID_OPERATION;
    }

    // We need to bind the texture regardless of whether there's a current
    // buffer.
    glBindTexture(mTexTarget, GLuint(tex));

    mEglDisplay = dpy;
    mEglContext = ctx;
    mTexName = tex;
    mAttached = true;

    if (mCurrentTextureImage != nullptr) {
        // This may wait for a buffer a second time. This is likely required if
        // this is a different context, since otherwise the wait could be skipped
        // by bouncing through another context. For the same context the extra
        // wait is redundant.
        status_t err =  bindTextureImageLocked();
        if (err != NO_ERROR) {
            return err;
        }
    }

    return OK;
}


status_t GLConsumer::syncForReleaseLocked(EGLDisplay dpy) {
    GLC_LOGV("syncForReleaseLocked");

    if (mCurrentTexture != BufferQueue::INVALID_BUFFER_SLOT) {
        if (SyncFeatures::getInstance().useNativeFenceSync()) {
            EGLSyncKHR sync = eglCreateSyncKHR(dpy,
                    EGL_SYNC_NATIVE_FENCE_ANDROID, nullptr);
            if (sync == EGL_NO_SYNC_KHR) {
                GLC_LOGE("syncForReleaseLocked: error creating EGL fence: %#x",
                        eglGetError());
                return UNKNOWN_ERROR;
            }
            glFlush();
            int fenceFd = eglDupNativeFenceFDANDROID(dpy, sync);
            eglDestroySyncKHR(dpy, sync);
            if (fenceFd == EGL_NO_NATIVE_FENCE_FD_ANDROID) {
                GLC_LOGE("syncForReleaseLocked: error dup'ing native fence "
                        "fd: %#x", eglGetError());
                return UNKNOWN_ERROR;
            }
            sp<Fence> fence(new Fence(fenceFd));
            status_t err = addReleaseFenceLocked(mCurrentTexture,
                    mCurrentTextureImage->graphicBuffer(), fence);
            if (err != OK) {
                GLC_LOGE("syncForReleaseLocked: error adding release fence: "
                        "%s (%d)", strerror(-err), err);
                return err;
            }
        } else if (mUseFenceSync && SyncFeatures::getInstance().useFenceSync()) {
            EGLSyncKHR fence = mEglSlots[mCurrentTexture].mEglFence;
            if (fence != EGL_NO_SYNC_KHR) {
                // There is already a fence for the current slot.  We need to
                // wait on that before replacing it with another fence to
                // ensure that all outstanding buffer accesses have completed
                // before the producer accesses it.
                EGLint result = eglClientWaitSyncKHR(dpy, fence, 0, 1000000000);
                if (result == EGL_FALSE) {
                    GLC_LOGE("syncForReleaseLocked: error waiting for previous "
                            "fence: %#x", eglGetError());
                    return UNKNOWN_ERROR;
                } else if (result == EGL_TIMEOUT_EXPIRED_KHR) {
                    GLC_LOGE("syncForReleaseLocked: timeout waiting for previous "
                            "fence");
                    return TIMED_OUT;
                }
                eglDestroySyncKHR(dpy, fence);
            }

            // Create a fence for the outstanding accesses in the current
            // OpenGL ES context.
            fence = eglCreateSyncKHR(dpy, EGL_SYNC_FENCE_KHR, nullptr);
            if (fence == EGL_NO_SYNC_KHR) {
                GLC_LOGE("syncForReleaseLocked: error creating fence: %#x",
                        eglGetError());
                return UNKNOWN_ERROR;
            }
            glFlush();
            mEglSlots[mCurrentTexture].mEglFence = fence;
        }
    }

    return OK;
}

uint32_t GLConsumer::getCurrentTextureTarget() const {
    return mTexTarget;
}

void GLConsumer::getTransformMatrix(float mtx[16]) {
    Mutex::Autolock lock(mMutex);
    memcpy(mtx, mCurrentTransformMatrix, sizeof(mCurrentTransformMatrix));
}

void GLConsumer::setFilteringEnabled(bool enabled) {
    Mutex::Autolock lock(mMutex);
    if (mAbandoned) {
        GLC_LOGE("setFilteringEnabled: GLConsumer is abandoned!");
        return;
    }
    bool needsRecompute = mFilteringEnabled != enabled;
    mFilteringEnabled = enabled;

    if (needsRecompute && mCurrentTextureImage==nullptr) {
        GLC_LOGD("setFilteringEnabled called with mCurrentTextureImage == NULL");
    }

    if (needsRecompute && mCurrentTextureImage != nullptr) {
        computeCurrentTransformMatrixLocked();
    }
}

void GLConsumer::computeCurrentTransformMatrixLocked() {
    GLC_LOGV("computeCurrentTransformMatrixLocked");
    sp<GraphicBuffer> buf = (mCurrentTextureImage == nullptr) ?
            nullptr : mCurrentTextureImage->graphicBuffer();
    if (buf == nullptr) {
        GLC_LOGD("computeCurrentTransformMatrixLocked: "
                "mCurrentTextureImage is NULL");
    }
    computeTransformMatrix(mCurrentTransformMatrix, buf, mCurrentCrop,
        mCurrentTransform, mFilteringEnabled);
}

Rect GLConsumer::scaleDownCrop(const Rect& crop, uint32_t bufferWidth, uint32_t bufferHeight) {
    Rect outCrop = crop;

    uint32_t newWidth = static_cast<uint32_t>(crop.width());
    uint32_t newHeight = static_cast<uint32_t>(crop.height());

    if (newWidth * bufferHeight > newHeight * bufferWidth) {
        newWidth = newHeight * bufferWidth / bufferHeight;
        ALOGV("too wide: newWidth = %d", newWidth);
    } else if (newWidth * bufferHeight < newHeight * bufferWidth) {
        newHeight = newWidth * bufferHeight / bufferWidth;
        ALOGV("too tall: newHeight = %d", newHeight);
    }

    uint32_t currentWidth = static_cast<uint32_t>(crop.width());
    uint32_t currentHeight = static_cast<uint32_t>(crop.height());

    // The crop is too wide
    if (newWidth < currentWidth) {
        uint32_t dw = currentWidth - newWidth;
        auto halfdw = dw / 2;
        outCrop.left += halfdw;
        // Not halfdw because it would subtract 1 too few when dw is odd
        outCrop.right -= (dw - halfdw);
        // The crop is too tall
    } else if (newHeight < currentHeight) {
        uint32_t dh = currentHeight - newHeight;
        auto halfdh = dh / 2;
        outCrop.top += halfdh;
        // Not halfdh because it would subtract 1 too few when dh is odd
        outCrop.bottom -= (dh - halfdh);
    }

    ALOGV("getCurrentCrop final crop [%d,%d,%d,%d]",
            outCrop.left, outCrop.top,
            outCrop.right,outCrop.bottom);

    return outCrop;
}

nsecs_t GLConsumer::getTimestamp() {
    GLC_LOGV("getTimestamp");
    Mutex::Autolock lock(mMutex);
    return mCurrentTimestamp;
}

android_dataspace GLConsumer::getCurrentDataSpace() {
    GLC_LOGV("getCurrentDataSpace");
    Mutex::Autolock lock(mMutex);
    return mCurrentDataSpace;
}

uint64_t GLConsumer::getFrameNumber() {
    GLC_LOGV("getFrameNumber");
    Mutex::Autolock lock(mMutex);
    return mCurrentFrameNumber;
}

sp<GraphicBuffer> GLConsumer::getCurrentBuffer(int* outSlot) const {
    Mutex::Autolock lock(mMutex);

    if (outSlot != nullptr) {
        *outSlot = mCurrentTexture;
    }

    return (mCurrentTextureImage == nullptr) ?
            nullptr : mCurrentTextureImage->graphicBuffer();
}

Rect GLConsumer::getCurrentCrop() const {
    Mutex::Autolock lock(mMutex);
    return (mCurrentScalingMode == NATIVE_WINDOW_SCALING_MODE_SCALE_CROP)
        ? scaleDownCrop(mCurrentCrop, mDefaultWidth, mDefaultHeight)
        : mCurrentCrop;
}

uint32_t GLConsumer::getCurrentTransform() const {
    Mutex::Autolock lock(mMutex);
    return mCurrentTransform;
}

uint32_t GLConsumer::getCurrentScalingMode() const {
    Mutex::Autolock lock(mMutex);
    return mCurrentScalingMode;
}

sp<Fence> GLConsumer::getCurrentFence() const {
    Mutex::Autolock lock(mMutex);
    return mCurrentFence;
}

std::shared_ptr<FenceTime> GLConsumer::getCurrentFenceTime() const {
    Mutex::Autolock lock(mMutex);
    return mCurrentFenceTime;
}

status_t GLConsumer::doGLFenceWaitLocked() const {

    EGLDisplay dpy = eglGetCurrentDisplay();
    EGLContext ctx = eglGetCurrentContext();

    if (mEglDisplay != dpy || mEglDisplay == EGL_NO_DISPLAY) {
        GLC_LOGE("doGLFenceWait: invalid current EGLDisplay");
        return INVALID_OPERATION;
    }

    if (mEglContext != ctx || mEglContext == EGL_NO_CONTEXT) {
        GLC_LOGE("doGLFenceWait: invalid current EGLContext");
        return INVALID_OPERATION;
    }

    if (mCurrentFence->isValid()) {
        if (SyncFeatures::getInstance().useWaitSync() &&
            SyncFeatures::getInstance().useNativeFenceSync()) {
            // Create an EGLSyncKHR from the current fence.
            int fenceFd = mCurrentFence->dup();
            if (fenceFd == -1) {
                GLC_LOGE("doGLFenceWait: error dup'ing fence fd: %d", errno);
                return -errno;
            }
            EGLint attribs[] = {
                EGL_SYNC_NATIVE_FENCE_FD_ANDROID, fenceFd,
                EGL_NONE
            };
            EGLSyncKHR sync = eglCreateSyncKHR(dpy,
                    EGL_SYNC_NATIVE_FENCE_ANDROID, attribs);
            if (sync == EGL_NO_SYNC_KHR) {
                close(fenceFd);
                GLC_LOGE("doGLFenceWait: error creating EGL fence: %#x",
                        eglGetError());
                return UNKNOWN_ERROR;
            }

            // XXX: The spec draft is inconsistent as to whether this should
            // return an EGLint or void.  Ignore the return value for now, as
            // it's not strictly needed.
            eglWaitSyncKHR(dpy, sync, 0);
            EGLint eglErr = eglGetError();
            eglDestroySyncKHR(dpy, sync);
            if (eglErr != EGL_SUCCESS) {
                GLC_LOGE("doGLFenceWait: error waiting for EGL fence: %#x",
                        eglErr);
                return UNKNOWN_ERROR;
            }
        } else {
            status_t err = mCurrentFence->waitForever(
                    "GLConsumer::doGLFenceWaitLocked");
            if (err != NO_ERROR) {
                GLC_LOGE("doGLFenceWait: error waiting for fence: %d", err);
                return err;
            }
        }
    }

    return NO_ERROR;
}

void GLConsumer::freeBufferLocked(int slotIndex) {
    GLC_LOGV("freeBufferLocked: slotIndex=%d", slotIndex);
    if (slotIndex == mCurrentTexture) {
        mCurrentTexture = BufferQueue::INVALID_BUFFER_SLOT;
    }
    mEglSlots[slotIndex].mEglImage.clear();
    ConsumerBase::freeBufferLocked(slotIndex);
}

void GLConsumer::abandonLocked() {
    GLC_LOGV("abandonLocked");
    mCurrentTextureImage.clear();
    ConsumerBase::abandonLocked();
}

status_t GLConsumer::setConsumerUsageBits(uint64_t usage) {
    return ConsumerBase::setConsumerUsageBits(usage | DEFAULT_USAGE_FLAGS);
}

void GLConsumer::dumpLocked(String8& result, const char* prefix) const
{
    result.appendFormat(
       "%smTexName=%d mCurrentTexture=%d\n"
       "%smCurrentCrop=[%d,%d,%d,%d] mCurrentTransform=%#x\n",
       prefix, mTexName, mCurrentTexture, prefix, mCurrentCrop.left,
       mCurrentCrop.top, mCurrentCrop.right, mCurrentCrop.bottom,
       mCurrentTransform);

    ConsumerBase::dumpLocked(result, prefix);
}

GLConsumer::EglImage::EglImage(sp<GraphicBuffer> graphicBuffer) :
    mGraphicBuffer(graphicBuffer),
    mEglImage(EGL_NO_IMAGE_KHR),
    mEglDisplay(EGL_NO_DISPLAY) {
}

GLConsumer::EglImage::~EglImage() {
    if (mEglImage != EGL_NO_IMAGE_KHR) {
        if (!eglDestroyImageKHR(mEglDisplay, mEglImage)) {
           ALOGE("~EglImage: eglDestroyImageKHR failed");
        }
        DEBUG_EGL_IMAGE_TRACKER_DESTROY();
        eglTerminate(mEglDisplay);
    }
}

status_t GLConsumer::EglImage::createIfNeeded(EGLDisplay eglDisplay,
                                              bool forceCreation) {
    // If there's an image and it's no longer valid, destroy it.
    bool haveImage = mEglImage != EGL_NO_IMAGE_KHR;
    bool displayInvalid = mEglDisplay != eglDisplay;
    if (haveImage && (displayInvalid || forceCreation)) {
        if (!eglDestroyImageKHR(mEglDisplay, mEglImage)) {
           ALOGE("createIfNeeded: eglDestroyImageKHR failed");
        }
        DEBUG_EGL_IMAGE_TRACKER_DESTROY();
        eglTerminate(mEglDisplay);
        mEglImage = EGL_NO_IMAGE_KHR;
        mEglDisplay = EGL_NO_DISPLAY;
    }

    // If there's no image, create one.
    if (mEglImage == EGL_NO_IMAGE_KHR) {
        mEglDisplay = eglDisplay;
        mEglImage = createImage(mEglDisplay, mGraphicBuffer);
    }

    // Fail if we can't create a valid image.
    if (mEglImage == EGL_NO_IMAGE_KHR) {
        mEglDisplay = EGL_NO_DISPLAY;
        const sp<GraphicBuffer>& buffer = mGraphicBuffer;
        ALOGE("Failed to create image. size=%ux%u st=%u usage=%#" PRIx64 " fmt=%d",
            buffer->getWidth(), buffer->getHeight(), buffer->getStride(),
            buffer->getUsage(), buffer->getPixelFormat());
        return UNKNOWN_ERROR;
    }

    return OK;
}

void GLConsumer::EglImage::bindToTextureTarget(uint32_t texTarget) {
    glEGLImageTargetTexture2DOES(texTarget,
            static_cast<GLeglImageOES>(mEglImage));
}

EGLImageKHR GLConsumer::EglImage::createImage(EGLDisplay dpy,
        const sp<GraphicBuffer>& graphicBuffer) {
    EGLClientBuffer cbuf =
            static_cast<EGLClientBuffer>(graphicBuffer->getNativeBuffer());
    const bool createProtectedImage =
            (graphicBuffer->getUsage() & GRALLOC_USAGE_PROTECTED) &&
            hasEglProtectedContent();
    EGLint attrs[] = {
        EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
        createProtectedImage ? EGL_PROTECTED_CONTENT_EXT : EGL_NONE,
        createProtectedImage ? EGL_TRUE : EGL_NONE,
        EGL_NONE,
    };
    eglInitialize(dpy, nullptr, nullptr);
    EGLImageKHR image = eglCreateImageKHR(dpy, EGL_NO_CONTEXT,
            EGL_NATIVE_BUFFER_ANDROID, cbuf, attrs);
    if (image == EGL_NO_IMAGE_KHR) {
        EGLint error = eglGetError();
        ALOGE("error creating EGLImage: %#x", error);
        eglTerminate(dpy);
    } else {
        DEBUG_EGL_IMAGE_TRACKER_CREATE();
    }

    return image;
}

}; // namespace android
