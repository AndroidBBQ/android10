/*
 * Copyright (C) 2014 The Android Open Source Project
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

#ifndef WEBMCONSTANTS_H_
#define WEBMCONSTANTS_H_

#include <stdint.h>

namespace webm {

const int kMinEbmlVoidSize = 2;
const int64_t kMaxMetaSeekSize = 64;
const int64_t kMkvUnknownLength = 0x01ffffffffffffffl;

// EBML element id's from http://matroska.org/technical/specs/index.html
enum Mkv {
    kMkvEbml = 0x1A45DFA3,
    kMkvEbmlVersion = 0x4286,
    kMkvEbmlReadVersion = 0x42F7,
    kMkvEbmlMaxIdlength = 0x42F2,
    kMkvEbmlMaxSizeLength = 0x42F3,
    kMkvDocType = 0x4282,
    kMkvDocTypeVersion = 0x4287,
    kMkvDocTypeReadVersion = 0x4285,
    kMkvVoid = 0xEC,
    kMkvSignatureSlot = 0x1B538667,
    kMkvSignatureAlgo = 0x7E8A,
    kMkvSignatureHash = 0x7E9A,
    kMkvSignaturePublicKey = 0x7EA5,
    kMkvSignature = 0x7EB5,
    kMkvSignatureElements = 0x7E5B,
    kMkvSignatureElementList = 0x7E7B,
    kMkvSignedElement = 0x6532,
    kMkvSegment = 0x18538067,
    kMkvSeekHead = 0x114D9B74,
    kMkvSeek = 0x4DBB,
    kMkvSeekId = 0x53AB,
    kMkvSeekPosition = 0x53AC,
    kMkvInfo = 0x1549A966,
    kMkvTimecodeScale = 0x2AD7B1,
    kMkvSegmentDuration = 0x4489,
    kMkvDateUtc = 0x4461,
    kMkvMuxingApp = 0x4D80,
    kMkvWritingApp = 0x5741,
    kMkvCluster = 0x1F43B675,
    kMkvTimecode = 0xE7,
    kMkvPrevSize = 0xAB,
    kMkvBlockGroup = 0xA0,
    kMkvBlock = 0xA1,
    kMkvBlockAdditions = 0x75A1,
    kMkvBlockMore = 0xA6,
    kMkvBlockAddId = 0xEE,
    kMkvBlockAdditional = 0xA5,
    kMkvBlockDuration = 0x9B,
    kMkvReferenceBlock = 0xFB,
    kMkvLaceNumber = 0xCC,
    kMkvSimpleBlock = 0xA3,
    kMkvTracks = 0x1654AE6B,
    kMkvTrackEntry = 0xAE,
    kMkvTrackNumber = 0xD7,
    kMkvTrackUid = 0x73C5,
    kMkvTrackType = 0x83,
    kMkvFlagEnabled = 0xB9,
    kMkvFlagDefault = 0x88,
    kMkvFlagForced = 0x55AA,
    kMkvFlagLacing = 0x9C,
    kMkvDefaultDuration = 0x23E383,
    kMkvMaxBlockAdditionId = 0x55EE,
    kMkvName = 0x536E,
    kMkvLanguage = 0x22B59C,
    kMkvCodecId = 0x86,
    kMkvCodecPrivate = 0x63A2,
    kMkvCodecName = 0x258688,
    kMkvVideo = 0xE0,
    kMkvFlagInterlaced = 0x9A,
    kMkvStereoMode = 0x53B8,
    kMkvAlphaMode = 0x53C0,
    kMkvPixelWidth = 0xB0,
    kMkvPixelHeight = 0xBA,
    kMkvPixelCropBottom = 0x54AA,
    kMkvPixelCropTop = 0x54BB,
    kMkvPixelCropLeft = 0x54CC,
    kMkvPixelCropRight = 0x54DD,
    kMkvDisplayWidth = 0x54B0,
    kMkvDisplayHeight = 0x54BA,
    kMkvDisplayUnit = 0x54B2,
    kMkvAspectRatioType = 0x54B3,
    kMkvColour = 0x55B0,
    kMkvMatrixCoefficients = 0x55B1,
    kMkvRange = 0x55B9,
    kMkvTransferCharacteristics = 0x55BA,
    kMkvPrimaries = 0x55BB,
    kMkvMaxCLL = 0x55BC,
    kMkvMaxFALL = 0x55BD,
    kMkvMasteringMetadata = 0x55D0,
    kMkvPrimaryRChromaticityX = 0x55D1,
    kMkvPrimaryRChromaticityY = 0x55D2,
    kMkvPrimaryGChromaticityX = 0x55D3,
    kMkvPrimaryGChromaticityY = 0x55D4,
    kMkvPrimaryBChromaticityX = 0x55D5,
    kMkvPrimaryBChromaticityY = 0x55D6,
    kMkvWhitePointChromaticityX = 0x55D7,
    kMkvWhitePointChromaticityY = 0x55D8,
    kMkvLuminanceMax = 0x55D9,
    kMkvLuminanceMin = 0x55DA,
    kMkvFrameRate = 0x2383E3,
    kMkvAudio = 0xE1,
    kMkvSamplingFrequency = 0xB5,
    kMkvOutputSamplingFrequency = 0x78B5,
    kMkvChannels = 0x9F,
    kMkvBitDepth = 0x6264,
    kMkvCues = 0x1C53BB6B,
    kMkvCuePoint = 0xBB,
    kMkvCueTime = 0xB3,
    kMkvCueTrackPositions = 0xB7,
    kMkvCueTrack = 0xF7,
    kMkvCueClusterPosition = 0xF1,
    kMkvCueBlockNumber = 0x5378
};

enum TrackTypes {
    kInvalidType = -1,
    kVideoType = 0x1,
    kAudioType = 0x2,
    kComplexType = 0x3,
    kLogoType = 0x10,
    kSubtitleType = 0x11,
    kButtonsType = 0x12,
    kControlType = 0x20
};

enum TrackNum {
    kVideoTrackNum = 0x1,
    kAudioTrackNum = 0x2
};
}

#endif /* WEBMCONSTANTS_H_ */
