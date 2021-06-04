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

#include <media/stagefright/RemoteMediaExtractor.h>
#include <media/stagefright/RemoteMediaSource.h>
#include <media/IMediaSource.h>

namespace android {

RemoteMediaSource::RemoteMediaSource(
        const sp<RemoteMediaExtractor> &extractor,
        MediaTrack *source,
        const sp<RefBase> &plugin)
    : mExtractor(extractor),
      mTrack(source),
      mExtractorPlugin(plugin) {}

RemoteMediaSource::~RemoteMediaSource() {
    delete mTrack;
    mExtractorPlugin = nullptr;
}

status_t RemoteMediaSource::start(MetaData *params) {
    if (params) {
        ALOGW("dropping start parameters:");
        params->dumpToLog();
    }
    return mTrack->start();
}

status_t RemoteMediaSource::stop() {
    return mTrack->stop();
}

sp<MetaData> RemoteMediaSource::getFormat() {
    sp<MetaData> meta = new MetaData();
    if (mTrack->getFormat(*meta.get()) == OK) {
        return meta;
    }
    return nullptr;
}

status_t RemoteMediaSource::read(
        MediaBufferBase **buffer, const MediaSource::ReadOptions *options) {
    return mTrack->read(buffer, reinterpret_cast<const MediaSource::ReadOptions*>(options));
}

bool RemoteMediaSource::supportNonblockingRead() {
    return mTrack->supportNonblockingRead();
}

status_t RemoteMediaSource::pause() {
    return ERROR_UNSUPPORTED;
}

status_t RemoteMediaSource::setStopTimeUs(int64_t /* stopTimeUs */) {
    return ERROR_UNSUPPORTED;
}

////////////////////////////////////////////////////////////////////////////////

// static
sp<IMediaSource> RemoteMediaSource::wrap(
        const sp<RemoteMediaExtractor> &extractor,
        MediaTrack *source, const sp<RefBase> &plugin) {
    if (source == nullptr) {
        return nullptr;
    }
    return new RemoteMediaSource(extractor, source, plugin);
}

}  // namespace android
