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
#include <media/stagefright/DataURISource.h>

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/foundation/base64.h>

namespace android {

// static
sp<DataURISource> DataURISource::Create(const char *uri) {
    if (strncasecmp("data:", uri, 5)) {
        return NULL;
    }

    const char *commaPos = strrchr(uri, ',');

    if (commaPos == NULL) {
        return NULL;
    }

    sp<ABuffer> buffer;

    AString tmp(&uri[5], commaPos - &uri[5]);

    if (tmp.endsWith(";base64")) {

        // strip all CR and LF characters.
        const char *src = commaPos+1;
        int len = strlen(src) + 1;
        char *cleansed = (char *) malloc(len);
        if (cleansed == NULL) return NULL;
        char *keeping = cleansed;
        int left = len;
        for (int i = 0; i < len ; i++)
        {
            const char c = *src++;
            if (c == '\r' || c == '\n') {
                continue;
            }
            *keeping++ = c;
            left--;
        }
        memset(keeping, 0, left);

        AString encoded(cleansed);
        free(cleansed);

        buffer = decodeBase64(encoded);

        if (buffer == NULL) {
            ALOGE("Malformed base64 encoded content found.");
            return NULL;
        }
    } else {
#if 0
        size_t dataLen = strlen(uri) - tmp.size() - 6;
        buffer = new ABuffer(dataLen);
        memcpy(buffer->data(), commaPos + 1, dataLen);

        // unescape
#else
        // MediaPlayer doesn't care for this right now as we don't
        // play any text-based media.
        return NULL;
#endif
    }

    // We don't really care about charset or mime type.

    return new DataURISource(buffer);
}

DataURISource::DataURISource(const sp<ABuffer> &buffer)
    : mBuffer(buffer) {
}

DataURISource::~DataURISource() {
}

status_t DataURISource::initCheck() const {
    return OK;
}

ssize_t DataURISource::readAt(off64_t offset, void *data, size_t size) {
    if ((offset < 0) || (offset >= (off64_t)mBuffer->size())) {
        return 0;
    }

    size_t copy = mBuffer->size() - offset;
    if (copy > size) {
        copy = size;
    }

    memcpy(data, mBuffer->data() + offset, copy);

    return copy;
}

status_t DataURISource::getSize(off64_t *size) {
    *size = mBuffer->size();

    return OK;
}

}  // namespace android

