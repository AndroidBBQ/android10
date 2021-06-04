/*
 * Copyright 2013 The Android Open Source Project
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

#include <renderengine/private/Description.h>

#include <stdint.h>

#include <utils/TypeHelpers.h>

namespace android {
namespace renderengine {

Description::TransferFunction Description::dataSpaceToTransferFunction(ui::Dataspace dataSpace) {
    ui::Dataspace transfer = static_cast<ui::Dataspace>(dataSpace & ui::Dataspace::TRANSFER_MASK);
    switch (transfer) {
        case ui::Dataspace::TRANSFER_ST2084:
            return Description::TransferFunction::ST2084;
        case ui::Dataspace::TRANSFER_HLG:
            return Description::TransferFunction::HLG;
        case ui::Dataspace::TRANSFER_LINEAR:
            return Description::TransferFunction::LINEAR;
        default:
            return Description::TransferFunction::SRGB;
    }
}

bool Description::hasInputTransformMatrix() const {
    const mat4 identity;
    return inputTransformMatrix != identity;
}

bool Description::hasOutputTransformMatrix() const {
    const mat4 identity;
    return outputTransformMatrix != identity;
}

bool Description::hasColorMatrix() const {
    const mat4 identity;
    return colorMatrix != identity;
}

} // namespace renderengine
} // namespace android
