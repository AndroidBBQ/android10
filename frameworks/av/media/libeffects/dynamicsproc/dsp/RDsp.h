/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef RDSP_H
#define RDSP_H

#include <complex>
#include <log/log.h>
#include <vector>
#include <map>
using FloatVec = std::vector<float>;
using IntVec = std::vector<int>;
using ComplexVec  = std::vector<std::complex<float>>;

// =======
// Helper Functions
// =======
template <class T>
static T dBtoLinear(T valueDb) {
    return pow (10, valueDb / 20.0);
}

template <class T>
static T linearToDb(T value) {
    return 20 * log10(value);
}

// =======
// DSP window creation
// =======

#define TWOPI (M_PI * 2)

enum rdsp_window_type {
    RDSP_WINDOW_RECTANGULAR,
    RDSP_WINDOW_TRIANGULAR,
    RDSP_WINDOW_TRIANGULAR_FLAT_TOP,
    RDSP_WINDOW_HAMMING,
    RDSP_WINDOW_HAMMING_FLAT_TOP,
    RDSP_WINDOW_HANNING,
    RDSP_WINDOW_HANNING_FLAT_TOP,
};

template <typename T>
static void fillRectangular(T &v) {
    const size_t size = v.size();
    for (size_t i = 0; i < size; i++) {
        v[i] = 1.0;
    }
} //rectangular

template <typename T>
static void fillTriangular(T &v, size_t overlap) {
    const size_t size = v.size();
    //ramp up
    size_t i = 0;
    if (overlap > 0) {
        for (; i < overlap; i++) {
            v[i] = (2.0 * i + 1) / (2 * overlap);
        }
    }

    //flat top
    for (; i < size - overlap; i++) {
        v[i] = 1.0;
    }

    //ramp down
    if (overlap > 0) {
        for (; i < size; i++) {
            v[i] = (2.0 * (size - i) - 1) / (2 * overlap);
        }
    }
} //triangular

template <typename T>
static void fillHamming(T &v, size_t overlap) {
    const size_t size = v.size();
    const size_t twoOverlap = 2 * overlap;
    size_t i = 0;
    if (overlap > 0) {
        for (; i < overlap; i++) {
            v[i] = 0.54 - 0.46 * cos(TWOPI * i /(twoOverlap - 1));
        }
    }

    //flat top
    for (; i < size - overlap; i++) {
        v[i] = 1.0;
    }

    //ramp down
    if (overlap > 0) {
        for (; i < size; i++) {
            int k = i - ((int)size - 2 * overlap);
            v[i] = 0.54 - 0.46 * cos(TWOPI * k / (twoOverlap - 1));
        }
    }
} //hamming

template <typename T>
static void fillHanning(T &v, size_t overlap) {
    const size_t size = v.size();
    const size_t twoOverlap = 2 * overlap;
    //ramp up
    size_t i = 0;
    if (overlap > 0) {
        for (; i < overlap; i++) {
            v[i] = 0.5 * (1.0 - cos(TWOPI * i / (twoOverlap - 1)));
        }
    }

    //flat top
    for (; i < size - overlap; i++) {
        v[i] = 1.0;
    }

    //ramp down
    if (overlap > 0) {
        for (; i < size; i++) {
            int k = i - ((int)size - 2 * overlap);
            v[i] = 0.5 * (1.0 - cos(TWOPI * k / (twoOverlap - 1)));
        }
    }
}

template <typename T>
static void fill_window(T &v, int type, size_t size, size_t overlap) {
    if (overlap > size / 2) {
        overlap = size / 2;
    }
    v.resize(size);

    switch (type) {
    case RDSP_WINDOW_RECTANGULAR:
        fillRectangular(v);
        break;
    case RDSP_WINDOW_TRIANGULAR:
        fillTriangular(v, size / 2);
        break;
    case RDSP_WINDOW_TRIANGULAR_FLAT_TOP:
        fillTriangular(v, overlap);
        break;
    case RDSP_WINDOW_HAMMING:
        fillHamming(v, size / 2);
        break;
    case RDSP_WINDOW_HAMMING_FLAT_TOP:
        fillHamming(v, overlap);
        break;
    case RDSP_WINDOW_HANNING:
        fillHanning(v, size / 2);
        break;
    case RDSP_WINDOW_HANNING_FLAT_TOP:
        fillHanning(v, overlap);
        break;
    default:
        ALOGE("Error: unknown window type %d", type);
    }
}

//};
#endif //RDSP_H
