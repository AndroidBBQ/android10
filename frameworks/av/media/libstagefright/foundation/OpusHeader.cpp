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

//#define LOG_NDEBUG 0
#define LOG_TAG "OpusHeader"
#include <cstring>
#include <inttypes.h>
#include <stdint.h>

#include <log/log.h>

#include "OpusHeader.h"

namespace android {

// Opus uses Vorbis channel mapping, and Vorbis channel mapping specifies
// mappings for up to 8 channels. This information is part of the Vorbis I
// Specification:
// http://www.xiph.org/vorbis/doc/Vorbis_I_spec.html
constexpr int kMaxChannels = 8;

constexpr uint8_t kOpusChannelMap[kMaxChannels][kMaxChannels] = {
        {0},
        {0, 1},
        {0, 2, 1},
        {0, 1, 2, 3},
        {0, 4, 1, 2, 3},
        {0, 4, 1, 2, 3, 5},
        {0, 4, 1, 2, 3, 5, 6},
        {0, 6, 1, 2, 3, 4, 5, 7},
};

// Size of the Opus header excluding optional mapping information.
constexpr size_t kOpusHeaderSize = 19;
// Offset to magic string that starts Opus header.
constexpr size_t kOpusHeaderLabelOffset = 0;
// Offset to Opus version in the Opus header.
constexpr size_t kOpusHeaderVersionOffset = 8;
// Offset to the channel count byte in the Opus header.
constexpr size_t kOpusHeaderChannelsOffset = 9;
// Offset to the pre-skip value in the Opus header.
constexpr size_t kOpusHeaderSkipSamplesOffset = 10;
// Offset to sample rate in the Opus header.
constexpr size_t kOpusHeaderSampleRateOffset = 12;
// Offset to the gain value in the Opus header.
constexpr size_t kOpusHeaderGainOffset = 16;
// Offset to the channel mapping byte in the Opus header.
constexpr size_t kOpusHeaderChannelMappingOffset = 18;
// Opus Header contains a stream map. The mapping values are in the header
// beyond the always present |kOpusHeaderSize| bytes of data. The mapping
// data contains stream count, coupling information, and per channel mapping
// values:
//   - Byte 0: Number of streams.
//   - Byte 1: Number coupled.
//   - Byte 2: Starting at byte 2 are |header->channels| uint8 mapping
//             values.
// Offset to the number of streams in the Opus header.
constexpr size_t kOpusHeaderNumStreamsOffset = 19;
// Offset to the number of streams that are coupled in the Opus header.
constexpr size_t kOpusHeaderNumCoupledStreamsOffset = 20;
// Offset to the stream to channel mapping in the Opus header.
constexpr size_t kOpusHeaderStreamMapOffset = 21;

// Default audio output channel layout. Used to initialize |stream_map| in
// OpusHeader, and passed to opus_multistream_decoder_create() when the header
// does not contain mapping information. The values are valid only for mono and
// stereo output: Opus streams with more than 2 channels require a stream map.
constexpr int kMaxChannelsWithDefaultLayout = 2;

static uint16_t ReadLE16(const uint8_t* data, size_t data_size, uint32_t read_offset) {
    // check whether the 2nd byte is within the buffer
    if (read_offset + 1 >= data_size) return 0;
    uint16_t val;
    val = data[read_offset];
    val |= data[read_offset + 1] << 8;
    return val;
}

// Parses Opus Header. Header spec: http://wiki.xiph.org/OggOpus#ID_Header
bool ParseOpusHeader(const uint8_t* data, size_t data_size, OpusHeader* header) {
    if (data == NULL) {
        return false;
    }
    if (data_size < kOpusHeaderSize) {
        ALOGV("Header size is too small.");
        return false;
    }
    header->channels = data[kOpusHeaderChannelsOffset];

    if (header->channels < 1 || header->channels > kMaxChannels) {
        ALOGV("Invalid Header, bad channel count: %d", header->channels);
        return false;
    }
    header->skip_samples = ReadLE16(data, data_size, kOpusHeaderSkipSamplesOffset);
    header->gain_db = static_cast<int16_t>(ReadLE16(data, data_size, kOpusHeaderGainOffset));
    header->channel_mapping = data[kOpusHeaderChannelMappingOffset];
    if (!header->channel_mapping) {
        if (header->channels > kMaxChannelsWithDefaultLayout) {
            ALOGV("Invalid Header, missing stream map.");
            return false;
        }
        header->num_streams = 1;
        header->num_coupled = header->channels > 1;
        header->stream_map[0] = 0;
        header->stream_map[1] = 1;
        return true;
    }
    if (data_size < kOpusHeaderStreamMapOffset + header->channels) {
        ALOGV("Invalid stream map; insufficient data for current channel "
              "count: %d",
              header->channels);
        return false;
    }
    header->num_streams = data[kOpusHeaderNumStreamsOffset];
    header->num_coupled = data[kOpusHeaderNumCoupledStreamsOffset];
    if (header->num_streams + header->num_coupled != header->channels) {
        ALOGV("Inconsistent channel mapping.");
        return false;
    }
    for (int i = 0; i < header->channels; ++i)
        header->stream_map[i] = data[kOpusHeaderStreamMapOffset + i];
    return true;
}

int WriteOpusHeader(const OpusHeader &header, int input_sample_rate,
                    uint8_t* output, size_t output_size) {
    // See https://wiki.xiph.org/OggOpus#ID_Header.
    const size_t total_size = kOpusHeaderStreamMapOffset + header.channels;
    if (output_size < total_size) {
        ALOGE("Output buffer too small for header.");
        return -1;
    }

    // ensure entire header is cleared, even though we overwrite much of it below
    memset(output, 0, output_size);

    // Set magic signature.
    memcpy(output + kOpusHeaderLabelOffset, "OpusHead", 8);
    // Set Opus version.
    output[kOpusHeaderVersionOffset] = 1;
    // Set channel count.
    output[kOpusHeaderChannelsOffset] = (uint8_t)header.channels;
    // Set pre-skip
    memcpy(output + kOpusHeaderSkipSamplesOffset, &header.skip_samples, sizeof(uint16_t));
    // Set original input sample rate in Hz.
    memcpy(output + kOpusHeaderSampleRateOffset, &input_sample_rate, sizeof(uint32_t));
    // Set output gain in dB.
    memcpy(output + kOpusHeaderGainOffset, &header.gain_db, sizeof(uint16_t));

    if (header.channels > 2) {
        // Set channel mapping
        output[kOpusHeaderChannelMappingOffset] = 1;
        // Assuming no coupled streams. This should actually be
        // channels() - |coupled_streams|.
        output[kOpusHeaderNumStreamsOffset] = header.channels;
        output[kOpusHeaderNumCoupledStreamsOffset] = 0;

        // Set the actual stream map.
        for (int i = 0; i < header.channels; ++i) {
            output[kOpusHeaderStreamMapOffset + i] = kOpusChannelMap[header.channels - 1][i];
        }
        return kOpusHeaderStreamMapOffset + header.channels + 1;
    } else {
        output[kOpusHeaderChannelMappingOffset] = 0;
        return kOpusHeaderChannelMappingOffset + 1;
    }
}

int WriteOpusHeaders(const OpusHeader &header, int inputSampleRate,
                     uint8_t* output, size_t outputSize, uint64_t codecDelay,
                     uint64_t seekPreRoll) {
    if (outputSize < AOPUS_UNIFIED_CSD_MINSIZE) {
        ALOGD("Buffer not large enough to hold unified OPUS CSD");
        return -1;
    }
    int headerLen = 0;

    // Add opus header
    /*
      Following is the CSD syntax for signalling OpusHeader
      (http://wiki.xiph.org/OggOpus#ID_Header)

      Marker (8 bytes) | Length (8 bytes) | OpusHeader

      Markers supported:
      AOPUS_CSD_OPUS_HEADER_MARKER - Signals Opus Header

      Length should be a value within AOPUS_OPUSHEAD_MINSIZE and AOPUS_OPUSHEAD_MAXSIZE.
    */

    memcpy(output + headerLen, AOPUS_CSD_OPUS_HEADER_MARKER, AOPUS_MARKER_SIZE);
    headerLen += AOPUS_MARKER_SIZE;

    // Place holder for opusHeader Size
    headerLen += AOPUS_LENGTH_SIZE;

    int headerSize = WriteOpusHeader(header, inputSampleRate, output + headerLen,
        outputSize - headerLen);
    if (headerSize < 0) {
        ALOGD("%s: WriteOpusHeader failed", __func__);
        return -1;
    }
    headerLen += headerSize;

    // Update opus headerSize after AOPUS_CSD_OPUS_HEADER_MARKER
    uint64_t length = headerSize;
    memcpy(output + AOPUS_MARKER_SIZE, &length, AOPUS_LENGTH_SIZE);

    /*
      Following is the CSD syntax for signalling codec delay and
      seek pre-roll which is to be appended after OpusHeader

      Marker (8 bytes) | Length (8 bytes) | Samples in ns (8 bytes)

      Markers supported:
      AOPUS_CSD_CODEC_DELAY_MARKER - codec delay as samples in ns, represented in 8 bytes
      AOPUS_CSD_SEEK_PREROLL_MARKER - preroll adjustment as samples in ns, represented in 8 bytes

    */
    length = sizeof(codecDelay);
    if (headerLen > (outputSize - AOPUS_MARKER_SIZE - AOPUS_LENGTH_SIZE - length)) {
        ALOGD("Buffer not large enough to hold codec delay");
        return -1;
    }
    // Add codec delay
    memcpy(output + headerLen, AOPUS_CSD_CODEC_DELAY_MARKER, AOPUS_MARKER_SIZE);
    headerLen += AOPUS_MARKER_SIZE;
    memcpy(output + headerLen, &length, AOPUS_LENGTH_SIZE);
    headerLen += AOPUS_LENGTH_SIZE;
    memcpy(output + headerLen, &codecDelay, length);
    headerLen += length;

    length = sizeof(seekPreRoll);
    if (headerLen > (outputSize - AOPUS_MARKER_SIZE - AOPUS_LENGTH_SIZE - length)) {
        ALOGD("Buffer not large enough to hold seek pre roll");
        return -1;
    }
    // Add skip pre roll
    memcpy(output + headerLen, AOPUS_CSD_SEEK_PREROLL_MARKER, AOPUS_MARKER_SIZE);
    headerLen += AOPUS_MARKER_SIZE;
    memcpy(output + headerLen, &length, AOPUS_LENGTH_SIZE);
    headerLen += AOPUS_LENGTH_SIZE;
    memcpy(output + headerLen, &seekPreRoll, length);
    headerLen += length;

    return headerLen;
}

bool IsOpusHeader(const uint8_t *data, size_t data_size) {
    if (data_size < AOPUS_MARKER_SIZE) {
        return false;
    }

    return !memcmp(data, AOPUS_CSD_OPUS_HEADER_MARKER, AOPUS_MARKER_SIZE);
}

bool GetOpusHeaderBuffers(const uint8_t *data, size_t data_size,
                          void **opusHeadBuf, size_t *opusHeadSize,
                          void **codecDelayBuf, size_t *codecDelaySize,
                          void **seekPreRollBuf, size_t *seekPreRollSize) {
    *codecDelayBuf = NULL;
    *codecDelaySize = 0;
    *seekPreRollBuf = NULL;
    *seekPreRollSize = 0;
    *opusHeadBuf = NULL;
    *opusHeadSize = 0;

    // AOPUS_MARKER_SIZE is 8 "OpusHead" is of size 8
    if (data_size < 8)
        return false;

    // Check if the CSD is in legacy format
    if (!memcmp("OpusHead", data, 8)) {
        if (data_size < AOPUS_OPUSHEAD_MINSIZE || data_size > AOPUS_OPUSHEAD_MAXSIZE) {
            ALOGD("Unexpected size for opusHeadSize %zu", data_size);
            return false;
        }
        *opusHeadBuf = (void *)data;
        *opusHeadSize = data_size;
        return true;
    } else if (memcmp(AOPUS_CSD_MARKER_PREFIX, data, AOPUS_CSD_MARKER_PREFIX_SIZE) == 0) {
        if (data_size < AOPUS_UNIFIED_CSD_MINSIZE || data_size > AOPUS_UNIFIED_CSD_MAXSIZE) {
            ALOGD("Unexpected size for unified opus csd %zu", data_size);
            return false;
        }
        size_t i = 0;
        bool found = false;
        while (i <= data_size - AOPUS_MARKER_SIZE - AOPUS_LENGTH_SIZE) {
            uint8_t *csdBuf = (uint8_t *)data + i;
            if (!memcmp(csdBuf, AOPUS_CSD_OPUS_HEADER_MARKER, AOPUS_MARKER_SIZE)) {
                uint64_t value;
                memcpy(&value, csdBuf + AOPUS_MARKER_SIZE, sizeof(value));
                if (value < AOPUS_OPUSHEAD_MINSIZE || value > AOPUS_OPUSHEAD_MAXSIZE) {
                    ALOGD("Unexpected size for opusHeadSize %" PRIu64, value);
                    return false;
                }
                i += AOPUS_MARKER_SIZE + AOPUS_LENGTH_SIZE + value;
                if (i > data_size) {
                    ALOGD("Marker signals a header that is larger than input");
                    return false;
                }
                *opusHeadBuf = csdBuf + AOPUS_MARKER_SIZE + AOPUS_LENGTH_SIZE;
                *opusHeadSize = value;
                found = true;
            } else if (!memcmp(csdBuf, AOPUS_CSD_CODEC_DELAY_MARKER, AOPUS_MARKER_SIZE)) {
                uint64_t value;
                memcpy(&value, csdBuf + AOPUS_MARKER_SIZE, sizeof(value));
                if (value != sizeof(uint64_t)) {
                    ALOGD("Unexpected size for codecDelay %" PRIu64, value);
                    return false;
                }
                i += AOPUS_MARKER_SIZE + AOPUS_LENGTH_SIZE + value;
                if (i > data_size) {
                    ALOGD("Marker signals a header that is larger than input");
                    return false;
                }
                *codecDelayBuf = csdBuf + AOPUS_MARKER_SIZE + AOPUS_LENGTH_SIZE;
                *codecDelaySize = value;
            } else if (!memcmp(csdBuf, AOPUS_CSD_SEEK_PREROLL_MARKER, AOPUS_MARKER_SIZE)) {
                uint64_t value;
                memcpy(&value, csdBuf + AOPUS_MARKER_SIZE, sizeof(value));
                if (value != sizeof(uint64_t)) {
                    ALOGD("Unexpected size for seekPreRollSize %" PRIu64, value);
                    return false;
                }
                i += AOPUS_MARKER_SIZE + AOPUS_LENGTH_SIZE + value;
                if (i > data_size) {
                    ALOGD("Marker signals a header that is larger than input");
                    return false;
                }
                *seekPreRollBuf = csdBuf + AOPUS_MARKER_SIZE + AOPUS_LENGTH_SIZE;
                *seekPreRollSize = value;
            } else {
                i++;
            }
        }
        return found;
    } else {
        return false;  // it isn't in either format
    }
}

}  // namespace android
