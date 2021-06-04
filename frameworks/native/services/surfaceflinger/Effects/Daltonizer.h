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

#ifndef SF_EFFECTS_DALTONIZER_H_
#define SF_EFFECTS_DALTONIZER_H_

#include <math/mat4.h>

namespace android {

enum class ColorBlindnessType {
    None,               // Disables the Daltonizer
    Protanomaly,        // L (red) cone deficient
    Deuteranomaly,      // M (green) cone deficient (most common)
    Tritanomaly         // S (blue) cone deficient
};

enum class ColorBlindnessMode {
    Simulation,
    Correction
};

class Daltonizer {
public:
    void setType(ColorBlindnessType type);
    void setMode(ColorBlindnessMode mode);

    // returns the color transform to apply in the shader
    const mat4& operator()();

private:
    void update();

    ColorBlindnessType mType = ColorBlindnessType::None;
    ColorBlindnessMode mMode = ColorBlindnessMode::Simulation;
    bool mDirty = true;
    mat4 mColorTransform;
};

} /* namespace android */
#endif /* SF_EFFECTS_DALTONIZER_H_ */
