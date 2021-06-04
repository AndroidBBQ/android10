/*
 * Copyright 2017, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef REMOTE_MEDIA_SOURCE_H_
#define REMOTE_MEDIA_SOURCE_H_

#include <media/IMediaSource.h>
#include <media/MediaSource.h>
#include <media/stagefright/foundation/ABase.h>

namespace android {

// IMediaSrouce wrapper to the MediaSource.
class RemoteMediaSource : public BnMediaSource {
public:
    static sp<IMediaSource> wrap(
            const sp<RemoteMediaExtractor> &extractor,
            MediaTrack *source,
            const sp<RefBase> &plugin);
    virtual ~RemoteMediaSource();
    virtual status_t start(MetaData *params = NULL);
    virtual status_t stop();
    virtual sp<MetaData> getFormat();
    virtual status_t read(
            MediaBufferBase **buffer,
            const MediaSource::ReadOptions *options = NULL);
    virtual bool supportNonblockingRead();
    virtual status_t pause();
    virtual status_t setStopTimeUs(int64_t stopTimeUs);

private:
    sp<RemoteMediaExtractor> mExtractor;
    MediaTrack *mTrack;
    sp<RefBase> mExtractorPlugin;

    explicit RemoteMediaSource(
            const sp<RemoteMediaExtractor> &extractor,
            MediaTrack *source,
            const sp<RefBase> &plugin);

    DISALLOW_EVIL_CONSTRUCTORS(RemoteMediaSource);
};

}  // namespace android

#endif  // REMOTE_MEDIA_SOURCE_H_
