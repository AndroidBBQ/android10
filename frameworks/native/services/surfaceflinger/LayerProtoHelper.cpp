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

#include "LayerProtoHelper.h"

namespace android {
namespace surfaceflinger {

void LayerProtoHelper::writePositionToProto(const float x, const float y,
                                            std::function<PositionProto*()> getPositionProto) {
    if (x != 0 || y != 0) {
        // Use a lambda do avoid writing the object header when the object is empty
        PositionProto* position = getPositionProto();
        position->set_x(x);
        position->set_y(y);
    }
}

void LayerProtoHelper::writeSizeToProto(const uint32_t w, const uint32_t h,
                                        std::function<SizeProto*()> getSizeProto) {
    if (w != 0 || h != 0) {
        // Use a lambda do avoid writing the object header when the object is empty
        SizeProto* size = getSizeProto();
        size->set_w(w);
        size->set_h(h);
    }
}

void LayerProtoHelper::writeToProto(const Region& region,
                                    std::function<RegionProto*()> getRegionProto) {
    if (region.isEmpty()) {
        return;
    }

    Region::const_iterator head = region.begin();
    Region::const_iterator const tail = region.end();
    // Use a lambda do avoid writing the object header when the object is empty
    RegionProto* regionProto = getRegionProto();
    while (head != tail) {
        std::function<RectProto*()> getProtoRect = [&]() { return regionProto->add_rect(); };
        writeToProto(*head, getProtoRect);
        head++;
    }
}

void LayerProtoHelper::writeToProto(const Rect& rect, std::function<RectProto*()> getRectProto) {
    if (rect.left != 0 || rect.right != 0 || rect.top != 0 || rect.bottom != 0) {
        // Use a lambda do avoid writing the object header when the object is empty
        RectProto* rectProto = getRectProto();
        rectProto->set_left(rect.left);
        rectProto->set_top(rect.top);
        rectProto->set_bottom(rect.bottom);
        rectProto->set_right(rect.right);
    }
}

void LayerProtoHelper::writeToProto(const FloatRect& rect,
                                    std::function<FloatRectProto*()> getFloatRectProto) {
    if (rect.left != 0 || rect.right != 0 || rect.top != 0 || rect.bottom != 0) {
        // Use a lambda do avoid writing the object header when the object is empty
        FloatRectProto* rectProto = getFloatRectProto();
        rectProto->set_left(rect.left);
        rectProto->set_top(rect.top);
        rectProto->set_bottom(rect.bottom);
        rectProto->set_right(rect.right);
    }
}

void LayerProtoHelper::writeToProto(const half4 color, std::function<ColorProto*()> getColorProto) {
    if (color.r != 0 || color.g != 0 || color.b != 0 || color.a != 0) {
        // Use a lambda do avoid writing the object header when the object is empty
        ColorProto* colorProto = getColorProto();
        colorProto->set_r(color.r);
        colorProto->set_g(color.g);
        colorProto->set_b(color.b);
        colorProto->set_a(color.a);
    }
}

void LayerProtoHelper::writeToProto(const ui::Transform& transform,
                                    TransformProto* transformProto) {
    const uint32_t type = transform.getType() | (transform.getOrientation() << 8);
    transformProto->set_type(type);

    // Rotations that are 90/180/270 have their own type so the transform matrix can be
    // reconstructed later. All other rotation have the type UKNOWN so we need to save the transform
    // values in that case.
    if (type & (ui::Transform::SCALE | ui::Transform::UNKNOWN)) {
        transformProto->set_dsdx(transform[0][0]);
        transformProto->set_dtdx(transform[0][1]);
        transformProto->set_dsdy(transform[1][0]);
        transformProto->set_dtdy(transform[1][1]);
    }
}

void LayerProtoHelper::writeToProto(const sp<GraphicBuffer>& buffer,
                                    std::function<ActiveBufferProto*()> getActiveBufferProto) {
    if (buffer->getWidth() != 0 || buffer->getHeight() != 0 || buffer->getStride() != 0 ||
        buffer->format != 0) {
        // Use a lambda do avoid writing the object header when the object is empty
        ActiveBufferProto* activeBufferProto = getActiveBufferProto();
        activeBufferProto->set_width(buffer->getWidth());
        activeBufferProto->set_height(buffer->getHeight());
        activeBufferProto->set_stride(buffer->getStride());
        activeBufferProto->set_format(buffer->format);
    }
}

void LayerProtoHelper::writeToProto(
        const InputWindowInfo& inputInfo, const wp<Layer>& touchableRegionBounds,
        std::function<InputWindowInfoProto*()> getInputWindowInfoProto) {
    if (inputInfo.token == nullptr) {
        return;
    }

    InputWindowInfoProto* proto = getInputWindowInfoProto();
    proto->set_layout_params_flags(inputInfo.layoutParamsFlags);
    proto->set_layout_params_type(inputInfo.layoutParamsType);

    LayerProtoHelper::writeToProto({inputInfo.frameLeft, inputInfo.frameTop, inputInfo.frameRight,
                                    inputInfo.frameBottom},
                                   [&]() { return proto->mutable_frame(); });
    LayerProtoHelper::writeToProto(inputInfo.touchableRegion,
                                   [&]() { return proto->mutable_touchable_region(); });

    proto->set_surface_inset(inputInfo.surfaceInset);
    proto->set_visible(inputInfo.visible);
    proto->set_can_receive_keys(inputInfo.canReceiveKeys);
    proto->set_has_focus(inputInfo.hasFocus);
    proto->set_has_wallpaper(inputInfo.hasWallpaper);

    proto->set_global_scale_factor(inputInfo.globalScaleFactor);
    proto->set_window_x_scale(inputInfo.windowXScale);
    proto->set_window_y_scale(inputInfo.windowYScale);
    proto->set_replace_touchable_region_with_crop(inputInfo.replaceTouchableRegionWithCrop);
    auto cropLayer = touchableRegionBounds.promote();
    if (cropLayer != nullptr) {
        proto->set_crop_layer_id(cropLayer->sequence);
        LayerProtoHelper::writeToProto(cropLayer->getScreenBounds(
                                               false /* reduceTransparentRegion */),
                                       [&]() { return proto->mutable_touchable_region_crop(); });
    }
}

} // namespace surfaceflinger
} // namespace android
