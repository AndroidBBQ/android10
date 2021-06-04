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
#define LOG_TAG "JsonWebKey"

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/foundation/base64.h>
#include <utils/Log.h>

#include "JsonWebKey.h"

namespace {
const android::String8 kKeysTag("keys");
const android::String8 kKeyTypeTag("kty");
const android::String8 kSymmetricKeyValue("oct");
const android::String8 kKeyTag("k");
const android::String8 kKeyIdTag("kid");
const android::String8 kBase64Padding("=");
}

namespace clearkeydrm {

using android::ABuffer;
using android::AString;

JsonWebKey::JsonWebKey() {
}

JsonWebKey::~JsonWebKey() {
}

/*
 * Parses a JSON Web Key Set string, initializes a KeyMap with key id:key
 * pairs from the JSON Web Key Set. Both key ids and keys are base64url
 * encoded. The KeyMap contains base64url decoded key id:key pairs.
 *
 * @return Returns false for errors, true for success.
 */
bool JsonWebKey::extractKeysFromJsonWebKeySet(const String8& jsonWebKeySet,
        KeyMap* keys) {

    keys->clear();
    if (!parseJsonWebKeySet(jsonWebKeySet, &mJsonObjects)) {
        return false;
    }

    // mJsonObjects[0] contains the entire JSON Web Key Set, including
    // all the base64 encoded keys. Each key is also stored separately as
    // a JSON object in mJsonObjects[1..n] where n is the total
    // number of keys in the set.
    if (!isJsonWebKeySet(mJsonObjects[0])) {
        return false;
    }

    String8 encodedKey, encodedKeyId;
    Vector<uint8_t> decodedKey, decodedKeyId;

    // mJsonObjects[1] contains the first JSON Web Key in the set
    for (size_t i = 1; i < mJsonObjects.size(); ++i) {
        encodedKeyId.clear();
        encodedKey.clear();

        if (!parseJsonObject(mJsonObjects[i], &mTokens))
            return false;

        if (findKey(mJsonObjects[i], &encodedKeyId, &encodedKey)) {
            if (encodedKeyId.isEmpty() || encodedKey.isEmpty()) {
                ALOGE("Must have both key id and key in the JsonWebKey set.");
                continue;
            }

            if (!decodeBase64String(encodedKeyId, &decodedKeyId)) {
                ALOGE("Failed to decode key id(%s)", encodedKeyId.string());
                continue;
            }

            if (!decodeBase64String(encodedKey, &decodedKey)) {
                ALOGE("Failed to decode key(%s)", encodedKey.string());
                continue;
            }

            keys->add(decodedKeyId, decodedKey);
        }
    }
    return true;
}

bool JsonWebKey::decodeBase64String(const String8& encodedText,
        Vector<uint8_t>* decodedText) {

    decodedText->clear();

    // encodedText should not contain padding characters as per EME spec.
    if (encodedText.find(kBase64Padding) != -1) {
        return false;
    }

    // Since android::decodeBase64() requires padding characters,
    // add them so length of encodedText is exactly a multiple of 4.
    int remainder = encodedText.length() % 4;
    String8 paddedText(encodedText);
    if (remainder > 0) {
        for (int i = 0; i < 4 - remainder; ++i) {
            paddedText.append(kBase64Padding);
        }
    }

    android::sp<ABuffer> buffer =
            android::decodeBase64(AString(paddedText.string()));
    if (buffer == NULL) {
        ALOGE("Malformed base64 encoded content found.");
        return false;
    }

    decodedText->appendArray(buffer->base(), buffer->size());
    return true;
}

bool JsonWebKey::findKey(const String8& jsonObject, String8* keyId,
        String8* encodedKey) {

    String8 key, value;

    // Only allow symmetric key, i.e. "kty":"oct" pair.
    if (jsonObject.find(kKeyTypeTag) >= 0) {
        findValue(kKeyTypeTag, &value);
        if (0 != value.compare(kSymmetricKeyValue))
            return false;
    }

    if (jsonObject.find(kKeyIdTag) >= 0) {
        findValue(kKeyIdTag, keyId);
    }

    if (jsonObject.find(kKeyTag) >= 0) {
        findValue(kKeyTag, encodedKey);
    }
    return true;
}

void JsonWebKey::findValue(const String8 &key, String8* value) {
    value->clear();
    const char* valueToken;
    for (Vector<String8>::const_iterator nextToken = mTokens.begin();
        nextToken != mTokens.end(); ++nextToken) {
        if (0 == (*nextToken).compare(key)) {
            if (nextToken + 1 == mTokens.end())
                break;
            valueToken = (*(nextToken + 1)).string();
            value->setTo(valueToken);
            nextToken++;
            break;
        }
    }
}

bool JsonWebKey::isJsonWebKeySet(const String8& jsonObject) const {
    if (jsonObject.find(kKeysTag) == -1) {
        ALOGE("JSON Web Key does not contain keys.");
        return false;
    }
    return true;
}

/*
 * Parses a JSON objects string and initializes a vector of tokens.
 *
 * @return Returns false for errors, true for success.
 */
bool JsonWebKey::parseJsonObject(const String8& jsonObject,
        Vector<String8>* tokens) {
    jsmn_parser parser;

    jsmn_init(&parser);
    int numTokens = jsmn_parse(&parser,
        jsonObject.string(), jsonObject.size(), NULL, 0);
    if (numTokens < 0) {
        ALOGE("Parser returns error code=%d", numTokens);
        return false;
    }

    unsigned int jsmnTokensSize = numTokens * sizeof(jsmntok_t);
    mJsmnTokens.clear();
    mJsmnTokens.setCapacity(jsmnTokensSize);

    jsmn_init(&parser);
    int status = jsmn_parse(&parser, jsonObject.string(),
        jsonObject.size(), mJsmnTokens.editArray(), numTokens);
    if (status < 0) {
        ALOGE("Parser returns error code=%d", status);
        return false;
    }

    tokens->clear();
    String8 token;
    const char *pjs;
    for (int j = 0; j < numTokens; ++j) {
        pjs = jsonObject.string() + mJsmnTokens[j].start;
        if (mJsmnTokens[j].type == JSMN_STRING ||
                mJsmnTokens[j].type == JSMN_PRIMITIVE) {
            token.setTo(pjs, mJsmnTokens[j].end - mJsmnTokens[j].start);
            tokens->add(token);
        }
    }
    return true;
}

/*
 * Parses JSON Web Key Set string and initializes a vector of JSON objects.
 *
 * @return Returns false for errors, true for success.
 */
bool JsonWebKey::parseJsonWebKeySet(const String8& jsonWebKeySet,
        Vector<String8>* jsonObjects) {
    if (jsonWebKeySet.isEmpty()) {
        ALOGE("Empty JSON Web Key");
        return false;
    }

    // The jsmn parser only supports unicode encoding.
    jsmn_parser parser;

    // Computes number of tokens. A token marks the type, offset in
    // the original string.
    jsmn_init(&parser);
    int numTokens = jsmn_parse(&parser,
            jsonWebKeySet.string(), jsonWebKeySet.size(), NULL, 0);
    if (numTokens < 0) {
        ALOGE("Parser returns error code=%d", numTokens);
        return false;
    }

    unsigned int jsmnTokensSize = numTokens * sizeof(jsmntok_t);
    mJsmnTokens.setCapacity(jsmnTokensSize);

    jsmn_init(&parser);
    int status = jsmn_parse(&parser, jsonWebKeySet.string(),
            jsonWebKeySet.size(), mJsmnTokens.editArray(), numTokens);
    if (status < 0) {
        ALOGE("Parser returns error code=%d", status);
        return false;
    }

    String8 token;
    const char *pjs;
    for (int i = 0; i < numTokens; ++i) {
        pjs = jsonWebKeySet.string() + mJsmnTokens[i].start;
        if (mJsmnTokens[i].type == JSMN_OBJECT) {
            token.setTo(pjs, mJsmnTokens[i].end - mJsmnTokens[i].start);
            jsonObjects->add(token);
        }
    }
    return true;
}

}  // clearkeydrm
