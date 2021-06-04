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

#define LOG_NDEBUG 0
#define LOG_TAG "SimpleMethodState"
#include <log/log.h>

#include "SimpleMethodState.h"

namespace android {

SimpleMethodState::Injecter::Injecter(SimpleMethodState *thiz) : mThiz(thiz) {}

void SimpleMethodState::Injecter::hang() {
    mThiz->mMode = HANG;
}

void SimpleMethodState::Injecter::fail(c2_status_t err, bool execute) {
    mThiz->mMode = execute ? EXECUTE : NO_OP;
    mThiz->mOverride = true;
    mThiz->mResultOverride = err;
}

SimpleMethodState::SimpleMethodState()
    : mMode(EXECUTE), mOverride(false), mResultOverride(C2_OK) {}

SimpleMethodState::Mode SimpleMethodState::getMode() const {
    return mMode;
}

bool SimpleMethodState::overrideResult(c2_status_t *result) const {
    if (!mOverride) {
        return false;
    }
    *result = mResultOverride;
    return true;
}

}  // namespace android
