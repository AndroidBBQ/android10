/*
 * Copyright (C) 2009 The Android Open Source Project
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
#define LOG_TAG "StagefrightMediaScanner"
#include <utils/Log.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <media/stagefright/MediaExtractorFactory.h>
#include <media/stagefright/StagefrightMediaScanner.h>

#include <media/IMediaHTTPService.h>
#include <media/mediametadataretriever.h>
#include <private/media/VideoFrame.h>

namespace android {

StagefrightMediaScanner::StagefrightMediaScanner() {}

StagefrightMediaScanner::~StagefrightMediaScanner() {}

static std::unordered_set<std::string> gSupportedExtensions;

static bool FileHasAcceptableExtension(const char *extension) {

    if (gSupportedExtensions.empty()) {
        // get the list from the service
        gSupportedExtensions = MediaExtractorFactory::getSupportedTypes();
    }

    return  gSupportedExtensions.count(std::string(extension + 1)) != 0;
}

MediaScanResult StagefrightMediaScanner::processFile(
        const char *path, const char *mimeType,
        MediaScannerClient &client) {
    ALOGV("processFile '%s'.", path);

    client.setLocale(locale());
    client.beginFile();
    MediaScanResult result = processFileInternal(path, mimeType, client);
    ALOGV("result: %d", result);
    if (mimeType == NULL && result != MEDIA_SCAN_RESULT_OK) {
        ALOGW("media scan failed for %s", path);
        client.setMimeType("application/octet-stream");
    }
    client.endFile();
    return result;
}

MediaScanResult StagefrightMediaScanner::processFileInternal(
        const char *path, const char * /* mimeType */,
        MediaScannerClient &client) {
    const char *extension = strrchr(path, '.');

    if (!extension) {
        return MEDIA_SCAN_RESULT_SKIPPED;
    }

    if (!FileHasAcceptableExtension(extension)) {
        return MEDIA_SCAN_RESULT_SKIPPED;
    }

    sp<MediaMetadataRetriever> mRetriever(new MediaMetadataRetriever);

    int fd = open(path, O_RDONLY | O_LARGEFILE);
    status_t status;
    if (fd < 0) {
        // couldn't open it locally, maybe the media server can?
        sp<IMediaHTTPService> nullService;
        status = mRetriever->setDataSource(nullService, path);
    } else {
        status = mRetriever->setDataSource(fd, 0, 0x7ffffffffffffffL);
        close(fd);
    }

    if (status) {
        return MEDIA_SCAN_RESULT_ERROR;
    }

    const char *value;
    if ((value = mRetriever->extractMetadata(
                    METADATA_KEY_MIMETYPE)) != NULL) {
        status = client.setMimeType(value);
        if (status) {
            return MEDIA_SCAN_RESULT_ERROR;
        }
    }

    struct KeyMap {
        const char *tag;
        int key;
    };
    static const KeyMap kKeyMap[] = {
        { "tracknumber", METADATA_KEY_CD_TRACK_NUMBER },
        { "discnumber", METADATA_KEY_DISC_NUMBER },
        { "album", METADATA_KEY_ALBUM },
        { "artist", METADATA_KEY_ARTIST },
        { "albumartist", METADATA_KEY_ALBUMARTIST },
        { "composer", METADATA_KEY_COMPOSER },
        { "genre", METADATA_KEY_GENRE },
        { "title", METADATA_KEY_TITLE },
        { "year", METADATA_KEY_YEAR },
        { "duration", METADATA_KEY_DURATION },
        { "writer", METADATA_KEY_WRITER },
        { "compilation", METADATA_KEY_COMPILATION },
        { "isdrm", METADATA_KEY_IS_DRM },
        { "date", METADATA_KEY_DATE },
        { "width", METADATA_KEY_VIDEO_WIDTH },
        { "height", METADATA_KEY_VIDEO_HEIGHT },
        { "colorstandard", METADATA_KEY_COLOR_STANDARD },
        { "colortransfer", METADATA_KEY_COLOR_TRANSFER },
        { "colorrange", METADATA_KEY_COLOR_RANGE },
        { "samplerate", METADATA_KEY_SAMPLERATE },
        { "bitspersample", METADATA_KEY_BITS_PER_SAMPLE },
    };
    static const size_t kNumEntries = sizeof(kKeyMap) / sizeof(kKeyMap[0]);

    for (size_t i = 0; i < kNumEntries; ++i) {
        const char *value;
        if ((value = mRetriever->extractMetadata(kKeyMap[i].key)) != NULL) {
            status = client.addStringTag(kKeyMap[i].tag, value);
            if (status != OK) {
                return MEDIA_SCAN_RESULT_ERROR;
            }
        }
    }

    return MEDIA_SCAN_RESULT_OK;
}

MediaAlbumArt *StagefrightMediaScanner::extractAlbumArt(int fd) {
    ALOGV("extractAlbumArt %d", fd);

    off64_t size = lseek64(fd, 0, SEEK_END);
    if (size < 0) {
        return NULL;
    }
    lseek64(fd, 0, SEEK_SET);

    sp<MediaMetadataRetriever> mRetriever(new MediaMetadataRetriever);
    if (mRetriever->setDataSource(fd, 0, size) == OK) {
        sp<IMemory> mem = mRetriever->extractAlbumArt();
        if (mem != NULL) {
            MediaAlbumArt *art = static_cast<MediaAlbumArt *>(mem->pointer());
            return art->clone();
        }
    }

    return NULL;
}

}  // namespace android
