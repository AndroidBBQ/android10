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

#ifndef HEVC_UTILS_H_

#define HEVC_UTILS_H_

#include <stdint.h>

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/StrongPointer.h>
#include <utils/Vector.h>

namespace android {

enum {
    kHevcNalUnitTypeVps = 32,
    kHevcNalUnitTypeSps = 33,
    kHevcNalUnitTypePps = 34,
    kHevcNalUnitTypePrefixSei = 39,
    kHevcNalUnitTypeSuffixSei = 40,
};

enum {
    // uint8_t
    kGeneralProfileSpace,
    // uint8_t
    kGeneralTierFlag,
    // uint8_t
    kGeneralProfileIdc,
    // uint32_t
    kGeneralProfileCompatibilityFlags,
    // uint64_t
    kGeneralConstraintIndicatorFlags,
    // uint8_t
    kGeneralLevelIdc,
    // uint8_t
    kChromaFormatIdc,
    // uint8_t
    kBitDepthLumaMinus8,
    // uint8_t
    kBitDepthChromaMinus8,
    // uint8_t
    kVideoFullRangeFlag,
    // uint8_t
    kColourPrimaries,
    // uint8_t
    kTransferCharacteristics,
    // uint8_t
    kMatrixCoeffs,
};

class HevcParameterSets {
public:
    enum Info : uint32_t {
        kInfoNone                = 0,
        kInfoIsHdr               = 1 << 0,
        kInfoHasColorDescription = 1 << 1,
    };

    HevcParameterSets();

    status_t addNalUnit(const uint8_t* data, size_t size);

    bool findParam8(uint32_t key, uint8_t *param);
    bool findParam16(uint32_t key, uint16_t *param);
    bool findParam32(uint32_t key, uint32_t *param);
    bool findParam64(uint32_t key, uint64_t *param);

    inline size_t getNumNalUnits() { return mNalUnits.size(); }
    size_t getNumNalUnitsOfType(uint8_t type);
    uint8_t getType(size_t index);
    size_t getSize(size_t index);
    // Note that this method does not write the start code.
    bool write(size_t index, uint8_t* dest, size_t size);
    status_t makeHvcc(uint8_t *hvcc, size_t *hvccSize, size_t nalSizeLength);

    Info getInfo() const { return mInfo; }

private:
    status_t parseVps(const uint8_t* data, size_t size);
    status_t parseSps(const uint8_t* data, size_t size);
    status_t parsePps(const uint8_t* data, size_t size);

    KeyedVector<uint32_t, uint64_t> mParams;
    Vector<sp<ABuffer>> mNalUnits;
    Info mInfo;

    DISALLOW_EVIL_CONSTRUCTORS(HevcParameterSets);
};

}  // namespace android

#endif  // HEVC_UTILS_H_
