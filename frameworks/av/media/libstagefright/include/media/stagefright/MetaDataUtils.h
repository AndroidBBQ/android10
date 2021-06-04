/*
 * Copyright 2018 The Android Open Source Project
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

#ifndef META_DATA_UTILS_H_

#define META_DATA_UTILS_H_

#include <media/stagefright/MetaData.h>

struct AMediaFormat;

namespace android {

struct ABuffer;
bool MakeAVCCodecSpecificData(MetaDataBase &meta, const uint8_t *data, size_t size);
bool MakeAVCCodecSpecificData(AMediaFormat *meta, const uint8_t *data, size_t size);

bool MakeAACCodecSpecificData(MetaDataBase &meta, const uint8_t *data, size_t size);
bool MakeAACCodecSpecificData(MetaDataBase &meta, unsigned profile, unsigned sampling_freq_index,
        unsigned channel_configuration);

bool MakeAACCodecSpecificData(AMediaFormat *meta, unsigned profile, unsigned sampling_freq_index,
        unsigned channel_configuration);

void parseVorbisComment(
        AMediaFormat *fileMeta, const char *comment, size_t commentLength);

}  // namespace android

#endif  // META_DATA_UTILS_H_
