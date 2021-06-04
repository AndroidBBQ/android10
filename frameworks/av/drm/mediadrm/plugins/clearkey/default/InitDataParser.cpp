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
#define LOG_TAG "ClearKeyCryptoPlugin"
#include <utils/Log.h>

#include <endian.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/foundation/base64.h>
#include <media/stagefright/MediaErrors.h>
#include <string.h>

#include "InitDataParser.h"

#include "ClearKeyUUID.h"
#include "MimeType.h"
#include "Utils.h"

namespace clearkeydrm {

using android::AString;
using android::String8;
using android::Vector;

namespace {
    const size_t kKeyIdSize = 16;
    const size_t kSystemIdSize = 16;
}

android::status_t InitDataParser::parse(const Vector<uint8_t>& initData,
        const String8& type,
        Vector<uint8_t>* licenseRequest) {
    // Build a list of the key IDs
    Vector<const uint8_t*> keyIds;
    if (type == kIsoBmffVideoMimeType ||
        type == kIsoBmffAudioMimeType ||
        type == kCencInitDataFormat) {
        android::status_t res = parsePssh(initData, &keyIds);
        if (res != android::OK) {
            return res;
        }
    } else if (type == kWebmVideoMimeType ||
        type == kWebmAudioMimeType ||
        type == kWebmInitDataFormat) {
        // WebM "init data" is just a single key ID
        if (initData.size() != kKeyIdSize) {
            return android::ERROR_DRM_CANNOT_HANDLE;
        }
        keyIds.push(initData.array());
    } else {
        return android::ERROR_DRM_CANNOT_HANDLE;
    }

    // Build the request
    String8 requestJson = generateRequest(keyIds);
    licenseRequest->clear();
    licenseRequest->appendArray(
            reinterpret_cast<const uint8_t*>(requestJson.string()),
            requestJson.size());
    return android::OK;
}

android::status_t InitDataParser::parsePssh(const Vector<uint8_t>& initData,
        Vector<const uint8_t*>* keyIds) {
    // Description of PSSH format:
    // https://w3c.github.io/encrypted-media/format-registry/initdata/cenc.html
    size_t readPosition = 0;

    uint32_t expectedSize = initData.size();
    const char psshIdentifier[4] = {'p', 's', 's', 'h'};
    const uint8_t psshVersion1[4] = {1, 0, 0, 0};
    uint32_t keyIdCount = 0;
    size_t headerSize = sizeof(expectedSize) + sizeof(psshIdentifier) +
                        sizeof(psshVersion1) + kSystemIdSize + sizeof(keyIdCount);
    if (initData.size() < headerSize) {
        return android::ERROR_DRM_CANNOT_HANDLE;
    }

    // Validate size field
    expectedSize = htonl(expectedSize);
    if (memcmp(&initData[readPosition], &expectedSize,
               sizeof(expectedSize)) != 0) {
        return android::ERROR_DRM_CANNOT_HANDLE;
    }
    readPosition += sizeof(expectedSize);

    // Validate PSSH box identifier
    if (memcmp(&initData[readPosition], psshIdentifier,
               sizeof(psshIdentifier)) != 0) {
        return android::ERROR_DRM_CANNOT_HANDLE;
    }
    readPosition += sizeof(psshIdentifier);

    // Validate EME version number
    if (memcmp(&initData[readPosition], psshVersion1,
               sizeof(psshVersion1)) != 0) {
        return android::ERROR_DRM_CANNOT_HANDLE;
    }
    readPosition += sizeof(psshVersion1);

    // Validate system ID
    if (!isClearKeyUUID(&initData[readPosition])) {
        return android::ERROR_DRM_CANNOT_HANDLE;
    }
    readPosition += kSystemIdSize;

    // Read key ID count
    memcpy(&keyIdCount, &initData[readPosition], sizeof(keyIdCount));
    keyIdCount = ntohl(keyIdCount);
    readPosition += sizeof(keyIdCount);

    uint64_t psshSize = 0;
    if (__builtin_mul_overflow(keyIdCount, kKeyIdSize, &psshSize) ||
        __builtin_add_overflow(readPosition, psshSize, &psshSize) ||
        psshSize != initData.size() - sizeof(uint32_t) /* DataSize(0) */) {
        return android::ERROR_DRM_CANNOT_HANDLE;
    }

    // Calculate the key ID offsets
    for (uint32_t i = 0; i < keyIdCount; ++i) {
        size_t keyIdPosition = readPosition + (i * kKeyIdSize);
        keyIds->push(&initData[keyIdPosition]);
    }
    return android::OK;
}

String8 InitDataParser::generateRequest(const Vector<const uint8_t*>& keyIds) {
    const String8 kRequestPrefix("{\"kids\":[");
    const String8 kRequestSuffix("],\"type\":\"temporary\"}");
    const String8 kBase64Padding("=");

    String8 request(kRequestPrefix);
    AString encodedId;
    for (size_t i = 0; i < keyIds.size(); ++i) {
        encodedId.clear();
        android::encodeBase64Url(keyIds[i], kKeyIdSize, &encodedId);
        if (i != 0) {
            request.append(",");
        }
        request.appendFormat("\"%s\"", encodedId.c_str());
    }
    request.append(kRequestSuffix);

    // Android's Base64 encoder produces padding. EME forbids padding.
    request.removeAll(kBase64Padding);
    return request;
}

} // namespace clearkeydrm
