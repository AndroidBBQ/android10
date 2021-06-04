/*
 * Copyright (C) 2011 The Android Open Source Project
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

#define LOG_TAG "InputListener"

//#define LOG_NDEBUG 0

#include "InputListener.h"

#include <android/log.h>

namespace android {

// --- NotifyConfigurationChangedArgs ---

NotifyConfigurationChangedArgs::NotifyConfigurationChangedArgs(
        uint32_t sequenceNum, nsecs_t eventTime) :
        NotifyArgs(sequenceNum, eventTime) {
}

NotifyConfigurationChangedArgs::NotifyConfigurationChangedArgs(
        const NotifyConfigurationChangedArgs& other) :
        NotifyArgs(other.sequenceNum, other.eventTime) {
}

bool NotifyConfigurationChangedArgs::operator==(const NotifyConfigurationChangedArgs& rhs) const {
    return sequenceNum == rhs.sequenceNum && eventTime == rhs.eventTime;
}

void NotifyConfigurationChangedArgs::notify(const sp<InputListenerInterface>& listener) const {
    listener->notifyConfigurationChanged(this);
}


// --- NotifyKeyArgs ---

NotifyKeyArgs::NotifyKeyArgs(uint32_t sequenceNum, nsecs_t eventTime, int32_t deviceId,
        uint32_t source, int32_t displayId, uint32_t policyFlags,
        int32_t action, int32_t flags, int32_t keyCode, int32_t scanCode,
        int32_t metaState, nsecs_t downTime) :
        NotifyArgs(sequenceNum, eventTime), deviceId(deviceId), source(source),
        displayId(displayId), policyFlags(policyFlags),
        action(action), flags(flags), keyCode(keyCode), scanCode(scanCode),
        metaState(metaState), downTime(downTime) {
}

NotifyKeyArgs::NotifyKeyArgs(const NotifyKeyArgs& other) :
        NotifyArgs(other.sequenceNum, other.eventTime), deviceId(other.deviceId),
        source(other.source), displayId(other.displayId), policyFlags(other.policyFlags),
        action(other.action), flags(other.flags),
        keyCode(other.keyCode), scanCode(other.scanCode),
        metaState(other.metaState), downTime(other.downTime) {
}

bool NotifyKeyArgs::operator==(const NotifyKeyArgs& rhs) const {
    return sequenceNum == rhs.sequenceNum
            && eventTime == rhs.eventTime
            && deviceId == rhs.deviceId
            && source == rhs.source
            && displayId == rhs.displayId
            && policyFlags == rhs.policyFlags
            && action == rhs.action
            && flags == rhs.flags
            && keyCode == rhs.keyCode
            && scanCode == rhs.scanCode
            && metaState == rhs.metaState
            && downTime == rhs.downTime;
}

void NotifyKeyArgs::notify(const sp<InputListenerInterface>& listener) const {
    listener->notifyKey(this);
}


// --- NotifyMotionArgs ---

NotifyMotionArgs::NotifyMotionArgs(uint32_t sequenceNum, nsecs_t eventTime, int32_t deviceId,
        uint32_t source, int32_t displayId, uint32_t policyFlags,
        int32_t action, int32_t actionButton, int32_t flags, int32_t metaState,
        int32_t buttonState, MotionClassification classification,
        int32_t edgeFlags, uint32_t deviceTimestamp, uint32_t pointerCount,
        const PointerProperties* pointerProperties, const PointerCoords* pointerCoords,
        float xPrecision, float yPrecision, nsecs_t downTime,
        const std::vector<TouchVideoFrame>& videoFrames) :
        NotifyArgs(sequenceNum, eventTime), deviceId(deviceId), source(source),
        displayId(displayId), policyFlags(policyFlags),
        action(action), actionButton(actionButton),
        flags(flags), metaState(metaState), buttonState(buttonState),
        classification(classification), edgeFlags(edgeFlags), deviceTimestamp(deviceTimestamp),
        pointerCount(pointerCount),
        xPrecision(xPrecision), yPrecision(yPrecision), downTime(downTime),
        videoFrames(videoFrames) {
    for (uint32_t i = 0; i < pointerCount; i++) {
        this->pointerProperties[i].copyFrom(pointerProperties[i]);
        this->pointerCoords[i].copyFrom(pointerCoords[i]);
    }
}

NotifyMotionArgs::NotifyMotionArgs(const NotifyMotionArgs& other) :
        NotifyArgs(other.sequenceNum, other.eventTime), deviceId(other.deviceId),
        source(other.source), displayId(other.displayId), policyFlags(other.policyFlags),
        action(other.action), actionButton(other.actionButton), flags(other.flags),
        metaState(other.metaState), buttonState(other.buttonState),
        classification(other.classification), edgeFlags(other.edgeFlags),
        deviceTimestamp(other.deviceTimestamp), pointerCount(other.pointerCount),
        xPrecision(other.xPrecision), yPrecision(other.yPrecision), downTime(other.downTime),
        videoFrames(other.videoFrames) {
    for (uint32_t i = 0; i < pointerCount; i++) {
        pointerProperties[i].copyFrom(other.pointerProperties[i]);
        pointerCoords[i].copyFrom(other.pointerCoords[i]);
    }
}

bool NotifyMotionArgs::operator==(const NotifyMotionArgs& rhs) const {
    bool equal =
            sequenceNum == rhs.sequenceNum
            && eventTime == rhs.eventTime
            && deviceId == rhs.deviceId
            && source == rhs.source
            && displayId == rhs.displayId
            && policyFlags == rhs.policyFlags
            && action == rhs.action
            && actionButton == rhs.actionButton
            && flags == rhs.flags
            && metaState == rhs.metaState
            && buttonState == rhs.buttonState
            && classification == rhs.classification
            && edgeFlags == rhs.edgeFlags
            && deviceTimestamp == rhs.deviceTimestamp
            && pointerCount == rhs.pointerCount
            // PointerProperties and PointerCoords are compared separately below
            && xPrecision == rhs.xPrecision
            && yPrecision == rhs.yPrecision
            && downTime == rhs.downTime
            && videoFrames == rhs.videoFrames;
    if (!equal) {
        return false;
    }

    for (size_t i = 0; i < pointerCount; i++) {
        equal =
                pointerProperties[i] == rhs.pointerProperties[i]
                && pointerCoords[i] == rhs.pointerCoords[i];
        if (!equal) {
            return false;
        }
    }
    return true;
}

void NotifyMotionArgs::notify(const sp<InputListenerInterface>& listener) const {
    listener->notifyMotion(this);
}


// --- NotifySwitchArgs ---

NotifySwitchArgs::NotifySwitchArgs(uint32_t sequenceNum, nsecs_t eventTime, uint32_t policyFlags,
        uint32_t switchValues, uint32_t switchMask) :
        NotifyArgs(sequenceNum, eventTime), policyFlags(policyFlags),
        switchValues(switchValues), switchMask(switchMask) {
}

NotifySwitchArgs::NotifySwitchArgs(const NotifySwitchArgs& other) :
        NotifyArgs(other.sequenceNum, other.eventTime), policyFlags(other.policyFlags),
        switchValues(other.switchValues), switchMask(other.switchMask) {
}

bool NotifySwitchArgs::operator==(const NotifySwitchArgs rhs) const {
    return sequenceNum == rhs.sequenceNum
            && eventTime == rhs.eventTime
            && policyFlags == rhs.policyFlags
            && switchValues == rhs.switchValues
            && switchMask == rhs.switchMask;
}

void NotifySwitchArgs::notify(const sp<InputListenerInterface>& listener) const {
    listener->notifySwitch(this);
}


// --- NotifyDeviceResetArgs ---

NotifyDeviceResetArgs::NotifyDeviceResetArgs(
        uint32_t sequenceNum, nsecs_t eventTime, int32_t deviceId) :
        NotifyArgs(sequenceNum, eventTime), deviceId(deviceId) {
}

NotifyDeviceResetArgs::NotifyDeviceResetArgs(const NotifyDeviceResetArgs& other) :
        NotifyArgs(other.sequenceNum, other.eventTime), deviceId(other.deviceId) {
}

bool NotifyDeviceResetArgs::operator==(const NotifyDeviceResetArgs& rhs) const {
    return sequenceNum == rhs.sequenceNum
            && eventTime == rhs.eventTime
            && deviceId == rhs.deviceId;
}

void NotifyDeviceResetArgs::notify(const sp<InputListenerInterface>& listener) const {
    listener->notifyDeviceReset(this);
}


// --- QueuedInputListener ---

QueuedInputListener::QueuedInputListener(const sp<InputListenerInterface>& innerListener) :
        mInnerListener(innerListener) {
}

QueuedInputListener::~QueuedInputListener() {
    size_t count = mArgsQueue.size();
    for (size_t i = 0; i < count; i++) {
        delete mArgsQueue[i];
    }
}

void QueuedInputListener::notifyConfigurationChanged(
        const NotifyConfigurationChangedArgs* args) {
    mArgsQueue.push_back(new NotifyConfigurationChangedArgs(*args));
}

void QueuedInputListener::notifyKey(const NotifyKeyArgs* args) {
    mArgsQueue.push_back(new NotifyKeyArgs(*args));
}

void QueuedInputListener::notifyMotion(const NotifyMotionArgs* args) {
    mArgsQueue.push_back(new NotifyMotionArgs(*args));
}

void QueuedInputListener::notifySwitch(const NotifySwitchArgs* args) {
    mArgsQueue.push_back(new NotifySwitchArgs(*args));
}

void QueuedInputListener::notifyDeviceReset(const NotifyDeviceResetArgs* args) {
    mArgsQueue.push_back(new NotifyDeviceResetArgs(*args));
}

void QueuedInputListener::flush() {
    size_t count = mArgsQueue.size();
    for (size_t i = 0; i < count; i++) {
        NotifyArgs* args = mArgsQueue[i];
        args->notify(mInnerListener);
        delete args;
    }
    mArgsQueue.clear();
}


} // namespace android
