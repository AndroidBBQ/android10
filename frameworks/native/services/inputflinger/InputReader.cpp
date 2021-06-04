/*
 * Copyright (C) 2010 The Android Open Source Project
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

#define LOG_TAG "InputReader"

//#define LOG_NDEBUG 0

// Log debug messages for each raw event received from the EventHub.
#define DEBUG_RAW_EVENTS 0

// Log debug messages about touch screen filtering hacks.
#define DEBUG_HACKS 0

// Log debug messages about virtual key processing.
#define DEBUG_VIRTUAL_KEYS 0

// Log debug messages about pointers.
#define DEBUG_POINTERS 0

// Log debug messages about pointer assignment calculations.
#define DEBUG_POINTER_ASSIGNMENT 0

// Log debug messages about gesture detection.
#define DEBUG_GESTURES 0

// Log debug messages about the vibrator.
#define DEBUG_VIBRATOR 0

// Log debug messages about fusing stylus data.
#define DEBUG_STYLUS_FUSION 0

#include "InputReader.h"

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

#include <log/log.h>

#include <android-base/stringprintf.h>
#include <input/Keyboard.h>
#include <input/VirtualKeyMap.h>
#include <statslog.h>

#define INDENT "  "
#define INDENT2 "    "
#define INDENT3 "      "
#define INDENT4 "        "
#define INDENT5 "          "

using android::base::StringPrintf;

namespace android {

// --- Constants ---

// Maximum number of slots supported when using the slot-based Multitouch Protocol B.
static constexpr size_t MAX_SLOTS = 32;

// Maximum amount of latency to add to touch events while waiting for data from an
// external stylus.
static constexpr nsecs_t EXTERNAL_STYLUS_DATA_TIMEOUT = ms2ns(72);

// Maximum amount of time to wait on touch data before pushing out new pressure data.
static constexpr nsecs_t TOUCH_DATA_TIMEOUT = ms2ns(20);

// Artificial latency on synthetic events created from stylus data without corresponding touch
// data.
static constexpr nsecs_t STYLUS_DATA_LATENCY = ms2ns(10);

// How often to report input event statistics
static constexpr nsecs_t STATISTICS_REPORT_FREQUENCY = seconds_to_nanoseconds(5 * 60);

// --- Static Functions ---

template<typename T>
inline static T abs(const T& value) {
    return value < 0 ? - value : value;
}

template<typename T>
inline static T min(const T& a, const T& b) {
    return a < b ? a : b;
}

template<typename T>
inline static void swap(T& a, T& b) {
    T temp = a;
    a = b;
    b = temp;
}

inline static float avg(float x, float y) {
    return (x + y) / 2;
}

inline static float distance(float x1, float y1, float x2, float y2) {
    return hypotf(x1 - x2, y1 - y2);
}

inline static int32_t signExtendNybble(int32_t value) {
    return value >= 8 ? value - 16 : value;
}

static inline const char* toString(bool value) {
    return value ? "true" : "false";
}

static int32_t rotateValueUsingRotationMap(int32_t value, int32_t orientation,
        const int32_t map[][4], size_t mapSize) {
    if (orientation != DISPLAY_ORIENTATION_0) {
        for (size_t i = 0; i < mapSize; i++) {
            if (value == map[i][0]) {
                return map[i][orientation];
            }
        }
    }
    return value;
}

static const int32_t keyCodeRotationMap[][4] = {
        // key codes enumerated counter-clockwise with the original (unrotated) key first
        // no rotation,        90 degree rotation,  180 degree rotation, 270 degree rotation
        { AKEYCODE_DPAD_DOWN,   AKEYCODE_DPAD_RIGHT,  AKEYCODE_DPAD_UP,     AKEYCODE_DPAD_LEFT },
        { AKEYCODE_DPAD_RIGHT,  AKEYCODE_DPAD_UP,     AKEYCODE_DPAD_LEFT,   AKEYCODE_DPAD_DOWN },
        { AKEYCODE_DPAD_UP,     AKEYCODE_DPAD_LEFT,   AKEYCODE_DPAD_DOWN,   AKEYCODE_DPAD_RIGHT },
        { AKEYCODE_DPAD_LEFT,   AKEYCODE_DPAD_DOWN,   AKEYCODE_DPAD_RIGHT,  AKEYCODE_DPAD_UP },
        { AKEYCODE_SYSTEM_NAVIGATION_DOWN, AKEYCODE_SYSTEM_NAVIGATION_RIGHT,
            AKEYCODE_SYSTEM_NAVIGATION_UP, AKEYCODE_SYSTEM_NAVIGATION_LEFT },
        { AKEYCODE_SYSTEM_NAVIGATION_RIGHT, AKEYCODE_SYSTEM_NAVIGATION_UP,
            AKEYCODE_SYSTEM_NAVIGATION_LEFT, AKEYCODE_SYSTEM_NAVIGATION_DOWN },
        { AKEYCODE_SYSTEM_NAVIGATION_UP, AKEYCODE_SYSTEM_NAVIGATION_LEFT,
            AKEYCODE_SYSTEM_NAVIGATION_DOWN, AKEYCODE_SYSTEM_NAVIGATION_RIGHT },
        { AKEYCODE_SYSTEM_NAVIGATION_LEFT, AKEYCODE_SYSTEM_NAVIGATION_DOWN,
            AKEYCODE_SYSTEM_NAVIGATION_RIGHT, AKEYCODE_SYSTEM_NAVIGATION_UP },
};
static const size_t keyCodeRotationMapSize =
        sizeof(keyCodeRotationMap) / sizeof(keyCodeRotationMap[0]);

static int32_t rotateStemKey(int32_t value, int32_t orientation,
        const int32_t map[][2], size_t mapSize) {
    if (orientation == DISPLAY_ORIENTATION_180) {
        for (size_t i = 0; i < mapSize; i++) {
            if (value == map[i][0]) {
                return map[i][1];
            }
        }
    }
    return value;
}

// The mapping can be defined using input device configuration properties keyboard.rotated.stem_X
static int32_t stemKeyRotationMap[][2] = {
        // key codes enumerated with the original (unrotated) key first
        // no rotation,           180 degree rotation
        { AKEYCODE_STEM_PRIMARY, AKEYCODE_STEM_PRIMARY },
        { AKEYCODE_STEM_1,       AKEYCODE_STEM_1 },
        { AKEYCODE_STEM_2,       AKEYCODE_STEM_2 },
        { AKEYCODE_STEM_3,       AKEYCODE_STEM_3 },
};
static const size_t stemKeyRotationMapSize =
        sizeof(stemKeyRotationMap) / sizeof(stemKeyRotationMap[0]);

static int32_t rotateKeyCode(int32_t keyCode, int32_t orientation) {
    keyCode = rotateStemKey(keyCode, orientation,
            stemKeyRotationMap, stemKeyRotationMapSize);
    return rotateValueUsingRotationMap(keyCode, orientation,
            keyCodeRotationMap, keyCodeRotationMapSize);
}

static void rotateDelta(int32_t orientation, float* deltaX, float* deltaY) {
    float temp;
    switch (orientation) {
    case DISPLAY_ORIENTATION_90:
        temp = *deltaX;
        *deltaX = *deltaY;
        *deltaY = -temp;
        break;

    case DISPLAY_ORIENTATION_180:
        *deltaX = -*deltaX;
        *deltaY = -*deltaY;
        break;

    case DISPLAY_ORIENTATION_270:
        temp = *deltaX;
        *deltaX = -*deltaY;
        *deltaY = temp;
        break;
    }
}

static inline bool sourcesMatchMask(uint32_t sources, uint32_t sourceMask) {
    return (sources & sourceMask & ~ AINPUT_SOURCE_CLASS_MASK) != 0;
}

// Returns true if the pointer should be reported as being down given the specified
// button states.  This determines whether the event is reported as a touch event.
static bool isPointerDown(int32_t buttonState) {
    return buttonState &
            (AMOTION_EVENT_BUTTON_PRIMARY | AMOTION_EVENT_BUTTON_SECONDARY
                    | AMOTION_EVENT_BUTTON_TERTIARY);
}

static float calculateCommonVector(float a, float b) {
    if (a > 0 && b > 0) {
        return a < b ? a : b;
    } else if (a < 0 && b < 0) {
        return a > b ? a : b;
    } else {
        return 0;
    }
}

static void synthesizeButtonKey(InputReaderContext* context, int32_t action,
        nsecs_t when, int32_t deviceId, uint32_t source, int32_t displayId,
        uint32_t policyFlags, int32_t lastButtonState, int32_t currentButtonState,
        int32_t buttonState, int32_t keyCode) {
    if (
            (action == AKEY_EVENT_ACTION_DOWN
                    && !(lastButtonState & buttonState)
                    && (currentButtonState & buttonState))
            || (action == AKEY_EVENT_ACTION_UP
                    && (lastButtonState & buttonState)
                    && !(currentButtonState & buttonState))) {
        NotifyKeyArgs args(context->getNextSequenceNum(), when, deviceId, source, displayId,
                policyFlags, action, 0, keyCode, 0, context->getGlobalMetaState(), when);
        context->getListener()->notifyKey(&args);
    }
}

static void synthesizeButtonKeys(InputReaderContext* context, int32_t action,
        nsecs_t when, int32_t deviceId, uint32_t source, int32_t displayId,
        uint32_t policyFlags, int32_t lastButtonState, int32_t currentButtonState) {
    synthesizeButtonKey(context, action, when, deviceId, source, displayId, policyFlags,
            lastButtonState, currentButtonState,
            AMOTION_EVENT_BUTTON_BACK, AKEYCODE_BACK);
    synthesizeButtonKey(context, action, when, deviceId, source, displayId, policyFlags,
            lastButtonState, currentButtonState,
            AMOTION_EVENT_BUTTON_FORWARD, AKEYCODE_FORWARD);
}


// --- InputReader ---

InputReader::InputReader(const sp<EventHubInterface>& eventHub,
        const sp<InputReaderPolicyInterface>& policy,
        const sp<InputListenerInterface>& listener) :
        mContext(this), mEventHub(eventHub), mPolicy(policy),
        mNextSequenceNum(1), mGlobalMetaState(0), mGeneration(1),
        mDisableVirtualKeysTimeout(LLONG_MIN), mNextTimeout(LLONG_MAX),
        mConfigurationChangesToRefresh(0) {
    mQueuedListener = new QueuedInputListener(listener);

    { // acquire lock
        AutoMutex _l(mLock);

        refreshConfigurationLocked(0);
        updateGlobalMetaStateLocked();
    } // release lock
}

InputReader::~InputReader() {
    for (size_t i = 0; i < mDevices.size(); i++) {
        delete mDevices.valueAt(i);
    }
}

void InputReader::loopOnce() {
    int32_t oldGeneration;
    int32_t timeoutMillis;
    bool inputDevicesChanged = false;
    std::vector<InputDeviceInfo> inputDevices;
    { // acquire lock
        AutoMutex _l(mLock);

        oldGeneration = mGeneration;
        timeoutMillis = -1;

        uint32_t changes = mConfigurationChangesToRefresh;
        if (changes) {
            mConfigurationChangesToRefresh = 0;
            timeoutMillis = 0;
            refreshConfigurationLocked(changes);
        } else if (mNextTimeout != LLONG_MAX) {
            nsecs_t now = systemTime(SYSTEM_TIME_MONOTONIC);
            timeoutMillis = toMillisecondTimeoutDelay(now, mNextTimeout);
        }
    } // release lock

    size_t count = mEventHub->getEvents(timeoutMillis, mEventBuffer, EVENT_BUFFER_SIZE);

    { // acquire lock
        AutoMutex _l(mLock);
        mReaderIsAliveCondition.broadcast();

        if (count) {
            processEventsLocked(mEventBuffer, count);
        }

        if (mNextTimeout != LLONG_MAX) {
            nsecs_t now = systemTime(SYSTEM_TIME_MONOTONIC);
            if (now >= mNextTimeout) {
#if DEBUG_RAW_EVENTS
                ALOGD("Timeout expired, latency=%0.3fms", (now - mNextTimeout) * 0.000001f);
#endif
                mNextTimeout = LLONG_MAX;
                timeoutExpiredLocked(now);
            }
        }

        if (oldGeneration != mGeneration) {
            inputDevicesChanged = true;
            getInputDevicesLocked(inputDevices);
        }
    } // release lock

    // Send out a message that the describes the changed input devices.
    if (inputDevicesChanged) {
        mPolicy->notifyInputDevicesChanged(inputDevices);
    }

    // Flush queued events out to the listener.
    // This must happen outside of the lock because the listener could potentially call
    // back into the InputReader's methods, such as getScanCodeState, or become blocked
    // on another thread similarly waiting to acquire the InputReader lock thereby
    // resulting in a deadlock.  This situation is actually quite plausible because the
    // listener is actually the input dispatcher, which calls into the window manager,
    // which occasionally calls into the input reader.
    mQueuedListener->flush();
}

void InputReader::processEventsLocked(const RawEvent* rawEvents, size_t count) {
    for (const RawEvent* rawEvent = rawEvents; count;) {
        int32_t type = rawEvent->type;
        size_t batchSize = 1;
        if (type < EventHubInterface::FIRST_SYNTHETIC_EVENT) {
            int32_t deviceId = rawEvent->deviceId;
            while (batchSize < count) {
                if (rawEvent[batchSize].type >= EventHubInterface::FIRST_SYNTHETIC_EVENT
                        || rawEvent[batchSize].deviceId != deviceId) {
                    break;
                }
                batchSize += 1;
            }
#if DEBUG_RAW_EVENTS
            ALOGD("BatchSize: %zu Count: %zu", batchSize, count);
#endif
            processEventsForDeviceLocked(deviceId, rawEvent, batchSize);
        } else {
            switch (rawEvent->type) {
            case EventHubInterface::DEVICE_ADDED:
                addDeviceLocked(rawEvent->when, rawEvent->deviceId);
                break;
            case EventHubInterface::DEVICE_REMOVED:
                removeDeviceLocked(rawEvent->when, rawEvent->deviceId);
                break;
            case EventHubInterface::FINISHED_DEVICE_SCAN:
                handleConfigurationChangedLocked(rawEvent->when);
                break;
            default:
                ALOG_ASSERT(false); // can't happen
                break;
            }
        }
        count -= batchSize;
        rawEvent += batchSize;
    }
}

void InputReader::addDeviceLocked(nsecs_t when, int32_t deviceId) {
    ssize_t deviceIndex = mDevices.indexOfKey(deviceId);
    if (deviceIndex >= 0) {
        ALOGW("Ignoring spurious device added event for deviceId %d.", deviceId);
        return;
    }

    InputDeviceIdentifier identifier = mEventHub->getDeviceIdentifier(deviceId);
    uint32_t classes = mEventHub->getDeviceClasses(deviceId);
    int32_t controllerNumber = mEventHub->getDeviceControllerNumber(deviceId);

    InputDevice* device = createDeviceLocked(deviceId, controllerNumber, identifier, classes);
    device->configure(when, &mConfig, 0);
    device->reset(when);

    if (device->isIgnored()) {
        ALOGI("Device added: id=%d, name='%s' (ignored non-input device)", deviceId,
                identifier.name.c_str());
    } else {
        ALOGI("Device added: id=%d, name='%s', sources=0x%08x", deviceId,
                identifier.name.c_str(), device->getSources());
    }

    mDevices.add(deviceId, device);
    bumpGenerationLocked();

    if (device->getClasses() & INPUT_DEVICE_CLASS_EXTERNAL_STYLUS) {
        notifyExternalStylusPresenceChanged();
    }
}

void InputReader::removeDeviceLocked(nsecs_t when, int32_t deviceId) {
    InputDevice* device = nullptr;
    ssize_t deviceIndex = mDevices.indexOfKey(deviceId);
    if (deviceIndex < 0) {
        ALOGW("Ignoring spurious device removed event for deviceId %d.", deviceId);
        return;
    }

    device = mDevices.valueAt(deviceIndex);
    mDevices.removeItemsAt(deviceIndex, 1);
    bumpGenerationLocked();

    if (device->isIgnored()) {
        ALOGI("Device removed: id=%d, name='%s' (ignored non-input device)",
                device->getId(), device->getName().c_str());
    } else {
        ALOGI("Device removed: id=%d, name='%s', sources=0x%08x",
                device->getId(), device->getName().c_str(), device->getSources());
    }

    if (device->getClasses() & INPUT_DEVICE_CLASS_EXTERNAL_STYLUS) {
        notifyExternalStylusPresenceChanged();
    }

    device->reset(when);
    delete device;
}

InputDevice* InputReader::createDeviceLocked(int32_t deviceId, int32_t controllerNumber,
        const InputDeviceIdentifier& identifier, uint32_t classes) {
    InputDevice* device = new InputDevice(&mContext, deviceId, bumpGenerationLocked(),
            controllerNumber, identifier, classes);

    // External devices.
    if (classes & INPUT_DEVICE_CLASS_EXTERNAL) {
        device->setExternal(true);
    }

    // Devices with mics.
    if (classes & INPUT_DEVICE_CLASS_MIC) {
        device->setMic(true);
    }

    // Switch-like devices.
    if (classes & INPUT_DEVICE_CLASS_SWITCH) {
        device->addMapper(new SwitchInputMapper(device));
    }

    // Scroll wheel-like devices.
    if (classes & INPUT_DEVICE_CLASS_ROTARY_ENCODER) {
        device->addMapper(new RotaryEncoderInputMapper(device));
    }

    // Vibrator-like devices.
    if (classes & INPUT_DEVICE_CLASS_VIBRATOR) {
        device->addMapper(new VibratorInputMapper(device));
    }

    // Keyboard-like devices.
    uint32_t keyboardSource = 0;
    int32_t keyboardType = AINPUT_KEYBOARD_TYPE_NON_ALPHABETIC;
    if (classes & INPUT_DEVICE_CLASS_KEYBOARD) {
        keyboardSource |= AINPUT_SOURCE_KEYBOARD;
    }
    if (classes & INPUT_DEVICE_CLASS_ALPHAKEY) {
        keyboardType = AINPUT_KEYBOARD_TYPE_ALPHABETIC;
    }
    if (classes & INPUT_DEVICE_CLASS_DPAD) {
        keyboardSource |= AINPUT_SOURCE_DPAD;
    }
    if (classes & INPUT_DEVICE_CLASS_GAMEPAD) {
        keyboardSource |= AINPUT_SOURCE_GAMEPAD;
    }

    if (keyboardSource != 0) {
        device->addMapper(new KeyboardInputMapper(device, keyboardSource, keyboardType));
    }

    // Cursor-like devices.
    if (classes & INPUT_DEVICE_CLASS_CURSOR) {
        device->addMapper(new CursorInputMapper(device));
    }

    // Touchscreens and touchpad devices.
    if (classes & INPUT_DEVICE_CLASS_TOUCH_MT) {
        device->addMapper(new MultiTouchInputMapper(device));
    } else if (classes & INPUT_DEVICE_CLASS_TOUCH) {
        device->addMapper(new SingleTouchInputMapper(device));
    }

    // Joystick-like devices.
    if (classes & INPUT_DEVICE_CLASS_JOYSTICK) {
        device->addMapper(new JoystickInputMapper(device));
    }

    // External stylus-like devices.
    if (classes & INPUT_DEVICE_CLASS_EXTERNAL_STYLUS) {
        device->addMapper(new ExternalStylusInputMapper(device));
    }

    return device;
}

void InputReader::processEventsForDeviceLocked(int32_t deviceId,
        const RawEvent* rawEvents, size_t count) {
    ssize_t deviceIndex = mDevices.indexOfKey(deviceId);
    if (deviceIndex < 0) {
        ALOGW("Discarding event for unknown deviceId %d.", deviceId);
        return;
    }

    InputDevice* device = mDevices.valueAt(deviceIndex);
    if (device->isIgnored()) {
        //ALOGD("Discarding event for ignored deviceId %d.", deviceId);
        return;
    }

    device->process(rawEvents, count);
}

void InputReader::timeoutExpiredLocked(nsecs_t when) {
    for (size_t i = 0; i < mDevices.size(); i++) {
        InputDevice* device = mDevices.valueAt(i);
        if (!device->isIgnored()) {
            device->timeoutExpired(when);
        }
    }
}

void InputReader::handleConfigurationChangedLocked(nsecs_t when) {
    // Reset global meta state because it depends on the list of all configured devices.
    updateGlobalMetaStateLocked();

    // Enqueue configuration changed.
    NotifyConfigurationChangedArgs args(mContext.getNextSequenceNum(), when);
    mQueuedListener->notifyConfigurationChanged(&args);
}

void InputReader::refreshConfigurationLocked(uint32_t changes) {
    mPolicy->getReaderConfiguration(&mConfig);
    mEventHub->setExcludedDevices(mConfig.excludedDeviceNames);

    if (changes) {
        ALOGI("Reconfiguring input devices.  changes=0x%08x", changes);
        nsecs_t now = systemTime(SYSTEM_TIME_MONOTONIC);

        if (changes & InputReaderConfiguration::CHANGE_MUST_REOPEN) {
            mEventHub->requestReopenDevices();
        } else {
            for (size_t i = 0; i < mDevices.size(); i++) {
                InputDevice* device = mDevices.valueAt(i);
                device->configure(now, &mConfig, changes);
            }
        }
    }
}

void InputReader::updateGlobalMetaStateLocked() {
    mGlobalMetaState = 0;

    for (size_t i = 0; i < mDevices.size(); i++) {
        InputDevice* device = mDevices.valueAt(i);
        mGlobalMetaState |= device->getMetaState();
    }
}

int32_t InputReader::getGlobalMetaStateLocked() {
    return mGlobalMetaState;
}

void InputReader::notifyExternalStylusPresenceChanged() {
    refreshConfigurationLocked(InputReaderConfiguration::CHANGE_EXTERNAL_STYLUS_PRESENCE);
}

void InputReader::getExternalStylusDevicesLocked(std::vector<InputDeviceInfo>& outDevices) {
    for (size_t i = 0; i < mDevices.size(); i++) {
        InputDevice* device = mDevices.valueAt(i);
        if (device->getClasses() & INPUT_DEVICE_CLASS_EXTERNAL_STYLUS && !device->isIgnored()) {
            InputDeviceInfo info;
            device->getDeviceInfo(&info);
            outDevices.push_back(info);
        }
    }
}

void InputReader::dispatchExternalStylusState(const StylusState& state) {
    for (size_t i = 0; i < mDevices.size(); i++) {
        InputDevice* device = mDevices.valueAt(i);
        device->updateExternalStylusState(state);
    }
}

void InputReader::disableVirtualKeysUntilLocked(nsecs_t time) {
    mDisableVirtualKeysTimeout = time;
}

bool InputReader::shouldDropVirtualKeyLocked(nsecs_t now,
        InputDevice* device, int32_t keyCode, int32_t scanCode) {
    if (now < mDisableVirtualKeysTimeout) {
        ALOGI("Dropping virtual key from device %s because virtual keys are "
                "temporarily disabled for the next %0.3fms.  keyCode=%d, scanCode=%d",
                device->getName().c_str(),
                (mDisableVirtualKeysTimeout - now) * 0.000001,
                keyCode, scanCode);
        return true;
    } else {
        return false;
    }
}

void InputReader::fadePointerLocked() {
    for (size_t i = 0; i < mDevices.size(); i++) {
        InputDevice* device = mDevices.valueAt(i);
        device->fadePointer();
    }
}

void InputReader::requestTimeoutAtTimeLocked(nsecs_t when) {
    if (when < mNextTimeout) {
        mNextTimeout = when;
        mEventHub->wake();
    }
}

int32_t InputReader::bumpGenerationLocked() {
    return ++mGeneration;
}

void InputReader::getInputDevices(std::vector<InputDeviceInfo>& outInputDevices) {
    AutoMutex _l(mLock);
    getInputDevicesLocked(outInputDevices);
}

void InputReader::getInputDevicesLocked(std::vector<InputDeviceInfo>& outInputDevices) {
    outInputDevices.clear();

    size_t numDevices = mDevices.size();
    for (size_t i = 0; i < numDevices; i++) {
        InputDevice* device = mDevices.valueAt(i);
        if (!device->isIgnored()) {
            InputDeviceInfo info;
            device->getDeviceInfo(&info);
            outInputDevices.push_back(info);
        }
    }
}

int32_t InputReader::getKeyCodeState(int32_t deviceId, uint32_t sourceMask,
        int32_t keyCode) {
    AutoMutex _l(mLock);

    return getStateLocked(deviceId, sourceMask, keyCode, &InputDevice::getKeyCodeState);
}

int32_t InputReader::getScanCodeState(int32_t deviceId, uint32_t sourceMask,
        int32_t scanCode) {
    AutoMutex _l(mLock);

    return getStateLocked(deviceId, sourceMask, scanCode, &InputDevice::getScanCodeState);
}

int32_t InputReader::getSwitchState(int32_t deviceId, uint32_t sourceMask, int32_t switchCode) {
    AutoMutex _l(mLock);

    return getStateLocked(deviceId, sourceMask, switchCode, &InputDevice::getSwitchState);
}

int32_t InputReader::getStateLocked(int32_t deviceId, uint32_t sourceMask, int32_t code,
        GetStateFunc getStateFunc) {
    int32_t result = AKEY_STATE_UNKNOWN;
    if (deviceId >= 0) {
        ssize_t deviceIndex = mDevices.indexOfKey(deviceId);
        if (deviceIndex >= 0) {
            InputDevice* device = mDevices.valueAt(deviceIndex);
            if (! device->isIgnored() && sourcesMatchMask(device->getSources(), sourceMask)) {
                result = (device->*getStateFunc)(sourceMask, code);
            }
        }
    } else {
        size_t numDevices = mDevices.size();
        for (size_t i = 0; i < numDevices; i++) {
            InputDevice* device = mDevices.valueAt(i);
            if (! device->isIgnored() && sourcesMatchMask(device->getSources(), sourceMask)) {
                // If any device reports AKEY_STATE_DOWN or AKEY_STATE_VIRTUAL, return that
                // value.  Otherwise, return AKEY_STATE_UP as long as one device reports it.
                int32_t currentResult = (device->*getStateFunc)(sourceMask, code);
                if (currentResult >= AKEY_STATE_DOWN) {
                    return currentResult;
                } else if (currentResult == AKEY_STATE_UP) {
                    result = currentResult;
                }
            }
        }
    }
    return result;
}

void InputReader::toggleCapsLockState(int32_t deviceId) {
    ssize_t deviceIndex = mDevices.indexOfKey(deviceId);
    if (deviceIndex < 0) {
        ALOGW("Ignoring toggleCapsLock for unknown deviceId %" PRId32 ".", deviceId);
        return;
    }

    InputDevice* device = mDevices.valueAt(deviceIndex);
    if (device->isIgnored()) {
        return;
    }

    device->updateMetaState(AKEYCODE_CAPS_LOCK);
}

bool InputReader::hasKeys(int32_t deviceId, uint32_t sourceMask,
        size_t numCodes, const int32_t* keyCodes, uint8_t* outFlags) {
    AutoMutex _l(mLock);

    memset(outFlags, 0, numCodes);
    return markSupportedKeyCodesLocked(deviceId, sourceMask, numCodes, keyCodes, outFlags);
}

bool InputReader::markSupportedKeyCodesLocked(int32_t deviceId, uint32_t sourceMask,
        size_t numCodes, const int32_t* keyCodes, uint8_t* outFlags) {
    bool result = false;
    if (deviceId >= 0) {
        ssize_t deviceIndex = mDevices.indexOfKey(deviceId);
        if (deviceIndex >= 0) {
            InputDevice* device = mDevices.valueAt(deviceIndex);
            if (! device->isIgnored() && sourcesMatchMask(device->getSources(), sourceMask)) {
                result = device->markSupportedKeyCodes(sourceMask,
                        numCodes, keyCodes, outFlags);
            }
        }
    } else {
        size_t numDevices = mDevices.size();
        for (size_t i = 0; i < numDevices; i++) {
            InputDevice* device = mDevices.valueAt(i);
            if (! device->isIgnored() && sourcesMatchMask(device->getSources(), sourceMask)) {
                result |= device->markSupportedKeyCodes(sourceMask,
                        numCodes, keyCodes, outFlags);
            }
        }
    }
    return result;
}

void InputReader::requestRefreshConfiguration(uint32_t changes) {
    AutoMutex _l(mLock);

    if (changes) {
        bool needWake = !mConfigurationChangesToRefresh;
        mConfigurationChangesToRefresh |= changes;

        if (needWake) {
            mEventHub->wake();
        }
    }
}

void InputReader::vibrate(int32_t deviceId, const nsecs_t* pattern, size_t patternSize,
        ssize_t repeat, int32_t token) {
    AutoMutex _l(mLock);

    ssize_t deviceIndex = mDevices.indexOfKey(deviceId);
    if (deviceIndex >= 0) {
        InputDevice* device = mDevices.valueAt(deviceIndex);
        device->vibrate(pattern, patternSize, repeat, token);
    }
}

void InputReader::cancelVibrate(int32_t deviceId, int32_t token) {
    AutoMutex _l(mLock);

    ssize_t deviceIndex = mDevices.indexOfKey(deviceId);
    if (deviceIndex >= 0) {
        InputDevice* device = mDevices.valueAt(deviceIndex);
        device->cancelVibrate(token);
    }
}

bool InputReader::isInputDeviceEnabled(int32_t deviceId) {
    AutoMutex _l(mLock);

    ssize_t deviceIndex = mDevices.indexOfKey(deviceId);
    if (deviceIndex >= 0) {
        InputDevice* device = mDevices.valueAt(deviceIndex);
        return device->isEnabled();
    }
    ALOGW("Ignoring invalid device id %" PRId32 ".", deviceId);
    return false;
}

bool InputReader::canDispatchToDisplay(int32_t deviceId, int32_t displayId) {
    AutoMutex _l(mLock);

    ssize_t deviceIndex = mDevices.indexOfKey(deviceId);
    if (deviceIndex < 0) {
        ALOGW("Ignoring invalid device id %" PRId32 ".", deviceId);
        return false;
    }

    InputDevice* device = mDevices.valueAt(deviceIndex);
    std::optional<int32_t> associatedDisplayId = device->getAssociatedDisplay();
    // No associated display. By default, can dispatch to all displays.
    if (!associatedDisplayId) {
        return true;
    }

    if (*associatedDisplayId == ADISPLAY_ID_NONE) {
        ALOGW("Device has associated, but no associated display id.");
        return true;
    }

    return *associatedDisplayId == displayId;
}

void InputReader::dump(std::string& dump) {
    AutoMutex _l(mLock);

    mEventHub->dump(dump);
    dump += "\n";

    dump += "Input Reader State:\n";

    for (size_t i = 0; i < mDevices.size(); i++) {
        mDevices.valueAt(i)->dump(dump);
    }

    dump += INDENT "Configuration:\n";
    dump += INDENT2 "ExcludedDeviceNames: [";
    for (size_t i = 0; i < mConfig.excludedDeviceNames.size(); i++) {
        if (i != 0) {
            dump += ", ";
        }
        dump += mConfig.excludedDeviceNames[i];
    }
    dump += "]\n";
    dump += StringPrintf(INDENT2 "VirtualKeyQuietTime: %0.1fms\n",
            mConfig.virtualKeyQuietTime * 0.000001f);

    dump += StringPrintf(INDENT2 "PointerVelocityControlParameters: "
            "scale=%0.3f, lowThreshold=%0.3f, highThreshold=%0.3f, acceleration=%0.3f\n",
            mConfig.pointerVelocityControlParameters.scale,
            mConfig.pointerVelocityControlParameters.lowThreshold,
            mConfig.pointerVelocityControlParameters.highThreshold,
            mConfig.pointerVelocityControlParameters.acceleration);

    dump += StringPrintf(INDENT2 "WheelVelocityControlParameters: "
            "scale=%0.3f, lowThreshold=%0.3f, highThreshold=%0.3f, acceleration=%0.3f\n",
            mConfig.wheelVelocityControlParameters.scale,
            mConfig.wheelVelocityControlParameters.lowThreshold,
            mConfig.wheelVelocityControlParameters.highThreshold,
            mConfig.wheelVelocityControlParameters.acceleration);

    dump += StringPrintf(INDENT2 "PointerGesture:\n");
    dump += StringPrintf(INDENT3 "Enabled: %s\n",
            toString(mConfig.pointerGesturesEnabled));
    dump += StringPrintf(INDENT3 "QuietInterval: %0.1fms\n",
            mConfig.pointerGestureQuietInterval * 0.000001f);
    dump += StringPrintf(INDENT3 "DragMinSwitchSpeed: %0.1fpx/s\n",
            mConfig.pointerGestureDragMinSwitchSpeed);
    dump += StringPrintf(INDENT3 "TapInterval: %0.1fms\n",
            mConfig.pointerGestureTapInterval * 0.000001f);
    dump += StringPrintf(INDENT3 "TapDragInterval: %0.1fms\n",
            mConfig.pointerGestureTapDragInterval * 0.000001f);
    dump += StringPrintf(INDENT3 "TapSlop: %0.1fpx\n",
            mConfig.pointerGestureTapSlop);
    dump += StringPrintf(INDENT3 "MultitouchSettleInterval: %0.1fms\n",
            mConfig.pointerGestureMultitouchSettleInterval * 0.000001f);
    dump += StringPrintf(INDENT3 "MultitouchMinDistance: %0.1fpx\n",
            mConfig.pointerGestureMultitouchMinDistance);
    dump += StringPrintf(INDENT3 "SwipeTransitionAngleCosine: %0.1f\n",
            mConfig.pointerGestureSwipeTransitionAngleCosine);
    dump += StringPrintf(INDENT3 "SwipeMaxWidthRatio: %0.1f\n",
            mConfig.pointerGestureSwipeMaxWidthRatio);
    dump += StringPrintf(INDENT3 "MovementSpeedRatio: %0.1f\n",
            mConfig.pointerGestureMovementSpeedRatio);
    dump += StringPrintf(INDENT3 "ZoomSpeedRatio: %0.1f\n",
            mConfig.pointerGestureZoomSpeedRatio);

    dump += INDENT3 "Viewports:\n";
    mConfig.dump(dump);
}

void InputReader::monitor() {
    // Acquire and release the lock to ensure that the reader has not deadlocked.
    mLock.lock();
    mEventHub->wake();
    mReaderIsAliveCondition.wait(mLock);
    mLock.unlock();

    // Check the EventHub
    mEventHub->monitor();
}


// --- InputReader::ContextImpl ---

InputReader::ContextImpl::ContextImpl(InputReader* reader) :
        mReader(reader) {
}

void InputReader::ContextImpl::updateGlobalMetaState() {
    // lock is already held by the input loop
    mReader->updateGlobalMetaStateLocked();
}

int32_t InputReader::ContextImpl::getGlobalMetaState() {
    // lock is already held by the input loop
    return mReader->getGlobalMetaStateLocked();
}

void InputReader::ContextImpl::disableVirtualKeysUntil(nsecs_t time) {
    // lock is already held by the input loop
    mReader->disableVirtualKeysUntilLocked(time);
}

bool InputReader::ContextImpl::shouldDropVirtualKey(nsecs_t now,
        InputDevice* device, int32_t keyCode, int32_t scanCode) {
    // lock is already held by the input loop
    return mReader->shouldDropVirtualKeyLocked(now, device, keyCode, scanCode);
}

void InputReader::ContextImpl::fadePointer() {
    // lock is already held by the input loop
    mReader->fadePointerLocked();
}

void InputReader::ContextImpl::requestTimeoutAtTime(nsecs_t when) {
    // lock is already held by the input loop
    mReader->requestTimeoutAtTimeLocked(when);
}

int32_t InputReader::ContextImpl::bumpGeneration() {
    // lock is already held by the input loop
    return mReader->bumpGenerationLocked();
}

void InputReader::ContextImpl::getExternalStylusDevices(std::vector<InputDeviceInfo>& outDevices) {
    // lock is already held by whatever called refreshConfigurationLocked
    mReader->getExternalStylusDevicesLocked(outDevices);
}

void InputReader::ContextImpl::dispatchExternalStylusState(const StylusState& state) {
    mReader->dispatchExternalStylusState(state);
}

InputReaderPolicyInterface* InputReader::ContextImpl::getPolicy() {
    return mReader->mPolicy.get();
}

InputListenerInterface* InputReader::ContextImpl::getListener() {
    return mReader->mQueuedListener.get();
}

EventHubInterface* InputReader::ContextImpl::getEventHub() {
    return mReader->mEventHub.get();
}

uint32_t InputReader::ContextImpl::getNextSequenceNum() {
    return (mReader->mNextSequenceNum)++;
}

// --- InputDevice ---

InputDevice::InputDevice(InputReaderContext* context, int32_t id, int32_t generation,
        int32_t controllerNumber, const InputDeviceIdentifier& identifier, uint32_t classes) :
        mContext(context), mId(id), mGeneration(generation), mControllerNumber(controllerNumber),
        mIdentifier(identifier), mClasses(classes),
        mSources(0), mIsExternal(false), mHasMic(false), mDropUntilNextSync(false) {
}

InputDevice::~InputDevice() {
    size_t numMappers = mMappers.size();
    for (size_t i = 0; i < numMappers; i++) {
        delete mMappers[i];
    }
    mMappers.clear();
}

bool InputDevice::isEnabled() {
    return getEventHub()->isDeviceEnabled(mId);
}

void InputDevice::setEnabled(bool enabled, nsecs_t when) {
    if (isEnabled() == enabled) {
        return;
    }

    if (enabled) {
        getEventHub()->enableDevice(mId);
        reset(when);
    } else {
        reset(when);
        getEventHub()->disableDevice(mId);
    }
    // Must change generation to flag this device as changed
    bumpGeneration();
}

void InputDevice::dump(std::string& dump) {
    InputDeviceInfo deviceInfo;
    getDeviceInfo(&deviceInfo);

    dump += StringPrintf(INDENT "Device %d: %s\n", deviceInfo.getId(),
            deviceInfo.getDisplayName().c_str());
    dump += StringPrintf(INDENT2 "Generation: %d\n", mGeneration);
    dump += StringPrintf(INDENT2 "IsExternal: %s\n", toString(mIsExternal));
    dump += StringPrintf(INDENT2 "AssociatedDisplayPort: ");
    if (mAssociatedDisplayPort) {
        dump += StringPrintf("%" PRIu8 "\n", *mAssociatedDisplayPort);
    } else {
        dump += "<none>\n";
    }
    dump += StringPrintf(INDENT2 "HasMic:     %s\n", toString(mHasMic));
    dump += StringPrintf(INDENT2 "Sources: 0x%08x\n", deviceInfo.getSources());
    dump += StringPrintf(INDENT2 "KeyboardType: %d\n", deviceInfo.getKeyboardType());

    const std::vector<InputDeviceInfo::MotionRange>& ranges = deviceInfo.getMotionRanges();
    if (!ranges.empty()) {
        dump += INDENT2 "Motion Ranges:\n";
        for (size_t i = 0; i < ranges.size(); i++) {
            const InputDeviceInfo::MotionRange& range = ranges[i];
            const char* label = getAxisLabel(range.axis);
            char name[32];
            if (label) {
                strncpy(name, label, sizeof(name));
                name[sizeof(name) - 1] = '\0';
            } else {
                snprintf(name, sizeof(name), "%d", range.axis);
            }
            dump += StringPrintf(INDENT3 "%s: source=0x%08x, "
                    "min=%0.3f, max=%0.3f, flat=%0.3f, fuzz=%0.3f, resolution=%0.3f\n",
                    name, range.source, range.min, range.max, range.flat, range.fuzz,
                    range.resolution);
        }
    }

    size_t numMappers = mMappers.size();
    for (size_t i = 0; i < numMappers; i++) {
        InputMapper* mapper = mMappers[i];
        mapper->dump(dump);
    }
}

void InputDevice::addMapper(InputMapper* mapper) {
    mMappers.push_back(mapper);
}

void InputDevice::configure(nsecs_t when, const InputReaderConfiguration* config, uint32_t changes) {
    mSources = 0;

    if (!isIgnored()) {
        if (!changes) { // first time only
            mContext->getEventHub()->getConfiguration(mId, &mConfiguration);
        }

        if (!changes || (changes & InputReaderConfiguration::CHANGE_KEYBOARD_LAYOUTS)) {
            if (!(mClasses & INPUT_DEVICE_CLASS_VIRTUAL)) {
                sp<KeyCharacterMap> keyboardLayout =
                        mContext->getPolicy()->getKeyboardLayoutOverlay(mIdentifier);
                if (mContext->getEventHub()->setKeyboardLayoutOverlay(mId, keyboardLayout)) {
                    bumpGeneration();
                }
            }
        }

        if (!changes || (changes & InputReaderConfiguration::CHANGE_DEVICE_ALIAS)) {
            if (!(mClasses & INPUT_DEVICE_CLASS_VIRTUAL)) {
                std::string alias = mContext->getPolicy()->getDeviceAlias(mIdentifier);
                if (mAlias != alias) {
                    mAlias = alias;
                    bumpGeneration();
                }
            }
        }

        if (!changes || (changes & InputReaderConfiguration::CHANGE_ENABLED_STATE)) {
            ssize_t index = config->disabledDevices.indexOf(mId);
            bool enabled = index < 0;
            setEnabled(enabled, when);
        }

        if (!changes || (changes & InputReaderConfiguration::CHANGE_DISPLAY_INFO)) {
             // In most situations, no port will be specified.
            mAssociatedDisplayPort = std::nullopt;
            // Find the display port that corresponds to the current input port.
            const std::string& inputPort = mIdentifier.location;
            if (!inputPort.empty()) {
                const std::unordered_map<std::string, uint8_t>& ports = config->portAssociations;
                const auto& displayPort = ports.find(inputPort);
                if (displayPort != ports.end()) {
                    mAssociatedDisplayPort = std::make_optional(displayPort->second);
                }
            }
        }

        for (InputMapper* mapper : mMappers) {
            mapper->configure(when, config, changes);
            mSources |= mapper->getSources();
        }
    }
}

void InputDevice::reset(nsecs_t when) {
    for (InputMapper* mapper : mMappers) {
        mapper->reset(when);
    }

    mContext->updateGlobalMetaState();

    notifyReset(when);
}

void InputDevice::process(const RawEvent* rawEvents, size_t count) {
    // Process all of the events in order for each mapper.
    // We cannot simply ask each mapper to process them in bulk because mappers may
    // have side-effects that must be interleaved.  For example, joystick movement events and
    // gamepad button presses are handled by different mappers but they should be dispatched
    // in the order received.
    for (const RawEvent* rawEvent = rawEvents; count != 0; rawEvent++) {
#if DEBUG_RAW_EVENTS
        ALOGD("Input event: device=%d type=0x%04x code=0x%04x value=0x%08x when=%" PRId64,
                rawEvent->deviceId, rawEvent->type, rawEvent->code, rawEvent->value,
                rawEvent->when);
#endif

        if (mDropUntilNextSync) {
            if (rawEvent->type == EV_SYN && rawEvent->code == SYN_REPORT) {
                mDropUntilNextSync = false;
#if DEBUG_RAW_EVENTS
                ALOGD("Recovered from input event buffer overrun.");
#endif
            } else {
#if DEBUG_RAW_EVENTS
                ALOGD("Dropped input event while waiting for next input sync.");
#endif
            }
        } else if (rawEvent->type == EV_SYN && rawEvent->code == SYN_DROPPED) {
            ALOGI("Detected input event buffer overrun for device %s.", getName().c_str());
            mDropUntilNextSync = true;
            reset(rawEvent->when);
        } else {
            for (InputMapper* mapper : mMappers) {
                mapper->process(rawEvent);
            }
        }
        --count;
    }
}

void InputDevice::timeoutExpired(nsecs_t when) {
    for (InputMapper* mapper : mMappers) {
        mapper->timeoutExpired(when);
    }
}

void InputDevice::updateExternalStylusState(const StylusState& state) {
    for (InputMapper* mapper : mMappers) {
        mapper->updateExternalStylusState(state);
    }
}

void InputDevice::getDeviceInfo(InputDeviceInfo* outDeviceInfo) {
    outDeviceInfo->initialize(mId, mGeneration, mControllerNumber, mIdentifier, mAlias,
            mIsExternal, mHasMic);
    for (InputMapper* mapper : mMappers) {
        mapper->populateDeviceInfo(outDeviceInfo);
    }
}

int32_t InputDevice::getKeyCodeState(uint32_t sourceMask, int32_t keyCode) {
    return getState(sourceMask, keyCode, & InputMapper::getKeyCodeState);
}

int32_t InputDevice::getScanCodeState(uint32_t sourceMask, int32_t scanCode) {
    return getState(sourceMask, scanCode, & InputMapper::getScanCodeState);
}

int32_t InputDevice::getSwitchState(uint32_t sourceMask, int32_t switchCode) {
    return getState(sourceMask, switchCode, & InputMapper::getSwitchState);
}

int32_t InputDevice::getState(uint32_t sourceMask, int32_t code, GetStateFunc getStateFunc) {
    int32_t result = AKEY_STATE_UNKNOWN;
    for (InputMapper* mapper : mMappers) {
        if (sourcesMatchMask(mapper->getSources(), sourceMask)) {
            // If any mapper reports AKEY_STATE_DOWN or AKEY_STATE_VIRTUAL, return that
            // value.  Otherwise, return AKEY_STATE_UP as long as one mapper reports it.
            int32_t currentResult = (mapper->*getStateFunc)(sourceMask, code);
            if (currentResult >= AKEY_STATE_DOWN) {
                return currentResult;
            } else if (currentResult == AKEY_STATE_UP) {
                result = currentResult;
            }
        }
    }
    return result;
}

bool InputDevice::markSupportedKeyCodes(uint32_t sourceMask, size_t numCodes,
        const int32_t* keyCodes, uint8_t* outFlags) {
    bool result = false;
    for (InputMapper* mapper : mMappers) {
        if (sourcesMatchMask(mapper->getSources(), sourceMask)) {
            result |= mapper->markSupportedKeyCodes(sourceMask, numCodes, keyCodes, outFlags);
        }
    }
    return result;
}

void InputDevice::vibrate(const nsecs_t* pattern, size_t patternSize, ssize_t repeat,
        int32_t token) {
    for (InputMapper* mapper : mMappers) {
        mapper->vibrate(pattern, patternSize, repeat, token);
    }
}

void InputDevice::cancelVibrate(int32_t token) {
    for (InputMapper* mapper : mMappers) {
        mapper->cancelVibrate(token);
    }
}

void InputDevice::cancelTouch(nsecs_t when) {
    for (InputMapper* mapper : mMappers) {
        mapper->cancelTouch(when);
    }
}

int32_t InputDevice::getMetaState() {
    int32_t result = 0;
    for (InputMapper* mapper : mMappers) {
        result |= mapper->getMetaState();
    }
    return result;
}

void InputDevice::updateMetaState(int32_t keyCode) {
    for (InputMapper* mapper : mMappers) {
        mapper->updateMetaState(keyCode);
    }
}

void InputDevice::fadePointer() {
    for (InputMapper* mapper : mMappers) {
        mapper->fadePointer();
    }
}

void InputDevice::bumpGeneration() {
    mGeneration = mContext->bumpGeneration();
}

void InputDevice::notifyReset(nsecs_t when) {
    NotifyDeviceResetArgs args(mContext->getNextSequenceNum(), when, mId);
    mContext->getListener()->notifyDeviceReset(&args);
}

std::optional<int32_t> InputDevice::getAssociatedDisplay() {
    for (InputMapper* mapper : mMappers) {
        std::optional<int32_t> associatedDisplayId = mapper->getAssociatedDisplay();
        if (associatedDisplayId) {
            return associatedDisplayId;
        }
    }

    return std::nullopt;
}

// --- CursorButtonAccumulator ---

CursorButtonAccumulator::CursorButtonAccumulator() {
    clearButtons();
}

void CursorButtonAccumulator::reset(InputDevice* device) {
    mBtnLeft = device->isKeyPressed(BTN_LEFT);
    mBtnRight = device->isKeyPressed(BTN_RIGHT);
    mBtnMiddle = device->isKeyPressed(BTN_MIDDLE);
    mBtnBack = device->isKeyPressed(BTN_BACK);
    mBtnSide = device->isKeyPressed(BTN_SIDE);
    mBtnForward = device->isKeyPressed(BTN_FORWARD);
    mBtnExtra = device->isKeyPressed(BTN_EXTRA);
    mBtnTask = device->isKeyPressed(BTN_TASK);
}

void CursorButtonAccumulator::clearButtons() {
    mBtnLeft = 0;
    mBtnRight = 0;
    mBtnMiddle = 0;
    mBtnBack = 0;
    mBtnSide = 0;
    mBtnForward = 0;
    mBtnExtra = 0;
    mBtnTask = 0;
}

void CursorButtonAccumulator::process(const RawEvent* rawEvent) {
    if (rawEvent->type == EV_KEY) {
        switch (rawEvent->code) {
        case BTN_LEFT:
            mBtnLeft = rawEvent->value;
            break;
        case BTN_RIGHT:
            mBtnRight = rawEvent->value;
            break;
        case BTN_MIDDLE:
            mBtnMiddle = rawEvent->value;
            break;
        case BTN_BACK:
            mBtnBack = rawEvent->value;
            break;
        case BTN_SIDE:
            mBtnSide = rawEvent->value;
            break;
        case BTN_FORWARD:
            mBtnForward = rawEvent->value;
            break;
        case BTN_EXTRA:
            mBtnExtra = rawEvent->value;
            break;
        case BTN_TASK:
            mBtnTask = rawEvent->value;
            break;
        }
    }
}

uint32_t CursorButtonAccumulator::getButtonState() const {
    uint32_t result = 0;
    if (mBtnLeft) {
        result |= AMOTION_EVENT_BUTTON_PRIMARY;
    }
    if (mBtnRight) {
        result |= AMOTION_EVENT_BUTTON_SECONDARY;
    }
    if (mBtnMiddle) {
        result |= AMOTION_EVENT_BUTTON_TERTIARY;
    }
    if (mBtnBack || mBtnSide) {
        result |= AMOTION_EVENT_BUTTON_BACK;
    }
    if (mBtnForward || mBtnExtra) {
        result |= AMOTION_EVENT_BUTTON_FORWARD;
    }
    return result;
}


// --- CursorMotionAccumulator ---

CursorMotionAccumulator::CursorMotionAccumulator() {
    clearRelativeAxes();
}

void CursorMotionAccumulator::reset(InputDevice* device) {
    clearRelativeAxes();
}

void CursorMotionAccumulator::clearRelativeAxes() {
    mRelX = 0;
    mRelY = 0;
}

void CursorMotionAccumulator::process(const RawEvent* rawEvent) {
    if (rawEvent->type == EV_REL) {
        switch (rawEvent->code) {
        case REL_X:
            mRelX = rawEvent->value;
            break;
        case REL_Y:
            mRelY = rawEvent->value;
            break;
        }
    }
}

void CursorMotionAccumulator::finishSync() {
    clearRelativeAxes();
}


// --- CursorScrollAccumulator ---

CursorScrollAccumulator::CursorScrollAccumulator() :
        mHaveRelWheel(false), mHaveRelHWheel(false) {
    clearRelativeAxes();
}

void CursorScrollAccumulator::configure(InputDevice* device) {
    mHaveRelWheel = device->getEventHub()->hasRelativeAxis(device->getId(), REL_WHEEL);
    mHaveRelHWheel = device->getEventHub()->hasRelativeAxis(device->getId(), REL_HWHEEL);
}

void CursorScrollAccumulator::reset(InputDevice* device) {
    clearRelativeAxes();
}

void CursorScrollAccumulator::clearRelativeAxes() {
    mRelWheel = 0;
    mRelHWheel = 0;
}

void CursorScrollAccumulator::process(const RawEvent* rawEvent) {
    if (rawEvent->type == EV_REL) {
        switch (rawEvent->code) {
        case REL_WHEEL:
            mRelWheel = rawEvent->value;
            break;
        case REL_HWHEEL:
            mRelHWheel = rawEvent->value;
            break;
        }
    }
}

void CursorScrollAccumulator::finishSync() {
    clearRelativeAxes();
}


// --- TouchButtonAccumulator ---

TouchButtonAccumulator::TouchButtonAccumulator() :
        mHaveBtnTouch(false), mHaveStylus(false) {
    clearButtons();
}

void TouchButtonAccumulator::configure(InputDevice* device) {
    mHaveBtnTouch = device->hasKey(BTN_TOUCH);
    mHaveStylus = device->hasKey(BTN_TOOL_PEN)
            || device->hasKey(BTN_TOOL_RUBBER)
            || device->hasKey(BTN_TOOL_BRUSH)
            || device->hasKey(BTN_TOOL_PENCIL)
            || device->hasKey(BTN_TOOL_AIRBRUSH);
}

void TouchButtonAccumulator::reset(InputDevice* device) {
    mBtnTouch = device->isKeyPressed(BTN_TOUCH);
    mBtnStylus = device->isKeyPressed(BTN_STYLUS);
    // BTN_0 is what gets mapped for the HID usage Digitizers.SecondaryBarrelSwitch
    mBtnStylus2 =
            device->isKeyPressed(BTN_STYLUS2) || device->isKeyPressed(BTN_0);
    mBtnToolFinger = device->isKeyPressed(BTN_TOOL_FINGER);
    mBtnToolPen = device->isKeyPressed(BTN_TOOL_PEN);
    mBtnToolRubber = device->isKeyPressed(BTN_TOOL_RUBBER);
    mBtnToolBrush = device->isKeyPressed(BTN_TOOL_BRUSH);
    mBtnToolPencil = device->isKeyPressed(BTN_TOOL_PENCIL);
    mBtnToolAirbrush = device->isKeyPressed(BTN_TOOL_AIRBRUSH);
    mBtnToolMouse = device->isKeyPressed(BTN_TOOL_MOUSE);
    mBtnToolLens = device->isKeyPressed(BTN_TOOL_LENS);
    mBtnToolDoubleTap = device->isKeyPressed(BTN_TOOL_DOUBLETAP);
    mBtnToolTripleTap = device->isKeyPressed(BTN_TOOL_TRIPLETAP);
    mBtnToolQuadTap = device->isKeyPressed(BTN_TOOL_QUADTAP);
}

void TouchButtonAccumulator::clearButtons() {
    mBtnTouch = 0;
    mBtnStylus = 0;
    mBtnStylus2 = 0;
    mBtnToolFinger = 0;
    mBtnToolPen = 0;
    mBtnToolRubber = 0;
    mBtnToolBrush = 0;
    mBtnToolPencil = 0;
    mBtnToolAirbrush = 0;
    mBtnToolMouse = 0;
    mBtnToolLens = 0;
    mBtnToolDoubleTap = 0;
    mBtnToolTripleTap = 0;
    mBtnToolQuadTap = 0;
}

void TouchButtonAccumulator::process(const RawEvent* rawEvent) {
    if (rawEvent->type == EV_KEY) {
        switch (rawEvent->code) {
        case BTN_TOUCH:
            mBtnTouch = rawEvent->value;
            break;
        case BTN_STYLUS:
            mBtnStylus = rawEvent->value;
            break;
        case BTN_STYLUS2:
        case BTN_0:// BTN_0 is what gets mapped for the HID usage Digitizers.SecondaryBarrelSwitch
            mBtnStylus2 = rawEvent->value;
            break;
        case BTN_TOOL_FINGER:
            mBtnToolFinger = rawEvent->value;
            break;
        case BTN_TOOL_PEN:
            mBtnToolPen = rawEvent->value;
            break;
        case BTN_TOOL_RUBBER:
            mBtnToolRubber = rawEvent->value;
            break;
        case BTN_TOOL_BRUSH:
            mBtnToolBrush = rawEvent->value;
            break;
        case BTN_TOOL_PENCIL:
            mBtnToolPencil = rawEvent->value;
            break;
        case BTN_TOOL_AIRBRUSH:
            mBtnToolAirbrush = rawEvent->value;
            break;
        case BTN_TOOL_MOUSE:
            mBtnToolMouse = rawEvent->value;
            break;
        case BTN_TOOL_LENS:
            mBtnToolLens = rawEvent->value;
            break;
        case BTN_TOOL_DOUBLETAP:
            mBtnToolDoubleTap = rawEvent->value;
            break;
        case BTN_TOOL_TRIPLETAP:
            mBtnToolTripleTap = rawEvent->value;
            break;
        case BTN_TOOL_QUADTAP:
            mBtnToolQuadTap = rawEvent->value;
            break;
        }
    }
}

uint32_t TouchButtonAccumulator::getButtonState() const {
    uint32_t result = 0;
    if (mBtnStylus) {
        result |= AMOTION_EVENT_BUTTON_STYLUS_PRIMARY;
    }
    if (mBtnStylus2) {
        result |= AMOTION_EVENT_BUTTON_STYLUS_SECONDARY;
    }
    return result;
}

int32_t TouchButtonAccumulator::getToolType() const {
    if (mBtnToolMouse || mBtnToolLens) {
        return AMOTION_EVENT_TOOL_TYPE_MOUSE;
    }
    if (mBtnToolRubber) {
        return AMOTION_EVENT_TOOL_TYPE_ERASER;
    }
    if (mBtnToolPen || mBtnToolBrush || mBtnToolPencil || mBtnToolAirbrush) {
        return AMOTION_EVENT_TOOL_TYPE_STYLUS;
    }
    if (mBtnToolFinger || mBtnToolDoubleTap || mBtnToolTripleTap || mBtnToolQuadTap) {
        return AMOTION_EVENT_TOOL_TYPE_FINGER;
    }
    return AMOTION_EVENT_TOOL_TYPE_UNKNOWN;
}

bool TouchButtonAccumulator::isToolActive() const {
    return mBtnTouch || mBtnToolFinger || mBtnToolPen || mBtnToolRubber
            || mBtnToolBrush || mBtnToolPencil || mBtnToolAirbrush
            || mBtnToolMouse || mBtnToolLens
            || mBtnToolDoubleTap || mBtnToolTripleTap || mBtnToolQuadTap;
}

bool TouchButtonAccumulator::isHovering() const {
    return mHaveBtnTouch && !mBtnTouch;
}

bool TouchButtonAccumulator::hasStylus() const {
    return mHaveStylus;
}


// --- RawPointerAxes ---

RawPointerAxes::RawPointerAxes() {
    clear();
}

void RawPointerAxes::clear() {
    x.clear();
    y.clear();
    pressure.clear();
    touchMajor.clear();
    touchMinor.clear();
    toolMajor.clear();
    toolMinor.clear();
    orientation.clear();
    distance.clear();
    tiltX.clear();
    tiltY.clear();
    trackingId.clear();
    slot.clear();
}


// --- RawPointerData ---

RawPointerData::RawPointerData() {
    clear();
}

void RawPointerData::clear() {
    pointerCount = 0;
    clearIdBits();
}

void RawPointerData::copyFrom(const RawPointerData& other) {
    pointerCount = other.pointerCount;
    hoveringIdBits = other.hoveringIdBits;
    touchingIdBits = other.touchingIdBits;

    for (uint32_t i = 0; i < pointerCount; i++) {
        pointers[i] = other.pointers[i];

        int id = pointers[i].id;
        idToIndex[id] = other.idToIndex[id];
    }
}

void RawPointerData::getCentroidOfTouchingPointers(float* outX, float* outY) const {
    float x = 0, y = 0;
    uint32_t count = touchingIdBits.count();
    if (count) {
        for (BitSet32 idBits(touchingIdBits); !idBits.isEmpty(); ) {
            uint32_t id = idBits.clearFirstMarkedBit();
            const Pointer& pointer = pointerForId(id);
            x += pointer.x;
            y += pointer.y;
        }
        x /= count;
        y /= count;
    }
    *outX = x;
    *outY = y;
}


// --- CookedPointerData ---

CookedPointerData::CookedPointerData() {
    clear();
}

void CookedPointerData::clear() {
    pointerCount = 0;
    hoveringIdBits.clear();
    touchingIdBits.clear();
}

void CookedPointerData::copyFrom(const CookedPointerData& other) {
    pointerCount = other.pointerCount;
    hoveringIdBits = other.hoveringIdBits;
    touchingIdBits = other.touchingIdBits;

    for (uint32_t i = 0; i < pointerCount; i++) {
        pointerProperties[i].copyFrom(other.pointerProperties[i]);
        pointerCoords[i].copyFrom(other.pointerCoords[i]);

        int id = pointerProperties[i].id;
        idToIndex[id] = other.idToIndex[id];
    }
}


// --- SingleTouchMotionAccumulator ---

SingleTouchMotionAccumulator::SingleTouchMotionAccumulator() {
    clearAbsoluteAxes();
}

void SingleTouchMotionAccumulator::reset(InputDevice* device) {
    mAbsX = device->getAbsoluteAxisValue(ABS_X);
    mAbsY = device->getAbsoluteAxisValue(ABS_Y);
    mAbsPressure = device->getAbsoluteAxisValue(ABS_PRESSURE);
    mAbsToolWidth = device->getAbsoluteAxisValue(ABS_TOOL_WIDTH);
    mAbsDistance = device->getAbsoluteAxisValue(ABS_DISTANCE);
    mAbsTiltX = device->getAbsoluteAxisValue(ABS_TILT_X);
    mAbsTiltY = device->getAbsoluteAxisValue(ABS_TILT_Y);
}

void SingleTouchMotionAccumulator::clearAbsoluteAxes() {
    mAbsX = 0;
    mAbsY = 0;
    mAbsPressure = 0;
    mAbsToolWidth = 0;
    mAbsDistance = 0;
    mAbsTiltX = 0;
    mAbsTiltY = 0;
}

void SingleTouchMotionAccumulator::process(const RawEvent* rawEvent) {
    if (rawEvent->type == EV_ABS) {
        switch (rawEvent->code) {
        case ABS_X:
            mAbsX = rawEvent->value;
            break;
        case ABS_Y:
            mAbsY = rawEvent->value;
            break;
        case ABS_PRESSURE:
            mAbsPressure = rawEvent->value;
            break;
        case ABS_TOOL_WIDTH:
            mAbsToolWidth = rawEvent->value;
            break;
        case ABS_DISTANCE:
            mAbsDistance = rawEvent->value;
            break;
        case ABS_TILT_X:
            mAbsTiltX = rawEvent->value;
            break;
        case ABS_TILT_Y:
            mAbsTiltY = rawEvent->value;
            break;
        }
    }
}


// --- MultiTouchMotionAccumulator ---

MultiTouchMotionAccumulator::MultiTouchMotionAccumulator() :
        mCurrentSlot(-1), mSlots(nullptr), mSlotCount(0), mUsingSlotsProtocol(false),
        mHaveStylus(false), mDeviceTimestamp(0) {
}

MultiTouchMotionAccumulator::~MultiTouchMotionAccumulator() {
    delete[] mSlots;
}

void MultiTouchMotionAccumulator::configure(InputDevice* device,
        size_t slotCount, bool usingSlotsProtocol) {
    mSlotCount = slotCount;
    mUsingSlotsProtocol = usingSlotsProtocol;
    mHaveStylus = device->hasAbsoluteAxis(ABS_MT_TOOL_TYPE);

    delete[] mSlots;
    mSlots = new Slot[slotCount];
}

void MultiTouchMotionAccumulator::reset(InputDevice* device) {
    // Unfortunately there is no way to read the initial contents of the slots.
    // So when we reset the accumulator, we must assume they are all zeroes.
    if (mUsingSlotsProtocol) {
        // Query the driver for the current slot index and use it as the initial slot
        // before we start reading events from the device.  It is possible that the
        // current slot index will not be the same as it was when the first event was
        // written into the evdev buffer, which means the input mapper could start
        // out of sync with the initial state of the events in the evdev buffer.
        // In the extremely unlikely case that this happens, the data from
        // two slots will be confused until the next ABS_MT_SLOT event is received.
        // This can cause the touch point to "jump", but at least there will be
        // no stuck touches.
        int32_t initialSlot;
        status_t status = device->getEventHub()->getAbsoluteAxisValue(device->getId(),
                ABS_MT_SLOT, &initialSlot);
        if (status) {
            ALOGD("Could not retrieve current multitouch slot index.  status=%d", status);
            initialSlot = -1;
        }
        clearSlots(initialSlot);
    } else {
        clearSlots(-1);
    }
    mDeviceTimestamp = 0;
}

void MultiTouchMotionAccumulator::clearSlots(int32_t initialSlot) {
    if (mSlots) {
        for (size_t i = 0; i < mSlotCount; i++) {
            mSlots[i].clear();
        }
    }
    mCurrentSlot = initialSlot;
}

void MultiTouchMotionAccumulator::process(const RawEvent* rawEvent) {
    if (rawEvent->type == EV_ABS) {
        bool newSlot = false;
        if (mUsingSlotsProtocol) {
            if (rawEvent->code == ABS_MT_SLOT) {
                mCurrentSlot = rawEvent->value;
                newSlot = true;
            }
        } else if (mCurrentSlot < 0) {
            mCurrentSlot = 0;
        }

        if (mCurrentSlot < 0 || size_t(mCurrentSlot) >= mSlotCount) {
#if DEBUG_POINTERS
            if (newSlot) {
                ALOGW("MultiTouch device emitted invalid slot index %d but it "
                        "should be between 0 and %zd; ignoring this slot.",
                        mCurrentSlot, mSlotCount - 1);
            }
#endif
        } else {
            Slot* slot = &mSlots[mCurrentSlot];

            switch (rawEvent->code) {
            case ABS_MT_POSITION_X:
                slot->mInUse = true;
                slot->mAbsMTPositionX = rawEvent->value;
                break;
            case ABS_MT_POSITION_Y:
                slot->mInUse = true;
                slot->mAbsMTPositionY = rawEvent->value;
                break;
            case ABS_MT_TOUCH_MAJOR:
                slot->mInUse = true;
                slot->mAbsMTTouchMajor = rawEvent->value;
                break;
            case ABS_MT_TOUCH_MINOR:
                slot->mInUse = true;
                slot->mAbsMTTouchMinor = rawEvent->value;
                slot->mHaveAbsMTTouchMinor = true;
                break;
            case ABS_MT_WIDTH_MAJOR:
                slot->mInUse = true;
                slot->mAbsMTWidthMajor = rawEvent->value;
                break;
            case ABS_MT_WIDTH_MINOR:
                slot->mInUse = true;
                slot->mAbsMTWidthMinor = rawEvent->value;
                slot->mHaveAbsMTWidthMinor = true;
                break;
            case ABS_MT_ORIENTATION:
                slot->mInUse = true;
                slot->mAbsMTOrientation = rawEvent->value;
                break;
            case ABS_MT_TRACKING_ID:
                if (mUsingSlotsProtocol && rawEvent->value < 0) {
                    // The slot is no longer in use but it retains its previous contents,
                    // which may be reused for subsequent touches.
                    slot->mInUse = false;
                } else {
                    slot->mInUse = true;
                    slot->mAbsMTTrackingId = rawEvent->value;
                }
                break;
            case ABS_MT_PRESSURE:
                slot->mInUse = true;
                slot->mAbsMTPressure = rawEvent->value;
                break;
            case ABS_MT_DISTANCE:
                slot->mInUse = true;
                slot->mAbsMTDistance = rawEvent->value;
                break;
            case ABS_MT_TOOL_TYPE:
                slot->mInUse = true;
                slot->mAbsMTToolType = rawEvent->value;
                slot->mHaveAbsMTToolType = true;
                break;
            }
        }
    } else if (rawEvent->type == EV_SYN && rawEvent->code == SYN_MT_REPORT) {
        // MultiTouch Sync: The driver has returned all data for *one* of the pointers.
        mCurrentSlot += 1;
    } else if (rawEvent->type == EV_MSC && rawEvent->code == MSC_TIMESTAMP) {
        mDeviceTimestamp = rawEvent->value;
    }
}

void MultiTouchMotionAccumulator::finishSync() {
    if (!mUsingSlotsProtocol) {
        clearSlots(-1);
    }
}

bool MultiTouchMotionAccumulator::hasStylus() const {
    return mHaveStylus;
}


// --- MultiTouchMotionAccumulator::Slot ---

MultiTouchMotionAccumulator::Slot::Slot() {
    clear();
}

void MultiTouchMotionAccumulator::Slot::clear() {
    mInUse = false;
    mHaveAbsMTTouchMinor = false;
    mHaveAbsMTWidthMinor = false;
    mHaveAbsMTToolType = false;
    mAbsMTPositionX = 0;
    mAbsMTPositionY = 0;
    mAbsMTTouchMajor = 0;
    mAbsMTTouchMinor = 0;
    mAbsMTWidthMajor = 0;
    mAbsMTWidthMinor = 0;
    mAbsMTOrientation = 0;
    mAbsMTTrackingId = -1;
    mAbsMTPressure = 0;
    mAbsMTDistance = 0;
    mAbsMTToolType = 0;
}

int32_t MultiTouchMotionAccumulator::Slot::getToolType() const {
    if (mHaveAbsMTToolType) {
        switch (mAbsMTToolType) {
        case MT_TOOL_FINGER:
            return AMOTION_EVENT_TOOL_TYPE_FINGER;
        case MT_TOOL_PEN:
            return AMOTION_EVENT_TOOL_TYPE_STYLUS;
        }
    }
    return AMOTION_EVENT_TOOL_TYPE_UNKNOWN;
}


// --- InputMapper ---

InputMapper::InputMapper(InputDevice* device) :
        mDevice(device), mContext(device->getContext()) {
}

InputMapper::~InputMapper() {
}

void InputMapper::populateDeviceInfo(InputDeviceInfo* info) {
    info->addSource(getSources());
}

void InputMapper::dump(std::string& dump) {
}

void InputMapper::configure(nsecs_t when,
        const InputReaderConfiguration* config, uint32_t changes) {
}

void InputMapper::reset(nsecs_t when) {
}

void InputMapper::timeoutExpired(nsecs_t when) {
}

int32_t InputMapper::getKeyCodeState(uint32_t sourceMask, int32_t keyCode) {
    return AKEY_STATE_UNKNOWN;
}

int32_t InputMapper::getScanCodeState(uint32_t sourceMask, int32_t scanCode) {
    return AKEY_STATE_UNKNOWN;
}

int32_t InputMapper::getSwitchState(uint32_t sourceMask, int32_t switchCode) {
    return AKEY_STATE_UNKNOWN;
}

bool InputMapper::markSupportedKeyCodes(uint32_t sourceMask, size_t numCodes,
        const int32_t* keyCodes, uint8_t* outFlags) {
    return false;
}

void InputMapper::vibrate(const nsecs_t* pattern, size_t patternSize, ssize_t repeat,
        int32_t token) {
}

void InputMapper::cancelVibrate(int32_t token) {
}

void InputMapper::cancelTouch(nsecs_t when) {
}

int32_t InputMapper::getMetaState() {
    return 0;
}

void InputMapper::updateMetaState(int32_t keyCode) {
}

void InputMapper::updateExternalStylusState(const StylusState& state) {

}

void InputMapper::fadePointer() {
}

status_t InputMapper::getAbsoluteAxisInfo(int32_t axis, RawAbsoluteAxisInfo* axisInfo) {
    return getEventHub()->getAbsoluteAxisInfo(getDeviceId(), axis, axisInfo);
}

void InputMapper::bumpGeneration() {
    mDevice->bumpGeneration();
}

void InputMapper::dumpRawAbsoluteAxisInfo(std::string& dump,
        const RawAbsoluteAxisInfo& axis, const char* name) {
    if (axis.valid) {
        dump += StringPrintf(INDENT4 "%s: min=%d, max=%d, flat=%d, fuzz=%d, resolution=%d\n",
                name, axis.minValue, axis.maxValue, axis.flat, axis.fuzz, axis.resolution);
    } else {
        dump += StringPrintf(INDENT4 "%s: unknown range\n", name);
    }
}

void InputMapper::dumpStylusState(std::string& dump, const StylusState& state) {
    dump += StringPrintf(INDENT4 "When: %" PRId64 "\n", state.when);
    dump += StringPrintf(INDENT4 "Pressure: %f\n", state.pressure);
    dump += StringPrintf(INDENT4 "Button State: 0x%08x\n", state.buttons);
    dump += StringPrintf(INDENT4 "Tool Type: %" PRId32 "\n", state.toolType);
}

// --- SwitchInputMapper ---

SwitchInputMapper::SwitchInputMapper(InputDevice* device) :
        InputMapper(device), mSwitchValues(0), mUpdatedSwitchMask(0) {
}

SwitchInputMapper::~SwitchInputMapper() {
}

uint32_t SwitchInputMapper::getSources() {
    return AINPUT_SOURCE_SWITCH;
}

void SwitchInputMapper::process(const RawEvent* rawEvent) {
    switch (rawEvent->type) {
    case EV_SW:
        processSwitch(rawEvent->code, rawEvent->value);
        break;

    case EV_SYN:
        if (rawEvent->code == SYN_REPORT) {
            sync(rawEvent->when);
        }
    }
}

void SwitchInputMapper::processSwitch(int32_t switchCode, int32_t switchValue) {
    if (switchCode >= 0 && switchCode < 32) {
        if (switchValue) {
            mSwitchValues |= 1 << switchCode;
        } else {
            mSwitchValues &= ~(1 << switchCode);
        }
        mUpdatedSwitchMask |= 1 << switchCode;
    }
}

void SwitchInputMapper::sync(nsecs_t when) {
    if (mUpdatedSwitchMask) {
        uint32_t updatedSwitchValues = mSwitchValues & mUpdatedSwitchMask;
        NotifySwitchArgs args(mContext->getNextSequenceNum(), when, 0, updatedSwitchValues,
                mUpdatedSwitchMask);
        getListener()->notifySwitch(&args);

        mUpdatedSwitchMask = 0;
    }
}

int32_t SwitchInputMapper::getSwitchState(uint32_t sourceMask, int32_t switchCode) {
    return getEventHub()->getSwitchState(getDeviceId(), switchCode);
}

void SwitchInputMapper::dump(std::string& dump) {
    dump += INDENT2 "Switch Input Mapper:\n";
    dump += StringPrintf(INDENT3 "SwitchValues: %x\n", mSwitchValues);
}

// --- VibratorInputMapper ---

VibratorInputMapper::VibratorInputMapper(InputDevice* device) :
        InputMapper(device), mVibrating(false) {
}

VibratorInputMapper::~VibratorInputMapper() {
}

uint32_t VibratorInputMapper::getSources() {
    return 0;
}

void VibratorInputMapper::populateDeviceInfo(InputDeviceInfo* info) {
    InputMapper::populateDeviceInfo(info);

    info->setVibrator(true);
}

void VibratorInputMapper::process(const RawEvent* rawEvent) {
    // TODO: Handle FF_STATUS, although it does not seem to be widely supported.
}

void VibratorInputMapper::vibrate(const nsecs_t* pattern, size_t patternSize, ssize_t repeat,
        int32_t token) {
#if DEBUG_VIBRATOR
    std::string patternStr;
    for (size_t i = 0; i < patternSize; i++) {
        if (i != 0) {
            patternStr += ", ";
        }
        patternStr += StringPrintf("%" PRId64, pattern[i]);
    }
    ALOGD("vibrate: deviceId=%d, pattern=[%s], repeat=%zd, token=%d",
            getDeviceId(), patternStr.c_str(), repeat, token);
#endif

    mVibrating = true;
    memcpy(mPattern, pattern, patternSize * sizeof(nsecs_t));
    mPatternSize = patternSize;
    mRepeat = repeat;
    mToken = token;
    mIndex = -1;

    nextStep();
}

void VibratorInputMapper::cancelVibrate(int32_t token) {
#if DEBUG_VIBRATOR
    ALOGD("cancelVibrate: deviceId=%d, token=%d", getDeviceId(), token);
#endif

    if (mVibrating && mToken == token) {
        stopVibrating();
    }
}

void VibratorInputMapper::timeoutExpired(nsecs_t when) {
    if (mVibrating) {
        if (when >= mNextStepTime) {
            nextStep();
        } else {
            getContext()->requestTimeoutAtTime(mNextStepTime);
        }
    }
}

void VibratorInputMapper::nextStep() {
    mIndex += 1;
    if (size_t(mIndex) >= mPatternSize) {
        if (mRepeat < 0) {
            // We are done.
            stopVibrating();
            return;
        }
        mIndex = mRepeat;
    }

    bool vibratorOn = mIndex & 1;
    nsecs_t duration = mPattern[mIndex];
    if (vibratorOn) {
#if DEBUG_VIBRATOR
        ALOGD("nextStep: sending vibrate deviceId=%d, duration=%" PRId64, getDeviceId(), duration);
#endif
        getEventHub()->vibrate(getDeviceId(), duration);
    } else {
#if DEBUG_VIBRATOR
        ALOGD("nextStep: sending cancel vibrate deviceId=%d", getDeviceId());
#endif
        getEventHub()->cancelVibrate(getDeviceId());
    }
    nsecs_t now = systemTime(SYSTEM_TIME_MONOTONIC);
    mNextStepTime = now + duration;
    getContext()->requestTimeoutAtTime(mNextStepTime);
#if DEBUG_VIBRATOR
    ALOGD("nextStep: scheduled timeout in %0.3fms", duration * 0.000001f);
#endif
}

void VibratorInputMapper::stopVibrating() {
    mVibrating = false;
#if DEBUG_VIBRATOR
    ALOGD("stopVibrating: sending cancel vibrate deviceId=%d", getDeviceId());
#endif
    getEventHub()->cancelVibrate(getDeviceId());
}

void VibratorInputMapper::dump(std::string& dump) {
    dump += INDENT2 "Vibrator Input Mapper:\n";
    dump += StringPrintf(INDENT3 "Vibrating: %s\n", toString(mVibrating));
}


// --- KeyboardInputMapper ---

KeyboardInputMapper::KeyboardInputMapper(InputDevice* device,
        uint32_t source, int32_t keyboardType) :
        InputMapper(device), mSource(source), mKeyboardType(keyboardType) {
}

KeyboardInputMapper::~KeyboardInputMapper() {
}

uint32_t KeyboardInputMapper::getSources() {
    return mSource;
}

int32_t KeyboardInputMapper::getOrientation() {
    if (mViewport) {
        return mViewport->orientation;
    }
    return DISPLAY_ORIENTATION_0;
}

int32_t KeyboardInputMapper::getDisplayId() {
    if (mViewport) {
        return mViewport->displayId;
    }
    return ADISPLAY_ID_NONE;
}

void KeyboardInputMapper::populateDeviceInfo(InputDeviceInfo* info) {
    InputMapper::populateDeviceInfo(info);

    info->setKeyboardType(mKeyboardType);
    info->setKeyCharacterMap(getEventHub()->getKeyCharacterMap(getDeviceId()));
}

void KeyboardInputMapper::dump(std::string& dump) {
    dump += INDENT2 "Keyboard Input Mapper:\n";
    dumpParameters(dump);
    dump += StringPrintf(INDENT3 "KeyboardType: %d\n", mKeyboardType);
    dump += StringPrintf(INDENT3 "Orientation: %d\n", getOrientation());
    dump += StringPrintf(INDENT3 "KeyDowns: %zu keys currently down\n", mKeyDowns.size());
    dump += StringPrintf(INDENT3 "MetaState: 0x%0x\n", mMetaState);
    dump += StringPrintf(INDENT3 "DownTime: %" PRId64 "\n", mDownTime);
}

void KeyboardInputMapper::configure(nsecs_t when,
        const InputReaderConfiguration* config, uint32_t changes) {
    InputMapper::configure(when, config, changes);

    if (!changes) { // first time only
        // Configure basic parameters.
        configureParameters();
    }

    if (!changes || (changes & InputReaderConfiguration::CHANGE_DISPLAY_INFO)) {
        if (mParameters.orientationAware) {
            mViewport = config->getDisplayViewportByType(ViewportType::VIEWPORT_INTERNAL);
        }
    }
}

static void mapStemKey(int32_t keyCode, const PropertyMap& config, char const *property) {
    int32_t mapped = 0;
    if (config.tryGetProperty(String8(property), mapped) && mapped > 0) {
        for (size_t i = 0; i < stemKeyRotationMapSize; i++) {
            if (stemKeyRotationMap[i][0] == keyCode) {
                stemKeyRotationMap[i][1] = mapped;
                return;
            }
        }
    }
}

void KeyboardInputMapper::configureParameters() {
    mParameters.orientationAware = false;
    const PropertyMap& config = getDevice()->getConfiguration();
    config.tryGetProperty(String8("keyboard.orientationAware"),
            mParameters.orientationAware);

    if (mParameters.orientationAware) {
        mapStemKey(AKEYCODE_STEM_PRIMARY, config, "keyboard.rotated.stem_primary");
        mapStemKey(AKEYCODE_STEM_1, config, "keyboard.rotated.stem_1");
        mapStemKey(AKEYCODE_STEM_2, config, "keyboard.rotated.stem_2");
        mapStemKey(AKEYCODE_STEM_3, config, "keyboard.rotated.stem_3");
    }

    mParameters.handlesKeyRepeat = false;
    config.tryGetProperty(String8("keyboard.handlesKeyRepeat"),
            mParameters.handlesKeyRepeat);
}

void KeyboardInputMapper::dumpParameters(std::string& dump) {
    dump += INDENT3 "Parameters:\n";
    dump += StringPrintf(INDENT4 "OrientationAware: %s\n",
            toString(mParameters.orientationAware));
    dump += StringPrintf(INDENT4 "HandlesKeyRepeat: %s\n",
            toString(mParameters.handlesKeyRepeat));
}

void KeyboardInputMapper::reset(nsecs_t when) {
    mMetaState = AMETA_NONE;
    mDownTime = 0;
    mKeyDowns.clear();
    mCurrentHidUsage = 0;

    resetLedState();

    InputMapper::reset(when);
}

void KeyboardInputMapper::process(const RawEvent* rawEvent) {
    switch (rawEvent->type) {
    case EV_KEY: {
        int32_t scanCode = rawEvent->code;
        int32_t usageCode = mCurrentHidUsage;
        mCurrentHidUsage = 0;

        if (isKeyboardOrGamepadKey(scanCode)) {
            processKey(rawEvent->when, rawEvent->value != 0, scanCode, usageCode);
        }
        break;
    }
    case EV_MSC: {
        if (rawEvent->code == MSC_SCAN) {
            mCurrentHidUsage = rawEvent->value;
        }
        break;
    }
    case EV_SYN: {
        if (rawEvent->code == SYN_REPORT) {
            mCurrentHidUsage = 0;
        }
    }
    }
}

bool KeyboardInputMapper::isKeyboardOrGamepadKey(int32_t scanCode) {
    return scanCode < BTN_MOUSE
        || scanCode >= KEY_OK
        || (scanCode >= BTN_MISC && scanCode < BTN_MOUSE)
        || (scanCode >= BTN_JOYSTICK && scanCode < BTN_DIGI);
}

bool KeyboardInputMapper::isMediaKey(int32_t keyCode) {
    switch (keyCode) {
    case AKEYCODE_MEDIA_PLAY:
    case AKEYCODE_MEDIA_PAUSE:
    case AKEYCODE_MEDIA_PLAY_PAUSE:
    case AKEYCODE_MUTE:
    case AKEYCODE_HEADSETHOOK:
    case AKEYCODE_MEDIA_STOP:
    case AKEYCODE_MEDIA_NEXT:
    case AKEYCODE_MEDIA_PREVIOUS:
    case AKEYCODE_MEDIA_REWIND:
    case AKEYCODE_MEDIA_RECORD:
    case AKEYCODE_MEDIA_FAST_FORWARD:
    case AKEYCODE_MEDIA_SKIP_FORWARD:
    case AKEYCODE_MEDIA_SKIP_BACKWARD:
    case AKEYCODE_MEDIA_STEP_FORWARD:
    case AKEYCODE_MEDIA_STEP_BACKWARD:
    case AKEYCODE_MEDIA_AUDIO_TRACK:
    case AKEYCODE_VOLUME_UP:
    case AKEYCODE_VOLUME_DOWN:
    case AKEYCODE_VOLUME_MUTE:
    case AKEYCODE_TV_AUDIO_DESCRIPTION:
    case AKEYCODE_TV_AUDIO_DESCRIPTION_MIX_UP:
    case AKEYCODE_TV_AUDIO_DESCRIPTION_MIX_DOWN:
        return true;
    }
    return false;
}

void KeyboardInputMapper::processKey(nsecs_t when, bool down, int32_t scanCode,
        int32_t usageCode) {
    int32_t keyCode;
    int32_t keyMetaState;
    uint32_t policyFlags;

    if (getEventHub()->mapKey(getDeviceId(), scanCode, usageCode, mMetaState,
                              &keyCode, &keyMetaState, &policyFlags)) {
        keyCode = AKEYCODE_UNKNOWN;
        keyMetaState = mMetaState;
        policyFlags = 0;
    }

    if (down) {
        // Rotate key codes according to orientation if needed.
        if (mParameters.orientationAware) {
            keyCode = rotateKeyCode(keyCode, getOrientation());
        }

        // Add key down.
        ssize_t keyDownIndex = findKeyDown(scanCode);
        if (keyDownIndex >= 0) {
            // key repeat, be sure to use same keycode as before in case of rotation
            keyCode = mKeyDowns[keyDownIndex].keyCode;
        } else {
            // key down
            if ((policyFlags & POLICY_FLAG_VIRTUAL)
                    && mContext->shouldDropVirtualKey(when,
                            getDevice(), keyCode, scanCode)) {
                return;
            }
            if (policyFlags & POLICY_FLAG_GESTURE) {
                mDevice->cancelTouch(when);
            }

            KeyDown keyDown;
            keyDown.keyCode = keyCode;
            keyDown.scanCode = scanCode;
            mKeyDowns.push_back(keyDown);
        }

        mDownTime = when;
    } else {
        // Remove key down.
        ssize_t keyDownIndex = findKeyDown(scanCode);
        if (keyDownIndex >= 0) {
            // key up, be sure to use same keycode as before in case of rotation
            keyCode = mKeyDowns[keyDownIndex].keyCode;
            mKeyDowns.erase(mKeyDowns.begin() + (size_t)keyDownIndex);
        } else {
            // key was not actually down
            ALOGI("Dropping key up from device %s because the key was not down.  "
                    "keyCode=%d, scanCode=%d",
                    getDeviceName().c_str(), keyCode, scanCode);
            return;
        }
    }

    if (updateMetaStateIfNeeded(keyCode, down)) {
        // If global meta state changed send it along with the key.
        // If it has not changed then we'll use what keymap gave us,
        // since key replacement logic might temporarily reset a few
        // meta bits for given key.
        keyMetaState = mMetaState;
    }

    nsecs_t downTime = mDownTime;

    // Key down on external an keyboard should wake the device.
    // We don't do this for internal keyboards to prevent them from waking up in your pocket.
    // For internal keyboards, the key layout file should specify the policy flags for
    // each wake key individually.
    // TODO: Use the input device configuration to control this behavior more finely.
    if (down && getDevice()->isExternal() && !isMediaKey(keyCode)) {
        policyFlags |= POLICY_FLAG_WAKE;
    }

    if (mParameters.handlesKeyRepeat) {
        policyFlags |= POLICY_FLAG_DISABLE_KEY_REPEAT;
    }

    NotifyKeyArgs args(mContext->getNextSequenceNum(), when, getDeviceId(), mSource,
            getDisplayId(), policyFlags, down ? AKEY_EVENT_ACTION_DOWN : AKEY_EVENT_ACTION_UP,
            AKEY_EVENT_FLAG_FROM_SYSTEM, keyCode, scanCode, keyMetaState, downTime);
    getListener()->notifyKey(&args);
}

ssize_t KeyboardInputMapper::findKeyDown(int32_t scanCode) {
    size_t n = mKeyDowns.size();
    for (size_t i = 0; i < n; i++) {
        if (mKeyDowns[i].scanCode == scanCode) {
            return i;
        }
    }
    return -1;
}

int32_t KeyboardInputMapper::getKeyCodeState(uint32_t sourceMask, int32_t keyCode) {
    return getEventHub()->getKeyCodeState(getDeviceId(), keyCode);
}

int32_t KeyboardInputMapper::getScanCodeState(uint32_t sourceMask, int32_t scanCode) {
    return getEventHub()->getScanCodeState(getDeviceId(), scanCode);
}

bool KeyboardInputMapper::markSupportedKeyCodes(uint32_t sourceMask, size_t numCodes,
        const int32_t* keyCodes, uint8_t* outFlags) {
    return getEventHub()->markSupportedKeyCodes(getDeviceId(), numCodes, keyCodes, outFlags);
}

int32_t KeyboardInputMapper::getMetaState() {
    return mMetaState;
}

void KeyboardInputMapper::updateMetaState(int32_t keyCode) {
    updateMetaStateIfNeeded(keyCode, false);
}

bool KeyboardInputMapper::updateMetaStateIfNeeded(int32_t keyCode, bool down) {
    int32_t oldMetaState = mMetaState;
    int32_t newMetaState = android::updateMetaState(keyCode, down, oldMetaState);
    bool metaStateChanged = oldMetaState != newMetaState;
    if (metaStateChanged) {
        mMetaState = newMetaState;
        updateLedState(false);

        getContext()->updateGlobalMetaState();
    }

    return metaStateChanged;
}

void KeyboardInputMapper::resetLedState() {
    initializeLedState(mCapsLockLedState, ALED_CAPS_LOCK);
    initializeLedState(mNumLockLedState, ALED_NUM_LOCK);
    initializeLedState(mScrollLockLedState, ALED_SCROLL_LOCK);

    updateLedState(true);
}

void KeyboardInputMapper::initializeLedState(LedState& ledState, int32_t led) {
    ledState.avail = getEventHub()->hasLed(getDeviceId(), led);
    ledState.on = false;
}

void KeyboardInputMapper::updateLedState(bool reset) {
    updateLedStateForModifier(mCapsLockLedState, ALED_CAPS_LOCK,
            AMETA_CAPS_LOCK_ON, reset);
    updateLedStateForModifier(mNumLockLedState, ALED_NUM_LOCK,
            AMETA_NUM_LOCK_ON, reset);
    updateLedStateForModifier(mScrollLockLedState, ALED_SCROLL_LOCK,
            AMETA_SCROLL_LOCK_ON, reset);
}

void KeyboardInputMapper::updateLedStateForModifier(LedState& ledState,
        int32_t led, int32_t modifier, bool reset) {
    if (ledState.avail) {
        bool desiredState = (mMetaState & modifier) != 0;
        if (reset || ledState.on != desiredState) {
            getEventHub()->setLedState(getDeviceId(), led, desiredState);
            ledState.on = desiredState;
        }
    }
}


// --- CursorInputMapper ---

CursorInputMapper::CursorInputMapper(InputDevice* device) :
        InputMapper(device) {
}

CursorInputMapper::~CursorInputMapper() {
}

uint32_t CursorInputMapper::getSources() {
    return mSource;
}

void CursorInputMapper::populateDeviceInfo(InputDeviceInfo* info) {
    InputMapper::populateDeviceInfo(info);

    if (mParameters.mode == Parameters::MODE_POINTER) {
        float minX, minY, maxX, maxY;
        if (mPointerController->getBounds(&minX, &minY, &maxX, &maxY)) {
            info->addMotionRange(AMOTION_EVENT_AXIS_X, mSource, minX, maxX, 0.0f, 0.0f, 0.0f);
            info->addMotionRange(AMOTION_EVENT_AXIS_Y, mSource, minY, maxY, 0.0f, 0.0f, 0.0f);
        }
    } else {
        info->addMotionRange(AMOTION_EVENT_AXIS_X, mSource, -1.0f, 1.0f, 0.0f, mXScale, 0.0f);
        info->addMotionRange(AMOTION_EVENT_AXIS_Y, mSource, -1.0f, 1.0f, 0.0f, mYScale, 0.0f);
    }
    info->addMotionRange(AMOTION_EVENT_AXIS_PRESSURE, mSource, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);

    if (mCursorScrollAccumulator.haveRelativeVWheel()) {
        info->addMotionRange(AMOTION_EVENT_AXIS_VSCROLL, mSource, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f);
    }
    if (mCursorScrollAccumulator.haveRelativeHWheel()) {
        info->addMotionRange(AMOTION_EVENT_AXIS_HSCROLL, mSource, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f);
    }
}

void CursorInputMapper::dump(std::string& dump) {
    dump += INDENT2 "Cursor Input Mapper:\n";
    dumpParameters(dump);
    dump += StringPrintf(INDENT3 "XScale: %0.3f\n", mXScale);
    dump += StringPrintf(INDENT3 "YScale: %0.3f\n", mYScale);
    dump += StringPrintf(INDENT3 "XPrecision: %0.3f\n", mXPrecision);
    dump += StringPrintf(INDENT3 "YPrecision: %0.3f\n", mYPrecision);
    dump += StringPrintf(INDENT3 "HaveVWheel: %s\n",
            toString(mCursorScrollAccumulator.haveRelativeVWheel()));
    dump += StringPrintf(INDENT3 "HaveHWheel: %s\n",
            toString(mCursorScrollAccumulator.haveRelativeHWheel()));
    dump += StringPrintf(INDENT3 "VWheelScale: %0.3f\n", mVWheelScale);
    dump += StringPrintf(INDENT3 "HWheelScale: %0.3f\n", mHWheelScale);
    dump += StringPrintf(INDENT3 "Orientation: %d\n", mOrientation);
    dump += StringPrintf(INDENT3 "ButtonState: 0x%08x\n", mButtonState);
    dump += StringPrintf(INDENT3 "Down: %s\n", toString(isPointerDown(mButtonState)));
    dump += StringPrintf(INDENT3 "DownTime: %" PRId64 "\n", mDownTime);
}

void CursorInputMapper::configure(nsecs_t when,
        const InputReaderConfiguration* config, uint32_t changes) {
    InputMapper::configure(when, config, changes);

    if (!changes) { // first time only
        mCursorScrollAccumulator.configure(getDevice());

        // Configure basic parameters.
        configureParameters();

        // Configure device mode.
        switch (mParameters.mode) {
        case Parameters::MODE_POINTER_RELATIVE:
            // Should not happen during first time configuration.
            ALOGE("Cannot start a device in MODE_POINTER_RELATIVE, starting in MODE_POINTER");
            mParameters.mode = Parameters::MODE_POINTER;
            [[fallthrough]];
        case Parameters::MODE_POINTER:
            mSource = AINPUT_SOURCE_MOUSE;
            mXPrecision = 1.0f;
            mYPrecision = 1.0f;
            mXScale = 1.0f;
            mYScale = 1.0f;
            mPointerController = getPolicy()->obtainPointerController(getDeviceId());
            break;
        case Parameters::MODE_NAVIGATION:
            mSource = AINPUT_SOURCE_TRACKBALL;
            mXPrecision = TRACKBALL_MOVEMENT_THRESHOLD;
            mYPrecision = TRACKBALL_MOVEMENT_THRESHOLD;
            mXScale = 1.0f / TRACKBALL_MOVEMENT_THRESHOLD;
            mYScale = 1.0f / TRACKBALL_MOVEMENT_THRESHOLD;
            break;
        }

        mVWheelScale = 1.0f;
        mHWheelScale = 1.0f;
    }

    if ((!changes && config->pointerCapture)
            || (changes & InputReaderConfiguration::CHANGE_POINTER_CAPTURE)) {
        if (config->pointerCapture) {
            if (mParameters.mode == Parameters::MODE_POINTER) {
                mParameters.mode = Parameters::MODE_POINTER_RELATIVE;
                mSource = AINPUT_SOURCE_MOUSE_RELATIVE;
                // Keep PointerController around in order to preserve the pointer position.
                mPointerController->fade(PointerControllerInterface::TRANSITION_IMMEDIATE);
            } else {
                ALOGE("Cannot request pointer capture, device is not in MODE_POINTER");
            }
        } else {
            if (mParameters.mode == Parameters::MODE_POINTER_RELATIVE) {
                mParameters.mode = Parameters::MODE_POINTER;
                mSource = AINPUT_SOURCE_MOUSE;
            } else {
                ALOGE("Cannot release pointer capture, device is not in MODE_POINTER_RELATIVE");
            }
        }
        bumpGeneration();
        if (changes) {
            getDevice()->notifyReset(when);
        }
    }

    if (!changes || (changes & InputReaderConfiguration::CHANGE_POINTER_SPEED)) {
        mPointerVelocityControl.setParameters(config->pointerVelocityControlParameters);
        mWheelXVelocityControl.setParameters(config->wheelVelocityControlParameters);
        mWheelYVelocityControl.setParameters(config->wheelVelocityControlParameters);
    }

    if (!changes || (changes & InputReaderConfiguration::CHANGE_DISPLAY_INFO)) {
        mOrientation = DISPLAY_ORIENTATION_0;
        if (mParameters.orientationAware && mParameters.hasAssociatedDisplay) {
            std::optional<DisplayViewport> internalViewport =
                    config->getDisplayViewportByType(ViewportType::VIEWPORT_INTERNAL);
            if (internalViewport) {
                mOrientation = internalViewport->orientation;
            }
        }

        // Update the PointerController if viewports changed.
        if (mParameters.mode == Parameters::MODE_POINTER) {
            getPolicy()->obtainPointerController(getDeviceId());
        }
        bumpGeneration();
    }
}

void CursorInputMapper::configureParameters() {
    mParameters.mode = Parameters::MODE_POINTER;
    String8 cursorModeString;
    if (getDevice()->getConfiguration().tryGetProperty(String8("cursor.mode"), cursorModeString)) {
        if (cursorModeString == "navigation") {
            mParameters.mode = Parameters::MODE_NAVIGATION;
        } else if (cursorModeString != "pointer" && cursorModeString != "default") {
            ALOGW("Invalid value for cursor.mode: '%s'", cursorModeString.string());
        }
    }

    mParameters.orientationAware = false;
    getDevice()->getConfiguration().tryGetProperty(String8("cursor.orientationAware"),
            mParameters.orientationAware);

    mParameters.hasAssociatedDisplay = false;
    if (mParameters.mode == Parameters::MODE_POINTER || mParameters.orientationAware) {
        mParameters.hasAssociatedDisplay = true;
    }
}

void CursorInputMapper::dumpParameters(std::string& dump) {
    dump += INDENT3 "Parameters:\n";
    dump += StringPrintf(INDENT4 "HasAssociatedDisplay: %s\n",
            toString(mParameters.hasAssociatedDisplay));

    switch (mParameters.mode) {
    case Parameters::MODE_POINTER:
        dump += INDENT4 "Mode: pointer\n";
        break;
    case Parameters::MODE_POINTER_RELATIVE:
        dump += INDENT4 "Mode: relative pointer\n";
        break;
    case Parameters::MODE_NAVIGATION:
        dump += INDENT4 "Mode: navigation\n";
        break;
    default:
        ALOG_ASSERT(false);
    }

    dump += StringPrintf(INDENT4 "OrientationAware: %s\n",
            toString(mParameters.orientationAware));
}

void CursorInputMapper::reset(nsecs_t when) {
    mButtonState = 0;
    mDownTime = 0;

    mPointerVelocityControl.reset();
    mWheelXVelocityControl.reset();
    mWheelYVelocityControl.reset();

    mCursorButtonAccumulator.reset(getDevice());
    mCursorMotionAccumulator.reset(getDevice());
    mCursorScrollAccumulator.reset(getDevice());

    InputMapper::reset(when);
}

void CursorInputMapper::process(const RawEvent* rawEvent) {
    mCursorButtonAccumulator.process(rawEvent);
    mCursorMotionAccumulator.process(rawEvent);
    mCursorScrollAccumulator.process(rawEvent);

    if (rawEvent->type == EV_SYN && rawEvent->code == SYN_REPORT) {
        sync(rawEvent->when);
    }
}

void CursorInputMapper::sync(nsecs_t when) {
    int32_t lastButtonState = mButtonState;
    int32_t currentButtonState = mCursorButtonAccumulator.getButtonState();
    mButtonState = currentButtonState;

    bool wasDown = isPointerDown(lastButtonState);
    bool down = isPointerDown(currentButtonState);
    bool downChanged;
    if (!wasDown && down) {
        mDownTime = when;
        downChanged = true;
    } else if (wasDown && !down) {
        downChanged = true;
    } else {
        downChanged = false;
    }
    nsecs_t downTime = mDownTime;
    bool buttonsChanged = currentButtonState != lastButtonState;
    int32_t buttonsPressed = currentButtonState & ~lastButtonState;
    int32_t buttonsReleased = lastButtonState & ~currentButtonState;

    float deltaX = mCursorMotionAccumulator.getRelativeX() * mXScale;
    float deltaY = mCursorMotionAccumulator.getRelativeY() * mYScale;
    bool moved = deltaX != 0 || deltaY != 0;

    // Rotate delta according to orientation if needed.
    if (mParameters.orientationAware && mParameters.hasAssociatedDisplay
            && (deltaX != 0.0f || deltaY != 0.0f)) {
        rotateDelta(mOrientation, &deltaX, &deltaY);
    }

    // Move the pointer.
    PointerProperties pointerProperties;
    pointerProperties.clear();
    pointerProperties.id = 0;
    pointerProperties.toolType = AMOTION_EVENT_TOOL_TYPE_MOUSE;

    PointerCoords pointerCoords;
    pointerCoords.clear();

    float vscroll = mCursorScrollAccumulator.getRelativeVWheel();
    float hscroll = mCursorScrollAccumulator.getRelativeHWheel();
    bool scrolled = vscroll != 0 || hscroll != 0;

    mWheelYVelocityControl.move(when, nullptr, &vscroll);
    mWheelXVelocityControl.move(when, &hscroll, nullptr);

    mPointerVelocityControl.move(when, &deltaX, &deltaY);

    int32_t displayId;
    if (mSource == AINPUT_SOURCE_MOUSE) {
        if (moved || scrolled || buttonsChanged) {
            mPointerController->setPresentation(
                    PointerControllerInterface::PRESENTATION_POINTER);

            if (moved) {
                mPointerController->move(deltaX, deltaY);
            }

            if (buttonsChanged) {
                mPointerController->setButtonState(currentButtonState);
            }

            mPointerController->unfade(PointerControllerInterface::TRANSITION_IMMEDIATE);
        }

        float x, y;
        mPointerController->getPosition(&x, &y);
        pointerCoords.setAxisValue(AMOTION_EVENT_AXIS_X, x);
        pointerCoords.setAxisValue(AMOTION_EVENT_AXIS_Y, y);
        pointerCoords.setAxisValue(AMOTION_EVENT_AXIS_RELATIVE_X, deltaX);
        pointerCoords.setAxisValue(AMOTION_EVENT_AXIS_RELATIVE_Y, deltaY);
        displayId = mPointerController->getDisplayId();
    } else {
        pointerCoords.setAxisValue(AMOTION_EVENT_AXIS_X, deltaX);
        pointerCoords.setAxisValue(AMOTION_EVENT_AXIS_Y, deltaY);
        displayId = ADISPLAY_ID_NONE;
    }

    pointerCoords.setAxisValue(AMOTION_EVENT_AXIS_PRESSURE, down ? 1.0f : 0.0f);

    // Moving an external trackball or mouse should wake the device.
    // We don't do this for internal cursor devices to prevent them from waking up
    // the device in your pocket.
    // TODO: Use the input device configuration to control this behavior more finely.
    uint32_t policyFlags = 0;
    if ((buttonsPressed || moved || scrolled) && getDevice()->isExternal()) {
        policyFlags |= POLICY_FLAG_WAKE;
    }

    // Synthesize key down from buttons if needed.
    synthesizeButtonKeys(getContext(), AKEY_EVENT_ACTION_DOWN, when, getDeviceId(), mSource,
            displayId, policyFlags, lastButtonState, currentButtonState);

    // Send motion event.
    if (downChanged || moved || scrolled || buttonsChanged) {
        int32_t metaState = mContext->getGlobalMetaState();
        int32_t buttonState = lastButtonState;
        int32_t motionEventAction;
        if (downChanged) {
            motionEventAction = down ? AMOTION_EVENT_ACTION_DOWN : AMOTION_EVENT_ACTION_UP;
        } else if (down || (mSource != AINPUT_SOURCE_MOUSE)) {
            motionEventAction = AMOTION_EVENT_ACTION_MOVE;
        } else {
            motionEventAction = AMOTION_EVENT_ACTION_HOVER_MOVE;
        }

        if (buttonsReleased) {
            BitSet32 released(buttonsReleased);
            while (!released.isEmpty()) {
                int32_t actionButton = BitSet32::valueForBit(released.clearFirstMarkedBit());
                buttonState &= ~actionButton;
                NotifyMotionArgs releaseArgs(mContext->getNextSequenceNum(), when, getDeviceId(),
                        mSource, displayId, policyFlags,
                        AMOTION_EVENT_ACTION_BUTTON_RELEASE, actionButton, 0,
                        metaState, buttonState,
                        MotionClassification::NONE, AMOTION_EVENT_EDGE_FLAG_NONE,
                        /* deviceTimestamp */ 0, 1, &pointerProperties, &pointerCoords,
                        mXPrecision, mYPrecision, downTime, /* videoFrames */ {});
                getListener()->notifyMotion(&releaseArgs);
            }
        }

        NotifyMotionArgs args(mContext->getNextSequenceNum(), when, getDeviceId(), mSource,
                displayId, policyFlags, motionEventAction, 0, 0, metaState, currentButtonState,
                MotionClassification::NONE, AMOTION_EVENT_EDGE_FLAG_NONE,
                /* deviceTimestamp */ 0, 1, &pointerProperties, &pointerCoords,
                mXPrecision, mYPrecision, downTime, /* videoFrames */ {});
        getListener()->notifyMotion(&args);

        if (buttonsPressed) {
            BitSet32 pressed(buttonsPressed);
            while (!pressed.isEmpty()) {
                int32_t actionButton = BitSet32::valueForBit(pressed.clearFirstMarkedBit());
                buttonState |= actionButton;
                NotifyMotionArgs pressArgs(mContext->getNextSequenceNum(), when, getDeviceId(),
                        mSource, displayId, policyFlags, AMOTION_EVENT_ACTION_BUTTON_PRESS,
                        actionButton, 0, metaState, buttonState,
                        MotionClassification::NONE, AMOTION_EVENT_EDGE_FLAG_NONE,
                        /* deviceTimestamp */ 0, 1, &pointerProperties, &pointerCoords,
                        mXPrecision, mYPrecision, downTime, /* videoFrames */ {});
                getListener()->notifyMotion(&pressArgs);
            }
        }

        ALOG_ASSERT(buttonState == currentButtonState);

        // Send hover move after UP to tell the application that the mouse is hovering now.
        if (motionEventAction == AMOTION_EVENT_ACTION_UP
                && (mSource == AINPUT_SOURCE_MOUSE)) {
            NotifyMotionArgs hoverArgs(mContext->getNextSequenceNum(), when, getDeviceId(),
                    mSource, displayId, policyFlags, AMOTION_EVENT_ACTION_HOVER_MOVE, 0, 0,
                    metaState, currentButtonState,
                    MotionClassification::NONE, AMOTION_EVENT_EDGE_FLAG_NONE,
                    /* deviceTimestamp */ 0, 1, &pointerProperties, &pointerCoords,
                    mXPrecision, mYPrecision, downTime, /* videoFrames */ {});
            getListener()->notifyMotion(&hoverArgs);
        }

        // Send scroll events.
        if (scrolled) {
            pointerCoords.setAxisValue(AMOTION_EVENT_AXIS_VSCROLL, vscroll);
            pointerCoords.setAxisValue(AMOTION_EVENT_AXIS_HSCROLL, hscroll);

            NotifyMotionArgs scrollArgs(mContext->getNextSequenceNum(), when, getDeviceId(),
                    mSource, displayId, policyFlags,
                    AMOTION_EVENT_ACTION_SCROLL, 0, 0, metaState, currentButtonState,
                    MotionClassification::NONE, AMOTION_EVENT_EDGE_FLAG_NONE,
                    /* deviceTimestamp */ 0, 1, &pointerProperties, &pointerCoords,
                    mXPrecision, mYPrecision, downTime, /* videoFrames */ {});
            getListener()->notifyMotion(&scrollArgs);
        }
    }

    // Synthesize key up from buttons if needed.
    synthesizeButtonKeys(getContext(), AKEY_EVENT_ACTION_UP, when, getDeviceId(), mSource,
            displayId, policyFlags, lastButtonState, currentButtonState);

    mCursorMotionAccumulator.finishSync();
    mCursorScrollAccumulator.finishSync();
}

int32_t CursorInputMapper::getScanCodeState(uint32_t sourceMask, int32_t scanCode) {
    if (scanCode >= BTN_MOUSE && scanCode < BTN_JOYSTICK) {
        return getEventHub()->getScanCodeState(getDeviceId(), scanCode);
    } else {
        return AKEY_STATE_UNKNOWN;
    }
}

void CursorInputMapper::fadePointer() {
    if (mPointerController != nullptr) {
        mPointerController->fade(PointerControllerInterface::TRANSITION_GRADUAL);
    }
}

std::optional<int32_t> CursorInputMapper::getAssociatedDisplay() {
    if (mParameters.hasAssociatedDisplay) {
        if (mParameters.mode == Parameters::MODE_POINTER) {
            return std::make_optional(mPointerController->getDisplayId());
        } else {
            // If the device is orientationAware and not a mouse,
            // it expects to dispatch events to any display
            return std::make_optional(ADISPLAY_ID_NONE);
        }
    }
    return std::nullopt;
}

// --- RotaryEncoderInputMapper ---

RotaryEncoderInputMapper::RotaryEncoderInputMapper(InputDevice* device) :
        InputMapper(device), mOrientation(DISPLAY_ORIENTATION_0) {
    mSource = AINPUT_SOURCE_ROTARY_ENCODER;
}

RotaryEncoderInputMapper::~RotaryEncoderInputMapper() {
}

uint32_t RotaryEncoderInputMapper::getSources() {
    return mSource;
}

void RotaryEncoderInputMapper::populateDeviceInfo(InputDeviceInfo* info) {
    InputMapper::populateDeviceInfo(info);

    if (mRotaryEncoderScrollAccumulator.haveRelativeVWheel()) {
        float res = 0.0f;
        if (!mDevice->getConfiguration().tryGetProperty(String8("device.res"), res)) {
            ALOGW("Rotary Encoder device configuration file didn't specify resolution!\n");
        }
        if (!mDevice->getConfiguration().tryGetProperty(String8("device.scalingFactor"),
            mScalingFactor)) {
            ALOGW("Rotary Encoder device configuration file didn't specify scaling factor,"
              "default to 1.0!\n");
            mScalingFactor = 1.0f;
        }
        info->addMotionRange(AMOTION_EVENT_AXIS_SCROLL, mSource, -1.0f, 1.0f, 0.0f, 0.0f,
            res * mScalingFactor);
    }
}

void RotaryEncoderInputMapper::dump(std::string& dump) {
    dump += INDENT2 "Rotary Encoder Input Mapper:\n";
    dump += StringPrintf(INDENT3 "HaveWheel: %s\n",
            toString(mRotaryEncoderScrollAccumulator.haveRelativeVWheel()));
}

void RotaryEncoderInputMapper::configure(nsecs_t when,
        const InputReaderConfiguration* config, uint32_t changes) {
    InputMapper::configure(when, config, changes);
    if (!changes) {
        mRotaryEncoderScrollAccumulator.configure(getDevice());
    }
    if (!changes || (changes & InputReaderConfiguration::CHANGE_DISPLAY_INFO)) {
        std::optional<DisplayViewport> internalViewport =
                config->getDisplayViewportByType(ViewportType::VIEWPORT_INTERNAL);
        if (internalViewport) {
            mOrientation = internalViewport->orientation;
        } else {
            mOrientation = DISPLAY_ORIENTATION_0;
        }
    }
}

void RotaryEncoderInputMapper::reset(nsecs_t when) {
    mRotaryEncoderScrollAccumulator.reset(getDevice());

    InputMapper::reset(when);
}

void RotaryEncoderInputMapper::process(const RawEvent* rawEvent) {
    mRotaryEncoderScrollAccumulator.process(rawEvent);

    if (rawEvent->type == EV_SYN && rawEvent->code == SYN_REPORT) {
        sync(rawEvent->when);
    }
}

void RotaryEncoderInputMapper::sync(nsecs_t when) {
    PointerCoords pointerCoords;
    pointerCoords.clear();

    PointerProperties pointerProperties;
    pointerProperties.clear();
    pointerProperties.id = 0;
    pointerProperties.toolType = AMOTION_EVENT_TOOL_TYPE_UNKNOWN;

    float scroll = mRotaryEncoderScrollAccumulator.getRelativeVWheel();
    bool scrolled = scroll != 0;

    // This is not a pointer, so it's not associated with a display.
    int32_t displayId = ADISPLAY_ID_NONE;

    // Moving the rotary encoder should wake the device (if specified).
    uint32_t policyFlags = 0;
    if (scrolled && getDevice()->isExternal()) {
        policyFlags |= POLICY_FLAG_WAKE;
    }

    if (mOrientation == DISPLAY_ORIENTATION_180) {
        scroll = -scroll;
    }

    // Send motion event.
    if (scrolled) {
        int32_t metaState = mContext->getGlobalMetaState();
        pointerCoords.setAxisValue(AMOTION_EVENT_AXIS_SCROLL, scroll * mScalingFactor);

        NotifyMotionArgs scrollArgs(mContext->getNextSequenceNum(), when, getDeviceId(),
                mSource, displayId, policyFlags,
                AMOTION_EVENT_ACTION_SCROLL, 0, 0, metaState, /* buttonState */ 0,
                MotionClassification::NONE, AMOTION_EVENT_EDGE_FLAG_NONE,
                /* deviceTimestamp */ 0, 1, &pointerProperties, &pointerCoords,
                0, 0, 0, /* videoFrames */ {});
        getListener()->notifyMotion(&scrollArgs);
    }

    mRotaryEncoderScrollAccumulator.finishSync();
}

// --- TouchInputMapper ---

TouchInputMapper::TouchInputMapper(InputDevice* device) :
        InputMapper(device),
        mSource(0), mDeviceMode(DEVICE_MODE_DISABLED),
        mSurfaceWidth(-1), mSurfaceHeight(-1), mSurfaceLeft(0), mSurfaceTop(0),
        mPhysicalWidth(-1), mPhysicalHeight(-1), mPhysicalLeft(0), mPhysicalTop(0),
        mSurfaceOrientation(DISPLAY_ORIENTATION_0) {
}

TouchInputMapper::~TouchInputMapper() {
}

uint32_t TouchInputMapper::getSources() {
    return mSource;
}

void TouchInputMapper::populateDeviceInfo(InputDeviceInfo* info) {
    InputMapper::populateDeviceInfo(info);

    if (mDeviceMode != DEVICE_MODE_DISABLED) {
        info->addMotionRange(mOrientedRanges.x);
        info->addMotionRange(mOrientedRanges.y);
        info->addMotionRange(mOrientedRanges.pressure);

        if (mOrientedRanges.haveSize) {
            info->addMotionRange(mOrientedRanges.size);
        }

        if (mOrientedRanges.haveTouchSize) {
            info->addMotionRange(mOrientedRanges.touchMajor);
            info->addMotionRange(mOrientedRanges.touchMinor);
        }

        if (mOrientedRanges.haveToolSize) {
            info->addMotionRange(mOrientedRanges.toolMajor);
            info->addMotionRange(mOrientedRanges.toolMinor);
        }

        if (mOrientedRanges.haveOrientation) {
            info->addMotionRange(mOrientedRanges.orientation);
        }

        if (mOrientedRanges.haveDistance) {
            info->addMotionRange(mOrientedRanges.distance);
        }

        if (mOrientedRanges.haveTilt) {
            info->addMotionRange(mOrientedRanges.tilt);
        }

        if (mCursorScrollAccumulator.haveRelativeVWheel()) {
            info->addMotionRange(AMOTION_EVENT_AXIS_VSCROLL, mSource, -1.0f, 1.0f, 0.0f, 0.0f,
                    0.0f);
        }
        if (mCursorScrollAccumulator.haveRelativeHWheel()) {
            info->addMotionRange(AMOTION_EVENT_AXIS_HSCROLL, mSource, -1.0f, 1.0f, 0.0f, 0.0f,
                    0.0f);
        }
        if (mCalibration.coverageCalibration == Calibration::COVERAGE_CALIBRATION_BOX) {
            const InputDeviceInfo::MotionRange& x = mOrientedRanges.x;
            const InputDeviceInfo::MotionRange& y = mOrientedRanges.y;
            info->addMotionRange(AMOTION_EVENT_AXIS_GENERIC_1, mSource, x.min, x.max, x.flat,
                    x.fuzz, x.resolution);
            info->addMotionRange(AMOTION_EVENT_AXIS_GENERIC_2, mSource, y.min, y.max, y.flat,
                    y.fuzz, y.resolution);
            info->addMotionRange(AMOTION_EVENT_AXIS_GENERIC_3, mSource, x.min, x.max, x.flat,
                    x.fuzz, x.resolution);
            info->addMotionRange(AMOTION_EVENT_AXIS_GENERIC_4, mSource, y.min, y.max, y.flat,
                    y.fuzz, y.resolution);
        }
        info->setButtonUnderPad(mParameters.hasButtonUnderPad);
    }
}

void TouchInputMapper::dump(std::string& dump) {
    dump += StringPrintf(INDENT2 "Touch Input Mapper (mode - %s):\n", modeToString(mDeviceMode));
    dumpParameters(dump);
    dumpVirtualKeys(dump);
    dumpRawPointerAxes(dump);
    dumpCalibration(dump);
    dumpAffineTransformation(dump);
    dumpSurface(dump);

    dump += StringPrintf(INDENT3 "Translation and Scaling Factors:\n");
    dump += StringPrintf(INDENT4 "XTranslate: %0.3f\n", mXTranslate);
    dump += StringPrintf(INDENT4 "YTranslate: %0.3f\n", mYTranslate);
    dump += StringPrintf(INDENT4 "XScale: %0.3f\n", mXScale);
    dump += StringPrintf(INDENT4 "YScale: %0.3f\n", mYScale);
    dump += StringPrintf(INDENT4 "XPrecision: %0.3f\n", mXPrecision);
    dump += StringPrintf(INDENT4 "YPrecision: %0.3f\n", mYPrecision);
    dump += StringPrintf(INDENT4 "GeometricScale: %0.3f\n", mGeometricScale);
    dump += StringPrintf(INDENT4 "PressureScale: %0.3f\n", mPressureScale);
    dump += StringPrintf(INDENT4 "SizeScale: %0.3f\n", mSizeScale);
    dump += StringPrintf(INDENT4 "OrientationScale: %0.3f\n", mOrientationScale);
    dump += StringPrintf(INDENT4 "DistanceScale: %0.3f\n", mDistanceScale);
    dump += StringPrintf(INDENT4 "HaveTilt: %s\n", toString(mHaveTilt));
    dump += StringPrintf(INDENT4 "TiltXCenter: %0.3f\n", mTiltXCenter);
    dump += StringPrintf(INDENT4 "TiltXScale: %0.3f\n", mTiltXScale);
    dump += StringPrintf(INDENT4 "TiltYCenter: %0.3f\n", mTiltYCenter);
    dump += StringPrintf(INDENT4 "TiltYScale: %0.3f\n", mTiltYScale);

    dump += StringPrintf(INDENT3 "Last Raw Button State: 0x%08x\n", mLastRawState.buttonState);
    dump += StringPrintf(INDENT3 "Last Raw Touch: pointerCount=%d\n",
            mLastRawState.rawPointerData.pointerCount);
    for (uint32_t i = 0; i < mLastRawState.rawPointerData.pointerCount; i++) {
        const RawPointerData::Pointer& pointer = mLastRawState.rawPointerData.pointers[i];
        dump += StringPrintf(INDENT4 "[%d]: id=%d, x=%d, y=%d, pressure=%d, "
                "touchMajor=%d, touchMinor=%d, toolMajor=%d, toolMinor=%d, "
                "orientation=%d, tiltX=%d, tiltY=%d, distance=%d, "
                "toolType=%d, isHovering=%s\n", i,
                pointer.id, pointer.x, pointer.y, pointer.pressure,
                pointer.touchMajor, pointer.touchMinor,
                pointer.toolMajor, pointer.toolMinor,
                pointer.orientation, pointer.tiltX, pointer.tiltY, pointer.distance,
                pointer.toolType, toString(pointer.isHovering));
    }

    dump += StringPrintf(INDENT3 "Last Cooked Button State: 0x%08x\n", mLastCookedState.buttonState);
    dump += StringPrintf(INDENT3 "Last Cooked Touch: pointerCount=%d\n",
            mLastCookedState.cookedPointerData.pointerCount);
    for (uint32_t i = 0; i < mLastCookedState.cookedPointerData.pointerCount; i++) {
        const PointerProperties& pointerProperties =
                mLastCookedState.cookedPointerData.pointerProperties[i];
        const PointerCoords& pointerCoords = mLastCookedState.cookedPointerData.pointerCoords[i];
        dump += StringPrintf(INDENT4 "[%d]: id=%d, x=%0.3f, y=%0.3f, pressure=%0.3f, "
                "touchMajor=%0.3f, touchMinor=%0.3f, toolMajor=%0.3f, toolMinor=%0.3f, "
                "orientation=%0.3f, tilt=%0.3f, distance=%0.3f, "
                "toolType=%d, isHovering=%s\n", i,
                pointerProperties.id,
                pointerCoords.getX(),
                pointerCoords.getY(),
                pointerCoords.getAxisValue(AMOTION_EVENT_AXIS_PRESSURE),
                pointerCoords.getAxisValue(AMOTION_EVENT_AXIS_TOUCH_MAJOR),
                pointerCoords.getAxisValue(AMOTION_EVENT_AXIS_TOUCH_MINOR),
                pointerCoords.getAxisValue(AMOTION_EVENT_AXIS_TOOL_MAJOR),
                pointerCoords.getAxisValue(AMOTION_EVENT_AXIS_TOOL_MINOR),
                pointerCoords.getAxisValue(AMOTION_EVENT_AXIS_ORIENTATION),
                pointerCoords.getAxisValue(AMOTION_EVENT_AXIS_TILT),
                pointerCoords.getAxisValue(AMOTION_EVENT_AXIS_DISTANCE),
                pointerProperties.toolType,
                toString(mLastCookedState.cookedPointerData.isHovering(i)));
    }

    dump += INDENT3 "Stylus Fusion:\n";
    dump += StringPrintf(INDENT4 "ExternalStylusConnected: %s\n",
            toString(mExternalStylusConnected));
    dump += StringPrintf(INDENT4 "External Stylus ID: %" PRId64 "\n", mExternalStylusId);
    dump += StringPrintf(INDENT4 "External Stylus Data Timeout: %" PRId64 "\n",
            mExternalStylusFusionTimeout);
    dump += INDENT3 "External Stylus State:\n";
    dumpStylusState(dump, mExternalStylusState);

    if (mDeviceMode == DEVICE_MODE_POINTER) {
        dump += StringPrintf(INDENT3 "Pointer Gesture Detector:\n");
        dump += StringPrintf(INDENT4 "XMovementScale: %0.3f\n",
                mPointerXMovementScale);
        dump += StringPrintf(INDENT4 "YMovementScale: %0.3f\n",
                mPointerYMovementScale);
        dump += StringPrintf(INDENT4 "XZoomScale: %0.3f\n",
                mPointerXZoomScale);
        dump += StringPrintf(INDENT4 "YZoomScale: %0.3f\n",
                mPointerYZoomScale);
        dump += StringPrintf(INDENT4 "MaxSwipeWidth: %f\n",
                mPointerGestureMaxSwipeWidth);
    }
}

const char* TouchInputMapper::modeToString(DeviceMode deviceMode) {
    switch (deviceMode) {
    case DEVICE_MODE_DISABLED:
        return "disabled";
    case DEVICE_MODE_DIRECT:
        return "direct";
    case DEVICE_MODE_UNSCALED:
        return "unscaled";
    case DEVICE_MODE_NAVIGATION:
        return "navigation";
    case DEVICE_MODE_POINTER:
        return "pointer";
    }
    return "unknown";
}

void TouchInputMapper::configure(nsecs_t when,
        const InputReaderConfiguration* config, uint32_t changes) {
    InputMapper::configure(when, config, changes);

    mConfig = *config;

    if (!changes) { // first time only
        // Configure basic parameters.
        configureParameters();

        // Configure common accumulators.
        mCursorScrollAccumulator.configure(getDevice());
        mTouchButtonAccumulator.configure(getDevice());

        // Configure absolute axis information.
        configureRawPointerAxes();

        // Prepare input device calibration.
        parseCalibration();
        resolveCalibration();
    }

    if (!changes || (changes & InputReaderConfiguration::CHANGE_TOUCH_AFFINE_TRANSFORMATION)) {
        // Update location calibration to reflect current settings
        updateAffineTransformation();
    }

    if (!changes || (changes & InputReaderConfiguration::CHANGE_POINTER_SPEED)) {
        // Update pointer speed.
        mPointerVelocityControl.setParameters(mConfig.pointerVelocityControlParameters);
        mWheelXVelocityControl.setParameters(mConfig.wheelVelocityControlParameters);
        mWheelYVelocityControl.setParameters(mConfig.wheelVelocityControlParameters);
    }

    bool resetNeeded = false;
    if (!changes || (changes & (InputReaderConfiguration::CHANGE_DISPLAY_INFO
            | InputReaderConfiguration::CHANGE_POINTER_GESTURE_ENABLEMENT
            | InputReaderConfiguration::CHANGE_SHOW_TOUCHES
            | InputReaderConfiguration::CHANGE_EXTERNAL_STYLUS_PRESENCE))) {
        // Configure device sources, surface dimensions, orientation and
        // scaling factors.
        configureSurface(when, &resetNeeded);
    }

    if (changes && resetNeeded) {
        // Send reset, unless this is the first time the device has been configured,
        // in which case the reader will call reset itself after all mappers are ready.
        getDevice()->notifyReset(when);
    }
}

void TouchInputMapper::resolveExternalStylusPresence() {
    std::vector<InputDeviceInfo> devices;
    mContext->getExternalStylusDevices(devices);
    mExternalStylusConnected = !devices.empty();

    if (!mExternalStylusConnected) {
        resetExternalStylus();
    }
}

void TouchInputMapper::configureParameters() {
    // Use the pointer presentation mode for devices that do not support distinct
    // multitouch.  The spot-based presentation relies on being able to accurately
    // locate two or more fingers on the touch pad.
    mParameters.gestureMode = getEventHub()->hasInputProperty(getDeviceId(), INPUT_PROP_SEMI_MT)
            ? Parameters::GESTURE_MODE_SINGLE_TOUCH : Parameters::GESTURE_MODE_MULTI_TOUCH;

    String8 gestureModeString;
    if (getDevice()->getConfiguration().tryGetProperty(String8("touch.gestureMode"),
            gestureModeString)) {
        if (gestureModeString == "single-touch") {
            mParameters.gestureMode = Parameters::GESTURE_MODE_SINGLE_TOUCH;
        } else if (gestureModeString == "multi-touch") {
            mParameters.gestureMode = Parameters::GESTURE_MODE_MULTI_TOUCH;
        } else if (gestureModeString != "default") {
            ALOGW("Invalid value for touch.gestureMode: '%s'", gestureModeString.string());
        }
    }

    if (getEventHub()->hasInputProperty(getDeviceId(), INPUT_PROP_DIRECT)) {
        // The device is a touch screen.
        mParameters.deviceType = Parameters::DEVICE_TYPE_TOUCH_SCREEN;
    } else if (getEventHub()->hasInputProperty(getDeviceId(), INPUT_PROP_POINTER)) {
        // The device is a pointing device like a track pad.
        mParameters.deviceType = Parameters::DEVICE_TYPE_POINTER;
    } else if (getEventHub()->hasRelativeAxis(getDeviceId(), REL_X)
            || getEventHub()->hasRelativeAxis(getDeviceId(), REL_Y)) {
        // The device is a cursor device with a touch pad attached.
        // By default don't use the touch pad to move the pointer.
        mParameters.deviceType = Parameters::DEVICE_TYPE_TOUCH_PAD;
    } else {
        // The device is a touch pad of unknown purpose.
        mParameters.deviceType = Parameters::DEVICE_TYPE_POINTER;
    }

    mParameters.hasButtonUnderPad=
            getEventHub()->hasInputProperty(getDeviceId(), INPUT_PROP_BUTTONPAD);

    String8 deviceTypeString;
    if (getDevice()->getConfiguration().tryGetProperty(String8("touch.deviceType"),
            deviceTypeString)) {
        if (deviceTypeString == "touchScreen") {
            mParameters.deviceType = Parameters::DEVICE_TYPE_TOUCH_SCREEN;
        } else if (deviceTypeString == "touchPad") {
            mParameters.deviceType = Parameters::DEVICE_TYPE_TOUCH_PAD;
        } else if (deviceTypeString == "touchNavigation") {
            mParameters.deviceType = Parameters::DEVICE_TYPE_TOUCH_NAVIGATION;
        } else if (deviceTypeString == "pointer") {
            mParameters.deviceType = Parameters::DEVICE_TYPE_POINTER;
        } else if (deviceTypeString != "default") {
            ALOGW("Invalid value for touch.deviceType: '%s'", deviceTypeString.string());
        }
    }

    mParameters.orientationAware = mParameters.deviceType == Parameters::DEVICE_TYPE_TOUCH_SCREEN;
    getDevice()->getConfiguration().tryGetProperty(String8("touch.orientationAware"),
            mParameters.orientationAware);

    mParameters.hasAssociatedDisplay = false;
    mParameters.associatedDisplayIsExternal = false;
    if (mParameters.orientationAware
            || mParameters.deviceType == Parameters::DEVICE_TYPE_TOUCH_SCREEN
            || mParameters.deviceType == Parameters::DEVICE_TYPE_POINTER) {
        mParameters.hasAssociatedDisplay = true;
        if (mParameters.deviceType == Parameters::DEVICE_TYPE_TOUCH_SCREEN) {
            mParameters.associatedDisplayIsExternal = getDevice()->isExternal();
            String8 uniqueDisplayId;
            getDevice()->getConfiguration().tryGetProperty(String8("touch.displayId"),
                    uniqueDisplayId);
            mParameters.uniqueDisplayId = uniqueDisplayId.c_str();
        }
    }
    if (getDevice()->getAssociatedDisplayPort()) {
        mParameters.hasAssociatedDisplay = true;
    }

    // Initial downs on external touch devices should wake the device.
    // Normally we don't do this for internal touch screens to prevent them from waking
    // up in your pocket but you can enable it using the input device configuration.
    mParameters.wake = getDevice()->isExternal();
    getDevice()->getConfiguration().tryGetProperty(String8("touch.wake"),
            mParameters.wake);
}

void TouchInputMapper::dumpParameters(std::string& dump) {
    dump += INDENT3 "Parameters:\n";

    switch (mParameters.gestureMode) {
    case Parameters::GESTURE_MODE_SINGLE_TOUCH:
        dump += INDENT4 "GestureMode: single-touch\n";
        break;
    case Parameters::GESTURE_MODE_MULTI_TOUCH:
        dump += INDENT4 "GestureMode: multi-touch\n";
        break;
    default:
        assert(false);
    }

    switch (mParameters.deviceType) {
    case Parameters::DEVICE_TYPE_TOUCH_SCREEN:
        dump += INDENT4 "DeviceType: touchScreen\n";
        break;
    case Parameters::DEVICE_TYPE_TOUCH_PAD:
        dump += INDENT4 "DeviceType: touchPad\n";
        break;
    case Parameters::DEVICE_TYPE_TOUCH_NAVIGATION:
        dump += INDENT4 "DeviceType: touchNavigation\n";
        break;
    case Parameters::DEVICE_TYPE_POINTER:
        dump += INDENT4 "DeviceType: pointer\n";
        break;
    default:
        ALOG_ASSERT(false);
    }

    dump += StringPrintf(
            INDENT4 "AssociatedDisplay: hasAssociatedDisplay=%s, isExternal=%s, displayId='%s'\n",
            toString(mParameters.hasAssociatedDisplay),
            toString(mParameters.associatedDisplayIsExternal),
            mParameters.uniqueDisplayId.c_str());
    dump += StringPrintf(INDENT4 "OrientationAware: %s\n",
            toString(mParameters.orientationAware));
}

void TouchInputMapper::configureRawPointerAxes() {
    mRawPointerAxes.clear();
}

void TouchInputMapper::dumpRawPointerAxes(std::string& dump) {
    dump += INDENT3 "Raw Touch Axes:\n";
    dumpRawAbsoluteAxisInfo(dump, mRawPointerAxes.x, "X");
    dumpRawAbsoluteAxisInfo(dump, mRawPointerAxes.y, "Y");
    dumpRawAbsoluteAxisInfo(dump, mRawPointerAxes.pressure, "Pressure");
    dumpRawAbsoluteAxisInfo(dump, mRawPointerAxes.touchMajor, "TouchMajor");
    dumpRawAbsoluteAxisInfo(dump, mRawPointerAxes.touchMinor, "TouchMinor");
    dumpRawAbsoluteAxisInfo(dump, mRawPointerAxes.toolMajor, "ToolMajor");
    dumpRawAbsoluteAxisInfo(dump, mRawPointerAxes.toolMinor, "ToolMinor");
    dumpRawAbsoluteAxisInfo(dump, mRawPointerAxes.orientation, "Orientation");
    dumpRawAbsoluteAxisInfo(dump, mRawPointerAxes.distance, "Distance");
    dumpRawAbsoluteAxisInfo(dump, mRawPointerAxes.tiltX, "TiltX");
    dumpRawAbsoluteAxisInfo(dump, mRawPointerAxes.tiltY, "TiltY");
    dumpRawAbsoluteAxisInfo(dump, mRawPointerAxes.trackingId, "TrackingId");
    dumpRawAbsoluteAxisInfo(dump, mRawPointerAxes.slot, "Slot");
}

bool TouchInputMapper::hasExternalStylus() const {
    return mExternalStylusConnected;
}

/**
 * Determine which DisplayViewport to use.
 * 1. If display port is specified, return the matching viewport. If matching viewport not
 * found, then return.
 * 2. If a device has associated display, get the matching viewport by either unique id or by
 * the display type (internal or external).
 * 3. Otherwise, use a non-display viewport.
 */
std::optional<DisplayViewport> TouchInputMapper::findViewport() {
    if (mParameters.hasAssociatedDisplay) {
        const std::optional<uint8_t> displayPort = mDevice->getAssociatedDisplayPort();
        if (displayPort) {
            // Find the viewport that contains the same port
            std::optional<DisplayViewport> v = mConfig.getDisplayViewportByPort(*displayPort);
            if (!v) {
                ALOGW("Input device %s should be associated with display on port %" PRIu8 ", "
                        "but the corresponding viewport is not found.",
                        getDeviceName().c_str(), *displayPort);
            }
            return v;
        }

        if (!mParameters.uniqueDisplayId.empty()) {
            return mConfig.getDisplayViewportByUniqueId(mParameters.uniqueDisplayId);
        }

        ViewportType viewportTypeToUse;
        if (mParameters.associatedDisplayIsExternal) {
            viewportTypeToUse = ViewportType::VIEWPORT_EXTERNAL;
        } else {
            viewportTypeToUse = ViewportType::VIEWPORT_INTERNAL;
        }

        std::optional<DisplayViewport> viewport =
                mConfig.getDisplayViewportByType(viewportTypeToUse);
        if (!viewport && viewportTypeToUse == ViewportType::VIEWPORT_EXTERNAL) {
            ALOGW("Input device %s should be associated with external display, "
                    "fallback to internal one for the external viewport is not found.",
                        getDeviceName().c_str());
            viewport = mConfig.getDisplayViewportByType(ViewportType::VIEWPORT_INTERNAL);
        }

        return viewport;
    }

    DisplayViewport newViewport;
    // Raw width and height in the natural orientation.
    int32_t rawWidth = mRawPointerAxes.getRawWidth();
    int32_t rawHeight = mRawPointerAxes.getRawHeight();
    newViewport.setNonDisplayViewport(rawWidth, rawHeight);
    return std::make_optional(newViewport);
}

void TouchInputMapper::configureSurface(nsecs_t when, bool* outResetNeeded) {
    int32_t oldDeviceMode = mDeviceMode;

    resolveExternalStylusPresence();

    // Determine device mode.
    if (mParameters.deviceType == Parameters::DEVICE_TYPE_POINTER
            && mConfig.pointerGesturesEnabled) {
        mSource = AINPUT_SOURCE_MOUSE;
        mDeviceMode = DEVICE_MODE_POINTER;
        if (hasStylus()) {
            mSource |= AINPUT_SOURCE_STYLUS;
        }
    } else if (mParameters.deviceType == Parameters::DEVICE_TYPE_TOUCH_SCREEN
            && mParameters.hasAssociatedDisplay) {
        mSource = AINPUT_SOURCE_TOUCHSCREEN;
        mDeviceMode = DEVICE_MODE_DIRECT;
        if (hasStylus()) {
            mSource |= AINPUT_SOURCE_STYLUS;
        }
        if (hasExternalStylus()) {
            mSource |= AINPUT_SOURCE_BLUETOOTH_STYLUS;
        }
    } else if (mParameters.deviceType == Parameters::DEVICE_TYPE_TOUCH_NAVIGATION) {
        mSource = AINPUT_SOURCE_TOUCH_NAVIGATION;
        mDeviceMode = DEVICE_MODE_NAVIGATION;
    } else {
        mSource = AINPUT_SOURCE_TOUCHPAD;
        mDeviceMode = DEVICE_MODE_UNSCALED;
    }

    // Ensure we have valid X and Y axes.
    if (!mRawPointerAxes.x.valid || !mRawPointerAxes.y.valid) {
        ALOGW("Touch device '%s' did not report support for X or Y axis!  "
                "The device will be inoperable.", getDeviceName().c_str());
        mDeviceMode = DEVICE_MODE_DISABLED;
        return;
    }

    // Get associated display dimensions.
    std::optional<DisplayViewport> newViewport = findViewport();
    if (!newViewport) {
        ALOGI("Touch device '%s' could not query the properties of its associated "
                "display.  The device will be inoperable until the display size "
                "becomes available.",
                getDeviceName().c_str());
        mDeviceMode = DEVICE_MODE_DISABLED;
        return;
    }

    // Raw width and height in the natural orientation.
    int32_t rawWidth = mRawPointerAxes.getRawWidth();
    int32_t rawHeight = mRawPointerAxes.getRawHeight();

    bool viewportChanged = mViewport != *newViewport;
    if (viewportChanged) {
        mViewport = *newViewport;

        if (mDeviceMode == DEVICE_MODE_DIRECT || mDeviceMode == DEVICE_MODE_POINTER) {
            // Convert rotated viewport to natural surface coordinates.
            int32_t naturalLogicalWidth, naturalLogicalHeight;
            int32_t naturalPhysicalWidth, naturalPhysicalHeight;
            int32_t naturalPhysicalLeft, naturalPhysicalTop;
            int32_t naturalDeviceWidth, naturalDeviceHeight;
            switch (mViewport.orientation) {
            case DISPLAY_ORIENTATION_90:
                naturalLogicalWidth = mViewport.logicalBottom - mViewport.logicalTop;
                naturalLogicalHeight = mViewport.logicalRight - mViewport.logicalLeft;
                naturalPhysicalWidth = mViewport.physicalBottom - mViewport.physicalTop;
                naturalPhysicalHeight = mViewport.physicalRight - mViewport.physicalLeft;
                naturalPhysicalLeft = mViewport.deviceHeight - mViewport.physicalBottom;
                naturalPhysicalTop = mViewport.physicalLeft;
                naturalDeviceWidth = mViewport.deviceHeight;
                naturalDeviceHeight = mViewport.deviceWidth;
                break;
            case DISPLAY_ORIENTATION_180:
                naturalLogicalWidth = mViewport.logicalRight - mViewport.logicalLeft;
                naturalLogicalHeight = mViewport.logicalBottom - mViewport.logicalTop;
                naturalPhysicalWidth = mViewport.physicalRight - mViewport.physicalLeft;
                naturalPhysicalHeight = mViewport.physicalBottom - mViewport.physicalTop;
                naturalPhysicalLeft = mViewport.deviceWidth - mViewport.physicalRight;
                naturalPhysicalTop = mViewport.deviceHeight - mViewport.physicalBottom;
                naturalDeviceWidth = mViewport.deviceWidth;
                naturalDeviceHeight = mViewport.deviceHeight;
                break;
            case DISPLAY_ORIENTATION_270:
                naturalLogicalWidth = mViewport.logicalBottom - mViewport.logicalTop;
                naturalLogicalHeight = mViewport.logicalRight - mViewport.logicalLeft;
                naturalPhysicalWidth = mViewport.physicalBottom - mViewport.physicalTop;
                naturalPhysicalHeight = mViewport.physicalRight - mViewport.physicalLeft;
                naturalPhysicalLeft = mViewport.physicalTop;
                naturalPhysicalTop = mViewport.deviceWidth - mViewport.physicalRight;
                naturalDeviceWidth = mViewport.deviceHeight;
                naturalDeviceHeight = mViewport.deviceWidth;
                break;
            case DISPLAY_ORIENTATION_0:
            default:
                naturalLogicalWidth = mViewport.logicalRight - mViewport.logicalLeft;
                naturalLogicalHeight = mViewport.logicalBottom - mViewport.logicalTop;
                naturalPhysicalWidth = mViewport.physicalRight - mViewport.physicalLeft;
                naturalPhysicalHeight = mViewport.physicalBottom - mViewport.physicalTop;
                naturalPhysicalLeft = mViewport.physicalLeft;
                naturalPhysicalTop = mViewport.physicalTop;
                naturalDeviceWidth = mViewport.deviceWidth;
                naturalDeviceHeight = mViewport.deviceHeight;
                break;
            }

            if (naturalPhysicalHeight == 0 || naturalPhysicalWidth == 0) {
                ALOGE("Viewport is not set properly: %s", mViewport.toString().c_str());
                naturalPhysicalHeight = naturalPhysicalHeight == 0 ? 1 : naturalPhysicalHeight;
                naturalPhysicalWidth = naturalPhysicalWidth == 0 ? 1 : naturalPhysicalWidth;
            }

            mPhysicalWidth = naturalPhysicalWidth;
            mPhysicalHeight = naturalPhysicalHeight;
            mPhysicalLeft = naturalPhysicalLeft;
            mPhysicalTop = naturalPhysicalTop;

            mSurfaceWidth = naturalLogicalWidth * naturalDeviceWidth / naturalPhysicalWidth;
            mSurfaceHeight = naturalLogicalHeight * naturalDeviceHeight / naturalPhysicalHeight;
            mSurfaceLeft = naturalPhysicalLeft * naturalLogicalWidth / naturalPhysicalWidth;
            mSurfaceTop = naturalPhysicalTop * naturalLogicalHeight / naturalPhysicalHeight;

            mSurfaceOrientation = mParameters.orientationAware ?
                    mViewport.orientation : DISPLAY_ORIENTATION_0;
        } else {
            mPhysicalWidth = rawWidth;
            mPhysicalHeight = rawHeight;
            mPhysicalLeft = 0;
            mPhysicalTop = 0;

            mSurfaceWidth = rawWidth;
            mSurfaceHeight = rawHeight;
            mSurfaceLeft = 0;
            mSurfaceTop = 0;
            mSurfaceOrientation = DISPLAY_ORIENTATION_0;
        }
    }

    // If moving between pointer modes, need to reset some state.
    bool deviceModeChanged = mDeviceMode != oldDeviceMode;
    if (deviceModeChanged) {
        mOrientedRanges.clear();
    }

    // Create or update pointer controller if needed.
    if (mDeviceMode == DEVICE_MODE_POINTER ||
            (mDeviceMode == DEVICE_MODE_DIRECT && mConfig.showTouches)) {
        if (mPointerController == nullptr || viewportChanged) {
            mPointerController = getPolicy()->obtainPointerController(getDeviceId());
        }
    } else {
        mPointerController.clear();
    }

    if (viewportChanged || deviceModeChanged) {
        ALOGI("Device reconfigured: id=%d, name='%s', size %dx%d, orientation %d, mode %d, "
                "display id %d",
                getDeviceId(), getDeviceName().c_str(), mSurfaceWidth, mSurfaceHeight,
                mSurfaceOrientation, mDeviceMode, mViewport.displayId);

        // Configure X and Y factors.
        mXScale = float(mSurfaceWidth) / rawWidth;
        mYScale = float(mSurfaceHeight) / rawHeight;
        mXTranslate = -mSurfaceLeft;
        mYTranslate = -mSurfaceTop;
        mXPrecision = 1.0f / mXScale;
        mYPrecision = 1.0f / mYScale;

        mOrientedRanges.x.axis = AMOTION_EVENT_AXIS_X;
        mOrientedRanges.x.source = mSource;
        mOrientedRanges.y.axis = AMOTION_EVENT_AXIS_Y;
        mOrientedRanges.y.source = mSource;

        configureVirtualKeys();

        // Scale factor for terms that are not oriented in a particular axis.
        // If the pixels are square then xScale == yScale otherwise we fake it
        // by choosing an average.
        mGeometricScale = avg(mXScale, mYScale);

        // Size of diagonal axis.
        float diagonalSize = hypotf(mSurfaceWidth, mSurfaceHeight);

        // Size factors.
        if (mCalibration.sizeCalibration != Calibration::SIZE_CALIBRATION_NONE) {
            if (mRawPointerAxes.touchMajor.valid
                    && mRawPointerAxes.touchMajor.maxValue != 0) {
                mSizeScale = 1.0f / mRawPointerAxes.touchMajor.maxValue;
            } else if (mRawPointerAxes.toolMajor.valid
                    && mRawPointerAxes.toolMajor.maxValue != 0) {
                mSizeScale = 1.0f / mRawPointerAxes.toolMajor.maxValue;
            } else {
                mSizeScale = 0.0f;
            }

            mOrientedRanges.haveTouchSize = true;
            mOrientedRanges.haveToolSize = true;
            mOrientedRanges.haveSize = true;

            mOrientedRanges.touchMajor.axis = AMOTION_EVENT_AXIS_TOUCH_MAJOR;
            mOrientedRanges.touchMajor.source = mSource;
            mOrientedRanges.touchMajor.min = 0;
            mOrientedRanges.touchMajor.max = diagonalSize;
            mOrientedRanges.touchMajor.flat = 0;
            mOrientedRanges.touchMajor.fuzz = 0;
            mOrientedRanges.touchMajor.resolution = 0;

            mOrientedRanges.touchMinor = mOrientedRanges.touchMajor;
            mOrientedRanges.touchMinor.axis = AMOTION_EVENT_AXIS_TOUCH_MINOR;

            mOrientedRanges.toolMajor.axis = AMOTION_EVENT_AXIS_TOOL_MAJOR;
            mOrientedRanges.toolMajor.source = mSource;
            mOrientedRanges.toolMajor.min = 0;
            mOrientedRanges.toolMajor.max = diagonalSize;
            mOrientedRanges.toolMajor.flat = 0;
            mOrientedRanges.toolMajor.fuzz = 0;
            mOrientedRanges.toolMajor.resolution = 0;

            mOrientedRanges.toolMinor = mOrientedRanges.toolMajor;
            mOrientedRanges.toolMinor.axis = AMOTION_EVENT_AXIS_TOOL_MINOR;

            mOrientedRanges.size.axis = AMOTION_EVENT_AXIS_SIZE;
            mOrientedRanges.size.source = mSource;
            mOrientedRanges.size.min = 0;
            mOrientedRanges.size.max = 1.0;
            mOrientedRanges.size.flat = 0;
            mOrientedRanges.size.fuzz = 0;
            mOrientedRanges.size.resolution = 0;
        } else {
            mSizeScale = 0.0f;
        }

        // Pressure factors.
        mPressureScale = 0;
        float pressureMax = 1.0;
        if (mCalibration.pressureCalibration == Calibration::PRESSURE_CALIBRATION_PHYSICAL
                || mCalibration.pressureCalibration
                        == Calibration::PRESSURE_CALIBRATION_AMPLITUDE) {
            if (mCalibration.havePressureScale) {
                mPressureScale = mCalibration.pressureScale;
                pressureMax = mPressureScale * mRawPointerAxes.pressure.maxValue;
            } else if (mRawPointerAxes.pressure.valid
                    && mRawPointerAxes.pressure.maxValue != 0) {
                mPressureScale = 1.0f / mRawPointerAxes.pressure.maxValue;
            }
        }

        mOrientedRanges.pressure.axis = AMOTION_EVENT_AXIS_PRESSURE;
        mOrientedRanges.pressure.source = mSource;
        mOrientedRanges.pressure.min = 0;
        mOrientedRanges.pressure.max = pressureMax;
        mOrientedRanges.pressure.flat = 0;
        mOrientedRanges.pressure.fuzz = 0;
        mOrientedRanges.pressure.resolution = 0;

        // Tilt
        mTiltXCenter = 0;
        mTiltXScale = 0;
        mTiltYCenter = 0;
        mTiltYScale = 0;
        mHaveTilt = mRawPointerAxes.tiltX.valid && mRawPointerAxes.tiltY.valid;
        if (mHaveTilt) {
            mTiltXCenter = avg(mRawPointerAxes.tiltX.minValue,
                    mRawPointerAxes.tiltX.maxValue);
            mTiltYCenter = avg(mRawPointerAxes.tiltY.minValue,
                    mRawPointerAxes.tiltY.maxValue);
            mTiltXScale = M_PI / 180;
            mTiltYScale = M_PI / 180;

            mOrientedRanges.haveTilt = true;

            mOrientedRanges.tilt.axis = AMOTION_EVENT_AXIS_TILT;
            mOrientedRanges.tilt.source = mSource;
            mOrientedRanges.tilt.min = 0;
            mOrientedRanges.tilt.max = M_PI_2;
            mOrientedRanges.tilt.flat = 0;
            mOrientedRanges.tilt.fuzz = 0;
            mOrientedRanges.tilt.resolution = 0;
        }

        // Orientation
        mOrientationScale = 0;
        if (mHaveTilt) {
            mOrientedRanges.haveOrientation = true;

            mOrientedRanges.orientation.axis = AMOTION_EVENT_AXIS_ORIENTATION;
            mOrientedRanges.orientation.source = mSource;
            mOrientedRanges.orientation.min = -M_PI;
            mOrientedRanges.orientation.max = M_PI;
            mOrientedRanges.orientation.flat = 0;
            mOrientedRanges.orientation.fuzz = 0;
            mOrientedRanges.orientation.resolution = 0;
        } else if (mCalibration.orientationCalibration !=
                Calibration::ORIENTATION_CALIBRATION_NONE) {
            if (mCalibration.orientationCalibration
                    == Calibration::ORIENTATION_CALIBRATION_INTERPOLATED) {
                if (mRawPointerAxes.orientation.valid) {
                    if (mRawPointerAxes.orientation.maxValue > 0) {
                        mOrientationScale = M_PI_2 / mRawPointerAxes.orientation.maxValue;
                    } else if (mRawPointerAxes.orientation.minValue < 0) {
                        mOrientationScale = -M_PI_2 / mRawPointerAxes.orientation.minValue;
                    } else {
                        mOrientationScale = 0;
                    }
                }
            }

            mOrientedRanges.haveOrientation = true;

            mOrientedRanges.orientation.axis = AMOTION_EVENT_AXIS_ORIENTATION;
            mOrientedRanges.orientation.source = mSource;
            mOrientedRanges.orientation.min = -M_PI_2;
            mOrientedRanges.orientation.max = M_PI_2;
            mOrientedRanges.orientation.flat = 0;
            mOrientedRanges.orientation.fuzz = 0;
            mOrientedRanges.orientation.resolution = 0;
        }

        // Distance
        mDistanceScale = 0;
        if (mCalibration.distanceCalibration != Calibration::DISTANCE_CALIBRATION_NONE) {
            if (mCalibration.distanceCalibration
                    == Calibration::DISTANCE_CALIBRATION_SCALED) {
                if (mCalibration.haveDistanceScale) {
                    mDistanceScale = mCalibration.distanceScale;
                } else {
                    mDistanceScale = 1.0f;
                }
            }

            mOrientedRanges.haveDistance = true;

            mOrientedRanges.distance.axis = AMOTION_EVENT_AXIS_DISTANCE;
            mOrientedRanges.distance.source = mSource;
            mOrientedRanges.distance.min =
                    mRawPointerAxes.distance.minValue * mDistanceScale;
            mOrientedRanges.distance.max =
                    mRawPointerAxes.distance.maxValue * mDistanceScale;
            mOrientedRanges.distance.flat = 0;
            mOrientedRanges.distance.fuzz =
                    mRawPointerAxes.distance.fuzz * mDistanceScale;
            mOrientedRanges.distance.resolution = 0;
        }

        // Compute oriented precision, scales and ranges.
        // Note that the maximum value reported is an inclusive maximum value so it is one
        // unit less than the total width or height of surface.
        switch (mSurfaceOrientation) {
        case DISPLAY_ORIENTATION_90:
        case DISPLAY_ORIENTATION_270:
            mOrientedXPrecision = mYPrecision;
            mOrientedYPrecision = mXPrecision;

            mOrientedRanges.x.min = mYTranslate;
            mOrientedRanges.x.max = mSurfaceHeight + mYTranslate - 1;
            mOrientedRanges.x.flat = 0;
            mOrientedRanges.x.fuzz = 0;
            mOrientedRanges.x.resolution = mRawPointerAxes.y.resolution * mYScale;

            mOrientedRanges.y.min = mXTranslate;
            mOrientedRanges.y.max = mSurfaceWidth + mXTranslate - 1;
            mOrientedRanges.y.flat = 0;
            mOrientedRanges.y.fuzz = 0;
            mOrientedRanges.y.resolution = mRawPointerAxes.x.resolution * mXScale;
            break;

        default:
            mOrientedXPrecision = mXPrecision;
            mOrientedYPrecision = mYPrecision;

            mOrientedRanges.x.min = mXTranslate;
            mOrientedRanges.x.max = mSurfaceWidth + mXTranslate - 1;
            mOrientedRanges.x.flat = 0;
            mOrientedRanges.x.fuzz = 0;
            mOrientedRanges.x.resolution = mRawPointerAxes.x.resolution * mXScale;

            mOrientedRanges.y.min = mYTranslate;
            mOrientedRanges.y.max = mSurfaceHeight + mYTranslate - 1;
            mOrientedRanges.y.flat = 0;
            mOrientedRanges.y.fuzz = 0;
            mOrientedRanges.y.resolution = mRawPointerAxes.y.resolution * mYScale;
            break;
        }

        // Location
        updateAffineTransformation();

        if (mDeviceMode == DEVICE_MODE_POINTER) {
            // Compute pointer gesture detection parameters.
            float rawDiagonal = hypotf(rawWidth, rawHeight);
            float displayDiagonal = hypotf(mSurfaceWidth, mSurfaceHeight);

            // Scale movements such that one whole swipe of the touch pad covers a
            // given area relative to the diagonal size of the display when no acceleration
            // is applied.
            // Assume that the touch pad has a square aspect ratio such that movements in
            // X and Y of the same number of raw units cover the same physical distance.
            mPointerXMovementScale = mConfig.pointerGestureMovementSpeedRatio
                    * displayDiagonal / rawDiagonal;
            mPointerYMovementScale = mPointerXMovementScale;

            // Scale zooms to cover a smaller range of the display than movements do.
            // This value determines the area around the pointer that is affected by freeform
            // pointer gestures.
            mPointerXZoomScale = mConfig.pointerGestureZoomSpeedRatio
                    * displayDiagonal / rawDiagonal;
            mPointerYZoomScale = mPointerXZoomScale;

            // Max width between pointers to detect a swipe gesture is more than some fraction
            // of the diagonal axis of the touch pad.  Touches that are wider than this are
            // translated into freeform gestures.
            mPointerGestureMaxSwipeWidth =
                    mConfig.pointerGestureSwipeMaxWidthRatio * rawDiagonal;

            // Abort current pointer usages because the state has changed.
            abortPointerUsage(when, 0 /*policyFlags*/);
        }

        // Inform the dispatcher about the changes.
        *outResetNeeded = true;
        bumpGeneration();
    }
}

void TouchInputMapper::dumpSurface(std::string& dump) {
    dump += StringPrintf(INDENT3 "%s\n", mViewport.toString().c_str());
    dump += StringPrintf(INDENT3 "SurfaceWidth: %dpx\n", mSurfaceWidth);
    dump += StringPrintf(INDENT3 "SurfaceHeight: %dpx\n", mSurfaceHeight);
    dump += StringPrintf(INDENT3 "SurfaceLeft: %d\n", mSurfaceLeft);
    dump += StringPrintf(INDENT3 "SurfaceTop: %d\n", mSurfaceTop);
    dump += StringPrintf(INDENT3 "PhysicalWidth: %dpx\n", mPhysicalWidth);
    dump += StringPrintf(INDENT3 "PhysicalHeight: %dpx\n", mPhysicalHeight);
    dump += StringPrintf(INDENT3 "PhysicalLeft: %d\n", mPhysicalLeft);
    dump += StringPrintf(INDENT3 "PhysicalTop: %d\n", mPhysicalTop);
    dump += StringPrintf(INDENT3 "SurfaceOrientation: %d\n", mSurfaceOrientation);
}

void TouchInputMapper::configureVirtualKeys() {
    std::vector<VirtualKeyDefinition> virtualKeyDefinitions;
    getEventHub()->getVirtualKeyDefinitions(getDeviceId(), virtualKeyDefinitions);

    mVirtualKeys.clear();

    if (virtualKeyDefinitions.size() == 0) {
        return;
    }

    int32_t touchScreenLeft = mRawPointerAxes.x.minValue;
    int32_t touchScreenTop = mRawPointerAxes.y.minValue;
    int32_t touchScreenWidth = mRawPointerAxes.getRawWidth();
    int32_t touchScreenHeight = mRawPointerAxes.getRawHeight();

    for (const VirtualKeyDefinition& virtualKeyDefinition : virtualKeyDefinitions) {
        VirtualKey virtualKey;

        virtualKey.scanCode = virtualKeyDefinition.scanCode;
        int32_t keyCode;
        int32_t dummyKeyMetaState;
        uint32_t flags;
        if (getEventHub()->mapKey(getDeviceId(), virtualKey.scanCode, 0, 0,
                                  &keyCode, &dummyKeyMetaState, &flags)) {
            ALOGW(INDENT "VirtualKey %d: could not obtain key code, ignoring",
                    virtualKey.scanCode);
            continue; // drop the key
        }

        virtualKey.keyCode = keyCode;
        virtualKey.flags = flags;

        // convert the key definition's display coordinates into touch coordinates for a hit box
        int32_t halfWidth = virtualKeyDefinition.width / 2;
        int32_t halfHeight = virtualKeyDefinition.height / 2;

        virtualKey.hitLeft = (virtualKeyDefinition.centerX - halfWidth)
                * touchScreenWidth / mSurfaceWidth + touchScreenLeft;
        virtualKey.hitRight= (virtualKeyDefinition.centerX + halfWidth)
                * touchScreenWidth / mSurfaceWidth + touchScreenLeft;
        virtualKey.hitTop = (virtualKeyDefinition.centerY - halfHeight)
                * touchScreenHeight / mSurfaceHeight + touchScreenTop;
        virtualKey.hitBottom = (virtualKeyDefinition.centerY + halfHeight)
                * touchScreenHeight / mSurfaceHeight + touchScreenTop;
        mVirtualKeys.push_back(virtualKey);
    }
}

void TouchInputMapper::dumpVirtualKeys(std::string& dump) {
    if (!mVirtualKeys.empty()) {
        dump += INDENT3 "Virtual Keys:\n";

        for (size_t i = 0; i < mVirtualKeys.size(); i++) {
            const VirtualKey& virtualKey = mVirtualKeys[i];
            dump += StringPrintf(INDENT4 "%zu: scanCode=%d, keyCode=%d, "
                    "hitLeft=%d, hitRight=%d, hitTop=%d, hitBottom=%d\n",
                    i, virtualKey.scanCode, virtualKey.keyCode,
                    virtualKey.hitLeft, virtualKey.hitRight,
                    virtualKey.hitTop, virtualKey.hitBottom);
        }
    }
}

void TouchInputMapper::parseCalibration() {
    const PropertyMap& in = getDevice()->getConfiguration();
    Calibration& out = mCalibration;

    // Size
    out.sizeCalibration = Calibration::SIZE_CALIBRATION_DEFAULT;
    String8 sizeCalibrationString;
    if (in.tryGetProperty(String8("touch.size.calibration"), sizeCalibrationString)) {
        if (sizeCalibrationString == "none") {
            out.sizeCalibration = Calibration::SIZE_CALIBRATION_NONE;
        } else if (sizeCalibrationString == "geometric") {
            out.sizeCalibration = Calibration::SIZE_CALIBRATION_GEOMETRIC;
        } else if (sizeCalibrationString == "diameter") {
            out.sizeCalibration = Calibration::SIZE_CALIBRATION_DIAMETER;
        } else if (sizeCalibrationString == "box") {
            out.sizeCalibration = Calibration::SIZE_CALIBRATION_BOX;
        } else if (sizeCalibrationString == "area") {
            out.sizeCalibration = Calibration::SIZE_CALIBRATION_AREA;
        } else if (sizeCalibrationString != "default") {
            ALOGW("Invalid value for touch.size.calibration: '%s'",
                    sizeCalibrationString.string());
        }
    }

    out.haveSizeScale = in.tryGetProperty(String8("touch.size.scale"),
            out.sizeScale);
    out.haveSizeBias = in.tryGetProperty(String8("touch.size.bias"),
            out.sizeBias);
    out.haveSizeIsSummed = in.tryGetProperty(String8("touch.size.isSummed"),
            out.sizeIsSummed);

    // Pressure
    out.pressureCalibration = Calibration::PRESSURE_CALIBRATION_DEFAULT;
    String8 pressureCalibrationString;
    if (in.tryGetProperty(String8("touch.pressure.calibration"), pressureCalibrationString)) {
        if (pressureCalibrationString == "none") {
            out.pressureCalibration = Calibration::PRESSURE_CALIBRATION_NONE;
        } else if (pressureCalibrationString == "physical") {
            out.pressureCalibration = Calibration::PRESSURE_CALIBRATION_PHYSICAL;
        } else if (pressureCalibrationString == "amplitude") {
            out.pressureCalibration = Calibration::PRESSURE_CALIBRATION_AMPLITUDE;
        } else if (pressureCalibrationString != "default") {
            ALOGW("Invalid value for touch.pressure.calibration: '%s'",
                    pressureCalibrationString.string());
        }
    }

    out.havePressureScale = in.tryGetProperty(String8("touch.pressure.scale"),
            out.pressureScale);

    // Orientation
    out.orientationCalibration = Calibration::ORIENTATION_CALIBRATION_DEFAULT;
    String8 orientationCalibrationString;
    if (in.tryGetProperty(String8("touch.orientation.calibration"), orientationCalibrationString)) {
        if (orientationCalibrationString == "none") {
            out.orientationCalibration = Calibration::ORIENTATION_CALIBRATION_NONE;
        } else if (orientationCalibrationString == "interpolated") {
            out.orientationCalibration = Calibration::ORIENTATION_CALIBRATION_INTERPOLATED;
        } else if (orientationCalibrationString == "vector") {
            out.orientationCalibration = Calibration::ORIENTATION_CALIBRATION_VECTOR;
        } else if (orientationCalibrationString != "default") {
            ALOGW("Invalid value for touch.orientation.calibration: '%s'",
                    orientationCalibrationString.string());
        }
    }

    // Distance
    out.distanceCalibration = Calibration::DISTANCE_CALIBRATION_DEFAULT;
    String8 distanceCalibrationString;
    if (in.tryGetProperty(String8("touch.distance.calibration"), distanceCalibrationString)) {
        if (distanceCalibrationString == "none") {
            out.distanceCalibration = Calibration::DISTANCE_CALIBRATION_NONE;
        } else if (distanceCalibrationString == "scaled") {
            out.distanceCalibration = Calibration::DISTANCE_CALIBRATION_SCALED;
        } else if (distanceCalibrationString != "default") {
            ALOGW("Invalid value for touch.distance.calibration: '%s'",
                    distanceCalibrationString.string());
        }
    }

    out.haveDistanceScale = in.tryGetProperty(String8("touch.distance.scale"),
            out.distanceScale);

    out.coverageCalibration = Calibration::COVERAGE_CALIBRATION_DEFAULT;
    String8 coverageCalibrationString;
    if (in.tryGetProperty(String8("touch.coverage.calibration"), coverageCalibrationString)) {
        if (coverageCalibrationString == "none") {
            out.coverageCalibration = Calibration::COVERAGE_CALIBRATION_NONE;
        } else if (coverageCalibrationString == "box") {
            out.coverageCalibration = Calibration::COVERAGE_CALIBRATION_BOX;
        } else if (coverageCalibrationString != "default") {
            ALOGW("Invalid value for touch.coverage.calibration: '%s'",
                    coverageCalibrationString.string());
        }
    }
}

void TouchInputMapper::resolveCalibration() {
    // Size
    if (mRawPointerAxes.touchMajor.valid || mRawPointerAxes.toolMajor.valid) {
        if (mCalibration.sizeCalibration == Calibration::SIZE_CALIBRATION_DEFAULT) {
            mCalibration.sizeCalibration = Calibration::SIZE_CALIBRATION_GEOMETRIC;
        }
    } else {
        mCalibration.sizeCalibration = Calibration::SIZE_CALIBRATION_NONE;
    }

    // Pressure
    if (mRawPointerAxes.pressure.valid) {
        if (mCalibration.pressureCalibration == Calibration::PRESSURE_CALIBRATION_DEFAULT) {
            mCalibration.pressureCalibration = Calibration::PRESSURE_CALIBRATION_PHYSICAL;
        }
    } else {
        mCalibration.pressureCalibration = Calibration::PRESSURE_CALIBRATION_NONE;
    }

    // Orientation
    if (mRawPointerAxes.orientation.valid) {
        if (mCalibration.orientationCalibration == Calibration::ORIENTATION_CALIBRATION_DEFAULT) {
            mCalibration.orientationCalibration = Calibration::ORIENTATION_CALIBRATION_INTERPOLATED;
        }
    } else {
        mCalibration.orientationCalibration = Calibration::ORIENTATION_CALIBRATION_NONE;
    }

    // Distance
    if (mRawPointerAxes.distance.valid) {
        if (mCalibration.distanceCalibration == Calibration::DISTANCE_CALIBRATION_DEFAULT) {
            mCalibration.distanceCalibration = Calibration::DISTANCE_CALIBRATION_SCALED;
        }
    } else {
        mCalibration.distanceCalibration = Calibration::DISTANCE_CALIBRATION_NONE;
    }

    // Coverage
    if (mCalibration.coverageCalibration == Calibration::COVERAGE_CALIBRATION_DEFAULT) {
        mCalibration.coverageCalibration = Calibration::COVERAGE_CALIBRATION_NONE;
    }
}

void TouchInputMapper::dumpCalibration(std::string& dump) {
    dump += INDENT3 "Calibration:\n";

    // Size
    switch (mCalibration.sizeCalibration) {
    case Calibration::SIZE_CALIBRATION_NONE:
        dump += INDENT4 "touch.size.calibration: none\n";
        break;
    case Calibration::SIZE_CALIBRATION_GEOMETRIC:
        dump += INDENT4 "touch.size.calibration: geometric\n";
        break;
    case Calibration::SIZE_CALIBRATION_DIAMETER:
        dump += INDENT4 "touch.size.calibration: diameter\n";
        break;
    case Calibration::SIZE_CALIBRATION_BOX:
        dump += INDENT4 "touch.size.calibration: box\n";
        break;
    case Calibration::SIZE_CALIBRATION_AREA:
        dump += INDENT4 "touch.size.calibration: area\n";
        break;
    default:
        ALOG_ASSERT(false);
    }

    if (mCalibration.haveSizeScale) {
        dump += StringPrintf(INDENT4 "touch.size.scale: %0.3f\n",
                mCalibration.sizeScale);
    }

    if (mCalibration.haveSizeBias) {
        dump += StringPrintf(INDENT4 "touch.size.bias: %0.3f\n",
                mCalibration.sizeBias);
    }

    if (mCalibration.haveSizeIsSummed) {
        dump += StringPrintf(INDENT4 "touch.size.isSummed: %s\n",
                toString(mCalibration.sizeIsSummed));
    }

    // Pressure
    switch (mCalibration.pressureCalibration) {
    case Calibration::PRESSURE_CALIBRATION_NONE:
        dump += INDENT4 "touch.pressure.calibration: none\n";
        break;
    case Calibration::PRESSURE_CALIBRATION_PHYSICAL:
        dump += INDENT4 "touch.pressure.calibration: physical\n";
        break;
    case Calibration::PRESSURE_CALIBRATION_AMPLITUDE:
        dump += INDENT4 "touch.pressure.calibration: amplitude\n";
        break;
    default:
        ALOG_ASSERT(false);
    }

    if (mCalibration.havePressureScale) {
        dump += StringPrintf(INDENT4 "touch.pressure.scale: %0.3f\n",
                mCalibration.pressureScale);
    }

    // Orientation
    switch (mCalibration.orientationCalibration) {
    case Calibration::ORIENTATION_CALIBRATION_NONE:
        dump += INDENT4 "touch.orientation.calibration: none\n";
        break;
    case Calibration::ORIENTATION_CALIBRATION_INTERPOLATED:
        dump += INDENT4 "touch.orientation.calibration: interpolated\n";
        break;
    case Calibration::ORIENTATION_CALIBRATION_VECTOR:
        dump += INDENT4 "touch.orientation.calibration: vector\n";
        break;
    default:
        ALOG_ASSERT(false);
    }

    // Distance
    switch (mCalibration.distanceCalibration) {
    case Calibration::DISTANCE_CALIBRATION_NONE:
        dump += INDENT4 "touch.distance.calibration: none\n";
        break;
    case Calibration::DISTANCE_CALIBRATION_SCALED:
        dump += INDENT4 "touch.distance.calibration: scaled\n";
        break;
    default:
        ALOG_ASSERT(false);
    }

    if (mCalibration.haveDistanceScale) {
        dump += StringPrintf(INDENT4 "touch.distance.scale: %0.3f\n",
                mCalibration.distanceScale);
    }

    switch (mCalibration.coverageCalibration) {
    case Calibration::COVERAGE_CALIBRATION_NONE:
        dump += INDENT4 "touch.coverage.calibration: none\n";
        break;
    case Calibration::COVERAGE_CALIBRATION_BOX:
        dump += INDENT4 "touch.coverage.calibration: box\n";
        break;
    default:
        ALOG_ASSERT(false);
    }
}

void TouchInputMapper::dumpAffineTransformation(std::string& dump) {
    dump += INDENT3 "Affine Transformation:\n";

    dump += StringPrintf(INDENT4 "X scale: %0.3f\n", mAffineTransform.x_scale);
    dump += StringPrintf(INDENT4 "X ymix: %0.3f\n", mAffineTransform.x_ymix);
    dump += StringPrintf(INDENT4 "X offset: %0.3f\n", mAffineTransform.x_offset);
    dump += StringPrintf(INDENT4 "Y xmix: %0.3f\n", mAffineTransform.y_xmix);
    dump += StringPrintf(INDENT4 "Y scale: %0.3f\n", mAffineTransform.y_scale);
    dump += StringPrintf(INDENT4 "Y offset: %0.3f\n", mAffineTransform.y_offset);
}

void TouchInputMapper::updateAffineTransformation() {
    mAffineTransform = getPolicy()->getTouchAffineTransformation(mDevice->getDescriptor(),
            mSurfaceOrientation);
}

void TouchInputMapper::reset(nsecs_t when) {
    mCursorButtonAccumulator.reset(getDevice());
    mCursorScrollAccumulator.reset(getDevice());
    mTouchButtonAccumulator.reset(getDevice());

    mPointerVelocityControl.reset();
    mWheelXVelocityControl.reset();
    mWheelYVelocityControl.reset();

    mRawStatesPending.clear();
    mCurrentRawState.clear();
    mCurrentCookedState.clear();
    mLastRawState.clear();
    mLastCookedState.clear();
    mPointerUsage = POINTER_USAGE_NONE;
    mSentHoverEnter = false;
    mHavePointerIds = false;
    mCurrentMotionAborted = false;
    mDownTime = 0;

    mCurrentVirtualKey.down = false;

    mPointerGesture.reset();
    mPointerSimple.reset();
    resetExternalStylus();

    if (mPointerController != nullptr) {
        mPointerController->fade(PointerControllerInterface::TRANSITION_GRADUAL);
        mPointerController->clearSpots();
    }

    InputMapper::reset(when);
}

void TouchInputMapper::resetExternalStylus() {
    mExternalStylusState.clear();
    mExternalStylusId = -1;
    mExternalStylusFusionTimeout = LLONG_MAX;
    mExternalStylusDataPending = false;
}

void TouchInputMapper::clearStylusDataPendingFlags() {
    mExternalStylusDataPending = false;
    mExternalStylusFusionTimeout = LLONG_MAX;
}

void TouchInputMapper::reportEventForStatistics(nsecs_t evdevTime) {
    nsecs_t now = systemTime(CLOCK_MONOTONIC);
    nsecs_t latency = now - evdevTime;
    mStatistics.addValue(nanoseconds_to_microseconds(latency));
    nsecs_t timeSinceLastReport = now - mStatistics.lastReportTime;
    if (timeSinceLastReport > STATISTICS_REPORT_FREQUENCY) {
        android::util::stats_write(android::util::TOUCH_EVENT_REPORTED,
                mStatistics.min, mStatistics.max,
                mStatistics.mean(), mStatistics.stdev(), mStatistics.count);
        mStatistics.reset(now);
    }
}

void TouchInputMapper::process(const RawEvent* rawEvent) {
    mCursorButtonAccumulator.process(rawEvent);
    mCursorScrollAccumulator.process(rawEvent);
    mTouchButtonAccumulator.process(rawEvent);

    if (rawEvent->type == EV_SYN && rawEvent->code == SYN_REPORT) {
        reportEventForStatistics(rawEvent->when);
        sync(rawEvent->when);
    }
}

void TouchInputMapper::sync(nsecs_t when) {
    const RawState* last = mRawStatesPending.empty() ?
            &mCurrentRawState : &mRawStatesPending.back();

    // Push a new state.
    mRawStatesPending.emplace_back();

    RawState* next = &mRawStatesPending.back();
    next->clear();
    next->when = when;

    // Sync button state.
    next->buttonState = mTouchButtonAccumulator.getButtonState()
            | mCursorButtonAccumulator.getButtonState();

    // Sync scroll
    next->rawVScroll = mCursorScrollAccumulator.getRelativeVWheel();
    next->rawHScroll = mCursorScrollAccumulator.getRelativeHWheel();
    mCursorScrollAccumulator.finishSync();

    // Sync touch
    syncTouch(when, next);

    // Assign pointer ids.
    if (!mHavePointerIds) {
        assignPointerIds(last, next);
    }

#if DEBUG_RAW_EVENTS
    ALOGD("syncTouch: pointerCount %d -> %d, touching ids 0x%08x -> 0x%08x, "
            "hovering ids 0x%08x -> 0x%08x",
            last->rawPointerData.pointerCount,
            next->rawPointerData.pointerCount,
            last->rawPointerData.touchingIdBits.value,
            next->rawPointerData.touchingIdBits.value,
            last->rawPointerData.hoveringIdBits.value,
            next->rawPointerData.hoveringIdBits.value);
#endif

    processRawTouches(false /*timeout*/);
}

void TouchInputMapper::processRawTouches(bool timeout) {
    if (mDeviceMode == DEVICE_MODE_DISABLED) {
        // Drop all input if the device is disabled.
        mCurrentRawState.clear();
        mRawStatesPending.clear();
        return;
    }

    // Drain any pending touch states. The invariant here is that the mCurrentRawState is always
    // valid and must go through the full cook and dispatch cycle. This ensures that anything
    // touching the current state will only observe the events that have been dispatched to the
    // rest of the pipeline.
    const size_t N = mRawStatesPending.size();
    size_t count;
    for(count = 0; count < N; count++) {
        const RawState& next = mRawStatesPending[count];

        // A failure to assign the stylus id means that we're waiting on stylus data
        // and so should defer the rest of the pipeline.
        if (assignExternalStylusId(next, timeout)) {
            break;
        }

        // All ready to go.
        clearStylusDataPendingFlags();
        mCurrentRawState.copyFrom(next);
        if (mCurrentRawState.when < mLastRawState.when) {
            mCurrentRawState.when = mLastRawState.when;
        }
        cookAndDispatch(mCurrentRawState.when);
    }
    if (count != 0) {
        mRawStatesPending.erase(mRawStatesPending.begin(), mRawStatesPending.begin() + count);
    }

    if (mExternalStylusDataPending) {
        if (timeout) {
            nsecs_t when = mExternalStylusFusionTimeout - STYLUS_DATA_LATENCY;
            clearStylusDataPendingFlags();
            mCurrentRawState.copyFrom(mLastRawState);
#if DEBUG_STYLUS_FUSION
            ALOGD("Timeout expired, synthesizing event with new stylus data");
#endif
            cookAndDispatch(when);
        } else if (mExternalStylusFusionTimeout == LLONG_MAX) {
            mExternalStylusFusionTimeout = mExternalStylusState.when + TOUCH_DATA_TIMEOUT;
            getContext()->requestTimeoutAtTime(mExternalStylusFusionTimeout);
        }
    }
}

void TouchInputMapper::cookAndDispatch(nsecs_t when) {
    // Always start with a clean state.
    mCurrentCookedState.clear();

    // Apply stylus buttons to current raw state.
    applyExternalStylusButtonState(when);

    // Handle policy on initial down or hover events.
    bool initialDown = mLastRawState.rawPointerData.pointerCount == 0
            && mCurrentRawState.rawPointerData.pointerCount != 0;

    uint32_t policyFlags = 0;
    bool buttonsPressed = mCurrentRawState.buttonState & ~mLastRawState.buttonState;
    if (initialDown || buttonsPressed) {
        // If this is a touch screen, hide the pointer on an initial down.
        if (mDeviceMode == DEVICE_MODE_DIRECT) {
            getContext()->fadePointer();
        }

        if (mParameters.wake) {
            policyFlags |= POLICY_FLAG_WAKE;
        }
    }

    // Consume raw off-screen touches before cooking pointer data.
    // If touches are consumed, subsequent code will not receive any pointer data.
    if (consumeRawTouches(when, policyFlags)) {
        mCurrentRawState.rawPointerData.clear();
    }

    // Cook pointer data.  This call populates the mCurrentCookedState.cookedPointerData structure
    // with cooked pointer data that has the same ids and indices as the raw data.
    // The following code can use either the raw or cooked data, as needed.
    cookPointerData();

    // Apply stylus pressure to current cooked state.
    applyExternalStylusTouchState(when);

    // Synthesize key down from raw buttons if needed.
    synthesizeButtonKeys(getContext(), AKEY_EVENT_ACTION_DOWN, when, getDeviceId(), mSource,
            mViewport.displayId, policyFlags,
            mLastCookedState.buttonState, mCurrentCookedState.buttonState);

    // Dispatch the touches either directly or by translation through a pointer on screen.
    if (mDeviceMode == DEVICE_MODE_POINTER) {
        for (BitSet32 idBits(mCurrentRawState.rawPointerData.touchingIdBits);
                !idBits.isEmpty(); ) {
            uint32_t id = idBits.clearFirstMarkedBit();
            const RawPointerData::Pointer& pointer =
                    mCurrentRawState.rawPointerData.pointerForId(id);
            if (pointer.toolType == AMOTION_EVENT_TOOL_TYPE_STYLUS
                    || pointer.toolType == AMOTION_EVENT_TOOL_TYPE_ERASER) {
                mCurrentCookedState.stylusIdBits.markBit(id);
            } else if (pointer.toolType == AMOTION_EVENT_TOOL_TYPE_FINGER
                    || pointer.toolType == AMOTION_EVENT_TOOL_TYPE_UNKNOWN) {
                mCurrentCookedState.fingerIdBits.markBit(id);
            } else if (pointer.toolType == AMOTION_EVENT_TOOL_TYPE_MOUSE) {
                mCurrentCookedState.mouseIdBits.markBit(id);
            }
        }
        for (BitSet32 idBits(mCurrentRawState.rawPointerData.hoveringIdBits);
                !idBits.isEmpty(); ) {
            uint32_t id = idBits.clearFirstMarkedBit();
            const RawPointerData::Pointer& pointer =
                    mCurrentRawState.rawPointerData.pointerForId(id);
            if (pointer.toolType == AMOTION_EVENT_TOOL_TYPE_STYLUS
                    || pointer.toolType == AMOTION_EVENT_TOOL_TYPE_ERASER) {
                mCurrentCookedState.stylusIdBits.markBit(id);
            }
        }

        // Stylus takes precedence over all tools, then mouse, then finger.
        PointerUsage pointerUsage = mPointerUsage;
        if (!mCurrentCookedState.stylusIdBits.isEmpty()) {
            mCurrentCookedState.mouseIdBits.clear();
            mCurrentCookedState.fingerIdBits.clear();
            pointerUsage = POINTER_USAGE_STYLUS;
        } else if (!mCurrentCookedState.mouseIdBits.isEmpty()) {
            mCurrentCookedState.fingerIdBits.clear();
            pointerUsage = POINTER_USAGE_MOUSE;
        } else if (!mCurrentCookedState.fingerIdBits.isEmpty() ||
                isPointerDown(mCurrentRawState.buttonState)) {
            pointerUsage = POINTER_USAGE_GESTURES;
        }

        dispatchPointerUsage(when, policyFlags, pointerUsage);
    } else {
        if (mDeviceMode == DEVICE_MODE_DIRECT
                && mConfig.showTouches && mPointerController != nullptr) {
            mPointerController->setPresentation(PointerControllerInterface::PRESENTATION_SPOT);
            mPointerController->fade(PointerControllerInterface::TRANSITION_GRADUAL);

            mPointerController->setButtonState(mCurrentRawState.buttonState);
            mPointerController->setSpots(mCurrentCookedState.cookedPointerData.pointerCoords,
                    mCurrentCookedState.cookedPointerData.idToIndex,
                    mCurrentCookedState.cookedPointerData.touchingIdBits,
                    mViewport.displayId);
        }

        if (!mCurrentMotionAborted) {
            dispatchButtonRelease(when, policyFlags);
            dispatchHoverExit(when, policyFlags);
            dispatchTouches(when, policyFlags);
            dispatchHoverEnterAndMove(when, policyFlags);
            dispatchButtonPress(when, policyFlags);
        }

        if (mCurrentCookedState.cookedPointerData.pointerCount == 0) {
            mCurrentMotionAborted = false;
        }
    }

    // Synthesize key up from raw buttons if needed.
    synthesizeButtonKeys(getContext(), AKEY_EVENT_ACTION_UP, when, getDeviceId(), mSource,
            mViewport.displayId, policyFlags,
            mLastCookedState.buttonState, mCurrentCookedState.buttonState);

    // Clear some transient state.
    mCurrentRawState.rawVScroll = 0;
    mCurrentRawState.rawHScroll = 0;

    // Copy current touch to last touch in preparation for the next cycle.
    mLastRawState.copyFrom(mCurrentRawState);
    mLastCookedState.copyFrom(mCurrentCookedState);
}

void TouchInputMapper::applyExternalStylusButtonState(nsecs_t when) {
    if (mDeviceMode == DEVICE_MODE_DIRECT && hasExternalStylus() && mExternalStylusId != -1) {
        mCurrentRawState.buttonState |= mExternalStylusState.buttons;
    }
}

void TouchInputMapper::applyExternalStylusTouchState(nsecs_t when) {
    CookedPointerData& currentPointerData = mCurrentCookedState.cookedPointerData;
    const CookedPointerData& lastPointerData = mLastCookedState.cookedPointerData;

    if (mExternalStylusId != -1 && currentPointerData.isTouching(mExternalStylusId)) {
        float pressure = mExternalStylusState.pressure;
        if (pressure == 0.0f && lastPointerData.isTouching(mExternalStylusId)) {
            const PointerCoords& coords = lastPointerData.pointerCoordsForId(mExternalStylusId);
            pressure = coords.getAxisValue(AMOTION_EVENT_AXIS_PRESSURE);
        }
        PointerCoords& coords = currentPointerData.editPointerCoordsWithId(mExternalStylusId);
        coords.setAxisValue(AMOTION_EVENT_AXIS_PRESSURE, pressure);

        PointerProperties& properties =
                currentPointerData.editPointerPropertiesWithId(mExternalStylusId);
        if (mExternalStylusState.toolType != AMOTION_EVENT_TOOL_TYPE_UNKNOWN) {
            properties.toolType = mExternalStylusState.toolType;
        }
    }
}

bool TouchInputMapper::assignExternalStylusId(const RawState& state, bool timeout) {
    if (mDeviceMode != DEVICE_MODE_DIRECT || !hasExternalStylus()) {
        return false;
    }

    const bool initialDown = mLastRawState.rawPointerData.pointerCount == 0
            && state.rawPointerData.pointerCount != 0;
    if (initialDown) {
        if (mExternalStylusState.pressure != 0.0f) {
#if DEBUG_STYLUS_FUSION
            ALOGD("Have both stylus and touch data, beginning fusion");
#endif
            mExternalStylusId = state.rawPointerData.touchingIdBits.firstMarkedBit();
        } else if (timeout) {
#if DEBUG_STYLUS_FUSION
            ALOGD("Timeout expired, assuming touch is not a stylus.");
#endif
            resetExternalStylus();
        } else {
            if (mExternalStylusFusionTimeout == LLONG_MAX) {
                mExternalStylusFusionTimeout = state.when + EXTERNAL_STYLUS_DATA_TIMEOUT;
            }
#if DEBUG_STYLUS_FUSION
            ALOGD("No stylus data but stylus is connected, requesting timeout "
                    "(%" PRId64 "ms)", mExternalStylusFusionTimeout);
#endif
            getContext()->requestTimeoutAtTime(mExternalStylusFusionTimeout);
            return true;
        }
    }

    // Check if the stylus pointer has gone up.
    if (mExternalStylusId != -1 &&
            !state.rawPointerData.touchingIdBits.hasBit(mExternalStylusId)) {
#if DEBUG_STYLUS_FUSION
            ALOGD("Stylus pointer is going up");
#endif
        mExternalStylusId = -1;
    }

    return false;
}

void TouchInputMapper::timeoutExpired(nsecs_t when) {
    if (mDeviceMode == DEVICE_MODE_POINTER) {
        if (mPointerUsage == POINTER_USAGE_GESTURES) {
            dispatchPointerGestures(when, 0 /*policyFlags*/, true /*isTimeout*/);
        }
    } else if (mDeviceMode == DEVICE_MODE_DIRECT) {
        if (mExternalStylusFusionTimeout < when) {
            processRawTouches(true /*timeout*/);
        } else if (mExternalStylusFusionTimeout != LLONG_MAX) {
            getContext()->requestTimeoutAtTime(mExternalStylusFusionTimeout);
        }
    }
}

void TouchInputMapper::updateExternalStylusState(const StylusState& state) {
    mExternalStylusState.copyFrom(state);
    if (mExternalStylusId != -1 || mExternalStylusFusionTimeout != LLONG_MAX) {
        // We're either in the middle of a fused stream of data or we're waiting on data before
        // dispatching the initial down, so go ahead and dispatch now that we have fresh stylus
        // data.
        mExternalStylusDataPending = true;
        processRawTouches(false /*timeout*/);
    }
}

bool TouchInputMapper::consumeRawTouches(nsecs_t when, uint32_t policyFlags) {
    // Check for release of a virtual key.
    if (mCurrentVirtualKey.down) {
        if (mCurrentRawState.rawPointerData.touchingIdBits.isEmpty()) {
            // Pointer went up while virtual key was down.
            mCurrentVirtualKey.down = false;
            if (!mCurrentVirtualKey.ignored) {
#if DEBUG_VIRTUAL_KEYS
                ALOGD("VirtualKeys: Generating key up: keyCode=%d, scanCode=%d",
                        mCurrentVirtualKey.keyCode, mCurrentVirtualKey.scanCode);
#endif
                dispatchVirtualKey(when, policyFlags,
                        AKEY_EVENT_ACTION_UP,
                        AKEY_EVENT_FLAG_FROM_SYSTEM | AKEY_EVENT_FLAG_VIRTUAL_HARD_KEY);
            }
            return true;
        }

        if (mCurrentRawState.rawPointerData.touchingIdBits.count() == 1) {
            uint32_t id = mCurrentRawState.rawPointerData.touchingIdBits.firstMarkedBit();
            const RawPointerData::Pointer& pointer =
                    mCurrentRawState.rawPointerData.pointerForId(id);
            const VirtualKey* virtualKey = findVirtualKeyHit(pointer.x, pointer.y);
            if (virtualKey && virtualKey->keyCode == mCurrentVirtualKey.keyCode) {
                // Pointer is still within the space of the virtual key.
                return true;
            }
        }

        // Pointer left virtual key area or another pointer also went down.
        // Send key cancellation but do not consume the touch yet.
        // This is useful when the user swipes through from the virtual key area
        // into the main display surface.
        mCurrentVirtualKey.down = false;
        if (!mCurrentVirtualKey.ignored) {
#if DEBUG_VIRTUAL_KEYS
            ALOGD("VirtualKeys: Canceling key: keyCode=%d, scanCode=%d",
                    mCurrentVirtualKey.keyCode, mCurrentVirtualKey.scanCode);
#endif
            dispatchVirtualKey(when, policyFlags,
                    AKEY_EVENT_ACTION_UP,
                    AKEY_EVENT_FLAG_FROM_SYSTEM | AKEY_EVENT_FLAG_VIRTUAL_HARD_KEY
                            | AKEY_EVENT_FLAG_CANCELED);
        }
    }

    if (mLastRawState.rawPointerData.touchingIdBits.isEmpty()
            && !mCurrentRawState.rawPointerData.touchingIdBits.isEmpty()) {
        // Pointer just went down.  Check for virtual key press or off-screen touches.
        uint32_t id = mCurrentRawState.rawPointerData.touchingIdBits.firstMarkedBit();
        const RawPointerData::Pointer& pointer = mCurrentRawState.rawPointerData.pointerForId(id);
        if (!isPointInsideSurface(pointer.x, pointer.y)) {
            // If exactly one pointer went down, check for virtual key hit.
            // Otherwise we will drop the entire stroke.
            if (mCurrentRawState.rawPointerData.touchingIdBits.count() == 1) {
                const VirtualKey* virtualKey = findVirtualKeyHit(pointer.x, pointer.y);
                if (virtualKey) {
                    mCurrentVirtualKey.down = true;
                    mCurrentVirtualKey.downTime = when;
                    mCurrentVirtualKey.keyCode = virtualKey->keyCode;
                    mCurrentVirtualKey.scanCode = virtualKey->scanCode;
                    mCurrentVirtualKey.ignored = mContext->shouldDropVirtualKey(
                            when, getDevice(), virtualKey->keyCode, virtualKey->scanCode);

                    if (!mCurrentVirtualKey.ignored) {
#if DEBUG_VIRTUAL_KEYS
                        ALOGD("VirtualKeys: Generating key down: keyCode=%d, scanCode=%d",
                                mCurrentVirtualKey.keyCode,
                                mCurrentVirtualKey.scanCode);
#endif
                        dispatchVirtualKey(when, policyFlags,
                                AKEY_EVENT_ACTION_DOWN,
                                AKEY_EVENT_FLAG_FROM_SYSTEM | AKEY_EVENT_FLAG_VIRTUAL_HARD_KEY);
                    }
                }
            }
            return true;
        }
    }

    // Disable all virtual key touches that happen within a short time interval of the
    // most recent touch within the screen area.  The idea is to filter out stray
    // virtual key presses when interacting with the touch screen.
    //
    // Problems we're trying to solve:
    //
    // 1. While scrolling a list or dragging the window shade, the user swipes down into a
    //    virtual key area that is implemented by a separate touch panel and accidentally
    //    triggers a virtual key.
    //
    // 2. While typing in the on screen keyboard, the user taps slightly outside the screen
    //    area and accidentally triggers a virtual key.  This often happens when virtual keys
    //    are layed out below the screen near to where the on screen keyboard's space bar
    //    is displayed.
    if (mConfig.virtualKeyQuietTime > 0 &&
            !mCurrentRawState.rawPointerData.touchingIdBits.isEmpty()) {
        mContext->disableVirtualKeysUntil(when + mConfig.virtualKeyQuietTime);
    }
    return false;
}

void TouchInputMapper::dispatchVirtualKey(nsecs_t when, uint32_t policyFlags,
        int32_t keyEventAction, int32_t keyEventFlags) {
    int32_t keyCode = mCurrentVirtualKey.keyCode;
    int32_t scanCode = mCurrentVirtualKey.scanCode;
    nsecs_t downTime = mCurrentVirtualKey.downTime;
    int32_t metaState = mContext->getGlobalMetaState();
    policyFlags |= POLICY_FLAG_VIRTUAL;

    NotifyKeyArgs args(mContext->getNextSequenceNum(), when, getDeviceId(), AINPUT_SOURCE_KEYBOARD,
            mViewport.displayId,
            policyFlags, keyEventAction, keyEventFlags, keyCode, scanCode, metaState, downTime);
    getListener()->notifyKey(&args);
}

void TouchInputMapper::abortTouches(nsecs_t when, uint32_t policyFlags) {
    BitSet32 currentIdBits = mCurrentCookedState.cookedPointerData.touchingIdBits;
    if (!currentIdBits.isEmpty()) {
        int32_t metaState = getContext()->getGlobalMetaState();
        int32_t buttonState = mCurrentCookedState.buttonState;
        dispatchMotion(when, policyFlags, mSource, AMOTION_EVENT_ACTION_CANCEL, 0, 0,
                metaState, buttonState, AMOTION_EVENT_EDGE_FLAG_NONE,
                mCurrentCookedState.deviceTimestamp,
                mCurrentCookedState.cookedPointerData.pointerProperties,
                mCurrentCookedState.cookedPointerData.pointerCoords,
                mCurrentCookedState.cookedPointerData.idToIndex,
                currentIdBits, -1,
                mOrientedXPrecision, mOrientedYPrecision, mDownTime);
        mCurrentMotionAborted = true;
    }
}

void TouchInputMapper::dispatchTouches(nsecs_t when, uint32_t policyFlags) {
    BitSet32 currentIdBits = mCurrentCookedState.cookedPointerData.touchingIdBits;
    BitSet32 lastIdBits = mLastCookedState.cookedPointerData.touchingIdBits;
    int32_t metaState = getContext()->getGlobalMetaState();
    int32_t buttonState = mCurrentCookedState.buttonState;

    if (currentIdBits == lastIdBits) {
        if (!currentIdBits.isEmpty()) {
            // No pointer id changes so this is a move event.
            // The listener takes care of batching moves so we don't have to deal with that here.
            dispatchMotion(when, policyFlags, mSource,
                    AMOTION_EVENT_ACTION_MOVE, 0, 0, metaState, buttonState,
                    AMOTION_EVENT_EDGE_FLAG_NONE,
                    mCurrentCookedState.deviceTimestamp,
                    mCurrentCookedState.cookedPointerData.pointerProperties,
                    mCurrentCookedState.cookedPointerData.pointerCoords,
                    mCurrentCookedState.cookedPointerData.idToIndex,
                    currentIdBits, -1,
                    mOrientedXPrecision, mOrientedYPrecision, mDownTime);
        }
    } else {
        // There may be pointers going up and pointers going down and pointers moving
        // all at the same time.
        BitSet32 upIdBits(lastIdBits.value & ~currentIdBits.value);
        BitSet32 downIdBits(currentIdBits.value & ~lastIdBits.value);
        BitSet32 moveIdBits(lastIdBits.value & currentIdBits.value);
        BitSet32 dispatchedIdBits(lastIdBits.value);

        // Update last coordinates of pointers that have moved so that we observe the new
        // pointer positions at the same time as other pointers that have just gone up.
        bool moveNeeded = updateMovedPointers(
                mCurrentCookedState.cookedPointerData.pointerProperties,
                mCurrentCookedState.cookedPointerData.pointerCoords,
                mCurrentCookedState.cookedPointerData.idToIndex,
                mLastCookedState.cookedPointerData.pointerProperties,
                mLastCookedState.cookedPointerData.pointerCoords,
                mLastCookedState.cookedPointerData.idToIndex,
                moveIdBits);
        if (buttonState != mLastCookedState.buttonState) {
            moveNeeded = true;
        }

        // Dispatch pointer up events.
        while (!upIdBits.isEmpty()) {
            uint32_t upId = upIdBits.clearFirstMarkedBit();

            dispatchMotion(when, policyFlags, mSource,
                    AMOTION_EVENT_ACTION_POINTER_UP, 0, 0, metaState, buttonState, 0,
                    mCurrentCookedState.deviceTimestamp,
                    mLastCookedState.cookedPointerData.pointerProperties,
                    mLastCookedState.cookedPointerData.pointerCoords,
                    mLastCookedState.cookedPointerData.idToIndex,
                    dispatchedIdBits, upId, mOrientedXPrecision, mOrientedYPrecision, mDownTime);
            dispatchedIdBits.clearBit(upId);
        }

        // Dispatch move events if any of the remaining pointers moved from their old locations.
        // Although applications receive new locations as part of individual pointer up
        // events, they do not generally handle them except when presented in a move event.
        if (moveNeeded && !moveIdBits.isEmpty()) {
            ALOG_ASSERT(moveIdBits.value == dispatchedIdBits.value);
            dispatchMotion(when, policyFlags, mSource,
                    AMOTION_EVENT_ACTION_MOVE, 0, 0, metaState, buttonState, 0,
                    mCurrentCookedState.deviceTimestamp,
                    mCurrentCookedState.cookedPointerData.pointerProperties,
                    mCurrentCookedState.cookedPointerData.pointerCoords,
                    mCurrentCookedState.cookedPointerData.idToIndex,
                    dispatchedIdBits, -1, mOrientedXPrecision, mOrientedYPrecision, mDownTime);
        }

        // Dispatch pointer down events using the new pointer locations.
        while (!downIdBits.isEmpty()) {
            uint32_t downId = downIdBits.clearFirstMarkedBit();
            dispatchedIdBits.markBit(downId);

            if (dispatchedIdBits.count() == 1) {
                // First pointer is going down.  Set down time.
                mDownTime = when;
            }

            dispatchMotion(when, policyFlags, mSource,
                    AMOTION_EVENT_ACTION_POINTER_DOWN, 0, 0, metaState, buttonState, 0,
                    mCurrentCookedState.deviceTimestamp,
                    mCurrentCookedState.cookedPointerData.pointerProperties,
                    mCurrentCookedState.cookedPointerData.pointerCoords,
                    mCurrentCookedState.cookedPointerData.idToIndex,
                    dispatchedIdBits, downId, mOrientedXPrecision, mOrientedYPrecision, mDownTime);
        }
    }
}

void TouchInputMapper::dispatchHoverExit(nsecs_t when, uint32_t policyFlags) {
    if (mSentHoverEnter &&
            (mCurrentCookedState.cookedPointerData.hoveringIdBits.isEmpty()
                    || !mCurrentCookedState.cookedPointerData.touchingIdBits.isEmpty())) {
        int32_t metaState = getContext()->getGlobalMetaState();
        dispatchMotion(when, policyFlags, mSource,
                AMOTION_EVENT_ACTION_HOVER_EXIT, 0, 0, metaState, mLastCookedState.buttonState, 0,
                mLastCookedState.deviceTimestamp,
                mLastCookedState.cookedPointerData.pointerProperties,
                mLastCookedState.cookedPointerData.pointerCoords,
                mLastCookedState.cookedPointerData.idToIndex,
                mLastCookedState.cookedPointerData.hoveringIdBits, -1,
                mOrientedXPrecision, mOrientedYPrecision, mDownTime);
        mSentHoverEnter = false;
    }
}

void TouchInputMapper::dispatchHoverEnterAndMove(nsecs_t when, uint32_t policyFlags) {
    if (mCurrentCookedState.cookedPointerData.touchingIdBits.isEmpty()
            && !mCurrentCookedState.cookedPointerData.hoveringIdBits.isEmpty()) {
        int32_t metaState = getContext()->getGlobalMetaState();
        if (!mSentHoverEnter) {
            dispatchMotion(when, policyFlags, mSource, AMOTION_EVENT_ACTION_HOVER_ENTER,
                    0, 0, metaState, mCurrentRawState.buttonState, 0,
                    mCurrentCookedState.deviceTimestamp,
                    mCurrentCookedState.cookedPointerData.pointerProperties,
                    mCurrentCookedState.cookedPointerData.pointerCoords,
                    mCurrentCookedState.cookedPointerData.idToIndex,
                    mCurrentCookedState.cookedPointerData.hoveringIdBits, -1,
                    mOrientedXPrecision, mOrientedYPrecision, mDownTime);
            mSentHoverEnter = true;
        }

        dispatchMotion(when, policyFlags, mSource,
                AMOTION_EVENT_ACTION_HOVER_MOVE, 0, 0, metaState,
                mCurrentRawState.buttonState, 0,
                mCurrentCookedState.deviceTimestamp,
                mCurrentCookedState.cookedPointerData.pointerProperties,
                mCurrentCookedState.cookedPointerData.pointerCoords,
                mCurrentCookedState.cookedPointerData.idToIndex,
                mCurrentCookedState.cookedPointerData.hoveringIdBits, -1,
                mOrientedXPrecision, mOrientedYPrecision, mDownTime);
    }
}

void TouchInputMapper::dispatchButtonRelease(nsecs_t when, uint32_t policyFlags) {
    BitSet32 releasedButtons(mLastCookedState.buttonState & ~mCurrentCookedState.buttonState);
    const BitSet32& idBits = findActiveIdBits(mLastCookedState.cookedPointerData);
    const int32_t metaState = getContext()->getGlobalMetaState();
    int32_t buttonState = mLastCookedState.buttonState;
    while (!releasedButtons.isEmpty()) {
        int32_t actionButton = BitSet32::valueForBit(releasedButtons.clearFirstMarkedBit());
        buttonState &= ~actionButton;
        dispatchMotion(when, policyFlags, mSource,
                    AMOTION_EVENT_ACTION_BUTTON_RELEASE, actionButton,
                    0, metaState, buttonState, 0,
                    mCurrentCookedState.deviceTimestamp,
                    mCurrentCookedState.cookedPointerData.pointerProperties,
                    mCurrentCookedState.cookedPointerData.pointerCoords,
                    mCurrentCookedState.cookedPointerData.idToIndex, idBits, -1,
                    mOrientedXPrecision, mOrientedYPrecision, mDownTime);
    }
}

void TouchInputMapper::dispatchButtonPress(nsecs_t when, uint32_t policyFlags) {
    BitSet32 pressedButtons(mCurrentCookedState.buttonState & ~mLastCookedState.buttonState);
    const BitSet32& idBits = findActiveIdBits(mCurrentCookedState.cookedPointerData);
    const int32_t metaState = getContext()->getGlobalMetaState();
    int32_t buttonState = mLastCookedState.buttonState;
    while (!pressedButtons.isEmpty()) {
        int32_t actionButton = BitSet32::valueForBit(pressedButtons.clearFirstMarkedBit());
        buttonState |= actionButton;
        dispatchMotion(when, policyFlags, mSource, AMOTION_EVENT_ACTION_BUTTON_PRESS, actionButton,
                    0, metaState, buttonState, 0,
                    mCurrentCookedState.deviceTimestamp,
                    mCurrentCookedState.cookedPointerData.pointerProperties,
                    mCurrentCookedState.cookedPointerData.pointerCoords,
                    mCurrentCookedState.cookedPointerData.idToIndex, idBits, -1,
                    mOrientedXPrecision, mOrientedYPrecision, mDownTime);
    }
}

const BitSet32& TouchInputMapper::findActiveIdBits(const CookedPointerData& cookedPointerData) {
    if (!cookedPointerData.touchingIdBits.isEmpty()) {
        return cookedPointerData.touchingIdBits;
    }
    return cookedPointerData.hoveringIdBits;
}

void TouchInputMapper::cookPointerData() {
    uint32_t currentPointerCount = mCurrentRawState.rawPointerData.pointerCount;

    mCurrentCookedState.cookedPointerData.clear();
    mCurrentCookedState.deviceTimestamp =
            mCurrentRawState.deviceTimestamp;
    mCurrentCookedState.cookedPointerData.pointerCount = currentPointerCount;
    mCurrentCookedState.cookedPointerData.hoveringIdBits =
            mCurrentRawState.rawPointerData.hoveringIdBits;
    mCurrentCookedState.cookedPointerData.touchingIdBits =
            mCurrentRawState.rawPointerData.touchingIdBits;

    if (mCurrentCookedState.cookedPointerData.pointerCount == 0) {
        mCurrentCookedState.buttonState = 0;
    } else {
        mCurrentCookedState.buttonState = mCurrentRawState.buttonState;
    }

    // Walk through the the active pointers and map device coordinates onto
    // surface coordinates and adjust for display orientation.
    for (uint32_t i = 0; i < currentPointerCount; i++) {
        const RawPointerData::Pointer& in = mCurrentRawState.rawPointerData.pointers[i];

        // Size
        float touchMajor, touchMinor, toolMajor, toolMinor, size;
        switch (mCalibration.sizeCalibration) {
        case Calibration::SIZE_CALIBRATION_GEOMETRIC:
        case Calibration::SIZE_CALIBRATION_DIAMETER:
        case Calibration::SIZE_CALIBRATION_BOX:
        case Calibration::SIZE_CALIBRATION_AREA:
            if (mRawPointerAxes.touchMajor.valid && mRawPointerAxes.toolMajor.valid) {
                touchMajor = in.touchMajor;
                touchMinor = mRawPointerAxes.touchMinor.valid ? in.touchMinor : in.touchMajor;
                toolMajor = in.toolMajor;
                toolMinor = mRawPointerAxes.toolMinor.valid ? in.toolMinor : in.toolMajor;
                size = mRawPointerAxes.touchMinor.valid
                        ? avg(in.touchMajor, in.touchMinor) : in.touchMajor;
            } else if (mRawPointerAxes.touchMajor.valid) {
                toolMajor = touchMajor = in.touchMajor;
                toolMinor = touchMinor = mRawPointerAxes.touchMinor.valid
                        ? in.touchMinor : in.touchMajor;
                size = mRawPointerAxes.touchMinor.valid
                        ? avg(in.touchMajor, in.touchMinor) : in.touchMajor;
            } else if (mRawPointerAxes.toolMajor.valid) {
                touchMajor = toolMajor = in.toolMajor;
                touchMinor = toolMinor = mRawPointerAxes.toolMinor.valid
                        ? in.toolMinor : in.toolMajor;
                size = mRawPointerAxes.toolMinor.valid
                        ? avg(in.toolMajor, in.toolMinor) : in.toolMajor;
            } else {
                ALOG_ASSERT(false, "No touch or tool axes.  "
                        "Size calibration should have been resolved to NONE.");
                touchMajor = 0;
                touchMinor = 0;
                toolMajor = 0;
                toolMinor = 0;
                size = 0;
            }

            if (mCalibration.haveSizeIsSummed && mCalibration.sizeIsSummed) {
                uint32_t touchingCount =
                        mCurrentRawState.rawPointerData.touchingIdBits.count();
                if (touchingCount > 1) {
                    touchMajor /= touchingCount;
                    touchMinor /= touchingCount;
                    toolMajor /= touchingCount;
                    toolMinor /= touchingCount;
                    size /= touchingCount;
                }
            }

            if (mCalibration.sizeCalibration == Calibration::SIZE_CALIBRATION_GEOMETRIC) {
                touchMajor *= mGeometricScale;
                touchMinor *= mGeometricScale;
                toolMajor *= mGeometricScale;
                toolMinor *= mGeometricScale;
            } else if (mCalibration.sizeCalibration == Calibration::SIZE_CALIBRATION_AREA) {
                touchMajor = touchMajor > 0 ? sqrtf(touchMajor) : 0;
                touchMinor = touchMajor;
                toolMajor = toolMajor > 0 ? sqrtf(toolMajor) : 0;
                toolMinor = toolMajor;
            } else if (mCalibration.sizeCalibration == Calibration::SIZE_CALIBRATION_DIAMETER) {
                touchMinor = touchMajor;
                toolMinor = toolMajor;
            }

            mCalibration.applySizeScaleAndBias(&touchMajor);
            mCalibration.applySizeScaleAndBias(&touchMinor);
            mCalibration.applySizeScaleAndBias(&toolMajor);
            mCalibration.applySizeScaleAndBias(&toolMinor);
            size *= mSizeScale;
            break;
        default:
            touchMajor = 0;
            touchMinor = 0;
            toolMajor = 0;
            toolMinor = 0;
            size = 0;
            break;
        }

        // Pressure
        float pressure;
        switch (mCalibration.pressureCalibration) {
        case Calibration::PRESSURE_CALIBRATION_PHYSICAL:
        case Calibration::PRESSURE_CALIBRATION_AMPLITUDE:
            pressure = in.pressure * mPressureScale;
            break;
        default:
            pressure = in.isHovering ? 0 : 1;
            break;
        }

        // Tilt and Orientation
        float tilt;
        float orientation;
        if (mHaveTilt) {
            float tiltXAngle = (in.tiltX - mTiltXCenter) * mTiltXScale;
            float tiltYAngle = (in.tiltY - mTiltYCenter) * mTiltYScale;
            orientation = atan2f(-sinf(tiltXAngle), sinf(tiltYAngle));
            tilt = acosf(cosf(tiltXAngle) * cosf(tiltYAngle));
        } else {
            tilt = 0;

            switch (mCalibration.orientationCalibration) {
            case Calibration::ORIENTATION_CALIBRATION_INTERPOLATED:
                orientation = in.orientation * mOrientationScale;
                break;
            case Calibration::ORIENTATION_CALIBRATION_VECTOR: {
                int32_t c1 = signExtendNybble((in.orientation & 0xf0) >> 4);
                int32_t c2 = signExtendNybble(in.orientation & 0x0f);
                if (c1 != 0 || c2 != 0) {
                    orientation = atan2f(c1, c2) * 0.5f;
                    float confidence = hypotf(c1, c2);
                    float scale = 1.0f + confidence / 16.0f;
                    touchMajor *= scale;
                    touchMinor /= scale;
                    toolMajor *= scale;
                    toolMinor /= scale;
                } else {
                    orientation = 0;
                }
                break;
            }
            default:
                orientation = 0;
            }
        }

        // Distance
        float distance;
        switch (mCalibration.distanceCalibration) {
        case Calibration::DISTANCE_CALIBRATION_SCALED:
            distance = in.distance * mDistanceScale;
            break;
        default:
            distance = 0;
        }

        // Coverage
        int32_t rawLeft, rawTop, rawRight, rawBottom;
        switch (mCalibration.coverageCalibration) {
        case Calibration::COVERAGE_CALIBRATION_BOX:
            rawLeft = (in.toolMinor & 0xffff0000) >> 16;
            rawRight = in.toolMinor & 0x0000ffff;
            rawBottom = in.toolMajor & 0x0000ffff;
            rawTop = (in.toolMajor & 0xffff0000) >> 16;
            break;
        default:
            rawLeft = rawTop = rawRight = rawBottom = 0;
            break;
        }

        // Adjust X,Y coords for device calibration
        // TODO: Adjust coverage coords?
        float xTransformed = in.x, yTransformed = in.y;
        mAffineTransform.applyTo(xTransformed, yTransformed);

        // Adjust X, Y, and coverage coords for surface orientation.
        float x, y;
        float left, top, right, bottom;

        switch (mSurfaceOrientation) {
        case DISPLAY_ORIENTATION_90:
            x = float(yTransformed - mRawPointerAxes.y.minValue) * mYScale + mYTranslate;
            y = float(mRawPointerAxes.x.maxValue - xTransformed) * mXScale + mXTranslate;
            left = float(rawTop - mRawPointerAxes.y.minValue) * mYScale + mYTranslate;
            right = float(rawBottom- mRawPointerAxes.y.minValue) * mYScale + mYTranslate;
            bottom = float(mRawPointerAxes.x.maxValue - rawLeft) * mXScale + mXTranslate;
            top = float(mRawPointerAxes.x.maxValue - rawRight) * mXScale + mXTranslate;
            orientation -= M_PI_2;
            if (mOrientedRanges.haveOrientation && orientation < mOrientedRanges.orientation.min) {
                orientation += (mOrientedRanges.orientation.max - mOrientedRanges.orientation.min);
            }
            break;
        case DISPLAY_ORIENTATION_180:
            x = float(mRawPointerAxes.x.maxValue - xTransformed) * mXScale;
            y = float(mRawPointerAxes.y.maxValue - yTransformed) * mYScale + mYTranslate;
            left = float(mRawPointerAxes.x.maxValue - rawRight) * mXScale;
            right = float(mRawPointerAxes.x.maxValue - rawLeft) * mXScale;
            bottom = float(mRawPointerAxes.y.maxValue - rawTop) * mYScale + mYTranslate;
            top = float(mRawPointerAxes.y.maxValue - rawBottom) * mYScale + mYTranslate;
            orientation -= M_PI;
            if (mOrientedRanges.haveOrientation && orientation < mOrientedRanges.orientation.min) {
                orientation += (mOrientedRanges.orientation.max - mOrientedRanges.orientation.min);
            }
            break;
        case DISPLAY_ORIENTATION_270:
            x = float(mRawPointerAxes.y.maxValue - yTransformed) * mYScale;
            y = float(xTransformed - mRawPointerAxes.x.minValue) * mXScale + mXTranslate;
            left = float(mRawPointerAxes.y.maxValue - rawBottom) * mYScale;
            right = float(mRawPointerAxes.y.maxValue - rawTop) * mYScale;
            bottom = float(rawRight - mRawPointerAxes.x.minValue) * mXScale + mXTranslate;
            top = float(rawLeft - mRawPointerAxes.x.minValue) * mXScale + mXTranslate;
            orientation += M_PI_2;
            if (mOrientedRanges.haveOrientation && orientation > mOrientedRanges.orientation.max) {
                orientation -= (mOrientedRanges.orientation.max - mOrientedRanges.orientation.min);
            }
            break;
        default:
            x = float(xTransformed - mRawPointerAxes.x.minValue) * mXScale + mXTranslate;
            y = float(yTransformed - mRawPointerAxes.y.minValue) * mYScale + mYTranslate;
            left = float(rawLeft - mRawPointerAxes.x.minValue) * mXScale + mXTranslate;
            right = float(rawRight - mRawPointerAxes.x.minValue) * mXScale + mXTranslate;
            bottom = float(rawBottom - mRawPointerAxes.y.minValue) * mYScale + mYTranslate;
            top = float(rawTop - mRawPointerAxes.y.minValue) * mYScale + mYTranslate;
            break;
        }

        // Write output coords.
        PointerCoords& out = mCurrentCookedState.cookedPointerData.pointerCoords[i];
        out.clear();
        out.setAxisValue(AMOTION_EVENT_AXIS_X, x);
        out.setAxisValue(AMOTION_EVENT_AXIS_Y, y);
        out.setAxisValue(AMOTION_EVENT_AXIS_PRESSURE, pressure);
        out.setAxisValue(AMOTION_EVENT_AXIS_SIZE, size);
        out.setAxisValue(AMOTION_EVENT_AXIS_TOUCH_MAJOR, touchMajor);
        out.setAxisValue(AMOTION_EVENT_AXIS_TOUCH_MINOR, touchMinor);
        out.setAxisValue(AMOTION_EVENT_AXIS_ORIENTATION, orientation);
        out.setAxisValue(AMOTION_EVENT_AXIS_TILT, tilt);
        out.setAxisValue(AMOTION_EVENT_AXIS_DISTANCE, distance);
        if (mCalibration.coverageCalibration == Calibration::COVERAGE_CALIBRATION_BOX) {
            out.setAxisValue(AMOTION_EVENT_AXIS_GENERIC_1, left);
            out.setAxisValue(AMOTION_EVENT_AXIS_GENERIC_2, top);
            out.setAxisValue(AMOTION_EVENT_AXIS_GENERIC_3, right);
            out.setAxisValue(AMOTION_EVENT_AXIS_GENERIC_4, bottom);
        } else {
            out.setAxisValue(AMOTION_EVENT_AXIS_TOOL_MAJOR, toolMajor);
            out.setAxisValue(AMOTION_EVENT_AXIS_TOOL_MINOR, toolMinor);
        }

        // Write output properties.
        PointerProperties& properties =
                mCurrentCookedState.cookedPointerData.pointerProperties[i];
        uint32_t id = in.id;
        properties.clear();
        properties.id = id;
        properties.toolType = in.toolType;

        // Write id index.
        mCurrentCookedState.cookedPointerData.idToIndex[id] = i;
    }
}

void TouchInputMapper::dispatchPointerUsage(nsecs_t when, uint32_t policyFlags,
        PointerUsage pointerUsage) {
    if (pointerUsage != mPointerUsage) {
        abortPointerUsage(when, policyFlags);
        mPointerUsage = pointerUsage;
    }

    switch (mPointerUsage) {
    case POINTER_USAGE_GESTURES:
        dispatchPointerGestures(when, policyFlags, false /*isTimeout*/);
        break;
    case POINTER_USAGE_STYLUS:
        dispatchPointerStylus(when, policyFlags);
        break;
    case POINTER_USAGE_MOUSE:
        dispatchPointerMouse(when, policyFlags);
        break;
    default:
        break;
    }
}

void TouchInputMapper::abortPointerUsage(nsecs_t when, uint32_t policyFlags) {
    switch (mPointerUsage) {
    case POINTER_USAGE_GESTURES:
        abortPointerGestures(when, policyFlags);
        break;
    case POINTER_USAGE_STYLUS:
        abortPointerStylus(when, policyFlags);
        break;
    case POINTER_USAGE_MOUSE:
        abortPointerMouse(when, policyFlags);
        break;
    default:
        break;
    }

    mPointerUsage = POINTER_USAGE_NONE;
}

void TouchInputMapper::dispatchPointerGestures(nsecs_t when, uint32_t policyFlags,
        bool isTimeout) {
    // Update current gesture coordinates.
    bool cancelPreviousGesture, finishPreviousGesture;
    bool sendEvents = preparePointerGestures(when,
            &cancelPreviousGesture, &finishPreviousGesture, isTimeout);
    if (!sendEvents) {
        return;
    }
    if (finishPreviousGesture) {
        cancelPreviousGesture = false;
    }

    // Update the pointer presentation and spots.
    if (mParameters.gestureMode == Parameters::GESTURE_MODE_MULTI_TOUCH) {
        mPointerController->setPresentation(PointerControllerInterface::PRESENTATION_POINTER);
        if (finishPreviousGesture || cancelPreviousGesture) {
            mPointerController->clearSpots();
        }

        if (mPointerGesture.currentGestureMode == PointerGesture::FREEFORM) {
            mPointerController->setSpots(mPointerGesture.currentGestureCoords,
                     mPointerGesture.currentGestureIdToIndex,
                     mPointerGesture.currentGestureIdBits,
                     mPointerController->getDisplayId());
        }
    } else {
        mPointerController->setPresentation(PointerControllerInterface::PRESENTATION_POINTER);
    }

    // Show or hide the pointer if needed.
    switch (mPointerGesture.currentGestureMode) {
    case PointerGesture::NEUTRAL:
    case PointerGesture::QUIET:
        if (mParameters.gestureMode == Parameters::GESTURE_MODE_MULTI_TOUCH
                && mPointerGesture.lastGestureMode == PointerGesture::FREEFORM) {
            // Remind the user of where the pointer is after finishing a gesture with spots.
            mPointerController->unfade(PointerControllerInterface::TRANSITION_GRADUAL);
        }
        break;
    case PointerGesture::TAP:
    case PointerGesture::TAP_DRAG:
    case PointerGesture::BUTTON_CLICK_OR_DRAG:
    case PointerGesture::HOVER:
    case PointerGesture::PRESS:
    case PointerGesture::SWIPE:
        // Unfade the pointer when the current gesture manipulates the
        // area directly under the pointer.
        mPointerController->unfade(PointerControllerInterface::TRANSITION_IMMEDIATE);
        break;
    case PointerGesture::FREEFORM:
        // Fade the pointer when the current gesture manipulates a different
        // area and there are spots to guide the user experience.
        if (mParameters.gestureMode == Parameters::GESTURE_MODE_MULTI_TOUCH) {
            mPointerController->fade(PointerControllerInterface::TRANSITION_GRADUAL);
        } else {
            mPointerController->unfade(PointerControllerInterface::TRANSITION_IMMEDIATE);
        }
        break;
    }

    // Send events!
    int32_t metaState = getContext()->getGlobalMetaState();
    int32_t buttonState = mCurrentCookedState.buttonState;

    // Update last coordinates of pointers that have moved so that we observe the new
    // pointer positions at the same time as other pointers that have just gone up.
    bool down = mPointerGesture.currentGestureMode == PointerGesture::TAP
            || mPointerGesture.currentGestureMode == PointerGesture::TAP_DRAG
            || mPointerGesture.currentGestureMode == PointerGesture::BUTTON_CLICK_OR_DRAG
            || mPointerGesture.currentGestureMode == PointerGesture::PRESS
            || mPointerGesture.currentGestureMode == PointerGesture::SWIPE
            || mPointerGesture.currentGestureMode == PointerGesture::FREEFORM;
    bool moveNeeded = false;
    if (down && !cancelPreviousGesture && !finishPreviousGesture
            && !mPointerGesture.lastGestureIdBits.isEmpty()
            && !mPointerGesture.currentGestureIdBits.isEmpty()) {
        BitSet32 movedGestureIdBits(mPointerGesture.currentGestureIdBits.value
                & mPointerGesture.lastGestureIdBits.value);
        moveNeeded = updateMovedPointers(mPointerGesture.currentGestureProperties,
                mPointerGesture.currentGestureCoords, mPointerGesture.currentGestureIdToIndex,
                mPointerGesture.lastGestureProperties,
                mPointerGesture.lastGestureCoords, mPointerGesture.lastGestureIdToIndex,
                movedGestureIdBits);
        if (buttonState != mLastCookedState.buttonState) {
            moveNeeded = true;
        }
    }

    // Send motion events for all pointers that went up or were canceled.
    BitSet32 dispatchedGestureIdBits(mPointerGesture.lastGestureIdBits);
    if (!dispatchedGestureIdBits.isEmpty()) {
        if (cancelPreviousGesture) {
            dispatchMotion(when, policyFlags, mSource,
                    AMOTION_EVENT_ACTION_CANCEL, 0, 0, metaState, buttonState,
                    AMOTION_EVENT_EDGE_FLAG_NONE, /* deviceTimestamp */ 0,
                    mPointerGesture.lastGestureProperties,
                    mPointerGesture.lastGestureCoords, mPointerGesture.lastGestureIdToIndex,
                    dispatchedGestureIdBits, -1, 0,
                    0, mPointerGesture.downTime);

            dispatchedGestureIdBits.clear();
        } else {
            BitSet32 upGestureIdBits;
            if (finishPreviousGesture) {
                upGestureIdBits = dispatchedGestureIdBits;
            } else {
                upGestureIdBits.value = dispatchedGestureIdBits.value
                        & ~mPointerGesture.currentGestureIdBits.value;
            }
            while (!upGestureIdBits.isEmpty()) {
                uint32_t id = upGestureIdBits.clearFirstMarkedBit();

                dispatchMotion(when, policyFlags, mSource,
                        AMOTION_EVENT_ACTION_POINTER_UP, 0, 0,
                        metaState, buttonState, AMOTION_EVENT_EDGE_FLAG_NONE,
                        /* deviceTimestamp */ 0,
                        mPointerGesture.lastGestureProperties,
                        mPointerGesture.lastGestureCoords, mPointerGesture.lastGestureIdToIndex,
                        dispatchedGestureIdBits, id,
                        0, 0, mPointerGesture.downTime);

                dispatchedGestureIdBits.clearBit(id);
            }
        }
    }

    // Send motion events for all pointers that moved.
    if (moveNeeded) {
        dispatchMotion(when, policyFlags, mSource,
                AMOTION_EVENT_ACTION_MOVE, 0, 0, metaState, buttonState,
                AMOTION_EVENT_EDGE_FLAG_NONE, /* deviceTimestamp */ 0,
                mPointerGesture.currentGestureProperties,
                mPointerGesture.currentGestureCoords, mPointerGesture.currentGestureIdToIndex,
                dispatchedGestureIdBits, -1,
                0, 0, mPointerGesture.downTime);
    }

    // Send motion events for all pointers that went down.
    if (down) {
        BitSet32 downGestureIdBits(mPointerGesture.currentGestureIdBits.value
                & ~dispatchedGestureIdBits.value);
        while (!downGestureIdBits.isEmpty()) {
            uint32_t id = downGestureIdBits.clearFirstMarkedBit();
            dispatchedGestureIdBits.markBit(id);

            if (dispatchedGestureIdBits.count() == 1) {
                mPointerGesture.downTime = when;
            }

            dispatchMotion(when, policyFlags, mSource,
                    AMOTION_EVENT_ACTION_POINTER_DOWN, 0, 0, metaState, buttonState, 0,
                    /* deviceTimestamp */ 0,
                    mPointerGesture.currentGestureProperties,
                    mPointerGesture.currentGestureCoords, mPointerGesture.currentGestureIdToIndex,
                    dispatchedGestureIdBits, id,
                    0, 0, mPointerGesture.downTime);
        }
    }

    // Send motion events for hover.
    if (mPointerGesture.currentGestureMode == PointerGesture::HOVER) {
        dispatchMotion(when, policyFlags, mSource,
                AMOTION_EVENT_ACTION_HOVER_MOVE, 0, 0,
                metaState, buttonState, AMOTION_EVENT_EDGE_FLAG_NONE, /* deviceTimestamp */ 0,
                mPointerGesture.currentGestureProperties,
                mPointerGesture.currentGestureCoords, mPointerGesture.currentGestureIdToIndex,
                mPointerGesture.currentGestureIdBits, -1,
                0, 0, mPointerGesture.downTime);
    } else if (dispatchedGestureIdBits.isEmpty()
            && !mPointerGesture.lastGestureIdBits.isEmpty()) {
        // Synthesize a hover move event after all pointers go up to indicate that
        // the pointer is hovering again even if the user is not currently touching
        // the touch pad.  This ensures that a view will receive a fresh hover enter
        // event after a tap.
        float x, y;
        mPointerController->getPosition(&x, &y);

        PointerProperties pointerProperties;
        pointerProperties.clear();
        pointerProperties.id = 0;
        pointerProperties.toolType = AMOTION_EVENT_TOOL_TYPE_FINGER;

        PointerCoords pointerCoords;
        pointerCoords.clear();
        pointerCoords.setAxisValue(AMOTION_EVENT_AXIS_X, x);
        pointerCoords.setAxisValue(AMOTION_EVENT_AXIS_Y, y);

        const int32_t displayId = mPointerController->getDisplayId();
        NotifyMotionArgs args(mContext->getNextSequenceNum(), when, getDeviceId(),
                mSource, displayId, policyFlags,
                AMOTION_EVENT_ACTION_HOVER_MOVE, 0, 0,
                metaState, buttonState, MotionClassification::NONE, AMOTION_EVENT_EDGE_FLAG_NONE,
                /* deviceTimestamp */ 0, 1, &pointerProperties, &pointerCoords,
                0, 0, mPointerGesture.downTime, /* videoFrames */ {});
        getListener()->notifyMotion(&args);
    }

    // Update state.
    mPointerGesture.lastGestureMode = mPointerGesture.currentGestureMode;
    if (!down) {
        mPointerGesture.lastGestureIdBits.clear();
    } else {
        mPointerGesture.lastGestureIdBits = mPointerGesture.currentGestureIdBits;
        for (BitSet32 idBits(mPointerGesture.currentGestureIdBits); !idBits.isEmpty(); ) {
            uint32_t id = idBits.clearFirstMarkedBit();
            uint32_t index = mPointerGesture.currentGestureIdToIndex[id];
            mPointerGesture.lastGestureProperties[index].copyFrom(
                    mPointerGesture.currentGestureProperties[index]);
            mPointerGesture.lastGestureCoords[index].copyFrom(
                    mPointerGesture.currentGestureCoords[index]);
            mPointerGesture.lastGestureIdToIndex[id] = index;
        }
    }
}

void TouchInputMapper::abortPointerGestures(nsecs_t when, uint32_t policyFlags) {
    // Cancel previously dispatches pointers.
    if (!mPointerGesture.lastGestureIdBits.isEmpty()) {
        int32_t metaState = getContext()->getGlobalMetaState();
        int32_t buttonState = mCurrentRawState.buttonState;
        dispatchMotion(when, policyFlags, mSource,
                AMOTION_EVENT_ACTION_CANCEL, 0, 0, metaState, buttonState,
                AMOTION_EVENT_EDGE_FLAG_NONE, /* deviceTimestamp */ 0,
                mPointerGesture.lastGestureProperties,
                mPointerGesture.lastGestureCoords, mPointerGesture.lastGestureIdToIndex,
                mPointerGesture.lastGestureIdBits, -1,
                0, 0, mPointerGesture.downTime);
    }

    // Reset the current pointer gesture.
    mPointerGesture.reset();
    mPointerVelocityControl.reset();

    // Remove any current spots.
    if (mPointerController != nullptr) {
        mPointerController->fade(PointerControllerInterface::TRANSITION_GRADUAL);
        mPointerController->clearSpots();
    }
}

bool TouchInputMapper::preparePointerGestures(nsecs_t when,
        bool* outCancelPreviousGesture, bool* outFinishPreviousGesture, bool isTimeout) {
    *outCancelPreviousGesture = false;
    *outFinishPreviousGesture = false;

    // Handle TAP timeout.
    if (isTimeout) {
#if DEBUG_GESTURES
        ALOGD("Gestures: Processing timeout");
#endif

        if (mPointerGesture.lastGestureMode == PointerGesture::TAP) {
            if (when <= mPointerGesture.tapUpTime + mConfig.pointerGestureTapDragInterval) {
                // The tap/drag timeout has not yet expired.
                getContext()->requestTimeoutAtTime(mPointerGesture.tapUpTime
                        + mConfig.pointerGestureTapDragInterval);
            } else {
                // The tap is finished.
#if DEBUG_GESTURES
                ALOGD("Gestures: TAP finished");
#endif
                *outFinishPreviousGesture = true;

                mPointerGesture.activeGestureId = -1;
                mPointerGesture.currentGestureMode = PointerGesture::NEUTRAL;
                mPointerGesture.currentGestureIdBits.clear();

                mPointerVelocityControl.reset();
                return true;
            }
        }

        // We did not handle this timeout.
        return false;
    }

    const uint32_t currentFingerCount = mCurrentCookedState.fingerIdBits.count();
    const uint32_t lastFingerCount = mLastCookedState.fingerIdBits.count();

    // Update the velocity tracker.
    {
        VelocityTracker::Position positions[MAX_POINTERS];
        uint32_t count = 0;
        for (BitSet32 idBits(mCurrentCookedState.fingerIdBits); !idBits.isEmpty(); count++) {
            uint32_t id = idBits.clearFirstMarkedBit();
            const RawPointerData::Pointer& pointer =
                    mCurrentRawState.rawPointerData.pointerForId(id);
            positions[count].x = pointer.x * mPointerXMovementScale;
            positions[count].y = pointer.y * mPointerYMovementScale;
        }
        mPointerGesture.velocityTracker.addMovement(when,
                mCurrentCookedState.fingerIdBits, positions);
    }

    // If the gesture ever enters a mode other than TAP, HOVER or TAP_DRAG, without first returning
    // to NEUTRAL, then we should not generate tap event.
    if (mPointerGesture.lastGestureMode != PointerGesture::HOVER
            && mPointerGesture.lastGestureMode != PointerGesture::TAP
            && mPointerGesture.lastGestureMode != PointerGesture::TAP_DRAG) {
        mPointerGesture.resetTap();
    }

    // Pick a new active touch id if needed.
    // Choose an arbitrary pointer that just went down, if there is one.
    // Otherwise choose an arbitrary remaining pointer.
    // This guarantees we always have an active touch id when there is at least one pointer.
    // We keep the same active touch id for as long as possible.
    int32_t lastActiveTouchId = mPointerGesture.activeTouchId;
    int32_t activeTouchId = lastActiveTouchId;
    if (activeTouchId < 0) {
        if (!mCurrentCookedState.fingerIdBits.isEmpty()) {
            activeTouchId = mPointerGesture.activeTouchId =
                    mCurrentCookedState.fingerIdBits.firstMarkedBit();
            mPointerGesture.firstTouchTime = when;
        }
    } else if (!mCurrentCookedState.fingerIdBits.hasBit(activeTouchId)) {
        if (!mCurrentCookedState.fingerIdBits.isEmpty()) {
            activeTouchId = mPointerGesture.activeTouchId =
                    mCurrentCookedState.fingerIdBits.firstMarkedBit();
        } else {
            activeTouchId = mPointerGesture.activeTouchId = -1;
        }
    }

    // Determine whether we are in quiet time.
    bool isQuietTime = false;
    if (activeTouchId < 0) {
        mPointerGesture.resetQuietTime();
    } else {
        isQuietTime = when < mPointerGesture.quietTime + mConfig.pointerGestureQuietInterval;
        if (!isQuietTime) {
            if ((mPointerGesture.lastGestureMode == PointerGesture::PRESS
                    || mPointerGesture.lastGestureMode == PointerGesture::SWIPE
                    || mPointerGesture.lastGestureMode == PointerGesture::FREEFORM)
                    && currentFingerCount < 2) {
                // Enter quiet time when exiting swipe or freeform state.
                // This is to prevent accidentally entering the hover state and flinging the
                // pointer when finishing a swipe and there is still one pointer left onscreen.
                isQuietTime = true;
            } else if (mPointerGesture.lastGestureMode == PointerGesture::BUTTON_CLICK_OR_DRAG
                    && currentFingerCount >= 2
                    && !isPointerDown(mCurrentRawState.buttonState)) {
                // Enter quiet time when releasing the button and there are still two or more
                // fingers down.  This may indicate that one finger was used to press the button
                // but it has not gone up yet.
                isQuietTime = true;
            }
            if (isQuietTime) {
                mPointerGesture.quietTime = when;
            }
        }
    }

    // Switch states based on button and pointer state.
    if (isQuietTime) {
        // Case 1: Quiet time. (QUIET)
#if DEBUG_GESTURES
        ALOGD("Gestures: QUIET for next %0.3fms", (mPointerGesture.quietTime
                + mConfig.pointerGestureQuietInterval - when) * 0.000001f);
#endif
        if (mPointerGesture.lastGestureMode != PointerGesture::QUIET) {
            *outFinishPreviousGesture = true;
        }

        mPointerGesture.activeGestureId = -1;
        mPointerGesture.currentGestureMode = PointerGesture::QUIET;
        mPointerGesture.currentGestureIdBits.clear();

        mPointerVelocityControl.reset();
    } else if (isPointerDown(mCurrentRawState.buttonState)) {
        // Case 2: Button is pressed. (BUTTON_CLICK_OR_DRAG)
        // The pointer follows the active touch point.
        // Emit DOWN, MOVE, UP events at the pointer location.
        //
        // Only the active touch matters; other fingers are ignored.  This policy helps
        // to handle the case where the user places a second finger on the touch pad
        // to apply the necessary force to depress an integrated button below the surface.
        // We don't want the second finger to be delivered to applications.
        //
        // For this to work well, we need to make sure to track the pointer that is really
        // active.  If the user first puts one finger down to click then adds another
        // finger to drag then the active pointer should switch to the finger that is
        // being dragged.
#if DEBUG_GESTURES
        ALOGD("Gestures: BUTTON_CLICK_OR_DRAG activeTouchId=%d, "
                "currentFingerCount=%d", activeTouchId, currentFingerCount);
#endif
        // Reset state when just starting.
        if (mPointerGesture.lastGestureMode != PointerGesture::BUTTON_CLICK_OR_DRAG) {
            *outFinishPreviousGesture = true;
            mPointerGesture.activeGestureId = 0;
        }

        // Switch pointers if needed.
        // Find the fastest pointer and follow it.
        if (activeTouchId >= 0 && currentFingerCount > 1) {
            int32_t bestId = -1;
            float bestSpeed = mConfig.pointerGestureDragMinSwitchSpeed;
            for (BitSet32 idBits(mCurrentCookedState.fingerIdBits); !idBits.isEmpty(); ) {
                uint32_t id = idBits.clearFirstMarkedBit();
                float vx, vy;
                if (mPointerGesture.velocityTracker.getVelocity(id, &vx, &vy)) {
                    float speed = hypotf(vx, vy);
                    if (speed > bestSpeed) {
                        bestId = id;
                        bestSpeed = speed;
                    }
                }
            }
            if (bestId >= 0 && bestId != activeTouchId) {
                mPointerGesture.activeTouchId = activeTouchId = bestId;
#if DEBUG_GESTURES
                ALOGD("Gestures: BUTTON_CLICK_OR_DRAG switched pointers, "
                        "bestId=%d, bestSpeed=%0.3f", bestId, bestSpeed);
#endif
            }
        }

        float deltaX = 0, deltaY = 0;
        if (activeTouchId >= 0 && mLastCookedState.fingerIdBits.hasBit(activeTouchId)) {
            const RawPointerData::Pointer& currentPointer =
                    mCurrentRawState.rawPointerData.pointerForId(activeTouchId);
            const RawPointerData::Pointer& lastPointer =
                    mLastRawState.rawPointerData.pointerForId(activeTouchId);
            deltaX = (currentPointer.x - lastPointer.x) * mPointerXMovementScale;
            deltaY = (currentPointer.y - lastPointer.y) * mPointerYMovementScale;

            rotateDelta(mSurfaceOrientation, &deltaX, &deltaY);
            mPointerVelocityControl.move(when, &deltaX, &deltaY);

            // Move the pointer using a relative motion.
            // When using spots, the click will occur at the position of the anchor
            // spot and all other spots will move there.
            mPointerController->move(deltaX, deltaY);
        } else {
            mPointerVelocityControl.reset();
        }

        float x, y;
        mPointerController->getPosition(&x, &y);

        mPointerGesture.currentGestureMode = PointerGesture::BUTTON_CLICK_OR_DRAG;
        mPointerGesture.currentGestureIdBits.clear();
        mPointerGesture.currentGestureIdBits.markBit(mPointerGesture.activeGestureId);
        mPointerGesture.currentGestureIdToIndex[mPointerGesture.activeGestureId] = 0;
        mPointerGesture.currentGestureProperties[0].clear();
        mPointerGesture.currentGestureProperties[0].id = mPointerGesture.activeGestureId;
        mPointerGesture.currentGestureProperties[0].toolType = AMOTION_EVENT_TOOL_TYPE_FINGER;
        mPointerGesture.currentGestureCoords[0].clear();
        mPointerGesture.currentGestureCoords[0].setAxisValue(AMOTION_EVENT_AXIS_X, x);
        mPointerGesture.currentGestureCoords[0].setAxisValue(AMOTION_EVENT_AXIS_Y, y);
        mPointerGesture.currentGestureCoords[0].setAxisValue(AMOTION_EVENT_AXIS_PRESSURE, 1.0f);
    } else if (currentFingerCount == 0) {
        // Case 3. No fingers down and button is not pressed. (NEUTRAL)
        if (mPointerGesture.lastGestureMode != PointerGesture::NEUTRAL) {
            *outFinishPreviousGesture = true;
        }

        // Watch for taps coming out of HOVER or TAP_DRAG mode.
        // Checking for taps after TAP_DRAG allows us to detect double-taps.
        bool tapped = false;
        if ((mPointerGesture.lastGestureMode == PointerGesture::HOVER
                || mPointerGesture.lastGestureMode == PointerGesture::TAP_DRAG)
                && lastFingerCount == 1) {
            if (when <= mPointerGesture.tapDownTime + mConfig.pointerGestureTapInterval) {
                float x, y;
                mPointerController->getPosition(&x, &y);
                if (fabs(x - mPointerGesture.tapX) <= mConfig.pointerGestureTapSlop
                        && fabs(y - mPointerGesture.tapY) <= mConfig.pointerGestureTapSlop) {
#if DEBUG_GESTURES
                    ALOGD("Gestures: TAP");
#endif

                    mPointerGesture.tapUpTime = when;
                    getContext()->requestTimeoutAtTime(when
                            + mConfig.pointerGestureTapDragInterval);

                    mPointerGesture.activeGestureId = 0;
                    mPointerGesture.currentGestureMode = PointerGesture::TAP;
                    mPointerGesture.currentGestureIdBits.clear();
                    mPointerGesture.currentGestureIdBits.markBit(
                            mPointerGesture.activeGestureId);
                    mPointerGesture.currentGestureIdToIndex[
                            mPointerGesture.activeGestureId] = 0;
                    mPointerGesture.currentGestureProperties[0].clear();
                    mPointerGesture.currentGestureProperties[0].id =
                            mPointerGesture.activeGestureId;
                    mPointerGesture.currentGestureProperties[0].toolType =
                            AMOTION_EVENT_TOOL_TYPE_FINGER;
                    mPointerGesture.currentGestureCoords[0].clear();
                    mPointerGesture.currentGestureCoords[0].setAxisValue(
                            AMOTION_EVENT_AXIS_X, mPointerGesture.tapX);
                    mPointerGesture.currentGestureCoords[0].setAxisValue(
                            AMOTION_EVENT_AXIS_Y, mPointerGesture.tapY);
                    mPointerGesture.currentGestureCoords[0].setAxisValue(
                            AMOTION_EVENT_AXIS_PRESSURE, 1.0f);

                    tapped = true;
                } else {
#if DEBUG_GESTURES
                    ALOGD("Gestures: Not a TAP, deltaX=%f, deltaY=%f",
                            x - mPointerGesture.tapX,
                            y - mPointerGesture.tapY);
#endif
                }
            } else {
#if DEBUG_GESTURES
                if (mPointerGesture.tapDownTime != LLONG_MIN) {
                    ALOGD("Gestures: Not a TAP, %0.3fms since down",
                            (when - mPointerGesture.tapDownTime) * 0.000001f);
                } else {
                    ALOGD("Gestures: Not a TAP, incompatible mode transitions");
                }
#endif
            }
        }

        mPointerVelocityControl.reset();

        if (!tapped) {
#if DEBUG_GESTURES
            ALOGD("Gestures: NEUTRAL");
#endif
            mPointerGesture.activeGestureId = -1;
            mPointerGesture.currentGestureMode = PointerGesture::NEUTRAL;
            mPointerGesture.currentGestureIdBits.clear();
        }
    } else if (currentFingerCount == 1) {
        // Case 4. Exactly one finger down, button is not pressed. (HOVER or TAP_DRAG)
        // The pointer follows the active touch point.
        // When in HOVER, emit HOVER_MOVE events at the pointer location.
        // When in TAP_DRAG, emit MOVE events at the pointer location.
        ALOG_ASSERT(activeTouchId >= 0);

        mPointerGesture.currentGestureMode = PointerGesture::HOVER;
        if (mPointerGesture.lastGestureMode == PointerGesture::TAP) {
            if (when <= mPointerGesture.tapUpTime + mConfig.pointerGestureTapDragInterval) {
                float x, y;
                mPointerController->getPosition(&x, &y);
                if (fabs(x - mPointerGesture.tapX) <= mConfig.pointerGestureTapSlop
                        && fabs(y - mPointerGesture.tapY) <= mConfig.pointerGestureTapSlop) {
                    mPointerGesture.currentGestureMode = PointerGesture::TAP_DRAG;
                } else {
#if DEBUG_GESTURES
                    ALOGD("Gestures: Not a TAP_DRAG, deltaX=%f, deltaY=%f",
                            x - mPointerGesture.tapX,
                            y - mPointerGesture.tapY);
#endif
                }
            } else {
#if DEBUG_GESTURES
                ALOGD("Gestures: Not a TAP_DRAG, %0.3fms time since up",
                        (when - mPointerGesture.tapUpTime) * 0.000001f);
#endif
            }
        } else if (mPointerGesture.lastGestureMode == PointerGesture::TAP_DRAG) {
            mPointerGesture.currentGestureMode = PointerGesture::TAP_DRAG;
        }

        float deltaX = 0, deltaY = 0;
        if (mLastCookedState.fingerIdBits.hasBit(activeTouchId)) {
            const RawPointerData::Pointer& currentPointer =
                    mCurrentRawState.rawPointerData.pointerForId(activeTouchId);
            const RawPointerData::Pointer& lastPointer =
                    mLastRawState.rawPointerData.pointerForId(activeTouchId);
            deltaX = (currentPointer.x - lastPointer.x) * mPointerXMovementScale;
            deltaY = (currentPointer.y - lastPointer.y) * mPointerYMovementScale;

            rotateDelta(mSurfaceOrientation, &deltaX, &deltaY);
            mPointerVelocityControl.move(when, &deltaX, &deltaY);

            // Move the pointer using a relative motion.
            // When using spots, the hover or drag will occur at the position of the anchor spot.
            mPointerController->move(deltaX, deltaY);
        } else {
            mPointerVelocityControl.reset();
        }

        bool down;
        if (mPointerGesture.currentGestureMode == PointerGesture::TAP_DRAG) {
#if DEBUG_GESTURES
            ALOGD("Gestures: TAP_DRAG");
#endif
            down = true;
        } else {
#if DEBUG_GESTURES
            ALOGD("Gestures: HOVER");
#endif
            if (mPointerGesture.lastGestureMode != PointerGesture::HOVER) {
                *outFinishPreviousGesture = true;
            }
            mPointerGesture.activeGestureId = 0;
            down = false;
        }

        float x, y;
        mPointerController->getPosition(&x, &y);

        mPointerGesture.currentGestureIdBits.clear();
        mPointerGesture.currentGestureIdBits.markBit(mPointerGesture.activeGestureId);
        mPointerGesture.currentGestureIdToIndex[mPointerGesture.activeGestureId] = 0;
        mPointerGesture.currentGestureProperties[0].clear();
        mPointerGesture.currentGestureProperties[0].id = mPointerGesture.activeGestureId;
        mPointerGesture.currentGestureProperties[0].toolType =
                AMOTION_EVENT_TOOL_TYPE_FINGER;
        mPointerGesture.currentGestureCoords[0].clear();
        mPointerGesture.currentGestureCoords[0].setAxisValue(AMOTION_EVENT_AXIS_X, x);
        mPointerGesture.currentGestureCoords[0].setAxisValue(AMOTION_EVENT_AXIS_Y, y);
        mPointerGesture.currentGestureCoords[0].setAxisValue(AMOTION_EVENT_AXIS_PRESSURE,
                down ? 1.0f : 0.0f);

        if (lastFingerCount == 0 && currentFingerCount != 0) {
            mPointerGesture.resetTap();
            mPointerGesture.tapDownTime = when;
            mPointerGesture.tapX = x;
            mPointerGesture.tapY = y;
        }
    } else {
        // Case 5. At least two fingers down, button is not pressed. (PRESS, SWIPE or FREEFORM)
        // We need to provide feedback for each finger that goes down so we cannot wait
        // for the fingers to move before deciding what to do.
        //
        // The ambiguous case is deciding what to do when there are two fingers down but they
        // have not moved enough to determine whether they are part of a drag or part of a
        // freeform gesture, or just a press or long-press at the pointer location.
        //
        // When there are two fingers we start with the PRESS hypothesis and we generate a
        // down at the pointer location.
        //
        // When the two fingers move enough or when additional fingers are added, we make
        // a decision to transition into SWIPE or FREEFORM mode accordingly.
        ALOG_ASSERT(activeTouchId >= 0);

        bool settled = when >= mPointerGesture.firstTouchTime
                + mConfig.pointerGestureMultitouchSettleInterval;
        if (mPointerGesture.lastGestureMode != PointerGesture::PRESS
                && mPointerGesture.lastGestureMode != PointerGesture::SWIPE
                && mPointerGesture.lastGestureMode != PointerGesture::FREEFORM) {
            *outFinishPreviousGesture = true;
        } else if (!settled && currentFingerCount > lastFingerCount) {
            // Additional pointers have gone down but not yet settled.
            // Reset the gesture.
#if DEBUG_GESTURES
            ALOGD("Gestures: Resetting gesture since additional pointers went down for MULTITOUCH, "
                    "settle time remaining %0.3fms", (mPointerGesture.firstTouchTime
                            + mConfig.pointerGestureMultitouchSettleInterval - when)
                            * 0.000001f);
#endif
            *outCancelPreviousGesture = true;
        } else {
            // Continue previous gesture.
            mPointerGesture.currentGestureMode = mPointerGesture.lastGestureMode;
        }

        if (*outFinishPreviousGesture || *outCancelPreviousGesture) {
            mPointerGesture.currentGestureMode = PointerGesture::PRESS;
            mPointerGesture.activeGestureId = 0;
            mPointerGesture.referenceIdBits.clear();
            mPointerVelocityControl.reset();

            // Use the centroid and pointer location as the reference points for the gesture.
#if DEBUG_GESTURES
            ALOGD("Gestures: Using centroid as reference for MULTITOUCH, "
                    "settle time remaining %0.3fms", (mPointerGesture.firstTouchTime
                            + mConfig.pointerGestureMultitouchSettleInterval - when)
                            * 0.000001f);
#endif
            mCurrentRawState.rawPointerData.getCentroidOfTouchingPointers(
                    &mPointerGesture.referenceTouchX,
                    &mPointerGesture.referenceTouchY);
            mPointerController->getPosition(&mPointerGesture.referenceGestureX,
                    &mPointerGesture.referenceGestureY);
        }

        // Clear the reference deltas for fingers not yet included in the reference calculation.
        for (BitSet32 idBits(mCurrentCookedState.fingerIdBits.value
                & ~mPointerGesture.referenceIdBits.value); !idBits.isEmpty(); ) {
            uint32_t id = idBits.clearFirstMarkedBit();
            mPointerGesture.referenceDeltas[id].dx = 0;
            mPointerGesture.referenceDeltas[id].dy = 0;
        }
        mPointerGesture.referenceIdBits = mCurrentCookedState.fingerIdBits;

        // Add delta for all fingers and calculate a common movement delta.
        float commonDeltaX = 0, commonDeltaY = 0;
        BitSet32 commonIdBits(mLastCookedState.fingerIdBits.value
                & mCurrentCookedState.fingerIdBits.value);
        for (BitSet32 idBits(commonIdBits); !idBits.isEmpty(); ) {
            bool first = (idBits == commonIdBits);
            uint32_t id = idBits.clearFirstMarkedBit();
            const RawPointerData::Pointer& cpd = mCurrentRawState.rawPointerData.pointerForId(id);
            const RawPointerData::Pointer& lpd = mLastRawState.rawPointerData.pointerForId(id);
            PointerGesture::Delta& delta = mPointerGesture.referenceDeltas[id];
            delta.dx += cpd.x - lpd.x;
            delta.dy += cpd.y - lpd.y;

            if (first) {
                commonDeltaX = delta.dx;
                commonDeltaY = delta.dy;
            } else {
                commonDeltaX = calculateCommonVector(commonDeltaX, delta.dx);
                commonDeltaY = calculateCommonVector(commonDeltaY, delta.dy);
            }
        }

        // Consider transitions from PRESS to SWIPE or MULTITOUCH.
        if (mPointerGesture.currentGestureMode == PointerGesture::PRESS) {
            float dist[MAX_POINTER_ID + 1];
            int32_t distOverThreshold = 0;
            for (BitSet32 idBits(mPointerGesture.referenceIdBits); !idBits.isEmpty(); ) {
                uint32_t id = idBits.clearFirstMarkedBit();
                PointerGesture::Delta& delta = mPointerGesture.referenceDeltas[id];
                dist[id] = hypotf(delta.dx * mPointerXZoomScale,
                        delta.dy * mPointerYZoomScale);
                if (dist[id] > mConfig.pointerGestureMultitouchMinDistance) {
                    distOverThreshold += 1;
                }
            }

            // Only transition when at least two pointers have moved further than
            // the minimum distance threshold.
            if (distOverThreshold >= 2) {
                if (currentFingerCount > 2) {
                    // There are more than two pointers, switch to FREEFORM.
#if DEBUG_GESTURES
                    ALOGD("Gestures: PRESS transitioned to FREEFORM, number of pointers %d > 2",
                            currentFingerCount);
#endif
                    *outCancelPreviousGesture = true;
                    mPointerGesture.currentGestureMode = PointerGesture::FREEFORM;
                } else {
                    // There are exactly two pointers.
                    BitSet32 idBits(mCurrentCookedState.fingerIdBits);
                    uint32_t id1 = idBits.clearFirstMarkedBit();
                    uint32_t id2 = idBits.firstMarkedBit();
                    const RawPointerData::Pointer& p1 =
                            mCurrentRawState.rawPointerData.pointerForId(id1);
                    const RawPointerData::Pointer& p2 =
                            mCurrentRawState.rawPointerData.pointerForId(id2);
                    float mutualDistance = distance(p1.x, p1.y, p2.x, p2.y);
                    if (mutualDistance > mPointerGestureMaxSwipeWidth) {
                        // There are two pointers but they are too far apart for a SWIPE,
                        // switch to FREEFORM.
#if DEBUG_GESTURES
                        ALOGD("Gestures: PRESS transitioned to FREEFORM, distance %0.3f > %0.3f",
                                mutualDistance, mPointerGestureMaxSwipeWidth);
#endif
                        *outCancelPreviousGesture = true;
                        mPointerGesture.currentGestureMode = PointerGesture::FREEFORM;
                    } else {
                        // There are two pointers.  Wait for both pointers to start moving
                        // before deciding whether this is a SWIPE or FREEFORM gesture.
                        float dist1 = dist[id1];
                        float dist2 = dist[id2];
                        if (dist1 >= mConfig.pointerGestureMultitouchMinDistance
                                && dist2 >= mConfig.pointerGestureMultitouchMinDistance) {
                            // Calculate the dot product of the displacement vectors.
                            // When the vectors are oriented in approximately the same direction,
                            // the angle betweeen them is near zero and the cosine of the angle
                            // approches 1.0.  Recall that dot(v1, v2) = cos(angle) * mag(v1) * mag(v2).
                            PointerGesture::Delta& delta1 = mPointerGesture.referenceDeltas[id1];
                            PointerGesture::Delta& delta2 = mPointerGesture.referenceDeltas[id2];
                            float dx1 = delta1.dx * mPointerXZoomScale;
                            float dy1 = delta1.dy * mPointerYZoomScale;
                            float dx2 = delta2.dx * mPointerXZoomScale;
                            float dy2 = delta2.dy * mPointerYZoomScale;
                            float dot = dx1 * dx2 + dy1 * dy2;
                            float cosine = dot / (dist1 * dist2); // denominator always > 0
                            if (cosine >= mConfig.pointerGestureSwipeTransitionAngleCosine) {
                                // Pointers are moving in the same direction.  Switch to SWIPE.
#if DEBUG_GESTURES
                                ALOGD("Gestures: PRESS transitioned to SWIPE, "
                                        "dist1 %0.3f >= %0.3f, dist2 %0.3f >= %0.3f, "
                                        "cosine %0.3f >= %0.3f",
                                        dist1, mConfig.pointerGestureMultitouchMinDistance,
                                        dist2, mConfig.pointerGestureMultitouchMinDistance,
                                        cosine, mConfig.pointerGestureSwipeTransitionAngleCosine);
#endif
                                mPointerGesture.currentGestureMode = PointerGesture::SWIPE;
                            } else {
                                // Pointers are moving in different directions.  Switch to FREEFORM.
#if DEBUG_GESTURES
                                ALOGD("Gestures: PRESS transitioned to FREEFORM, "
                                        "dist1 %0.3f >= %0.3f, dist2 %0.3f >= %0.3f, "
                                        "cosine %0.3f < %0.3f",
                                        dist1, mConfig.pointerGestureMultitouchMinDistance,
                                        dist2, mConfig.pointerGestureMultitouchMinDistance,
                                        cosine, mConfig.pointerGestureSwipeTransitionAngleCosine);
#endif
                                *outCancelPreviousGesture = true;
                                mPointerGesture.currentGestureMode = PointerGesture::FREEFORM;
                            }
                        }
                    }
                }
            }
        } else if (mPointerGesture.currentGestureMode == PointerGesture::SWIPE) {
            // Switch from SWIPE to FREEFORM if additional pointers go down.
            // Cancel previous gesture.
            if (currentFingerCount > 2) {
#if DEBUG_GESTURES
                ALOGD("Gestures: SWIPE transitioned to FREEFORM, number of pointers %d > 2",
                        currentFingerCount);
#endif
                *outCancelPreviousGesture = true;
                mPointerGesture.currentGestureMode = PointerGesture::FREEFORM;
            }
        }

        // Move the reference points based on the overall group motion of the fingers
        // except in PRESS mode while waiting for a transition to occur.
        if (mPointerGesture.currentGestureMode != PointerGesture::PRESS
                && (commonDeltaX || commonDeltaY)) {
            for (BitSet32 idBits(mPointerGesture.referenceIdBits); !idBits.isEmpty(); ) {
                uint32_t id = idBits.clearFirstMarkedBit();
                PointerGesture::Delta& delta = mPointerGesture.referenceDeltas[id];
                delta.dx = 0;
                delta.dy = 0;
            }

            mPointerGesture.referenceTouchX += commonDeltaX;
            mPointerGesture.referenceTouchY += commonDeltaY;

            commonDeltaX *= mPointerXMovementScale;
            commonDeltaY *= mPointerYMovementScale;

            rotateDelta(mSurfaceOrientation, &commonDeltaX, &commonDeltaY);
            mPointerVelocityControl.move(when, &commonDeltaX, &commonDeltaY);

            mPointerGesture.referenceGestureX += commonDeltaX;
            mPointerGesture.referenceGestureY += commonDeltaY;
        }

        // Report gestures.
        if (mPointerGesture.currentGestureMode == PointerGesture::PRESS
                || mPointerGesture.currentGestureMode == PointerGesture::SWIPE) {
            // PRESS or SWIPE mode.
#if DEBUG_GESTURES
            ALOGD("Gestures: PRESS or SWIPE activeTouchId=%d,"
                    "activeGestureId=%d, currentTouchPointerCount=%d",
                    activeTouchId, mPointerGesture.activeGestureId, currentFingerCount);
#endif
            ALOG_ASSERT(mPointerGesture.activeGestureId >= 0);

            mPointerGesture.currentGestureIdBits.clear();
            mPointerGesture.currentGestureIdBits.markBit(mPointerGesture.activeGestureId);
            mPointerGesture.currentGestureIdToIndex[mPointerGesture.activeGestureId] = 0;
            mPointerGesture.currentGestureProperties[0].clear();
            mPointerGesture.currentGestureProperties[0].id = mPointerGesture.activeGestureId;
            mPointerGesture.currentGestureProperties[0].toolType =
                    AMOTION_EVENT_TOOL_TYPE_FINGER;
            mPointerGesture.currentGestureCoords[0].clear();
            mPointerGesture.currentGestureCoords[0].setAxisValue(AMOTION_EVENT_AXIS_X,
                    mPointerGesture.referenceGestureX);
            mPointerGesture.currentGestureCoords[0].setAxisValue(AMOTION_EVENT_AXIS_Y,
                    mPointerGesture.referenceGestureY);
            mPointerGesture.currentGestureCoords[0].setAxisValue(AMOTION_EVENT_AXIS_PRESSURE, 1.0f);
        } else if (mPointerGesture.currentGestureMode == PointerGesture::FREEFORM) {
            // FREEFORM mode.
#if DEBUG_GESTURES
            ALOGD("Gestures: FREEFORM activeTouchId=%d,"
                    "activeGestureId=%d, currentTouchPointerCount=%d",
                    activeTouchId, mPointerGesture.activeGestureId, currentFingerCount);
#endif
            ALOG_ASSERT(mPointerGesture.activeGestureId >= 0);

            mPointerGesture.currentGestureIdBits.clear();

            BitSet32 mappedTouchIdBits;
            BitSet32 usedGestureIdBits;
            if (mPointerGesture.lastGestureMode != PointerGesture::FREEFORM) {
                // Initially, assign the active gesture id to the active touch point
                // if there is one.  No other touch id bits are mapped yet.
                if (!*outCancelPreviousGesture) {
                    mappedTouchIdBits.markBit(activeTouchId);
                    usedGestureIdBits.markBit(mPointerGesture.activeGestureId);
                    mPointerGesture.freeformTouchToGestureIdMap[activeTouchId] =
                            mPointerGesture.activeGestureId;
                } else {
                    mPointerGesture.activeGestureId = -1;
                }
            } else {
                // Otherwise, assume we mapped all touches from the previous frame.
                // Reuse all mappings that are still applicable.
                mappedTouchIdBits.value = mLastCookedState.fingerIdBits.value
                        & mCurrentCookedState.fingerIdBits.value;
                usedGestureIdBits = mPointerGesture.lastGestureIdBits;

                // Check whether we need to choose a new active gesture id because the
                // current went went up.
                for (BitSet32 upTouchIdBits(mLastCookedState.fingerIdBits.value
                        & ~mCurrentCookedState.fingerIdBits.value);
                        !upTouchIdBits.isEmpty(); ) {
                    uint32_t upTouchId = upTouchIdBits.clearFirstMarkedBit();
                    uint32_t upGestureId = mPointerGesture.freeformTouchToGestureIdMap[upTouchId];
                    if (upGestureId == uint32_t(mPointerGesture.activeGestureId)) {
                        mPointerGesture.activeGestureId = -1;
                        break;
                    }
                }
            }

#if DEBUG_GESTURES
            ALOGD("Gestures: FREEFORM follow up "
                    "mappedTouchIdBits=0x%08x, usedGestureIdBits=0x%08x, "
                    "activeGestureId=%d",
                    mappedTouchIdBits.value, usedGestureIdBits.value,
                    mPointerGesture.activeGestureId);
#endif

            BitSet32 idBits(mCurrentCookedState.fingerIdBits);
            for (uint32_t i = 0; i < currentFingerCount; i++) {
                uint32_t touchId = idBits.clearFirstMarkedBit();
                uint32_t gestureId;
                if (!mappedTouchIdBits.hasBit(touchId)) {
                    gestureId = usedGestureIdBits.markFirstUnmarkedBit();
                    mPointerGesture.freeformTouchToGestureIdMap[touchId] = gestureId;
#if DEBUG_GESTURES
                    ALOGD("Gestures: FREEFORM "
                            "new mapping for touch id %d -> gesture id %d",
                            touchId, gestureId);
#endif
                } else {
                    gestureId = mPointerGesture.freeformTouchToGestureIdMap[touchId];
#if DEBUG_GESTURES
                    ALOGD("Gestures: FREEFORM "
                            "existing mapping for touch id %d -> gesture id %d",
                            touchId, gestureId);
#endif
                }
                mPointerGesture.currentGestureIdBits.markBit(gestureId);
                mPointerGesture.currentGestureIdToIndex[gestureId] = i;

                const RawPointerData::Pointer& pointer =
                        mCurrentRawState.rawPointerData.pointerForId(touchId);
                float deltaX = (pointer.x - mPointerGesture.referenceTouchX)
                        * mPointerXZoomScale;
                float deltaY = (pointer.y - mPointerGesture.referenceTouchY)
                        * mPointerYZoomScale;
                rotateDelta(mSurfaceOrientation, &deltaX, &deltaY);

                mPointerGesture.currentGestureProperties[i].clear();
                mPointerGesture.currentGestureProperties[i].id = gestureId;
                mPointerGesture.currentGestureProperties[i].toolType =
                        AMOTION_EVENT_TOOL_TYPE_FINGER;
                mPointerGesture.currentGestureCoords[i].clear();
                mPointerGesture.currentGestureCoords[i].setAxisValue(
                        AMOTION_EVENT_AXIS_X, mPointerGesture.referenceGestureX + deltaX);
                mPointerGesture.currentGestureCoords[i].setAxisValue(
                        AMOTION_EVENT_AXIS_Y, mPointerGesture.referenceGestureY + deltaY);
                mPointerGesture.currentGestureCoords[i].setAxisValue(
                        AMOTION_EVENT_AXIS_PRESSURE, 1.0f);
            }

            if (mPointerGesture.activeGestureId < 0) {
                mPointerGesture.activeGestureId =
                        mPointerGesture.currentGestureIdBits.firstMarkedBit();
#if DEBUG_GESTURES
                ALOGD("Gestures: FREEFORM new "
                        "activeGestureId=%d", mPointerGesture.activeGestureId);
#endif
            }
        }
    }

    mPointerController->setButtonState(mCurrentRawState.buttonState);

#if DEBUG_GESTURES
    ALOGD("Gestures: finishPreviousGesture=%s, cancelPreviousGesture=%s, "
            "currentGestureMode=%d, currentGestureIdBits=0x%08x, "
            "lastGestureMode=%d, lastGestureIdBits=0x%08x",
            toString(*outFinishPreviousGesture), toString(*outCancelPreviousGesture),
            mPointerGesture.currentGestureMode, mPointerGesture.currentGestureIdBits.value,
            mPointerGesture.lastGestureMode, mPointerGesture.lastGestureIdBits.value);
    for (BitSet32 idBits = mPointerGesture.currentGestureIdBits; !idBits.isEmpty(); ) {
        uint32_t id = idBits.clearFirstMarkedBit();
        uint32_t index = mPointerGesture.currentGestureIdToIndex[id];
        const PointerProperties& properties = mPointerGesture.currentGestureProperties[index];
        const PointerCoords& coords = mPointerGesture.currentGestureCoords[index];
        ALOGD("  currentGesture[%d]: index=%d, toolType=%d, "
                "x=%0.3f, y=%0.3f, pressure=%0.3f",
                id, index, properties.toolType,
                coords.getAxisValue(AMOTION_EVENT_AXIS_X),
                coords.getAxisValue(AMOTION_EVENT_AXIS_Y),
                coords.getAxisValue(AMOTION_EVENT_AXIS_PRESSURE));
    }
    for (BitSet32 idBits = mPointerGesture.lastGestureIdBits; !idBits.isEmpty(); ) {
        uint32_t id = idBits.clearFirstMarkedBit();
        uint32_t index = mPointerGesture.lastGestureIdToIndex[id];
        const PointerProperties& properties = mPointerGesture.lastGestureProperties[index];
        const PointerCoords& coords = mPointerGesture.lastGestureCoords[index];
        ALOGD("  lastGesture[%d]: index=%d, toolType=%d, "
                "x=%0.3f, y=%0.3f, pressure=%0.3f",
                id, index, properties.toolType,
                coords.getAxisValue(AMOTION_EVENT_AXIS_X),
                coords.getAxisValue(AMOTION_EVENT_AXIS_Y),
                coords.getAxisValue(AMOTION_EVENT_AXIS_PRESSURE));
    }
#endif
    return true;
}

void TouchInputMapper::dispatchPointerStylus(nsecs_t when, uint32_t policyFlags) {
    mPointerSimple.currentCoords.clear();
    mPointerSimple.currentProperties.clear();

    bool down, hovering;
    if (!mCurrentCookedState.stylusIdBits.isEmpty()) {
        uint32_t id = mCurrentCookedState.stylusIdBits.firstMarkedBit();
        uint32_t index = mCurrentCookedState.cookedPointerData.idToIndex[id];
        float x = mCurrentCookedState.cookedPointerData.pointerCoords[index].getX();
        float y = mCurrentCookedState.cookedPointerData.pointerCoords[index].getY();
        mPointerController->setPosition(x, y);

        hovering = mCurrentCookedState.cookedPointerData.hoveringIdBits.hasBit(id);
        down = !hovering;

        mPointerController->getPosition(&x, &y);
        mPointerSimple.currentCoords.copyFrom(
                mCurrentCookedState.cookedPointerData.pointerCoords[index]);
        mPointerSimple.currentCoords.setAxisValue(AMOTION_EVENT_AXIS_X, x);
        mPointerSimple.currentCoords.setAxisValue(AMOTION_EVENT_AXIS_Y, y);
        mPointerSimple.currentProperties.id = 0;
        mPointerSimple.currentProperties.toolType =
                mCurrentCookedState.cookedPointerData.pointerProperties[index].toolType;
    } else {
        down = false;
        hovering = false;
    }

    dispatchPointerSimple(when, policyFlags, down, hovering);
}

void TouchInputMapper::abortPointerStylus(nsecs_t when, uint32_t policyFlags) {
    abortPointerSimple(when, policyFlags);
}

void TouchInputMapper::dispatchPointerMouse(nsecs_t when, uint32_t policyFlags) {
    mPointerSimple.currentCoords.clear();
    mPointerSimple.currentProperties.clear();

    bool down, hovering;
    if (!mCurrentCookedState.mouseIdBits.isEmpty()) {
        uint32_t id = mCurrentCookedState.mouseIdBits.firstMarkedBit();
        uint32_t currentIndex = mCurrentRawState.rawPointerData.idToIndex[id];
        float deltaX = 0, deltaY = 0;
        if (mLastCookedState.mouseIdBits.hasBit(id)) {
            uint32_t lastIndex = mCurrentRawState.rawPointerData.idToIndex[id];
            deltaX = (mCurrentRawState.rawPointerData.pointers[currentIndex].x
                    - mLastRawState.rawPointerData.pointers[lastIndex].x)
                    * mPointerXMovementScale;
            deltaY = (mCurrentRawState.rawPointerData.pointers[currentIndex].y
                    - mLastRawState.rawPointerData.pointers[lastIndex].y)
                    * mPointerYMovementScale;

            rotateDelta(mSurfaceOrientation, &deltaX, &deltaY);
            mPointerVelocityControl.move(when, &deltaX, &deltaY);

            mPointerController->move(deltaX, deltaY);
        } else {
            mPointerVelocityControl.reset();
        }

        down = isPointerDown(mCurrentRawState.buttonState);
        hovering = !down;

        float x, y;
        mPointerController->getPosition(&x, &y);
        mPointerSimple.currentCoords.copyFrom(
                mCurrentCookedState.cookedPointerData.pointerCoords[currentIndex]);
        mPointerSimple.currentCoords.setAxisValue(AMOTION_EVENT_AXIS_X, x);
        mPointerSimple.currentCoords.setAxisValue(AMOTION_EVENT_AXIS_Y, y);
        mPointerSimple.currentCoords.setAxisValue(AMOTION_EVENT_AXIS_PRESSURE,
                hovering ? 0.0f : 1.0f);
        mPointerSimple.currentProperties.id = 0;
        mPointerSimple.currentProperties.toolType =
                mCurrentCookedState.cookedPointerData.pointerProperties[currentIndex].toolType;
    } else {
        mPointerVelocityControl.reset();

        down = false;
        hovering = false;
    }

    dispatchPointerSimple(when, policyFlags, down, hovering);
}

void TouchInputMapper::abortPointerMouse(nsecs_t when, uint32_t policyFlags) {
    abortPointerSimple(when, policyFlags);

    mPointerVelocityControl.reset();
}

void TouchInputMapper::dispatchPointerSimple(nsecs_t when, uint32_t policyFlags,
        bool down, bool hovering) {
    int32_t metaState = getContext()->getGlobalMetaState();
    int32_t displayId = mViewport.displayId;

    if (mPointerController != nullptr) {
        if (down || hovering) {
            mPointerController->setPresentation(PointerControllerInterface::PRESENTATION_POINTER);
            mPointerController->clearSpots();
            mPointerController->setButtonState(mCurrentRawState.buttonState);
            mPointerController->unfade(PointerControllerInterface::TRANSITION_IMMEDIATE);
        } else if (!down && !hovering && (mPointerSimple.down || mPointerSimple.hovering)) {
            mPointerController->fade(PointerControllerInterface::TRANSITION_GRADUAL);
        }
        displayId = mPointerController->getDisplayId();
    }

    if (mPointerSimple.down && !down) {
        mPointerSimple.down = false;

        // Send up.
        NotifyMotionArgs args(mContext->getNextSequenceNum(), when, getDeviceId(),
                mSource, displayId, policyFlags,
                AMOTION_EVENT_ACTION_UP, 0, 0, metaState, mLastRawState.buttonState,
                MotionClassification::NONE, AMOTION_EVENT_EDGE_FLAG_NONE, /* deviceTimestamp */ 0,
                1, &mPointerSimple.lastProperties, &mPointerSimple.lastCoords,
                mOrientedXPrecision, mOrientedYPrecision,
                mPointerSimple.downTime, /* videoFrames */ {});
        getListener()->notifyMotion(&args);
    }

    if (mPointerSimple.hovering && !hovering) {
        mPointerSimple.hovering = false;

        // Send hover exit.
        NotifyMotionArgs args(mContext->getNextSequenceNum(), when, getDeviceId(),
                mSource, displayId, policyFlags,
                AMOTION_EVENT_ACTION_HOVER_EXIT, 0, 0, metaState, mLastRawState.buttonState,
                MotionClassification::NONE, AMOTION_EVENT_EDGE_FLAG_NONE, /* deviceTimestamp */ 0,
                1, &mPointerSimple.lastProperties, &mPointerSimple.lastCoords,
                mOrientedXPrecision, mOrientedYPrecision,
                mPointerSimple.downTime, /* videoFrames */ {});
        getListener()->notifyMotion(&args);
    }

    if (down) {
        if (!mPointerSimple.down) {
            mPointerSimple.down = true;
            mPointerSimple.downTime = when;

            // Send down.
            NotifyMotionArgs args(mContext->getNextSequenceNum(), when, getDeviceId(),
                    mSource, displayId, policyFlags,
                    AMOTION_EVENT_ACTION_DOWN, 0, 0, metaState, mCurrentRawState.buttonState,
                    MotionClassification::NONE, AMOTION_EVENT_EDGE_FLAG_NONE,
                    /* deviceTimestamp */ 0,
                    1, &mPointerSimple.currentProperties, &mPointerSimple.currentCoords,
                    mOrientedXPrecision, mOrientedYPrecision,
                    mPointerSimple.downTime, /* videoFrames */ {});
            getListener()->notifyMotion(&args);
        }

        // Send move.
        NotifyMotionArgs args(mContext->getNextSequenceNum(), when, getDeviceId(),
                mSource, displayId, policyFlags,
                AMOTION_EVENT_ACTION_MOVE, 0, 0, metaState, mCurrentRawState.buttonState,
                MotionClassification::NONE, AMOTION_EVENT_EDGE_FLAG_NONE, /* deviceTimestamp */ 0,
                1, &mPointerSimple.currentProperties, &mPointerSimple.currentCoords,
                mOrientedXPrecision, mOrientedYPrecision,
                mPointerSimple.downTime, /* videoFrames */ {});
        getListener()->notifyMotion(&args);
    }

    if (hovering) {
        if (!mPointerSimple.hovering) {
            mPointerSimple.hovering = true;

            // Send hover enter.
            NotifyMotionArgs args(mContext->getNextSequenceNum(), when, getDeviceId(),
                    mSource, displayId, policyFlags,
                    AMOTION_EVENT_ACTION_HOVER_ENTER, 0, 0, metaState,
                    mCurrentRawState.buttonState, MotionClassification::NONE,
                    AMOTION_EVENT_EDGE_FLAG_NONE, /* deviceTimestamp */ 0,
                    1, &mPointerSimple.currentProperties, &mPointerSimple.currentCoords,
                    mOrientedXPrecision, mOrientedYPrecision,
                    mPointerSimple.downTime, /* videoFrames */ {});
            getListener()->notifyMotion(&args);
        }

        // Send hover move.
        NotifyMotionArgs args(mContext->getNextSequenceNum(), when, getDeviceId(),
                mSource, displayId, policyFlags,
                AMOTION_EVENT_ACTION_HOVER_MOVE, 0, 0, metaState,
                mCurrentRawState.buttonState, MotionClassification::NONE,
                AMOTION_EVENT_EDGE_FLAG_NONE, /* deviceTimestamp */ 0,
                1, &mPointerSimple.currentProperties, &mPointerSimple.currentCoords,
                mOrientedXPrecision, mOrientedYPrecision,
                mPointerSimple.downTime, /* videoFrames */ {});
        getListener()->notifyMotion(&args);
    }

    if (mCurrentRawState.rawVScroll || mCurrentRawState.rawHScroll) {
        float vscroll = mCurrentRawState.rawVScroll;
        float hscroll = mCurrentRawState.rawHScroll;
        mWheelYVelocityControl.move(when, nullptr, &vscroll);
        mWheelXVelocityControl.move(when, &hscroll, nullptr);

        // Send scroll.
        PointerCoords pointerCoords;
        pointerCoords.copyFrom(mPointerSimple.currentCoords);
        pointerCoords.setAxisValue(AMOTION_EVENT_AXIS_VSCROLL, vscroll);
        pointerCoords.setAxisValue(AMOTION_EVENT_AXIS_HSCROLL, hscroll);

        NotifyMotionArgs args(mContext->getNextSequenceNum(), when, getDeviceId(),
                mSource, displayId, policyFlags,
                AMOTION_EVENT_ACTION_SCROLL, 0, 0, metaState, mCurrentRawState.buttonState,
                MotionClassification::NONE, AMOTION_EVENT_EDGE_FLAG_NONE, /* deviceTimestamp */ 0,
                1, &mPointerSimple.currentProperties, &pointerCoords,
                mOrientedXPrecision, mOrientedYPrecision,
                mPointerSimple.downTime, /* videoFrames */ {});
        getListener()->notifyMotion(&args);
    }

    // Save state.
    if (down || hovering) {
        mPointerSimple.lastCoords.copyFrom(mPointerSimple.currentCoords);
        mPointerSimple.lastProperties.copyFrom(mPointerSimple.currentProperties);
    } else {
        mPointerSimple.reset();
    }
}

void TouchInputMapper::abortPointerSimple(nsecs_t when, uint32_t policyFlags) {
    mPointerSimple.currentCoords.clear();
    mPointerSimple.currentProperties.clear();

    dispatchPointerSimple(when, policyFlags, false, false);
}

void TouchInputMapper::dispatchMotion(nsecs_t when, uint32_t policyFlags, uint32_t source,
        int32_t action, int32_t actionButton, int32_t flags,
        int32_t metaState, int32_t buttonState, int32_t edgeFlags, uint32_t deviceTimestamp,
        const PointerProperties* properties, const PointerCoords* coords,
        const uint32_t* idToIndex, BitSet32 idBits, int32_t changedId,
        float xPrecision, float yPrecision, nsecs_t downTime) {
    PointerCoords pointerCoords[MAX_POINTERS];
    PointerProperties pointerProperties[MAX_POINTERS];
    uint32_t pointerCount = 0;
    while (!idBits.isEmpty()) {
        uint32_t id = idBits.clearFirstMarkedBit();
        uint32_t index = idToIndex[id];
        pointerProperties[pointerCount].copyFrom(properties[index]);
        pointerCoords[pointerCount].copyFrom(coords[index]);

        if (changedId >= 0 && id == uint32_t(changedId)) {
            action |= pointerCount << AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        }

        pointerCount += 1;
    }

    ALOG_ASSERT(pointerCount != 0);

    if (changedId >= 0 && pointerCount == 1) {
        // Replace initial down and final up action.
        // We can compare the action without masking off the changed pointer index
        // because we know the index is 0.
        if (action == AMOTION_EVENT_ACTION_POINTER_DOWN) {
            action = AMOTION_EVENT_ACTION_DOWN;
        } else if (action == AMOTION_EVENT_ACTION_POINTER_UP) {
            action = AMOTION_EVENT_ACTION_UP;
        } else {
            // Can't happen.
            ALOG_ASSERT(false);
        }
    }
    const int32_t displayId = getAssociatedDisplay().value_or(ADISPLAY_ID_NONE);
    const int32_t deviceId = getDeviceId();
    std::vector<TouchVideoFrame> frames = mDevice->getEventHub()->getVideoFrames(deviceId);
    std::for_each(frames.begin(), frames.end(),
            [this](TouchVideoFrame& frame) { frame.rotate(this->mSurfaceOrientation); });
    NotifyMotionArgs args(mContext->getNextSequenceNum(), when, deviceId,
            source, displayId, policyFlags,
            action, actionButton, flags, metaState, buttonState, MotionClassification::NONE,
            edgeFlags, deviceTimestamp, pointerCount, pointerProperties, pointerCoords,
            xPrecision, yPrecision, downTime, std::move(frames));
    getListener()->notifyMotion(&args);
}

bool TouchInputMapper::updateMovedPointers(const PointerProperties* inProperties,
        const PointerCoords* inCoords, const uint32_t* inIdToIndex,
        PointerProperties* outProperties, PointerCoords* outCoords, const uint32_t* outIdToIndex,
        BitSet32 idBits) const {
    bool changed = false;
    while (!idBits.isEmpty()) {
        uint32_t id = idBits.clearFirstMarkedBit();
        uint32_t inIndex = inIdToIndex[id];
        uint32_t outIndex = outIdToIndex[id];

        const PointerProperties& curInProperties = inProperties[inIndex];
        const PointerCoords& curInCoords = inCoords[inIndex];
        PointerProperties& curOutProperties = outProperties[outIndex];
        PointerCoords& curOutCoords = outCoords[outIndex];

        if (curInProperties != curOutProperties) {
            curOutProperties.copyFrom(curInProperties);
            changed = true;
        }

        if (curInCoords != curOutCoords) {
            curOutCoords.copyFrom(curInCoords);
            changed = true;
        }
    }
    return changed;
}

void TouchInputMapper::fadePointer() {
    if (mPointerController != nullptr) {
        mPointerController->fade(PointerControllerInterface::TRANSITION_GRADUAL);
    }
}

void TouchInputMapper::cancelTouch(nsecs_t when) {
    abortPointerUsage(when, 0 /*policyFlags*/);
    abortTouches(when, 0 /* policyFlags*/);
}

bool TouchInputMapper::isPointInsideSurface(int32_t x, int32_t y) {
    const float scaledX = x * mXScale;
    const float scaledY = y * mYScale;
    return x >= mRawPointerAxes.x.minValue && x <= mRawPointerAxes.x.maxValue
            && scaledX >= mPhysicalLeft && scaledX <= mPhysicalLeft + mPhysicalWidth
            && y >= mRawPointerAxes.y.minValue && y <= mRawPointerAxes.y.maxValue
            && scaledY >= mPhysicalTop && scaledY <= mPhysicalTop + mPhysicalHeight;
}

const TouchInputMapper::VirtualKey* TouchInputMapper::findVirtualKeyHit(int32_t x, int32_t y) {

    for (const VirtualKey& virtualKey: mVirtualKeys) {
#if DEBUG_VIRTUAL_KEYS
        ALOGD("VirtualKeys: Hit test (%d, %d): keyCode=%d, scanCode=%d, "
                "left=%d, top=%d, right=%d, bottom=%d",
                x, y,
                virtualKey.keyCode, virtualKey.scanCode,
                virtualKey.hitLeft, virtualKey.hitTop,
                virtualKey.hitRight, virtualKey.hitBottom);
#endif

        if (virtualKey.isHit(x, y)) {
            return & virtualKey;
        }
    }

    return nullptr;
}

void TouchInputMapper::assignPointerIds(const RawState* last, RawState* current) {
    uint32_t currentPointerCount = current->rawPointerData.pointerCount;
    uint32_t lastPointerCount = last->rawPointerData.pointerCount;

    current->rawPointerData.clearIdBits();

    if (currentPointerCount == 0) {
        // No pointers to assign.
        return;
    }

    if (lastPointerCount == 0) {
        // All pointers are new.
        for (uint32_t i = 0; i < currentPointerCount; i++) {
            uint32_t id = i;
            current->rawPointerData.pointers[i].id = id;
            current->rawPointerData.idToIndex[id] = i;
            current->rawPointerData.markIdBit(id, current->rawPointerData.isHovering(i));
        }
        return;
    }

    if (currentPointerCount == 1 && lastPointerCount == 1
            && current->rawPointerData.pointers[0].toolType
                    == last->rawPointerData.pointers[0].toolType) {
        // Only one pointer and no change in count so it must have the same id as before.
        uint32_t id = last->rawPointerData.pointers[0].id;
        current->rawPointerData.pointers[0].id = id;
        current->rawPointerData.idToIndex[id] = 0;
        current->rawPointerData.markIdBit(id, current->rawPointerData.isHovering(0));
        return;
    }

    // General case.
    // We build a heap of squared euclidean distances between current and last pointers
    // associated with the current and last pointer indices.  Then, we find the best
    // match (by distance) for each current pointer.
    // The pointers must have the same tool type but it is possible for them to
    // transition from hovering to touching or vice-versa while retaining the same id.
    PointerDistanceHeapElement heap[MAX_POINTERS * MAX_POINTERS];

    uint32_t heapSize = 0;
    for (uint32_t currentPointerIndex = 0; currentPointerIndex < currentPointerCount;
            currentPointerIndex++) {
        for (uint32_t lastPointerIndex = 0; lastPointerIndex < lastPointerCount;
                lastPointerIndex++) {
            const RawPointerData::Pointer& currentPointer =
                    current->rawPointerData.pointers[currentPointerIndex];
            const RawPointerData::Pointer& lastPointer =
                    last->rawPointerData.pointers[lastPointerIndex];
            if (currentPointer.toolType == lastPointer.toolType) {
                int64_t deltaX = currentPointer.x - lastPointer.x;
                int64_t deltaY = currentPointer.y - lastPointer.y;

                uint64_t distance = uint64_t(deltaX * deltaX + deltaY * deltaY);

                // Insert new element into the heap (sift up).
                heap[heapSize].currentPointerIndex = currentPointerIndex;
                heap[heapSize].lastPointerIndex = lastPointerIndex;
                heap[heapSize].distance = distance;
                heapSize += 1;
            }
        }
    }

    // Heapify
    for (uint32_t startIndex = heapSize / 2; startIndex != 0; ) {
        startIndex -= 1;
        for (uint32_t parentIndex = startIndex; ;) {
            uint32_t childIndex = parentIndex * 2 + 1;
            if (childIndex >= heapSize) {
                break;
            }

            if (childIndex + 1 < heapSize
                    && heap[childIndex + 1].distance < heap[childIndex].distance) {
                childIndex += 1;
            }

            if (heap[parentIndex].distance <= heap[childIndex].distance) {
                break;
            }

            swap(heap[parentIndex], heap[childIndex]);
            parentIndex = childIndex;
        }
    }

#if DEBUG_POINTER_ASSIGNMENT
    ALOGD("assignPointerIds - initial distance min-heap: size=%d", heapSize);
    for (size_t i = 0; i < heapSize; i++) {
        ALOGD("  heap[%zu]: cur=%" PRIu32 ", last=%" PRIu32 ", distance=%" PRIu64,
                i, heap[i].currentPointerIndex, heap[i].lastPointerIndex,
                heap[i].distance);
    }
#endif

    // Pull matches out by increasing order of distance.
    // To avoid reassigning pointers that have already been matched, the loop keeps track
    // of which last and current pointers have been matched using the matchedXXXBits variables.
    // It also tracks the used pointer id bits.
    BitSet32 matchedLastBits(0);
    BitSet32 matchedCurrentBits(0);
    BitSet32 usedIdBits(0);
    bool first = true;
    for (uint32_t i = min(currentPointerCount, lastPointerCount); heapSize > 0 && i > 0; i--) {
        while (heapSize > 0) {
            if (first) {
                // The first time through the loop, we just consume the root element of
                // the heap (the one with smallest distance).
                first = false;
            } else {
                // Previous iterations consumed the root element of the heap.
                // Pop root element off of the heap (sift down).
                heap[0] = heap[heapSize];
                for (uint32_t parentIndex = 0; ;) {
                    uint32_t childIndex = parentIndex * 2 + 1;
                    if (childIndex >= heapSize) {
                        break;
                    }

                    if (childIndex + 1 < heapSize
                            && heap[childIndex + 1].distance < heap[childIndex].distance) {
                        childIndex += 1;
                    }

                    if (heap[parentIndex].distance <= heap[childIndex].distance) {
                        break;
                    }

                    swap(heap[parentIndex], heap[childIndex]);
                    parentIndex = childIndex;
                }

#if DEBUG_POINTER_ASSIGNMENT
                ALOGD("assignPointerIds - reduced distance min-heap: size=%d", heapSize);
                for (size_t i = 0; i < heapSize; i++) {
                    ALOGD("  heap[%zu]: cur=%" PRIu32 ", last=%" PRIu32 ", distance=%" PRIu64,
                            i, heap[i].currentPointerIndex, heap[i].lastPointerIndex,
                            heap[i].distance);
                }
#endif
            }

            heapSize -= 1;

            uint32_t currentPointerIndex = heap[0].currentPointerIndex;
            if (matchedCurrentBits.hasBit(currentPointerIndex)) continue; // already matched

            uint32_t lastPointerIndex = heap[0].lastPointerIndex;
            if (matchedLastBits.hasBit(lastPointerIndex)) continue; // already matched

            matchedCurrentBits.markBit(currentPointerIndex);
            matchedLastBits.markBit(lastPointerIndex);

            uint32_t id = last->rawPointerData.pointers[lastPointerIndex].id;
            current->rawPointerData.pointers[currentPointerIndex].id = id;
            current->rawPointerData.idToIndex[id] = currentPointerIndex;
            current->rawPointerData.markIdBit(id,
                    current->rawPointerData.isHovering(currentPointerIndex));
            usedIdBits.markBit(id);

#if DEBUG_POINTER_ASSIGNMENT
            ALOGD("assignPointerIds - matched: cur=%" PRIu32 ", last=%" PRIu32
                    ", id=%" PRIu32 ", distance=%" PRIu64,
                    lastPointerIndex, currentPointerIndex, id, heap[0].distance);
#endif
            break;
        }
    }

    // Assign fresh ids to pointers that were not matched in the process.
    for (uint32_t i = currentPointerCount - matchedCurrentBits.count(); i != 0; i--) {
        uint32_t currentPointerIndex = matchedCurrentBits.markFirstUnmarkedBit();
        uint32_t id = usedIdBits.markFirstUnmarkedBit();

        current->rawPointerData.pointers[currentPointerIndex].id = id;
        current->rawPointerData.idToIndex[id] = currentPointerIndex;
        current->rawPointerData.markIdBit(id,
                current->rawPointerData.isHovering(currentPointerIndex));

#if DEBUG_POINTER_ASSIGNMENT
        ALOGD("assignPointerIds - assigned: cur=%" PRIu32 ", id=%" PRIu32, currentPointerIndex, id);
#endif
    }
}

int32_t TouchInputMapper::getKeyCodeState(uint32_t sourceMask, int32_t keyCode) {
    if (mCurrentVirtualKey.down && mCurrentVirtualKey.keyCode == keyCode) {
        return AKEY_STATE_VIRTUAL;
    }

    for (const VirtualKey& virtualKey : mVirtualKeys) {
        if (virtualKey.keyCode == keyCode) {
            return AKEY_STATE_UP;
        }
    }

    return AKEY_STATE_UNKNOWN;
}

int32_t TouchInputMapper::getScanCodeState(uint32_t sourceMask, int32_t scanCode) {
    if (mCurrentVirtualKey.down && mCurrentVirtualKey.scanCode == scanCode) {
        return AKEY_STATE_VIRTUAL;
    }

    for (const VirtualKey& virtualKey : mVirtualKeys) {
        if (virtualKey.scanCode == scanCode) {
            return AKEY_STATE_UP;
        }
    }

    return AKEY_STATE_UNKNOWN;
}

bool TouchInputMapper::markSupportedKeyCodes(uint32_t sourceMask, size_t numCodes,
        const int32_t* keyCodes, uint8_t* outFlags) {
    for (const VirtualKey& virtualKey : mVirtualKeys) {
        for (size_t i = 0; i < numCodes; i++) {
            if (virtualKey.keyCode == keyCodes[i]) {
                outFlags[i] = 1;
            }
        }
    }

    return true;
}

std::optional<int32_t> TouchInputMapper::getAssociatedDisplay() {
    if (mParameters.hasAssociatedDisplay) {
        if (mDeviceMode == DEVICE_MODE_POINTER) {
            return std::make_optional(mPointerController->getDisplayId());
        } else {
            return std::make_optional(mViewport.displayId);
        }
    }
    return std::nullopt;
}

// --- SingleTouchInputMapper ---

SingleTouchInputMapper::SingleTouchInputMapper(InputDevice* device) :
        TouchInputMapper(device) {
}

SingleTouchInputMapper::~SingleTouchInputMapper() {
}

void SingleTouchInputMapper::reset(nsecs_t when) {
    mSingleTouchMotionAccumulator.reset(getDevice());

    TouchInputMapper::reset(when);
}

void SingleTouchInputMapper::process(const RawEvent* rawEvent) {
    TouchInputMapper::process(rawEvent);

    mSingleTouchMotionAccumulator.process(rawEvent);
}

void SingleTouchInputMapper::syncTouch(nsecs_t when, RawState* outState) {
    if (mTouchButtonAccumulator.isToolActive()) {
        outState->rawPointerData.pointerCount = 1;
        outState->rawPointerData.idToIndex[0] = 0;

        bool isHovering = mTouchButtonAccumulator.getToolType() != AMOTION_EVENT_TOOL_TYPE_MOUSE
                && (mTouchButtonAccumulator.isHovering()
                        || (mRawPointerAxes.pressure.valid
                                && mSingleTouchMotionAccumulator.getAbsolutePressure() <= 0));
        outState->rawPointerData.markIdBit(0, isHovering);

        RawPointerData::Pointer& outPointer = outState->rawPointerData.pointers[0];
        outPointer.id = 0;
        outPointer.x = mSingleTouchMotionAccumulator.getAbsoluteX();
        outPointer.y = mSingleTouchMotionAccumulator.getAbsoluteY();
        outPointer.pressure = mSingleTouchMotionAccumulator.getAbsolutePressure();
        outPointer.touchMajor = 0;
        outPointer.touchMinor = 0;
        outPointer.toolMajor = mSingleTouchMotionAccumulator.getAbsoluteToolWidth();
        outPointer.toolMinor = mSingleTouchMotionAccumulator.getAbsoluteToolWidth();
        outPointer.orientation = 0;
        outPointer.distance = mSingleTouchMotionAccumulator.getAbsoluteDistance();
        outPointer.tiltX = mSingleTouchMotionAccumulator.getAbsoluteTiltX();
        outPointer.tiltY = mSingleTouchMotionAccumulator.getAbsoluteTiltY();
        outPointer.toolType = mTouchButtonAccumulator.getToolType();
        if (outPointer.toolType == AMOTION_EVENT_TOOL_TYPE_UNKNOWN) {
            outPointer.toolType = AMOTION_EVENT_TOOL_TYPE_FINGER;
        }
        outPointer.isHovering = isHovering;
    }
}

void SingleTouchInputMapper::configureRawPointerAxes() {
    TouchInputMapper::configureRawPointerAxes();

    getAbsoluteAxisInfo(ABS_X, &mRawPointerAxes.x);
    getAbsoluteAxisInfo(ABS_Y, &mRawPointerAxes.y);
    getAbsoluteAxisInfo(ABS_PRESSURE, &mRawPointerAxes.pressure);
    getAbsoluteAxisInfo(ABS_TOOL_WIDTH, &mRawPointerAxes.toolMajor);
    getAbsoluteAxisInfo(ABS_DISTANCE, &mRawPointerAxes.distance);
    getAbsoluteAxisInfo(ABS_TILT_X, &mRawPointerAxes.tiltX);
    getAbsoluteAxisInfo(ABS_TILT_Y, &mRawPointerAxes.tiltY);
}

bool SingleTouchInputMapper::hasStylus() const {
    return mTouchButtonAccumulator.hasStylus();
}


// --- MultiTouchInputMapper ---

MultiTouchInputMapper::MultiTouchInputMapper(InputDevice* device) :
        TouchInputMapper(device) {
}

MultiTouchInputMapper::~MultiTouchInputMapper() {
}

void MultiTouchInputMapper::reset(nsecs_t when) {
    mMultiTouchMotionAccumulator.reset(getDevice());

    mPointerIdBits.clear();

    TouchInputMapper::reset(when);
}

void MultiTouchInputMapper::process(const RawEvent* rawEvent) {
    TouchInputMapper::process(rawEvent);

    mMultiTouchMotionAccumulator.process(rawEvent);
}

void MultiTouchInputMapper::syncTouch(nsecs_t when, RawState* outState) {
    size_t inCount = mMultiTouchMotionAccumulator.getSlotCount();
    size_t outCount = 0;
    BitSet32 newPointerIdBits;
    mHavePointerIds = true;

    for (size_t inIndex = 0; inIndex < inCount; inIndex++) {
        const MultiTouchMotionAccumulator::Slot* inSlot =
                mMultiTouchMotionAccumulator.getSlot(inIndex);
        if (!inSlot->isInUse()) {
            continue;
        }

        if (outCount >= MAX_POINTERS) {
#if DEBUG_POINTERS
            ALOGD("MultiTouch device %s emitted more than maximum of %d pointers; "
                    "ignoring the rest.",
                    getDeviceName().c_str(), MAX_POINTERS);
#endif
            break; // too many fingers!
        }

        RawPointerData::Pointer& outPointer = outState->rawPointerData.pointers[outCount];
        outPointer.x = inSlot->getX();
        outPointer.y = inSlot->getY();
        outPointer.pressure = inSlot->getPressure();
        outPointer.touchMajor = inSlot->getTouchMajor();
        outPointer.touchMinor = inSlot->getTouchMinor();
        outPointer.toolMajor = inSlot->getToolMajor();
        outPointer.toolMinor = inSlot->getToolMinor();
        outPointer.orientation = inSlot->getOrientation();
        outPointer.distance = inSlot->getDistance();
        outPointer.tiltX = 0;
        outPointer.tiltY = 0;

        outPointer.toolType = inSlot->getToolType();
        if (outPointer.toolType == AMOTION_EVENT_TOOL_TYPE_UNKNOWN) {
            outPointer.toolType = mTouchButtonAccumulator.getToolType();
            if (outPointer.toolType == AMOTION_EVENT_TOOL_TYPE_UNKNOWN) {
                outPointer.toolType = AMOTION_EVENT_TOOL_TYPE_FINGER;
            }
        }

        bool isHovering = mTouchButtonAccumulator.getToolType() != AMOTION_EVENT_TOOL_TYPE_MOUSE
                && (mTouchButtonAccumulator.isHovering()
                        || (mRawPointerAxes.pressure.valid && inSlot->getPressure() <= 0));
        outPointer.isHovering = isHovering;

        // Assign pointer id using tracking id if available.
        if (mHavePointerIds) {
            int32_t trackingId = inSlot->getTrackingId();
            int32_t id = -1;
            if (trackingId >= 0) {
                for (BitSet32 idBits(mPointerIdBits); !idBits.isEmpty(); ) {
                    uint32_t n = idBits.clearFirstMarkedBit();
                    if (mPointerTrackingIdMap[n] == trackingId) {
                        id = n;
                    }
                }

                if (id < 0 && !mPointerIdBits.isFull()) {
                    id = mPointerIdBits.markFirstUnmarkedBit();
                    mPointerTrackingIdMap[id] = trackingId;
                }
            }
            if (id < 0) {
                mHavePointerIds = false;
                outState->rawPointerData.clearIdBits();
                newPointerIdBits.clear();
            } else {
                outPointer.id = id;
                outState->rawPointerData.idToIndex[id] = outCount;
                outState->rawPointerData.markIdBit(id, isHovering);
                newPointerIdBits.markBit(id);
            }
        }
        outCount += 1;
    }

    outState->deviceTimestamp = mMultiTouchMotionAccumulator.getDeviceTimestamp();
    outState->rawPointerData.pointerCount = outCount;
    mPointerIdBits = newPointerIdBits;

    mMultiTouchMotionAccumulator.finishSync();
}

void MultiTouchInputMapper::configureRawPointerAxes() {
    TouchInputMapper::configureRawPointerAxes();

    getAbsoluteAxisInfo(ABS_MT_POSITION_X, &mRawPointerAxes.x);
    getAbsoluteAxisInfo(ABS_MT_POSITION_Y, &mRawPointerAxes.y);
    getAbsoluteAxisInfo(ABS_MT_TOUCH_MAJOR, &mRawPointerAxes.touchMajor);
    getAbsoluteAxisInfo(ABS_MT_TOUCH_MINOR, &mRawPointerAxes.touchMinor);
    getAbsoluteAxisInfo(ABS_MT_WIDTH_MAJOR, &mRawPointerAxes.toolMajor);
    getAbsoluteAxisInfo(ABS_MT_WIDTH_MINOR, &mRawPointerAxes.toolMinor);
    getAbsoluteAxisInfo(ABS_MT_ORIENTATION, &mRawPointerAxes.orientation);
    getAbsoluteAxisInfo(ABS_MT_PRESSURE, &mRawPointerAxes.pressure);
    getAbsoluteAxisInfo(ABS_MT_DISTANCE, &mRawPointerAxes.distance);
    getAbsoluteAxisInfo(ABS_MT_TRACKING_ID, &mRawPointerAxes.trackingId);
    getAbsoluteAxisInfo(ABS_MT_SLOT, &mRawPointerAxes.slot);

    if (mRawPointerAxes.trackingId.valid
            && mRawPointerAxes.slot.valid
            && mRawPointerAxes.slot.minValue == 0 && mRawPointerAxes.slot.maxValue > 0) {
        size_t slotCount = mRawPointerAxes.slot.maxValue + 1;
        if (slotCount > MAX_SLOTS) {
            ALOGW("MultiTouch Device %s reported %zu slots but the framework "
                    "only supports a maximum of %zu slots at this time.",
                    getDeviceName().c_str(), slotCount, MAX_SLOTS);
            slotCount = MAX_SLOTS;
        }
        mMultiTouchMotionAccumulator.configure(getDevice(),
                slotCount, true /*usingSlotsProtocol*/);
    } else {
        mMultiTouchMotionAccumulator.configure(getDevice(),
                MAX_POINTERS, false /*usingSlotsProtocol*/);
    }
}

bool MultiTouchInputMapper::hasStylus() const {
    return mMultiTouchMotionAccumulator.hasStylus()
            || mTouchButtonAccumulator.hasStylus();
}

// --- ExternalStylusInputMapper

ExternalStylusInputMapper::ExternalStylusInputMapper(InputDevice* device) :
    InputMapper(device) {

}

uint32_t ExternalStylusInputMapper::getSources() {
    return AINPUT_SOURCE_STYLUS;
}

void ExternalStylusInputMapper::populateDeviceInfo(InputDeviceInfo* info) {
    InputMapper::populateDeviceInfo(info);
    info->addMotionRange(AMOTION_EVENT_AXIS_PRESSURE, AINPUT_SOURCE_STYLUS,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
}

void ExternalStylusInputMapper::dump(std::string& dump) {
    dump += INDENT2 "External Stylus Input Mapper:\n";
    dump += INDENT3 "Raw Stylus Axes:\n";
    dumpRawAbsoluteAxisInfo(dump, mRawPressureAxis, "Pressure");
    dump += INDENT3 "Stylus State:\n";
    dumpStylusState(dump, mStylusState);
}

void ExternalStylusInputMapper::configure(nsecs_t when,
        const InputReaderConfiguration* config, uint32_t changes) {
    getAbsoluteAxisInfo(ABS_PRESSURE, &mRawPressureAxis);
    mTouchButtonAccumulator.configure(getDevice());
}

void ExternalStylusInputMapper::reset(nsecs_t when) {
    InputDevice* device = getDevice();
    mSingleTouchMotionAccumulator.reset(device);
    mTouchButtonAccumulator.reset(device);
    InputMapper::reset(when);
}

void ExternalStylusInputMapper::process(const RawEvent* rawEvent) {
    mSingleTouchMotionAccumulator.process(rawEvent);
    mTouchButtonAccumulator.process(rawEvent);

    if (rawEvent->type == EV_SYN && rawEvent->code == SYN_REPORT) {
        sync(rawEvent->when);
    }
}

void ExternalStylusInputMapper::sync(nsecs_t when) {
    mStylusState.clear();

    mStylusState.when = when;

    mStylusState.toolType = mTouchButtonAccumulator.getToolType();
    if (mStylusState.toolType == AMOTION_EVENT_TOOL_TYPE_UNKNOWN) {
        mStylusState.toolType = AMOTION_EVENT_TOOL_TYPE_STYLUS;
    }

    int32_t pressure = mSingleTouchMotionAccumulator.getAbsolutePressure();
    if (mRawPressureAxis.valid) {
        mStylusState.pressure = float(pressure) / mRawPressureAxis.maxValue;
    } else if (mTouchButtonAccumulator.isToolActive()) {
        mStylusState.pressure = 1.0f;
    } else {
        mStylusState.pressure = 0.0f;
    }

    mStylusState.buttons = mTouchButtonAccumulator.getButtonState();

    mContext->dispatchExternalStylusState(mStylusState);
}


// --- JoystickInputMapper ---

JoystickInputMapper::JoystickInputMapper(InputDevice* device) :
        InputMapper(device) {
}

JoystickInputMapper::~JoystickInputMapper() {
}

uint32_t JoystickInputMapper::getSources() {
    return AINPUT_SOURCE_JOYSTICK;
}

void JoystickInputMapper::populateDeviceInfo(InputDeviceInfo* info) {
    InputMapper::populateDeviceInfo(info);

    for (size_t i = 0; i < mAxes.size(); i++) {
        const Axis& axis = mAxes.valueAt(i);
        addMotionRange(axis.axisInfo.axis, axis, info);

        if (axis.axisInfo.mode == AxisInfo::MODE_SPLIT) {
            addMotionRange(axis.axisInfo.highAxis, axis, info);

        }
    }
}

void JoystickInputMapper::addMotionRange(int32_t axisId, const Axis& axis,
        InputDeviceInfo* info) {
    info->addMotionRange(axisId, AINPUT_SOURCE_JOYSTICK,
            axis.min, axis.max, axis.flat, axis.fuzz, axis.resolution);
    /* In order to ease the transition for developers from using the old axes
     * to the newer, more semantically correct axes, we'll continue to register
     * the old axes as duplicates of their corresponding new ones.  */
    int32_t compatAxis = getCompatAxis(axisId);
    if (compatAxis >= 0) {
        info->addMotionRange(compatAxis, AINPUT_SOURCE_JOYSTICK,
                axis.min, axis.max, axis.flat, axis.fuzz, axis.resolution);
    }
}

/* A mapping from axes the joystick actually has to the axes that should be
 * artificially created for compatibility purposes.
 * Returns -1 if no compatibility axis is needed. */
int32_t JoystickInputMapper::getCompatAxis(int32_t axis) {
    switch(axis) {
    case AMOTION_EVENT_AXIS_LTRIGGER:
        return AMOTION_EVENT_AXIS_BRAKE;
    case AMOTION_EVENT_AXIS_RTRIGGER:
        return AMOTION_EVENT_AXIS_GAS;
    }
    return -1;
}

void JoystickInputMapper::dump(std::string& dump) {
    dump += INDENT2 "Joystick Input Mapper:\n";

    dump += INDENT3 "Axes:\n";
    size_t numAxes = mAxes.size();
    for (size_t i = 0; i < numAxes; i++) {
        const Axis& axis = mAxes.valueAt(i);
        const char* label = getAxisLabel(axis.axisInfo.axis);
        if (label) {
            dump += StringPrintf(INDENT4 "%s", label);
        } else {
            dump += StringPrintf(INDENT4 "%d", axis.axisInfo.axis);
        }
        if (axis.axisInfo.mode == AxisInfo::MODE_SPLIT) {
            label = getAxisLabel(axis.axisInfo.highAxis);
            if (label) {
                dump += StringPrintf(" / %s (split at %d)", label, axis.axisInfo.splitValue);
            } else {
                dump += StringPrintf(" / %d (split at %d)", axis.axisInfo.highAxis,
                        axis.axisInfo.splitValue);
            }
        } else if (axis.axisInfo.mode == AxisInfo::MODE_INVERT) {
            dump += " (invert)";
        }

        dump += StringPrintf(": min=%0.5f, max=%0.5f, flat=%0.5f, fuzz=%0.5f, resolution=%0.5f\n",
                axis.min, axis.max, axis.flat, axis.fuzz, axis.resolution);
        dump += StringPrintf(INDENT4 "  scale=%0.5f, offset=%0.5f, "
                "highScale=%0.5f, highOffset=%0.5f\n",
                axis.scale, axis.offset, axis.highScale, axis.highOffset);
        dump += StringPrintf(INDENT4 "  rawAxis=%d, rawMin=%d, rawMax=%d, "
                "rawFlat=%d, rawFuzz=%d, rawResolution=%d\n",
                mAxes.keyAt(i), axis.rawAxisInfo.minValue, axis.rawAxisInfo.maxValue,
                axis.rawAxisInfo.flat, axis.rawAxisInfo.fuzz, axis.rawAxisInfo.resolution);
    }
}

void JoystickInputMapper::configure(nsecs_t when,
        const InputReaderConfiguration* config, uint32_t changes) {
    InputMapper::configure(when, config, changes);

    if (!changes) { // first time only
        // Collect all axes.
        for (int32_t abs = 0; abs <= ABS_MAX; abs++) {
            if (!(getAbsAxisUsage(abs, getDevice()->getClasses())
                    & INPUT_DEVICE_CLASS_JOYSTICK)) {
                continue; // axis must be claimed by a different device
            }

            RawAbsoluteAxisInfo rawAxisInfo;
            getAbsoluteAxisInfo(abs, &rawAxisInfo);
            if (rawAxisInfo.valid) {
                // Map axis.
                AxisInfo axisInfo;
                bool explicitlyMapped = !getEventHub()->mapAxis(getDeviceId(), abs, &axisInfo);
                if (!explicitlyMapped) {
                    // Axis is not explicitly mapped, will choose a generic axis later.
                    axisInfo.mode = AxisInfo::MODE_NORMAL;
                    axisInfo.axis = -1;
                }

                // Apply flat override.
                int32_t rawFlat = axisInfo.flatOverride < 0
                        ? rawAxisInfo.flat : axisInfo.flatOverride;

                // Calculate scaling factors and limits.
                Axis axis;
                if (axisInfo.mode == AxisInfo::MODE_SPLIT) {
                    float scale = 1.0f / (axisInfo.splitValue - rawAxisInfo.minValue);
                    float highScale = 1.0f / (rawAxisInfo.maxValue - axisInfo.splitValue);
                    axis.initialize(rawAxisInfo, axisInfo, explicitlyMapped,
                            scale, 0.0f, highScale, 0.0f,
                            0.0f, 1.0f, rawFlat * scale, rawAxisInfo.fuzz * scale,
                            rawAxisInfo.resolution * scale);
                } else if (isCenteredAxis(axisInfo.axis)) {
                    float scale = 2.0f / (rawAxisInfo.maxValue - rawAxisInfo.minValue);
                    float offset = avg(rawAxisInfo.minValue, rawAxisInfo.maxValue) * -scale;
                    axis.initialize(rawAxisInfo, axisInfo, explicitlyMapped,
                            scale, offset, scale, offset,
                            -1.0f, 1.0f, rawFlat * scale, rawAxisInfo.fuzz * scale,
                            rawAxisInfo.resolution * scale);
                } else {
                    float scale = 1.0f / (rawAxisInfo.maxValue - rawAxisInfo.minValue);
                    axis.initialize(rawAxisInfo, axisInfo, explicitlyMapped,
                            scale, 0.0f, scale, 0.0f,
                            0.0f, 1.0f, rawFlat * scale, rawAxisInfo.fuzz * scale,
                            rawAxisInfo.resolution * scale);
                }

                // To eliminate noise while the joystick is at rest, filter out small variations
                // in axis values up front.
                axis.filter = axis.fuzz ? axis.fuzz : axis.flat * 0.25f;

                mAxes.add(abs, axis);
            }
        }

        // If there are too many axes, start dropping them.
        // Prefer to keep explicitly mapped axes.
        if (mAxes.size() > PointerCoords::MAX_AXES) {
            ALOGI("Joystick '%s' has %zu axes but the framework only supports a maximum of %d.",
                    getDeviceName().c_str(), mAxes.size(), PointerCoords::MAX_AXES);
            pruneAxes(true);
            pruneAxes(false);
        }

        // Assign generic axis ids to remaining axes.
        int32_t nextGenericAxisId = AMOTION_EVENT_AXIS_GENERIC_1;
        size_t numAxes = mAxes.size();
        for (size_t i = 0; i < numAxes; i++) {
            Axis& axis = mAxes.editValueAt(i);
            if (axis.axisInfo.axis < 0) {
                while (nextGenericAxisId <= AMOTION_EVENT_AXIS_GENERIC_16
                        && haveAxis(nextGenericAxisId)) {
                    nextGenericAxisId += 1;
                }

                if (nextGenericAxisId <= AMOTION_EVENT_AXIS_GENERIC_16) {
                    axis.axisInfo.axis = nextGenericAxisId;
                    nextGenericAxisId += 1;
                } else {
                    ALOGI("Ignoring joystick '%s' axis %d because all of the generic axis ids "
                            "have already been assigned to other axes.",
                            getDeviceName().c_str(), mAxes.keyAt(i));
                    mAxes.removeItemsAt(i--);
                    numAxes -= 1;
                }
            }
        }
    }
}

bool JoystickInputMapper::haveAxis(int32_t axisId) {
    size_t numAxes = mAxes.size();
    for (size_t i = 0; i < numAxes; i++) {
        const Axis& axis = mAxes.valueAt(i);
        if (axis.axisInfo.axis == axisId
                || (axis.axisInfo.mode == AxisInfo::MODE_SPLIT
                        && axis.axisInfo.highAxis == axisId)) {
            return true;
        }
    }
    return false;
}

void JoystickInputMapper::pruneAxes(bool ignoreExplicitlyMappedAxes) {
    size_t i = mAxes.size();
    while (mAxes.size() > PointerCoords::MAX_AXES && i-- > 0) {
        if (ignoreExplicitlyMappedAxes && mAxes.valueAt(i).explicitlyMapped) {
            continue;
        }
        ALOGI("Discarding joystick '%s' axis %d because there are too many axes.",
                getDeviceName().c_str(), mAxes.keyAt(i));
        mAxes.removeItemsAt(i);
    }
}

bool JoystickInputMapper::isCenteredAxis(int32_t axis) {
    switch (axis) {
    case AMOTION_EVENT_AXIS_X:
    case AMOTION_EVENT_AXIS_Y:
    case AMOTION_EVENT_AXIS_Z:
    case AMOTION_EVENT_AXIS_RX:
    case AMOTION_EVENT_AXIS_RY:
    case AMOTION_EVENT_AXIS_RZ:
    case AMOTION_EVENT_AXIS_HAT_X:
    case AMOTION_EVENT_AXIS_HAT_Y:
    case AMOTION_EVENT_AXIS_ORIENTATION:
    case AMOTION_EVENT_AXIS_RUDDER:
    case AMOTION_EVENT_AXIS_WHEEL:
        return true;
    default:
        return false;
    }
}

void JoystickInputMapper::reset(nsecs_t when) {
    // Recenter all axes.
    size_t numAxes = mAxes.size();
    for (size_t i = 0; i < numAxes; i++) {
        Axis& axis = mAxes.editValueAt(i);
        axis.resetValue();
    }

    InputMapper::reset(when);
}

void JoystickInputMapper::process(const RawEvent* rawEvent) {
    switch (rawEvent->type) {
    case EV_ABS: {
        ssize_t index = mAxes.indexOfKey(rawEvent->code);
        if (index >= 0) {
            Axis& axis = mAxes.editValueAt(index);
            float newValue, highNewValue;
            switch (axis.axisInfo.mode) {
            case AxisInfo::MODE_INVERT:
                newValue = (axis.rawAxisInfo.maxValue - rawEvent->value)
                        * axis.scale + axis.offset;
                highNewValue = 0.0f;
                break;
            case AxisInfo::MODE_SPLIT:
                if (rawEvent->value < axis.axisInfo.splitValue) {
                    newValue = (axis.axisInfo.splitValue - rawEvent->value)
                            * axis.scale + axis.offset;
                    highNewValue = 0.0f;
                } else if (rawEvent->value > axis.axisInfo.splitValue) {
                    newValue = 0.0f;
                    highNewValue = (rawEvent->value - axis.axisInfo.splitValue)
                            * axis.highScale + axis.highOffset;
                } else {
                    newValue = 0.0f;
                    highNewValue = 0.0f;
                }
                break;
            default:
                newValue = rawEvent->value * axis.scale + axis.offset;
                highNewValue = 0.0f;
                break;
            }
            axis.newValue = newValue;
            axis.highNewValue = highNewValue;
        }
        break;
    }

    case EV_SYN:
        switch (rawEvent->code) {
        case SYN_REPORT:
            sync(rawEvent->when, false /*force*/);
            break;
        }
        break;
    }
}

void JoystickInputMapper::sync(nsecs_t when, bool force) {
    if (!filterAxes(force)) {
        return;
    }

    int32_t metaState = mContext->getGlobalMetaState();
    int32_t buttonState = 0;

    PointerProperties pointerProperties;
    pointerProperties.clear();
    pointerProperties.id = 0;
    pointerProperties.toolType = AMOTION_EVENT_TOOL_TYPE_UNKNOWN;

    PointerCoords pointerCoords;
    pointerCoords.clear();

    size_t numAxes = mAxes.size();
    for (size_t i = 0; i < numAxes; i++) {
        const Axis& axis = mAxes.valueAt(i);
        setPointerCoordsAxisValue(&pointerCoords, axis.axisInfo.axis, axis.currentValue);
        if (axis.axisInfo.mode == AxisInfo::MODE_SPLIT) {
            setPointerCoordsAxisValue(&pointerCoords, axis.axisInfo.highAxis,
                    axis.highCurrentValue);
        }
    }

    // Moving a joystick axis should not wake the device because joysticks can
    // be fairly noisy even when not in use.  On the other hand, pushing a gamepad
    // button will likely wake the device.
    // TODO: Use the input device configuration to control this behavior more finely.
    uint32_t policyFlags = 0;

    NotifyMotionArgs args(mContext->getNextSequenceNum(), when, getDeviceId(),
            AINPUT_SOURCE_JOYSTICK, ADISPLAY_ID_NONE, policyFlags,
            AMOTION_EVENT_ACTION_MOVE, 0, 0, metaState, buttonState, MotionClassification::NONE,
            AMOTION_EVENT_EDGE_FLAG_NONE, /* deviceTimestamp */ 0, 1,
            &pointerProperties, &pointerCoords, 0, 0, 0, /* videoFrames */ {});
    getListener()->notifyMotion(&args);
}

void JoystickInputMapper::setPointerCoordsAxisValue(PointerCoords* pointerCoords,
        int32_t axis, float value) {
    pointerCoords->setAxisValue(axis, value);
    /* In order to ease the transition for developers from using the old axes
     * to the newer, more semantically correct axes, we'll continue to produce
     * values for the old axes as mirrors of the value of their corresponding
     * new axes. */
    int32_t compatAxis = getCompatAxis(axis);
    if (compatAxis >= 0) {
        pointerCoords->setAxisValue(compatAxis, value);
    }
}

bool JoystickInputMapper::filterAxes(bool force) {
    bool atLeastOneSignificantChange = force;
    size_t numAxes = mAxes.size();
    for (size_t i = 0; i < numAxes; i++) {
        Axis& axis = mAxes.editValueAt(i);
        if (force || hasValueChangedSignificantly(axis.filter,
                axis.newValue, axis.currentValue, axis.min, axis.max)) {
            axis.currentValue = axis.newValue;
            atLeastOneSignificantChange = true;
        }
        if (axis.axisInfo.mode == AxisInfo::MODE_SPLIT) {
            if (force || hasValueChangedSignificantly(axis.filter,
                    axis.highNewValue, axis.highCurrentValue, axis.min, axis.max)) {
                axis.highCurrentValue = axis.highNewValue;
                atLeastOneSignificantChange = true;
            }
        }
    }
    return atLeastOneSignificantChange;
}

bool JoystickInputMapper::hasValueChangedSignificantly(
        float filter, float newValue, float currentValue, float min, float max) {
    if (newValue != currentValue) {
        // Filter out small changes in value unless the value is converging on the axis
        // bounds or center point.  This is intended to reduce the amount of information
        // sent to applications by particularly noisy joysticks (such as PS3).
        if (fabs(newValue - currentValue) > filter
                || hasMovedNearerToValueWithinFilteredRange(filter, newValue, currentValue, min)
                || hasMovedNearerToValueWithinFilteredRange(filter, newValue, currentValue, max)
                || hasMovedNearerToValueWithinFilteredRange(filter, newValue, currentValue, 0)) {
            return true;
        }
    }
    return false;
}

bool JoystickInputMapper::hasMovedNearerToValueWithinFilteredRange(
        float filter, float newValue, float currentValue, float thresholdValue) {
    float newDistance = fabs(newValue - thresholdValue);
    if (newDistance < filter) {
        float oldDistance = fabs(currentValue - thresholdValue);
        if (newDistance < oldDistance) {
            return true;
        }
    }
    return false;
}

} // namespace android
