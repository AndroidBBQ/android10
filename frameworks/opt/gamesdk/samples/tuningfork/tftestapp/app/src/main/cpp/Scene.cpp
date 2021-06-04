/*
 * Copyright 2019 The Android Open Source Project
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

#include "Scene.h"

#include <cmath>
#include <cstdlib>
#include <string>
#include <random>

#define LOG_TAG "Scene"
#include "Log.h"

namespace {

auto const gVertexShader = R"(
    uniform float uRadius;
    uniform mat4 uMVPMatrix;
    uniform mat3 uNormalMatrix;
    uniform bool uAddLoad;
    uniform vec3 uLightDir;
    attribute vec4 vPosition;
    attribute vec3 vColor;
    attribute vec3 vNormal;
    varying vec3 aColor;
    void main() {
      float ambient, diffuse;
      gl_Position = uMVPMatrix * (vPosition * vec4(vec3(uRadius), 1.0));
      vec3 aNormal = normalize( uNormalMatrix * vNormal );
      diffuse = max(dot(aNormal, normalize(uLightDir)), 0.0);
      ambient = 0.1f;
      aColor = (ambient+diffuse)*vColor;
    }
)";
auto const gFragmentShader = R"(
    varying vec3 aColor;
    void main() {
      gl_FragColor = vec4(aColor, 1.0);
    }
)";

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

        radiusHandle = glGetUniformLocation(program, "uRadius");
        checkGlError("glGetUniformLocation(uRadius)");
        normalMatrixHandle = glGetUniformLocation(program, "uNormalMatrix");
        checkGlError("glGetUniformLocation(uNormalMatrix)");
        mvpMatrixHandle = glGetUniformLocation(program, "uMVPMatrix");
        checkGlError("glGetUniformLocation(uMVPMatrix)");
        vPositionHandle = glGetAttribLocation(program, "vPosition");
        checkGlError("glGetAttribLocation(vPosition)");
        vColorHandle = glGetAttribLocation(program, "vColor");
        checkGlError("glGetAttribLocation(vColor)");
        lightDirHandle = glGetUniformLocation(program, "uLightDir");
        checkGlError("glGetUniformLocation(uLightDir)");
        vNormalHandle = glGetAttribLocation(program, "vNormal");
        checkGlError("glGetAttribLocation(vNormal)");
    }

    GLuint program;
    GLint radiusHandle;
    GLint mvpMatrixHandle;
    GLint normalMatrixHandle;
    GLint vPositionHandle;
    GLint vColorHandle;
    GLint lightDirHandle;
    GLint vNormalHandle;
};

} // anonymous namespace

namespace samples {

void getMatrices(std::array<GLfloat, 16>& mvpMatrix, std::array<GLfloat, 9>& normalMatrix, GLfloat aspectRatio, Vector3 pos, Quaternion rot) {
    mvpMatrix[0] = (1 - 2*(rot.y*rot.y + rot.z*rot.z));
    mvpMatrix[1] = 2*(rot.x*rot.y - rot.z*rot.w);
    mvpMatrix[2] = 2*(rot.x*rot.z + rot.y*rot.w);
    mvpMatrix[3] = 0;
    mvpMatrix[4] = 2*(rot.x*rot.y + rot.z*rot.w);
    mvpMatrix[5] = (1 - 2*(rot.x*rot.x + rot.z*rot.z));
    mvpMatrix[6] = 2*(rot.y*rot.z - rot.x*rot.w);
    mvpMatrix[7] = 0;
    mvpMatrix[8] = 2*(rot.x*rot.z - rot.y*rot.w);
    mvpMatrix[9] = 2*(rot.y*rot.z + rot.x*rot.w);
    mvpMatrix[10] = (1 - 2*(rot.x*rot.x + rot.y*rot.y));
    mvpMatrix[11] = 0;
    mvpMatrix[12] = pos.x;
    mvpMatrix[13] = pos.y;
    mvpMatrix[14] = pos.z;
    mvpMatrix[15] = 1.0f;
    normalMatrix[0] = mvpMatrix[0];
    normalMatrix[1] = mvpMatrix[1];
    normalMatrix[2] = mvpMatrix[2];

    normalMatrix[3] = mvpMatrix[4];
    normalMatrix[4] = mvpMatrix[5];
    normalMatrix[5] = mvpMatrix[6];

    normalMatrix[6] = mvpMatrix[8];
    normalMatrix[7] = mvpMatrix[9];
    normalMatrix[8] = mvpMatrix[10];

    mvpMatrix[0] *= 1.0f / aspectRatio;
    mvpMatrix[4] *= 1.0f / aspectRatio;
    mvpMatrix[8] *= 1.0f / aspectRatio;
};

int Sphere::numVerticesForWorkload(int workload, int& nPhi, int& nTheta) {
    float loadF = sqrtf(workload / 100.0f);
    nPhi = loadF*(MAX_PHI - MIN_PHI) + MIN_PHI;
    nPhi = (nPhi/2)*2; // Make it divisible by 2
    nTheta = loadF*(MAX_THETA - MIN_PHI) + MIN_THETA;
    return nPhi*nTheta*2;
}

int Sphere::numVerticesForWorkload(int workload) {
    int nPhi;
    int nTheta;
    return numVerticesForWorkload(workload, nPhi, nTheta);
}

std::mt19937 gen;
std::uniform_real_distribution<float> dist(-1,1);

inline GLfloat* addVertex(GLfloat* v, float phi, float theta) {
    float x = cos(phi) * sin(theta);
    float y = sin(phi) * sin(theta);
    float z = cos(theta);

    *(v++) = x;
    *(v++) = y;
    *(v++) = z;

    // Color
    *(v++) = fabs(x);
    *(v++) = fabs(y);
    *(v++) = fabs(z);

    // Normals are the same as positions
    *(v++) = x;
    *(v++) = y;
    *(v++) = z;

    return v;
}
std::vector<GLfloat> Sphere::initializeVertices(int workload) {
    int nPhi;
    int nTheta;
    int num_vert = numVerticesForWorkload(workload, nPhi, nTheta);
    std::vector<GLfloat> vertices = std::vector<GLfloat>( num_vert * 9); // 3 pos + 3 colour + 3 normal
    const float dTheta = static_cast<float>(M_PI / (nTheta-1));
    const float dPhi = static_cast<float>(2 * M_PI / nPhi);
    // Top tri
    float phi = 0;

    GLfloat* v = vertices.data();
    for(int ph=0; ph< nPhi; ++ph) {
        float theta = 0;
        // vertical strip
        for (int th = 0; th < nTheta; ++th) {
            v = addVertex(v, phi, theta);
            v = addVertex(v, phi + dPhi, theta);
            theta += dTheta;
        }
        phi += dPhi;
    }
    return vertices;
}

void Sphere::draw(float aspectRatio, const std::vector<Sphere> &spheres, int n_to_draw, int workload) {
    static ProgramState state;

    int num_vertices = numVerticesForWorkload(workload);

    glUseProgram(state.program);
    checkGlError("glUseProgram");

    // Positions
    glVertexAttribPointer(static_cast<GLuint>(state.vPositionHandle), 3, GL_FLOAT, GL_FALSE, 9*sizeof(GLfloat),
                          getVertices(workload).data());
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(static_cast<GLuint>(state.vPositionHandle));
    checkGlError("glEnableVertexAttribArray");

    // Colors
    glVertexAttribPointer(static_cast<GLuint>(state.vColorHandle), 3, GL_FLOAT, GL_FALSE, 9*sizeof(GLfloat),
                          getVertices(workload).data()+3);
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(static_cast<GLuint>(state.vColorHandle));
    checkGlError("glEnableVertexAttribArray");

    // Normals
    glVertexAttribPointer(static_cast<GLuint>(state.vNormalHandle), 3, GL_FLOAT, GL_FALSE, 9*sizeof(GLfloat),
                          getVertices(workload).data()+6);
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(static_cast<GLuint>(state.vNormalHandle));
    checkGlError("glEnableVertexAttribArray");

    glUniform3f(state.lightDirHandle, 0.3, 0.3, -1);
    checkGlError("glUniform3f(lightDir)");

    for (int i=0; i < n_to_draw; ++i) {
        const Sphere& sphere = spheres[i];

        glUniform1f(state.radiusHandle, sphere.radius);
        checkGlError("glUniform1f(radius)");

        std::array<float,16> mvpMatrix;
        std::array<float,9> normalMatrix;
        getMatrices(mvpMatrix, normalMatrix, aspectRatio, sphere.pos, sphere.rot);
        glUniformMatrix4fv(state.mvpMatrixHandle, 1, GL_FALSE, mvpMatrix.data());
        checkGlError("glUniformMatrix4fv(mvpMatrix)");

        glUniformMatrix3fv(state.normalMatrixHandle, 1, GL_FALSE, normalMatrix.data());
        checkGlError("glUniformMatrix4fv(normalMatrix)");

        glDrawArrays(GL_TRIANGLE_STRIP, 0, num_vertices);
        checkGlError("glDrawArrays");
    }
}

std::vector<GLfloat> &Sphere::getVertices(int workload) {
    static std::vector<GLfloat> vertices;
    static int prev = -1;
    if (prev != workload) {
        vertices = initializeVertices(workload);
        prev = workload;
    }
    return vertices;
}

Scene::Scene(int nSpheres) {
    for(int i=0;i<nSpheres; ++i) {
        Vector3 velocity = {dist(gen), dist(gen), dist(gen)};
        Vector3 angular_velocity = {dist(gen), dist(gen), dist(gen)};
        spheres.push_back( {Color{0.0f, 1.0f, 1.0f}, 0.1f, {0,0,0}, {0,0,0,1}, velocity, angular_velocity} );
    }
}

void Scene::draw(float aspectRatio, int workload) {

    Sphere::draw(aspectRatio, spheres, nSpheres, workload);
}

void normalize(Quaternion& q) {
    float a = q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z;
    q.x/=a;
    q.y/=a;
    q.z/=a;
    q.w/=a;
}
void update(Quaternion& rot, const Vector3& angular_velocity, float dt) {

    Quaternion q = { angular_velocity.x*rot.w + angular_velocity.y*rot.z - angular_velocity.z*rot.y,
                     angular_velocity.y*rot.w + angular_velocity.z*rot.x - angular_velocity.x*rot.z,
                     angular_velocity.z*rot.w + angular_velocity.x*rot.y - angular_velocity.y*rot.x,
                     -angular_velocity.x*rot.x - angular_velocity.y*rot.y - angular_velocity.z*rot.z
                     };
    rot.x += q.x*dt/2;
    rot.y += q.y*dt/2;
    rot.z += q.z*dt/2;
    rot.w += q.w*dt/2;
    normalize(rot);
}

void Scene::tick(float deltaSeconds) {

    for(auto& sphere: spheres) {
        sphere.pos.x += sphere.velocity.x * deltaSeconds;
        sphere.pos.y += sphere.velocity.y * deltaSeconds;
        if (sphere.pos.x > 0.8f) {
            sphere.velocity.x *= -1.0f;
            sphere.pos.x = 1.6f - sphere.pos.x;
        } else if (sphere.pos.x < -0.8f) {
            sphere.velocity.x *= -1.0f;
            sphere.pos.x = -1.6f - sphere.pos.x;
        }
        if (sphere.pos.y > 0.8f) {
            sphere.velocity.y *= -1.0f;
            sphere.pos.y = 1.6f - sphere.pos.y;
        } else if (sphere.pos.y < -0.8f) {
            sphere.velocity.y *= -1.0f;
            sphere.pos.y = -1.6f - sphere.pos.y;
        }
        update(sphere.rot, sphere.angular_velocity, deltaSeconds);
    }

}

} // namespace samples
