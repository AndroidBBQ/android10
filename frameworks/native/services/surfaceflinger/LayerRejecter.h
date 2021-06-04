/*
 * Copyright (C) 2007 The Android Open Source Project
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

#ifndef ANDROID_LAYER_REJECTER_H
#define ANDROID_LAYER_REJECTER_H

#include "Layer.h"
#include "BufferLayerConsumer.h"

namespace android {
    class LayerRejecter : public BufferLayerConsumer::BufferRejecter {
    public:
        LayerRejecter(Layer::State &front,
                      Layer::State &current,
                      bool &recomputeVisibleRegions,
                      bool stickySet,
                      const char *name,
                      int32_t overrideScalingMode,
                      bool transformToDisplayInverse,
                      bool &freezePositionUpdates);

        virtual bool reject(const sp<GraphicBuffer> &buf, const BufferItem &item);

    private:
        Layer::State &mFront;
        Layer::State &mCurrent;
        bool &mRecomputeVisibleRegions;
        bool mStickyTransformSet;
        const char *mName;
        int32_t mOverrideScalingMode;
        bool mTransformToDisplayInverse;
        bool &mFreezeGeometryUpdates;
    };
}  // namespace android

#endif  // ANDROID_LAYER_REJECTER_H
