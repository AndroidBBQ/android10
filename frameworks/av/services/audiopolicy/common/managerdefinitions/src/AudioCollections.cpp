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

#define LOG_TAG "APM::AudioCollections"
//#define LOG_NDEBUG 0

#include "AudioCollections.h"
#include "AudioPort.h"
#include "AudioRoute.h"
#include "HwModule.h"
#include "AudioGain.h"

namespace android {

sp<AudioPort> AudioPortVector::findByTagName(const String8 &tagName) const
{
    for (const auto& port : *this) {
        if (port->getTagName() == tagName) {
            return port;
        }
    }
    return nullptr;
}

void AudioRouteVector::dump(String8 *dst, int spaces) const
{
    if (isEmpty()) {
        return;
    }
    dst->appendFormat("\n%*sAudio Routes (%zu):\n", spaces, "", size());
    for (size_t i = 0; i < size(); i++) {
        dst->appendFormat("%*s- Route %zu:\n", spaces, "", i + 1);
        itemAt(i)->dump(dst, 4);
    }
}

} // namespace android
