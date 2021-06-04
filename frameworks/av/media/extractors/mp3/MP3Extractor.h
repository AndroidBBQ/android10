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

#ifndef MP3_EXTRACTOR_H_

#define MP3_EXTRACTOR_H_

#include <utils/Errors.h>
#include <media/MediaExtractorPluginApi.h>
#include <media/MediaExtractorPluginHelper.h>
#include <media/NdkMediaFormat.h>

namespace android {

class DataSourceHelper;

struct AMessage;
struct MP3Seeker;
class String8;
struct Mp3Meta;

class MP3Extractor : public MediaExtractorPluginHelper {
public:
    MP3Extractor(DataSourceHelper *source, Mp3Meta *meta);
    ~MP3Extractor();

    virtual size_t countTracks();
    virtual MediaTrackHelper *getTrack(size_t index);
    virtual media_status_t getTrackMetaData(AMediaFormat *meta, size_t index, uint32_t flags);

    virtual media_status_t getMetaData(AMediaFormat *meta);
    virtual const char * name() { return "MP3Extractor"; }

private:
    status_t mInitCheck;

    DataSourceHelper *mDataSource;
    off64_t mFirstFramePos;
    AMediaFormat *mMeta;
    uint32_t mFixedHeader;
    MP3Seeker *mSeeker;

    MP3Extractor(const MP3Extractor &);
    MP3Extractor &operator=(const MP3Extractor &);
};

}  // namespace android

#endif  // MP3_EXTRACTOR_H_
