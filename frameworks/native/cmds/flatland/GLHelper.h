/*
 * Copyright (C) 2012 The Android Open Source Project
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

#include <gui/GLConsumer.h>
#include <gui/Surface.h>
#include <gui/SurfaceControl.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

namespace android {

class SurfaceComposerClient;
class SurfaceControl;

enum { MAX_SHADER_LINES = 128 };

struct ShaderDesc {
    const char* name;
    const char* vertexShader[MAX_SHADER_LINES];
    const char* fragmentShader[MAX_SHADER_LINES];
};

class GLHelper {

public:

    enum { DITHER_KERNEL_SIZE = 4 };

    GLHelper();

    ~GLHelper();

    bool setUp(const ShaderDesc* shaderDescs, size_t numShaders);

    void tearDown();

    bool makeCurrent(EGLSurface surface);

    bool createSurfaceTexture(uint32_t w, uint32_t h,
            sp<GLConsumer>* surfaceTexture, EGLSurface* surface,
            GLuint* name);

    bool createWindowSurface(uint32_t w, uint32_t h,
            sp<SurfaceControl>* surfaceControl, EGLSurface* surface);

    void destroySurface(EGLSurface* surface);

    bool swapBuffers(EGLSurface surface);

    bool getShaderProgram(const char* name, GLuint* outPgm);

    bool getDitherTexture(GLuint* outTexName);

private:

    bool createNamedSurfaceTexture(GLuint name, uint32_t w, uint32_t h,
            sp<GLConsumer>* surfaceTexture, EGLSurface* surface);

    bool computeWindowScale(uint32_t w, uint32_t h, float* scale);

    bool setUpShaders(const ShaderDesc* shaderDescs, size_t numShaders);

    EGLDisplay mDisplay;
    EGLContext mContext;
    EGLSurface mDummySurface;
    sp<GLConsumer> mDummyGLConsumer;
    EGLConfig mConfig;

    sp<SurfaceComposerClient> mSurfaceComposerClient;

    GLuint* mShaderPrograms;
    const ShaderDesc* mShaderDescs;
    size_t mNumShaders;

    GLuint mDitherTexture;
};

} // namespace android
