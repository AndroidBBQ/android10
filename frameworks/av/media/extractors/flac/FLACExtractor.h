/*
 * Copyright (C) 2011 The Android Open Source Project
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

#ifndef FLAC_EXTRACTOR_H_
#define FLAC_EXTRACTOR_H_

#include <media/DataSourceBase.h>
#include <media/MediaExtractorPluginApi.h>
#include <media/MediaExtractorPluginHelper.h>
#include <media/NdkMediaFormat.h>
#include <utils/String8.h>

namespace android {

class FLACParser;

class FLACExtractor : public MediaExtractorPluginHelper {

public:
    explicit FLACExtractor(DataSourceHelper *source);

    virtual size_t countTracks();
    virtual MediaTrackHelper *getTrack(size_t index);
    virtual media_status_t getTrackMetaData(AMediaFormat *meta, size_t index, uint32_t flags);

    virtual media_status_t getMetaData(AMediaFormat *meta);
    virtual const char * name() { return "FLACExtractor"; }

protected:
    virtual ~FLACExtractor();

private:
    DataSourceHelper *mDataSource;
    FLACParser *mParser;
    status_t mInitCheck;
    AMediaFormat *mFileMetadata;

    // There is only one track
    AMediaFormat *mTrackMetadata;

    FLACExtractor(const FLACExtractor &);
    FLACExtractor &operator=(const FLACExtractor &);

};

}  // namespace android

#endif  // FLAC_EXTRACTOR_H_
