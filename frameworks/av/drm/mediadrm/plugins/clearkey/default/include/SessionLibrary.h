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

#ifndef CLEARKEY_SESSION_LIBRARY_H_
#define CLEARKEY_SESSION_LIBRARY_H_

#include <utils/KeyedVector.h>
#include <utils/Mutex.h>
#include <utils/StrongPointer.h>
#include <utils/Vector.h>

#include "Session.h"
#include "Utils.h"

namespace clearkeydrm {

class SessionLibrary {
public:
    static SessionLibrary* get();

    android::sp<Session> createSession();

    android::sp<Session> findSession(
            const android::Vector<uint8_t>& sessionId);

    void destroySession(const android::sp<Session>& session);

private:
    DISALLOW_EVIL_CONSTRUCTORS(SessionLibrary);

    SessionLibrary() : mNextSessionId(1) {}

    static android::Mutex sSingletonLock;
    static SessionLibrary* sSingleton;

    android::Mutex mSessionsLock;
    uint32_t mNextSessionId;
    android::KeyedVector<android::Vector<uint8_t>, android::sp<Session> >
            mSessions;
};

} // namespace clearkeydrm

#endif // CLEARKEY_SESSION_LIBRARY_H_
