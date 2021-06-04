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

#include <ui/DisplayInfo.h>
#include <gui/SurfaceComposerClient.h>

#include "GLHelper.h"

 namespace android {

GLHelper::GLHelper() :
    mDisplay(EGL_NO_DISPLAY),
    mContext(EGL_NO_CONTEXT),
    mDummySurface(EGL_NO_SURFACE),
    mConfig(0),
    mShaderPrograms(nullptr),
    mDitherTexture(0) {
}

GLHelper::~GLHelper() {
}

bool GLHelper::setUp(const ShaderDesc* shaderDescs, size_t numShaders) {
    bool result;

    mDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (mDisplay == EGL_NO_DISPLAY) {
        fprintf(stderr, "eglGetDisplay error: %#x\n", eglGetError());
        return false;
    }

    EGLint majorVersion;
    EGLint minorVersion;
    result = eglInitialize(mDisplay, &majorVersion, &minorVersion);
    if (result != EGL_TRUE) {
        fprintf(stderr, "eglInitialize error: %#x\n", eglGetError());
        return false;
    }

    EGLint numConfigs = 0;
    EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };
    result = eglChooseConfig(mDisplay, configAttribs, &mConfig, 1,
            &numConfigs);
    if (result != EGL_TRUE) {
        fprintf(stderr, "eglChooseConfig error: %#x\n", eglGetError());
        return false;
    }

    EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    mContext = eglCreateContext(mDisplay, mConfig, EGL_NO_CONTEXT,
            contextAttribs);
    if (mContext == EGL_NO_CONTEXT) {
        fprintf(stderr, "eglCreateContext error: %#x\n", eglGetError());
        return false;
    }

    bool resultb = createNamedSurfaceTexture(0, 1, 1, &mDummyGLConsumer,
            &mDummySurface);
    if (!resultb) {
        return false;
    }

    resultb = makeCurrent(mDummySurface);
    if (!resultb) {
        return false;
    }

    resultb = setUpShaders(shaderDescs, numShaders);
    if (!resultb) {
        return false;
    }

    return true;
}

void GLHelper::tearDown() {
    if (mShaderPrograms != nullptr) {
        delete[] mShaderPrograms;
        mShaderPrograms = nullptr;
    }

    if (mSurfaceComposerClient != nullptr) {
        mSurfaceComposerClient->dispose();
        mSurfaceComposerClient.clear();
    }

    if (mDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE,
                EGL_NO_CONTEXT);
    }

    if (mContext != EGL_NO_CONTEXT) {
        eglDestroyContext(mDisplay, mContext);
    }

    if (mDummySurface != EGL_NO_SURFACE) {
        eglDestroySurface(mDisplay, mDummySurface);
    }

    mDisplay = EGL_NO_DISPLAY;
    mContext = EGL_NO_CONTEXT;
    mDummySurface = EGL_NO_SURFACE;
    mDummyGLConsumer.clear();
    mConfig = 0;
}

bool GLHelper::makeCurrent(EGLSurface surface) {
    EGLint result;

    result = eglMakeCurrent(mDisplay, surface, surface, mContext);
    if (result != EGL_TRUE) {
        fprintf(stderr, "eglMakeCurrent error: %#x\n", eglGetError());
        return false;
    }

    EGLint w, h;
    eglQuerySurface(mDisplay, surface, EGL_WIDTH, &w);
    eglQuerySurface(mDisplay, surface, EGL_HEIGHT, &h);
    glViewport(0, 0, w, h);

    return true;
}

bool GLHelper::createSurfaceTexture(uint32_t w, uint32_t h,
        sp<GLConsumer>* glConsumer, EGLSurface* surface,
        GLuint* name) {
    if (!makeCurrent(mDummySurface)) {
        return false;
    }

    *name = 0;
    glGenTextures(1, name);
    if (*name == 0) {
        fprintf(stderr, "glGenTextures error: %#x\n", glGetError());
        return false;
    }

    return createNamedSurfaceTexture(*name, w, h, glConsumer, surface);
}

void GLHelper::destroySurface(EGLSurface* surface) {
    if (eglGetCurrentSurface(EGL_READ) == *surface ||
            eglGetCurrentSurface(EGL_DRAW) == *surface) {
        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE,
                EGL_NO_CONTEXT);
    }
    eglDestroySurface(mDisplay, *surface);
    *surface = EGL_NO_SURFACE;
}

bool GLHelper::swapBuffers(EGLSurface surface) {
    EGLint result;
    result = eglSwapBuffers(mDisplay, surface);
    if (result != EGL_TRUE) {
        fprintf(stderr, "eglSwapBuffers error: %#x\n", eglGetError());
        return false;
    }
    return true;
}

bool GLHelper::getShaderProgram(const char* name, GLuint* outPgm) {
    for (size_t i = 0; i < mNumShaders; i++) {
        if (strcmp(mShaderDescs[i].name, name) == 0) {
            *outPgm = mShaderPrograms[i];
            return true;
        }
    }

    fprintf(stderr, "unknown shader name: \"%s\"\n", name);

    return false;
}

bool GLHelper::createNamedSurfaceTexture(GLuint name, uint32_t w, uint32_t h,
        sp<GLConsumer>* glConsumer, EGLSurface* surface) {
    sp<IGraphicBufferProducer> producer;
    sp<IGraphicBufferConsumer> consumer;
    BufferQueue::createBufferQueue(&producer, &consumer);
    sp<GLConsumer> glc = new GLConsumer(consumer, name,
            GL_TEXTURE_EXTERNAL_OES, false, true);
    glc->setDefaultBufferSize(w, h);
    producer->setMaxDequeuedBufferCount(2);
    glc->setConsumerUsageBits(GRALLOC_USAGE_HW_COMPOSER);

    sp<ANativeWindow> anw = new Surface(producer);
    EGLSurface s = eglCreateWindowSurface(mDisplay, mConfig, anw.get(), nullptr);
    if (s == EGL_NO_SURFACE) {
        fprintf(stderr, "eglCreateWindowSurface error: %#x\n", eglGetError());
        return false;
    }

    *glConsumer = glc;
    *surface = s;
    return true;
}

bool GLHelper::computeWindowScale(uint32_t w, uint32_t h, float* scale) {
    const sp<IBinder> dpy = mSurfaceComposerClient->getInternalDisplayToken();
    if (dpy == nullptr) {
        fprintf(stderr, "SurfaceComposer::getInternalDisplayToken failed.\n");
        return false;
    }

    DisplayInfo info;
    status_t err = mSurfaceComposerClient->getDisplayInfo(dpy, &info);
    if (err != NO_ERROR) {
        fprintf(stderr, "SurfaceComposer::getDisplayInfo failed: %#x\n", err);
        return false;
    }

    float scaleX = float(info.w) / float(w);
    float scaleY = float(info.h) / float(h);
    *scale = scaleX < scaleY ? scaleX : scaleY;

    return true;
}

bool GLHelper::createWindowSurface(uint32_t w, uint32_t h,
        sp<SurfaceControl>* surfaceControl, EGLSurface* surface) {
    bool result;
    status_t err;

    if (mSurfaceComposerClient == nullptr) {
        mSurfaceComposerClient = new SurfaceComposerClient;
    }
    err = mSurfaceComposerClient->initCheck();
    if (err != NO_ERROR) {
        fprintf(stderr, "SurfaceComposerClient::initCheck error: %#x\n", err);
        return false;
    }

    sp<SurfaceControl> sc = mSurfaceComposerClient->createSurface(
            String8("Benchmark"), w, h, PIXEL_FORMAT_RGBA_8888, 0);
    if (sc == nullptr || !sc->isValid()) {
        fprintf(stderr, "Failed to create SurfaceControl.\n");
        return false;
    }

    float scale;
    result = computeWindowScale(w, h, &scale);
    if (!result) {
        return false;
    }

    SurfaceComposerClient::Transaction{}.setLayer(sc, 0x7FFFFFFF)
            .setMatrix(sc, scale, 0.0f, 0.0f, scale)
            .show(sc)
            .apply();

    sp<ANativeWindow> anw = sc->getSurface();
    EGLSurface s = eglCreateWindowSurface(mDisplay, mConfig, anw.get(), nullptr);
    if (s == EGL_NO_SURFACE) {
        fprintf(stderr, "eglCreateWindowSurface error: %#x\n", eglGetError());
        return false;
    }

    *surfaceControl = sc;
    *surface = s;
    return true;
}

static bool compileShader(GLenum shaderType, const char* src,
        GLuint* outShader) {
    GLuint shader = glCreateShader(shaderType);
    if (shader == 0) {
        fprintf(stderr, "glCreateShader error: %#x\n", glGetError());
        return false;
    }

    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen) {
            char* buf = new char[infoLen];
            if (buf) {
                glGetShaderInfoLog(shader, infoLen, nullptr, buf);
                fprintf(stderr, "Shader compile log:\n%s\n", buf);
                delete[] buf;
            }
        }
        glDeleteShader(shader);
        return false;
    }
    *outShader = shader;
    return true;
}

static void printShaderSource(const char* const* src) {
    for (size_t i = 0; i < MAX_SHADER_LINES && src[i] != nullptr; i++) {
        fprintf(stderr, "%3zu: %s\n", i+1, src[i]);
    }
}

static const char* makeShaderString(const char* const* src) {
    size_t len = 0;
    for (size_t i = 0; i < MAX_SHADER_LINES && src[i] != nullptr; i++) {
        // The +1 is for the '\n' that will be added.
        len += strlen(src[i]) + 1;
    }

    char* result = new char[len+1];
    char* end = result;
    for (size_t i = 0; i < MAX_SHADER_LINES && src[i] != nullptr; i++) {
        strcpy(end, src[i]);
        end += strlen(src[i]);
        *end = '\n';
        end++;
    }
    *end = '\0';

    return result;
}

static bool compileShaderLines(GLenum shaderType, const char* const* lines,
        GLuint* outShader) {
    const char* src = makeShaderString(lines);
    bool result = compileShader(shaderType, src, outShader);
    if (!result) {
        fprintf(stderr, "Shader source:\n");
        printShaderSource(lines);
        delete[] src;
        return false;
    }
    delete[] src;

    return true;
}

static bool linkShaderProgram(GLuint vs, GLuint fs, GLuint* outPgm) {
    GLuint program = glCreateProgram();
    if (program == 0) {
        fprintf(stderr, "glCreateProgram error: %#x\n", glGetError());
        return false;
    }

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    GLint linkStatus = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE) {
        GLint bufLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
        if (bufLength) {
            char* buf = new char[bufLength];
            if (buf) {
                glGetProgramInfoLog(program, bufLength, nullptr, buf);
                fprintf(stderr, "Program link log:\n%s\n", buf);
                delete[] buf;
            }
        }
        glDeleteProgram(program);
        program = 0;
    }

    *outPgm = program;
    return program != 0;
}

bool GLHelper::setUpShaders(const ShaderDesc* shaderDescs, size_t numShaders) {
    mShaderPrograms = new GLuint[numShaders];
    bool result = true;

    for (size_t i = 0; i < numShaders && result; i++) {
        GLuint vs, fs;

        result = compileShaderLines(GL_VERTEX_SHADER,
                shaderDescs[i].vertexShader, &vs);
        if (!result) {
            return false;
        }

        result = compileShaderLines(GL_FRAGMENT_SHADER,
                shaderDescs[i].fragmentShader, &fs);
        if (!result) {
            glDeleteShader(vs);
            return false;
        }

        result = linkShaderProgram(vs, fs, &mShaderPrograms[i]);
        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    mNumShaders = numShaders;
    mShaderDescs = shaderDescs;

    return result;
}

bool GLHelper::getDitherTexture(GLuint* outTexName) {
    if (mDitherTexture == 0) {
        const uint8_t pattern[] = {
             0,  8,  2, 10,
            12,  4, 14,  6,
             3, 11,  1,  9,
            15,  7, 13,  5
        };

        glGenTextures(1, &mDitherTexture);
        glBindTexture(GL_TEXTURE_2D, mDitherTexture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, DITHER_KERNEL_SIZE,
                DITHER_KERNEL_SIZE, 0, GL_ALPHA, GL_UNSIGNED_BYTE, &pattern);
    }

    *outTexName = mDitherTexture;

    return true;
}

}
