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

//#define LOG_NDEBUG 0
#define LOG_TAG "MidiExtractor"
#include <utils/Log.h>

#include "MidiExtractor.h"

#include <media/MidiIoWrapper.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaBufferGroup.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <libsonivox/eas_reverb.h>
#include <watchdog/Watchdog.h>

namespace android {

// how many Sonivox output buffers to aggregate into one MediaBuffer
static const int NUM_COMBINE_BUFFERS = 4;

class MidiSource : public MediaTrackHelper {

public:
    MidiSource(
            MidiEngine &engine,
            AMediaFormat *trackMetadata);

    virtual media_status_t start();
    virtual media_status_t stop();
    virtual media_status_t getFormat(AMediaFormat *);

    virtual media_status_t read(
            MediaBufferHelper **buffer, const ReadOptions *options = NULL);

protected:
    virtual ~MidiSource();

private:
    MidiEngine &mEngine;
    AMediaFormat *mTrackMetadata;
    bool mInitCheck;
    bool mStarted;

    status_t init();

    // no copy constructor or assignment
    MidiSource(const MidiSource &);
    MidiSource &operator=(const MidiSource &);

};


// Midisource

MidiSource::MidiSource(
        MidiEngine &engine,
        AMediaFormat *trackMetadata)
    : mEngine(engine),
      mTrackMetadata(trackMetadata),
      mInitCheck(false),
      mStarted(false)
{
    ALOGV("MidiSource ctor");
    mInitCheck = init();
}

MidiSource::~MidiSource()
{
    ALOGV("MidiSource dtor");
    if (mStarted) {
        stop();
    }
}

media_status_t MidiSource::start()
{
    ALOGV("MidiSource::start");

    CHECK(!mStarted);
    mStarted = true;
    mEngine.allocateBuffers(mBufferGroup);
    return AMEDIA_OK;
}

media_status_t MidiSource::stop()
{
    ALOGV("MidiSource::stop");

    CHECK(mStarted);
    mStarted = false;
    mEngine.releaseBuffers();

    return AMEDIA_OK;
}

media_status_t MidiSource::getFormat(AMediaFormat *meta)
{
    return AMediaFormat_copy(meta, mTrackMetadata);
}

media_status_t MidiSource::read(
        MediaBufferHelper **outBuffer, const ReadOptions *options)
{
    ALOGV("MidiSource::read");

    MediaBufferHelper *buffer;
    // process an optional seek request
    int64_t seekTimeUs;
    ReadOptions::SeekMode mode;
    if ((NULL != options) && options->getSeekTo(&seekTimeUs, &mode)) {
        if (seekTimeUs <= 0LL) {
            seekTimeUs = 0LL;
        }
        mEngine.seekTo(seekTimeUs);
    }
    buffer = mEngine.readBuffer();
    *outBuffer = buffer;
    ALOGV("MidiSource::read %p done", this);
    return buffer != NULL ? AMEDIA_OK : AMEDIA_ERROR_END_OF_STREAM;
}

status_t MidiSource::init()
{
    ALOGV("MidiSource::init");
    return OK;
}

// MidiEngine
using namespace std::chrono_literals;
static constexpr auto kTimeout = 10s;

MidiEngine::MidiEngine(CDataSource *dataSource,
        AMediaFormat *fileMetadata,
        AMediaFormat *trackMetadata) :
            mEasData(NULL),
            mEasHandle(NULL),
            mEasConfig(NULL),
            mIsInitialized(false) {
    Watchdog watchdog(kTimeout);

    mIoWrapper = new MidiIoWrapper(dataSource);
    // spin up a new EAS engine
    EAS_I32 temp;
    EAS_RESULT result = EAS_Init(&mEasData);

    if (result == EAS_SUCCESS) {
        result = EAS_OpenFile(mEasData, mIoWrapper->getLocator(), &mEasHandle);
    }
    if (result == EAS_SUCCESS) {
        result = EAS_Prepare(mEasData, mEasHandle);
    }
    if (result == EAS_SUCCESS) {
        result = EAS_ParseMetaData(mEasData, mEasHandle, &temp);
    }

    if (result != EAS_SUCCESS) {
        return;
    }

    if (fileMetadata != NULL) {
        AMediaFormat_setString(fileMetadata, AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_MIDI);
    }

    if (trackMetadata != NULL) {
        AMediaFormat_setString(trackMetadata, AMEDIAFORMAT_KEY_MIME, MEDIA_MIMETYPE_AUDIO_RAW);
        AMediaFormat_setInt64(
                trackMetadata, AMEDIAFORMAT_KEY_DURATION, 1000ll * temp); // milli->micro
        mEasConfig = EAS_Config();
        AMediaFormat_setInt32(
                trackMetadata, AMEDIAFORMAT_KEY_SAMPLE_RATE, mEasConfig->sampleRate);
        AMediaFormat_setInt32(
                trackMetadata, AMEDIAFORMAT_KEY_CHANNEL_COUNT, mEasConfig->numChannels);
        AMediaFormat_setInt32(
                trackMetadata, AMEDIAFORMAT_KEY_PCM_ENCODING, kAudioEncodingPcm16bit);
    }
    mIsInitialized = true;
}

MidiEngine::~MidiEngine() {
    Watchdog watchdog(kTimeout);

    if (mEasHandle) {
        EAS_CloseFile(mEasData, mEasHandle);
    }
    if (mEasData) {
        EAS_Shutdown(mEasData);
    }
    delete mIoWrapper;
}

status_t MidiEngine::initCheck() {
    return mIsInitialized ? OK : UNKNOWN_ERROR;
}

status_t MidiEngine::allocateBuffers(MediaBufferGroupHelper *group) {
    // select reverb preset and enable
    EAS_SetParameter(mEasData, EAS_MODULE_REVERB, EAS_PARAM_REVERB_PRESET, EAS_PARAM_REVERB_CHAMBER);
    EAS_SetParameter(mEasData, EAS_MODULE_REVERB, EAS_PARAM_REVERB_BYPASS, EAS_FALSE);

    int bufsize = sizeof(EAS_PCM)
            * mEasConfig->mixBufferSize * mEasConfig->numChannels * NUM_COMBINE_BUFFERS;
    ALOGV("using %d byte buffer", bufsize);
    mGroup = group;
    mGroup->add_buffer(bufsize);
    return OK;
}

status_t MidiEngine::releaseBuffers() {
    return OK;
}

status_t MidiEngine::seekTo(int64_t positionUs) {
    Watchdog watchdog(kTimeout);

    ALOGV("seekTo %lld", (long long)positionUs);
    EAS_RESULT result = EAS_Locate(mEasData, mEasHandle, positionUs / 1000, false);
    return result == EAS_SUCCESS ? OK : UNKNOWN_ERROR;
}

MediaBufferHelper* MidiEngine::readBuffer() {
    Watchdog watchdog(kTimeout);

    EAS_STATE state;
    EAS_State(mEasData, mEasHandle, &state);
    if ((state == EAS_STATE_STOPPED) || (state == EAS_STATE_ERROR)) {
        return NULL;
    }
    MediaBufferHelper *buffer;
    status_t err = mGroup->acquire_buffer(&buffer);
    if (err != OK) {
        ALOGE("readBuffer: no buffer");
        return NULL;
    }
    EAS_I32 timeMs;
    EAS_GetLocation(mEasData, mEasHandle, &timeMs);
    int64_t timeUs = 1000ll * timeMs;
    AMediaFormat *meta = buffer->meta_data();
    AMediaFormat_setInt64(meta, AMEDIAFORMAT_KEY_TIME_US, timeUs);
    AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_IS_SYNC_FRAME, 1);

    EAS_PCM* p = (EAS_PCM*) buffer->data();
    int numBytesOutput = 0;
    for (int i = 0; i < NUM_COMBINE_BUFFERS; i++) {
        EAS_I32 numRendered;
        EAS_RESULT result = EAS_Render(mEasData, p, mEasConfig->mixBufferSize, &numRendered);
        if (result != EAS_SUCCESS) {
            ALOGE("EAS_Render() returned %ld, numBytesOutput = %d", result, numBytesOutput);
            buffer->release();
            return NULL; // Stop processing to prevent infinite loops.
        }
        p += numRendered * mEasConfig->numChannels;
        numBytesOutput += numRendered * mEasConfig->numChannels * sizeof(EAS_PCM);
    }
    buffer->set_range(0, numBytesOutput);
    ALOGV("readBuffer: returning %zd in buffer %p", buffer->range_length(), buffer);
    return buffer;
}


// MidiExtractor

MidiExtractor::MidiExtractor(
        CDataSource *dataSource)
    : mDataSource(dataSource),
      mInitCheck(false)
{
    ALOGV("MidiExtractor ctor");
    mFileMetadata = AMediaFormat_new();
    mTrackMetadata = AMediaFormat_new();
    mEngine = new MidiEngine(mDataSource, mFileMetadata, mTrackMetadata);
    mInitCheck = mEngine->initCheck();
}

MidiExtractor::~MidiExtractor()
{
    ALOGV("MidiExtractor dtor");
    AMediaFormat_delete(mFileMetadata);
    AMediaFormat_delete(mTrackMetadata);
    delete mEngine;
}

size_t MidiExtractor::countTracks()
{
    return mInitCheck == OK ? 1 : 0;
}

MediaTrackHelper *MidiExtractor::getTrack(size_t index)
{
    if (mInitCheck != OK || index > 0) {
        return NULL;
    }
    return new MidiSource(*mEngine, mTrackMetadata);
}

media_status_t MidiExtractor::getTrackMetaData(
        AMediaFormat *meta,
        size_t index, uint32_t /* flags */) {
    ALOGV("MidiExtractor::getTrackMetaData");
    if (mInitCheck != OK || index > 0) {
        return AMEDIA_ERROR_UNKNOWN;
    }
    return AMediaFormat_copy(meta, mTrackMetadata);
}

media_status_t MidiExtractor::getMetaData(AMediaFormat *meta)
{
    ALOGV("MidiExtractor::getMetaData");
    return AMediaFormat_copy(meta, mFileMetadata);
}

// Sniffer

bool SniffMidi(CDataSource *source, float *confidence)
{
    MidiEngine p(source, NULL, NULL);
    if (p.initCheck() == OK) {
        *confidence = 0.8;
        ALOGV("SniffMidi: yes");
        return true;
    }
    ALOGV("SniffMidi: no");
    return false;

}

static const char *extensions[] = {
    "imy",
    "mid",
    "midi",
    "mxmf",
    "ota",
    "rtttl",
    "rtx",
    "smf",
    "xmf",
    NULL
};

extern "C" {
// This is the only symbol that needs to be exported
__attribute__ ((visibility ("default")))
ExtractorDef GETEXTRACTORDEF() {
    return {
        EXTRACTORDEF_VERSION,
        UUID("ef6cca0a-f8a2-43e6-ba5f-dfcd7c9a7ef2"),
        1,
        "MIDI Extractor",
        {
            .v3 = {
                [](
                CDataSource *source,
                float *confidence,
                void **,
                FreeMetaFunc *) -> CreatorFunc {
                    if (SniffMidi(source, confidence)) {
                        return [](
                                CDataSource *source,
                                void *) -> CMediaExtractor* {
                            return wrap(new MidiExtractor(source));};
                    }
                    return NULL;
                },
                extensions
            }
        },
    };
}

} // extern "C"

}  // namespace android
