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

#include <compositionengine/impl/DumpHelpers.h>
#include <compositionengine/impl/OutputCompositionState.h>

namespace android::compositionengine::impl {

void OutputCompositionState::dump(std::string& out) const {
    out.append("   ");
    dumpVal(out, "isEnabled", isEnabled);
    dumpVal(out, "isSecure", isSecure);

    dumpVal(out, "layerStack", layerStackId);
    dumpVal(out, "layerStackInternal", layerStackInternal);

    out.append("\n   ");

    dumpVal(out, "transform", transform);

    out.append("\n   ");

    dumpVal(out, "frame", frame);
    dumpVal(out, "viewport", viewport);
    dumpVal(out, "scissor", scissor);
    dumpVal(out, "needsFiltering", needsFiltering);

    out.append("\n   ");

    dumpVal(out, "colorMode", toString(colorMode), colorMode);
    dumpVal(out, "renderIntent", toString(renderIntent), renderIntent);
    dumpVal(out, "dataspace", toString(dataspace), dataspace);
    dumpVal(out, "colorTransform", colorTransform);

    out.append("\n");
}

} // namespace android::compositionengine::impl
