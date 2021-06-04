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

#define LOG_TAG "IPCThreadStateBase"

#include <binderthreadstate/IPCThreadStateBase.h>
#include <android-base/macros.h>

#include <utils/Log.h>

#include <errno.h>
#include <inttypes.h>
#include <pthread.h>

namespace android {

static pthread_mutex_t gTLSMutex = PTHREAD_MUTEX_INITIALIZER;
static bool gHaveTLS = false;
static pthread_key_t gTLS = 0;

IPCThreadStateBase::IPCThreadStateBase() {
    pthread_setspecific(gTLS, this);
}

IPCThreadStateBase* IPCThreadStateBase::self()
{
    if (gHaveTLS) {
restart:
        const pthread_key_t k = gTLS;
        IPCThreadStateBase* st = (IPCThreadStateBase*)pthread_getspecific(k);
        if (st) return st;
        return new IPCThreadStateBase;
    }

    pthread_mutex_lock(&gTLSMutex);
    if (!gHaveTLS) {
        int key_create_value = pthread_key_create(&gTLS, threadDestructor);
        if (key_create_value != 0) {
            pthread_mutex_unlock(&gTLSMutex);
            ALOGW("IPCThreadStateBase::self() unable to create TLS key, expect a crash: %s\n",
                    strerror(key_create_value));
            return nullptr;
        }
        gHaveTLS = true;
    }
    pthread_mutex_unlock(&gTLSMutex);
    goto restart;
}

void IPCThreadStateBase::pushCurrentState(CallState callState) {
    mCallStateStack.emplace(callState);
}

IPCThreadStateBase::CallState IPCThreadStateBase::popCurrentState() {
    ALOG_ASSERT(mCallStateStack.size > 0);
    CallState val = mCallStateStack.top();
    mCallStateStack.pop();
    return val;
}

IPCThreadStateBase::CallState IPCThreadStateBase::getCurrentBinderCallState() {
    if (mCallStateStack.size() > 0) {
        return mCallStateStack.top();
    }
    return CallState::NONE;
}

void IPCThreadStateBase::threadDestructor(void *st)
{
    IPCThreadStateBase* const self = static_cast<IPCThreadStateBase*>(st);
    if (self) {
        delete self;
    }
}

}; // namespace android
