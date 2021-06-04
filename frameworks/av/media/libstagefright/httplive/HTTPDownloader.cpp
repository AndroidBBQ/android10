/*
 * Copyright 2015 The Android Open Source Project
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
#define LOG_TAG "HTTPDownloader"
#include <utils/Log.h>

#include "HTTPDownloader.h"
#include "M3UParser.h"

#include <media/DataSource.h>
#include <media/MediaHTTPConnection.h>
#include <media/MediaHTTPService.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/ClearMediaHTTP.h>
#include <media/stagefright/ClearFileSource.h>
#include <openssl/aes.h>
#include <openssl/md5.h>
#include <utils/Mutex.h>
#include <inttypes.h>

namespace android {

HTTPDownloader::HTTPDownloader(
        const sp<MediaHTTPService> &httpService,
        const KeyedVector<String8, String8> &headers) :
    mHTTPDataSource(new ClearMediaHTTP(httpService->makeHTTPConnection())),
    mExtraHeaders(headers),
    mDisconnecting(false) {
}

void HTTPDownloader::reconnect() {
    AutoMutex _l(mLock);
    mDisconnecting = false;
}

void HTTPDownloader::disconnect() {
    {
        AutoMutex _l(mLock);
        mDisconnecting = true;
    }
    mHTTPDataSource->disconnect();
}

bool HTTPDownloader::isDisconnecting() {
    AutoMutex _l(mLock);
    return mDisconnecting;
}

/*
 * Illustration of parameters:
 *
 * 0      `range_offset`
 * +------------+-------------------------------------------------------+--+--+
 * |            |                                 | next block to fetch |  |  |
 * |            | `source` handle => `out` buffer |                     |  |  |
 * | `url` file |<--------- buffer size --------->|<--- `block_size` -->|  |  |
 * |            |<----------- `range_length` / buffer capacity ----------->|  |
 * |<------------------------------ file_size ------------------------------->|
 *
 * Special parameter values:
 * - range_length == -1 means entire file
 * - block_size == 0 means entire range
 *
 */
ssize_t HTTPDownloader::fetchBlock(
        const char *url, sp<ABuffer> *out,
        int64_t range_offset, int64_t range_length,
        uint32_t block_size, /* download block size */
        String8 *actualUrl,
        bool reconnect /* force connect HTTP when resuing source */) {
    if (isDisconnecting()) {
        return ERROR_NOT_CONNECTED;
    }

    off64_t size;

    if (reconnect) {
        if (!strncasecmp(url, "file://", 7)) {
            mDataSource = new ClearFileSource(url + 7);
        } else if (strncasecmp(url, "http://", 7)
                && strncasecmp(url, "https://", 8)) {
            return ERROR_UNSUPPORTED;
        } else {
            KeyedVector<String8, String8> headers = mExtraHeaders;
            if (range_offset > 0 || range_length >= 0) {
                headers.add(
                        String8("Range"),
                        String8(
                            AStringPrintf(
                                "bytes=%lld-%s",
                                range_offset,
                                range_length < 0
                                    ? "" : AStringPrintf("%lld",
                                            range_offset + range_length - 1).c_str()).c_str()));
            }

            status_t err = mHTTPDataSource->connect(url, &headers);

            if (isDisconnecting()) {
                return ERROR_NOT_CONNECTED;
            }

            if (err != OK) {
                return err;
            }

            mDataSource = mHTTPDataSource;
        }
    }

    status_t getSizeErr = mDataSource->getSize(&size);

    if (isDisconnecting()) {
        return ERROR_NOT_CONNECTED;
    }

    if (getSizeErr != OK) {
        size = 65536;
    }

    sp<ABuffer> buffer = *out != NULL ? *out : new ABuffer(size);
    if (*out == NULL) {
        buffer->setRange(0, 0);
    }

    ssize_t bytesRead = 0;
    // adjust range_length if only reading partial block
    if (block_size > 0 && (range_length == -1 || (int64_t)(buffer->size() + block_size) < range_length)) {
        range_length = buffer->size() + block_size;
    }
    for (;;) {
        // Only resize when we don't know the size.
        size_t bufferRemaining = buffer->capacity() - buffer->size();
        if (bufferRemaining == 0 && getSizeErr != OK) {
            size_t bufferIncrement = buffer->size() / 2;
            if (bufferIncrement < 32768) {
                bufferIncrement = 32768;
            }
            bufferRemaining = bufferIncrement;

            ALOGV("increasing download buffer to %zu bytes",
                 buffer->size() + bufferRemaining);

            sp<ABuffer> copy = new ABuffer(buffer->size() + bufferRemaining);
            if (copy->data() == NULL) {
                android_errorWriteLog(0x534e4554, "68399439");
                ALOGE("not enough memory to download: requesting %zu + %zu",
                        buffer->size(), bufferRemaining);
                return NO_MEMORY;
            }
            memcpy(copy->data(), buffer->data(), buffer->size());
            copy->setRange(0, buffer->size());

            buffer = copy;
        }

        size_t maxBytesToRead = bufferRemaining;
        if (range_length >= 0) {
            int64_t bytesLeftInRange = range_length - buffer->size();
            if (bytesLeftInRange < 0) {
                ALOGE("range_length %" PRId64 " wrapped around", range_length);
                return ERROR_OUT_OF_RANGE;
            } else if (bytesLeftInRange < (int64_t)maxBytesToRead) {
                maxBytesToRead = bytesLeftInRange;

                if (bytesLeftInRange == 0) {
                    break;
                }
            }
        }

        // The DataSource is responsible for informing us of error (n < 0) or eof (n == 0)
        // to help us break out of the loop.
        ssize_t n = mDataSource->readAt(
                buffer->size(), buffer->data() + buffer->size(),
                maxBytesToRead);

        if (isDisconnecting()) {
            return ERROR_NOT_CONNECTED;
        }

        if (n < 0) {
            return n;
        }

        if (n == 0) {
            break;
        }

        buffer->setRange(0, buffer->size() + (size_t)n);
        bytesRead += n;
    }

    *out = buffer;
    if (actualUrl != NULL) {
        *actualUrl = mDataSource->getUri();
        if (actualUrl->isEmpty()) {
            *actualUrl = url;
        }
    }

    return bytesRead;
}

ssize_t HTTPDownloader::fetchFile(
        const char *url, sp<ABuffer> *out, String8 *actualUrl) {
    ssize_t err = fetchBlock(url, out, 0, -1, 0, actualUrl, true /* reconnect */);

    // close off the connection after use
    mHTTPDataSource->disconnect();

    return err;
}

sp<M3UParser> HTTPDownloader::fetchPlaylist(
        const char *url, uint8_t *curPlaylistHash, bool *unchanged) {
    ALOGV("fetchPlaylist '%s'", url);

    *unchanged = false;

    sp<ABuffer> buffer;
    String8 actualUrl;
    ssize_t err = fetchFile(url, &buffer, &actualUrl);

    // close off the connection after use
    mHTTPDataSource->disconnect();

    if (err <= 0) {
        return NULL;
    }

    // MD5 functionality is not available on the simulator, treat all
    // playlists as changed.

#if defined(__ANDROID__)
    uint8_t hash[16];

    MD5_CTX m;
    MD5_Init(&m);
    MD5_Update(&m, buffer->data(), buffer->size());

    MD5_Final(hash, &m);

    if (curPlaylistHash != NULL && !memcmp(hash, curPlaylistHash, 16)) {
        // playlist unchanged
        *unchanged = true;

        return NULL;
    }
#endif

    sp<M3UParser> playlist =
        new M3UParser(actualUrl.string(), buffer->data(), buffer->size());

    if (playlist->initCheck() != OK) {
        ALOGE("failed to parse .m3u8 playlist");

        return NULL;
    }

#if defined(__ANDROID__)
    if (curPlaylistHash != NULL) {

        memcpy(curPlaylistHash, hash, sizeof(hash));
    }
#endif

    return playlist;
}

}  // namespace android
