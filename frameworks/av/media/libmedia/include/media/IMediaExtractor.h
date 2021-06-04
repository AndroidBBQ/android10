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

#ifndef IMEDIA_EXTRACTOR_BASE_H_

#define IMEDIA_EXTRACTOR_BASE_H_

#include <media/DataSource.h>
#include <media/IMediaSource.h>
#include <vector>

namespace android {

class MetaData;
typedef std::vector<uint8_t> HInterfaceToken;

class IMediaExtractor : public IInterface {
public:
    DECLARE_META_INTERFACE(MediaExtractor);

    virtual size_t countTracks() = 0;
    // This function could return NULL IMediaSource even when index is within the
    // track count returned by countTracks, since it's possible the track is malformed
    // and it's not detected during countTracks call.
    virtual sp<IMediaSource> getTrack(size_t index) = 0;

    enum GetTrackMetaDataFlags {
        kIncludeExtensiveMetaData = 1
    };
    virtual sp<MetaData> getTrackMetaData(
            size_t index, uint32_t flags = 0) = 0;

    // Return container specific meta-data. The default implementation
    // returns an empty metadata object.
    virtual sp<MetaData> getMetaData() = 0;

    virtual status_t getMetrics(Parcel *reply) = 0;

    enum Flags {
        CAN_SEEK_BACKWARD  = 1,  // the "seek 10secs back button"
        CAN_SEEK_FORWARD   = 2,  // the "seek 10secs forward button"
        CAN_PAUSE          = 4,
        CAN_SEEK           = 8,  // the "seek bar"
    };

    // If subclasses do _not_ override this, the default is
    // CAN_SEEK_BACKWARD | CAN_SEEK_FORWARD | CAN_SEEK | CAN_PAUSE
    virtual uint32_t flags() const = 0;

    virtual status_t setMediaCas(const HInterfaceToken &casToken) = 0;

    virtual const char * name() = 0;
};


class BnMediaExtractor: public BnInterface<IMediaExtractor>
{
public:
    virtual status_t    onTransact(uint32_t code, const Parcel& data, Parcel* reply,
                                uint32_t flags = 0);
};

void registerMediaExtractor(
        const sp<IMediaExtractor> &extractor,
        const sp<DataSource> &source,
        const char *mime);

void registerMediaSource(
        const sp<IMediaExtractor> &extractor,
        const sp<IMediaSource> &source);

status_t dumpExtractors(int fd, const Vector<String16>& args);


}  // namespace android

#endif  // IMEDIA_EXTRACTOR_BASE_H_
