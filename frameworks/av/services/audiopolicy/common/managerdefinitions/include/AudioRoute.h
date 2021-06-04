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

#pragma once

#include "AudioCollections.h"
#include <utils/String8.h>
#include <utils/Vector.h>
#include <utils/RefBase.h>
#include <utils/Errors.h>

namespace android
{

class AudioPort;
class DeviceDescriptor;

typedef enum {
    AUDIO_ROUTE_MUX = 0,
    AUDIO_ROUTE_MIX = 1
} audio_route_type_t;

class AudioRoute  : public virtual RefBase
{
public:
    explicit AudioRoute(audio_route_type_t type) : mType(type) {}

    void setSources(const AudioPortVector &sources) { mSources = sources; }
    const AudioPortVector &getSources() const { return mSources; }

    void setSink(const sp<AudioPort> &sink) { mSink = sink; }
    const sp<AudioPort> &getSink() const { return mSink; }

    audio_route_type_t getType() const { return mType; }

    /**
     * @brief supportsPatch checks if an audio patch is supported by a Route declared in
     * the audio_policy_configuration.xml file.
     * If the patch is supported natively by an AudioHAL (which supports of course Routing API 3.0),
     * audiopolicy will not request AudioFlinger to use a software bridge to realize a patch
     * between 2 ports.
     * @param srcPort (aka the source) to be considered
     * @param dstPort (aka the sink) to be considered
     * @return true if the audio route supports the connection between the sink and the source,
     * false otherwise
     */
    bool supportsPatch(const sp<AudioPort> &srcPort, const sp<AudioPort> &dstPort) const;

    void dump(String8 *dst, int spaces) const;

private:
    AudioPortVector mSources;
    sp<AudioPort> mSink;
    audio_route_type_t mType;

};

} // namespace android
