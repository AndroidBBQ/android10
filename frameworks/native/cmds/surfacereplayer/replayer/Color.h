/*
 * Copyright 2016 The Android Open Source Project
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

#ifndef ANDROID_SURFACEREPLAYER_COLOR_H
#define ANDROID_SURFACEREPLAYER_COLOR_H

#include <cmath>
#include <cstdlib>

namespace android {

constexpr double modulateFactor = .0001;
constexpr double modulateLimit = .80;

struct RGB {
    RGB(uint8_t rIn, uint8_t gIn, uint8_t bIn) : r(rIn), g(gIn), b(bIn) {}

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

struct HSV {
    HSV() = default;
    HSV(double hIn, double sIn, double vIn) : h(hIn), s(sIn), v(vIn) {}

    double h = 0;
    double s = 0;
    double v = 0;

    RGB getRGB() const;

    bool modulateUp = false;

    void modulate();
};

void inline HSV::modulate() {
    if(modulateUp) {
        v += modulateFactor;
    } else {
        v -= modulateFactor;
    }

    if(v <= modulateLimit || v >= 1) {
        modulateUp = !modulateUp;
    }
}

inline RGB HSV::getRGB() const {
    using namespace std;
    double r = 0, g = 0, b = 0;

    if (s == 0) {
        r = v;
        g = v;
        b = v;
    } else {
        auto tempHue = static_cast<int>(h) % 360;
        tempHue = tempHue / 60;

        int i = static_cast<int>(trunc(tempHue));
        double f = h - i;

        double x = v * (1.0 - s);
        double y = v * (1.0 - (s * f));
        double z = v * (1.0 - (s * (1.0 - f)));

        switch (i) {
            case 0:
                r = v;
                g = z;
                b = x;
                break;

            case 1:
                r = y;
                g = v;
                b = x;
                break;

            case 2:
                r = x;
                g = v;
                b = z;
                break;

            case 3:
                r = x;
                g = y;
                b = v;
                break;

            case 4:
                r = z;
                g = x;
                b = v;
                break;

            default:
                r = v;
                g = x;
                b = y;
                break;
        }
    }

    return RGB(round(r * 255), round(g * 255), round(b * 255));
}
}
#endif
