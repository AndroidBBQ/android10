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
#include <compositionengine/impl/DumpHelpers.h>
#include <compositionengine/impl/LayerCompositionState.h>

namespace android::compositionengine::impl {

namespace {

using android::compositionengine::impl::dumpVal;

void dumpVal(std::string& out, const char* name, Hwc2::IComposerClient::Color value) {
    using android::base::StringAppendF;
    StringAppendF(&out, "%s=[%d %d %d] ", name, value.r, value.g, value.b);
}

void dumpFrontEnd(std::string& out, const LayerFECompositionState& state) {
    out.append("      ");
    dumpVal(out, "isSecure", state.isSecure);
    dumpVal(out, "geomUsesSourceCrop", state.geomUsesSourceCrop);
    dumpVal(out, "geomBufferUsesDisplayInverseTransform",
            state.geomBufferUsesDisplayInverseTransform);
    dumpVal(out, "geomLayerTransform", state.geomLayerTransform);

    out.append("\n      ");
    dumpVal(out, "geomBufferSize", state.geomBufferSize);
    dumpVal(out, "geomContentCrop", state.geomContentCrop);
    dumpVal(out, "geomCrop", state.geomCrop);
    dumpVal(out, "geomBufferTransform", state.geomBufferTransform);

    out.append("\n      ");
    dumpVal(out, "geomActiveTransparentRegion", state.geomActiveTransparentRegion);

    out.append("      ");
    dumpVal(out, "geomLayerBounds", state.geomLayerBounds);

    out.append("\n      ");
    dumpVal(out, "blend", toString(state.blendMode), state.blendMode);
    dumpVal(out, "alpha", state.alpha);

    out.append("\n      ");
    dumpVal(out, "type", state.type);
    dumpVal(out, "appId", state.appId);

    dumpVal(out, "composition type", toString(state.compositionType), state.compositionType);

    out.append("\n      buffer: ");
    dumpVal(out, "buffer", state.buffer.get());
    dumpVal(out, "slot", state.bufferSlot);

    out.append("\n      ");
    dumpVal(out, "sideband stream", state.sidebandStream.get());

    out.append("\n      ");
    dumpVal(out, "color", state.color);

    out.append("\n      ");
    dumpVal(out, "dataspace", toString(state.dataspace), state.dataspace);
    dumpVal(out, "hdr metadata types", state.hdrMetadata.validTypes);
    dumpVal(out, "colorTransform", state.colorTransform);

    out.append("\n");
}

} // namespace

void LayerCompositionState::dump(std::string& out) const {
    out.append("    frontend:\n");
    dumpFrontEnd(out, frontEnd);
}

} // namespace android::compositionengine::impl
