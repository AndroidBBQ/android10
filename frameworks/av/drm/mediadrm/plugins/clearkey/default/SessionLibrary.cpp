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

//#define LOG_NDEBUG 0
#define LOG_TAG "ClearKeySessionLibrary"
#include <utils/Log.h>

#include <utils/String8.h>

#include "SessionLibrary.h"

namespace clearkeydrm {

using android::Mutex;
using android::sp;
using android::String8;
using android::Vector;

Mutex SessionLibrary::sSingletonLock;
SessionLibrary* SessionLibrary::sSingleton = NULL;

SessionLibrary* SessionLibrary::get() {
    Mutex::Autolock lock(sSingletonLock);

    if (sSingleton == NULL) {
        ALOGD("Instantiating Session Library Singleton.");
        sSingleton = new SessionLibrary();
    }

    return sSingleton;
}

sp<Session> SessionLibrary::createSession() {
    Mutex::Autolock lock(mSessionsLock);

    String8 sessionIdString = String8::format("%u", mNextSessionId);
    mNextSessionId += 1;
    Vector<uint8_t> sessionId;
    sessionId.appendArray(
            reinterpret_cast<const uint8_t*>(sessionIdString.string()),
            sessionIdString.size());

    mSessions.add(sessionId, new Session(sessionId));
    return mSessions.valueFor(sessionId);
}

sp<Session> SessionLibrary::findSession(
        const Vector<uint8_t>& sessionId) {
    Mutex::Autolock lock(mSessionsLock);
    if (mSessions.indexOfKey(sessionId) < 0) {
        return sp<Session>(NULL);
    }
    return mSessions.valueFor(sessionId);
}

void SessionLibrary::destroySession(const sp<Session>& session) {
    Mutex::Autolock lock(mSessionsLock);\
    mSessions.removeItem(session->sessionId());
}

} // namespace clearkeydrm
