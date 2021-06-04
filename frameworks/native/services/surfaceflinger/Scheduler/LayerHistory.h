/*
 * Copyright 2018 The Android Open Source Project
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

#include <array>
#include <cinttypes>
#include <cstdint>
#include <numeric>
#include <string>
#include <unordered_map>

#include <utils/Timers.h>

#include "LayerInfo.h"
#include "SchedulerUtils.h"

namespace android {
namespace scheduler {

/*
 * This class represents information about layers that are considered current. We keep an
 * unordered map between layer name and LayerInfo.
 */
class LayerHistory {
public:
    // Handle for each layer we keep track of.
    class LayerHandle {
    public:
        LayerHandle(LayerHistory& lh, int64_t id) : mId(id), mLayerHistory(lh) {}
        ~LayerHandle() { mLayerHistory.destroyLayer(mId); }

        const int64_t mId;

    private:
        LayerHistory& mLayerHistory;
    };

    LayerHistory();
    ~LayerHistory();

    // When the layer is first created, register it.
    std::unique_ptr<LayerHandle> createLayer(const std::string name, float minRefreshRate,
                                             float maxRefreshRate);

    // Method for inserting layers and their requested present time into the unordered map.
    void insert(const std::unique_ptr<LayerHandle>& layerHandle, nsecs_t presentTime, bool isHdr);
    // Method for setting layer visibility
    void setVisibility(const std::unique_ptr<LayerHandle>& layerHandle, bool visible);

    // Returns the desired refresh rate, which is a max refresh rate of all the current
    // layers. See go/content-fps-detection-in-scheduler for more information.
    std::pair<float, bool> getDesiredRefreshRateAndHDR();

    // Clears all layer history.
    void clearHistory();

    // Removes the handle and the object from the map.
    void destroyLayer(const int64_t id);

private:
    // Removes the layers that have been idle for a given amount of time from mLayerInfos.
    void removeIrrelevantLayers() REQUIRES(mLock);

    // Information about currently active layers.
    std::mutex mLock;
    std::unordered_map<int64_t, std::shared_ptr<LayerInfo>> mActiveLayerInfos GUARDED_BY(mLock);
    std::unordered_map<int64_t, std::shared_ptr<LayerInfo>> mInactiveLayerInfos GUARDED_BY(mLock);

    // Each layer has it's own ID. This variable keeps track of the count.
    static std::atomic<int64_t> sNextId;

    // Flag whether to log layer FPS in systrace
    bool mTraceEnabled = false;
};

} // namespace scheduler
} // namespace android