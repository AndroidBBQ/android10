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

//#define LOG_NDEBUG 0
#define LOG_TAG "ClearMediaHTTP"
#include <utils/Log.h>

#include <media/stagefright/ClearMediaHTTP.h>

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/Utils.h>

#include <media/MediaHTTPConnection.h>

namespace android {

ClearMediaHTTP::ClearMediaHTTP(const sp<MediaHTTPConnection> &conn)
    : mInitCheck((conn != NULL) ? OK : NO_INIT),
      mHTTPConnection(conn),
      mCachedSizeValid(false),
      mCachedSize(0ll) {
}

ClearMediaHTTP::~ClearMediaHTTP() {
}

status_t ClearMediaHTTP::connect(
        const char *uri,
        const KeyedVector<String8, String8> *headers,
        off64_t /* offset */) {
    if (mInitCheck != OK) {
        return mInitCheck;
    }

    KeyedVector<String8, String8> extHeaders;
    if (headers != NULL) {
        extHeaders = *headers;
    }

    if (extHeaders.indexOfKey(String8("User-Agent")) < 0) {
        extHeaders.add(String8("User-Agent"), String8(MakeUserAgent().c_str()));
    }

    mLastURI = uri;
    // reconnect() calls with uri == old mLastURI.c_str(), which gets zapped
    // as part of the above assignment. Ensure no accidental later use.
    uri = NULL;

    bool success = mHTTPConnection->connect(mLastURI.c_str(), &extHeaders);

    mLastHeaders = extHeaders;

    mCachedSizeValid = false;

    if (success) {
        AString sanitized = uriDebugString(mLastURI);
        mName = String8::format("ClearMediaHTTP(%s)", sanitized.c_str());
    }

    return success ? OK : UNKNOWN_ERROR;
}

void ClearMediaHTTP::close() {
    disconnect();
}

void ClearMediaHTTP::disconnect() {
    mName = String8("ClearMediaHTTP(<disconnected>)");
    if (mInitCheck != OK) {
        return;
    }

    mHTTPConnection->disconnect();
}

status_t ClearMediaHTTP::initCheck() const {
    return mInitCheck;
}

ssize_t ClearMediaHTTP::readAt(off64_t offset, void *data, size_t size) {
    if (mInitCheck != OK) {
        return mInitCheck;
    }

    int64_t startTimeUs = ALooper::GetNowUs();

    size_t numBytesRead = 0;
    while (numBytesRead < size) {
        size_t copy = size - numBytesRead;

        if (copy > 64 * 1024) {
            // limit the buffer sizes transferred across binder boundaries
            // to avoid spurious transaction failures.
            copy = 64 * 1024;
        }

        ssize_t n = mHTTPConnection->readAt(
                offset + numBytesRead, (uint8_t *)data + numBytesRead, copy);

        if (n < 0) {
            return n;
        } else if (n == 0) {
            break;
        }

        numBytesRead += n;
    }

    int64_t delayUs = ALooper::GetNowUs() - startTimeUs;

    addBandwidthMeasurement(numBytesRead, delayUs);

    return numBytesRead;
}

status_t ClearMediaHTTP::getSize(off64_t *size) {
    if (mInitCheck != OK) {
        return mInitCheck;
    }

    // Caching the returned size so that it stays valid even after a
    // disconnect. NuCachedSource2 relies on this.

    if (!mCachedSizeValid) {
        mCachedSize = mHTTPConnection->getSize();
        mCachedSizeValid = true;
    }

    *size = mCachedSize;

    return *size < 0 ? *size : static_cast<status_t>(OK);
}

uint32_t ClearMediaHTTP::flags() {
    return kWantsPrefetching | kIsHTTPBasedSource;
}

status_t ClearMediaHTTP::reconnectAtOffset(off64_t offset) {
    return connect(mLastURI.c_str(), &mLastHeaders, offset);
}


String8 ClearMediaHTTP::getUri() {
    if (mInitCheck != OK) {
        return String8::empty();
    }

    String8 uri;
    if (OK == mHTTPConnection->getUri(&uri)) {
        return uri;
    }
    return String8(mLastURI.c_str());
}

String8 ClearMediaHTTP::getMIMEType() const {
    if (mInitCheck != OK) {
        return String8("application/octet-stream");
    }

    String8 mimeType;
    status_t err = mHTTPConnection->getMIMEType(&mimeType);

    if (err != OK) {
        return String8("application/octet-stream");
    }

    return mimeType;
}

}  // namespace android
