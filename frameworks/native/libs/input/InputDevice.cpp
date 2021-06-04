/*
 * Copyright (C) 2012 The Android Open Source Project
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

#define LOG_TAG "InputDevice"

#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include <android-base/stringprintf.h>
#include <input/InputDevice.h>
#include <input/InputEventLabels.h>

using android::base::StringPrintf;

namespace android {

static const char* CONFIGURATION_FILE_DIR[] = {
        "idc/",
        "keylayout/",
        "keychars/",
};

static const char* CONFIGURATION_FILE_EXTENSION[] = {
        ".idc",
        ".kl",
        ".kcm",
};

static bool isValidNameChar(char ch) {
    return isascii(ch) && (isdigit(ch) || isalpha(ch) || ch == '-' || ch == '_');
}

static void appendInputDeviceConfigurationFileRelativePath(std::string& path,
        const std::string& name, InputDeviceConfigurationFileType type) {
    path += CONFIGURATION_FILE_DIR[type];
    path += name;
    path += CONFIGURATION_FILE_EXTENSION[type];
}

std::string getInputDeviceConfigurationFilePathByDeviceIdentifier(
        const InputDeviceIdentifier& deviceIdentifier,
        InputDeviceConfigurationFileType type) {
    if (deviceIdentifier.vendor !=0 && deviceIdentifier.product != 0) {
        if (deviceIdentifier.version != 0) {
            // Try vendor product version.
            std::string versionPath = getInputDeviceConfigurationFilePathByName(
                    StringPrintf("Vendor_%04x_Product_%04x_Version_%04x",
                            deviceIdentifier.vendor, deviceIdentifier.product,
                            deviceIdentifier.version),
                    type);
            if (!versionPath.empty()) {
                return versionPath;
            }
        }

        // Try vendor product.
        std::string productPath = getInputDeviceConfigurationFilePathByName(
                StringPrintf("Vendor_%04x_Product_%04x",
                        deviceIdentifier.vendor, deviceIdentifier.product),
                type);
        if (!productPath.empty()) {
            return productPath;
        }
    }

    // Try device name.
    return getInputDeviceConfigurationFilePathByName(deviceIdentifier.getCanonicalName(), type);
}

std::string getInputDeviceConfigurationFilePathByName(
        const std::string& name, InputDeviceConfigurationFileType type) {
    // Search system repository.
    std::string path;

    // Treblized input device config files will be located /odm/usr or /vendor/usr.
    const char *rootsForPartition[] {"/odm", "/vendor", getenv("ANDROID_ROOT")};
    for (size_t i = 0; i < size(rootsForPartition); i++) {
        if (rootsForPartition[i] == nullptr) {
            continue;
        }
        path = rootsForPartition[i];
        path += "/usr/";
        appendInputDeviceConfigurationFileRelativePath(path, name, type);
#if DEBUG_PROBE
        ALOGD("Probing for system provided input device configuration file: path='%s'",
              path.c_str());
#endif
        if (!access(path.c_str(), R_OK)) {
#if DEBUG_PROBE
            ALOGD("Found");
#endif
            return path;
        }
    }

    // Search user repository.
    // TODO Should only look here if not in safe mode.
    path = "";
    char *androidData = getenv("ANDROID_DATA");
    if (androidData != nullptr) {
        path += androidData;
    }
    path += "/system/devices/";
    appendInputDeviceConfigurationFileRelativePath(path, name, type);
#if DEBUG_PROBE
    ALOGD("Probing for system user input device configuration file: path='%s'", path.c_str());
#endif
    if (!access(path.c_str(), R_OK)) {
#if DEBUG_PROBE
        ALOGD("Found");
#endif
        return path;
    }

    // Not found.
#if DEBUG_PROBE
    ALOGD("Probe failed to find input device configuration file: name='%s', type=%d",
            name.c_str(), type);
#endif
    return "";
}

// --- InputDeviceIdentifier

std::string InputDeviceIdentifier::getCanonicalName() const {
    std::string replacedName = name;
    for (char& ch : replacedName) {
        if (!isValidNameChar(ch)) {
            ch = '_';
        }
    }
    return replacedName;
}


// --- InputDeviceInfo ---

InputDeviceInfo::InputDeviceInfo() {
    initialize(-1, 0, -1, InputDeviceIdentifier(), "", false, false);
}

InputDeviceInfo::InputDeviceInfo(const InputDeviceInfo& other) :
        mId(other.mId), mGeneration(other.mGeneration), mControllerNumber(other.mControllerNumber),
        mIdentifier(other.mIdentifier), mAlias(other.mAlias), mIsExternal(other.mIsExternal),
        mHasMic(other.mHasMic), mSources(other.mSources),
        mKeyboardType(other.mKeyboardType), mKeyCharacterMap(other.mKeyCharacterMap),
        mHasVibrator(other.mHasVibrator), mHasButtonUnderPad(other.mHasButtonUnderPad),
        mMotionRanges(other.mMotionRanges) {
}

InputDeviceInfo::~InputDeviceInfo() {
}

void InputDeviceInfo::initialize(int32_t id, int32_t generation, int32_t controllerNumber,
        const InputDeviceIdentifier& identifier, const std::string& alias, bool isExternal,
        bool hasMic) {
    mId = id;
    mGeneration = generation;
    mControllerNumber = controllerNumber;
    mIdentifier = identifier;
    mAlias = alias;
    mIsExternal = isExternal;
    mHasMic = hasMic;
    mSources = 0;
    mKeyboardType = AINPUT_KEYBOARD_TYPE_NONE;
    mHasVibrator = false;
    mHasButtonUnderPad = false;
    mMotionRanges.clear();
}

const InputDeviceInfo::MotionRange* InputDeviceInfo::getMotionRange(
        int32_t axis, uint32_t source) const {
    size_t numRanges = mMotionRanges.size();
    for (size_t i = 0; i < numRanges; i++) {
        const MotionRange& range = mMotionRanges[i];
        if (range.axis == axis && range.source == source) {
            return &range;
        }
    }
    return nullptr;
}

void InputDeviceInfo::addSource(uint32_t source) {
    mSources |= source;
}

void InputDeviceInfo::addMotionRange(int32_t axis, uint32_t source, float min, float max,
        float flat, float fuzz, float resolution) {
    MotionRange range = { axis, source, min, max, flat, fuzz, resolution };
    mMotionRanges.push_back(range);
}

void InputDeviceInfo::addMotionRange(const MotionRange& range) {
    mMotionRanges.push_back(range);
}

} // namespace android
