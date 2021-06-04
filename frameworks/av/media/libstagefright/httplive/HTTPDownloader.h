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

#ifndef HTTP_DOWNLOADER_H_

#define HTTP_DOWNLOADER_H_

#include <media/stagefright/foundation/ADebug.h>
#include <utils/KeyedVector.h>
#include <utils/Mutex.h>
#include <utils/RefBase.h>

namespace android {

struct ABuffer;
class DataSource;
struct HTTPBase;
struct MediaHTTPService;
struct M3UParser;

struct HTTPDownloader : public RefBase {
    HTTPDownloader(
            const sp<MediaHTTPService> &httpService,
            const KeyedVector<String8, String8> &headers);

    void reconnect();
    void disconnect();
    bool isDisconnecting();
    // If given a non-zero block_size (default 0), it is used to cap the number of
    // bytes read in from the DataSource. If given a non-NULL buffer, new content
    // is read into the end.
    //
    // The DataSource we read from is responsible for signaling error or EOF to help us
    // break out of the read loop. The DataSource can be returned to the caller, so
    // that the caller can reuse it for subsequent fetches (within the initially
    // requested range).
    //
    // For reused HTTP sources, the caller must download a file sequentially without
    // any overlaps or gaps to prevent reconnection.
    ssize_t fetchBlock(
            const char *url,
            sp<ABuffer> *out,
            int64_t range_offset, /* open file at range_offset */
            int64_t range_length, /* open file for range_length (-1: entire file) */
            uint32_t block_size,  /* download block size (0: entire range) */
            String8 *actualUrl,   /* returns actual URL */
            bool reconnect        /* force connect http */
            );

    // simplified version to fetch a single file
    ssize_t fetchFile(
            const char *url,
            sp<ABuffer> *out,
            String8 *actualUrl = NULL);

    // fetch a playlist file
    sp<M3UParser> fetchPlaylist(
            const char *url, uint8_t *curPlaylistHash, bool *unchanged);

private:
    sp<HTTPBase> mHTTPDataSource;
    sp<DataSource> mDataSource;
    KeyedVector<String8, String8> mExtraHeaders;

    Mutex mLock;
    bool mDisconnecting;

    DISALLOW_EVIL_CONSTRUCTORS(HTTPDownloader);
};

}  // namespace android

#endif  // HTTP_DOWNLOADER_H_
