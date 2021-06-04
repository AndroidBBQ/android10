/*
 * Copyright 2019 The Android Open Source Project
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
#pragma once

#include "SurfaceFlinger.h"

namespace android {

using RefreshRateType = scheduler::RefreshRateConfigs::RefreshRateType;

class RefreshRateOverlay {
public:
    RefreshRateOverlay(SurfaceFlinger& flinger);

    void changeRefreshRate(RefreshRateType type);

private:
    bool createLayer();

    SurfaceFlinger& mFlinger;
    sp<Client> mClient;
    sp<Layer> mLayer;
    sp<IBinder> mIBinder;
    sp<IGraphicBufferProducer> mGbp;

    const half3 RED = half3(1.0f, 0.0f, 0.0f);
    const half3 GREEN = half3(0.0f, 1.0f, 0.0f);
};

}; // namespace android
