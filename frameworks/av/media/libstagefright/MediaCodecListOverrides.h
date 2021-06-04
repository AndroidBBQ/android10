/*
 * Copyright 2015 The Android Open Source Project
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

#ifndef MEDIA_CODEC_LIST_OVERRIDES_H_

#define MEDIA_CODEC_LIST_OVERRIDES_H_

#include <media/MediaCodecInfo.h>
#include <media/stagefright/foundation/AString.h>

#include <utils/StrongPointer.h>
#include <utils/KeyedVector.h>
#include <vector>

namespace android {

struct MediaCodecInfo;

AString getProfilingVersionString();

bool splitString(const AString &s, const AString &delimiter, AString *s1, AString *s2);

// profile codecs and save the result to xml file named kProfilingResults.
void profileCodecs(const std::vector<sp<MediaCodecInfo>> &infos,
        const char* profilingResults);

// profile codecs and save the result to global_results, encoder_results and decoder_results.
void profileCodecs(
        const std::vector<sp<MediaCodecInfo>> &infos,
        CodecSettings *global_results,
        KeyedVector<AString, CodecSettings> *encoder_results,
        KeyedVector<AString, CodecSettings> *decoder_results,
        bool forceToMeasure = false);

void exportResultsToXML(
        const char *fileName,
        const CodecSettings& global_results,
        const KeyedVector<AString, CodecSettings>& encoder_results,
        const KeyedVector<AString, CodecSettings>& decoder_results);

}  // namespace android

#endif  // MEDIA_CODEC_LIST_OVERRIDES_H_
