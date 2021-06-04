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

#ifndef AUDIO_SF_DECODER_H_
#define AUDIO_SF_DECODER_H_

#include <media/DataSource.h>
#include <media/MediaSource.h>
#include <media/stagefright/FileSource.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MetaData.h>
#include "NuCachedSource2.h"
#include "ThrottledSource.h"

#include "android_GenericPlayer.h"

//--------------------------------------------------------------------------------------------------
namespace android {

// keep in sync with the entries of kPcmDecodeMetadataKeys[]
#define ANDROID_KEY_INDEX_PCMFORMAT_NUMCHANNELS   0
#define ANDROID_KEY_INDEX_PCMFORMAT_SAMPLERATE    1
#define ANDROID_KEY_INDEX_PCMFORMAT_BITSPERSAMPLE 2
#define ANDROID_KEY_INDEX_PCMFORMAT_CONTAINERSIZE 3
#define ANDROID_KEY_INDEX_PCMFORMAT_CHANNELMASK   4
#define ANDROID_KEY_INDEX_PCMFORMAT_ENDIANNESS    5

// to keep in sync with the ANDROID_KEY_INDEX_PCMFORMAT_* constants in android_AudioSfDecoder.cpp
static const char* const kPcmDecodeMetadataKeys[] = {
        ANDROID_KEY_PCMFORMAT_NUMCHANNELS, ANDROID_KEY_PCMFORMAT_SAMPLERATE,
        ANDROID_KEY_PCMFORMAT_BITSPERSAMPLE, ANDROID_KEY_PCMFORMAT_CONTAINERSIZE,
        ANDROID_KEY_PCMFORMAT_CHANNELMASK, ANDROID_KEY_PCMFORMAT_ENDIANNESS };
#define NB_PCMMETADATA_KEYS (sizeof(kPcmDecodeMetadataKeys)/sizeof(kPcmDecodeMetadataKeys[0]))

// abstract base class for AudioToCbRenderer and it's subclasses
class AudioSfDecoder : public GenericPlayer
{
public:

    explicit AudioSfDecoder(const AudioPlayback_Parameters* params);
    virtual ~AudioSfDecoder();

    virtual void preDestroy();

    // overridden from GenericPlayer
    virtual void play();
    virtual void getPositionMsec(int* msec); //msec != NULL, ANDROID_UNKNOWN_TIME if unknown

    uint32_t getPcmFormatKeyCount() const;
    bool     getPcmFormatKeySize(uint32_t index, uint32_t* pKeySize);
    bool     getPcmFormatKeyName(uint32_t index, uint32_t keySize, char* keyName);
    bool     getPcmFormatValueSize(uint32_t index, uint32_t* pValueSize);
    bool     getPcmFormatKeyValue(uint32_t index, uint32_t size, uint32_t* pValue);

protected:

    enum {
        kWhatDecode       = 'deco',
        kWhatRender       = 'rend',
        kWhatCheckCache   = 'cach'
    };

    // Async event handlers (called from the AudioSfDecoder's event loop)
    void onDecode();
    void onCheckCache(const sp<AMessage> &msg);
    virtual void onRender() = 0;

    // Async event handlers (called from GenericPlayer's event loop)
    virtual void onPrepare();
    virtual void onPlay();
    virtual void onPause();
    virtual void onSeek(const sp<AMessage> &msg);
    virtual void onLoop(const sp<AMessage> &msg);

    // overridden from GenericPlayer
    virtual void onNotify(const sp<AMessage> &msg);
    virtual void onMessageReceived(const sp<AMessage> &msg);

    // to be implemented by subclasses of AudioSfDecoder to do something with the audio samples
    // (called from GenericPlayer's event loop)
    virtual void createAudioSink() = 0;
    virtual void updateAudioSink() = 0; // called with mBufferSourceLock held
    virtual void startAudioSink() = 0;
    virtual void pauseAudioSink() = 0;

    sp<DataSource>  mDataSource; // where the raw data comes from
    sp<MediaSource> mAudioSource;// the decoder reading from the data source
    // used to indicate mAudioSource was successfully started, but wasn't stopped
    bool            mAudioSourceStarted;

    // negative values indicate invalid value
    int64_t mBitrate;  // in bits/sec
    int64_t mDurationUsec; // ANDROID_UNKNOWN_TIME if unknown

    // buffer passed from decoder to renderer
    MediaBufferBase *mDecodeBuffer;

    // mutex used to protect the decode buffer, the audio source and its running state
    Mutex       mBufferSourceLock;

    void notifyPrepared(status_t prepareRes);

    int64_t mSeekTimeMsec;
    int64_t mLastDecodedPositionUs; // ANDROID_UNKNOWN_TIME if unknown
    // mutex used for seek flag, seek time (mSeekTimeMsec),
    //   and last decoded position (mLastDecodedPositionUs)
    Mutex mTimeLock;

    // informations that can be retrieved in the PCM format queries
    //  these values are only written in the event loop
    uint32_t mPcmFormatValues[NB_PCMMETADATA_KEYS];
    // protects mPcmFormatValues
    Mutex    mPcmFormatLock;

    virtual bool advancesPositionInRealTime() const { return false; }

private:
    bool wantPrefetch();
    CacheStatus_t getCacheRemaining(bool *eos);
    int64_t getPositionUsec(); // ANDROID_UNKNOWN_TIME if unknown

    // convenience function to update internal state when decoding parameters have changed,
    // called with a lock on mBufferSourceLock
    void hasNewDecodeParams();

    static bool isSupportedCodec(const char* mime);

private:
    DISALLOW_EVIL_CONSTRUCTORS(AudioSfDecoder);

};

} // namespace android

#endif // AUDIO_SF_DECODER_H_
