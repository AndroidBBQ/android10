/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <timestatsproto/TimeStatsProtoHeader.h>
#include <utils/Timers.h>

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace android {
namespace surfaceflinger {

class TimeStatsHelper {
public:
    class Histogram {
    public:
        // Key is the delta time between timestamps
        // Value is the number of appearances of that delta
        std::unordered_map<int32_t, int32_t> hist;

        void insert(int32_t delta);
        int64_t totalTime() const;
        float averageTime() const;
        std::string toString() const;
    };

    class TimeStatsLayer {
    public:
        std::string layerName;
        std::string packageName;
        int32_t totalFrames = 0;
        int32_t droppedFrames = 0;
        std::unordered_map<std::string, Histogram> deltas;

        std::string toString() const;
        SFTimeStatsLayerProto toProto() const;
    };

    class TimeStatsGlobal {
    public:
        int64_t statsStart = 0;
        int64_t statsEnd = 0;
        int32_t totalFrames = 0;
        int32_t missedFrames = 0;
        int32_t clientCompositionFrames = 0;
        int64_t displayOnTime = 0;
        Histogram presentToPresent;
        std::unordered_map<std::string, TimeStatsLayer> stats;
        std::unordered_map<uint32_t, nsecs_t> refreshRateStats;

        std::string toString(std::optional<uint32_t> maxLayers) const;
        SFTimeStatsGlobalProto toProto(std::optional<uint32_t> maxLayers) const;

    private:
        std::vector<TimeStatsLayer const*> generateDumpStats(
                std::optional<uint32_t> maxLayers) const;
    };
};

} // namespace surfaceflinger
} // namespace android
