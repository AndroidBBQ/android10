/*
**
** Copyright 2008, The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "MediaMetadataRetriever"

#include <inttypes.h>

#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <media/mediametadataretriever.h>
#include <media/IMediaHTTPService.h>
#include <media/IMediaPlayerService.h>
#include <utils/Log.h>
#include <dlfcn.h>

namespace android {

// client singleton for binder interface to service
Mutex MediaMetadataRetriever::sServiceLock;
sp<IMediaPlayerService> MediaMetadataRetriever::sService;
sp<MediaMetadataRetriever::DeathNotifier> MediaMetadataRetriever::sDeathNotifier;

const sp<IMediaPlayerService> MediaMetadataRetriever::getService()
{
    Mutex::Autolock lock(sServiceLock);
    if (sService == 0) {
        sp<IServiceManager> sm = defaultServiceManager();
        sp<IBinder> binder;
        do {
            binder = sm->getService(String16("media.player"));
            if (binder != 0) {
                break;
            }
            ALOGW("MediaPlayerService not published, waiting...");
            usleep(500000); // 0.5 s
        } while (true);
        if (sDeathNotifier == NULL) {
            sDeathNotifier = new DeathNotifier();
        }
        binder->linkToDeath(sDeathNotifier);
        sService = interface_cast<IMediaPlayerService>(binder);
    }
    ALOGE_IF(sService == 0, "no MediaPlayerService!?");
    return sService;
}

MediaMetadataRetriever::MediaMetadataRetriever()
{
    ALOGV("constructor");
    const sp<IMediaPlayerService> service(getService());
    if (service == 0) {
        ALOGE("failed to obtain MediaMetadataRetrieverService");
        return;
    }
    sp<IMediaMetadataRetriever> retriever(service->createMetadataRetriever());
    if (retriever == 0) {
        ALOGE("failed to create IMediaMetadataRetriever object from server");
    }
    mRetriever = retriever;
}

MediaMetadataRetriever::~MediaMetadataRetriever()
{
    ALOGV("destructor");
    disconnect();
    IPCThreadState::self()->flushCommands();
}

void MediaMetadataRetriever::disconnect()
{
    ALOGV("disconnect");
    sp<IMediaMetadataRetriever> retriever;
    {
        Mutex::Autolock _l(mLock);
        retriever = mRetriever;
        mRetriever.clear();
    }
    if (retriever != 0) {
        retriever->disconnect();
    }
}

status_t MediaMetadataRetriever::setDataSource(
        const sp<IMediaHTTPService> &httpService,
        const char *srcUrl,
        const KeyedVector<String8, String8> *headers)
{
    ALOGV("setDataSource");
    Mutex::Autolock _l(mLock);
    if (mRetriever == 0) {
        ALOGE("retriever is not initialized");
        return INVALID_OPERATION;
    }
    if (srcUrl == NULL) {
        ALOGE("data source is a null pointer");
        return UNKNOWN_ERROR;
    }
    ALOGV("data source (%s)", srcUrl);
    return mRetriever->setDataSource(httpService, srcUrl, headers);
}

status_t MediaMetadataRetriever::setDataSource(int fd, int64_t offset, int64_t length)
{
    ALOGV("setDataSource(%d, %" PRId64 ", %" PRId64 ")", fd, offset, length);
    Mutex::Autolock _l(mLock);
    if (mRetriever == 0) {
        ALOGE("retriever is not initialized");
        return INVALID_OPERATION;
    }
    if (fd < 0 || offset < 0 || length < 0) {
        ALOGE("Invalid negative argument");
        return UNKNOWN_ERROR;
    }
    return mRetriever->setDataSource(fd, offset, length);
}

status_t MediaMetadataRetriever::setDataSource(
    const sp<IDataSource>& dataSource, const char *mime)
{
    ALOGV("setDataSource(IDataSource)");
    Mutex::Autolock _l(mLock);
    if (mRetriever == 0) {
        ALOGE("retriever is not initialized");
        return INVALID_OPERATION;
    }
    return mRetriever->setDataSource(dataSource, mime);
}

sp<IMemory> MediaMetadataRetriever::getFrameAtTime(
        int64_t timeUs, int option, int colorFormat, bool metaOnly)
{
    ALOGV("getFrameAtTime: time(%" PRId64 " us) option(%d) colorFormat(%d) metaOnly(%d)",
            timeUs, option, colorFormat, metaOnly);
    Mutex::Autolock _l(mLock);
    if (mRetriever == 0) {
        ALOGE("retriever is not initialized");
        return NULL;
    }
    return mRetriever->getFrameAtTime(timeUs, option, colorFormat, metaOnly);
}

sp<IMemory> MediaMetadataRetriever::getImageAtIndex(
        int index, int colorFormat, bool metaOnly, bool thumbnail) {
    ALOGV("getImageAtIndex: index(%d) colorFormat(%d) metaOnly(%d) thumbnail(%d)",
            index, colorFormat, metaOnly, thumbnail);
    Mutex::Autolock _l(mLock);
    if (mRetriever == 0) {
        ALOGE("retriever is not initialized");
        return NULL;
    }
    return mRetriever->getImageAtIndex(index, colorFormat, metaOnly, thumbnail);
}

sp<IMemory> MediaMetadataRetriever::getImageRectAtIndex(
        int index, int colorFormat, int left, int top, int right, int bottom) {
    ALOGV("getImageRectAtIndex: index(%d) colorFormat(%d) rect {%d, %d, %d, %d}",
            index, colorFormat, left, top, right, bottom);
    Mutex::Autolock _l(mLock);
    if (mRetriever == 0) {
        ALOGE("retriever is not initialized");
        return NULL;
    }
    return mRetriever->getImageRectAtIndex(
            index, colorFormat, left, top, right, bottom);
}

status_t MediaMetadataRetriever::getFrameAtIndex(
        std::vector<sp<IMemory> > *frames,
        int frameIndex, int numFrames, int colorFormat, bool metaOnly) {
    ALOGV("getFrameAtIndex: frameIndex(%d), numFrames(%d), colorFormat(%d) metaOnly(%d)",
            frameIndex, numFrames, colorFormat, metaOnly);
    Mutex::Autolock _l(mLock);
    if (mRetriever == 0) {
        ALOGE("retriever is not initialized");
        return INVALID_OPERATION;
    }
    return mRetriever->getFrameAtIndex(
            frames, frameIndex, numFrames, colorFormat, metaOnly);
}

const char* MediaMetadataRetriever::extractMetadata(int keyCode)
{
    ALOGV("extractMetadata(%d)", keyCode);
    Mutex::Autolock _l(mLock);
    if (mRetriever == 0) {
        ALOGE("retriever is not initialized");
        return NULL;
    }
    return mRetriever->extractMetadata(keyCode);
}

sp<IMemory> MediaMetadataRetriever::extractAlbumArt()
{
    ALOGV("extractAlbumArt");
    Mutex::Autolock _l(mLock);
    if (mRetriever == 0) {
        ALOGE("retriever is not initialized");
        return NULL;
    }
    return mRetriever->extractAlbumArt();
}

void MediaMetadataRetriever::DeathNotifier::binderDied(const wp<IBinder>& who __unused) {
    Mutex::Autolock lock(MediaMetadataRetriever::sServiceLock);
    MediaMetadataRetriever::sService.clear();
    ALOGW("MediaMetadataRetriever server died!");
}

MediaMetadataRetriever::DeathNotifier::~DeathNotifier()
{
    Mutex::Autolock lock(sServiceLock);
    if (sService != 0) {
        IInterface::asBinder(sService)->unlinkToDeath(this);
    }
}

} // namespace android
