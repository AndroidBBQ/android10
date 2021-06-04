/*
 * Copyright (C) 2009 The Android Open Source Project
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
#define LOG_TAG "Utils"
#include <utils/Log.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>

#include <utility>
#include <vector>

#include "include/ESDS.h"
#include "include/HevcUtils.h"

#include <cutils/properties.h>
#include <media/openmax/OMX_Audio.h>
#include <media/openmax/OMX_Video.h>
#include <media/openmax/OMX_VideoExt.h>
#include <media/stagefright/CodecBase.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALookup.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/ByteUtils.h>
#include <media/stagefright/foundation/OpusHeader.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/MediaCodecConstants.h>
#include <media/stagefright/MediaDefs.h>
#include <media/AudioSystem.h>
#include <media/MediaPlayerInterface.h>
#include <media/stagefright/Utils.h>
#include <media/AudioParameter.h>
#include <system/audio.h>

namespace android {

static status_t copyNALUToABuffer(sp<ABuffer> *buffer, const uint8_t *ptr, size_t length) {
    if (((*buffer)->size() + 4 + length) > ((*buffer)->capacity() - (*buffer)->offset())) {
        sp<ABuffer> tmpBuffer = new (std::nothrow) ABuffer((*buffer)->size() + 4 + length + 1024);
        if (tmpBuffer.get() == NULL || tmpBuffer->base() == NULL) {
            return NO_MEMORY;
        }
        memcpy(tmpBuffer->data(), (*buffer)->data(), (*buffer)->size());
        tmpBuffer->setRange(0, (*buffer)->size());
        (*buffer) = tmpBuffer;
    }

    memcpy((*buffer)->data() + (*buffer)->size(), "\x00\x00\x00\x01", 4);
    memcpy((*buffer)->data() + (*buffer)->size() + 4, ptr, length);
    (*buffer)->setRange((*buffer)->offset(), (*buffer)->size() + 4 + length);
    return OK;
}

#if 0
static void convertMetaDataToMessageInt32(
        const sp<MetaData> &meta, sp<AMessage> &msg, uint32_t key, const char *name) {
    int32_t value;
    if (meta->findInt32(key, &value)) {
        msg->setInt32(name, value);
    }
}
#endif

static void convertMetaDataToMessageColorAspects(const MetaDataBase *meta, sp<AMessage> &msg) {
    // 0 values are unspecified
    int32_t range = 0;
    int32_t primaries = 0;
    int32_t transferFunction = 0;
    int32_t colorMatrix = 0;
    meta->findInt32(kKeyColorRange, &range);
    meta->findInt32(kKeyColorPrimaries, &primaries);
    meta->findInt32(kKeyTransferFunction, &transferFunction);
    meta->findInt32(kKeyColorMatrix, &colorMatrix);
    ColorAspects colorAspects;
    memset(&colorAspects, 0, sizeof(colorAspects));
    colorAspects.mRange = (ColorAspects::Range)range;
    colorAspects.mPrimaries = (ColorAspects::Primaries)primaries;
    colorAspects.mTransfer = (ColorAspects::Transfer)transferFunction;
    colorAspects.mMatrixCoeffs = (ColorAspects::MatrixCoeffs)colorMatrix;

    int32_t rangeMsg, standardMsg, transferMsg;
    if (CodecBase::convertCodecColorAspectsToPlatformAspects(
            colorAspects, &rangeMsg, &standardMsg, &transferMsg) != OK) {
        return;
    }

    // save specified values to msg
    if (rangeMsg != 0) {
        msg->setInt32("color-range", rangeMsg);
    }
    if (standardMsg != 0) {
        msg->setInt32("color-standard", standardMsg);
    }
    if (transferMsg != 0) {
        msg->setInt32("color-transfer", transferMsg);
    }
}

static bool isHdr(const sp<AMessage> &format) {
    // if CSD specifies HDR transfer(s), we assume HDR. Otherwise, if it specifies non-HDR
    // transfers, we must assume non-HDR. This is because CSD trumps any color-transfer key
    // in the format.
    int32_t isHdr;
    if (format->findInt32("android._is-hdr", &isHdr)) {
        return isHdr;
    }

    // if user/container supplied HDR static info without transfer set, assume true
    if ((format->contains("hdr-static-info") || format->contains("hdr10-plus-info"))
            && !format->contains("color-transfer")) {
        return true;
    }
    // otherwise, verify that an HDR transfer function is set
    int32_t transfer;
    if (format->findInt32("color-transfer", &transfer)) {
        return transfer == ColorUtils::kColorTransferST2084
                || transfer == ColorUtils::kColorTransferHLG;
    }
    return false;
}

static void parseAacProfileFromCsd(const sp<ABuffer> &csd, sp<AMessage> &format) {
    if (csd->size() < 2) {
        return;
    }

    uint16_t audioObjectType = U16_AT((uint8_t*)csd->data());
    if ((audioObjectType & 0xF800) == 0xF800) {
        audioObjectType = 32 + ((audioObjectType >> 5) & 0x3F);
    } else {
        audioObjectType >>= 11;
    }

    const static ALookup<uint16_t, OMX_AUDIO_AACPROFILETYPE> profiles {
        { 1,  OMX_AUDIO_AACObjectMain     },
        { 2,  OMX_AUDIO_AACObjectLC       },
        { 3,  OMX_AUDIO_AACObjectSSR      },
        { 4,  OMX_AUDIO_AACObjectLTP      },
        { 5,  OMX_AUDIO_AACObjectHE       },
        { 6,  OMX_AUDIO_AACObjectScalable },
        { 17, OMX_AUDIO_AACObjectERLC     },
        { 23, OMX_AUDIO_AACObjectLD       },
        { 29, OMX_AUDIO_AACObjectHE_PS    },
        { 39, OMX_AUDIO_AACObjectELD      },
        { 42, OMX_AUDIO_AACObjectXHE      },
    };

    OMX_AUDIO_AACPROFILETYPE profile;
    if (profiles.map(audioObjectType, &profile)) {
        format->setInt32("profile", profile);
    }
}

static void parseAvcProfileLevelFromAvcc(const uint8_t *ptr, size_t size, sp<AMessage> &format) {
    if (size < 4 || ptr[0] != 1) {  // configurationVersion == 1
        return;
    }
    const uint8_t profile = ptr[1];
    const uint8_t constraints = ptr[2];
    const uint8_t level = ptr[3];

    const static ALookup<uint8_t, OMX_VIDEO_AVCLEVELTYPE> levels {
        {  9, OMX_VIDEO_AVCLevel1b }, // technically, 9 is only used for High+ profiles
        { 10, OMX_VIDEO_AVCLevel1  },
        { 11, OMX_VIDEO_AVCLevel11 }, // prefer level 1.1 for the value 11
        { 11, OMX_VIDEO_AVCLevel1b },
        { 12, OMX_VIDEO_AVCLevel12 },
        { 13, OMX_VIDEO_AVCLevel13 },
        { 20, OMX_VIDEO_AVCLevel2  },
        { 21, OMX_VIDEO_AVCLevel21 },
        { 22, OMX_VIDEO_AVCLevel22 },
        { 30, OMX_VIDEO_AVCLevel3  },
        { 31, OMX_VIDEO_AVCLevel31 },
        { 32, OMX_VIDEO_AVCLevel32 },
        { 40, OMX_VIDEO_AVCLevel4  },
        { 41, OMX_VIDEO_AVCLevel41 },
        { 42, OMX_VIDEO_AVCLevel42 },
        { 50, OMX_VIDEO_AVCLevel5  },
        { 51, OMX_VIDEO_AVCLevel51 },
        { 52, OMX_VIDEO_AVCLevel52 },
        { 60, OMX_VIDEO_AVCLevel6  },
        { 61, OMX_VIDEO_AVCLevel61 },
        { 62, OMX_VIDEO_AVCLevel62 },
    };
    const static ALookup<uint8_t, OMX_VIDEO_AVCPROFILETYPE> profiles {
        { 66, OMX_VIDEO_AVCProfileBaseline },
        { 77, OMX_VIDEO_AVCProfileMain     },
        { 88, OMX_VIDEO_AVCProfileExtended },
        { 100, OMX_VIDEO_AVCProfileHigh    },
        { 110, OMX_VIDEO_AVCProfileHigh10  },
        { 122, OMX_VIDEO_AVCProfileHigh422 },
        { 244, OMX_VIDEO_AVCProfileHigh444 },
    };

    // set profile & level if they are recognized
    OMX_VIDEO_AVCPROFILETYPE codecProfile;
    OMX_VIDEO_AVCLEVELTYPE codecLevel;
    if (profiles.map(profile, &codecProfile)) {
        if (profile == 66 && (constraints & 0x40)) {
            codecProfile = (OMX_VIDEO_AVCPROFILETYPE)OMX_VIDEO_AVCProfileConstrainedBaseline;
        } else if (profile == 100 && (constraints & 0x0C) == 0x0C) {
            codecProfile = (OMX_VIDEO_AVCPROFILETYPE)OMX_VIDEO_AVCProfileConstrainedHigh;
        }
        format->setInt32("profile", codecProfile);
        if (levels.map(level, &codecLevel)) {
            // for 9 && 11 decide level based on profile and constraint_set3 flag
            if (level == 11 && (profile == 66 || profile == 77 || profile == 88)) {
                codecLevel = (constraints & 0x10) ? OMX_VIDEO_AVCLevel1b : OMX_VIDEO_AVCLevel11;
            }
            format->setInt32("level", codecLevel);
        }
    }
}

static void parseH263ProfileLevelFromD263(const uint8_t *ptr, size_t size, sp<AMessage> &format) {
    if (size < 7) {
        return;
    }

    const uint8_t profile = ptr[6];
    const uint8_t level = ptr[5];

    const static ALookup<uint8_t, OMX_VIDEO_H263PROFILETYPE> profiles {
        { 0, OMX_VIDEO_H263ProfileBaseline },
        { 1, OMX_VIDEO_H263ProfileH320Coding },
        { 2, OMX_VIDEO_H263ProfileBackwardCompatible },
        { 3, OMX_VIDEO_H263ProfileISWV2 },
        { 4, OMX_VIDEO_H263ProfileISWV3 },
        { 5, OMX_VIDEO_H263ProfileHighCompression },
        { 6, OMX_VIDEO_H263ProfileInternet },
        { 7, OMX_VIDEO_H263ProfileInterlace },
        { 8, OMX_VIDEO_H263ProfileHighLatency },
    };

    const static ALookup<uint8_t, OMX_VIDEO_H263LEVELTYPE> levels {
        { 10, OMX_VIDEO_H263Level10 },
        { 20, OMX_VIDEO_H263Level20 },
        { 30, OMX_VIDEO_H263Level30 },
        { 40, OMX_VIDEO_H263Level40 },
        { 45, OMX_VIDEO_H263Level45 },
        { 50, OMX_VIDEO_H263Level50 },
        { 60, OMX_VIDEO_H263Level60 },
        { 70, OMX_VIDEO_H263Level70 },
    };

    // set profile & level if they are recognized
    OMX_VIDEO_H263PROFILETYPE codecProfile;
    OMX_VIDEO_H263LEVELTYPE codecLevel;
    if (profiles.map(profile, &codecProfile)) {
        format->setInt32("profile", codecProfile);
        if (levels.map(level, &codecLevel)) {
            format->setInt32("level", codecLevel);
        }
    }
}

static void parseHevcProfileLevelFromHvcc(const uint8_t *ptr, size_t size, sp<AMessage> &format) {
    if (size < 13 || ptr[0] != 1) {  // configurationVersion == 1
        return;
    }

    const uint8_t profile = ptr[1] & 0x1F;
    const uint8_t tier = (ptr[1] & 0x20) >> 5;
    const uint8_t level = ptr[12];

    const static ALookup<std::pair<uint8_t, uint8_t>, OMX_VIDEO_HEVCLEVELTYPE> levels {
        { { 0, 30  }, OMX_VIDEO_HEVCMainTierLevel1  },
        { { 0, 60  }, OMX_VIDEO_HEVCMainTierLevel2  },
        { { 0, 63  }, OMX_VIDEO_HEVCMainTierLevel21 },
        { { 0, 90  }, OMX_VIDEO_HEVCMainTierLevel3  },
        { { 0, 93  }, OMX_VIDEO_HEVCMainTierLevel31 },
        { { 0, 120 }, OMX_VIDEO_HEVCMainTierLevel4  },
        { { 0, 123 }, OMX_VIDEO_HEVCMainTierLevel41 },
        { { 0, 150 }, OMX_VIDEO_HEVCMainTierLevel5  },
        { { 0, 153 }, OMX_VIDEO_HEVCMainTierLevel51 },
        { { 0, 156 }, OMX_VIDEO_HEVCMainTierLevel52 },
        { { 0, 180 }, OMX_VIDEO_HEVCMainTierLevel6  },
        { { 0, 183 }, OMX_VIDEO_HEVCMainTierLevel61 },
        { { 0, 186 }, OMX_VIDEO_HEVCMainTierLevel62 },
        { { 1, 30  }, OMX_VIDEO_HEVCHighTierLevel1  },
        { { 1, 60  }, OMX_VIDEO_HEVCHighTierLevel2  },
        { { 1, 63  }, OMX_VIDEO_HEVCHighTierLevel21 },
        { { 1, 90  }, OMX_VIDEO_HEVCHighTierLevel3  },
        { { 1, 93  }, OMX_VIDEO_HEVCHighTierLevel31 },
        { { 1, 120 }, OMX_VIDEO_HEVCHighTierLevel4  },
        { { 1, 123 }, OMX_VIDEO_HEVCHighTierLevel41 },
        { { 1, 150 }, OMX_VIDEO_HEVCHighTierLevel5  },
        { { 1, 153 }, OMX_VIDEO_HEVCHighTierLevel51 },
        { { 1, 156 }, OMX_VIDEO_HEVCHighTierLevel52 },
        { { 1, 180 }, OMX_VIDEO_HEVCHighTierLevel6  },
        { { 1, 183 }, OMX_VIDEO_HEVCHighTierLevel61 },
        { { 1, 186 }, OMX_VIDEO_HEVCHighTierLevel62 },
    };

    const static ALookup<uint8_t, OMX_VIDEO_HEVCPROFILETYPE> profiles {
        { 1, OMX_VIDEO_HEVCProfileMain   },
        { 2, OMX_VIDEO_HEVCProfileMain10 },
        // use Main for Main Still Picture decoding
        { 3, OMX_VIDEO_HEVCProfileMain },
    };

    // set profile & level if they are recognized
    OMX_VIDEO_HEVCPROFILETYPE codecProfile;
    OMX_VIDEO_HEVCLEVELTYPE codecLevel;
    if (!profiles.map(profile, &codecProfile)) {
        if (ptr[2] & 0x40 /* general compatibility flag 1 */) {
            // Note that this case covers Main Still Picture too
            codecProfile = OMX_VIDEO_HEVCProfileMain;
        } else if (ptr[2] & 0x20 /* general compatibility flag 2 */) {
            codecProfile = OMX_VIDEO_HEVCProfileMain10;
        } else {
            return;
        }
    }

    // bump to HDR profile
    if (isHdr(format) && codecProfile == OMX_VIDEO_HEVCProfileMain10) {
        codecProfile = OMX_VIDEO_HEVCProfileMain10HDR10;
    }

    format->setInt32("profile", codecProfile);
    if (levels.map(std::make_pair(tier, level), &codecLevel)) {
        format->setInt32("level", codecLevel);
    }
}

static void parseMpeg2ProfileLevelFromHeader(
        const uint8_t *data, size_t size, sp<AMessage> &format) {
    // find sequence extension
    const uint8_t *seq = (const uint8_t*)memmem(data, size, "\x00\x00\x01\xB5", 4);
    if (seq != NULL && seq + 5 < data + size) {
        const uint8_t start_code = seq[4] >> 4;
        if (start_code != 1 /* sequence extension ID */) {
            return;
        }
        const uint8_t indication = ((seq[4] & 0xF) << 4) | ((seq[5] & 0xF0) >> 4);

        const static ALookup<uint8_t, OMX_VIDEO_MPEG2PROFILETYPE> profiles {
            { 0x50, OMX_VIDEO_MPEG2ProfileSimple  },
            { 0x40, OMX_VIDEO_MPEG2ProfileMain    },
            { 0x30, OMX_VIDEO_MPEG2ProfileSNR     },
            { 0x20, OMX_VIDEO_MPEG2ProfileSpatial },
            { 0x10, OMX_VIDEO_MPEG2ProfileHigh    },
        };

        const static ALookup<uint8_t, OMX_VIDEO_MPEG2LEVELTYPE> levels {
            { 0x0A, OMX_VIDEO_MPEG2LevelLL  },
            { 0x08, OMX_VIDEO_MPEG2LevelML  },
            { 0x06, OMX_VIDEO_MPEG2LevelH14 },
            { 0x04, OMX_VIDEO_MPEG2LevelHL  },
            { 0x02, OMX_VIDEO_MPEG2LevelHP  },
        };

        const static ALookup<uint8_t,
                std::pair<OMX_VIDEO_MPEG2PROFILETYPE, OMX_VIDEO_MPEG2LEVELTYPE>> escapes {
            /* unsupported
            { 0x8E, { XXX_MPEG2ProfileMultiView, OMX_VIDEO_MPEG2LevelLL  } },
            { 0x8D, { XXX_MPEG2ProfileMultiView, OMX_VIDEO_MPEG2LevelML  } },
            { 0x8B, { XXX_MPEG2ProfileMultiView, OMX_VIDEO_MPEG2LevelH14 } },
            { 0x8A, { XXX_MPEG2ProfileMultiView, OMX_VIDEO_MPEG2LevelHL  } }, */
            { 0x85, { OMX_VIDEO_MPEG2Profile422, OMX_VIDEO_MPEG2LevelML  } },
            { 0x82, { OMX_VIDEO_MPEG2Profile422, OMX_VIDEO_MPEG2LevelHL  } },
        };

        OMX_VIDEO_MPEG2PROFILETYPE profile;
        OMX_VIDEO_MPEG2LEVELTYPE level;
        std::pair<OMX_VIDEO_MPEG2PROFILETYPE, OMX_VIDEO_MPEG2LEVELTYPE> profileLevel;
        if (escapes.map(indication, &profileLevel)) {
            format->setInt32("profile", profileLevel.first);
            format->setInt32("level", profileLevel.second);
        } else if (profiles.map(indication & 0x70, &profile)) {
            format->setInt32("profile", profile);
            if (levels.map(indication & 0xF, &level)) {
                format->setInt32("level", level);
            }
        }
    }
}

static void parseMpeg2ProfileLevelFromEsds(ESDS &esds, sp<AMessage> &format) {
    // esds seems to only contain the profile for MPEG-2
    uint8_t objType;
    if (esds.getObjectTypeIndication(&objType) == OK) {
        const static ALookup<uint8_t, OMX_VIDEO_MPEG2PROFILETYPE> profiles{
            { 0x60, OMX_VIDEO_MPEG2ProfileSimple  },
            { 0x61, OMX_VIDEO_MPEG2ProfileMain    },
            { 0x62, OMX_VIDEO_MPEG2ProfileSNR     },
            { 0x63, OMX_VIDEO_MPEG2ProfileSpatial },
            { 0x64, OMX_VIDEO_MPEG2ProfileHigh    },
            { 0x65, OMX_VIDEO_MPEG2Profile422     },
        };

        OMX_VIDEO_MPEG2PROFILETYPE profile;
        if (profiles.map(objType, &profile)) {
            format->setInt32("profile", profile);
        }
    }
}

static void parseMpeg4ProfileLevelFromCsd(const sp<ABuffer> &csd, sp<AMessage> &format) {
    const uint8_t *data = csd->data();
    // find visual object sequence
    const uint8_t *seq = (const uint8_t*)memmem(data, csd->size(), "\x00\x00\x01\xB0", 4);
    if (seq != NULL && seq + 4 < data + csd->size()) {
        const uint8_t indication = seq[4];

        const static ALookup<uint8_t,
                std::pair<OMX_VIDEO_MPEG4PROFILETYPE, OMX_VIDEO_MPEG4LEVELTYPE>> table {
            { 0b00000001, { OMX_VIDEO_MPEG4ProfileSimple,            OMX_VIDEO_MPEG4Level1  } },
            { 0b00000010, { OMX_VIDEO_MPEG4ProfileSimple,            OMX_VIDEO_MPEG4Level2  } },
            { 0b00000011, { OMX_VIDEO_MPEG4ProfileSimple,            OMX_VIDEO_MPEG4Level3  } },
            { 0b00000100, { OMX_VIDEO_MPEG4ProfileSimple,            OMX_VIDEO_MPEG4Level4a } },
            { 0b00000101, { OMX_VIDEO_MPEG4ProfileSimple,            OMX_VIDEO_MPEG4Level5  } },
            { 0b00000110, { OMX_VIDEO_MPEG4ProfileSimple,            OMX_VIDEO_MPEG4Level6  } },
            { 0b00001000, { OMX_VIDEO_MPEG4ProfileSimple,            OMX_VIDEO_MPEG4Level0  } },
            { 0b00001001, { OMX_VIDEO_MPEG4ProfileSimple,            OMX_VIDEO_MPEG4Level0b } },
            { 0b00010000, { OMX_VIDEO_MPEG4ProfileSimpleScalable,    OMX_VIDEO_MPEG4Level0  } },
            { 0b00010001, { OMX_VIDEO_MPEG4ProfileSimpleScalable,    OMX_VIDEO_MPEG4Level1  } },
            { 0b00010010, { OMX_VIDEO_MPEG4ProfileSimpleScalable,    OMX_VIDEO_MPEG4Level2  } },
            /* unsupported
            { 0b00011101, { XXX_MPEG4ProfileSimpleScalableER,        OMX_VIDEO_MPEG4Level0  } },
            { 0b00011110, { XXX_MPEG4ProfileSimpleScalableER,        OMX_VIDEO_MPEG4Level1  } },
            { 0b00011111, { XXX_MPEG4ProfileSimpleScalableER,        OMX_VIDEO_MPEG4Level2  } }, */
            { 0b00100001, { OMX_VIDEO_MPEG4ProfileCore,              OMX_VIDEO_MPEG4Level1  } },
            { 0b00100010, { OMX_VIDEO_MPEG4ProfileCore,              OMX_VIDEO_MPEG4Level2  } },
            { 0b00110010, { OMX_VIDEO_MPEG4ProfileMain,              OMX_VIDEO_MPEG4Level2  } },
            { 0b00110011, { OMX_VIDEO_MPEG4ProfileMain,              OMX_VIDEO_MPEG4Level3  } },
            { 0b00110100, { OMX_VIDEO_MPEG4ProfileMain,              OMX_VIDEO_MPEG4Level4  } },
            /* deprecated
            { 0b01000010, { OMX_VIDEO_MPEG4ProfileNbit,              OMX_VIDEO_MPEG4Level2  } }, */
            { 0b01010001, { OMX_VIDEO_MPEG4ProfileScalableTexture,   OMX_VIDEO_MPEG4Level1  } },
            { 0b01100001, { OMX_VIDEO_MPEG4ProfileSimpleFace,        OMX_VIDEO_MPEG4Level1  } },
            { 0b01100010, { OMX_VIDEO_MPEG4ProfileSimpleFace,        OMX_VIDEO_MPEG4Level2  } },
            { 0b01100011, { OMX_VIDEO_MPEG4ProfileSimpleFBA,         OMX_VIDEO_MPEG4Level1  } },
            { 0b01100100, { OMX_VIDEO_MPEG4ProfileSimpleFBA,         OMX_VIDEO_MPEG4Level2  } },
            { 0b01110001, { OMX_VIDEO_MPEG4ProfileBasicAnimated,     OMX_VIDEO_MPEG4Level1  } },
            { 0b01110010, { OMX_VIDEO_MPEG4ProfileBasicAnimated,     OMX_VIDEO_MPEG4Level2  } },
            { 0b10000001, { OMX_VIDEO_MPEG4ProfileHybrid,            OMX_VIDEO_MPEG4Level1  } },
            { 0b10000010, { OMX_VIDEO_MPEG4ProfileHybrid,            OMX_VIDEO_MPEG4Level2  } },
            { 0b10010001, { OMX_VIDEO_MPEG4ProfileAdvancedRealTime,  OMX_VIDEO_MPEG4Level1  } },
            { 0b10010010, { OMX_VIDEO_MPEG4ProfileAdvancedRealTime,  OMX_VIDEO_MPEG4Level2  } },
            { 0b10010011, { OMX_VIDEO_MPEG4ProfileAdvancedRealTime,  OMX_VIDEO_MPEG4Level3  } },
            { 0b10010100, { OMX_VIDEO_MPEG4ProfileAdvancedRealTime,  OMX_VIDEO_MPEG4Level4  } },
            { 0b10100001, { OMX_VIDEO_MPEG4ProfileCoreScalable,      OMX_VIDEO_MPEG4Level1  } },
            { 0b10100010, { OMX_VIDEO_MPEG4ProfileCoreScalable,      OMX_VIDEO_MPEG4Level2  } },
            { 0b10100011, { OMX_VIDEO_MPEG4ProfileCoreScalable,      OMX_VIDEO_MPEG4Level3  } },
            { 0b10110001, { OMX_VIDEO_MPEG4ProfileAdvancedCoding,    OMX_VIDEO_MPEG4Level1  } },
            { 0b10110010, { OMX_VIDEO_MPEG4ProfileAdvancedCoding,    OMX_VIDEO_MPEG4Level2  } },
            { 0b10110011, { OMX_VIDEO_MPEG4ProfileAdvancedCoding,    OMX_VIDEO_MPEG4Level3  } },
            { 0b10110100, { OMX_VIDEO_MPEG4ProfileAdvancedCoding,    OMX_VIDEO_MPEG4Level4  } },
            { 0b11000001, { OMX_VIDEO_MPEG4ProfileAdvancedCore,      OMX_VIDEO_MPEG4Level1  } },
            { 0b11000010, { OMX_VIDEO_MPEG4ProfileAdvancedCore,      OMX_VIDEO_MPEG4Level2  } },
            { 0b11010001, { OMX_VIDEO_MPEG4ProfileAdvancedScalable,  OMX_VIDEO_MPEG4Level1  } },
            { 0b11010010, { OMX_VIDEO_MPEG4ProfileAdvancedScalable,  OMX_VIDEO_MPEG4Level2  } },
            { 0b11010011, { OMX_VIDEO_MPEG4ProfileAdvancedScalable,  OMX_VIDEO_MPEG4Level3  } },
            /* unsupported
            { 0b11100001, { XXX_MPEG4ProfileSimpleStudio,            OMX_VIDEO_MPEG4Level1  } },
            { 0b11100010, { XXX_MPEG4ProfileSimpleStudio,            OMX_VIDEO_MPEG4Level2  } },
            { 0b11100011, { XXX_MPEG4ProfileSimpleStudio,            OMX_VIDEO_MPEG4Level3  } },
            { 0b11100100, { XXX_MPEG4ProfileSimpleStudio,            OMX_VIDEO_MPEG4Level4  } },
            { 0b11100101, { XXX_MPEG4ProfileCoreStudio,              OMX_VIDEO_MPEG4Level1  } },
            { 0b11100110, { XXX_MPEG4ProfileCoreStudio,              OMX_VIDEO_MPEG4Level2  } },
            { 0b11100111, { XXX_MPEG4ProfileCoreStudio,              OMX_VIDEO_MPEG4Level3  } },
            { 0b11101000, { XXX_MPEG4ProfileCoreStudio,              OMX_VIDEO_MPEG4Level4  } },
            { 0b11101011, { XXX_MPEG4ProfileSimpleStudio,            OMX_VIDEO_MPEG4Level5  } },
            { 0b11101100, { XXX_MPEG4ProfileSimpleStudio,            OMX_VIDEO_MPEG4Level6  } }, */
            { 0b11110000, { OMX_VIDEO_MPEG4ProfileAdvancedSimple,    OMX_VIDEO_MPEG4Level0  } },
            { 0b11110001, { OMX_VIDEO_MPEG4ProfileAdvancedSimple,    OMX_VIDEO_MPEG4Level1  } },
            { 0b11110010, { OMX_VIDEO_MPEG4ProfileAdvancedSimple,    OMX_VIDEO_MPEG4Level2  } },
            { 0b11110011, { OMX_VIDEO_MPEG4ProfileAdvancedSimple,    OMX_VIDEO_MPEG4Level3  } },
            { 0b11110100, { OMX_VIDEO_MPEG4ProfileAdvancedSimple,    OMX_VIDEO_MPEG4Level4  } },
            { 0b11110101, { OMX_VIDEO_MPEG4ProfileAdvancedSimple,    OMX_VIDEO_MPEG4Level5  } },
            { 0b11110111, { OMX_VIDEO_MPEG4ProfileAdvancedSimple,    OMX_VIDEO_MPEG4Level3b } },
            /* deprecated
            { 0b11111000, { XXX_MPEG4ProfileFineGranularityScalable, OMX_VIDEO_MPEG4Level0  } },
            { 0b11111001, { XXX_MPEG4ProfileFineGranularityScalable, OMX_VIDEO_MPEG4Level1  } },
            { 0b11111010, { XXX_MPEG4ProfileFineGranularityScalable, OMX_VIDEO_MPEG4Level2  } },
            { 0b11111011, { XXX_MPEG4ProfileFineGranularityScalable, OMX_VIDEO_MPEG4Level3  } },
            { 0b11111100, { XXX_MPEG4ProfileFineGranularityScalable, OMX_VIDEO_MPEG4Level4  } },
            { 0b11111101, { XXX_MPEG4ProfileFineGranularityScalable, OMX_VIDEO_MPEG4Level5  } }, */
        };

        std::pair<OMX_VIDEO_MPEG4PROFILETYPE, OMX_VIDEO_MPEG4LEVELTYPE> profileLevel;
        if (table.map(indication, &profileLevel)) {
            format->setInt32("profile", profileLevel.first);
            format->setInt32("level", profileLevel.second);
        }
    }
}

static void parseVp9ProfileLevelFromCsd(const sp<ABuffer> &csd, sp<AMessage> &format) {
    const uint8_t *data = csd->data();
    size_t remaining = csd->size();

    while (remaining >= 2) {
        const uint8_t id = data[0];
        const uint8_t length = data[1];
        remaining -= 2;
        data += 2;
        if (length > remaining) {
            break;
        }
        switch (id) {
            case 1 /* profileId */:
                if (length >= 1) {
                    const static ALookup<uint8_t, OMX_VIDEO_VP9PROFILETYPE> profiles {
                        { 0, OMX_VIDEO_VP9Profile0 },
                        { 1, OMX_VIDEO_VP9Profile1 },
                        { 2, OMX_VIDEO_VP9Profile2 },
                        { 3, OMX_VIDEO_VP9Profile3 },
                    };

                    const static ALookup<OMX_VIDEO_VP9PROFILETYPE, OMX_VIDEO_VP9PROFILETYPE> toHdr {
                        { OMX_VIDEO_VP9Profile2, OMX_VIDEO_VP9Profile2HDR },
                        { OMX_VIDEO_VP9Profile3, OMX_VIDEO_VP9Profile3HDR },
                    };

                    OMX_VIDEO_VP9PROFILETYPE profile;
                    if (profiles.map(data[0], &profile)) {
                        // convert to HDR profile
                        if (isHdr(format)) {
                            toHdr.lookup(profile, &profile);
                        }

                        format->setInt32("profile", profile);
                    }
                }
                break;
            case 2 /* levelId */:
                if (length >= 1) {
                    const static ALookup<uint8_t, OMX_VIDEO_VP9LEVELTYPE> levels {
                        { 10, OMX_VIDEO_VP9Level1  },
                        { 11, OMX_VIDEO_VP9Level11 },
                        { 20, OMX_VIDEO_VP9Level2  },
                        { 21, OMX_VIDEO_VP9Level21 },
                        { 30, OMX_VIDEO_VP9Level3  },
                        { 31, OMX_VIDEO_VP9Level31 },
                        { 40, OMX_VIDEO_VP9Level4  },
                        { 41, OMX_VIDEO_VP9Level41 },
                        { 50, OMX_VIDEO_VP9Level5  },
                        { 51, OMX_VIDEO_VP9Level51 },
                        { 52, OMX_VIDEO_VP9Level52 },
                        { 60, OMX_VIDEO_VP9Level6  },
                        { 61, OMX_VIDEO_VP9Level61 },
                        { 62, OMX_VIDEO_VP9Level62 },
                    };

                    OMX_VIDEO_VP9LEVELTYPE level;
                    if (levels.map(data[0], &level)) {
                        format->setInt32("level", level);
                    }
                }
                break;
            default:
                break;
        }
        remaining -= length;
        data += length;
    }
}

static void parseAV1ProfileLevelFromCsd(const sp<ABuffer> &csd, sp<AMessage> &format) {
    // Parse CSD structure to extract profile level information
    // https://aomediacodec.github.io/av1-isobmff/#av1codecconfigurationbox
    const uint8_t *data = csd->data();
    size_t remaining = csd->size();
    if (remaining < 4 || data[0] != 0x81) {  // configurationVersion == 1
        return;
    }
    uint8_t profileData = (data[1] & 0xE0) >> 5;
    uint8_t levelData = data[1] & 0x1F;
    uint8_t highBitDepth = (data[2] & 0x40) >> 6;

    const static ALookup<std::pair<uint8_t, uint8_t>, int32_t> profiles {
        { { 0, 0 }, AV1ProfileMain8 },
        { { 1, 0 }, AV1ProfileMain10 },
    };

    int32_t profile;
    if (profiles.map(std::make_pair(highBitDepth, profileData), &profile)) {
        // bump to HDR profile
        if (isHdr(format) && profile == AV1ProfileMain10) {
            if (format->contains("hdr10-plus-info")) {
                profile = AV1ProfileMain10HDR10Plus;
            } else {
                profile = AV1ProfileMain10HDR10;
            }
        }
        format->setInt32("profile", profile);
    }
    const static ALookup<uint8_t, int32_t> levels {
        { 0, AV1Level2   },
        { 1, AV1Level21  },
        { 2, AV1Level22  },
        { 3, AV1Level23  },
        { 4, AV1Level3   },
        { 5, AV1Level31  },
        { 6, AV1Level32  },
        { 7, AV1Level33  },
        { 8, AV1Level4   },
        { 9, AV1Level41  },
        { 10, AV1Level42  },
        { 11, AV1Level43  },
        { 12, AV1Level5   },
        { 13, AV1Level51  },
        { 14, AV1Level52  },
        { 15, AV1Level53  },
        { 16, AV1Level6   },
        { 17, AV1Level61  },
        { 18, AV1Level62  },
        { 19, AV1Level63  },
        { 20, AV1Level7   },
        { 21, AV1Level71  },
        { 22, AV1Level72  },
        { 23, AV1Level73  },
    };

    int32_t level;
    if (levels.map(levelData, &level)) {
        format->setInt32("level", level);
    }
}


static std::vector<std::pair<const char *, uint32_t>> stringMappings {
    {
        { "album", kKeyAlbum },
        { "albumartist", kKeyAlbumArtist },
        { "artist", kKeyArtist },
        { "author", kKeyAuthor },
        { "cdtracknum", kKeyCDTrackNumber },
        { "compilation", kKeyCompilation },
        { "composer", kKeyComposer },
        { "date", kKeyDate },
        { "discnum", kKeyDiscNumber },
        { "genre", kKeyGenre },
        { "location", kKeyLocation },
        { "lyricist", kKeyWriter },
        { "manufacturer", kKeyManufacturer },
        { "title", kKeyTitle },
        { "year", kKeyYear },
    }
};

static std::vector<std::pair<const char *, uint32_t>> floatMappings {
    {
        { "capture-rate", kKeyCaptureFramerate },
    }
};

static std::vector<std::pair<const char *, uint32_t>> int64Mappings {
    {
        { "exif-offset", kKeyExifOffset },
        { "exif-size", kKeyExifSize },
        { "target-time", kKeyTargetTime },
        { "thumbnail-time", kKeyThumbnailTime },
        { "timeUs", kKeyTime },
        { "durationUs", kKeyDuration },
    }
};

static std::vector<std::pair<const char *, uint32_t>> int32Mappings {
    {
        { "loop", kKeyAutoLoop },
        { "time-scale", kKeyTimeScale },
        { "crypto-mode", kKeyCryptoMode },
        { "crypto-default-iv-size", kKeyCryptoDefaultIVSize },
        { "crypto-encrypted-byte-block", kKeyEncryptedByteBlock },
        { "crypto-skip-byte-block", kKeySkipByteBlock },
        { "frame-count", kKeyFrameCount },
        { "max-bitrate", kKeyMaxBitRate },
        { "pcm-big-endian", kKeyPcmBigEndian },
        { "temporal-layer-count", kKeyTemporalLayerCount },
        { "temporal-layer-id", kKeyTemporalLayerId },
        { "thumbnail-width", kKeyThumbnailWidth },
        { "thumbnail-height", kKeyThumbnailHeight },
        { "valid-samples", kKeyValidSamples },
    }
};

static std::vector<std::pair<const char *, uint32_t>> bufferMappings {
    {
        { "albumart", kKeyAlbumArt },
        { "audio-presentation-info", kKeyAudioPresentationInfo },
        { "pssh", kKeyPssh },
        { "crypto-iv", kKeyCryptoIV },
        { "crypto-key", kKeyCryptoKey },
        { "crypto-encrypted-sizes", kKeyEncryptedSizes },
        { "crypto-plain-sizes", kKeyPlainSizes },
        { "icc-profile", kKeyIccProfile },
        { "sei", kKeySEI },
        { "text-format-data", kKeyTextFormatData },
        { "thumbnail-csd-hevc", kKeyThumbnailHVCC },
    }
};

static std::vector<std::pair<const char *, uint32_t>> CSDMappings {
    {
        { "csd-0", kKeyOpaqueCSD0 },
        { "csd-1", kKeyOpaqueCSD1 },
        { "csd-2", kKeyOpaqueCSD2 },
    }
};

void convertMessageToMetaDataFromMappings(const sp<AMessage> &msg, sp<MetaData> &meta) {
    for (auto elem : stringMappings) {
        AString value;
        if (msg->findString(elem.first, &value)) {
            meta->setCString(elem.second, value.c_str());
        }
    }

    for (auto elem : floatMappings) {
        float value;
        if (msg->findFloat(elem.first, &value)) {
            meta->setFloat(elem.second, value);
        }
    }

    for (auto elem : int64Mappings) {
        int64_t value;
        if (msg->findInt64(elem.first, &value)) {
            meta->setInt64(elem.second, value);
        }
    }

    for (auto elem : int32Mappings) {
        int32_t value;
        if (msg->findInt32(elem.first, &value)) {
            meta->setInt32(elem.second, value);
        }
    }

    for (auto elem : bufferMappings) {
        sp<ABuffer> value;
        if (msg->findBuffer(elem.first, &value)) {
            meta->setData(elem.second,
                    MetaDataBase::Type::TYPE_NONE, value->data(), value->size());
        }
    }

    for (auto elem : CSDMappings) {
        sp<ABuffer> value;
        if (msg->findBuffer(elem.first, &value)) {
            meta->setData(elem.second,
                    MetaDataBase::Type::TYPE_NONE, value->data(), value->size());
        }
    }
}

void convertMetaDataToMessageFromMappings(const MetaDataBase *meta, sp<AMessage> format) {
    for (auto elem : stringMappings) {
        const char *value;
        if (meta->findCString(elem.second, &value)) {
            format->setString(elem.first, value, strlen(value));
        }
    }

    for (auto elem : floatMappings) {
        float value;
        if (meta->findFloat(elem.second, &value)) {
            format->setFloat(elem.first, value);
        }
    }

    for (auto elem : int64Mappings) {
        int64_t value;
        if (meta->findInt64(elem.second, &value)) {
            format->setInt64(elem.first, value);
        }
    }

    for (auto elem : int32Mappings) {
        int32_t value;
        if (meta->findInt32(elem.second, &value)) {
            format->setInt32(elem.first, value);
        }
    }

    for (auto elem : bufferMappings) {
        uint32_t type;
        const void* data;
        size_t size;
        if (meta->findData(elem.second, &type, &data, &size)) {
            sp<ABuffer> buf = ABuffer::CreateAsCopy(data, size);
            format->setBuffer(elem.first, buf);
        }
    }

    for (auto elem : CSDMappings) {
        uint32_t type;
        const void* data;
        size_t size;
        if (meta->findData(elem.second, &type, &data, &size)) {
            sp<ABuffer> buf = ABuffer::CreateAsCopy(data, size);
            buf->meta()->setInt32("csd", true);
            buf->meta()->setInt64("timeUs", 0);
            format->setBuffer(elem.first, buf);
        }
    }
}

status_t convertMetaDataToMessage(
        const sp<MetaData> &meta, sp<AMessage> *format) {
    return convertMetaDataToMessage(meta.get(), format);
}

status_t convertMetaDataToMessage(
        const MetaDataBase *meta, sp<AMessage> *format) {

    format->clear();

    if (meta == NULL) {
        ALOGE("convertMetaDataToMessage: NULL input");
        return BAD_VALUE;
    }

    const char *mime;
    if (!meta->findCString(kKeyMIMEType, &mime)) {
        return BAD_VALUE;
    }

    sp<AMessage> msg = new AMessage;
    msg->setString("mime", mime);

    convertMetaDataToMessageFromMappings(meta, msg);

    uint32_t type;
    const void *data;
    size_t size;
    if (meta->findData(kKeyCASessionID, &type, &data, &size)) {
        sp<ABuffer> buffer = new (std::nothrow) ABuffer(size);
        if (buffer.get() == NULL || buffer->base() == NULL) {
            return NO_MEMORY;
        }

        msg->setBuffer("ca-session-id", buffer);
        memcpy(buffer->data(), data, size);
    }

    if (meta->findData(kKeyCAPrivateData, &type, &data, &size)) {
        sp<ABuffer> buffer = new (std::nothrow) ABuffer(size);
        if (buffer.get() == NULL || buffer->base() == NULL) {
            return NO_MEMORY;
        }

        msg->setBuffer("ca-private-data", buffer);
        memcpy(buffer->data(), data, size);
    }

    int32_t systemId;
    if (meta->findInt32(kKeyCASystemID, &systemId)) {
        msg->setInt32("ca-system-id", systemId);
    }

    if (!strncasecmp("video/scrambled", mime, 15)
            || !strncasecmp("audio/scrambled", mime, 15)) {

        *format = msg;
        return OK;
    }

    int64_t durationUs;
    if (meta->findInt64(kKeyDuration, &durationUs)) {
        msg->setInt64("durationUs", durationUs);
    }

    int32_t avgBitRate = 0;
    if (meta->findInt32(kKeyBitRate, &avgBitRate) && avgBitRate > 0) {
        msg->setInt32("bitrate", avgBitRate);
    }

    int32_t maxBitRate;
    if (meta->findInt32(kKeyMaxBitRate, &maxBitRate)
            && maxBitRate > 0 && maxBitRate >= avgBitRate) {
        msg->setInt32("max-bitrate", maxBitRate);
    }

    int32_t isSync;
    if (meta->findInt32(kKeyIsSyncFrame, &isSync) && isSync != 0) {
        msg->setInt32("is-sync-frame", 1);
    }

    // this only needs to be translated from meta to message as it is an extractor key
    int32_t trackID;
    if (meta->findInt32(kKeyTrackID, &trackID)) {
        msg->setInt32("track-id", trackID);
    }

    const char *lang;
    if (meta->findCString(kKeyMediaLanguage, &lang)) {
        msg->setString("language", lang);
    }

    if (!strncasecmp("video/", mime, 6) ||
            !strncasecmp("image/", mime, 6)) {
        int32_t width, height;
        if (!meta->findInt32(kKeyWidth, &width)
                || !meta->findInt32(kKeyHeight, &height)) {
            return BAD_VALUE;
        }

        msg->setInt32("width", width);
        msg->setInt32("height", height);

        int32_t displayWidth, displayHeight;
        if (meta->findInt32(kKeyDisplayWidth, &displayWidth)
                && meta->findInt32(kKeyDisplayHeight, &displayHeight)) {
            msg->setInt32("display-width", displayWidth);
            msg->setInt32("display-height", displayHeight);
        }

        int32_t sarWidth, sarHeight;
        if (meta->findInt32(kKeySARWidth, &sarWidth)
                && meta->findInt32(kKeySARHeight, &sarHeight)) {
            msg->setInt32("sar-width", sarWidth);
            msg->setInt32("sar-height", sarHeight);
        }

        if (!strncasecmp("image/", mime, 6)) {
            int32_t tileWidth, tileHeight, gridRows, gridCols;
            if (meta->findInt32(kKeyTileWidth, &tileWidth)
                    && meta->findInt32(kKeyTileHeight, &tileHeight)
                    && meta->findInt32(kKeyGridRows, &gridRows)
                    && meta->findInt32(kKeyGridCols, &gridCols)) {
                msg->setInt32("tile-width", tileWidth);
                msg->setInt32("tile-height", tileHeight);
                msg->setInt32("grid-rows", gridRows);
                msg->setInt32("grid-cols", gridCols);
            }
            int32_t isPrimary;
            if (meta->findInt32(kKeyTrackIsDefault, &isPrimary) && isPrimary) {
                msg->setInt32("is-default", 1);
            }
        }

        int32_t colorFormat;
        if (meta->findInt32(kKeyColorFormat, &colorFormat)) {
            msg->setInt32("color-format", colorFormat);
        }

        int32_t cropLeft, cropTop, cropRight, cropBottom;
        if (meta->findRect(kKeyCropRect,
                           &cropLeft,
                           &cropTop,
                           &cropRight,
                           &cropBottom)) {
            msg->setRect("crop", cropLeft, cropTop, cropRight, cropBottom);
        }

        int32_t rotationDegrees;
        if (meta->findInt32(kKeyRotation, &rotationDegrees)) {
            msg->setInt32("rotation-degrees", rotationDegrees);
        }

        uint32_t type;
        const void *data;
        size_t size;
        if (meta->findData(kKeyHdrStaticInfo, &type, &data, &size)
                && type == 'hdrS' && size == sizeof(HDRStaticInfo)) {
            ColorUtils::setHDRStaticInfoIntoFormat(*(HDRStaticInfo*)data, msg);
        }

        if (meta->findData(kKeyHdr10PlusInfo, &type, &data, &size)
                && size > 0) {
            sp<ABuffer> buffer = new (std::nothrow) ABuffer(size);
            if (buffer.get() == NULL || buffer->base() == NULL) {
                return NO_MEMORY;
            }
            memcpy(buffer->data(), data, size);
            msg->setBuffer("hdr10-plus-info", buffer);
        }

        convertMetaDataToMessageColorAspects(meta, msg);
    } else if (!strncasecmp("audio/", mime, 6)) {
        int32_t numChannels, sampleRate;
        if (!meta->findInt32(kKeyChannelCount, &numChannels)
                || !meta->findInt32(kKeySampleRate, &sampleRate)) {
            return BAD_VALUE;
        }

        msg->setInt32("channel-count", numChannels);
        msg->setInt32("sample-rate", sampleRate);

        int32_t bitsPerSample;
        if (meta->findInt32(kKeyBitsPerSample, &bitsPerSample)) {
            msg->setInt32("bits-per-sample", bitsPerSample);
        }

        int32_t channelMask;
        if (meta->findInt32(kKeyChannelMask, &channelMask)) {
            msg->setInt32("channel-mask", channelMask);
        }

        int32_t delay = 0;
        if (meta->findInt32(kKeyEncoderDelay, &delay)) {
            msg->setInt32("encoder-delay", delay);
        }
        int32_t padding = 0;
        if (meta->findInt32(kKeyEncoderPadding, &padding)) {
            msg->setInt32("encoder-padding", padding);
        }

        int32_t isADTS;
        if (meta->findInt32(kKeyIsADTS, &isADTS)) {
            msg->setInt32("is-adts", isADTS);
        }

        int32_t aacProfile = -1;
        if (meta->findInt32(kKeyAACAOT, &aacProfile)) {
            msg->setInt32("aac-profile", aacProfile);
        }

        int32_t pcmEncoding;
        if (meta->findInt32(kKeyPcmEncoding, &pcmEncoding)) {
            msg->setInt32("pcm-encoding", pcmEncoding);
        }

        int32_t hapticChannelCount;
        if (meta->findInt32(kKeyHapticChannelCount, &hapticChannelCount)) {
            msg->setInt32("haptic-channel-count", hapticChannelCount);
        }
    }

    int32_t maxInputSize;
    if (meta->findInt32(kKeyMaxInputSize, &maxInputSize)) {
        msg->setInt32("max-input-size", maxInputSize);
    }

    int32_t maxWidth;
    if (meta->findInt32(kKeyMaxWidth, &maxWidth)) {
        msg->setInt32("max-width", maxWidth);
    }

    int32_t maxHeight;
    if (meta->findInt32(kKeyMaxHeight, &maxHeight)) {
        msg->setInt32("max-height", maxHeight);
    }

    int32_t rotationDegrees;
    if (meta->findInt32(kKeyRotation, &rotationDegrees)) {
        msg->setInt32("rotation-degrees", rotationDegrees);
    }

    int32_t fps;
    if (meta->findInt32(kKeyFrameRate, &fps) && fps > 0) {
        msg->setInt32("frame-rate", fps);
    }

    if (meta->findData(kKeyAVCC, &type, &data, &size)) {
        // Parse the AVCDecoderConfigurationRecord

        const uint8_t *ptr = (const uint8_t *)data;

        if (size < 7 || ptr[0] != 1) {  // configurationVersion == 1
            ALOGE("b/23680780");
            return BAD_VALUE;
        }

        parseAvcProfileLevelFromAvcc(ptr, size, msg);

        // There is decodable content out there that fails the following
        // assertion, let's be lenient for now...
        // CHECK((ptr[4] >> 2) == 0x3f);  // reserved

        size_t lengthSize __unused = 1 + (ptr[4] & 3);

        // commented out check below as H264_QVGA_500_NO_AUDIO.3gp
        // violates it...
        // CHECK((ptr[5] >> 5) == 7);  // reserved

        size_t numSeqParameterSets = ptr[5] & 31;

        ptr += 6;
        size -= 6;

        sp<ABuffer> buffer = new (std::nothrow) ABuffer(1024);
        if (buffer.get() == NULL || buffer->base() == NULL) {
            return NO_MEMORY;
        }
        buffer->setRange(0, 0);

        for (size_t i = 0; i < numSeqParameterSets; ++i) {
            if (size < 2) {
                ALOGE("b/23680780");
                return BAD_VALUE;
            }
            size_t length = U16_AT(ptr);

            ptr += 2;
            size -= 2;

            if (size < length) {
                return BAD_VALUE;
            }
            status_t err = copyNALUToABuffer(&buffer, ptr, length);
            if (err != OK) {
                return err;
            }

            ptr += length;
            size -= length;
        }

        buffer->meta()->setInt32("csd", true);
        buffer->meta()->setInt64("timeUs", 0);

        msg->setBuffer("csd-0", buffer);

        buffer = new (std::nothrow) ABuffer(1024);
        if (buffer.get() == NULL || buffer->base() == NULL) {
            return NO_MEMORY;
        }
        buffer->setRange(0, 0);

        if (size < 1) {
            ALOGE("b/23680780");
            return BAD_VALUE;
        }
        size_t numPictureParameterSets = *ptr;
        ++ptr;
        --size;

        for (size_t i = 0; i < numPictureParameterSets; ++i) {
            if (size < 2) {
                ALOGE("b/23680780");
                return BAD_VALUE;
            }
            size_t length = U16_AT(ptr);

            ptr += 2;
            size -= 2;

            if (size < length) {
                return BAD_VALUE;
            }
            status_t err = copyNALUToABuffer(&buffer, ptr, length);
            if (err != OK) {
                return err;
            }

            ptr += length;
            size -= length;
        }

        buffer->meta()->setInt32("csd", true);
        buffer->meta()->setInt64("timeUs", 0);
        msg->setBuffer("csd-1", buffer);
    } else if (meta->findData(kKeyHVCC, &type, &data, &size)) {
        const uint8_t *ptr = (const uint8_t *)data;

        if (size < 23 || (ptr[0] != 1 && ptr[0] != 0)) {
            // configurationVersion == 1 or 0
            // 1 is what the standard dictates, but some old muxers may have used 0.
            ALOGE("b/23680780");
            return BAD_VALUE;
        }

        const size_t dataSize = size; // save for later
        ptr += 22;
        size -= 22;

        size_t numofArrays = (char)ptr[0];
        ptr += 1;
        size -= 1;
        size_t j = 0, i = 0;

        sp<ABuffer> buffer = new (std::nothrow) ABuffer(1024);
        if (buffer.get() == NULL || buffer->base() == NULL) {
            return NO_MEMORY;
        }
        buffer->setRange(0, 0);

        HevcParameterSets hvcc;

        for (i = 0; i < numofArrays; i++) {
            if (size < 3) {
                ALOGE("b/23680780");
                return BAD_VALUE;
            }
            ptr += 1;
            size -= 1;

            //Num of nals
            size_t numofNals = U16_AT(ptr);

            ptr += 2;
            size -= 2;

            for (j = 0; j < numofNals; j++) {
                if (size < 2) {
                    ALOGE("b/23680780");
                    return BAD_VALUE;
                }
                size_t length = U16_AT(ptr);

                ptr += 2;
                size -= 2;

                if (size < length) {
                    return BAD_VALUE;
                }
                status_t err = copyNALUToABuffer(&buffer, ptr, length);
                if (err != OK) {
                    return err;
                }
                (void)hvcc.addNalUnit(ptr, length);

                ptr += length;
                size -= length;
            }
        }
        buffer->meta()->setInt32("csd", true);
        buffer->meta()->setInt64("timeUs", 0);
        msg->setBuffer("csd-0", buffer);

        // if we saw VUI color information we know whether this is HDR because VUI trumps other
        // format parameters for HEVC.
        HevcParameterSets::Info info = hvcc.getInfo();
        if (info & hvcc.kInfoHasColorDescription) {
            msg->setInt32("android._is-hdr", (info & hvcc.kInfoIsHdr) != 0);
        }

        uint32_t isoPrimaries, isoTransfer, isoMatrix, isoRange;
        if (hvcc.findParam32(kColourPrimaries, &isoPrimaries)
                && hvcc.findParam32(kTransferCharacteristics, &isoTransfer)
                && hvcc.findParam32(kMatrixCoeffs, &isoMatrix)
                && hvcc.findParam32(kVideoFullRangeFlag, &isoRange)) {
            ALOGV("found iso color aspects : primaris=%d, transfer=%d, matrix=%d, range=%d",
                    isoPrimaries, isoTransfer, isoMatrix, isoRange);

            ColorAspects aspects;
            ColorUtils::convertIsoColorAspectsToCodecAspects(
                    isoPrimaries, isoTransfer, isoMatrix, isoRange, aspects);

            if (aspects.mPrimaries == ColorAspects::PrimariesUnspecified) {
                int32_t primaries;
                if (meta->findInt32(kKeyColorPrimaries, &primaries)) {
                    ALOGV("unspecified primaries found, replaced to %d", primaries);
                    aspects.mPrimaries = static_cast<ColorAspects::Primaries>(primaries);
                }
            }
            if (aspects.mTransfer == ColorAspects::TransferUnspecified) {
                int32_t transferFunction;
                if (meta->findInt32(kKeyTransferFunction, &transferFunction)) {
                    ALOGV("unspecified transfer found, replaced to %d", transferFunction);
                    aspects.mTransfer = static_cast<ColorAspects::Transfer>(transferFunction);
                }
            }
            if (aspects.mMatrixCoeffs == ColorAspects::MatrixUnspecified) {
                int32_t colorMatrix;
                if (meta->findInt32(kKeyColorMatrix, &colorMatrix)) {
                    ALOGV("unspecified matrix found, replaced to %d", colorMatrix);
                    aspects.mMatrixCoeffs = static_cast<ColorAspects::MatrixCoeffs>(colorMatrix);
                }
            }
            if (aspects.mRange == ColorAspects::RangeUnspecified) {
                int32_t range;
                if (meta->findInt32(kKeyColorRange, &range)) {
                    ALOGV("unspecified range found, replaced to %d", range);
                    aspects.mRange = static_cast<ColorAspects::Range>(range);
                }
            }

            int32_t standard, transfer, range;
            if (ColorUtils::convertCodecColorAspectsToPlatformAspects(
                    aspects, &range, &standard, &transfer) == OK) {
                msg->setInt32("color-standard", standard);
                msg->setInt32("color-transfer", transfer);
                msg->setInt32("color-range", range);
            }
        }

        parseHevcProfileLevelFromHvcc((const uint8_t *)data, dataSize, msg);
    } else if (meta->findData(kKeyAV1C, &type, &data, &size)) {
        sp<ABuffer> buffer = new (std::nothrow) ABuffer(size);
        if (buffer.get() == NULL || buffer->base() == NULL) {
            return NO_MEMORY;
        }
        memcpy(buffer->data(), data, size);

        buffer->meta()->setInt32("csd", true);
        buffer->meta()->setInt64("timeUs", 0);
        msg->setBuffer("csd-0", buffer);
        parseAV1ProfileLevelFromCsd(buffer, msg);
    } else if (meta->findData(kKeyESDS, &type, &data, &size)) {
        ESDS esds((const char *)data, size);
        if (esds.InitCheck() != (status_t)OK) {
            return BAD_VALUE;
        }

        const void *codec_specific_data;
        size_t codec_specific_data_size;
        esds.getCodecSpecificInfo(
                &codec_specific_data, &codec_specific_data_size);

        sp<ABuffer> buffer = new (std::nothrow) ABuffer(codec_specific_data_size);
        if (buffer.get() == NULL || buffer->base() == NULL) {
            return NO_MEMORY;
        }

        memcpy(buffer->data(), codec_specific_data,
               codec_specific_data_size);

        buffer->meta()->setInt32("csd", true);
        buffer->meta()->setInt64("timeUs", 0);
        msg->setBuffer("csd-0", buffer);

        if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_MPEG4)) {
            parseMpeg4ProfileLevelFromCsd(buffer, msg);
        } else if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_MPEG2)) {
            parseMpeg2ProfileLevelFromEsds(esds, msg);
            if (meta->findData(kKeyStreamHeader, &type, &data, &size)) {
                parseMpeg2ProfileLevelFromHeader((uint8_t*)data, size, msg);
            }
        } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AAC)) {
            parseAacProfileFromCsd(buffer, msg);
        }

        uint32_t maxBitrate, avgBitrate;
        if (esds.getBitRate(&maxBitrate, &avgBitrate) == OK) {
            if (!meta->hasData(kKeyBitRate)
                    && avgBitrate > 0 && avgBitrate <= INT32_MAX) {
                msg->setInt32("bitrate", (int32_t)avgBitrate);
            } else {
                (void)msg->findInt32("bitrate", (int32_t*)&avgBitrate);
            }
            if (!meta->hasData(kKeyMaxBitRate)
                    && maxBitrate > 0 && maxBitrate <= INT32_MAX && maxBitrate >= avgBitrate) {
                msg->setInt32("max-bitrate", (int32_t)maxBitrate);
            }
        }
    } else if (meta->findData(kKeyD263, &type, &data, &size)) {
        const uint8_t *ptr = (const uint8_t *)data;
        parseH263ProfileLevelFromD263(ptr, size, msg);
    } else if (meta->findData(kKeyOpusHeader, &type, &data, &size)) {
        sp<ABuffer> buffer = new (std::nothrow) ABuffer(size);
        if (buffer.get() == NULL || buffer->base() == NULL) {
            return NO_MEMORY;
        }
        memcpy(buffer->data(), data, size);

        buffer->meta()->setInt32("csd", true);
        buffer->meta()->setInt64("timeUs", 0);
        msg->setBuffer("csd-0", buffer);

        if (!meta->findData(kKeyOpusCodecDelay, &type, &data, &size)) {
            return -EINVAL;
        }

        buffer = new (std::nothrow) ABuffer(size);
        if (buffer.get() == NULL || buffer->base() == NULL) {
            return NO_MEMORY;
        }
        memcpy(buffer->data(), data, size);

        buffer->meta()->setInt32("csd", true);
        buffer->meta()->setInt64("timeUs", 0);
        msg->setBuffer("csd-1", buffer);

        if (!meta->findData(kKeyOpusSeekPreRoll, &type, &data, &size)) {
            return -EINVAL;
        }

        buffer = new (std::nothrow) ABuffer(size);
        if (buffer.get() == NULL || buffer->base() == NULL) {
            return NO_MEMORY;
        }
        memcpy(buffer->data(), data, size);

        buffer->meta()->setInt32("csd", true);
        buffer->meta()->setInt64("timeUs", 0);
        msg->setBuffer("csd-2", buffer);
    } else if (meta->findData(kKeyVp9CodecPrivate, &type, &data, &size)) {
        sp<ABuffer> buffer = new (std::nothrow) ABuffer(size);
        if (buffer.get() == NULL || buffer->base() == NULL) {
            return NO_MEMORY;
        }
        memcpy(buffer->data(), data, size);

        buffer->meta()->setInt32("csd", true);
        buffer->meta()->setInt64("timeUs", 0);
        msg->setBuffer("csd-0", buffer);

        parseVp9ProfileLevelFromCsd(buffer, msg);
    } else if (meta->findData(kKeyAlacMagicCookie, &type, &data, &size)) {
        ALOGV("convertMetaDataToMessage found kKeyAlacMagicCookie of size %zu\n", size);
        sp<ABuffer> buffer = new (std::nothrow) ABuffer(size);
        if (buffer.get() == NULL || buffer->base() == NULL) {
            return NO_MEMORY;
        }
        memcpy(buffer->data(), data, size);

        buffer->meta()->setInt32("csd", true);
        buffer->meta()->setInt64("timeUs", 0);
        msg->setBuffer("csd-0", buffer);
    }

    *format = msg;

    return OK;
}

const uint8_t *findNextNalStartCode(const uint8_t *data, size_t length) {
    uint8_t *res = NULL;
    if (length > 4) {
        // minus 1 as to not match NAL start code at end
        res = (uint8_t *)memmem(data, length - 1, "\x00\x00\x00\x01", 4);
    }
    return res != NULL && res < data + length - 4 ? res : &data[length];
}

static size_t reassembleAVCC(const sp<ABuffer> &csd0, const sp<ABuffer> &csd1, char *avcc) {
    avcc[0] = 1;        // version
    avcc[1] = 0x64;     // profile (default to high)
    avcc[2] = 0;        // constraints (default to none)
    avcc[3] = 0xd;      // level (default to 1.3)
    avcc[4] = 0xff;     // reserved+size

    size_t i = 0;
    int numparams = 0;
    int lastparamoffset = 0;
    int avccidx = 6;
    do {
        i = findNextNalStartCode(csd0->data() + i, csd0->size() - i) - csd0->data();
        ALOGV("block at %zu, last was %d", i, lastparamoffset);
        if (lastparamoffset > 0) {
            const uint8_t *lastparam = csd0->data() + lastparamoffset;
            int size = i - lastparamoffset;
            if (size > 3) {
                if (numparams && memcmp(avcc + 1, lastparam + 1, 3)) {
                    ALOGW("Inconsisted profile/level found in SPS: %x,%x,%x vs %x,%x,%x",
                            avcc[1], avcc[2], avcc[3], lastparam[1], lastparam[2], lastparam[3]);
                } else if (!numparams) {
                    // fill in profile, constraints and level
                    memcpy(avcc + 1, lastparam + 1, 3);
                }
            }
            avcc[avccidx++] = size >> 8;
            avcc[avccidx++] = size & 0xff;
            memcpy(avcc+avccidx, lastparam, size);
            avccidx += size;
            numparams++;
        }
        i += 4;
        lastparamoffset = i;
    } while(i < csd0->size());
    ALOGV("csd0 contains %d params", numparams);

    avcc[5] = 0xe0 | numparams;
    //and now csd-1
    i = 0;
    numparams = 0;
    lastparamoffset = 0;
    int numpicparamsoffset = avccidx;
    avccidx++;
    do {
        i = findNextNalStartCode(csd1->data() + i, csd1->size() - i) - csd1->data();
        ALOGV("block at %zu, last was %d", i, lastparamoffset);
        if (lastparamoffset > 0) {
            int size = i - lastparamoffset;
            avcc[avccidx++] = size >> 8;
            avcc[avccidx++] = size & 0xff;
            memcpy(avcc+avccidx, csd1->data() + lastparamoffset, size);
            avccidx += size;
            numparams++;
        }
        i += 4;
        lastparamoffset = i;
    } while(i < csd1->size());
    avcc[numpicparamsoffset] = numparams;
    return avccidx;
}

static void reassembleESDS(const sp<ABuffer> &csd0, char *esds) {
    int csd0size = csd0->size();
    esds[0] = 3; // kTag_ESDescriptor;
    int esdescriptorsize = 26 + csd0size;
    CHECK(esdescriptorsize < 268435456); // 7 bits per byte, so max is 2^28-1
    esds[1] = 0x80 | (esdescriptorsize >> 21);
    esds[2] = 0x80 | ((esdescriptorsize >> 14) & 0x7f);
    esds[3] = 0x80 | ((esdescriptorsize >> 7) & 0x7f);
    esds[4] = (esdescriptorsize & 0x7f);
    esds[5] = esds[6] = 0; // es id
    esds[7] = 0; // flags
    esds[8] = 4; // kTag_DecoderConfigDescriptor
    int configdescriptorsize = 18 + csd0size;
    esds[9] = 0x80 | (configdescriptorsize >> 21);
    esds[10] = 0x80 | ((configdescriptorsize >> 14) & 0x7f);
    esds[11] = 0x80 | ((configdescriptorsize >> 7) & 0x7f);
    esds[12] = (configdescriptorsize & 0x7f);
    esds[13] = 0x40; // objectTypeIndication
    // bytes 14-25 are examples from a real file. they are unused/overwritten by muxers.
    esds[14] = 0x15; // streamType(5), upStream(0),
    esds[15] = 0x00; // 15-17: bufferSizeDB (6KB)
    esds[16] = 0x18;
    esds[17] = 0x00;
    esds[18] = 0x00; // 18-21: maxBitrate (64kbps)
    esds[19] = 0x00;
    esds[20] = 0xfa;
    esds[21] = 0x00;
    esds[22] = 0x00; // 22-25: avgBitrate (64kbps)
    esds[23] = 0x00;
    esds[24] = 0xfa;
    esds[25] = 0x00;
    esds[26] = 5; // kTag_DecoderSpecificInfo;
    esds[27] = 0x80 | (csd0size >> 21);
    esds[28] = 0x80 | ((csd0size >> 14) & 0x7f);
    esds[29] = 0x80 | ((csd0size >> 7) & 0x7f);
    esds[30] = (csd0size & 0x7f);
    memcpy((void*)&esds[31], csd0->data(), csd0size);
    // data following this is ignored, so don't bother appending it
}

static size_t reassembleHVCC(const sp<ABuffer> &csd0, uint8_t *hvcc, size_t hvccSize, size_t nalSizeLength) {
    HevcParameterSets paramSets;
    uint8_t* data = csd0->data();
    if (csd0->size() < 4) {
        ALOGE("csd0 too small");
        return 0;
    }
    if (memcmp(data, "\x00\x00\x00\x01", 4) != 0) {
        ALOGE("csd0 doesn't start with a start code");
        return 0;
    }
    size_t prevNalOffset = 4;
    status_t err = OK;
    for (size_t i = 1; i < csd0->size() - 4; ++i) {
        if (memcmp(&data[i], "\x00\x00\x00\x01", 4) != 0) {
            continue;
        }
        err = paramSets.addNalUnit(&data[prevNalOffset], i - prevNalOffset);
        if (err != OK) {
            return 0;
        }
        prevNalOffset = i + 4;
    }
    err = paramSets.addNalUnit(&data[prevNalOffset], csd0->size() - prevNalOffset);
    if (err != OK) {
        return 0;
    }
    size_t size = hvccSize;
    err = paramSets.makeHvcc(hvcc, &size, nalSizeLength);
    if (err != OK) {
        return 0;
    }
    return size;
}

#if 0
static void convertMessageToMetaDataInt32(
        const sp<AMessage> &msg, sp<MetaData> &meta, uint32_t key, const char *name) {
    int32_t value;
    if (msg->findInt32(name, &value)) {
        meta->setInt32(key, value);
    }
}
#endif

static void convertMessageToMetaDataColorAspects(const sp<AMessage> &msg, sp<MetaData> &meta) {
    // 0 values are unspecified
    int32_t range = 0, standard = 0, transfer = 0;
    (void)msg->findInt32("color-range", &range);
    (void)msg->findInt32("color-standard", &standard);
    (void)msg->findInt32("color-transfer", &transfer);

    ColorAspects colorAspects;
    memset(&colorAspects, 0, sizeof(colorAspects));
    if (CodecBase::convertPlatformColorAspectsToCodecAspects(
            range, standard, transfer, colorAspects) != OK) {
        return;
    }

    // save specified values to meta
    if (colorAspects.mRange != 0) {
        meta->setInt32(kKeyColorRange, colorAspects.mRange);
    }
    if (colorAspects.mPrimaries != 0) {
        meta->setInt32(kKeyColorPrimaries, colorAspects.mPrimaries);
    }
    if (colorAspects.mTransfer != 0) {
        meta->setInt32(kKeyTransferFunction, colorAspects.mTransfer);
    }
    if (colorAspects.mMatrixCoeffs != 0) {
        meta->setInt32(kKeyColorMatrix, colorAspects.mMatrixCoeffs);
    }
}

void convertMessageToMetaData(const sp<AMessage> &msg, sp<MetaData> &meta) {
    AString mime;
    if (msg->findString("mime", &mime)) {
        meta->setCString(kKeyMIMEType, mime.c_str());
    } else {
        ALOGW("did not find mime type");
    }

    convertMessageToMetaDataFromMappings(msg, meta);

    int32_t systemId;
    if (msg->findInt32("ca-system-id", &systemId)) {
        meta->setInt32(kKeyCASystemID, systemId);

        sp<ABuffer> caSessionId, caPvtData;
        if (msg->findBuffer("ca-session-id", &caSessionId)) {
            meta->setData(kKeyCASessionID, 0, caSessionId->data(), caSessionId->size());
        }
        if (msg->findBuffer("ca-private-data", &caPvtData)) {
            meta->setData(kKeyCAPrivateData, 0, caPvtData->data(), caPvtData->size());
        }
    }

    int64_t durationUs;
    if (msg->findInt64("durationUs", &durationUs)) {
        meta->setInt64(kKeyDuration, durationUs);
    }

    int32_t isSync;
    if (msg->findInt32("is-sync-frame", &isSync) && isSync != 0) {
        meta->setInt32(kKeyIsSyncFrame, 1);
    }

    int32_t avgBitrate = 0;
    int32_t maxBitrate;
    if (msg->findInt32("bitrate", &avgBitrate) && avgBitrate > 0) {
        meta->setInt32(kKeyBitRate, avgBitrate);
    }
    if (msg->findInt32("max-bitrate", &maxBitrate) && maxBitrate > 0 && maxBitrate >= avgBitrate) {
        meta->setInt32(kKeyMaxBitRate, maxBitrate);
    }

    AString lang;
    if (msg->findString("language", &lang)) {
        meta->setCString(kKeyMediaLanguage, lang.c_str());
    }

    if (mime.startsWith("video/") || mime.startsWith("image/")) {
        int32_t width;
        int32_t height;
        if (msg->findInt32("width", &width) && msg->findInt32("height", &height)) {
            meta->setInt32(kKeyWidth, width);
            meta->setInt32(kKeyHeight, height);
        } else {
            ALOGV("did not find width and/or height");
        }

        int32_t sarWidth, sarHeight;
        if (msg->findInt32("sar-width", &sarWidth)
                && msg->findInt32("sar-height", &sarHeight)) {
            meta->setInt32(kKeySARWidth, sarWidth);
            meta->setInt32(kKeySARHeight, sarHeight);
        }

        int32_t displayWidth, displayHeight;
        if (msg->findInt32("display-width", &displayWidth)
                && msg->findInt32("display-height", &displayHeight)) {
            meta->setInt32(kKeyDisplayWidth, displayWidth);
            meta->setInt32(kKeyDisplayHeight, displayHeight);
        }

        if (mime.startsWith("image/")){
            int32_t isPrimary;
            if (msg->findInt32("is-default", &isPrimary) && isPrimary) {
                meta->setInt32(kKeyTrackIsDefault, 1);
            }
            int32_t tileWidth, tileHeight, gridRows, gridCols;
            if (msg->findInt32("tile-width", &tileWidth)) {
                meta->setInt32(kKeyTileWidth, tileWidth);
            }
            if (msg->findInt32("tile-height", &tileHeight)) {
                meta->setInt32(kKeyTileHeight, tileHeight);
            }
            if (msg->findInt32("grid-rows", &gridRows)) {
                meta->setInt32(kKeyGridRows, gridRows);
            }
            if (msg->findInt32("grid-cols", &gridCols)) {
                meta->setInt32(kKeyGridCols, gridCols);
            }
        }

        int32_t colorFormat;
        if (msg->findInt32("color-format", &colorFormat)) {
            meta->setInt32(kKeyColorFormat, colorFormat);
        }

        int32_t cropLeft, cropTop, cropRight, cropBottom;
        if (msg->findRect("crop",
                          &cropLeft,
                          &cropTop,
                          &cropRight,
                          &cropBottom)) {
            meta->setRect(kKeyCropRect, cropLeft, cropTop, cropRight, cropBottom);
        }

        int32_t rotationDegrees;
        if (msg->findInt32("rotation-degrees", &rotationDegrees)) {
            meta->setInt32(kKeyRotation, rotationDegrees);
        }

        if (msg->contains("hdr-static-info")) {
            HDRStaticInfo info;
            if (ColorUtils::getHDRStaticInfoFromFormat(msg, &info)) {
                meta->setData(kKeyHdrStaticInfo, 'hdrS', &info, sizeof(info));
            }
        }

        sp<ABuffer> hdr10PlusInfo;
        if (msg->findBuffer("hdr10-plus-info", &hdr10PlusInfo)) {
            meta->setData(kKeyHdr10PlusInfo, 0,
                    hdr10PlusInfo->data(), hdr10PlusInfo->size());
        }

        convertMessageToMetaDataColorAspects(msg, meta);

        AString tsSchema;
        if (msg->findString("ts-schema", &tsSchema)) {
            unsigned int numLayers = 0;
            unsigned int numBLayers = 0;
            char dummy;
            int tags = sscanf(tsSchema.c_str(), "android.generic.%u%c%u%c",
                    &numLayers, &dummy, &numBLayers, &dummy);
            if ((tags == 1 || (tags == 3 && dummy == '+'))
                    && numLayers > 0 && numLayers < UINT32_MAX - numBLayers
                    && numLayers + numBLayers <= INT32_MAX) {
                meta->setInt32(kKeyTemporalLayerCount, numLayers + numBLayers);
            }
        }
    } else if (mime.startsWith("audio/")) {
        int32_t numChannels;
        if (msg->findInt32("channel-count", &numChannels)) {
            meta->setInt32(kKeyChannelCount, numChannels);
        }
        int32_t sampleRate;
        if (msg->findInt32("sample-rate", &sampleRate)) {
            meta->setInt32(kKeySampleRate, sampleRate);
        }
        int32_t bitsPerSample;
        if (msg->findInt32("bits-per-sample", &bitsPerSample)) {
            meta->setInt32(kKeyBitsPerSample, bitsPerSample);
        }
        int32_t channelMask;
        if (msg->findInt32("channel-mask", &channelMask)) {
            meta->setInt32(kKeyChannelMask, channelMask);
        }
        int32_t delay = 0;
        if (msg->findInt32("encoder-delay", &delay)) {
            meta->setInt32(kKeyEncoderDelay, delay);
        }
        int32_t padding = 0;
        if (msg->findInt32("encoder-padding", &padding)) {
            meta->setInt32(kKeyEncoderPadding, padding);
        }

        int32_t isADTS;
        if (msg->findInt32("is-adts", &isADTS)) {
            meta->setInt32(kKeyIsADTS, isADTS);
        }

        int32_t aacProfile = -1;
        if (msg->findInt32("aac-profile", &aacProfile)) {
            meta->setInt32(kKeyAACAOT, aacProfile);
        }

        int32_t pcmEncoding;
        if (msg->findInt32("pcm-encoding", &pcmEncoding)) {
            meta->setInt32(kKeyPcmEncoding, pcmEncoding);
        }

        int32_t hapticChannelCount;
        if (msg->findInt32("haptic-channel-count", &hapticChannelCount)) {
            meta->setInt32(kKeyHapticChannelCount, hapticChannelCount);
        }
    }

    int32_t maxInputSize;
    if (msg->findInt32("max-input-size", &maxInputSize)) {
        meta->setInt32(kKeyMaxInputSize, maxInputSize);
    }

    int32_t maxWidth;
    if (msg->findInt32("max-width", &maxWidth)) {
        meta->setInt32(kKeyMaxWidth, maxWidth);
    }

    int32_t maxHeight;
    if (msg->findInt32("max-height", &maxHeight)) {
        meta->setInt32(kKeyMaxHeight, maxHeight);
    }

    int32_t fps;
    float fpsFloat;
    if (msg->findInt32("frame-rate", &fps) && fps > 0) {
        meta->setInt32(kKeyFrameRate, fps);
    } else if (msg->findFloat("frame-rate", &fpsFloat)
            && fpsFloat >= 1 && fpsFloat <= INT32_MAX) {
        // truncate values to distinguish between e.g. 24 vs 23.976 fps
        meta->setInt32(kKeyFrameRate, (int32_t)fpsFloat);
    }

    // reassemble the csd data into its original form
    sp<ABuffer> csd0, csd1, csd2;
    if (msg->findBuffer("csd-0", &csd0)) {
        int csd0size = csd0->size();
        if (mime == MEDIA_MIMETYPE_VIDEO_AVC) {
            sp<ABuffer> csd1;
            if (msg->findBuffer("csd-1", &csd1)) {
                std::vector<char> avcc(csd0size + csd1->size() + 1024);
                size_t outsize = reassembleAVCC(csd0, csd1, avcc.data());
                meta->setData(kKeyAVCC, kTypeAVCC, avcc.data(), outsize);
            }
        } else if (mime == MEDIA_MIMETYPE_AUDIO_AAC ||
                mime == MEDIA_MIMETYPE_VIDEO_MPEG4 ||
                mime == MEDIA_MIMETYPE_AUDIO_WMA ||
                mime == MEDIA_MIMETYPE_AUDIO_MS_ADPCM ||
                mime == MEDIA_MIMETYPE_AUDIO_DVI_IMA_ADPCM) {
            std::vector<char> esds(csd0size + 31);
            // The written ESDS is actually for an audio stream, but it's enough
            // for transporting the CSD to muxers.
            reassembleESDS(csd0, esds.data());
            meta->setData(kKeyESDS, kTypeESDS, esds.data(), esds.size());
        } else if (mime == MEDIA_MIMETYPE_VIDEO_HEVC ||
                   mime == MEDIA_MIMETYPE_IMAGE_ANDROID_HEIC) {
            std::vector<uint8_t> hvcc(csd0size + 1024);
            size_t outsize = reassembleHVCC(csd0, hvcc.data(), hvcc.size(), 4);
            meta->setData(kKeyHVCC, kTypeHVCC, hvcc.data(), outsize);
        } else if (mime == MEDIA_MIMETYPE_VIDEO_AV1) {
            meta->setData(kKeyAV1C, 0, csd0->data(), csd0->size());
        } else if (mime == MEDIA_MIMETYPE_VIDEO_VP9) {
            meta->setData(kKeyVp9CodecPrivate, 0, csd0->data(), csd0->size());
        } else if (mime == MEDIA_MIMETYPE_AUDIO_OPUS) {
            size_t opusHeadSize = csd0->size();
            size_t codecDelayBufSize = 0;
            size_t seekPreRollBufSize = 0;
            void *opusHeadBuf = csd0->data();
            void *codecDelayBuf = NULL;
            void *seekPreRollBuf = NULL;
            if (msg->findBuffer("csd-1", &csd1)) {
                codecDelayBufSize = csd1->size();
                codecDelayBuf = csd1->data();
            }
            if (msg->findBuffer("csd-2", &csd2)) {
                seekPreRollBufSize = csd2->size();
                seekPreRollBuf = csd2->data();
            }
            /* Extract codec delay and seek pre roll from csd-0,
             * if csd-1 and csd-2 are not present */
            if (!codecDelayBuf && !seekPreRollBuf) {
                GetOpusHeaderBuffers(csd0->data(), csd0->size(), &opusHeadBuf,
                                    &opusHeadSize, &codecDelayBuf,
                                    &codecDelayBufSize, &seekPreRollBuf,
                                    &seekPreRollBufSize);
            }
            meta->setData(kKeyOpusHeader, 0, opusHeadBuf, opusHeadSize);
            if (codecDelayBuf) {
                meta->setData(kKeyOpusCodecDelay, 0, codecDelayBuf, codecDelayBufSize);
            }
            if (seekPreRollBuf) {
                meta->setData(kKeyOpusSeekPreRoll, 0, seekPreRollBuf, seekPreRollBufSize);
            }
        } else if (mime == MEDIA_MIMETYPE_AUDIO_ALAC) {
            meta->setData(kKeyAlacMagicCookie, 0, csd0->data(), csd0->size());
        }
    } else if (mime == MEDIA_MIMETYPE_VIDEO_AVC && msg->findBuffer("csd-avc", &csd0)) {
        meta->setData(kKeyAVCC, kTypeAVCC, csd0->data(), csd0->size());
    } else if ((mime == MEDIA_MIMETYPE_VIDEO_HEVC || mime == MEDIA_MIMETYPE_IMAGE_ANDROID_HEIC)
            && msg->findBuffer("csd-hevc", &csd0)) {
        meta->setData(kKeyHVCC, kTypeHVCC, csd0->data(), csd0->size());
    } else if (msg->findBuffer("esds", &csd0)) {
        meta->setData(kKeyESDS, kTypeESDS, csd0->data(), csd0->size());
    } else if (msg->findBuffer("mpeg2-stream-header", &csd0)) {
        meta->setData(kKeyStreamHeader, 'mdat', csd0->data(), csd0->size());
    } else if (msg->findBuffer("d263", &csd0)) {
        meta->setData(kKeyD263, kTypeD263, csd0->data(), csd0->size());
    }

    // XXX TODO add whatever other keys there are

#if 0
    ALOGI("converted %s to:", msg->debugString(0).c_str());
    meta->dumpToLog();
#endif
}

AString MakeUserAgent() {
    AString ua;
    ua.append("stagefright/1.2 (Linux;Android ");

#if (PROPERTY_VALUE_MAX < 8)
#error "PROPERTY_VALUE_MAX must be at least 8"
#endif

    char value[PROPERTY_VALUE_MAX];
    property_get("ro.build.version.release", value, "Unknown");
    ua.append(value);
    ua.append(")");

    return ua;
}

status_t sendMetaDataToHal(sp<MediaPlayerBase::AudioSink>& sink,
                           const sp<MetaData>& meta)
{
    int32_t sampleRate = 0;
    int32_t bitRate = 0;
    int32_t channelMask = 0;
    int32_t delaySamples = 0;
    int32_t paddingSamples = 0;

    AudioParameter param = AudioParameter();

    if (meta->findInt32(kKeySampleRate, &sampleRate)) {
        param.addInt(String8(AUDIO_OFFLOAD_CODEC_SAMPLE_RATE), sampleRate);
    }
    if (meta->findInt32(kKeyChannelMask, &channelMask)) {
        param.addInt(String8(AUDIO_OFFLOAD_CODEC_NUM_CHANNEL), channelMask);
    }
    if (meta->findInt32(kKeyBitRate, &bitRate)) {
        param.addInt(String8(AUDIO_OFFLOAD_CODEC_AVG_BIT_RATE), bitRate);
    }
    if (meta->findInt32(kKeyEncoderDelay, &delaySamples)) {
        param.addInt(String8(AUDIO_OFFLOAD_CODEC_DELAY_SAMPLES), delaySamples);
    }
    if (meta->findInt32(kKeyEncoderPadding, &paddingSamples)) {
        param.addInt(String8(AUDIO_OFFLOAD_CODEC_PADDING_SAMPLES), paddingSamples);
    }

    ALOGV("sendMetaDataToHal: bitRate %d, sampleRate %d, chanMask %d,"
          "delaySample %d, paddingSample %d", bitRate, sampleRate,
          channelMask, delaySamples, paddingSamples);

    sink->setParameters(param.toString());
    return OK;
}

struct mime_conv_t {
    const char* mime;
    audio_format_t format;
};

static const struct mime_conv_t mimeLookup[] = {
    { MEDIA_MIMETYPE_AUDIO_MPEG,        AUDIO_FORMAT_MP3 },
    { MEDIA_MIMETYPE_AUDIO_RAW,         AUDIO_FORMAT_PCM_16_BIT },
    { MEDIA_MIMETYPE_AUDIO_AMR_NB,      AUDIO_FORMAT_AMR_NB },
    { MEDIA_MIMETYPE_AUDIO_AMR_WB,      AUDIO_FORMAT_AMR_WB },
    { MEDIA_MIMETYPE_AUDIO_AAC,         AUDIO_FORMAT_AAC },
    { MEDIA_MIMETYPE_AUDIO_VORBIS,      AUDIO_FORMAT_VORBIS },
    { MEDIA_MIMETYPE_AUDIO_OPUS,        AUDIO_FORMAT_OPUS},
    { MEDIA_MIMETYPE_AUDIO_AC3,         AUDIO_FORMAT_AC3},
    { MEDIA_MIMETYPE_AUDIO_EAC3,        AUDIO_FORMAT_E_AC3},
    { MEDIA_MIMETYPE_AUDIO_EAC3_JOC,    AUDIO_FORMAT_E_AC3_JOC},
    { MEDIA_MIMETYPE_AUDIO_AC4,         AUDIO_FORMAT_AC4},
    { MEDIA_MIMETYPE_AUDIO_FLAC,        AUDIO_FORMAT_FLAC},
    { MEDIA_MIMETYPE_AUDIO_ALAC,        AUDIO_FORMAT_ALAC },
    { 0, AUDIO_FORMAT_INVALID }
};

status_t mapMimeToAudioFormat( audio_format_t& format, const char* mime )
{
const struct mime_conv_t* p = &mimeLookup[0];
    while (p->mime != NULL) {
        if (0 == strcasecmp(mime, p->mime)) {
            format = p->format;
            return OK;
        }
        ++p;
    }

    return BAD_VALUE;
}

struct aac_format_conv_t {
    OMX_AUDIO_AACPROFILETYPE eAacProfileType;
    audio_format_t format;
};

static const struct aac_format_conv_t profileLookup[] = {
    { OMX_AUDIO_AACObjectMain,        AUDIO_FORMAT_AAC_MAIN},
    { OMX_AUDIO_AACObjectLC,          AUDIO_FORMAT_AAC_LC},
    { OMX_AUDIO_AACObjectSSR,         AUDIO_FORMAT_AAC_SSR},
    { OMX_AUDIO_AACObjectLTP,         AUDIO_FORMAT_AAC_LTP},
    { OMX_AUDIO_AACObjectHE,          AUDIO_FORMAT_AAC_HE_V1},
    { OMX_AUDIO_AACObjectScalable,    AUDIO_FORMAT_AAC_SCALABLE},
    { OMX_AUDIO_AACObjectERLC,        AUDIO_FORMAT_AAC_ERLC},
    { OMX_AUDIO_AACObjectLD,          AUDIO_FORMAT_AAC_LD},
    { OMX_AUDIO_AACObjectHE_PS,       AUDIO_FORMAT_AAC_HE_V2},
    { OMX_AUDIO_AACObjectELD,         AUDIO_FORMAT_AAC_ELD},
    { OMX_AUDIO_AACObjectXHE,         AUDIO_FORMAT_AAC_XHE},
    { OMX_AUDIO_AACObjectNull,        AUDIO_FORMAT_AAC},
};

void mapAACProfileToAudioFormat( audio_format_t& format, uint64_t eAacProfile)
{
const struct aac_format_conv_t* p = &profileLookup[0];
    while (p->eAacProfileType != OMX_AUDIO_AACObjectNull) {
        if (eAacProfile == p->eAacProfileType) {
            format = p->format;
            return;
        }
        ++p;
    }
    format = AUDIO_FORMAT_AAC;
    return;
}

status_t getAudioOffloadInfo(const sp<MetaData>& meta, bool hasVideo,
        bool isStreaming, audio_stream_type_t streamType, audio_offload_info_t *info)
{
    const char *mime;
    if (meta == NULL) {
        return BAD_VALUE;
    }
    CHECK(meta->findCString(kKeyMIMEType, &mime));

    (*info) = AUDIO_INFO_INITIALIZER;

    info->format = AUDIO_FORMAT_INVALID;
    if (mapMimeToAudioFormat(info->format, mime) != OK) {
        ALOGE(" Couldn't map mime type \"%s\" to a valid AudioSystem::audio_format !", mime);
        return BAD_VALUE;
    } else {
        ALOGV("Mime type \"%s\" mapped to audio_format %d", mime, info->format);
    }

    if (AUDIO_FORMAT_INVALID == info->format) {
        // can't offload if we don't know what the source format is
        ALOGE("mime type \"%s\" not a known audio format", mime);
        return BAD_VALUE;
    }

    // Redefine aac format according to its profile
    // Offloading depends on audio DSP capabilities.
    int32_t aacaot = -1;
    if (meta->findInt32(kKeyAACAOT, &aacaot)) {
        mapAACProfileToAudioFormat(info->format,(OMX_AUDIO_AACPROFILETYPE) aacaot);
    }

    int32_t srate = -1;
    if (!meta->findInt32(kKeySampleRate, &srate)) {
        ALOGV("track of type '%s' does not publish sample rate", mime);
    }
    info->sample_rate = srate;

    int32_t cmask = 0;
    if (!meta->findInt32(kKeyChannelMask, &cmask) || cmask == CHANNEL_MASK_USE_CHANNEL_ORDER) {
        ALOGV("track of type '%s' does not publish channel mask", mime);

        // Try a channel count instead
        int32_t channelCount;
        if (!meta->findInt32(kKeyChannelCount, &channelCount)) {
            ALOGV("track of type '%s' does not publish channel count", mime);
        } else {
            cmask = audio_channel_out_mask_from_count(channelCount);
        }
    }
    info->channel_mask = cmask;

    int64_t duration = 0;
    if (!meta->findInt64(kKeyDuration, &duration)) {
        ALOGV("track of type '%s' does not publish duration", mime);
    }
    info->duration_us = duration;

    int32_t brate = -1;
    if (!meta->findInt32(kKeyBitRate, &brate)) {
        ALOGV("track of type '%s' does not publish bitrate", mime);
    }
    info->bit_rate = brate;


    info->stream_type = streamType;
    info->has_video = hasVideo;
    info->is_streaming = isStreaming;
    return OK;
}

bool canOffloadStream(const sp<MetaData>& meta, bool hasVideo,
                      bool isStreaming, audio_stream_type_t streamType)
{
    audio_offload_info_t info = AUDIO_INFO_INITIALIZER;
    if (OK != getAudioOffloadInfo(meta, hasVideo, isStreaming, streamType, &info)) {
        return false;
    }
    // Check if offload is possible for given format, stream type, sample rate,
    // bit rate, duration, video and streaming
    return AudioSystem::isOffloadSupported(info);
}

AString uriDebugString(const AString &uri, bool incognito) {
    if (incognito) {
        return AString("<URI suppressed>");
    }

    if (property_get_bool("media.stagefright.log-uri", false)) {
        return uri;
    }

    // find scheme
    AString scheme;
    const char *chars = uri.c_str();
    for (size_t i = 0; i < uri.size(); i++) {
        const char c = chars[i];
        if (!isascii(c)) {
            break;
        } else if (isalpha(c)) {
            continue;
        } else if (i == 0) {
            // first character must be a letter
            break;
        } else if (isdigit(c) || c == '+' || c == '.' || c =='-') {
            continue;
        } else if (c != ':') {
            break;
        }
        scheme = AString(uri, 0, i);
        scheme.append("://<suppressed>");
        return scheme;
    }
    return AString("<no-scheme URI suppressed>");
}

HLSTime::HLSTime(const sp<AMessage>& meta) :
    mSeq(-1),
    mTimeUs(-1LL),
    mMeta(meta) {
    if (meta != NULL) {
        CHECK(meta->findInt32("discontinuitySeq", &mSeq));
        CHECK(meta->findInt64("timeUs", &mTimeUs));
    }
}

int64_t HLSTime::getSegmentTimeUs() const {
    int64_t segmentStartTimeUs = -1LL;
    if (mMeta != NULL) {
        CHECK(mMeta->findInt64("segmentStartTimeUs", &segmentStartTimeUs));

        int64_t segmentFirstTimeUs;
        if (mMeta->findInt64("segmentFirstTimeUs", &segmentFirstTimeUs)) {
            segmentStartTimeUs += mTimeUs - segmentFirstTimeUs;
        }

        // adjust segment time by playlist age (for live streaming)
        int64_t playlistTimeUs;
        if (mMeta->findInt64("playlistTimeUs", &playlistTimeUs)) {
            int64_t playlistAgeUs = ALooper::GetNowUs() - playlistTimeUs;

            int64_t durationUs;
            CHECK(mMeta->findInt64("segmentDurationUs", &durationUs));

            // round to nearest whole segment
            playlistAgeUs = (playlistAgeUs + durationUs / 2)
                    / durationUs * durationUs;

            segmentStartTimeUs -= playlistAgeUs;
            if (segmentStartTimeUs < 0) {
                segmentStartTimeUs = 0;
            }
        }
    }
    return segmentStartTimeUs;
}

bool operator <(const HLSTime &t0, const HLSTime &t1) {
    // we can only compare discontinuity sequence and timestamp.
    // (mSegmentTimeUs is not reliable in live streaming case, it's the
    // time starting from beginning of playlist but playlist could change.)
    return t0.mSeq < t1.mSeq
            || (t0.mSeq == t1.mSeq && t0.mTimeUs < t1.mTimeUs);
}

void writeToAMessage(const sp<AMessage> &msg, const AudioPlaybackRate &rate) {
    msg->setFloat("speed", rate.mSpeed);
    msg->setFloat("pitch", rate.mPitch);
    msg->setInt32("audio-fallback-mode", rate.mFallbackMode);
    msg->setInt32("audio-stretch-mode", rate.mStretchMode);
}

void readFromAMessage(const sp<AMessage> &msg, AudioPlaybackRate *rate /* nonnull */) {
    *rate = AUDIO_PLAYBACK_RATE_DEFAULT;
    CHECK(msg->findFloat("speed", &rate->mSpeed));
    CHECK(msg->findFloat("pitch", &rate->mPitch));
    CHECK(msg->findInt32("audio-fallback-mode", (int32_t *)&rate->mFallbackMode));
    CHECK(msg->findInt32("audio-stretch-mode", (int32_t *)&rate->mStretchMode));
}

void writeToAMessage(const sp<AMessage> &msg, const AVSyncSettings &sync, float videoFpsHint) {
    msg->setInt32("sync-source", sync.mSource);
    msg->setInt32("audio-adjust-mode", sync.mAudioAdjustMode);
    msg->setFloat("tolerance", sync.mTolerance);
    msg->setFloat("video-fps", videoFpsHint);
}

void readFromAMessage(
        const sp<AMessage> &msg,
        AVSyncSettings *sync /* nonnull */,
        float *videoFps /* nonnull */) {
    AVSyncSettings settings;
    CHECK(msg->findInt32("sync-source", (int32_t *)&settings.mSource));
    CHECK(msg->findInt32("audio-adjust-mode", (int32_t *)&settings.mAudioAdjustMode));
    CHECK(msg->findFloat("tolerance", &settings.mTolerance));
    CHECK(msg->findFloat("video-fps", videoFps));
    *sync = settings;
}

void writeToAMessage(const sp<AMessage> &msg, const BufferingSettings &buffering) {
    msg->setInt32("init-ms", buffering.mInitialMarkMs);
    msg->setInt32("resume-playback-ms", buffering.mResumePlaybackMarkMs);
}

void readFromAMessage(const sp<AMessage> &msg, BufferingSettings *buffering /* nonnull */) {
    int32_t value;
    if (msg->findInt32("init-ms", &value)) {
        buffering->mInitialMarkMs = value;
    }
    if (msg->findInt32("resume-playback-ms", &value)) {
        buffering->mResumePlaybackMarkMs = value;
    }
}

AString nameForFd(int fd) {
    const size_t SIZE = 256;
    char buffer[SIZE];
    AString result;
    snprintf(buffer, SIZE, "/proc/%d/fd/%d", getpid(), fd);
    struct stat s;
    if (lstat(buffer, &s) == 0) {
        if ((s.st_mode & S_IFMT) == S_IFLNK) {
            char linkto[256];
            int len = readlink(buffer, linkto, sizeof(linkto));
            if(len > 0) {
                if(len > 255) {
                    linkto[252] = '.';
                    linkto[253] = '.';
                    linkto[254] = '.';
                    linkto[255] = 0;
                } else {
                    linkto[len] = 0;
                }
                result.append(linkto);
            }
        } else {
            result.append("unexpected type for ");
            result.append(buffer);
        }
    } else {
        result.append("couldn't open ");
        result.append(buffer);
    }
    return result;
}

}  // namespace android
