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

#ifndef ANDROID_MEDIAMETADATARETRIEVERSERVICE_H
#define ANDROID_MEDIAMETADATARETRIEVERSERVICE_H

#include <utils/Log.h>
#include <utils/threads.h>
#include <utils/List.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <binder/IMemory.h>

#include <media/MediaMetadataRetrieverInterface.h>


namespace android {

struct IMediaHTTPService;
class IMediaPlayerService;
class MemoryDealer;

class MetadataRetrieverClient : public BnMediaMetadataRetriever
{
public:
    MetadataRetrieverClient(const sp<IMediaPlayerService>& service, pid_t pid, int32_t connId);

    // Implements IMediaMetadataRetriever interface
    // These methods are called in IMediaMetadataRetriever.cpp?
    virtual void                    disconnect();

    virtual status_t                setDataSource(
            const sp<IMediaHTTPService> &httpService,
            const char *url,
            const KeyedVector<String8, String8> *headers);

    virtual status_t                setDataSource(int fd, int64_t offset, int64_t length);
    virtual status_t                setDataSource(const sp<IDataSource>& source, const char *mime);
    virtual sp<IMemory>             getFrameAtTime(
            int64_t timeUs, int option, int colorFormat, bool metaOnly);
    virtual sp<IMemory>             getImageAtIndex(
            int index, int colorFormat, bool metaOnly, bool thumbnail);
    virtual sp<IMemory>             getImageRectAtIndex(
            int index, int colorFormat, int left, int top, int right, int bottom);
    virtual status_t getFrameAtIndex(
                std::vector<sp<IMemory> > *frames,
                int frameIndex, int numFrames, int colorFormat, bool metaOnly);
    virtual sp<IMemory>             extractAlbumArt();
    virtual const char*             extractMetadata(int keyCode);

    virtual status_t                dump(int fd, const Vector<String16>& args);

private:
    friend class MediaPlayerService;

    explicit MetadataRetrieverClient(pid_t pid);
    virtual ~MetadataRetrieverClient();

    mutable Mutex                          mLock;
    static  Mutex                          sLock;
    sp<MediaMetadataRetrieverBase>         mRetriever;
    pid_t                                  mPid;

    // Keep the shared memory copy of album art
    sp<IMemory>                            mAlbumArt;
};

}; // namespace android

#endif // ANDROID_MEDIAMETADATARETRIEVERSERVICE_H

