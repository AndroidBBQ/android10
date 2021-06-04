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

#ifndef SF_RENDER_ENGINE_TEXTURE_H
#define SF_RENDER_ENGINE_TEXTURE_H

#include <stdint.h>

#include <math/mat4.h>

namespace android {
namespace renderengine {

class Texture {
public:
    enum Target { TEXTURE_2D = 0x0DE1, TEXTURE_EXTERNAL = 0x8D65 };

    Texture();
    Texture(Target textureTarget, uint32_t textureName);
    ~Texture();

    void init(Target textureTarget, uint32_t textureName);

    void setMatrix(float const* matrix);
    void setFiltering(bool enabled);
    void setDimensions(size_t width, size_t height);

    uint32_t getTextureName() const;
    uint32_t getTextureTarget() const;

    const mat4& getMatrix() const;
    bool getFiltering() const;
    size_t getWidth() const;
    size_t getHeight() const;

private:
    uint32_t mTextureName;
    uint32_t mTextureTarget;
    size_t mWidth;
    size_t mHeight;
    bool mFiltering;
    mat4 mTextureMatrix;
};

} // namespace renderengine
} // namespace android
#endif /* SF_RENDER_ENGINE_TEXTURE_H */
