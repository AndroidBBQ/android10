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
#define LOG_TAG "AC4Parser"

#include <inttypes.h>
#include <utils/Log.h>
#include <utils/misc.h>

#include "AC4Parser.h"

#define BOOLSTR(a)  ((a)?"true":"false")
#define BYTE_ALIGN mBitReader.skipBits(mBitReader.numBitsLeft() % 8)
#define CHECK_BITS_LEFT(n) if (mBitReader.numBitsLeft() < n) {return false;}

namespace android {

AC4Parser::AC4Parser() {
}

AC4DSIParser::AC4DSIParser(ABitReader &br)
    : mBitReader(br){

    mDSISize = mBitReader.numBitsLeft();
}

// ETSI TS 103 190-2 V1.1.1 (2015-09) Table 79: channel_mode
static const char *ChannelModes[] = {
    "mono",
    "stereo",
    "3.0",
    "5.0",
    "5.1",
    "7.0 (3/4/0)",
    "7.1 (3/4/0.1)",
    "7.0 (5/2/0)",
    "7.1 (5/2/0.1)",
    "7.0 (3/2/2)",
    "7.1 (3/2/2.1)",
    "7.0.4",
    "7.1.4",
    "9.0.4",
    "9.1.4",
    "22.2"
};

static const char* ContentClassifier[] = {
    "Complete Main",
    "Music and Effects",
    "Visually Impaired",
    "Hearing Impaired",
    "Dialog",
    "Commentary",
    "Emergency",
    "Voice Over"
};

bool AC4DSIParser::parseLanguageTag(uint32_t presentationID, uint32_t substreamID){
    CHECK_BITS_LEFT(6);
    uint32_t n_language_tag_bytes = mBitReader.getBits(6);
    if (n_language_tag_bytes < 2 || n_language_tag_bytes >= 42) {
        return false;
    }
    CHECK_BITS_LEFT(n_language_tag_bytes * 8);
    char language_tag_bytes[42]; // TS 103 190 part 1 4.3.3.8.7
    for (uint32_t i = 0; i < n_language_tag_bytes; i++) {
        language_tag_bytes[i] = (char)mBitReader.getBits(8);
    }
    language_tag_bytes[n_language_tag_bytes] = 0;
    ALOGV("%u.%u: language_tag = %s\n", presentationID, substreamID, language_tag_bytes);

    std::string language(language_tag_bytes, n_language_tag_bytes);
    mPresentations[presentationID].mLanguage = language;

    return true;
}

// TS 103 190-1 v1.2.1 E.5 and TS 103 190-2 v1.1.1 E.9
bool AC4DSIParser::parseSubstreamDSI(uint32_t presentationID, uint32_t substreamID){
    CHECK_BITS_LEFT(5);
    uint32_t channel_mode = mBitReader.getBits(5);
    CHECK_BITS_LEFT(2);
    uint32_t dsi_sf_multiplier = mBitReader.getBits(2);
    CHECK_BITS_LEFT(1);
    bool b_substream_bitrate_indicator = (mBitReader.getBits(1) == 1);
    ALOGV("%u.%u: channel_mode = %u (%s)\n", presentationID, substreamID, channel_mode,
    channel_mode < NELEM(ChannelModes) ? ChannelModes[channel_mode] : "reserved");
    ALOGV("%u.%u: dsi_sf_multiplier = %u\n", presentationID,
        substreamID, dsi_sf_multiplier);
    ALOGV("%u.%u: b_substream_bitrate_indicator = %s\n", presentationID,
        substreamID, BOOLSTR(b_substream_bitrate_indicator));

    if (b_substream_bitrate_indicator) {
        CHECK_BITS_LEFT(5);
        uint32_t substream_bitrate_indicator = mBitReader.getBits(5);
        ALOGV("%u.%u: substream_bitrate_indicator = %u\n", presentationID, substreamID,
            substream_bitrate_indicator);
    }
    if (channel_mode >= 7 && channel_mode <= 10) {
        CHECK_BITS_LEFT(1);
        uint32_t add_ch_base = mBitReader.getBits(1);
        ALOGV("%u.%u: add_ch_base = %u\n", presentationID, substreamID, add_ch_base);
    }
    CHECK_BITS_LEFT(1);
    bool b_content_type = (mBitReader.getBits(1) == 1);
    ALOGV("%u.%u: b_content_type = %s\n", presentationID, substreamID, BOOLSTR(b_content_type));
    if (b_content_type) {
        CHECK_BITS_LEFT(3);
        uint32_t content_classifier = mBitReader.getBits(3);
        ALOGV("%u.%u: content_classifier = %u (%s)\n", presentationID, substreamID,
            content_classifier, ContentClassifier[content_classifier]);

        // For streams based on TS 103 190 part 1 the presentation level channel_mode doesn't
        // exist and so we use the channel_mode from either the CM or M&E substream
        // (they are mutually exclusive)
        if (mPresentations[presentationID].mChannelMode == -1 &&
            (content_classifier == 0 || content_classifier == 1)) {
            mPresentations[presentationID].mChannelMode = channel_mode;
        }
        mPresentations[presentationID].mContentClassifier = content_classifier;
        CHECK_BITS_LEFT(1);
        bool b_language_indicator = (mBitReader.getBits(1) == 1);
        ALOGV("%u.%u: b_language_indicator = %s\n", presentationID, substreamID,
            BOOLSTR(b_language_indicator));
        if (b_language_indicator) {
            if (!parseLanguageTag(presentationID, substreamID)) {
                return false;
            }
        }
    }

    return true;
}

// ETSI TS 103 190-2 v1.1.1 section E.11
bool AC4DSIParser::parseSubstreamGroupDSI(uint32_t presentationID, uint32_t groupID)
{
    CHECK_BITS_LEFT(1);
    bool b_substreams_present = (mBitReader.getBits(1) == 1);
    CHECK_BITS_LEFT(1);
    bool b_hsf_ext = (mBitReader.getBits(1) == 1);
    CHECK_BITS_LEFT(1);
    bool b_channel_coded = (mBitReader.getBits(1) == 1);
    CHECK_BITS_LEFT(8);
    uint32_t n_substreams = mBitReader.getBits(8);
    ALOGV("%u.%u: b_substreams_present = %s\n", presentationID, groupID,
        BOOLSTR(b_substreams_present));
    ALOGV("%u.%u: b_hsf_ext = %s\n", presentationID, groupID, BOOLSTR(b_hsf_ext));
    ALOGV("%u.%u: b_channel_coded = %s\n", presentationID, groupID, BOOLSTR(b_channel_coded));
    ALOGV("%u.%u: n_substreams = %u\n", presentationID, groupID, n_substreams);

    for (uint32_t i = 0; i < n_substreams; i++) {
        CHECK_BITS_LEFT(2);
        uint32_t dsi_sf_multiplier = mBitReader.getBits(2);
        CHECK_BITS_LEFT(1);
        bool b_substream_bitrate_indicator = (mBitReader.getBits(1) == 1);
        ALOGV("%u.%u.%u: dsi_sf_multiplier = %u\n", presentationID, groupID, i, dsi_sf_multiplier);
        ALOGV("%u.%u.%u: b_substream_bitrate_indicator = %s\n", presentationID, groupID, i,
            BOOLSTR(b_substream_bitrate_indicator));

        if (b_substream_bitrate_indicator) {
            CHECK_BITS_LEFT(5);
            uint32_t substream_bitrate_indicator = mBitReader.getBits(5);
            ALOGV("%u.%u.%u: substream_bitrate_indicator = %u\n", presentationID, groupID, i,
                substream_bitrate_indicator);
        }
        if (b_channel_coded) {
            CHECK_BITS_LEFT(24);
            uint32_t dsi_substream_channel_mask = mBitReader.getBits(24);
            ALOGV("%u.%u.%u: dsi_substream_channel_mask = 0x%06x\n", presentationID, groupID, i,
                dsi_substream_channel_mask);
        } else {
            CHECK_BITS_LEFT(1);
            bool b_ajoc = (mBitReader.getBits(1) == 1);
            ALOGV("%u.%u.%u: b_ajoc = %s\n", presentationID, groupID, i, BOOLSTR(b_ajoc));
            if (b_ajoc) {
                CHECK_BITS_LEFT(1);
                bool b_static_dmx = (mBitReader.getBits(1) == 1);
                ALOGV("%u.%u.%u: b_static_dmx = %s\n", presentationID, groupID, i,
                    BOOLSTR(b_static_dmx));
                if (!b_static_dmx) {
                    CHECK_BITS_LEFT(4);
                    uint32_t n_dmx_objects_minus1 = mBitReader.getBits(4);
                    ALOGV("%u.%u.%u: n_dmx_objects_minus1 = %u\n", presentationID, groupID, i,
                        n_dmx_objects_minus1);
                }
                CHECK_BITS_LEFT(6);
                uint32_t n_umx_objects_minus1 = mBitReader.getBits(6);
                ALOGV("%u.%u.%u: n_umx_objects_minus1 = %u\n", presentationID, groupID, i,
                    n_umx_objects_minus1);
            }
            CHECK_BITS_LEFT(4);
            mBitReader.skipBits(4); // objects_assignment_mask
        }
    }

    CHECK_BITS_LEFT(1);
    bool b_content_type = (mBitReader.getBits(1) == 1);
    ALOGV("%u.%u: b_content_type = %s\n", presentationID, groupID, BOOLSTR(b_content_type));
    if (b_content_type) {
        CHECK_BITS_LEFT(3);
        uint32_t content_classifier = mBitReader.getBits(3);
        ALOGV("%u.%u: content_classifier = %s (%u)\n", presentationID, groupID,
            ContentClassifier[content_classifier], content_classifier);

        mPresentations[presentationID].mContentClassifier = content_classifier;

        CHECK_BITS_LEFT(1);
        bool b_language_indicator = (mBitReader.getBits(1) == 1);
        ALOGV("%u.%u: b_language_indicator = %s\n", presentationID, groupID,
            BOOLSTR(b_language_indicator));

        if (b_language_indicator) {
            if (!parseLanguageTag(presentationID, groupID)) {
                return false;
            }
        }
    }

    return true;
}

bool AC4DSIParser::parseBitrateDsi() {
    CHECK_BITS_LEFT(2 + 32 + 32);
    mBitReader.skipBits(2); // bit_rate_mode
    mBitReader.skipBits(32); // bit_rate
    mBitReader.skipBits(32); // bit_rate_precision

    return true;
}

// TS 103 190-1 section E.4 (ac4_dsi) and TS 103 190-2 section E.6 (ac4_dsi_v1)
bool AC4DSIParser::parse() {
    CHECK_BITS_LEFT(3);
    uint32_t ac4_dsi_version = mBitReader.getBits(3);
    if (ac4_dsi_version > 1) {
        ALOGE("error while parsing ac-4 dsi: only versions 0 and 1 are supported");
        return false;
    }

    CHECK_BITS_LEFT(7 + 1 + 4 + 9);
    uint32_t bitstream_version = mBitReader.getBits(7);
    mBitReader.skipBits(1); // fs_index
    mBitReader.skipBits(4); // frame_rate_index
    uint32_t n_presentations = mBitReader.getBits(9);

    int32_t short_program_id = -1;
    if (bitstream_version > 1) {
        if (ac4_dsi_version == 0) {
            ALOGE("invalid ac4 dsi");
            return false;
        }
        CHECK_BITS_LEFT(1);
        bool b_program_id = (mBitReader.getBits(1) == 1);
        if (b_program_id) {
            CHECK_BITS_LEFT(16 + 1);
            short_program_id = mBitReader.getBits(16);
            bool b_uuid = (mBitReader.getBits(1) == 1);
            if (b_uuid) {
                const uint32_t kAC4UUIDSizeInBytes = 16;
                char program_uuid[kAC4UUIDSizeInBytes];
                CHECK_BITS_LEFT(kAC4UUIDSizeInBytes * 8);
                for (uint32_t i = 0; i < kAC4UUIDSizeInBytes; i++) {
                    program_uuid[i] = (char)(mBitReader.getBits(8));
                }
                ALOGV("UUID = %s", program_uuid);
            }
        }
    }

    if (ac4_dsi_version == 1) {
        if (!parseBitrateDsi()) {
            return false;
        }
        BYTE_ALIGN;
    }

    for (uint32_t presentation = 0; presentation < n_presentations; presentation++) {
        mPresentations[presentation].mProgramID = short_program_id;
        // known as b_single_substream in ac4_dsi_version 0
        bool b_single_substream_group = false;
        uint32_t presentation_config = 0, presentation_version = 0;
        uint32_t pres_bytes = 0;
        uint64_t start = 0;

        if (ac4_dsi_version == 0) {
            CHECK_BITS_LEFT(1 + 5 + 5);
            b_single_substream_group = (mBitReader.getBits(1) == 1);
            presentation_config = mBitReader.getBits(5);
            presentation_version = mBitReader.getBits(5);
        } else if (ac4_dsi_version == 1) {
            CHECK_BITS_LEFT(8 + 8);
            presentation_version = mBitReader.getBits(8);
            pres_bytes = mBitReader.getBits(8);
            if (pres_bytes == 0xff) {
                CHECK_BITS_LEFT(16);
                pres_bytes += mBitReader.getBits(16);
            }
            ALOGV("%u: pres_bytes = %u\n", presentation, pres_bytes);
            if (presentation_version > 2) {
                CHECK_BITS_LEFT(pres_bytes * 8);
                mBitReader.skipBits(pres_bytes * 8);
                continue;
            }
            /* record a marker, less the size of the presentation_config */
            start = (mDSISize - mBitReader.numBitsLeft()) / 8;
            // ac4_presentation_v0_dsi(), ac4_presentation_v1_dsi() and ac4_presentation_v2_dsi()
            // all start with a presentation_config of 5 bits
            CHECK_BITS_LEFT(5);
            presentation_config = mBitReader.getBits(5);
            b_single_substream_group = (presentation_config == 0x1f);
        }

        static const char *PresentationConfig[] = {
            "Music&Effects + Dialog",
            "Main + DE",
            "Main + Associate",
            "Music&Effects + Dialog + Associate",
            "Main + DE + Associate",
            "Arbitrary substream groups",
            "EMDF only"
        };
        ALOGV("%u: b_single_substream/group = %s\n", presentation,
            BOOLSTR(b_single_substream_group));
        ALOGV("%u: presentation_version = %u\n", presentation, presentation_version);
        ALOGV("%u: presentation_config = %u (%s)\n", presentation, presentation_config,
            (presentation_config >= NELEM(PresentationConfig) ?
            "reserved" : PresentationConfig[presentation_config]));

        bool b_add_emdf_substreams = false;
        if (!b_single_substream_group && presentation_config == 6) {
            b_add_emdf_substreams = true;
            ALOGV("%u: b_add_emdf_substreams = %s\n", presentation, BOOLSTR(b_add_emdf_substreams));
        } else {
            CHECK_BITS_LEFT(3 + 1);
            uint32_t mdcompat = mBitReader.getBits(3);
            ALOGV("%u: mdcompat = %d\n", presentation, mdcompat);

            bool b_presentation_group_index = (mBitReader.getBits(1) == 1);
            ALOGV("%u: b_presentation_group_index = %s\n", presentation,
                BOOLSTR(b_presentation_group_index));
            if (b_presentation_group_index) {
                CHECK_BITS_LEFT(5);
                mPresentations[presentation].mGroupIndex = mBitReader.getBits(5);
                ALOGV("%u: presentation_group_index = %d\n", presentation,
                    mPresentations[presentation].mGroupIndex);
            }
            CHECK_BITS_LEFT(2);
            uint32_t dsi_frame_rate_multiply_info = mBitReader.getBits(2);
            ALOGV("%u: dsi_frame_rate_multiply_info = %d\n", presentation,
                dsi_frame_rate_multiply_info);
            if (ac4_dsi_version == 1 && (presentation_version == 1 || presentation_version == 2)) {
                CHECK_BITS_LEFT(2);
                uint32_t dsi_frame_rate_fraction_info = mBitReader.getBits(2);
                ALOGV("%u: dsi_frame_rate_fraction_info = %d\n", presentation,
                    dsi_frame_rate_fraction_info);
            }
            CHECK_BITS_LEFT(5 + 10);
            uint32_t presentation_emdf_version = mBitReader.getBits(5);
            uint32_t presentation_key_id = mBitReader.getBits(10);
            ALOGV("%u: presentation_emdf_version = %d\n", presentation, presentation_emdf_version);
            ALOGV("%u: presentation_key_id = %d\n", presentation, presentation_key_id);

            if (ac4_dsi_version == 1) {
                bool b_presentation_channel_coded = false;
                if (presentation_version == 0) {
                    b_presentation_channel_coded = true;
                } else {
                    CHECK_BITS_LEFT(1);
                    b_presentation_channel_coded = (mBitReader.getBits(1) == 1);
                }
                ALOGV("%u: b_presentation_channel_coded = %s\n", presentation,
                    BOOLSTR(b_presentation_channel_coded));
                if (b_presentation_channel_coded) {
                    if (presentation_version == 1 || presentation_version == 2) {
                        CHECK_BITS_LEFT(5);
                        uint32_t dsi_presentation_ch_mode = mBitReader.getBits(5);
                        mPresentations[presentation].mChannelMode = dsi_presentation_ch_mode;
                        ALOGV("%u: dsi_presentation_ch_mode = %d (%s)\n", presentation,
                            dsi_presentation_ch_mode,
                            dsi_presentation_ch_mode < NELEM(ChannelModes) ?
                            ChannelModes[dsi_presentation_ch_mode] : "reserved");

                        if (dsi_presentation_ch_mode >= 11 && dsi_presentation_ch_mode <= 14) {
                            CHECK_BITS_LEFT(1 + 2);
                            uint32_t pres_b_4_back_channels_present = mBitReader.getBits(1);
                            uint32_t pres_top_channel_pairs = mBitReader.getBits(2);
                            ALOGV("%u: pres_b_4_back_channels_present = %s\n", presentation,
                                BOOLSTR(pres_b_4_back_channels_present));
                            ALOGV("%u: pres_top_channel_pairs = %d\n", presentation,
                                pres_top_channel_pairs);
                        }
                    }
                    // presentation_channel_mask in ac4_presentation_v0_dsi()
                    CHECK_BITS_LEFT(24);
                    uint32_t presentation_channel_mask_v1 = mBitReader.getBits(24);
                    ALOGV("%u: presentation_channel_mask_v1 = 0x%06x\n", presentation,
                        presentation_channel_mask_v1);
                }
                if (presentation_version == 1 || presentation_version == 2) {
                    CHECK_BITS_LEFT(1);
                    bool b_presentation_core_differs = (mBitReader.getBits(1) == 1);
                    ALOGV("%u: b_presentation_core_differs = %s\n", presentation,
                        BOOLSTR(b_presentation_core_differs));
                    if (b_presentation_core_differs) {
                        CHECK_BITS_LEFT(1);
                        bool b_presentation_core_channel_coded = (mBitReader.getBits(1) == 1);
                        if (b_presentation_core_channel_coded) {
                            CHECK_BITS_LEFT(2);
                            mBitReader.skipBits(2); // dsi_presentation_channel_mode_core
                        }
                    }
                    CHECK_BITS_LEFT(1);
                    bool b_presentation_filter = (mBitReader.getBits(1) == 1);
                    ALOGV("%u: b_presentation_filter = %s\n", presentation,
                        BOOLSTR(b_presentation_filter));
                    if (b_presentation_filter) {
                        CHECK_BITS_LEFT(1 + 8);
                        bool b_enable_presentation = (mBitReader.getBits(1) == 1);
                        if (!b_enable_presentation) {
                            mPresentations[presentation].mEnabled = false;
                        }
                        ALOGV("%u: b_enable_presentation = %s\n", presentation,
                            BOOLSTR(b_enable_presentation));
                        uint32_t n_filter_bytes = mBitReader.getBits(8);
                        CHECK_BITS_LEFT(n_filter_bytes * 8);
                        for (uint32_t i = 0; i < n_filter_bytes; i++) {
                            mBitReader.skipBits(8); // filter_data
                        }
                    }
                }
            } /* ac4_dsi_version == 1 */

            if (b_single_substream_group) {
                if (presentation_version == 0) {
                    if (!parseSubstreamDSI(presentation, 0)) {
                        return false;
                    }
                } else {
                    if (!parseSubstreamGroupDSI(presentation, 0)) {
                        return false;
                    }
                }
            } else {
                if (ac4_dsi_version == 1) {
                    CHECK_BITS_LEFT(1);
                    bool b_multi_pid = (mBitReader.getBits(1) == 1);
                    ALOGV("%u: b_multi_pid = %s\n", presentation, BOOLSTR(b_multi_pid));
                } else {
                    CHECK_BITS_LEFT(1);
                    bool b_hsf_ext = (mBitReader.getBits(1) == 1);
                    ALOGV("%u: b_hsf_ext = %s\n", presentation, BOOLSTR(b_hsf_ext));
                }
                switch (presentation_config) {
                case 0:
                case 1:
                case 2:
                    if (presentation_version == 0) {
                        if (!parseSubstreamDSI(presentation, 0)) {
                            return false;
                        }
                        if (!parseSubstreamDSI(presentation, 1)) {
                            return false;
                        }
                    } else {
                        if (!parseSubstreamGroupDSI(presentation, 0)) {
                            return false;
                        }
                        if (!parseSubstreamGroupDSI(presentation, 1)) {
                            return false;
                        }
                    }
                    break;
                case 3:
                case 4:
                    if (presentation_version == 0) {
                        if (!parseSubstreamDSI(presentation, 0)) {
                            return false;
                        }
                        if (!parseSubstreamDSI(presentation, 1)) {
                            return false;
                        }
                        if (!parseSubstreamDSI(presentation, 2)) {
                            return false;
                        }
                    } else {
                        if (!parseSubstreamGroupDSI(presentation, 0)) {
                            return false;
                        }
                        if (!parseSubstreamGroupDSI(presentation, 1)) {
                            return false;
                        }
                        if (!parseSubstreamGroupDSI(presentation, 2)) {
                            return false;
                        }
                    }
                    break;
                case 5:
                    if (presentation_version == 0) {
                        if (!parseSubstreamDSI(presentation, 0)) {
                            return false;
                        }
                    } else {
                        CHECK_BITS_LEFT(3);
                        uint32_t n_substream_groups_minus2 = mBitReader.getBits(3);
                        ALOGV("%u: n_substream_groups_minus2 = %d\n", presentation,
                            n_substream_groups_minus2);
                        for (uint32_t sg = 0; sg < n_substream_groups_minus2 + 2; sg++) {
                            if (!parseSubstreamGroupDSI(presentation, sg)) {
                                return false;
                            }
                        }
                    }
                    break;
                default:
                    CHECK_BITS_LEFT(7);
                    uint32_t n_skip_bytes = mBitReader.getBits(7);
                    CHECK_BITS_LEFT(n_skip_bytes * 8)
                    for (uint32_t j = 0; j < n_skip_bytes; j++) {
                        mBitReader.getBits(8);
                    }
                    break;
                }
            }
            CHECK_BITS_LEFT(1 + 1);
            bool b_pre_virtualized = (mBitReader.getBits(1) == 1);
            mPresentations[presentation].mPreVirtualized = b_pre_virtualized;
            b_add_emdf_substreams = (mBitReader.getBits(1) == 1);
            ALOGV("%u: b_pre_virtualized = %s\n", presentation, BOOLSTR(b_pre_virtualized));
            ALOGV("%u: b_add_emdf_substreams = %s\n", presentation,
                BOOLSTR(b_add_emdf_substreams));
        }
        if (b_add_emdf_substreams) {
            CHECK_BITS_LEFT(7);
            uint32_t n_add_emdf_substreams = mBitReader.getBits(7);
            for (uint32_t j = 0; j < n_add_emdf_substreams; j++) {
                CHECK_BITS_LEFT(5 + 10);
                uint32_t substream_emdf_version = mBitReader.getBits(5);
                uint32_t substream_key_id = mBitReader.getBits(10);
                ALOGV("%u: emdf_substream[%d]: version=%d, key_id=%d\n", presentation, j,
                    substream_emdf_version, substream_key_id);
            }
        }

        bool b_presentation_bitrate_info = false;
        if (presentation_version > 0) {
            CHECK_BITS_LEFT(1);
            b_presentation_bitrate_info = (mBitReader.getBits(1) == 1);
        }

        ALOGV("b_presentation_bitrate_info = %s\n", BOOLSTR(b_presentation_bitrate_info));
        if (b_presentation_bitrate_info) {
            if (!parseBitrateDsi()) {
                return false;
            }
        }

        if (presentation_version > 0) {
            CHECK_BITS_LEFT(1);
            bool b_alternative = (mBitReader.getBits(1) == 1);
            ALOGV("b_alternative = %s\n", BOOLSTR(b_alternative));
            if (b_alternative) {
                BYTE_ALIGN;
                CHECK_BITS_LEFT(16);
                uint32_t name_len = mBitReader.getBits(16);
                CHECK_BITS_LEFT(name_len * 8);
                std::string &presentation_name =
                    mPresentations[presentation].mDescription;
                presentation_name.clear();
                presentation_name.resize(name_len);
                for (uint32_t i = 0; i < name_len; i++) {
                    presentation_name[i] = (char)(mBitReader.getBits(8));
                }
                CHECK_BITS_LEFT(5);
                uint32_t n_targets = mBitReader.getBits(5);
                CHECK_BITS_LEFT(n_targets * (3 + 8));
                for (uint32_t i = 0; i < n_targets; i++){
                    mBitReader.skipBits(3); // target_md_compat
                    mBitReader.skipBits(8); // target_device_category
                }
            }
        }

        BYTE_ALIGN;

        if (ac4_dsi_version == 1) {
            uint64_t end = (mDSISize - mBitReader.numBitsLeft()) / 8;
            uint64_t presentation_bytes = end - start;
            uint64_t skip_bytes = pres_bytes - presentation_bytes;
            ALOGV("skipping = %" PRIu64 " bytes", skip_bytes);
            CHECK_BITS_LEFT(skip_bytes * 8);
            mBitReader.skipBits(skip_bytes * 8);
        }

        // we should know this or something is probably wrong
        // with the bitstream (or we don't support it)
        if (mPresentations[presentation].mChannelMode == -1) {
            ALOGE("could not determing channel mode of presentation %d", presentation);
            return false;
        }
    } /* each presentation */

    return true;
}

};
