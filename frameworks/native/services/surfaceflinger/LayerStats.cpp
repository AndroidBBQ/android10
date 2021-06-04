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
#undef LOG_TAG
#define LOG_TAG "LayerStats"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "LayerStats.h"
#include "DisplayHardware/HWComposer.h"
#include "ui/DebugUtils.h"

#include <android-base/stringprintf.h>
#include <log/log.h>
#include <utils/Trace.h>

namespace android {

using base::StringAppendF;
using base::StringPrintf;

void LayerStats::enable() {
    ATRACE_CALL();
    std::lock_guard<std::mutex> lock(mMutex);
    if (mEnabled) return;
    mLayerShapeStatsMap.clear();
    mEnabled = true;
    ALOGD("Logging enabled");
}

void LayerStats::disable() {
    ATRACE_CALL();
    std::lock_guard<std::mutex> lock(mMutex);
    if (!mEnabled) return;
    mEnabled = false;
    ALOGD("Logging disabled");
}

void LayerStats::clear() {
    ATRACE_CALL();
    std::lock_guard<std::mutex> lock(mMutex);
    mLayerShapeStatsMap.clear();
    ALOGD("Cleared current layer stats");
}

bool LayerStats::isEnabled() {
    return mEnabled;
}

void LayerStats::traverseLayerTreeStatsLocked(
        const std::vector<LayerProtoParser::Layer*>& layerTree,
        const LayerProtoParser::LayerGlobal& layerGlobal,
        std::vector<std::string>* const outLayerShapeVec) {
    for (const auto& layer : layerTree) {
        if (!layer) continue;
        traverseLayerTreeStatsLocked(layer->children, layerGlobal, outLayerShapeVec);
        std::string key = "";
        StringAppendF(&key, ",%s", layer->type.c_str());
        StringAppendF(&key, ",%s", layerCompositionType(layer->hwcCompositionType));
        StringAppendF(&key, ",%d", layer->isProtected);
        StringAppendF(&key, ",%s", layerTransform(layer->hwcTransform));
        StringAppendF(&key, ",%s", layerPixelFormat(layer->activeBuffer.format).c_str());
        StringAppendF(&key, ",%s", layer->dataspace.c_str());
        StringAppendF(&key, ",%s",
                      destinationLocation(layer->hwcFrame.left, layerGlobal.resolution[0], true));
        StringAppendF(&key, ",%s",
                      destinationLocation(layer->hwcFrame.top, layerGlobal.resolution[1], false));
        StringAppendF(&key, ",%s",
                      destinationSize(layer->hwcFrame.right - layer->hwcFrame.left,
                                      layerGlobal.resolution[0], true));
        StringAppendF(&key, ",%s",
                      destinationSize(layer->hwcFrame.bottom - layer->hwcFrame.top,
                                      layerGlobal.resolution[1], false));
        StringAppendF(&key, ",%s", scaleRatioWH(layer).c_str());
        StringAppendF(&key, ",%s", alpha(static_cast<float>(layer->color.a)));

        outLayerShapeVec->push_back(key);
        ALOGV("%s", key.c_str());
    }
}

void LayerStats::logLayerStats(const LayersProto& layersProto) {
    ATRACE_CALL();
    ALOGV("Logging");
    auto layerGlobal = LayerProtoParser::generateLayerGlobalInfo(layersProto);
    auto layerTree = LayerProtoParser::generateLayerTree(layersProto);
    std::vector<std::string> layerShapeVec;

    std::lock_guard<std::mutex> lock(mMutex);
    traverseLayerTreeStatsLocked(layerTree.topLevelLayers, layerGlobal, &layerShapeVec);

    std::string layerShapeKey =
            StringPrintf("%d,%s,%s,%s", static_cast<int32_t>(layerShapeVec.size()),
                         layerGlobal.colorMode.c_str(), layerGlobal.colorTransform.c_str(),
                         layerTransform(layerGlobal.globalTransform));
    ALOGV("%s", layerShapeKey.c_str());

    std::sort(layerShapeVec.begin(), layerShapeVec.end(), std::greater<std::string>());
    for (auto const& s : layerShapeVec) {
        layerShapeKey += s;
    }

    mLayerShapeStatsMap[layerShapeKey]++;
}

void LayerStats::dump(std::string& result) {
    ATRACE_CALL();
    ALOGD("Dumping");
    std::lock_guard<std::mutex> lock(mMutex);
    result.append("Frequency,LayerCount,ColorMode,ColorTransform,Orientation\n");
    result.append("LayerType,CompositionType,IsProtected,Transform,PixelFormat,Dataspace,");
    result.append("DstX,DstY,DstWidth,DstHeight,WScale,HScale,Alpha\n");
    for (auto& u : mLayerShapeStatsMap) {
        StringAppendF(&result, "%u,%s\n", u.second, u.first.c_str());
    }
}

const char* LayerStats::destinationLocation(int32_t location, int32_t range, bool isHorizontal) {
    static const char* locationArray[8] = {"0", "1/8", "1/4", "3/8", "1/2", "5/8", "3/4", "7/8"};
    int32_t ratio = location * 8 / range;
    if (ratio < 0) return "N/A";
    if (isHorizontal) {
        // X location is divided into 4 buckets {"0", "1/4", "1/2", "3/4"}
        if (ratio > 6) return "3/4";
        // use index 0, 2, 4, 6
        return locationArray[ratio & ~1];
    }
    if (ratio > 7) return "7/8";
    return locationArray[ratio];
}

const char* LayerStats::destinationSize(int32_t size, int32_t range, bool isWidth) {
    static const char* sizeArray[8] = {"1/8", "1/4", "3/8", "1/2", "5/8", "3/4", "7/8", "1"};
    int32_t ratio = size * 8 / range;
    if (ratio < 0) return "N/A";
    if (isWidth) {
        // width is divided into 4 buckets {"1/4", "1/2", "3/4", "1"}
        if (ratio > 6) return "1";
        // use index 1, 3, 5, 7
        return sizeArray[ratio | 1];
    }
    if (ratio > 7) return "1";
    return sizeArray[ratio];
}

const char* LayerStats::layerTransform(int32_t transform) {
    return getTransformName(static_cast<hwc_transform_t>(transform));
}

const char* LayerStats::layerCompositionType(int32_t compositionType) {
    return getCompositionName(static_cast<hwc2_composition_t>(compositionType));
}

std::string LayerStats::layerPixelFormat(int32_t pixelFormat) {
    return decodePixelFormat(pixelFormat);
}

std::string LayerStats::scaleRatioWH(const LayerProtoParser::Layer* layer) {
    if (!layer->type.compare("ColorLayer")) return "N/A,N/A";
    std::string ret = "";
    if (isRotated(layer->hwcTransform)) {
        ret += scaleRatio(layer->hwcFrame.right - layer->hwcFrame.left,
                          static_cast<int32_t>(layer->hwcCrop.bottom - layer->hwcCrop.top));
        ret += ",";
        ret += scaleRatio(layer->hwcFrame.bottom - layer->hwcFrame.top,
                          static_cast<int32_t>(layer->hwcCrop.right - layer->hwcCrop.left));
    } else {
        ret += scaleRatio(layer->hwcFrame.right - layer->hwcFrame.left,
                          static_cast<int32_t>(layer->hwcCrop.right - layer->hwcCrop.left));
        ret += ",";
        ret += scaleRatio(layer->hwcFrame.bottom - layer->hwcFrame.top,
                          static_cast<int32_t>(layer->hwcCrop.bottom - layer->hwcCrop.top));
    }
    return ret;
}

const char* LayerStats::scaleRatio(int32_t destinationScale, int32_t sourceScale) {
    // Make scale buckets from <1/64 to >= 16, to avoid floating point
    // calculation, x64 on destinationScale first
    int32_t scale = destinationScale * 64 / sourceScale;
    if (!scale) return "<1/64";
    if (scale < 2) return "1/64";
    if (scale < 4) return "1/32";
    if (scale < 8) return "1/16";
    if (scale < 16) return "1/8";
    if (scale < 32) return "1/4";
    if (scale < 64) return "1/2";
    if (scale < 128) return "1";
    if (scale < 256) return "2";
    if (scale < 512) return "4";
    if (scale < 1024) return "8";
    return ">=16";
}

const char* LayerStats::alpha(float a) {
    if (a == 1.0f) return "1.0";
    if (a > 0.9f) return "0.99";
    if (a > 0.8f) return "0.9";
    if (a > 0.7f) return "0.8";
    if (a > 0.6f) return "0.7";
    if (a > 0.5f) return "0.6";
    if (a > 0.4f) return "0.5";
    if (a > 0.3f) return "0.4";
    if (a > 0.2f) return "0.3";
    if (a > 0.1f) return "0.2";
    if (a > 0.0f) return "0.1";
    return "0.0";
}

bool LayerStats::isRotated(int32_t transform) {
    return transform & HWC_TRANSFORM_ROT_90;
}

bool LayerStats::isVFlipped(int32_t transform) {
    return transform & HWC_TRANSFORM_FLIP_V;
}

bool LayerStats::isHFlipped(int32_t transform) {
    return transform & HWC_TRANSFORM_FLIP_H;
}

}  // namespace android
