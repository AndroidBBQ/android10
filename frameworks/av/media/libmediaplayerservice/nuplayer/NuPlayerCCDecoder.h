/*
 * Copyright 2014 The Android Open Source Project
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

#ifndef NUPLAYER_CCDECODER_H_

#define NUPLAYER_CCDECODER_H_

#include "NuPlayer.h"

namespace android {

struct NuPlayer::CCDecoder : public RefBase {
    enum {
        kWhatClosedCaptionData,
        kWhatTrackAdded,
    };

    enum {
        kTrackTypeCEA608,
        kTrackTypeCEA708,
    };

    explicit CCDecoder(const sp<AMessage> &notify);

    size_t getTrackCount() const;
    sp<AMessage> getTrackInfo(size_t index) const;
    status_t selectTrack(size_t index, bool select);
    ssize_t getSelectedTrack(media_track_type type) const;
    bool isSelected() const;
    void decode(const sp<ABuffer> &accessUnit);
    void display(int64_t timeUs);
    void flush();

private:
    // CC track identifier.
    struct CCTrack {
        CCTrack() : mTrackType(0), mTrackChannel(0) { }

        CCTrack(const int32_t trackType, const size_t trackChannel)
            : mTrackType(trackType), mTrackChannel(trackChannel) { }

        int32_t mTrackType;
        size_t mTrackChannel;

        // The ordering of CCTracks is to build a map of track to index.
        // It is necessary to find the index of the matched CCTrack when CC data comes.
        int compare(const NuPlayer::CCDecoder::CCTrack& rhs) const;
        inline bool operator<(const NuPlayer::CCDecoder::CCTrack& rhs) const;
        inline bool operator==(const NuPlayer::CCDecoder::CCTrack& rhs) const;
        inline bool operator!=(const NuPlayer::CCDecoder::CCTrack& rhs) const;
    };

    sp<AMessage> mNotify;
    KeyedVector<int64_t, sp<ABuffer> > mCCMap;
    ssize_t mSelectedTrack;
    KeyedVector<CCTrack, size_t> mTrackIndices;
    Vector<CCTrack> mTracks;

    // CEA-608 closed caption
    size_t mLine21Channels[2]; // The current channels of NTSC_CC_FIELD_{1, 2}

    // CEA-708 closed caption
    sp<ABuffer> mDTVCCPacket;

    bool isTrackValid(size_t index) const;
    size_t getTrackIndex(int32_t trackType, size_t channel, bool *trackAdded);

    // Extract from H.264 SEIs
    bool extractFromSEI(const sp<ABuffer> &accessUnit);
    bool parseSEINalUnit(int64_t timeUs, const uint8_t *data, size_t size);

    // Extract from MPEG user data
    bool extractFromMPEGUserData(const sp<ABuffer> &accessUnit);
    bool parseMPEGUserDataUnit(int64_t timeUs, const uint8_t *data, size_t size);

    // Extract CC tracks from MPEG_cc_data
    bool parseMPEGCCData(int64_t timeUs, const uint8_t *data, size_t size);
    bool parseDTVCCPacket(int64_t timeUs, const uint8_t *data, size_t size);

    DISALLOW_EVIL_CONSTRUCTORS(CCDecoder);
};

}  // namespace android

#endif  // NUPLAYER_CCDECODER_H_
