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

#ifndef MEDIA_EXTRACTOR_H_

#define MEDIA_EXTRACTOR_H_

#include <stdio.h>
#include <vector>

#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/RefBase.h>
#include <media/MediaExtractorPluginApi.h>
#include <media/MediaExtractorPluginHelper.h>

namespace android {

class DataSourceBase;
class MetaDataBase;
struct MediaTrack;


class ExtractorAllocTracker {
public:
    ExtractorAllocTracker() {
        ALOGD("extractor allocated: %p", this);
    }
    virtual ~ExtractorAllocTracker() {
        ALOGD("extractor freed: %p", this);
    }
};

class MediaExtractor
// : public ExtractorAllocTracker
{
public:
    virtual ~MediaExtractor();
    virtual size_t countTracks() = 0;
    virtual MediaTrack *getTrack(size_t index) = 0;

    enum GetTrackMetaDataFlags {
        kIncludeExtensiveMetaData = 1
    };
    virtual status_t getTrackMetaData(
            MetaDataBase& meta,
            size_t index, uint32_t flags = 0) = 0;

    // Return container specific meta-data. The default implementation
    // returns an empty metadata object.
    virtual status_t getMetaData(MetaDataBase& meta) = 0;

    enum Flags {
        CAN_SEEK_BACKWARD  = 1,  // the "seek 10secs back button"
        CAN_SEEK_FORWARD   = 2,  // the "seek 10secs forward button"
        CAN_PAUSE          = 4,
        CAN_SEEK           = 8,  // the "seek bar"
    };

    // If subclasses do _not_ override this, the default is
    // CAN_SEEK_BACKWARD | CAN_SEEK_FORWARD | CAN_SEEK | CAN_PAUSE
    virtual uint32_t flags() const;

    virtual status_t setMediaCas(const uint8_t* /*casToken*/, size_t /*size*/) {
        return INVALID_OPERATION;
    }

    virtual const char * name() { return "<unspecified>"; }

protected:
    MediaExtractor();

private:
    MediaExtractor(const MediaExtractor &);
    MediaExtractor &operator=(const MediaExtractor &);
};

class MediaExtractorCUnwrapper : public MediaExtractor {
public:
    explicit MediaExtractorCUnwrapper(CMediaExtractor *plugin);
    virtual size_t countTracks();
    virtual MediaTrack *getTrack(size_t index);
    virtual status_t getTrackMetaData(MetaDataBase& meta, size_t index, uint32_t flags = 0);
    virtual status_t getMetaData(MetaDataBase& meta);
    virtual const char * name();
    virtual uint32_t flags() const;
    virtual status_t setMediaCas(const uint8_t* casToken, size_t size);
protected:
    virtual ~MediaExtractorCUnwrapper();
private:
    CMediaExtractor *plugin;
};

}  // namespace android

#endif  // MEDIA_EXTRACTOR_H_
