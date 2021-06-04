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

#include "Flatland.h"
#include "GLHelper.h"

namespace android {

static float colors[][4] = {
    { .85f, .14f, .44f, 1.0f },
    { .91f, .72f, .10f, 1.0f },
    { .04f, .66f, .42f, 1.0f },
    { .84f, .39f, .68f, 1.0f },
    { .38f, .53f, .78f, 1.0f },
};

static size_t g_colorIndex;

const float* genColor() {
    float* color = colors[g_colorIndex];
    g_colorIndex = (g_colorIndex + 1) % NELEMS(colors);
    return color;
}

void resetColorGenerator() {
    g_colorIndex = 0;
}

class GradientRenderer {

public:

    bool setUp(GLHelper* helper) {
        bool result;

        result = helper->getShaderProgram("Gradient", &mGradPgm);
        if (!result) {
            return false;
        }

        result = helper->getDitherTexture(&mDitherTexName);
        if (!result) {
            return false;
        }

        mPosAttribLoc = glGetAttribLocation(mGradPgm, "position");
        mUVAttribLoc = glGetAttribLocation(mGradPgm, "uv");
        mUVToInterpUniformLoc = glGetUniformLocation(mGradPgm, "uvToInterp");
        mObjToNdcUniformLoc = glGetUniformLocation(mGradPgm, "objToNdc");
        mDitherKernelSamplerLoc = glGetUniformLocation(mGradPgm, "ditherKernel");
        mInvDitherKernelSizeUniformLoc = glGetUniformLocation(mGradPgm,
                "invDitherKernelSize");
        mInvDitherKernelSizeSqUniformLoc = glGetUniformLocation(mGradPgm,
                "invDitherKernelSizeSq");
        mColor0UniformLoc = glGetUniformLocation(mGradPgm, "color0");
        mColor1UniformLoc = glGetUniformLocation(mGradPgm, "color1");

        return true;
    }

    void tearDown() {
    }

    bool drawGradient() {
        float identity[16] = {
            1.0f,   0.0f,   0.0f,   0.0f,
            0.0f,   1.0f,   0.0f,   0.0f,
            0.0f,   0.0f,   1.0f,   0.0f,
            0.0f,   0.0f,   0.0f,   1.0f,
        };
        const float pos[] = {
            -1.0f,  -1.0f,
            1.0f,   -1.0f,
            -1.0f,  1.0f,
            1.0f,   1.0f,
        };
        const float uv[] = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f,
        };
        const float* color0 = genColor();
        const float* color1 = genColor();

        glUseProgram(mGradPgm);

        glVertexAttribPointer(mPosAttribLoc, 2, GL_FLOAT, GL_FALSE, 0, pos);
        glVertexAttribPointer(mUVAttribLoc, 2, GL_FLOAT, GL_FALSE, 0, uv);
        glEnableVertexAttribArray(mPosAttribLoc);
        glEnableVertexAttribArray(mUVAttribLoc);

        float invDitherKernelSize = 1.0f / float(GLHelper::DITHER_KERNEL_SIZE);
        float invDitherKernelSizeSq = invDitherKernelSize * invDitherKernelSize;

        glUniformMatrix4fv(mObjToNdcUniformLoc, 1, GL_FALSE, identity);
        glUniformMatrix4fv(mUVToInterpUniformLoc, 1, GL_FALSE, identity);
        glUniform1f(mInvDitherKernelSizeUniformLoc, invDitherKernelSize);
        glUniform1f(mInvDitherKernelSizeSqUniformLoc, invDitherKernelSizeSq);
        glUniform4fv(mColor0UniformLoc, 1, color0);
        glUniform4fv(mColor1UniformLoc, 1, color1);

        if (glGetError() != GL_NO_ERROR) {
            fprintf(stderr, "GL error! 0\n");
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mDitherTexName);

        if (glGetError() != GL_NO_ERROR) {
            fprintf(stderr, "GL error! 1\n");
        }

        glUniform1i(mDitherKernelSamplerLoc, 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glDisableVertexAttribArray(mPosAttribLoc);
        glDisableVertexAttribArray(mUVAttribLoc);

        if (glGetError() != GL_NO_ERROR) {
            fprintf(stderr, "GL error! 2\n");
        }

        return true;
    }

    GLuint mGradPgm;
    GLuint mDitherTexName;
    GLuint mPosAttribLoc;
    GLuint mUVAttribLoc;
    GLuint mObjToNdcUniformLoc;
    GLuint mUVToInterpUniformLoc;
    GLuint mDitherKernelSamplerLoc;
    GLuint mInvDitherKernelSizeUniformLoc;
    GLuint mInvDitherKernelSizeSqUniformLoc;
    GLuint mColor0UniformLoc;
    GLuint mColor1UniformLoc;
};

Renderer* staticGradient() {
    class NoRenderer : public Renderer {
        virtual bool setUp(GLHelper* helper) {
            mIsFirstFrame = true;
            mGLHelper = helper;
            return mGradientRenderer.setUp(helper);
        }

        virtual void tearDown() {
            mGradientRenderer.tearDown();
        }

        virtual bool render(EGLSurface surface) {
            if (mIsFirstFrame) {
                bool result;
                mIsFirstFrame = false;

                result = mGLHelper->makeCurrent(surface);
                if (!result) {
                    return false;
                }

                result = mGradientRenderer.drawGradient();
                if (!result) {
                    return false;
                }

                result = mGLHelper->swapBuffers(surface);
                if (!result) {
                    return false;
                }
            }
            return true;
        }

        bool mIsFirstFrame;
        GLHelper* mGLHelper;
        GradientRenderer mGradientRenderer;
    };
    return new NoRenderer;
}


} // namespace android
