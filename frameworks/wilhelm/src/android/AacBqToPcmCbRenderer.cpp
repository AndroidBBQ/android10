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

//#define USE_LOG SLAndroidLogLevel_Debug

#include "sles_allinclusive.h"
#include "android/include/AacBqToPcmCbRenderer.h"
#include "android/channels.h"
#include <media/stagefright/SimpleDecodingSource.h>

namespace android {

// ADTS header size is 7, but frame size information ends on byte 6 (when counting from byte 1)
#define ADTS_HEADER_SIZE_UP_TO_FRAMESIZE 6

/**
 * Returns the size of an AAC ADTS frame.
 * Note that if the returned value + offset > size, it means that a partial frame starts at that
 *   offset, but this function will still return the size of the full frame.
 * @param data pointer to the compressed audio data
 * @param offset offset in bytes relative to data of where the frame is supposed to start
 * @param size the size in bytes of the data block starting at data
 * @return the size in bytes of the AAC ADTS frame starting at the given offset of the given
 *    memory address, 0 if the frame couldn't be parsed.
 */
static size_t getAdtsFrameSize(const uint8_t *data, off64_t offset, size_t size) {
    size_t frameSize = 0;

    if (!(offset + ADTS_HEADER_SIZE_UP_TO_FRAMESIZE < (off64_t) size)) {
        SL_LOGE("AacBqToPcmCbRenderer::getAdtsFrameSize() returns 0 (can't read syncword or header)"
                );
        return 0;
    }

    const uint8_t *syncword = data + offset;
    if ((syncword[0] != 0xff) || ((syncword[1] & 0xf6) != 0xf0)) {
        SL_LOGE("AacBqToPcmCbRenderer::getAdtsFrameSize() returns 0 (wrong syncword)");
        return 0;
    }

    const uint8_t protectionAbsent = data[offset+1] & 0x1;

    const uint8_t* header = data + offset + 3;
    frameSize = (header[0] & 0x3) << 11 | header[1] << 3 | header[2] >> 5;
    // the frame size read already contains the size of the header, so no need to add it here

    // protectionAbsent is 0 if there is CRC
    static const size_t kAdtsHeaderLengthNoCrc = 7;
    static const size_t kAdtsHeaderLengthWithCrc = 9;
    size_t headSize = protectionAbsent ? kAdtsHeaderLengthNoCrc : kAdtsHeaderLengthWithCrc;
    if (headSize > frameSize) {
        SL_LOGE("AacBqToPcmCbRenderer::getAdtsFrameSize() returns 0 (frameSize %zu < headSize %zu)",
                frameSize, headSize);
        return 0;
    }

    SL_LOGV("AacBqToPcmCbRenderer::getAdtsFrameSize() returns %u", frameSize);

    return frameSize;
}

/**
 * Returns whether a block of memory starts and ends on AAC ADTS frame boundaries
 * @param data pointer to the compressed audio data
 * @param size the size in bytes of the data block to validate
 * @return SL_RESULT_SUCCESS if there is AAC ADTS data, and it starts and ends on frame boundaries,
 *    or an appropriate error code otherwise:
 *      SL_RESULT_PARAMETER_INVALID if not possible to attempt validation of even one frame
 *      SL_RESULT_CONTENT_CORRUPTED if the frame contents are otherwise invalid
 */
SLresult AacBqToPcmCbRenderer::validateBufferStartEndOnFrameBoundaries(void* data, size_t size)
{
    off64_t offset = 0;
    size_t frameSize = 0;

    if ((NULL == data) || (size == 0)) {
        SL_LOGE("No ADTS to validate");
        return SL_RESULT_PARAMETER_INVALID;
    }

    while (offset < (off64_t) size) {
        if ((frameSize = getAdtsFrameSize((uint8_t *)data, offset, size)) == 0) {
            SL_LOGE("found ADTS frame of size 0 at offset %lld", (long long) offset);
            return SL_RESULT_CONTENT_CORRUPTED;
        }
        //SL_LOGV("last good offset %llu", offset);
        offset += frameSize;
        if (offset > (off64_t) size) {
            SL_LOGE("found incomplete ADTS frame at end of data");
            return SL_RESULT_CONTENT_CORRUPTED;
        }
    }
    if (offset != (off64_t) size) {
        SL_LOGE("ADTS parsing error: reached end of incomplete frame");
    }
    assert(offset == (off64_t) size);
    return SL_RESULT_SUCCESS;
}

//--------------------------------------------------------------------------------------------------
AacBqToPcmCbRenderer::AacBqToPcmCbRenderer(const AudioPlayback_Parameters* params,
        IAndroidBufferQueue *androidBufferQueue) :
        AudioToCbRenderer(params),
        mBqSource(new BufferQueueSource(androidBufferQueue))
{
    SL_LOGD("AacBqToPcmCbRenderer::AacBqToPcmCbRenderer()");
}


AacBqToPcmCbRenderer::~AacBqToPcmCbRenderer() {
    SL_LOGD("AacBqToPcmCbRenderer::~AacBqToPcmCbRenderer()");

}


//--------------------------------------------------
// Event handlers
void AacBqToPcmCbRenderer::onPrepare() {
    SL_LOGD("AacBqToPcmCbRenderer::onPrepare()");
    Mutex::Autolock _l(mBufferSourceLock);

    // Initialize the PCM format info with the known parameters before the start of the decode
    {
        android::Mutex::Autolock autoLock(mPcmFormatLock);
        mPcmFormatValues[ANDROID_KEY_INDEX_PCMFORMAT_BITSPERSAMPLE] = SL_PCMSAMPLEFORMAT_FIXED_16;
        mPcmFormatValues[ANDROID_KEY_INDEX_PCMFORMAT_CONTAINERSIZE] = 16;
        //FIXME not true on all platforms
        mPcmFormatValues[ANDROID_KEY_INDEX_PCMFORMAT_ENDIANNESS] = SL_BYTEORDER_LITTLEENDIAN;
        //    initialization with the default values: they will be replaced by the actual values
        //      once the decoder has figured them out
        mPcmFormatValues[ANDROID_KEY_INDEX_PCMFORMAT_NUMCHANNELS] = UNKNOWN_NUMCHANNELS;
        mPcmFormatValues[ANDROID_KEY_INDEX_PCMFORMAT_SAMPLERATE] = UNKNOWN_SAMPLERATE;
        mPcmFormatValues[ANDROID_KEY_INDEX_PCMFORMAT_CHANNELMASK] = SL_ANDROID_UNKNOWN_CHANNELMASK;
    }

    sp<AacAdtsExtractor> extractor = new AacAdtsExtractor(mBqSource);

    // only decoding a single track of data
    const size_t kTrackToDecode = 0;

    sp<MediaSource> source = extractor->getTrack(kTrackToDecode);
    if (source == 0) {
        SL_LOGE("AacBqToPcmCbRenderer::onPrepare: error getting source from extractor");
        notifyPrepared(ERROR_UNSUPPORTED);
        return;
    }

    // the audio content is not raw PCM, so we need a decoder
    source = SimpleDecodingSource::Create(source);

    if (source == NULL) {
        SL_LOGE("AacBqToPcmCbRenderer::onPrepare: Could not instantiate decoder.");
        notifyPrepared(ERROR_UNSUPPORTED);
        return;
    }

    sp<MetaData> meta = source->getFormat();

    SL_LOGD("AacBqToPcmCbRenderer::onPrepare() after instantiating decoder");

    if (source->start() != OK) {
        SL_LOGE("AacBqToPcmCbRenderer::onPrepare() Failed to start source/decoder.");
        notifyPrepared(MEDIA_ERROR_BASE);
        return;
    }

    //---------------------------------
    int32_t channelCount;
    CHECK(meta->findInt32(kKeyChannelCount, &channelCount));
    int32_t sr;
    CHECK(meta->findInt32(kKeySampleRate, &sr));
    // FIXME similar to AudioSfDecoder::onPrepare()

    // already "good to go" (compare to AudioSfDecoder::onPrepare)
    mCacheStatus = kStatusHigh;
    mCacheFill = 1000;
    notifyStatus();
    notifyCacheFill();

    {
        android::Mutex::Autolock autoLock(mPcmFormatLock);
        mPcmFormatValues[ANDROID_KEY_INDEX_PCMFORMAT_SAMPLERATE] = sr;
        mPcmFormatValues[ANDROID_KEY_INDEX_PCMFORMAT_NUMCHANNELS] = channelCount;
        mPcmFormatValues[ANDROID_KEY_INDEX_PCMFORMAT_CHANNELMASK] =
                sles_channel_out_mask_from_count(channelCount);
    }
    SL_LOGV("AacBqToPcmCbRenderer::onPrepare() channel count=%d SR=%d",
            channelCount, sr);

    //---------------------------------
    // The data source, and audio source (a decoder) are ready to be used
    mDataSource = mBqSource;
    mAudioSource = source;
    mAudioSourceStarted = true;

    //-------------------------------------
    // signal successful completion of prepare
    mStateFlags |= kFlagPrepared;

    // skipping past AudioToCbRenderer and AudioSfDecoder
    GenericPlayer::onPrepare();

    SL_LOGD("AacBqToPcmCbRenderer::onPrepare() done, mStateFlags=0x%x", mStateFlags);
}

} // namespace android
