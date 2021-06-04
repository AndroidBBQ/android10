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

//#define LOG_NDEBUG 0
#define LOG_TAG "ATSParser"
#include <utils/Log.h>
#include "ATSParser.h"
#include "AnotherPacketSource.h"
#include "CasManager.h"
#include "ESQueue.h"

#include <android/hardware/cas/native/1.0/IDescrambler.h>
#include <android/hidl/allocator/1.0/IAllocator.h>
#include <android/hidl/memory/1.0/IMemory.h>
#include <cutils/native_handle.h>
#include <hidlmemory/mapping.h>
#include <media/cas/DescramblerAPI.h>
#include <media/stagefright/foundation/ABitReader.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/ByteUtils.h>
#include <media/stagefright/foundation/MediaKeys.h>
#include <media/stagefright/foundation/avc_utils.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaData.h>
#include <media/IStreamSource.h>
#include <utils/KeyedVector.h>
#include <utils/Vector.h>

#include <inttypes.h>

namespace android {
using hardware::hidl_string;
using hardware::hidl_vec;
using hardware::hidl_memory;
using namespace hardware::cas::V1_0;
using namespace hardware::cas::native::V1_0;
typedef hidl::allocator::V1_0::IAllocator TAllocator;
typedef hidl::memory::V1_0::IMemory TMemory;

// I want the expression "y" evaluated even if verbose logging is off.
#define MY_LOGV(x, y) \
    do { unsigned tmp = y; ALOGV(x, tmp); } while (0)

static const size_t kTSPacketSize = 188;

struct ATSParser::Program : public RefBase {
    Program(ATSParser *parser, unsigned programNumber, unsigned programMapPID,
            int64_t lastRecoveredPTS);

    bool parsePSISection(
            unsigned pid, ABitReader *br, status_t *err);

    // Pass to appropriate stream according to pid, and set event if it's a PES
    // with a sync frame.
    // Note that the method itself does not touch event.
    bool parsePID(
            unsigned pid, unsigned continuity_counter,
            unsigned payload_unit_start_indicator,
            unsigned transport_scrambling_control,
            unsigned random_access_indicator,
            ABitReader *br, status_t *err, SyncEvent *event);

    void signalDiscontinuity(
            DiscontinuityType type, const sp<AMessage> &extra);

    void signalEOS(status_t finalResult);

    sp<AnotherPacketSource> getSource(SourceType type);
    bool hasSource(SourceType type) const;

    int64_t convertPTSToTimestamp(uint64_t PTS);

    bool PTSTimeDeltaEstablished() const {
        return mFirstPTSValid;
    }

    unsigned number() const { return mProgramNumber; }

    void updateProgramMapPID(unsigned programMapPID) {
        mProgramMapPID = programMapPID;
    }

    unsigned programMapPID() const {
        return mProgramMapPID;
    }

    uint32_t parserFlags() const {
        return mParser->mFlags;
    }

    sp<CasManager> casManager() const {
        return mParser->mCasManager;
    }

    uint64_t firstPTS() const {
        return mFirstPTS;
    }

    void updateCasSessions();

    void signalNewSampleAesKey(const sp<AMessage> &keyItem);

private:

    ATSParser *mParser;
    unsigned mProgramNumber;
    unsigned mProgramMapPID;
    uint32_t mPMTVersion;
    uint32_t mPMT_CRC;
    KeyedVector<unsigned, sp<Stream> > mStreams;
    bool mFirstPTSValid;
    uint64_t mFirstPTS;
    int64_t mLastRecoveredPTS;
    sp<AMessage> mSampleAesKeyItem;

    status_t parseProgramMap(ABitReader *br);
    int64_t recoverPTS(uint64_t PTS_33bit);
    bool findCADescriptor(
            ABitReader *br, unsigned infoLength, CADescriptor *caDescriptor);
    bool switchPIDs(const Vector<StreamInfo> &infos);

    DISALLOW_EVIL_CONSTRUCTORS(Program);
};

struct ATSParser::Stream : public RefBase {
    Stream(Program *program, unsigned PCR_PID, const StreamInfo &info);

    unsigned type() const { return mStreamType; }
    unsigned typeExt() const { return mStreamTypeExt; }
    unsigned pid() const { return mElementaryPID; }
    void setPID(unsigned pid) { mElementaryPID = pid; }
    void setAudioPresentations(AudioPresentationCollection audioPresentations) {
        mAudioPresentations = audioPresentations;
    }

    void setCasInfo(
            int32_t systemId,
            const sp<IDescrambler> &descrambler,
            const std::vector<uint8_t> &sessionId);

    // Parse the payload and set event when PES with a sync frame is detected.
    // This method knows when a PES starts; so record mPesStartOffsets in that
    // case.
    status_t parse(
            unsigned continuity_counter,
            unsigned payload_unit_start_indicator,
            unsigned transport_scrambling_control,
            unsigned random_access_indicator,
            ABitReader *br,
            SyncEvent *event);

    void signalDiscontinuity(
            DiscontinuityType type, const sp<AMessage> &extra);

    void signalEOS(status_t finalResult);

    SourceType getSourceType();
    sp<AnotherPacketSource> getSource(SourceType type);

    bool isAudio() const;
    bool isVideo() const;
    bool isMeta() const;

    void signalNewSampleAesKey(const sp<AMessage> &keyItem);

protected:
    virtual ~Stream();

private:
    struct SubSampleInfo {
        size_t subSampleSize;
        unsigned transport_scrambling_mode;
        unsigned random_access_indicator;
    };
    Program *mProgram;
    unsigned mElementaryPID;
    unsigned mStreamType;
    unsigned mStreamTypeExt;
    unsigned mPCR_PID;
    int32_t mExpectedContinuityCounter;

    sp<ABuffer> mBuffer;
    sp<AnotherPacketSource> mSource;
    bool mPayloadStarted;
    bool mEOSReached;

    uint64_t mPrevPTS;
    List<off64_t> mPesStartOffsets;

    ElementaryStreamQueue *mQueue;

    bool mScrambled;
    bool mSampleEncrypted;
    sp<AMessage> mSampleAesKeyItem;
    sp<TMemory> mHidlMemory;
    sp<TAllocator> mHidlAllocator;
    hardware::cas::native::V1_0::SharedBuffer mDescramblerSrcBuffer;
    sp<ABuffer> mDescrambledBuffer;
    List<SubSampleInfo> mSubSamples;
    sp<IDescrambler> mDescrambler;
    AudioPresentationCollection mAudioPresentations;

    // Send audio presentations along with access units.
    void addAudioPresentations(const sp<ABuffer> &buffer);

    // Flush accumulated payload if necessary --- i.e. at EOS or at the start of
    // another payload. event is set if the flushed payload is PES with a sync
    // frame.
    status_t flush(SyncEvent *event);

    // Flush accumulated payload for scrambled streams if necessary --- i.e. at
    // EOS or at the start of another payload. event is set if the flushed
    // payload is PES with a sync frame.
    status_t flushScrambled(SyncEvent *event);

    // Check if a PES packet is scrambled at PES level.
    uint32_t getPesScramblingControl(ABitReader *br, int32_t *pesOffset);

    // Strip and parse PES headers and pass remaining payload into onPayload
    // with parsed metadata. event is set if the PES contains a sync frame.
    status_t parsePES(ABitReader *br, SyncEvent *event);

    // Feed the payload into mQueue and if a packet is identified, queue it
    // into mSource. If the packet is a sync frame. set event with start offset
    // and timestamp of the packet.
    void onPayloadData(
            unsigned PTS_DTS_flags, uint64_t PTS, uint64_t DTS,
            unsigned PES_scrambling_control,
            const uint8_t *data, size_t size,
            int32_t payloadOffset, SyncEvent *event);

    // Ensure internal buffers can hold specified size, and will re-allocate
    // as needed.
    bool ensureBufferCapacity(size_t size);

    DISALLOW_EVIL_CONSTRUCTORS(Stream);
};

struct ATSParser::PSISection : public RefBase {
    PSISection();

    status_t append(const void *data, size_t size);
    void setSkipBytes(uint8_t skip);
    void clear();

    bool isComplete() const;
    bool isEmpty() const;
    bool isCRCOkay() const;

    const uint8_t *data() const;
    size_t size() const;

protected:
    virtual ~PSISection();

private:
    sp<ABuffer> mBuffer;
    uint8_t mSkipBytes;
    static uint32_t CRC_TABLE[];

    DISALLOW_EVIL_CONSTRUCTORS(PSISection);
};

ATSParser::SyncEvent::SyncEvent(off64_t offset)
    : mHasReturnedData(false), mOffset(offset), mTimeUs(0) {}

void ATSParser::SyncEvent::init(off64_t offset, const sp<AnotherPacketSource> &source,
        int64_t timeUs, SourceType type) {
    mHasReturnedData = true;
    mOffset = offset;
    mMediaSource = source;
    mTimeUs = timeUs;
    mType = type;
}

void ATSParser::SyncEvent::reset() {
    mHasReturnedData = false;
}
////////////////////////////////////////////////////////////////////////////////

ATSParser::Program::Program(
        ATSParser *parser, unsigned programNumber, unsigned programMapPID,
        int64_t lastRecoveredPTS)
    : mParser(parser),
      mProgramNumber(programNumber),
      mProgramMapPID(programMapPID),
      mPMTVersion(0xffffffff),
      mPMT_CRC(0xffffffff),
      mFirstPTSValid(false),
      mFirstPTS(0),
      mLastRecoveredPTS(lastRecoveredPTS) {
    ALOGV("new program number %u", programNumber);
}

bool ATSParser::Program::parsePSISection(
        unsigned pid, ABitReader *br, status_t *err) {
    *err = OK;

    if (pid != mProgramMapPID) {
        return false;
    }

    *err = parseProgramMap(br);

    return true;
}

bool ATSParser::Program::parsePID(
        unsigned pid, unsigned continuity_counter,
        unsigned payload_unit_start_indicator,
        unsigned transport_scrambling_control,
        unsigned random_access_indicator,
        ABitReader *br, status_t *err, SyncEvent *event) {
    *err = OK;

    ssize_t index = mStreams.indexOfKey(pid);
    if (index < 0) {
        return false;
    }

    *err = mStreams.editValueAt(index)->parse(
            continuity_counter,
            payload_unit_start_indicator,
            transport_scrambling_control,
            random_access_indicator,
            br, event);

    return true;
}

void ATSParser::Program::signalDiscontinuity(
        DiscontinuityType type, const sp<AMessage> &extra) {
    int64_t mediaTimeUs;
    if ((type & DISCONTINUITY_TIME)
            && extra != NULL
            && extra->findInt64(
                kATSParserKeyMediaTimeUs, &mediaTimeUs)) {
        mFirstPTSValid = false;
    }

    for (size_t i = 0; i < mStreams.size(); ++i) {
        mStreams.editValueAt(i)->signalDiscontinuity(type, extra);
    }
}

void ATSParser::Program::signalEOS(status_t finalResult) {
    for (size_t i = 0; i < mStreams.size(); ++i) {
        mStreams.editValueAt(i)->signalEOS(finalResult);
    }
}

bool ATSParser::Program::switchPIDs(const Vector<StreamInfo> &infos) {
    bool success = false;

    if (mStreams.size() == infos.size()) {
        // build type->PIDs map for old and new mapping
        size_t i;
        KeyedVector<int32_t, Vector<int32_t> > oldType2PIDs, newType2PIDs;
        for (i = 0; i < mStreams.size(); ++i) {
            ssize_t index = oldType2PIDs.indexOfKey(mStreams[i]->type());
            if (index < 0) {
                oldType2PIDs.add(mStreams[i]->type(), Vector<int32_t>());
            }
            oldType2PIDs.editValueFor(mStreams[i]->type()).push_back(mStreams[i]->pid());
        }
        for (i = 0; i < infos.size(); ++i) {
            ssize_t index = newType2PIDs.indexOfKey(infos[i].mType);
            if (index < 0) {
                newType2PIDs.add(infos[i].mType, Vector<int32_t>());
            }
            newType2PIDs.editValueFor(infos[i].mType).push_back(infos[i].mPID);
        }

        // we can recover if the number of streams for each type hasn't changed
        if (oldType2PIDs.size() == newType2PIDs.size()) {
            success = true;
            for (i = 0; i < oldType2PIDs.size(); ++i) {
                // KeyedVector is sorted, we just compare key and size of each index
                if (oldType2PIDs.keyAt(i) != newType2PIDs.keyAt(i)
                        || oldType2PIDs[i].size() != newType2PIDs[i].size()) {
                     success = false;
                     break;
                }
            }
        }

        if (success) {
            // save current streams to temp
            KeyedVector<int32_t, sp<Stream> > temp;
            for (i = 0; i < mStreams.size(); ++i) {
                 temp.add(mStreams.keyAt(i), mStreams.editValueAt(i));
            }

            mStreams.clear();
            for (i = 0; i < temp.size(); ++i) {
                // The two checks below shouldn't happen,
                // we already checked above the stream count matches
                ssize_t index = newType2PIDs.indexOfKey(temp[i]->type());
                if (index < 0) {
                    return false;
                }
                Vector<int32_t> &newPIDs = newType2PIDs.editValueAt(index);
                if (newPIDs.isEmpty()) {
                    return false;
                }

                // get the next PID for temp[i]->type() in the new PID map
                Vector<int32_t>::iterator it = newPIDs.begin();

                // change the PID of the stream, and add it back
                temp.editValueAt(i)->setPID(*it);
                mStreams.add(temp[i]->pid(), temp.editValueAt(i));

                // removed the used PID
                newPIDs.erase(it);
            }
        }
    }
    return success;
}

bool ATSParser::Program::findCADescriptor(
        ABitReader *br, unsigned infoLength,
        ATSParser::CADescriptor *caDescriptor) {
    bool found = false;
    while (infoLength > 2) {
        unsigned descriptor_tag = br->getBits(8);
        ALOGV("      tag = 0x%02x", descriptor_tag);

        unsigned descriptor_length = br->getBits(8);
        ALOGV("      len = %u", descriptor_length);

        infoLength -= 2;
        if (descriptor_length > infoLength) {
            break;
        }
        if (descriptor_tag == DESCRIPTOR_CA && descriptor_length >= 4) {
            found = true;
            caDescriptor->mSystemID = br->getBits(16);
            caDescriptor->mPID = br->getBits(16) & 0x1fff;
            infoLength -= 4;
            caDescriptor->mPrivateData.assign(
                    br->data(), br->data() + descriptor_length - 4);
            break;
        } else {
            infoLength -= descriptor_length;
            br->skipBits(descriptor_length * 8);
        }
    }
    br->skipBits(infoLength * 8);
    return found;
}

status_t ATSParser::Program::parseProgramMap(ABitReader *br) {
    unsigned table_id = br->getBits(8);
    ALOGV("  table_id = %u", table_id);
    if (table_id != 0x02u) {
        ALOGE("PMT data error!");
        return ERROR_MALFORMED;
    }
    unsigned section_syntax_indicator = br->getBits(1);
    ALOGV("  section_syntax_indicator = %u", section_syntax_indicator);
    if (section_syntax_indicator != 1u) {
        ALOGE("PMT data error!");
        return ERROR_MALFORMED;
    }

    br->skipBits(1);  // '0'
    MY_LOGV("  reserved = %u", br->getBits(2));

    unsigned section_length = br->getBits(12);
    ALOGV("  section_length = %u", section_length);

    MY_LOGV("  program_number = %u", br->getBits(16));
    MY_LOGV("  reserved = %u", br->getBits(2));
    bool audioPresentationsChanged = false;
    unsigned pmtVersion = br->getBits(5);
    if (pmtVersion != mPMTVersion) {
        audioPresentationsChanged = true;
        mPMTVersion = pmtVersion;
    }
    MY_LOGV("  version_number = %u", pmtVersion);
    MY_LOGV("  current_next_indicator = %u", br->getBits(1));
    MY_LOGV("  section_number = %u", br->getBits(8));
    MY_LOGV("  last_section_number = %u", br->getBits(8));
    MY_LOGV("  reserved = %u", br->getBits(3));

    unsigned PCR_PID = br->getBits(13);
    ALOGV("  PCR_PID = 0x%04x", PCR_PID);

    MY_LOGV("  reserved = %u", br->getBits(4));

    unsigned program_info_length = br->getBits(12);
    ALOGV("  program_info_length = %u", program_info_length);

    // descriptors
    CADescriptor programCA;
    bool hasProgramCA = findCADescriptor(br, program_info_length, &programCA);
    if (hasProgramCA && !mParser->mCasManager->addProgram(
            mProgramNumber, programCA)) {
        return ERROR_MALFORMED;
    }

    Vector<StreamInfo> infos;

    // infoBytesRemaining is the number of bytes that make up the
    // variable length section of ES_infos. It does not include the
    // final CRC.
    int32_t infoBytesRemaining = section_length - 9 - program_info_length - 4;

    while (infoBytesRemaining >= 5) {
        StreamInfo info;
        info.mType = br->getBits(8);
        ALOGV("    stream_type = 0x%02x", info.mType);
        MY_LOGV("    reserved = %u", br->getBits(3));

        info.mPID = br->getBits(13);
        ALOGV("    elementary_PID = 0x%04x", info.mPID);

        MY_LOGV("    reserved = %u", br->getBits(4));

        unsigned ES_info_length = br->getBits(12);
        ALOGV("    ES_info_length = %u", ES_info_length);
        infoBytesRemaining -= 5 + ES_info_length;

        CADescriptor streamCA;
        info.mTypeExt = EXT_DESCRIPTOR_DVB_RESERVED_MAX;

        info.mAudioPresentations.clear();
        bool hasStreamCA = false;
        while (ES_info_length > 2 && infoBytesRemaining >= 0) {
            unsigned descriptor_tag = br->getBits(8);
            ALOGV("      tag = 0x%02x", descriptor_tag);

            unsigned descriptor_length = br->getBits(8);
            ALOGV("      len = %u", descriptor_length);

            ES_info_length -= 2;
            if (descriptor_length > ES_info_length) {
                return ERROR_MALFORMED;
            }
            if (descriptor_tag == DESCRIPTOR_CA && descriptor_length >= 4) {
                hasStreamCA = true;
                streamCA.mSystemID = br->getBits(16);
                streamCA.mPID = br->getBits(16) & 0x1fff;
                ES_info_length -= descriptor_length;
                descriptor_length -= 4;
                streamCA.mPrivateData.assign(br->data(), br->data() + descriptor_length);
                br->skipBits(descriptor_length * 8);
            } else if (info.mType == STREAMTYPE_PES_PRIVATE_DATA &&
                       descriptor_tag == DESCRIPTOR_DVB_EXTENSION && descriptor_length >= 1) {
                unsigned descTagExt = br->getBits(8);
                ALOGV("      tag_ext = 0x%02x", descTagExt);
                ES_info_length -= descriptor_length;
                descriptor_length--;
                // The AC4 descriptor is used in the PSI PMT to identify streams which carry AC4
                // audio.
                if (descTagExt == EXT_DESCRIPTOR_DVB_AC4) {
                    info.mTypeExt = EXT_DESCRIPTOR_DVB_AC4;
                    br->skipBits(descriptor_length * 8);
                } else if (descTagExt == EXT_DESCRIPTOR_DVB_AUDIO_PRESELECTION &&
                           descriptor_length >= 1) {
                    // DVB BlueBook A038 Table 110
                    unsigned num_preselections = br->getBits(5);
                    br->skipBits(3);  // reserved
                    for (unsigned i = 0; i < num_preselections; ++i) {
                        if (br->numBitsLeft() < 16) {
                            ALOGE("Not enough data left in bitreader!");
                            return ERROR_MALFORMED;
                        }
                        AudioPresentationV1 ap;
                        ap.mPresentationId = br->getBits(5);  // preselection_id

                        // audio_rendering_indication
                        ap.mMasteringIndication = static_cast<MasteringIndication>(br->getBits(3));
                        ap.mAudioDescriptionAvailable = (br->getBits(1) == 1);
                        ap.mSpokenSubtitlesAvailable = (br->getBits(1) == 1);
                        ap.mDialogueEnhancementAvailable = (br->getBits(1) == 1);

                        bool interactivity_enabled = (br->getBits(1) == 1);
                        MY_LOGV("      interactivity_enabled = %d", interactivity_enabled);

                        bool language_code_present = (br->getBits(1) == 1);
                        bool text_label_present = (br->getBits(1) == 1);

                        bool multi_stream_info_present = (br->getBits(1) == 1);
                        bool future_extension = (br->getBits(1) == 1);
                        if (language_code_present) {
                            if (br->numBitsLeft() < 24) {
                                ALOGE("Not enough data left in bitreader!");
                                return ERROR_MALFORMED;
                            }
                            char language[4];
                            language[0] = br->getBits(8);
                            language[1] = br->getBits(8);
                            language[2] = br->getBits(8);
                            language[3] = 0;
                            ap.mLanguage = String8(language);
                        }

                        // This maps the presentation id to the message id in the
                        // EXT_DESCRIPTOR_DVB_MESSAGE so that we can get the presentation label.
                        if (text_label_present) {
                            if (br->numBitsLeft() < 8) {
                                ALOGE("Not enough data left in bitreader!");
                                return ERROR_MALFORMED;
                            }
                            unsigned message_id = br->getBits(8);
                            MY_LOGV("      message_id = %u", message_id);
                        }

                        if (multi_stream_info_present) {
                            if (br->numBitsLeft() < 8) {
                                ALOGE("Not enough data left in bitreader!");
                                return ERROR_MALFORMED;
                            }
                            unsigned num_aux_components = br->getBits(3);
                            br->skipBits(5);  // reserved
                            if (br->numBitsLeft() < (num_aux_components * 8)) {
                                ALOGE("Not enough data left in bitreader!");
                                return ERROR_MALFORMED;
                            }
                            br->skipBits(num_aux_components * 8);  // component_tag
                        }
                        if (future_extension) {
                            if (br->numBitsLeft() < 8) {
                                return ERROR_MALFORMED;
                            }
                            br->skipBits(3);  // reserved
                            unsigned future_extension_length = br->getBits(5);
                            if (br->numBitsLeft() < (future_extension_length * 8)) {
                                ALOGE("Not enough data left in bitreader!");
                                return ERROR_MALFORMED;
                            }
                            br->skipBits(future_extension_length * 8);  // future_extension_byte
                        }
                        info.mAudioPresentations.push_back(std::move(ap));
                    }
                } else {
                    br->skipBits(descriptor_length * 8);
                }
            } else {
                ES_info_length -= descriptor_length;
                br->skipBits(descriptor_length * 8);
            }
        }
        if (hasStreamCA && !mParser->mCasManager->addStream(
                mProgramNumber, info.mPID, streamCA)) {
            return ERROR_MALFORMED;
        }
        if (hasProgramCA) {
            info.mCADescriptor = programCA;
        } else if (hasStreamCA) {
            info.mCADescriptor = streamCA;
        }

        infos.push(info);
    }

    if (infoBytesRemaining != 0) {
        ALOGW("Section data remains unconsumed");
    }
    unsigned crc = br->getBits(32);
    if (crc != mPMT_CRC) {
        audioPresentationsChanged = true;
        mPMT_CRC = crc;
    }
    MY_LOGV("  CRC = 0x%08x", crc);

    bool PIDsChanged = false;
    for (size_t i = 0; i < infos.size(); ++i) {
        StreamInfo &info = infos.editItemAt(i);

        ssize_t index = mStreams.indexOfKey(info.mPID);

        if (index >= 0 && mStreams.editValueAt(index)->type() != info.mType) {
            ALOGI("uh oh. stream PIDs have changed.");
            PIDsChanged = true;
            break;
        }
    }

    if (PIDsChanged) {
#if 0
        ALOGI("before:");
        for (size_t i = 0; i < mStreams.size(); ++i) {
            sp<Stream> stream = mStreams.editValueAt(i);

            ALOGI("PID 0x%08x => type 0x%02x", stream->pid(), stream->type());
        }

        ALOGI("after:");
        for (size_t i = 0; i < infos.size(); ++i) {
            StreamInfo &info = infos.editItemAt(i);

            ALOGI("PID 0x%08x => type 0x%02x", info.mPID, info.mType);
        }
#endif

        // we can recover if number of streams for each type remain the same
        bool success = switchPIDs(infos);

        if (!success) {
            ALOGI("Stream PIDs changed and we cannot recover.");
            return ERROR_MALFORMED;
        }
    }

    bool isAddingScrambledStream = false;
    for (size_t i = 0; i < infos.size(); ++i) {
        StreamInfo &info = infos.editItemAt(i);

        if (mParser->mCasManager->isCAPid(info.mPID)) {
            // skip CA streams (EMM/ECM)
            continue;
        }
        ssize_t index = mStreams.indexOfKey(info.mPID);

        if (index < 0) {
            sp<Stream> stream = new Stream(this, PCR_PID, info);

            if (mSampleAesKeyItem != NULL) {
                stream->signalNewSampleAesKey(mSampleAesKeyItem);
            }

            isAddingScrambledStream |= info.mCADescriptor.mSystemID >= 0;
            mStreams.add(info.mPID, stream);
        }
        else if (index >= 0 && mStreams.editValueAt(index)->isAudio()
                 && audioPresentationsChanged) {
            mStreams.editValueAt(index)->setAudioPresentations(info.mAudioPresentations);
        }
    }

    if (isAddingScrambledStream) {
        ALOGI("Receiving scrambled streams without descrambler!");
        return ERROR_DRM_DECRYPT_UNIT_NOT_INITIALIZED;
    }
    return OK;
}

int64_t ATSParser::Program::recoverPTS(uint64_t PTS_33bit) {
    // We only have the lower 33-bit of the PTS. It could overflow within a
    // reasonable amount of time. To handle the wrap-around, use fancy math
    // to get an extended PTS that is within [-0xffffffff, 0xffffffff]
    // of the latest recovered PTS.
    if (mLastRecoveredPTS < 0LL) {
        // Use the original 33bit number for 1st frame, the reason is that
        // if 1st frame wraps to negative that's far away from 0, we could
        // never start. Only start wrapping around from 2nd frame.
        mLastRecoveredPTS = static_cast<int64_t>(PTS_33bit);
    } else {
        mLastRecoveredPTS = static_cast<int64_t>(
                ((mLastRecoveredPTS - static_cast<int64_t>(PTS_33bit) + 0x100000000LL)
                & 0xfffffffe00000000ull) | PTS_33bit);
        // We start from 0, but recovered PTS could be slightly below 0.
        // Clamp it to 0 as rest of the pipeline doesn't take negative pts.
        // (eg. video is read first and starts at 0, but audio starts at 0xfffffff0)
        if (mLastRecoveredPTS < 0LL) {
            ALOGI("Clamping negative recovered PTS (%" PRId64 ") to 0", mLastRecoveredPTS);
            mLastRecoveredPTS = 0LL;
        }
    }

    return mLastRecoveredPTS;
}

sp<AnotherPacketSource> ATSParser::Program::getSource(SourceType type) {
    for (size_t i = 0; i < mStreams.size(); ++i) {
        sp<AnotherPacketSource> source = mStreams.editValueAt(i)->getSource(type);
        if (source != NULL) {
            return source;
        }
    }

    return NULL;
}

bool ATSParser::Program::hasSource(SourceType type) const {
    for (size_t i = 0; i < mStreams.size(); ++i) {
        const sp<Stream> &stream = mStreams.valueAt(i);
        if (type == AUDIO && stream->isAudio()) {
            return true;
        } else if (type == VIDEO && stream->isVideo()) {
            return true;
        } else if (type == META && stream->isMeta()) {
            return true;
        }
    }

    return false;
}

int64_t ATSParser::Program::convertPTSToTimestamp(uint64_t PTS) {
    PTS = recoverPTS(PTS);

    if (!(mParser->mFlags & TS_TIMESTAMPS_ARE_ABSOLUTE)) {
        if (!mFirstPTSValid) {
            mFirstPTSValid = true;
            mFirstPTS = PTS;
            PTS = 0;
        } else if (PTS < mFirstPTS) {
            PTS = 0;
        } else {
            PTS -= mFirstPTS;
        }
    }

    int64_t timeUs = (PTS * 100) / 9;

    if (mParser->mAbsoluteTimeAnchorUs >= 0LL) {
        timeUs += mParser->mAbsoluteTimeAnchorUs;
    }

    if (mParser->mTimeOffsetValid) {
        timeUs += mParser->mTimeOffsetUs;
    }

    return timeUs;
}

void ATSParser::Program::updateCasSessions() {
    for (size_t i = 0; i < mStreams.size(); ++i) {
        sp<Stream> &stream = mStreams.editValueAt(i);
        sp<IDescrambler> descrambler;
        std::vector<uint8_t> sessionId;
        int32_t systemId;
        if (mParser->mCasManager->getCasInfo(mProgramNumber, stream->pid(),
                &systemId, &descrambler, &sessionId)) {
            stream->setCasInfo(systemId, descrambler, sessionId);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
static const size_t kInitialStreamBufferSize = 192 * 1024;

ATSParser::Stream::Stream(
        Program *program, unsigned PCR_PID, const StreamInfo &info)
    : mProgram(program),
      mElementaryPID(info.mPID),
      mStreamType(info.mType),
      mStreamTypeExt(info.mTypeExt),
      mPCR_PID(PCR_PID),
      mExpectedContinuityCounter(-1),
      mPayloadStarted(false),
      mEOSReached(false),
      mPrevPTS(0),
      mQueue(NULL),
      mScrambled(info.mCADescriptor.mSystemID >= 0),
      mAudioPresentations(info.mAudioPresentations) {
    mSampleEncrypted =
            mStreamType == STREAMTYPE_H264_ENCRYPTED ||
            mStreamType == STREAMTYPE_AAC_ENCRYPTED  ||
            mStreamType == STREAMTYPE_AC3_ENCRYPTED;

    ALOGV("new stream PID 0x%02x, type 0x%02x, scrambled %d, SampleEncrypted: %d",
            info.mPID, info.mType, mScrambled, mSampleEncrypted);

    uint32_t flags = 0;
    if (((isVideo() || isAudio()) && mScrambled)) {
        flags = ElementaryStreamQueue::kFlag_ScrambledData;
    } else if (mSampleEncrypted) {
        flags = ElementaryStreamQueue::kFlag_SampleEncryptedData;
    }

    ElementaryStreamQueue::Mode mode = ElementaryStreamQueue::INVALID;

    switch (mStreamType) {
        case STREAMTYPE_H264:
        case STREAMTYPE_H264_ENCRYPTED:
            mode = ElementaryStreamQueue::H264;
            flags |= (mProgram->parserFlags() & ALIGNED_VIDEO_DATA) ?
                    ElementaryStreamQueue::kFlag_AlignedData : 0;
            break;

        case STREAMTYPE_MPEG2_AUDIO_ADTS:
        case STREAMTYPE_AAC_ENCRYPTED:
            mode = ElementaryStreamQueue::AAC;
            break;

        case STREAMTYPE_MPEG1_AUDIO:
        case STREAMTYPE_MPEG2_AUDIO:
            mode = ElementaryStreamQueue::MPEG_AUDIO;
            break;

        case STREAMTYPE_MPEG1_VIDEO:
        case STREAMTYPE_MPEG2_VIDEO:
            mode = ElementaryStreamQueue::MPEG_VIDEO;
            break;

        case STREAMTYPE_MPEG4_VIDEO:
            mode = ElementaryStreamQueue::MPEG4_VIDEO;
            break;

        case STREAMTYPE_LPCM_AC3:
        case STREAMTYPE_AC3:
        case STREAMTYPE_AC3_ENCRYPTED:
            mode = ElementaryStreamQueue::AC3;
            break;

        case STREAMTYPE_EAC3:
            mode = ElementaryStreamQueue::EAC3;
            break;

        case STREAMTYPE_PES_PRIVATE_DATA:
            if (mStreamTypeExt == EXT_DESCRIPTOR_DVB_AC4) {
                mode = ElementaryStreamQueue::AC4;
            }
            break;

        case STREAMTYPE_METADATA:
            mode = ElementaryStreamQueue::METADATA;
            break;

        default:
            ALOGE("stream PID 0x%02x has invalid stream type 0x%02x",
                    info.mPID, info.mType);
            return;
    }

    mQueue = new ElementaryStreamQueue(mode, flags);

    if (mQueue != NULL) {
        if (mSampleAesKeyItem != NULL) {
            mQueue->signalNewSampleAesKey(mSampleAesKeyItem);
        }

        ensureBufferCapacity(kInitialStreamBufferSize);

        if (mScrambled && (isAudio() || isVideo())) {
            // Set initial format to scrambled
            sp<MetaData> meta = new MetaData();
            meta->setCString(kKeyMIMEType,
                    isAudio() ? MEDIA_MIMETYPE_AUDIO_SCRAMBLED
                              : MEDIA_MIMETYPE_VIDEO_SCRAMBLED);
            // for MediaExtractor.CasInfo
            const CADescriptor &descriptor = info.mCADescriptor;
            meta->setInt32(kKeyCASystemID, descriptor.mSystemID);

            meta->setData(kKeyCAPrivateData, 0,
                    descriptor.mPrivateData.data(),
                    descriptor.mPrivateData.size());

            mSource = new AnotherPacketSource(meta);
        }
    }
}

ATSParser::Stream::~Stream() {
    delete mQueue;
    mQueue = NULL;
}

bool ATSParser::Stream::ensureBufferCapacity(size_t neededSize) {
    if (mBuffer != NULL && mBuffer->capacity() >= neededSize) {
        return true;
    }

    ALOGV("ensureBufferCapacity: current size %zu, new size %zu, scrambled %d",
            mBuffer == NULL ? 0 : mBuffer->capacity(), neededSize, mScrambled);

    sp<ABuffer> newBuffer, newScrambledBuffer;
    sp<TMemory> newMem;
    if (mScrambled) {
        if (mHidlAllocator == nullptr) {
            mHidlAllocator = TAllocator::getService("ashmem");
            if (mHidlAllocator == nullptr) {
                ALOGE("[stream %d] can't get hidl allocator", mElementaryPID);
                return false;
            }
        }

        hidl_memory hidlMemToken;
        bool success;
        auto transStatus = mHidlAllocator->allocate(
                neededSize,
                [&success, &hidlMemToken](
                        bool s,
                        hidl_memory const& m) {
                    success = s;
                    hidlMemToken = m;
                });

        if (!transStatus.isOk()) {
            ALOGE("[stream %d] hidl allocator failed at the transport: %s",
                    mElementaryPID, transStatus.description().c_str());
            return false;
        }
        if (!success) {
            ALOGE("[stream %d] hidl allocator failed", mElementaryPID);
            return false;
        }
        newMem = mapMemory(hidlMemToken);
        if (newMem == nullptr || newMem->getPointer() == nullptr) {
            ALOGE("[stream %d] hidl failed to map memory", mElementaryPID);
            return false;
        }

        newScrambledBuffer = new ABuffer(newMem->getPointer(), newMem->getSize());

        if (mDescrambledBuffer != NULL) {
            memcpy(newScrambledBuffer->data(),
                    mDescrambledBuffer->data(), mDescrambledBuffer->size());
            newScrambledBuffer->setRange(0, mDescrambledBuffer->size());
        } else {
            newScrambledBuffer->setRange(0, 0);
        }
        mHidlMemory = newMem;
        mDescrambledBuffer = newScrambledBuffer;

        mDescramblerSrcBuffer.heapBase = hidlMemToken;
        mDescramblerSrcBuffer.offset = 0ULL;
        mDescramblerSrcBuffer.size =  (uint64_t)neededSize;

        ALOGD("[stream %d] created shared buffer for descrambling, size %zu",
                mElementaryPID, neededSize);
    } else {
        // Align to multiples of 64K.
        neededSize = (neededSize + 65535) & ~65535;
    }

    newBuffer = new ABuffer(neededSize);
    if (mBuffer != NULL) {
        memcpy(newBuffer->data(), mBuffer->data(), mBuffer->size());
        newBuffer->setRange(0, mBuffer->size());
    } else {
        newBuffer->setRange(0, 0);
    }
    mBuffer = newBuffer;
    return true;
}

status_t ATSParser::Stream::parse(
        unsigned continuity_counter,
        unsigned payload_unit_start_indicator,
        unsigned transport_scrambling_control,
        unsigned random_access_indicator,
        ABitReader *br, SyncEvent *event) {
    if (mQueue == NULL) {
        return OK;
    }

    if (mExpectedContinuityCounter >= 0
            && (unsigned)mExpectedContinuityCounter != continuity_counter) {
        ALOGI("discontinuity on stream pid 0x%04x", mElementaryPID);

        mPayloadStarted = false;
        mPesStartOffsets.clear();
        mBuffer->setRange(0, 0);
        mSubSamples.clear();
        mExpectedContinuityCounter = -1;

#if 0
        // Uncomment this if you'd rather see no corruption whatsoever on
        // screen and suspend updates until we come across another IDR frame.

        if (mStreamType == STREAMTYPE_H264) {
            ALOGI("clearing video queue");
            mQueue->clear(true /* clearFormat */);
        }
#endif

        if (!payload_unit_start_indicator) {
            return OK;
        }
    }

    mExpectedContinuityCounter = (continuity_counter + 1) & 0x0f;

    if (payload_unit_start_indicator) {
        off64_t offset = (event != NULL) ? event->getOffset() : 0;
        if (mPayloadStarted) {
            // Otherwise we run the danger of receiving the trailing bytes
            // of a PES packet that we never saw the start of and assuming
            // we have a a complete PES packet.

            status_t err = flush(event);

            if (err != OK) {
                ALOGW("Error (%08x) happened while flushing; we simply discard "
                      "the PES packet and continue.", err);
            }
        }

        mPayloadStarted = true;
        // There should be at most 2 elements in |mPesStartOffsets|.
        while (mPesStartOffsets.size() >= 2) {
            mPesStartOffsets.erase(mPesStartOffsets.begin());
        }
        mPesStartOffsets.push_back(offset);
    }

    if (!mPayloadStarted) {
        return OK;
    }

    size_t payloadSizeBits = br->numBitsLeft();
    if (payloadSizeBits % 8 != 0u) {
        ALOGE("Wrong value");
        return BAD_VALUE;
    }

    size_t neededSize = mBuffer->size() + payloadSizeBits / 8;
    if (!ensureBufferCapacity(neededSize)) {
        return NO_MEMORY;
    }

    memcpy(mBuffer->data() + mBuffer->size(), br->data(), payloadSizeBits / 8);
    mBuffer->setRange(0, mBuffer->size() + payloadSizeBits / 8);

    if (mScrambled) {
        mSubSamples.push_back({payloadSizeBits / 8,
                 transport_scrambling_control, random_access_indicator});
    }

    return OK;
}

bool ATSParser::Stream::isVideo() const {
    switch (mStreamType) {
        case STREAMTYPE_H264:
        case STREAMTYPE_H264_ENCRYPTED:
        case STREAMTYPE_MPEG1_VIDEO:
        case STREAMTYPE_MPEG2_VIDEO:
        case STREAMTYPE_MPEG4_VIDEO:
            return true;

        default:
            return false;
    }
}

bool ATSParser::Stream::isAudio() const {
    switch (mStreamType) {
        case STREAMTYPE_MPEG1_AUDIO:
        case STREAMTYPE_MPEG2_AUDIO:
        case STREAMTYPE_MPEG2_AUDIO_ADTS:
        case STREAMTYPE_LPCM_AC3:
        case STREAMTYPE_AC3:
        case STREAMTYPE_EAC3:
        case STREAMTYPE_AAC_ENCRYPTED:
        case STREAMTYPE_AC3_ENCRYPTED:
            return true;
        case STREAMTYPE_PES_PRIVATE_DATA:
            return mStreamTypeExt == EXT_DESCRIPTOR_DVB_AC4;

        default:
            return false;
    }
}

bool ATSParser::Stream::isMeta() const {
    if (mStreamType == STREAMTYPE_METADATA) {
        return true;
    }
    return false;
}

void ATSParser::Stream::signalDiscontinuity(
        DiscontinuityType type, const sp<AMessage> &extra) {
    mExpectedContinuityCounter = -1;

    if (mQueue == NULL) {
        return;
    }

    mPayloadStarted = false;
    mPesStartOffsets.clear();
    mEOSReached = false;
    mBuffer->setRange(0, 0);
    mSubSamples.clear();

    bool clearFormat = false;
    if (isAudio()) {
        if (type & DISCONTINUITY_AUDIO_FORMAT) {
            clearFormat = true;
        }
    } else {
        if (type & DISCONTINUITY_VIDEO_FORMAT) {
            clearFormat = true;
        }
    }

    mQueue->clear(clearFormat);

    if (type & DISCONTINUITY_TIME) {
        uint64_t resumeAtPTS;
        if (extra != NULL
                && extra->findInt64(
                    kATSParserKeyResumeAtPTS,
                    (int64_t *)&resumeAtPTS)) {
            int64_t resumeAtMediaTimeUs =
                mProgram->convertPTSToTimestamp(resumeAtPTS);

            extra->setInt64("resume-at-mediaTimeUs", resumeAtMediaTimeUs);
        }
    }

    if (mSource != NULL) {
        sp<MetaData> meta = mSource->getFormat();
        const char* mime;
        if (clearFormat && meta != NULL && meta->findCString(kKeyMIMEType, &mime)
                && (!strncasecmp(mime, MEDIA_MIMETYPE_AUDIO_SCRAMBLED, 15)
                 || !strncasecmp(mime, MEDIA_MIMETYPE_VIDEO_SCRAMBLED, 15))){
            mSource->clear();
        } else {
            mSource->queueDiscontinuity(type, extra, true);
        }
    }
}

void ATSParser::Stream::signalEOS(status_t finalResult) {
    if (mSource != NULL) {
        mSource->signalEOS(finalResult);
    }
    mEOSReached = true;
    flush(NULL);
}

status_t ATSParser::Stream::parsePES(ABitReader *br, SyncEvent *event) {
    const uint8_t *basePtr = br->data();

    unsigned packet_startcode_prefix = br->getBits(24);

    ALOGV("packet_startcode_prefix = 0x%08x", packet_startcode_prefix);

    if (packet_startcode_prefix != 1) {
        ALOGV("Supposedly payload_unit_start=1 unit does not start "
             "with startcode.");

        return ERROR_MALFORMED;
    }

    unsigned stream_id = br->getBits(8);
    ALOGV("stream_id = 0x%02x", stream_id);

    unsigned PES_packet_length = br->getBits(16);
    ALOGV("PES_packet_length = %u", PES_packet_length);

    if (stream_id != 0xbc  // program_stream_map
            && stream_id != 0xbe  // padding_stream
            && stream_id != 0xbf  // private_stream_2
            && stream_id != 0xf0  // ECM
            && stream_id != 0xf1  // EMM
            && stream_id != 0xff  // program_stream_directory
            && stream_id != 0xf2  // DSMCC
            && stream_id != 0xf8) {  // H.222.1 type E
        if (br->getBits(2) != 2u) {
            return ERROR_MALFORMED;
        }

        unsigned PES_scrambling_control = br->getBits(2);
        ALOGV("PES_scrambling_control = %u", PES_scrambling_control);

        MY_LOGV("PES_priority = %u", br->getBits(1));
        MY_LOGV("data_alignment_indicator = %u", br->getBits(1));
        MY_LOGV("copyright = %u", br->getBits(1));
        MY_LOGV("original_or_copy = %u", br->getBits(1));

        unsigned PTS_DTS_flags = br->getBits(2);
        ALOGV("PTS_DTS_flags = %u", PTS_DTS_flags);

        unsigned ESCR_flag = br->getBits(1);
        ALOGV("ESCR_flag = %u", ESCR_flag);

        unsigned ES_rate_flag = br->getBits(1);
        ALOGV("ES_rate_flag = %u", ES_rate_flag);

        unsigned DSM_trick_mode_flag = br->getBits(1);
        ALOGV("DSM_trick_mode_flag = %u", DSM_trick_mode_flag);

        unsigned additional_copy_info_flag = br->getBits(1);
        ALOGV("additional_copy_info_flag = %u", additional_copy_info_flag);

        MY_LOGV("PES_CRC_flag = %u", br->getBits(1));
        MY_LOGV("PES_extension_flag = %u", br->getBits(1));

        unsigned PES_header_data_length = br->getBits(8);
        ALOGV("PES_header_data_length = %u", PES_header_data_length);

        unsigned optional_bytes_remaining = PES_header_data_length;

        uint64_t PTS = 0, DTS = 0;

        if (PTS_DTS_flags == 2 || PTS_DTS_flags == 3) {
            if (optional_bytes_remaining < 5u) {
                return ERROR_MALFORMED;
            }

            if (br->getBits(4) != PTS_DTS_flags) {
                return ERROR_MALFORMED;
            }
            PTS = ((uint64_t)br->getBits(3)) << 30;
            if (br->getBits(1) != 1u) {
                return ERROR_MALFORMED;
            }
            PTS |= ((uint64_t)br->getBits(15)) << 15;
            if (br->getBits(1) != 1u) {
                return ERROR_MALFORMED;
            }
            PTS |= br->getBits(15);
            if (br->getBits(1) != 1u) {
                return ERROR_MALFORMED;
            }

            ALOGV("PTS = 0x%016" PRIx64 " (%.2f)", PTS, PTS / 90000.0);

            optional_bytes_remaining -= 5;

            if (PTS_DTS_flags == 3) {
                if (optional_bytes_remaining < 5u) {
                    return ERROR_MALFORMED;
                }

                if (br->getBits(4) != 1u) {
                    return ERROR_MALFORMED;
                }

                DTS = ((uint64_t)br->getBits(3)) << 30;
                if (br->getBits(1) != 1u) {
                    return ERROR_MALFORMED;
                }
                DTS |= ((uint64_t)br->getBits(15)) << 15;
                if (br->getBits(1) != 1u) {
                    return ERROR_MALFORMED;
                }
                DTS |= br->getBits(15);
                if (br->getBits(1) != 1u) {
                    return ERROR_MALFORMED;
                }

                ALOGV("DTS = %" PRIu64, DTS);

                optional_bytes_remaining -= 5;
            }
        }

        if (ESCR_flag) {
            if (optional_bytes_remaining < 6u) {
                return ERROR_MALFORMED;
            }

            br->getBits(2);

            uint64_t ESCR = ((uint64_t)br->getBits(3)) << 30;
            if (br->getBits(1) != 1u) {
                return ERROR_MALFORMED;
            }
            ESCR |= ((uint64_t)br->getBits(15)) << 15;
            if (br->getBits(1) != 1u) {
                return ERROR_MALFORMED;
            }
            ESCR |= br->getBits(15);
            if (br->getBits(1) != 1u) {
                return ERROR_MALFORMED;
            }

            ALOGV("ESCR = %" PRIu64, ESCR);
            MY_LOGV("ESCR_extension = %u", br->getBits(9));

            if (br->getBits(1) != 1u) {
                return ERROR_MALFORMED;
            }

            optional_bytes_remaining -= 6;
        }

        if (ES_rate_flag) {
            if (optional_bytes_remaining < 3u) {
                return ERROR_MALFORMED;
            }

            if (br->getBits(1) != 1u) {
                return ERROR_MALFORMED;
            }
            MY_LOGV("ES_rate = %u", br->getBits(22));
            if (br->getBits(1) != 1u) {
                return ERROR_MALFORMED;
            }

            optional_bytes_remaining -= 3;
        }

        br->skipBits(optional_bytes_remaining * 8);

        // ES data follows.
        int32_t pesOffset = br->data() - basePtr;

        if (PES_packet_length != 0) {
            if (PES_packet_length < PES_header_data_length + 3) {
                return ERROR_MALFORMED;
            }

            unsigned dataLength =
                PES_packet_length - 3 - PES_header_data_length;

            if (br->numBitsLeft() < dataLength * 8) {
                ALOGE("PES packet does not carry enough data to contain "
                     "payload. (numBitsLeft = %zu, required = %u)",
                     br->numBitsLeft(), dataLength * 8);

                return ERROR_MALFORMED;
            }

            ALOGV("There's %u bytes of payload, PES_packet_length=%u, offset=%d",
                    dataLength, PES_packet_length, pesOffset);

            onPayloadData(
                    PTS_DTS_flags, PTS, DTS, PES_scrambling_control,
                    br->data(), dataLength, pesOffset, event);

            br->skipBits(dataLength * 8);
        } else {
            onPayloadData(
                    PTS_DTS_flags, PTS, DTS, PES_scrambling_control,
                    br->data(), br->numBitsLeft() / 8, pesOffset, event);

            size_t payloadSizeBits = br->numBitsLeft();
            if (payloadSizeBits % 8 != 0u) {
                return ERROR_MALFORMED;
            }

            ALOGV("There's %zu bytes of payload, offset=%d",
                    payloadSizeBits / 8, pesOffset);
        }
    } else if (stream_id == 0xbe) {  // padding_stream
        if (PES_packet_length == 0u) {
            return ERROR_MALFORMED;
        }
        br->skipBits(PES_packet_length * 8);
    } else {
        if (PES_packet_length == 0u) {
            return ERROR_MALFORMED;
        }
        br->skipBits(PES_packet_length * 8);
    }

    return OK;
}

uint32_t ATSParser::Stream::getPesScramblingControl(
        ABitReader *br, int32_t *pesOffset) {
    unsigned packet_startcode_prefix = br->getBits(24);

    ALOGV("packet_startcode_prefix = 0x%08x", packet_startcode_prefix);

    if (packet_startcode_prefix != 1) {
        ALOGV("unit does not start with startcode.");
        return 0;
    }

    if (br->numBitsLeft() < 48) {
        return 0;
    }

    unsigned stream_id = br->getBits(8);
    ALOGV("stream_id = 0x%02x", stream_id);

    br->skipBits(16); // PES_packet_length

    if (stream_id != 0xbc  // program_stream_map
            && stream_id != 0xbe  // padding_stream
            && stream_id != 0xbf  // private_stream_2
            && stream_id != 0xf0  // ECM
            && stream_id != 0xf1  // EMM
            && stream_id != 0xff  // program_stream_directory
            && stream_id != 0xf2  // DSMCC
            && stream_id != 0xf8) {  // H.222.1 type E
        if (br->getBits(2) != 2u) {
            return 0;
        }

        unsigned PES_scrambling_control = br->getBits(2);
        ALOGV("PES_scrambling_control = %u", PES_scrambling_control);

        if (PES_scrambling_control == 0) {
            return 0;
        }

        br->skipBits(12); // don't care

        unsigned PES_header_data_length = br->getBits(8);
        ALOGV("PES_header_data_length = %u", PES_header_data_length);

        if (PES_header_data_length * 8 > br->numBitsLeft()) {
            return 0;
        }

        *pesOffset = 9 + PES_header_data_length;
        ALOGD("found PES_scrambling_control=%d, PES offset=%d",
                PES_scrambling_control, *pesOffset);
        return PES_scrambling_control;
    }

    return 0;
}

status_t ATSParser::Stream::flushScrambled(SyncEvent *event) {
    if (mDescrambler == NULL) {
        ALOGE("received scrambled packets without descrambler!");
        return UNKNOWN_ERROR;
    }

    if (mDescrambledBuffer == NULL || mHidlMemory == NULL) {
        ALOGE("received scrambled packets without shared memory!");

        return UNKNOWN_ERROR;
    }

    int32_t pesOffset = 0;
    int32_t descrambleSubSamples = 0, descrambleBytes = 0;
    uint32_t tsScramblingControl = 0, pesScramblingControl = 0;

    // First, go over subsamples to find TS-level scrambling key id, and
    // calculate how many subsample we need to descramble (assuming we don't
    // have PES-level scrambling).
    for (auto it = mSubSamples.begin(); it != mSubSamples.end(); it++) {
        if (it->transport_scrambling_mode != 0) {
            // TODO: handle keyId change, use the first non-zero keyId for now.
            if (tsScramblingControl == 0) {
                tsScramblingControl = it->transport_scrambling_mode;
            }
        }
        if (tsScramblingControl == 0 || descrambleSubSamples == 0
                || !mQueue->isScrambled()) {
            descrambleSubSamples++;
            descrambleBytes += it->subSampleSize;
        }
    }
    // If not scrambled at TS-level, check PES-level scrambling
    if (tsScramblingControl == 0) {
        ABitReader br(mBuffer->data(), mBuffer->size());
        pesScramblingControl = getPesScramblingControl(&br, &pesOffset);
        // If not scrambled at PES-level either, or scrambled at PES-level but
        // requires output to remain scrambled, we don't need to descramble
        // anything.
        if (pesScramblingControl == 0 || mQueue->isScrambled()) {
            descrambleSubSamples = 0;
            descrambleBytes = 0;
        }
    }

    uint32_t sctrl = tsScramblingControl != 0 ?
            tsScramblingControl : pesScramblingControl;
    if (mQueue->isScrambled()) {
        sctrl |= DescramblerPlugin::kScrambling_Flag_PesHeader;
    }

    // Perform the 1st pass descrambling if needed
    if (descrambleBytes > 0) {
        memcpy(mDescrambledBuffer->data(), mBuffer->data(), descrambleBytes);
        mDescrambledBuffer->setRange(0, mBuffer->size());

        hidl_vec<SubSample> subSamples;
        subSamples.resize(descrambleSubSamples);

        int32_t i = 0;
        for (auto it = mSubSamples.begin();
                it != mSubSamples.end() && i < descrambleSubSamples; it++, i++) {
            if (it->transport_scrambling_mode != 0 || pesScramblingControl != 0) {
                subSamples[i].numBytesOfClearData = 0;
                subSamples[i].numBytesOfEncryptedData = it->subSampleSize;
            } else {
                subSamples[i].numBytesOfClearData = it->subSampleSize;
                subSamples[i].numBytesOfEncryptedData = 0;
            }
        }

        // If scrambled at PES-level, PES header is in the clear
        if (pesScramblingControl != 0) {
            subSamples[0].numBytesOfClearData = pesOffset;
            subSamples[0].numBytesOfEncryptedData -= pesOffset;
        }

        Status status = Status::OK;
        uint32_t bytesWritten = 0;
        hidl_string detailedError;

        DestinationBuffer dstBuffer;
        dstBuffer.type = BufferType::SHARED_MEMORY;
        dstBuffer.nonsecureMemory = mDescramblerSrcBuffer;

        auto returnVoid = mDescrambler->descramble(
                (ScramblingControl) sctrl,
                subSamples,
                mDescramblerSrcBuffer,
                0 /*srcOffset*/,
                dstBuffer,
                0 /*dstOffset*/,
                [&status, &bytesWritten, &detailedError] (
                        Status _status, uint32_t _bytesWritten,
                        const hidl_string& _detailedError) {
                    status = _status;
                    bytesWritten = _bytesWritten;
                    detailedError = _detailedError;
                });

        if (!returnVoid.isOk() || status != Status::OK) {
            ALOGE("[stream %d] descramble failed, trans=%s, status=%d",
                    mElementaryPID, returnVoid.description().c_str(), status);
            return UNKNOWN_ERROR;
        }

        ALOGV("[stream %d] descramble succeeded, %d bytes",
                mElementaryPID, bytesWritten);

        // Set descrambleBytes to the returned result.
        // Note that this might be smaller than the total length of input data.
        // (eg. when we're descrambling the PES header portion of a secure stream,
        // the plugin might cut it off right after the PES header.)
        descrambleBytes = bytesWritten;
    }

    // |buffer| points to the buffer from which we'd parse the PES header.
    // When the output stream is scrambled, it points to mDescrambledBuffer
    // (unless all packets in this PES are actually clear, in which case,
    // it points to mBuffer since we never copied into mDescrambledBuffer).
    // When the output stream is clear, it points to mBuffer, and we'll
    // copy all descrambled data back to mBuffer.
    sp<ABuffer> buffer = mBuffer;
    if (mQueue->isScrambled()) {
        // Queue subSample info for scrambled queue
        sp<ABuffer> clearSizesBuffer = new ABuffer(mSubSamples.size() * 4);
        sp<ABuffer> encSizesBuffer = new ABuffer(mSubSamples.size() * 4);
        int32_t *clearSizePtr = (int32_t*)clearSizesBuffer->data();
        int32_t *encSizePtr = (int32_t*)encSizesBuffer->data();
        int32_t isSync = 0;
        int32_t i = 0;
        for (auto it = mSubSamples.begin();
                it != mSubSamples.end(); it++, i++) {
            if ((it->transport_scrambling_mode == 0
                    && pesScramblingControl == 0)) {
                clearSizePtr[i] = it->subSampleSize;
                encSizePtr[i] = 0;
            } else {
                clearSizePtr[i] = 0;
                encSizePtr[i] = it->subSampleSize;
            }
            isSync |= it->random_access_indicator;
        }

        // If scrambled at PES-level, PES header is in the clear
        if (pesScramblingControl != 0) {
            clearSizePtr[0] = pesOffset;
            encSizePtr[0] -= pesOffset;
        }
        // Pass the original TS subsample size now. The PES header adjust
        // will be applied when the scrambled AU is dequeued.
        // Note that if descrambleBytes is 0, it means this PES contains only
        // all ts packets, leadingClearBytes is entire buffer size.
        mQueue->appendScrambledData(
                mBuffer->data(), mBuffer->size(),
                (descrambleBytes > 0) ? descrambleBytes : mBuffer->size(),
                sctrl, isSync, clearSizesBuffer, encSizesBuffer);

        if (descrambleBytes > 0) {
            buffer = mDescrambledBuffer;
        }
    } else {
        memcpy(mBuffer->data(), mDescrambledBuffer->data(), descrambleBytes);
    }

    ABitReader br(buffer->data(), buffer->size());
    status_t err = parsePES(&br, event);

    if (err != OK) {
        ALOGE("[stream %d] failed to parse descrambled PES, err=%d",
                mElementaryPID, err);
    }

    return err;
}


status_t ATSParser::Stream::flush(SyncEvent *event) {
    if (mBuffer == NULL || mBuffer->size() == 0) {
        return OK;
    }

    ALOGV("flushing stream 0x%04x size = %zu", mElementaryPID, mBuffer->size());

    status_t err = OK;
    if (mScrambled) {
        err = flushScrambled(event);
        mSubSamples.clear();
    } else {
        ABitReader br(mBuffer->data(), mBuffer->size());
        err = parsePES(&br, event);
    }

    mBuffer->setRange(0, 0);

    return err;
}

void ATSParser::Stream::addAudioPresentations(const sp<ABuffer> &buffer) {
    std::ostringstream outStream(std::ios::out);
    serializeAudioPresentations(mAudioPresentations, &outStream);
    sp<ABuffer> ap = ABuffer::CreateAsCopy(outStream.str().data(), outStream.str().size());
    buffer->meta()->setBuffer("audio-presentation-info", ap);
}

void ATSParser::Stream::onPayloadData(
        unsigned PTS_DTS_flags, uint64_t PTS, uint64_t /* DTS */,
        unsigned PES_scrambling_control,
        const uint8_t *data, size_t size,
        int32_t payloadOffset, SyncEvent *event) {
#if 0
    ALOGI("payload streamType 0x%02x, PTS = 0x%016llx, dPTS = %lld",
          mStreamType,
          PTS,
          (int64_t)PTS - mPrevPTS);
    mPrevPTS = PTS;
#endif

    ALOGV("onPayloadData mStreamType=0x%02x size: %zu", mStreamType, size);

    int64_t timeUs = 0LL;  // no presentation timestamp available.
    if (PTS_DTS_flags == 2 || PTS_DTS_flags == 3) {
        timeUs = mProgram->convertPTSToTimestamp(PTS);
    }

    status_t err = mQueue->appendData(
            data, size, timeUs, payloadOffset, PES_scrambling_control);

    if (mEOSReached) {
        mQueue->signalEOS();
    }

    if (err != OK) {
        return;
    }

    sp<ABuffer> accessUnit;
    bool found = false;
    while ((accessUnit = mQueue->dequeueAccessUnit()) != NULL) {
        if (mSource == NULL) {
            sp<MetaData> meta = mQueue->getFormat();

            if (meta != NULL) {
                ALOGV("Stream PID 0x%08x of type 0x%02x now has data.",
                     mElementaryPID, mStreamType);

                const char *mime;
                if (meta->findCString(kKeyMIMEType, &mime)
                        && !strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_AVC)) {
                    int32_t sync = 0;
                    if (!accessUnit->meta()->findInt32("isSync", &sync) || !sync) {
                        continue;
                    }
                }
                mSource = new AnotherPacketSource(meta);
                if (mAudioPresentations.size() > 0) {
                    addAudioPresentations(accessUnit);
                }
                mSource->queueAccessUnit(accessUnit);
                ALOGV("onPayloadData: created AnotherPacketSource PID 0x%08x of type 0x%02x",
                        mElementaryPID, mStreamType);
            }
        } else if (mQueue->getFormat() != NULL) {
            // After a discontinuity we invalidate the queue's format
            // and won't enqueue any access units to the source until
            // the queue has reestablished the new format.

            if (mSource->getFormat() == NULL) {
                mSource->setFormat(mQueue->getFormat());
            }
            if (mAudioPresentations.size() > 0) {
                addAudioPresentations(accessUnit);
            }
            mSource->queueAccessUnit(accessUnit);
        }

        // Every access unit has a pesStartOffset queued in |mPesStartOffsets|.
        off64_t pesStartOffset = -1;
        if (!mPesStartOffsets.empty()) {
            pesStartOffset = *mPesStartOffsets.begin();
            mPesStartOffsets.erase(mPesStartOffsets.begin());
        }

        if (pesStartOffset >= 0 && (event != NULL) && !found && mQueue->getFormat() != NULL) {
            int32_t sync = 0;
            if (accessUnit->meta()->findInt32("isSync", &sync) && sync) {
                int64_t timeUs;
                if (accessUnit->meta()->findInt64("timeUs", &timeUs)) {
                    found = true;
                    event->init(pesStartOffset, mSource, timeUs, getSourceType());
                }
            }
        }
    }
}

ATSParser::SourceType ATSParser::Stream::getSourceType() {
    if (isVideo()) {
        return VIDEO;
    } else if (isAudio()) {
        return AUDIO;
    } else if (isMeta()) {
        return META;
    }
    return NUM_SOURCE_TYPES;
}

sp<AnotherPacketSource> ATSParser::Stream::getSource(SourceType type) {
    switch (type) {
        case VIDEO:
        {
            if (isVideo()) {
                return mSource;
            }
            break;
        }

        case AUDIO:
        {
            if (isAudio()) {
                return mSource;
            }
            break;
        }

        case META:
        {
            if (isMeta()) {
                return mSource;
            }
            break;
        }

        default:
            break;
    }

    return NULL;
}

void ATSParser::Stream::setCasInfo(
        int32_t systemId, const sp<IDescrambler> &descrambler,
        const std::vector<uint8_t> &sessionId) {
    if (mSource != NULL && mDescrambler == NULL && descrambler != NULL) {
        signalDiscontinuity(DISCONTINUITY_FORMAT_ONLY, NULL);
        mDescrambler = descrambler;
        if (mQueue->isScrambled()) {
            mQueue->setCasInfo(systemId, sessionId);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

ATSParser::ATSParser(uint32_t flags)
    : mFlags(flags),
      mAbsoluteTimeAnchorUs(-1LL),
      mTimeOffsetValid(false),
      mTimeOffsetUs(0LL),
      mLastRecoveredPTS(-1LL),
      mNumTSPacketsParsed(0),
      mNumPCRs(0) {
    mPSISections.add(0 /* PID */, new PSISection);
    mCasManager = new CasManager();
}

ATSParser::~ATSParser() {
}

status_t ATSParser::feedTSPacket(const void *data, size_t size,
        SyncEvent *event) {
    if (size != kTSPacketSize) {
        ALOGE("Wrong TS packet size");
        return BAD_VALUE;
    }

    ABitReader br((const uint8_t *)data, kTSPacketSize);
    return parseTS(&br, event);
}

status_t ATSParser::setMediaCas(const sp<ICas> &cas) {
    status_t err = mCasManager->setMediaCas(cas);
    if (err != OK) {
        return err;
    }
    for (size_t i = 0; i < mPrograms.size(); ++i) {
        mPrograms.editItemAt(i)->updateCasSessions();
    }
    return OK;
}

void ATSParser::signalDiscontinuity(
        DiscontinuityType type, const sp<AMessage> &extra) {
    int64_t mediaTimeUs;
    if ((type & DISCONTINUITY_TIME) && extra != NULL) {
        if (extra->findInt64(kATSParserKeyMediaTimeUs, &mediaTimeUs)) {
            mAbsoluteTimeAnchorUs = mediaTimeUs;
        }
        if ((mFlags & TS_TIMESTAMPS_ARE_ABSOLUTE)
                && extra->findInt64(
                    kATSParserKeyRecentMediaTimeUs, &mediaTimeUs)) {
            if (mAbsoluteTimeAnchorUs >= 0LL) {
                mediaTimeUs -= mAbsoluteTimeAnchorUs;
            }
            if (mTimeOffsetValid) {
                mediaTimeUs -= mTimeOffsetUs;
            }
            mLastRecoveredPTS = (mediaTimeUs * 9) / 100;
        }
    } else if (type == DISCONTINUITY_ABSOLUTE_TIME) {
        int64_t timeUs;
        if (!extra->findInt64("timeUs", &timeUs)) {
            ALOGE("timeUs not found");
            return;
        }

        if (!mPrograms.empty()) {
            ALOGE("mPrograms is not empty");
            return;
        }
        mAbsoluteTimeAnchorUs = timeUs;
        return;
    } else if (type == DISCONTINUITY_TIME_OFFSET) {
        int64_t offset;
        if (!extra->findInt64("offset", &offset)) {
            ALOGE("offset not found");
            return;
        }

        mTimeOffsetValid = true;
        mTimeOffsetUs = offset;
        return;
    }

    for (size_t i = 0; i < mPrograms.size(); ++i) {
        mPrograms.editItemAt(i)->signalDiscontinuity(type, extra);
    }
}

void ATSParser::signalEOS(status_t finalResult) {
    if (finalResult == (status_t) OK) {
        ALOGE("finalResult not OK");
        return;
    }

    for (size_t i = 0; i < mPrograms.size(); ++i) {
        mPrograms.editItemAt(i)->signalEOS(finalResult);
    }
}

void ATSParser::parseProgramAssociationTable(ABitReader *br) {
    unsigned table_id = br->getBits(8);
    ALOGV("  table_id = %u", table_id);
    if (table_id != 0x00u) {
        ALOGE("PAT data error!");
        return ;
    }
    unsigned section_syntax_indictor = br->getBits(1);
    ALOGV("  section_syntax_indictor = %u", section_syntax_indictor);

    br->skipBits(1);  // '0'
    MY_LOGV("  reserved = %u", br->getBits(2));

    unsigned section_length = br->getBits(12);
    ALOGV("  section_length = %u", section_length);

    MY_LOGV("  transport_stream_id = %u", br->getBits(16));
    MY_LOGV("  reserved = %u", br->getBits(2));
    MY_LOGV("  version_number = %u", br->getBits(5));
    MY_LOGV("  current_next_indicator = %u", br->getBits(1));
    MY_LOGV("  section_number = %u", br->getBits(8));
    MY_LOGV("  last_section_number = %u", br->getBits(8));

    size_t numProgramBytes = (section_length - 5 /* header */ - 4 /* crc */);

    for (size_t i = 0; i < numProgramBytes / 4; ++i) {
        unsigned program_number = br->getBits(16);
        ALOGV("    program_number = %u", program_number);

        MY_LOGV("    reserved = %u", br->getBits(3));

        if (program_number == 0) {
            MY_LOGV("    network_PID = 0x%04x", br->getBits(13));
        } else {
            unsigned programMapPID = br->getBits(13);

            ALOGV("    program_map_PID = 0x%04x", programMapPID);

            bool found = false;
            for (size_t index = 0; index < mPrograms.size(); ++index) {
                const sp<Program> &program = mPrograms.itemAt(index);

                if (program->number() == program_number) {
                    program->updateProgramMapPID(programMapPID);
                    found = true;
                    break;
                }
            }

            if (!found) {
                mPrograms.push(
                        new Program(this, program_number, programMapPID, mLastRecoveredPTS));
                if (mSampleAesKeyItem != NULL) {
                    mPrograms.top()->signalNewSampleAesKey(mSampleAesKeyItem);
                }
            }

            if (mPSISections.indexOfKey(programMapPID) < 0) {
                mPSISections.add(programMapPID, new PSISection);
            }
        }
    }

    MY_LOGV("  CRC = 0x%08x", br->getBits(32));
}

status_t ATSParser::parsePID(
        ABitReader *br, unsigned PID,
        unsigned continuity_counter,
        unsigned payload_unit_start_indicator,
        unsigned transport_scrambling_control,
        unsigned random_access_indicator,
        SyncEvent *event) {
    ssize_t sectionIndex = mPSISections.indexOfKey(PID);

    if (sectionIndex >= 0) {
        sp<PSISection> section = mPSISections.valueAt(sectionIndex);

        if (payload_unit_start_indicator) {
            if (!section->isEmpty()) {
                ALOGW("parsePID encounters payload_unit_start_indicator when section is not empty");
                section->clear();
            }

            unsigned skip = br->getBits(8);
            section->setSkipBytes(skip + 1);  // skip filler bytes + pointer field itself
            br->skipBits(skip * 8);
        }

        if (br->numBitsLeft() % 8 != 0) {
            return ERROR_MALFORMED;
        }
        status_t err = section->append(br->data(), br->numBitsLeft() / 8);

        if (err != OK) {
            return err;
        }

        if (!section->isComplete()) {
            return OK;
        }

        if (!section->isCRCOkay()) {
            return BAD_VALUE;
        }
        ABitReader sectionBits(section->data(), section->size());

        if (PID == 0) {
            parseProgramAssociationTable(&sectionBits);
        } else {
            bool handled = false;
            for (size_t i = 0; i < mPrograms.size(); ++i) {
                status_t err;
                if (!mPrograms.editItemAt(i)->parsePSISection(
                            PID, &sectionBits, &err)) {
                    continue;
                }

                if (err != OK) {
                    return err;
                }

                handled = true;
                break;
            }

            if (!handled) {
                mPSISections.removeItem(PID);
                section.clear();
            }
        }

        if (section != NULL) {
            section->clear();
        }

        return OK;
    }

    bool handled = false;
    for (size_t i = 0; i < mPrograms.size(); ++i) {
        status_t err;
        if (mPrograms.editItemAt(i)->parsePID(
                    PID, continuity_counter,
                    payload_unit_start_indicator,
                    transport_scrambling_control,
                    random_access_indicator,
                    br, &err, event)) {
            if (err != OK) {
                return err;
            }

            handled = true;
            break;
        }
    }

    if (!handled) {
        handled = mCasManager->parsePID(br, PID);
    }

    if (!handled) {
        ALOGV("PID 0x%04x not handled.", PID);
    }

    return OK;
}

status_t ATSParser::parseAdaptationField(
        ABitReader *br, unsigned PID, unsigned *random_access_indicator) {
    *random_access_indicator = 0;
    unsigned adaptation_field_length = br->getBits(8);

    if (adaptation_field_length > 0) {
        if (adaptation_field_length * 8 > br->numBitsLeft()) {
            ALOGV("Adaptation field should be included in a single TS packet.");
            return ERROR_MALFORMED;
        }

        unsigned discontinuity_indicator = br->getBits(1);

        if (discontinuity_indicator) {
            ALOGV("PID 0x%04x: discontinuity_indicator = 1 (!!!)", PID);
        }

        *random_access_indicator = br->getBits(1);
        if (*random_access_indicator) {
            ALOGV("PID 0x%04x: random_access_indicator = 1", PID);
        }

        unsigned elementary_stream_priority_indicator = br->getBits(1);
        if (elementary_stream_priority_indicator) {
            ALOGV("PID 0x%04x: elementary_stream_priority_indicator = 1", PID);
        }

        unsigned PCR_flag = br->getBits(1);

        size_t numBitsRead = 4;

        if (PCR_flag) {
            if (adaptation_field_length * 8 < 52) {
                return ERROR_MALFORMED;
            }
            br->skipBits(4);
            uint64_t PCR_base = br->getBits(32);
            PCR_base = (PCR_base << 1) | br->getBits(1);

            br->skipBits(6);
            unsigned PCR_ext = br->getBits(9);

            // The number of bytes from the start of the current
            // MPEG2 transport stream packet up and including
            // the final byte of this PCR_ext field.
            size_t byteOffsetFromStartOfTSPacket =
                (188 - br->numBitsLeft() / 8);

            uint64_t PCR = PCR_base * 300 + PCR_ext;

            ALOGV("PID 0x%04x: PCR = 0x%016" PRIx64 " (%.2f)",
                  PID, PCR, PCR / 27E6);

            // The number of bytes received by this parser up to and
            // including the final byte of this PCR_ext field.
            uint64_t byteOffsetFromStart =
                uint64_t(mNumTSPacketsParsed) * 188 + byteOffsetFromStartOfTSPacket;

            for (size_t i = 0; i < mPrograms.size(); ++i) {
                updatePCR(PID, PCR, byteOffsetFromStart);
            }

            numBitsRead += 52;
        }

        br->skipBits(adaptation_field_length * 8 - numBitsRead);
    }
    return OK;
}

status_t ATSParser::parseTS(ABitReader *br, SyncEvent *event) {
    ALOGV("---");

    unsigned sync_byte = br->getBits(8);
    if (sync_byte != 0x47u) {
        ALOGE("[error] parseTS: return error as sync_byte=0x%x", sync_byte);
        return BAD_VALUE;
    }

    if (br->getBits(1)) {  // transport_error_indicator
        // silently ignore.
        return OK;
    }

    unsigned payload_unit_start_indicator = br->getBits(1);
    ALOGV("payload_unit_start_indicator = %u", payload_unit_start_indicator);

    MY_LOGV("transport_priority = %u", br->getBits(1));

    unsigned PID = br->getBits(13);
    ALOGV("PID = 0x%04x", PID);

    unsigned transport_scrambling_control = br->getBits(2);
    ALOGV("transport_scrambling_control = %u", transport_scrambling_control);

    unsigned adaptation_field_control = br->getBits(2);
    ALOGV("adaptation_field_control = %u", adaptation_field_control);

    unsigned continuity_counter = br->getBits(4);
    ALOGV("PID = 0x%04x, continuity_counter = %u", PID, continuity_counter);

    // ALOGI("PID = 0x%04x, continuity_counter = %u", PID, continuity_counter);

    status_t err = OK;

    unsigned random_access_indicator = 0;
    if (adaptation_field_control == 2 || adaptation_field_control == 3) {
        err = parseAdaptationField(br, PID, &random_access_indicator);
    }
    if (err == OK) {
        if (adaptation_field_control == 1 || adaptation_field_control == 3) {
            err = parsePID(br, PID, continuity_counter,
                    payload_unit_start_indicator,
                    transport_scrambling_control,
                    random_access_indicator,
                    event);
        }
    }

    ++mNumTSPacketsParsed;

    return err;
}

sp<AnotherPacketSource> ATSParser::getSource(SourceType type) {
    sp<AnotherPacketSource> firstSourceFound;
    for (size_t i = 0; i < mPrograms.size(); ++i) {
        const sp<Program> &program = mPrograms.editItemAt(i);
        sp<AnotherPacketSource> source = program->getSource(type);
        if (source == NULL) {
            continue;
        }
        if (firstSourceFound == NULL) {
            firstSourceFound = source;
        }
        // Prefer programs with both audio/video
        switch (type) {
            case VIDEO: {
                if (program->hasSource(AUDIO)) {
                    return source;
                }
                break;
            }

            case AUDIO: {
                if (program->hasSource(VIDEO)) {
                    return source;
                }
                break;
            }

            default:
                return source;
        }
    }

    return firstSourceFound;
}

bool ATSParser::hasSource(SourceType type) const {
    for (size_t i = 0; i < mPrograms.size(); ++i) {
        const sp<Program> &program = mPrograms.itemAt(i);
        if (program->hasSource(type)) {
            return true;
        }
    }

    return false;
}

bool ATSParser::PTSTimeDeltaEstablished() {
    if (mPrograms.isEmpty()) {
        return false;
    }

    return mPrograms.editItemAt(0)->PTSTimeDeltaEstablished();
}

int64_t ATSParser::getFirstPTSTimeUs() {
    for (size_t i = 0; i < mPrograms.size(); ++i) {
        sp<ATSParser::Program> program = mPrograms.itemAt(i);
        if (program->PTSTimeDeltaEstablished()) {
            return (program->firstPTS() * 100) / 9;
        }
    }
    return -1;
}

__attribute__((no_sanitize("integer")))
void ATSParser::updatePCR(
        unsigned /* PID */, uint64_t PCR, uint64_t byteOffsetFromStart) {
    ALOGV("PCR 0x%016" PRIx64 " @ %" PRIx64, PCR, byteOffsetFromStart);

    if (mNumPCRs == 2) {
        mPCR[0] = mPCR[1];
        mPCRBytes[0] = mPCRBytes[1];
        mSystemTimeUs[0] = mSystemTimeUs[1];
        mNumPCRs = 1;
    }

    mPCR[mNumPCRs] = PCR;
    mPCRBytes[mNumPCRs] = byteOffsetFromStart;
    mSystemTimeUs[mNumPCRs] = ALooper::GetNowUs();

    ++mNumPCRs;

    if (mNumPCRs == 2) {
        /* Unsigned overflow here */
        double transportRate =
            (mPCRBytes[1] - mPCRBytes[0]) * 27E6 / (mPCR[1] - mPCR[0]);

        ALOGV("transportRate = %.2f bytes/sec", transportRate);
    }
}

////////////////////////////////////////////////////////////////////////////////


// CRC32 used for PSI section. The table was generated by following command:
// $ python pycrc.py --model crc-32-mpeg --algorithm table-driven --generate c
// Visit http://www.tty1.net/pycrc/index_en.html for more details.
uint32_t ATSParser::PSISection::CRC_TABLE[] = {
    0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
    0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
    0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
    0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
    0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
    0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
    0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
    0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
    0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
    0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
    0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
    0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
    0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
    0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
    0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
    0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
    0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
    0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
    0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
    0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
    0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
    0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
    0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
    0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
    0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
    0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
    0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
    0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
    0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
    0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
    0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
    0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
    0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
    0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
    0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
    0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
    0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
    0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
    0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
    0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
    0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
    0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
    0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
    0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
    0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
    0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
    0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
    0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
    0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
    0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
    0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
    0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
    0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
    0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
    0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
    0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
    0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
    0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
    0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
    0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
    0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
    0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
    0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
    0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
    };

ATSParser::PSISection::PSISection() :
    mSkipBytes(0) {
}

ATSParser::PSISection::~PSISection() {
}

status_t ATSParser::PSISection::append(const void *data, size_t size) {
    if (mBuffer == NULL || mBuffer->size() + size > mBuffer->capacity()) {
        size_t newCapacity =
            (mBuffer == NULL) ? size : mBuffer->capacity() + size;

        newCapacity = (newCapacity + 1023) & ~1023;

        sp<ABuffer> newBuffer = new ABuffer(newCapacity);

        if (mBuffer != NULL) {
            memcpy(newBuffer->data(), mBuffer->data(), mBuffer->size());
            newBuffer->setRange(0, mBuffer->size());
        } else {
            newBuffer->setRange(0, 0);
        }

        mBuffer = newBuffer;
    }

    memcpy(mBuffer->data() + mBuffer->size(), data, size);
    mBuffer->setRange(0, mBuffer->size() + size);

    return OK;
}

void ATSParser::PSISection::setSkipBytes(uint8_t skip) {
    mSkipBytes = skip;
}

void ATSParser::PSISection::clear() {
    if (mBuffer != NULL) {
        mBuffer->setRange(0, 0);
    }
    mSkipBytes = 0;
}

bool ATSParser::PSISection::isComplete() const {
    if (mBuffer == NULL || mBuffer->size() < 3) {
        return false;
    }

    unsigned sectionLength = U16_AT(mBuffer->data() + 1) & 0xfff;
    return mBuffer->size() >= sectionLength + 3;
}

bool ATSParser::PSISection::isEmpty() const {
    return mBuffer == NULL || mBuffer->size() == 0;
}

const uint8_t *ATSParser::PSISection::data() const {
    return mBuffer == NULL ? NULL : mBuffer->data();
}

size_t ATSParser::PSISection::size() const {
    return mBuffer == NULL ? 0 : mBuffer->size();
}

bool ATSParser::PSISection::isCRCOkay() const {
    if (!isComplete()) {
        return false;
    }
    uint8_t* data = mBuffer->data();

    // Return true if section_syntax_indicator says no section follows the field section_length.
    if ((data[1] & 0x80) == 0) {
        return true;
    }

    unsigned sectionLength = U16_AT(data + 1) & 0xfff;
    ALOGV("sectionLength %u, skip %u", sectionLength, mSkipBytes);


    if(sectionLength < mSkipBytes) {
        ALOGE("b/28333006");
        android_errorWriteLog(0x534e4554, "28333006");
        return false;
    }

    // Skip the preceding field present when payload start indicator is on.
    sectionLength -= mSkipBytes;

    uint32_t crc = 0xffffffff;
    for(unsigned i = 0; i < sectionLength + 4 /* crc */; i++) {
        uint8_t b = data[i];
        int index = ((crc >> 24) ^ (b & 0xff)) & 0xff;
        crc = CRC_TABLE[index] ^ (crc << 8);
    }
    ALOGV("crc: %08x\n", crc);
    return (crc == 0);
}

// SAMPLE_AES key handling
// TODO: Merge these to their respective class after Widevine-HLS
void ATSParser::signalNewSampleAesKey(const sp<AMessage> &keyItem) {
    ALOGD("signalNewSampleAesKey: %p", keyItem.get());

    mSampleAesKeyItem = keyItem;

    // a NULL key item will propagate to existing ElementaryStreamQueues
    for (size_t i = 0; i < mPrograms.size(); ++i) {
        mPrograms[i]->signalNewSampleAesKey(keyItem);
    }
}

void ATSParser::Program::signalNewSampleAesKey(const sp<AMessage> &keyItem) {
    ALOGD("Program::signalNewSampleAesKey: %p", keyItem.get());

    mSampleAesKeyItem = keyItem;

    // a NULL key item will propagate to existing ElementaryStreamQueues
    for (size_t i = 0; i < mStreams.size(); ++i) {
        mStreams[i]->signalNewSampleAesKey(keyItem);
    }
}

void ATSParser::Stream::signalNewSampleAesKey(const sp<AMessage> &keyItem) {
    ALOGD("Stream::signalNewSampleAesKey: 0x%04x size = %zu keyItem: %p",
          mElementaryPID, mBuffer->size(), keyItem.get());

    // a NULL key item will propagate to existing ElementaryStreamQueues
    mSampleAesKeyItem = keyItem;

    flush(NULL);
    mQueue->signalNewSampleAesKey(keyItem);
}

}  // namespace android
