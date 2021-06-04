/*
 * Copyright 2016 The Android Open Source Project
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

#ifndef LEGACY_AUDIO_STREAM_RECORD_H
#define LEGACY_AUDIO_STREAM_RECORD_H

#include <media/AudioRecord.h>
#include <aaudio/AAudio.h>

#include "AudioStreamBuilder.h"
#include "AudioStream.h"
#include "AAudioLegacy.h"
#include "legacy/AudioStreamLegacy.h"
#include "utility/FixedBlockWriter.h"

namespace aaudio {

/**
 * Internal stream that uses the legacy AudioRecord path.
 */
class AudioStreamRecord : public AudioStreamLegacy {
public:
    AudioStreamRecord();

    virtual ~AudioStreamRecord();

    aaudio_result_t open(const AudioStreamBuilder & builder) override;
    aaudio_result_t close() override;

    aaudio_result_t requestStart() override;
    aaudio_result_t requestStop() override;

    virtual aaudio_result_t getTimestamp(clockid_t clockId,
                                         int64_t *framePosition,
                                         int64_t *timeNanoseconds) override;

    aaudio_result_t read(void *buffer,
                             int32_t numFrames,
                             int64_t timeoutNanoseconds) override;

    aaudio_result_t setBufferSize(int32_t requestedFrames) override;

    int32_t getBufferSize() const override;

    int32_t getBufferCapacity() const override;

    int32_t getXRunCount() const override;

    int64_t getFramesWritten() override;

    int32_t getFramesPerBurst() const override;

    aaudio_result_t updateStateMachine() override;

    aaudio_direction_t getDirection() const override {
        return AAUDIO_DIRECTION_INPUT;
    }

    // This is public so it can be called from the C callback function.
    void processCallback(int event, void *info) override;

    int64_t incrementClientFrameCounter(int32_t frames) override {
        return incrementFramesRead(frames);
    }

    const void * maybeConvertDeviceData(const void *audioData, int32_t numFrames) override;

private:
    android::sp<android::AudioRecord> mAudioRecord;
    // adapts between variable sized blocks and fixed size blocks
    FixedBlockWriter                 mFixedBlockWriter;

    // TODO add 64-bit position reporting to AudioRecord and use it.
    android::String16                mOpPackageName;

    // Only one type of conversion buffer is used.
    std::unique_ptr<float[]>         mFormatConversionBufferFloat;
    std::unique_ptr<int16_t[]>       mFormatConversionBufferI16;
    int32_t                          mFormatConversionBufferSizeInFrames = 0;
};

} /* namespace aaudio */

#endif /* LEGACY_AUDIO_STREAM_RECORD_H */
