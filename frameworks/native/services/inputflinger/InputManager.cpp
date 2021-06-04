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

#define LOG_TAG "InputManager"

//#define LOG_NDEBUG 0

#include "InputManager.h"
#include "InputReaderFactory.h"

#include <binder/IPCThreadState.h>

#include <log/log.h>
#include <unordered_map>

#include <private/android_filesystem_config.h>

namespace android {

InputManager::InputManager(
        const sp<InputReaderPolicyInterface>& readerPolicy,
        const sp<InputDispatcherPolicyInterface>& dispatcherPolicy) {
    mDispatcher = new InputDispatcher(dispatcherPolicy);
    mClassifier = new InputClassifier(mDispatcher);
    mReader = createInputReader(readerPolicy, mClassifier);
    initialize();
}

InputManager::~InputManager() {
    stop();
}

void InputManager::initialize() {
    mReaderThread = new InputReaderThread(mReader);
    mDispatcherThread = new InputDispatcherThread(mDispatcher);
}

status_t InputManager::start() {
    status_t result = mDispatcherThread->run("InputDispatcher", PRIORITY_URGENT_DISPLAY);
    if (result) {
        ALOGE("Could not start InputDispatcher thread due to error %d.", result);
        return result;
    }

    result = mReaderThread->run("InputReader", PRIORITY_URGENT_DISPLAY);
    if (result) {
        ALOGE("Could not start InputReader thread due to error %d.", result);

        mDispatcherThread->requestExit();
        return result;
    }

    return OK;
}

status_t InputManager::stop() {
    status_t result = mReaderThread->requestExitAndWait();
    if (result) {
        ALOGW("Could not stop InputReader thread due to error %d.", result);
    }

    result = mDispatcherThread->requestExitAndWait();
    if (result) {
        ALOGW("Could not stop InputDispatcher thread due to error %d.", result);
    }

    return OK;
}

sp<InputReaderInterface> InputManager::getReader() {
    return mReader;
}

sp<InputClassifierInterface> InputManager::getClassifier() {
    return mClassifier;
}

sp<InputDispatcherInterface> InputManager::getDispatcher() {
    return mDispatcher;
}

class BinderWindowHandle : public InputWindowHandle {
public:
    BinderWindowHandle(const InputWindowInfo& info) {
        mInfo = info;
    }

    bool updateInfo() override {
        return true;
    }
};

void InputManager::setInputWindows(const std::vector<InputWindowInfo>& infos,
        const sp<ISetInputWindowsListener>& setInputWindowsListener) {
    std::unordered_map<int32_t, std::vector<sp<InputWindowHandle>>> handlesPerDisplay;

    std::vector<sp<InputWindowHandle>> handles;
    for (const auto& info : infos) {
        handlesPerDisplay.emplace(info.displayId, std::vector<sp<InputWindowHandle>>());
        handlesPerDisplay[info.displayId].push_back(new BinderWindowHandle(info));
    }
    for (auto const& i : handlesPerDisplay) {
        mDispatcher->setInputWindows(i.second, i.first, setInputWindowsListener);
    }
}

// Used by tests only.
void InputManager::registerInputChannel(const sp<InputChannel>& channel) {
    IPCThreadState* ipc = IPCThreadState::self();
    const int uid = ipc->getCallingUid();
    if (uid != AID_SHELL && uid != AID_ROOT) {
        ALOGE("Invalid attempt to register input channel over IPC"
                "from non shell/root entity (PID: %d)", ipc->getCallingPid());
        return;
    }
    mDispatcher->registerInputChannel(channel, false);
}

void InputManager::unregisterInputChannel(const sp<InputChannel>& channel) {
    mDispatcher->unregisterInputChannel(channel);
}

void InputManager::setMotionClassifierEnabled(bool enabled) {
    mClassifier->setMotionClassifierEnabled(enabled);
}

} // namespace android
