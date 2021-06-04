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
#define LOG_TAG "DataSource"

#include "include/HTTPBase.h"
#include "include/NuCachedSource2.h"

#include <media/MediaHTTPConnection.h>
#include <media/MediaHTTPService.h>
#include <media/stagefright/DataSourceFactory.h>
#include <media/stagefright/DataURISource.h>
#include <media/stagefright/FileSource.h>
#include <media/stagefright/MediaHTTP.h>
#include <utils/String8.h>

namespace android {

// static
sp<DataSource> DataSourceFactory::CreateFromURI(
        const sp<MediaHTTPService> &httpService,
        const char *uri,
        const KeyedVector<String8, String8> *headers,
        String8 *contentType,
        HTTPBase *httpSource) {
    if (contentType != NULL) {
        *contentType = "";
    }

    sp<DataSource> source;
    if (!strncasecmp("file://", uri, 7)) {
        source = new FileSource(uri + 7);
    } else if (!strncasecmp("http://", uri, 7) || !strncasecmp("https://", uri, 8)) {
        if (httpService == NULL) {
            ALOGE("Invalid http service!");
            return NULL;
        }

        if (httpSource == NULL) {
            sp<MediaHTTPConnection> conn = httpService->makeHTTPConnection();
            if (conn == NULL) {
                ALOGE("Failed to make http connection from http service!");
                return NULL;
            }
            httpSource = new MediaHTTP(conn);
        }

        String8 cacheConfig;
        bool disconnectAtHighwatermark = false;
        KeyedVector<String8, String8> nonCacheSpecificHeaders;
        if (headers != NULL) {
            nonCacheSpecificHeaders = *headers;
            NuCachedSource2::RemoveCacheSpecificHeaders(
                    &nonCacheSpecificHeaders,
                    &cacheConfig,
                    &disconnectAtHighwatermark);
        }

        if (httpSource->connect(uri, &nonCacheSpecificHeaders) != OK) {
            ALOGE("Failed to connect http source!");
            return NULL;
        }

        if (contentType != NULL) {
            *contentType = httpSource->getMIMEType();
        }

        source = NuCachedSource2::Create(
                httpSource,
                cacheConfig.isEmpty() ? NULL : cacheConfig.string(),
                disconnectAtHighwatermark);
    } else if (!strncasecmp("data:", uri, 5)) {
        source = DataURISource::Create(uri);
    } else {
        // Assume it's a filename.
        source = new FileSource(uri);
    }

    if (source == NULL || source->initCheck() != OK) {
        return NULL;
    }

    return source;
}

sp<DataSource> DataSourceFactory::CreateFromFd(int fd, int64_t offset, int64_t length) {
    sp<FileSource> source = new FileSource(fd, offset, length);
    return source->initCheck() != OK ? nullptr : source;
}

sp<DataSource> DataSourceFactory::CreateMediaHTTP(const sp<MediaHTTPService> &httpService) {
    if (httpService == NULL) {
        return NULL;
    }

    sp<MediaHTTPConnection> conn = httpService->makeHTTPConnection();
    if (conn == NULL) {
        return NULL;
    } else {
        return new MediaHTTP(conn);
    }
}

}  // namespace android
