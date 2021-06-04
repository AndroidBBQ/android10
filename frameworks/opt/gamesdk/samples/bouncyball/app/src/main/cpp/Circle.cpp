/*
 * Copyright 2018 The Android Open Source Project
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

#include "Circle.h"

#define LOG_TAG "Circle"

#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>

#include "Log.h"

namespace {

std::vector<GLfloat> initializeVertices(int segments) {
    std::vector<GLfloat> vertices = std::vector<GLfloat>(2 * segments + 2);
    const float dTheta = static_cast<float>(2 * M_PI / segments);
    for (size_t i = 0; i < segments + 1; i++) {
        vertices[(i + 1) * 2] = cos(dTheta * i);
        vertices[(i + 1) * 2 + 1] = sin(dTheta * i);
    }
    return vertices;
}

auto const gVertexShader =
        "uniform float uRadius;\n"
                "uniform mat4 uMVPMatrix;\n"
                "uniform bool uAddLoad;\n"
                "attribute vec4 vPosition;\n"
                "void main() {\n"
                "  gl_Position = uMVPMatrix * (vPosition * vec4(vec3(uRadius), 1.0));\n"
                "}\n";
auto const gFragmentShader =
        "uniform mediump vec3 uColor;\n"
                "void main() {\n"
                "  gl_FragColor = vec4(uColor, 1.0);\n"
                "}\n";

void checkGlError(const char *op) {
    for (GLint error = glGetError(); error; error = glGetError()) {
        ALOGI("after %s() glError (0x%x)\n", op, error);
    }
}

GLuint loadShader(GLenum shaderType, const char *pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader == 0) {
        return shader;
    }

    glShaderSource(shader, 1, &pSource, NULL);
    glCompileShader(shader);
    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLength);
        if (infoLength > 0) {
            std::vector<char> info(infoLength, '\0');
            glGetShaderInfoLog(shader, infoLength, NULL, info.data());
            ALOGE("Could not compile shader %d:\n%s\n", shaderType, info.data());
            glDeleteShader(shader);
            shader = 0;
        }
    }

    return shader;
}

GLuint createProgram(const char *pVertexSource, const char *pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program == 0) {
        return program;
    }

    glAttachShader(program, vertexShader);
    checkGlError("glAttachShader");
    glAttachShader(program, pixelShader);
    checkGlError("glAttachShader");
    glLinkProgram(program);
    GLint linkStatus = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (!linkStatus) {
        GLint infoLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLength);
        if (infoLength > 0) {
            std::vector<char> info(infoLength, '\0');
            glGetProgramInfoLog(program, infoLength, NULL, info.data());
            ALOGE("Could not link program:\n%s\n", info.data());
        }
        glDeleteProgram(program);
        program = 0;
    }
    return program;
}

struct ProgramState {
    ProgramState() {
        program = createProgram(gVertexShader, gFragmentShader);
        if (program == 0) {
            ALOGE("Failed to create program");
            checkGlError("createProgram");
            return;
        }

        colorHandle = glGetUniformLocation(program, "uColor");
        checkGlError("glGetUniformLocation(uColor)");
        radiusHandle = glGetUniformLocation(program, "uRadius");
        checkGlError("glGetUniformLocation(uRadius)");
        mvpMatrixHandle = glGetUniformLocation(program, "uMVPMatrix");
        checkGlError("glGetUniformLocation(uMVPMatrix)");
        vPositionHandle = glGetAttribLocation(program, "vPosition");
        checkGlError("glGetAttribLocation(vPosition)");
    }

    GLuint program;
    GLint colorHandle;
    GLint radiusHandle;
    GLint mvpMatrixHandle;
    GLint vPositionHandle;
};

std::array<GLfloat, 16> getMvpMatrix(GLfloat aspectRatio, GLfloat x, GLfloat y) {
    std::array<GLfloat, 16> mvpMatrix = {0.0f};
    mvpMatrix[0] = 1.0f / aspectRatio;
    mvpMatrix[5] = 1.0f;
    mvpMatrix[10] = 1.0f;
    mvpMatrix[12] = x;
    mvpMatrix[13] = y;
    mvpMatrix[15] = 1.0f;
    return mvpMatrix;
};

} // anonymous namespace

namespace samples {

void Circle::draw(float aspectRatio, const std::vector<Circle> &circles, int workload) {
    static ProgramState state;

    int segments = getSegmentsForWorkload(workload);

    glUseProgram(state.program);
    checkGlError("glUseProgram");

    glVertexAttribPointer(static_cast<GLuint>(state.vPositionHandle), 2, GL_FLOAT, GL_FALSE, 0,
                          getVertices(segments).data());
    checkGlError("glVertexAttribPointer");

    glEnableVertexAttribArray(static_cast<GLuint>(state.vPositionHandle));
    checkGlError("glEnableVertexAttribArray");

    for (const Circle &circle : circles) {
        glUniform3fv(state.colorHandle, 1, circle.color.values.data());
        checkGlError("glUniform3fv(color)");

        glUniform1f(state.radiusHandle, circle.radius);
        checkGlError("glUniform1f(radius)");

        auto mvpMatrix = getMvpMatrix(aspectRatio, circle.x, circle.y);
        glUniformMatrix4fv(state.mvpMatrixHandle, 1, GL_FALSE, mvpMatrix.data());
        checkGlError("glUniformMatrix4fv(mvpMatrix)");


        glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2);
        checkGlError("glDrawArrays");
    }
}

// getVertices will return the corresponding based on segments parameter.
// This function needs to be called with the same segments number in the same frame
std::vector<GLfloat> &Circle::getVertices(int segments) {
    static auto vertices = initializeVertices(segments);
    static int prev = segments;
    if (prev != segments) {
        vertices = initializeVertices(segments);
        prev = segments;
    }
    return vertices;
}

} // namespace samples
