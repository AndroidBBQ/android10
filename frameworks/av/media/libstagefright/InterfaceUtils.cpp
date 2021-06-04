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

#include "include/CallbackDataSource.h"

#include <media/stagefright/CallbackMediaSource.h>
#include <media/stagefright/InterfaceUtils.h>
#include <media/stagefright/RemoteDataSource.h>
#include <media/stagefright/RemoteMediaSource.h>

namespace android {

sp<DataSource> CreateDataSourceFromIDataSource(const sp<IDataSource> &source) {
    if (source == nullptr) {
        return nullptr;
    }
    return new TinyCacheSource(new CallbackDataSource(source));
}

sp<IDataSource> CreateIDataSourceFromDataSource(const sp<DataSource> &source) {
    if (source == nullptr) {
        return nullptr;
    }
    return RemoteDataSource::wrap(source);
}

sp<IMediaExtractor> CreateIMediaExtractorFromMediaExtractor(
        MediaExtractor *extractor,
        const sp<DataSource> &source,
        const sp<RefBase> &plugin) {
    if (extractor == nullptr) {
        return nullptr;
    }
    return RemoteMediaExtractor::wrap(extractor, source, plugin);
}

sp<MediaSource> CreateMediaSourceFromIMediaSource(const sp<IMediaSource> &source) {
    if (source == nullptr) {
        return nullptr;
    }
    return new CallbackMediaSource(source);
}

sp<IMediaSource> CreateIMediaSourceFromMediaSourceBase(
        const sp<RemoteMediaExtractor> &extractor,
        MediaTrack *source, const sp<RefBase> &plugin) {
    if (source == nullptr) {
        return nullptr;
    }
    return RemoteMediaSource::wrap(extractor, source, plugin);
}

}  // namespace android
