/*
 * Copyright 2015 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "SurfaceUtils"
#include <utils/Log.h>

#include <android/api-level.h>
#include <media/hardware/VideoAPI.h>
#include <media/stagefright/SurfaceUtils.h>
#include <gui/Surface.h>

extern "C" int android_get_application_target_sdk_version();

namespace android {

status_t setNativeWindowSizeFormatAndUsage(
        ANativeWindow *nativeWindow /* nonnull */,
        int width, int height, int format, int rotation, int usage, bool reconnect) {
    status_t err = NO_ERROR;

    // In some cases we need to reconnect so that we can dequeue all buffers
    if (reconnect) {
        err = nativeWindowDisconnect(nativeWindow, "setNativeWindowSizeFormatAndUsage");
        if (err != NO_ERROR) {
            ALOGE("nativeWindowDisconnect failed: %s (%d)", strerror(-err), -err);
            return err;
        }

        err = nativeWindowConnect(nativeWindow, "setNativeWindowSizeFormatAndUsage");
        if (err != NO_ERROR) {
            ALOGE("nativeWindowConnect failed: %s (%d)", strerror(-err), -err);
            return err;
        }
    }

    err = native_window_set_buffers_dimensions(nativeWindow, width, height);
    if (err != NO_ERROR) {
        ALOGE("native_window_set_buffers_dimensions failed: %s (%d)", strerror(-err), -err);
        return err;
    }

    err = native_window_set_buffers_format(nativeWindow, format);
    if (err != NO_ERROR) {
        ALOGE("native_window_set_buffers_format failed: %s (%d)", strerror(-err), -err);
        return err;
    }

    int transform = 0;
    if ((rotation % 90) == 0) {
        switch ((rotation / 90) & 3) {
            case 1:  transform = HAL_TRANSFORM_ROT_90;  break;
            case 2:  transform = HAL_TRANSFORM_ROT_180; break;
            case 3:  transform = HAL_TRANSFORM_ROT_270; break;
            default: transform = 0;                     break;
        }
    }

    err = native_window_set_buffers_transform(nativeWindow, transform);
    if (err != NO_ERROR) {
        ALOGE("native_window_set_buffers_transform failed: %s (%d)", strerror(-err), -err);
        return err;
    }

    int consumerUsage = 0;
    err = nativeWindow->query(nativeWindow, NATIVE_WINDOW_CONSUMER_USAGE_BITS, &consumerUsage);
    if (err != NO_ERROR) {
        ALOGW("failed to get consumer usage bits. ignoring");
        err = NO_ERROR;
    }

    // Make sure to check whether either Stagefright or the video decoder
    // requested protected buffers.
    if (usage & GRALLOC_USAGE_PROTECTED) {
        // Check if the ANativeWindow sends images directly to SurfaceFlinger.
        int queuesToNativeWindow = 0;
        err = nativeWindow->query(
                nativeWindow, NATIVE_WINDOW_QUEUES_TO_WINDOW_COMPOSER, &queuesToNativeWindow);
        if (err != NO_ERROR) {
            ALOGE("error authenticating native window: %s (%d)", strerror(-err), -err);
            return err;
        }

        // Check if the consumer end of the ANativeWindow can handle protected content.
        int isConsumerProtected = 0;
        err = nativeWindow->query(
                nativeWindow, NATIVE_WINDOW_CONSUMER_IS_PROTECTED, &isConsumerProtected);
        if (err != NO_ERROR) {
            ALOGE("error query native window: %s (%d)", strerror(-err), -err);
            return err;
        }

        // Deny queuing into native window if neither condition is satisfied.
        if (queuesToNativeWindow != 1 && isConsumerProtected != 1) {
            ALOGE("native window cannot handle protected buffers: the consumer should either be "
                  "a hardware composer or support hardware protection");
            return PERMISSION_DENIED;
        }
    }

    int finalUsage = usage | consumerUsage;
    ALOGV("gralloc usage: %#x(producer) + %#x(consumer) = %#x", usage, consumerUsage, finalUsage);
    err = native_window_set_usage(nativeWindow, finalUsage);
    if (err != NO_ERROR) {
        ALOGE("native_window_set_usage failed: %s (%d)", strerror(-err), -err);
        return err;
    }

    err = native_window_set_scaling_mode(
            nativeWindow, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
    if (err != NO_ERROR) {
        ALOGE("native_window_set_scaling_mode failed: %s (%d)", strerror(-err), -err);
        return err;
    }

    ALOGD("set up nativeWindow %p for %dx%d, color %#x, rotation %d, usage %#x",
            nativeWindow, width, height, format, rotation, finalUsage);
    return NO_ERROR;
}

void setNativeWindowHdrMetadata(ANativeWindow *nativeWindow, HDRStaticInfo *info) {
    struct android_smpte2086_metadata smpte2086_meta = {
            .displayPrimaryRed = {
                    info->sType1.mR.x * 0.00002f,
                    info->sType1.mR.y * 0.00002f
            },
            .displayPrimaryGreen = {
                    info->sType1.mG.x * 0.00002f,
                    info->sType1.mG.y * 0.00002f
            },
            .displayPrimaryBlue = {
                    info->sType1.mB.x * 0.00002f,
                    info->sType1.mB.y * 0.00002f
            },
            .whitePoint = {
                    info->sType1.mW.x * 0.00002f,
                    info->sType1.mW.y * 0.00002f
            },
            .maxLuminance = (float) info->sType1.mMaxDisplayLuminance,
            .minLuminance = info->sType1.mMinDisplayLuminance * 0.0001f
    };

    int err = native_window_set_buffers_smpte2086_metadata(nativeWindow, &smpte2086_meta);
    ALOGW_IF(err != 0, "failed to set smpte2086 metadata on surface (%d)", err);

    struct android_cta861_3_metadata cta861_meta = {
            .maxContentLightLevel = (float) info->sType1.mMaxContentLightLevel,
            .maxFrameAverageLightLevel = (float) info->sType1.mMaxFrameAverageLightLevel
    };

    err = native_window_set_buffers_cta861_3_metadata(nativeWindow, &cta861_meta);
    ALOGW_IF(err != 0, "failed to set cta861_3 metadata on surface (%d)", err);
}

status_t pushBlankBuffersToNativeWindow(ANativeWindow *nativeWindow /* nonnull */) {
    status_t err = NO_ERROR;
    ANativeWindowBuffer* anb = NULL;
    int numBufs = 0;
    int minUndequeuedBufs = 0;

    // We need to reconnect to the ANativeWindow as a CPU client to ensure that
    // no frames get dropped by SurfaceFlinger assuming that these are video
    // frames.
    err = nativeWindowDisconnect(nativeWindow, "pushBlankBuffersToNativeWindow");
    if (err != NO_ERROR) {
        ALOGE("error pushing blank frames: api_disconnect failed: %s (%d)", strerror(-err), -err);
        return err;
    }

    err = native_window_api_connect(nativeWindow, NATIVE_WINDOW_API_CPU);
    if (err != NO_ERROR) {
        ALOGE("error pushing blank frames: api_connect failed: %s (%d)", strerror(-err), -err);
        (void)nativeWindowConnect(nativeWindow, "pushBlankBuffersToNativeWindow(err)");
        return err;
    }

    err = setNativeWindowSizeFormatAndUsage(
            nativeWindow, 1, 1, HAL_PIXEL_FORMAT_RGBX_8888, 0, GRALLOC_USAGE_SW_WRITE_OFTEN,
            false /* reconnect */);
    if (err != NO_ERROR) {
        goto error;
    }

    static_cast<Surface*>(nativeWindow)->getIGraphicBufferProducer()->allowAllocation(true);

    err = nativeWindow->query(nativeWindow,
            NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, &minUndequeuedBufs);
    if (err != NO_ERROR) {
        ALOGE("error pushing blank frames: MIN_UNDEQUEUED_BUFFERS query "
                "failed: %s (%d)", strerror(-err), -err);
        goto error;
    }

    numBufs = minUndequeuedBufs + 1;
    err = native_window_set_buffer_count(nativeWindow, numBufs);
    if (err != NO_ERROR) {
        ALOGE("error pushing blank frames: set_buffer_count failed: %s (%d)", strerror(-err), -err);
        goto error;
    }

    // We push numBufs + 1 buffers to ensure that we've drawn into the same
    // buffer twice.  This should guarantee that the buffer has been displayed
    // on the screen and then been replaced, so an previous video frames are
    // guaranteed NOT to be currently displayed.
    for (int i = 0; i < numBufs + 1; i++) {
        err = native_window_dequeue_buffer_and_wait(nativeWindow, &anb);
        if (err != NO_ERROR) {
            ALOGE("error pushing blank frames: dequeueBuffer failed: %s (%d)",
                    strerror(-err), -err);
            break;
        }

        sp<GraphicBuffer> buf(GraphicBuffer::from(anb));

        // Fill the buffer with the a 1x1 checkerboard pattern ;)
        uint32_t *img = NULL;
        err = buf->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)(&img));
        if (err != NO_ERROR) {
            ALOGE("error pushing blank frames: lock failed: %s (%d)", strerror(-err), -err);
            break;
        }

        *img = 0;

        err = buf->unlock();
        if (err != NO_ERROR) {
            ALOGE("error pushing blank frames: unlock failed: %s (%d)", strerror(-err), -err);
            break;
        }

        err = nativeWindow->queueBuffer(nativeWindow, buf->getNativeBuffer(), -1);
        if (err != NO_ERROR) {
            ALOGE("error pushing blank frames: queueBuffer failed: %s (%d)", strerror(-err), -err);
            break;
        }

        anb = NULL;
    }

error:

    if (anb != NULL) {
        nativeWindow->cancelBuffer(nativeWindow, anb, -1);
        anb = NULL;
    }

    // Clean up after success or error.
    status_t err2 = native_window_api_disconnect(nativeWindow, NATIVE_WINDOW_API_CPU);
    if (err2 != NO_ERROR) {
        ALOGE("error pushing blank frames: api_disconnect failed: %s (%d)", strerror(-err2), -err2);
        if (err == NO_ERROR) {
            err = err2;
        }
    }

    err2 = nativeWindowConnect(nativeWindow, "pushBlankBuffersToNativeWindow(err2)");
    if (err2 != NO_ERROR) {
        ALOGE("error pushing blank frames: api_connect failed: %s (%d)", strerror(-err), -err);
        if (err == NO_ERROR) {
            err = err2;
        }
    }

    return err;
}

status_t nativeWindowConnect(ANativeWindow *surface, const char *reason) {
    ALOGD("connecting to surface %p, reason %s", surface, reason);

    status_t err = native_window_api_connect(surface, NATIVE_WINDOW_API_MEDIA);
    ALOGE_IF(err != OK, "Failed to connect to surface %p, err %d", surface, err);

    return err;
}

status_t nativeWindowDisconnect(ANativeWindow *surface, const char *reason) {
    ALOGD("disconnecting from surface %p, reason %s", surface, reason);

    status_t err = native_window_api_disconnect(surface, NATIVE_WINDOW_API_MEDIA);
    ALOGE_IF(err != OK, "Failed to disconnect from surface %p, err %d", surface, err);

    return err;
}

status_t disableLegacyBufferDropPostQ(const sp<Surface> &surface) {
    sp<IGraphicBufferProducer> igbp =
            surface ? surface->getIGraphicBufferProducer() : nullptr;
    if (igbp) {
        int targetSdk = android_get_application_target_sdk_version();
        // When the caller is not an app (e.g. MediaPlayer in mediaserver)
        // targetSdk is __ANDROID_API_FUTURE__.
        bool drop =
                targetSdk < __ANDROID_API_Q__ ||
                targetSdk == __ANDROID_API_FUTURE__;
        if (!drop) {
            status_t err = igbp->setLegacyBufferDrop(false);
            if (err == NO_ERROR) {
                ALOGD("legacy buffer drop disabled: target sdk (%d)",
                      targetSdk);
            } else {
                ALOGD("disabling legacy buffer drop failed: %d", err);
            }
        }
    }
    return NO_ERROR;
}
}  // namespace android

