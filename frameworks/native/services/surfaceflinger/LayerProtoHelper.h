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

#include <layerproto/LayerProtoHeader.h>

#include <Layer.h>
#include <input/InputWindow.h>
#include <math/vec4.h>
#include <ui/GraphicBuffer.h>
#include <ui/Rect.h>
#include <ui/Region.h>
#include <ui/Transform.h>

namespace android {
namespace surfaceflinger {
class LayerProtoHelper {
public:
    static void writePositionToProto(const float x, const float y,
                                     std::function<PositionProto*()> getPositionProto);
    static void writeSizeToProto(const uint32_t w, const uint32_t h,
                                 std::function<SizeProto*()> getSizeProto);
    static void writeToProto(const Rect& rect, std::function<RectProto*()> getRectProto);
    static void writeToProto(const FloatRect& rect,
                             std::function<FloatRectProto*()> getFloatRectProto);
    static void writeToProto(const Region& region, std::function<RegionProto*()> getRegionProto);
    static void writeToProto(const half4 color, std::function<ColorProto*()> getColorProto);
    static void writeToProto(const ui::Transform& transform, TransformProto* transformProto);
    static void writeToProto(const sp<GraphicBuffer>& buffer,
                             std::function<ActiveBufferProto*()> getActiveBufferProto);
    static void writeToProto(const InputWindowInfo& inputInfo,
                             const wp<Layer>& touchableRegionBounds,
                             std::function<InputWindowInfoProto*()> getInputWindowInfoProto);
};

} // namespace surfaceflinger
} // namespace android
