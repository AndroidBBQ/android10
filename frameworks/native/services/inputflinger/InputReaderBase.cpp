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

#define LOG_TAG "InputReaderBase"

//#define LOG_NDEBUG 0

#include "InputReaderBase.h"

#include <android/log.h>
#include <android-base/stringprintf.h>

#define INDENT "  "
#define INDENT2 "    "
#define INDENT3 "      "
#define INDENT4 "        "
#define INDENT5 "          "

using android::base::StringPrintf;

namespace android {

// --- InputReaderThread ---

InputReaderThread::InputReaderThread(const sp<InputReaderInterface>& reader) :
        Thread(/*canCallJava*/ true), mReader(reader) {
}

InputReaderThread::~InputReaderThread() {
}

bool InputReaderThread::threadLoop() {
    mReader->loopOnce();
    return true;
}

// --- InputReaderConfiguration ---

std::optional<DisplayViewport> InputReaderConfiguration::getDisplayViewportByUniqueId(
        const std::string& uniqueDisplayId) const {
    if (uniqueDisplayId.empty()) {
        ALOGE("Empty string provided to %s", __func__);
        return std::nullopt;
    }
    size_t count = 0;
    std::optional<DisplayViewport> result = std::nullopt;
    for (const DisplayViewport& currentViewport : mDisplays) {
        if (uniqueDisplayId == currentViewport.uniqueId) {
            result = std::make_optional(currentViewport);
            count++;
        }
    }
    if (count > 1) {
        ALOGE("Found %zu viewports with uniqueId %s, but expected 1 at most",
            count, uniqueDisplayId.c_str());
    }
    return result;
}

std::optional<DisplayViewport> InputReaderConfiguration::getDisplayViewportByType(ViewportType type)
        const {
    size_t count = 0;
    std::optional<DisplayViewport> result = std::nullopt;
    for (const DisplayViewport& currentViewport : mDisplays) {
        // Return the first match
        if (currentViewport.type == type && !result) {
            result = std::make_optional(currentViewport);
            count++;
        }
    }
    if (count > 1) {
        ALOGE("Found %zu viewports with type %s, but expected 1 at most",
                count, viewportTypeToString(type));
    }
    return result;
}

std::optional<DisplayViewport> InputReaderConfiguration::getDisplayViewportByPort(
        uint8_t displayPort) const {
    for (const DisplayViewport& currentViewport : mDisplays) {
        const std::optional<uint8_t>& physicalPort = currentViewport.physicalPort;
        if (physicalPort && (*physicalPort == displayPort)) {
            return std::make_optional(currentViewport);
        }
    }
    return std::nullopt;
}

void InputReaderConfiguration::setDisplayViewports(const std::vector<DisplayViewport>& viewports) {
    mDisplays = viewports;
}

void InputReaderConfiguration::dump(std::string& dump) const {
    for (const DisplayViewport& viewport : mDisplays) {
        dumpViewport(dump, viewport);
    }
}

void InputReaderConfiguration::dumpViewport(std::string& dump, const DisplayViewport& viewport)
        const {
    dump += StringPrintf(INDENT4 "%s\n", viewport.toString().c_str());
}


// -- TouchAffineTransformation --
void TouchAffineTransformation::applyTo(float& x, float& y) const {
    float newX, newY;
    newX = x * x_scale + y * x_ymix + x_offset;
    newY = x * y_xmix + y * y_scale + y_offset;

    x = newX;
    y = newY;
}

} // namespace android