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

#ifndef _UI_INPUT_LISTENER_H
#define _UI_INPUT_LISTENER_H

#include <vector>

#include <input/Input.h>
#include <input/TouchVideoFrame.h>
#include <utils/RefBase.h>
#include <utils/Vector.h>

namespace android {

class InputListenerInterface;


/* Superclass of all input event argument objects */
struct NotifyArgs {
    uint32_t sequenceNum;
    nsecs_t eventTime;

    inline NotifyArgs() : sequenceNum(0), eventTime(0) { }

    inline explicit NotifyArgs(uint32_t sequenceNum, nsecs_t eventTime) :
            sequenceNum(sequenceNum), eventTime(eventTime) { }

    virtual ~NotifyArgs() { }

    virtual void notify(const sp<InputListenerInterface>& listener) const = 0;
};


/* Describes a configuration change event. */
struct NotifyConfigurationChangedArgs : public NotifyArgs {

    inline NotifyConfigurationChangedArgs() { }

    bool operator==(const NotifyConfigurationChangedArgs& rhs) const;

    NotifyConfigurationChangedArgs(uint32_t sequenceNum, nsecs_t eventTime);

    NotifyConfigurationChangedArgs(const NotifyConfigurationChangedArgs& other);

    virtual ~NotifyConfigurationChangedArgs() { }

    virtual void notify(const sp<InputListenerInterface>& listener) const;
};


/* Describes a key event. */
struct NotifyKeyArgs : public NotifyArgs {
    int32_t deviceId;
    uint32_t source;
    int32_t displayId;
    uint32_t policyFlags;
    int32_t action;
    int32_t flags;
    int32_t keyCode;
    int32_t scanCode;
    int32_t metaState;
    nsecs_t downTime;

    inline NotifyKeyArgs() { }

    NotifyKeyArgs(uint32_t sequenceNum, nsecs_t eventTime, int32_t deviceId, uint32_t source,
            int32_t displayId, uint32_t policyFlags, int32_t action, int32_t flags, int32_t keyCode,
            int32_t scanCode, int32_t metaState, nsecs_t downTime);

    bool operator==(const NotifyKeyArgs& rhs) const;

    NotifyKeyArgs(const NotifyKeyArgs& other);

    virtual ~NotifyKeyArgs() { }

    virtual void notify(const sp<InputListenerInterface>& listener) const;
};


/* Describes a motion event. */
struct NotifyMotionArgs : public NotifyArgs {
    int32_t deviceId;
    uint32_t source;
    int32_t displayId;
    uint32_t policyFlags;
    int32_t action;
    int32_t actionButton;
    int32_t flags;
    int32_t metaState;
    int32_t buttonState;
    /**
     * Classification of the current touch gesture
     */
    MotionClassification classification;
    int32_t edgeFlags;
    /**
     * A timestamp in the input device's time base, not the platform's.
     * The units are microseconds since the last reset.
     * This can only be compared to other device timestamps from the same device.
     * This value will overflow after a little over an hour.
     */
    uint32_t deviceTimestamp;
    uint32_t pointerCount;
    PointerProperties pointerProperties[MAX_POINTERS];
    PointerCoords pointerCoords[MAX_POINTERS];
    float xPrecision;
    float yPrecision;
    nsecs_t downTime;
    std::vector<TouchVideoFrame> videoFrames;

    inline NotifyMotionArgs() { }

    NotifyMotionArgs(uint32_t sequenceNum, nsecs_t eventTime, int32_t deviceId, uint32_t source,
            int32_t displayId, uint32_t policyFlags,
            int32_t action, int32_t actionButton, int32_t flags,
            int32_t metaState, int32_t buttonState, MotionClassification classification,
            int32_t edgeFlags, uint32_t deviceTimestamp, uint32_t pointerCount,
            const PointerProperties* pointerProperties, const PointerCoords* pointerCoords,
            float xPrecision, float yPrecision, nsecs_t downTime,
            const std::vector<TouchVideoFrame>& videoFrames);

    NotifyMotionArgs(const NotifyMotionArgs& other);

    virtual ~NotifyMotionArgs() { }

    bool operator==(const NotifyMotionArgs& rhs) const;

    virtual void notify(const sp<InputListenerInterface>& listener) const;
};


/* Describes a switch event. */
struct NotifySwitchArgs : public NotifyArgs {
    uint32_t policyFlags;
    uint32_t switchValues;
    uint32_t switchMask;

    inline NotifySwitchArgs() { }

    NotifySwitchArgs(uint32_t sequenceNum, nsecs_t eventTime, uint32_t policyFlags,
            uint32_t switchValues, uint32_t switchMask);

    NotifySwitchArgs(const NotifySwitchArgs& other);

    bool operator==(const NotifySwitchArgs rhs) const;

    virtual ~NotifySwitchArgs() { }

    virtual void notify(const sp<InputListenerInterface>& listener) const;
};


/* Describes a device reset event, such as when a device is added,
 * reconfigured, or removed. */
struct NotifyDeviceResetArgs : public NotifyArgs {
    int32_t deviceId;

    inline NotifyDeviceResetArgs() { }

    NotifyDeviceResetArgs(uint32_t sequenceNum, nsecs_t eventTime, int32_t deviceId);

    NotifyDeviceResetArgs(const NotifyDeviceResetArgs& other);

    bool operator==(const NotifyDeviceResetArgs& rhs) const;

    virtual ~NotifyDeviceResetArgs() { }

    virtual void notify(const sp<InputListenerInterface>& listener) const;
};


/*
 * The interface used by the InputReader to notify the InputListener about input events.
 */
class InputListenerInterface : public virtual RefBase {
protected:
    InputListenerInterface() { }
    virtual ~InputListenerInterface() { }

public:
    virtual void notifyConfigurationChanged(const NotifyConfigurationChangedArgs* args) = 0;
    virtual void notifyKey(const NotifyKeyArgs* args) = 0;
    virtual void notifyMotion(const NotifyMotionArgs* args) = 0;
    virtual void notifySwitch(const NotifySwitchArgs* args) = 0;
    virtual void notifyDeviceReset(const NotifyDeviceResetArgs* args) = 0;
};


/*
 * An implementation of the listener interface that queues up and defers dispatch
 * of decoded events until flushed.
 */
class QueuedInputListener : public InputListenerInterface {
protected:
    virtual ~QueuedInputListener();

public:
    explicit QueuedInputListener(const sp<InputListenerInterface>& innerListener);

    virtual void notifyConfigurationChanged(const NotifyConfigurationChangedArgs* args);
    virtual void notifyKey(const NotifyKeyArgs* args);
    virtual void notifyMotion(const NotifyMotionArgs* args);
    virtual void notifySwitch(const NotifySwitchArgs* args);
    virtual void notifyDeviceReset(const NotifyDeviceResetArgs* args);

    void flush();

private:
    sp<InputListenerInterface> mInnerListener;
    std::vector<NotifyArgs*> mArgsQueue;
};

} // namespace android

#endif // _UI_INPUT_LISTENER_H
