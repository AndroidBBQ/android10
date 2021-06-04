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

#pragma once

#include <array>
#include <vector>

#include <GLES2/gl2.h>

namespace samples {

struct Color {
    Color(GLfloat r, GLfloat g, GLfloat b) : r(r), g(g), b(b) {}

    union {
        std::array<GLfloat, 3> values;
        struct {
            GLfloat r;
            GLfloat g;
            GLfloat b;
        };
    };
};
struct Vector3 {
    Vector3(GLfloat x, GLfloat y, GLfloat z) : x(x), y(y), z(z) {}

    union {
        std::array<GLfloat, 3> values;
        struct {
            GLfloat x;
            GLfloat y;
            GLfloat z;
        };
    };
};
struct Quaternion {
    Quaternion(GLfloat x, GLfloat y, GLfloat z, GLfloat w) : x(x), y(y), z(z), w(w) {}
    union {
        std::array<GLfloat, 4> values;
        struct {
            GLfloat x;
            GLfloat y;
            GLfloat z;
            GLfloat w;
        };
    };
};
struct Object3D {
    Vector3 pos;
    Quaternion rot;
    Vector3 velocity;
    Vector3 angular_velocity;
};
struct Sphere : Object3D {

    Sphere(const Color &color = {1,0,0}, float radius=1,
        const Vector3& pos = {0,0,0}, const Quaternion& rot = {0,0,0,1}, const Vector3& velocity = {0,0,0}, const Vector3& angular_velocity = {0,0,0})
      : Object3D({pos, rot, velocity, angular_velocity}), color(color), radius(radius) {};

    static void draw(float aspectRatio, const std::vector<Sphere> &spheres, int nSpheres, int workload);

    static int numVerticesForWorkload(int workload, int& nPhi, int& nTheta);
    static int numVerticesForWorkload(int workload);
    static std::vector<GLfloat> initializeVertices(int workload);

    static const long MAX_PHI = 200;
    static const long MAX_THETA = 100;
    static const long MIN_PHI = 4;
    static const long MIN_THETA = 3;

    static std::vector<GLfloat> &getVertices(int);

    const Color color;
    const float radius;
};

class Scene {
public:
    Scene(int nSpheres);
    void draw(float aspectRatio, int workload);
    void tick(float deltaSeconds);
    std::vector<Sphere> spheres;
    int nSpheres = 20; // How many to draw
};

} // namespace samples
