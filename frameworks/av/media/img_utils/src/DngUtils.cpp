/*
 * Copyright 2014 The Android Open Source Project
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

#include <img_utils/DngUtils.h>

#include <inttypes.h>

#include <algorithm>
#include <vector>
#include <math.h>

namespace android {
namespace img_utils {

OpcodeListBuilder::OpcodeListBuilder() : mCount(0), mOpList(), mEndianOut(&mOpList, BIG) {
    if(mEndianOut.open() != OK) {
        ALOGE("%s: Open failed.", __FUNCTION__);
    }
}

OpcodeListBuilder::~OpcodeListBuilder() {
    if(mEndianOut.close() != OK) {
        ALOGE("%s: Close failed.", __FUNCTION__);
    }
}

size_t OpcodeListBuilder::getSize() const {
    return mOpList.getSize() + sizeof(mCount);
}

uint32_t OpcodeListBuilder::getCount() const {
    return mCount;
}

status_t OpcodeListBuilder::buildOpList(uint8_t* buf) const {
    uint32_t count = convertToBigEndian(mCount);
    memcpy(buf, &count, sizeof(count));
    memcpy(buf + sizeof(count), mOpList.getArray(), mOpList.getSize());
    return OK;
}

status_t OpcodeListBuilder::addGainMapsForMetadata(uint32_t lsmWidth,
                                                   uint32_t lsmHeight,
                                                   uint32_t activeAreaTop,
                                                   uint32_t activeAreaLeft,
                                                   uint32_t activeAreaBottom,
                                                   uint32_t activeAreaRight,
                                                   CfaLayout cfa,
                                                   const float* lensShadingMap) {
    status_t err = OK;
    uint32_t activeAreaWidth = activeAreaRight - activeAreaLeft;
    uint32_t activeAreaHeight = activeAreaBottom - activeAreaTop;

    switch (cfa) {
        case CFA_RGGB:
        case CFA_GRBG:
        case CFA_GBRG:
        case CFA_BGGR:
            err = addBayerGainMapsForMetadata(lsmWidth, lsmHeight, activeAreaWidth,
                    activeAreaHeight, cfa, lensShadingMap);
            break;
        case CFA_NONE:
            err = addMonochromeGainMapsForMetadata(lsmWidth, lsmHeight, activeAreaWidth,
                    activeAreaHeight, lensShadingMap);
            break;
        default:
            ALOGE("%s: Unknown CFA layout %d", __FUNCTION__, cfa);
            err = BAD_VALUE;
            break;
    }
    return err;
}

status_t OpcodeListBuilder::addBayerGainMapsForMetadata(uint32_t lsmWidth,
                                                   uint32_t lsmHeight,
                                                   uint32_t activeAreaWidth,
                                                   uint32_t activeAreaHeight,
                                                   CfaLayout cfa,
                                                   const float* lensShadingMap) {
    uint32_t redTop = 0;
    uint32_t redLeft = 0;
    uint32_t greenEvenTop = 0;
    uint32_t greenEvenLeft = 1;
    uint32_t greenOddTop = 1;
    uint32_t greenOddLeft = 0;
    uint32_t blueTop = 1;
    uint32_t blueLeft = 1;

    switch(cfa) {
        case CFA_RGGB:
            redTop = 0;
            redLeft = 0;
            greenEvenTop = 0;
            greenEvenLeft = 1;
            greenOddTop = 1;
            greenOddLeft = 0;
            blueTop = 1;
            blueLeft = 1;
            break;
        case CFA_GRBG:
            redTop = 0;
            redLeft = 1;
            greenEvenTop = 0;
            greenEvenLeft = 0;
            greenOddTop = 1;
            greenOddLeft = 1;
            blueTop = 1;
            blueLeft = 0;
            break;
        case CFA_GBRG:
            redTop = 1;
            redLeft = 0;
            greenEvenTop = 0;
            greenEvenLeft = 0;
            greenOddTop = 1;
            greenOddLeft = 1;
            blueTop = 0;
            blueLeft = 1;
            break;
        case CFA_BGGR:
            redTop = 1;
            redLeft = 1;
            greenEvenTop = 0;
            greenEvenLeft = 1;
            greenOddTop = 1;
            greenOddLeft = 0;
            blueTop = 0;
            blueLeft = 0;
            break;
        default:
            ALOGE("%s: Unknown CFA layout %d", __FUNCTION__, cfa);
            return BAD_VALUE;
    }

    std::vector<float> redMapVector(lsmWidth * lsmHeight);
    float *redMap = redMapVector.data();

    std::vector<float> greenEvenMapVector(lsmWidth * lsmHeight);
    float *greenEvenMap = greenEvenMapVector.data();

    std::vector<float> greenOddMapVector(lsmWidth * lsmHeight);
    float *greenOddMap = greenOddMapVector.data();

    std::vector<float> blueMapVector(lsmWidth * lsmHeight);
    float *blueMap = blueMapVector.data();

    double spacingV = 1.0 / std::max(1u, lsmHeight - 1);
    double spacingH = 1.0 / std::max(1u, lsmWidth - 1);

    size_t lsmMapSize = lsmWidth * lsmHeight * 4;

    // Split lens shading map channels into separate arrays
    size_t j = 0;
    for (size_t i = 0; i < lsmMapSize; i += 4, ++j) {
        redMap[j] = lensShadingMap[i + LSM_R_IND];
        greenEvenMap[j] = lensShadingMap[i + LSM_GE_IND];
        greenOddMap[j] = lensShadingMap[i + LSM_GO_IND];
        blueMap[j] = lensShadingMap[i + LSM_B_IND];
    }

    status_t err = addGainMap(/*top*/redTop,
                              /*left*/redLeft,
                              /*bottom*/activeAreaHeight,
                              /*right*/activeAreaWidth,
                              /*plane*/0,
                              /*planes*/1,
                              /*rowPitch*/2,
                              /*colPitch*/2,
                              /*mapPointsV*/lsmHeight,
                              /*mapPointsH*/lsmWidth,
                              /*mapSpacingV*/spacingV,
                              /*mapSpacingH*/spacingH,
                              /*mapOriginV*/0,
                              /*mapOriginH*/0,
                              /*mapPlanes*/1,
                              /*mapGains*/redMap);
    if (err != OK) return err;

    err = addGainMap(/*top*/greenEvenTop,
                     /*left*/greenEvenLeft,
                     /*bottom*/activeAreaHeight,
                     /*right*/activeAreaWidth,
                     /*plane*/0,
                     /*planes*/1,
                     /*rowPitch*/2,
                     /*colPitch*/2,
                     /*mapPointsV*/lsmHeight,
                     /*mapPointsH*/lsmWidth,
                     /*mapSpacingV*/spacingV,
                     /*mapSpacingH*/spacingH,
                     /*mapOriginV*/0,
                     /*mapOriginH*/0,
                     /*mapPlanes*/1,
                     /*mapGains*/greenEvenMap);
    if (err != OK) return err;

    err = addGainMap(/*top*/greenOddTop,
                     /*left*/greenOddLeft,
                     /*bottom*/activeAreaHeight,
                     /*right*/activeAreaWidth,
                     /*plane*/0,
                     /*planes*/1,
                     /*rowPitch*/2,
                     /*colPitch*/2,
                     /*mapPointsV*/lsmHeight,
                     /*mapPointsH*/lsmWidth,
                     /*mapSpacingV*/spacingV,
                     /*mapSpacingH*/spacingH,
                     /*mapOriginV*/0,
                     /*mapOriginH*/0,
                     /*mapPlanes*/1,
                     /*mapGains*/greenOddMap);
    if (err != OK) return err;

    err = addGainMap(/*top*/blueTop,
                     /*left*/blueLeft,
                     /*bottom*/activeAreaHeight,
                     /*right*/activeAreaWidth,
                     /*plane*/0,
                     /*planes*/1,
                     /*rowPitch*/2,
                     /*colPitch*/2,
                     /*mapPointsV*/lsmHeight,
                     /*mapPointsH*/lsmWidth,
                     /*mapSpacingV*/spacingV,
                     /*mapSpacingH*/spacingH,
                     /*mapOriginV*/0,
                     /*mapOriginH*/0,
                     /*mapPlanes*/1,
                     /*mapGains*/blueMap);
    return err;
}

status_t OpcodeListBuilder::addMonochromeGainMapsForMetadata(uint32_t lsmWidth,
                                                   uint32_t lsmHeight,
                                                   uint32_t activeAreaWidth,
                                                   uint32_t activeAreaHeight,
                                                   const float* lensShadingMap) {
    std::vector<float> mapVector(lsmWidth * lsmHeight);
    float *map = mapVector.data();

    double spacingV = 1.0 / std::max(1u, lsmHeight - 1);
    double spacingH = 1.0 / std::max(1u, lsmWidth - 1);

    size_t lsmMapSize = lsmWidth * lsmHeight * 4;

    // Split lens shading map channels into separate arrays
    size_t j = 0;
    for (size_t i = 0; i < lsmMapSize; i += 4, ++j) {
        map[j] = lensShadingMap[i];
    }

    status_t err = addGainMap(/*top*/0,
                              /*left*/0,
                              /*bottom*/activeAreaHeight,
                              /*right*/activeAreaWidth,
                              /*plane*/0,
                              /*planes*/1,
                              /*rowPitch*/1,
                              /*colPitch*/1,
                              /*mapPointsV*/lsmHeight,
                              /*mapPointsH*/lsmWidth,
                              /*mapSpacingV*/spacingV,
                              /*mapSpacingH*/spacingH,
                              /*mapOriginV*/0,
                              /*mapOriginH*/0,
                              /*mapPlanes*/1,
                              /*mapGains*/map);
    if (err != OK) return err;

    return err;
}

status_t OpcodeListBuilder::addGainMap(uint32_t top,
                                       uint32_t left,
                                       uint32_t bottom,
                                       uint32_t right,
                                       uint32_t plane,
                                       uint32_t planes,
                                       uint32_t rowPitch,
                                       uint32_t colPitch,
                                       uint32_t mapPointsV,
                                       uint32_t mapPointsH,
                                       double mapSpacingV,
                                       double mapSpacingH,
                                       double mapOriginV,
                                       double mapOriginH,
                                       uint32_t mapPlanes,
                                       const float* mapGains) {

    status_t err = addOpcodePreamble(GAIN_MAP_ID);
    if (err != OK) return err;

    // Allow this opcode to be skipped if not supported
    uint32_t flags = FLAG_OPTIONAL;

    err = mEndianOut.write(&flags, 0, 1);
    if (err != OK) return err;

    const uint32_t NUMBER_INT_ARGS = 11;
    const uint32_t NUMBER_DOUBLE_ARGS = 4;

    uint32_t totalSize = NUMBER_INT_ARGS * sizeof(uint32_t) + NUMBER_DOUBLE_ARGS * sizeof(double) +
            mapPointsV * mapPointsH * mapPlanes * sizeof(float);

    err = mEndianOut.write(&totalSize, 0, 1);
    if (err != OK) return err;

    // Batch writes as much as possible
    uint32_t settings1[] = { top,
                            left,
                            bottom,
                            right,
                            plane,
                            planes,
                            rowPitch,
                            colPitch,
                            mapPointsV,
                            mapPointsH };

    err = mEndianOut.write(settings1, 0, NELEMS(settings1));
    if (err != OK) return err;

    double settings2[] = { mapSpacingV,
                          mapSpacingH,
                          mapOriginV,
                          mapOriginH };

    err = mEndianOut.write(settings2, 0, NELEMS(settings2));
    if (err != OK) return err;

    err = mEndianOut.write(&mapPlanes, 0, 1);
    if (err != OK) return err;

    err = mEndianOut.write(mapGains, 0, mapPointsV * mapPointsH * mapPlanes);
    if (err != OK) return err;

    mCount++;

    return OK;
}

status_t OpcodeListBuilder::addWarpRectilinearForMetadata(const float* kCoeffs,
                                                          uint32_t activeArrayWidth,
                                                          uint32_t activeArrayHeight,
                                                          float opticalCenterX,
                                                          float opticalCenterY) {
    if (activeArrayWidth <= 1 || activeArrayHeight <= 1) {
        ALOGE("%s: Cannot add opcode for active array with dimensions w=%" PRIu32 ", h=%" PRIu32,
                __FUNCTION__, activeArrayWidth, activeArrayHeight);
        return BAD_VALUE;
    }

    double normalizedOCX = opticalCenterX / static_cast<double>(activeArrayWidth);
    double normalizedOCY = opticalCenterY / static_cast<double>(activeArrayHeight);

    normalizedOCX = CLAMP(normalizedOCX, 0, 1);
    normalizedOCY = CLAMP(normalizedOCY, 0, 1);

    double coeffs[6] = {
        kCoeffs[0],
        kCoeffs[1],
        kCoeffs[2],
        kCoeffs[3],
        kCoeffs[4],
        kCoeffs[5]
    };

    return addWarpRectilinear(/*numPlanes*/1,
                              /*opticalCenterX*/normalizedOCX,
                              /*opticalCenterY*/normalizedOCY,
                              coeffs);
}

status_t OpcodeListBuilder::addWarpRectilinear(uint32_t numPlanes,
                                               double opticalCenterX,
                                               double opticalCenterY,
                                               const double* kCoeffs) {

    status_t err = addOpcodePreamble(WARP_RECTILINEAR_ID);
    if (err != OK) return err;

    // Allow this opcode to be skipped if not supported
    uint32_t flags = FLAG_OPTIONAL;

    err = mEndianOut.write(&flags, 0, 1);
    if (err != OK) return err;

    const uint32_t NUMBER_CENTER_ARGS = 2;
    const uint32_t NUMBER_COEFFS = numPlanes * 6;
    uint32_t totalSize = (NUMBER_CENTER_ARGS + NUMBER_COEFFS) * sizeof(double) + sizeof(uint32_t);

    err = mEndianOut.write(&totalSize, 0, 1);
    if (err != OK) return err;

    err = mEndianOut.write(&numPlanes, 0, 1);
    if (err != OK) return err;

    err = mEndianOut.write(kCoeffs, 0, NUMBER_COEFFS);
    if (err != OK) return err;

    err = mEndianOut.write(&opticalCenterX, 0, 1);
    if (err != OK) return err;

    err = mEndianOut.write(&opticalCenterY, 0, 1);
    if (err != OK) return err;

    mCount++;

    return OK;
}

status_t OpcodeListBuilder::addBadPixelListForMetadata(const uint32_t* hotPixels,
                                                       uint32_t xyPairCount,
                                                       uint32_t colorFilterArrangement) {
    if (colorFilterArrangement > 3) {
        ALOGE("%s:  Unknown color filter arrangement %" PRIu32, __FUNCTION__,
                colorFilterArrangement);
        return BAD_VALUE;
    }

    return addBadPixelList(colorFilterArrangement, xyPairCount, 0, hotPixels, nullptr);
}

status_t OpcodeListBuilder::addBadPixelList(uint32_t bayerPhase,
                                            uint32_t badPointCount,
                                            uint32_t badRectCount,
                                            const uint32_t* badPointRowColPairs,
                                            const uint32_t* badRectTopLeftBottomRightTuples) {

    status_t err = addOpcodePreamble(FIX_BAD_PIXELS_LIST);
    if (err != OK) return err;

    // Allow this opcode to be skipped if not supported
    uint32_t flags = FLAG_OPTIONAL;

    err = mEndianOut.write(&flags, 0, 1);
    if (err != OK) return err;

    const uint32_t NUM_NON_VARLEN_FIELDS = 3;
    const uint32_t SIZE_OF_POINT = 2;
    const uint32_t SIZE_OF_RECT = 4;

    uint32_t totalSize =  (NUM_NON_VARLEN_FIELDS  + badPointCount * SIZE_OF_POINT +
            badRectCount * SIZE_OF_RECT) * sizeof(uint32_t);
    err = mEndianOut.write(&totalSize, 0, 1);
    if (err != OK) return err;

    err = mEndianOut.write(&bayerPhase, 0, 1);
    if (err != OK) return err;

    err = mEndianOut.write(&badPointCount, 0, 1);
    if (err != OK) return err;

    err = mEndianOut.write(&badRectCount, 0, 1);
    if (err != OK) return err;

    if (badPointCount > 0) {
        err = mEndianOut.write(badPointRowColPairs, 0, SIZE_OF_POINT * badPointCount);
        if (err != OK) return err;
    }

    if (badRectCount > 0) {
        err = mEndianOut.write(badRectTopLeftBottomRightTuples, 0, SIZE_OF_RECT * badRectCount);
        if (err != OK) return err;
    }

    mCount++;
    return OK;
}

status_t OpcodeListBuilder::addOpcodePreamble(uint32_t opcodeId) {
    status_t err = mEndianOut.write(&opcodeId, 0, 1);
    if (err != OK) return err;

    uint8_t version[] = {1, 3, 0, 0};
    err = mEndianOut.write(version, 0, NELEMS(version));
    if (err != OK) return err;
    return OK;
}

} /*namespace img_utils*/
} /*namespace android*/
