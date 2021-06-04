/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef ANDROID_AAUDIO_FLOW_GRAPH_H
#define ANDROID_AAUDIO_FLOW_GRAPH_H

#include <memory>
#include <stdint.h>
#include <sys/types.h>
#include <system/audio.h>

#include <aaudio/AAudio.h>
#include <flowgraph/ClipToRange.h>
#include <flowgraph/MonoToMultiConverter.h>
#include <flowgraph/RampLinear.h>

class AAudioFlowGraph {
public:
    /** Connect several modules together to convert from source to sink.
     * This should only be called once for each instance.
     *
     * @param sourceFormat
     * @param sourceChannelCount
     * @param sinkFormat
     * @param sinkChannelCount
     * @return
     */
    aaudio_result_t configure(audio_format_t sourceFormat,
                              int32_t sourceChannelCount,
                              audio_format_t sinkFormat,
                              int32_t sinkChannelCount);

    void process(const void *source, void *destination, int32_t numFrames);

    /**
     * @param volume between 0.0 and 1.0
     */
    void setTargetVolume(float volume);

    void setRampLengthInFrames(int32_t numFrames);

private:
    std::unique_ptr<flowgraph::AudioSource>          mSource;
    std::unique_ptr<flowgraph::RampLinear>           mVolumeRamp;
    std::unique_ptr<flowgraph::ClipToRange>          mClipper;
    std::unique_ptr<flowgraph::MonoToMultiConverter> mChannelConverter;
    std::unique_ptr<flowgraph::AudioSink>            mSink;
};


#endif //ANDROID_AAUDIO_FLOW_GRAPH_H
