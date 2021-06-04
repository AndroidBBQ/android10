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

#pragma once

#include <array>

#include <GLES2/gl2.h>

namespace samples {

struct Circle {
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

    Circle(const Color &color, float radius, float x, float y) : color(color), radius(radius), x(x),
                                                                 y(y) {};

    static void draw(float aspectRatio, const std::vector<Circle> &circles, int workload);

    static int getSegmentsForWorkload(int workload) {
        float loadF = workload / 100.0f;

        int num_segmets = (MAX_SEGMENTS - MIN_SEGMENTS) * loadF + MIN_SEGMENTS;
        // make sure we get full triangles
        num_segmets = (num_segmets / 3) * 3;
        return num_segmets;
    }

    static const long MAX_SEGMENTS = 28800000;
    static const long MIN_SEGMENTS = 36;

    static std::vector<GLfloat> &getVertices(int);

    const Color color;
    const float radius;
    const float x;
    const float y;
};

} // namespace samples
