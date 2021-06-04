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

#ifndef SF_RENDER_ENGINE_DESCRIPTION_H_
#define SF_RENDER_ENGINE_DESCRIPTION_H_

#include <renderengine/Texture.h>
#include <ui/GraphicTypes.h>

namespace android {
namespace renderengine {

/*
 * This is the structure that holds the state of the rendering engine.
 * This class is used to generate a corresponding GLSL program and set the
 * appropriate uniform.
 */
struct Description {
    enum class TransferFunction : int {
        LINEAR,
        SRGB,
        ST2084,
        HLG, // Hybrid Log-Gamma for HDR.
    };

    static TransferFunction dataSpaceToTransferFunction(ui::Dataspace dataSpace);

    Description() = default;
    ~Description() = default;

    bool hasInputTransformMatrix() const;
    bool hasOutputTransformMatrix() const;
    bool hasColorMatrix() const;

    // whether textures are premultiplied
    bool isPremultipliedAlpha = false;
    // whether this layer is marked as opaque
    bool isOpaque = true;

    // corner radius of the layer
    float cornerRadius = 0;

    // Size of the rounded rectangle we are cropping to
    half2 cropSize;

    // Texture this layer uses
    Texture texture;
    bool textureEnabled = false;

    // color used when texturing is disabled or when setting alpha.
    half4 color;

    // true if the sampled pixel values are in Y410/BT2020 rather than RGBA
    bool isY410BT2020 = false;

    // transfer functions for the input/output
    TransferFunction inputTransferFunction = TransferFunction::LINEAR;
    TransferFunction outputTransferFunction = TransferFunction::LINEAR;

    float displayMaxLuminance;

    // projection matrix
    mat4 projectionMatrix;

    // The color matrix will be applied in linear space right before OETF.
    mat4 colorMatrix;
    mat4 inputTransformMatrix;
    mat4 outputTransformMatrix;
};

} // namespace renderengine
} // namespace android

#endif /* SF_RENDER_ENGINE_DESCRIPTION_H_ */
