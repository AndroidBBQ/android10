/*
 * Copyright (C) 2010 The Android Open Source Project
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

#ifndef AVC_UTILS_H_

#define AVC_UTILS_H_

#include <media/stagefright/foundation/ABuffer.h>
#include <utils/Errors.h>

namespace android {

class ABitReader;

enum {
    kAVCProfileBaseline      = 0x42,
    kAVCProfileMain          = 0x4d,
    kAVCProfileExtended      = 0x58,
    kAVCProfileHigh          = 0x64,
    kAVCProfileHigh10        = 0x6e,
    kAVCProfileHigh422       = 0x7a,
    kAVCProfileHigh444       = 0xf4,
    kAVCProfileCAVLC444Intra = 0x2c
};

struct NALPosition {
    uint32_t nalOffset;
    uint32_t nalSize;
};

// Optionally returns sample aspect ratio as well.
void FindAVCDimensions(
        const sp<ABuffer> &seqParamSet,
        int32_t *width, int32_t *height,
        int32_t *sarWidth = NULL, int32_t *sarHeight = NULL);

// Gets and returns an unsigned exp-golomb (ue) value from a bit reader |br|. Aborts if the value
// is more than 64 bits long (>=0xFFFF (!)) or the bit reader overflows.
unsigned parseUE(ABitReader *br);

// Gets and returns a signed exp-golomb (se) value from a bit reader |br|. Aborts if the value is
// more than 64 bits long (>0x7FFF || <-0x7FFF (!)) or the bit reader overflows.
signed parseSE(ABitReader *br);

// Gets an unsigned exp-golomb (ue) value from a bit reader |br|, and returns it if it was
// successful. Returns |fallback| if it was unsuccessful. Note: if the value was longer that 64
// bits, it reads past the value and still returns |fallback|.
unsigned parseUEWithFallback(ABitReader *br, unsigned fallback);

// Gets a signed exp-golomb (se) value from a bit reader |br|, and returns it if it was successful.
// Returns |fallback| if it was unsuccessful. Note: if the value was longer that 64 bits, it reads
// past the value and still returns |fallback|.
signed parseSEWithFallback(ABitReader *br, signed fallback);

// Skips an unsigned exp-golomb (ue) value from bit reader |br|.
inline void skipUE(ABitReader *br) {
    (void)parseUEWithFallback(br, 0U);
}

// Skips a signed exp-golomb (se) value from bit reader |br|.
inline void skipSE(ABitReader *br) {
    (void)parseSEWithFallback(br, 0);
}

status_t getNextNALUnit(
        const uint8_t **_data, size_t *_size,
        const uint8_t **nalStart, size_t *nalSize,
        bool startCodeFollows = false);

sp<ABuffer> MakeAVCCodecSpecificData(
        const sp<ABuffer> &accessUnit, int32_t *width, int32_t *height,
        int32_t *sarWidth = nullptr, int32_t *sarHeight = nullptr);

bool IsIDR(const uint8_t *data, size_t size);
bool IsAVCReferenceFrame(const sp<ABuffer> &accessUnit);
uint32_t FindAVCLayerId(const uint8_t *data, size_t size);

const char *AVCProfileToString(uint8_t profile);

// Given an MPEG4 video VOL-header chunk (starting with 0x00 0x00 0x01 0x2?)
// parse it and fill in dimensions, returns true iff successful.
bool ExtractDimensionsFromVOLHeader(
        const uint8_t *data, size_t size, int32_t *width, int32_t *height);

bool GetMPEGAudioFrameSize(
        uint32_t header, size_t *frame_size,
        int *out_sampling_rate = NULL, int *out_channels = NULL,
        int *out_bitrate = NULL, int *out_num_samples = NULL);

}  // namespace android

#endif  // AVC_UTILS_H_
