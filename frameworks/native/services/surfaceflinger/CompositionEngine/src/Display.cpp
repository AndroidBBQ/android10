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

#include <android-base/stringprintf.h>
#include <compositionengine/CompositionEngine.h>
#include <compositionengine/DisplayCreationArgs.h>
#include <compositionengine/DisplaySurface.h>
#include <compositionengine/impl/Display.h>
#include <compositionengine/impl/DisplayColorProfile.h>
#include <compositionengine/impl/DumpHelpers.h>
#include <compositionengine/impl/RenderSurface.h>

#include "DisplayHardware/HWComposer.h"

namespace android::compositionengine::impl {

std::shared_ptr<compositionengine::Display> createDisplay(
        const compositionengine::CompositionEngine& compositionEngine,
        compositionengine::DisplayCreationArgs&& args) {
    return std::make_shared<Display>(compositionEngine, std::move(args));
}

Display::Display(const CompositionEngine& compositionEngine, DisplayCreationArgs&& args)
      : compositionengine::impl::Output(compositionEngine),
        mIsVirtual(args.isVirtual),
        mId(args.displayId) {
    editState().isSecure = args.isSecure;
}

Display::~Display() = default;

const std::optional<DisplayId>& Display::getId() const {
    return mId;
}

bool Display::isSecure() const {
    return getState().isSecure;
}

bool Display::isVirtual() const {
    return mIsVirtual;
}

void Display::disconnect() {
    if (!mId) {
        return;
    }

    auto& hwc = getCompositionEngine().getHwComposer();
    hwc.disconnectDisplay(*mId);
    mId.reset();
}

void Display::setColorTransform(const mat4& transform) {
    Output::setColorTransform(transform);

    auto& hwc = getCompositionEngine().getHwComposer();
    status_t result = hwc.setColorTransform(*mId, transform);
    ALOGE_IF(result != NO_ERROR, "Failed to set color transform on display \"%s\": %d",
             mId ? to_string(*mId).c_str() : "", result);
}

void Display::setColorMode(ui::ColorMode mode, ui::Dataspace dataspace,
                           ui::RenderIntent renderIntent) {
    if (mode == getState().colorMode && dataspace == getState().dataspace &&
        renderIntent == getState().renderIntent) {
        return;
    }

    if (mIsVirtual) {
        ALOGW("%s: Invalid operation on virtual display", __FUNCTION__);
        return;
    }

    Output::setColorMode(mode, dataspace, renderIntent);

    auto& hwc = getCompositionEngine().getHwComposer();
    hwc.setActiveColorMode(*mId, mode, renderIntent);
}

void Display::dump(std::string& out) const {
    using android::base::StringAppendF;

    StringAppendF(&out, "   Composition Display State: [\"%s\"]", getName().c_str());

    out.append("\n   ");

    dumpVal(out, "isVirtual", mIsVirtual);
    if (mId) {
        dumpVal(out, "hwcId", to_string(*mId));
    } else {
        StringAppendF(&out, "no hwcId, ");
    }

    out.append("\n");

    Output::dumpBase(out);
}

void Display::createDisplayColorProfile(DisplayColorProfileCreationArgs&& args) {
    setDisplayColorProfile(compositionengine::impl::createDisplayColorProfile(std::move(args)));
}

void Display::createRenderSurface(RenderSurfaceCreationArgs&& args) {
    setRenderSurface(compositionengine::impl::createRenderSurface(getCompositionEngine(), *this,
                                                                  std::move(args)));
}

} // namespace android::compositionengine::impl
