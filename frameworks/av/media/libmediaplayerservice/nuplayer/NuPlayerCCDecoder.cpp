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

//#define LOG_NDEBUG 0
#define LOG_TAG "NuPlayerCCDecoder"
#include <utils/Log.h>
#include <inttypes.h>

#include "NuPlayerCCDecoder.h"

#include <media/stagefright/foundation/ABitReader.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/avc_utils.h>
#include <media/stagefright/MediaDefs.h>

namespace android {

// In CEA-708B, the maximum bandwidth of CC is set to 9600bps.
static const size_t kMaxBandwithSizeBytes = 9600 / 8;

struct CCData {
    CCData(uint8_t type, uint8_t data1, uint8_t data2)
        : mType(type), mData1(data1), mData2(data2) {
    }
    bool getChannel(size_t *channel) const {
        if (mData1 >= 0x10 && mData1 <= 0x1f) {
            *channel = (mData1 >= 0x18 ? 1 : 0) + (mType ? 2 : 0);
            return true;
        }
        return false;
    }

    uint8_t mType;
    uint8_t mData1;
    uint8_t mData2;
};

static bool isNullPad(CCData *cc) {
    return cc->mData1 < 0x10 && cc->mData2 < 0x10;
}

static void dumpBytePair(const sp<ABuffer> &ccBuf) __attribute__ ((unused));
static void dumpBytePair(const sp<ABuffer> &ccBuf) {
    size_t offset = 0;
    AString out;

    while (offset < ccBuf->size()) {
        char tmp[128];

        CCData *cc = (CCData *) (ccBuf->data() + offset);

        if (isNullPad(cc)) {
            // 1 null pad or XDS metadata, ignore
            offset += sizeof(CCData);
            continue;
        }

        if (cc->mData1 >= 0x20 && cc->mData1 <= 0x7f) {
            // 2 basic chars
            snprintf(tmp, sizeof(tmp), "[%d]Basic: %c %c", cc->mType, cc->mData1, cc->mData2);
        } else if ((cc->mData1 == 0x11 || cc->mData1 == 0x19)
                 && cc->mData2 >= 0x30 && cc->mData2 <= 0x3f) {
            // 1 special char
            snprintf(tmp, sizeof(tmp), "[%d]Special: %02x %02x", cc->mType, cc->mData1, cc->mData2);
        } else if ((cc->mData1 == 0x12 || cc->mData1 == 0x1A)
                 && cc->mData2 >= 0x20 && cc->mData2 <= 0x3f){
            // 1 Spanish/French char
            snprintf(tmp, sizeof(tmp), "[%d]Spanish: %02x %02x", cc->mType, cc->mData1, cc->mData2);
        } else if ((cc->mData1 == 0x13 || cc->mData1 == 0x1B)
                 && cc->mData2 >= 0x20 && cc->mData2 <= 0x3f){
            // 1 Portuguese/German/Danish char
            snprintf(tmp, sizeof(tmp), "[%d]German: %02x %02x", cc->mType, cc->mData1, cc->mData2);
        } else if ((cc->mData1 == 0x11 || cc->mData1 == 0x19)
                 && cc->mData2 >= 0x20 && cc->mData2 <= 0x2f){
            // Mid-Row Codes (Table 69)
            snprintf(tmp, sizeof(tmp), "[%d]Mid-row: %02x %02x", cc->mType, cc->mData1, cc->mData2);
        } else if (((cc->mData1 == 0x14 || cc->mData1 == 0x1c)
                  && cc->mData2 >= 0x20 && cc->mData2 <= 0x2f)
                  ||
                   ((cc->mData1 == 0x17 || cc->mData1 == 0x1f)
                  && cc->mData2 >= 0x21 && cc->mData2 <= 0x23)){
            // Misc Control Codes (Table 70)
            snprintf(tmp, sizeof(tmp), "[%d]Ctrl: %02x %02x", cc->mType, cc->mData1, cc->mData2);
        } else if ((cc->mData1 & 0x70) == 0x10
                && (cc->mData2 & 0x40) == 0x40
                && ((cc->mData1 & 0x07) || !(cc->mData2 & 0x20)) ) {
            // Preamble Address Codes (Table 71)
            snprintf(tmp, sizeof(tmp), "[%d]PAC: %02x %02x", cc->mType, cc->mData1, cc->mData2);
        } else {
            snprintf(tmp, sizeof(tmp), "[%d]Invalid: %02x %02x", cc->mType, cc->mData1, cc->mData2);
        }

        if (out.size() > 0) {
            out.append(", ");
        }

        out.append(tmp);

        offset += sizeof(CCData);
    }

    ALOGI("%s", out.c_str());
}

NuPlayer::CCDecoder::CCDecoder(const sp<AMessage> &notify)
    : mNotify(notify),
      mSelectedTrack(-1),
      mDTVCCPacket(new ABuffer(kMaxBandwithSizeBytes)) {
    mDTVCCPacket->setRange(0, 0);

    // In CEA-608, streams from packets which have the value 0 of cc_type contain CC1 and CC2, and
    // streams from packets which have the value 1 of cc_type contain CC3 and CC4.
    // The following array indicates the current transmitting channels for each value of cc_type.
    mLine21Channels[0] = 0; // CC1
    mLine21Channels[1] = 2; // CC3
}

size_t NuPlayer::CCDecoder::getTrackCount() const {
    return mTracks.size();
}

sp<AMessage> NuPlayer::CCDecoder::getTrackInfo(size_t index) const {
    if (!isTrackValid(index)) {
        return NULL;
    }

    sp<AMessage> format = new AMessage();

    CCTrack track = mTracks[index];

    format->setInt32("type", MEDIA_TRACK_TYPE_SUBTITLE);
    format->setString("language", "und");

    switch (track.mTrackType) {
        case kTrackTypeCEA608:
            format->setString("mime", MEDIA_MIMETYPE_TEXT_CEA_608);
            break;
        case kTrackTypeCEA708:
            format->setString("mime", MEDIA_MIMETYPE_TEXT_CEA_708);
            break;
        default:
            ALOGE("Unknown track type: %d", track.mTrackType);
            format->setInt32("type", MEDIA_TRACK_TYPE_UNKNOWN);
            format->setString("mime", "application/octet-stream");
            return format;
    }

    // For CEA-608 CC1, field 0 channel 0
    bool isDefaultAuto = track.mTrackType == kTrackTypeCEA608
            && track.mTrackChannel == 0;
    // For CEA-708, Primary Caption Service.
    bool isDefaultOnly = track.mTrackType == kTrackTypeCEA708
            && track.mTrackChannel == 1;
    format->setInt32("auto", isDefaultAuto);
    format->setInt32("default", isDefaultAuto || isDefaultOnly);
    format->setInt32("forced", 0);

    return format;
}

status_t NuPlayer::CCDecoder::selectTrack(size_t index, bool select) {
    if (!isTrackValid(index)) {
        return BAD_VALUE;
    }

    if (select) {
        if (mSelectedTrack == (ssize_t)index) {
            ALOGE("track %zu already selected", index);
            return BAD_VALUE;
        }
        ALOGV("selected track %zu", index);
        mSelectedTrack = index;
    } else {
        if (mSelectedTrack != (ssize_t)index) {
            ALOGE("track %zu is not selected", index);
            return BAD_VALUE;
        }
        ALOGV("unselected track %zu", index);
        mSelectedTrack = -1;
    }

    // Clear the previous track payloads
    mCCMap.clear();

    return OK;
}

ssize_t NuPlayer::CCDecoder::getSelectedTrack(media_track_type type) const {
    if (mSelectedTrack != -1) {
        CCTrack track = mTracks[mSelectedTrack];
        if (track.mTrackType == kTrackTypeCEA608 || track.mTrackType == kTrackTypeCEA708) {
            return (type == MEDIA_TRACK_TYPE_SUBTITLE ? mSelectedTrack : -1);
        }
        return (type == MEDIA_TRACK_TYPE_UNKNOWN ? mSelectedTrack : -1);
    }

    return -1;
}

bool NuPlayer::CCDecoder::isSelected() const {
    return mSelectedTrack >= 0 && mSelectedTrack < (int32_t)getTrackCount();
}

bool NuPlayer::CCDecoder::isTrackValid(size_t index) const {
    return index < getTrackCount();
}

// returns true if a new CC track is found
bool NuPlayer::CCDecoder::extractFromSEI(const sp<ABuffer> &accessUnit) {
    sp<ABuffer> sei;
    if (!accessUnit->meta()->findBuffer("sei", &sei) || sei == NULL) {
        return false;
    }

    int64_t timeUs;
    CHECK(accessUnit->meta()->findInt64("timeUs", &timeUs));

    bool trackAdded = false;

    const NALPosition *nal = (NALPosition *)sei->data();

    for (size_t i = 0; i < sei->size() / sizeof(NALPosition); ++i, ++nal) {
        trackAdded |= parseSEINalUnit(
                timeUs, accessUnit->data() + nal->nalOffset, nal->nalSize);
    }

    return trackAdded;
}

// returns true if a new CC track is found
bool NuPlayer::CCDecoder::parseSEINalUnit(int64_t timeUs, const uint8_t *data, size_t size) {
    unsigned nalType = data[0] & 0x1f;

    // the buffer should only have SEI in it
    if (nalType != 6) {
        return false;
    }

    bool trackAdded = false;
    NALBitReader br(data + 1, size - 1);

    // sei_message()
    while (br.atLeastNumBitsLeft(16)) { // at least 16-bit for sei_message()
        uint32_t payload_type = 0;
        size_t payload_size = 0;
        uint8_t last_byte;

        do {
            last_byte = br.getBits(8);
            payload_type += last_byte;
        } while (last_byte == 0xFF);

        do {
            last_byte = br.getBits(8);
            payload_size += last_byte;
        } while (last_byte == 0xFF);

        if (payload_size > SIZE_MAX / 8
                || !br.atLeastNumBitsLeft(payload_size * 8)) {
            ALOGV("Malformed SEI payload");
            break;
        }

        // sei_payload()
        if (payload_type == 4) {
            bool isCC = false;
            if (payload_size > 1 + 2 + 4 + 1) {
                // user_data_registered_itu_t_t35()

                // ATSC A/72: 6.4.2
                uint8_t itu_t_t35_country_code = br.getBits(8);
                uint16_t itu_t_t35_provider_code = br.getBits(16);
                uint32_t user_identifier = br.getBits(32);
                uint8_t user_data_type_code = br.getBits(8);

                payload_size -= 1 + 2 + 4 + 1;

                isCC = itu_t_t35_country_code == 0xB5
                        && itu_t_t35_provider_code == 0x0031
                        && user_identifier == 'GA94'
                        && user_data_type_code == 0x3;
            }

            if (isCC && payload_size > 2) {
                trackAdded |= parseMPEGCCData(timeUs, br.data(), br.numBitsLeft() / 8);
            } else {
                ALOGV("Malformed SEI payload type 4");
            }
        } else {
            ALOGV("Unsupported SEI payload type %d", payload_type);
        }

        // skipping remaining bits of this payload
        br.skipBits(payload_size * 8);
    }

    return trackAdded;
}

// returns true if a new CC track is found
bool NuPlayer::CCDecoder::extractFromMPEGUserData(const sp<ABuffer> &accessUnit) {
    sp<ABuffer> mpegUserData;
    if (!accessUnit->meta()->findBuffer("mpeg-user-data", &mpegUserData)
            || mpegUserData == NULL) {
        return false;
    }

    int64_t timeUs;
    CHECK(accessUnit->meta()->findInt64("timeUs", &timeUs));

    bool trackAdded = false;

    const size_t *userData = (size_t *)mpegUserData->data();

    for (size_t i = 0; i < mpegUserData->size() / sizeof(size_t); ++i) {
        if (accessUnit->size() < userData[i]) {
            ALOGW("b/129068792, skip invalid offset for user data");
            android_errorWriteLog(0x534e4554, "129068792");
            continue;
        }
        trackAdded |= parseMPEGUserDataUnit(
                timeUs, accessUnit->data() + userData[i], accessUnit->size() - userData[i]);
    }

    return trackAdded;
}

// returns true if a new CC track is found
bool NuPlayer::CCDecoder::parseMPEGUserDataUnit(int64_t timeUs, const uint8_t *data, size_t size) {
    if (size < 9) {
        ALOGW("b/129068792, MPEG user data size too small %zu", size);
        android_errorWriteLog(0x534e4554, "129068792");
        return false;
    }

    ABitReader br(data + 4, 5);

    uint32_t user_identifier = br.getBits(32);
    uint8_t user_data_type = br.getBits(8);

    if (user_identifier == 'GA94' && user_data_type == 0x3) {
        return parseMPEGCCData(timeUs, data + 9, size - 9);
    }

    return false;
}

// returns true if a new CC track is found
bool NuPlayer::CCDecoder::parseMPEGCCData(int64_t timeUs, const uint8_t *data, size_t size) {
    bool trackAdded = false;

    // MPEG_cc_data()
    // ATSC A/53 Part 4: 6.2.3.1
    ABitReader br(data, size);

    if (br.numBitsLeft() <= 16) {
        return false;
    }

    br.skipBits(1);
    bool process_cc_data_flag = br.getBits(1);
    br.skipBits(1);
    size_t cc_count = br.getBits(5);
    br.skipBits(8);

    if (!process_cc_data_flag || 3 * 8 * cc_count >= br.numBitsLeft()) {
        return false;
    }

    sp<ABuffer> line21CCBuf = NULL;

    for (size_t i = 0; i < cc_count; ++i) {
        br.skipBits(5);
        bool cc_valid = br.getBits(1);
        uint8_t cc_type = br.getBits(2);

        if (cc_valid) {
            if (cc_type == 3) {
                if (mDTVCCPacket->size() > 0) {
                    trackAdded |= parseDTVCCPacket(
                            timeUs, mDTVCCPacket->data(), mDTVCCPacket->size());
                    mDTVCCPacket->setRange(0, 0);
                }
                memcpy(mDTVCCPacket->data() + mDTVCCPacket->size(), br.data(), 2);
                mDTVCCPacket->setRange(0, mDTVCCPacket->size() + 2);
                br.skipBits(16);
            } else if (mDTVCCPacket->size() > 0 && cc_type == 2) {
                if (mDTVCCPacket->capacity() - mDTVCCPacket->size() >= 2) {
                    memcpy(mDTVCCPacket->data() + mDTVCCPacket->size(), br.data(), 2);
                    mDTVCCPacket->setRange(0, mDTVCCPacket->size() + 2);
                } else {
                    ALOGW("b/129068792, skip CC due to too much data(%zu, %zu)",
                          mDTVCCPacket->capacity(), mDTVCCPacket->size());
                    android_errorWriteLog(0x534e4554, "129068792");
                }
                br.skipBits(16);
            } else if (cc_type == 0 || cc_type == 1) {
                uint8_t cc_data_1 = br.getBits(8) & 0x7f;
                uint8_t cc_data_2 = br.getBits(8) & 0x7f;

                CCData cc(cc_type, cc_data_1, cc_data_2);

                if (isNullPad(&cc)) {
                    continue;
                }

                size_t channel;
                if (cc.getChannel(&channel)) {
                    mLine21Channels[cc_type] = channel;

                    // create a new track if it does not exist.
                    getTrackIndex(kTrackTypeCEA608, channel, &trackAdded);
                }

                if (isSelected() && mTracks[mSelectedTrack].mTrackType == kTrackTypeCEA608
                        && mTracks[mSelectedTrack].mTrackChannel == mLine21Channels[cc_type]) {
                    if (line21CCBuf == NULL) {
                        line21CCBuf = new ABuffer((cc_count - i) * sizeof(CCData));
                        line21CCBuf->setRange(0, 0);
                    }
                    memcpy(line21CCBuf->data() + line21CCBuf->size(), &cc, sizeof(cc));
                    line21CCBuf->setRange(0, line21CCBuf->size() + sizeof(CCData));
                }
            } else {
                br.skipBits(16);
            }
        } else {
            if ((cc_type == 3 || cc_type == 2) && mDTVCCPacket->size() > 0) {
                trackAdded |= parseDTVCCPacket(timeUs, mDTVCCPacket->data(), mDTVCCPacket->size());
                mDTVCCPacket->setRange(0, 0);
            }
            br.skipBits(16);
        }
    }

    if (isSelected() && mTracks[mSelectedTrack].mTrackType == kTrackTypeCEA608
            && line21CCBuf != NULL && line21CCBuf->size() > 0) {
        mCCMap.add(timeUs, line21CCBuf);
    }

    return trackAdded;
}

// returns true if a new CC track is found
bool NuPlayer::CCDecoder::parseDTVCCPacket(int64_t timeUs, const uint8_t *data, size_t size) {
    // CEA-708B 5 DTVCC Packet Layer.
    ABitReader br(data, size);
    br.skipBits(2);

    size_t packet_size = br.getBits(6);
    if (packet_size == 0) packet_size = 64;
    packet_size *= 2;

    if (size != packet_size) {
        return false;
    }

    bool trackAdded = false;

    while (br.numBitsLeft() >= 16) {
        // CEA-708B Figure 5 and 6.
        uint8_t service_number = br.getBits(3);
        size_t block_size = br.getBits(5);

        if (service_number == 64) {
            br.skipBits(2);
            service_number = br.getBits(6);

            if (service_number < 64) {
                return trackAdded;
            }
        }

        if (br.numBitsLeft() < block_size * 8) {
            return trackAdded;
        }

        if (block_size > 0) {
            size_t trackIndex = getTrackIndex(kTrackTypeCEA708, service_number, &trackAdded);
            if (mSelectedTrack == (ssize_t)trackIndex) {
                sp<ABuffer> ccPacket = new ABuffer(block_size);
                if (ccPacket->capacity() == 0) {
                    ALOGW("b/129068792, no memory available, %zu", block_size);
                    android_errorWriteLog(0x534e4554, "129068792");
                    return false;
                }
                memcpy(ccPacket->data(), br.data(), block_size);
                mCCMap.add(timeUs, ccPacket);
            }
        }
        br.skipBits(block_size * 8);
    }

    return trackAdded;
}

// return the track index for a given type and channel.
// if the track does not exist, creates a new one.
size_t NuPlayer::CCDecoder::getTrackIndex(
        int32_t trackType, size_t channel, bool *trackAdded) {
    CCTrack track(trackType, channel);
    ssize_t index = mTrackIndices.indexOfKey(track);

    if (index < 0) {
        // A new track is added.
        index = mTracks.size();
        mTrackIndices.add(track, index);
        mTracks.add(track);
        *trackAdded = true;
        return index;
    }

    return mTrackIndices.valueAt(index);
}

void NuPlayer::CCDecoder::decode(const sp<ABuffer> &accessUnit) {
    if (extractFromMPEGUserData(accessUnit) || extractFromSEI(accessUnit)) {
        sp<AMessage> msg = mNotify->dup();
        msg->setInt32("what", kWhatTrackAdded);
        msg->post();
    }
    // TODO: extract CC from other sources
}

void NuPlayer::CCDecoder::display(int64_t timeUs) {
    if (!isSelected()) {
        return;
    }

    ssize_t index = mCCMap.indexOfKey(timeUs);
    if (index < 0) {
        ALOGV("cc for timestamp %" PRId64 " not found", timeUs);
        return;
    }

    sp<ABuffer> ccBuf;

    if (index == 0) {
        ccBuf = mCCMap.valueAt(index);
    } else {
        size_t size = 0;

        for (ssize_t i = 0; i <= index; ++i) {
            size += mCCMap.valueAt(i)->size();
        }

        ccBuf = new ABuffer(size);
        ccBuf->setRange(0, 0);

        for (ssize_t i = 0; i <= index; ++i) {
            sp<ABuffer> buf = mCCMap.valueAt(i);
            memcpy(ccBuf->data() + ccBuf->size(), buf->data(), buf->size());
            ccBuf->setRange(0, ccBuf->size() + buf->size());
        }
    }

    if (ccBuf->size() > 0) {
#if 0
        dumpBytePair(ccBuf);
#endif

        ccBuf->meta()->setInt32("track-index", mSelectedTrack);
        ccBuf->meta()->setInt64("timeUs", timeUs);
        ccBuf->meta()->setInt64("durationUs", 0LL);

        sp<AMessage> msg = mNotify->dup();
        msg->setInt32("what", kWhatClosedCaptionData);
        msg->setBuffer("buffer", ccBuf);
        msg->post();
    }

    // remove all entries before timeUs
    mCCMap.removeItemsAt(0, index + 1);
}

void NuPlayer::CCDecoder::flush() {
    mCCMap.clear();
    mDTVCCPacket->setRange(0, 0);
}

int32_t NuPlayer::CCDecoder::CCTrack::compare(const NuPlayer::CCDecoder::CCTrack& rhs) const {
    int32_t cmp = mTrackType - rhs.mTrackType;
    if (cmp != 0) return cmp;
    return mTrackChannel - rhs.mTrackChannel;
}

bool NuPlayer::CCDecoder::CCTrack::operator<(const NuPlayer::CCDecoder::CCTrack& rhs) const {
    return compare(rhs) < 0;
}

bool NuPlayer::CCDecoder::CCTrack::operator==(const NuPlayer::CCDecoder::CCTrack& rhs) const {
    return compare(rhs) == 0;
}

bool NuPlayer::CCDecoder::CCTrack::operator!=(const NuPlayer::CCDecoder::CCTrack& rhs) const {
    return compare(rhs) != 0;
}

}  // namespace android

