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

#include <stdint.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <gui/GLConsumer.h>

namespace android {

#define NELEMS(x) ((int) (sizeof(x) / sizeof((x)[0])))

enum { MAX_NUM_LAYERS = 16 };
enum { MAX_TEST_RUNS = 16 };

class Composer;
class Renderer;
class GLHelper;

struct LayerDesc {
    uint32_t flags;
    Renderer* (*rendererFactory)();
    Composer* (*composerFactory)();
    int32_t x;
    int32_t y;
    uint32_t width;
    uint32_t height;
};

void resetColorGenerator();

class Composer {
public:
    virtual ~Composer() {}
    virtual bool setUp(const LayerDesc& desc, GLHelper* helper) = 0;
    virtual void tearDown() = 0;
    virtual bool compose(GLuint texName, const sp<GLConsumer>& glc) = 0;
};

Composer* nocomp();
Composer* opaque();
Composer* opaqueShrink();
Composer* blend();
Composer* blendShrink();

class Renderer {
public:
    virtual ~Renderer() {}
    virtual bool setUp(GLHelper* helper) = 0;
    virtual void tearDown() = 0;
    virtual bool render(EGLSurface surface) = 0;
};

Renderer* staticGradient();

} // namespace android
