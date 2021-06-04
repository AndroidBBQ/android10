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

#ifndef CLEARKEY_SESSION_H_
#define CLEARKEY_SESSION_H_

#include <media/stagefright/foundation/ABase.h>
#include <utils/Errors.h>
#include <utils/Mutex.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/Vector.h>

#include "ClearKeyTypes.h"
#include "Utils.h"

namespace clearkeydrm {

class Session : public android::RefBase {
public:
    explicit Session(const android::Vector<uint8_t>& sessionId)
            : mSessionId(sessionId) {}
    virtual ~Session() {}

    const android::Vector<uint8_t>& sessionId() const { return mSessionId; }

    android::status_t getKeyRequest(
            const android::Vector<uint8_t>& mimeType,
            const android::String8& initDataType,
            android::Vector<uint8_t>* keyRequest) const;

    android::status_t provideKeyResponse(
            const android::Vector<uint8_t>& response);

    android::status_t decrypt(
            const KeyId keyId, const Iv iv, const void* source,
            void* destination, const SubSample* subSamples,
            size_t numSubSamples, size_t* bytesDecryptedOut);

private:
    DISALLOW_EVIL_CONSTRUCTORS(Session);

    const android::Vector<uint8_t> mSessionId;

    android::Mutex mMapLock;
    KeyMap mKeyMap;
};

} // namespace clearkeydrm

#endif // CLEARKEY_SESSION_H_
