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
#define LOG_TAG "Codec2InfoBuilder"
#include <log/log.h>

#include <strings.h>

#include <C2Component.h>
#include <C2Config.h>
#include <C2Debug.h>
#include <C2PlatformSupport.h>
#include <Codec2Mapper.h>

#include <OMX_Audio.h>
#include <OMX_AudioExt.h>
#include <OMX_IndexExt.h>
#include <OMX_Types.h>
#include <OMX_Video.h>
#include <OMX_VideoExt.h>
#include <OMX_AsString.h>

#include <android/hardware/media/omx/1.0/IOmx.h>
#include <android/hardware/media/omx/1.0/IOmxObserver.h>
#include <android/hardware/media/omx/1.0/IOmxNode.h>
#include <android/hardware/media/omx/1.0/types.h>

#include <android-base/properties.h>
#include <codec2/hidl/client.h>
#include <cutils/native_handle.h>
#include <media/omx/1.0/WOmxNode.h>
#include <media/stagefright/MediaCodecConstants.h>
#include <media/stagefright/foundation/ALookup.h>
#include <media/stagefright/foundation/MediaDefs.h>
#include <media/stagefright/omx/OMXUtils.h>
#include <media/stagefright/xmlparser/MediaCodecsXmlParser.h>

#include "Codec2InfoBuilder.h"

namespace android {

using Traits = C2Component::Traits;

namespace /* unnamed */ {

bool hasPrefix(const std::string& s, const char* prefix) {
    size_t prefixLen = strlen(prefix);
    return s.compare(0, prefixLen, prefix) == 0;
}

bool hasSuffix(const std::string& s, const char* suffix) {
    size_t suffixLen = strlen(suffix);
    return suffixLen > s.size() ? false :
            s.compare(s.size() - suffixLen, suffixLen, suffix) == 0;
}

void addSupportedProfileLevels(
        std::shared_ptr<Codec2Client::Interface> intf,
        MediaCodecInfo::CapabilitiesWriter *caps,
        const Traits& trait, const std::string &mediaType) {
    std::shared_ptr<C2Mapper::ProfileLevelMapper> mapper =
        C2Mapper::GetProfileLevelMapper(trait.mediaType);
    // if we don't know the media type, pass through all values unmapped

    // TODO: we cannot find levels that are local 'maxima' without knowing the coding
    // e.g. H.263 level 45 and level 30 could be two values for highest level as
    // they don't include one another. For now we use the last supported value.
    bool encoder = trait.kind == C2Component::KIND_ENCODER;
    C2StreamProfileLevelInfo pl(encoder /* output */, 0u);
    std::vector<C2FieldSupportedValuesQuery> profileQuery = {
        C2FieldSupportedValuesQuery::Possible(C2ParamField(&pl, &pl.profile))
    };

    c2_status_t err = intf->querySupportedValues(profileQuery, C2_DONT_BLOCK);
    ALOGV("query supported profiles -> %s | %s", asString(err), asString(profileQuery[0].status));
    if (err != C2_OK || profileQuery[0].status != C2_OK) {
        return;
    }

    // we only handle enumerated values
    if (profileQuery[0].values.type != C2FieldSupportedValues::VALUES) {
        return;
    }

    // determine if codec supports HDR
    bool supportsHdr = false;
    bool supportsHdr10Plus = false;

    std::vector<std::shared_ptr<C2ParamDescriptor>> paramDescs;
    c2_status_t err1 = intf->querySupportedParams(&paramDescs);
    if (err1 == C2_OK) {
        for (const std::shared_ptr<C2ParamDescriptor> &desc : paramDescs) {
            switch ((uint32_t)desc->index()) {
            case C2StreamHdr10PlusInfo::output::PARAM_TYPE:
                supportsHdr10Plus = true;
                break;
            case C2StreamHdrStaticInfo::output::PARAM_TYPE:
                supportsHdr = true;
                break;
            default:
                break;
            }
        }
    }

    // For VP9, the static info is always propagated by framework.
    supportsHdr |= (mediaType == MIMETYPE_VIDEO_VP9);

    for (C2Value::Primitive profile : profileQuery[0].values.values) {
        pl.profile = (C2Config::profile_t)profile.ref<uint32_t>();
        std::vector<std::unique_ptr<C2SettingResult>> failures;
        err = intf->config({&pl}, C2_DONT_BLOCK, &failures);
        ALOGV("set profile to %u -> %s", pl.profile, asString(err));
        std::vector<C2FieldSupportedValuesQuery> levelQuery = {
            C2FieldSupportedValuesQuery::Current(C2ParamField(&pl, &pl.level))
        };
        err = intf->querySupportedValues(levelQuery, C2_DONT_BLOCK);
        ALOGV("query supported levels -> %s | %s", asString(err), asString(levelQuery[0].status));
        if (err != C2_OK || levelQuery[0].status != C2_OK
                || levelQuery[0].values.type != C2FieldSupportedValues::VALUES
                || levelQuery[0].values.values.size() == 0) {
            continue;
        }

        C2Value::Primitive level = levelQuery[0].values.values.back();
        pl.level = (C2Config::level_t)level.ref<uint32_t>();
        ALOGV("supporting level: %u", pl.level);
        int32_t sdkProfile, sdkLevel;
        if (mapper && mapper->mapProfile(pl.profile, &sdkProfile)
                && mapper->mapLevel(pl.level, &sdkLevel)) {
            caps->addProfileLevel((uint32_t)sdkProfile, (uint32_t)sdkLevel);
            // also list HDR profiles if component supports HDR
            if (supportsHdr) {
                auto hdrMapper = C2Mapper::GetHdrProfileLevelMapper(trait.mediaType);
                if (hdrMapper && hdrMapper->mapProfile(pl.profile, &sdkProfile)) {
                    caps->addProfileLevel((uint32_t)sdkProfile, (uint32_t)sdkLevel);
                }
                if (supportsHdr10Plus) {
                    hdrMapper = C2Mapper::GetHdrProfileLevelMapper(
                            trait.mediaType, true /*isHdr10Plus*/);
                    if (hdrMapper && hdrMapper->mapProfile(pl.profile, &sdkProfile)) {
                        caps->addProfileLevel((uint32_t)sdkProfile, (uint32_t)sdkLevel);
                    }
                }
            }
        } else if (!mapper) {
            caps->addProfileLevel(pl.profile, pl.level);
        }

        // for H.263 also advertise the second highest level if the
        // codec supports level 45, as level 45 only covers level 10
        // TODO: move this to some form of a setting so it does not
        // have to be here
        if (mediaType == MIMETYPE_VIDEO_H263) {
            C2Config::level_t nextLevel = C2Config::LEVEL_UNUSED;
            for (C2Value::Primitive v : levelQuery[0].values.values) {
                C2Config::level_t level = (C2Config::level_t)v.ref<uint32_t>();
                if (level < C2Config::LEVEL_H263_45 && level > nextLevel) {
                    nextLevel = level;
                }
            }
            if (nextLevel != C2Config::LEVEL_UNUSED
                    && nextLevel != pl.level
                    && mapper
                    && mapper->mapProfile(pl.profile, &sdkProfile)
                    && mapper->mapLevel(nextLevel, &sdkLevel)) {
                caps->addProfileLevel(
                        (uint32_t)sdkProfile, (uint32_t)sdkLevel);
            }
        }
    }
}

void addSupportedColorFormats(
        std::shared_ptr<Codec2Client::Interface> intf,
        MediaCodecInfo::CapabilitiesWriter *caps,
        const Traits& trait, const std::string &mediaType) {
    (void)intf;

    // TODO: get this from intf() as well, but how do we map them to
    // MediaCodec color formats?
    bool encoder = trait.kind == C2Component::KIND_ENCODER;
    if (mediaType.find("video") != std::string::npos
            || mediaType.find("image") != std::string::npos) {
        // vendor video codecs prefer opaque format
        if (trait.name.find("android") == std::string::npos) {
            caps->addColorFormat(COLOR_FormatSurface);
        }
        caps->addColorFormat(COLOR_FormatYUV420Flexible);
        caps->addColorFormat(COLOR_FormatYUV420Planar);
        caps->addColorFormat(COLOR_FormatYUV420SemiPlanar);
        caps->addColorFormat(COLOR_FormatYUV420PackedPlanar);
        caps->addColorFormat(COLOR_FormatYUV420PackedSemiPlanar);
        // framework video encoders must support surface format, though it is unclear
        // that they will be able to map it if it is opaque
        if (encoder && trait.name.find("android") != std::string::npos) {
            caps->addColorFormat(COLOR_FormatSurface);
        }
    }
}

class Switch {
    enum Flags : uint8_t {
        // flags
        IS_ENABLED = (1 << 0),
        BY_DEFAULT = (1 << 1),
    };

    constexpr Switch(uint8_t flags) : mFlags(flags) {}

    uint8_t mFlags;

public:
    // have to create class due to this bool conversion operator...
    constexpr operator bool() const {
        return mFlags & IS_ENABLED;
    }

    constexpr Switch operator!() const {
        return Switch(mFlags ^ IS_ENABLED);
    }

    static constexpr Switch DISABLED() { return 0; };
    static constexpr Switch ENABLED() { return IS_ENABLED; };
    static constexpr Switch DISABLED_BY_DEFAULT() { return BY_DEFAULT; };
    static constexpr Switch ENABLED_BY_DEFAULT() { return IS_ENABLED | BY_DEFAULT; };

    const char *toString(const char *def = "??") const {
        switch (mFlags) {
        case 0:                         return "0";
        case IS_ENABLED:                return "1";
        case BY_DEFAULT:                return "(0)";
        case IS_ENABLED | BY_DEFAULT:   return "(1)";
        default: return def;
        }
    }

};

const char *asString(const Switch &s, const char *def = "??") {
    return s.toString(def);
}

Switch isSettingEnabled(
        std::string setting, const MediaCodecsXmlParser::AttributeMap &settings,
        Switch def = Switch::DISABLED_BY_DEFAULT()) {
    const auto enablement = settings.find(setting);
    if (enablement == settings.end()) {
        return def;
    }
    return enablement->second == "1" ? Switch::ENABLED() : Switch::DISABLED();
}

Switch isVariantEnabled(
        std::string variant, const MediaCodecsXmlParser::AttributeMap &settings) {
    return isSettingEnabled("variant-" + variant, settings);
}

Switch isVariantExpressionEnabled(
        std::string exp, const MediaCodecsXmlParser::AttributeMap &settings) {
    if (!exp.empty() && exp.at(0) == '!') {
        return !isVariantEnabled(exp.substr(1, exp.size() - 1), settings);
    }
    return isVariantEnabled(exp, settings);
}

Switch isDomainEnabled(
        std::string domain, const MediaCodecsXmlParser::AttributeMap &settings) {
    return isSettingEnabled("domain-" + domain, settings);
}

} // unnamed namespace

status_t Codec2InfoBuilder::buildMediaCodecList(MediaCodecListWriter* writer) {
    // TODO: Remove run-time configurations once all codecs are working
    // properly. (Assume "full" behavior eventually.)
    //
    // debug.stagefright.ccodec supports 5 values.
    //   0 - No Codec 2.0 components are available.
    //   1 - Audio decoders and encoders with prefix "c2.android." are available
    //       and ranked first.
    //       All other components with prefix "c2.android." are available with
    //       their normal ranks.
    //       Components with prefix "c2.vda." are available with their normal
    //       ranks.
    //       All other components with suffix ".avc.decoder" or ".avc.encoder"
    //       are available but ranked last.
    //   2 - Components with prefix "c2.android." are available and ranked
    //       first.
    //       Components with prefix "c2.vda." are available with their normal
    //       ranks.
    //       All other components with suffix ".avc.decoder" or ".avc.encoder"
    //       are available but ranked last.
    //   3 - Components with prefix "c2.android." are available and ranked
    //       first.
    //       All other components are available with their normal ranks.
    //   4 - All components are available with their normal ranks.
    //
    // The default value (boot time) is 1.
    //
    // Note: Currently, OMX components have default rank 0x100, while all
    // Codec2.0 software components have default rank 0x200.
    int option = ::android::base::GetIntProperty("debug.stagefright.ccodec", 4);

    // Obtain Codec2Client
    std::vector<Traits> traits = Codec2Client::ListComponents();

    // parse APEX XML first, followed by vendor XML
    MediaCodecsXmlParser parser;
    parser.parseXmlFilesInSearchDirs(
            parser.getDefaultXmlNames(),
            { "/apex/com.android.media.swcodec/etc" });

    // TODO: remove these c2-specific files once product moved to default file names
    parser.parseXmlFilesInSearchDirs(
            { "media_codecs_c2.xml", "media_codecs_performance_c2.xml" });

    // parse default XML files
    parser.parseXmlFilesInSearchDirs();

    if (parser.getParsingStatus() != OK) {
        ALOGD("XML parser no good");
        return OK;
    }

    MediaCodecsXmlParser::AttributeMap settings = parser.getServiceAttributeMap();
    for (const auto &v : settings) {
        if (!hasPrefix(v.first, "media-type-")
                && !hasPrefix(v.first, "domain-")
                && !hasPrefix(v.first, "variant-")) {
            writer->addGlobalSetting(v.first.c_str(), v.second.c_str());
        }
    }

    for (const Traits& trait : traits) {
        C2Component::rank_t rank = trait.rank;

        // Interface must be accessible for us to list the component, and there also
        // must be an XML entry for the codec. Codec aliases listed in the traits
        // allow additional XML entries to be specified for each alias. These will
        // be listed as separate codecs. If no XML entry is specified for an alias,
        // those will be treated as an additional alias specified in the XML entry
        // for the interface name.
        std::vector<std::string> nameAndAliases = trait.aliases;
        nameAndAliases.insert(nameAndAliases.begin(), trait.name);
        for (const std::string &nameOrAlias : nameAndAliases) {
            bool isAlias = trait.name != nameOrAlias;
            std::shared_ptr<Codec2Client::Interface> intf =
                Codec2Client::CreateInterfaceByName(nameOrAlias.c_str());
            if (!intf) {
                ALOGD("could not create interface for %s'%s'",
                        isAlias ? "alias " : "",
                        nameOrAlias.c_str());
                continue;
            }
            if (parser.getCodecMap().count(nameOrAlias) == 0) {
                if (isAlias) {
                    std::unique_ptr<MediaCodecInfoWriter> baseCodecInfo =
                        writer->findMediaCodecInfo(trait.name.c_str());
                    if (!baseCodecInfo) {
                        ALOGD("alias '%s' not found in xml but canonical codec info '%s' missing",
                                nameOrAlias.c_str(),
                                trait.name.c_str());
                    } else {
                        ALOGD("alias '%s' not found in xml; use an XML <Alias> tag for this",
                                nameOrAlias.c_str());
                        // merge alias into existing codec
                        baseCodecInfo->addAlias(nameOrAlias.c_str());
                    }
                } else {
                    ALOGD("component '%s' not found in xml", trait.name.c_str());
                }
                continue;
            }
            std::string canonName = trait.name;

            // TODO: Remove this block once all codecs are enabled by default.
            switch (option) {
            case 0:
                continue;
            case 1:
                if (hasPrefix(canonName, "c2.vda.")) {
                    break;
                }
                if (hasPrefix(canonName, "c2.android.")) {
                    if (trait.domain == C2Component::DOMAIN_AUDIO) {
                        rank = 1;
                        break;
                    }
                    break;
                }
                if (hasSuffix(canonName, ".avc.decoder") ||
                        hasSuffix(canonName, ".avc.encoder")) {
                    rank = std::numeric_limits<decltype(rank)>::max();
                    break;
                }
                continue;
            case 2:
                if (hasPrefix(canonName, "c2.vda.")) {
                    break;
                }
                if (hasPrefix(canonName, "c2.android.")) {
                    rank = 1;
                    break;
                }
                if (hasSuffix(canonName, ".avc.decoder") ||
                        hasSuffix(canonName, ".avc.encoder")) {
                    rank = std::numeric_limits<decltype(rank)>::max();
                    break;
                }
                continue;
            case 3:
                if (hasPrefix(canonName, "c2.android.")) {
                    rank = 1;
                }
                break;
            }

            const MediaCodecsXmlParser::CodecProperties &codec =
                parser.getCodecMap().at(nameOrAlias);

            // verify that either the codec is explicitly enabled, or one of its domains is
            bool codecEnabled = codec.quirkSet.find("attribute::disabled") == codec.quirkSet.end();
            if (!codecEnabled) {
                for (const std::string &domain : codec.domainSet) {
                    const Switch enabled = isDomainEnabled(domain, settings);
                    ALOGV("codec entry '%s' is in domain '%s' that is '%s'",
                            nameOrAlias.c_str(), domain.c_str(), asString(enabled));
                    if (enabled) {
                        codecEnabled = true;
                        break;
                    }
                }
            }
            // if codec has variants, also check that at least one of them is enabled
            bool variantEnabled = codec.variantSet.empty();
            for (const std::string &variant : codec.variantSet) {
                const Switch enabled = isVariantExpressionEnabled(variant, settings);
                ALOGV("codec entry '%s' has a variant '%s' that is '%s'",
                        nameOrAlias.c_str(), variant.c_str(), asString(enabled));
                if (enabled) {
                    variantEnabled = true;
                    break;
                }
            }
            if (!codecEnabled || !variantEnabled) {
                ALOGD("codec entry for '%s' is disabled", nameOrAlias.c_str());
                continue;
            }

            ALOGV("adding codec entry for '%s'", nameOrAlias.c_str());
            std::unique_ptr<MediaCodecInfoWriter> codecInfo = writer->addMediaCodecInfo();
            codecInfo->setName(nameOrAlias.c_str());
            codecInfo->setOwner(("codec2::" + trait.owner).c_str());

            bool encoder = trait.kind == C2Component::KIND_ENCODER;
            typename std::underlying_type<MediaCodecInfo::Attributes>::type attrs = 0;

            if (encoder) {
                attrs |= MediaCodecInfo::kFlagIsEncoder;
            }
            if (trait.owner == "software") {
                attrs |= MediaCodecInfo::kFlagIsSoftwareOnly;
            } else {
                attrs |= MediaCodecInfo::kFlagIsVendor;
                if (trait.owner == "vendor-software") {
                    attrs |= MediaCodecInfo::kFlagIsSoftwareOnly;
                } else if (codec.quirkSet.find("attribute::software-codec")
                        == codec.quirkSet.end()) {
                    attrs |= MediaCodecInfo::kFlagIsHardwareAccelerated;
                }
            }
            codecInfo->setAttributes(attrs);
            if (!codec.rank.empty()) {
                uint32_t xmlRank;
                char dummy;
                if (sscanf(codec.rank.c_str(), "%u%c", &xmlRank, &dummy) == 1) {
                    rank = xmlRank;
                }
            }
            ALOGV("rank: %u", (unsigned)rank);
            codecInfo->setRank(rank);

            for (const std::string &alias : codec.aliases) {
                ALOGV("adding alias '%s'", alias.c_str());
                codecInfo->addAlias(alias.c_str());
            }

            for (auto typeIt = codec.typeMap.begin(); typeIt != codec.typeMap.end(); ++typeIt) {
                const std::string &mediaType = typeIt->first;
                const Switch typeEnabled = isSettingEnabled(
                        "media-type-" + mediaType, settings, Switch::ENABLED_BY_DEFAULT());
                const Switch domainTypeEnabled = isSettingEnabled(
                        "media-type-" + mediaType + (encoder ? "-encoder" : "-decoder"),
                        settings, Switch::ENABLED_BY_DEFAULT());
                ALOGV("type '%s-%s' is '%s/%s'",
                        mediaType.c_str(), (encoder ? "encoder" : "decoder"),
                        asString(typeEnabled), asString(domainTypeEnabled));
                if (!typeEnabled || !domainTypeEnabled) {
                    ALOGD("media type '%s' for codec entry '%s' is disabled", mediaType.c_str(),
                            nameOrAlias.c_str());
                    continue;
                }

                ALOGI("adding type '%s'", typeIt->first.c_str());
                const MediaCodecsXmlParser::AttributeMap &attrMap = typeIt->second;
                std::unique_ptr<MediaCodecInfo::CapabilitiesWriter> caps =
                    codecInfo->addMediaType(mediaType.c_str());
                for (const auto &v : attrMap) {
                    std::string key = v.first;
                    std::string value = v.second;

                    size_t variantSep = key.find(":::");
                    if (variantSep != std::string::npos) {
                        std::string variant = key.substr(0, variantSep);
                        const Switch enabled = isVariantExpressionEnabled(variant, settings);
                        ALOGV("variant '%s' is '%s'", variant.c_str(), asString(enabled));
                        if (!enabled) {
                            continue;
                        }
                        key = key.substr(variantSep + 3);
                    }

                    if (key.find("feature-") == 0 && key.find("feature-bitrate-modes") != 0) {
                        int32_t intValue = 0;
                        // Ignore trailing bad characters and default to 0.
                        (void)sscanf(value.c_str(), "%d", &intValue);
                        caps->addDetail(key.c_str(), intValue);
                    } else {
                        caps->addDetail(key.c_str(), value.c_str());
                    }
                }

                addSupportedProfileLevels(intf, caps.get(), trait, mediaType);
                addSupportedColorFormats(intf, caps.get(), trait, mediaType);
            }
        }
    }
    return OK;
}

}  // namespace android

extern "C" android::MediaCodecListBuilderBase *CreateBuilder() {
    return new android::Codec2InfoBuilder;
}
