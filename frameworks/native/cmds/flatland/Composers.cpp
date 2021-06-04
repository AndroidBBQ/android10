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

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "Flatland.h"
#include "GLHelper.h"

namespace android {

class Blitter {
public:

    bool setUp(GLHelper* helper) {
        bool result;

        result = helper->getShaderProgram("Blit", &mBlitPgm);
        if (!result) {
            return false;
        }

        mPosAttribLoc = glGetAttribLocation(mBlitPgm, "position");
        mUVAttribLoc = glGetAttribLocation(mBlitPgm, "uv");
        mUVToTexUniformLoc = glGetUniformLocation(mBlitPgm, "uvToTex");
        mObjToNdcUniformLoc = glGetUniformLocation(mBlitPgm, "objToNdc");
        mBlitSrcSamplerLoc = glGetUniformLocation(mBlitPgm, "blitSrc");
        mModColorUniformLoc = glGetUniformLocation(mBlitPgm, "modColor");

        return true;
    }

    bool blit(GLuint texName, const float* texMatrix,
            int32_t x, int32_t y, uint32_t w, uint32_t h) {
        float modColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        return modBlit(texName, texMatrix, modColor, x, y, w, h);
    }

    bool modBlit(GLuint texName, const float* texMatrix, float* modColor,
            int32_t x, int32_t y, uint32_t w, uint32_t h) {
        glUseProgram(mBlitPgm);

        GLint vp[4];
        glGetIntegerv(GL_VIEWPORT, vp);
        float screenToNdc[16] = {
            2.0f/float(vp[2]),  0.0f,               0.0f,   0.0f,
            0.0f,               -2.0f/float(vp[3]), 0.0f,   0.0f,
            0.0f,               0.0f,               1.0f,   0.0f,
            -1.0f,              1.0f,               0.0f,   1.0f,
        };
        const float pos[] = {
            float(x),   float(y),
            float(x+w), float(y),
            float(x),   float(y+h),
            float(x+w), float(y+h),
        };
        const float uv[] = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f,
        };

        glVertexAttribPointer(mPosAttribLoc, 2, GL_FLOAT, GL_FALSE, 0, pos);
        glVertexAttribPointer(mUVAttribLoc, 2, GL_FLOAT, GL_FALSE, 0, uv);
        glEnableVertexAttribArray(mPosAttribLoc);
        glEnableVertexAttribArray(mUVAttribLoc);

        glUniformMatrix4fv(mObjToNdcUniformLoc, 1, GL_FALSE, screenToNdc);
        glUniformMatrix4fv(mUVToTexUniformLoc, 1, GL_FALSE, texMatrix);
        glUniform4fv(mModColorUniformLoc, 1, modColor);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, texName);
        glUniform1i(mBlitSrcSamplerLoc, 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glDisableVertexAttribArray(mPosAttribLoc);
        glDisableVertexAttribArray(mUVAttribLoc);

        if (glGetError() != GL_NO_ERROR) {
            fprintf(stderr, "GL error!\n");
        }

        return true;
    }

private:
    GLuint mBlitPgm;
    GLint mPosAttribLoc;
    GLint mUVAttribLoc;
    GLint mUVToTexUniformLoc;
    GLint mObjToNdcUniformLoc;
    GLint mBlitSrcSamplerLoc;
    GLint mModColorUniformLoc;
};

class ComposerBase : public Composer {
public:
    virtual ~ComposerBase() {}

    virtual bool setUp(const LayerDesc& desc,
            GLHelper* helper) {
        mLayerDesc = desc;
        return setUp(helper);
    }

    virtual void tearDown() {
    }

    virtual bool compose(GLuint /*texName*/, const sp<GLConsumer>& /*glc*/) {
        return true;
    }

protected:
    virtual bool setUp(GLHelper* /*helper*/) {
        return true;
    }

    LayerDesc mLayerDesc;
};

Composer* nocomp() {
    class NoComp : public ComposerBase {
    };
    return new NoComp();
}

Composer* opaque() {
    class OpaqueComp : public ComposerBase {
        virtual bool setUp(GLHelper* helper) {
            return mBlitter.setUp(helper);
        }

        virtual bool compose(GLuint texName, const sp<GLConsumer>& glc) {
            float texMatrix[16];
            glc->getTransformMatrix(texMatrix);

            int32_t x = mLayerDesc.x;
            int32_t y = mLayerDesc.y;
            int32_t w = mLayerDesc.width;
            int32_t h = mLayerDesc.height;

            return mBlitter.blit(texName, texMatrix, x, y, w, h);
        }

        Blitter mBlitter;
    };
    return new OpaqueComp();
}

Composer* opaqueShrink() {
    class OpaqueComp : public ComposerBase {
        virtual bool setUp(GLHelper* helper) {
            mParity = false;
            return mBlitter.setUp(helper);
        }

        virtual bool compose(GLuint texName, const sp<GLConsumer>& glc) {
            float texMatrix[16];
            glc->getTransformMatrix(texMatrix);

            int32_t x = mLayerDesc.x;
            int32_t y = mLayerDesc.y;
            int32_t w = mLayerDesc.width;
            int32_t h = mLayerDesc.height;

            mParity = !mParity;
            if (mParity) {
                x += w / 128;
                y += h / 128;
                w -= w / 64;
                h -= h / 64;
            }

            return mBlitter.blit(texName, texMatrix, x, y, w, h);
        }

        Blitter mBlitter;
        bool mParity;
    };
    return new OpaqueComp();
}

Composer* blend() {
    class BlendComp : public ComposerBase {
        virtual bool setUp(GLHelper* helper) {
            return mBlitter.setUp(helper);
        }

        virtual bool compose(GLuint texName, const sp<GLConsumer>& glc) {
            bool result;

            float texMatrix[16];
            glc->getTransformMatrix(texMatrix);

            float modColor[4] = { .75f, .75f, .75f, .75f };

            int32_t x = mLayerDesc.x;
            int32_t y = mLayerDesc.y;
            int32_t w = mLayerDesc.width;
            int32_t h = mLayerDesc.height;

            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

            result = mBlitter.modBlit(texName, texMatrix, modColor,
                    x, y, w, h);
            if (!result) {
                return false;
            }

            glDisable(GL_BLEND);

            return true;
        }

        Blitter mBlitter;
    };
    return new BlendComp();
}

Composer* blendShrink() {
    class BlendShrinkComp : public ComposerBase {
        virtual bool setUp(GLHelper* helper) {
            mParity = false;
            return mBlitter.setUp(helper);
        }

        virtual bool compose(GLuint texName, const sp<GLConsumer>& glc) {
            bool result;

            float texMatrix[16];
            glc->getTransformMatrix(texMatrix);

            float modColor[4] = { .75f, .75f, .75f, .75f };

            int32_t x = mLayerDesc.x;
            int32_t y = mLayerDesc.y;
            int32_t w = mLayerDesc.width;
            int32_t h = mLayerDesc.height;

            mParity = !mParity;
            if (mParity) {
                x += w / 128;
                y += h / 128;
                w -= w / 64;
                h -= h / 64;
            }

            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

            result = mBlitter.modBlit(texName, texMatrix, modColor,
                    x, y, w, h);
            if (!result) {
                return false;
            }

            glDisable(GL_BLEND);

            return true;
        }

        Blitter mBlitter;
        bool mParity;
    };
    return new BlendShrinkComp();
}

} // namespace android
