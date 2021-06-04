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
#ifndef CLEARKEY_JSON_WEB_KEY_H_
#define CLEARKEY_JSON_WEB_KEY_H_

#include <media/stagefright/foundation/ABase.h>
#include <utils/KeyedVector.h>
#include <utils/String8.h>
#include <utils/StrongPointer.h>

#include "jsmn.h"
#include "Utils.h"
#include "ClearKeyTypes.h"

namespace clearkeydrm {

using android::KeyedVector;
using android::sp;
using android::String8;
using android::Vector;

class JsonWebKey {
 public:
    JsonWebKey();
    virtual ~JsonWebKey();

    bool extractKeysFromJsonWebKeySet(const String8& jsonWebKeySet,
            KeyMap* keys);

 private:
    Vector<jsmntok_t> mJsmnTokens;
    Vector<String8> mJsonObjects;
    Vector<String8> mTokens;

    bool decodeBase64String(const String8& encodedText,
            Vector<uint8_t>* decodedText);
    bool findKey(const String8& jsonObject, String8* keyId,
            String8* encodedKey);
    void findValue(const String8 &key, String8* value);
    bool isJsonWebKeySet(const String8& jsonObject) const;
    bool parseJsonObject(const String8& jsonObject, Vector<String8>* tokens);
    bool parseJsonWebKeySet(const String8& jsonWebKeySet, Vector<String8>* jsonObjects);

    DISALLOW_EVIL_CONSTRUCTORS(JsonWebKey);
};

}  // namespace clearkeydrm

#endif  // CLEARKEY_JSON_WEB_KEY_H_
