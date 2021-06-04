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

#define LOG_TAG "AAudioFlowGraph"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include "AAudioFlowGraph.h"

#include <flowgraph/ClipToRange.h>
#include <flowgraph/MonoToMultiConverter.h>
#include <flowgraph/RampLinear.h>
#include <flowgraph/SinkFloat.h>
#include <flowgraph/SinkI16.h>
#include <flowgraph/SinkI24.h>
#include <flowgraph/SourceFloat.h>
#include <flowgraph/SourceI16.h>
#include <flowgraph/SourceI24.h>

using namespace flowgraph;

aaudio_result_t AAudioFlowGraph::configure(audio_format_t sourceFormat,
                          int32_t sourceChannelCount,
                          audio_format_t sinkFormat,
                          int32_t sinkChannelCount) {
    AudioFloatOutputPort *lastOutput = nullptr;

    ALOGV("%s() source format = 0x%08x, channels = %d, sink format = 0x%08x, channels = %d",
          __func__, sourceFormat, sourceChannelCount, sinkFormat, sinkChannelCount);

    switch (sourceFormat) {
        case AUDIO_FORMAT_PCM_FLOAT:
            mSource = std::make_unique<SourceFloat>(sourceChannelCount);
            break;
        case AUDIO_FORMAT_PCM_16_BIT:
            mSource = std::make_unique<SourceI16>(sourceChannelCount);
            break;
        case AUDIO_FORMAT_PCM_24_BIT_PACKED:
            mSource = std::make_unique<SourceI24>(sourceChannelCount);
            break;
        default: // TODO add I32
            ALOGE("%s() Unsupported source format = %d", __func__, sourceFormat);
            return AAUDIO_ERROR_UNIMPLEMENTED;
    }
    lastOutput = &mSource->output;

    // Apply volume as a ramp to avoid pops.
    mVolumeRamp = std::make_unique<RampLinear>(sourceChannelCount);
    lastOutput->connect(&mVolumeRamp->input);
    lastOutput = &mVolumeRamp->output;

    // For a pure float graph, there is chance that the data range may be very large.
    // So we should clip to a reasonable value that allows a little headroom.
    if (sourceFormat == AUDIO_FORMAT_PCM_FLOAT && sinkFormat == AUDIO_FORMAT_PCM_FLOAT) {
        mClipper = std::make_unique<ClipToRange>(sourceChannelCount);
        lastOutput->connect(&mClipper->input);
        lastOutput = &mClipper->output;
    }

    // Expand the number of channels if required.
    if (sourceChannelCount == 1 && sinkChannelCount > 1) {
        mChannelConverter = std::make_unique<MonoToMultiConverter>(sinkChannelCount);
        lastOutput->connect(&mChannelConverter->input);
        lastOutput = &mChannelConverter->output;
    } else if (sourceChannelCount != sinkChannelCount) {
        ALOGE("%s() Channel reduction not supported.", __func__);
        return AAUDIO_ERROR_UNIMPLEMENTED;
    }

    switch (sinkFormat) {
        case AUDIO_FORMAT_PCM_FLOAT:
            mSink = std::make_unique<SinkFloat>(sinkChannelCount);
            break;
        case AUDIO_FORMAT_PCM_16_BIT:
            mSink = std::make_unique<SinkI16>(sinkChannelCount);
            break;
        case AUDIO_FORMAT_PCM_24_BIT_PACKED:
            mSink = std::make_unique<SinkI24>(sinkChannelCount);
            break;
        default: // TODO add I32
            ALOGE("%s() Unsupported sink format = %d", __func__, sinkFormat);
            return AAUDIO_ERROR_UNIMPLEMENTED;
    }
    lastOutput->connect(&mSink->input);

    return AAUDIO_OK;
}

void AAudioFlowGraph::process(const void *source, void *destination, int32_t numFrames) {
    mSource->setData(source, numFrames);
    mSink->read(destination, numFrames);
}

/**
 * @param volume between 0.0 and 1.0
 */
void AAudioFlowGraph::setTargetVolume(float volume) {
    mVolumeRamp->setTarget(volume);
}

void AAudioFlowGraph::setRampLengthInFrames(int32_t numFrames) {
    mVolumeRamp->setLengthInFrames(numFrames);
}
