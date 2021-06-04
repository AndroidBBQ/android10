/*
 * Copyright (C) 2014 The Android Open Source Project
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

#include "Configuration.h"
#include "FastThreadState.h"

namespace android {

FastThreadState::FastThreadState() :
    mCommand(INITIAL), mColdFutexAddr(NULL), mColdGen(0), mDumpState(NULL), mNBLogWriter(NULL)

{
}

FastThreadState::~FastThreadState()
{
}

// static
const char *FastThreadState::commandToString(FastThreadState::Command command)
{
    switch (command) {
    case FastThreadState::INITIAL:      return "INITIAL";
    case FastThreadState::HOT_IDLE:     return "HOT_IDLE";
    case FastThreadState::COLD_IDLE:    return "COLD_IDLE";
    case FastThreadState::EXIT:         return "EXIT";
    }
    return NULL;
}

}   // namespace android
