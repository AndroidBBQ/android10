/*
 * Copyright (C) 2017 The Android Open Source Project
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
#define LOG_TAG "OmxInfoBuilder"

#ifdef __LP64__
#define OMX_ANDROID_COMPILE_AS_32BIT_ON_64BIT_PLATFORMS
#endif

#include <android-base/properties.h>
#include <utils/Log.h>

#include <media/stagefright/foundation/MediaDefs.h>
#include <media/stagefright/OmxInfoBuilder.h>
#include <media/stagefright/ACodec.h>

#include <android/hardware/media/omx/1.0/IOmxStore.h>
#include <android/hardware/media/omx/1.0/IOmx.h>
#include <android/hardware/media/omx/1.0/IOmxNode.h>
#include <media/stagefright/omx/OMXUtils.h>

#include <media/IOMX.h>
#include <media/omx/1.0/WOmx.h>
#include <media/stagefright/omx/1.0/OmxStore.h>

#include <media/openmax/OMX_Index.h>
#include <media/openmax/OMX_IndexExt.h>
#include <media/openmax/OMX_Audio.h>
#include <media/openmax/OMX_AudioExt.h>
#include <media/openmax/OMX_Video.h>
#include <media/openmax/OMX_VideoExt.h>

namespace android {

using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using namespace ::android::hardware::media::omx::V1_0;

namespace /* unnamed */ {

bool hasPrefix(const hidl_string& s, const char* prefix) {
    return strncasecmp(s.c_str(), prefix, strlen(prefix)) == 0;
}

status_t queryCapabilities(
        const IOmxStore::NodeInfo& node, const char* mediaType, bool isEncoder,
        MediaCodecInfo::CapabilitiesWriter* caps) {
    sp<ACodec> codec = new ACodec();
    for (const auto& attribute : node.attributes) {
        // All features have an int32 value except
        // "feature-bitrate-modes", which has a string value.
        if (hasPrefix(attribute.key, "feature-") &&
                !hasPrefix(attribute.key, "feature-bitrate-modes")) {
            // If this attribute.key is a feature that is not bitrate modes,
            // add an int32 value.
            caps->addDetail(
                    attribute.key.c_str(),
                    hasPrefix(attribute.value, "1") ? 1 : 0);
        } else {
            // Non-feature attributes
            caps->addDetail(
                    attribute.key.c_str(), attribute.value.c_str());
        }
    }
    // query capabilities may remove capabilities that are not actually supported by the codec
    status_t err = codec->queryCapabilities(
            node.owner.c_str(), node.name.c_str(), mediaType, isEncoder, caps);
    if (err != OK) {
        return err;
    }
    return OK;
}

}  // unnamed namespace

OmxInfoBuilder::OmxInfoBuilder(bool allowSurfaceEncoders)
    : mAllowSurfaceEncoders(allowSurfaceEncoders) {
}

status_t OmxInfoBuilder::buildMediaCodecList(MediaCodecListWriter* writer) {
    // Obtain IOmxStore
    sp<IOmxStore> omxStore = IOmxStore::getService();
    if (omxStore == nullptr) {
        ALOGE("Cannot find an IOmxStore service.");
        return NO_INIT;
    }

    // List service attributes (global settings)
    Status status;
    hidl_vec<IOmxStore::RoleInfo> roles;
    auto transStatus = omxStore->listRoles(
            [&roles] (
            const hidl_vec<IOmxStore::RoleInfo>& inRoleList) {
                roles = inRoleList;
            });
    if (!transStatus.isOk()) {
        ALOGE("Fail to obtain codec roles from IOmxStore.");
        return NO_INIT;
    }

    hidl_vec<IOmxStore::ServiceAttribute> serviceAttributes;
    transStatus = omxStore->listServiceAttributes(
            [&status, &serviceAttributes] (
            Status inStatus,
            const hidl_vec<IOmxStore::ServiceAttribute>& inAttributes) {
                status = inStatus;
                serviceAttributes = inAttributes;
            });
    if (!transStatus.isOk()) {
        ALOGE("Fail to obtain global settings from IOmxStore.");
        return NO_INIT;
    }
    if (status != Status::OK) {
        ALOGE("IOmxStore reports parsing error.");
        return NO_INIT;
    }
    for (const auto& p : serviceAttributes) {
        writer->addGlobalSetting(
                p.key.c_str(), p.value.c_str());
    }

    // Convert roles to lists of codecs

    // codec name -> index into swCodecs/hwCodecs
    std::map<hidl_string, std::unique_ptr<MediaCodecInfoWriter>> codecName2Info;

    uint32_t defaultRank =
        ::android::base::GetUintProperty("debug.stagefright.omx_default_rank", 0x100u);
    uint32_t defaultSwAudioRank =
        ::android::base::GetUintProperty("debug.stagefright.omx_default_rank.sw-audio", 0x10u);
    uint32_t defaultSwOtherRank =
        ::android::base::GetUintProperty("debug.stagefright.omx_default_rank.sw-other", 0x210u);

    for (const IOmxStore::RoleInfo& role : roles) {
        const hidl_string& typeName = role.type;
        bool isEncoder = role.isEncoder;
        bool isAudio = hasPrefix(role.type, "audio/");
        bool isVideoOrImage = hasPrefix(role.type, "video/") || hasPrefix(role.type, "image/");

        for (const IOmxStore::NodeInfo &node : role.nodes) {
            const hidl_string& nodeName = node.name;

            // currently image and video encoders use surface input
            if (!mAllowSurfaceEncoders && isVideoOrImage && isEncoder) {
                ALOGD("disabling %s for media type %s because we are not using OMX input surface",
                        nodeName.c_str(), role.type.c_str());
                continue;
            }

            bool isSoftware = hasPrefix(nodeName, "OMX.google");
            uint32_t rank = isSoftware
                    ? (isAudio ? defaultSwAudioRank : defaultSwOtherRank)
                    : defaultRank;
            // get rank from IOmxStore via attribute
            for (const IOmxStore::Attribute& attribute : node.attributes) {
                if (attribute.key == "rank") {
                    uint32_t oldRank = rank;
                    char dummy;
                    if (sscanf(attribute.value.c_str(), "%u%c", &rank, &dummy) != 1) {
                        rank = oldRank;
                    }
                    break;
                }
            }

            MediaCodecInfoWriter* info;
            auto c2i = codecName2Info.find(nodeName);
            if (c2i == codecName2Info.end()) {
                // Create a new MediaCodecInfo for a new node.
                c2i = codecName2Info.insert(std::make_pair(
                        nodeName, writer->addMediaCodecInfo())).first;
                info = c2i->second.get();
                info->setName(nodeName.c_str());
                info->setOwner(node.owner.c_str());
                info->setRank(rank);

                typename std::underlying_type<MediaCodecInfo::Attributes>::type attrs = 0;
                // all OMX codecs are vendor codecs (in the vendor partition), but
                // treat OMX.google codecs as non-hardware-accelerated and non-vendor
                if (!isSoftware) {
                    attrs |= MediaCodecInfo::kFlagIsVendor;
                    if (!std::count_if(
                            node.attributes.begin(), node.attributes.end(),
                            [](const IOmxStore::Attribute &i) -> bool {
                                return i.key == "attribute::software-codec";
                                                                      })) {
                        attrs |= MediaCodecInfo::kFlagIsHardwareAccelerated;
                    }
                }
                if (isEncoder) {
                    attrs |= MediaCodecInfo::kFlagIsEncoder;
                }
                info->setAttributes(attrs);
            } else {
                // The node has been seen before. Simply retrieve the
                // existing MediaCodecInfoWriter.
                info = c2i->second.get();
            }
            std::unique_ptr<MediaCodecInfo::CapabilitiesWriter> caps =
                    info->addMediaType(typeName.c_str());
            if (queryCapabilities(
                    node, typeName.c_str(), isEncoder, caps.get()) != OK) {
                ALOGW("Fail to add media type %s to codec %s",
                        typeName.c_str(), nodeName.c_str());
                info->removeMediaType(typeName.c_str());
            }
        }
    }
    return OK;
}

}  // namespace android
