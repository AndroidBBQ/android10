/*
 * Copyright (C) 2011 The Android Open Source Project
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

#include "sles_allinclusive.h"

#include <media/openmax/OMX_Types.h>
#include <media/stagefright/MediaCodecList.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/SimpleDecodingSource.h>


namespace android {

// listed in same order as VideoCodecIds[] in file "../devices.c" with ANDROID defined
static const char *kVideoMimeTypes[] = {
        MEDIA_MIMETYPE_VIDEO_MPEG2,
        MEDIA_MIMETYPE_VIDEO_H263,
        MEDIA_MIMETYPE_VIDEO_MPEG4,
        MEDIA_MIMETYPE_VIDEO_AVC,
        MEDIA_MIMETYPE_VIDEO_VP8,
        MEDIA_MIMETYPE_VIDEO_VP9
};
// must == kMaxVideoDecoders
static const size_t kNbVideoMimeTypes = sizeof(kVideoMimeTypes) / sizeof(kVideoMimeTypes[0]);

// codec capabilities in the following arrays maps to the mime types defined in kVideoMimeTypes
struct CodecCapabilities {
    Vector<MediaCodecInfo::ProfileLevel> mProfileLevels;
};

static CodecCapabilities VideoDecoderCapabilities[kNbVideoMimeTypes];
static XAuint32 VideoDecoderNbProfLevel[kNbVideoMimeTypes];

static XAuint32 NbSupportedDecoderTypes = 0;


XAuint32 convertOpenMaxIlToAl(OMX_U32 ilVideoProfileOrLevel) {
    // For video codec profiles and levels, the number of trailing zeroes in OpenMAX IL
    // are equal to the matching OpenMAX AL constant value plus 1, for example:
    //    XA_VIDEOPROFILE_H263_BACKWARDCOMPATIBLE ((XAuint32) 0x00000003)
    //        matches
    //    OMX_VIDEO_H263ProfileBackwardCompatible  = 0x04
    return (XAuint32) (__builtin_ctz(ilVideoProfileOrLevel) + 1);
}


bool android_videoCodec_expose() {
    SL_LOGV("android_videoCodec_expose()");

    sp<IMediaCodecList> list = MediaCodecList::getInstance();
    if (list == NULL) {
        SL_LOGE("could not get MediaCodecList");
        return false;
    }

    // used to check whether no codecs were found, which is a sign of failure
    NbSupportedDecoderTypes = 0;
    for (size_t m = 0 ; m < kNbVideoMimeTypes ; m++) {
        VideoDecoderNbProfLevel[m] = 0;
        for (ssize_t index = 0;; ++index) {
            index = list->findCodecByType(
                    kVideoMimeTypes[m], false /* encoder */, index);
            if (index < 0) {
                break;
            }

            sp<MediaCodecInfo> info = list->getCodecInfo(index);
            if (info == NULL || MediaCodecList::isSoftwareCodec(info->getCodecName())) {
                continue; // HW codec only
            }

            sp<MediaCodecInfo::Capabilities> caps = info->getCapabilitiesFor(kVideoMimeTypes[m]);
            if (caps == NULL) {
                continue; // this should not happen
            }

            // get the number of profiles and levels
            Vector<MediaCodecInfo::ProfileLevel> &profileLevels =
                VideoDecoderCapabilities[m].mProfileLevels;
            caps->getSupportedProfileLevels(&profileLevels);
#if 0   // Intentionally disabled example of making modifications to profile / level combinations
            if (VideoDecoderIds[m] == XA_VIDEOCODEC_AVC) {
                // remove non-core profile / level combinations
                for (size_t i = 0, size = profileLevels.size(); i < size; ) {
                    MediaCodecInfo::ProfileLevel profileLevel = profileLevels.itemAt(i);
                    if (profileLevel.mProfile == XA_VIDEOPROFILE_AVC_BASELINE) {
                        // either skip past this item and don't change vector size
                        ++i;
                    } else {
                        // or remove this item, decrement the vector size,
                        // and next time through the loop check a different item at same index
                        profileLevels.removeAt(i);
                        --size;
                    }
                }
            }
#endif
            if ((VideoDecoderNbProfLevel[m] = profileLevels.size()) > 0) {
                NbSupportedDecoderTypes++;
            }

            // only consider first codec implementation for given decoder ID / MIME type
            break;
        }
    }

    return (NbSupportedDecoderTypes > 0);
}


void android_videoCodec_deinit() {
    SL_LOGV("android_videoCodec_deinit()");
    // not needed
    // memset(VideoDecoderNbProfLevel, 0, sizeof(VideoDecoderNbProfLevel));
    // NbSupportedDecoderTypes = 0;
}


XAuint32 android_videoCodec_getNbDecoders() {
    return NbSupportedDecoderTypes;
}


void android_videoCodec_getDecoderIds(XAuint32 nbDecoders, XAuint32 *pDecoderIds) {
    XAuint32 *pIds = pDecoderIds;
    XAuint32 nbFound = 0;
    for (size_t m = 0 ; m < kNbVideoMimeTypes ; m++) {
        if (VideoDecoderNbProfLevel[m] > 0) {
            *pIds = VideoDecoderIds[m];
            pIds++;
            nbFound++;
        }
        // range check: function can be called for fewer codecs than there are
        if (nbFound == nbDecoders) {
            break;
        }
    }
}


SLresult android_videoCodec_getProfileLevelCombinationNb(XAuint32 decoderId, XAuint32 *pNb)
{
    // translate a decoder ID to an index in the codec table
    size_t decoderIndex = 0;
    while (decoderIndex < kNbVideoMimeTypes) {
        if (decoderId == VideoDecoderIds[decoderIndex]) {
            *pNb = VideoDecoderNbProfLevel[decoderIndex];
            return XA_RESULT_SUCCESS;
        }
        decoderIndex++;
    }

    // spec doesn't allow a decoder to report zero profile/level combinations
    *pNb = 0;
    return XA_RESULT_PARAMETER_INVALID;
}


SLresult android_videoCodec_getProfileLevelCombination(XAuint32 decoderId, XAuint32 plIndex,
        XAVideoCodecDescriptor *pDescr)
{
    // translate a decoder ID to an index in the codec table
    size_t decoderIndex = 0;
    while (decoderIndex < kNbVideoMimeTypes) {
        if (decoderId == VideoDecoderIds[decoderIndex]) {
            // We only look at the first codec implementation for a given decoder ID / MIME type.
            // OpenMAX AL doesn't let you expose the capabilities of multiple codec implementations.
            if (!(plIndex < VideoDecoderCapabilities[decoderIndex].mProfileLevels.size()))
            {
                // asking for invalid profile/level
                return XA_RESULT_PARAMETER_INVALID;
            }
            //     set the fields we know about
            pDescr->codecId = decoderId;
            pDescr->profileSetting = convertOpenMaxIlToAl(VideoDecoderCapabilities[decoderIndex].
                    mProfileLevels.itemAt(plIndex).mProfile);
            pDescr->levelSetting = convertOpenMaxIlToAl(VideoDecoderCapabilities[decoderIndex].
                    mProfileLevels.itemAt(plIndex).mLevel);
            //     initialize the fields we don't know about
            pDescr->maxWidth = 0;
            pDescr->maxHeight = 0;
            pDescr->maxFrameRate = 0;
            pDescr->maxBitRate = 0;
            pDescr->rateControlSupported = 0;
            break;
        }
        decoderIndex++;
    }
    return (decoderIndex < kNbVideoMimeTypes) ? XA_RESULT_SUCCESS : XA_RESULT_PARAMETER_INVALID;
}

} // namespace android
