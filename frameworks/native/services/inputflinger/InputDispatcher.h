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

#ifndef _UI_INPUT_DISPATCHER_H
#define _UI_INPUT_DISPATCHER_H

#include <condition_variable>
#include <input/Input.h>
#include <input/InputApplication.h>
#include <input/InputTransport.h>
#include <input/InputWindow.h>
#include <input/ISetInputWindowsListener.h>
#include <optional>
#include <ui/Region.h>
#include <utils/threads.h>
#include <utils/Timers.h>
#include <utils/RefBase.h>
#include <utils/Looper.h>
#include <utils/BitSet.h>
#include <cutils/atomic.h>
#include <unordered_map>

#include <stddef.h>
#include <unistd.h>
#include <limits.h>
#include <unordered_map>

#include "InputListener.h"
#include "InputReporterInterface.h"

namespace android {

/*
 * Constants used to report the outcome of input event injection.
 */
enum {
    /* (INTERNAL USE ONLY) Specifies that injection is pending and its outcome is unknown. */
    INPUT_EVENT_INJECTION_PENDING = -1,

    /* Injection succeeded. */
    INPUT_EVENT_INJECTION_SUCCEEDED = 0,

    /* Injection failed because the injector did not have permission to inject
     * into the application with input focus. */
    INPUT_EVENT_INJECTION_PERMISSION_DENIED = 1,

    /* Injection failed because there were no available input targets. */
    INPUT_EVENT_INJECTION_FAILED = 2,

    /* Injection failed due to a timeout. */
    INPUT_EVENT_INJECTION_TIMED_OUT = 3
};

/*
 * Constants used to determine the input event injection synchronization mode.
 */
enum {
    /* Injection is asynchronous and is assumed always to be successful. */
    INPUT_EVENT_INJECTION_SYNC_NONE = 0,

    /* Waits for previous events to be dispatched so that the input dispatcher can determine
     * whether input event injection willbe permitted based on the current input focus.
     * Does not wait for the input event to finish processing. */
    INPUT_EVENT_INJECTION_SYNC_WAIT_FOR_RESULT = 1,

    /* Waits for the input event to be completely processed. */
    INPUT_EVENT_INJECTION_SYNC_WAIT_FOR_FINISHED = 2,
};


/*
 * An input target specifies how an input event is to be dispatched to a particular window
 * including the window's input channel, control flags, a timeout, and an X / Y offset to
 * be added to input event coordinates to compensate for the absolute position of the
 * window area.
 */
struct InputTarget {
    enum {
        /* This flag indicates that the event is being delivered to a foreground application. */
        FLAG_FOREGROUND = 1 << 0,

        /* This flag indicates that the MotionEvent falls within the area of the target
         * obscured by another visible window above it.  The motion event should be
         * delivered with flag AMOTION_EVENT_FLAG_WINDOW_IS_OBSCURED. */
        FLAG_WINDOW_IS_OBSCURED = 1 << 1,

        /* This flag indicates that a motion event is being split across multiple windows. */
        FLAG_SPLIT = 1 << 2,

        /* This flag indicates that the pointer coordinates dispatched to the application
         * will be zeroed out to avoid revealing information to an application. This is
         * used in conjunction with FLAG_DISPATCH_AS_OUTSIDE to prevent apps not sharing
         * the same UID from watching all touches. */
        FLAG_ZERO_COORDS = 1 << 3,

        /* This flag indicates that the event should be sent as is.
         * Should always be set unless the event is to be transmuted. */
        FLAG_DISPATCH_AS_IS = 1 << 8,

        /* This flag indicates that a MotionEvent with AMOTION_EVENT_ACTION_DOWN falls outside
         * of the area of this target and so should instead be delivered as an
         * AMOTION_EVENT_ACTION_OUTSIDE to this target. */
        FLAG_DISPATCH_AS_OUTSIDE = 1 << 9,

        /* This flag indicates that a hover sequence is starting in the given window.
         * The event is transmuted into ACTION_HOVER_ENTER. */
        FLAG_DISPATCH_AS_HOVER_ENTER = 1 << 10,

        /* This flag indicates that a hover event happened outside of a window which handled
         * previous hover events, signifying the end of the current hover sequence for that
         * window.
         * The event is transmuted into ACTION_HOVER_ENTER. */
        FLAG_DISPATCH_AS_HOVER_EXIT = 1 << 11,

        /* This flag indicates that the event should be canceled.
         * It is used to transmute ACTION_MOVE into ACTION_CANCEL when a touch slips
         * outside of a window. */
        FLAG_DISPATCH_AS_SLIPPERY_EXIT = 1 << 12,

        /* This flag indicates that the event should be dispatched as an initial down.
         * It is used to transmute ACTION_MOVE into ACTION_DOWN when a touch slips
         * into a new window. */
        FLAG_DISPATCH_AS_SLIPPERY_ENTER = 1 << 13,

        /* Mask for all dispatch modes. */
        FLAG_DISPATCH_MASK = FLAG_DISPATCH_AS_IS
                | FLAG_DISPATCH_AS_OUTSIDE
                | FLAG_DISPATCH_AS_HOVER_ENTER
                | FLAG_DISPATCH_AS_HOVER_EXIT
                | FLAG_DISPATCH_AS_SLIPPERY_EXIT
                | FLAG_DISPATCH_AS_SLIPPERY_ENTER,

        /* This flag indicates that the target of a MotionEvent is partly or wholly
         * obscured by another visible window above it.  The motion event should be
         * delivered with flag AMOTION_EVENT_FLAG_WINDOW_IS_PARTIALLY_OBSCURED. */
        FLAG_WINDOW_IS_PARTIALLY_OBSCURED = 1 << 14,

    };

    // The input channel to be targeted.
    sp<InputChannel> inputChannel;

    // Flags for the input target.
    int32_t flags;

    // The x and y offset to add to a MotionEvent as it is delivered.
    // (ignored for KeyEvents)
    float xOffset, yOffset;

    // Scaling factor to apply to MotionEvent as it is delivered.
    // (ignored for KeyEvents)
    float globalScaleFactor;
    float windowXScale = 1.0f;
    float windowYScale = 1.0f;

    // The subset of pointer ids to include in motion events dispatched to this input target
    // if FLAG_SPLIT is set.
    BitSet32 pointerIds;
};


/*
 * Input dispatcher configuration.
 *
 * Specifies various options that modify the behavior of the input dispatcher.
 * The values provided here are merely defaults. The actual values will come from ViewConfiguration
 * and are passed into the dispatcher during initialization.
 */
struct InputDispatcherConfiguration {
    // The key repeat initial timeout.
    nsecs_t keyRepeatTimeout;

    // The key repeat inter-key delay.
    nsecs_t keyRepeatDelay;

    InputDispatcherConfiguration() :
            keyRepeatTimeout(500 * 1000000LL),
            keyRepeatDelay(50 * 1000000LL) { }
};


/*
 * Input dispatcher policy interface.
 *
 * The input reader policy is used by the input reader to interact with the Window Manager
 * and other system components.
 *
 * The actual implementation is partially supported by callbacks into the DVM
 * via JNI.  This interface is also mocked in the unit tests.
 */
class InputDispatcherPolicyInterface : public virtual RefBase {
protected:
    InputDispatcherPolicyInterface() { }
    virtual ~InputDispatcherPolicyInterface() { }

public:
    /* Notifies the system that a configuration change has occurred. */
    virtual void notifyConfigurationChanged(nsecs_t when) = 0;

    /* Notifies the system that an application is not responding.
     * Returns a new timeout to continue waiting, or 0 to abort dispatch. */
    virtual nsecs_t notifyANR(const sp<InputApplicationHandle>& inputApplicationHandle,
            const sp<IBinder>& token,
            const std::string& reason) = 0;

    /* Notifies the system that an input channel is unrecoverably broken. */
    virtual void notifyInputChannelBroken(const sp<IBinder>& token) = 0;
    virtual void notifyFocusChanged(const sp<IBinder>& oldToken, const sp<IBinder>& newToken) = 0;

    /* Gets the input dispatcher configuration. */
    virtual void getDispatcherConfiguration(InputDispatcherConfiguration* outConfig) = 0;

    /* Filters an input event.
     * Return true to dispatch the event unmodified, false to consume the event.
     * A filter can also transform and inject events later by passing POLICY_FLAG_FILTERED
     * to injectInputEvent.
     */
    virtual bool filterInputEvent(const InputEvent* inputEvent, uint32_t policyFlags) = 0;

    /* Intercepts a key event immediately before queueing it.
     * The policy can use this method as an opportunity to perform power management functions
     * and early event preprocessing such as updating policy flags.
     *
     * This method is expected to set the POLICY_FLAG_PASS_TO_USER policy flag if the event
     * should be dispatched to applications.
     */
    virtual void interceptKeyBeforeQueueing(const KeyEvent* keyEvent, uint32_t& policyFlags) = 0;

    /* Intercepts a touch, trackball or other motion event before queueing it.
     * The policy can use this method as an opportunity to perform power management functions
     * and early event preprocessing such as updating policy flags.
     *
     * This method is expected to set the POLICY_FLAG_PASS_TO_USER policy flag if the event
     * should be dispatched to applications.
     */
    virtual void interceptMotionBeforeQueueing(const int32_t displayId, nsecs_t when,
            uint32_t& policyFlags) = 0;

    /* Allows the policy a chance to intercept a key before dispatching. */
    virtual nsecs_t interceptKeyBeforeDispatching(const sp<IBinder>& token,
            const KeyEvent* keyEvent, uint32_t policyFlags) = 0;

    /* Allows the policy a chance to perform default processing for an unhandled key.
     * Returns an alternate keycode to redispatch as a fallback, or 0 to give up. */
    virtual bool dispatchUnhandledKey(const sp<IBinder>& token,
            const KeyEvent* keyEvent, uint32_t policyFlags, KeyEvent* outFallbackKeyEvent) = 0;

    /* Notifies the policy about switch events.
     */
    virtual void notifySwitch(nsecs_t when,
            uint32_t switchValues, uint32_t switchMask, uint32_t policyFlags) = 0;

    /* Poke user activity for an event dispatched to a window. */
    virtual void pokeUserActivity(nsecs_t eventTime, int32_t eventType) = 0;

    /* Checks whether a given application pid/uid has permission to inject input events
     * into other applications.
     *
     * This method is special in that its implementation promises to be non-reentrant and
     * is safe to call while holding other locks.  (Most other methods make no such guarantees!)
     */
    virtual bool checkInjectEventsPermissionNonReentrant(
            int32_t injectorPid, int32_t injectorUid) = 0;

    /* Notifies the policy that a pointer down event has occurred outside the current focused
     * window.
     *
     * The touchedToken passed as an argument is the window that received the input event.
     */
    virtual void onPointerDownOutsideFocus(const sp<IBinder>& touchedToken) = 0;
};


/* Notifies the system about input events generated by the input reader.
 * The dispatcher is expected to be mostly asynchronous. */
class InputDispatcherInterface : public virtual RefBase, public InputListenerInterface {
protected:
    InputDispatcherInterface() { }
    virtual ~InputDispatcherInterface() { }

public:
    /* Dumps the state of the input dispatcher.
     *
     * This method may be called on any thread (usually by the input manager). */
    virtual void dump(std::string& dump) = 0;

    /* Called by the heatbeat to ensures that the dispatcher has not deadlocked. */
    virtual void monitor() = 0;

    /* Runs a single iteration of the dispatch loop.
     * Nominally processes one queued event, a timeout, or a response from an input consumer.
     *
     * This method should only be called on the input dispatcher thread.
     */
    virtual void dispatchOnce() = 0;

    /* Injects an input event and optionally waits for sync.
     * The synchronization mode determines whether the method blocks while waiting for
     * input injection to proceed.
     * Returns one of the INPUT_EVENT_INJECTION_XXX constants.
     *
     * This method may be called on any thread (usually by the input manager).
     */
    virtual int32_t injectInputEvent(const InputEvent* event,
            int32_t injectorPid, int32_t injectorUid, int32_t syncMode, int32_t timeoutMillis,
            uint32_t policyFlags) = 0;

    /* Sets the list of input windows.
     *
     * This method may be called on any thread (usually by the input manager).
     */
    virtual void setInputWindows(const std::vector<sp<InputWindowHandle> >& inputWindowHandles,
            int32_t displayId,
            const sp<ISetInputWindowsListener>& setInputWindowsListener = nullptr) = 0;

    /* Sets the focused application on the given display.
     *
     * This method may be called on any thread (usually by the input manager).
     */
    virtual void setFocusedApplication(
            int32_t displayId, const sp<InputApplicationHandle>& inputApplicationHandle) = 0;

    /* Sets the focused display.
     *
     * This method may be called on any thread (usually by the input manager).
     */
    virtual void setFocusedDisplay(int32_t displayId) = 0;

    /* Sets the input dispatching mode.
     *
     * This method may be called on any thread (usually by the input manager).
     */
    virtual void setInputDispatchMode(bool enabled, bool frozen) = 0;

    /* Sets whether input event filtering is enabled.
     * When enabled, incoming input events are sent to the policy's filterInputEvent
     * method instead of being dispatched.  The filter is expected to use
     * injectInputEvent to inject the events it would like to have dispatched.
     * It should include POLICY_FLAG_FILTERED in the policy flags during injection.
     */
    virtual void setInputFilterEnabled(bool enabled) = 0;

    /* Transfers touch focus from one window to another window.
     *
     * Returns true on success.  False if the window did not actually have touch focus.
     */
    virtual bool transferTouchFocus(const sp<IBinder>& fromToken, const sp<IBinder>& toToken) = 0;

    /* Registers input channels that may be used as targets for input events.
     *
     * This method may be called on any thread (usually by the input manager).
     */
    virtual status_t registerInputChannel(
            const sp<InputChannel>& inputChannel, int32_t displayId) = 0;

    /* Registers input channels to be used to monitor input events.
     *
     * Each monitor must target a specific display and will only receive input events sent to that
     * display. If the monitor is a gesture monitor, it will only receive pointer events on the
     * targeted display.
     *
     * This method may be called on any thread (usually by the input manager).
     */
    virtual status_t registerInputMonitor(
            const sp<InputChannel>& inputChannel, int32_t displayId, bool gestureMonitor) = 0;

    /* Unregister input channels that will no longer receive input events.
     *
     * This method may be called on any thread (usually by the input manager).
     */
    virtual status_t unregisterInputChannel(const sp<InputChannel>& inputChannel) = 0;

    /* Allows an input monitor steal the current pointer stream away from normal input windows.
     *
     * This method may be called on any thread (usually by the input manager).
     */
    virtual status_t pilferPointers(const sp<IBinder>& token) = 0;

};

/* Dispatches events to input targets.  Some functions of the input dispatcher, such as
 * identifying input targets, are controlled by a separate policy object.
 *
 * IMPORTANT INVARIANT:
 *     Because the policy can potentially block or cause re-entrance into the input dispatcher,
 *     the input dispatcher never calls into the policy while holding its internal locks.
 *     The implementation is also carefully designed to recover from scenarios such as an
 *     input channel becoming unregistered while identifying input targets or processing timeouts.
 *
 *     Methods marked 'Locked' must be called with the lock acquired.
 *
 *     Methods marked 'LockedInterruptible' must be called with the lock acquired but
 *     may during the course of their execution release the lock, call into the policy, and
 *     then reacquire the lock.  The caller is responsible for recovering gracefully.
 *
 *     A 'LockedInterruptible' method may called a 'Locked' method, but NOT vice-versa.
 */
class InputDispatcher : public InputDispatcherInterface {
protected:
    virtual ~InputDispatcher();

public:
    explicit InputDispatcher(const sp<InputDispatcherPolicyInterface>& policy);

    virtual void dump(std::string& dump) override;
    virtual void monitor() override;

    virtual void dispatchOnce() override;

    virtual void notifyConfigurationChanged(const NotifyConfigurationChangedArgs* args) override;
    virtual void notifyKey(const NotifyKeyArgs* args) override;
    virtual void notifyMotion(const NotifyMotionArgs* args) override;
    virtual void notifySwitch(const NotifySwitchArgs* args) override;
    virtual void notifyDeviceReset(const NotifyDeviceResetArgs* args) override;

    virtual int32_t injectInputEvent(const InputEvent* event,
            int32_t injectorPid, int32_t injectorUid, int32_t syncMode, int32_t timeoutMillis,
            uint32_t policyFlags) override;

    virtual void setInputWindows(const std::vector<sp<InputWindowHandle> >& inputWindowHandles,
            int32_t displayId,
            const sp<ISetInputWindowsListener>& setInputWindowsListener = nullptr) override;
    virtual void setFocusedApplication(int32_t displayId,
            const sp<InputApplicationHandle>& inputApplicationHandle) override;
    virtual void setFocusedDisplay(int32_t displayId) override;
    virtual void setInputDispatchMode(bool enabled, bool frozen) override;
    virtual void setInputFilterEnabled(bool enabled) override;

    virtual bool transferTouchFocus(const sp<IBinder>& fromToken, const sp<IBinder>& toToken)
            override;

    virtual status_t registerInputChannel(const sp<InputChannel>& inputChannel,
            int32_t displayId) override;
    virtual status_t registerInputMonitor(const sp<InputChannel>& inputChannel,
            int32_t displayId, bool isGestureMonitor) override;
    virtual status_t unregisterInputChannel(const sp<InputChannel>& inputChannel) override;
    virtual status_t pilferPointers(const sp<IBinder>& token) override;

private:
    template <typename T>
    struct Link {
        T* next;
        T* prev;

    protected:
        inline Link() : next(nullptr), prev(nullptr) { }
    };

    struct InjectionState {
        mutable int32_t refCount;

        int32_t injectorPid;
        int32_t injectorUid;
        int32_t injectionResult;  // initially INPUT_EVENT_INJECTION_PENDING
        bool injectionIsAsync; // set to true if injection is not waiting for the result
        int32_t pendingForegroundDispatches; // the number of foreground dispatches in progress

        InjectionState(int32_t injectorPid, int32_t injectorUid);
        void release();

    private:
        ~InjectionState();
    };

    struct EventEntry : Link<EventEntry> {
        enum {
            TYPE_CONFIGURATION_CHANGED,
            TYPE_DEVICE_RESET,
            TYPE_KEY,
            TYPE_MOTION
        };

        uint32_t sequenceNum;
        mutable int32_t refCount;
        int32_t type;
        nsecs_t eventTime;
        uint32_t policyFlags;
        InjectionState* injectionState;

        bool dispatchInProgress; // initially false, set to true while dispatching

        inline bool isInjected() const { return injectionState != nullptr; }

        void release();

        virtual void appendDescription(std::string& msg) const = 0;

    protected:
        EventEntry(uint32_t sequenceNum, int32_t type, nsecs_t eventTime, uint32_t policyFlags);
        virtual ~EventEntry();
        void releaseInjectionState();
    };

    struct ConfigurationChangedEntry : EventEntry {
        explicit ConfigurationChangedEntry(uint32_t sequenceNum, nsecs_t eventTime);
        virtual void appendDescription(std::string& msg) const;

    protected:
        virtual ~ConfigurationChangedEntry();
    };

    struct DeviceResetEntry : EventEntry {
        int32_t deviceId;

        DeviceResetEntry(uint32_t sequenceNum, nsecs_t eventTime, int32_t deviceId);
        virtual void appendDescription(std::string& msg) const;

    protected:
        virtual ~DeviceResetEntry();
    };

    struct KeyEntry : EventEntry {
        int32_t deviceId;
        uint32_t source;
        int32_t displayId;
        int32_t action;
        int32_t flags;
        int32_t keyCode;
        int32_t scanCode;
        int32_t metaState;
        int32_t repeatCount;
        nsecs_t downTime;

        bool syntheticRepeat; // set to true for synthetic key repeats

        enum InterceptKeyResult {
            INTERCEPT_KEY_RESULT_UNKNOWN,
            INTERCEPT_KEY_RESULT_SKIP,
            INTERCEPT_KEY_RESULT_CONTINUE,
            INTERCEPT_KEY_RESULT_TRY_AGAIN_LATER,
        };
        InterceptKeyResult interceptKeyResult; // set based on the interception result
        nsecs_t interceptKeyWakeupTime; // used with INTERCEPT_KEY_RESULT_TRY_AGAIN_LATER

        KeyEntry(uint32_t sequenceNum, nsecs_t eventTime,
                int32_t deviceId, uint32_t source, int32_t displayId, uint32_t policyFlags,
                int32_t action, int32_t flags, int32_t keyCode, int32_t scanCode, int32_t metaState,
                int32_t repeatCount, nsecs_t downTime);
        virtual void appendDescription(std::string& msg) const;
        void recycle();

    protected:
        virtual ~KeyEntry();
    };

    struct MotionEntry : EventEntry {
        nsecs_t eventTime;
        int32_t deviceId;
        uint32_t source;
        int32_t displayId;
        int32_t action;
        int32_t actionButton;
        int32_t flags;
        int32_t metaState;
        int32_t buttonState;
        MotionClassification classification;
        int32_t edgeFlags;
        float xPrecision;
        float yPrecision;
        nsecs_t downTime;
        uint32_t pointerCount;
        PointerProperties pointerProperties[MAX_POINTERS];
        PointerCoords pointerCoords[MAX_POINTERS];

        MotionEntry(uint32_t sequenceNum, nsecs_t eventTime,
                int32_t deviceId, uint32_t source, int32_t displayId, uint32_t policyFlags,
                int32_t action, int32_t actionButton, int32_t flags,
                int32_t metaState, int32_t buttonState, MotionClassification classification,
                int32_t edgeFlags, float xPrecision, float yPrecision,
                nsecs_t downTime, uint32_t pointerCount,
                const PointerProperties* pointerProperties, const PointerCoords* pointerCoords,
                float xOffset, float yOffset);
        virtual void appendDescription(std::string& msg) const;

    protected:
        virtual ~MotionEntry();
    };

    // Tracks the progress of dispatching a particular event to a particular connection.
    struct DispatchEntry : Link<DispatchEntry> {
        const uint32_t seq; // unique sequence number, never 0

        EventEntry* eventEntry; // the event to dispatch
        int32_t targetFlags;
        float xOffset;
        float yOffset;
        float globalScaleFactor;
        float windowXScale = 1.0f;
        float windowYScale = 1.0f;
        nsecs_t deliveryTime; // time when the event was actually delivered

        // Set to the resolved action and flags when the event is enqueued.
        int32_t resolvedAction;
        int32_t resolvedFlags;

        DispatchEntry(EventEntry* eventEntry,
                int32_t targetFlags, float xOffset, float yOffset,
                float globalScaleFactor, float windowXScale, float windowYScale);
        ~DispatchEntry();

        inline bool hasForegroundTarget() const {
            return targetFlags & InputTarget::FLAG_FOREGROUND;
        }

        inline bool isSplit() const {
            return targetFlags & InputTarget::FLAG_SPLIT;
        }

    private:
        static volatile int32_t sNextSeqAtomic;

        static uint32_t nextSeq();
    };

    // A command entry captures state and behavior for an action to be performed in the
    // dispatch loop after the initial processing has taken place.  It is essentially
    // a kind of continuation used to postpone sensitive policy interactions to a point
    // in the dispatch loop where it is safe to release the lock (generally after finishing
    // the critical parts of the dispatch cycle).
    //
    // The special thing about commands is that they can voluntarily release and reacquire
    // the dispatcher lock at will.  Initially when the command starts running, the
    // dispatcher lock is held.  However, if the command needs to call into the policy to
    // do some work, it can release the lock, do the work, then reacquire the lock again
    // before returning.
    //
    // This mechanism is a bit clunky but it helps to preserve the invariant that the dispatch
    // never calls into the policy while holding its lock.
    //
    // Commands are implicitly 'LockedInterruptible'.
    struct CommandEntry;
    typedef void (InputDispatcher::*Command)(CommandEntry* commandEntry);

    class Connection;
    struct CommandEntry : Link<CommandEntry> {
        explicit CommandEntry(Command command);
        ~CommandEntry();

        Command command;

        // parameters for the command (usage varies by command)
        sp<Connection> connection;
        nsecs_t eventTime;
        KeyEntry* keyEntry;
        sp<InputApplicationHandle> inputApplicationHandle;
        std::string reason;
        int32_t userActivityEventType;
        uint32_t seq;
        bool handled;
        sp<InputChannel> inputChannel;
        sp<IBinder> oldToken;
        sp<IBinder> newToken;
    };

    // Generic queue implementation.
    template <typename T>
    struct Queue {
        T* head;
        T* tail;
        uint32_t entryCount;

        inline Queue() : head(nullptr), tail(nullptr), entryCount(0) {
        }

        inline bool isEmpty() const {
            return !head;
        }

        inline void enqueueAtTail(T* entry) {
            entryCount++;
            entry->prev = tail;
            if (tail) {
                tail->next = entry;
            } else {
                head = entry;
            }
            entry->next = nullptr;
            tail = entry;
        }

        inline void enqueueAtHead(T* entry) {
            entryCount++;
            entry->next = head;
            if (head) {
                head->prev = entry;
            } else {
                tail = entry;
            }
            entry->prev = nullptr;
            head = entry;
        }

        inline void dequeue(T* entry) {
            entryCount--;
            if (entry->prev) {
                entry->prev->next = entry->next;
            } else {
                head = entry->next;
            }
            if (entry->next) {
                entry->next->prev = entry->prev;
            } else {
                tail = entry->prev;
            }
        }

        inline T* dequeueAtHead() {
            entryCount--;
            T* entry = head;
            head = entry->next;
            if (head) {
                head->prev = nullptr;
            } else {
                tail = nullptr;
            }
            return entry;
        }

        uint32_t count() const {
            return entryCount;
        }
    };

    /* Specifies which events are to be canceled and why. */
    struct CancelationOptions {
        enum Mode {
            CANCEL_ALL_EVENTS = 0,
            CANCEL_POINTER_EVENTS = 1,
            CANCEL_NON_POINTER_EVENTS = 2,
            CANCEL_FALLBACK_EVENTS = 3,
        };

        // The criterion to use to determine which events should be canceled.
        Mode mode;

        // Descriptive reason for the cancelation.
        const char* reason;

        // The specific keycode of the key event to cancel, or nullopt to cancel any key event.
        std::optional<int32_t> keyCode = std::nullopt;

        // The specific device id of events to cancel, or nullopt to cancel events from any device.
        std::optional<int32_t> deviceId = std::nullopt;

        // The specific display id of events to cancel, or nullopt to cancel events on any display.
        std::optional<int32_t> displayId = std::nullopt;

        CancelationOptions(Mode mode, const char* reason) : mode(mode), reason(reason) { }
    };

    /* Tracks dispatched key and motion event state so that cancelation events can be
     * synthesized when events are dropped. */
    class InputState {
    public:
        InputState();
        ~InputState();

        // Returns true if there is no state to be canceled.
        bool isNeutral() const;

        // Returns true if the specified source is known to have received a hover enter
        // motion event.
        bool isHovering(int32_t deviceId, uint32_t source, int32_t displayId) const;

        // Records tracking information for a key event that has just been published.
        // Returns true if the event should be delivered, false if it is inconsistent
        // and should be skipped.
        bool trackKey(const KeyEntry* entry, int32_t action, int32_t flags);

        // Records tracking information for a motion event that has just been published.
        // Returns true if the event should be delivered, false if it is inconsistent
        // and should be skipped.
        bool trackMotion(const MotionEntry* entry, int32_t action, int32_t flags);

        // Synthesizes cancelation events for the current state and resets the tracked state.
        void synthesizeCancelationEvents(nsecs_t currentTime,
                std::vector<EventEntry*>& outEvents, const CancelationOptions& options);

        // Clears the current state.
        void clear();

        // Copies pointer-related parts of the input state to another instance.
        void copyPointerStateTo(InputState& other) const;

        // Gets the fallback key associated with a keycode.
        // Returns -1 if none.
        // Returns AKEYCODE_UNKNOWN if we are only dispatching the unhandled key to the policy.
        int32_t getFallbackKey(int32_t originalKeyCode);

        // Sets the fallback key for a particular keycode.
        void setFallbackKey(int32_t originalKeyCode, int32_t fallbackKeyCode);

        // Removes the fallback key for a particular keycode.
        void removeFallbackKey(int32_t originalKeyCode);

        inline const KeyedVector<int32_t, int32_t>& getFallbackKeys() const {
            return mFallbackKeys;
        }

    private:
        struct KeyMemento {
            int32_t deviceId;
            uint32_t source;
            int32_t displayId;
            int32_t keyCode;
            int32_t scanCode;
            int32_t metaState;
            int32_t flags;
            nsecs_t downTime;
            uint32_t policyFlags;
        };

        struct MotionMemento {
            int32_t deviceId;
            uint32_t source;
            int32_t displayId;
            int32_t flags;
            float xPrecision;
            float yPrecision;
            nsecs_t downTime;
            uint32_t pointerCount;
            PointerProperties pointerProperties[MAX_POINTERS];
            PointerCoords pointerCoords[MAX_POINTERS];
            bool hovering;
            uint32_t policyFlags;

            void setPointers(const MotionEntry* entry);
        };

        std::vector<KeyMemento> mKeyMementos;
        std::vector<MotionMemento> mMotionMementos;
        KeyedVector<int32_t, int32_t> mFallbackKeys;

        ssize_t findKeyMemento(const KeyEntry* entry) const;
        ssize_t findMotionMemento(const MotionEntry* entry, bool hovering) const;

        void addKeyMemento(const KeyEntry* entry, int32_t flags);
        void addMotionMemento(const MotionEntry* entry, int32_t flags, bool hovering);

        static bool shouldCancelKey(const KeyMemento& memento,
                const CancelationOptions& options);
        static bool shouldCancelMotion(const MotionMemento& memento,
                const CancelationOptions& options);
    };

    /* Manages the dispatch state associated with a single input channel. */
    class Connection : public RefBase {
    protected:
        virtual ~Connection();

    public:
        enum Status {
            // Everything is peachy.
            STATUS_NORMAL,
            // An unrecoverable communication error has occurred.
            STATUS_BROKEN,
            // The input channel has been unregistered.
            STATUS_ZOMBIE
        };

        Status status;
        sp<InputChannel> inputChannel; // never null
        bool monitor;
        InputPublisher inputPublisher;
        InputState inputState;

        // True if the socket is full and no further events can be published until
        // the application consumes some of the input.
        bool inputPublisherBlocked;

        // Queue of events that need to be published to the connection.
        Queue<DispatchEntry> outboundQueue;

        // Queue of events that have been published to the connection but that have not
        // yet received a "finished" response from the application.
        Queue<DispatchEntry> waitQueue;

        explicit Connection(const sp<InputChannel>& inputChannel, bool monitor);

        inline const std::string getInputChannelName() const { return inputChannel->getName(); }

        const std::string getWindowName() const;
        const char* getStatusLabel() const;

        DispatchEntry* findWaitQueueEntry(uint32_t seq);
    };

    struct Monitor {
        sp<InputChannel> inputChannel; // never null

        explicit Monitor(const sp<InputChannel>& inputChannel);
    };

    enum DropReason {
        DROP_REASON_NOT_DROPPED = 0,
        DROP_REASON_POLICY = 1,
        DROP_REASON_APP_SWITCH = 2,
        DROP_REASON_DISABLED = 3,
        DROP_REASON_BLOCKED = 4,
        DROP_REASON_STALE = 5,
    };

    sp<InputDispatcherPolicyInterface> mPolicy;
    InputDispatcherConfiguration mConfig;

    std::mutex mLock;

    std::condition_variable mDispatcherIsAlive;

    sp<Looper> mLooper;

    EventEntry* mPendingEvent GUARDED_BY(mLock);
    Queue<EventEntry> mInboundQueue GUARDED_BY(mLock);
    Queue<EventEntry> mRecentQueue GUARDED_BY(mLock);
    Queue<CommandEntry> mCommandQueue GUARDED_BY(mLock);

    DropReason mLastDropReason GUARDED_BY(mLock);

    void dispatchOnceInnerLocked(nsecs_t* nextWakeupTime) REQUIRES(mLock);

    // Enqueues an inbound event.  Returns true if mLooper->wake() should be called.
    bool enqueueInboundEventLocked(EventEntry* entry) REQUIRES(mLock);

    // Cleans up input state when dropping an inbound event.
    void dropInboundEventLocked(EventEntry* entry, DropReason dropReason) REQUIRES(mLock);

    // Adds an event to a queue of recent events for debugging purposes.
    void addRecentEventLocked(EventEntry* entry) REQUIRES(mLock);

    // App switch latency optimization.
    bool mAppSwitchSawKeyDown GUARDED_BY(mLock);
    nsecs_t mAppSwitchDueTime GUARDED_BY(mLock);

    bool isAppSwitchKeyEvent(KeyEntry* keyEntry);
    bool isAppSwitchPendingLocked() REQUIRES(mLock);
    void resetPendingAppSwitchLocked(bool handled) REQUIRES(mLock);

    // Stale event latency optimization.
    static bool isStaleEvent(nsecs_t currentTime, EventEntry* entry);

    // Blocked event latency optimization.  Drops old events when the user intends
    // to transfer focus to a new application.
    EventEntry* mNextUnblockedEvent GUARDED_BY(mLock);

    sp<InputWindowHandle> findTouchedWindowAtLocked(int32_t displayId, int32_t x, int32_t y,
            bool addOutsideTargets = false, bool addPortalWindows = false) REQUIRES(mLock);

    // All registered connections mapped by channel file descriptor.
    KeyedVector<int, sp<Connection> > mConnectionsByFd GUARDED_BY(mLock);

    struct IBinderHash {
        std::size_t operator()(const sp<IBinder>& b) const {
            return std::hash<IBinder *>{}(b.get());
        }
    };
    std::unordered_map<sp<IBinder>, sp<InputChannel>, IBinderHash> mInputChannelsByToken
            GUARDED_BY(mLock);

    // Finds the display ID of the gesture monitor identified by the provided token.
    std::optional<int32_t> findGestureMonitorDisplayByTokenLocked(const sp<IBinder>& token)
            REQUIRES(mLock);

    ssize_t getConnectionIndexLocked(const sp<InputChannel>& inputChannel) REQUIRES(mLock);

    // Input channels that will receive a copy of all input events sent to the provided display.
    std::unordered_map<int32_t, std::vector<Monitor>> mGlobalMonitorsByDisplay
            GUARDED_BY(mLock);

    // Input channels that will receive pointer events that start within the corresponding display.
    // These are a bit special when compared to global monitors since they'll cause gesture streams
    // to continue even when there isn't a touched window,and have the ability to steal the rest of
    // the pointer stream in order to claim it for a system gesture.
    std::unordered_map<int32_t, std::vector<Monitor>> mGestureMonitorsByDisplay
            GUARDED_BY(mLock);


    // Event injection and synchronization.
    std::condition_variable mInjectionResultAvailable;
    bool hasInjectionPermission(int32_t injectorPid, int32_t injectorUid);
    void setInjectionResult(EventEntry* entry, int32_t injectionResult);

    std::condition_variable mInjectionSyncFinished;
    void incrementPendingForegroundDispatches(EventEntry* entry);
    void decrementPendingForegroundDispatches(EventEntry* entry);

    // Key repeat tracking.
    struct KeyRepeatState {
        KeyEntry* lastKeyEntry; // or null if no repeat
        nsecs_t nextRepeatTime;
    } mKeyRepeatState GUARDED_BY(mLock);

    void resetKeyRepeatLocked() REQUIRES(mLock);
    KeyEntry* synthesizeKeyRepeatLocked(nsecs_t currentTime) REQUIRES(mLock);

    // Key replacement tracking
    struct KeyReplacement {
        int32_t keyCode;
        int32_t deviceId;
        bool operator==(const KeyReplacement& rhs) const {
            return keyCode == rhs.keyCode && deviceId == rhs.deviceId;
        }
        bool operator<(const KeyReplacement& rhs) const {
            return keyCode != rhs.keyCode ? keyCode < rhs.keyCode : deviceId < rhs.deviceId;
        }
    };
    // Maps the key code replaced, device id tuple to the key code it was replaced with
    KeyedVector<KeyReplacement, int32_t> mReplacedKeys GUARDED_BY(mLock);
    // Process certain Meta + Key combinations
    void accelerateMetaShortcuts(const int32_t deviceId, const int32_t action,
            int32_t& keyCode, int32_t& metaState);

    // Deferred command processing.
    bool haveCommandsLocked() const REQUIRES(mLock);
    bool runCommandsLockedInterruptible() REQUIRES(mLock);
    CommandEntry* postCommandLocked(Command command) REQUIRES(mLock);

    // Input filter processing.
    bool shouldSendKeyToInputFilterLocked(const NotifyKeyArgs* args) REQUIRES(mLock);
    bool shouldSendMotionToInputFilterLocked(const NotifyMotionArgs* args) REQUIRES(mLock);

    // Inbound event processing.
    void drainInboundQueueLocked() REQUIRES(mLock);
    void releasePendingEventLocked() REQUIRES(mLock);
    void releaseInboundEventLocked(EventEntry* entry) REQUIRES(mLock);

    // Dispatch state.
    bool mDispatchEnabled GUARDED_BY(mLock);
    bool mDispatchFrozen GUARDED_BY(mLock);
    bool mInputFilterEnabled GUARDED_BY(mLock);

    std::unordered_map<int32_t, std::vector<sp<InputWindowHandle>>> mWindowHandlesByDisplay
            GUARDED_BY(mLock);
    // Get window handles by display, return an empty vector if not found.
    std::vector<sp<InputWindowHandle>> getWindowHandlesLocked(int32_t displayId) const
            REQUIRES(mLock);
    sp<InputWindowHandle> getWindowHandleLocked(const sp<IBinder>& windowHandleToken) const
            REQUIRES(mLock);
    sp<InputChannel> getInputChannelLocked(const sp<IBinder>& windowToken) const REQUIRES(mLock);
    bool hasWindowHandleLocked(const sp<InputWindowHandle>& windowHandle) const REQUIRES(mLock);

    // Focus tracking for keys, trackball, etc.
    std::unordered_map<int32_t, sp<InputWindowHandle>> mFocusedWindowHandlesByDisplay
            GUARDED_BY(mLock);

    // Focus tracking for touch.
    struct TouchedWindow {
        sp<InputWindowHandle> windowHandle;
        int32_t targetFlags;
        BitSet32 pointerIds;        // zero unless target flag FLAG_SPLIT is set
    };

    // For tracking the offsets we need to apply when adding gesture monitor targets.
    struct TouchedMonitor {
        Monitor monitor;
        float xOffset = 0.f;
        float yOffset = 0.f;

        explicit TouchedMonitor(const Monitor& monitor, float xOffset, float yOffset);
    };

    struct TouchState {
        bool down;
        bool split;
        int32_t deviceId; // id of the device that is currently down, others are rejected
        uint32_t source;  // source of the device that is current down, others are rejected
        int32_t displayId; // id to the display that currently has a touch, others are rejected
        std::vector<TouchedWindow> windows;

        // This collects the portal windows that the touch has gone through. Each portal window
        // targets a display (embedded display for most cases). With this info, we can add the
        // monitoring channels of the displays touched.
        std::vector<sp<InputWindowHandle>> portalWindows;

        std::vector<TouchedMonitor> gestureMonitors;

        TouchState();
        ~TouchState();
        void reset();
        void copyFrom(const TouchState& other);
        void addOrUpdateWindow(const sp<InputWindowHandle>& windowHandle,
                int32_t targetFlags, BitSet32 pointerIds);
        void addPortalWindow(const sp<InputWindowHandle>& windowHandle);
        void addGestureMonitors(const std::vector<TouchedMonitor>& monitors);
        void removeWindow(const sp<InputWindowHandle>& windowHandle);
        void removeWindowByToken(const sp<IBinder>& token);
        void filterNonAsIsTouchWindows();
        void filterNonMonitors();
        sp<InputWindowHandle> getFirstForegroundWindowHandle() const;
        bool isSlippery() const;
    };

    KeyedVector<int32_t, TouchState> mTouchStatesByDisplay GUARDED_BY(mLock);
    TouchState mTempTouchState GUARDED_BY(mLock);

    // Focused applications.
    std::unordered_map<int32_t, sp<InputApplicationHandle>> mFocusedApplicationHandlesByDisplay
            GUARDED_BY(mLock);

    // Top focused display.
    int32_t mFocusedDisplayId GUARDED_BY(mLock);

    // Dispatcher state at time of last ANR.
    std::string mLastANRState GUARDED_BY(mLock);

    // Dispatch inbound events.
    bool dispatchConfigurationChangedLocked(
            nsecs_t currentTime, ConfigurationChangedEntry* entry) REQUIRES(mLock);
    bool dispatchDeviceResetLocked(
            nsecs_t currentTime, DeviceResetEntry* entry) REQUIRES(mLock);
    bool dispatchKeyLocked(
            nsecs_t currentTime, KeyEntry* entry,
            DropReason* dropReason, nsecs_t* nextWakeupTime) REQUIRES(mLock);
    bool dispatchMotionLocked(
            nsecs_t currentTime, MotionEntry* entry,
            DropReason* dropReason, nsecs_t* nextWakeupTime) REQUIRES(mLock);
    void dispatchEventLocked(nsecs_t currentTime, EventEntry* entry,
            const std::vector<InputTarget>& inputTargets) REQUIRES(mLock);

    void logOutboundKeyDetails(const char* prefix, const KeyEntry* entry);
    void logOutboundMotionDetails(const char* prefix, const MotionEntry* entry);

    // Keeping track of ANR timeouts.
    enum InputTargetWaitCause {
        INPUT_TARGET_WAIT_CAUSE_NONE,
        INPUT_TARGET_WAIT_CAUSE_SYSTEM_NOT_READY,
        INPUT_TARGET_WAIT_CAUSE_APPLICATION_NOT_READY,
    };

    InputTargetWaitCause mInputTargetWaitCause GUARDED_BY(mLock);
    nsecs_t mInputTargetWaitStartTime GUARDED_BY(mLock);
    nsecs_t mInputTargetWaitTimeoutTime GUARDED_BY(mLock);
    bool mInputTargetWaitTimeoutExpired GUARDED_BY(mLock);
    sp<IBinder> mInputTargetWaitApplicationToken GUARDED_BY(mLock);

    // Contains the last window which received a hover event.
    sp<InputWindowHandle> mLastHoverWindowHandle GUARDED_BY(mLock);

    // Finding targets for input events.
    int32_t handleTargetsNotReadyLocked(nsecs_t currentTime, const EventEntry* entry,
            const sp<InputApplicationHandle>& applicationHandle,
            const sp<InputWindowHandle>& windowHandle,
            nsecs_t* nextWakeupTime, const char* reason) REQUIRES(mLock);

    void removeWindowByTokenLocked(const sp<IBinder>& token) REQUIRES(mLock);

    void resumeAfterTargetsNotReadyTimeoutLocked(nsecs_t newTimeout,
            const sp<InputChannel>& inputChannel) REQUIRES(mLock);
    nsecs_t getTimeSpentWaitingForApplicationLocked(nsecs_t currentTime) REQUIRES(mLock);
    void resetANRTimeoutsLocked() REQUIRES(mLock);

    int32_t getTargetDisplayId(const EventEntry* entry);
    int32_t findFocusedWindowTargetsLocked(nsecs_t currentTime, const EventEntry* entry,
            std::vector<InputTarget>& inputTargets, nsecs_t* nextWakeupTime) REQUIRES(mLock);
    int32_t findTouchedWindowTargetsLocked(nsecs_t currentTime, const MotionEntry* entry,
            std::vector<InputTarget>& inputTargets, nsecs_t* nextWakeupTime,
            bool* outConflictingPointerActions) REQUIRES(mLock);
    std::vector<TouchedMonitor> findTouchedGestureMonitorsLocked(int32_t displayId,
            const std::vector<sp<InputWindowHandle>>& portalWindows) REQUIRES(mLock);
    void addGestureMonitors(const std::vector<Monitor>& monitors,
            std::vector<TouchedMonitor>& outTouchedMonitors, float xOffset = 0, float yOffset = 0);

    void addWindowTargetLocked(const sp<InputWindowHandle>& windowHandle,
            int32_t targetFlags, BitSet32 pointerIds, std::vector<InputTarget>& inputTargets)
            REQUIRES(mLock);
    void addMonitoringTargetLocked(const Monitor& monitor, float xOffset, float yOffset,
            std::vector<InputTarget>& inputTargets) REQUIRES(mLock);
    void addGlobalMonitoringTargetsLocked(std::vector<InputTarget>& inputTargets,
            int32_t displayId, float xOffset = 0, float yOffset = 0) REQUIRES(mLock);

    void pokeUserActivityLocked(const EventEntry* eventEntry) REQUIRES(mLock);
    bool checkInjectionPermission(const sp<InputWindowHandle>& windowHandle,
            const InjectionState* injectionState);
    bool isWindowObscuredAtPointLocked(const sp<InputWindowHandle>& windowHandle,
            int32_t x, int32_t y) const REQUIRES(mLock);
    bool isWindowObscuredLocked(const sp<InputWindowHandle>& windowHandle) const REQUIRES(mLock);
    std::string getApplicationWindowLabel(const sp<InputApplicationHandle>& applicationHandle,
            const sp<InputWindowHandle>& windowHandle);

    std::string checkWindowReadyForMoreInputLocked(nsecs_t currentTime,
            const sp<InputWindowHandle>& windowHandle, const EventEntry* eventEntry,
            const char* targetType) REQUIRES(mLock);

    // Manage the dispatch cycle for a single connection.
    // These methods are deliberately not Interruptible because doing all of the work
    // with the mutex held makes it easier to ensure that connection invariants are maintained.
    // If needed, the methods post commands to run later once the critical bits are done.
    void prepareDispatchCycleLocked(nsecs_t currentTime, const sp<Connection>& connection,
            EventEntry* eventEntry, const InputTarget* inputTarget) REQUIRES(mLock);
    void enqueueDispatchEntriesLocked(nsecs_t currentTime, const sp<Connection>& connection,
            EventEntry* eventEntry, const InputTarget* inputTarget) REQUIRES(mLock);
    void enqueueDispatchEntryLocked(const sp<Connection>& connection,
            EventEntry* eventEntry, const InputTarget* inputTarget, int32_t dispatchMode)
            REQUIRES(mLock);
    void startDispatchCycleLocked(nsecs_t currentTime, const sp<Connection>& connection)
            REQUIRES(mLock);
    void finishDispatchCycleLocked(nsecs_t currentTime, const sp<Connection>& connection,
            uint32_t seq, bool handled) REQUIRES(mLock);
    void abortBrokenDispatchCycleLocked(nsecs_t currentTime, const sp<Connection>& connection,
            bool notify) REQUIRES(mLock);
    void drainDispatchQueue(Queue<DispatchEntry>* queue);
    void releaseDispatchEntry(DispatchEntry* dispatchEntry);
    static int handleReceiveCallback(int fd, int events, void* data);
    // The action sent should only be of type AMOTION_EVENT_*
    void dispatchPointerDownOutsideFocus(uint32_t source, int32_t action,
            const sp<IBinder>& newToken) REQUIRES(mLock);

    void synthesizeCancelationEventsForAllConnectionsLocked(
            const CancelationOptions& options) REQUIRES(mLock);
    void synthesizeCancelationEventsForMonitorsLocked(
            const CancelationOptions& options) REQUIRES(mLock);
    void synthesizeCancelationEventsForMonitorsLocked(const CancelationOptions& options,
            std::unordered_map<int32_t, std::vector<Monitor>>& monitorsByDisplay) REQUIRES(mLock);
    void synthesizeCancelationEventsForInputChannelLocked(const sp<InputChannel>& channel,
            const CancelationOptions& options) REQUIRES(mLock);
    void synthesizeCancelationEventsForConnectionLocked(const sp<Connection>& connection,
            const CancelationOptions& options) REQUIRES(mLock);

    // Splitting motion events across windows.
    MotionEntry* splitMotionEvent(const MotionEntry* originalMotionEntry, BitSet32 pointerIds);

    // Reset and drop everything the dispatcher is doing.
    void resetAndDropEverythingLocked(const char* reason) REQUIRES(mLock);

    // Dump state.
    void dumpDispatchStateLocked(std::string& dump) REQUIRES(mLock);
    void dumpMonitors(std::string& dump, const std::vector<Monitor>& monitors);
    void logDispatchStateLocked() REQUIRES(mLock);

    // Registration.
    void removeMonitorChannelLocked(const sp<InputChannel>& inputChannel) REQUIRES(mLock);
    void removeMonitorChannelLocked(const sp<InputChannel>& inputChannel,
        std::unordered_map<int32_t, std::vector<Monitor>>& monitorsByDisplay)
            REQUIRES(mLock);
    status_t unregisterInputChannelLocked(const sp<InputChannel>& inputChannel, bool notify)
            REQUIRES(mLock);

    // Interesting events that we might like to log or tell the framework about.
    void onDispatchCycleFinishedLocked(
            nsecs_t currentTime, const sp<Connection>& connection, uint32_t seq, bool handled)
             REQUIRES(mLock);
    void onDispatchCycleBrokenLocked(
            nsecs_t currentTime, const sp<Connection>& connection) REQUIRES(mLock);
    void onFocusChangedLocked(const sp<InputWindowHandle>& oldFocus,
            const sp<InputWindowHandle>& newFocus) REQUIRES(mLock);
    void onANRLocked(
            nsecs_t currentTime, const sp<InputApplicationHandle>& applicationHandle,
            const sp<InputWindowHandle>& windowHandle,
            nsecs_t eventTime, nsecs_t waitStartTime, const char* reason) REQUIRES(mLock);

    // Outbound policy interactions.
    void doNotifyConfigurationChangedLockedInterruptible(CommandEntry* commandEntry)
            REQUIRES(mLock);
    void doNotifyInputChannelBrokenLockedInterruptible(CommandEntry* commandEntry) REQUIRES(mLock);
    void doNotifyFocusChangedLockedInterruptible(CommandEntry* commandEntry) REQUIRES(mLock);
    void doNotifyANRLockedInterruptible(CommandEntry* commandEntry) REQUIRES(mLock);
    void doInterceptKeyBeforeDispatchingLockedInterruptible(CommandEntry* commandEntry)
            REQUIRES(mLock);
    void doDispatchCycleFinishedLockedInterruptible(CommandEntry* commandEntry) REQUIRES(mLock);
    bool afterKeyEventLockedInterruptible(const sp<Connection>& connection,
            DispatchEntry* dispatchEntry, KeyEntry* keyEntry, bool handled) REQUIRES(mLock);
    bool afterMotionEventLockedInterruptible(const sp<Connection>& connection,
            DispatchEntry* dispatchEntry, MotionEntry* motionEntry, bool handled) REQUIRES(mLock);
    void doPokeUserActivityLockedInterruptible(CommandEntry* commandEntry) REQUIRES(mLock);
    void initializeKeyEvent(KeyEvent* event, const KeyEntry* entry);
    void doOnPointerDownOutsideFocusLockedInterruptible(CommandEntry* commandEntry)
            REQUIRES(mLock);

    // Statistics gathering.
    void updateDispatchStatistics(nsecs_t currentTime, const EventEntry* entry,
            int32_t injectionResult, nsecs_t timeSpentWaitingForApplication);
    void traceInboundQueueLengthLocked() REQUIRES(mLock);
    void traceOutboundQueueLength(const sp<Connection>& connection);
    void traceWaitQueueLength(const sp<Connection>& connection);

    sp<InputReporterInterface> mReporter;
};

/* Enqueues and dispatches input events, endlessly. */
class InputDispatcherThread : public Thread {
public:
    explicit InputDispatcherThread(const sp<InputDispatcherInterface>& dispatcher);
    ~InputDispatcherThread();

private:
    virtual bool threadLoop();

    sp<InputDispatcherInterface> mDispatcher;
};

} // namespace android

#endif // _UI_INPUT_DISPATCHER_H
