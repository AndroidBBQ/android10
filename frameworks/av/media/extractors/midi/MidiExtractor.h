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

#ifndef MIDI_EXTRACTOR_H_
#define MIDI_EXTRACTOR_H_

#include <media/DataSourceBase.h>
#include <media/MediaExtractorPluginApi.h>
#include <media/MediaExtractorPluginHelper.h>
#include <media/stagefright/MediaBufferBase.h>
#include <media/stagefright/MediaBufferGroup.h>
#include <media/MidiIoWrapper.h>
#include <media/NdkMediaFormat.h>
#include <utils/String8.h>
#include <libsonivox/eas.h>

namespace android {

class MidiEngine {
public:
    explicit MidiEngine(CDataSource *dataSource,
            AMediaFormat *fileMetadata,
            AMediaFormat *trackMetadata);
    ~MidiEngine();

    status_t initCheck();

    status_t allocateBuffers(MediaBufferGroupHelper *group);
    status_t releaseBuffers();
    status_t seekTo(int64_t positionUs);
    MediaBufferHelper* readBuffer();
private:
    MidiIoWrapper *mIoWrapper;
    MediaBufferGroupHelper *mGroup;
    EAS_DATA_HANDLE mEasData;
    EAS_HANDLE mEasHandle;
    const S_EAS_LIB_CONFIG* mEasConfig;
    bool mIsInitialized;
};

class MidiExtractor : public MediaExtractorPluginHelper {

public:
    explicit MidiExtractor(CDataSource *source);

    virtual size_t countTracks();
    virtual MediaTrackHelper *getTrack(size_t index);
    virtual media_status_t getTrackMetaData(AMediaFormat *meta, size_t index, uint32_t flags);

    virtual media_status_t getMetaData(AMediaFormat *meta);
    virtual const char * name() { return "MidiExtractor"; }

protected:
    virtual ~MidiExtractor();

private:
    CDataSource *mDataSource;
    status_t mInitCheck;
    AMediaFormat *mFileMetadata;

    // There is only one track
    AMediaFormat *mTrackMetadata;

    MidiEngine *mEngine;

    EAS_DATA_HANDLE     mEasData;
    EAS_HANDLE          mEasHandle;
    EAS_PCM*            mAudioBuffer;
    EAS_I32             mPlayTime;
    EAS_I32             mDuration;
    EAS_STATE           mState;
    EAS_FILE            mFileLocator;

    MidiExtractor(const MidiExtractor &);
    MidiExtractor &operator=(const MidiExtractor &);

};

bool SniffMidi(CDataSource *source, float *confidence);

}  // namespace android

#endif  // MIDI_EXTRACTOR_H_
