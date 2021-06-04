/*
 * Copyright 2015 The Android Open Source Project
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
#define LOG_TAG "MediaCodecListOverrides"
#include <utils/Log.h>

#include "MediaCodecListOverrides.h"

#include <cutils/properties.h>
#include <gui/Surface.h>
#include <media/ICrypto.h>
#include <media/IMediaCodecList.h>
#include <media/MediaCodecInfo.h>
#include <media/MediaResourcePolicy.h>
#include <media/openmax/OMX_IVCommon.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/MediaCodecList.h>

namespace android {

AString getProfilingVersionString() {
    char val[PROPERTY_VALUE_MAX];
    if (property_get("ro.build.display.id", val, NULL) && (strlen(val) > 0)) {
        return AStringPrintf("<!-- Profiled-with: %s -->", val);
    }

    return "<!-- Profiled-with: UNKNOWN_BUILD_ID -->";
}

// a limit to avoid allocating unreasonable number of codec instances in the measurement.
// this should be in sync with the MAX_SUPPORTED_INSTANCES defined in MediaCodecInfo.java.
static const int kMaxInstances = 32;

// TODO: move MediaCodecInfo to C++. Until then, some temp methods to parse out info.
static bool getMeasureSize(const sp<MediaCodecInfo::Capabilities> &caps, int32_t *width, int32_t *height) {
    AString sizeRange;
    if (!caps->getDetails()->findString("size-range", &sizeRange)) {
        return false;
    }
    AString minSize;
    AString maxSize;
    if (!splitString(sizeRange, "-", &minSize, &maxSize)) {
        return false;
    }
    AString sWidth;
    AString sHeight;
    if (!splitString(minSize, "x", &sWidth, &sHeight)) {
        if (!splitString(minSize, "*", &sWidth, &sHeight)) {
            return false;
        }
    }

    *width = strtol(sWidth.c_str(), NULL, 10);
    *height = strtol(sHeight.c_str(), NULL, 10);
    return (*width > 0) && (*height > 0);
}

static void getMeasureBitrate(const sp<MediaCodecInfo::Capabilities> &caps, int32_t *bitrate) {
    // Until have native MediaCodecInfo, we cannot get bitrates based on profile/levels.
    // We use 200000 as default value for our measurement.
    *bitrate = 200000;
    AString bitrateRange;
    if (!caps->getDetails()->findString("bitrate-range", &bitrateRange)) {
        return;
    }
    AString minBitrate;
    AString maxBitrate;
    if (!splitString(bitrateRange, "-", &minBitrate, &maxBitrate)) {
        return;
    }

    *bitrate = strtol(minBitrate.c_str(), NULL, 10);
}

static sp<AMessage> getMeasureFormat(
        bool isEncoder, const AString &mime, const sp<MediaCodecInfo::Capabilities> &caps) {
    sp<AMessage> format = new AMessage();
    format->setString("mime", mime);

    if (isEncoder) {
        int32_t bitrate = 0;
        getMeasureBitrate(caps, &bitrate);
        format->setInt32("bitrate", bitrate);
        format->setInt32("encoder", 1);
    }

    if (mime.startsWith("video/")) {
        int32_t width = 0;
        int32_t height = 0;
        if (!getMeasureSize(caps, &width, &height)) {
            return NULL;
        }
        format->setInt32("width", width);
        format->setInt32("height", height);

        Vector<uint32_t> colorFormats;
        caps->getSupportedColorFormats(&colorFormats);
        if (colorFormats.size() == 0) {
            return NULL;
        }
        format->setInt32("color-format", colorFormats[0]);

        format->setFloat("frame-rate", 10.0);
        format->setInt32("i-frame-interval", 10);
    } else {
        // TODO: profile hw audio
        return NULL;
    }

    return format;
}

static size_t doProfileCodecs(
        bool isEncoder, const AString &name, const AString &mime, const sp<MediaCodecInfo::Capabilities> &caps) {
    sp<AMessage> format = getMeasureFormat(isEncoder, mime, caps);
    if (format == NULL) {
        return 0;
    }
    ALOGV("doProfileCodecs %s %s %s %s",
            name.c_str(), mime.c_str(), isEncoder ? "encoder" : "decoder",
            format->debugString().c_str());

    status_t err = OK;
    Vector<sp<MediaCodec>> codecs;
    while (err == OK && codecs.size() < kMaxInstances) {
        sp<ALooper> looper = new ALooper;
        looper->setName("MediaCodec_looper");
        ALOGV("doProfileCodecs for codec #%zu", codecs.size());
        ALOGV("doProfileCodecs start looper");
        looper->start(
                false /* runOnCallingThread */, false /* canCallJava */, ANDROID_PRIORITY_AUDIO);
        ALOGV("doProfileCodecs CreateByComponentName");
        sp<MediaCodec> codec = MediaCodec::CreateByComponentName(looper, name.c_str(), &err);
        if (err != OK) {
            ALOGV("Failed to create codec: %s", name.c_str());
            break;
        }
        const sp<Surface> nativeWindow;
        const sp<ICrypto> crypto;
        uint32_t flags = isEncoder ? MediaCodec::CONFIGURE_FLAG_ENCODE : 0;
        ALOGV("doProfileCodecs configure");
        err = codec->configure(format, nativeWindow, crypto, flags);
        if (err != OK) {
            ALOGV("Failed to configure codec: %s with mime: %s", name.c_str(), mime.c_str());
            codec->release();
            break;
        }
        ALOGV("doProfileCodecs start");
        err = codec->start();
        if (err != OK) {
            ALOGV("Failed to start codec: %s with mime: %s", name.c_str(), mime.c_str());
            codec->release();
            break;
        }
        codecs.push_back(codec);
    }

    for (size_t i = 0; i < codecs.size(); ++i) {
        ALOGV("doProfileCodecs release %s", name.c_str());
        err = codecs[i]->release();
        if (err != OK) {
            ALOGE("Failed to release codec: %s with mime: %s", name.c_str(), mime.c_str());
        }
    }

    return codecs.size();
}

bool splitString(const AString &s, const AString &delimiter, AString *s1, AString *s2) {
    ssize_t pos = s.find(delimiter.c_str());
    if (pos < 0) {
        return false;
    }
    *s1 = AString(s, 0, pos);
    *s2 = AString(s, pos + 1, s.size() - pos - 1);
    return true;
}

bool splitString(
        const AString &s, const AString &delimiter, AString *s1, AString *s2, AString *s3) {
    AString temp;
    if (!splitString(s, delimiter, s1, &temp)) {
        return false;
    }
    if (!splitString(temp, delimiter, s2, s3)) {
        return false;
    }
    return true;
}

void profileCodecs(const std::vector<sp<MediaCodecInfo>> &infos,
        const char* profilingResults) {
    CodecSettings global_results;
    KeyedVector<AString, CodecSettings> encoder_results;
    KeyedVector<AString, CodecSettings> decoder_results;
    profileCodecs(infos, &global_results, &encoder_results, &decoder_results);
    exportResultsToXML(profilingResults, global_results, encoder_results, decoder_results);
}

void profileCodecs(
        const std::vector<sp<MediaCodecInfo>> &infos,
        CodecSettings *global_results,
        KeyedVector<AString, CodecSettings> *encoder_results,
        KeyedVector<AString, CodecSettings> *decoder_results,
        bool forceToMeasure) {
    KeyedVector<AString, sp<MediaCodecInfo::Capabilities>> codecsNeedMeasure;
    AString supportMultipleSecureCodecs = "true";
    for (const auto& info : infos) {
        AString name = info->getCodecName();
        if (name.startsWith("OMX.google.") || name.startsWith("c2.android.") ||
                // TODO: reenable below codecs once fixed
                name == "OMX.Intel.VideoDecoder.VP9.hybrid") {
            continue;
        }

        Vector<AString> mediaTypes;
        info->getSupportedMediaTypes(&mediaTypes);
        for (size_t i = 0; i < mediaTypes.size(); ++i) {
            const sp<MediaCodecInfo::Capabilities> &caps =
                    info->getCapabilitiesFor(mediaTypes[i].c_str());
            if (!forceToMeasure &&
                (caps->getDetails()->contains("max-supported-instances") ||
                 caps->getDetails()->contains("max-concurrent-instances"))) {
                continue;
            }

            size_t max = doProfileCodecs(info->isEncoder(), name, mediaTypes[i], caps);
            if (max > 0) {
                CodecSettings settings;
                char maxStr[32];
                sprintf(maxStr, "%zu", max);
                settings.add("max-supported-instances", maxStr);

                AString key = name;
                key.append(" ");
                key.append(mediaTypes[i]);

                if (info->isEncoder()) {
                    encoder_results->add(key, settings);
                } else {
                    decoder_results->add(key, settings);
                }

                if (name.endsWith(".secure")) {
                    if (max <= 1) {
                        supportMultipleSecureCodecs = "false";
                    }
                }
            }
        }
    }
    global_results->add(kPolicySupportsMultipleSecureCodecs, supportMultipleSecureCodecs);
}

static AString globalResultsToXml(const CodecSettings &results) {
    AString ret;
    for (size_t i = 0; i < results.size(); ++i) {
        AString setting = AStringPrintf(
                "        <Setting name=\"%s\" value=\"%s\" />\n",
                results.keyAt(i).c_str(),
                results.valueAt(i).c_str());
        ret.append(setting);
    }
    return ret;
}

static AString codecResultsToXml(const KeyedVector<AString, CodecSettings> &results) {
    AString ret;
    for (size_t i = 0; i < results.size(); ++i) {
        AString name;
        AString mime;
        if (!splitString(results.keyAt(i), " ", &name, &mime)) {
            continue;
        }
        AString codec =
                AStringPrintf("        <MediaCodec name=\"%s\" type=\"%s\" update=\"true\" >\n",
                              name.c_str(),
                              mime.c_str());
        ret.append(codec);
        const CodecSettings &settings = results.valueAt(i);
        for (size_t i = 0; i < settings.size(); ++i) {
            // WARNING: we assume all the settings are "Limit". Currently we have only one type
            // of setting in this case, which is "max-supported-instances".
            AString setting = AStringPrintf(
                    "            <Limit name=\"%s\" value=\"%s\" />\n",
                    settings.keyAt(i).c_str(),
                    settings.valueAt(i).c_str());
            ret.append(setting);
        }
        ret.append("        </MediaCodec>\n");
    }
    return ret;
}

void exportResultsToXML(
        const char *fileName,
        const CodecSettings &global_results,
        const KeyedVector<AString, CodecSettings> &encoder_results,
        const KeyedVector<AString, CodecSettings> &decoder_results) {
    if (global_results.size() == 0 && encoder_results.size() == 0 && decoder_results.size() == 0) {
        return;
    }

    AString overrides;
    overrides.append(getProfilingVersionString());
    overrides.append("\n");
    overrides.append("<MediaCodecs>\n");
    if (global_results.size() > 0) {
        overrides.append("    <Settings>\n");
        overrides.append(globalResultsToXml(global_results));
        overrides.append("    </Settings>\n");
    }
    if (encoder_results.size() > 0) {
        overrides.append("    <Encoders>\n");
        overrides.append(codecResultsToXml(encoder_results));
        overrides.append("    </Encoders>\n");
    }
    if (decoder_results.size() > 0) {
        overrides.append("    <Decoders>\n");
        overrides.append(codecResultsToXml(decoder_results));
        overrides.append("    </Decoders>\n");
    }
    overrides.append("</MediaCodecs>\n");

    FILE *f = fopen(fileName, "wb");
    if (f == NULL) {
        ALOGE("Failed to open %s for writing.", fileName);
        return;
    }
    if (fwrite(overrides.c_str(), 1, overrides.size(), f) != overrides.size()) {
        ALOGE("Failed to write to %s.", fileName);
    }
    fclose(f);
}

}  // namespace android
