/*
 * Copyright (C) 2014 The Android Open Source Project
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

// #define LOG_NDEBUG 0
#define LOG_TAG "WebmWriter"

#include "EbmlUtil.h"
#include "WebmWriter.h"

#include <media/stagefright/MetaData.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/foundation/OpusHeader.h>

#include <utils/Errors.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <inttypes.h>

using namespace webm;

namespace {
size_t XiphLaceCodeLen(size_t size) {
    return size / 0xff + 1;
}

size_t XiphLaceEnc(uint8_t *buf, size_t size) {
    size_t i;
    for (i = 0; size >= 0xff; ++i, size -= 0xff) {
        buf[i] = 0xff;
    }
    buf[i++] = size;
    return i;
}
}

namespace android {

static const int64_t kMinStreamableFileSizeInBytes = 5 * 1024 * 1024;

WebmWriter::WebmWriter(int fd)
    : mFd(dup(fd)),
      mInitCheck(mFd < 0 ? NO_INIT : OK),
      mTimeCodeScale(1000000),
      mStartTimestampUs(0),
      mStartTimeOffsetMs(0),
      mSegmentOffset(0),
      mSegmentDataStart(0),
      mInfoOffset(0),
      mInfoSize(0),
      mTracksOffset(0),
      mCuesOffset(0),
      mPaused(false),
      mStarted(false),
      mIsFileSizeLimitExplicitlyRequested(false),
      mIsRealTimeRecording(false),
      mStreamableFile(true),
      mEstimatedCuesSize(0) {
    mStreams[kAudioIndex] = WebmStream(kAudioType, "Audio", &WebmWriter::audioTrack);
    mStreams[kVideoIndex] = WebmStream(kVideoType, "Video", &WebmWriter::videoTrack);
    mSinkThread = new WebmFrameSinkThread(
            mFd,
            mSegmentDataStart,
            mStreams[kVideoIndex].mSink,
            mStreams[kAudioIndex].mSink,
            mCuePoints);
}

// static
sp<WebmElement> WebmWriter::videoTrack(const sp<MetaData>& md) {
    int32_t width, height;
    const char *mimeType;
    if (!md->findInt32(kKeyWidth, &width)
            || !md->findInt32(kKeyHeight, &height)
            || !md->findCString(kKeyMIMEType, &mimeType)) {
        ALOGE("Missing format keys for video track");
        md->dumpToLog();
        return NULL;
    }
    const char *codec;
    if (!strncasecmp(
            mimeType,
            MEDIA_MIMETYPE_VIDEO_VP8,
            strlen(MEDIA_MIMETYPE_VIDEO_VP8))) {
        codec = "V_VP8";
    } else if (!strncasecmp(
            mimeType,
            MEDIA_MIMETYPE_VIDEO_VP9,
            strlen(MEDIA_MIMETYPE_VIDEO_VP9))) {
        codec = "V_VP9";
    } else {
        ALOGE("Unsupported codec: %s", mimeType);
        return NULL;
    }
    return WebmElement::VideoTrackEntry(codec, width, height, md);
}

// static
sp<WebmElement> WebmWriter::audioTrack(const sp<MetaData>& md) {
    int32_t nChannels, samplerate;
    const char* mimeType;

    if (!md->findInt32(kKeyChannelCount, &nChannels)
        || !md->findInt32(kKeySampleRate, &samplerate)
        || !md->findCString(kKeyMIMEType, &mimeType)) {
        ALOGE("Missing format keys for audio track");
        md->dumpToLog();
        return NULL;
    }

    int32_t bitsPerSample = 0;
    if (!md->findInt32(kKeyBitsPerSample, &bitsPerSample)) {
        ALOGV("kKeyBitsPerSample not available");
    }

    if (!strncasecmp(mimeType, MEDIA_MIMETYPE_AUDIO_OPUS, strlen(MEDIA_MIMETYPE_AUDIO_OPUS))) {
        // Opus in WebM is a well-known, yet under-documented, format. The codec private data
        // of the track is an Opus Ogg header (https://tools.ietf.org/html/rfc7845#section-5.1)
        // The name of the track isn't standardized, its value should be "A_OPUS".
        OpusHeader header;
        header.channels = nChannels;
        header.num_streams = nChannels;
        header.num_coupled = 0;
        // - Channel mapping family (8 bits unsigned)
        //  --  0 = one stream: mono or L,R stereo
        //  --  1 = channels in vorbis spec order: mono or L,R stereo or ... or FL,C,FR,RL,RR,LFE, ...
        //  --  2..254 = reserved (treat as 255)
        //  --  255 = no defined channel meaning
        //
        //  our implementation encodes:  0, 1, or 255
        header.channel_mapping = ((nChannels > 8) ? 255 : (nChannels > 2));
        header.gain_db = 0;
        header.skip_samples = 0;

        // headers are 21-bytes + something driven by channel count
        // expect numbers in the low 30's here. WriteOpusHeader() will tell us
        // if things are bad.
        unsigned char header_data[100];
        int headerSize = WriteOpusHeader(header, samplerate, (uint8_t*)header_data,
                                            sizeof(header_data));

        if (headerSize < 0) {
            // didn't fill out that header for some reason
            ALOGE("failed to generate OPUS header");
            return NULL;
        }

        size_t codecPrivateSize = 0;
        codecPrivateSize += headerSize;

        off_t off = 0;
        sp<ABuffer> codecPrivateBuf = new ABuffer(codecPrivateSize);
        uint8_t* codecPrivateData = codecPrivateBuf->data();

        memcpy(codecPrivateData + off, (uint8_t*)header_data, headerSize);
        sp<WebmElement> entry = WebmElement::AudioTrackEntry("A_OPUS", nChannels, samplerate,
                                                             codecPrivateBuf, bitsPerSample);
        return entry;
    } else if (!strncasecmp(mimeType,
                            MEDIA_MIMETYPE_AUDIO_VORBIS,
                            strlen(MEDIA_MIMETYPE_AUDIO_VORBIS))) {
        uint32_t type;
        const void *headerData1;
        const char headerData2[] = { 3, 'v', 'o', 'r', 'b', 'i', 's', 7, 0, 0, 0,
                'a', 'n', 'd', 'r', 'o', 'i', 'd', 0, 0, 0, 0, 1 };
        const void *headerData3;
        size_t headerSize1, headerSize2 = sizeof(headerData2), headerSize3;

        if (!md->findData(kKeyOpaqueCSD0, &type, &headerData1, &headerSize1)
            || !md->findData(kKeyOpaqueCSD1, &type, &headerData3, &headerSize3)) {
            ALOGE("Missing header format keys for vorbis track");
            md->dumpToLog();
            return NULL;
        }

        size_t codecPrivateSize = 1;
        codecPrivateSize += XiphLaceCodeLen(headerSize1);
        codecPrivateSize += XiphLaceCodeLen(headerSize2);
        codecPrivateSize += headerSize1 + headerSize2 + headerSize3;

        off_t off = 0;
        sp<ABuffer> codecPrivateBuf = new ABuffer(codecPrivateSize);
        uint8_t *codecPrivateData = codecPrivateBuf->data();
        codecPrivateData[off++] = 2;

        off += XiphLaceEnc(codecPrivateData + off, headerSize1);
        off += XiphLaceEnc(codecPrivateData + off, headerSize2);

        memcpy(codecPrivateData + off, headerData1, headerSize1);
        off += headerSize1;
        memcpy(codecPrivateData + off, headerData2, headerSize2);
        off += headerSize2;
        memcpy(codecPrivateData + off, headerData3, headerSize3);

        sp<WebmElement> entry = WebmElement::AudioTrackEntry("A_VORBIS", nChannels, samplerate,
                                                             codecPrivateBuf, bitsPerSample);
        return entry;
    } else {
        ALOGE("Track (%s) is not a supported audio format", mimeType);
        return NULL;
    }
}

size_t WebmWriter::numTracks() {
    Mutex::Autolock autolock(mLock);

    size_t numTracks = 0;
    for (size_t i = 0; i < kMaxStreams; ++i) {
        if (mStreams[i].mTrackEntry != NULL) {
            numTracks++;
        }
    }

    return numTracks;
}

uint64_t WebmWriter::estimateCuesSize(int32_t bitRate) {
    // This implementation is based on estimateMoovBoxSize in MPEG4Writer.
    //
    // Statistical analysis shows that metadata usually accounts
    // for a small portion of the total file size, usually < 0.6%.

    // The default MIN_MOOV_BOX_SIZE is set to 0.6% x 1MB / 2,
    // where 1MB is the common file size limit for MMS application.
    // The default MAX _MOOV_BOX_SIZE value is based on about 3
    // minute video recording with a bit rate about 3 Mbps, because
    // statistics also show that most of the video captured are going
    // to be less than 3 minutes.

    // If the estimation is wrong, we will pay the price of wasting
    // some reserved space. This should not happen so often statistically.
    static const int32_t factor = 2;
    static const int64_t MIN_CUES_SIZE = 3 * 1024;  // 3 KB
    static const int64_t MAX_CUES_SIZE = (180 * 3000000 * 6LL / 8000);
    int64_t size = MIN_CUES_SIZE;

    // Max file size limit is set
    if (mMaxFileSizeLimitBytes != 0 && mIsFileSizeLimitExplicitlyRequested) {
        size = mMaxFileSizeLimitBytes * 6 / 1000;
    }

    // Max file duration limit is set
    if (mMaxFileDurationLimitUs != 0) {
        if (bitRate > 0) {
            int64_t size2 = ((mMaxFileDurationLimitUs * bitRate * 6) / 1000 / 8000000);
            if (mMaxFileSizeLimitBytes != 0 && mIsFileSizeLimitExplicitlyRequested) {
                // When both file size and duration limits are set,
                // we use the smaller limit of the two.
                if (size > size2) {
                    size = size2;
                }
            } else {
                // Only max file duration limit is set
                size = size2;
            }
        }
    }

    if (size < MIN_CUES_SIZE) {
        size = MIN_CUES_SIZE;
    }

    // Any long duration recording will be probably end up with
    // non-streamable webm file.
    if (size > MAX_CUES_SIZE) {
        size = MAX_CUES_SIZE;
    }

    ALOGV("limits: %" PRId64 "/%" PRId64 " bytes/us,"
            " bit rate: %d bps and the estimated cues size %" PRId64 " bytes",
            mMaxFileSizeLimitBytes, mMaxFileDurationLimitUs, bitRate, size);
    return factor * size;
}

void WebmWriter::initStream(size_t idx) {
    if (mStreams[idx].mThread != NULL) {
        return;
    }
    if (mStreams[idx].mSource == NULL) {
        ALOGV("adding dummy source ... ");
        mStreams[idx].mThread = new WebmFrameEmptySourceThread(
                mStreams[idx].mType, mStreams[idx].mSink);
    } else {
        ALOGV("adding source %p", mStreams[idx].mSource.get());
        mStreams[idx].mThread = new WebmFrameMediaSourceThread(
                mStreams[idx].mSource,
                mStreams[idx].mType,
                mStreams[idx].mSink,
                mTimeCodeScale,
                mStartTimestampUs,
                mStartTimeOffsetMs,
                numTracks(),
                mIsRealTimeRecording);
    }
}

void WebmWriter::release() {
    close(mFd);
    mFd = -1;
    mInitCheck = NO_INIT;
    mStarted = false;
    for (size_t ix = 0; ix < kMaxStreams; ++ix) {
        mStreams[ix].mTrackEntry.clear();
        mStreams[ix].mSource.clear();
    }
    mStreamsInOrder.clear();
}

status_t WebmWriter::reset() {
    if (mInitCheck != OK) {
        return OK;
    } else {
        if (!mStarted) {
            release();
            return OK;
        }
    }

    status_t err = OK;
    int64_t maxDurationUs = 0;
    int64_t minDurationUs = 0x7fffffffffffffffLL;
    for (int i = 0; i < kMaxStreams; ++i) {
        if (mStreams[i].mThread == NULL) {
            continue;
        }

        status_t status = mStreams[i].mThread->stop();
        if (err == OK && status != OK) {
            err = status;
        }

        int64_t durationUs = mStreams[i].mThread->getDurationUs();
        if (durationUs > maxDurationUs) {
            maxDurationUs = durationUs;
        }
        if (durationUs < minDurationUs) {
            minDurationUs = durationUs;
        }

        mStreams[i].mThread.clear();
    }

    if (numTracks() > 1) {
        ALOGD("Duration from tracks range is [%" PRId64 ", %" PRId64 "] us", minDurationUs, maxDurationUs);
    }

    mSinkThread->stop();

    // Do not write out movie header on error.
    if (err != OK) {
        release();
        return err;
    }

    sp<WebmElement> cues = new WebmMaster(kMkvCues, mCuePoints);
    uint64_t cuesSize = cues->totalSize();
    // TRICKY Even when the cues do fit in the space we reserved, if they do not fit
    // perfectly, we still need to check if there is enough "extra space" to write an
    // EBML void element.
    if (cuesSize != mEstimatedCuesSize && cuesSize > mEstimatedCuesSize - kMinEbmlVoidSize) {
        mCuesOffset = ::lseek(mFd, 0, SEEK_CUR);
        cues->write(mFd, cuesSize);
    } else {
        uint64_t spaceSize;
        ::lseek(mFd, mCuesOffset, SEEK_SET);
        cues->write(mFd, cuesSize);
        sp<WebmElement> space = new EbmlVoid(mEstimatedCuesSize - cuesSize);
        space->write(mFd, spaceSize);
    }

    mCuePoints.clear();
    mStreams[kVideoIndex].mSink.clear();
    mStreams[kAudioIndex].mSink.clear();

    uint8_t bary[sizeof(uint64_t)];
    uint64_t totalSize = ::lseek(mFd, 0, SEEK_END);
    uint64_t segmentSize = totalSize - mSegmentDataStart;
    ::lseek(mFd, mSegmentOffset + sizeOf(kMkvSegment), SEEK_SET);
    uint64_t segmentSizeCoded = encodeUnsigned(segmentSize, sizeOf(kMkvUnknownLength));
    serializeCodedUnsigned(segmentSizeCoded, bary);
    ::write(mFd, bary, sizeOf(kMkvUnknownLength));

    uint64_t durationOffset = mInfoOffset + sizeOf(kMkvInfo) + sizeOf(mInfoSize)
        + sizeOf(kMkvSegmentDuration) + sizeOf(sizeof(double));
    sp<WebmElement> duration = new WebmFloat(
            kMkvSegmentDuration,
            (double) (maxDurationUs * 1000 / mTimeCodeScale));
    duration->serializePayload(bary);
    ::lseek(mFd, durationOffset, SEEK_SET);
    ::write(mFd, bary, sizeof(double));

    List<sp<WebmElement> > seekEntries;
    seekEntries.push_back(WebmElement::SeekEntry(kMkvInfo, mInfoOffset - mSegmentDataStart));
    seekEntries.push_back(WebmElement::SeekEntry(kMkvTracks, mTracksOffset - mSegmentDataStart));
    seekEntries.push_back(WebmElement::SeekEntry(kMkvCues, mCuesOffset - mSegmentDataStart));
    sp<WebmElement> seekHead = new WebmMaster(kMkvSeekHead, seekEntries);

    uint64_t metaSeekSize;
    ::lseek(mFd, mSegmentDataStart, SEEK_SET);
    seekHead->write(mFd, metaSeekSize);

    uint64_t spaceSize;
    sp<WebmElement> space = new EbmlVoid(kMaxMetaSeekSize - metaSeekSize);
    space->write(mFd, spaceSize);

    release();
    return err;
}

status_t WebmWriter::addSource(const sp<MediaSource> &source) {
    Mutex::Autolock l(mLock);
    if (mStarted) {
        ALOGE("Attempt to add source AFTER recording is started");
        return UNKNOWN_ERROR;
    }

    // At most 2 tracks can be supported.
    if (mStreams[kVideoIndex].mTrackEntry != NULL
            && mStreams[kAudioIndex].mTrackEntry != NULL) {
        ALOGE("Too many tracks (2) to add");
        return ERROR_UNSUPPORTED;
    }

    CHECK(source != NULL);

    // A track of type other than video or audio is not supported.
    const char *mime;
    source->getFormat()->findCString(kKeyMIMEType, &mime);
    const char *vp8 = MEDIA_MIMETYPE_VIDEO_VP8;
    const char *vp9 = MEDIA_MIMETYPE_VIDEO_VP9;
    const char *vorbis = MEDIA_MIMETYPE_AUDIO_VORBIS;
    const char* opus = MEDIA_MIMETYPE_AUDIO_OPUS;

    size_t streamIndex;
    if (!strncasecmp(mime, vp8, strlen(vp8)) ||
        !strncasecmp(mime, vp9, strlen(vp9))) {
        streamIndex = kVideoIndex;
    } else if (!strncasecmp(mime, vorbis, strlen(vorbis)) ||
               !strncasecmp(mime, opus, strlen(opus))) {
        streamIndex = kAudioIndex;
    } else {
        ALOGE("Track (%s) other than %s, %s, %s, or %s is not supported",
              mime, vp8, vp9, vorbis, opus);
        return ERROR_UNSUPPORTED;
    }

    // No more than one video or one audio track is supported.
    if (mStreams[streamIndex].mTrackEntry != NULL) {
        ALOGE("%s track already exists", mStreams[streamIndex].mName);
        return ERROR_UNSUPPORTED;
    }

    // This is the first track of either audio or video.
    // Go ahead to add the track.
    mStreams[streamIndex].mSource = source;
    mStreams[streamIndex].mTrackEntry = mStreams[streamIndex].mMakeTrack(source->getFormat());
    if (mStreams[streamIndex].mTrackEntry == NULL) {
        mStreams[streamIndex].mSource.clear();
        return BAD_VALUE;
    }
    mStreamsInOrder.push_back(mStreams[streamIndex].mTrackEntry);

    return OK;
}

status_t WebmWriter::start(MetaData *params) {
    if (mInitCheck != OK) {
        return UNKNOWN_ERROR;
    }

    if (mStreams[kVideoIndex].mTrackEntry == NULL
            && mStreams[kAudioIndex].mTrackEntry == NULL) {
        ALOGE("No source added");
        return INVALID_OPERATION;
    }

    if (mMaxFileSizeLimitBytes != 0) {
        mIsFileSizeLimitExplicitlyRequested = true;
    }

    if (params) {
        int32_t isRealTimeRecording;
        params->findInt32(kKeyRealTimeRecording, &isRealTimeRecording);
        mIsRealTimeRecording = isRealTimeRecording;
    }

    if (mStarted) {
        if (mPaused) {
            mPaused = false;
            mStreams[kAudioIndex].mThread->resume();
            mStreams[kVideoIndex].mThread->resume();
        }
        return OK;
    }

    if (params) {
        int32_t tcsl;
        if (params->findInt32(kKeyTimeScale, &tcsl)) {
            mTimeCodeScale = tcsl;
        }
    }
    if (mTimeCodeScale == 0) {
        ALOGE("movie time scale is 0");
        return BAD_VALUE;
    }
    ALOGV("movie time scale: %" PRIu64, mTimeCodeScale);

    /*
     * When the requested file size limit is small, the priority
     * is to meet the file size limit requirement, rather than
     * to make the file streamable. mStreamableFile does not tell
     * whether the actual recorded file is streamable or not.
     */
    mStreamableFile = (!mMaxFileSizeLimitBytes)
        || (mMaxFileSizeLimitBytes >= kMinStreamableFileSizeInBytes);

    /*
     * Write various metadata.
     */
    sp<WebmElement> ebml, segment, info, seekHead, tracks, cues;
    ebml = WebmElement::EbmlHeader();
    segment = new WebmMaster(kMkvSegment);
    seekHead = new EbmlVoid(kMaxMetaSeekSize);
    info = WebmElement::SegmentInfo(mTimeCodeScale, 0);

    List<sp<WebmElement> > children;
    for (size_t i = 0; i < mStreamsInOrder.size(); ++i) {
        children.push_back(mStreamsInOrder[i]);
    }
    tracks = new WebmMaster(kMkvTracks, children);

    if (!mStreamableFile) {
        cues = NULL;
    } else {
        int32_t bitRate = -1;
        if (params) {
            params->findInt32(kKeyBitRate, &bitRate);
        }
        mEstimatedCuesSize = estimateCuesSize(bitRate);
        CHECK_GE(mEstimatedCuesSize, 8u);
        cues = new EbmlVoid(mEstimatedCuesSize);
    }

    sp<WebmElement> elems[] = { ebml, segment, seekHead, info, tracks, cues };
    static const size_t nElems = sizeof(elems) / sizeof(elems[0]);
    uint64_t offsets[nElems];
    uint64_t sizes[nElems];
    for (uint32_t i = 0; i < nElems; i++) {
        WebmElement *e = elems[i].get();
        if (!e) {
            continue;
        }

        uint64_t size;
        offsets[i] = ::lseek(mFd, 0, SEEK_CUR);
        sizes[i] = e->mSize;
        e->write(mFd, size);
    }

    mSegmentOffset = offsets[1];
    mSegmentDataStart = offsets[2];
    mInfoOffset = offsets[3];
    mInfoSize = sizes[3];
    mTracksOffset = offsets[4];
    mCuesOffset = offsets[5];

    // start threads
    if (params) {
        params->findInt64(kKeyTime, &mStartTimestampUs);
    }

    initStream(kAudioIndex);
    initStream(kVideoIndex);

    mStreams[kAudioIndex].mThread->start();
    mStreams[kVideoIndex].mThread->start();
    mSinkThread->start();

    mStarted = true;
    return OK;
}

status_t WebmWriter::pause() {
    if (mInitCheck != OK) {
        return OK;
    }
    mPaused = true;
    status_t err = OK;
    for (int i = 0; i < kMaxStreams; ++i) {
        if (mStreams[i].mThread == NULL) {
            continue;
        }
        status_t status = mStreams[i].mThread->pause();
        if (status != OK) {
            err = status;
        }
    }
    return err;
}

status_t WebmWriter::stop() {
    return reset();
}

bool WebmWriter::reachedEOS() {
    return !mSinkThread->running();
}
} /* namespace android */
