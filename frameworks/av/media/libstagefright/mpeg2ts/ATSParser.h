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

#ifndef A_TS_PARSER_H_

#define A_TS_PARSER_H_

#include <sys/types.h>

#include <media/MediaSource.h>
#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AudioPresentationInfo.h>
#include <utils/KeyedVector.h>
#include <utils/Vector.h>
#include <utils/RefBase.h>
#include <vector>

namespace android {
namespace hardware {
namespace cas {
namespace V1_0 {
struct ICas;
}}}
using hardware::cas::V1_0::ICas;

class ABitReader;
struct ABuffer;
struct AnotherPacketSource;

struct ATSParser : public RefBase {
    enum DiscontinuityType {
        DISCONTINUITY_NONE              = 0,
        DISCONTINUITY_TIME              = 1,
        DISCONTINUITY_AUDIO_FORMAT      = 2,
        DISCONTINUITY_VIDEO_FORMAT      = 4,
        DISCONTINUITY_ABSOLUTE_TIME     = 8,
        DISCONTINUITY_TIME_OFFSET       = 16,

        // For legacy reasons this also implies a time discontinuity.
        DISCONTINUITY_FORMATCHANGE      =
            DISCONTINUITY_AUDIO_FORMAT
                | DISCONTINUITY_VIDEO_FORMAT
                | DISCONTINUITY_TIME,
        DISCONTINUITY_FORMAT_ONLY       =
            DISCONTINUITY_AUDIO_FORMAT
                | DISCONTINUITY_VIDEO_FORMAT,
    };

    enum Flags {
        // The 90kHz clock (PTS/DTS) is absolute, i.e. PTS=0 corresponds to
        // a media time of 0.
        // If this flag is _not_ specified, the first PTS encountered in a
        // program of this stream will be assumed to correspond to media time 0
        // instead.
        TS_TIMESTAMPS_ARE_ABSOLUTE = 1,
        // Video PES packets contain exactly one (aligned) access unit.
        ALIGNED_VIDEO_DATA         = 2,
    };

    enum SourceType {
        VIDEO = 0,
        AUDIO = 1,
        META  = 2,
        NUM_SOURCE_TYPES = 3
    };

    // Event is used to signal sync point event at feedTSPacket().
    struct SyncEvent {
        explicit SyncEvent(off64_t offset);

        void init(off64_t offset, const sp<AnotherPacketSource> &source,
                int64_t timeUs, SourceType type);

        bool hasReturnedData() const { return mHasReturnedData; }
        void reset();
        off64_t getOffset() const { return mOffset; }
        const sp<AnotherPacketSource> &getMediaSource() const { return mMediaSource; }
        int64_t getTimeUs() const { return mTimeUs; }
        SourceType getType() const { return mType; }

    private:
        bool mHasReturnedData;
        /*
         * mHasReturnedData == false: the current offset (or undefined if the returned data
                                      has been invalidated via reset())
         * mHasReturnedData == true: the start offset of sync payload
         */
        off64_t mOffset;
        /* The media source object for this event. */
        sp<AnotherPacketSource> mMediaSource;
        /* The timestamp of the sync frame. */
        int64_t mTimeUs;
        SourceType mType;
    };

    explicit ATSParser(uint32_t flags = 0);

    status_t setMediaCas(const sp<ICas> &cas);

    // Feed a TS packet into the parser. uninitialized event with the start
    // offset of this TS packet goes in, and if the parser detects PES with
    // a sync frame, the event will be initiailzed with the start offset of the
    // PES. Note that the offset of the event can be different from what we fed,
    // as a PES may consist of multiple TS packets.
    //
    // Even in the case feedTSPacket() returns non-OK value, event still may be
    // initialized if the parsing failed after the detection.
    status_t feedTSPacket(
            const void *data, size_t size, SyncEvent *event = NULL);

    void signalDiscontinuity(
            DiscontinuityType type, const sp<AMessage> &extra);

    void signalEOS(status_t finalResult);

    sp<AnotherPacketSource> getSource(SourceType type);
    bool hasSource(SourceType type) const;

    bool PTSTimeDeltaEstablished();

    int64_t getFirstPTSTimeUs();

    void signalNewSampleAesKey(const sp<AMessage> &keyItem);

    enum {
        // From ISO/IEC 13818-1: 2000 (E), Table 2-29
        STREAMTYPE_RESERVED             = 0x00,
        STREAMTYPE_MPEG1_VIDEO          = 0x01,
        STREAMTYPE_MPEG2_VIDEO          = 0x02,
        STREAMTYPE_MPEG1_AUDIO          = 0x03,
        STREAMTYPE_MPEG2_AUDIO          = 0x04,
        STREAMTYPE_PES_PRIVATE_DATA     = 0x06,
        STREAMTYPE_MPEG2_AUDIO_ADTS     = 0x0f,
        STREAMTYPE_MPEG4_VIDEO          = 0x10,
        STREAMTYPE_METADATA             = 0x15,
        STREAMTYPE_H264                 = 0x1b,

        // From ATSC A/53 Part 3:2009, 6.7.1
        STREAMTYPE_AC3                  = 0x81,

        // Stream type 0x83 is non-standard,
        // it could be LPCM or TrueHD AC3
        STREAMTYPE_LPCM_AC3             = 0x83,
        STREAMTYPE_EAC3                 = 0x87,

        //Sample Encrypted types
        STREAMTYPE_H264_ENCRYPTED       = 0xDB,
        STREAMTYPE_AAC_ENCRYPTED        = 0xCF,
        STREAMTYPE_AC3_ENCRYPTED        = 0xC1,
    };

    enum {
        // From ISO/IEC 13818-1: 2007 (E), Table 2-45
        DESCRIPTOR_CA                   = 0x09,

        // DVB BlueBook A038 Table 12
        DESCRIPTOR_DVB_EXTENSION        = 0x7F,
    };

    // DVB BlueBook A038 Table 109
    enum {
        EXT_DESCRIPTOR_DVB_AC4                  = 0x15,
        EXT_DESCRIPTOR_DVB_AUDIO_PRESELECTION   = 0x19,
        EXT_DESCRIPTOR_DVB_RESERVED_MAX         = 0x7F,
    };

protected:
    virtual ~ATSParser();

private:
    struct Program;
    struct Stream;
    struct PSISection;
    struct CasManager;
    struct CADescriptor {
        CADescriptor() : mPID(0), mSystemID(-1) {}
        unsigned mPID;
        int32_t mSystemID;
        std::vector<uint8_t> mPrivateData;
    };

    struct StreamInfo {
        unsigned mType;
        unsigned mTypeExt;
        unsigned mPID;
        CADescriptor mCADescriptor;
        AudioPresentationCollection mAudioPresentations;
    };

    sp<CasManager> mCasManager;

    uint32_t mFlags;
    Vector<sp<Program> > mPrograms;

    // Keyed by PID
    KeyedVector<unsigned, sp<PSISection> > mPSISections;

    int64_t mAbsoluteTimeAnchorUs;

    bool mTimeOffsetValid;
    int64_t mTimeOffsetUs;
    int64_t mLastRecoveredPTS;

    size_t mNumTSPacketsParsed;

    sp<AMessage> mSampleAesKeyItem;

    void parseProgramAssociationTable(ABitReader *br);
    void parseProgramMap(ABitReader *br);
    // Parse PES packet where br is pointing to. If the PES contains a sync
    // frame, set event with the time and the start offset of this PES.
    // Note that the method itself does not touch event.
    void parsePES(ABitReader *br, SyncEvent *event);

    // Strip remaining packet headers and pass to appropriate program/stream
    // to parse the payload. If the payload turns out to be PES and contains
    // a sync frame, event shall be set with the time and start offset of the
    // PES.
    // Note that the method itself does not touch event.
    status_t parsePID(
        ABitReader *br, unsigned PID,
        unsigned continuity_counter,
        unsigned payload_unit_start_indicator,
        unsigned transport_scrambling_control,
        unsigned random_access_indicator,
        SyncEvent *event);

    status_t parseAdaptationField(
            ABitReader *br, unsigned PID, unsigned *random_access_indicator);

    // see feedTSPacket().
    status_t parseTS(ABitReader *br, SyncEvent *event);

    void updatePCR(unsigned PID, uint64_t PCR, uint64_t byteOffsetFromStart);

    uint64_t mPCR[2];
    uint64_t mPCRBytes[2];
    int64_t mSystemTimeUs[2];
    size_t mNumPCRs;

    DISALLOW_EVIL_CONSTRUCTORS(ATSParser);
};

}  // namespace android

#endif  // A_TS_PARSER_H_
