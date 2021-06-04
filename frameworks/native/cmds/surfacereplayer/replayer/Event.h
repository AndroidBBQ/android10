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

#ifndef ANDROID_SURFACEREPLAYER_EVENT_H
#define ANDROID_SURFACEREPLAYER_EVENT_H

#include <frameworks/native/cmds/surfacereplayer/proto/src/trace.pb.h>

#include <condition_variable>
#include <mutex>

namespace android {

class Event {
  public:
    Event(Increment::IncrementCase);

    enum class EventState {
        SettingUp,  // Completing as much time-independent work as possible
        Waiting,    // Waiting for signal from main thread to finish execution
        Signaled,   // Signaled by main thread, about to immediately switch to Running
        Running     // Finishing execution of rest of work
    };

    void readyToExecute();
    void complete();

    Increment::IncrementCase getIncrementType();

  private:
    void waitUntil(EventState state);
    void changeState(EventState state);

    std::mutex mLock;
    std::condition_variable mCond;

    EventState mState = EventState::SettingUp;

    Increment::IncrementCase mIncrementType;
};
}
#endif
