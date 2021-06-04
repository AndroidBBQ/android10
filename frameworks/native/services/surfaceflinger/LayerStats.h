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

#include <layerproto/LayerProtoHeader.h>
#include <layerproto/LayerProtoParser.h>
#include <mutex>
#include <unordered_map>

using namespace android::surfaceflinger;

namespace android {

class LayerStats {
public:
    void enable();
    void disable();
    void clear();
    bool isEnabled();
    void logLayerStats(const LayersProto& layersProto);
    void dump(std::string& result);

private:
    // Traverse layer tree to get all visible layers' stats
    void traverseLayerTreeStatsLocked(
            const std::vector<LayerProtoParser::Layer*>& layerTree,
            const LayerProtoParser::LayerGlobal& layerGlobal,
            std::vector<std::string>* const outLayerShapeVec);
    // Convert layer's top-left position into 8x8 percentage of the display
    static const char* destinationLocation(int32_t location, int32_t range, bool isHorizontal);
    // Convert layer's size into 8x8 percentage of the display
    static const char* destinationSize(int32_t size, int32_t range, bool isWidth);
    // Return the name of the transform
    static const char* layerTransform(int32_t transform);
    // Return the name of the composition type
    static const char* layerCompositionType(int32_t compositionType);
    // Return the name of the pixel format
    static std::string layerPixelFormat(int32_t pixelFormat);
    // Calculate scale ratios of layer's width/height with rotation information
    static std::string scaleRatioWH(const LayerProtoParser::Layer* layer);
    // Calculate scale ratio from source to destination and convert to string
    static const char* scaleRatio(int32_t destinationScale, int32_t sourceScale);
    // Bucket the alpha into designed buckets
    static const char* alpha(float a);
    // Return whether the original buffer is rotated in final composition
    static bool isRotated(int32_t transform);
    // Return whether the original buffer is V-flipped in final composition
    static bool isVFlipped(int32_t transform);
    // Return whether the original buffer is H-flipped in final composition
    static bool isHFlipped(int32_t transform);

    bool mEnabled = false;
    // Protect mLayersStatsMap
    std::mutex mMutex;
    // Hashmap for tracking the frame(layer shape) stats
    // KEY is a concatenation of all layers' properties within a frame
    // VALUE is the number of times this particular set has been scanned out
    std::unordered_map<std::string, uint32_t> mLayerShapeStatsMap;
};

}  // namespace android
