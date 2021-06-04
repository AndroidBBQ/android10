/*
 * Copyright (C) 2015 The Android Open Source Project
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

#define LOG_TAG "APM::AudioRoute"
//#define LOG_NDEBUG 0

#include "AudioRoute.h"
#include "HwModule.h"
#include "AudioGain.h"

namespace android
{

void AudioRoute::dump(String8 *dst, int spaces) const
{
    dst->appendFormat("%*s- Type: %s\n", spaces, "", mType == AUDIO_ROUTE_MUX ? "Mux" : "Mix");
    dst->appendFormat("%*s- Sink: %s\n", spaces, "", mSink->getTagName().string());
    if (mSources.size() != 0) {
        dst->appendFormat("%*s- Sources: \n", spaces, "");
        for (size_t i = 0; i < mSources.size(); i++) {
            dst->appendFormat("%*s%s \n", spaces + 4, "", mSources[i]->getTagName().string());
        }
    }
    dst->append("\n");
}

bool AudioRoute::supportsPatch(const sp<AudioPort> &srcPort, const sp<AudioPort> &dstPort) const
{
    if (mSink == 0 || dstPort == 0 || dstPort != mSink) {
        return false;
    }
    ALOGV("%s: sinks %s matching", __FUNCTION__, mSink->getTagName().string());
    for (const auto &sourcePort : mSources) {
        if (sourcePort == srcPort) {
            ALOGV("%s: sources %s matching", __FUNCTION__, sourcePort->getTagName().string());
            return true;
        }
    }
    return false;
}

}
