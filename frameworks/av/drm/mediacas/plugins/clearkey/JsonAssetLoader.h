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
#ifndef JSON_ASSET_LOADER_H_
#define JSON_ASSET_LOADER_H_

#include <media/stagefright/foundation/ABase.h>
#include <utils/KeyedVector.h>
#include <utils/String8.h>
#include <utils/StrongPointer.h>
#include "protos/license_protos.pb.h"

#include "jsmn.h"

namespace android {
struct ABuffer;

namespace clearkeycas {

class JsonAssetLoader {
 public:
    JsonAssetLoader();
    virtual ~JsonAssetLoader();

    status_t extractAssetFromString(
            const String8& jsonAssetString, Asset *asset);

 private:
    Vector<jsmntok_t> mJsmnTokens;
    Vector<String8> mJsonObjects;
    Vector<String8> mTokens;

    static sp<ABuffer> decodeBase64String(
            const String8& encodedText);
    bool findKey(const String8& jsonObject, Asset *asset);
    void findValue(
            const String8 &key, String8* value);
    bool parseJsonObject(
            const String8& jsonObject, Vector<String8>* tokens);
    bool parseJsonAssetString(
            const String8& jsonString, Vector<String8>* jsonObjects);

    DISALLOW_EVIL_CONSTRUCTORS(JsonAssetLoader);
};

}  // namespace clearkeycas
}  // namespace android

#endif  // JSON_ASSET_LOADER_H_
