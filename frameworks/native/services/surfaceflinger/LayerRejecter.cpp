/*
 * Copyright (C) 2011 The Android Open Source Project
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

#include "LayerRejecter.h"

#include <gui/BufferItem.h>
#include <system/window.h>

#define DEBUG_RESIZE 0

namespace android {

LayerRejecter::LayerRejecter(Layer::State& front,
                             Layer::State& current,
                             bool& recomputeVisibleRegions,
                             bool stickySet,
                             const char* name,
                             int32_t overrideScalingMode,
                             bool transformToDisplayInverse,
                             bool& freezePositionUpdates)
  : mFront(front),
    mCurrent(current),
    mRecomputeVisibleRegions(recomputeVisibleRegions),
    mStickyTransformSet(stickySet),
    mName(name),
    mOverrideScalingMode(overrideScalingMode),
    mTransformToDisplayInverse(transformToDisplayInverse),
    mFreezeGeometryUpdates(freezePositionUpdates) {}

bool LayerRejecter::reject(const sp<GraphicBuffer>& buf, const BufferItem& item) {
    if (buf == nullptr) {
        return false;
    }

    uint32_t bufWidth = buf->getWidth();
    uint32_t bufHeight = buf->getHeight();

    // check that we received a buffer of the right size
    // (Take the buffer's orientation into account)
    if (item.mTransform & ui::Transform::ROT_90) {
        std::swap(bufWidth, bufHeight);
    }

    if (mTransformToDisplayInverse) {
        uint32_t invTransform = DisplayDevice::getPrimaryDisplayOrientationTransform();
        if (invTransform & ui::Transform::ROT_90) {
            std::swap(bufWidth, bufHeight);
        }
    }

    int actualScalingMode = mOverrideScalingMode >= 0 ? mOverrideScalingMode : item.mScalingMode;
    bool isFixedSize = actualScalingMode != NATIVE_WINDOW_SCALING_MODE_FREEZE;
    if (mFront.active_legacy != mFront.requested_legacy) {
        if (isFixedSize ||
            (bufWidth == mFront.requested_legacy.w && bufHeight == mFront.requested_legacy.h)) {
            // Here we pretend the transaction happened by updating the
            // current and drawing states. Drawing state is only accessed
            // in this thread, no need to have it locked
            mFront.active_legacy = mFront.requested_legacy;

            // We also need to update the current state so that
            // we don't end-up overwriting the drawing state with
            // this stale current state during the next transaction
            //
            // NOTE: We don't need to hold the transaction lock here
            // because State::active_legacy is only accessed from this thread.
            mCurrent.active_legacy = mFront.active_legacy;
            mCurrent.modified = true;

            // recompute visible region
            mRecomputeVisibleRegions = true;

            mFreezeGeometryUpdates = false;

            if (mFront.crop_legacy != mFront.requestedCrop_legacy) {
                mFront.crop_legacy = mFront.requestedCrop_legacy;
                mCurrent.crop_legacy = mFront.requestedCrop_legacy;
                mRecomputeVisibleRegions = true;
            }
        }

        ALOGD_IF(DEBUG_RESIZE,
                 "[%s] latchBuffer/reject: buffer (%ux%u, tr=%02x), scalingMode=%d\n"
                 "  drawing={ active_legacy   ={ wh={%4u,%4u} crop_legacy={%4d,%4d,%4d,%4d} "
                 "(%4d,%4d) "
                 "}\n"
                 "            requested_legacy={ wh={%4u,%4u} }}\n",
                 mName, bufWidth, bufHeight, item.mTransform, item.mScalingMode,
                 mFront.active_legacy.w, mFront.active_legacy.h, mFront.crop_legacy.left,
                 mFront.crop_legacy.top, mFront.crop_legacy.right, mFront.crop_legacy.bottom,
                 mFront.crop_legacy.getWidth(), mFront.crop_legacy.getHeight(),
                 mFront.requested_legacy.w, mFront.requested_legacy.h);
    }

    if (!isFixedSize && !mStickyTransformSet) {
        if (mFront.active_legacy.w != bufWidth || mFront.active_legacy.h != bufHeight) {
            // reject this buffer
            ALOGE("[%s] rejecting buffer: "
                  "bufWidth=%d, bufHeight=%d, front.active_legacy.{w=%d, h=%d}",
                  mName, bufWidth, bufHeight, mFront.active_legacy.w, mFront.active_legacy.h);
            return true;
        }
    }

    // if the transparent region has changed (this test is
    // conservative, but that's fine, worst case we're doing
    // a bit of extra work), we latch the new one and we
    // trigger a visible-region recompute.
    //
    // We latch the transparent region here, instead of above where we latch
    // the rest of the geometry because it is only content but not necessarily
    // resize dependent.
    if (!mFront.activeTransparentRegion_legacy.isTriviallyEqual(
                mFront.requestedTransparentRegion_legacy)) {
        mFront.activeTransparentRegion_legacy = mFront.requestedTransparentRegion_legacy;

        // We also need to update the current state so that
        // we don't end-up overwriting the drawing state with
        // this stale current state during the next transaction
        //
        // NOTE: We don't need to hold the transaction lock here
        // because State::active_legacy is only accessed from this thread.
        mCurrent.activeTransparentRegion_legacy = mFront.activeTransparentRegion_legacy;

        // recompute visible region
        mRecomputeVisibleRegions = true;
    }

    return false;
}

}  // namespace android
