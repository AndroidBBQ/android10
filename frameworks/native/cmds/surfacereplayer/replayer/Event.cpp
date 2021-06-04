/*
 * Copyright 2016 The Android Open Source Project
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

#include "Event.h"

using namespace android;

Event::Event(Increment::IncrementCase type) : mIncrementType(type) {}

void Event::readyToExecute() {
    changeState(Event::EventState::Waiting);
    waitUntil(Event::EventState::Signaled);
    changeState(Event::EventState::Running);
}

void Event::complete() {
    waitUntil(Event::EventState::Waiting);
    changeState(Event::EventState::Signaled);
    waitUntil(Event::EventState::Running);
}

void Event::waitUntil(Event::EventState state) {
    std::unique_lock<std::mutex> lock(mLock);
    mCond.wait(lock, [this, state] { return (mState == state); });
}

void Event::changeState(Event::EventState state) {
    std::unique_lock<std::mutex> lock(mLock);
    mState = state;
    lock.unlock();

    mCond.notify_one();
}

Increment::IncrementCase Event::getIncrementType() {
    return mIncrementType;
}
