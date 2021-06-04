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

#define LOG_TAG "GLConsumerUtils"
//#define LOG_NDEBUG 0

#include <gui/GLConsumer.h>
#include <math/mat4.h>
#include <system/window.h>
#include <utils/Log.h>

namespace android {

void GLConsumer::computeTransformMatrix(float outTransform[16],
        const sp<GraphicBuffer>& buf, const Rect& cropRect, uint32_t transform,
        bool filtering) {
    // Transform matrices
    static const mat4 mtxFlipH(
        -1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        1, 0, 0, 1
    );
    static const mat4 mtxFlipV(
        1, 0, 0, 0,
        0, -1, 0, 0,
        0, 0, 1, 0,
        0, 1, 0, 1
    );
    static const mat4 mtxRot90(
        0, 1, 0, 0,
        -1, 0, 0, 0,
        0, 0, 1, 0,
        1, 0, 0, 1
    );

    mat4 xform;
    if (transform & NATIVE_WINDOW_TRANSFORM_FLIP_H) {
        xform *= mtxFlipH;
    }
    if (transform & NATIVE_WINDOW_TRANSFORM_FLIP_V) {
        xform *= mtxFlipV;
    }
    if (transform & NATIVE_WINDOW_TRANSFORM_ROT_90) {
        xform *= mtxRot90;
    }

    if (!cropRect.isEmpty()) {
        float tx = 0.0f, ty = 0.0f, sx = 1.0f, sy = 1.0f;
        float bufferWidth = buf->getWidth();
        float bufferHeight = buf->getHeight();
        float shrinkAmount = 0.0f;
        if (filtering) {
            // In order to prevent bilinear sampling beyond the edge of the
            // crop rectangle we may need to shrink it by 2 texels in each
            // dimension.  Normally this would just need to take 1/2 a texel
            // off each end, but because the chroma channels of YUV420 images
            // are subsampled we may need to shrink the crop region by a whole
            // texel on each side.
            switch (buf->getPixelFormat()) {
                case PIXEL_FORMAT_RGBA_8888:
                case PIXEL_FORMAT_RGBX_8888:
                case PIXEL_FORMAT_RGBA_FP16:
                case PIXEL_FORMAT_RGBA_1010102:
                case PIXEL_FORMAT_RGB_888:
                case PIXEL_FORMAT_RGB_565:
                case PIXEL_FORMAT_BGRA_8888:
                    // We know there's no subsampling of any channels, so we
                    // only need to shrink by a half a pixel.
                    shrinkAmount = 0.5;
                    break;

                default:
                    // If we don't recognize the format, we must assume the
                    // worst case (that we care about), which is YUV420.
                    shrinkAmount = 1.0;
                    break;
            }
        }

        // Only shrink the dimensions that are not the size of the buffer.
        if (cropRect.width() < bufferWidth) {
            tx = (float(cropRect.left) + shrinkAmount) / bufferWidth;
            sx = (float(cropRect.width()) - (2.0f * shrinkAmount)) /
                    bufferWidth;
        }
        if (cropRect.height() < bufferHeight) {
            ty = (float(bufferHeight - cropRect.bottom) + shrinkAmount) /
                    bufferHeight;
            sy = (float(cropRect.height()) - (2.0f * shrinkAmount)) /
                    bufferHeight;
        }

        mat4 crop(
            sx, 0, 0, 0,
            0, sy, 0, 0,
            0, 0, 1, 0,
            tx, ty, 0, 1
        );
        xform = crop * xform;
    }

    // GLConsumer uses the GL convention where (0, 0) is the bottom-left
    // corner and (1, 1) is the top-right corner.  Add an additional vertical
    // flip after all other transforms to map from GL convention to buffer
    // queue memory layout, where (0, 0) is the top-left corner.
    xform = mtxFlipV * xform;

    memcpy(outTransform, xform.asArray(), sizeof(xform));
}

}; // namespace android
