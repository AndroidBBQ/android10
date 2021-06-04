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

#ifndef ANDROID_SURFACE_FLINGER_COLORIZER_H
#define ANDROID_SURFACE_FLINGER_COLORIZER_H

#include <android-base/stringprintf.h>

namespace android {

// ---------------------------------------------------------------------------

class Colorizer {
    bool mEnabled;
public:
    enum color {
        RED     = 31,
        GREEN   = 32,
        YELLOW  = 33,
        BLUE    = 34,
        MAGENTA = 35,
        CYAN    = 36,
        WHITE   = 37
    };

    explicit Colorizer(bool enabled)
        : mEnabled(enabled) {
    }

    void colorize(std::string& out, color c) {
        if (mEnabled) {
            base::StringAppendF(&out, "\e[%dm", c);
        }
    }

    void bold(std::string& out) {
        if (mEnabled) {
            out.append("\e[1m");
        }
    }

    void reset(std::string& out) {
        if (mEnabled) {
            out.append("\e[0m");
        }
    }
};

// ---------------------------------------------------------------------------

}; // namespace android


#endif /* ANDROID_SURFACE_FLINGER_COLORIZER_H */
