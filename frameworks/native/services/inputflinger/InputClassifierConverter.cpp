/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "InputClassifierConverter.h"

using android::hardware::hidl_bitfield;
using namespace android::hardware::input;

namespace android {

static common::V1_0::Source getSource(uint32_t source) {
    static_assert(static_cast<common::V1_0::Source>(AINPUT_SOURCE_UNKNOWN) ==
            common::V1_0::Source::UNKNOWN, "SOURCE_UNKNOWN mismatch");
    static_assert(static_cast<common::V1_0::Source>(AINPUT_SOURCE_KEYBOARD) ==
            common::V1_0::Source::KEYBOARD, "SOURCE_KEYBOARD mismatch");
    static_assert(static_cast<common::V1_0::Source>(AINPUT_SOURCE_DPAD) ==
            common::V1_0::Source::DPAD, "SOURCE_DPAD mismatch");
    static_assert(static_cast<common::V1_0::Source>(AINPUT_SOURCE_GAMEPAD) ==
            common::V1_0::Source::GAMEPAD, "SOURCE_GAMEPAD mismatch");
    static_assert(static_cast<common::V1_0::Source>(AINPUT_SOURCE_TOUCHSCREEN) ==
            common::V1_0::Source::TOUCHSCREEN, "SOURCE_TOUCHSCREEN mismatch");
    static_assert(static_cast<common::V1_0::Source>(AINPUT_SOURCE_MOUSE) ==
            common::V1_0::Source::MOUSE, "SOURCE_MOUSE mismatch");
    static_assert(static_cast<common::V1_0::Source>(AINPUT_SOURCE_STYLUS) ==
            common::V1_0::Source::STYLUS, "SOURCE_STYLUS mismatch");
    static_assert(static_cast<common::V1_0::Source>(AINPUT_SOURCE_BLUETOOTH_STYLUS) ==
            common::V1_0::Source::BLUETOOTH_STYLUS, "SOURCE_BLUETOOTH_STYLUS mismatch");
    static_assert(static_cast<common::V1_0::Source>(AINPUT_SOURCE_TRACKBALL) ==
            common::V1_0::Source::TRACKBALL, "SOURCE_TRACKBALL mismatch");
    static_assert(static_cast<common::V1_0::Source>(AINPUT_SOURCE_MOUSE_RELATIVE) ==
            common::V1_0::Source::MOUSE_RELATIVE, "SOURCE_MOUSE_RELATIVE mismatch");
    static_assert(static_cast<common::V1_0::Source>(AINPUT_SOURCE_TOUCHPAD) ==
            common::V1_0::Source::TOUCHPAD, "SOURCE_TOUCHPAD mismatch");
    static_assert(static_cast<common::V1_0::Source>(AINPUT_SOURCE_TOUCH_NAVIGATION) ==
            common::V1_0::Source::TOUCH_NAVIGATION, "SOURCE_TOUCH_NAVIGATION mismatch");
    static_assert(static_cast<common::V1_0::Source>(AINPUT_SOURCE_JOYSTICK) ==
            common::V1_0::Source::JOYSTICK, "SOURCE_JOYSTICK mismatch");
    static_assert(static_cast<common::V1_0::Source>(AINPUT_SOURCE_ROTARY_ENCODER) ==
            common::V1_0::Source::ROTARY_ENCODER, "SOURCE_ROTARY_ENCODER mismatch");
    static_assert(static_cast<common::V1_0::Source>(AINPUT_SOURCE_ANY) ==
            common::V1_0::Source::ANY, "SOURCE_ANY mismatch");
    return static_cast<common::V1_0::Source>(source);
}

static common::V1_0::Action getAction(int32_t actionMasked) {
    static_assert(static_cast<common::V1_0::Action>(AMOTION_EVENT_ACTION_DOWN) ==
            common::V1_0::Action::DOWN, "ACTION_DOWN mismatch");
    static_assert(static_cast<common::V1_0::Action>(AMOTION_EVENT_ACTION_UP) ==
            common::V1_0::Action::UP, "ACTION_UP mismatch");
    static_assert(static_cast<common::V1_0::Action>(AMOTION_EVENT_ACTION_MOVE) ==
            common::V1_0::Action::MOVE, "ACTION_MOVE mismatch");
    static_assert(static_cast<common::V1_0::Action>(AMOTION_EVENT_ACTION_CANCEL) ==
            common::V1_0::Action::CANCEL, "ACTION_CANCEL mismatch");
    static_assert(static_cast<common::V1_0::Action>(AMOTION_EVENT_ACTION_OUTSIDE) ==
            common::V1_0::Action::OUTSIDE, "ACTION_OUTSIDE mismatch");
    static_assert(static_cast<common::V1_0::Action>(AMOTION_EVENT_ACTION_POINTER_DOWN) ==
            common::V1_0::Action::POINTER_DOWN, "ACTION_POINTER_DOWN mismatch");
    static_assert(static_cast<common::V1_0::Action>(AMOTION_EVENT_ACTION_POINTER_UP) ==
            common::V1_0::Action::POINTER_UP, "ACTION_POINTER_UP mismatch");
    static_assert(static_cast<common::V1_0::Action>( AMOTION_EVENT_ACTION_HOVER_MOVE) ==
            common::V1_0::Action::HOVER_MOVE, "ACTION_HOVER_MOVE mismatch");
    static_assert(static_cast<common::V1_0::Action>(AMOTION_EVENT_ACTION_SCROLL) ==
            common::V1_0::Action::SCROLL, "ACTION_SCROLL mismatch");
    static_assert(static_cast<common::V1_0::Action>(AMOTION_EVENT_ACTION_HOVER_ENTER) ==
            common::V1_0::Action::HOVER_ENTER, "ACTION_HOVER_ENTER mismatch");
    static_assert(static_cast<common::V1_0::Action>(AMOTION_EVENT_ACTION_HOVER_EXIT) ==
            common::V1_0::Action::HOVER_EXIT, "ACTION_HOVER_EXIT mismatch");
    static_assert(static_cast<common::V1_0::Action>(AMOTION_EVENT_ACTION_BUTTON_PRESS) ==
            common::V1_0::Action::BUTTON_PRESS, "ACTION_BUTTON_PRESS mismatch");
    static_assert(static_cast<common::V1_0::Action>(AMOTION_EVENT_ACTION_BUTTON_RELEASE) ==
            common::V1_0::Action::BUTTON_RELEASE, "ACTION_BUTTON_RELEASE mismatch");
    return static_cast<common::V1_0::Action>(actionMasked);
}

static common::V1_0::Button getActionButton(int32_t actionButton) {
    static_assert(static_cast<common::V1_0::Button>(0) ==
            common::V1_0::Button::NONE, "BUTTON_NONE mismatch");
    static_assert(static_cast<common::V1_0::Button>(AMOTION_EVENT_BUTTON_PRIMARY) ==
            common::V1_0::Button::PRIMARY, "BUTTON_PRIMARY mismatch");
    static_assert(static_cast<common::V1_0::Button>(AMOTION_EVENT_BUTTON_SECONDARY) ==
            common::V1_0::Button::SECONDARY, "BUTTON_SECONDARY mismatch");
    static_assert(static_cast<common::V1_0::Button>(AMOTION_EVENT_BUTTON_TERTIARY) ==
            common::V1_0::Button::TERTIARY, "BUTTON_TERTIARY mismatch");
    static_assert(static_cast<common::V1_0::Button>(AMOTION_EVENT_BUTTON_BACK) ==
            common::V1_0::Button::BACK, "BUTTON_BACK mismatch");
    static_assert(static_cast<common::V1_0::Button>(AMOTION_EVENT_BUTTON_FORWARD) ==
            common::V1_0::Button::FORWARD, "BUTTON_FORWARD mismatch");
    static_assert(static_cast<common::V1_0::Button>(AMOTION_EVENT_BUTTON_STYLUS_PRIMARY) ==
            common::V1_0::Button::STYLUS_PRIMARY, "BUTTON_STYLUS_PRIMARY mismatch");
    static_assert(static_cast<common::V1_0::Button>(AMOTION_EVENT_BUTTON_STYLUS_SECONDARY) ==
            common::V1_0::Button::STYLUS_SECONDARY, "BUTTON_STYLUS_SECONDARY mismatch");
    return static_cast<common::V1_0::Button>(actionButton);
}

static hidl_bitfield<common::V1_0::Flag> getFlags(int32_t flags) {
    static_assert(static_cast<common::V1_0::Flag>(AMOTION_EVENT_FLAG_WINDOW_IS_OBSCURED) ==
            common::V1_0::Flag::WINDOW_IS_OBSCURED);
    static_assert(static_cast<common::V1_0::Flag>(AMOTION_EVENT_FLAG_IS_GENERATED_GESTURE) ==
            common::V1_0::Flag::IS_GENERATED_GESTURE);
    static_assert(static_cast<common::V1_0::Flag>(AMOTION_EVENT_FLAG_TAINTED) ==
            common::V1_0::Flag::TAINTED);
    return static_cast<hidl_bitfield<common::V1_0::Flag>>(flags);
}

static hidl_bitfield<common::V1_0::PolicyFlag> getPolicyFlags(int32_t flags) {
    static_assert(static_cast<common::V1_0::PolicyFlag>(POLICY_FLAG_WAKE) ==
            common::V1_0::PolicyFlag::WAKE);
    static_assert(static_cast<common::V1_0::PolicyFlag>(POLICY_FLAG_VIRTUAL) ==
            common::V1_0::PolicyFlag::VIRTUAL);
    static_assert(static_cast<common::V1_0::PolicyFlag>(POLICY_FLAG_FUNCTION) ==
            common::V1_0::PolicyFlag::FUNCTION);
    static_assert(static_cast<common::V1_0::PolicyFlag>(POLICY_FLAG_GESTURE) ==
            common::V1_0::PolicyFlag::GESTURE);
    static_assert(static_cast<common::V1_0::PolicyFlag>(POLICY_FLAG_INJECTED) ==
            common::V1_0::PolicyFlag::INJECTED);
    static_assert(static_cast<common::V1_0::PolicyFlag>(POLICY_FLAG_TRUSTED) ==
            common::V1_0::PolicyFlag::TRUSTED);
    static_assert(static_cast<common::V1_0::PolicyFlag>(POLICY_FLAG_FILTERED) ==
            common::V1_0::PolicyFlag::FILTERED);
    static_assert(static_cast<common::V1_0::PolicyFlag>(POLICY_FLAG_DISABLE_KEY_REPEAT) ==
            common::V1_0::PolicyFlag::DISABLE_KEY_REPEAT);
    static_assert(static_cast<common::V1_0::PolicyFlag>(POLICY_FLAG_INTERACTIVE) ==
            common::V1_0::PolicyFlag::INTERACTIVE);
    static_assert(static_cast<common::V1_0::PolicyFlag>(POLICY_FLAG_PASS_TO_USER) ==
            common::V1_0::PolicyFlag::PASS_TO_USER);
    return static_cast<hidl_bitfield<common::V1_0::PolicyFlag>>(flags);
}

static hidl_bitfield<common::V1_0::EdgeFlag> getEdgeFlags(int32_t flags) {
    static_assert(static_cast<common::V1_0::EdgeFlag>(AMOTION_EVENT_EDGE_FLAG_NONE) ==
            common::V1_0::EdgeFlag::NONE);
    static_assert(static_cast<common::V1_0::EdgeFlag>(AMOTION_EVENT_EDGE_FLAG_TOP) ==
            common::V1_0::EdgeFlag::TOP);
    static_assert(static_cast<common::V1_0::EdgeFlag>(AMOTION_EVENT_EDGE_FLAG_BOTTOM) ==
            common::V1_0::EdgeFlag::BOTTOM);
    static_assert(static_cast<common::V1_0::EdgeFlag>(AMOTION_EVENT_EDGE_FLAG_LEFT) ==
            common::V1_0::EdgeFlag::LEFT);
    static_assert(static_cast<common::V1_0::EdgeFlag>(AMOTION_EVENT_EDGE_FLAG_RIGHT) ==
            common::V1_0::EdgeFlag::RIGHT);
    return static_cast<hidl_bitfield<common::V1_0::EdgeFlag>>(flags);
}

static hidl_bitfield<common::V1_0::Meta> getMetastate(int32_t state) {
    static_assert(static_cast<common::V1_0::Meta>(AMETA_NONE) ==
            common::V1_0::Meta::NONE);
    static_assert(static_cast<common::V1_0::Meta>(AMETA_ALT_ON) ==
            common::V1_0::Meta::ALT_ON);
    static_assert(static_cast<common::V1_0::Meta>(AMETA_ALT_LEFT_ON) ==
            common::V1_0::Meta::ALT_LEFT_ON);
    static_assert(static_cast<common::V1_0::Meta>(AMETA_ALT_RIGHT_ON) ==
            common::V1_0::Meta::ALT_RIGHT_ON);
    static_assert(static_cast<common::V1_0::Meta>(AMETA_SHIFT_ON) ==
            common::V1_0::Meta::SHIFT_ON);
    static_assert(static_cast<common::V1_0::Meta>(AMETA_SHIFT_LEFT_ON) ==
            common::V1_0::Meta::SHIFT_LEFT_ON);
    static_assert(static_cast<common::V1_0::Meta>(AMETA_SHIFT_RIGHT_ON) ==
            common::V1_0::Meta::SHIFT_RIGHT_ON);
    static_assert(static_cast<common::V1_0::Meta>(AMETA_SYM_ON) ==
            common::V1_0::Meta::SYM_ON);
    static_assert(static_cast<common::V1_0::Meta>(AMETA_FUNCTION_ON) ==
            common::V1_0::Meta::FUNCTION_ON);
    static_assert(static_cast<common::V1_0::Meta>(AMETA_CTRL_ON) ==
            common::V1_0::Meta::CTRL_ON);
    static_assert(static_cast<common::V1_0::Meta>(AMETA_CTRL_LEFT_ON) ==
            common::V1_0::Meta::CTRL_LEFT_ON);
    static_assert(static_cast<common::V1_0::Meta>(AMETA_CTRL_RIGHT_ON) ==
            common::V1_0::Meta::CTRL_RIGHT_ON);
    static_assert(static_cast<common::V1_0::Meta>(AMETA_META_ON) ==
            common::V1_0::Meta::META_ON);
    static_assert(static_cast<common::V1_0::Meta>(AMETA_META_LEFT_ON) ==
            common::V1_0::Meta::META_LEFT_ON);
    static_assert(static_cast<common::V1_0::Meta>(AMETA_META_RIGHT_ON) ==
            common::V1_0::Meta::META_RIGHT_ON);
    static_assert(static_cast<common::V1_0::Meta>(AMETA_CAPS_LOCK_ON) ==
            common::V1_0::Meta::CAPS_LOCK_ON);
    static_assert(static_cast<common::V1_0::Meta>(AMETA_NUM_LOCK_ON) ==
            common::V1_0::Meta::NUM_LOCK_ON);
    static_assert(static_cast<common::V1_0::Meta>(AMETA_SCROLL_LOCK_ON) ==
            common::V1_0::Meta::SCROLL_LOCK_ON);
    return static_cast<hidl_bitfield<common::V1_0::Meta>>(state);
}

static hidl_bitfield<common::V1_0::Button> getButtonState(int32_t buttonState) {
    // No need for static_assert here.
    // The button values have already been asserted in getActionButton(..) above
    return static_cast<hidl_bitfield<common::V1_0::Button>>(buttonState);
}

static common::V1_0::ToolType getToolType(int32_t toolType) {
    static_assert(static_cast<common::V1_0::ToolType>(AMOTION_EVENT_TOOL_TYPE_UNKNOWN) ==
            common::V1_0::ToolType::UNKNOWN);
    static_assert(static_cast<common::V1_0::ToolType>(AMOTION_EVENT_TOOL_TYPE_FINGER) ==
            common::V1_0::ToolType::FINGER);
    static_assert(static_cast<common::V1_0::ToolType>(AMOTION_EVENT_TOOL_TYPE_STYLUS) ==
            common::V1_0::ToolType::STYLUS);
    static_assert(static_cast<common::V1_0::ToolType>(AMOTION_EVENT_TOOL_TYPE_MOUSE) ==
            common::V1_0::ToolType::MOUSE);
    static_assert(static_cast<common::V1_0::ToolType>(AMOTION_EVENT_TOOL_TYPE_ERASER) ==
            common::V1_0::ToolType::ERASER);
    return static_cast<common::V1_0::ToolType>(toolType);
}

// MotionEvent axes asserts
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_X) ==
        common::V1_0::Axis::X);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_Y) ==
        common::V1_0::Axis::Y);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_PRESSURE) ==
        common::V1_0::Axis::PRESSURE);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_SIZE) ==
        common::V1_0::Axis::SIZE);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_TOUCH_MAJOR) ==
        common::V1_0::Axis::TOUCH_MAJOR);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_TOUCH_MINOR) ==
        common::V1_0::Axis::TOUCH_MINOR);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_TOOL_MAJOR) ==
        common::V1_0::Axis::TOOL_MAJOR);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_TOOL_MINOR) ==
        common::V1_0::Axis::TOOL_MINOR);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_ORIENTATION) ==
        common::V1_0::Axis::ORIENTATION);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_VSCROLL) ==
        common::V1_0::Axis::VSCROLL);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_HSCROLL) ==
        common::V1_0::Axis::HSCROLL);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_Z) ==
        common::V1_0::Axis::Z);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_RX) ==
        common::V1_0::Axis::RX);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_RY) ==
        common::V1_0::Axis::RY);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_RZ) ==
        common::V1_0::Axis::RZ);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_HAT_X) ==
        common::V1_0::Axis::HAT_X);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_HAT_Y) ==
        common::V1_0::Axis::HAT_Y);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_LTRIGGER) ==
        common::V1_0::Axis::LTRIGGER);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_RTRIGGER) ==
        common::V1_0::Axis::RTRIGGER);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_THROTTLE) ==
        common::V1_0::Axis::THROTTLE);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_RUDDER) ==
        common::V1_0::Axis::RUDDER);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_WHEEL) ==
        common::V1_0::Axis::WHEEL);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_GAS) ==
        common::V1_0::Axis::GAS);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_BRAKE) ==
        common::V1_0::Axis::BRAKE);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_DISTANCE) ==
        common::V1_0::Axis::DISTANCE);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_TILT) ==
        common::V1_0::Axis::TILT);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_SCROLL) ==
        common::V1_0::Axis::SCROLL);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_RELATIVE_X) ==
        common::V1_0::Axis::RELATIVE_X);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_RELATIVE_Y) ==
        common::V1_0::Axis::RELATIVE_Y);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_GENERIC_1) ==
        common::V1_0::Axis::GENERIC_1);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_GENERIC_2) ==
        common::V1_0::Axis::GENERIC_2);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_GENERIC_3) ==
        common::V1_0::Axis::GENERIC_3);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_GENERIC_4) ==
        common::V1_0::Axis::GENERIC_4);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_GENERIC_5) ==
        common::V1_0::Axis::GENERIC_5);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_GENERIC_6) ==
        common::V1_0::Axis::GENERIC_6);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_GENERIC_7) ==
        common::V1_0::Axis::GENERIC_7);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_GENERIC_8) ==
        common::V1_0::Axis::GENERIC_8);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_GENERIC_9) ==
        common::V1_0::Axis::GENERIC_9);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_GENERIC_10) ==
        common::V1_0::Axis::GENERIC_10);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_GENERIC_11) ==
        common::V1_0::Axis::GENERIC_11);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_GENERIC_12) ==
        common::V1_0::Axis::GENERIC_12);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_GENERIC_13) ==
        common::V1_0::Axis::GENERIC_13);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_GENERIC_14) ==
        common::V1_0::Axis::GENERIC_14);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_GENERIC_15) ==
        common::V1_0::Axis::GENERIC_15);
static_assert(static_cast<common::V1_0::Axis>(AMOTION_EVENT_AXIS_GENERIC_16) ==
        common::V1_0::Axis::GENERIC_16);

static common::V1_0::VideoFrame getHalVideoFrame(const TouchVideoFrame& frame) {
    common::V1_0::VideoFrame out;
    out.width = frame.getWidth();
    out.height = frame.getHeight();
    out.data = frame.getData();
    struct timeval timestamp = frame.getTimestamp();
    out.timestamp = seconds_to_nanoseconds(timestamp.tv_sec) +
             microseconds_to_nanoseconds(timestamp.tv_usec);
    return out;
}

static std::vector<common::V1_0::VideoFrame> convertVideoFrames(
        const std::vector<TouchVideoFrame>& frames) {
    std::vector<common::V1_0::VideoFrame> out;
    for (const TouchVideoFrame& frame : frames) {
        out.push_back(getHalVideoFrame(frame));
    }
    return out;
}

static uint8_t getActionIndex(int32_t action) {
    return (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
            AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
}

static void getHidlPropertiesAndCoords(const NotifyMotionArgs& args,
        std::vector<common::V1_0::PointerProperties>* outPointerProperties,
        std::vector<common::V1_0::PointerCoords>* outPointerCoords) {
    outPointerProperties->reserve(args.pointerCount);
    outPointerCoords->reserve(args.pointerCount);
    for (size_t i = 0; i < args.pointerCount; i++) {
        common::V1_0::PointerProperties properties;
        properties.id = args.pointerProperties[i].id;
        properties.toolType = getToolType(args.pointerProperties[i].toolType);
        outPointerProperties->push_back(properties);

        common::V1_0::PointerCoords coords;
        // OK to copy bits because we have static_assert for pointerCoords axes
        coords.bits = args.pointerCoords[i].bits;
        coords.values = std::vector<float>(
                args.pointerCoords[i].values,
                args.pointerCoords[i].values + BitSet64::count(args.pointerCoords[i].bits));
        outPointerCoords->push_back(coords);
    }
}

common::V1_0::MotionEvent notifyMotionArgsToHalMotionEvent(const NotifyMotionArgs& args) {
    common::V1_0::MotionEvent event;
    event.deviceId = args.deviceId;
    event.source = getSource(args.source);
    event.displayId = args.displayId;
    event.downTime = args.downTime;
    event.eventTime = args.eventTime;
    event.action = getAction(args.action & AMOTION_EVENT_ACTION_MASK);
    event.actionIndex = getActionIndex(args.action);
    event.actionButton = getActionButton(args.actionButton);
    event.flags = getFlags(args.flags);
    event.policyFlags = getPolicyFlags(args.policyFlags);
    event.edgeFlags = getEdgeFlags(args.edgeFlags);
    event.metaState = getMetastate(args.metaState);
    event.buttonState = getButtonState(args.buttonState);
    event.xPrecision = args.xPrecision;
    event.yPrecision = args.yPrecision;

    std::vector<common::V1_0::PointerProperties> pointerProperties;
    std::vector<common::V1_0::PointerCoords> pointerCoords;
    getHidlPropertiesAndCoords(args, /*out*/&pointerProperties, /*out*/&pointerCoords);
    event.pointerProperties = pointerProperties;
    event.pointerCoords = pointerCoords;

    event.deviceTimestamp = args.deviceTimestamp;
    event.frames = convertVideoFrames(args.videoFrames);

    return event;
}

} // namespace android
