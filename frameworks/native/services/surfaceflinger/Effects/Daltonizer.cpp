/*
 * Copyright 2013 The Android Open Source Project
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

#include "Daltonizer.h"
#include <math/mat4.h>

namespace android {

void Daltonizer::setType(ColorBlindnessType type) {
    if (type != mType) {
        mDirty = true;
        mType = type;
    }
}

void Daltonizer::setMode(ColorBlindnessMode mode) {
    if (mode != mMode) {
        mDirty = true;
        mMode = mode;
    }
}

const mat4& Daltonizer::operator()() {
    if (mDirty) {
        mDirty = false;
        update();
    }
    return mColorTransform;
}

void Daltonizer::update() {
    if (mType == ColorBlindnessType::None) {
        mColorTransform = mat4();
        return;
    }

    // converts a linear RGB color to the XYZ space
    const mat4 rgb2xyz( 0.4124, 0.2126, 0.0193, 0,
                        0.3576, 0.7152, 0.1192, 0,
                        0.1805, 0.0722, 0.9505, 0,
                        0     , 0     , 0     , 1);

    // converts a XYZ color to the LMS space.
    const mat4 xyz2lms( 0.7328,-0.7036, 0.0030, 0,
                        0.4296, 1.6975, 0.0136, 0,
                       -0.1624, 0.0061, 0.9834, 0,
                        0     , 0     , 0     , 1);

    // Direct conversion from linear RGB to LMS
    const mat4 rgb2lms(xyz2lms*rgb2xyz);

    // And back from LMS to linear RGB
    const mat4 lms2rgb(inverse(rgb2lms));

    // To simulate color blindness we need to "remove" the data lost by the absence of
    // a cone. This cannot be done by just zeroing out the corresponding LMS component
    // because it would create a color outside of the RGB gammut.
    // Instead we project the color along the axis of the missing component onto a plane
    // within the RGB gammut:
    //  - since the projection happens along the axis of the missing component, a
    //    color blind viewer perceives the projected color the same.
    //  - We use the plane defined by 3 points in LMS space: black, white and
    //    blue and red for protanopia/deuteranopia and tritanopia respectively.

    // LMS space red
    const vec3& lms_r(rgb2lms[0].rgb);
    // LMS space blue
    const vec3& lms_b(rgb2lms[2].rgb);
    // LMS space white
    const vec3 lms_w((rgb2lms * vec4(1)).rgb);

    // To find the planes we solve the a*L + b*M + c*S = 0 equation for the LMS values
    // of the three known points. This equation is trivially solved, and has for
    // solution the following cross-products:
    const vec3 p0 = cross(lms_w, lms_b);    // protanopia/deuteranopia
    const vec3 p1 = cross(lms_w, lms_r);    // tritanopia

    // The following 3 matrices perform the projection of a LMS color onto the given plane
    // along the selected axis

    // projection for protanopia (L = 0)
    const mat4 lms2lmsp(  0.0000, 0.0000, 0.0000, 0,
                    -p0.y / p0.x, 1.0000, 0.0000, 0,
                    -p0.z / p0.x, 0.0000, 1.0000, 0,
                          0     , 0     , 0     , 1);

    // projection for deuteranopia (M = 0)
    const mat4 lms2lmsd(  1.0000, -p0.x / p0.y, 0.0000, 0,
                          0.0000,       0.0000, 0.0000, 0,
                          0.0000, -p0.z / p0.y, 1.0000, 0,
                          0     ,       0     , 0     , 1);

    // projection for tritanopia (S = 0)
    const mat4 lms2lmst(  1.0000, 0.0000, -p1.x / p1.z, 0,
                          0.0000, 1.0000, -p1.y / p1.z, 0,
                          0.0000, 0.0000,       0.0000, 0,
                          0     ,       0     , 0     , 1);

    // We will calculate the error between the color and the color viewed by
    // a color blind user and "spread" this error onto the healthy cones.
    // The matrices below perform this last step and have been chosen arbitrarily.

    // The amount of correction can be adjusted here.

    // error spread for protanopia
    const mat4 errp(    1.0, 0.7, 0.7, 0,
                        0.0, 1.0, 0.0, 0,
                        0.0, 0.0, 1.0, 0,
                          0,   0,   0, 1);

    // error spread for deuteranopia
    const mat4 errd(    1.0, 0.0, 0.0, 0,
                        0.7, 1.0, 0.7, 0,
                        0.0, 0.0, 1.0, 0,
                          0,   0,   0, 1);

    // error spread for tritanopia
    const mat4 errt(    1.0, 0.0, 0.0, 0,
                        0.0, 1.0, 0.0, 0,
                        0.7, 0.7, 1.0, 0,
                          0,   0,   0, 1);

    // And the magic happens here...
    // We construct the matrix that will perform the whole correction.

    // simulation: type of color blindness to simulate:
    // set to either lms2lmsp, lms2lmsd, lms2lmst
    mat4 simulation;

    // correction: type of color blindness correction (should match the simulation above):
    // set to identity, errp, errd, errt ([0] for simulation only)
    mat4 correction(0);

    switch (mType) {
        case ColorBlindnessType::Protanomaly:
            simulation = lms2lmsp;
            if (mMode == ColorBlindnessMode::Correction)
                correction = errp;
            break;
        case ColorBlindnessType::Deuteranomaly:
            simulation = lms2lmsd;
            if (mMode == ColorBlindnessMode::Correction)
                correction = errd;
            break;
        case ColorBlindnessType::Tritanomaly:
            simulation = lms2lmst;
            if (mMode == ColorBlindnessMode::Correction)
                correction = errt;
            break;
        case ColorBlindnessType::None:
            // We already caught this at the beginning of the method, but the
            // compiler doesn't know that
            break;
    }

    mColorTransform = lms2rgb *
        (simulation * rgb2lms + correction * (rgb2lms - simulation * rgb2lms));
}

} /* namespace android */
