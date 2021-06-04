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

#include "sllog.h"
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MetaDataUtils.h>
#include <utils/Log.h>

#include "android/include/AacAdtsExtractor.h"


namespace android {

#define ADTS_HEADER_LENGTH 7
// ADTS header size is 7, but frame size information ends on byte 6 (when counting from byte 1)
#define ADTS_HEADER_SIZE_UP_TO_FRAMESIZE 6

////////////////////////////////////////////////////////////////////////////////

// Returns the sample rate based on the sampling frequency index
static uint32_t get_sample_rate(const uint8_t sf_index)
{
    static const uint32_t sample_rates[] =
    {
        96000, 88200, 64000, 48000, 44100, 32000,
        24000, 22050, 16000, 12000, 11025, 8000
    };

    if (sf_index < sizeof(sample_rates) / sizeof(sample_rates[0])) {
        return sample_rates[sf_index];
    }

    return 0;
}

static size_t getFrameSize(const sp<DataSource> &source, off64_t offset) {
    size_t frameSize = 0;

    uint8_t syncHeader[ADTS_HEADER_SIZE_UP_TO_FRAMESIZE];
    const uint8_t *syncword = syncHeader;
    const uint8_t *header = syncHeader + 3;

    ssize_t readSize = source->readAt(offset, &syncHeader, ADTS_HEADER_SIZE_UP_TO_FRAMESIZE);
    if (readSize == 0) {
        // EOS is normal, not an error
        SL_LOGV("AacAdtsExtractor::getFrameSize EOS");
        return 0;
    }
    if (readSize != ADTS_HEADER_SIZE_UP_TO_FRAMESIZE) {
        SL_LOGE("AacAdtsExtractor:: getFrameSize() returns %d (syncword and header read error)",
                (int) readSize);
        return 0;
    }

    if ((syncword[0] != 0xff) || ((syncword[1] & 0xf6) != 0xf0)) {
        SL_LOGE("AacAdtsExtractor:: getFrameSize() returns 0 (syncword pb)");
        return 0;
    }

    const uint8_t protectionAbsent = syncword[1] & 0x1;

    frameSize = (header[0] & 0x3) << 11 | header[1] << 3 | header[2] >> 5;
    // the frame size read already contains the size of the ADTS header, so no need to add it here

    // protectionAbsent is 0 if there is CRC
    static const size_t kAdtsHeaderLengthNoCrc = 7;
    static const size_t kAdtsHeaderLengthWithCrc = 9;
    size_t headSize = protectionAbsent ? kAdtsHeaderLengthNoCrc : kAdtsHeaderLengthWithCrc;
    if (headSize > frameSize) {
        SL_LOGE("AacAdtsExtractor:: getFrameSize() returns 0 (frameSize %zu < headSize %zu)",
                frameSize, headSize);
        return 0;
    }

    //SL_LOGV("AacAdtsExtractor:: getFrameSize() returns %u", frameSize);

    return frameSize;
}


AacAdtsExtractor::AacAdtsExtractor(const sp<DataSource> &source)
    : mDataSource(source),
      mMeta(new MetaData),
      mInitCheck(NO_INIT),
      mFrameDurationUs(0) {

    // difference with framework's AAC Extractor: we have already validated the data
    // upon enqueueing, so no need to sniff the data:
    //    String8 mimeType;
    //    float confidence;
    //    if (!SniffAAC(mDataSource, &mimeType, &confidence, NULL)) {
    //        return;
    //    }

    uint8_t profile, sf_index, channel, header[2];
    ssize_t readSize = mDataSource->readAt(2, &header, 2);
    if (readSize != 2) {
        SL_LOGE("Unable to determine sample rate");
        return;
    }

    profile = (header[0] >> 6) & 0x3;
    sf_index = (header[0] >> 2) & 0xf;
    uint32_t sr = get_sample_rate(sf_index);

    if (sr == 0) {
        SL_LOGE("Invalid sample rate");
        return;
    }
    channel = (header[0] & 0x1) << 2 | (header[1] >> 6);

    SL_LOGV("AacAdtsExtractor has found sr=%d channel=%d", sr, channel);

    // Never fails
    MakeAACCodecSpecificData(*mMeta, profile, sf_index, channel);

    // Round up and get the duration of each frame
    mFrameDurationUs = (1024 * 1000000LL + (sr - 1)) / sr;

    off64_t streamSize;
    if (mDataSource->getSize(&streamSize) == OK) {
        off64_t offset = 0, numFrames = 0;
        while (offset < streamSize) {
            size_t frameSize;
            if ((frameSize = getFrameSize(mDataSource, offset)) == 0) {
                // Usually frameSize == 0 due to EOS is benign (and getFrameSize() doesn't SL_LOGE),
                // but in this case we were told the total size of the data source and so an EOS
                // should not happen.
                SL_LOGE("AacAdtsExtractor() failed querying framesize at offset=%lld",
                        (long long) offset);
                return;
            }

            offset += frameSize;
            if (offset > streamSize) {
                SL_LOGE("AacAdtsExtractor() frame of size %zu at offset=%lld is beyond EOF %lld",
                        frameSize, (long long) offset, (long long) streamSize);
                return;
            }
            numFrames ++;
        }

        // Compute total duration
        int64_t duration = numFrames * mFrameDurationUs;
        mMeta->setInt64(kKeyDuration, duration);
    }

    // Any earlier "return" would leave mInitCheck as NO_INIT, causing later methods to fail quickly
    mInitCheck = OK;

}


AacAdtsExtractor::~AacAdtsExtractor() {
}

sp<MediaSource> AacAdtsExtractor::getTrack(size_t index) {
    if (mInitCheck != OK || index != 0) {
        return NULL;
    }

    return new AacAdtsSource(mDataSource, mMeta, mFrameDurationUs);
}


////////////////////////////////////////////////////////////////////////////////

// 8192 = 2^13, 13bit AAC frame size (in bytes)
const size_t AacAdtsSource::kMaxFrameSize = 8192;

AacAdtsSource::AacAdtsSource(
        const sp<DataSource> &source, const sp<MetaData> &meta,
        int64_t frame_duration_us)
    : mDataSource(source),
      mMeta(meta),
      mOffset(0),
      mCurrentTimeUs(0),
      mStarted(false),
      mGroup(NULL),
      mFrameDurationUs(frame_duration_us) {
}


AacAdtsSource::~AacAdtsSource() {
    if (mStarted) {
        stop();
    }
}


status_t AacAdtsSource::start(MetaData *params) {
    CHECK(!mStarted);

    mOffset = 0;
    mCurrentTimeUs = 0;
    mGroup = new MediaBufferGroup;
    mGroup->add_buffer(new MediaBuffer(kMaxFrameSize));
    mStarted = true;

    return OK;
}


status_t AacAdtsSource::stop() {
    CHECK(mStarted);

    delete mGroup;
    mGroup = NULL;

    mStarted = false;
    return OK;
}


sp<MetaData> AacAdtsSource::getFormat() {
    return mMeta;
}


status_t AacAdtsSource::read(
        MediaBufferBase **out, const ReadOptions *options) {
    *out = NULL;

    int64_t seekTimeUs;
    ReadOptions::SeekMode mode;
    if (options && options->getSeekTo(&seekTimeUs, &mode)) {
        // difference with framework's AAC Extractor: no seeking
        SL_LOGE("Can't seek in AAC ADTS buffer queue");
    }

    size_t frameSize, frameSizeWithoutHeader;
    SL_LOGV("AacAdtsSource::read() offset=%lld", mOffset);
    if ((frameSize = getFrameSize(mDataSource, mOffset)) == 0) {
        // EOS is normal, not an error
        SL_LOGV("AacAdtsSource::read() returns EOS");
        return ERROR_END_OF_STREAM;
    }

    MediaBufferBase *buffer;
    status_t err = mGroup->acquire_buffer(&buffer);
    if (err != OK) {
        return err;
    }

    frameSizeWithoutHeader = frameSize - ADTS_HEADER_LENGTH;
    ssize_t readSize = mDataSource->readAt(mOffset + ADTS_HEADER_LENGTH, buffer->data(),
            frameSizeWithoutHeader);
    //SL_LOGV("AacAdtsSource::read() readAt returned %u bytes", readSize);
    if (readSize != (ssize_t)frameSizeWithoutHeader) {
        SL_LOGW("AacAdtsSource::read() readSize != frameSizeWithoutHeader");
        buffer->release();
        buffer = NULL;
        return ERROR_IO;
    }

    buffer->set_range(0, frameSizeWithoutHeader);
    buffer->meta_data().setInt64(kKeyTime, mCurrentTimeUs);
    buffer->meta_data().setInt32(kKeyIsSyncFrame, 1);

    mOffset += frameSize;
    mCurrentTimeUs += mFrameDurationUs;

    *out = buffer;
    return OK;
}

}  // namespace android
