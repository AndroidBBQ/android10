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
#define LOG_TAG "hidl_JsonWebKey"

#include <utils/Log.h>

#include "JsonWebKey.h"

#include "Base64.h"

namespace {
const std::string kBase64Padding("=");
const std::string kKeysTag("keys");
const std::string kKeyTypeTag("kty");
const std::string kKeyTag("k");
const std::string kKeyIdTag("kid");
const std::string kMediaSessionType("type");
const std::string kPersistentLicenseSession("persistent-license");
const std::string kSymmetricKeyValue("oct");
const std::string kTemporaryLicenseSession("temporary");
}

namespace android {
namespace hardware {
namespace drm {
namespace V1_2 {
namespace clearkey {

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
bool JsonWebKey::extractKeysFromJsonWebKeySet(const std::string& jsonWebKeySet,
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

    std::string encodedKey, encodedKeyId;
    std::vector<uint8_t> decodedKey, decodedKeyId;

    // mJsonObjects[1] contains the first JSON Web Key in the set
    for (size_t i = 1; i < mJsonObjects.size(); ++i) {
        encodedKeyId.clear();
        encodedKey.clear();

        if (!parseJsonObject(mJsonObjects[i], &mTokens))
            return false;

        if (findKey(mJsonObjects[i], &encodedKeyId, &encodedKey)) {
            if (encodedKeyId.empty() || encodedKey.empty()) {
                ALOGE("Must have both key id and key in the JsonWebKey set.");
                continue;
            }

            if (!decodeBase64String(encodedKeyId, &decodedKeyId)) {
                ALOGE("Failed to decode key id(%s)", encodedKeyId.c_str());
                continue;
            }

            if (!decodeBase64String(encodedKey, &decodedKey)) {
                ALOGE("Failed to decode key(%s)", encodedKey.c_str());
                continue;
            }

            keys->insert(std::pair<std::vector<uint8_t>,
                    std::vector<uint8_t> >(decodedKeyId, decodedKey));
        }
    }
    return true;
}

bool JsonWebKey::decodeBase64String(const std::string& encodedText,
        std::vector<uint8_t>* decodedText) {

    decodedText->clear();

    // encodedText should not contain padding characters as per EME spec.
    if (encodedText.find(kBase64Padding) != std::string::npos) {
        return false;
    }

    // Since decodeBase64() requires padding characters,
    // add them so length of encodedText is exactly a multiple of 4.
    int remainder = encodedText.length() % 4;
    std::string paddedText(encodedText);
    if (remainder > 0) {
        for (int i = 0; i < 4 - remainder; ++i) {
            paddedText.append(kBase64Padding);
        }
    }

    sp<Buffer> buffer = decodeBase64(paddedText);
    if (buffer == nullptr) {
        ALOGE("Malformed base64 encoded content found.");
        return false;
    }

    decodedText->insert(decodedText->end(), buffer->base(), buffer->base() + buffer->size());
    return true;
}

bool JsonWebKey::findKey(const std::string& jsonObject, std::string* keyId,
        std::string* encodedKey) {

    std::string key, value;

    // Only allow symmetric key, i.e. "kty":"oct" pair.
    if (jsonObject.find(kKeyTypeTag) != std::string::npos) {
        findValue(kKeyTypeTag, &value);
        if (0 != value.compare(kSymmetricKeyValue))
            return false;
    }

    if (jsonObject.find(kKeyIdTag) != std::string::npos) {
        findValue(kKeyIdTag, keyId);
    }

    if (jsonObject.find(kKeyTag) != std::string::npos) {
        findValue(kKeyTag, encodedKey);
    }
    return true;
}

void JsonWebKey::findValue(const std::string &key, std::string* value) {
    value->clear();
    const char* valueToken;
    for (std::vector<std::string>::const_iterator nextToken = mTokens.begin();
        nextToken != mTokens.end(); ++nextToken) {
        if (0 == (*nextToken).compare(key)) {
            if (nextToken + 1 == mTokens.end())
                break;
            valueToken = (*(nextToken + 1)).c_str();
            value->assign(valueToken);
            nextToken++;
            break;
        }
    }
}

bool JsonWebKey::isJsonWebKeySet(const std::string& jsonObject) const {
    if (jsonObject.find(kKeysTag) == std::string::npos) {
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
bool JsonWebKey::parseJsonObject(const std::string& jsonObject,
        std::vector<std::string>* tokens) {
    jsmn_parser parser;

    jsmn_init(&parser);
    int numTokens = jsmn_parse(&parser,
        jsonObject.c_str(), jsonObject.size(), nullptr, 0);
    if (numTokens < 0) {
        ALOGE("Parser returns error code=%d", numTokens);
        return false;
    }

    unsigned int jsmnTokensSize = numTokens * sizeof(jsmntok_t);
    mJsmnTokens.clear();
    mJsmnTokens.resize(jsmnTokensSize);

    jsmn_init(&parser);
    int status = jsmn_parse(&parser, jsonObject.c_str(),
        jsonObject.size(), mJsmnTokens.data(), numTokens);
    if (status < 0) {
        ALOGE("Parser returns error code=%d", status);
        return false;
    }

    tokens->clear();
    std::string token;
    const char *pjs;
    for (int j = 0; j < numTokens; ++j) {
        pjs = jsonObject.c_str() + mJsmnTokens[j].start;
        if (mJsmnTokens[j].type == JSMN_STRING ||
                mJsmnTokens[j].type == JSMN_PRIMITIVE) {
            token.assign(pjs, mJsmnTokens[j].end - mJsmnTokens[j].start);
            tokens->push_back(token);
        }
    }
    return true;
}

/*
 * Parses JSON Web Key Set string and initializes a vector of JSON objects.
 *
 * @return Returns false for errors, true for success.
 */
bool JsonWebKey::parseJsonWebKeySet(const std::string& jsonWebKeySet,
        std::vector<std::string>* jsonObjects) {
    if (jsonWebKeySet.empty()) {
        ALOGE("Empty JSON Web Key");
        return false;
    }

    // The jsmn parser only supports unicode encoding.
    jsmn_parser parser;

    // Computes number of tokens. A token marks the type, offset in
    // the original string.
    jsmn_init(&parser);
    int numTokens = jsmn_parse(&parser,
            jsonWebKeySet.c_str(), jsonWebKeySet.size(), nullptr, 0);
    if (numTokens < 0) {
        ALOGE("Parser returns error code=%d", numTokens);
        return false;
    }

    unsigned int jsmnTokensSize = numTokens * sizeof(jsmntok_t);
    mJsmnTokens.resize(jsmnTokensSize);

    jsmn_init(&parser);
    int status = jsmn_parse(&parser, jsonWebKeySet.c_str(),
            jsonWebKeySet.size(), mJsmnTokens.data(), numTokens);
    if (status < 0) {
        ALOGE("Parser returns error code=%d", status);
        return false;
    }

    std::string token;
    const char *pjs;
    for (int i = 0; i < numTokens; ++i) {
        pjs = jsonWebKeySet.c_str() + mJsmnTokens[i].start;
        if (mJsmnTokens[i].type == JSMN_OBJECT) {
            token.assign(pjs, mJsmnTokens[i].end - mJsmnTokens[i].start);
            jsonObjects->push_back(token);
        }
    }
    return true;
}

} // namespace clearkey
} // namespace V1_2
} // namespace drm
} // namespace hardware
} // namespace android

