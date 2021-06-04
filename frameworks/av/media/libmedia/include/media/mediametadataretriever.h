/*
 * Copyright (C) 2008 The Android Open Source Project
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


#ifndef MEDIAMETADATARETRIEVER_H
#define MEDIAMETADATARETRIEVER_H

#include <utils/Errors.h>  // for status_t
#include <utils/threads.h>
#include <binder/IMemory.h>
#include <media/IMediaMetadataRetriever.h>

namespace android {

class IDataSource;
struct IMediaHTTPService;
class IMediaPlayerService;
class IMediaMetadataRetriever;

// Keep these in synch with the constants defined in MediaMetadataRetriever.java
// class.
enum {
    METADATA_KEY_CD_TRACK_NUMBER = 0,
    METADATA_KEY_ALBUM           = 1,
    METADATA_KEY_ARTIST          = 2,
    METADATA_KEY_AUTHOR          = 3,
    METADATA_KEY_COMPOSER        = 4,
    METADATA_KEY_DATE            = 5,
    METADATA_KEY_GENRE           = 6,
    METADATA_KEY_TITLE           = 7,
    METADATA_KEY_YEAR            = 8,
    METADATA_KEY_DURATION        = 9,
    METADATA_KEY_NUM_TRACKS      = 10,
    METADATA_KEY_WRITER          = 11,
    METADATA_KEY_MIMETYPE        = 12,
    METADATA_KEY_ALBUMARTIST     = 13,
    METADATA_KEY_DISC_NUMBER     = 14,
    METADATA_KEY_COMPILATION     = 15,
    METADATA_KEY_HAS_AUDIO       = 16,
    METADATA_KEY_HAS_VIDEO       = 17,
    METADATA_KEY_VIDEO_WIDTH     = 18,
    METADATA_KEY_VIDEO_HEIGHT    = 19,
    METADATA_KEY_BITRATE         = 20,
    METADATA_KEY_TIMED_TEXT_LANGUAGES      = 21,
    METADATA_KEY_IS_DRM          = 22,
    METADATA_KEY_LOCATION        = 23,
    METADATA_KEY_VIDEO_ROTATION  = 24,
    METADATA_KEY_CAPTURE_FRAMERATE = 25,
    METADATA_KEY_HAS_IMAGE       = 26,
    METADATA_KEY_IMAGE_COUNT     = 27,
    METADATA_KEY_IMAGE_PRIMARY   = 28,
    METADATA_KEY_IMAGE_WIDTH     = 29,
    METADATA_KEY_IMAGE_HEIGHT    = 30,
    METADATA_KEY_IMAGE_ROTATION  = 31,
    METADATA_KEY_VIDEO_FRAME_COUNT  = 32,
    METADATA_KEY_EXIF_OFFSET     = 33,
    METADATA_KEY_EXIF_LENGTH     = 34,
    METADATA_KEY_COLOR_STANDARD  = 35,
    METADATA_KEY_COLOR_TRANSFER  = 36,
    METADATA_KEY_COLOR_RANGE     = 37,
    METADATA_KEY_SAMPLERATE      = 38,
    METADATA_KEY_BITS_PER_SAMPLE = 39,

    // Add more here...
};

class MediaMetadataRetriever: public RefBase
{
public:
    MediaMetadataRetriever();
    ~MediaMetadataRetriever();
    void disconnect();

    status_t setDataSource(
            const sp<IMediaHTTPService> &httpService,
            const char *dataSourceUrl,
            const KeyedVector<String8, String8> *headers = NULL);

    status_t setDataSource(int fd, int64_t offset, int64_t length);
    status_t setDataSource(
            const sp<IDataSource>& dataSource, const char *mime = NULL);
    sp<IMemory> getFrameAtTime(int64_t timeUs, int option,
            int colorFormat = HAL_PIXEL_FORMAT_RGB_565, bool metaOnly = false);
    sp<IMemory> getImageAtIndex(int index,
            int colorFormat = HAL_PIXEL_FORMAT_RGB_565, bool metaOnly = false, bool thumbnail = false);
    sp<IMemory> getImageRectAtIndex(
            int index, int colorFormat, int left, int top, int right, int bottom);
    status_t getFrameAtIndex(
            std::vector<sp<IMemory> > *frames, int frameIndex, int numFrames = 1,
            int colorFormat = HAL_PIXEL_FORMAT_RGB_565, bool metaOnly = false);
    sp<IMemory> extractAlbumArt();
    const char* extractMetadata(int keyCode);

private:
    static const sp<IMediaPlayerService> getService();

    class DeathNotifier: public IBinder::DeathRecipient
    {
    public:
        DeathNotifier() {}
        virtual ~DeathNotifier();
        virtual void binderDied(const wp<IBinder>& who);
    };

    static sp<DeathNotifier>                  sDeathNotifier;
    static Mutex                              sServiceLock;
    static sp<IMediaPlayerService>            sService;

    Mutex                                     mLock;
    sp<IMediaMetadataRetriever>               mRetriever;

};

}; // namespace android

#endif // MEDIAMETADATARETRIEVER_H
