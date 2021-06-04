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
#define LOG_TAG "CCodecConfig"
#include <cutils/properties.h>
#include <log/log.h>

#include <C2Component.h>
#include <C2Debug.h>
#include <C2Param.h>
#include <util/C2InterfaceHelper.h>

#include <media/stagefright/MediaCodecConstants.h>

#include "CCodecConfig.h"
#include "Codec2Mapper.h"

#define DRC_DEFAULT_MOBILE_REF_LEVEL 64  /* 64*-0.25dB = -16 dB below full scale for mobile conf */
#define DRC_DEFAULT_MOBILE_DRC_CUT   127 /* maximum compression of dynamic range for mobile conf */
#define DRC_DEFAULT_MOBILE_DRC_BOOST 127 /* maximum compression of dynamic range for mobile conf */
#define DRC_DEFAULT_MOBILE_DRC_HEAVY 1   /* switch for heavy compression for mobile conf */
#define DRC_DEFAULT_MOBILE_DRC_EFFECT 3  /* MPEG-D DRC effect type; 3 => Limited playback range */
#define DRC_DEFAULT_MOBILE_ENC_LEVEL (-1) /* encoder target level; -1 => the value is unknown, otherwise dB step value (e.g. 64 for -16 dB) */
// names of properties that can be used to override the default DRC settings
#define PROP_DRC_OVERRIDE_REF_LEVEL  "aac_drc_reference_level"
#define PROP_DRC_OVERRIDE_CUT        "aac_drc_cut"
#define PROP_DRC_OVERRIDE_BOOST      "aac_drc_boost"
#define PROP_DRC_OVERRIDE_HEAVY      "aac_drc_heavy"
#define PROP_DRC_OVERRIDE_ENC_LEVEL  "aac_drc_enc_target_level"
#define PROP_DRC_OVERRIDE_EFFECT     "ro.aac_drc_effect_type"

namespace android {

// CCodecConfig

namespace {

/**
 * mapping between SDK and Codec 2.0 configurations.
 */
struct ConfigMapper {
    /**
     * Value mapper (C2Value => C2Value)
     */
    typedef std::function<C2Value(C2Value)> Mapper;

    /// shorthand
    typedef CCodecConfig::Domain Domain;

    ConfigMapper(std::string mediaKey, C2String c2struct, C2String c2field)
        : mDomain(Domain::ALL), mMediaKey(mediaKey), mStruct(c2struct), mField(c2field) { }

    /// Limits this parameter to the given domain
    ConfigMapper &limitTo(uint32_t domain) {
        C2_CHECK(domain & Domain::GUARD_BIT);
        mDomain = Domain(mDomain & domain);
        return *this;
    }

    /// Adds SDK => Codec 2.0 mapper (should not be in the SDK format)
    ConfigMapper &withMapper(Mapper mapper) {
        C2_CHECK(!mMapper);
        C2_CHECK(!mReverse);
        mMapper = mapper;
        return *this;
    }

    /// Adds SDK <=> Codec 2.0 value mappers
    ConfigMapper &withMappers(Mapper mapper, Mapper reverse) {
        C2_CHECK(!mMapper);
        C2_CHECK(!mReverse);
        mMapper = mapper;
        mReverse = reverse;
        return *this;
    }

    /// Adds SDK <=> Codec 2.0 value mappers based on C2Mapper
    template<typename C2Type, typename SdkType=int32_t>
    ConfigMapper &withC2Mappers() {
        C2_CHECK(!mMapper);
        C2_CHECK(!mReverse);
        mMapper = [](C2Value v) -> C2Value {
            SdkType sdkValue;
            C2Type c2Value;
            if (v.get(&sdkValue) && C2Mapper::map(sdkValue, &c2Value)) {
                return c2Value;
            }
            return C2Value();
        };
        mReverse = [](C2Value v) -> C2Value {
            SdkType sdkValue;
            C2Type c2Value;
            using C2ValueType=typename _c2_reduce_enum_to_underlying_type<C2Type>::type;
            if (v.get((C2ValueType*)&c2Value) && C2Mapper::map(c2Value, &sdkValue)) {
                return sdkValue;
            }
            return C2Value();
        };
        return *this;
    }

    /// Maps from SDK values in an AMessage to a suitable C2Value.
    C2Value mapFromMessage(const AMessage::ItemData &item) const {
        C2Value value;
        int32_t int32Value;
        int64_t int64Value;
        float floatValue;
        double doubleValue;
        if (item.find(&int32Value)) {
            value = int32Value;
        } else if (item.find(&int64Value)) {
            value = int64Value;
        } else if (item.find(&floatValue)) {
            value = floatValue;
        } else if (item.find(&doubleValue)) {
            value = (float)doubleValue;
        }
        if (value.type() != C2Value::NO_INIT && mMapper) {
            value = mMapper(value);
        }
        return value;
    }

    /// Maps from a C2Value to an SDK value in an AMessage.
    AMessage::ItemData mapToMessage(C2Value value) const {
        AMessage::ItemData item;
        int32_t int32Value;
        uint32_t uint32Value;
        int64_t int64Value;
        uint64_t uint64Value;
        float floatValue;
        if (value.type() != C2Value::NO_INIT && mReverse) {
            value = mReverse(value);
        }
        if (value.get(&int32Value)) {
            item.set(int32Value);
        } else if (value.get(&uint32Value) && uint32Value <= uint32_t(INT32_MAX)) {
            // SDK does not support unsigned values
            item.set((int32_t)uint32Value);
        } else if (value.get(&int64Value)) {
            item.set(int64Value);
        } else if (value.get(&uint64Value) && uint64Value <= uint64_t(INT64_MAX)) {
            // SDK does not support unsigned values
            item.set((int64_t)uint64Value);
        } else if (value.get(&floatValue)) {
            item.set(floatValue);
        }
        return item;
    }

    Domain domain() const { return mDomain; }
    std::string mediaKey() const { return mMediaKey; }
    std::string path() const { return mField.size() ? mStruct + '.' + mField : mStruct; }
    Mapper mapper() const { return mMapper; }
    Mapper reverse() const { return mReverse; }

private:
    Domain mDomain;         ///< parameter domain (mask) containing port, kind and config domains
    std::string mMediaKey;  ///< SDK key
    C2String mStruct;       ///< Codec 2.0 struct name
    C2String mField;        ///< Codec 2.0 field name
    Mapper mMapper;         ///< optional SDK => Codec 2.0 value mapper
    Mapper mReverse;        ///< optional Codec 2.0 => SDK value mapper
};

template <typename PORT, typename STREAM>
AString QueryMediaTypeImpl(
        const std::shared_ptr<Codec2Client::Component> &component) {
    AString mediaType;
    std::vector<std::unique_ptr<C2Param>> queried;
    c2_status_t c2err = component->query(
            {}, { PORT::PARAM_TYPE, STREAM::PARAM_TYPE }, C2_DONT_BLOCK, &queried);
    if (c2err != C2_OK && queried.size() == 0) {
        ALOGD("Query media type failed => %s", asString(c2err));
    } else {
        PORT *portMediaType =
            PORT::From(queried[0].get());
        if (portMediaType) {
            mediaType = AString(
                    portMediaType->m.value,
                    strnlen(portMediaType->m.value, portMediaType->flexCount()));
        } else {
            STREAM *streamMediaType = STREAM::From(queried[0].get());
            if (streamMediaType) {
                mediaType = AString(
                        streamMediaType->m.value,
                        strnlen(streamMediaType->m.value, streamMediaType->flexCount()));
            }
        }
        ALOGD("read media type: %s", mediaType.c_str());
    }
    return mediaType;
}

AString QueryMediaType(
        bool input, const std::shared_ptr<Codec2Client::Component> &component) {
    typedef C2PortMediaTypeSetting P;
    typedef C2StreamMediaTypeSetting S;
    if (input) {
        return QueryMediaTypeImpl<P::input, S::input>(component);
    } else {
        return QueryMediaTypeImpl<P::output, S::output>(component);
    }
}

}  // namespace

/**
 * Set of standard parameters used by CCodec that are exposed to MediaCodec.
 */
struct StandardParams {
    typedef CCodecConfig::Domain Domain;

    // standard (MediaCodec) params are keyed by media format key
    typedef std::string SdkKey;

    /// used to return reference to no config mappers in getConfigMappersForSdkKey
    static const std::vector<ConfigMapper> NO_MAPPERS;

    /// Returns Codec 2.0 equivalent parameters for an SDK format key.
    const std::vector<ConfigMapper> &getConfigMappersForSdkKey(std::string key) const {
        auto it = mConfigMappers.find(key);
        if (it == mConfigMappers.end()) {
            if (mComplained.count(key) == 0) {
                ALOGD("no c2 equivalents for %s", key.c_str());
                mComplained.insert(key);
            }
            return NO_MAPPERS;
        }
        ALOGV("found %zu eqs for %s", it->second.size(), key.c_str());
        return it->second;
    }

    /**
     * Adds a SDK <=> Codec 2.0 parameter mapping. Multiple Codec 2.0 parameters may map to a
     * single SDK key, in which case they shall be ordered from least authoritative to most
     * authoritative. When constructing SDK formats, the last mapped Codec 2.0 parameter that
     * is supported by the component will determine the exposed value. (TODO: perhaps restrict this
     * by domain.)
     */
    void add(const ConfigMapper &cm) {
        auto it = mConfigMappers.find(cm.mediaKey());
        ALOGV("%c%c%c%c %c%c%c %04x %9s %s => %s",
              ((cm.domain() & Domain::IS_INPUT) ? 'I' : ' '),
              ((cm.domain() & Domain::IS_OUTPUT) ? 'O' : ' '),
              ((cm.domain() & Domain::IS_CODED) ? 'C' : ' '),
              ((cm.domain() & Domain::IS_RAW) ? 'R' : ' '),
              ((cm.domain() & Domain::IS_CONFIG) ? 'c' : ' '),
              ((cm.domain() & Domain::IS_PARAM) ? 'p' : ' '),
              ((cm.domain() & Domain::IS_READ) ? 'r' : ' '),
              cm.domain(),
              it == mConfigMappers.end() ? "adding" : "extending",
              cm.mediaKey().c_str(), cm.path().c_str());
        if (it == mConfigMappers.end()) {
            std::vector<ConfigMapper> eqs = { cm };
            mConfigMappers.emplace(cm.mediaKey(), eqs);
        } else {
            it->second.push_back(cm);
        }
    }

    /**
     * Returns all paths for a specific domain.
     *
     * \param any maximum domain mask. Returned parameters must match at least one of the domains
     *            in the mask.
     * \param all minimum domain mask. Returned parameters must match all of the domains in the
     *            mask. This is restricted to the bits of the maximum mask.
     */
    std::vector<std::string> getPathsForDomain(
            Domain any, Domain all = Domain::ALL) const {
        std::vector<std::string> res;
        for (const std::pair<std::string, std::vector<ConfigMapper>> &el : mConfigMappers) {
            for (const ConfigMapper &cm : el.second) {
                ALOGV("filtering %s %x %x %x %x", cm.path().c_str(), cm.domain(), any,
                        (cm.domain() & any), (cm.domain() & any & all));
                if ((cm.domain() & any) && ((cm.domain() & any & all) == (any & all))) {
                    res.push_back(cm.path());
                }
            }
        }
        return res;
    }

    /**
     * Returns SDK <=> Codec 2.0 mappings.
     *
     * TODO: replace these with better methods as this exposes the inner structure.
     */
    const std::map<SdkKey, std::vector<ConfigMapper>> getKeys() const {
        return mConfigMappers;
    }

private:
    std::map<SdkKey, std::vector<ConfigMapper>> mConfigMappers;
    mutable std::set<std::string> mComplained;
};

const std::vector<ConfigMapper> StandardParams::NO_MAPPERS;


CCodecConfig::CCodecConfig()
    : mInputFormat(new AMessage),
      mOutputFormat(new AMessage),
      mUsingSurface(false) { }

void CCodecConfig::initializeStandardParams() {
    typedef Domain D;
    mStandardParams = std::make_shared<StandardParams>();
    std::function<void(const ConfigMapper &)> add =
        [params = mStandardParams](const ConfigMapper &cm) {
            params->add(cm);
    };
    std::function<void(const ConfigMapper &)> deprecated = add;

    // allow int32 or float SDK values and represent them as float
    ConfigMapper::Mapper makeFloat = [](C2Value v) -> C2Value {
        // convert from i32 to float
        int32_t i32Value;
        float fpValue;
        if (v.get(&i32Value)) {
            return (float)i32Value;
        } else if (v.get(&fpValue)) {
            return fpValue;
        }
        return C2Value();
    };

    ConfigMapper::Mapper negate = [](C2Value v) -> C2Value {
        int32_t value;
        if (v.get(&value)) {
            return -value;
        }
        return C2Value();
    };

    add(ConfigMapper(KEY_MIME,     C2_PARAMKEY_INPUT_MEDIA_TYPE,    "value")
        .limitTo(D::INPUT & D::READ));
    add(ConfigMapper(KEY_MIME,     C2_PARAMKEY_OUTPUT_MEDIA_TYPE,   "value")
        .limitTo(D::OUTPUT & D::READ));

    add(ConfigMapper(KEY_BIT_RATE, C2_PARAMKEY_BITRATE, "value")
        .limitTo(D::ENCODER & D::OUTPUT));
    // we also need to put the bitrate in the max bitrate field
    add(ConfigMapper(KEY_MAX_BIT_RATE, C2_PARAMKEY_BITRATE, "value")
        .limitTo(D::ENCODER & D::READ & D::OUTPUT));
    add(ConfigMapper(PARAMETER_KEY_VIDEO_BITRATE, C2_PARAMKEY_BITRATE, "value")
        .limitTo(D::ENCODER & D::VIDEO & D::PARAM));
    add(ConfigMapper(KEY_BITRATE_MODE, C2_PARAMKEY_BITRATE_MODE, "value")
        .limitTo(D::ENCODER & D::CODED)
        .withC2Mappers<C2Config::bitrate_mode_t>());
    // remove when codecs switch to PARAMKEY and new modes
    deprecated(ConfigMapper(KEY_BITRATE_MODE, "coded.bitrate-mode", "value")
               .limitTo(D::ENCODER));
    add(ConfigMapper(KEY_FRAME_RATE, C2_PARAMKEY_FRAME_RATE, "value")
        .limitTo(D::VIDEO)
        .withMappers(makeFloat, [](C2Value v) -> C2Value {
            // read back always as int
            float value;
            if (v.get(&value)) {
                return (int32_t)value;
            }
            return C2Value();
        }));

    add(ConfigMapper(KEY_MAX_INPUT_SIZE, C2_PARAMKEY_INPUT_MAX_BUFFER_SIZE, "value")
        .limitTo(D::INPUT));
    // remove when codecs switch to PARAMKEY
    deprecated(ConfigMapper(KEY_MAX_INPUT_SIZE, "coded.max-frame-size", "value")
               .limitTo(D::INPUT));

    // Rotation
    // Note: SDK rotation is clock-wise, while C2 rotation is counter-clock-wise
    add(ConfigMapper(KEY_ROTATION, C2_PARAMKEY_VUI_ROTATION, "value")
        .limitTo(D::VIDEO & D::CODED)
        .withMappers(negate, negate));
    add(ConfigMapper(KEY_ROTATION, C2_PARAMKEY_ROTATION, "value")
        .limitTo(D::VIDEO & D::RAW)
        .withMappers(negate, negate));

    // android 'video-scaling'
    add(ConfigMapper("android._video-scaling", C2_PARAMKEY_SURFACE_SCALING_MODE, "value")
        .limitTo(D::VIDEO & D::DECODER & D::RAW));

    // Color Aspects
    //
    // configure default for decoders
    add(ConfigMapper(KEY_COLOR_RANGE,       C2_PARAMKEY_DEFAULT_COLOR_ASPECTS,   "range")
        .limitTo((D::VIDEO | D::IMAGE) & D::DECODER  & D::CODED & (D::CONFIG | D::PARAM))
        .withC2Mappers<C2Color::range_t>());
    add(ConfigMapper(KEY_COLOR_TRANSFER,    C2_PARAMKEY_DEFAULT_COLOR_ASPECTS,   "transfer")
        .limitTo((D::VIDEO | D::IMAGE) & D::DECODER  & D::CODED & (D::CONFIG | D::PARAM))
        .withC2Mappers<C2Color::transfer_t>());
    add(ConfigMapper("color-primaries",     C2_PARAMKEY_DEFAULT_COLOR_ASPECTS,   "primaries")
        .limitTo((D::VIDEO | D::IMAGE) & D::DECODER  & D::CODED & (D::CONFIG | D::PARAM)));
    add(ConfigMapper("color-matrix",        C2_PARAMKEY_DEFAULT_COLOR_ASPECTS,   "matrix")
        .limitTo((D::VIDEO | D::IMAGE) & D::DECODER  & D::CODED & (D::CONFIG | D::PARAM)));

    // read back final for decoder output (also, configure final aspects as well. This should be
    // overwritten based on coded/default values if component supports color aspects, but is used
    // as final values if component does not support aspects at all)
    add(ConfigMapper(KEY_COLOR_RANGE,       C2_PARAMKEY_COLOR_ASPECTS,   "range")
        .limitTo((D::VIDEO | D::IMAGE) & D::DECODER  & D::RAW)
        .withC2Mappers<C2Color::range_t>());
    add(ConfigMapper(KEY_COLOR_TRANSFER,    C2_PARAMKEY_COLOR_ASPECTS,   "transfer")
        .limitTo((D::VIDEO | D::IMAGE) & D::DECODER  & D::RAW)
        .withC2Mappers<C2Color::transfer_t>());
    add(ConfigMapper("color-primaries",     C2_PARAMKEY_COLOR_ASPECTS,   "primaries")
        .limitTo((D::VIDEO | D::IMAGE) & D::DECODER  & D::RAW));
    add(ConfigMapper("color-matrix",        C2_PARAMKEY_COLOR_ASPECTS,   "matrix")
        .limitTo((D::VIDEO | D::IMAGE) & D::DECODER  & D::RAW));

    // configure source aspects for encoders and read them back on the coded(!) port.
    // This is to ensure muxing the desired aspects into the container.
    add(ConfigMapper(KEY_COLOR_RANGE,       C2_PARAMKEY_COLOR_ASPECTS,   "range")
        .limitTo((D::VIDEO | D::IMAGE) & D::ENCODER  & D::CODED)
        .withC2Mappers<C2Color::range_t>());
    add(ConfigMapper(KEY_COLOR_TRANSFER,    C2_PARAMKEY_COLOR_ASPECTS,   "transfer")
        .limitTo((D::VIDEO | D::IMAGE) & D::ENCODER  & D::CODED)
        .withC2Mappers<C2Color::transfer_t>());
    add(ConfigMapper("color-primaries",     C2_PARAMKEY_COLOR_ASPECTS,   "primaries")
        .limitTo((D::VIDEO | D::IMAGE) & D::ENCODER  & D::CODED));
    add(ConfigMapper("color-matrix",        C2_PARAMKEY_COLOR_ASPECTS,   "matrix")
        .limitTo((D::VIDEO | D::IMAGE) & D::ENCODER  & D::CODED));

    // read back coded aspects for encoders (on the raw port), but also configure
    // desired aspects here.
    add(ConfigMapper(KEY_COLOR_RANGE,       C2_PARAMKEY_VUI_COLOR_ASPECTS,   "range")
        .limitTo((D::VIDEO | D::IMAGE) & D::ENCODER  & D::RAW)
        .withC2Mappers<C2Color::range_t>());
    add(ConfigMapper(KEY_COLOR_TRANSFER,    C2_PARAMKEY_VUI_COLOR_ASPECTS,   "transfer")
        .limitTo((D::VIDEO | D::IMAGE) & D::ENCODER  & D::RAW)
        .withC2Mappers<C2Color::transfer_t>());
    add(ConfigMapper("color-primaries",     C2_PARAMKEY_VUI_COLOR_ASPECTS,   "primaries")
        .limitTo((D::VIDEO | D::IMAGE) & D::ENCODER  & D::RAW));
    add(ConfigMapper("color-matrix",        C2_PARAMKEY_VUI_COLOR_ASPECTS,   "matrix")
        .limitTo((D::VIDEO | D::IMAGE) & D::ENCODER  & D::RAW));

    // Dataspace
    add(ConfigMapper("android._dataspace", C2_PARAMKEY_DATA_SPACE, "value")
        .limitTo((D::VIDEO | D::IMAGE) & D::RAW));

    // HDR
    add(ConfigMapper("smpte2086.red.x", C2_PARAMKEY_HDR_STATIC_INFO, "mastering.red.x")
        .limitTo((D::VIDEO | D::IMAGE) & D::RAW));
    add(ConfigMapper("smpte2086.red.y", C2_PARAMKEY_HDR_STATIC_INFO, "mastering.red.y")
        .limitTo((D::VIDEO | D::IMAGE) & D::RAW));
    add(ConfigMapper("smpte2086.green.x", C2_PARAMKEY_HDR_STATIC_INFO, "mastering.green.x")
        .limitTo((D::VIDEO | D::IMAGE) & D::RAW));
    add(ConfigMapper("smpte2086.green.y", C2_PARAMKEY_HDR_STATIC_INFO, "mastering.green.y")
        .limitTo((D::VIDEO | D::IMAGE) & D::RAW));
    add(ConfigMapper("smpte2086.blue.x", C2_PARAMKEY_HDR_STATIC_INFO, "mastering.blue.x")
        .limitTo((D::VIDEO | D::IMAGE) & D::RAW));
    add(ConfigMapper("smpte2086.blue.y", C2_PARAMKEY_HDR_STATIC_INFO, "mastering.blue.y")
        .limitTo((D::VIDEO | D::IMAGE) & D::RAW));
    add(ConfigMapper("smpte2086.white.x", C2_PARAMKEY_HDR_STATIC_INFO, "mastering.white.x")
        .limitTo((D::VIDEO | D::IMAGE) & D::RAW));
    add(ConfigMapper("smpte2086.white.y", C2_PARAMKEY_HDR_STATIC_INFO, "mastering.white.y")
        .limitTo((D::VIDEO | D::IMAGE) & D::RAW));
    add(ConfigMapper("smpte2086.max-luminance", C2_PARAMKEY_HDR_STATIC_INFO, "mastering.max-luminance")
        .limitTo((D::VIDEO | D::IMAGE) & D::RAW));
    add(ConfigMapper("smpte2086.min-luminance", C2_PARAMKEY_HDR_STATIC_INFO, "mastering.min-luminance")
        .limitTo((D::VIDEO | D::IMAGE) & D::RAW));
    add(ConfigMapper("cta861.max-cll", C2_PARAMKEY_HDR_STATIC_INFO, "max-cll")
        .limitTo((D::VIDEO | D::IMAGE) & D::RAW));
    add(ConfigMapper("cta861.max-fall", C2_PARAMKEY_HDR_STATIC_INFO, "max-fall")
        .limitTo((D::VIDEO | D::IMAGE) & D::RAW));

    add(ConfigMapper(std::string(KEY_FEATURE_) + FEATURE_SecurePlayback,
                     C2_PARAMKEY_SECURE_MODE, "value"));

    add(ConfigMapper(KEY_PREPEND_HEADERS_TO_SYNC_FRAMES,
                     C2_PARAMKEY_PREPEND_HEADER_MODE, "value")
        .limitTo(D::ENCODER & D::VIDEO)
        .withMappers([](C2Value v) -> C2Value {
            int32_t value;
            if (v.get(&value)) {
                return value ? C2Value(C2Config::PREPEND_HEADER_TO_ALL_SYNC)
                             : C2Value(C2Config::PREPEND_HEADER_TO_NONE);
            }
            return C2Value();
        }, [](C2Value v) -> C2Value {
            C2Config::prepend_header_mode_t value;
            using C2ValueType=typename _c2_reduce_enum_to_underlying_type<decltype(value)>::type;
            if (v.get((C2ValueType *)&value)) {
                switch (value) {
                    case C2Config::PREPEND_HEADER_TO_NONE:      return 0;
                    case C2Config::PREPEND_HEADER_TO_ALL_SYNC:  return 1;
                    case C2Config::PREPEND_HEADER_ON_CHANGE:    [[fallthrough]];
                    default:                                    return C2Value();
                }
            }
            return C2Value();
        }));
    // remove when codecs switch to PARAMKEY
    deprecated(ConfigMapper(KEY_PREPEND_HEADERS_TO_SYNC_FRAMES,
                            "coding.add-csd-to-sync-frames", "value")
               .limitTo(D::ENCODER & D::VIDEO));
    // convert to timestamp base
    add(ConfigMapper(KEY_I_FRAME_INTERVAL, C2_PARAMKEY_SYNC_FRAME_INTERVAL, "value")
        .limitTo(D::VIDEO & D::ENCODER & D::CONFIG)
        .withMapper([](C2Value v) -> C2Value {
            // convert from i32 to float
            int32_t i32Value;
            float fpValue;
            if (v.get(&i32Value)) {
                return int64_t(1000000) * i32Value;
            } else if (v.get(&fpValue)) {
                return int64_t(c2_min(1000000 * fpValue + 0.5, (double)INT64_MAX));
            }
            return C2Value();
        }));
    // remove when codecs switch to proper coding.gop (add support for calculating gop)
    deprecated(ConfigMapper("i-frame-period", "coding.gop", "intra-period")
               .limitTo(D::ENCODER & D::VIDEO));
    add(ConfigMapper(KEY_INTRA_REFRESH_PERIOD, C2_PARAMKEY_INTRA_REFRESH, "period")
        .limitTo(D::VIDEO & D::ENCODER)
        .withMappers(makeFloat, [](C2Value v) -> C2Value {
            // read back always as int
            float value;
            if (v.get(&value)) {
                return (int32_t)value;
            }
            return C2Value();
        }));
    deprecated(ConfigMapper(PARAMETER_KEY_REQUEST_SYNC_FRAME,
                     "coding.request-sync", "value")
        .limitTo(D::PARAM & D::ENCODER)
        .withMapper([](C2Value) -> C2Value { return uint32_t(1); }));
    add(ConfigMapper(PARAMETER_KEY_REQUEST_SYNC_FRAME,
                     C2_PARAMKEY_REQUEST_SYNC_FRAME, "value")
        .limitTo(D::PARAM & D::ENCODER)
        .withMapper([](C2Value) -> C2Value { return uint32_t(1); }));

    add(ConfigMapper(KEY_OPERATING_RATE,   C2_PARAMKEY_OPERATING_RATE,     "value")
        .limitTo(D::PARAM | D::CONFIG) // write-only
        .withMapper(makeFloat));
    // C2 priorities are inverted
    add(ConfigMapper(KEY_PRIORITY,         C2_PARAMKEY_PRIORITY,           "value")
        .withMappers(negate, negate));
    // remove when codecs switch to PARAMKEY
    deprecated(ConfigMapper(KEY_OPERATING_RATE,   "ctrl.operating-rate",     "value")
               .withMapper(makeFloat));
    deprecated(ConfigMapper(KEY_PRIORITY,         "ctrl.priority",           "value"));

    add(ConfigMapper(KEY_WIDTH,         C2_PARAMKEY_PICTURE_SIZE,       "width")
        .limitTo(D::VIDEO | D::IMAGE));
    add(ConfigMapper(KEY_HEIGHT,        C2_PARAMKEY_PICTURE_SIZE,       "height")
        .limitTo(D::VIDEO | D::IMAGE));

    add(ConfigMapper("crop-left",       C2_PARAMKEY_CROP_RECT,       "left")
        .limitTo(D::VIDEO | D::IMAGE));
    add(ConfigMapper("crop-top",        C2_PARAMKEY_CROP_RECT,       "top")
        .limitTo(D::VIDEO | D::IMAGE));
    add(ConfigMapper("crop-width",      C2_PARAMKEY_CROP_RECT,       "width")
        .limitTo(D::VIDEO | D::IMAGE));
    add(ConfigMapper("crop-height",     C2_PARAMKEY_CROP_RECT,       "height")
        .limitTo(D::VIDEO | D::IMAGE));

    add(ConfigMapper(KEY_MAX_WIDTH,     C2_PARAMKEY_MAX_PICTURE_SIZE,    "width")
        .limitTo((D::VIDEO | D::IMAGE) & D::RAW));
    add(ConfigMapper(KEY_MAX_HEIGHT,    C2_PARAMKEY_MAX_PICTURE_SIZE,    "height")
        .limitTo((D::VIDEO | D::IMAGE) & D::RAW));

    add(ConfigMapper("csd-0",           C2_PARAMKEY_INIT_DATA,       "value")
        .limitTo(D::OUTPUT & D::READ));

    add(ConfigMapper(KEY_HDR10_PLUS_INFO, C2_PARAMKEY_INPUT_HDR10_PLUS_INFO, "value")
        .limitTo(D::VIDEO & D::PARAM & D::INPUT));

    add(ConfigMapper(KEY_HDR10_PLUS_INFO, C2_PARAMKEY_OUTPUT_HDR10_PLUS_INFO, "value")
        .limitTo(D::VIDEO & D::OUTPUT));

    add(ConfigMapper(C2_PARAMKEY_TEMPORAL_LAYERING, C2_PARAMKEY_TEMPORAL_LAYERING, "")
        .limitTo(D::ENCODER & D::VIDEO & D::OUTPUT));

    // Pixel Format (use local key for actual pixel format as we don't distinguish between
    // SDK layouts for flexible format and we need the actual SDK color format in the media format)
    add(ConfigMapper("android._color-format",  C2_PARAMKEY_PIXEL_FORMAT, "value")
        .limitTo((D::VIDEO | D::IMAGE) & D::RAW)
        .withMappers([](C2Value v) -> C2Value {
            int32_t value;
            if (v.get(&value)) {
                switch (value) {
                    case COLOR_FormatSurface:
                        return (uint32_t)HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED;
                    case COLOR_FormatYUV420Flexible:
                        return (uint32_t)HAL_PIXEL_FORMAT_YCBCR_420_888;
                    case COLOR_FormatYUV420Planar:
                    case COLOR_FormatYUV420SemiPlanar:
                    case COLOR_FormatYUV420PackedPlanar:
                    case COLOR_FormatYUV420PackedSemiPlanar:
                        return (uint32_t)HAL_PIXEL_FORMAT_YV12;
                    default:
                        // TODO: support some sort of passthrough
                        break;
                }
            }
            return C2Value();
        }, [](C2Value v) -> C2Value {
            uint32_t value;
            if (v.get(&value)) {
                switch (value) {
                    case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
                        return COLOR_FormatSurface;
                    case HAL_PIXEL_FORMAT_YV12:
                    case HAL_PIXEL_FORMAT_YCBCR_420_888:
                        return COLOR_FormatYUV420Flexible;
                    default:
                        // TODO: support some sort of passthrough
                        break;
                }
            }
            return C2Value();
        }));

    add(ConfigMapper(KEY_CHANNEL_COUNT, C2_PARAMKEY_CHANNEL_COUNT,       "value")
        .limitTo(D::AUDIO)); // read back to both formats
    add(ConfigMapper(KEY_CHANNEL_COUNT, C2_PARAMKEY_CODED_CHANNEL_COUNT, "value")
        .limitTo(D::AUDIO & D::CODED));

    add(ConfigMapper(KEY_SAMPLE_RATE,   C2_PARAMKEY_SAMPLE_RATE,        "value")
        .limitTo(D::AUDIO)); // read back to both port formats
    add(ConfigMapper(KEY_SAMPLE_RATE,   C2_PARAMKEY_CODED_SAMPLE_RATE,  "value")
        .limitTo(D::AUDIO & D::CODED));

    add(ConfigMapper(KEY_PCM_ENCODING,  C2_PARAMKEY_PCM_ENCODING,       "value")
        .limitTo(D::AUDIO)
        .withMappers([](C2Value v) -> C2Value {
            int32_t value;
            C2Config::pcm_encoding_t to;
            if (v.get(&value) && C2Mapper::map(value, &to)) {
                return to;
            }
            return C2Value();
        }, [](C2Value v) -> C2Value {
            C2Config::pcm_encoding_t value;
            int32_t to;
            using C2ValueType=typename _c2_reduce_enum_to_underlying_type<decltype(value)>::type;
            if (v.get((C2ValueType*)&value) && C2Mapper::map(value, &to)) {
                return to;
            }
            return C2Value();
        }));

    add(ConfigMapper(KEY_IS_ADTS, C2_PARAMKEY_AAC_PACKAGING, "value")
        .limitTo(D::AUDIO & D::CODED)
        .withMappers([](C2Value v) -> C2Value {
            int32_t value;
            if (v.get(&value) && value) {
                return C2Config::AAC_PACKAGING_ADTS;
            }
            return C2Value();
        }, [](C2Value v) -> C2Value {
            uint32_t value;
            if (v.get(&value) && value == C2Config::AAC_PACKAGING_ADTS) {
                return (int32_t)1;
            }
            return C2Value();
        }));

    std::shared_ptr<C2Mapper::ProfileLevelMapper> mapper =
        C2Mapper::GetProfileLevelMapper(mCodingMediaType);

    add(ConfigMapper(KEY_PROFILE, C2_PARAMKEY_PROFILE_LEVEL, "profile")
        .limitTo(D::CODED)
        .withMappers([mapper](C2Value v) -> C2Value {
            C2Config::profile_t c2 = PROFILE_UNUSED;
            int32_t sdk;
            if (mapper && v.get(&sdk) && mapper->mapProfile(sdk, &c2)) {
                return c2;
            }
            return PROFILE_UNUSED;
        }, [mapper](C2Value v) -> C2Value {
            C2Config::profile_t c2;
            int32_t sdk;
            using C2ValueType=typename _c2_reduce_enum_to_underlying_type<decltype(c2)>::type;
            if (mapper && v.get((C2ValueType*)&c2) && mapper->mapProfile(c2, &sdk)) {
                return sdk;
            }
            return C2Value();
        }));

    add(ConfigMapper(KEY_LEVEL, C2_PARAMKEY_PROFILE_LEVEL, "level")
        .limitTo(D::CODED)
        .withMappers([mapper](C2Value v) -> C2Value {
            C2Config::level_t c2 = LEVEL_UNUSED;
            int32_t sdk;
            if (mapper && v.get(&sdk) && mapper->mapLevel(sdk, &c2)) {
                return c2;
            }
            return LEVEL_UNUSED;
        }, [mapper](C2Value v) -> C2Value {
            C2Config::level_t c2;
            int32_t sdk;
            using C2ValueType=typename _c2_reduce_enum_to_underlying_type<decltype(c2)>::type;
            if (mapper && v.get((C2ValueType*)&c2) && mapper->mapLevel(c2, &sdk)) {
                return sdk;
            }
            return C2Value();
        }));

    // convert to dBFS and add default
    add(ConfigMapper(KEY_AAC_DRC_TARGET_REFERENCE_LEVEL, C2_PARAMKEY_DRC_TARGET_REFERENCE_LEVEL, "value")
        .limitTo(D::AUDIO & D::DECODER & D::CONFIG)
        .withMapper([](C2Value v) -> C2Value {
            int32_t value;
            if (!v.get(&value) || value < 0) {
                value = property_get_int32(PROP_DRC_OVERRIDE_REF_LEVEL, DRC_DEFAULT_MOBILE_REF_LEVEL);
            }
            return float(-0.25 * c2_min(value, 127));
        }));

    // convert to 0-1 (%) and add default
    add(ConfigMapper(KEY_AAC_DRC_ATTENUATION_FACTOR, C2_PARAMKEY_DRC_ATTENUATION_FACTOR, "value")
        .limitTo(D::AUDIO & D::DECODER & D::CONFIG)
        .withMapper([](C2Value v) -> C2Value {
            int32_t value;
            if (!v.get(&value) || value < 0) {
                value = property_get_int32(PROP_DRC_OVERRIDE_CUT, DRC_DEFAULT_MOBILE_DRC_CUT);
            }
            return float(c2_min(value, 127) / 127.);
        }));

    // convert to 0-1 (%) and add default
    add(ConfigMapper(KEY_AAC_DRC_BOOST_FACTOR, C2_PARAMKEY_DRC_BOOST_FACTOR, "value")
        .limitTo(D::AUDIO & D::DECODER & D::CONFIG)
        .withMapper([](C2Value v) -> C2Value {
            int32_t value;
            if (!v.get(&value) || value < 0) {
                value = property_get_int32(PROP_DRC_OVERRIDE_BOOST, DRC_DEFAULT_MOBILE_DRC_BOOST);
            }
            return float(c2_min(value, 127) / 127.);
        }));

    // convert to compression type and add default
    add(ConfigMapper(KEY_AAC_DRC_HEAVY_COMPRESSION, C2_PARAMKEY_DRC_COMPRESSION_MODE, "value")
        .limitTo(D::AUDIO & D::DECODER & D::CONFIG)
        .withMapper([](C2Value v) -> C2Value {
            int32_t value;
            if (!v.get(&value) || value < 0) {
                value = property_get_int32(PROP_DRC_OVERRIDE_HEAVY, DRC_DEFAULT_MOBILE_DRC_HEAVY);
            }
            return value == 1 ? C2Config::DRC_COMPRESSION_HEAVY : C2Config::DRC_COMPRESSION_LIGHT;
        }));

    // convert to dBFS and add default
    add(ConfigMapper(KEY_AAC_ENCODED_TARGET_LEVEL, C2_PARAMKEY_DRC_ENCODED_TARGET_LEVEL, "value")
        .limitTo(D::AUDIO & D::DECODER & D::CONFIG)
        .withMapper([](C2Value v) -> C2Value {
            int32_t value;
            if (!v.get(&value) || value < 0) {
                value = property_get_int32(PROP_DRC_OVERRIDE_ENC_LEVEL, DRC_DEFAULT_MOBILE_ENC_LEVEL);
            }
            return float(-0.25 * c2_min(value, 127));
        }));

    // convert to effect type (these map to SDK values) and add default
    add(ConfigMapper(KEY_AAC_DRC_EFFECT_TYPE, C2_PARAMKEY_DRC_EFFECT_TYPE, "value")
        .limitTo(D::AUDIO & D::DECODER & D::CONFIG)
        .withMapper([](C2Value v) -> C2Value {
            int32_t value;
            if (!v.get(&value) || value < -1 || value > 8) {
                value = property_get_int32(PROP_DRC_OVERRIDE_EFFECT, DRC_DEFAULT_MOBILE_DRC_EFFECT);
                // ensure value is within range
                if (value < -1 || value > 8) {
                    value = DRC_DEFAULT_MOBILE_DRC_EFFECT;
                }
            }
            return value;
        }));

    add(ConfigMapper(KEY_AAC_MAX_OUTPUT_CHANNEL_COUNT, C2_PARAMKEY_MAX_CHANNEL_COUNT, "value")
        .limitTo(D::AUDIO));

    add(ConfigMapper(KEY_AAC_SBR_MODE, C2_PARAMKEY_AAC_SBR_MODE, "value")
        .limitTo(D::AUDIO & D::ENCODER & D::CONFIG)
        .withMapper([](C2Value v) -> C2Value {
            int32_t value;
            if (!v.get(&value) || value < 0) {
                return C2Config::AAC_SBR_AUTO;
            }
            switch (value) {
                case 0: return C2Config::AAC_SBR_OFF;
                case 1: return C2Config::AAC_SBR_SINGLE_RATE;
                case 2: return C2Config::AAC_SBR_DUAL_RATE;
                default: return C2Config::AAC_SBR_AUTO + 1; // invalid value
            }
        }));

    add(ConfigMapper(KEY_QUALITY, C2_PARAMKEY_QUALITY, "value")
        .limitTo(D::ENCODER & (D::CONFIG | D::PARAM)));
    add(ConfigMapper(KEY_FLAC_COMPRESSION_LEVEL, C2_PARAMKEY_COMPLEXITY, "value")
        .limitTo(D::AUDIO & D::ENCODER));
    add(ConfigMapper("complexity", C2_PARAMKEY_COMPLEXITY, "value")
        .limitTo(D::ENCODER & (D::CONFIG | D::PARAM)));

    add(ConfigMapper(KEY_GRID_COLUMNS, C2_PARAMKEY_TILE_LAYOUT, "columns")
        .limitTo(D::IMAGE));
    add(ConfigMapper(KEY_GRID_ROWS, C2_PARAMKEY_TILE_LAYOUT, "rows")
        .limitTo(D::IMAGE));
    add(ConfigMapper(KEY_TILE_WIDTH, C2_PARAMKEY_TILE_LAYOUT, "tile.width")
        .limitTo(D::IMAGE));
    add(ConfigMapper(KEY_TILE_HEIGHT, C2_PARAMKEY_TILE_LAYOUT, "tile.height")
        .limitTo(D::IMAGE));

    add(ConfigMapper(KEY_LATENCY, C2_PARAMKEY_PIPELINE_DELAY_REQUEST, "value")
        .limitTo(D::VIDEO & D::ENCODER));

    add(ConfigMapper(C2_PARAMKEY_INPUT_TIME_STRETCH, C2_PARAMKEY_INPUT_TIME_STRETCH, "value"));

    /* still to do
    constexpr char KEY_PUSH_BLANK_BUFFERS_ON_STOP[] = "push-blank-buffers-on-shutdown";

       not yet used by MediaCodec, but defined as MediaFormat
    KEY_AUDIO_SESSION_ID // we use "audio-hw-sync"
    KEY_OUTPUT_REORDER_DEPTH
    */
}

status_t CCodecConfig::initialize(
        const std::shared_ptr<Codec2Client> &client,
        const std::shared_ptr<Codec2Client::Component> &component) {
    C2ComponentDomainSetting domain(C2Component::DOMAIN_OTHER);
    C2ComponentKindSetting kind(C2Component::KIND_OTHER);

    std::vector<std::unique_ptr<C2Param>> queried;
    c2_status_t c2err = component->query({ &domain, &kind }, {}, C2_DONT_BLOCK, &queried);
    if (c2err != C2_OK) {
        ALOGD("Query domain & kind failed => %s", asString(c2err));
        // TEMP: determine kind from component name
        if (kind.value == C2Component::KIND_OTHER) {
            if (component->getName().find("encoder") != std::string::npos) {
                kind.value = C2Component::KIND_ENCODER;
            } else if (component->getName().find("decoder") != std::string::npos) {
                kind.value = C2Component::KIND_DECODER;
            }
        }

        // TEMP: determine domain from media type (port (preferred) or stream #0)
        if (domain.value == C2Component::DOMAIN_OTHER) {
            AString mediaType = QueryMediaType(true /* input */, component);
            if (mediaType.startsWith("audio/")) {
                domain.value = C2Component::DOMAIN_AUDIO;
            } else if (mediaType.startsWith("video/")) {
                domain.value = C2Component::DOMAIN_VIDEO;
            } else if (mediaType.startsWith("image/")) {
                domain.value = C2Component::DOMAIN_IMAGE;
            }
        }
    }

    mDomain = (domain.value == C2Component::DOMAIN_VIDEO ? Domain::IS_VIDEO :
               domain.value == C2Component::DOMAIN_IMAGE ? Domain::IS_IMAGE :
               domain.value == C2Component::DOMAIN_AUDIO ? Domain::IS_AUDIO : Domain::OTHER_DOMAIN)
            | (kind.value == C2Component::KIND_DECODER ? Domain::IS_DECODER :
               kind.value == C2Component::KIND_ENCODER ? Domain::IS_ENCODER : Domain::OTHER_KIND);

    mInputDomain = Domain(((mDomain & IS_DECODER) ? IS_CODED : IS_RAW) | IS_INPUT);
    mOutputDomain = Domain(((mDomain & IS_ENCODER) ? IS_CODED : IS_RAW) | IS_OUTPUT);

    ALOGV("domain is %#x (%u %u)", mDomain, domain.value, kind.value);

    std::vector<C2Param::Index> paramIndices;
    switch (kind.value) {
    case C2Component::KIND_DECODER:
        mCodingMediaType = QueryMediaType(true /* input */, component).c_str();
        break;
    case C2Component::KIND_ENCODER:
        mCodingMediaType = QueryMediaType(false /* input */, component).c_str();
        break;
    default:
        mCodingMediaType = "";
    }

    c2err = component->querySupportedParams(&mParamDescs);
    if (c2err != C2_OK) {
        ALOGD("Query supported params failed after returning %zu values => %s",
                mParamDescs.size(), asString(c2err));
        return UNKNOWN_ERROR;
    }
    for (const std::shared_ptr<C2ParamDescriptor> &desc : mParamDescs) {
        mSupportedIndices.emplace(desc->index());
    }

    mReflector = client->getParamReflector();
    if (mReflector == nullptr) {
        ALOGE("Failed to get param reflector");
        return UNKNOWN_ERROR;
    }

    // enumerate all fields
    mParamUpdater = std::make_shared<ReflectedParamUpdater>();
    mParamUpdater->clear();
    mParamUpdater->supportWholeParam(
            C2_PARAMKEY_TEMPORAL_LAYERING, C2StreamTemporalLayeringTuning::CORE_INDEX);
    mParamUpdater->addParamDesc(mReflector, mParamDescs);

    // TEMP: add some standard fields even if not reflected
    if (kind.value == C2Component::KIND_ENCODER) {
        mParamUpdater->addStandardParam<C2StreamInitDataInfo::output>(C2_PARAMKEY_INIT_DATA);
    }
    if (domain.value == C2Component::DOMAIN_IMAGE || domain.value == C2Component::DOMAIN_VIDEO) {
        if (kind.value != C2Component::KIND_ENCODER) {
            addLocalParam<C2StreamPictureSizeInfo::output>(C2_PARAMKEY_PICTURE_SIZE);
            addLocalParam<C2StreamCropRectInfo::output>(C2_PARAMKEY_CROP_RECT);
            addLocalParam(
                    new C2StreamPixelAspectRatioInfo::output(0u, 1u, 1u),
                    C2_PARAMKEY_PIXEL_ASPECT_RATIO);
            addLocalParam(new C2StreamRotationInfo::output(0u, 0), C2_PARAMKEY_ROTATION);
            addLocalParam(new C2StreamColorAspectsInfo::output(0u), C2_PARAMKEY_COLOR_ASPECTS);
            addLocalParam<C2StreamDataSpaceInfo::output>(C2_PARAMKEY_DATA_SPACE);
            addLocalParam<C2StreamHdrStaticInfo::output>(C2_PARAMKEY_HDR_STATIC_INFO);
            addLocalParam(new C2StreamSurfaceScalingInfo::output(0u, VIDEO_SCALING_MODE_SCALE_TO_FIT),
                          C2_PARAMKEY_SURFACE_SCALING_MODE);
        } else {
            addLocalParam(new C2StreamColorAspectsInfo::input(0u), C2_PARAMKEY_COLOR_ASPECTS);
        }
    }

    initializeStandardParams();

    // subscribe to all supported standard (exposed) params
    // TODO: limit this to params that are actually in the domain
    std::vector<std::string> formatKeys = mStandardParams->getPathsForDomain(Domain(1 << 30));
    std::vector<C2Param::Index> indices;
    mParamUpdater->getParamIndicesForKeys(formatKeys, &indices);
    mSubscribedIndices.insert(indices.begin(), indices.end());

    // also subscribe to some non-SDK standard parameters
    // for number of input/output buffers
    mSubscribedIndices.emplace(C2PortSuggestedBufferCountTuning::input::PARAM_TYPE);
    mSubscribedIndices.emplace(C2PortSuggestedBufferCountTuning::output::PARAM_TYPE);
    mSubscribedIndices.emplace(C2ActualPipelineDelayTuning::PARAM_TYPE);
    mSubscribedIndices.emplace(C2PortActualDelayTuning::input::PARAM_TYPE);
    mSubscribedIndices.emplace(C2PortActualDelayTuning::output::PARAM_TYPE);
    // for output buffer array allocation
    mSubscribedIndices.emplace(C2StreamMaxBufferSizeInfo::output::PARAM_TYPE);
    // init data (CSD)
    mSubscribedIndices.emplace(C2StreamInitDataInfo::output::PARAM_TYPE);

    return OK;
}

status_t CCodecConfig::subscribeToConfigUpdate(
        const std::shared_ptr<Codec2Client::Component> &component,
        const std::vector<C2Param::Index> &indices,
        c2_blocking_t blocking) {
    mSubscribedIndices.insert(indices.begin(), indices.end());
    // TODO: enable this when components no longer crash on this config
    if (mSubscribedIndices.size() != mSubscribedIndicesSize && false) {
        std::vector<uint32_t> indices;
        for (C2Param::Index ix : mSubscribedIndices) {
            indices.push_back(ix);
        }
        std::unique_ptr<C2SubscribedParamIndicesTuning> subscribeTuning =
            C2SubscribedParamIndicesTuning::AllocUnique(indices);
        std::vector<std::unique_ptr<C2SettingResult>> results;
        c2_status_t c2Err = component->config({ subscribeTuning.get() }, blocking, &results);
        if (c2Err != C2_OK && c2Err != C2_BAD_INDEX) {
            ALOGD("Failed to subscribe to parameters => %s", asString(c2Err));
            // TODO: error
        }
        ALOGV("Subscribed to %zu params", mSubscribedIndices.size());
        mSubscribedIndicesSize = mSubscribedIndices.size();
    }
    return OK;
}

status_t CCodecConfig::queryConfiguration(
        const std::shared_ptr<Codec2Client::Component> &component) {
    // query all subscribed parameters
    std::vector<C2Param::Index> indices(mSubscribedIndices.begin(), mSubscribedIndices.end());
    std::vector<std::unique_ptr<C2Param>> queried;
    c2_status_t c2Err = component->query({}, indices, C2_MAY_BLOCK, &queried);
    if (c2Err != OK) {
        ALOGI("query failed after returning %zu values (%s)", queried.size(), asString(c2Err));
        // TODO: error
    }

    updateConfiguration(queried, ALL);
    return OK;
}

bool CCodecConfig::updateConfiguration(
        std::vector<std::unique_ptr<C2Param>> &configUpdate, Domain domain) {
    ALOGV("updating configuration with %zu params", configUpdate.size());
    bool changed = false;
    for (std::unique_ptr<C2Param> &p : configUpdate) {
        if (p && *p) {
            auto insertion = mCurrentConfig.emplace(p->index(), nullptr);
            if (insertion.second || *insertion.first->second != *p) {
                if (mSupportedIndices.count(p->index()) || mLocalParams.count(p->index())) {
                    // only track changes in supported (reflected or local) indices
                    changed = true;
                } else {
                    ALOGV("an unlisted config was %s: %#x",
                            insertion.second ? "added" : "updated", p->index());
                }
            }
            insertion.first->second = std::move(p);
        }
    }

    ALOGV("updated configuration has %zu params (%s)", mCurrentConfig.size(),
            changed ? "CHANGED" : "no change");
    if (changed) {
        return updateFormats(domain);
    }
    return false;
}

bool CCodecConfig::updateFormats(Domain domain) {
    // get addresses of params in the current config
    std::vector<C2Param*> paramPointers;
    for (const auto &it : mCurrentConfig) {
        paramPointers.push_back(it.second.get());
    }

    ReflectedParamUpdater::Dict reflected = mParamUpdater->getParams(paramPointers);
    std::string config = reflected.debugString();
    std::set<std::string> configLines;
    std::string diff;
    for (size_t start = 0; start != std::string::npos; ) {
        size_t end = config.find('\n', start);
        size_t count = (end == std::string::npos)
                ? std::string::npos
                : end - start + 1;
        std::string line = config.substr(start, count);
        configLines.insert(line);
        if (mLastConfig.count(line) == 0) {
            diff.append(line);
        }
        start = (end == std::string::npos) ? std::string::npos : end + 1;
    }
    if (!diff.empty()) {
        ALOGD("c2 config diff is %s", diff.c_str());
    }
    mLastConfig.swap(configLines);

    bool changed = false;
    if (domain & mInputDomain) {
        sp<AMessage> oldFormat = mInputFormat;
        mInputFormat = mInputFormat->dup(); // trigger format changed
        mInputFormat->extend(getSdkFormatForDomain(reflected, mInputDomain));
        if (mInputFormat->countEntries() != oldFormat->countEntries()
                || mInputFormat->changesFrom(oldFormat)->countEntries() > 0) {
            changed = true;
        } else {
            mInputFormat = oldFormat; // no change
        }
    }
    if (domain & mOutputDomain) {
        sp<AMessage> oldFormat = mOutputFormat;
        mOutputFormat = mOutputFormat->dup(); // trigger output format changed
        mOutputFormat->extend(getSdkFormatForDomain(reflected, mOutputDomain));
        if (mOutputFormat->countEntries() != oldFormat->countEntries()
                || mOutputFormat->changesFrom(oldFormat)->countEntries() > 0) {
            changed = true;
        } else {
            mOutputFormat = oldFormat; // no change
        }
    }
    ALOGV_IF(changed, "format(s) changed");
    return changed;
}

sp<AMessage> CCodecConfig::getSdkFormatForDomain(
        const ReflectedParamUpdater::Dict &reflected, Domain portDomain) const {
    sp<AMessage> msg = new AMessage;
    for (const std::pair<std::string, std::vector<ConfigMapper>> &el : mStandardParams->getKeys()) {
        for (const ConfigMapper &cm : el.second) {
            if ((cm.domain() & portDomain) == 0 // input-output-coded-raw
                || (cm.domain() & mDomain) != mDomain // component domain + kind (these must match)
                || (cm.domain() & IS_READ) == 0) {
                continue;
            }
            auto it = reflected.find(cm.path());
            if (it == reflected.end()) {
                continue;
            }
            C2Value c2Value;
            sp<ABuffer> bufValue;
            AString strValue;
            AMessage::ItemData item;
            if (it->second.find(&c2Value)) {
                item = cm.mapToMessage(c2Value);
            } else if (it->second.find(&bufValue)) {
                item.set(bufValue);
            } else if (it->second.find(&strValue)) {
                item.set(strValue);
            } else {
                ALOGD("unexpected untyped query value for key: %s", cm.path().c_str());
                continue;
            }
            msg->setItem(el.first.c_str(), item);
        }
    }

    { // convert from Codec 2.0 rect to MediaFormat rect and add crop rect if not present
        int32_t left, top, width, height;
        if (msg->findInt32("crop-left", &left) && msg->findInt32("crop-width", &width)
                && msg->findInt32("crop-top", &top) && msg->findInt32("crop-height", &height)
                && left >= 0 && width >=0 && width <= INT32_MAX - left
                && top >= 0 && height >=0 && height <= INT32_MAX - top) {
            msg->removeEntryAt(msg->findEntryByName("crop-left"));
            msg->removeEntryAt(msg->findEntryByName("crop-top"));
            msg->removeEntryAt(msg->findEntryByName("crop-width"));
            msg->removeEntryAt(msg->findEntryByName("crop-height"));
            msg->setRect("crop", left, top, left + width - 1, top + height - 1);
        } else if (msg->findInt32("width", &width) && msg->findInt32("height", &height)) {
            msg->setRect("crop", 0, 0, width - 1, height - 1);
        }
    }

    { // convert temporal layering to schema
        sp<ABuffer> tmp;
        if (msg->findBuffer(C2_PARAMKEY_TEMPORAL_LAYERING, &tmp) && tmp != nullptr) {
            C2StreamTemporalLayeringTuning *layering =
                C2StreamTemporalLayeringTuning::From(C2Param::From(tmp->data(), tmp->size()));
            if (layering && layering->m.layerCount > 0
                    && layering->m.bLayerCount < layering->m.layerCount) {
                // check if this is webrtc compatible
                AString mime;
                if (msg->findString(KEY_MIME, &mime) &&
                        mime.equalsIgnoreCase(MIMETYPE_VIDEO_VP8) &&
                        layering->m.bLayerCount == 0 &&
                        (layering->m.layerCount == 1
                                || (layering->m.layerCount == 2
                                        && layering->flexCount() >= 1
                                        && layering->m.bitrateRatios[0] == .6f)
                                || (layering->m.layerCount == 3
                                        && layering->flexCount() >= 2
                                        && layering->m.bitrateRatios[0] == .4f
                                        && layering->m.bitrateRatios[1] == .6f)
                                || (layering->m.layerCount == 4
                                        && layering->flexCount() >= 3
                                        && layering->m.bitrateRatios[0] == .25f
                                        && layering->m.bitrateRatios[1] == .4f
                                        && layering->m.bitrateRatios[2] == .6f))) {
                    msg->setString(KEY_TEMPORAL_LAYERING, AStringPrintf(
                            "webrtc.vp8.%u-layer", layering->m.layerCount));
                } else if (layering->m.bLayerCount) {
                    msg->setString(KEY_TEMPORAL_LAYERING, AStringPrintf(
                            "android.generic.%u+%u",
                            layering->m.layerCount - layering->m.bLayerCount,
                            layering->m.bLayerCount));
                } else if (layering->m.bLayerCount) {
                    msg->setString(KEY_TEMPORAL_LAYERING, AStringPrintf(
                            "android.generic.%u", layering->m.layerCount));
                }
            }
            msg->removeEntryAt(msg->findEntryByName(C2_PARAMKEY_TEMPORAL_LAYERING));
        }
    }

    { // convert color info
        C2Color::primaries_t primaries;
        C2Color::matrix_t matrix;
        if (msg->findInt32("color-primaries", (int32_t*)&primaries)
                && msg->findInt32("color-matrix", (int32_t*)&matrix)) {
            int32_t standard;

            if (C2Mapper::map(primaries, matrix, &standard)) {
                msg->setInt32(KEY_COLOR_STANDARD, standard);
            }

            msg->removeEntryAt(msg->findEntryByName("color-primaries"));
            msg->removeEntryAt(msg->findEntryByName("color-matrix"));
        }


        // calculate dataspace for raw graphic buffers if not specified by component, or if
        // using surface with unspecified aspects (as those must be defaulted which may change
        // the dataspace)
        if ((portDomain & IS_RAW) && (mDomain & (IS_IMAGE | IS_VIDEO))) {
            android_dataspace dataspace;
            ColorAspects aspects = {
                ColorAspects::RangeUnspecified, ColorAspects::PrimariesUnspecified,
                ColorAspects::TransferUnspecified, ColorAspects::MatrixUnspecified
            };
            ColorUtils::getColorAspectsFromFormat(msg, aspects);
            ColorAspects origAspects = aspects;
            if (mUsingSurface) {
                // get image size (default to HD)
                int32_t width = 1280;
                int32_t height = 720;
                int32_t left, top, right, bottom;
                if (msg->findRect("crop", &left, &top, &right, &bottom)) {
                    width = right - left + 1;
                    height = bottom - top + 1;
                } else {
                    (void)msg->findInt32(KEY_WIDTH, &width);
                    (void)msg->findInt32(KEY_HEIGHT, &height);
                }
                ColorUtils::setDefaultCodecColorAspectsIfNeeded(aspects, width, height);
                ColorUtils::setColorAspectsIntoFormat(aspects, msg);
            }

            if (!msg->findInt32("android._dataspace", (int32_t*)&dataspace)
                    || aspects.mRange != origAspects.mRange
                    || aspects.mPrimaries != origAspects.mPrimaries
                    || aspects.mTransfer != origAspects.mTransfer
                    || aspects.mMatrixCoeffs != origAspects.mMatrixCoeffs) {
                dataspace = ColorUtils::getDataSpaceForColorAspects(aspects, true /* mayExpand */);
                msg->setInt32("android._dataspace", dataspace);
            }
        }

        // HDR static info

        C2HdrStaticMetadataStruct hdr;
        if (msg->findFloat("smpte2086.red.x", &hdr.mastering.red.x)
                && msg->findFloat("smpte2086.red.y", &hdr.mastering.red.y)
                && msg->findFloat("smpte2086.green.x", &hdr.mastering.green.x)
                && msg->findFloat("smpte2086.green.y", &hdr.mastering.green.y)
                && msg->findFloat("smpte2086.blue.x", &hdr.mastering.blue.x)
                && msg->findFloat("smpte2086.blue.y", &hdr.mastering.blue.y)
                && msg->findFloat("smpte2086.white.x", &hdr.mastering.white.x)
                && msg->findFloat("smpte2086.white.y", &hdr.mastering.white.y)
                && msg->findFloat("smpte2086.max-luminance", &hdr.mastering.maxLuminance)
                && msg->findFloat("smpte2086.min-luminance", &hdr.mastering.minLuminance)
                && msg->findFloat("cta861.max-cll", &hdr.maxCll)
                && msg->findFloat("cta861.max-fall", &hdr.maxFall)) {
            if (hdr.mastering.red.x >= 0                && hdr.mastering.red.x <= 1
                    && hdr.mastering.red.y >= 0         && hdr.mastering.red.y <= 1
                    && hdr.mastering.green.x >= 0       && hdr.mastering.green.x <= 1
                    && hdr.mastering.green.y >= 0       && hdr.mastering.green.y <= 1
                    && hdr.mastering.blue.x >= 0        && hdr.mastering.blue.x <= 1
                    && hdr.mastering.blue.y >= 0        && hdr.mastering.blue.y <= 1
                    && hdr.mastering.white.x >= 0       && hdr.mastering.white.x <= 1
                    && hdr.mastering.white.y >= 0       && hdr.mastering.white.y <= 1
                    && hdr.mastering.maxLuminance >= 0  && hdr.mastering.maxLuminance <= 65535
                    && hdr.mastering.minLuminance >= 0  && hdr.mastering.minLuminance <= 6.5535
                    && hdr.maxCll >= 0                  && hdr.maxCll <= 65535
                    && hdr.maxFall >= 0                 && hdr.maxFall <= 65535) {
                HDRStaticInfo meta;
                meta.mID = meta.kType1;
                meta.sType1.mR.x = hdr.mastering.red.x / 0.00002 + 0.5;
                meta.sType1.mR.y = hdr.mastering.red.y / 0.00002 + 0.5;
                meta.sType1.mG.x = hdr.mastering.green.x / 0.00002 + 0.5;
                meta.sType1.mG.y = hdr.mastering.green.y / 0.00002 + 0.5;
                meta.sType1.mB.x = hdr.mastering.blue.x / 0.00002 + 0.5;
                meta.sType1.mB.y = hdr.mastering.blue.y / 0.00002 + 0.5;
                meta.sType1.mW.x = hdr.mastering.white.x / 0.00002 + 0.5;
                meta.sType1.mW.y = hdr.mastering.white.y / 0.00002 + 0.5;
                meta.sType1.mMaxDisplayLuminance = hdr.mastering.maxLuminance + 0.5;
                meta.sType1.mMinDisplayLuminance = hdr.mastering.minLuminance / 0.0001 + 0.5;
                meta.sType1.mMaxContentLightLevel = hdr.maxCll + 0.5;
                meta.sType1.mMaxFrameAverageLightLevel = hdr.maxFall + 0.5;
                msg->removeEntryAt(msg->findEntryByName("smpte2086.red.x"));
                msg->removeEntryAt(msg->findEntryByName("smpte2086.red.y"));
                msg->removeEntryAt(msg->findEntryByName("smpte2086.green.x"));
                msg->removeEntryAt(msg->findEntryByName("smpte2086.green.y"));
                msg->removeEntryAt(msg->findEntryByName("smpte2086.blue.x"));
                msg->removeEntryAt(msg->findEntryByName("smpte2086.blue.y"));
                msg->removeEntryAt(msg->findEntryByName("smpte2086.white.x"));
                msg->removeEntryAt(msg->findEntryByName("smpte2086.white.y"));
                msg->removeEntryAt(msg->findEntryByName("smpte2086.max-luminance"));
                msg->removeEntryAt(msg->findEntryByName("smpte2086.min-luminance"));
                msg->removeEntryAt(msg->findEntryByName("cta861.max-cll"));
                msg->removeEntryAt(msg->findEntryByName("cta861.max-fall"));
                msg->setBuffer(KEY_HDR_STATIC_INFO, ABuffer::CreateAsCopy(&meta, sizeof(meta)));
            } else {
                ALOGD("found invalid HDR static metadata %s", msg->debugString(8).c_str());
            }
        }
    }

    ALOGV("converted to SDK values as %s", msg->debugString().c_str());
    return msg;
}

/// converts an AMessage value to a ParamUpdater value
static void convert(const AMessage::ItemData &from, ReflectedParamUpdater::Value *to) {
    int32_t int32Value;
    int64_t int64Value;
    sp<ABuffer> bufValue;
    AString strValue;
    float floatValue;
    double doubleValue;

    if (from.find(&int32Value)) {
        to->set(int32Value);
    } else if (from.find(&int64Value)) {
        to->set(int64Value);
    } else if (from.find(&bufValue)) {
        to->set(bufValue);
    } else if (from.find(&strValue)) {
        to->set(strValue);
    } else if (from.find(&floatValue)) {
        to->set(C2Value(floatValue));
    } else if (from.find(&doubleValue)) {
        // convert double to float
        to->set(C2Value((float)doubleValue));
    }
    // ignore all other AMessage types
}

/// relaxes Codec 2.0 specific value types to SDK types (mainly removes signedness and counterness
/// from 32/64-bit values.)
static void relaxValues(ReflectedParamUpdater::Value &item) {
    C2Value c2Value;
    int32_t int32Value;
    int64_t int64Value;
    (void)item.find(&c2Value);
    if (c2Value.get(&int32Value) || c2Value.get((uint32_t*)&int32Value)
            || c2Value.get((c2_cntr32_t*)&int32Value)) {
        item.set(int32Value);
    } else if (c2Value.get(&int64Value)
            || c2Value.get((uint64_t*)&int64Value)
            || c2Value.get((c2_cntr64_t*)&int64Value)) {
        item.set(int64Value);
    }
}

ReflectedParamUpdater::Dict CCodecConfig::getReflectedFormat(
        const sp<AMessage> &params_, Domain configDomain) const {
    // create a modifiable copy of params
    sp<AMessage> params = params_->dup();
    ALOGV("filtering with config domain %x", configDomain);

    // convert some macro parameters to Codec 2.0 specific expressions

    { // make i-frame-interval frame based
        float iFrameInterval;
        if (params->findAsFloat(KEY_I_FRAME_INTERVAL, &iFrameInterval)) {
            float frameRate;
            if (params->findAsFloat(KEY_FRAME_RATE, &frameRate)) {
                params->setInt32("i-frame-period",
                        (frameRate <= 0 || iFrameInterval < 0)
                                 ? -1 /* no sync frames */
                                 : (int32_t)c2_min(iFrameInterval * frameRate + 0.5,
                                                   (float)INT32_MAX));
            }
        }
    }

    if (mDomain == (IS_VIDEO | IS_ENCODER)) {
        // convert capture-rate into input-time-stretch
        float frameRate, captureRate;
        if (params->findAsFloat(KEY_FRAME_RATE, &frameRate)) {
            if (!params->findAsFloat("time-lapse-fps", &captureRate)
                    && !params->findAsFloat(KEY_CAPTURE_RATE, &captureRate)) {
                captureRate = frameRate;
            }
            if (captureRate > 0 && frameRate > 0) {
                params->setFloat(C2_PARAMKEY_INPUT_TIME_STRETCH, captureRate / frameRate);
            }
        }
    }

    {   // reflect temporal layering into a binary blob
        AString schema;
        if (params->findString(KEY_TEMPORAL_LAYERING, &schema)) {
            unsigned int numLayers = 0;
            unsigned int numBLayers = 0;
            int tags;
            char dummy;
            std::unique_ptr<C2StreamTemporalLayeringTuning::output> layering;
            if (sscanf(schema.c_str(), "webrtc.vp8.%u-layer%c", &numLayers, &dummy) == 1
                && numLayers > 0) {
                switch (numLayers) {
                    case 1:
                        layering = C2StreamTemporalLayeringTuning::output::AllocUnique(
                                {}, 0u, 1u, 0u);
                        break;
                    case 2:
                        layering = C2StreamTemporalLayeringTuning::output::AllocUnique(
                                { .6f }, 0u, 2u, 0u);
                        break;
                    case 3:
                        layering = C2StreamTemporalLayeringTuning::output::AllocUnique(
                                { .4f, .6f }, 0u, 3u, 0u);
                        break;
                    default:
                        layering = C2StreamTemporalLayeringTuning::output::AllocUnique(
                                { .25f, .4f, .6f }, 0u, 4u, 0u);
                        break;
                }
            } else if ((tags = sscanf(schema.c_str(), "android.generic.%u%c%u%c",
                        &numLayers, &dummy, &numBLayers, &dummy))
                && (tags == 1 || (tags == 3 && dummy == '+'))
                && numLayers > 0 && numLayers < UINT32_MAX - numBLayers) {
                layering = C2StreamTemporalLayeringTuning::output::AllocUnique(
                        {}, 0u, numLayers + numBLayers, numBLayers);
            } else {
                ALOGD("Ignoring unsupported ts-schema [%s]", schema.c_str());
            }
            if (layering) {
                params->setBuffer(C2_PARAMKEY_TEMPORAL_LAYERING,
                                  ABuffer::CreateAsCopy(layering.get(), layering->size()));
            }
        }
    }

    { // convert from MediaFormat rect to Codec 2.0 rect
        int32_t offset;
        int32_t end;
        AMessage::ItemData item;
        if (params->findInt32("crop-left", &offset) && params->findInt32("crop-right", &end)
                && offset >= 0 && end >= offset - 1) {
            size_t ix = params->findEntryByName("crop-right");
            params->setEntryNameAt(ix, "crop-width");
            item.set(end - offset + 1);
            params->setEntryAt(ix, item);
        }
        if (params->findInt32("crop-top", &offset) && params->findInt32("crop-bottom", &end)
                && offset >= 0 && end >= offset - 1) {
            size_t ix = params->findEntryByName("crop-bottom");
            params->setEntryNameAt(ix, "crop-height");
            item.set(end - offset + 1);
            params->setEntryAt(ix, item);
        }
    }

    { // convert color info
        int32_t standard;
        if (params->findInt32(KEY_COLOR_STANDARD, &standard)) {
            C2Color::primaries_t primaries;
            C2Color::matrix_t matrix;

            if (C2Mapper::map(standard, &primaries, &matrix)) {
                params->setInt32("color-primaries", primaries);
                params->setInt32("color-matrix", matrix);
            }
        }

        sp<ABuffer> hdrMeta;
        if (params->findBuffer(KEY_HDR_STATIC_INFO, &hdrMeta)
                && hdrMeta->size() == sizeof(HDRStaticInfo)) {
            HDRStaticInfo *meta = (HDRStaticInfo*)hdrMeta->data();
            if (meta->mID == meta->kType1) {
                params->setFloat("smpte2086.red.x", meta->sType1.mR.x * 0.00002);
                params->setFloat("smpte2086.red.y", meta->sType1.mR.y * 0.00002);
                params->setFloat("smpte2086.green.x", meta->sType1.mG.x * 0.00002);
                params->setFloat("smpte2086.green.y", meta->sType1.mG.y * 0.00002);
                params->setFloat("smpte2086.blue.x", meta->sType1.mB.x * 0.00002);
                params->setFloat("smpte2086.blue.y", meta->sType1.mB.y * 0.00002);
                params->setFloat("smpte2086.white.x", meta->sType1.mW.x * 0.00002);
                params->setFloat("smpte2086.white.y", meta->sType1.mW.y * 0.00002);
                params->setFloat("smpte2086.max-luminance", meta->sType1.mMaxDisplayLuminance);
                params->setFloat("smpte2086.min-luminance", meta->sType1.mMinDisplayLuminance * 0.0001);
                params->setFloat("cta861.max-cll", meta->sType1.mMaxContentLightLevel);
                params->setFloat("cta861.max-fall", meta->sType1.mMaxFrameAverageLightLevel);
            }
        }
    }

    // this is to verify that we set proper signedness for standard parameters
    bool beVeryStrict = property_get_bool("debug.stagefright.ccodec_strict_type", false);
    // this is to allow vendors to use the wrong signedness for standard parameters
    bool beVeryLax = property_get_bool("debug.stagefright.ccodec_lax_type", false);

    ReflectedParamUpdater::Dict filtered;
    for (size_t ix = 0; ix < params->countEntries(); ++ix) {
        AMessage::Type type;
        AString name = params->getEntryNameAt(ix, &type);
        AMessage::ItemData msgItem = params->getEntryAt(ix);
        ReflectedParamUpdater::Value item;
        convert(msgItem, &item); // convert item to param updater item

        if (name.startsWith("vendor.")) {
            // vendor params pass through as is
            filtered.emplace(name.c_str(), item);
            continue;
        }
        // standard parameters may get modified, filtered or duplicated
        for (const ConfigMapper &cm : mStandardParams->getConfigMappersForSdkKey(name.c_str())) {
            // note: we ignore port domain for configuration
            if ((cm.domain() & configDomain)
                    // component domain + kind (these must match)
                    && (cm.domain() & mDomain) == mDomain) {
                // map arithmetic values, pass through string or buffer
                switch (type) {
                    case AMessage::kTypeBuffer:
                    case AMessage::kTypeString:
                        break;
                    case AMessage::kTypeInt32:
                    case AMessage::kTypeInt64:
                    case AMessage::kTypeFloat:
                    case AMessage::kTypeDouble:
                        // for now only map settings with mappers as we are not creating
                        // signed <=> unsigned mappers
                        // TODO: be precise about signed unsigned
                        if (beVeryStrict || cm.mapper()) {
                            item.set(cm.mapFromMessage(params->getEntryAt(ix)));
                            // also allow to relax type strictness
                            if (beVeryLax) {
                                relaxValues(item);
                            }
                        }
                        break;
                    default:
                        continue;
                }
                filtered.emplace(cm.path(), item);
            }
        }
    }
    ALOGV("filtered %s to %s", params->debugString(4).c_str(),
            filtered.debugString(4).c_str());
    return filtered;
}

status_t CCodecConfig::getConfigUpdateFromSdkParams(
        std::shared_ptr<Codec2Client::Component> component,
        const sp<AMessage> &sdkParams, Domain configDomain,
        c2_blocking_t blocking,
        std::vector<std::unique_ptr<C2Param>> *configUpdate) const {
    ReflectedParamUpdater::Dict params = getReflectedFormat(sdkParams, configDomain);

    std::vector<C2Param::Index> indices;
    mParamUpdater->getParamIndicesFromMessage(params, &indices);
    if (indices.empty()) {
        ALOGD("no recognized params in: %s", params.debugString().c_str());
        return OK;
    }

    configUpdate->clear();
    std::vector<C2Param::Index> supportedIndices;
    for (C2Param::Index ix : indices) {
        if (mSupportedIndices.count(ix)) {
            supportedIndices.push_back(ix);
        } else if (mLocalParams.count(ix)) {
            // query local parameter here
            auto it = mCurrentConfig.find(ix);
            if (it != mCurrentConfig.end()) {
                configUpdate->emplace_back(C2Param::Copy(*it->second));
            }
        }
    }

    c2_status_t err = component->query({ }, supportedIndices, blocking, configUpdate);
    if (err != C2_OK) {
        ALOGD("query failed after returning %zu params => %s", configUpdate->size(), asString(err));
    }

    if (configUpdate->size()) {
        mParamUpdater->updateParamsFromMessage(params, configUpdate);
    }
    return OK;
}

status_t CCodecConfig::setParameters(
        std::shared_ptr<Codec2Client::Component> component,
        std::vector<std::unique_ptr<C2Param>> &configUpdate,
        c2_blocking_t blocking) {
    status_t result = OK;
    if (configUpdate.empty()) {
        return OK;
    }

    std::vector<C2Param::Index> indices;
    std::vector<C2Param *> paramVector;
    for (const std::unique_ptr<C2Param> &param : configUpdate) {
        if (mSupportedIndices.count(param->index())) {
            // component parameter
            paramVector.push_back(param.get());
            indices.push_back(param->index());
        } else if (mLocalParams.count(param->index())) {
            // handle local parameter here
            LocalParamValidator validator = mLocalParams.find(param->index())->second;
            c2_status_t err = C2_OK;
            std::unique_ptr<C2Param> copy = C2Param::Copy(*param);
            if (validator) {
                err = validator(copy);
            }
            if (err == C2_OK) {
                ALOGV("updated local parameter value for %s",
                        mParamUpdater->getParamName(param->index()).c_str());

                mCurrentConfig[param->index()] = std::move(copy);
            } else {
                ALOGD("failed to set parameter value for %s => %s",
                        mParamUpdater->getParamName(param->index()).c_str(), asString(err));
                result = BAD_VALUE;
            }
        }
    }
    // update subscribed param indices
    subscribeToConfigUpdate(component, indices, blocking);

    std::vector<std::unique_ptr<C2SettingResult>> failures;
    c2_status_t err = component->config(paramVector, blocking, &failures);
    if (err != C2_OK) {
        ALOGD("config failed => %s", asString(err));
        // This is non-fatal.
    }
    for (const std::unique_ptr<C2SettingResult> &failure : failures) {
        switch (failure->failure) {
            case C2SettingResult::BAD_VALUE:
                ALOGD("Bad parameter value");
                result = BAD_VALUE;
                break;
            default:
                ALOGV("failure = %d", int(failure->failure));
                break;
        }
    }

    // Re-query parameter values in case config could not update them and update the current
    // configuration.
    configUpdate.clear();
    err = component->query({}, indices, blocking, &configUpdate);
    if (err != C2_OK) {
        ALOGD("query failed after returning %zu params => %s", configUpdate.size(), asString(err));
    }
    (void)updateConfiguration(configUpdate, ALL);

    // TODO: error value
    return result;
}

const C2Param *CCodecConfig::getConfigParameterValue(C2Param::Index index) const {
    auto it = mCurrentConfig.find(index);
    if (it == mCurrentConfig.end()) {
        return nullptr;
    } else {
        return it->second.get();
    }
}

}  // namespace android
