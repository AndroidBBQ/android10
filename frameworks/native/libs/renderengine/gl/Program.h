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

#ifndef SF_RENDER_ENGINE_PROGRAM_H
#define SF_RENDER_ENGINE_PROGRAM_H

#include <stdint.h>

#include <GLES2/gl2.h>
#include <renderengine/private/Description.h>
#include "ProgramCache.h"

namespace android {

class String8;

namespace renderengine {
namespace gl {

/*
 * Abstracts a GLSL program comprising a vertex and fragment shader
 */
class Program {
public:
    // known locations for position and texture coordinates
    enum {
        /* position of each vertex for vertex shader */
        position = 0,

        /* UV coordinates for texture mapping */
        texCoords = 1,

        /* Crop coordinates, in pixels */
        cropCoords = 2
    };

    Program(const ProgramCache::Key& needs, const char* vertex, const char* fragment);
    ~Program() = default;

    /* whether this object is usable */
    bool isValid() const;

    /* Binds this program to the GLES context */
    void use();

    /* Returns the location of the specified attribute */
    GLuint getAttrib(const char* name) const;

    /* Returns the location of the specified uniform */
    GLint getUniform(const char* name) const;

    /* set-up uniforms from the description */
    void setUniforms(const Description& desc);

private:
    GLuint buildShader(const char* source, GLenum type);

    // whether the initialization succeeded
    bool mInitialized;

    // Name of the OpenGL program and shaders
    GLuint mProgram;
    GLuint mVertexShader;
    GLuint mFragmentShader;

    /* location of the projection matrix uniform */
    GLint mProjectionMatrixLoc;

    /* location of the texture matrix uniform */
    GLint mTextureMatrixLoc;

    /* location of the sampler uniform */
    GLint mSamplerLoc;

    /* location of the color uniform */
    GLint mColorLoc;

    /* location of display luminance uniform */
    GLint mDisplayMaxLuminanceLoc;

    /* location of transform matrix */
    GLint mInputTransformMatrixLoc;
    GLint mOutputTransformMatrixLoc;

    /* location of corner radius uniform */
    GLint mCornerRadiusLoc;

    /* location of surface crop origin uniform, for rounded corner clipping */
    GLint mCropCenterLoc;
};

} // namespace gl
} // namespace renderengine
} // namespace android

#endif /* SF_RENDER_ENGINE_PROGRAM_H */
