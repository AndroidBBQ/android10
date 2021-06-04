/*
 * Copyright (C) 2010 The Android Open Source Project
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

#ifndef OGG_EXTRACTOR_H_

#define OGG_EXTRACTOR_H_

#include <utils/Errors.h>
#include <media/MediaExtractorPluginApi.h>
#include <media/MediaExtractorPluginHelper.h>
#include <media/NdkMediaFormat.h>

namespace android {

struct AMessage;
class String8;

struct MyOggExtractor;
struct OggSource;

struct OggExtractor : public MediaExtractorPluginHelper {
    explicit OggExtractor(DataSourceHelper *source);

    virtual size_t countTracks();
    virtual MediaTrackHelper *getTrack(size_t index);
    virtual media_status_t getTrackMetaData(AMediaFormat *meta, size_t index, uint32_t flags);

    virtual media_status_t getMetaData(AMediaFormat *meta);
    virtual const char * name() { return "OggExtractor"; }

protected:
    virtual ~OggExtractor();

private:
    friend struct OggSource;

    DataSourceHelper *mDataSource;
    status_t mInitCheck;

    MyOggExtractor *mImpl;

    OggExtractor(const OggExtractor &);
    OggExtractor &operator=(const OggExtractor &);
};

}  // namespace android

#endif  // OGG_EXTRACTOR_H_
