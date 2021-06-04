
/*
 * Copyright (C) 2017 The Android Open Source Project
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
#define LOG_TAG "JsonAssetLoader"

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/foundation/base64.h>
#include <media/stagefright/MediaErrors.h>
#include <utils/Log.h>

#include "JsonAssetLoader.h"
#include "protos/license_protos.pb.h"

namespace android {
namespace clearkeycas {

const String8 kIdTag("id");
const String8 kNameTag("name");
const String8 kLowerCaseOgranizationNameTag("lowercase_organization_name");
const String8 kEncryptionKeyTag("encryption_key");
const String8 kCasTypeTag("cas_type");
const String8 kBase64Padding("=");

JsonAssetLoader::JsonAssetLoader() {
}

JsonAssetLoader::~JsonAssetLoader() {
}

/*
 * Extract a clear key asset from a JSON string.
 *
 * Returns OK if a clear key asset is extracted successfully,
 * or ERROR_CAS_NO_LICENSE if the string doesn't contain a valid
 * clear key asset.
 */
status_t JsonAssetLoader::extractAssetFromString(
        const String8& jsonAssetString, Asset *asset) {
    if (!parseJsonAssetString(jsonAssetString, &mJsonObjects)) {
        return ERROR_CAS_NO_LICENSE;
    }

    if (mJsonObjects.size() < 1) {
        return ERROR_CAS_NO_LICENSE;
    }

    if (!parseJsonObject(mJsonObjects[0], &mTokens))
        return ERROR_CAS_NO_LICENSE;

    if (!findKey(mJsonObjects[0], asset)) {
        return ERROR_CAS_NO_LICENSE;
    }
    return OK;
}

//static
sp<ABuffer> JsonAssetLoader::decodeBase64String(const String8& encodedText) {
    // Since android::decodeBase64() requires padding characters,
    // add them so length of encodedText is exactly a multiple of 4.
    int remainder = encodedText.length() % 4;
    String8 paddedText(encodedText);
    if (remainder > 0) {
        for (int i = 0; i < 4 - remainder; ++i) {
            paddedText.append(kBase64Padding);
        }
    }

    return decodeBase64(AString(paddedText.string()));
}

bool JsonAssetLoader::findKey(const String8& jsonObject, Asset *asset) {

    String8 value;

    if (jsonObject.find(kIdTag) < 0) {
        return false;
    }
    findValue(kIdTag, &value);
    ALOGV("found %s=%s", kIdTag.string(), value.string());
    asset->set_id(atoi(value.string()));

    if (jsonObject.find(kNameTag) < 0) {
        return false;
    }
    findValue(kNameTag, &value);
    ALOGV("found %s=%s", kNameTag.string(), value.string());
    asset->set_name(value.string());

    if (jsonObject.find(kLowerCaseOgranizationNameTag) < 0) {
        return false;
    }
    findValue(kLowerCaseOgranizationNameTag, &value);
    ALOGV("found %s=%s", kLowerCaseOgranizationNameTag.string(), value.string());
    asset->set_lowercase_organization_name(value.string());

    if (jsonObject.find(kCasTypeTag) < 0) {
        return false;
    }
    findValue(kCasTypeTag, &value);
    ALOGV("found %s=%s", kCasTypeTag.string(), value.string());
    // Asset_CasType_CLEARKEY_CAS = 1
    asset->set_cas_type((Asset_CasType)atoi(value.string()));

    return true;
}

void JsonAssetLoader::findValue(const String8 &key, String8* value) {
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

/*
 * Parses a JSON objects string and initializes a vector of tokens.
 *
 * @return Returns false for errors, true for success.
 */
bool JsonAssetLoader::parseJsonObject(const String8& jsonObject,
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
    ALOGV("numTokens: %d", numTokens);
    for (int j = 0; j < numTokens; ++j) {
        pjs = jsonObject.string() + mJsmnTokens[j].start;
        if (mJsmnTokens[j].type == JSMN_STRING ||
                mJsmnTokens[j].type == JSMN_PRIMITIVE) {
            token.setTo(pjs, mJsmnTokens[j].end - mJsmnTokens[j].start);
            tokens->add(token);
            ALOGV("add token: %s", token.string());
        }
    }
    return true;
}

/*
 * Parses JSON asset string and initializes a vector of JSON objects.
 *
 * @return Returns false for errors, true for success.
 */
bool JsonAssetLoader::parseJsonAssetString(const String8& jsonAsset,
        Vector<String8>* jsonObjects) {
    if (jsonAsset.isEmpty()) {
        ALOGE("Empty JSON Web Key");
        return false;
    }

    // The jsmn parser only supports unicode encoding.
    jsmn_parser parser;

    // Computes number of tokens. A token marks the type, offset in
    // the original string.
    jsmn_init(&parser);
    int numTokens = jsmn_parse(&parser,
            jsonAsset.string(), jsonAsset.size(), NULL, 0);
    if (numTokens < 0) {
        ALOGE("Parser returns error code=%d", numTokens);
        return false;
    }

    unsigned int jsmnTokensSize = numTokens * sizeof(jsmntok_t);
    mJsmnTokens.setCapacity(jsmnTokensSize);

    jsmn_init(&parser);
    int status = jsmn_parse(&parser, jsonAsset.string(),
            jsonAsset.size(), mJsmnTokens.editArray(), numTokens);
    if (status < 0) {
        ALOGE("Parser returns error code=%d", status);
        return false;
    }

    String8 token;
    const char *pjs;
    for (int i = 0; i < numTokens; ++i) {
        pjs = jsonAsset.string() + mJsmnTokens[i].start;
        if (mJsmnTokens[i].type == JSMN_OBJECT) {
            token.setTo(pjs, mJsmnTokens[i].end - mJsmnTokens[i].start);
            jsonObjects->add(token);
        }
    }
    return true;
}

}  // namespace clearkeycas
}  // namespace android
