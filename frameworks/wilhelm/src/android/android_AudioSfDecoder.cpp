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

//#define USE_LOG SLAndroidLogLevel_Verbose

#include "sles_allinclusive.h"
#include "android/android_AudioSfDecoder.h"
#include "android/channels.h"

#include <binder/IServiceManager.h>
#include <media/IMediaHTTPService.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/DataSourceFactory.h>
#include <media/stagefright/InterfaceUtils.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaExtractorFactory.h>
#include <media/stagefright/SimpleDecodingSource.h>


#define SIZE_CACHED_HIGH_BYTES 1000000
#define SIZE_CACHED_MED_BYTES   700000
#define SIZE_CACHED_LOW_BYTES   400000

namespace android {

//--------------------------------------------------------------------------------------------------
AudioSfDecoder::AudioSfDecoder(const AudioPlayback_Parameters* params) : GenericPlayer(params),
        mDataSource(0),
        mAudioSource(0),
        mAudioSourceStarted(false),
        mBitrate(-1),
        mDurationUsec(ANDROID_UNKNOWN_TIME),
        mDecodeBuffer(NULL),
        mSeekTimeMsec(0),
        // play event logic depends on the initial time being zero not ANDROID_UNKNOWN_TIME
        mLastDecodedPositionUs(0)
{
    SL_LOGD("AudioSfDecoder::AudioSfDecoder()");
}


AudioSfDecoder::~AudioSfDecoder() {
    SL_LOGD("AudioSfDecoder::~AudioSfDecoder()");
}


void AudioSfDecoder::preDestroy() {
    GenericPlayer::preDestroy();
    SL_LOGD("AudioSfDecoder::preDestroy()");
    {
        Mutex::Autolock _l(mBufferSourceLock);

        if (NULL != mDecodeBuffer) {
            mDecodeBuffer->release();
            mDecodeBuffer = NULL;
        }

        if ((mAudioSource != 0) && mAudioSourceStarted) {
            mAudioSource->stop();
            mAudioSourceStarted = false;
        }
    }
}


//--------------------------------------------------
void AudioSfDecoder::play() {
    SL_LOGD("AudioSfDecoder::play");

    GenericPlayer::play();
    (new AMessage(kWhatDecode, this))->post();
}


void AudioSfDecoder::getPositionMsec(int* msec) {
    int64_t timeUsec = getPositionUsec();
    if (timeUsec == ANDROID_UNKNOWN_TIME) {
        *msec = ANDROID_UNKNOWN_TIME;
    } else {
        *msec = timeUsec / 1000;
    }
}


//--------------------------------------------------
uint32_t AudioSfDecoder::getPcmFormatKeyCount() const {
    return NB_PCMMETADATA_KEYS;
}


//--------------------------------------------------
bool AudioSfDecoder::getPcmFormatKeySize(uint32_t index, uint32_t* pKeySize) {
    if (index >= NB_PCMMETADATA_KEYS) {
        return false;
    } else {
        *pKeySize = strlen(kPcmDecodeMetadataKeys[index]) +1;
        return true;
    }
}


//--------------------------------------------------
bool AudioSfDecoder::getPcmFormatKeyName(uint32_t index, uint32_t keySize, char* keyName) {
    uint32_t actualKeySize;
    if (!getPcmFormatKeySize(index, &actualKeySize)) {
        return false;
    }
    if (keySize < actualKeySize) {
        return false;
    }
    strncpy(keyName, kPcmDecodeMetadataKeys[index], actualKeySize);
    return true;
}


//--------------------------------------------------
bool AudioSfDecoder::getPcmFormatValueSize(uint32_t index, uint32_t* pValueSize) {
    if (index >= NB_PCMMETADATA_KEYS) {
        *pValueSize = 0;
        return false;
    } else {
        *pValueSize = sizeof(uint32_t);
        return true;
    }
}


//--------------------------------------------------
bool AudioSfDecoder::getPcmFormatKeyValue(uint32_t index, uint32_t size, uint32_t* pValue) {
    uint32_t valueSize = 0;
    if (!getPcmFormatValueSize(index, &valueSize)) {
        return false;
    } else if (size != valueSize) {
        // this ensures we are accessing mPcmFormatValues with a valid size for that index
        SL_LOGE("Error retrieving metadata value at index %d: using size of %d, should be %d",
                index, size, valueSize);
        return false;
    } else {
        android::Mutex::Autolock autoLock(mPcmFormatLock);
        *pValue = mPcmFormatValues[index];
        return true;
    }
}


//--------------------------------------------------
// Event handlers
//  it is strictly verboten to call those methods outside of the event loop

// Initializes the data and audio sources, and update the PCM format info
// post-condition: upon successful initialization based on the player data locator
//    GenericPlayer::onPrepare() was called
//    mDataSource != 0
//    mAudioSource != 0
//    mAudioSourceStarted == true
// All error returns from this method are via notifyPrepared(status) followed by "return".
void AudioSfDecoder::onPrepare() {
    SL_LOGD("AudioSfDecoder::onPrepare()");
    Mutex::Autolock _l(mBufferSourceLock);

    {
    android::Mutex::Autolock autoLock(mPcmFormatLock);
    // Initialize the PCM format info with the known parameters before the start of the decode
    mPcmFormatValues[ANDROID_KEY_INDEX_PCMFORMAT_BITSPERSAMPLE] = SL_PCMSAMPLEFORMAT_FIXED_16;
    mPcmFormatValues[ANDROID_KEY_INDEX_PCMFORMAT_CONTAINERSIZE] = 16;
    mPcmFormatValues[ANDROID_KEY_INDEX_PCMFORMAT_ENDIANNESS] = SL_BYTEORDER_LITTLEENDIAN;
    //    initialization with the default values: they will be replaced by the actual values
    //      once the decoder has figured them out
    mPcmFormatValues[ANDROID_KEY_INDEX_PCMFORMAT_NUMCHANNELS] = UNKNOWN_NUMCHANNELS;
    mPcmFormatValues[ANDROID_KEY_INDEX_PCMFORMAT_SAMPLERATE] = UNKNOWN_SAMPLERATE;
    mPcmFormatValues[ANDROID_KEY_INDEX_PCMFORMAT_CHANNELMASK] = SL_ANDROID_UNKNOWN_CHANNELMASK;
    }

    //---------------------------------
    // Instantiate and initialize the data source for the decoder
    sp<DataSource> dataSource;

    switch (mDataLocatorType) {

    case kDataLocatorNone:
        SL_LOGE("AudioSfDecoder::onPrepare: no data locator set");
        notifyPrepared(MEDIA_ERROR_BASE);
        return;

    case kDataLocatorUri:
        dataSource = DataSourceFactory::CreateFromURI(
                NULL /* XXX httpService */, mDataLocator.uriRef);
        if (dataSource == NULL) {
            SL_LOGE("AudioSfDecoder::onPrepare(): Error opening %s", mDataLocator.uriRef);
            notifyPrepared(MEDIA_ERROR_BASE);
            return;
        }
        break;

    case kDataLocatorFd:
    {
        // As FileSource unconditionally takes ownership of the fd and closes it, then
        // we have to make a dup for FileSource if the app wants to keep ownership itself
        int fd = mDataLocator.fdi.fd;
        if (mDataLocator.fdi.mCloseAfterUse) {
            mDataLocator.fdi.mCloseAfterUse = false;
        } else {
            fd = ::dup(fd);
        }
        dataSource = new FileSource(fd, mDataLocator.fdi.offset, mDataLocator.fdi.length);
        status_t err = dataSource->initCheck();
        if (err != OK) {
            notifyPrepared(err);
            return;
        }
        break;
    }

    // AndroidBufferQueue data source is handled by a subclass,
    // which does not call up to this method.  Hence, the missing case.
    default:
        TRESPASS();
    }

    //---------------------------------
    // Instantiate and initialize the decoder attached to the data source
    sp<IMediaExtractor> extractor = MediaExtractorFactory::Create(dataSource);
    if (extractor == NULL) {
        SL_LOGE("AudioSfDecoder::onPrepare: Could not instantiate extractor.");
        notifyPrepared(ERROR_UNSUPPORTED);
        return;
    }

    ssize_t audioTrackIndex = -1;
    bool isRawAudio = false;
    for (size_t i = 0; i < extractor->countTracks(); ++i) {
        sp<MetaData> meta = extractor->getTrackMetaData(i);

        const char *mime;
        CHECK(meta->findCString(kKeyMIMEType, &mime));

        if (!strncasecmp("audio/", mime, 6)) {
            if (isSupportedCodec(mime)) {
                audioTrackIndex = i;

                if (!strcasecmp(MEDIA_MIMETYPE_AUDIO_RAW, mime)) {
                    isRawAudio = true;
                }
                break;
            }
        }
    }

    if (audioTrackIndex < 0) {
        SL_LOGE("AudioSfDecoder::onPrepare: Could not find a supported audio track.");
        notifyPrepared(ERROR_UNSUPPORTED);
        return;
    }

    sp<MediaSource> source = CreateMediaSourceFromIMediaSource(
            extractor->getTrack(audioTrackIndex));
    sp<MetaData> meta = source->getFormat();

    // we can't trust the OMXCodec (if there is one) to issue a INFO_FORMAT_CHANGED so we want
    // to have some meaningful values as soon as possible.
    int32_t channelCount;
    bool hasChannelCount = meta->findInt32(kKeyChannelCount, &channelCount);
    int32_t sr;
    bool hasSampleRate = meta->findInt32(kKeySampleRate, &sr);

    // first compute the duration
    off64_t size;
    int64_t durationUs;
    int32_t durationMsec;
    if (dataSource->getSize(&size) == OK
            && meta->findInt64(kKeyDuration, &durationUs)) {
        if (durationUs != 0) {
            mBitrate = size * 8000000LL / durationUs;  // in bits/sec
        } else {
            mBitrate = -1;
        }
        mDurationUsec = durationUs;
        durationMsec = durationUs / 1000;
    } else {
        mBitrate = -1;
        mDurationUsec = ANDROID_UNKNOWN_TIME;
        durationMsec = ANDROID_UNKNOWN_TIME;
    }

    // then assign the duration under the settings lock
    {
        Mutex::Autolock _l(mSettingsLock);
        mDurationMsec = durationMsec;
    }

    // the audio content is not raw PCM, so we need a decoder
    if (!isRawAudio) {
        source = SimpleDecodingSource::Create(source);
        if (source == NULL) {
            SL_LOGE("AudioSfDecoder::onPrepare: Could not instantiate decoder.");
            notifyPrepared(ERROR_UNSUPPORTED);
            return;
        }

        meta = source->getFormat();
    }


    if (source->start() != OK) {
        SL_LOGE("AudioSfDecoder::onPrepare: Failed to start source/decoder.");
        notifyPrepared(MEDIA_ERROR_BASE);
        return;
    }

    //---------------------------------
    // The data source, and audio source (a decoder if required) are ready to be used
    mDataSource = dataSource;
    mAudioSource = source;
    mAudioSourceStarted = true;

    if (!hasChannelCount) {
        CHECK(meta->findInt32(kKeyChannelCount, &channelCount));
    }

    if (!hasSampleRate) {
        CHECK(meta->findInt32(kKeySampleRate, &sr));
    }
    // FIXME add code below once channel mask support is in, currently initialized to default
    //       value computed from the channel count
    //    if (!hasChannelMask) {
    //        CHECK(meta->findInt32(kKeyChannelMask, &channelMask));
    //    }

    if (!wantPrefetch()) {
        SL_LOGV("AudioSfDecoder::onPrepare: no need to prefetch");
        // doesn't need prefetching, notify good to go
        mCacheStatus = kStatusHigh;
        mCacheFill = 1000;
        notifyStatus();
        notifyCacheFill();
    }

    {
        android::Mutex::Autolock autoLock(mPcmFormatLock);
        mPcmFormatValues[ANDROID_KEY_INDEX_PCMFORMAT_SAMPLERATE] = sr;
        mPcmFormatValues[ANDROID_KEY_INDEX_PCMFORMAT_NUMCHANNELS] = channelCount;
        mPcmFormatValues[ANDROID_KEY_INDEX_PCMFORMAT_CHANNELMASK] =
                sles_channel_out_mask_from_count(channelCount);
    }

    // at this point we have enough information about the source to create the sink that
    // will consume the data
    createAudioSink();

    // signal successful completion of prepare
    mStateFlags |= kFlagPrepared;

    GenericPlayer::onPrepare();
    SL_LOGD("AudioSfDecoder::onPrepare() done, mStateFlags=0x%x", mStateFlags);
}


void AudioSfDecoder::onPause() {
    SL_LOGV("AudioSfDecoder::onPause()");
    GenericPlayer::onPause();
    pauseAudioSink();
}


void AudioSfDecoder::onPlay() {
    SL_LOGV("AudioSfDecoder::onPlay()");
    GenericPlayer::onPlay();
    startAudioSink();
}


void AudioSfDecoder::onSeek(const sp<AMessage> &msg) {
    SL_LOGV("AudioSfDecoder::onSeek");
    int64_t timeMsec;
    CHECK(msg->findInt64(WHATPARAM_SEEK_SEEKTIME_MS, &timeMsec));

    Mutex::Autolock _l(mTimeLock);
    mStateFlags |= kFlagSeeking;
    mSeekTimeMsec = timeMsec;
    // don't set mLastDecodedPositionUs to ANDROID_UNKNOWN_TIME; getPositionUsec
    // ignores mLastDecodedPositionUs while seeking, and substitutes the seek goal instead

    // nop for now
    GenericPlayer::onSeek(msg);
}


void AudioSfDecoder::onLoop(const sp<AMessage> &msg) {
    SL_LOGV("AudioSfDecoder::onLoop");
    int32_t loop;
    CHECK(msg->findInt32(WHATPARAM_LOOP_LOOPING, &loop));

    if (loop) {
        //SL_LOGV("AudioSfDecoder::onLoop start looping");
        mStateFlags |= kFlagLooping;
    } else {
        //SL_LOGV("AudioSfDecoder::onLoop stop looping");
        mStateFlags &= ~kFlagLooping;
    }

    // nop for now
    GenericPlayer::onLoop(msg);
}


void AudioSfDecoder::onCheckCache(const sp<AMessage> &msg) {
    //SL_LOGV("AudioSfDecoder::onCheckCache");
    bool eos;
    CacheStatus_t status = getCacheRemaining(&eos);

    if (eos || status == kStatusHigh
            || ((mStateFlags & kFlagPreparing) && (status >= kStatusEnough))) {
        if (mStateFlags & kFlagPlaying) {
            startAudioSink();
        }
        mStateFlags &= ~kFlagBuffering;

        SL_LOGV("AudioSfDecoder::onCheckCache: buffering done.");

        if (mStateFlags & kFlagPreparing) {
            //SL_LOGV("AudioSfDecoder::onCheckCache: preparation done.");
            mStateFlags &= ~kFlagPreparing;
        }

        if (mStateFlags & kFlagPlaying) {
            (new AMessage(kWhatDecode, this))->post();
        }
        return;
    }

    msg->post(100000);
}


void AudioSfDecoder::onDecode() {
    SL_LOGV("AudioSfDecoder::onDecode");

    //-------------------------------- Need to buffer some more before decoding?
    bool eos;
    if (mDataSource == 0) {
        // application set play state to paused which failed, then set play state to playing
        return;
    }

    if (wantPrefetch()
            && (getCacheRemaining(&eos) == kStatusLow)
            && !eos) {
        SL_LOGV("buffering more.");

        if (mStateFlags & kFlagPlaying) {
            pauseAudioSink();
        }
        mStateFlags |= kFlagBuffering;
        (new AMessage(kWhatCheckCache, this))->post(100000);
        return;
    }

    if (!(mStateFlags & (kFlagPlaying | kFlagBuffering | kFlagPreparing))) {
        // don't decode if we're not buffering, prefetching or playing
        //SL_LOGV("don't decode: not buffering, prefetching or playing");
        return;
    }

    //-------------------------------- Decode
    status_t err;
    MediaSource::ReadOptions readOptions;
    if (mStateFlags & kFlagSeeking) {
        assert(mSeekTimeMsec != ANDROID_UNKNOWN_TIME);
        readOptions.setSeekTo(mSeekTimeMsec * 1000);
    }

    int64_t timeUsec = ANDROID_UNKNOWN_TIME;
    {
        Mutex::Autolock _l(mBufferSourceLock);

        if (NULL != mDecodeBuffer) {
            // the current decoded buffer hasn't been rendered, drop it
            mDecodeBuffer->release();
            mDecodeBuffer = NULL;
        }
        if (!mAudioSourceStarted) {
            return;
        }
        err = mAudioSource->read(&mDecodeBuffer, &readOptions);
        if (err == OK) {
            // FIXME workaround apparent bug in AAC decoder: kKeyTime is 3 frames old if length is 0
            if (mDecodeBuffer->range_length() == 0) {
                timeUsec = ANDROID_UNKNOWN_TIME;
            } else {
                CHECK(mDecodeBuffer->meta_data().findInt64(kKeyTime, &timeUsec));
            }
        } else {
            // errors are handled below
        }
    }

    {
        Mutex::Autolock _l(mTimeLock);
        if (mStateFlags & kFlagSeeking) {
            mStateFlags &= ~kFlagSeeking;
            mSeekTimeMsec = ANDROID_UNKNOWN_TIME;
        }
        if (timeUsec != ANDROID_UNKNOWN_TIME) {
            // Note that though we've decoded this position, we haven't rendered it yet.
            // So a GetPosition called after this point will observe the advanced position,
            // even though the PCM may not have been supplied to the sink.  That's OK as
            // we don't claim to provide AAC frame-accurate (let alone sample-accurate) GetPosition.
            mLastDecodedPositionUs = timeUsec;
        }
    }

    //-------------------------------- Handle return of decode
    if (err != OK) {
        bool continueDecoding = false;
        switch (err) {
            case ERROR_END_OF_STREAM:
                if (0 < mDurationUsec) {
                    Mutex::Autolock _l(mTimeLock);
                    mLastDecodedPositionUs = mDurationUsec;
                }
                // handle notification and looping at end of stream
                if (mStateFlags & kFlagPlaying) {
                    notify(PLAYEREVENT_ENDOFSTREAM, 1, true /*async*/);
                }
                if (mStateFlags & kFlagLooping) {
                    seek(0);
                    // kick-off decoding again
                    continueDecoding = true;
                }
                break;
            case INFO_FORMAT_CHANGED:
                SL_LOGD("MediaSource::read encountered INFO_FORMAT_CHANGED");
                // reconfigure output
                {
                    Mutex::Autolock _l(mBufferSourceLock);
                    hasNewDecodeParams();
                }
                continueDecoding = true;
                break;
            case INFO_DISCONTINUITY:
                SL_LOGD("MediaSource::read encountered INFO_DISCONTINUITY");
                continueDecoding = true;
                break;
            default:
                SL_LOGE("MediaSource::read returned error %d", err);
                break;
        }
        if (continueDecoding) {
            if (NULL == mDecodeBuffer) {
                (new AMessage(kWhatDecode, this))->post();
                return;
            }
        } else {
            return;
        }
    }

    //-------------------------------- Render
    sp<AMessage> msg = new AMessage(kWhatRender, this);
    msg->post();

}


void AudioSfDecoder::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatDecode:
            onDecode();
            break;

        case kWhatRender:
            onRender();
            break;

        case kWhatCheckCache:
            onCheckCache(msg);
            break;

        default:
            GenericPlayer::onMessageReceived(msg);
            break;
    }
}

//--------------------------------------------------
// Prepared state, prefetch status notifications
void AudioSfDecoder::notifyPrepared(status_t prepareRes) {
    assert(!(mStateFlags & (kFlagPrepared | kFlagPreparedUnsuccessfully)));
    if (NO_ERROR == prepareRes) {
        // The "then" fork is not currently used, but is kept here to make it easier
        // to replace by a new signalPrepareCompletion(status) if we re-visit this later.
        mStateFlags |= kFlagPrepared;
    } else {
        mStateFlags |= kFlagPreparedUnsuccessfully;
    }
    // Do not call the superclass onPrepare to notify, because it uses a default error
    // status code but we can provide a more specific one.
    // GenericPlayer::onPrepare();
    notify(PLAYEREVENT_PREPARED, (int32_t)prepareRes, true /*async*/);
    SL_LOGD("AudioSfDecoder::onPrepare() done, mStateFlags=0x%x", mStateFlags);
}


void AudioSfDecoder::onNotify(const sp<AMessage> &msg) {
    notif_cbf_t notifyClient;
    void*       notifyUser;
    {
        android::Mutex::Autolock autoLock(mNotifyClientLock);
        if (NULL == mNotifyClient) {
            return;
        } else {
            notifyClient = mNotifyClient;
            notifyUser   = mNotifyUser;
        }
    }
    int32_t val;
    if (msg->findInt32(PLAYEREVENT_PREFETCHSTATUSCHANGE, &val)) {
        SL_LOGV("\tASfPlayer notifying %s = %d", PLAYEREVENT_PREFETCHSTATUSCHANGE, val);
        notifyClient(kEventPrefetchStatusChange, val, 0, notifyUser);
    }
    else if (msg->findInt32(PLAYEREVENT_PREFETCHFILLLEVELUPDATE, &val)) {
        SL_LOGV("\tASfPlayer notifying %s = %d", PLAYEREVENT_PREFETCHFILLLEVELUPDATE, val);
        notifyClient(kEventPrefetchFillLevelUpdate, val, 0, notifyUser);
    }
    else if (msg->findInt32(PLAYEREVENT_ENDOFSTREAM, &val)) {
        SL_LOGV("\tASfPlayer notifying %s = %d", PLAYEREVENT_ENDOFSTREAM, val);
        notifyClient(kEventEndOfStream, val, 0, notifyUser);
    }
    else {
        GenericPlayer::onNotify(msg);
    }
}


//--------------------------------------------------
// Private utility functions

bool AudioSfDecoder::wantPrefetch() {
    if (mDataSource != 0) {
        return (mDataSource->flags() & DataSource::kWantsPrefetching);
    } else {
        // happens if an improper data locator was passed, if the media extractor couldn't be
        //  initialized, if there is no audio track in the media, if the OMX decoder couldn't be
        //  instantiated, if the source couldn't be opened, or if the MediaSource
        //  couldn't be started
        SL_LOGV("AudioSfDecoder::wantPrefetch() tries to access NULL mDataSource");
        return false;
    }
}


int64_t AudioSfDecoder::getPositionUsec() {
    Mutex::Autolock _l(mTimeLock);
    if (mStateFlags & kFlagSeeking) {
        return mSeekTimeMsec * 1000;
    } else {
        return mLastDecodedPositionUs;
    }
}


CacheStatus_t AudioSfDecoder::getCacheRemaining(bool *eos) {
    sp<NuCachedSource2> cachedSource =
        static_cast<NuCachedSource2 *>(mDataSource.get());

    CacheStatus_t oldStatus = mCacheStatus;

    status_t finalStatus;
    size_t dataRemaining = cachedSource->approxDataRemaining(&finalStatus);
    *eos = (finalStatus != OK);

    CHECK_GE(mBitrate, 0);

    int64_t dataRemainingUs = dataRemaining * 8000000LL / mBitrate;
    //SL_LOGV("AudioSfDecoder::getCacheRemaining: approx %.2f secs remaining (eos=%d)",
    //       dataRemainingUs / 1E6, *eos);

    if (*eos) {
        // data is buffered up to the end of the stream, it can't get any better than this
        mCacheStatus = kStatusHigh;
        mCacheFill = 1000;

    } else {
        if (mDurationUsec > 0) {
            // known duration:

            //   fill level is ratio of how much has been played + how much is
            //   cached, divided by total duration
            int64_t currentPositionUsec = getPositionUsec();
            if (currentPositionUsec == ANDROID_UNKNOWN_TIME) {
                // if we don't know where we are, assume the worst for the fill ratio
                currentPositionUsec = 0;
            }
            if (mDurationUsec > 0) {
                mCacheFill = (int16_t) ((1000.0
                        * (double)(currentPositionUsec + dataRemainingUs) / mDurationUsec));
            } else {
                mCacheFill = 0;
            }
            //SL_LOGV("cacheFill = %d", mCacheFill);

            //   cache status is evaluated against duration thresholds
            if (dataRemainingUs > DURATION_CACHED_HIGH_MS*1000) {
                mCacheStatus = kStatusHigh;
                //ALOGV("high");
            } else if (dataRemainingUs > DURATION_CACHED_MED_MS*1000) {
                //ALOGV("enough");
                mCacheStatus = kStatusEnough;
            } else if (dataRemainingUs < DURATION_CACHED_LOW_MS*1000) {
                //ALOGV("low");
                mCacheStatus = kStatusLow;
            } else {
                mCacheStatus = kStatusIntermediate;
            }

        } else {
            // unknown duration:

            //   cache status is evaluated against cache amount thresholds
            //   (no duration so we don't have the bitrate either, could be derived from format?)
            if (dataRemaining > SIZE_CACHED_HIGH_BYTES) {
                mCacheStatus = kStatusHigh;
            } else if (dataRemaining > SIZE_CACHED_MED_BYTES) {
                mCacheStatus = kStatusEnough;
            } else if (dataRemaining < SIZE_CACHED_LOW_BYTES) {
                mCacheStatus = kStatusLow;
            } else {
                mCacheStatus = kStatusIntermediate;
            }
        }

    }

    if (oldStatus != mCacheStatus) {
        notifyStatus();
    }

    if (abs(mCacheFill - mLastNotifiedCacheFill) > mCacheFillNotifThreshold) {
        notifyCacheFill();
    }

    return mCacheStatus;
}


void AudioSfDecoder::hasNewDecodeParams() {

    if ((mAudioSource != 0) && mAudioSourceStarted) {
        sp<MetaData> meta = mAudioSource->getFormat();

        int32_t channelCount;
        CHECK(meta->findInt32(kKeyChannelCount, &channelCount));
        int32_t sr;
        CHECK(meta->findInt32(kKeySampleRate, &sr));

        // FIXME similar to onPrepare()
        {
            android::Mutex::Autolock autoLock(mPcmFormatLock);
            SL_LOGV("format changed: old sr=%d, channels=%d; new sr=%d, channels=%d",
                    mPcmFormatValues[ANDROID_KEY_INDEX_PCMFORMAT_SAMPLERATE],
                    mPcmFormatValues[ANDROID_KEY_INDEX_PCMFORMAT_NUMCHANNELS],
                    sr, channelCount);
            mPcmFormatValues[ANDROID_KEY_INDEX_PCMFORMAT_NUMCHANNELS] = channelCount;
            mPcmFormatValues[ANDROID_KEY_INDEX_PCMFORMAT_SAMPLERATE] = sr;
            mPcmFormatValues[ANDROID_KEY_INDEX_PCMFORMAT_CHANNELMASK] =
                    sles_channel_out_mask_from_count(channelCount);
        }
        // there's no need to do a notify of PLAYEREVENT_CHANNEL_COUNT,
        // because the only listener is for volume updates, and decoders don't support that
    }

    // alert users of those params
    updateAudioSink();
}

static const char* const kPlaybackOnlyCodecs[] = { MEDIA_MIMETYPE_AUDIO_AMR_NB,
        MEDIA_MIMETYPE_AUDIO_AMR_WB };
#define NB_PLAYBACK_ONLY_CODECS (sizeof(kPlaybackOnlyCodecs)/sizeof(kPlaybackOnlyCodecs[0]))

bool AudioSfDecoder::isSupportedCodec(const char* mime) {
    bool codecRequiresPermission = false;
    for (unsigned int i = 0 ; i < NB_PLAYBACK_ONLY_CODECS ; i++) {
        if (!strcasecmp(mime, kPlaybackOnlyCodecs[i])) {
            codecRequiresPermission = true;
            break;
        }
    }
    if (codecRequiresPermission) {
        // verify only the system can decode, for playback only
        return checkCallingPermission(
                String16("android.permission.ALLOW_ANY_CODEC_FOR_PLAYBACK"));
    } else {
        return true;
    }
}

} // namespace android
