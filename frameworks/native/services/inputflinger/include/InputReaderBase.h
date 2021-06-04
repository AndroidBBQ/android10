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

#ifndef _UI_INPUT_READER_BASE_H
#define _UI_INPUT_READER_BASE_H

#include "PointerControllerInterface.h"

#include <input/Input.h>
#include <input/InputDevice.h>
#include <input/DisplayViewport.h>
#include <input/VelocityControl.h>
#include <input/VelocityTracker.h>
#include <utils/KeyedVector.h>
#include <utils/Thread.h>
#include <utils/RefBase.h>
#include <utils/SortedVector.h>

#include <optional>
#include <stddef.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

// Maximum supported size of a vibration pattern.
// Must be at least 2.
#define MAX_VIBRATE_PATTERN_SIZE 100

// Maximum allowable delay value in a vibration pattern before
// which the delay will be truncated.
#define MAX_VIBRATE_PATTERN_DELAY_NSECS (1000000 * 1000000000LL)

namespace android {

/* Processes raw input events and sends cooked event data to an input listener. */
class InputReaderInterface : public virtual RefBase {
protected:
    InputReaderInterface() { }
    virtual ~InputReaderInterface() { }

public:
    /* Dumps the state of the input reader.
     *
     * This method may be called on any thread (usually by the input manager). */
    virtual void dump(std::string& dump) = 0;

    /* Called by the heatbeat to ensures that the reader has not deadlocked. */
    virtual void monitor() = 0;

    /* Returns true if the input device is enabled. */
    virtual bool isInputDeviceEnabled(int32_t deviceId) = 0;

    /* Runs a single iteration of the processing loop.
     * Nominally reads and processes one incoming message from the EventHub.
     *
     * This method should be called on the input reader thread.
     */
    virtual void loopOnce() = 0;

    /* Gets information about all input devices.
     *
     * This method may be called on any thread (usually by the input manager).
     */
    virtual void getInputDevices(std::vector<InputDeviceInfo>& outInputDevices) = 0;

    /* Query current input state. */
    virtual int32_t getScanCodeState(int32_t deviceId, uint32_t sourceMask,
            int32_t scanCode) = 0;
    virtual int32_t getKeyCodeState(int32_t deviceId, uint32_t sourceMask,
            int32_t keyCode) = 0;
    virtual int32_t getSwitchState(int32_t deviceId, uint32_t sourceMask,
            int32_t sw) = 0;

    /* Toggle Caps Lock */
    virtual void toggleCapsLockState(int32_t deviceId) = 0;

    /* Determine whether physical keys exist for the given framework-domain key codes. */
    virtual bool hasKeys(int32_t deviceId, uint32_t sourceMask,
            size_t numCodes, const int32_t* keyCodes, uint8_t* outFlags) = 0;

    /* Requests that a reconfiguration of all input devices.
     * The changes flag is a bitfield that indicates what has changed and whether
     * the input devices must all be reopened. */
    virtual void requestRefreshConfiguration(uint32_t changes) = 0;

    /* Controls the vibrator of a particular input device. */
    virtual void vibrate(int32_t deviceId, const nsecs_t* pattern, size_t patternSize,
            ssize_t repeat, int32_t token) = 0;
    virtual void cancelVibrate(int32_t deviceId, int32_t token) = 0;

    /* Return true if the device can send input events to the specified display. */
    virtual bool canDispatchToDisplay(int32_t deviceId, int32_t displayId) = 0;
};

/* Reads raw events from the event hub and processes them, endlessly. */
class InputReaderThread : public Thread {
public:
    explicit InputReaderThread(const sp<InputReaderInterface>& reader);
    virtual ~InputReaderThread();

private:
    sp<InputReaderInterface> mReader;

    virtual bool threadLoop();
};

/*
 * Input reader configuration.
 *
 * Specifies various options that modify the behavior of the input reader.
 */
struct InputReaderConfiguration {
    // Describes changes that have occurred.
    enum {
        // The pointer speed changed.
        CHANGE_POINTER_SPEED = 1 << 0,

        // The pointer gesture control changed.
        CHANGE_POINTER_GESTURE_ENABLEMENT = 1 << 1,

        // The display size or orientation changed.
        CHANGE_DISPLAY_INFO = 1 << 2,

        // The visible touches option changed.
        CHANGE_SHOW_TOUCHES = 1 << 3,

        // The keyboard layouts must be reloaded.
        CHANGE_KEYBOARD_LAYOUTS = 1 << 4,

        // The device name alias supplied by the may have changed for some devices.
        CHANGE_DEVICE_ALIAS = 1 << 5,

        // The location calibration matrix changed.
        CHANGE_TOUCH_AFFINE_TRANSFORMATION = 1 << 6,

        // The presence of an external stylus has changed.
        CHANGE_EXTERNAL_STYLUS_PRESENCE = 1 << 7,

        // The pointer capture mode has changed.
        CHANGE_POINTER_CAPTURE = 1 << 8,

        // The set of disabled input devices (disabledDevices) has changed.
        CHANGE_ENABLED_STATE = 1 << 9,

        // All devices must be reopened.
        CHANGE_MUST_REOPEN = 1 << 31,
    };

    // Gets the amount of time to disable virtual keys after the screen is touched
    // in order to filter out accidental virtual key presses due to swiping gestures
    // or taps near the edge of the display.  May be 0 to disable the feature.
    nsecs_t virtualKeyQuietTime;

    // The excluded device names for the platform.
    // Devices with these names will be ignored.
    std::vector<std::string> excludedDeviceNames;

    // The associations between input ports and display ports.
    // Used to determine which DisplayViewport should be tied to which InputDevice.
    std::unordered_map<std::string, uint8_t> portAssociations;

    // Velocity control parameters for mouse pointer movements.
    VelocityControlParameters pointerVelocityControlParameters;

    // Velocity control parameters for mouse wheel movements.
    VelocityControlParameters wheelVelocityControlParameters;

    // True if pointer gestures are enabled.
    bool pointerGesturesEnabled;

    // Quiet time between certain pointer gesture transitions.
    // Time to allow for all fingers or buttons to settle into a stable state before
    // starting a new gesture.
    nsecs_t pointerGestureQuietInterval;

    // The minimum speed that a pointer must travel for us to consider switching the active
    // touch pointer to it during a drag.  This threshold is set to avoid switching due
    // to noise from a finger resting on the touch pad (perhaps just pressing it down).
    float pointerGestureDragMinSwitchSpeed; // in pixels per second

    // Tap gesture delay time.
    // The time between down and up must be less than this to be considered a tap.
    nsecs_t pointerGestureTapInterval;

    // Tap drag gesture delay time.
    // The time between the previous tap's up and the next down must be less than
    // this to be considered a drag.  Otherwise, the previous tap is finished and a
    // new tap begins.
    //
    // Note that the previous tap will be held down for this entire duration so this
    // interval must be shorter than the long press timeout.
    nsecs_t pointerGestureTapDragInterval;

    // The distance in pixels that the pointer is allowed to move from initial down
    // to up and still be called a tap.
    float pointerGestureTapSlop; // in pixels

    // Time after the first touch points go down to settle on an initial centroid.
    // This is intended to be enough time to handle cases where the user puts down two
    // fingers at almost but not quite exactly the same time.
    nsecs_t pointerGestureMultitouchSettleInterval;

    // The transition from PRESS to SWIPE or FREEFORM gesture mode is made when
    // at least two pointers have moved at least this far from their starting place.
    float pointerGestureMultitouchMinDistance; // in pixels

    // The transition from PRESS to SWIPE gesture mode can only occur when the
    // cosine of the angle between the two vectors is greater than or equal to than this value
    // which indicates that the vectors are oriented in the same direction.
    // When the vectors are oriented in the exactly same direction, the cosine is 1.0.
    // (In exactly opposite directions, the cosine is -1.0.)
    float pointerGestureSwipeTransitionAngleCosine;

    // The transition from PRESS to SWIPE gesture mode can only occur when the
    // fingers are no more than this far apart relative to the diagonal size of
    // the touch pad.  For example, a ratio of 0.5 means that the fingers must be
    // no more than half the diagonal size of the touch pad apart.
    float pointerGestureSwipeMaxWidthRatio;

    // The gesture movement speed factor relative to the size of the display.
    // Movement speed applies when the fingers are moving in the same direction.
    // Without acceleration, a full swipe of the touch pad diagonal in movement mode
    // will cover this portion of the display diagonal.
    float pointerGestureMovementSpeedRatio;

    // The gesture zoom speed factor relative to the size of the display.
    // Zoom speed applies when the fingers are mostly moving relative to each other
    // to execute a scale gesture or similar.
    // Without acceleration, a full swipe of the touch pad diagonal in zoom mode
    // will cover this portion of the display diagonal.
    float pointerGestureZoomSpeedRatio;

    // True to show the location of touches on the touch screen as spots.
    bool showTouches;

    // True if pointer capture is enabled.
    bool pointerCapture;

    // The set of currently disabled input devices.
    SortedVector<int32_t> disabledDevices;

    InputReaderConfiguration() :
            virtualKeyQuietTime(0),
            pointerVelocityControlParameters(1.0f, 500.0f, 3000.0f, 3.0f),
            wheelVelocityControlParameters(1.0f, 15.0f, 50.0f, 4.0f),
            pointerGesturesEnabled(true),
            pointerGestureQuietInterval(100 * 1000000LL), // 100 ms
            pointerGestureDragMinSwitchSpeed(50), // 50 pixels per second
            pointerGestureTapInterval(150 * 1000000LL), // 150 ms
            pointerGestureTapDragInterval(150 * 1000000LL), // 150 ms
            pointerGestureTapSlop(10.0f), // 10 pixels
            pointerGestureMultitouchSettleInterval(100 * 1000000LL), // 100 ms
            pointerGestureMultitouchMinDistance(15), // 15 pixels
            pointerGestureSwipeTransitionAngleCosine(0.2588f), // cosine of 75 degrees
            pointerGestureSwipeMaxWidthRatio(0.25f),
            pointerGestureMovementSpeedRatio(0.8f),
            pointerGestureZoomSpeedRatio(0.3f),
            showTouches(false), pointerCapture(false) { }

    std::optional<DisplayViewport> getDisplayViewportByType(ViewportType type) const;
    std::optional<DisplayViewport> getDisplayViewportByUniqueId(const std::string& uniqueDisplayId)
            const;
    std::optional<DisplayViewport> getDisplayViewportByPort(uint8_t physicalPort) const;
    void setDisplayViewports(const std::vector<DisplayViewport>& viewports);


    void dump(std::string& dump) const;
    void dumpViewport(std::string& dump, const DisplayViewport& viewport) const;

private:
    std::vector<DisplayViewport> mDisplays;
};

struct TouchAffineTransformation {
    float x_scale;
    float x_ymix;
    float x_offset;
    float y_xmix;
    float y_scale;
    float y_offset;

    TouchAffineTransformation() :
        x_scale(1.0f), x_ymix(0.0f), x_offset(0.0f),
        y_xmix(0.0f), y_scale(1.0f), y_offset(0.0f) {
    }

    TouchAffineTransformation(float xscale, float xymix, float xoffset,
            float yxmix, float yscale, float yoffset) :
        x_scale(xscale), x_ymix(xymix), x_offset(xoffset),
        y_xmix(yxmix), y_scale(yscale), y_offset(yoffset) {
    }

    void applyTo(float& x, float& y) const;
};

/*
 * Input reader policy interface.
 *
 * The input reader policy is used by the input reader to interact with the Window Manager
 * and other system components.
 *
 * The actual implementation is partially supported by callbacks into the DVM
 * via JNI.  This interface is also mocked in the unit tests.
 *
 * These methods must NOT re-enter the input reader since they may be called while
 * holding the input reader lock.
 */
class InputReaderPolicyInterface : public virtual RefBase {
protected:
    InputReaderPolicyInterface() { }
    virtual ~InputReaderPolicyInterface() { }

public:
    /* Gets the input reader configuration. */
    virtual void getReaderConfiguration(InputReaderConfiguration* outConfig) = 0;

    /* Gets a pointer controller associated with the specified cursor device (ie. a mouse). */
    virtual sp<PointerControllerInterface> obtainPointerController(int32_t deviceId) = 0;

    /* Notifies the input reader policy that some input devices have changed
     * and provides information about all current input devices.
     */
    virtual void notifyInputDevicesChanged(const std::vector<InputDeviceInfo>& inputDevices) = 0;

    /* Gets the keyboard layout for a particular input device. */
    virtual sp<KeyCharacterMap> getKeyboardLayoutOverlay(
            const InputDeviceIdentifier& identifier) = 0;

    /* Gets a user-supplied alias for a particular input device, or an empty string if none. */
    virtual std::string getDeviceAlias(const InputDeviceIdentifier& identifier) = 0;

    /* Gets the affine calibration associated with the specified device. */
    virtual TouchAffineTransformation getTouchAffineTransformation(
            const std::string& inputDeviceDescriptor, int32_t surfaceRotation) = 0;
};

} // namespace android

#endif // _UI_INPUT_READER_COMMON_H