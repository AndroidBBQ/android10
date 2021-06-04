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

#ifndef INTERFACE_UTILS_H_
#define INTERFACE_UTILS_H_

#include <utils/RefBase.h>
#include <media/stagefright/RemoteMediaExtractor.h>
#include <media/MediaSource.h>
#include <media/IMediaExtractor.h>
#include <media/IMediaSource.h>

namespace android {

class DataSource;
class MediaExtractor;
struct MediaSource;
class IDataSource;
class IMediaExtractor;
class IMediaSource;

// Creates a DataSource which wraps the given IDataSource object.
sp<DataSource> CreateDataSourceFromIDataSource(const sp<IDataSource> &source);

// creates an IDataSource wrapper to the DataSource.
sp<IDataSource> CreateIDataSourceFromDataSource(const sp<DataSource> &source);

// Creates an IMediaExtractor wrapper to the given MediaExtractor.
sp<IMediaExtractor> CreateIMediaExtractorFromMediaExtractor(
        MediaExtractor *extractor,
        const sp<DataSource> &source,
        const sp<RefBase> &plugin);

// Creates a MediaSource which wraps the given IMediaSource object.
sp<MediaSource> CreateMediaSourceFromIMediaSource(const sp<IMediaSource> &source);

// Creates an IMediaSource wrapper to the given MediaSource.
sp<IMediaSource> CreateIMediaSourceFromMediaSourceBase(
        const sp<RemoteMediaExtractor> &extractor,
        MediaTrack *source, const sp<RefBase> &plugin);

}  // namespace android

#endif  // INTERFACE_UTILS_H_
