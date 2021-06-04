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

#define LOG_TAG "InputWindow"
#define LOG_NDEBUG 0

#include <binder/Parcel.h>
#include <input/InputWindow.h>
#include <input/InputTransport.h>

#include <log/log.h>

#include <ui/Rect.h>
#include <ui/Region.h>

namespace android {

// --- InputWindowInfo ---
void InputWindowInfo::addTouchableRegion(const Rect& region) {
    touchableRegion.orSelf(region);
}

bool InputWindowInfo::touchableRegionContainsPoint(int32_t x, int32_t y) const {
    return touchableRegion.contains(x,y);
}

bool InputWindowInfo::frameContainsPoint(int32_t x, int32_t y) const {
    return x >= frameLeft && x < frameRight
            && y >= frameTop && y < frameBottom;
}

bool InputWindowInfo::isTrustedOverlay() const {
    return layoutParamsType == TYPE_INPUT_METHOD
            || layoutParamsType == TYPE_INPUT_METHOD_DIALOG
            || layoutParamsType == TYPE_MAGNIFICATION_OVERLAY
            || layoutParamsType == TYPE_STATUS_BAR
            || layoutParamsType == TYPE_NAVIGATION_BAR
            || layoutParamsType == TYPE_NAVIGATION_BAR_PANEL
            || layoutParamsType == TYPE_SECURE_SYSTEM_OVERLAY
            || layoutParamsType == TYPE_DOCK_DIVIDER
            || layoutParamsType == TYPE_ACCESSIBILITY_OVERLAY
            || layoutParamsType == TYPE_INPUT_CONSUMER;
}

bool InputWindowInfo::supportsSplitTouch() const {
    return layoutParamsFlags & FLAG_SPLIT_TOUCH;
}

bool InputWindowInfo::overlaps(const InputWindowInfo* other) const {
    return frameLeft < other->frameRight && frameRight > other->frameLeft
            && frameTop < other->frameBottom && frameBottom > other->frameTop;
}

status_t InputWindowInfo::write(Parcel& output) const {
    if (token == nullptr) {
        output.writeInt32(0);
        return OK;
    }
    output.writeInt32(1);
    status_t s = output.writeStrongBinder(token);
    if (s != OK) return s;

    output.writeString8(String8(name.c_str()));
    output.writeInt32(layoutParamsFlags);
    output.writeInt32(layoutParamsType);
    output.writeInt64(dispatchingTimeout);
    output.writeInt32(frameLeft);
    output.writeInt32(frameTop);
    output.writeInt32(frameRight);
    output.writeInt32(frameBottom);
    output.writeInt32(surfaceInset);
    output.writeFloat(globalScaleFactor);
    output.writeFloat(windowXScale);
    output.writeFloat(windowYScale);
    output.writeBool(visible);
    output.writeBool(canReceiveKeys);
    output.writeBool(hasFocus);
    output.writeBool(hasWallpaper);
    output.writeBool(paused);
    output.writeInt32(layer);
    output.writeInt32(ownerPid);
    output.writeInt32(ownerUid);
    output.writeInt32(inputFeatures);
    output.writeInt32(displayId);
    output.writeInt32(portalToDisplayId);
    applicationInfo.write(output);
    output.write(touchableRegion);
    output.writeBool(replaceTouchableRegionWithCrop);
    output.writeWeakBinder(touchableRegionCropHandle);
    return OK;
}

InputWindowInfo InputWindowInfo::read(const Parcel& from) {
    InputWindowInfo ret;

    if (from.readInt32() == 0) {
        return ret;
    }

    sp<IBinder> token = from.readStrongBinder();
    if (token == nullptr) {
        return ret;
    }

    ret.token = token;
    ret.name = from.readString8().c_str();
    ret.layoutParamsFlags = from.readInt32();
    ret.layoutParamsType = from.readInt32();
    ret.dispatchingTimeout = from.readInt64();
    ret.frameLeft = from.readInt32();
    ret.frameTop = from.readInt32();
    ret.frameRight = from.readInt32();
    ret.frameBottom = from.readInt32();
    ret.surfaceInset = from.readInt32();
    ret.globalScaleFactor = from.readFloat();
    ret.windowXScale = from.readFloat();
    ret.windowYScale = from.readFloat();
    ret.visible = from.readBool();
    ret.canReceiveKeys = from.readBool();
    ret.hasFocus = from.readBool();
    ret.hasWallpaper = from.readBool();
    ret.paused = from.readBool();
    ret.layer = from.readInt32();
    ret.ownerPid = from.readInt32();
    ret.ownerUid = from.readInt32();
    ret.inputFeatures = from.readInt32();
    ret.displayId = from.readInt32();
    ret.portalToDisplayId = from.readInt32();
    ret.applicationInfo = InputApplicationInfo::read(from);
    from.read(ret.touchableRegion);
    ret.replaceTouchableRegionWithCrop = from.readBool();
    ret.touchableRegionCropHandle = from.readWeakBinder();

    return ret;
}

InputWindowInfo::InputWindowInfo(const Parcel& from) {
    *this = read(from);
}

// --- InputWindowHandle ---

InputWindowHandle::InputWindowHandle() {
}

InputWindowHandle::~InputWindowHandle() {
}

void InputWindowHandle::releaseChannel() {
    mInfo.token.clear();
}

sp<IBinder> InputWindowHandle::getToken() const {
    return mInfo.token;
}

void InputWindowHandle::updateFrom(sp<InputWindowHandle> handle) {
    mInfo = handle->mInfo;
}

} // namespace android
