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

#define LOG_TAG "InputDispatcher"
#define ATRACE_TAG ATRACE_TAG_INPUT

#define LOG_NDEBUG 0

// Log detailed debug messages about each inbound event notification to the dispatcher.
#define DEBUG_INBOUND_EVENT_DETAILS 0

// Log detailed debug messages about each outbound event processed by the dispatcher.
#define DEBUG_OUTBOUND_EVENT_DETAILS 0

// Log debug messages about the dispatch cycle.
#define DEBUG_DISPATCH_CYCLE 0

// Log debug messages about registrations.
#define DEBUG_REGISTRATION 0

// Log debug messages about input event injection.
#define DEBUG_INJECTION 0

// Log debug messages about input focus tracking.
#define DEBUG_FOCUS 0

// Log debug messages about the app switch latency optimization.
#define DEBUG_APP_SWITCH 0

// Log debug messages about hover events.
#define DEBUG_HOVER 0

#include "InputDispatcher.h"

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <sstream>
#include <stddef.h>
#include <time.h>
#include <unistd.h>

#include <android-base/chrono_utils.h>
#include <android-base/stringprintf.h>
#include <log/log.h>
#include <utils/Trace.h>
#include <powermanager/PowerManager.h>
#include <binder/Binder.h>

#define INDENT "  "
#define INDENT2 "    "
#define INDENT3 "      "
#define INDENT4 "        "

using android::base::StringPrintf;

namespace android {

// Default input dispatching timeout if there is no focused application or paused window
// from which to determine an appropriate dispatching timeout.
constexpr nsecs_t DEFAULT_INPUT_DISPATCHING_TIMEOUT = 5000 * 1000000LL; // 5 sec

// Amount of time to allow for all pending events to be processed when an app switch
// key is on the way.  This is used to preempt input dispatch and drop input events
// when an application takes too long to respond and the user has pressed an app switch key.
constexpr nsecs_t APP_SWITCH_TIMEOUT = 500 * 1000000LL; // 0.5sec

// Amount of time to allow for an event to be dispatched (measured since its eventTime)
// before considering it stale and dropping it.
constexpr nsecs_t STALE_EVENT_TIMEOUT = 10000 * 1000000LL; // 10sec

// Amount of time to allow touch events to be streamed out to a connection before requiring
// that the first event be finished.  This value extends the ANR timeout by the specified
// amount.  For example, if streaming is allowed to get ahead by one second relative to the
// queue of waiting unfinished events, then ANRs will similarly be delayed by one second.
constexpr nsecs_t STREAM_AHEAD_EVENT_TIMEOUT = 500 * 1000000LL; // 0.5sec

// Log a warning when an event takes longer than this to process, even if an ANR does not occur.
constexpr nsecs_t SLOW_EVENT_PROCESSING_WARNING_TIMEOUT = 2000 * 1000000LL; // 2sec

// Log a warning when an interception call takes longer than this to process.
constexpr std::chrono::milliseconds SLOW_INTERCEPTION_THRESHOLD = 50ms;

// Number of recent events to keep for debugging purposes.
constexpr size_t RECENT_QUEUE_MAX_SIZE = 10;

// Sequence number for synthesized or injected events.
constexpr uint32_t SYNTHESIZED_EVENT_SEQUENCE_NUM = 0;


static inline nsecs_t now() {
    return systemTime(SYSTEM_TIME_MONOTONIC);
}

static inline const char* toString(bool value) {
    return value ? "true" : "false";
}

static std::string dispatchModeToString(int32_t dispatchMode) {
    switch (dispatchMode) {
        case InputTarget::FLAG_DISPATCH_AS_IS:
            return "DISPATCH_AS_IS";
        case InputTarget::FLAG_DISPATCH_AS_OUTSIDE:
            return "DISPATCH_AS_OUTSIDE";
        case InputTarget::FLAG_DISPATCH_AS_HOVER_ENTER:
            return "DISPATCH_AS_HOVER_ENTER";
        case InputTarget::FLAG_DISPATCH_AS_HOVER_EXIT:
            return "DISPATCH_AS_HOVER_EXIT";
        case InputTarget::FLAG_DISPATCH_AS_SLIPPERY_EXIT:
            return "DISPATCH_AS_SLIPPERY_EXIT";
        case InputTarget::FLAG_DISPATCH_AS_SLIPPERY_ENTER:
            return "DISPATCH_AS_SLIPPERY_ENTER";
    }
    return StringPrintf("%" PRId32, dispatchMode);
}

static inline int32_t getMotionEventActionPointerIndex(int32_t action) {
    return (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
            >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
}

static bool isValidKeyAction(int32_t action) {
    switch (action) {
    case AKEY_EVENT_ACTION_DOWN:
    case AKEY_EVENT_ACTION_UP:
        return true;
    default:
        return false;
    }
}

static bool validateKeyEvent(int32_t action) {
    if (! isValidKeyAction(action)) {
        ALOGE("Key event has invalid action code 0x%x", action);
        return false;
    }
    return true;
}

static bool isValidMotionAction(int32_t action, int32_t actionButton, int32_t pointerCount) {
    switch (action & AMOTION_EVENT_ACTION_MASK) {
    case AMOTION_EVENT_ACTION_DOWN:
    case AMOTION_EVENT_ACTION_UP:
    case AMOTION_EVENT_ACTION_CANCEL:
    case AMOTION_EVENT_ACTION_MOVE:
    case AMOTION_EVENT_ACTION_OUTSIDE:
    case AMOTION_EVENT_ACTION_HOVER_ENTER:
    case AMOTION_EVENT_ACTION_HOVER_MOVE:
    case AMOTION_EVENT_ACTION_HOVER_EXIT:
    case AMOTION_EVENT_ACTION_SCROLL:
        return true;
    case AMOTION_EVENT_ACTION_POINTER_DOWN:
    case AMOTION_EVENT_ACTION_POINTER_UP: {
        int32_t index = getMotionEventActionPointerIndex(action);
        return index >= 0 && index < pointerCount;
    }
    case AMOTION_EVENT_ACTION_BUTTON_PRESS:
    case AMOTION_EVENT_ACTION_BUTTON_RELEASE:
        return actionButton != 0;
    default:
        return false;
    }
}

static bool validateMotionEvent(int32_t action, int32_t actionButton, size_t pointerCount,
        const PointerProperties* pointerProperties) {
    if (! isValidMotionAction(action, actionButton, pointerCount)) {
        ALOGE("Motion event has invalid action code 0x%x", action);
        return false;
    }
    if (pointerCount < 1 || pointerCount > MAX_POINTERS) {
        ALOGE("Motion event has invalid pointer count %zu; value must be between 1 and %d.",
                pointerCount, MAX_POINTERS);
        return false;
    }
    BitSet32 pointerIdBits;
    for (size_t i = 0; i < pointerCount; i++) {
        int32_t id = pointerProperties[i].id;
        if (id < 0 || id > MAX_POINTER_ID) {
            ALOGE("Motion event has invalid pointer id %d; value must be between 0 and %d",
                    id, MAX_POINTER_ID);
            return false;
        }
        if (pointerIdBits.hasBit(id)) {
            ALOGE("Motion event has duplicate pointer id %d", id);
            return false;
        }
        pointerIdBits.markBit(id);
    }
    return true;
}

static void dumpRegion(std::string& dump, const Region& region) {
    if (region.isEmpty()) {
        dump += "<empty>";
        return;
    }

    bool first = true;
    Region::const_iterator cur = region.begin();
    Region::const_iterator const tail = region.end();
    while (cur != tail) {
        if (first) {
            first = false;
        } else {
            dump += "|";
        }
        dump += StringPrintf("[%d,%d][%d,%d]", cur->left, cur->top, cur->right, cur->bottom);
        cur++;
    }
}

template<typename T, typename U>
static T getValueByKey(std::unordered_map<U, T>& map, U key) {
    typename std::unordered_map<U, T>::const_iterator it = map.find(key);
    return it != map.end() ? it->second : T{};
}


// --- InputDispatcher ---

InputDispatcher::InputDispatcher(const sp<InputDispatcherPolicyInterface>& policy) :
    mPolicy(policy),
    mPendingEvent(nullptr), mLastDropReason(DROP_REASON_NOT_DROPPED),
    mAppSwitchSawKeyDown(false), mAppSwitchDueTime(LONG_LONG_MAX),
    mNextUnblockedEvent(nullptr),
    mDispatchEnabled(false), mDispatchFrozen(false), mInputFilterEnabled(false),
    mFocusedDisplayId(ADISPLAY_ID_DEFAULT),
    mInputTargetWaitCause(INPUT_TARGET_WAIT_CAUSE_NONE) {
    mLooper = new Looper(false);
    mReporter = createInputReporter();

    mKeyRepeatState.lastKeyEntry = nullptr;

    policy->getDispatcherConfiguration(&mConfig);
}

InputDispatcher::~InputDispatcher() {
    { // acquire lock
        std::scoped_lock _l(mLock);

        resetKeyRepeatLocked();
        releasePendingEventLocked();
        drainInboundQueueLocked();
    }

    while (mConnectionsByFd.size() != 0) {
        unregisterInputChannel(mConnectionsByFd.valueAt(0)->inputChannel);
    }
}

void InputDispatcher::dispatchOnce() {
    nsecs_t nextWakeupTime = LONG_LONG_MAX;
    { // acquire lock
        std::scoped_lock _l(mLock);
        mDispatcherIsAlive.notify_all();

        // Run a dispatch loop if there are no pending commands.
        // The dispatch loop might enqueue commands to run afterwards.
        if (!haveCommandsLocked()) {
            dispatchOnceInnerLocked(&nextWakeupTime);
        }

        // Run all pending commands if there are any.
        // If any commands were run then force the next poll to wake up immediately.
        if (runCommandsLockedInterruptible()) {
            nextWakeupTime = LONG_LONG_MIN;
        }
    } // release lock

    // Wait for callback or timeout or wake.  (make sure we round up, not down)
    nsecs_t currentTime = now();
    int timeoutMillis = toMillisecondTimeoutDelay(currentTime, nextWakeupTime);
    mLooper->pollOnce(timeoutMillis);
}

void InputDispatcher::dispatchOnceInnerLocked(nsecs_t* nextWakeupTime) {
    nsecs_t currentTime = now();

    // Reset the key repeat timer whenever normal dispatch is suspended while the
    // device is in a non-interactive state.  This is to ensure that we abort a key
    // repeat if the device is just coming out of sleep.
    if (!mDispatchEnabled) {
        resetKeyRepeatLocked();
    }

    // If dispatching is frozen, do not process timeouts or try to deliver any new events.
    if (mDispatchFrozen) {
#if DEBUG_FOCUS
        ALOGD("Dispatch frozen.  Waiting some more.");
#endif
        return;
    }

    // Optimize latency of app switches.
    // Essentially we start a short timeout when an app switch key (HOME / ENDCALL) has
    // been pressed.  When it expires, we preempt dispatch and drop all other pending events.
    bool isAppSwitchDue = mAppSwitchDueTime <= currentTime;
    if (mAppSwitchDueTime < *nextWakeupTime) {
        *nextWakeupTime = mAppSwitchDueTime;
    }

    // Ready to start a new event.
    // If we don't already have a pending event, go grab one.
    if (! mPendingEvent) {
        if (mInboundQueue.isEmpty()) {
            if (isAppSwitchDue) {
                // The inbound queue is empty so the app switch key we were waiting
                // for will never arrive.  Stop waiting for it.
                resetPendingAppSwitchLocked(false);
                isAppSwitchDue = false;
            }

            // Synthesize a key repeat if appropriate.
            if (mKeyRepeatState.lastKeyEntry) {
                if (currentTime >= mKeyRepeatState.nextRepeatTime) {
                    mPendingEvent = synthesizeKeyRepeatLocked(currentTime);
                } else {
                    if (mKeyRepeatState.nextRepeatTime < *nextWakeupTime) {
                        *nextWakeupTime = mKeyRepeatState.nextRepeatTime;
                    }
                }
            }

            // Nothing to do if there is no pending event.
            if (!mPendingEvent) {
                return;
            }
        } else {
            // Inbound queue has at least one entry.
            mPendingEvent = mInboundQueue.dequeueAtHead();
            traceInboundQueueLengthLocked();
        }

        // Poke user activity for this event.
        if (mPendingEvent->policyFlags & POLICY_FLAG_PASS_TO_USER) {
            pokeUserActivityLocked(mPendingEvent);
        }

        // Get ready to dispatch the event.
        resetANRTimeoutsLocked();
    }

    // Now we have an event to dispatch.
    // All events are eventually dequeued and processed this way, even if we intend to drop them.
    ALOG_ASSERT(mPendingEvent != nullptr);
    bool done = false;
    DropReason dropReason = DROP_REASON_NOT_DROPPED;
    if (!(mPendingEvent->policyFlags & POLICY_FLAG_PASS_TO_USER)) {
        dropReason = DROP_REASON_POLICY;
    } else if (!mDispatchEnabled) {
        dropReason = DROP_REASON_DISABLED;
    }

    if (mNextUnblockedEvent == mPendingEvent) {
        mNextUnblockedEvent = nullptr;
    }

    switch (mPendingEvent->type) {
    case EventEntry::TYPE_CONFIGURATION_CHANGED: {
        ConfigurationChangedEntry* typedEntry =
                static_cast<ConfigurationChangedEntry*>(mPendingEvent);
        done = dispatchConfigurationChangedLocked(currentTime, typedEntry);
        dropReason = DROP_REASON_NOT_DROPPED; // configuration changes are never dropped
        break;
    }

    case EventEntry::TYPE_DEVICE_RESET: {
        DeviceResetEntry* typedEntry =
                static_cast<DeviceResetEntry*>(mPendingEvent);
        done = dispatchDeviceResetLocked(currentTime, typedEntry);
        dropReason = DROP_REASON_NOT_DROPPED; // device resets are never dropped
        break;
    }

    case EventEntry::TYPE_KEY: {
        KeyEntry* typedEntry = static_cast<KeyEntry*>(mPendingEvent);
        if (isAppSwitchDue) {
            if (isAppSwitchKeyEvent(typedEntry)) {
                resetPendingAppSwitchLocked(true);
                isAppSwitchDue = false;
            } else if (dropReason == DROP_REASON_NOT_DROPPED) {
                dropReason = DROP_REASON_APP_SWITCH;
            }
        }
        if (dropReason == DROP_REASON_NOT_DROPPED
                && isStaleEvent(currentTime, typedEntry)) {
            dropReason = DROP_REASON_STALE;
        }
        if (dropReason == DROP_REASON_NOT_DROPPED && mNextUnblockedEvent) {
            dropReason = DROP_REASON_BLOCKED;
        }
        done = dispatchKeyLocked(currentTime, typedEntry, &dropReason, nextWakeupTime);
        break;
    }

    case EventEntry::TYPE_MOTION: {
        MotionEntry* typedEntry = static_cast<MotionEntry*>(mPendingEvent);
        if (dropReason == DROP_REASON_NOT_DROPPED && isAppSwitchDue) {
            dropReason = DROP_REASON_APP_SWITCH;
        }
        if (dropReason == DROP_REASON_NOT_DROPPED
                && isStaleEvent(currentTime, typedEntry)) {
            dropReason = DROP_REASON_STALE;
        }
        if (dropReason == DROP_REASON_NOT_DROPPED && mNextUnblockedEvent) {
            dropReason = DROP_REASON_BLOCKED;
        }
        done = dispatchMotionLocked(currentTime, typedEntry,
                &dropReason, nextWakeupTime);
        break;
    }

    default:
        ALOG_ASSERT(false);
        break;
    }

    if (done) {
        if (dropReason != DROP_REASON_NOT_DROPPED) {
            dropInboundEventLocked(mPendingEvent, dropReason);
        }
        mLastDropReason = dropReason;

        releasePendingEventLocked();
        *nextWakeupTime = LONG_LONG_MIN;  // force next poll to wake up immediately
    }
}

bool InputDispatcher::enqueueInboundEventLocked(EventEntry* entry) {
    bool needWake = mInboundQueue.isEmpty();
    mInboundQueue.enqueueAtTail(entry);
    traceInboundQueueLengthLocked();

    switch (entry->type) {
    case EventEntry::TYPE_KEY: {
        // Optimize app switch latency.
        // If the application takes too long to catch up then we drop all events preceding
        // the app switch key.
        KeyEntry* keyEntry = static_cast<KeyEntry*>(entry);
        if (isAppSwitchKeyEvent(keyEntry)) {
            if (keyEntry->action == AKEY_EVENT_ACTION_DOWN) {
                mAppSwitchSawKeyDown = true;
            } else if (keyEntry->action == AKEY_EVENT_ACTION_UP) {
                if (mAppSwitchSawKeyDown) {
#if DEBUG_APP_SWITCH
                    ALOGD("App switch is pending!");
#endif
                    mAppSwitchDueTime = keyEntry->eventTime + APP_SWITCH_TIMEOUT;
                    mAppSwitchSawKeyDown = false;
                    needWake = true;
                }
            }
        }
        break;
    }

    case EventEntry::TYPE_MOTION: {
        // Optimize case where the current application is unresponsive and the user
        // decides to touch a window in a different application.
        // If the application takes too long to catch up then we drop all events preceding
        // the touch into the other window.
        MotionEntry* motionEntry = static_cast<MotionEntry*>(entry);
        if (motionEntry->action == AMOTION_EVENT_ACTION_DOWN
                && (motionEntry->source & AINPUT_SOURCE_CLASS_POINTER)
                && mInputTargetWaitCause == INPUT_TARGET_WAIT_CAUSE_APPLICATION_NOT_READY
                && mInputTargetWaitApplicationToken != nullptr) {
            int32_t displayId = motionEntry->displayId;
            int32_t x = int32_t(motionEntry->pointerCoords[0].
                    getAxisValue(AMOTION_EVENT_AXIS_X));
            int32_t y = int32_t(motionEntry->pointerCoords[0].
                    getAxisValue(AMOTION_EVENT_AXIS_Y));
            sp<InputWindowHandle> touchedWindowHandle = findTouchedWindowAtLocked(displayId, x, y);
            if (touchedWindowHandle != nullptr
                    && touchedWindowHandle->getApplicationToken()
                            != mInputTargetWaitApplicationToken) {
                // User touched a different application than the one we are waiting on.
                // Flag the event, and start pruning the input queue.
                mNextUnblockedEvent = motionEntry;
                needWake = true;
            }
        }
        break;
    }
    }

    return needWake;
}

void InputDispatcher::addRecentEventLocked(EventEntry* entry) {
    entry->refCount += 1;
    mRecentQueue.enqueueAtTail(entry);
    if (mRecentQueue.count() > RECENT_QUEUE_MAX_SIZE) {
        mRecentQueue.dequeueAtHead()->release();
    }
}

sp<InputWindowHandle> InputDispatcher::findTouchedWindowAtLocked(int32_t displayId,
        int32_t x, int32_t y, bool addOutsideTargets, bool addPortalWindows) {
    // Traverse windows from front to back to find touched window.
    const std::vector<sp<InputWindowHandle>> windowHandles = getWindowHandlesLocked(displayId);
    for (const sp<InputWindowHandle>& windowHandle : windowHandles) {
        const InputWindowInfo* windowInfo = windowHandle->getInfo();
        if (windowInfo->displayId == displayId) {
            int32_t flags = windowInfo->layoutParamsFlags;

            if (windowInfo->visible) {
                if (!(flags & InputWindowInfo::FLAG_NOT_TOUCHABLE)) {
                    bool isTouchModal = (flags & (InputWindowInfo::FLAG_NOT_FOCUSABLE
                            | InputWindowInfo::FLAG_NOT_TOUCH_MODAL)) == 0;
                    if (isTouchModal || windowInfo->touchableRegionContainsPoint(x, y)) {
                        int32_t portalToDisplayId = windowInfo->portalToDisplayId;
                        if (portalToDisplayId != ADISPLAY_ID_NONE
                                && portalToDisplayId != displayId) {
                            if (addPortalWindows) {
                                // For the monitoring channels of the display.
                                mTempTouchState.addPortalWindow(windowHandle);
                            }
                            return findTouchedWindowAtLocked(
                                    portalToDisplayId, x, y, addOutsideTargets, addPortalWindows);
                        }
                        // Found window.
                        return windowHandle;
                    }
                }

                if (addOutsideTargets && (flags & InputWindowInfo::FLAG_WATCH_OUTSIDE_TOUCH)) {
                    mTempTouchState.addOrUpdateWindow(
                            windowHandle, InputTarget::FLAG_DISPATCH_AS_OUTSIDE, BitSet32(0));
                }
            }
        }
    }
    return nullptr;
}

std::vector<InputDispatcher::TouchedMonitor> InputDispatcher::findTouchedGestureMonitorsLocked(
        int32_t displayId, const std::vector<sp<InputWindowHandle>>& portalWindows) {
    std::vector<TouchedMonitor> touchedMonitors;

    std::vector<Monitor> monitors = getValueByKey(mGestureMonitorsByDisplay, displayId);
    addGestureMonitors(monitors, touchedMonitors);
    for (const sp<InputWindowHandle>& portalWindow : portalWindows) {
        const InputWindowInfo* windowInfo = portalWindow->getInfo();
        monitors = getValueByKey(mGestureMonitorsByDisplay, windowInfo->portalToDisplayId);
        addGestureMonitors(monitors, touchedMonitors,
                -windowInfo->frameLeft, -windowInfo->frameTop);
    }
    return touchedMonitors;
}

void InputDispatcher::addGestureMonitors(const std::vector<Monitor>& monitors,
        std::vector<TouchedMonitor>& outTouchedMonitors, float xOffset, float yOffset) {
    if (monitors.empty()) {
        return;
    }
    outTouchedMonitors.reserve(monitors.size() + outTouchedMonitors.size());
    for (const Monitor& monitor : monitors) {
        outTouchedMonitors.emplace_back(monitor, xOffset, yOffset);
    }
}

void InputDispatcher::dropInboundEventLocked(EventEntry* entry, DropReason dropReason) {
    const char* reason;
    switch (dropReason) {
    case DROP_REASON_POLICY:
#if DEBUG_INBOUND_EVENT_DETAILS
        ALOGD("Dropped event because policy consumed it.");
#endif
        reason = "inbound event was dropped because the policy consumed it";
        break;
    case DROP_REASON_DISABLED:
        if (mLastDropReason != DROP_REASON_DISABLED) {
            ALOGI("Dropped event because input dispatch is disabled.");
        }
        reason = "inbound event was dropped because input dispatch is disabled";
        break;
    case DROP_REASON_APP_SWITCH:
        ALOGI("Dropped event because of pending overdue app switch.");
        reason = "inbound event was dropped because of pending overdue app switch";
        break;
    case DROP_REASON_BLOCKED:
        ALOGI("Dropped event because the current application is not responding and the user "
                "has started interacting with a different application.");
        reason = "inbound event was dropped because the current application is not responding "
                "and the user has started interacting with a different application";
        break;
    case DROP_REASON_STALE:
        ALOGI("Dropped event because it is stale.");
        reason = "inbound event was dropped because it is stale";
        break;
    default:
        ALOG_ASSERT(false);
        return;
    }

    switch (entry->type) {
    case EventEntry::TYPE_KEY: {
        CancelationOptions options(CancelationOptions::CANCEL_NON_POINTER_EVENTS, reason);
        synthesizeCancelationEventsForAllConnectionsLocked(options);
        break;
    }
    case EventEntry::TYPE_MOTION: {
        MotionEntry* motionEntry = static_cast<MotionEntry*>(entry);
        if (motionEntry->source & AINPUT_SOURCE_CLASS_POINTER) {
            CancelationOptions options(CancelationOptions::CANCEL_POINTER_EVENTS, reason);
            synthesizeCancelationEventsForAllConnectionsLocked(options);
        } else {
            CancelationOptions options(CancelationOptions::CANCEL_NON_POINTER_EVENTS, reason);
            synthesizeCancelationEventsForAllConnectionsLocked(options);
        }
        break;
    }
    }
}

static bool isAppSwitchKeyCode(int32_t keyCode) {
    return keyCode == AKEYCODE_HOME
            || keyCode == AKEYCODE_ENDCALL
            || keyCode == AKEYCODE_APP_SWITCH;
}

bool InputDispatcher::isAppSwitchKeyEvent(KeyEntry* keyEntry) {
    return ! (keyEntry->flags & AKEY_EVENT_FLAG_CANCELED)
            && isAppSwitchKeyCode(keyEntry->keyCode)
            && (keyEntry->policyFlags & POLICY_FLAG_TRUSTED)
            && (keyEntry->policyFlags & POLICY_FLAG_PASS_TO_USER);
}

bool InputDispatcher::isAppSwitchPendingLocked() {
    return mAppSwitchDueTime != LONG_LONG_MAX;
}

void InputDispatcher::resetPendingAppSwitchLocked(bool handled) {
    mAppSwitchDueTime = LONG_LONG_MAX;

#if DEBUG_APP_SWITCH
    if (handled) {
        ALOGD("App switch has arrived.");
    } else {
        ALOGD("App switch was abandoned.");
    }
#endif
}

bool InputDispatcher::isStaleEvent(nsecs_t currentTime, EventEntry* entry) {
    return currentTime - entry->eventTime >= STALE_EVENT_TIMEOUT;
}

bool InputDispatcher::haveCommandsLocked() const {
    return !mCommandQueue.isEmpty();
}

bool InputDispatcher::runCommandsLockedInterruptible() {
    if (mCommandQueue.isEmpty()) {
        return false;
    }

    do {
        CommandEntry* commandEntry = mCommandQueue.dequeueAtHead();

        Command command = commandEntry->command;
        (this->*command)(commandEntry); // commands are implicitly 'LockedInterruptible'

        commandEntry->connection.clear();
        delete commandEntry;
    } while (! mCommandQueue.isEmpty());
    return true;
}

InputDispatcher::CommandEntry* InputDispatcher::postCommandLocked(Command command) {
    CommandEntry* commandEntry = new CommandEntry(command);
    mCommandQueue.enqueueAtTail(commandEntry);
    return commandEntry;
}

void InputDispatcher::drainInboundQueueLocked() {
    while (! mInboundQueue.isEmpty()) {
        EventEntry* entry = mInboundQueue.dequeueAtHead();
        releaseInboundEventLocked(entry);
    }
    traceInboundQueueLengthLocked();
}

void InputDispatcher::releasePendingEventLocked() {
    if (mPendingEvent) {
        resetANRTimeoutsLocked();
        releaseInboundEventLocked(mPendingEvent);
        mPendingEvent = nullptr;
    }
}

void InputDispatcher::releaseInboundEventLocked(EventEntry* entry) {
    InjectionState* injectionState = entry->injectionState;
    if (injectionState && injectionState->injectionResult == INPUT_EVENT_INJECTION_PENDING) {
#if DEBUG_DISPATCH_CYCLE
        ALOGD("Injected inbound event was dropped.");
#endif
        setInjectionResult(entry, INPUT_EVENT_INJECTION_FAILED);
    }
    if (entry == mNextUnblockedEvent) {
        mNextUnblockedEvent = nullptr;
    }
    addRecentEventLocked(entry);
    entry->release();
}

void InputDispatcher::resetKeyRepeatLocked() {
    if (mKeyRepeatState.lastKeyEntry) {
        mKeyRepeatState.lastKeyEntry->release();
        mKeyRepeatState.lastKeyEntry = nullptr;
    }
}

InputDispatcher::KeyEntry* InputDispatcher::synthesizeKeyRepeatLocked(nsecs_t currentTime) {
    KeyEntry* entry = mKeyRepeatState.lastKeyEntry;

    // Reuse the repeated key entry if it is otherwise unreferenced.
    uint32_t policyFlags = entry->policyFlags &
            (POLICY_FLAG_RAW_MASK | POLICY_FLAG_PASS_TO_USER | POLICY_FLAG_TRUSTED);
    if (entry->refCount == 1) {
        entry->recycle();
        entry->eventTime = currentTime;
        entry->policyFlags = policyFlags;
        entry->repeatCount += 1;
    } else {
        KeyEntry* newEntry = new KeyEntry(SYNTHESIZED_EVENT_SEQUENCE_NUM, currentTime,
                entry->deviceId, entry->source, entry->displayId, policyFlags,
                entry->action, entry->flags, entry->keyCode, entry->scanCode,
                entry->metaState, entry->repeatCount + 1, entry->downTime);

        mKeyRepeatState.lastKeyEntry = newEntry;
        entry->release();

        entry = newEntry;
    }
    entry->syntheticRepeat = true;

    // Increment reference count since we keep a reference to the event in
    // mKeyRepeatState.lastKeyEntry in addition to the one we return.
    entry->refCount += 1;

    mKeyRepeatState.nextRepeatTime = currentTime + mConfig.keyRepeatDelay;
    return entry;
}

bool InputDispatcher::dispatchConfigurationChangedLocked(
        nsecs_t currentTime, ConfigurationChangedEntry* entry) {
#if DEBUG_OUTBOUND_EVENT_DETAILS
    ALOGD("dispatchConfigurationChanged - eventTime=%" PRId64, entry->eventTime);
#endif

    // Reset key repeating in case a keyboard device was added or removed or something.
    resetKeyRepeatLocked();

    // Enqueue a command to run outside the lock to tell the policy that the configuration changed.
    CommandEntry* commandEntry = postCommandLocked(
            & InputDispatcher::doNotifyConfigurationChangedLockedInterruptible);
    commandEntry->eventTime = entry->eventTime;
    return true;
}

bool InputDispatcher::dispatchDeviceResetLocked(
        nsecs_t currentTime, DeviceResetEntry* entry) {
#if DEBUG_OUTBOUND_EVENT_DETAILS
    ALOGD("dispatchDeviceReset - eventTime=%" PRId64 ", deviceId=%d", entry->eventTime,
            entry->deviceId);
#endif

    CancelationOptions options(CancelationOptions::CANCEL_ALL_EVENTS,
            "device was reset");
    options.deviceId = entry->deviceId;
    synthesizeCancelationEventsForAllConnectionsLocked(options);
    return true;
}

bool InputDispatcher::dispatchKeyLocked(nsecs_t currentTime, KeyEntry* entry,
        DropReason* dropReason, nsecs_t* nextWakeupTime) {
    // Preprocessing.
    if (! entry->dispatchInProgress) {
        if (entry->repeatCount == 0
                && entry->action == AKEY_EVENT_ACTION_DOWN
                && (entry->policyFlags & POLICY_FLAG_TRUSTED)
                && (!(entry->policyFlags & POLICY_FLAG_DISABLE_KEY_REPEAT))) {
            if (mKeyRepeatState.lastKeyEntry
                    && mKeyRepeatState.lastKeyEntry->keyCode == entry->keyCode) {
                // We have seen two identical key downs in a row which indicates that the device
                // driver is automatically generating key repeats itself.  We take note of the
                // repeat here, but we disable our own next key repeat timer since it is clear that
                // we will not need to synthesize key repeats ourselves.
                entry->repeatCount = mKeyRepeatState.lastKeyEntry->repeatCount + 1;
                resetKeyRepeatLocked();
                mKeyRepeatState.nextRepeatTime = LONG_LONG_MAX; // don't generate repeats ourselves
            } else {
                // Not a repeat.  Save key down state in case we do see a repeat later.
                resetKeyRepeatLocked();
                mKeyRepeatState.nextRepeatTime = entry->eventTime + mConfig.keyRepeatTimeout;
            }
            mKeyRepeatState.lastKeyEntry = entry;
            entry->refCount += 1;
        } else if (! entry->syntheticRepeat) {
            resetKeyRepeatLocked();
        }

        if (entry->repeatCount == 1) {
            entry->flags |= AKEY_EVENT_FLAG_LONG_PRESS;
        } else {
            entry->flags &= ~AKEY_EVENT_FLAG_LONG_PRESS;
        }

        entry->dispatchInProgress = true;

        logOutboundKeyDetails("dispatchKey - ", entry);
    }

    // Handle case where the policy asked us to try again later last time.
    if (entry->interceptKeyResult == KeyEntry::INTERCEPT_KEY_RESULT_TRY_AGAIN_LATER) {
        if (currentTime < entry->interceptKeyWakeupTime) {
            if (entry->interceptKeyWakeupTime < *nextWakeupTime) {
                *nextWakeupTime = entry->interceptKeyWakeupTime;
            }
            return false; // wait until next wakeup
        }
        entry->interceptKeyResult = KeyEntry::INTERCEPT_KEY_RESULT_UNKNOWN;
        entry->interceptKeyWakeupTime = 0;
    }

    // Give the policy a chance to intercept the key.
    if (entry->interceptKeyResult == KeyEntry::INTERCEPT_KEY_RESULT_UNKNOWN) {
        if (entry->policyFlags & POLICY_FLAG_PASS_TO_USER) {
            CommandEntry* commandEntry = postCommandLocked(
                    & InputDispatcher::doInterceptKeyBeforeDispatchingLockedInterruptible);
            sp<InputWindowHandle> focusedWindowHandle =
                    getValueByKey(mFocusedWindowHandlesByDisplay, getTargetDisplayId(entry));
            if (focusedWindowHandle != nullptr) {
                commandEntry->inputChannel =
                    getInputChannelLocked(focusedWindowHandle->getToken());
            }
            commandEntry->keyEntry = entry;
            entry->refCount += 1;
            return false; // wait for the command to run
        } else {
            entry->interceptKeyResult = KeyEntry::INTERCEPT_KEY_RESULT_CONTINUE;
        }
    } else if (entry->interceptKeyResult == KeyEntry::INTERCEPT_KEY_RESULT_SKIP) {
        if (*dropReason == DROP_REASON_NOT_DROPPED) {
            *dropReason = DROP_REASON_POLICY;
        }
    }

    // Clean up if dropping the event.
    if (*dropReason != DROP_REASON_NOT_DROPPED) {
        setInjectionResult(entry, *dropReason == DROP_REASON_POLICY
                ? INPUT_EVENT_INJECTION_SUCCEEDED : INPUT_EVENT_INJECTION_FAILED);
        mReporter->reportDroppedKey(entry->sequenceNum);
        return true;
    }

    // Identify targets.
    std::vector<InputTarget> inputTargets;
    int32_t injectionResult = findFocusedWindowTargetsLocked(currentTime,
            entry, inputTargets, nextWakeupTime);
    if (injectionResult == INPUT_EVENT_INJECTION_PENDING) {
        return false;
    }

    setInjectionResult(entry, injectionResult);
    if (injectionResult != INPUT_EVENT_INJECTION_SUCCEEDED) {
        return true;
    }

    // Add monitor channels from event's or focused display.
    addGlobalMonitoringTargetsLocked(inputTargets, getTargetDisplayId(entry));

    // Dispatch the key.
    dispatchEventLocked(currentTime, entry, inputTargets);
    return true;
}

void InputDispatcher::logOutboundKeyDetails(const char* prefix, const KeyEntry* entry) {
#if DEBUG_OUTBOUND_EVENT_DETAILS
    ALOGD("%seventTime=%" PRId64 ", deviceId=%d, source=0x%x, displayId=%" PRId32 ", "
            "policyFlags=0x%x, action=0x%x, flags=0x%x, keyCode=0x%x, scanCode=0x%x, "
            "metaState=0x%x, repeatCount=%d, downTime=%" PRId64,
            prefix,
            entry->eventTime, entry->deviceId, entry->source, entry->displayId, entry->policyFlags,
            entry->action, entry->flags, entry->keyCode, entry->scanCode, entry->metaState,
            entry->repeatCount, entry->downTime);
#endif
}

bool InputDispatcher::dispatchMotionLocked(
        nsecs_t currentTime, MotionEntry* entry, DropReason* dropReason, nsecs_t* nextWakeupTime) {
    ATRACE_CALL();
    // Preprocessing.
    if (! entry->dispatchInProgress) {
        entry->dispatchInProgress = true;

        logOutboundMotionDetails("dispatchMotion - ", entry);
    }

    // Clean up if dropping the event.
    if (*dropReason != DROP_REASON_NOT_DROPPED) {
        setInjectionResult(entry, *dropReason == DROP_REASON_POLICY
                ? INPUT_EVENT_INJECTION_SUCCEEDED : INPUT_EVENT_INJECTION_FAILED);
        return true;
    }

    bool isPointerEvent = entry->source & AINPUT_SOURCE_CLASS_POINTER;

    // Identify targets.
    std::vector<InputTarget> inputTargets;

    bool conflictingPointerActions = false;
    int32_t injectionResult;
    if (isPointerEvent) {
        // Pointer event.  (eg. touchscreen)
        injectionResult = findTouchedWindowTargetsLocked(currentTime,
                entry, inputTargets, nextWakeupTime, &conflictingPointerActions);
    } else {
        // Non touch event.  (eg. trackball)
        injectionResult = findFocusedWindowTargetsLocked(currentTime,
                entry, inputTargets, nextWakeupTime);
    }
    if (injectionResult == INPUT_EVENT_INJECTION_PENDING) {
        return false;
    }

    setInjectionResult(entry, injectionResult);
    if (injectionResult != INPUT_EVENT_INJECTION_SUCCEEDED) {
        if (injectionResult != INPUT_EVENT_INJECTION_PERMISSION_DENIED) {
            CancelationOptions::Mode mode(isPointerEvent ?
                    CancelationOptions::CANCEL_POINTER_EVENTS :
                    CancelationOptions::CANCEL_NON_POINTER_EVENTS);
            CancelationOptions options(mode, "input event injection failed");
            synthesizeCancelationEventsForMonitorsLocked(options);
        }
        return true;
    }

    // Add monitor channels from event's or focused display.
    addGlobalMonitoringTargetsLocked(inputTargets, getTargetDisplayId(entry));

    if (isPointerEvent) {
        ssize_t stateIndex = mTouchStatesByDisplay.indexOfKey(entry->displayId);
        if (stateIndex >= 0) {
            const TouchState& state = mTouchStatesByDisplay.valueAt(stateIndex);
            if (!state.portalWindows.empty()) {
                // The event has gone through these portal windows, so we add monitoring targets of
                // the corresponding displays as well.
                for (size_t i = 0; i < state.portalWindows.size(); i++) {
                    const InputWindowInfo* windowInfo = state.portalWindows[i]->getInfo();
                    addGlobalMonitoringTargetsLocked(inputTargets, windowInfo->portalToDisplayId,
                            -windowInfo->frameLeft, -windowInfo->frameTop);
                }
            }
        }
    }

    // Dispatch the motion.
    if (conflictingPointerActions) {
        CancelationOptions options(CancelationOptions::CANCEL_POINTER_EVENTS,
                "conflicting pointer actions");
        synthesizeCancelationEventsForAllConnectionsLocked(options);
    }
    dispatchEventLocked(currentTime, entry, inputTargets);
    return true;
}


void InputDispatcher::logOutboundMotionDetails(const char* prefix, const MotionEntry* entry) {
#if DEBUG_OUTBOUND_EVENT_DETAILS
    ALOGD("%seventTime=%" PRId64 ", deviceId=%d, source=0x%x, displayId=%" PRId32
            ", policyFlags=0x%x, "
            "action=0x%x, actionButton=0x%x, flags=0x%x, "
            "metaState=0x%x, buttonState=0x%x,"
            "edgeFlags=0x%x, xPrecision=%f, yPrecision=%f, downTime=%" PRId64,
            prefix,
            entry->eventTime, entry->deviceId, entry->source, entry->displayId, entry->policyFlags,
            entry->action, entry->actionButton, entry->flags,
            entry->metaState, entry->buttonState,
            entry->edgeFlags, entry->xPrecision, entry->yPrecision,
            entry->downTime);

    for (uint32_t i = 0; i < entry->pointerCount; i++) {
        ALOGD("  Pointer %d: id=%d, toolType=%d, "
                "x=%f, y=%f, pressure=%f, size=%f, "
                "touchMajor=%f, touchMinor=%f, toolMajor=%f, toolMinor=%f, "
                "orientation=%f",
                i, entry->pointerProperties[i].id,
                entry->pointerProperties[i].toolType,
                entry->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_X),
                entry->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_Y),
                entry->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_PRESSURE),
                entry->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_SIZE),
                entry->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_TOUCH_MAJOR),
                entry->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_TOUCH_MINOR),
                entry->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_TOOL_MAJOR),
                entry->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_TOOL_MINOR),
                entry->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_ORIENTATION));
    }
#endif
}

void InputDispatcher::dispatchEventLocked(nsecs_t currentTime,
        EventEntry* eventEntry, const std::vector<InputTarget>& inputTargets) {
    ATRACE_CALL();
#if DEBUG_DISPATCH_CYCLE
    ALOGD("dispatchEventToCurrentInputTargets");
#endif

    ALOG_ASSERT(eventEntry->dispatchInProgress); // should already have been set to true

    pokeUserActivityLocked(eventEntry);

    for (const InputTarget& inputTarget : inputTargets) {
        ssize_t connectionIndex = getConnectionIndexLocked(inputTarget.inputChannel);
        if (connectionIndex >= 0) {
            sp<Connection> connection = mConnectionsByFd.valueAt(connectionIndex);
            prepareDispatchCycleLocked(currentTime, connection, eventEntry, &inputTarget);
        } else {
#if DEBUG_FOCUS
            ALOGD("Dropping event delivery to target with channel '%s' because it "
                    "is no longer registered with the input dispatcher.",
                    inputTarget.inputChannel->getName().c_str());
#endif
        }
    }
}

int32_t InputDispatcher::handleTargetsNotReadyLocked(nsecs_t currentTime,
        const EventEntry* entry,
        const sp<InputApplicationHandle>& applicationHandle,
        const sp<InputWindowHandle>& windowHandle,
        nsecs_t* nextWakeupTime, const char* reason) {
    if (applicationHandle == nullptr && windowHandle == nullptr) {
        if (mInputTargetWaitCause != INPUT_TARGET_WAIT_CAUSE_SYSTEM_NOT_READY) {
#if DEBUG_FOCUS
            ALOGD("Waiting for system to become ready for input.  Reason: %s", reason);
#endif
            mInputTargetWaitCause = INPUT_TARGET_WAIT_CAUSE_SYSTEM_NOT_READY;
            mInputTargetWaitStartTime = currentTime;
            mInputTargetWaitTimeoutTime = LONG_LONG_MAX;
            mInputTargetWaitTimeoutExpired = false;
            mInputTargetWaitApplicationToken.clear();
        }
    } else {
        if (mInputTargetWaitCause != INPUT_TARGET_WAIT_CAUSE_APPLICATION_NOT_READY) {
#if DEBUG_FOCUS
            ALOGD("Waiting for application to become ready for input: %s.  Reason: %s",
                    getApplicationWindowLabel(applicationHandle, windowHandle).c_str(),
                    reason);
#endif
            nsecs_t timeout;
            if (windowHandle != nullptr) {
                timeout = windowHandle->getDispatchingTimeout(DEFAULT_INPUT_DISPATCHING_TIMEOUT);
            } else if (applicationHandle != nullptr) {
                timeout = applicationHandle->getDispatchingTimeout(
                        DEFAULT_INPUT_DISPATCHING_TIMEOUT);
            } else {
                timeout = DEFAULT_INPUT_DISPATCHING_TIMEOUT;
            }

            mInputTargetWaitCause = INPUT_TARGET_WAIT_CAUSE_APPLICATION_NOT_READY;
            mInputTargetWaitStartTime = currentTime;
            mInputTargetWaitTimeoutTime = currentTime + timeout;
            mInputTargetWaitTimeoutExpired = false;
            mInputTargetWaitApplicationToken.clear();

            if (windowHandle != nullptr) {
                mInputTargetWaitApplicationToken = windowHandle->getApplicationToken();
            }
            if (mInputTargetWaitApplicationToken == nullptr && applicationHandle != nullptr) {
                mInputTargetWaitApplicationToken = applicationHandle->getApplicationToken();
            }
        }
    }

    if (mInputTargetWaitTimeoutExpired) {
        return INPUT_EVENT_INJECTION_TIMED_OUT;
    }

    if (currentTime >= mInputTargetWaitTimeoutTime) {
        onANRLocked(currentTime, applicationHandle, windowHandle,
                entry->eventTime, mInputTargetWaitStartTime, reason);

        // Force poll loop to wake up immediately on next iteration once we get the
        // ANR response back from the policy.
        *nextWakeupTime = LONG_LONG_MIN;
        return INPUT_EVENT_INJECTION_PENDING;
    } else {
        // Force poll loop to wake up when timeout is due.
        if (mInputTargetWaitTimeoutTime < *nextWakeupTime) {
            *nextWakeupTime = mInputTargetWaitTimeoutTime;
        }
        return INPUT_EVENT_INJECTION_PENDING;
    }
}

void InputDispatcher::removeWindowByTokenLocked(const sp<IBinder>& token) {
    for (size_t d = 0; d < mTouchStatesByDisplay.size(); d++) {
        TouchState& state = mTouchStatesByDisplay.editValueAt(d);
        state.removeWindowByToken(token);
    }
}

void InputDispatcher::resumeAfterTargetsNotReadyTimeoutLocked(nsecs_t newTimeout,
        const sp<InputChannel>& inputChannel) {
    if (newTimeout > 0) {
        // Extend the timeout.
        mInputTargetWaitTimeoutTime = now() + newTimeout;
    } else {
        // Give up.
        mInputTargetWaitTimeoutExpired = true;

        // Input state will not be realistic.  Mark it out of sync.
        if (inputChannel.get()) {
            ssize_t connectionIndex = getConnectionIndexLocked(inputChannel);
            if (connectionIndex >= 0) {
                sp<Connection> connection = mConnectionsByFd.valueAt(connectionIndex);
                sp<IBinder> token = connection->inputChannel->getToken();

                if (token != nullptr) {
                    removeWindowByTokenLocked(token);
                }

                if (connection->status == Connection::STATUS_NORMAL) {
                    CancelationOptions options(CancelationOptions::CANCEL_ALL_EVENTS,
                            "application not responding");
                    synthesizeCancelationEventsForConnectionLocked(connection, options);
                }
            }
        }
    }
}

nsecs_t InputDispatcher::getTimeSpentWaitingForApplicationLocked(
        nsecs_t currentTime) {
    if (mInputTargetWaitCause == INPUT_TARGET_WAIT_CAUSE_APPLICATION_NOT_READY) {
        return currentTime - mInputTargetWaitStartTime;
    }
    return 0;
}

void InputDispatcher::resetANRTimeoutsLocked() {
#if DEBUG_FOCUS
        ALOGD("Resetting ANR timeouts.");
#endif

    // Reset input target wait timeout.
    mInputTargetWaitCause = INPUT_TARGET_WAIT_CAUSE_NONE;
    mInputTargetWaitApplicationToken.clear();
}

/**
 * Get the display id that the given event should go to. If this event specifies a valid display id,
 * then it should be dispatched to that display. Otherwise, the event goes to the focused display.
 * Focused display is the display that the user most recently interacted with.
 */
int32_t InputDispatcher::getTargetDisplayId(const EventEntry* entry) {
    int32_t displayId;
    switch (entry->type) {
    case EventEntry::TYPE_KEY: {
        const KeyEntry* typedEntry = static_cast<const KeyEntry*>(entry);
        displayId = typedEntry->displayId;
        break;
    }
    case EventEntry::TYPE_MOTION: {
        const MotionEntry* typedEntry = static_cast<const MotionEntry*>(entry);
        displayId = typedEntry->displayId;
        break;
    }
    default: {
        ALOGE("Unsupported event type '%" PRId32 "' for target display.", entry->type);
        return ADISPLAY_ID_NONE;
    }
    }
    return displayId == ADISPLAY_ID_NONE ? mFocusedDisplayId : displayId;
}

int32_t InputDispatcher::findFocusedWindowTargetsLocked(nsecs_t currentTime,
        const EventEntry* entry, std::vector<InputTarget>& inputTargets, nsecs_t* nextWakeupTime) {
    int32_t injectionResult;
    std::string reason;

    int32_t displayId = getTargetDisplayId(entry);
    sp<InputWindowHandle> focusedWindowHandle =
            getValueByKey(mFocusedWindowHandlesByDisplay, displayId);
    sp<InputApplicationHandle> focusedApplicationHandle =
            getValueByKey(mFocusedApplicationHandlesByDisplay, displayId);

    // If there is no currently focused window and no focused application
    // then drop the event.
    if (focusedWindowHandle == nullptr) {
        if (focusedApplicationHandle != nullptr) {
            injectionResult = handleTargetsNotReadyLocked(currentTime, entry,
                    focusedApplicationHandle, nullptr, nextWakeupTime,
                    "Waiting because no window has focus but there is a "
                    "focused application that may eventually add a window "
                    "when it finishes starting up.");
            goto Unresponsive;
        }

        ALOGI("Dropping event because there is no focused window or focused application in display "
                "%" PRId32 ".", displayId);
        injectionResult = INPUT_EVENT_INJECTION_FAILED;
        goto Failed;
    }

    // Check permissions.
    if (!checkInjectionPermission(focusedWindowHandle, entry->injectionState)) {
        injectionResult = INPUT_EVENT_INJECTION_PERMISSION_DENIED;
        goto Failed;
    }

    // Check whether the window is ready for more input.
    reason = checkWindowReadyForMoreInputLocked(currentTime,
            focusedWindowHandle, entry, "focused");
    if (!reason.empty()) {
        injectionResult = handleTargetsNotReadyLocked(currentTime, entry,
                focusedApplicationHandle, focusedWindowHandle, nextWakeupTime, reason.c_str());
        goto Unresponsive;
    }

    // Success!  Output targets.
    injectionResult = INPUT_EVENT_INJECTION_SUCCEEDED;
    addWindowTargetLocked(focusedWindowHandle,
            InputTarget::FLAG_FOREGROUND | InputTarget::FLAG_DISPATCH_AS_IS, BitSet32(0),
            inputTargets);

    // Done.
Failed:
Unresponsive:
    nsecs_t timeSpentWaitingForApplication = getTimeSpentWaitingForApplicationLocked(currentTime);
    updateDispatchStatistics(currentTime, entry, injectionResult, timeSpentWaitingForApplication);
#if DEBUG_FOCUS
    ALOGD("findFocusedWindow finished: injectionResult=%d, "
            "timeSpentWaitingForApplication=%0.1fms",
            injectionResult, timeSpentWaitingForApplication / 1000000.0);
#endif
    return injectionResult;
}

int32_t InputDispatcher::findTouchedWindowTargetsLocked(nsecs_t currentTime,
        const MotionEntry* entry, std::vector<InputTarget>& inputTargets, nsecs_t* nextWakeupTime,
        bool* outConflictingPointerActions) {
    ATRACE_CALL();
    enum InjectionPermission {
        INJECTION_PERMISSION_UNKNOWN,
        INJECTION_PERMISSION_GRANTED,
        INJECTION_PERMISSION_DENIED
    };

    // For security reasons, we defer updating the touch state until we are sure that
    // event injection will be allowed.
    int32_t displayId = entry->displayId;
    int32_t action = entry->action;
    int32_t maskedAction = action & AMOTION_EVENT_ACTION_MASK;

    // Update the touch state as needed based on the properties of the touch event.
    int32_t injectionResult = INPUT_EVENT_INJECTION_PENDING;
    InjectionPermission injectionPermission = INJECTION_PERMISSION_UNKNOWN;
    sp<InputWindowHandle> newHoverWindowHandle;

    // Copy current touch state into mTempTouchState.
    // This state is always reset at the end of this function, so if we don't find state
    // for the specified display then our initial state will be empty.
    const TouchState* oldState = nullptr;
    ssize_t oldStateIndex = mTouchStatesByDisplay.indexOfKey(displayId);
    if (oldStateIndex >= 0) {
        oldState = &mTouchStatesByDisplay.valueAt(oldStateIndex);
        mTempTouchState.copyFrom(*oldState);
    }

    bool isSplit = mTempTouchState.split;
    bool switchedDevice = mTempTouchState.deviceId >= 0 && mTempTouchState.displayId >= 0
            && (mTempTouchState.deviceId != entry->deviceId
                    || mTempTouchState.source != entry->source
                    || mTempTouchState.displayId != displayId);
    bool isHoverAction = (maskedAction == AMOTION_EVENT_ACTION_HOVER_MOVE
            || maskedAction == AMOTION_EVENT_ACTION_HOVER_ENTER
            || maskedAction == AMOTION_EVENT_ACTION_HOVER_EXIT);
    bool newGesture = (maskedAction == AMOTION_EVENT_ACTION_DOWN
            || maskedAction == AMOTION_EVENT_ACTION_SCROLL
            || isHoverAction);
    bool wrongDevice = false;
    if (newGesture) {
        bool down = maskedAction == AMOTION_EVENT_ACTION_DOWN;
        if (switchedDevice && mTempTouchState.down && !down && !isHoverAction) {
#if DEBUG_FOCUS
            ALOGD("Dropping event because a pointer for a different device is already down "
                    "in display %" PRId32, displayId);
#endif
            // TODO: test multiple simultaneous input streams.
            injectionResult = INPUT_EVENT_INJECTION_FAILED;
            switchedDevice = false;
            wrongDevice = true;
            goto Failed;
        }
        mTempTouchState.reset();
        mTempTouchState.down = down;
        mTempTouchState.deviceId = entry->deviceId;
        mTempTouchState.source = entry->source;
        mTempTouchState.displayId = displayId;
        isSplit = false;
    } else if (switchedDevice && maskedAction == AMOTION_EVENT_ACTION_MOVE) {
#if DEBUG_FOCUS
        ALOGI("Dropping move event because a pointer for a different device is already active "
                "in display %" PRId32, displayId);
#endif
        // TODO: test multiple simultaneous input streams.
        injectionResult = INPUT_EVENT_INJECTION_PERMISSION_DENIED;
        switchedDevice = false;
        wrongDevice = true;
        goto Failed;
    }

    if (newGesture || (isSplit && maskedAction == AMOTION_EVENT_ACTION_POINTER_DOWN)) {
        /* Case 1: New splittable pointer going down, or need target for hover or scroll. */

        int32_t pointerIndex = getMotionEventActionPointerIndex(action);
        int32_t x = int32_t(entry->pointerCoords[pointerIndex].
                getAxisValue(AMOTION_EVENT_AXIS_X));
        int32_t y = int32_t(entry->pointerCoords[pointerIndex].
                getAxisValue(AMOTION_EVENT_AXIS_Y));
        bool isDown = maskedAction == AMOTION_EVENT_ACTION_DOWN;
        sp<InputWindowHandle> newTouchedWindowHandle = findTouchedWindowAtLocked(
                displayId, x, y, isDown /*addOutsideTargets*/, true /*addPortalWindows*/);

        std::vector<TouchedMonitor> newGestureMonitors = isDown
                ? findTouchedGestureMonitorsLocked(displayId, mTempTouchState.portalWindows)
                : std::vector<TouchedMonitor>{};

        // Figure out whether splitting will be allowed for this window.
        if (newTouchedWindowHandle != nullptr
                && newTouchedWindowHandle->getInfo()->supportsSplitTouch()) {
            // New window supports splitting.
            isSplit = true;
        } else if (isSplit) {
            // New window does not support splitting but we have already split events.
            // Ignore the new window.
            newTouchedWindowHandle = nullptr;
        }

        // Handle the case where we did not find a window.
        if (newTouchedWindowHandle == nullptr) {
            // Try to assign the pointer to the first foreground window we find, if there is one.
            newTouchedWindowHandle = mTempTouchState.getFirstForegroundWindowHandle();
        }

        if (newTouchedWindowHandle == nullptr && newGestureMonitors.empty()) {
            ALOGI("Dropping event because there is no touchable window or gesture monitor at "
                    "(%d, %d) in display %" PRId32 ".", x, y, displayId);
            injectionResult = INPUT_EVENT_INJECTION_FAILED;
            goto Failed;
        }

        if (newTouchedWindowHandle != nullptr) {
            // Set target flags.
            int32_t targetFlags = InputTarget::FLAG_FOREGROUND | InputTarget::FLAG_DISPATCH_AS_IS;
            if (isSplit) {
                targetFlags |= InputTarget::FLAG_SPLIT;
            }
            if (isWindowObscuredAtPointLocked(newTouchedWindowHandle, x, y)) {
                targetFlags |= InputTarget::FLAG_WINDOW_IS_OBSCURED;
            } else if (isWindowObscuredLocked(newTouchedWindowHandle)) {
                targetFlags |= InputTarget::FLAG_WINDOW_IS_PARTIALLY_OBSCURED;
            }

            // Update hover state.
            if (isHoverAction) {
                newHoverWindowHandle = newTouchedWindowHandle;
            } else if (maskedAction == AMOTION_EVENT_ACTION_SCROLL) {
                newHoverWindowHandle = mLastHoverWindowHandle;
            }

            // Update the temporary touch state.
            BitSet32 pointerIds;
            if (isSplit) {
                uint32_t pointerId = entry->pointerProperties[pointerIndex].id;
                pointerIds.markBit(pointerId);
            }
            mTempTouchState.addOrUpdateWindow(newTouchedWindowHandle, targetFlags, pointerIds);
        }

        mTempTouchState.addGestureMonitors(newGestureMonitors);
    } else {
        /* Case 2: Pointer move, up, cancel or non-splittable pointer down. */

        // If the pointer is not currently down, then ignore the event.
        if (! mTempTouchState.down) {
#if DEBUG_FOCUS
            ALOGD("Dropping event because the pointer is not down or we previously "
                    "dropped the pointer down event in display %" PRId32, displayId);
#endif
            injectionResult = INPUT_EVENT_INJECTION_FAILED;
            goto Failed;
        }

        // Check whether touches should slip outside of the current foreground window.
        if (maskedAction == AMOTION_EVENT_ACTION_MOVE
                && entry->pointerCount == 1
                && mTempTouchState.isSlippery()) {
            int32_t x = int32_t(entry->pointerCoords[0].getAxisValue(AMOTION_EVENT_AXIS_X));
            int32_t y = int32_t(entry->pointerCoords[0].getAxisValue(AMOTION_EVENT_AXIS_Y));

            sp<InputWindowHandle> oldTouchedWindowHandle =
                    mTempTouchState.getFirstForegroundWindowHandle();
            sp<InputWindowHandle> newTouchedWindowHandle =
                    findTouchedWindowAtLocked(displayId, x, y);
            if (oldTouchedWindowHandle != newTouchedWindowHandle
                    && oldTouchedWindowHandle != nullptr
                    && newTouchedWindowHandle != nullptr) {
#if DEBUG_FOCUS
                ALOGD("Touch is slipping out of window %s into window %s in display %" PRId32,
                        oldTouchedWindowHandle->getName().c_str(),
                        newTouchedWindowHandle->getName().c_str(),
                        displayId);
#endif
                // Make a slippery exit from the old window.
                mTempTouchState.addOrUpdateWindow(oldTouchedWindowHandle,
                        InputTarget::FLAG_DISPATCH_AS_SLIPPERY_EXIT, BitSet32(0));

                // Make a slippery entrance into the new window.
                if (newTouchedWindowHandle->getInfo()->supportsSplitTouch()) {
                    isSplit = true;
                }

                int32_t targetFlags = InputTarget::FLAG_FOREGROUND
                        | InputTarget::FLAG_DISPATCH_AS_SLIPPERY_ENTER;
                if (isSplit) {
                    targetFlags |= InputTarget::FLAG_SPLIT;
                }
                if (isWindowObscuredAtPointLocked(newTouchedWindowHandle, x, y)) {
                    targetFlags |= InputTarget::FLAG_WINDOW_IS_OBSCURED;
                }

                BitSet32 pointerIds;
                if (isSplit) {
                    pointerIds.markBit(entry->pointerProperties[0].id);
                }
                mTempTouchState.addOrUpdateWindow(newTouchedWindowHandle, targetFlags, pointerIds);
            }
        }
    }

    if (newHoverWindowHandle != mLastHoverWindowHandle) {
        // Let the previous window know that the hover sequence is over.
        if (mLastHoverWindowHandle != nullptr) {
#if DEBUG_HOVER
            ALOGD("Sending hover exit event to window %s.",
                    mLastHoverWindowHandle->getName().c_str());
#endif
            mTempTouchState.addOrUpdateWindow(mLastHoverWindowHandle,
                    InputTarget::FLAG_DISPATCH_AS_HOVER_EXIT, BitSet32(0));
        }

        // Let the new window know that the hover sequence is starting.
        if (newHoverWindowHandle != nullptr) {
#if DEBUG_HOVER
            ALOGD("Sending hover enter event to window %s.",
                    newHoverWindowHandle->getName().c_str());
#endif
            mTempTouchState.addOrUpdateWindow(newHoverWindowHandle,
                    InputTarget::FLAG_DISPATCH_AS_HOVER_ENTER, BitSet32(0));
        }
    }

    // Check permission to inject into all touched foreground windows and ensure there
    // is at least one touched foreground window.
    {
        bool haveForegroundWindow = false;
        for (const TouchedWindow& touchedWindow : mTempTouchState.windows) {
            if (touchedWindow.targetFlags & InputTarget::FLAG_FOREGROUND) {
                haveForegroundWindow = true;
                if (! checkInjectionPermission(touchedWindow.windowHandle,
                        entry->injectionState)) {
                    injectionResult = INPUT_EVENT_INJECTION_PERMISSION_DENIED;
                    injectionPermission = INJECTION_PERMISSION_DENIED;
                    goto Failed;
                }
            }
        }
        bool hasGestureMonitor = !mTempTouchState.gestureMonitors.empty();
        if (!haveForegroundWindow && !hasGestureMonitor) {
#if DEBUG_FOCUS
            ALOGD("Dropping event because there is no touched foreground window in display %"
                    PRId32 " or gesture monitor to receive it.", displayId);
#endif
            injectionResult = INPUT_EVENT_INJECTION_FAILED;
            goto Failed;
        }

        // Permission granted to injection into all touched foreground windows.
        injectionPermission = INJECTION_PERMISSION_GRANTED;
    }

    // Check whether windows listening for outside touches are owned by the same UID. If it is
    // set the policy flag that we will not reveal coordinate information to this window.
    if (maskedAction == AMOTION_EVENT_ACTION_DOWN) {
        sp<InputWindowHandle> foregroundWindowHandle =
                mTempTouchState.getFirstForegroundWindowHandle();
        if (foregroundWindowHandle) {
            const int32_t foregroundWindowUid = foregroundWindowHandle->getInfo()->ownerUid;
            for (const TouchedWindow& touchedWindow : mTempTouchState.windows) {
                if (touchedWindow.targetFlags & InputTarget::FLAG_DISPATCH_AS_OUTSIDE) {
                    sp<InputWindowHandle> inputWindowHandle = touchedWindow.windowHandle;
                    if (inputWindowHandle->getInfo()->ownerUid != foregroundWindowUid) {
                        mTempTouchState.addOrUpdateWindow(inputWindowHandle,
                                InputTarget::FLAG_ZERO_COORDS, BitSet32(0));
                    }
                }
            }
        }
    }

    // Ensure all touched foreground windows are ready for new input.
    for (const TouchedWindow& touchedWindow : mTempTouchState.windows) {
        if (touchedWindow.targetFlags & InputTarget::FLAG_FOREGROUND) {
            // Check whether the window is ready for more input.
            std::string reason = checkWindowReadyForMoreInputLocked(currentTime,
                    touchedWindow.windowHandle, entry, "touched");
            if (!reason.empty()) {
                injectionResult = handleTargetsNotReadyLocked(currentTime, entry,
                        nullptr, touchedWindow.windowHandle, nextWakeupTime, reason.c_str());
                goto Unresponsive;
            }
        }
    }

    // If this is the first pointer going down and the touched window has a wallpaper
    // then also add the touched wallpaper windows so they are locked in for the duration
    // of the touch gesture.
    // We do not collect wallpapers during HOVER_MOVE or SCROLL because the wallpaper
    // engine only supports touch events.  We would need to add a mechanism similar
    // to View.onGenericMotionEvent to enable wallpapers to handle these events.
    if (maskedAction == AMOTION_EVENT_ACTION_DOWN) {
        sp<InputWindowHandle> foregroundWindowHandle =
                mTempTouchState.getFirstForegroundWindowHandle();
        if (foregroundWindowHandle && foregroundWindowHandle->getInfo()->hasWallpaper) {
            const std::vector<sp<InputWindowHandle>> windowHandles =
                    getWindowHandlesLocked(displayId);
            for (const sp<InputWindowHandle>& windowHandle : windowHandles) {
                const InputWindowInfo* info = windowHandle->getInfo();
                if (info->displayId == displayId
                        && windowHandle->getInfo()->layoutParamsType
                                == InputWindowInfo::TYPE_WALLPAPER) {
                    mTempTouchState.addOrUpdateWindow(windowHandle,
                            InputTarget::FLAG_WINDOW_IS_OBSCURED
                                    | InputTarget::FLAG_WINDOW_IS_PARTIALLY_OBSCURED
                                    | InputTarget::FLAG_DISPATCH_AS_IS,
                            BitSet32(0));
                }
            }
        }
    }

    // Success!  Output targets.
    injectionResult = INPUT_EVENT_INJECTION_SUCCEEDED;

    for (const TouchedWindow& touchedWindow : mTempTouchState.windows) {
        addWindowTargetLocked(touchedWindow.windowHandle, touchedWindow.targetFlags,
                touchedWindow.pointerIds, inputTargets);
    }

    for (const TouchedMonitor& touchedMonitor : mTempTouchState.gestureMonitors) {
        addMonitoringTargetLocked(touchedMonitor.monitor, touchedMonitor.xOffset,
                touchedMonitor.yOffset, inputTargets);
    }

    // Drop the outside or hover touch windows since we will not care about them
    // in the next iteration.
    mTempTouchState.filterNonAsIsTouchWindows();

Failed:
    // Check injection permission once and for all.
    if (injectionPermission == INJECTION_PERMISSION_UNKNOWN) {
        if (checkInjectionPermission(nullptr, entry->injectionState)) {
            injectionPermission = INJECTION_PERMISSION_GRANTED;
        } else {
            injectionPermission = INJECTION_PERMISSION_DENIED;
        }
    }

    // Update final pieces of touch state if the injector had permission.
    if (injectionPermission == INJECTION_PERMISSION_GRANTED) {
        if (!wrongDevice) {
            if (switchedDevice) {
#if DEBUG_FOCUS
                ALOGD("Conflicting pointer actions: Switched to a different device.");
#endif
                *outConflictingPointerActions = true;
            }

            if (isHoverAction) {
                // Started hovering, therefore no longer down.
                if (oldState && oldState->down) {
#if DEBUG_FOCUS
                    ALOGD("Conflicting pointer actions: Hover received while pointer was down.");
#endif
                    *outConflictingPointerActions = true;
                }
                mTempTouchState.reset();
                if (maskedAction == AMOTION_EVENT_ACTION_HOVER_ENTER
                        || maskedAction == AMOTION_EVENT_ACTION_HOVER_MOVE) {
                    mTempTouchState.deviceId = entry->deviceId;
                    mTempTouchState.source = entry->source;
                    mTempTouchState.displayId = displayId;
                }
            } else if (maskedAction == AMOTION_EVENT_ACTION_UP
                    || maskedAction == AMOTION_EVENT_ACTION_CANCEL) {
                // All pointers up or canceled.
                mTempTouchState.reset();
            } else if (maskedAction == AMOTION_EVENT_ACTION_DOWN) {
                // First pointer went down.
                if (oldState && oldState->down) {
#if DEBUG_FOCUS
                    ALOGD("Conflicting pointer actions: Down received while already down.");
#endif
                    *outConflictingPointerActions = true;
                }
            } else if (maskedAction == AMOTION_EVENT_ACTION_POINTER_UP) {
                // One pointer went up.
                if (isSplit) {
                    int32_t pointerIndex = getMotionEventActionPointerIndex(action);
                    uint32_t pointerId = entry->pointerProperties[pointerIndex].id;

                    for (size_t i = 0; i < mTempTouchState.windows.size(); ) {
                        TouchedWindow& touchedWindow = mTempTouchState.windows[i];
                        if (touchedWindow.targetFlags & InputTarget::FLAG_SPLIT) {
                            touchedWindow.pointerIds.clearBit(pointerId);
                            if (touchedWindow.pointerIds.isEmpty()) {
                                mTempTouchState.windows.erase(mTempTouchState.windows.begin() + i);
                                continue;
                            }
                        }
                        i += 1;
                    }
                }
            }

            // Save changes unless the action was scroll in which case the temporary touch
            // state was only valid for this one action.
            if (maskedAction != AMOTION_EVENT_ACTION_SCROLL) {
                if (mTempTouchState.displayId >= 0) {
                    if (oldStateIndex >= 0) {
                        mTouchStatesByDisplay.editValueAt(oldStateIndex).copyFrom(mTempTouchState);
                    } else {
                        mTouchStatesByDisplay.add(displayId, mTempTouchState);
                    }
                } else if (oldStateIndex >= 0) {
                    mTouchStatesByDisplay.removeItemsAt(oldStateIndex);
                }
            }

            // Update hover state.
            mLastHoverWindowHandle = newHoverWindowHandle;
        }
    } else {
#if DEBUG_FOCUS
        ALOGD("Not updating touch focus because injection was denied.");
#endif
    }

Unresponsive:
    // Reset temporary touch state to ensure we release unnecessary references to input channels.
    mTempTouchState.reset();

    nsecs_t timeSpentWaitingForApplication = getTimeSpentWaitingForApplicationLocked(currentTime);
    updateDispatchStatistics(currentTime, entry, injectionResult, timeSpentWaitingForApplication);
#if DEBUG_FOCUS
    ALOGD("findTouchedWindow finished: injectionResult=%d, injectionPermission=%d, "
            "timeSpentWaitingForApplication=%0.1fms",
            injectionResult, injectionPermission, timeSpentWaitingForApplication / 1000000.0);
#endif
    return injectionResult;
}

void InputDispatcher::addWindowTargetLocked(const sp<InputWindowHandle>& windowHandle,
        int32_t targetFlags, BitSet32 pointerIds, std::vector<InputTarget>& inputTargets) {
    sp<InputChannel> inputChannel = getInputChannelLocked(windowHandle->getToken());
    if (inputChannel == nullptr) {
        ALOGW("Window %s already unregistered input channel", windowHandle->getName().c_str());
        return;
    }

    const InputWindowInfo* windowInfo = windowHandle->getInfo();
    InputTarget target;
    target.inputChannel = inputChannel;
    target.flags = targetFlags;
    target.xOffset = - windowInfo->frameLeft;
    target.yOffset = - windowInfo->frameTop;
    target.globalScaleFactor = windowInfo->globalScaleFactor;
    target.windowXScale = windowInfo->windowXScale;
    target.windowYScale = windowInfo->windowYScale;
    target.pointerIds = pointerIds;
    inputTargets.push_back(target);
}

void InputDispatcher::addGlobalMonitoringTargetsLocked(std::vector<InputTarget>& inputTargets,
         int32_t displayId, float xOffset, float yOffset) {

    std::unordered_map<int32_t, std::vector<Monitor>>::const_iterator it =
            mGlobalMonitorsByDisplay.find(displayId);

    if (it != mGlobalMonitorsByDisplay.end()) {
        const std::vector<Monitor>& monitors = it->second;
        for (const Monitor& monitor : monitors) {
            addMonitoringTargetLocked(monitor, xOffset, yOffset, inputTargets);
        }
    }
}

void InputDispatcher::addMonitoringTargetLocked(const Monitor& monitor,
        float xOffset, float yOffset, std::vector<InputTarget>& inputTargets) {
    InputTarget target;
    target.inputChannel = monitor.inputChannel;
    target.flags = InputTarget::FLAG_DISPATCH_AS_IS;
    target.xOffset = xOffset;
    target.yOffset = yOffset;
    target.pointerIds.clear();
    target.globalScaleFactor = 1.0f;
    inputTargets.push_back(target);
}

bool InputDispatcher::checkInjectionPermission(const sp<InputWindowHandle>& windowHandle,
        const InjectionState* injectionState) {
    if (injectionState
            && (windowHandle == nullptr
                    || windowHandle->getInfo()->ownerUid != injectionState->injectorUid)
            && !hasInjectionPermission(injectionState->injectorPid, injectionState->injectorUid)) {
        if (windowHandle != nullptr) {
            ALOGW("Permission denied: injecting event from pid %d uid %d to window %s "
                    "owned by uid %d",
                    injectionState->injectorPid, injectionState->injectorUid,
                    windowHandle->getName().c_str(),
                    windowHandle->getInfo()->ownerUid);
        } else {
            ALOGW("Permission denied: injecting event from pid %d uid %d",
                    injectionState->injectorPid, injectionState->injectorUid);
        }
        return false;
    }
    return true;
}

bool InputDispatcher::isWindowObscuredAtPointLocked(
        const sp<InputWindowHandle>& windowHandle, int32_t x, int32_t y) const {
    int32_t displayId = windowHandle->getInfo()->displayId;
    const std::vector<sp<InputWindowHandle>> windowHandles = getWindowHandlesLocked(displayId);
    for (const sp<InputWindowHandle>& otherHandle : windowHandles) {
        if (otherHandle == windowHandle) {
            break;
        }

        const InputWindowInfo* otherInfo = otherHandle->getInfo();
        if (otherInfo->displayId == displayId
                && otherInfo->visible && !otherInfo->isTrustedOverlay()
                && otherInfo->frameContainsPoint(x, y)) {
            return true;
        }
    }
    return false;
}


bool InputDispatcher::isWindowObscuredLocked(const sp<InputWindowHandle>& windowHandle) const {
    int32_t displayId = windowHandle->getInfo()->displayId;
    const std::vector<sp<InputWindowHandle>> windowHandles = getWindowHandlesLocked(displayId);
    const InputWindowInfo* windowInfo = windowHandle->getInfo();
    for (const sp<InputWindowHandle>& otherHandle : windowHandles) {
        if (otherHandle == windowHandle) {
            break;
        }

        const InputWindowInfo* otherInfo = otherHandle->getInfo();
        if (otherInfo->displayId == displayId
                && otherInfo->visible && !otherInfo->isTrustedOverlay()
                && otherInfo->overlaps(windowInfo)) {
            return true;
        }
    }
    return false;
}

std::string InputDispatcher::checkWindowReadyForMoreInputLocked(nsecs_t currentTime,
        const sp<InputWindowHandle>& windowHandle, const EventEntry* eventEntry,
        const char* targetType) {
    // If the window is paused then keep waiting.
    if (windowHandle->getInfo()->paused) {
        return StringPrintf("Waiting because the %s window is paused.", targetType);
    }

    // If the window's connection is not registered then keep waiting.
    ssize_t connectionIndex = getConnectionIndexLocked(
            getInputChannelLocked(windowHandle->getToken()));
    if (connectionIndex < 0) {
        return StringPrintf("Waiting because the %s window's input channel is not "
                "registered with the input dispatcher.  The window may be in the process "
                "of being removed.", targetType);
    }

    // If the connection is dead then keep waiting.
    sp<Connection> connection = mConnectionsByFd.valueAt(connectionIndex);
    if (connection->status != Connection::STATUS_NORMAL) {
        return StringPrintf("Waiting because the %s window's input connection is %s."
                "The window may be in the process of being removed.", targetType,
                connection->getStatusLabel());
    }

    // If the connection is backed up then keep waiting.
    if (connection->inputPublisherBlocked) {
        return StringPrintf("Waiting because the %s window's input channel is full.  "
                "Outbound queue length: %d.  Wait queue length: %d.",
                targetType, connection->outboundQueue.count(), connection->waitQueue.count());
    }

    // Ensure that the dispatch queues aren't too far backed up for this event.
    if (eventEntry->type == EventEntry::TYPE_KEY) {
        // If the event is a key event, then we must wait for all previous events to
        // complete before delivering it because previous events may have the
        // side-effect of transferring focus to a different window and we want to
        // ensure that the following keys are sent to the new window.
        //
        // Suppose the user touches a button in a window then immediately presses "A".
        // If the button causes a pop-up window to appear then we want to ensure that
        // the "A" key is delivered to the new pop-up window.  This is because users
        // often anticipate pending UI changes when typing on a keyboard.
        // To obtain this behavior, we must serialize key events with respect to all
        // prior input events.
        if (!connection->outboundQueue.isEmpty() || !connection->waitQueue.isEmpty()) {
            return StringPrintf("Waiting to send key event because the %s window has not "
                    "finished processing all of the input events that were previously "
                    "delivered to it.  Outbound queue length: %d.  Wait queue length: %d.",
                    targetType, connection->outboundQueue.count(), connection->waitQueue.count());
        }
    } else {
        // Touch events can always be sent to a window immediately because the user intended
        // to touch whatever was visible at the time.  Even if focus changes or a new
        // window appears moments later, the touch event was meant to be delivered to
        // whatever window happened to be on screen at the time.
        //
        // Generic motion events, such as trackball or joystick events are a little trickier.
        // Like key events, generic motion events are delivered to the focused window.
        // Unlike key events, generic motion events don't tend to transfer focus to other
        // windows and it is not important for them to be serialized.  So we prefer to deliver
        // generic motion events as soon as possible to improve efficiency and reduce lag
        // through batching.
        //
        // The one case where we pause input event delivery is when the wait queue is piling
        // up with lots of events because the application is not responding.
        // This condition ensures that ANRs are detected reliably.
        if (!connection->waitQueue.isEmpty()
                && currentTime >= connection->waitQueue.head->deliveryTime
                        + STREAM_AHEAD_EVENT_TIMEOUT) {
            return StringPrintf("Waiting to send non-key event because the %s window has not "
                    "finished processing certain input events that were delivered to it over "
                    "%0.1fms ago.  Wait queue length: %d.  Wait queue head age: %0.1fms.",
                    targetType, STREAM_AHEAD_EVENT_TIMEOUT * 0.000001f,
                    connection->waitQueue.count(),
                    (currentTime - connection->waitQueue.head->deliveryTime) * 0.000001f);
        }
    }
    return "";
}

std::string InputDispatcher::getApplicationWindowLabel(
        const sp<InputApplicationHandle>& applicationHandle,
        const sp<InputWindowHandle>& windowHandle) {
    if (applicationHandle != nullptr) {
        if (windowHandle != nullptr) {
            std::string label(applicationHandle->getName());
            label += " - ";
            label += windowHandle->getName();
            return label;
        } else {
            return applicationHandle->getName();
        }
    } else if (windowHandle != nullptr) {
        return windowHandle->getName();
    } else {
        return "<unknown application or window>";
    }
}

void InputDispatcher::pokeUserActivityLocked(const EventEntry* eventEntry) {
    int32_t displayId = getTargetDisplayId(eventEntry);
    sp<InputWindowHandle> focusedWindowHandle =
            getValueByKey(mFocusedWindowHandlesByDisplay, displayId);
    if (focusedWindowHandle != nullptr) {
        const InputWindowInfo* info = focusedWindowHandle->getInfo();
        if (info->inputFeatures & InputWindowInfo::INPUT_FEATURE_DISABLE_USER_ACTIVITY) {
#if DEBUG_DISPATCH_CYCLE
            ALOGD("Not poking user activity: disabled by window '%s'.", info->name.c_str());
#endif
            return;
        }
    }

    int32_t eventType = USER_ACTIVITY_EVENT_OTHER;
    switch (eventEntry->type) {
    case EventEntry::TYPE_MOTION: {
        const MotionEntry* motionEntry = static_cast<const MotionEntry*>(eventEntry);
        if (motionEntry->action == AMOTION_EVENT_ACTION_CANCEL) {
            return;
        }

        if (MotionEvent::isTouchEvent(motionEntry->source, motionEntry->action)) {
            eventType = USER_ACTIVITY_EVENT_TOUCH;
        }
        break;
    }
    case EventEntry::TYPE_KEY: {
        const KeyEntry* keyEntry = static_cast<const KeyEntry*>(eventEntry);
        if (keyEntry->flags & AKEY_EVENT_FLAG_CANCELED) {
            return;
        }
        eventType = USER_ACTIVITY_EVENT_BUTTON;
        break;
    }
    }

    CommandEntry* commandEntry = postCommandLocked(
            & InputDispatcher::doPokeUserActivityLockedInterruptible);
    commandEntry->eventTime = eventEntry->eventTime;
    commandEntry->userActivityEventType = eventType;
}

void InputDispatcher::prepareDispatchCycleLocked(nsecs_t currentTime,
        const sp<Connection>& connection, EventEntry* eventEntry, const InputTarget* inputTarget) {
    if (ATRACE_ENABLED()) {
        std::string message = StringPrintf(
                "prepareDispatchCycleLocked(inputChannel=%s, sequenceNum=%" PRIu32 ")",
                connection->getInputChannelName().c_str(), eventEntry->sequenceNum);
        ATRACE_NAME(message.c_str());
    }
#if DEBUG_DISPATCH_CYCLE
    ALOGD("channel '%s' ~ prepareDispatchCycle - flags=0x%08x, "
            "xOffset=%f, yOffset=%f, globalScaleFactor=%f, "
            "windowScaleFactor=(%f, %f), pointerIds=0x%x",
            connection->getInputChannelName().c_str(), inputTarget->flags,
            inputTarget->xOffset, inputTarget->yOffset,
            inputTarget->globalScaleFactor,
            inputTarget->windowXScale, inputTarget->windowYScale,
            inputTarget->pointerIds.value);
#endif

    // Skip this event if the connection status is not normal.
    // We don't want to enqueue additional outbound events if the connection is broken.
    if (connection->status != Connection::STATUS_NORMAL) {
#if DEBUG_DISPATCH_CYCLE
        ALOGD("channel '%s' ~ Dropping event because the channel status is %s",
                connection->getInputChannelName().c_str(), connection->getStatusLabel());
#endif
        return;
    }

    // Split a motion event if needed.
    if (inputTarget->flags & InputTarget::FLAG_SPLIT) {
        ALOG_ASSERT(eventEntry->type == EventEntry::TYPE_MOTION);

        MotionEntry* originalMotionEntry = static_cast<MotionEntry*>(eventEntry);
        if (inputTarget->pointerIds.count() != originalMotionEntry->pointerCount) {
            MotionEntry* splitMotionEntry = splitMotionEvent(
                    originalMotionEntry, inputTarget->pointerIds);
            if (!splitMotionEntry) {
                return; // split event was dropped
            }
#if DEBUG_FOCUS
            ALOGD("channel '%s' ~ Split motion event.",
                    connection->getInputChannelName().c_str());
            logOutboundMotionDetails("  ", splitMotionEntry);
#endif
            enqueueDispatchEntriesLocked(currentTime, connection,
                    splitMotionEntry, inputTarget);
            splitMotionEntry->release();
            return;
        }
    }

    // Not splitting.  Enqueue dispatch entries for the event as is.
    enqueueDispatchEntriesLocked(currentTime, connection, eventEntry, inputTarget);
}

void InputDispatcher::enqueueDispatchEntriesLocked(nsecs_t currentTime,
        const sp<Connection>& connection, EventEntry* eventEntry, const InputTarget* inputTarget) {
    if (ATRACE_ENABLED()) {
        std::string message = StringPrintf(
                "enqueueDispatchEntriesLocked(inputChannel=%s, sequenceNum=%" PRIu32 ")",
                connection->getInputChannelName().c_str(), eventEntry->sequenceNum);
        ATRACE_NAME(message.c_str());
    }

    bool wasEmpty = connection->outboundQueue.isEmpty();

    // Enqueue dispatch entries for the requested modes.
    enqueueDispatchEntryLocked(connection, eventEntry, inputTarget,
            InputTarget::FLAG_DISPATCH_AS_HOVER_EXIT);
    enqueueDispatchEntryLocked(connection, eventEntry, inputTarget,
            InputTarget::FLAG_DISPATCH_AS_OUTSIDE);
    enqueueDispatchEntryLocked(connection, eventEntry, inputTarget,
            InputTarget::FLAG_DISPATCH_AS_HOVER_ENTER);
    enqueueDispatchEntryLocked(connection, eventEntry, inputTarget,
            InputTarget::FLAG_DISPATCH_AS_IS);
    enqueueDispatchEntryLocked(connection, eventEntry, inputTarget,
            InputTarget::FLAG_DISPATCH_AS_SLIPPERY_EXIT);
    enqueueDispatchEntryLocked(connection, eventEntry, inputTarget,
            InputTarget::FLAG_DISPATCH_AS_SLIPPERY_ENTER);

    // If the outbound queue was previously empty, start the dispatch cycle going.
    if (wasEmpty && !connection->outboundQueue.isEmpty()) {
        startDispatchCycleLocked(currentTime, connection);
    }
}

void InputDispatcher::enqueueDispatchEntryLocked(
        const sp<Connection>& connection, EventEntry* eventEntry, const InputTarget* inputTarget,
        int32_t dispatchMode) {
    if (ATRACE_ENABLED()) {
        std::string message = StringPrintf(
                "enqueueDispatchEntry(inputChannel=%s, dispatchMode=%s)",
                connection->getInputChannelName().c_str(),
                dispatchModeToString(dispatchMode).c_str());
        ATRACE_NAME(message.c_str());
    }
    int32_t inputTargetFlags = inputTarget->flags;
    if (!(inputTargetFlags & dispatchMode)) {
        return;
    }
    inputTargetFlags = (inputTargetFlags & ~InputTarget::FLAG_DISPATCH_MASK) | dispatchMode;

    // This is a new event.
    // Enqueue a new dispatch entry onto the outbound queue for this connection.
    DispatchEntry* dispatchEntry = new DispatchEntry(eventEntry, // increments ref
            inputTargetFlags, inputTarget->xOffset, inputTarget->yOffset,
            inputTarget->globalScaleFactor, inputTarget->windowXScale,
            inputTarget->windowYScale);

    // Apply target flags and update the connection's input state.
    switch (eventEntry->type) {
    case EventEntry::TYPE_KEY: {
        KeyEntry* keyEntry = static_cast<KeyEntry*>(eventEntry);
        dispatchEntry->resolvedAction = keyEntry->action;
        dispatchEntry->resolvedFlags = keyEntry->flags;

        if (!connection->inputState.trackKey(keyEntry,
                dispatchEntry->resolvedAction, dispatchEntry->resolvedFlags)) {
#if DEBUG_DISPATCH_CYCLE
            ALOGD("channel '%s' ~ enqueueDispatchEntryLocked: skipping inconsistent key event",
                    connection->getInputChannelName().c_str());
#endif
            delete dispatchEntry;
            return; // skip the inconsistent event
        }
        break;
    }

    case EventEntry::TYPE_MOTION: {
        MotionEntry* motionEntry = static_cast<MotionEntry*>(eventEntry);
        if (dispatchMode & InputTarget::FLAG_DISPATCH_AS_OUTSIDE) {
            dispatchEntry->resolvedAction = AMOTION_EVENT_ACTION_OUTSIDE;
        } else if (dispatchMode & InputTarget::FLAG_DISPATCH_AS_HOVER_EXIT) {
            dispatchEntry->resolvedAction = AMOTION_EVENT_ACTION_HOVER_EXIT;
        } else if (dispatchMode & InputTarget::FLAG_DISPATCH_AS_HOVER_ENTER) {
            dispatchEntry->resolvedAction = AMOTION_EVENT_ACTION_HOVER_ENTER;
        } else if (dispatchMode & InputTarget::FLAG_DISPATCH_AS_SLIPPERY_EXIT) {
            dispatchEntry->resolvedAction = AMOTION_EVENT_ACTION_CANCEL;
        } else if (dispatchMode & InputTarget::FLAG_DISPATCH_AS_SLIPPERY_ENTER) {
            dispatchEntry->resolvedAction = AMOTION_EVENT_ACTION_DOWN;
        } else {
            dispatchEntry->resolvedAction = motionEntry->action;
        }
        if (dispatchEntry->resolvedAction == AMOTION_EVENT_ACTION_HOVER_MOVE
                && !connection->inputState.isHovering(
                        motionEntry->deviceId, motionEntry->source, motionEntry->displayId)) {
#if DEBUG_DISPATCH_CYCLE
        ALOGD("channel '%s' ~ enqueueDispatchEntryLocked: filling in missing hover enter event",
                connection->getInputChannelName().c_str());
#endif
            dispatchEntry->resolvedAction = AMOTION_EVENT_ACTION_HOVER_ENTER;
        }

        dispatchEntry->resolvedFlags = motionEntry->flags;
        if (dispatchEntry->targetFlags & InputTarget::FLAG_WINDOW_IS_OBSCURED) {
            dispatchEntry->resolvedFlags |= AMOTION_EVENT_FLAG_WINDOW_IS_OBSCURED;
        }
        if (dispatchEntry->targetFlags & InputTarget::FLAG_WINDOW_IS_PARTIALLY_OBSCURED) {
            dispatchEntry->resolvedFlags |= AMOTION_EVENT_FLAG_WINDOW_IS_PARTIALLY_OBSCURED;
        }

        if (!connection->inputState.trackMotion(motionEntry,
                dispatchEntry->resolvedAction, dispatchEntry->resolvedFlags)) {
#if DEBUG_DISPATCH_CYCLE
            ALOGD("channel '%s' ~ enqueueDispatchEntryLocked: skipping inconsistent motion event",
                    connection->getInputChannelName().c_str());
#endif
            delete dispatchEntry;
            return; // skip the inconsistent event
        }

        dispatchPointerDownOutsideFocus(motionEntry->source,
                dispatchEntry->resolvedAction, inputTarget->inputChannel->getToken());

        break;
    }
    }

    // Remember that we are waiting for this dispatch to complete.
    if (dispatchEntry->hasForegroundTarget()) {
        incrementPendingForegroundDispatches(eventEntry);
    }

    // Enqueue the dispatch entry.
    connection->outboundQueue.enqueueAtTail(dispatchEntry);
    traceOutboundQueueLength(connection);

}

void InputDispatcher::dispatchPointerDownOutsideFocus(uint32_t source, int32_t action,
        const sp<IBinder>& newToken) {
    int32_t maskedAction = action & AMOTION_EVENT_ACTION_MASK;
    uint32_t maskedSource = source & AINPUT_SOURCE_CLASS_MASK;
    if (maskedSource != AINPUT_SOURCE_CLASS_POINTER || maskedAction != AMOTION_EVENT_ACTION_DOWN) {
        return;
    }

    sp<InputWindowHandle> inputWindowHandle = getWindowHandleLocked(newToken);
    if (inputWindowHandle == nullptr) {
        return;
    }

    sp<InputWindowHandle> focusedWindowHandle =
            getValueByKey(mFocusedWindowHandlesByDisplay, mFocusedDisplayId);

    bool hasFocusChanged = !focusedWindowHandle || focusedWindowHandle->getToken() != newToken;

    if (!hasFocusChanged) {
        return;
    }

    CommandEntry* commandEntry = postCommandLocked(
            & InputDispatcher::doOnPointerDownOutsideFocusLockedInterruptible);
    commandEntry->newToken = newToken;
}

void InputDispatcher::startDispatchCycleLocked(nsecs_t currentTime,
        const sp<Connection>& connection) {
    if (ATRACE_ENABLED()) {
        std::string message = StringPrintf("startDispatchCycleLocked(inputChannel=%s)",
                connection->getInputChannelName().c_str());
        ATRACE_NAME(message.c_str());
    }
#if DEBUG_DISPATCH_CYCLE
    ALOGD("channel '%s' ~ startDispatchCycle",
            connection->getInputChannelName().c_str());
#endif

    while (connection->status == Connection::STATUS_NORMAL
            && !connection->outboundQueue.isEmpty()) {
        DispatchEntry* dispatchEntry = connection->outboundQueue.head;
        dispatchEntry->deliveryTime = currentTime;

        // Publish the event.
        status_t status;
        EventEntry* eventEntry = dispatchEntry->eventEntry;
        switch (eventEntry->type) {
        case EventEntry::TYPE_KEY: {
            KeyEntry* keyEntry = static_cast<KeyEntry*>(eventEntry);

            // Publish the key event.
            status = connection->inputPublisher.publishKeyEvent(dispatchEntry->seq,
                    keyEntry->deviceId, keyEntry->source, keyEntry->displayId,
                    dispatchEntry->resolvedAction, dispatchEntry->resolvedFlags,
                    keyEntry->keyCode, keyEntry->scanCode,
                    keyEntry->metaState, keyEntry->repeatCount, keyEntry->downTime,
                    keyEntry->eventTime);
            break;
        }

        case EventEntry::TYPE_MOTION: {
            MotionEntry* motionEntry = static_cast<MotionEntry*>(eventEntry);

            PointerCoords scaledCoords[MAX_POINTERS];
            const PointerCoords* usingCoords = motionEntry->pointerCoords;

            // Set the X and Y offset depending on the input source.
            float xOffset, yOffset;
            if ((motionEntry->source & AINPUT_SOURCE_CLASS_POINTER)
                    && !(dispatchEntry->targetFlags & InputTarget::FLAG_ZERO_COORDS)) {
                float globalScaleFactor = dispatchEntry->globalScaleFactor;
                float wxs = dispatchEntry->windowXScale;
                float wys = dispatchEntry->windowYScale;
                xOffset = dispatchEntry->xOffset * wxs;
                yOffset = dispatchEntry->yOffset * wys;
                if (wxs != 1.0f || wys != 1.0f || globalScaleFactor != 1.0f) {
                    for (uint32_t i = 0; i < motionEntry->pointerCount; i++) {
                        scaledCoords[i] = motionEntry->pointerCoords[i];
                        scaledCoords[i].scale(globalScaleFactor, wxs, wys);
                    }
                    usingCoords = scaledCoords;
                }
            } else {
                xOffset = 0.0f;
                yOffset = 0.0f;

                // We don't want the dispatch target to know.
                if (dispatchEntry->targetFlags & InputTarget::FLAG_ZERO_COORDS) {
                    for (uint32_t i = 0; i < motionEntry->pointerCount; i++) {
                        scaledCoords[i].clear();
                    }
                    usingCoords = scaledCoords;
                }
            }

            // Publish the motion event.
            status = connection->inputPublisher.publishMotionEvent(dispatchEntry->seq,
                    motionEntry->deviceId, motionEntry->source, motionEntry->displayId,
                    dispatchEntry->resolvedAction, motionEntry->actionButton,
                    dispatchEntry->resolvedFlags, motionEntry->edgeFlags,
                    motionEntry->metaState, motionEntry->buttonState, motionEntry->classification,
                    xOffset, yOffset, motionEntry->xPrecision, motionEntry->yPrecision,
                    motionEntry->downTime, motionEntry->eventTime,
                    motionEntry->pointerCount, motionEntry->pointerProperties,
                    usingCoords);
            break;
        }

        default:
            ALOG_ASSERT(false);
            return;
        }

        // Check the result.
        if (status) {
            if (status == WOULD_BLOCK) {
                if (connection->waitQueue.isEmpty()) {
                    ALOGE("channel '%s' ~ Could not publish event because the pipe is full. "
                            "This is unexpected because the wait queue is empty, so the pipe "
                            "should be empty and we shouldn't have any problems writing an "
                            "event to it, status=%d", connection->getInputChannelName().c_str(),
                            status);
                    abortBrokenDispatchCycleLocked(currentTime, connection, true /*notify*/);
                } else {
                    // Pipe is full and we are waiting for the app to finish process some events
                    // before sending more events to it.
#if DEBUG_DISPATCH_CYCLE
                    ALOGD("channel '%s' ~ Could not publish event because the pipe is full, "
                            "waiting for the application to catch up",
                            connection->getInputChannelName().c_str());
#endif
                    connection->inputPublisherBlocked = true;
                }
            } else {
                ALOGE("channel '%s' ~ Could not publish event due to an unexpected error, "
                        "status=%d", connection->getInputChannelName().c_str(), status);
                abortBrokenDispatchCycleLocked(currentTime, connection, true /*notify*/);
            }
            return;
        }

        // Re-enqueue the event on the wait queue.
        connection->outboundQueue.dequeue(dispatchEntry);
        traceOutboundQueueLength(connection);
        connection->waitQueue.enqueueAtTail(dispatchEntry);
        traceWaitQueueLength(connection);
    }
}

void InputDispatcher::finishDispatchCycleLocked(nsecs_t currentTime,
        const sp<Connection>& connection, uint32_t seq, bool handled) {
#if DEBUG_DISPATCH_CYCLE
    ALOGD("channel '%s' ~ finishDispatchCycle - seq=%u, handled=%s",
            connection->getInputChannelName().c_str(), seq, toString(handled));
#endif

    connection->inputPublisherBlocked = false;

    if (connection->status == Connection::STATUS_BROKEN
            || connection->status == Connection::STATUS_ZOMBIE) {
        return;
    }

    // Notify other system components and prepare to start the next dispatch cycle.
    onDispatchCycleFinishedLocked(currentTime, connection, seq, handled);
}

void InputDispatcher::abortBrokenDispatchCycleLocked(nsecs_t currentTime,
        const sp<Connection>& connection, bool notify) {
#if DEBUG_DISPATCH_CYCLE
    ALOGD("channel '%s' ~ abortBrokenDispatchCycle - notify=%s",
            connection->getInputChannelName().c_str(), toString(notify));
#endif

    // Clear the dispatch queues.
    drainDispatchQueue(&connection->outboundQueue);
    traceOutboundQueueLength(connection);
    drainDispatchQueue(&connection->waitQueue);
    traceWaitQueueLength(connection);

    // The connection appears to be unrecoverably broken.
    // Ignore already broken or zombie connections.
    if (connection->status == Connection::STATUS_NORMAL) {
        connection->status = Connection::STATUS_BROKEN;

        if (notify) {
            // Notify other system components.
            onDispatchCycleBrokenLocked(currentTime, connection);
        }
    }
}

void InputDispatcher::drainDispatchQueue(Queue<DispatchEntry>* queue) {
    while (!queue->isEmpty()) {
        DispatchEntry* dispatchEntry = queue->dequeueAtHead();
        releaseDispatchEntry(dispatchEntry);
    }
}

void InputDispatcher::releaseDispatchEntry(DispatchEntry* dispatchEntry) {
    if (dispatchEntry->hasForegroundTarget()) {
        decrementPendingForegroundDispatches(dispatchEntry->eventEntry);
    }
    delete dispatchEntry;
}

int InputDispatcher::handleReceiveCallback(int fd, int events, void* data) {
    InputDispatcher* d = static_cast<InputDispatcher*>(data);

    { // acquire lock
        std::scoped_lock _l(d->mLock);

        ssize_t connectionIndex = d->mConnectionsByFd.indexOfKey(fd);
        if (connectionIndex < 0) {
            ALOGE("Received spurious receive callback for unknown input channel.  "
                    "fd=%d, events=0x%x", fd, events);
            return 0; // remove the callback
        }

        bool notify;
        sp<Connection> connection = d->mConnectionsByFd.valueAt(connectionIndex);
        if (!(events & (ALOOPER_EVENT_ERROR | ALOOPER_EVENT_HANGUP))) {
            if (!(events & ALOOPER_EVENT_INPUT)) {
                ALOGW("channel '%s' ~ Received spurious callback for unhandled poll event.  "
                        "events=0x%x", connection->getInputChannelName().c_str(), events);
                return 1;
            }

            nsecs_t currentTime = now();
            bool gotOne = false;
            status_t status;
            for (;;) {
                uint32_t seq;
                bool handled;
                status = connection->inputPublisher.receiveFinishedSignal(&seq, &handled);
                if (status) {
                    break;
                }
                d->finishDispatchCycleLocked(currentTime, connection, seq, handled);
                gotOne = true;
            }
            if (gotOne) {
                d->runCommandsLockedInterruptible();
                if (status == WOULD_BLOCK) {
                    return 1;
                }
            }

            notify = status != DEAD_OBJECT || !connection->monitor;
            if (notify) {
                ALOGE("channel '%s' ~ Failed to receive finished signal.  status=%d",
                        connection->getInputChannelName().c_str(), status);
            }
        } else {
            // Monitor channels are never explicitly unregistered.
            // We do it automatically when the remote endpoint is closed so don't warn
            // about them.
            notify = !connection->monitor;
            if (notify) {
                ALOGW("channel '%s' ~ Consumer closed input channel or an error occurred.  "
                        "events=0x%x", connection->getInputChannelName().c_str(), events);
            }
        }

        // Unregister the channel.
        d->unregisterInputChannelLocked(connection->inputChannel, notify);
        return 0; // remove the callback
    } // release lock
}

void InputDispatcher::synthesizeCancelationEventsForAllConnectionsLocked (
        const CancelationOptions& options) {
    for (size_t i = 0; i < mConnectionsByFd.size(); i++) {
        synthesizeCancelationEventsForConnectionLocked(
                mConnectionsByFd.valueAt(i), options);
    }
}

void InputDispatcher::synthesizeCancelationEventsForMonitorsLocked (
        const CancelationOptions& options) {
    synthesizeCancelationEventsForMonitorsLocked(options, mGlobalMonitorsByDisplay);
    synthesizeCancelationEventsForMonitorsLocked(options, mGestureMonitorsByDisplay);
}

void InputDispatcher::synthesizeCancelationEventsForMonitorsLocked(
        const CancelationOptions& options,
        std::unordered_map<int32_t, std::vector<Monitor>>& monitorsByDisplay) {
    for (const auto& it : monitorsByDisplay) {
        const std::vector<Monitor>& monitors = it.second;
        for (const Monitor& monitor : monitors) {
            synthesizeCancelationEventsForInputChannelLocked(monitor.inputChannel, options);
        }
    }
}

void InputDispatcher::synthesizeCancelationEventsForInputChannelLocked(
        const sp<InputChannel>& channel, const CancelationOptions& options) {
    ssize_t index = getConnectionIndexLocked(channel);
    if (index >= 0) {
        synthesizeCancelationEventsForConnectionLocked(
                mConnectionsByFd.valueAt(index), options);
    }
}

void InputDispatcher::synthesizeCancelationEventsForConnectionLocked(
        const sp<Connection>& connection, const CancelationOptions& options) {
    if (connection->status == Connection::STATUS_BROKEN) {
        return;
    }

    nsecs_t currentTime = now();

    std::vector<EventEntry*> cancelationEvents;
    connection->inputState.synthesizeCancelationEvents(currentTime,
            cancelationEvents, options);

    if (!cancelationEvents.empty()) {
#if DEBUG_OUTBOUND_EVENT_DETAILS
        ALOGD("channel '%s' ~ Synthesized %zu cancelation events to bring channel back in sync "
                "with reality: %s, mode=%d.",
                connection->getInputChannelName().c_str(), cancelationEvents.size(),
                options.reason, options.mode);
#endif
        for (size_t i = 0; i < cancelationEvents.size(); i++) {
            EventEntry* cancelationEventEntry = cancelationEvents[i];
            switch (cancelationEventEntry->type) {
            case EventEntry::TYPE_KEY:
                logOutboundKeyDetails("cancel - ",
                        static_cast<KeyEntry*>(cancelationEventEntry));
                break;
            case EventEntry::TYPE_MOTION:
                logOutboundMotionDetails("cancel - ",
                        static_cast<MotionEntry*>(cancelationEventEntry));
                break;
            }

            InputTarget target;
            sp<InputWindowHandle> windowHandle = getWindowHandleLocked(
                    connection->inputChannel->getToken());
            if (windowHandle != nullptr) {
                const InputWindowInfo* windowInfo = windowHandle->getInfo();
                target.xOffset = -windowInfo->frameLeft;
                target.yOffset = -windowInfo->frameTop;
                target.globalScaleFactor = windowInfo->globalScaleFactor;
                target.windowXScale = windowInfo->windowXScale;
                target.windowYScale = windowInfo->windowYScale;
            } else {
                target.xOffset = 0;
                target.yOffset = 0;
                target.globalScaleFactor = 1.0f;
            }
            target.inputChannel = connection->inputChannel;
            target.flags = InputTarget::FLAG_DISPATCH_AS_IS;

            enqueueDispatchEntryLocked(connection, cancelationEventEntry, // increments ref
                    &target, InputTarget::FLAG_DISPATCH_AS_IS);

            cancelationEventEntry->release();
        }

        startDispatchCycleLocked(currentTime, connection);
    }
}

InputDispatcher::MotionEntry*
InputDispatcher::splitMotionEvent(const MotionEntry* originalMotionEntry, BitSet32 pointerIds) {
    ALOG_ASSERT(pointerIds.value != 0);

    uint32_t splitPointerIndexMap[MAX_POINTERS];
    PointerProperties splitPointerProperties[MAX_POINTERS];
    PointerCoords splitPointerCoords[MAX_POINTERS];

    uint32_t originalPointerCount = originalMotionEntry->pointerCount;
    uint32_t splitPointerCount = 0;

    for (uint32_t originalPointerIndex = 0; originalPointerIndex < originalPointerCount;
            originalPointerIndex++) {
        const PointerProperties& pointerProperties =
                originalMotionEntry->pointerProperties[originalPointerIndex];
        uint32_t pointerId = uint32_t(pointerProperties.id);
        if (pointerIds.hasBit(pointerId)) {
            splitPointerIndexMap[splitPointerCount] = originalPointerIndex;
            splitPointerProperties[splitPointerCount].copyFrom(pointerProperties);
            splitPointerCoords[splitPointerCount].copyFrom(
                    originalMotionEntry->pointerCoords[originalPointerIndex]);
            splitPointerCount += 1;
        }
    }

    if (splitPointerCount != pointerIds.count()) {
        // This is bad.  We are missing some of the pointers that we expected to deliver.
        // Most likely this indicates that we received an ACTION_MOVE events that has
        // different pointer ids than we expected based on the previous ACTION_DOWN
        // or ACTION_POINTER_DOWN events that caused us to decide to split the pointers
        // in this way.
        ALOGW("Dropping split motion event because the pointer count is %d but "
                "we expected there to be %d pointers.  This probably means we received "
                "a broken sequence of pointer ids from the input device.",
                splitPointerCount, pointerIds.count());
        return nullptr;
    }

    int32_t action = originalMotionEntry->action;
    int32_t maskedAction = action & AMOTION_EVENT_ACTION_MASK;
    if (maskedAction == AMOTION_EVENT_ACTION_POINTER_DOWN
            || maskedAction == AMOTION_EVENT_ACTION_POINTER_UP) {
        int32_t originalPointerIndex = getMotionEventActionPointerIndex(action);
        const PointerProperties& pointerProperties =
                originalMotionEntry->pointerProperties[originalPointerIndex];
        uint32_t pointerId = uint32_t(pointerProperties.id);
        if (pointerIds.hasBit(pointerId)) {
            if (pointerIds.count() == 1) {
                // The first/last pointer went down/up.
                action = maskedAction == AMOTION_EVENT_ACTION_POINTER_DOWN
                        ? AMOTION_EVENT_ACTION_DOWN : AMOTION_EVENT_ACTION_UP;
            } else {
                // A secondary pointer went down/up.
                uint32_t splitPointerIndex = 0;
                while (pointerId != uint32_t(splitPointerProperties[splitPointerIndex].id)) {
                    splitPointerIndex += 1;
                }
                action = maskedAction | (splitPointerIndex
                        << AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT);
            }
        } else {
            // An unrelated pointer changed.
            action = AMOTION_EVENT_ACTION_MOVE;
        }
    }

    MotionEntry* splitMotionEntry = new MotionEntry(
            originalMotionEntry->sequenceNum,
            originalMotionEntry->eventTime,
            originalMotionEntry->deviceId,
            originalMotionEntry->source,
            originalMotionEntry->displayId,
            originalMotionEntry->policyFlags,
            action,
            originalMotionEntry->actionButton,
            originalMotionEntry->flags,
            originalMotionEntry->metaState,
            originalMotionEntry->buttonState,
            originalMotionEntry->classification,
            originalMotionEntry->edgeFlags,
            originalMotionEntry->xPrecision,
            originalMotionEntry->yPrecision,
            originalMotionEntry->downTime,
            splitPointerCount, splitPointerProperties, splitPointerCoords, 0, 0);

    if (originalMotionEntry->injectionState) {
        splitMotionEntry->injectionState = originalMotionEntry->injectionState;
        splitMotionEntry->injectionState->refCount += 1;
    }

    return splitMotionEntry;
}

void InputDispatcher::notifyConfigurationChanged(const NotifyConfigurationChangedArgs* args) {
#if DEBUG_INBOUND_EVENT_DETAILS
    ALOGD("notifyConfigurationChanged - eventTime=%" PRId64, args->eventTime);
#endif

    bool needWake;
    { // acquire lock
        std::scoped_lock _l(mLock);

        ConfigurationChangedEntry* newEntry =
                new ConfigurationChangedEntry(args->sequenceNum, args->eventTime);
        needWake = enqueueInboundEventLocked(newEntry);
    } // release lock

    if (needWake) {
        mLooper->wake();
    }
}

/**
 * If one of the meta shortcuts is detected, process them here:
 *     Meta + Backspace -> generate BACK
 *     Meta + Enter -> generate HOME
 * This will potentially overwrite keyCode and metaState.
 */
void InputDispatcher::accelerateMetaShortcuts(const int32_t deviceId, const int32_t action,
        int32_t& keyCode, int32_t& metaState) {
    if (metaState & AMETA_META_ON && action == AKEY_EVENT_ACTION_DOWN) {
        int32_t newKeyCode = AKEYCODE_UNKNOWN;
        if (keyCode == AKEYCODE_DEL) {
            newKeyCode = AKEYCODE_BACK;
        } else if (keyCode == AKEYCODE_ENTER) {
            newKeyCode = AKEYCODE_HOME;
        }
        if (newKeyCode != AKEYCODE_UNKNOWN) {
            std::scoped_lock _l(mLock);
            struct KeyReplacement replacement = {keyCode, deviceId};
            mReplacedKeys.add(replacement, newKeyCode);
            keyCode = newKeyCode;
            metaState &= ~(AMETA_META_ON | AMETA_META_LEFT_ON | AMETA_META_RIGHT_ON);
        }
    } else if (action == AKEY_EVENT_ACTION_UP) {
        // In order to maintain a consistent stream of up and down events, check to see if the key
        // going up is one we've replaced in a down event and haven't yet replaced in an up event,
        // even if the modifier was released between the down and the up events.
        std::scoped_lock _l(mLock);
        struct KeyReplacement replacement = {keyCode, deviceId};
        ssize_t index = mReplacedKeys.indexOfKey(replacement);
        if (index >= 0) {
            keyCode = mReplacedKeys.valueAt(index);
            mReplacedKeys.removeItemsAt(index);
            metaState &= ~(AMETA_META_ON | AMETA_META_LEFT_ON | AMETA_META_RIGHT_ON);
        }
    }
}

void InputDispatcher::notifyKey(const NotifyKeyArgs* args) {
#if DEBUG_INBOUND_EVENT_DETAILS
    ALOGD("notifyKey - eventTime=%" PRId64
            ", deviceId=%d, source=0x%x, displayId=%" PRId32 "policyFlags=0x%x, action=0x%x, "
            "flags=0x%x, keyCode=0x%x, scanCode=0x%x, metaState=0x%x, downTime=%" PRId64,
            args->eventTime, args->deviceId, args->source, args->displayId, args->policyFlags,
            args->action, args->flags, args->keyCode, args->scanCode,
            args->metaState, args->downTime);
#endif
    if (!validateKeyEvent(args->action)) {
        return;
    }

    uint32_t policyFlags = args->policyFlags;
    int32_t flags = args->flags;
    int32_t metaState = args->metaState;
    // InputDispatcher tracks and generates key repeats on behalf of
    // whatever notifies it, so repeatCount should always be set to 0
    constexpr int32_t repeatCount = 0;
    if ((policyFlags & POLICY_FLAG_VIRTUAL) || (flags & AKEY_EVENT_FLAG_VIRTUAL_HARD_KEY)) {
        policyFlags |= POLICY_FLAG_VIRTUAL;
        flags |= AKEY_EVENT_FLAG_VIRTUAL_HARD_KEY;
    }
    if (policyFlags & POLICY_FLAG_FUNCTION) {
        metaState |= AMETA_FUNCTION_ON;
    }

    policyFlags |= POLICY_FLAG_TRUSTED;

    int32_t keyCode = args->keyCode;
    accelerateMetaShortcuts(args->deviceId, args->action, keyCode, metaState);

    KeyEvent event;
    event.initialize(args->deviceId, args->source, args->displayId, args->action,
            flags, keyCode, args->scanCode, metaState, repeatCount,
            args->downTime, args->eventTime);

    android::base::Timer t;
    mPolicy->interceptKeyBeforeQueueing(&event, /*byref*/ policyFlags);
    if (t.duration() > SLOW_INTERCEPTION_THRESHOLD) {
        ALOGW("Excessive delay in interceptKeyBeforeQueueing; took %s ms",
                std::to_string(t.duration().count()).c_str());
    }

    bool needWake;
    { // acquire lock
        mLock.lock();

        if (shouldSendKeyToInputFilterLocked(args)) {
            mLock.unlock();

            policyFlags |= POLICY_FLAG_FILTERED;
            if (!mPolicy->filterInputEvent(&event, policyFlags)) {
                return; // event was consumed by the filter
            }

            mLock.lock();
        }

        KeyEntry* newEntry = new KeyEntry(args->sequenceNum, args->eventTime,
                args->deviceId, args->source, args->displayId, policyFlags,
                args->action, flags, keyCode, args->scanCode,
                metaState, repeatCount, args->downTime);

        needWake = enqueueInboundEventLocked(newEntry);
        mLock.unlock();
    } // release lock

    if (needWake) {
        mLooper->wake();
    }
}

bool InputDispatcher::shouldSendKeyToInputFilterLocked(const NotifyKeyArgs* args) {
    return mInputFilterEnabled;
}

void InputDispatcher::notifyMotion(const NotifyMotionArgs* args) {
#if DEBUG_INBOUND_EVENT_DETAILS
    ALOGD("notifyMotion - eventTime=%" PRId64 ", deviceId=%d, source=0x%x, displayId=%" PRId32
            ", policyFlags=0x%x, "
            "action=0x%x, actionButton=0x%x, flags=0x%x, metaState=0x%x, buttonState=0x%x,"
            "edgeFlags=0x%x, xPrecision=%f, yPrecision=%f, downTime=%" PRId64,
            args->eventTime, args->deviceId, args->source, args->displayId, args->policyFlags,
            args->action, args->actionButton, args->flags, args->metaState, args->buttonState,
            args->edgeFlags, args->xPrecision, args->yPrecision, args->downTime);
    for (uint32_t i = 0; i < args->pointerCount; i++) {
        ALOGD("  Pointer %d: id=%d, toolType=%d, "
                "x=%f, y=%f, pressure=%f, size=%f, "
                "touchMajor=%f, touchMinor=%f, toolMajor=%f, toolMinor=%f, "
                "orientation=%f",
                i, args->pointerProperties[i].id,
                args->pointerProperties[i].toolType,
                args->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_X),
                args->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_Y),
                args->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_PRESSURE),
                args->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_SIZE),
                args->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_TOUCH_MAJOR),
                args->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_TOUCH_MINOR),
                args->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_TOOL_MAJOR),
                args->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_TOOL_MINOR),
                args->pointerCoords[i].getAxisValue(AMOTION_EVENT_AXIS_ORIENTATION));
    }
#endif
    if (!validateMotionEvent(args->action, args->actionButton,
                args->pointerCount, args->pointerProperties)) {
        return;
    }

    uint32_t policyFlags = args->policyFlags;
    policyFlags |= POLICY_FLAG_TRUSTED;

    android::base::Timer t;
    mPolicy->interceptMotionBeforeQueueing(args->displayId, args->eventTime, /*byref*/ policyFlags);
    if (t.duration() > SLOW_INTERCEPTION_THRESHOLD) {
        ALOGW("Excessive delay in interceptMotionBeforeQueueing; took %s ms",
                std::to_string(t.duration().count()).c_str());
    }

    bool needWake;
    { // acquire lock
        mLock.lock();

        if (shouldSendMotionToInputFilterLocked(args)) {
            mLock.unlock();

            MotionEvent event;
            event.initialize(args->deviceId, args->source, args->displayId,
                    args->action, args->actionButton,
                    args->flags, args->edgeFlags, args->metaState, args->buttonState,
                    args->classification, 0, 0, args->xPrecision, args->yPrecision,
                    args->downTime, args->eventTime,
                    args->pointerCount, args->pointerProperties, args->pointerCoords);

            policyFlags |= POLICY_FLAG_FILTERED;
            if (!mPolicy->filterInputEvent(&event, policyFlags)) {
                return; // event was consumed by the filter
            }

            mLock.lock();
        }

        // Just enqueue a new motion event.
        MotionEntry* newEntry = new MotionEntry(args->sequenceNum, args->eventTime,
                args->deviceId, args->source, args->displayId, policyFlags,
                args->action, args->actionButton, args->flags,
                args->metaState, args->buttonState, args->classification,
                args->edgeFlags, args->xPrecision, args->yPrecision, args->downTime,
                args->pointerCount, args->pointerProperties, args->pointerCoords, 0, 0);

        needWake = enqueueInboundEventLocked(newEntry);
        mLock.unlock();
    } // release lock

    if (needWake) {
        mLooper->wake();
    }
}

bool InputDispatcher::shouldSendMotionToInputFilterLocked(const NotifyMotionArgs* args) {
    return mInputFilterEnabled;
}

void InputDispatcher::notifySwitch(const NotifySwitchArgs* args) {
#if DEBUG_INBOUND_EVENT_DETAILS
    ALOGD("notifySwitch - eventTime=%" PRId64 ", policyFlags=0x%x, switchValues=0x%08x, "
            "switchMask=0x%08x",
            args->eventTime, args->policyFlags, args->switchValues, args->switchMask);
#endif

    uint32_t policyFlags = args->policyFlags;
    policyFlags |= POLICY_FLAG_TRUSTED;
    mPolicy->notifySwitch(args->eventTime,
            args->switchValues, args->switchMask, policyFlags);
}

void InputDispatcher::notifyDeviceReset(const NotifyDeviceResetArgs* args) {
#if DEBUG_INBOUND_EVENT_DETAILS
    ALOGD("notifyDeviceReset - eventTime=%" PRId64 ", deviceId=%d",
            args->eventTime, args->deviceId);
#endif

    bool needWake;
    { // acquire lock
        std::scoped_lock _l(mLock);

        DeviceResetEntry* newEntry =
                new DeviceResetEntry(args->sequenceNum, args->eventTime, args->deviceId);
        needWake = enqueueInboundEventLocked(newEntry);
    } // release lock

    if (needWake) {
        mLooper->wake();
    }
}

int32_t InputDispatcher::injectInputEvent(const InputEvent* event,
        int32_t injectorPid, int32_t injectorUid, int32_t syncMode, int32_t timeoutMillis,
        uint32_t policyFlags) {
#if DEBUG_INBOUND_EVENT_DETAILS
    ALOGD("injectInputEvent - eventType=%d, injectorPid=%d, injectorUid=%d, "
            "syncMode=%d, timeoutMillis=%d, policyFlags=0x%08x",
            event->getType(), injectorPid, injectorUid, syncMode, timeoutMillis, policyFlags);
#endif

    nsecs_t endTime = now() + milliseconds_to_nanoseconds(timeoutMillis);

    policyFlags |= POLICY_FLAG_INJECTED;
    if (hasInjectionPermission(injectorPid, injectorUid)) {
        policyFlags |= POLICY_FLAG_TRUSTED;
    }

    EventEntry* firstInjectedEntry;
    EventEntry* lastInjectedEntry;
    switch (event->getType()) {
    case AINPUT_EVENT_TYPE_KEY: {
        KeyEvent keyEvent;
        keyEvent.initialize(*static_cast<const KeyEvent*>(event));
        int32_t action = keyEvent.getAction();
        if (! validateKeyEvent(action)) {
            return INPUT_EVENT_INJECTION_FAILED;
        }

        int32_t flags = keyEvent.getFlags();
        int32_t keyCode = keyEvent.getKeyCode();
        int32_t metaState = keyEvent.getMetaState();
        accelerateMetaShortcuts(keyEvent.getDeviceId(), action,
                /*byref*/ keyCode, /*byref*/ metaState);
        keyEvent.initialize(keyEvent.getDeviceId(), keyEvent.getSource(), keyEvent.getDisplayId(),
            action, flags, keyCode, keyEvent.getScanCode(), metaState, keyEvent.getRepeatCount(),
            keyEvent.getDownTime(), keyEvent.getEventTime());

        if (flags & AKEY_EVENT_FLAG_VIRTUAL_HARD_KEY) {
            policyFlags |= POLICY_FLAG_VIRTUAL;
        }

        if (!(policyFlags & POLICY_FLAG_FILTERED)) {
            android::base::Timer t;
            mPolicy->interceptKeyBeforeQueueing(&keyEvent, /*byref*/ policyFlags);
            if (t.duration() > SLOW_INTERCEPTION_THRESHOLD) {
                ALOGW("Excessive delay in interceptKeyBeforeQueueing; took %s ms",
                        std::to_string(t.duration().count()).c_str());
            }
        }

        mLock.lock();
        firstInjectedEntry = new KeyEntry(SYNTHESIZED_EVENT_SEQUENCE_NUM, keyEvent.getEventTime(),
                keyEvent.getDeviceId(), keyEvent.getSource(), keyEvent.getDisplayId(),
                policyFlags, action, flags,
                keyEvent.getKeyCode(), keyEvent.getScanCode(), keyEvent.getMetaState(),
                keyEvent.getRepeatCount(), keyEvent.getDownTime());
        lastInjectedEntry = firstInjectedEntry;
        break;
    }

    case AINPUT_EVENT_TYPE_MOTION: {
        const MotionEvent* motionEvent = static_cast<const MotionEvent*>(event);
        int32_t action = motionEvent->getAction();
        size_t pointerCount = motionEvent->getPointerCount();
        const PointerProperties* pointerProperties = motionEvent->getPointerProperties();
        int32_t actionButton = motionEvent->getActionButton();
        int32_t displayId = motionEvent->getDisplayId();
        if (! validateMotionEvent(action, actionButton, pointerCount, pointerProperties)) {
            return INPUT_EVENT_INJECTION_FAILED;
        }

        if (!(policyFlags & POLICY_FLAG_FILTERED)) {
            nsecs_t eventTime = motionEvent->getEventTime();
            android::base::Timer t;
            mPolicy->interceptMotionBeforeQueueing(displayId, eventTime, /*byref*/ policyFlags);
            if (t.duration() > SLOW_INTERCEPTION_THRESHOLD) {
                ALOGW("Excessive delay in interceptMotionBeforeQueueing; took %s ms",
                        std::to_string(t.duration().count()).c_str());
            }
        }

        mLock.lock();
        const nsecs_t* sampleEventTimes = motionEvent->getSampleEventTimes();
        const PointerCoords* samplePointerCoords = motionEvent->getSamplePointerCoords();
        firstInjectedEntry = new MotionEntry(SYNTHESIZED_EVENT_SEQUENCE_NUM, *sampleEventTimes,
                motionEvent->getDeviceId(), motionEvent->getSource(), motionEvent->getDisplayId(),
                policyFlags,
                action, actionButton, motionEvent->getFlags(),
                motionEvent->getMetaState(), motionEvent->getButtonState(),
                motionEvent->getClassification(), motionEvent->getEdgeFlags(),
                motionEvent->getXPrecision(), motionEvent->getYPrecision(),
                motionEvent->getDownTime(),
                uint32_t(pointerCount), pointerProperties, samplePointerCoords,
                motionEvent->getXOffset(), motionEvent->getYOffset());
        lastInjectedEntry = firstInjectedEntry;
        for (size_t i = motionEvent->getHistorySize(); i > 0; i--) {
            sampleEventTimes += 1;
            samplePointerCoords += pointerCount;
            MotionEntry* nextInjectedEntry = new MotionEntry(SYNTHESIZED_EVENT_SEQUENCE_NUM,
                    *sampleEventTimes,
                    motionEvent->getDeviceId(), motionEvent->getSource(),
                    motionEvent->getDisplayId(), policyFlags,
                    action, actionButton, motionEvent->getFlags(),
                    motionEvent->getMetaState(), motionEvent->getButtonState(),
                    motionEvent->getClassification(), motionEvent->getEdgeFlags(),
                    motionEvent->getXPrecision(), motionEvent->getYPrecision(),
                    motionEvent->getDownTime(),
                    uint32_t(pointerCount), pointerProperties, samplePointerCoords,
                    motionEvent->getXOffset(), motionEvent->getYOffset());
            lastInjectedEntry->next = nextInjectedEntry;
            lastInjectedEntry = nextInjectedEntry;
        }
        break;
    }

    default:
        ALOGW("Cannot inject event of type %d", event->getType());
        return INPUT_EVENT_INJECTION_FAILED;
    }

    InjectionState* injectionState = new InjectionState(injectorPid, injectorUid);
    if (syncMode == INPUT_EVENT_INJECTION_SYNC_NONE) {
        injectionState->injectionIsAsync = true;
    }

    injectionState->refCount += 1;
    lastInjectedEntry->injectionState = injectionState;

    bool needWake = false;
    for (EventEntry* entry = firstInjectedEntry; entry != nullptr; ) {
        EventEntry* nextEntry = entry->next;
        needWake |= enqueueInboundEventLocked(entry);
        entry = nextEntry;
    }

    mLock.unlock();

    if (needWake) {
        mLooper->wake();
    }

    int32_t injectionResult;
    { // acquire lock
        std::unique_lock _l(mLock);

        if (syncMode == INPUT_EVENT_INJECTION_SYNC_NONE) {
            injectionResult = INPUT_EVENT_INJECTION_SUCCEEDED;
        } else {
            for (;;) {
                injectionResult = injectionState->injectionResult;
                if (injectionResult != INPUT_EVENT_INJECTION_PENDING) {
                    break;
                }

                nsecs_t remainingTimeout = endTime - now();
                if (remainingTimeout <= 0) {
#if DEBUG_INJECTION
                    ALOGD("injectInputEvent - Timed out waiting for injection result "
                            "to become available.");
#endif
                    injectionResult = INPUT_EVENT_INJECTION_TIMED_OUT;
                    break;
                }

                mInjectionResultAvailable.wait_for(_l, std::chrono::nanoseconds(remainingTimeout));
            }

            if (injectionResult == INPUT_EVENT_INJECTION_SUCCEEDED
                    && syncMode == INPUT_EVENT_INJECTION_SYNC_WAIT_FOR_FINISHED) {
                while (injectionState->pendingForegroundDispatches != 0) {
#if DEBUG_INJECTION
                    ALOGD("injectInputEvent - Waiting for %d pending foreground dispatches.",
                            injectionState->pendingForegroundDispatches);
#endif
                    nsecs_t remainingTimeout = endTime - now();
                    if (remainingTimeout <= 0) {
#if DEBUG_INJECTION
                    ALOGD("injectInputEvent - Timed out waiting for pending foreground "
                            "dispatches to finish.");
#endif
                        injectionResult = INPUT_EVENT_INJECTION_TIMED_OUT;
                        break;
                    }

                    mInjectionSyncFinished.wait_for(_l, std::chrono::nanoseconds(remainingTimeout));
                }
            }
        }

        injectionState->release();
    } // release lock

#if DEBUG_INJECTION
    ALOGD("injectInputEvent - Finished with result %d.  "
            "injectorPid=%d, injectorUid=%d",
            injectionResult, injectorPid, injectorUid);
#endif

    return injectionResult;
}

bool InputDispatcher::hasInjectionPermission(int32_t injectorPid, int32_t injectorUid) {
    return injectorUid == 0
            || mPolicy->checkInjectEventsPermissionNonReentrant(injectorPid, injectorUid);
}

void InputDispatcher::setInjectionResult(EventEntry* entry, int32_t injectionResult) {
    InjectionState* injectionState = entry->injectionState;
    if (injectionState) {
#if DEBUG_INJECTION
        ALOGD("Setting input event injection result to %d.  "
                "injectorPid=%d, injectorUid=%d",
                 injectionResult, injectionState->injectorPid, injectionState->injectorUid);
#endif

        if (injectionState->injectionIsAsync
                && !(entry->policyFlags & POLICY_FLAG_FILTERED)) {
            // Log the outcome since the injector did not wait for the injection result.
            switch (injectionResult) {
            case INPUT_EVENT_INJECTION_SUCCEEDED:
                ALOGV("Asynchronous input event injection succeeded.");
                break;
            case INPUT_EVENT_INJECTION_FAILED:
                ALOGW("Asynchronous input event injection failed.");
                break;
            case INPUT_EVENT_INJECTION_PERMISSION_DENIED:
                ALOGW("Asynchronous input event injection permission denied.");
                break;
            case INPUT_EVENT_INJECTION_TIMED_OUT:
                ALOGW("Asynchronous input event injection timed out.");
                break;
            }
        }

        injectionState->injectionResult = injectionResult;
        mInjectionResultAvailable.notify_all();
    }
}

void InputDispatcher::incrementPendingForegroundDispatches(EventEntry* entry) {
    InjectionState* injectionState = entry->injectionState;
    if (injectionState) {
        injectionState->pendingForegroundDispatches += 1;
    }
}

void InputDispatcher::decrementPendingForegroundDispatches(EventEntry* entry) {
    InjectionState* injectionState = entry->injectionState;
    if (injectionState) {
        injectionState->pendingForegroundDispatches -= 1;

        if (injectionState->pendingForegroundDispatches == 0) {
            mInjectionSyncFinished.notify_all();
        }
    }
}

std::vector<sp<InputWindowHandle>> InputDispatcher::getWindowHandlesLocked(
        int32_t displayId) const {
    std::unordered_map<int32_t, std::vector<sp<InputWindowHandle>>>::const_iterator it =
            mWindowHandlesByDisplay.find(displayId);
    if(it != mWindowHandlesByDisplay.end()) {
        return it->second;
    }

    // Return an empty one if nothing found.
    return std::vector<sp<InputWindowHandle>>();
}

sp<InputWindowHandle> InputDispatcher::getWindowHandleLocked(
        const sp<IBinder>& windowHandleToken) const {
    for (auto& it : mWindowHandlesByDisplay) {
        const std::vector<sp<InputWindowHandle>> windowHandles = it.second;
        for (const sp<InputWindowHandle>& windowHandle : windowHandles) {
            if (windowHandle->getToken() == windowHandleToken) {
                return windowHandle;
            }
        }
    }
    return nullptr;
}

bool InputDispatcher::hasWindowHandleLocked(const sp<InputWindowHandle>& windowHandle) const {
    for (auto& it : mWindowHandlesByDisplay) {
        const std::vector<sp<InputWindowHandle>> windowHandles = it.second;
        for (const sp<InputWindowHandle>& handle : windowHandles) {
            if (handle->getToken() == windowHandle->getToken()) {
                if (windowHandle->getInfo()->displayId != it.first) {
                    ALOGE("Found window %s in display %" PRId32
                            ", but it should belong to display %" PRId32,
                            windowHandle->getName().c_str(), it.first,
                            windowHandle->getInfo()->displayId);
                }
                return true;
            }
        }
    }
    return false;
}

sp<InputChannel> InputDispatcher::getInputChannelLocked(const sp<IBinder>& token) const {
    size_t count = mInputChannelsByToken.count(token);
    if (count == 0) {
        return nullptr;
    }
    return mInputChannelsByToken.at(token);
}

/**
 * Called from InputManagerService, update window handle list by displayId that can receive input.
 * A window handle contains information about InputChannel, Touch Region, Types, Focused,...
 * If set an empty list, remove all handles from the specific display.
 * For focused handle, check if need to change and send a cancel event to previous one.
 * For removed handle, check if need to send a cancel event if already in touch.
 */
void InputDispatcher::setInputWindows(const std::vector<sp<InputWindowHandle>>& inputWindowHandles,
        int32_t displayId, const sp<ISetInputWindowsListener>& setInputWindowsListener) {
#if DEBUG_FOCUS
    ALOGD("setInputWindows displayId=%" PRId32, displayId);
#endif
    { // acquire lock
        std::scoped_lock _l(mLock);

        // Copy old handles for release if they are no longer present.
        const std::vector<sp<InputWindowHandle>> oldWindowHandles =
                getWindowHandlesLocked(displayId);

        sp<InputWindowHandle> newFocusedWindowHandle = nullptr;
        bool foundHoveredWindow = false;

        if (inputWindowHandles.empty()) {
            // Remove all handles on a display if there are no windows left.
            mWindowHandlesByDisplay.erase(displayId);
        } else {
            // Since we compare the pointer of input window handles across window updates, we need
            // to make sure the handle object for the same window stays unchanged across updates.
            const std::vector<sp<InputWindowHandle>>& oldHandles =
                    mWindowHandlesByDisplay[displayId];
            std::unordered_map<sp<IBinder>, sp<InputWindowHandle>, IBinderHash> oldHandlesByTokens;
            for (const sp<InputWindowHandle>& handle : oldHandles) {
                oldHandlesByTokens[handle->getToken()] = handle;
            }

            std::vector<sp<InputWindowHandle>> newHandles;
            for (const sp<InputWindowHandle>& handle : inputWindowHandles) {
                if (!handle->updateInfo()) {
                    // handle no longer valid
                    continue;
                }
                const InputWindowInfo* info = handle->getInfo();

                if ((getInputChannelLocked(handle->getToken()) == nullptr &&
                     info->portalToDisplayId == ADISPLAY_ID_NONE)) {
                    const bool noInputChannel =
                            info->inputFeatures & InputWindowInfo::INPUT_FEATURE_NO_INPUT_CHANNEL;
                    const bool canReceiveInput =
                            !(info->layoutParamsFlags & InputWindowInfo::FLAG_NOT_TOUCHABLE) ||
                            !(info->layoutParamsFlags & InputWindowInfo::FLAG_NOT_FOCUSABLE);
                    if (canReceiveInput && !noInputChannel) {
                        ALOGE("Window handle %s has no registered input channel",
                              handle->getName().c_str());
                    }
                    continue;
                }

                if (info->displayId != displayId) {
                    ALOGE("Window %s updated by wrong display %d, should belong to display %d",
                          handle->getName().c_str(), displayId, info->displayId);
                    continue;
                }

                if (oldHandlesByTokens.find(handle->getToken()) != oldHandlesByTokens.end()) {
                    const sp<InputWindowHandle> oldHandle =
                            oldHandlesByTokens.at(handle->getToken());
                    oldHandle->updateFrom(handle);
                    newHandles.push_back(oldHandle);
                } else {
                    newHandles.push_back(handle);
                }
            }

            for (const sp<InputWindowHandle>& windowHandle : newHandles) {
                // Set newFocusedWindowHandle to the top most focused window instead of the last one
                if (!newFocusedWindowHandle && windowHandle->getInfo()->hasFocus
                        && windowHandle->getInfo()->visible) {
                    newFocusedWindowHandle = windowHandle;
                }
                if (windowHandle == mLastHoverWindowHandle) {
                    foundHoveredWindow = true;
                }
            }

            // Insert or replace
            mWindowHandlesByDisplay[displayId] = newHandles;
        }

        if (!foundHoveredWindow) {
            mLastHoverWindowHandle = nullptr;
        }

        sp<InputWindowHandle> oldFocusedWindowHandle =
                getValueByKey(mFocusedWindowHandlesByDisplay, displayId);

        if (oldFocusedWindowHandle != newFocusedWindowHandle) {
            if (oldFocusedWindowHandle != nullptr) {
#if DEBUG_FOCUS
                ALOGD("Focus left window: %s in display %" PRId32,
                        oldFocusedWindowHandle->getName().c_str(), displayId);
#endif
                sp<InputChannel> focusedInputChannel = getInputChannelLocked(
                        oldFocusedWindowHandle->getToken());
                if (focusedInputChannel != nullptr) {
                    CancelationOptions options(CancelationOptions::CANCEL_NON_POINTER_EVENTS,
                            "focus left window");
                    synthesizeCancelationEventsForInputChannelLocked(
                            focusedInputChannel, options);
                }
                mFocusedWindowHandlesByDisplay.erase(displayId);
            }
            if (newFocusedWindowHandle != nullptr) {
#if DEBUG_FOCUS
                ALOGD("Focus entered window: %s in display %" PRId32,
                        newFocusedWindowHandle->getName().c_str(), displayId);
#endif
                mFocusedWindowHandlesByDisplay[displayId] = newFocusedWindowHandle;
            }

            if (mFocusedDisplayId == displayId) {
                onFocusChangedLocked(oldFocusedWindowHandle, newFocusedWindowHandle);
            }

        }

        ssize_t stateIndex = mTouchStatesByDisplay.indexOfKey(displayId);
        if (stateIndex >= 0) {
            TouchState& state = mTouchStatesByDisplay.editValueAt(stateIndex);
            for (size_t i = 0; i < state.windows.size(); ) {
                TouchedWindow& touchedWindow = state.windows[i];
                if (!hasWindowHandleLocked(touchedWindow.windowHandle)) {
#if DEBUG_FOCUS
                    ALOGD("Touched window was removed: %s in display %" PRId32,
                            touchedWindow.windowHandle->getName().c_str(), displayId);
#endif
                    sp<InputChannel> touchedInputChannel =
                            getInputChannelLocked(touchedWindow.windowHandle->getToken());
                    if (touchedInputChannel != nullptr) {
                        CancelationOptions options(CancelationOptions::CANCEL_POINTER_EVENTS,
                                "touched window was removed");
                        synthesizeCancelationEventsForInputChannelLocked(
                                touchedInputChannel, options);
                    }
                    state.windows.erase(state.windows.begin() + i);
                } else {
                  ++i;
                }
            }
        }

        // Release information for windows that are no longer present.
        // This ensures that unused input channels are released promptly.
        // Otherwise, they might stick around until the window handle is destroyed
        // which might not happen until the next GC.
        for (const sp<InputWindowHandle>& oldWindowHandle : oldWindowHandles) {
            if (!hasWindowHandleLocked(oldWindowHandle)) {
#if DEBUG_FOCUS
                ALOGD("Window went away: %s", oldWindowHandle->getName().c_str());
#endif
                oldWindowHandle->releaseChannel();
            }
        }
    } // release lock

    // Wake up poll loop since it may need to make new input dispatching choices.
    mLooper->wake();

    if (setInputWindowsListener) {
        setInputWindowsListener->onSetInputWindowsFinished();
    }
}

void InputDispatcher::setFocusedApplication(
        int32_t displayId, const sp<InputApplicationHandle>& inputApplicationHandle) {
#if DEBUG_FOCUS
    ALOGD("setFocusedApplication displayId=%" PRId32, displayId);
#endif
    { // acquire lock
        std::scoped_lock _l(mLock);

        sp<InputApplicationHandle> oldFocusedApplicationHandle =
                getValueByKey(mFocusedApplicationHandlesByDisplay, displayId);
        if (inputApplicationHandle != nullptr && inputApplicationHandle->updateInfo()) {
            if (oldFocusedApplicationHandle != inputApplicationHandle) {
                if (oldFocusedApplicationHandle != nullptr) {
                    resetANRTimeoutsLocked();
                }
                mFocusedApplicationHandlesByDisplay[displayId] = inputApplicationHandle;
            }
        } else if (oldFocusedApplicationHandle != nullptr) {
            resetANRTimeoutsLocked();
            oldFocusedApplicationHandle.clear();
            mFocusedApplicationHandlesByDisplay.erase(displayId);
        }

#if DEBUG_FOCUS
        //logDispatchStateLocked();
#endif
    } // release lock

    // Wake up poll loop since it may need to make new input dispatching choices.
    mLooper->wake();
}

/**
 * Sets the focused display, which is responsible for receiving focus-dispatched input events where
 * the display not specified.
 *
 * We track any unreleased events for each window. If a window loses the ability to receive the
 * released event, we will send a cancel event to it. So when the focused display is changed, we
 * cancel all the unreleased display-unspecified events for the focused window on the old focused
 * display. The display-specified events won't be affected.
 */
void InputDispatcher::setFocusedDisplay(int32_t displayId) {
#if DEBUG_FOCUS
    ALOGD("setFocusedDisplay displayId=%" PRId32, displayId);
#endif
    { // acquire lock
        std::scoped_lock _l(mLock);

        if (mFocusedDisplayId != displayId) {
            sp<InputWindowHandle> oldFocusedWindowHandle =
                    getValueByKey(mFocusedWindowHandlesByDisplay, mFocusedDisplayId);
            if (oldFocusedWindowHandle != nullptr) {
                sp<InputChannel> inputChannel =
                    getInputChannelLocked(oldFocusedWindowHandle->getToken());
                if (inputChannel != nullptr) {
                    CancelationOptions options(
                            CancelationOptions::CANCEL_NON_POINTER_EVENTS,
                            "The display which contains this window no longer has focus.");
                    options.displayId = ADISPLAY_ID_NONE;
                    synthesizeCancelationEventsForInputChannelLocked(inputChannel, options);
                }
            }
            mFocusedDisplayId = displayId;

            // Sanity check
            sp<InputWindowHandle> newFocusedWindowHandle =
                    getValueByKey(mFocusedWindowHandlesByDisplay, displayId);
            onFocusChangedLocked(oldFocusedWindowHandle, newFocusedWindowHandle);

            if (newFocusedWindowHandle == nullptr) {
                ALOGW("Focused display #%" PRId32 " does not have a focused window.", displayId);
                if (!mFocusedWindowHandlesByDisplay.empty()) {
                    ALOGE("But another display has a focused window:");
                    for (auto& it : mFocusedWindowHandlesByDisplay) {
                        const int32_t displayId = it.first;
                        const sp<InputWindowHandle>& windowHandle = it.second;
                        ALOGE("Display #%" PRId32 " has focused window: '%s'\n",
                                displayId, windowHandle->getName().c_str());
                    }
                }
            }
        }

#if DEBUG_FOCUS
        logDispatchStateLocked();
#endif
    } // release lock

    // Wake up poll loop since it may need to make new input dispatching choices.
    mLooper->wake();
}

void InputDispatcher::setInputDispatchMode(bool enabled, bool frozen) {
#if DEBUG_FOCUS
    ALOGD("setInputDispatchMode: enabled=%d, frozen=%d", enabled, frozen);
#endif

    bool changed;
    { // acquire lock
        std::scoped_lock _l(mLock);

        if (mDispatchEnabled != enabled || mDispatchFrozen != frozen) {
            if (mDispatchFrozen && !frozen) {
                resetANRTimeoutsLocked();
            }

            if (mDispatchEnabled && !enabled) {
                resetAndDropEverythingLocked("dispatcher is being disabled");
            }

            mDispatchEnabled = enabled;
            mDispatchFrozen = frozen;
            changed = true;
        } else {
            changed = false;
        }

#if DEBUG_FOCUS
        logDispatchStateLocked();
#endif
    } // release lock

    if (changed) {
        // Wake up poll loop since it may need to make new input dispatching choices.
        mLooper->wake();
    }
}

void InputDispatcher::setInputFilterEnabled(bool enabled) {
#if DEBUG_FOCUS
    ALOGD("setInputFilterEnabled: enabled=%d", enabled);
#endif

    { // acquire lock
        std::scoped_lock _l(mLock);

        if (mInputFilterEnabled == enabled) {
            return;
        }

        mInputFilterEnabled = enabled;
        resetAndDropEverythingLocked("input filter is being enabled or disabled");
    } // release lock

    // Wake up poll loop since there might be work to do to drop everything.
    mLooper->wake();
}

bool InputDispatcher::transferTouchFocus(const sp<IBinder>& fromToken, const sp<IBinder>& toToken) {
    if (fromToken == toToken) {
#if DEBUG_FOCUS
        ALOGD("Trivial transfer to same window.");
#endif
        return true;
    }

    { // acquire lock
        std::scoped_lock _l(mLock);

        sp<InputWindowHandle> fromWindowHandle = getWindowHandleLocked(fromToken);
        sp<InputWindowHandle> toWindowHandle = getWindowHandleLocked(toToken);
        if (fromWindowHandle == nullptr || toWindowHandle == nullptr) {
            ALOGW("Cannot transfer focus because from or to window not found.");
            return false;
        }
#if DEBUG_FOCUS
        ALOGD("transferTouchFocus: fromWindowHandle=%s, toWindowHandle=%s",
            fromWindowHandle->getName().c_str(), toWindowHandle->getName().c_str());
#endif
        if (fromWindowHandle->getInfo()->displayId != toWindowHandle->getInfo()->displayId) {
#if DEBUG_FOCUS
            ALOGD("Cannot transfer focus because windows are on different displays.");
#endif
            return false;
        }

        bool found = false;
        for (size_t d = 0; d < mTouchStatesByDisplay.size(); d++) {
            TouchState& state = mTouchStatesByDisplay.editValueAt(d);
            for (size_t i = 0; i < state.windows.size(); i++) {
                const TouchedWindow& touchedWindow = state.windows[i];
                if (touchedWindow.windowHandle == fromWindowHandle) {
                    int32_t oldTargetFlags = touchedWindow.targetFlags;
                    BitSet32 pointerIds = touchedWindow.pointerIds;

                    state.windows.erase(state.windows.begin() + i);

                    int32_t newTargetFlags = oldTargetFlags
                            & (InputTarget::FLAG_FOREGROUND
                                    | InputTarget::FLAG_SPLIT | InputTarget::FLAG_DISPATCH_AS_IS);
                    state.addOrUpdateWindow(toWindowHandle, newTargetFlags, pointerIds);

                    found = true;
                    goto Found;
                }
            }
        }
Found:

        if (! found) {
#if DEBUG_FOCUS
            ALOGD("Focus transfer failed because from window did not have focus.");
#endif
            return false;
        }


        sp<InputChannel> fromChannel = getInputChannelLocked(fromToken);
        sp<InputChannel> toChannel = getInputChannelLocked(toToken);
        ssize_t fromConnectionIndex = getConnectionIndexLocked(fromChannel);
        ssize_t toConnectionIndex = getConnectionIndexLocked(toChannel);
        if (fromConnectionIndex >= 0 && toConnectionIndex >= 0) {
            sp<Connection> fromConnection = mConnectionsByFd.valueAt(fromConnectionIndex);
            sp<Connection> toConnection = mConnectionsByFd.valueAt(toConnectionIndex);

            fromConnection->inputState.copyPointerStateTo(toConnection->inputState);
            CancelationOptions options(CancelationOptions::CANCEL_POINTER_EVENTS,
                    "transferring touch focus from this window to another window");
            synthesizeCancelationEventsForConnectionLocked(fromConnection, options);
        }

#if DEBUG_FOCUS
        logDispatchStateLocked();
#endif
    } // release lock

    // Wake up poll loop since it may need to make new input dispatching choices.
    mLooper->wake();
    return true;
}

void InputDispatcher::resetAndDropEverythingLocked(const char* reason) {
#if DEBUG_FOCUS
    ALOGD("Resetting and dropping all events (%s).", reason);
#endif

    CancelationOptions options(CancelationOptions::CANCEL_ALL_EVENTS, reason);
    synthesizeCancelationEventsForAllConnectionsLocked(options);

    resetKeyRepeatLocked();
    releasePendingEventLocked();
    drainInboundQueueLocked();
    resetANRTimeoutsLocked();

    mTouchStatesByDisplay.clear();
    mLastHoverWindowHandle.clear();
    mReplacedKeys.clear();
}

void InputDispatcher::logDispatchStateLocked() {
    std::string dump;
    dumpDispatchStateLocked(dump);

    std::istringstream stream(dump);
    std::string line;

    while (std::getline(stream, line, '\n')) {
        ALOGD("%s", line.c_str());
    }
}

void InputDispatcher::dumpDispatchStateLocked(std::string& dump) {
    dump += StringPrintf(INDENT "DispatchEnabled: %s\n", toString(mDispatchEnabled));
    dump += StringPrintf(INDENT "DispatchFrozen: %s\n", toString(mDispatchFrozen));
    dump += StringPrintf(INDENT "InputFilterEnabled: %s\n", toString(mInputFilterEnabled));
    dump += StringPrintf(INDENT "FocusedDisplayId: %" PRId32 "\n", mFocusedDisplayId);

    if (!mFocusedApplicationHandlesByDisplay.empty()) {
        dump += StringPrintf(INDENT "FocusedApplications:\n");
        for (auto& it : mFocusedApplicationHandlesByDisplay) {
            const int32_t displayId = it.first;
            const sp<InputApplicationHandle>& applicationHandle = it.second;
            dump += StringPrintf(
                    INDENT2 "displayId=%" PRId32 ", name='%s', dispatchingTimeout=%0.3fms\n",
                    displayId,
                    applicationHandle->getName().c_str(),
                    applicationHandle->getDispatchingTimeout(
                            DEFAULT_INPUT_DISPATCHING_TIMEOUT) / 1000000.0);
        }
    } else {
        dump += StringPrintf(INDENT "FocusedApplications: <none>\n");
    }

    if (!mFocusedWindowHandlesByDisplay.empty()) {
        dump += StringPrintf(INDENT "FocusedWindows:\n");
        for (auto& it : mFocusedWindowHandlesByDisplay) {
            const int32_t displayId = it.first;
            const sp<InputWindowHandle>& windowHandle = it.second;
            dump += StringPrintf(INDENT2 "displayId=%" PRId32 ", name='%s'\n",
                    displayId, windowHandle->getName().c_str());
        }
    } else {
        dump += StringPrintf(INDENT "FocusedWindows: <none>\n");
    }

    if (!mTouchStatesByDisplay.isEmpty()) {
        dump += StringPrintf(INDENT "TouchStatesByDisplay:\n");
        for (size_t i = 0; i < mTouchStatesByDisplay.size(); i++) {
            const TouchState& state = mTouchStatesByDisplay.valueAt(i);
            dump += StringPrintf(INDENT2 "%d: down=%s, split=%s, deviceId=%d, source=0x%08x\n",
                    state.displayId, toString(state.down), toString(state.split),
                    state.deviceId, state.source);
            if (!state.windows.empty()) {
                dump += INDENT3 "Windows:\n";
                for (size_t i = 0; i < state.windows.size(); i++) {
                    const TouchedWindow& touchedWindow = state.windows[i];
                    dump += StringPrintf(INDENT4 "%zu: name='%s', pointerIds=0x%0x, targetFlags=0x%x\n",
                            i, touchedWindow.windowHandle->getName().c_str(),
                            touchedWindow.pointerIds.value,
                            touchedWindow.targetFlags);
                }
            } else {
                dump += INDENT3 "Windows: <none>\n";
            }
            if (!state.portalWindows.empty()) {
                dump += INDENT3 "Portal windows:\n";
                for (size_t i = 0; i < state.portalWindows.size(); i++) {
                    const sp<InputWindowHandle> portalWindowHandle = state.portalWindows[i];
                    dump += StringPrintf(INDENT4 "%zu: name='%s'\n",
                            i, portalWindowHandle->getName().c_str());
                }
            }
        }
    } else {
        dump += INDENT "TouchStates: <no displays touched>\n";
    }

    if (!mWindowHandlesByDisplay.empty()) {
       for (auto& it : mWindowHandlesByDisplay) {
            const std::vector<sp<InputWindowHandle>> windowHandles = it.second;
            dump += StringPrintf(INDENT "Display: %" PRId32 "\n", it.first);
            if (!windowHandles.empty()) {
                dump += INDENT2 "Windows:\n";
                for (size_t i = 0; i < windowHandles.size(); i++) {
                    const sp<InputWindowHandle>& windowHandle = windowHandles[i];
                    const InputWindowInfo* windowInfo = windowHandle->getInfo();

                    dump += StringPrintf(INDENT3 "%zu: name='%s', displayId=%d, "
                            "portalToDisplayId=%d, paused=%s, hasFocus=%s, hasWallpaper=%s, "
                            "visible=%s, canReceiveKeys=%s, flags=0x%08x, type=0x%08x, layer=%d, "
                            "frame=[%d,%d][%d,%d], globalScale=%f, windowScale=(%f,%f), "
                            "touchableRegion=",
                            i, windowInfo->name.c_str(), windowInfo->displayId,
                            windowInfo->portalToDisplayId,
                            toString(windowInfo->paused),
                            toString(windowInfo->hasFocus),
                            toString(windowInfo->hasWallpaper),
                            toString(windowInfo->visible),
                            toString(windowInfo->canReceiveKeys),
                            windowInfo->layoutParamsFlags, windowInfo->layoutParamsType,
                            windowInfo->layer,
                            windowInfo->frameLeft, windowInfo->frameTop,
                            windowInfo->frameRight, windowInfo->frameBottom,
                            windowInfo->globalScaleFactor,
                            windowInfo->windowXScale, windowInfo->windowYScale);
                    dumpRegion(dump, windowInfo->touchableRegion);
                    dump += StringPrintf(", inputFeatures=0x%08x", windowInfo->inputFeatures);
                    dump += StringPrintf(", ownerPid=%d, ownerUid=%d, dispatchingTimeout=%0.3fms\n",
                            windowInfo->ownerPid, windowInfo->ownerUid,
                            windowInfo->dispatchingTimeout / 1000000.0);
                }
            } else {
                dump += INDENT2 "Windows: <none>\n";
            }
        }
    } else {
        dump += INDENT "Displays: <none>\n";
    }

    if (!mGlobalMonitorsByDisplay.empty() || !mGestureMonitorsByDisplay.empty()) {
       for (auto& it : mGlobalMonitorsByDisplay) {
            const std::vector<Monitor>& monitors = it.second;
            dump += StringPrintf(INDENT "Global monitors in display %" PRId32 ":\n", it.first);
            dumpMonitors(dump, monitors);
       }
       for (auto& it : mGestureMonitorsByDisplay) {
            const std::vector<Monitor>& monitors = it.second;
            dump += StringPrintf(INDENT "Gesture monitors in display %" PRId32 ":\n", it.first);
            dumpMonitors(dump, monitors);
       }
    } else {
        dump += INDENT "Monitors: <none>\n";
    }

    nsecs_t currentTime = now();

    // Dump recently dispatched or dropped events from oldest to newest.
    if (!mRecentQueue.isEmpty()) {
        dump += StringPrintf(INDENT "RecentQueue: length=%u\n", mRecentQueue.count());
        for (EventEntry* entry = mRecentQueue.head; entry; entry = entry->next) {
            dump += INDENT2;
            entry->appendDescription(dump);
            dump += StringPrintf(", age=%0.1fms\n",
                    (currentTime - entry->eventTime) * 0.000001f);
        }
    } else {
        dump += INDENT "RecentQueue: <empty>\n";
    }

    // Dump event currently being dispatched.
    if (mPendingEvent) {
        dump += INDENT "PendingEvent:\n";
        dump += INDENT2;
        mPendingEvent->appendDescription(dump);
        dump += StringPrintf(", age=%0.1fms\n",
                (currentTime - mPendingEvent->eventTime) * 0.000001f);
    } else {
        dump += INDENT "PendingEvent: <none>\n";
    }

    // Dump inbound events from oldest to newest.
    if (!mInboundQueue.isEmpty()) {
        dump += StringPrintf(INDENT "InboundQueue: length=%u\n", mInboundQueue.count());
        for (EventEntry* entry = mInboundQueue.head; entry; entry = entry->next) {
            dump += INDENT2;
            entry->appendDescription(dump);
            dump += StringPrintf(", age=%0.1fms\n",
                    (currentTime - entry->eventTime) * 0.000001f);
        }
    } else {
        dump += INDENT "InboundQueue: <empty>\n";
    }

    if (!mReplacedKeys.isEmpty()) {
        dump += INDENT "ReplacedKeys:\n";
        for (size_t i = 0; i < mReplacedKeys.size(); i++) {
            const KeyReplacement& replacement = mReplacedKeys.keyAt(i);
            int32_t newKeyCode = mReplacedKeys.valueAt(i);
            dump += StringPrintf(INDENT2 "%zu: originalKeyCode=%d, deviceId=%d, newKeyCode=%d\n",
                    i, replacement.keyCode, replacement.deviceId, newKeyCode);
        }
    } else {
        dump += INDENT "ReplacedKeys: <empty>\n";
    }

    if (!mConnectionsByFd.isEmpty()) {
        dump += INDENT "Connections:\n";
        for (size_t i = 0; i < mConnectionsByFd.size(); i++) {
            const sp<Connection>& connection = mConnectionsByFd.valueAt(i);
            dump += StringPrintf(INDENT2 "%zu: channelName='%s', windowName='%s', "
                    "status=%s, monitor=%s, inputPublisherBlocked=%s\n",
                    i, connection->getInputChannelName().c_str(),
                    connection->getWindowName().c_str(),
                    connection->getStatusLabel(), toString(connection->monitor),
                    toString(connection->inputPublisherBlocked));

            if (!connection->outboundQueue.isEmpty()) {
                dump += StringPrintf(INDENT3 "OutboundQueue: length=%u\n",
                        connection->outboundQueue.count());
                for (DispatchEntry* entry = connection->outboundQueue.head; entry;
                        entry = entry->next) {
                    dump.append(INDENT4);
                    entry->eventEntry->appendDescription(dump);
                    dump += StringPrintf(", targetFlags=0x%08x, resolvedAction=%d, age=%0.1fms\n",
                            entry->targetFlags, entry->resolvedAction,
                            (currentTime - entry->eventEntry->eventTime) * 0.000001f);
                }
            } else {
                dump += INDENT3 "OutboundQueue: <empty>\n";
            }

            if (!connection->waitQueue.isEmpty()) {
                dump += StringPrintf(INDENT3 "WaitQueue: length=%u\n",
                        connection->waitQueue.count());
                for (DispatchEntry* entry = connection->waitQueue.head; entry;
                        entry = entry->next) {
                    dump += INDENT4;
                    entry->eventEntry->appendDescription(dump);
                    dump += StringPrintf(", targetFlags=0x%08x, resolvedAction=%d, "
                            "age=%0.1fms, wait=%0.1fms\n",
                            entry->targetFlags, entry->resolvedAction,
                            (currentTime - entry->eventEntry->eventTime) * 0.000001f,
                            (currentTime - entry->deliveryTime) * 0.000001f);
                }
            } else {
                dump += INDENT3 "WaitQueue: <empty>\n";
            }
        }
    } else {
        dump += INDENT "Connections: <none>\n";
    }

    if (isAppSwitchPendingLocked()) {
        dump += StringPrintf(INDENT "AppSwitch: pending, due in %0.1fms\n",
                (mAppSwitchDueTime - now()) / 1000000.0);
    } else {
        dump += INDENT "AppSwitch: not pending\n";
    }

    dump += INDENT "Configuration:\n";
    dump += StringPrintf(INDENT2 "KeyRepeatDelay: %0.1fms\n",
            mConfig.keyRepeatDelay * 0.000001f);
    dump += StringPrintf(INDENT2 "KeyRepeatTimeout: %0.1fms\n",
            mConfig.keyRepeatTimeout * 0.000001f);
}

void InputDispatcher::dumpMonitors(std::string& dump, const std::vector<Monitor>& monitors) {
    const size_t numMonitors = monitors.size();
    for (size_t i = 0; i < numMonitors; i++) {
        const Monitor& monitor = monitors[i];
        const sp<InputChannel>& channel = monitor.inputChannel;
        dump += StringPrintf(INDENT2 "%zu: '%s', ", i, channel->getName().c_str());
        dump += "\n";
    }
}

status_t InputDispatcher::registerInputChannel(const sp<InputChannel>& inputChannel,
        int32_t displayId) {
#if DEBUG_REGISTRATION
    ALOGD("channel '%s' ~ registerInputChannel - displayId=%" PRId32,
            inputChannel->getName().c_str(), displayId);
#endif

    { // acquire lock
        std::scoped_lock _l(mLock);

        if (getConnectionIndexLocked(inputChannel) >= 0) {
            ALOGW("Attempted to register already registered input channel '%s'",
                    inputChannel->getName().c_str());
            return BAD_VALUE;
        }

        sp<Connection> connection = new Connection(inputChannel, false /*monitor*/);

        int fd = inputChannel->getFd();
        mConnectionsByFd.add(fd, connection);
        mInputChannelsByToken[inputChannel->getToken()] = inputChannel;

        mLooper->addFd(fd, 0, ALOOPER_EVENT_INPUT, handleReceiveCallback, this);
    } // release lock

    // Wake the looper because some connections have changed.
    mLooper->wake();
    return OK;
}

status_t InputDispatcher::registerInputMonitor(const sp<InputChannel>& inputChannel,
        int32_t displayId, bool isGestureMonitor) {
    { // acquire lock
        std::scoped_lock _l(mLock);

        if (displayId < 0) {
            ALOGW("Attempted to register input monitor without a specified display.");
            return BAD_VALUE;
        }

        if (inputChannel->getToken() == nullptr) {
            ALOGW("Attempted to register input monitor without an identifying token.");
            return BAD_VALUE;
        }

        sp<Connection> connection = new Connection(inputChannel, true /*monitor*/);

        const int fd = inputChannel->getFd();
        mConnectionsByFd.add(fd, connection);
        mInputChannelsByToken[inputChannel->getToken()] = inputChannel;

        auto& monitorsByDisplay = isGestureMonitor
                ? mGestureMonitorsByDisplay
                : mGlobalMonitorsByDisplay;
        monitorsByDisplay[displayId].emplace_back(inputChannel);

        mLooper->addFd(fd, 0, ALOOPER_EVENT_INPUT, handleReceiveCallback, this);

    }
    // Wake the looper because some connections have changed.
    mLooper->wake();
    return OK;
}

status_t InputDispatcher::unregisterInputChannel(const sp<InputChannel>& inputChannel) {
#if DEBUG_REGISTRATION
    ALOGD("channel '%s' ~ unregisterInputChannel", inputChannel->getName().c_str());
#endif

    { // acquire lock
        std::scoped_lock _l(mLock);

        status_t status = unregisterInputChannelLocked(inputChannel, false /*notify*/);
        if (status) {
            return status;
        }
    } // release lock

    // Wake the poll loop because removing the connection may have changed the current
    // synchronization state.
    mLooper->wake();
    return OK;
}

status_t InputDispatcher::unregisterInputChannelLocked(const sp<InputChannel>& inputChannel,
        bool notify) {
    ssize_t connectionIndex = getConnectionIndexLocked(inputChannel);
    if (connectionIndex < 0) {
        ALOGW("Attempted to unregister already unregistered input channel '%s'",
                inputChannel->getName().c_str());
        return BAD_VALUE;
    }

    sp<Connection> connection = mConnectionsByFd.valueAt(connectionIndex);
    mConnectionsByFd.removeItemsAt(connectionIndex);

    mInputChannelsByToken.erase(inputChannel->getToken());

    if (connection->monitor) {
        removeMonitorChannelLocked(inputChannel);
    }

    mLooper->removeFd(inputChannel->getFd());

    nsecs_t currentTime = now();
    abortBrokenDispatchCycleLocked(currentTime, connection, notify);

    connection->status = Connection::STATUS_ZOMBIE;
    return OK;
}

void InputDispatcher::removeMonitorChannelLocked(const sp<InputChannel>& inputChannel) {
    removeMonitorChannelLocked(inputChannel, mGlobalMonitorsByDisplay);
    removeMonitorChannelLocked(inputChannel, mGestureMonitorsByDisplay);
}

void InputDispatcher::removeMonitorChannelLocked(const sp<InputChannel>& inputChannel,
        std::unordered_map<int32_t, std::vector<Monitor>>& monitorsByDisplay) {
    for (auto it = monitorsByDisplay.begin(); it != monitorsByDisplay.end(); ) {
        std::vector<Monitor>& monitors = it->second;
        const size_t numMonitors = monitors.size();
        for (size_t i = 0; i < numMonitors; i++) {
             if (monitors[i].inputChannel == inputChannel) {
                 monitors.erase(monitors.begin() + i);
                 break;
             }
        }
        if (monitors.empty()) {
            it = monitorsByDisplay.erase(it);
        } else {
            ++it;
        }
    }
}

status_t InputDispatcher::pilferPointers(const sp<IBinder>& token) {
    { // acquire lock
        std::scoped_lock _l(mLock);
        std::optional<int32_t> foundDisplayId = findGestureMonitorDisplayByTokenLocked(token);

        if (!foundDisplayId) {
            ALOGW("Attempted to pilfer pointers from an un-registered monitor or invalid token");
            return BAD_VALUE;
        }
        int32_t displayId = foundDisplayId.value();

        ssize_t stateIndex = mTouchStatesByDisplay.indexOfKey(displayId);
        if (stateIndex < 0) {
            ALOGW("Failed to pilfer pointers: no pointers on display %" PRId32 ".", displayId);
            return BAD_VALUE;
        }

        TouchState& state = mTouchStatesByDisplay.editValueAt(stateIndex);
        std::optional<int32_t> foundDeviceId;
        for (const TouchedMonitor& touchedMonitor : state.gestureMonitors) {
            if (touchedMonitor.monitor.inputChannel->getToken() == token) {
                foundDeviceId = state.deviceId;
            }
        }
        if (!foundDeviceId || !state.down) {
            ALOGW("Attempted to pilfer points from a monitor without any on-going pointer streams."
                    " Ignoring.");
            return BAD_VALUE;
        }
        int32_t deviceId = foundDeviceId.value();

        // Send cancel events to all the input channels we're stealing from.
        CancelationOptions options(CancelationOptions::CANCEL_POINTER_EVENTS,
                "gesture monitor stole pointer stream");
        options.deviceId = deviceId;
        options.displayId = displayId;
        for (const TouchedWindow& window : state.windows) {
            sp<InputChannel> channel = getInputChannelLocked(window.windowHandle->getToken());
            synthesizeCancelationEventsForInputChannelLocked(channel, options);
        }
        // Then clear the current touch state so we stop dispatching to them as well.
        state.filterNonMonitors();
    }
    return OK;
}


std::optional<int32_t> InputDispatcher::findGestureMonitorDisplayByTokenLocked(
        const sp<IBinder>& token) {
    for (const auto& it : mGestureMonitorsByDisplay) {
        const std::vector<Monitor>& monitors = it.second;
        for (const Monitor& monitor : monitors) {
            if (monitor.inputChannel->getToken() == token) {
                return it.first;
            }
        }
    }
    return std::nullopt;
}

ssize_t InputDispatcher::getConnectionIndexLocked(const sp<InputChannel>& inputChannel) {
    if (inputChannel == nullptr) {
        return -1;
    }

    for (size_t i = 0; i < mConnectionsByFd.size(); i++) {
        sp<Connection> connection = mConnectionsByFd.valueAt(i);
        if (connection->inputChannel->getToken() == inputChannel->getToken()) {
            return i;
        }
    }

    return -1;
}

void InputDispatcher::onDispatchCycleFinishedLocked(
        nsecs_t currentTime, const sp<Connection>& connection, uint32_t seq, bool handled) {
    CommandEntry* commandEntry = postCommandLocked(
            & InputDispatcher::doDispatchCycleFinishedLockedInterruptible);
    commandEntry->connection = connection;
    commandEntry->eventTime = currentTime;
    commandEntry->seq = seq;
    commandEntry->handled = handled;
}

void InputDispatcher::onDispatchCycleBrokenLocked(
        nsecs_t currentTime, const sp<Connection>& connection) {
    ALOGE("channel '%s' ~ Channel is unrecoverably broken and will be disposed!",
            connection->getInputChannelName().c_str());

    CommandEntry* commandEntry = postCommandLocked(
            & InputDispatcher::doNotifyInputChannelBrokenLockedInterruptible);
    commandEntry->connection = connection;
}

void InputDispatcher::onFocusChangedLocked(const sp<InputWindowHandle>& oldFocus,
        const sp<InputWindowHandle>& newFocus) {
    sp<IBinder> oldToken = oldFocus != nullptr ? oldFocus->getToken() : nullptr;
    sp<IBinder> newToken = newFocus != nullptr ? newFocus->getToken() : nullptr;
    CommandEntry* commandEntry = postCommandLocked(
            & InputDispatcher::doNotifyFocusChangedLockedInterruptible);
    commandEntry->oldToken = oldToken;
    commandEntry->newToken = newToken;
}

void InputDispatcher::onANRLocked(
        nsecs_t currentTime, const sp<InputApplicationHandle>& applicationHandle,
        const sp<InputWindowHandle>& windowHandle,
        nsecs_t eventTime, nsecs_t waitStartTime, const char* reason) {
    float dispatchLatency = (currentTime - eventTime) * 0.000001f;
    float waitDuration = (currentTime - waitStartTime) * 0.000001f;
    ALOGI("Application is not responding: %s.  "
            "It has been %0.1fms since event, %0.1fms since wait started.  Reason: %s",
            getApplicationWindowLabel(applicationHandle, windowHandle).c_str(),
            dispatchLatency, waitDuration, reason);

    // Capture a record of the InputDispatcher state at the time of the ANR.
    time_t t = time(nullptr);
    struct tm tm;
    localtime_r(&t, &tm);
    char timestr[64];
    strftime(timestr, sizeof(timestr), "%F %T", &tm);
    mLastANRState.clear();
    mLastANRState += INDENT "ANR:\n";
    mLastANRState += StringPrintf(INDENT2 "Time: %s\n", timestr);
    mLastANRState += StringPrintf(INDENT2 "Window: %s\n",
            getApplicationWindowLabel(applicationHandle, windowHandle).c_str());
    mLastANRState += StringPrintf(INDENT2 "DispatchLatency: %0.1fms\n", dispatchLatency);
    mLastANRState += StringPrintf(INDENT2 "WaitDuration: %0.1fms\n", waitDuration);
    mLastANRState += StringPrintf(INDENT2 "Reason: %s\n", reason);
    dumpDispatchStateLocked(mLastANRState);

    CommandEntry* commandEntry = postCommandLocked(
            & InputDispatcher::doNotifyANRLockedInterruptible);
    commandEntry->inputApplicationHandle = applicationHandle;
    commandEntry->inputChannel = windowHandle != nullptr ?
            getInputChannelLocked(windowHandle->getToken()) : nullptr;
    commandEntry->reason = reason;
}

void InputDispatcher::doNotifyConfigurationChangedLockedInterruptible (
        CommandEntry* commandEntry) {
    mLock.unlock();

    mPolicy->notifyConfigurationChanged(commandEntry->eventTime);

    mLock.lock();
}

void InputDispatcher::doNotifyInputChannelBrokenLockedInterruptible(
        CommandEntry* commandEntry) {
    sp<Connection> connection = commandEntry->connection;

    if (connection->status != Connection::STATUS_ZOMBIE) {
        mLock.unlock();

        mPolicy->notifyInputChannelBroken(connection->inputChannel->getToken());

        mLock.lock();
    }
}

void InputDispatcher::doNotifyFocusChangedLockedInterruptible(
        CommandEntry* commandEntry) {
    sp<IBinder> oldToken = commandEntry->oldToken;
    sp<IBinder> newToken = commandEntry->newToken;
    mLock.unlock();
    mPolicy->notifyFocusChanged(oldToken, newToken);
    mLock.lock();
}

void InputDispatcher::doNotifyANRLockedInterruptible(
        CommandEntry* commandEntry) {
    mLock.unlock();

    nsecs_t newTimeout = mPolicy->notifyANR(
            commandEntry->inputApplicationHandle,
            commandEntry->inputChannel ? commandEntry->inputChannel->getToken() : nullptr,
            commandEntry->reason);

    mLock.lock();

    resumeAfterTargetsNotReadyTimeoutLocked(newTimeout,
            commandEntry->inputChannel);
}

void InputDispatcher::doInterceptKeyBeforeDispatchingLockedInterruptible(
        CommandEntry* commandEntry) {
    KeyEntry* entry = commandEntry->keyEntry;

    KeyEvent event;
    initializeKeyEvent(&event, entry);

    mLock.unlock();

    android::base::Timer t;
    sp<IBinder> token = commandEntry->inputChannel != nullptr ?
        commandEntry->inputChannel->getToken() : nullptr;
    nsecs_t delay = mPolicy->interceptKeyBeforeDispatching(token,
            &event, entry->policyFlags);
    if (t.duration() > SLOW_INTERCEPTION_THRESHOLD) {
        ALOGW("Excessive delay in interceptKeyBeforeDispatching; took %s ms",
                std::to_string(t.duration().count()).c_str());
    }

    mLock.lock();

    if (delay < 0) {
        entry->interceptKeyResult = KeyEntry::INTERCEPT_KEY_RESULT_SKIP;
    } else if (!delay) {
        entry->interceptKeyResult = KeyEntry::INTERCEPT_KEY_RESULT_CONTINUE;
    } else {
        entry->interceptKeyResult = KeyEntry::INTERCEPT_KEY_RESULT_TRY_AGAIN_LATER;
        entry->interceptKeyWakeupTime = now() + delay;
    }
    entry->release();
}

void InputDispatcher::doOnPointerDownOutsideFocusLockedInterruptible(CommandEntry* commandEntry) {
    mLock.unlock();
    mPolicy->onPointerDownOutsideFocus(commandEntry->newToken);
    mLock.lock();
}

void InputDispatcher::doDispatchCycleFinishedLockedInterruptible(
        CommandEntry* commandEntry) {
    sp<Connection> connection = commandEntry->connection;
    nsecs_t finishTime = commandEntry->eventTime;
    uint32_t seq = commandEntry->seq;
    bool handled = commandEntry->handled;

    // Handle post-event policy actions.
    DispatchEntry* dispatchEntry = connection->findWaitQueueEntry(seq);
    if (dispatchEntry) {
        nsecs_t eventDuration = finishTime - dispatchEntry->deliveryTime;
        if (eventDuration > SLOW_EVENT_PROCESSING_WARNING_TIMEOUT) {
            std::string msg =
                    StringPrintf("Window '%s' spent %0.1fms processing the last input event: ",
                    connection->getWindowName().c_str(), eventDuration * 0.000001f);
            dispatchEntry->eventEntry->appendDescription(msg);
            ALOGI("%s", msg.c_str());
        }

        bool restartEvent;
        if (dispatchEntry->eventEntry->type == EventEntry::TYPE_KEY) {
            KeyEntry* keyEntry = static_cast<KeyEntry*>(dispatchEntry->eventEntry);
            restartEvent = afterKeyEventLockedInterruptible(connection,
                    dispatchEntry, keyEntry, handled);
        } else if (dispatchEntry->eventEntry->type == EventEntry::TYPE_MOTION) {
            MotionEntry* motionEntry = static_cast<MotionEntry*>(dispatchEntry->eventEntry);
            restartEvent = afterMotionEventLockedInterruptible(connection,
                    dispatchEntry, motionEntry, handled);
        } else {
            restartEvent = false;
        }

        // Dequeue the event and start the next cycle.
        // Note that because the lock might have been released, it is possible that the
        // contents of the wait queue to have been drained, so we need to double-check
        // a few things.
        if (dispatchEntry == connection->findWaitQueueEntry(seq)) {
            connection->waitQueue.dequeue(dispatchEntry);
            traceWaitQueueLength(connection);
            if (restartEvent && connection->status == Connection::STATUS_NORMAL) {
                connection->outboundQueue.enqueueAtHead(dispatchEntry);
                traceOutboundQueueLength(connection);
            } else {
                releaseDispatchEntry(dispatchEntry);
            }
        }

        // Start the next dispatch cycle for this connection.
        startDispatchCycleLocked(now(), connection);
    }
}

bool InputDispatcher::afterKeyEventLockedInterruptible(const sp<Connection>& connection,
        DispatchEntry* dispatchEntry, KeyEntry* keyEntry, bool handled) {
    if (keyEntry->flags & AKEY_EVENT_FLAG_FALLBACK) {
        if (!handled) {
            // Report the key as unhandled, since the fallback was not handled.
            mReporter->reportUnhandledKey(keyEntry->sequenceNum);
        }
        return false;
    }

    // Get the fallback key state.
    // Clear it out after dispatching the UP.
    int32_t originalKeyCode = keyEntry->keyCode;
    int32_t fallbackKeyCode = connection->inputState.getFallbackKey(originalKeyCode);
    if (keyEntry->action == AKEY_EVENT_ACTION_UP) {
        connection->inputState.removeFallbackKey(originalKeyCode);
    }

    if (handled || !dispatchEntry->hasForegroundTarget()) {
        // If the application handles the original key for which we previously
        // generated a fallback or if the window is not a foreground window,
        // then cancel the associated fallback key, if any.
        if (fallbackKeyCode != -1) {
            // Dispatch the unhandled key to the policy with the cancel flag.
#if DEBUG_OUTBOUND_EVENT_DETAILS
            ALOGD("Unhandled key event: Asking policy to cancel fallback action.  "
                    "keyCode=%d, action=%d, repeatCount=%d, policyFlags=0x%08x",
                    keyEntry->keyCode, keyEntry->action, keyEntry->repeatCount,
                    keyEntry->policyFlags);
#endif
            KeyEvent event;
            initializeKeyEvent(&event, keyEntry);
            event.setFlags(event.getFlags() | AKEY_EVENT_FLAG_CANCELED);

            mLock.unlock();

            mPolicy->dispatchUnhandledKey(connection->inputChannel->getToken(),
                                          &event, keyEntry->policyFlags, &event);

            mLock.lock();

            // Cancel the fallback key.
            if (fallbackKeyCode != AKEYCODE_UNKNOWN) {
                CancelationOptions options(CancelationOptions::CANCEL_FALLBACK_EVENTS,
                                           "application handled the original non-fallback key "
                                           "or is no longer a foreground target, "
                                           "canceling previously dispatched fallback key");
                options.keyCode = fallbackKeyCode;
                synthesizeCancelationEventsForConnectionLocked(connection, options);
            }
            connection->inputState.removeFallbackKey(originalKeyCode);
        }
    } else {
        // If the application did not handle a non-fallback key, first check
        // that we are in a good state to perform unhandled key event processing
        // Then ask the policy what to do with it.
        bool initialDown = keyEntry->action == AKEY_EVENT_ACTION_DOWN
                && keyEntry->repeatCount == 0;
        if (fallbackKeyCode == -1 && !initialDown) {
#if DEBUG_OUTBOUND_EVENT_DETAILS
            ALOGD("Unhandled key event: Skipping unhandled key event processing "
                    "since this is not an initial down.  "
                    "keyCode=%d, action=%d, repeatCount=%d, policyFlags=0x%08x",
                    originalKeyCode, keyEntry->action, keyEntry->repeatCount,
                    keyEntry->policyFlags);
#endif
            return false;
        }

        // Dispatch the unhandled key to the policy.
#if DEBUG_OUTBOUND_EVENT_DETAILS
        ALOGD("Unhandled key event: Asking policy to perform fallback action.  "
                "keyCode=%d, action=%d, repeatCount=%d, policyFlags=0x%08x",
                keyEntry->keyCode, keyEntry->action, keyEntry->repeatCount,
                keyEntry->policyFlags);
#endif
        KeyEvent event;
        initializeKeyEvent(&event, keyEntry);

        mLock.unlock();

        bool fallback = mPolicy->dispatchUnhandledKey(connection->inputChannel->getToken(),
                                                      &event, keyEntry->policyFlags, &event);

        mLock.lock();

        if (connection->status != Connection::STATUS_NORMAL) {
            connection->inputState.removeFallbackKey(originalKeyCode);
            return false;
        }

        // Latch the fallback keycode for this key on an initial down.
        // The fallback keycode cannot change at any other point in the lifecycle.
        if (initialDown) {
            if (fallback) {
                fallbackKeyCode = event.getKeyCode();
            } else {
                fallbackKeyCode = AKEYCODE_UNKNOWN;
            }
            connection->inputState.setFallbackKey(originalKeyCode, fallbackKeyCode);
        }

        ALOG_ASSERT(fallbackKeyCode != -1);

        // Cancel the fallback key if the policy decides not to send it anymore.
        // We will continue to dispatch the key to the policy but we will no
        // longer dispatch a fallback key to the application.
        if (fallbackKeyCode != AKEYCODE_UNKNOWN
                && (!fallback || fallbackKeyCode != event.getKeyCode())) {
#if DEBUG_OUTBOUND_EVENT_DETAILS
            if (fallback) {
                ALOGD("Unhandled key event: Policy requested to send key %d"
                        "as a fallback for %d, but on the DOWN it had requested "
                        "to send %d instead.  Fallback canceled.",
                        event.getKeyCode(), originalKeyCode, fallbackKeyCode);
            } else {
                ALOGD("Unhandled key event: Policy did not request fallback for %d, "
                        "but on the DOWN it had requested to send %d.  "
                        "Fallback canceled.",
                        originalKeyCode, fallbackKeyCode);
            }
#endif

            CancelationOptions options(CancelationOptions::CANCEL_FALLBACK_EVENTS,
                                       "canceling fallback, policy no longer desires it");
            options.keyCode = fallbackKeyCode;
            synthesizeCancelationEventsForConnectionLocked(connection, options);

            fallback = false;
            fallbackKeyCode = AKEYCODE_UNKNOWN;
            if (keyEntry->action != AKEY_EVENT_ACTION_UP) {
                connection->inputState.setFallbackKey(originalKeyCode,
                                                      fallbackKeyCode);
            }
        }

#if DEBUG_OUTBOUND_EVENT_DETAILS
        {
            std::string msg;
            const KeyedVector<int32_t, int32_t>& fallbackKeys =
                    connection->inputState.getFallbackKeys();
            for (size_t i = 0; i < fallbackKeys.size(); i++) {
                msg += StringPrintf(", %d->%d", fallbackKeys.keyAt(i),
                        fallbackKeys.valueAt(i));
            }
            ALOGD("Unhandled key event: %zu currently tracked fallback keys%s.",
                    fallbackKeys.size(), msg.c_str());
        }
#endif

        if (fallback) {
            // Restart the dispatch cycle using the fallback key.
            keyEntry->eventTime = event.getEventTime();
            keyEntry->deviceId = event.getDeviceId();
            keyEntry->source = event.getSource();
            keyEntry->displayId = event.getDisplayId();
            keyEntry->flags = event.getFlags() | AKEY_EVENT_FLAG_FALLBACK;
            keyEntry->keyCode = fallbackKeyCode;
            keyEntry->scanCode = event.getScanCode();
            keyEntry->metaState = event.getMetaState();
            keyEntry->repeatCount = event.getRepeatCount();
            keyEntry->downTime = event.getDownTime();
            keyEntry->syntheticRepeat = false;

#if DEBUG_OUTBOUND_EVENT_DETAILS
            ALOGD("Unhandled key event: Dispatching fallback key.  "
                    "originalKeyCode=%d, fallbackKeyCode=%d, fallbackMetaState=%08x",
                    originalKeyCode, fallbackKeyCode, keyEntry->metaState);
#endif
            return true; // restart the event
        } else {
#if DEBUG_OUTBOUND_EVENT_DETAILS
            ALOGD("Unhandled key event: No fallback key.");
#endif

            // Report the key as unhandled, since there is no fallback key.
            mReporter->reportUnhandledKey(keyEntry->sequenceNum);
        }
    }
    return false;
}

bool InputDispatcher::afterMotionEventLockedInterruptible(const sp<Connection>& connection,
        DispatchEntry* dispatchEntry, MotionEntry* motionEntry, bool handled) {
    return false;
}

void InputDispatcher::doPokeUserActivityLockedInterruptible(CommandEntry* commandEntry) {
    mLock.unlock();

    mPolicy->pokeUserActivity(commandEntry->eventTime, commandEntry->userActivityEventType);

    mLock.lock();
}

void InputDispatcher::initializeKeyEvent(KeyEvent* event, const KeyEntry* entry) {
    event->initialize(entry->deviceId, entry->source, entry->displayId, entry->action, entry->flags,
            entry->keyCode, entry->scanCode, entry->metaState, entry->repeatCount,
            entry->downTime, entry->eventTime);
}

void InputDispatcher::updateDispatchStatistics(nsecs_t currentTime, const EventEntry* entry,
        int32_t injectionResult, nsecs_t timeSpentWaitingForApplication) {
    // TODO Write some statistics about how long we spend waiting.
}

void InputDispatcher::traceInboundQueueLengthLocked() {
    if (ATRACE_ENABLED()) {
        ATRACE_INT("iq", mInboundQueue.count());
    }
}

void InputDispatcher::traceOutboundQueueLength(const sp<Connection>& connection) {
    if (ATRACE_ENABLED()) {
        char counterName[40];
        snprintf(counterName, sizeof(counterName), "oq:%s", connection->getWindowName().c_str());
        ATRACE_INT(counterName, connection->outboundQueue.count());
    }
}

void InputDispatcher::traceWaitQueueLength(const sp<Connection>& connection) {
    if (ATRACE_ENABLED()) {
        char counterName[40];
        snprintf(counterName, sizeof(counterName), "wq:%s", connection->getWindowName().c_str());
        ATRACE_INT(counterName, connection->waitQueue.count());
    }
}

void InputDispatcher::dump(std::string& dump) {
    std::scoped_lock _l(mLock);

    dump += "Input Dispatcher State:\n";
    dumpDispatchStateLocked(dump);

    if (!mLastANRState.empty()) {
        dump += "\nInput Dispatcher State at time of last ANR:\n";
        dump += mLastANRState;
    }
}

void InputDispatcher::monitor() {
    // Acquire and release the lock to ensure that the dispatcher has not deadlocked.
    std::unique_lock _l(mLock);
    mLooper->wake();
    mDispatcherIsAlive.wait(_l);
}


// --- InputDispatcher::InjectionState ---

InputDispatcher::InjectionState::InjectionState(int32_t injectorPid, int32_t injectorUid) :
        refCount(1),
        injectorPid(injectorPid), injectorUid(injectorUid),
        injectionResult(INPUT_EVENT_INJECTION_PENDING), injectionIsAsync(false),
        pendingForegroundDispatches(0) {
}

InputDispatcher::InjectionState::~InjectionState() {
}

void InputDispatcher::InjectionState::release() {
    refCount -= 1;
    if (refCount == 0) {
        delete this;
    } else {
        ALOG_ASSERT(refCount > 0);
    }
}


// --- InputDispatcher::EventEntry ---

InputDispatcher::EventEntry::EventEntry(uint32_t sequenceNum, int32_t type,
        nsecs_t eventTime, uint32_t policyFlags) :
        sequenceNum(sequenceNum), refCount(1), type(type), eventTime(eventTime),
        policyFlags(policyFlags), injectionState(nullptr), dispatchInProgress(false) {
}

InputDispatcher::EventEntry::~EventEntry() {
    releaseInjectionState();
}

void InputDispatcher::EventEntry::release() {
    refCount -= 1;
    if (refCount == 0) {
        delete this;
    } else {
        ALOG_ASSERT(refCount > 0);
    }
}

void InputDispatcher::EventEntry::releaseInjectionState() {
    if (injectionState) {
        injectionState->release();
        injectionState = nullptr;
    }
}


// --- InputDispatcher::ConfigurationChangedEntry ---

InputDispatcher::ConfigurationChangedEntry::ConfigurationChangedEntry(
        uint32_t sequenceNum, nsecs_t eventTime) :
        EventEntry(sequenceNum, TYPE_CONFIGURATION_CHANGED, eventTime, 0) {
}

InputDispatcher::ConfigurationChangedEntry::~ConfigurationChangedEntry() {
}

void InputDispatcher::ConfigurationChangedEntry::appendDescription(std::string& msg) const {
    msg += StringPrintf("ConfigurationChangedEvent(), policyFlags=0x%08x", policyFlags);
}


// --- InputDispatcher::DeviceResetEntry ---

InputDispatcher::DeviceResetEntry::DeviceResetEntry(
        uint32_t sequenceNum, nsecs_t eventTime, int32_t deviceId) :
        EventEntry(sequenceNum, TYPE_DEVICE_RESET, eventTime, 0),
        deviceId(deviceId) {
}

InputDispatcher::DeviceResetEntry::~DeviceResetEntry() {
}

void InputDispatcher::DeviceResetEntry::appendDescription(std::string& msg) const {
    msg += StringPrintf("DeviceResetEvent(deviceId=%d), policyFlags=0x%08x",
            deviceId, policyFlags);
}


// --- InputDispatcher::KeyEntry ---

InputDispatcher::KeyEntry::KeyEntry(uint32_t sequenceNum, nsecs_t eventTime,
        int32_t deviceId, uint32_t source, int32_t displayId, uint32_t policyFlags, int32_t action,
        int32_t flags, int32_t keyCode, int32_t scanCode, int32_t metaState,
        int32_t repeatCount, nsecs_t downTime) :
        EventEntry(sequenceNum, TYPE_KEY, eventTime, policyFlags),
        deviceId(deviceId), source(source), displayId(displayId), action(action), flags(flags),
        keyCode(keyCode), scanCode(scanCode), metaState(metaState),
        repeatCount(repeatCount), downTime(downTime),
        syntheticRepeat(false), interceptKeyResult(KeyEntry::INTERCEPT_KEY_RESULT_UNKNOWN),
        interceptKeyWakeupTime(0) {
}

InputDispatcher::KeyEntry::~KeyEntry() {
}

void InputDispatcher::KeyEntry::appendDescription(std::string& msg) const {
    msg += StringPrintf("KeyEvent");
}

void InputDispatcher::KeyEntry::recycle() {
    releaseInjectionState();

    dispatchInProgress = false;
    syntheticRepeat = false;
    interceptKeyResult = KeyEntry::INTERCEPT_KEY_RESULT_UNKNOWN;
    interceptKeyWakeupTime = 0;
}


// --- InputDispatcher::MotionEntry ---

InputDispatcher::MotionEntry::MotionEntry(uint32_t sequenceNum, nsecs_t eventTime, int32_t deviceId,
        uint32_t source, int32_t displayId, uint32_t policyFlags, int32_t action,
        int32_t actionButton,
        int32_t flags, int32_t metaState, int32_t buttonState, MotionClassification classification,
        int32_t edgeFlags, float xPrecision, float yPrecision, nsecs_t downTime,
        uint32_t pointerCount,
        const PointerProperties* pointerProperties, const PointerCoords* pointerCoords,
        float xOffset, float yOffset) :
        EventEntry(sequenceNum, TYPE_MOTION, eventTime, policyFlags),
        eventTime(eventTime),
        deviceId(deviceId), source(source), displayId(displayId), action(action),
        actionButton(actionButton), flags(flags), metaState(metaState), buttonState(buttonState),
        classification(classification), edgeFlags(edgeFlags),
        xPrecision(xPrecision), yPrecision(yPrecision),
        downTime(downTime), pointerCount(pointerCount) {
    for (uint32_t i = 0; i < pointerCount; i++) {
        this->pointerProperties[i].copyFrom(pointerProperties[i]);
        this->pointerCoords[i].copyFrom(pointerCoords[i]);
        if (xOffset || yOffset) {
            this->pointerCoords[i].applyOffset(xOffset, yOffset);
        }
    }
}

InputDispatcher::MotionEntry::~MotionEntry() {
}

void InputDispatcher::MotionEntry::appendDescription(std::string& msg) const {
    msg += StringPrintf("MotionEvent");
}


// --- InputDispatcher::DispatchEntry ---

volatile int32_t InputDispatcher::DispatchEntry::sNextSeqAtomic;

InputDispatcher::DispatchEntry::DispatchEntry(EventEntry* eventEntry,
        int32_t targetFlags, float xOffset, float yOffset, float globalScaleFactor,
        float windowXScale, float windowYScale) :
        seq(nextSeq()),
        eventEntry(eventEntry), targetFlags(targetFlags),
        xOffset(xOffset), yOffset(yOffset), globalScaleFactor(globalScaleFactor),
        windowXScale(windowXScale), windowYScale(windowYScale),
        deliveryTime(0), resolvedAction(0), resolvedFlags(0) {
    eventEntry->refCount += 1;
}

InputDispatcher::DispatchEntry::~DispatchEntry() {
    eventEntry->release();
}

uint32_t InputDispatcher::DispatchEntry::nextSeq() {
    // Sequence number 0 is reserved and will never be returned.
    uint32_t seq;
    do {
        seq = android_atomic_inc(&sNextSeqAtomic);
    } while (!seq);
    return seq;
}


// --- InputDispatcher::InputState ---

InputDispatcher::InputState::InputState() {
}

InputDispatcher::InputState::~InputState() {
}

bool InputDispatcher::InputState::isNeutral() const {
    return mKeyMementos.empty() && mMotionMementos.empty();
}

bool InputDispatcher::InputState::isHovering(int32_t deviceId, uint32_t source,
        int32_t displayId) const {
    for (const MotionMemento& memento : mMotionMementos) {
        if (memento.deviceId == deviceId
                && memento.source == source
                && memento.displayId == displayId
                && memento.hovering) {
            return true;
        }
    }
    return false;
}

bool InputDispatcher::InputState::trackKey(const KeyEntry* entry,
        int32_t action, int32_t flags) {
    switch (action) {
    case AKEY_EVENT_ACTION_UP: {
        if (entry->flags & AKEY_EVENT_FLAG_FALLBACK) {
            for (size_t i = 0; i < mFallbackKeys.size(); ) {
                if (mFallbackKeys.valueAt(i) == entry->keyCode) {
                    mFallbackKeys.removeItemsAt(i);
                } else {
                    i += 1;
                }
            }
        }
        ssize_t index = findKeyMemento(entry);
        if (index >= 0) {
            mKeyMementos.erase(mKeyMementos.begin() + index);
            return true;
        }
        /* FIXME: We can't just drop the key up event because that prevents creating
         * popup windows that are automatically shown when a key is held and then
         * dismissed when the key is released.  The problem is that the popup will
         * not have received the original key down, so the key up will be considered
         * to be inconsistent with its observed state.  We could perhaps handle this
         * by synthesizing a key down but that will cause other problems.
         *
         * So for now, allow inconsistent key up events to be dispatched.
         *
#if DEBUG_OUTBOUND_EVENT_DETAILS
        ALOGD("Dropping inconsistent key up event: deviceId=%d, source=%08x, "
                "keyCode=%d, scanCode=%d",
                entry->deviceId, entry->source, entry->keyCode, entry->scanCode);
#endif
        return false;
        */
        return true;
    }

    case AKEY_EVENT_ACTION_DOWN: {
        ssize_t index = findKeyMemento(entry);
        if (index >= 0) {
            mKeyMementos.erase(mKeyMementos.begin() + index);
        }
        addKeyMemento(entry, flags);
        return true;
    }

    default:
        return true;
    }
}

bool InputDispatcher::InputState::trackMotion(const MotionEntry* entry,
        int32_t action, int32_t flags) {
    int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
    switch (actionMasked) {
    case AMOTION_EVENT_ACTION_UP:
    case AMOTION_EVENT_ACTION_CANCEL: {
        ssize_t index = findMotionMemento(entry, false /*hovering*/);
        if (index >= 0) {
            mMotionMementos.erase(mMotionMementos.begin() + index);
            return true;
        }
#if DEBUG_OUTBOUND_EVENT_DETAILS
        ALOGD("Dropping inconsistent motion up or cancel event: deviceId=%d, source=%08x, "
                "displayId=%" PRId32 ", actionMasked=%d",
                entry->deviceId, entry->source, entry->displayId, actionMasked);
#endif
        return false;
    }

    case AMOTION_EVENT_ACTION_DOWN: {
        ssize_t index = findMotionMemento(entry, false /*hovering*/);
        if (index >= 0) {
            mMotionMementos.erase(mMotionMementos.begin() + index);
        }
        addMotionMemento(entry, flags, false /*hovering*/);
        return true;
    }

    case AMOTION_EVENT_ACTION_POINTER_UP:
    case AMOTION_EVENT_ACTION_POINTER_DOWN:
    case AMOTION_EVENT_ACTION_MOVE: {
        if (entry->source & AINPUT_SOURCE_CLASS_NAVIGATION) {
            // Trackballs can send MOVE events with a corresponding DOWN or UP. There's no need to
            // generate cancellation events for these since they're based in relative rather than
            // absolute units.
            return true;
        }

        ssize_t index = findMotionMemento(entry, false /*hovering*/);

        if (entry->source & AINPUT_SOURCE_CLASS_JOYSTICK) {
            // Joysticks can send MOVE events without a corresponding DOWN or UP. Since all
            // joystick axes are normalized to [-1, 1] we can trust that 0 means it's neutral. Any
            // other value and we need to track the motion so we can send cancellation events for
            // anything generating fallback events (e.g. DPad keys for joystick movements).
            if (index >= 0) {
                if (entry->pointerCoords[0].isEmpty()) {
                    mMotionMementos.erase(mMotionMementos.begin() + index);
                } else {
                    MotionMemento& memento = mMotionMementos[index];
                    memento.setPointers(entry);
                }
            } else if (!entry->pointerCoords[0].isEmpty()) {
                addMotionMemento(entry, flags, false /*hovering*/);
            }

            // Joysticks and trackballs can send MOVE events without corresponding DOWN or UP.
            return true;
        }
        if (index >= 0) {
            MotionMemento& memento = mMotionMementos[index];
            memento.setPointers(entry);
            return true;
        }
#if DEBUG_OUTBOUND_EVENT_DETAILS
        ALOGD("Dropping inconsistent motion pointer up/down or move event: "
                "deviceId=%d, source=%08x, displayId=%" PRId32 ", actionMasked=%d",
                entry->deviceId, entry->source, entry->displayId, actionMasked);
#endif
        return false;
    }

    case AMOTION_EVENT_ACTION_HOVER_EXIT: {
        ssize_t index = findMotionMemento(entry, true /*hovering*/);
        if (index >= 0) {
            mMotionMementos.erase(mMotionMementos.begin() + index);
            return true;
        }
#if DEBUG_OUTBOUND_EVENT_DETAILS
        ALOGD("Dropping inconsistent motion hover exit event: deviceId=%d, source=%08x, "
                "displayId=%" PRId32,
                entry->deviceId, entry->source, entry->displayId);
#endif
        return false;
    }

    case AMOTION_EVENT_ACTION_HOVER_ENTER:
    case AMOTION_EVENT_ACTION_HOVER_MOVE: {
        ssize_t index = findMotionMemento(entry, true /*hovering*/);
        if (index >= 0) {
            mMotionMementos.erase(mMotionMementos.begin() + index);
        }
        addMotionMemento(entry, flags, true /*hovering*/);
        return true;
    }

    default:
        return true;
    }
}

ssize_t InputDispatcher::InputState::findKeyMemento(const KeyEntry* entry) const {
    for (size_t i = 0; i < mKeyMementos.size(); i++) {
        const KeyMemento& memento = mKeyMementos[i];
        if (memento.deviceId == entry->deviceId
                && memento.source == entry->source
                && memento.displayId == entry->displayId
                && memento.keyCode == entry->keyCode
                && memento.scanCode == entry->scanCode) {
            return i;
        }
    }
    return -1;
}

ssize_t InputDispatcher::InputState::findMotionMemento(const MotionEntry* entry,
        bool hovering) const {
    for (size_t i = 0; i < mMotionMementos.size(); i++) {
        const MotionMemento& memento = mMotionMementos[i];
        if (memento.deviceId == entry->deviceId
                && memento.source == entry->source
                && memento.displayId == entry->displayId
                && memento.hovering == hovering) {
            return i;
        }
    }
    return -1;
}

void InputDispatcher::InputState::addKeyMemento(const KeyEntry* entry, int32_t flags) {
    KeyMemento memento;
    memento.deviceId = entry->deviceId;
    memento.source = entry->source;
    memento.displayId = entry->displayId;
    memento.keyCode = entry->keyCode;
    memento.scanCode = entry->scanCode;
    memento.metaState = entry->metaState;
    memento.flags = flags;
    memento.downTime = entry->downTime;
    memento.policyFlags = entry->policyFlags;
    mKeyMementos.push_back(memento);
}

void InputDispatcher::InputState::addMotionMemento(const MotionEntry* entry,
        int32_t flags, bool hovering) {
    MotionMemento memento;
    memento.deviceId = entry->deviceId;
    memento.source = entry->source;
    memento.displayId = entry->displayId;
    memento.flags = flags;
    memento.xPrecision = entry->xPrecision;
    memento.yPrecision = entry->yPrecision;
    memento.downTime = entry->downTime;
    memento.setPointers(entry);
    memento.hovering = hovering;
    memento.policyFlags = entry->policyFlags;
    mMotionMementos.push_back(memento);
}

void InputDispatcher::InputState::MotionMemento::setPointers(const MotionEntry* entry) {
    pointerCount = entry->pointerCount;
    for (uint32_t i = 0; i < entry->pointerCount; i++) {
        pointerProperties[i].copyFrom(entry->pointerProperties[i]);
        pointerCoords[i].copyFrom(entry->pointerCoords[i]);
    }
}

void InputDispatcher::InputState::synthesizeCancelationEvents(nsecs_t currentTime,
        std::vector<EventEntry*>& outEvents, const CancelationOptions& options) {
    for (KeyMemento& memento : mKeyMementos) {
        if (shouldCancelKey(memento, options)) {
            outEvents.push_back(new KeyEntry(SYNTHESIZED_EVENT_SEQUENCE_NUM, currentTime,
                    memento.deviceId, memento.source, memento.displayId, memento.policyFlags,
                    AKEY_EVENT_ACTION_UP, memento.flags | AKEY_EVENT_FLAG_CANCELED,
                    memento.keyCode, memento.scanCode, memento.metaState, 0, memento.downTime));
        }
    }

    for (const MotionMemento& memento : mMotionMementos) {
        if (shouldCancelMotion(memento, options)) {
            const int32_t action = memento.hovering ?
                    AMOTION_EVENT_ACTION_HOVER_EXIT : AMOTION_EVENT_ACTION_CANCEL;
            outEvents.push_back(new MotionEntry(SYNTHESIZED_EVENT_SEQUENCE_NUM, currentTime,
                    memento.deviceId, memento.source, memento.displayId, memento.policyFlags,
                    action, 0 /*actionButton*/, memento.flags, AMETA_NONE, 0 /*buttonState*/,
                    MotionClassification::NONE, AMOTION_EVENT_EDGE_FLAG_NONE,
                    memento.xPrecision, memento.yPrecision, memento.downTime,
                    memento.pointerCount, memento.pointerProperties, memento.pointerCoords,
                    0 /*xOffset*/, 0 /*yOffset*/));
        }
    }
}

void InputDispatcher::InputState::clear() {
    mKeyMementos.clear();
    mMotionMementos.clear();
    mFallbackKeys.clear();
}

void InputDispatcher::InputState::copyPointerStateTo(InputState& other) const {
    for (size_t i = 0; i < mMotionMementos.size(); i++) {
        const MotionMemento& memento = mMotionMementos[i];
        if (memento.source & AINPUT_SOURCE_CLASS_POINTER) {
            for (size_t j = 0; j < other.mMotionMementos.size(); ) {
                const MotionMemento& otherMemento = other.mMotionMementos[j];
                if (memento.deviceId == otherMemento.deviceId
                        && memento.source == otherMemento.source
                        && memento.displayId == otherMemento.displayId) {
                    other.mMotionMementos.erase(other.mMotionMementos.begin() + j);
                } else {
                    j += 1;
                }
            }
            other.mMotionMementos.push_back(memento);
        }
    }
}

int32_t InputDispatcher::InputState::getFallbackKey(int32_t originalKeyCode) {
    ssize_t index = mFallbackKeys.indexOfKey(originalKeyCode);
    return index >= 0 ? mFallbackKeys.valueAt(index) : -1;
}

void InputDispatcher::InputState::setFallbackKey(int32_t originalKeyCode,
        int32_t fallbackKeyCode) {
    ssize_t index = mFallbackKeys.indexOfKey(originalKeyCode);
    if (index >= 0) {
        mFallbackKeys.replaceValueAt(index, fallbackKeyCode);
    } else {
        mFallbackKeys.add(originalKeyCode, fallbackKeyCode);
    }
}

void InputDispatcher::InputState::removeFallbackKey(int32_t originalKeyCode) {
    mFallbackKeys.removeItem(originalKeyCode);
}

bool InputDispatcher::InputState::shouldCancelKey(const KeyMemento& memento,
        const CancelationOptions& options) {
    if (options.keyCode && memento.keyCode != options.keyCode.value()) {
        return false;
    }

    if (options.deviceId  && memento.deviceId != options.deviceId.value()) {
        return false;
    }

    if (options.displayId && memento.displayId != options.displayId.value()) {
        return false;
    }

    switch (options.mode) {
    case CancelationOptions::CANCEL_ALL_EVENTS:
    case CancelationOptions::CANCEL_NON_POINTER_EVENTS:
        return true;
    case CancelationOptions::CANCEL_FALLBACK_EVENTS:
        return memento.flags & AKEY_EVENT_FLAG_FALLBACK;
    default:
        return false;
    }
}

bool InputDispatcher::InputState::shouldCancelMotion(const MotionMemento& memento,
        const CancelationOptions& options) {
    if (options.deviceId && memento.deviceId != options.deviceId.value()) {
        return false;
    }

    if (options.displayId && memento.displayId != options.displayId.value()) {
        return false;
    }

    switch (options.mode) {
    case CancelationOptions::CANCEL_ALL_EVENTS:
        return true;
    case CancelationOptions::CANCEL_POINTER_EVENTS:
        return memento.source & AINPUT_SOURCE_CLASS_POINTER;
    case CancelationOptions::CANCEL_NON_POINTER_EVENTS:
        return !(memento.source & AINPUT_SOURCE_CLASS_POINTER);
    default:
        return false;
    }
}


// --- InputDispatcher::Connection ---

InputDispatcher::Connection::Connection(const sp<InputChannel>& inputChannel, bool monitor) :
        status(STATUS_NORMAL), inputChannel(inputChannel),
        monitor(monitor),
        inputPublisher(inputChannel), inputPublisherBlocked(false) {
}

InputDispatcher::Connection::~Connection() {
}

const std::string InputDispatcher::Connection::getWindowName() const {
    if (inputChannel != nullptr) {
        return inputChannel->getName();
    }
    if (monitor) {
        return "monitor";
    }
    return "?";
}

const char* InputDispatcher::Connection::getStatusLabel() const {
    switch (status) {
    case STATUS_NORMAL:
        return "NORMAL";

    case STATUS_BROKEN:
        return "BROKEN";

    case STATUS_ZOMBIE:
        return "ZOMBIE";

    default:
        return "UNKNOWN";
    }
}

InputDispatcher::DispatchEntry* InputDispatcher::Connection::findWaitQueueEntry(uint32_t seq) {
    for (DispatchEntry* entry = waitQueue.head; entry != nullptr; entry = entry->next) {
        if (entry->seq == seq) {
            return entry;
        }
    }
    return nullptr;
}

// --- InputDispatcher::Monitor
InputDispatcher::Monitor::Monitor(const sp<InputChannel>& inputChannel) :
    inputChannel(inputChannel) {
}


// --- InputDispatcher::CommandEntry ---
//
InputDispatcher::CommandEntry::CommandEntry(Command command) :
    command(command), eventTime(0), keyEntry(nullptr), userActivityEventType(0),
    seq(0), handled(false) {
}

InputDispatcher::CommandEntry::~CommandEntry() {
}

// --- InputDispatcher::TouchedMonitor ---
InputDispatcher::TouchedMonitor::TouchedMonitor(const Monitor& monitor, float xOffset,
        float yOffset) : monitor(monitor), xOffset(xOffset), yOffset(yOffset) {
}

// --- InputDispatcher::TouchState ---

InputDispatcher::TouchState::TouchState() :
    down(false), split(false), deviceId(-1), source(0), displayId(ADISPLAY_ID_NONE) {
}

InputDispatcher::TouchState::~TouchState() {
}

void InputDispatcher::TouchState::reset() {
    down = false;
    split = false;
    deviceId = -1;
    source = 0;
    displayId = ADISPLAY_ID_NONE;
    windows.clear();
    portalWindows.clear();
    gestureMonitors.clear();
}

void InputDispatcher::TouchState::copyFrom(const TouchState& other) {
    down = other.down;
    split = other.split;
    deviceId = other.deviceId;
    source = other.source;
    displayId = other.displayId;
    windows = other.windows;
    portalWindows = other.portalWindows;
    gestureMonitors = other.gestureMonitors;
}

void InputDispatcher::TouchState::addOrUpdateWindow(const sp<InputWindowHandle>& windowHandle,
        int32_t targetFlags, BitSet32 pointerIds) {
    if (targetFlags & InputTarget::FLAG_SPLIT) {
        split = true;
    }

    for (size_t i = 0; i < windows.size(); i++) {
        TouchedWindow& touchedWindow = windows[i];
        if (touchedWindow.windowHandle == windowHandle) {
            touchedWindow.targetFlags |= targetFlags;
            if (targetFlags & InputTarget::FLAG_DISPATCH_AS_SLIPPERY_EXIT) {
                touchedWindow.targetFlags &= ~InputTarget::FLAG_DISPATCH_AS_IS;
            }
            touchedWindow.pointerIds.value |= pointerIds.value;
            return;
        }
    }

    TouchedWindow touchedWindow;
    touchedWindow.windowHandle = windowHandle;
    touchedWindow.targetFlags = targetFlags;
    touchedWindow.pointerIds = pointerIds;
    windows.push_back(touchedWindow);
}

void InputDispatcher::TouchState::addPortalWindow(const sp<InputWindowHandle>& windowHandle) {
    size_t numWindows = portalWindows.size();
    for (size_t i = 0; i < numWindows; i++) {
        if (portalWindows[i] == windowHandle) {
            return;
        }
    }
    portalWindows.push_back(windowHandle);
}

void InputDispatcher::TouchState::addGestureMonitors(
        const std::vector<TouchedMonitor>& newMonitors) {
    const size_t newSize = gestureMonitors.size() + newMonitors.size();
    gestureMonitors.reserve(newSize);
    gestureMonitors.insert(std::end(gestureMonitors),
            std::begin(newMonitors), std::end(newMonitors));
}

void InputDispatcher::TouchState::removeWindow(const sp<InputWindowHandle>& windowHandle) {
    for (size_t i = 0; i < windows.size(); i++) {
        if (windows[i].windowHandle == windowHandle) {
            windows.erase(windows.begin() + i);
            return;
        }
    }
}

void InputDispatcher::TouchState::removeWindowByToken(const sp<IBinder>& token) {
    for (size_t i = 0; i < windows.size(); i++) {
        if (windows[i].windowHandle->getToken() == token) {
            windows.erase(windows.begin() + i);
            return;
        }
    }
}

void InputDispatcher::TouchState::filterNonAsIsTouchWindows() {
    for (size_t i = 0 ; i < windows.size(); ) {
        TouchedWindow& window = windows[i];
        if (window.targetFlags & (InputTarget::FLAG_DISPATCH_AS_IS
                | InputTarget::FLAG_DISPATCH_AS_SLIPPERY_ENTER)) {
            window.targetFlags &= ~InputTarget::FLAG_DISPATCH_MASK;
            window.targetFlags |= InputTarget::FLAG_DISPATCH_AS_IS;
            i += 1;
        } else {
            windows.erase(windows.begin() + i);
        }
    }
}

void InputDispatcher::TouchState::filterNonMonitors() {
    windows.clear();
    portalWindows.clear();
}

sp<InputWindowHandle> InputDispatcher::TouchState::getFirstForegroundWindowHandle() const {
    for (size_t i = 0; i < windows.size(); i++) {
        const TouchedWindow& window = windows[i];
        if (window.targetFlags & InputTarget::FLAG_FOREGROUND) {
            return window.windowHandle;
        }
    }
    return nullptr;
}

bool InputDispatcher::TouchState::isSlippery() const {
    // Must have exactly one foreground window.
    bool haveSlipperyForegroundWindow = false;
    for (const TouchedWindow& window : windows) {
        if (window.targetFlags & InputTarget::FLAG_FOREGROUND) {
            if (haveSlipperyForegroundWindow
                    || !(window.windowHandle->getInfo()->layoutParamsFlags
                            & InputWindowInfo::FLAG_SLIPPERY)) {
                return false;
            }
            haveSlipperyForegroundWindow = true;
        }
    }
    return haveSlipperyForegroundWindow;
}


// --- InputDispatcherThread ---

InputDispatcherThread::InputDispatcherThread(const sp<InputDispatcherInterface>& dispatcher) :
        Thread(/*canCallJava*/ true), mDispatcher(dispatcher) {
}

InputDispatcherThread::~InputDispatcherThread() {
}

bool InputDispatcherThread::threadLoop() {
    mDispatcher->dispatchOnce();
    return true;
}

} // namespace android
