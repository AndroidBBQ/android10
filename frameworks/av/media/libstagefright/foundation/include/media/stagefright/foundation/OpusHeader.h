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

/*
 * The Opus specification is part of IETF RFC 6716:
 * http://tools.ietf.org/html/rfc6716
 */

#ifndef OPUS_HEADER_H_
#define OPUS_HEADER_H_

namespace android {

/* Constants used for delimiting Opus CSD */
#define AOPUS_CSD_MARKER_PREFIX "AOPUS"
#define AOPUS_CSD_MARKER_PREFIX_SIZE (sizeof(AOPUS_CSD_MARKER_PREFIX) - 1)
#define AOPUS_CSD_OPUS_HEADER_MARKER AOPUS_CSD_MARKER_PREFIX "HDR"
#define AOPUS_CSD_CODEC_DELAY_MARKER AOPUS_CSD_MARKER_PREFIX "DLY"
#define AOPUS_CSD_SEEK_PREROLL_MARKER AOPUS_CSD_MARKER_PREFIX "PRL"
#define AOPUS_MARKER_SIZE 8
#define AOPUS_LENGTH_SIZE sizeof(uint64_t)
#define AOPUS_CSD_CODEC_DELAY_SIZE \
     (AOPUS_MARKER_SIZE) + (AOPUS_LENGTH_SIZE) + sizeof(uint64_t)
#define AOPUS_CSD_SEEK_PREROLL_SIZE \
     (AOPUS_MARKER_SIZE) + (AOPUS_LENGTH_SIZE) + sizeof(uint64_t)

/* OpusHead csd minimum size is 19 */
#define AOPUS_OPUSHEAD_MINSIZE 19
#define AOPUS_CSD_OPUSHEAD_MINSIZE \
    (AOPUS_MARKER_SIZE) + (AOPUS_LENGTH_SIZE) + (AOPUS_OPUSHEAD_MINSIZE)

#define AOPUS_UNIFIED_CSD_MINSIZE \
    ((AOPUS_CSD_OPUSHEAD_MINSIZE) + \
     (AOPUS_CSD_CODEC_DELAY_SIZE) + \
     (AOPUS_CSD_SEEK_PREROLL_SIZE))

/* OpusHead csd at max can be AOPUS_CSD_OPUSHEAD_MINSIZE + 2 + max number of channels (255) */
#define AOPUS_OPUSHEAD_MAXSIZE ((AOPUS_OPUSHEAD_MINSIZE) + 2 + 255)
#define AOPUS_CSD_OPUSHEAD_MAXSIZE \
    (AOPUS_MARKER_SIZE) + (AOPUS_LENGTH_SIZE) + (AOPUS_OPUSHEAD_MAXSIZE)

#define AOPUS_UNIFIED_CSD_MAXSIZE \
    ((AOPUS_CSD_OPUSHEAD_MAXSIZE) + \
     (AOPUS_CSD_CODEC_DELAY_SIZE) + \
     (AOPUS_CSD_SEEK_PREROLL_SIZE))

struct OpusHeader {
    int channels;
    int channel_mapping;
    int num_streams;
    int num_coupled;
    int16_t gain_db;
    int skip_samples;
    uint8_t stream_map[8];
};

bool ParseOpusHeader(const uint8_t* data, size_t data_size, OpusHeader* header);
int WriteOpusHeader(const OpusHeader &header, int input_sample_rate, uint8_t* output, size_t output_size);
bool GetOpusHeaderBuffers(const uint8_t *data, size_t data_size,
                          void **opusHeadBuf, size_t *opusHeadSize,
                          void **codecDelayBuf, size_t *codecDelaySize,
                          void **seekPreRollBuf, size_t *seekPreRollSize);
int WriteOpusHeaders(const OpusHeader &header, int inputSampleRate,
                     uint8_t* output, size_t outputSize, uint64_t codecDelay,
                     uint64_t seekPreRoll);
bool IsOpusHeader(const uint8_t *data, size_t data_size);
}  // namespace android

#endif  // OPUS_HEADER_H_
