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

//#define LOG_NDEBUG 0
#define LOG_TAG "HevcUtils"

#include <cstring>
#include <utility>

#include "include/HevcUtils.h"

#include <media/stagefright/foundation/ABitReader.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/avc_utils.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/Utils.h>

namespace android {

static const uint8_t kHevcNalUnitTypes[5] = {
    kHevcNalUnitTypeVps,
    kHevcNalUnitTypeSps,
    kHevcNalUnitTypePps,
    kHevcNalUnitTypePrefixSei,
    kHevcNalUnitTypeSuffixSei,
};

HevcParameterSets::HevcParameterSets()
    : mInfo(kInfoNone) {
}

status_t HevcParameterSets::addNalUnit(const uint8_t* data, size_t size) {
    if (size < 1) {
        ALOGE("empty NAL b/35467107");
        return ERROR_MALFORMED;
    }
    uint8_t nalUnitType = (data[0] >> 1) & 0x3f;
    status_t err = OK;
    switch (nalUnitType) {
        case 32:  // VPS
            if (size < 2) {
                ALOGE("invalid NAL/VPS size b/35467107");
                return ERROR_MALFORMED;
            }
            err = parseVps(data + 2, size - 2);
            break;
        case 33:  // SPS
            if (size < 2) {
                ALOGE("invalid NAL/SPS size b/35467107");
                return ERROR_MALFORMED;
            }
            err = parseSps(data + 2, size - 2);
            break;
        case 34:  // PPS
            if (size < 2) {
                ALOGE("invalid NAL/PPS size b/35467107");
                return ERROR_MALFORMED;
            }
            err = parsePps(data + 2, size - 2);
            break;
        case 39:  // Prefix SEI
        case 40:  // Suffix SEI
            // Ignore
            break;
        default:
            ALOGE("Unrecognized NAL unit type.");
            return ERROR_MALFORMED;
    }

    if (err != OK) {
        return err;
    }

    sp<ABuffer> buffer = ABuffer::CreateAsCopy(data, size);
    buffer->setInt32Data(nalUnitType);
    mNalUnits.push(buffer);
    return OK;
}

template <typename T>
static bool findParam(uint32_t key, T *param,
        KeyedVector<uint32_t, uint64_t> &params) {
    CHECK(param);
    if (params.indexOfKey(key) < 0) {
        return false;
    }
    *param = (T) params[key];
    return true;
}

bool HevcParameterSets::findParam8(uint32_t key, uint8_t *param) {
    return findParam(key, param, mParams);
}

bool HevcParameterSets::findParam16(uint32_t key, uint16_t *param) {
    return findParam(key, param, mParams);
}

bool HevcParameterSets::findParam32(uint32_t key, uint32_t *param) {
    return findParam(key, param, mParams);
}

bool HevcParameterSets::findParam64(uint32_t key, uint64_t *param) {
    return findParam(key, param, mParams);
}

size_t HevcParameterSets::getNumNalUnitsOfType(uint8_t type) {
    size_t num = 0;
    for (size_t i = 0; i < mNalUnits.size(); ++i) {
        if (getType(i) == type) {
            ++num;
        }
    }
    return num;
}

uint8_t HevcParameterSets::getType(size_t index) {
    CHECK_LT(index, mNalUnits.size());
    return mNalUnits[index]->int32Data();
}

size_t HevcParameterSets::getSize(size_t index) {
    CHECK_LT(index, mNalUnits.size());
    return mNalUnits[index]->size();
}

bool HevcParameterSets::write(size_t index, uint8_t* dest, size_t size) {
    CHECK_LT(index, mNalUnits.size());
    const sp<ABuffer>& nalUnit = mNalUnits[index];
    if (size < nalUnit->size()) {
        ALOGE("dest buffer size too small: %zu vs. %zu to be written",
                size, nalUnit->size());
        return false;
    }
    memcpy(dest, nalUnit->data(), nalUnit->size());
    return true;
}

status_t HevcParameterSets::parseVps(const uint8_t* data, size_t size) {
    // See Rec. ITU-T H.265 v3 (04/2015) Chapter 7.3.2.1 for reference
    NALBitReader reader(data, size);
    // Skip vps_video_parameter_set_id
    reader.skipBits(4);
    // Skip vps_base_layer_internal_flag
    reader.skipBits(1);
    // Skip vps_base_layer_available_flag
    reader.skipBits(1);
    // Skip vps_max_layers_minus_1
    reader.skipBits(6);
    // Skip vps_max_sub_layers_minus1
    reader.skipBits(3);
    // Skip vps_temporal_id_nesting_flags
    reader.skipBits(1);
    // Skip reserved
    reader.skipBits(16);

    if (reader.atLeastNumBitsLeft(96)) {
        mParams.add(kGeneralProfileSpace, reader.getBits(2));
        mParams.add(kGeneralTierFlag, reader.getBits(1));
        mParams.add(kGeneralProfileIdc, reader.getBits(5));
        mParams.add(kGeneralProfileCompatibilityFlags, reader.getBits(32));
        mParams.add(
                kGeneralConstraintIndicatorFlags,
                ((uint64_t)reader.getBits(16) << 32) | reader.getBits(32));
        mParams.add(kGeneralLevelIdc, reader.getBits(8));
        // 96 bits total for general profile.
    } else {
        reader.skipBits(96);
    }

    return reader.overRead() ? ERROR_MALFORMED : OK;
}

status_t HevcParameterSets::parseSps(const uint8_t* data, size_t size) {
    // See Rec. ITU-T H.265 v3 (04/2015) Chapter 7.3.2.2 for reference
    NALBitReader reader(data, size);
    // Skip sps_video_parameter_set_id
    reader.skipBits(4);
    uint8_t maxSubLayersMinus1 = reader.getBitsWithFallback(3, 0);
    // Skip sps_temporal_id_nesting_flag;
    reader.skipBits(1);
    // Skip general profile
    reader.skipBits(96);
    if (maxSubLayersMinus1 > 0) {
        bool subLayerProfilePresentFlag[8];
        bool subLayerLevelPresentFlag[8];
        for (int i = 0; i < maxSubLayersMinus1; ++i) {
            subLayerProfilePresentFlag[i] = reader.getBitsWithFallback(1, 0);
            subLayerLevelPresentFlag[i] = reader.getBitsWithFallback(1, 0);
        }
        // Skip reserved
        reader.skipBits(2 * (8 - maxSubLayersMinus1));
        for (int i = 0; i < maxSubLayersMinus1; ++i) {
            if (subLayerProfilePresentFlag[i]) {
                // Skip profile
                reader.skipBits(88);
            }
            if (subLayerLevelPresentFlag[i]) {
                // Skip sub_layer_level_idc[i]
                reader.skipBits(8);
            }
        }
    }
    // Skip sps_seq_parameter_set_id
    skipUE(&reader);
    uint8_t chromaFormatIdc = parseUEWithFallback(&reader, 0);
    mParams.add(kChromaFormatIdc, chromaFormatIdc);
    if (chromaFormatIdc == 3) {
        // Skip separate_colour_plane_flag
        reader.skipBits(1);
    }
    // Skip pic_width_in_luma_samples
    skipUE(&reader);
    // Skip pic_height_in_luma_samples
    skipUE(&reader);
    if (reader.getBitsWithFallback(1, 0) /* i.e. conformance_window_flag */) {
        // Skip conf_win_left_offset
        skipUE(&reader);
        // Skip conf_win_right_offset
        skipUE(&reader);
        // Skip conf_win_top_offset
        skipUE(&reader);
        // Skip conf_win_bottom_offset
        skipUE(&reader);
    }
    mParams.add(kBitDepthLumaMinus8, parseUEWithFallback(&reader, 0));
    mParams.add(kBitDepthChromaMinus8, parseUEWithFallback(&reader, 0));

    // log2_max_pic_order_cnt_lsb_minus4
    size_t log2MaxPicOrderCntLsb = parseUEWithFallback(&reader, 0) + (size_t)4;
    bool spsSubLayerOrderingInfoPresentFlag = reader.getBitsWithFallback(1, 0);
    for (uint32_t i = spsSubLayerOrderingInfoPresentFlag ? 0 : maxSubLayersMinus1;
            i <= maxSubLayersMinus1; ++i) {
        skipUE(&reader); // sps_max_dec_pic_buffering_minus1[i]
        skipUE(&reader); // sps_max_num_reorder_pics[i]
        skipUE(&reader); // sps_max_latency_increase_plus1[i]
    }

    skipUE(&reader); // log2_min_luma_coding_block_size_minus3
    skipUE(&reader); // log2_diff_max_min_luma_coding_block_size
    skipUE(&reader); // log2_min_luma_transform_block_size_minus2
    skipUE(&reader); // log2_diff_max_min_luma_transform_block_size
    skipUE(&reader); // max_transform_hierarchy_depth_inter
    skipUE(&reader); // max_transform_hierarchy_depth_intra
    if (reader.getBitsWithFallback(1, 0)) { // scaling_list_enabled_flag u(1)
        // scaling_list_data
        if (reader.getBitsWithFallback(1, 0)) { // sps_scaling_list_data_present_flag
            for (uint32_t sizeId = 0; sizeId < 4; ++sizeId) {
                for (uint32_t matrixId = 0; matrixId < 6; matrixId += (sizeId == 3) ? 3 : 1) {
                    if (!reader.getBitsWithFallback(1, 1)) {
                        // scaling_list_pred_mode_flag[sizeId][matrixId]
                        skipUE(&reader); // scaling_list_pred_matrix_id_delta[sizeId][matrixId]
                    } else {
                        uint32_t coefNum = std::min(64, (1 << (4 + (sizeId << 1))));
                        if (sizeId > 1) {
                            skipSE(&reader); // scaling_list_dc_coef_minus8[sizeId − 2][matrixId]
                        }
                        for (uint32_t i = 0; i < coefNum; ++i) {
                            skipSE(&reader); // scaling_list_delta_coef
                        }
                    }
                }
            }
        }
    }
    reader.skipBits(1); // amp_enabled_flag
    reader.skipBits(1); // sample_adaptive_offset_enabled_flag u(1)
    if (reader.getBitsWithFallback(1, 0)) { // pcm_enabled_flag
        reader.skipBits(4); // pcm_sample_bit_depth_luma_minus1
        reader.skipBits(4); // pcm_sample_bit_depth_chroma_minus1 u(4)
        skipUE(&reader); // log2_min_pcm_luma_coding_block_size_minus3
        skipUE(&reader); // log2_diff_max_min_pcm_luma_coding_block_size
        reader.skipBits(1); // pcm_loop_filter_disabled_flag
    }
    uint32_t numShortTermRefPicSets = parseUEWithFallback(&reader, 0);
    uint32_t numPics = 0;
    for (uint32_t i = 0; i < numShortTermRefPicSets; ++i) {
        // st_ref_pic_set(i)
        if (i != 0 && reader.getBitsWithFallback(1, 0)) { // inter_ref_pic_set_prediction_flag
            reader.skipBits(1); // delta_rps_sign
            skipUE(&reader); // abs_delta_rps_minus1
            uint32_t nextNumPics = 0;
            for (uint32_t j = 0; j <= numPics; ++j) {
                if (reader.getBitsWithFallback(1, 0) // used_by_curr_pic_flag[j]
                        || reader.getBitsWithFallback(1, 0)) { // use_delta_flag[j]
                    ++nextNumPics;
                }
            }
            numPics = nextNumPics;
        } else {
            uint32_t numNegativePics = parseUEWithFallback(&reader, 0);
            uint32_t numPositivePics = parseUEWithFallback(&reader, 0);
            if (numNegativePics > UINT32_MAX - numPositivePics) {
                return ERROR_MALFORMED;
            }
            numPics = numNegativePics + numPositivePics;
            for (uint32_t j = 0; j < numPics; ++j) {
                skipUE(&reader); // delta_poc_s0|1_minus1[i]
                reader.skipBits(1); // used_by_curr_pic_s0|1_flag[i]
                if (reader.overRead()) {
                    return ERROR_MALFORMED;
                }
            }
        }
        if (reader.overRead()) {
            return ERROR_MALFORMED;
        }
    }
    if (reader.getBitsWithFallback(1, 0)) { // long_term_ref_pics_present_flag
        uint32_t numLongTermRefPicSps = parseUEWithFallback(&reader, 0);
        for (uint32_t i = 0; i < numLongTermRefPicSps; ++i) {
            reader.skipBits(log2MaxPicOrderCntLsb); // lt_ref_pic_poc_lsb_sps[i]
            reader.skipBits(1); // used_by_curr_pic_lt_sps_flag[i]
            if (reader.overRead()) {
                return ERROR_MALFORMED;
            }
        }
    }
    reader.skipBits(1); // sps_temporal_mvp_enabled_flag
    reader.skipBits(1); // strong_intra_smoothing_enabled_flag
    if (reader.getBitsWithFallback(1, 0)) { // vui_parameters_present_flag
        if (reader.getBitsWithFallback(1, 0)) { // aspect_ratio_info_present_flag
            uint32_t aspectRatioIdc = reader.getBitsWithFallback(8, 0);
            if (aspectRatioIdc == 0xFF /* EXTENDED_SAR */) {
                reader.skipBits(16); // sar_width
                reader.skipBits(16); // sar_height
            }
        }
        if (reader.getBitsWithFallback(1, 0)) { // overscan_info_present_flag
            reader.skipBits(1); // overscan_appropriate_flag
        }
        if (reader.getBitsWithFallback(1, 0)) { // video_signal_type_present_flag
            reader.skipBits(3); // video_format
            uint32_t videoFullRangeFlag;
            if (reader.getBitsGraceful(1, &videoFullRangeFlag)) {
                mParams.add(kVideoFullRangeFlag, videoFullRangeFlag);
            }
            if (reader.getBitsWithFallback(1, 0)) { // colour_description_present_flag
                mInfo = (Info)(mInfo | kInfoHasColorDescription);
                uint32_t colourPrimaries, transferCharacteristics, matrixCoeffs;
                if (reader.getBitsGraceful(8, &colourPrimaries)) {
                    mParams.add(kColourPrimaries, colourPrimaries);
                }
                if (reader.getBitsGraceful(8, &transferCharacteristics)) {
                    mParams.add(kTransferCharacteristics, transferCharacteristics);
                    if (transferCharacteristics == 16 /* ST 2084 */
                            || transferCharacteristics == 18 /* ARIB STD-B67 HLG */) {
                        mInfo = (Info)(mInfo | kInfoIsHdr);
                    }
                }
                if (reader.getBitsGraceful(8, &matrixCoeffs)) {
                    mParams.add(kMatrixCoeffs, matrixCoeffs);
                }
            }
            // skip rest of VUI
        }
    }

    return reader.overRead() ? ERROR_MALFORMED : OK;
}

status_t HevcParameterSets::parsePps(
        const uint8_t* data __unused, size_t size __unused) {
    return OK;
}

status_t HevcParameterSets::makeHvcc(uint8_t *hvcc, size_t *hvccSize,
        size_t nalSizeLength) {
    if (hvcc == NULL || hvccSize == NULL
            || (nalSizeLength != 4 && nalSizeLength != 2)) {
        return BAD_VALUE;
    }
    // ISO 14496-15: HEVC file format
    size_t size = 23;  // 23 bytes in the header
    size_t numOfArrays = 0;
    const size_t numNalUnits = getNumNalUnits();
    for (size_t i = 0; i < ARRAY_SIZE(kHevcNalUnitTypes); ++i) {
        uint8_t type = kHevcNalUnitTypes[i];
        size_t numNalus = getNumNalUnitsOfType(type);
        if (numNalus == 0) {
            continue;
        }
        ++numOfArrays;
        size += 3;
        for (size_t j = 0; j < numNalUnits; ++j) {
            if (getType(j) != type) {
                continue;
            }
            size += 2 + getSize(j);
        }
    }
    uint8_t generalProfileSpace, generalTierFlag, generalProfileIdc;
    if (!findParam8(kGeneralProfileSpace, &generalProfileSpace)
            || !findParam8(kGeneralTierFlag, &generalTierFlag)
            || !findParam8(kGeneralProfileIdc, &generalProfileIdc)) {
        return ERROR_MALFORMED;
    }
    uint32_t compatibilityFlags;
    uint64_t constraintIdcFlags;
    if (!findParam32(kGeneralProfileCompatibilityFlags, &compatibilityFlags)
            || !findParam64(kGeneralConstraintIndicatorFlags, &constraintIdcFlags)) {
        return ERROR_MALFORMED;
    }
    uint8_t generalLevelIdc;
    if (!findParam8(kGeneralLevelIdc, &generalLevelIdc)) {
        return ERROR_MALFORMED;
    }
    uint8_t chromaFormatIdc, bitDepthLumaMinus8, bitDepthChromaMinus8;
    if (!findParam8(kChromaFormatIdc, &chromaFormatIdc)
            || !findParam8(kBitDepthLumaMinus8, &bitDepthLumaMinus8)
            || !findParam8(kBitDepthChromaMinus8, &bitDepthChromaMinus8)) {
        return ERROR_MALFORMED;
    }
    if (size > *hvccSize) {
        return NO_MEMORY;
    }
    *hvccSize = size;

    uint8_t *header = hvcc;
    header[0] = 1;
    header[1] = (generalProfileSpace << 6) | (generalTierFlag << 5) | generalProfileIdc;
    header[2] = (compatibilityFlags >> 24) & 0xff;
    header[3] = (compatibilityFlags >> 16) & 0xff;
    header[4] = (compatibilityFlags >> 8) & 0xff;
    header[5] = compatibilityFlags & 0xff;
    header[6] = (constraintIdcFlags >> 40) & 0xff;
    header[7] = (constraintIdcFlags >> 32) & 0xff;
    header[8] = (constraintIdcFlags >> 24) & 0xff;
    header[9] = (constraintIdcFlags >> 16) & 0xff;
    header[10] = (constraintIdcFlags >> 8) & 0xff;
    header[11] = constraintIdcFlags & 0xff;
    header[12] = generalLevelIdc;
    // FIXME: parse min_spatial_segmentation_idc.
    header[13] = 0xf0;
    header[14] = 0;
    // FIXME: derive parallelismType properly.
    header[15] = 0xfc;
    header[16] = 0xfc | chromaFormatIdc;
    header[17] = 0xf8 | bitDepthLumaMinus8;
    header[18] = 0xf8 | bitDepthChromaMinus8;
    // FIXME: derive avgFrameRate
    header[19] = 0;
    header[20] = 0;
    // constantFrameRate, numTemporalLayers, temporalIdNested all set to 0.
    header[21] = nalSizeLength - 1;
    header[22] = numOfArrays;
    header += 23;
    for (size_t i = 0; i < ARRAY_SIZE(kHevcNalUnitTypes); ++i) {
        uint8_t type = kHevcNalUnitTypes[i];
        size_t numNalus = getNumNalUnitsOfType(type);
        if (numNalus == 0) {
            continue;
        }
        // array_completeness set to 1.
        header[0] = type | 0x80;
        header[1] = (numNalus >> 8) & 0xff;
        header[2] = numNalus & 0xff;
        header += 3;
        for (size_t j = 0; j < numNalUnits; ++j) {
            if (getType(j) != type) {
                continue;
            }
            header[0] = (getSize(j) >> 8) & 0xff;
            header[1] = getSize(j) & 0xff;
            if (!write(j, header + 2, size - (header - (uint8_t *)hvcc))) {
                return NO_MEMORY;
            }
            header += (2 + getSize(j));
        }
    }
    CHECK_EQ(header - size, hvcc);

    return OK;
}

}  // namespace android
