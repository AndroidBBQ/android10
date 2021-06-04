/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <gui/LayerMetadata.h>
#include <math/vec4.h>

#include <memory>
#include <unordered_map>
#include <vector>

namespace android {
namespace surfaceflinger {

class LayerProtoParser {
public:
    class ActiveBuffer {
    public:
        uint32_t width;
        uint32_t height;
        uint32_t stride;
        int32_t format;

        std::string to_string() const;
    };

    class Transform {
    public:
        float dsdx;
        float dtdx;
        float dsdy;
        float dtdy;

        std::string to_string() const;
    };

    class Rect {
    public:
        int32_t left;
        int32_t top;
        int32_t right;
        int32_t bottom;

        std::string to_string() const;
    };

    class FloatRect {
    public:
        float left;
        float top;
        float right;
        float bottom;

        std::string to_string() const;
    };

    class Region {
    public:
        uint64_t id;
        std::vector<Rect> rects;

        std::string to_string(const char* what) const;
    };

    class Layer {
    public:
        int32_t id;
        std::string name;
        std::vector<Layer*> children;
        std::vector<Layer*> relatives;
        std::string type;
        LayerProtoParser::Region transparentRegion;
        LayerProtoParser::Region visibleRegion;
        LayerProtoParser::Region damageRegion;
        uint32_t layerStack;
        int32_t z;
        float2 position;
        float2 requestedPosition;
        int2 size;
        LayerProtoParser::Rect crop;
        bool isOpaque;
        bool invalidate;
        std::string dataspace;
        std::string pixelFormat;
        half4 color;
        half4 requestedColor;
        uint32_t flags;
        Transform transform;
        Transform requestedTransform;
        Layer* parent = 0;
        Layer* zOrderRelativeOf = 0;
        LayerProtoParser::ActiveBuffer activeBuffer;
        Transform bufferTransform;
        int32_t queuedFrames;
        bool refreshPending;
        LayerProtoParser::Rect hwcFrame;
        LayerProtoParser::FloatRect hwcCrop;
        int32_t hwcTransform;
        int32_t hwcCompositionType;
        bool isProtected;
        float cornerRadius;
        LayerMetadata metadata;

        std::string to_string() const;
    };

    class LayerGlobal {
    public:
        int2 resolution;
        std::string colorMode;
        std::string colorTransform;
        int32_t globalTransform;
    };

    class LayerTree {
    public:
        // all layers in LayersProto and in the original order
        std::vector<Layer> allLayers;

        // pointers to top-level layers in allLayers
        std::vector<Layer*> topLevelLayers;
    };

    static const LayerGlobal generateLayerGlobalInfo(const LayersProto& layersProto);
    static LayerTree generateLayerTree(const LayersProto& layersProto);
    static std::string layerTreeToString(const LayerTree& layerTree);

private:
    static std::vector<Layer> generateLayerList(const LayersProto& layersProto);
    static LayerProtoParser::Layer generateLayer(const LayerProto& layerProto);
    static LayerProtoParser::Region generateRegion(const RegionProto& regionProto);
    static LayerProtoParser::Rect generateRect(const RectProto& rectProto);
    static LayerProtoParser::FloatRect generateFloatRect(const FloatRectProto& rectProto);
    static LayerProtoParser::Transform generateTransform(const TransformProto& transformProto);
    static LayerProtoParser::ActiveBuffer generateActiveBuffer(
            const ActiveBufferProto& activeBufferProto);
    static void updateChildrenAndRelative(const LayerProto& layerProto,
                                          std::unordered_map<int32_t, Layer*>& layerMap);

    static std::string layerToString(const LayerProtoParser::Layer* layer);
};

} // namespace surfaceflinger
} // namespace android
