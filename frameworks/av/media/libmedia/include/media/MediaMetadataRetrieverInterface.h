/*
**
** Copyright (C) 2008 The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef ANDROID_MEDIAMETADATARETRIEVERINTERFACE_H
#define ANDROID_MEDIAMETADATARETRIEVERINTERFACE_H

#include <utils/RefBase.h>
#include <media/mediametadataretriever.h>
#include <media/mediascanner.h>
#include <private/media/VideoFrame.h>
#include <media/stagefright/MediaErrors.h>

namespace android {

class DataSource;
struct IMediaHTTPService;

// Abstract base class
class MediaMetadataRetrieverBase : public RefBase
{
public:
                        MediaMetadataRetrieverBase() {}
    virtual             ~MediaMetadataRetrieverBase() {}

    virtual status_t    setDataSource(
            const sp<IMediaHTTPService> &httpService,
            const char *url,
            const KeyedVector<String8, String8> *headers = NULL) = 0;

    virtual status_t    setDataSource(int fd, int64_t offset, int64_t length) = 0;
    virtual status_t    setDataSource(const sp<DataSource>& source, const char *mime) = 0;
    virtual sp<IMemory> getFrameAtTime(
            int64_t timeUs, int option, int colorFormat, bool metaOnly) = 0;
    virtual sp<IMemory> getImageAtIndex(
            int index, int colorFormat, bool metaOnly, bool thumbnail) = 0;
    virtual sp<IMemory> getImageRectAtIndex(
            int index, int colorFormat, int left, int top, int right, int bottom) = 0;
    virtual status_t getFrameAtIndex(
            std::vector<sp<IMemory> >* frames,
            int frameIndex, int numFrames, int colorFormat, bool metaOnly) = 0;
    virtual MediaAlbumArt* extractAlbumArt() = 0;
    virtual const char* extractMetadata(int keyCode) = 0;
};

}; // namespace android

#endif // ANDROID_MEDIAMETADATARETRIEVERINTERFACE_H
