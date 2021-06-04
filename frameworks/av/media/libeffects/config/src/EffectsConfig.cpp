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

#define LOG_TAG "EffectsConfig"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <unistd.h>

#include <tinyxml2.h>
#include <log/log.h>

#include <media/EffectsConfig.h>

using namespace tinyxml2;

namespace android {
namespace effectsConfig {

/** All functions except `parse(const char*)` are static. */
namespace {

/** @return all `node`s children that are elements and match the tag if provided. */
std::vector<std::reference_wrapper<const XMLElement>> getChildren(const XMLNode& node,
                                                                  const char* childTag = nullptr) {
    std::vector<std::reference_wrapper<const XMLElement>> children;
    for (auto* child = node.FirstChildElement(childTag); child != nullptr;
            child = child->NextSiblingElement(childTag)) {
        children.emplace_back(*child);
    }
    return children;
}

/** @return xml dump of the provided element.
 * By not providing a printer, it is implicitly created in the caller context.
 * In such case the return pointer has the same lifetime as the expression containing dump().
 */
const char* dump(const XMLElement& element, XMLPrinter&& printer = {}) {
    element.Accept(&printer);
    return printer.CStr();
}


bool stringToUuid(const char *str, effect_uuid_t *uuid)
{
    uint32_t tmp[10];

    if (sscanf(str, "%08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x",
            tmp, tmp+1, tmp+2, tmp+3, tmp+4, tmp+5, tmp+6, tmp+7, tmp+8, tmp+9) < 10) {
        return false;
    }
    uuid->timeLow = (uint32_t)tmp[0];
    uuid->timeMid = (uint16_t)tmp[1];
    uuid->timeHiAndVersion = (uint16_t)tmp[2];
    uuid->clockSeq = (uint16_t)tmp[3];
    uuid->node[0] = (uint8_t)tmp[4];
    uuid->node[1] = (uint8_t)tmp[5];
    uuid->node[2] = (uint8_t)tmp[6];
    uuid->node[3] = (uint8_t)tmp[7];
    uuid->node[4] = (uint8_t)tmp[8];
    uuid->node[5] = (uint8_t)tmp[9];

    return true;
}

/** Map the enum and string representation of a string type.
 *  Intended to be specialized for each enum to deserialize.
 *  The general template is disabled.
 */
template <class Enum>
constexpr std::enable_if<false, Enum> STREAM_NAME_MAP;

/** All output stream types which support effects.
 * This need to be kept in sync with the xsd streamOutputType.
 */
template <>
constexpr std::pair<audio_stream_type_t, const char*> STREAM_NAME_MAP<audio_stream_type_t>[] = {
        {AUDIO_STREAM_VOICE_CALL, "voice_call"},
        {AUDIO_STREAM_SYSTEM, "system"},
        {AUDIO_STREAM_RING, "ring"},
        {AUDIO_STREAM_MUSIC, "music"},
        {AUDIO_STREAM_ALARM, "alarm"},
        {AUDIO_STREAM_NOTIFICATION, "notification"},
        {AUDIO_STREAM_BLUETOOTH_SCO, "bluetooth_sco"},
        {AUDIO_STREAM_ENFORCED_AUDIBLE, "enforced_audible"},
        {AUDIO_STREAM_DTMF, "dtmf"},
        {AUDIO_STREAM_TTS, "tts"},
};

/** All input stream types which support effects.
 * This need to be kept in sync with the xsd streamOutputType.
 */
template <>
constexpr std::pair<audio_source_t, const char*> STREAM_NAME_MAP<audio_source_t>[] = {
        {AUDIO_SOURCE_MIC, "mic"},
        {AUDIO_SOURCE_VOICE_UPLINK, "voice_uplink"},
        {AUDIO_SOURCE_VOICE_DOWNLINK, "voice_downlink"},
        {AUDIO_SOURCE_VOICE_CALL, "voice_call"},
        {AUDIO_SOURCE_CAMCORDER, "camcorder"},
        {AUDIO_SOURCE_VOICE_RECOGNITION, "voice_recognition"},
        {AUDIO_SOURCE_VOICE_COMMUNICATION, "voice_communication"},
        {AUDIO_SOURCE_UNPROCESSED, "unprocessed"},
        {AUDIO_SOURCE_VOICE_PERFORMANCE, "voice_performance"},
};

/** Find the stream type enum corresponding to the stream type name or return false */
template <class Type>
bool stringToStreamType(const char *streamName, Type* type)
{
    for (auto& streamNamePair : STREAM_NAME_MAP<Type>) {
        if (strcmp(streamNamePair.second, streamName) == 0) {
            *type = streamNamePair.first;
            return true;
        }
    }
    return false;
}

/** Parse a library xml note and push the result in libraries or return false on failure. */
bool parseLibrary(const XMLElement& xmlLibrary, Libraries* libraries) {
    const char* name = xmlLibrary.Attribute("name");
    const char* path = xmlLibrary.Attribute("path");
    if (name == nullptr || path == nullptr) {
        ALOGE("library must have a name and a path: %s", dump(xmlLibrary));
        return false;
    }
    libraries->push_back({name, path});
    return true;
}

/** Find an element in a collection by its name.
 * @return nullptr if not found, the element address if found.
 */
template <class T>
T* findByName(const char* name, std::vector<T>& collection) {
    auto it = find_if(begin(collection), end(collection),
                         [name] (auto& item) { return item.name == name; });
    return it != end(collection) ? &*it : nullptr;
}

/** Parse an effect from an xml element describing it.
 * @return true and pushes the effect in effects on success,
 *         false on failure. */
bool parseEffect(const XMLElement& xmlEffect, Libraries& libraries, Effects* effects) {
    Effect effect{};

    const char* name = xmlEffect.Attribute("name");
    if (name == nullptr) {
        ALOGE("%s must have a name: %s", xmlEffect.Value(), dump(xmlEffect));
        return false;
    }
    effect.name = name;

    // Function to parse effect.library and effect.uuid from xml
    auto parseImpl = [&libraries](const XMLElement& xmlImpl, EffectImpl& effect) {
        // Retrieve library name and uuid from xml
        const char* libraryName = xmlImpl.Attribute("library");
        const char* uuid = xmlImpl.Attribute("uuid");
        if (libraryName == nullptr || uuid == nullptr) {
            ALOGE("effect must have a library name and a uuid: %s", dump(xmlImpl));
            return false;
        }

        // Convert library name to a pointer to the previously loaded library
        auto* library = findByName(libraryName, libraries);
        if (library == nullptr) {
            ALOGE("Could not find library referenced in: %s", dump(xmlImpl));
            return false;
        }
        effect.library = library;

        if (!stringToUuid(uuid, &effect.uuid)) {
            ALOGE("Invalid uuid in: %s", dump(xmlImpl));
            return false;
        }
        return true;
    };

    if (!parseImpl(xmlEffect, effect)) {
        return false;
    }

    // Handle proxy effects
    effect.isProxy = false;
    if (std::strcmp(xmlEffect.Name(), "effectProxy") == 0) {
        effect.isProxy = true;

        // Function to parse libhw and libsw
        auto parseProxy = [&xmlEffect, &parseImpl](const char* tag, EffectImpl& proxyLib) {
            auto* xmlProxyLib = xmlEffect.FirstChildElement(tag);
            if (xmlProxyLib == nullptr) {
                ALOGE("effectProxy must contain a <%s>: %s", tag, dump(xmlEffect));
                return false;
            }
            return parseImpl(*xmlProxyLib, proxyLib);
        };
        if (!parseProxy("libhw", effect.libHw) || !parseProxy("libsw", effect.libSw)) {
            return false;
        }
    }

    effects->push_back(std::move(effect));
    return true;
}

/** Parse an stream from an xml element describing it.
 * @return true and pushes the stream in streams on success,
 *         false on failure. */
template <class Stream>
bool parseStream(const XMLElement& xmlStream, Effects& effects, std::vector<Stream>* streams) {
    const char* streamType = xmlStream.Attribute("type");
    if (streamType == nullptr) {
        ALOGE("stream must have a type: %s", dump(xmlStream));
        return false;
    }
    Stream stream;
    if (!stringToStreamType(streamType, &stream.type)) {
        ALOGE("Invalid stream type %s: %s", streamType, dump(xmlStream));
        return false;
    }

    for (auto& xmlApply : getChildren(xmlStream, "apply")) {
        const char* effectName = xmlApply.get().Attribute("effect");
        if (effectName == nullptr) {
            ALOGE("stream/apply must have reference an effect: %s", dump(xmlApply));
            return false;
        }
        auto* effect = findByName(effectName, effects);
        if (effect == nullptr) {
            ALOGE("Could not find effect referenced in: %s", dump(xmlApply));
            return false;
        }
        stream.effects.emplace_back(*effect);
    }
    streams->push_back(std::move(stream));
    return true;
}

/** Internal version of the public parse(const char* path) where path always exist. */
ParsingResult parseWithPath(std::string&& path) {
    XMLDocument doc;
    doc.LoadFile(path.c_str());
    if (doc.Error()) {
        ALOGE("Failed to parse %s: Tinyxml2 error (%d): %s", path.c_str(),
              doc.ErrorID(), doc.ErrorStr());
        return {nullptr, 0, std::move(path)};
    }

    auto config = std::make_unique<Config>();
    size_t nbSkippedElements = 0;
    auto registerFailure = [&nbSkippedElements](bool result) {
        nbSkippedElements += result ? 0 : 1;
    };
    for (auto& xmlConfig : getChildren(doc, "audio_effects_conf")) {

        // Parse library
        for (auto& xmlLibraries : getChildren(xmlConfig, "libraries")) {
            for (auto& xmlLibrary : getChildren(xmlLibraries, "library")) {
                registerFailure(parseLibrary(xmlLibrary, &config->libraries));
            }
        }

        // Parse effects
        for (auto& xmlEffects : getChildren(xmlConfig, "effects")) {
            for (auto& xmlEffect : getChildren(xmlEffects)) {
                registerFailure(parseEffect(xmlEffect, config->libraries, &config->effects));
            }
        }

        // Parse pre processing chains
        for (auto& xmlPreprocess : getChildren(xmlConfig, "preprocess")) {
            for (auto& xmlStream : getChildren(xmlPreprocess, "stream")) {
                registerFailure(parseStream(xmlStream, config->effects, &config->preprocess));
            }
        }

        // Parse post processing chains
        for (auto& xmlPostprocess : getChildren(xmlConfig, "postprocess")) {
            for (auto& xmlStream : getChildren(xmlPostprocess, "stream")) {
                registerFailure(parseStream(xmlStream, config->effects, &config->postprocess));
            }
        }
    }
    return {std::move(config), nbSkippedElements, std::move(path)};
}

}; // namespace

ParsingResult parse(const char* path) {
    if (path != nullptr) {
        return parseWithPath(path);
    }

    for (const std::string& location : DEFAULT_LOCATIONS) {
        std::string defaultPath = location + '/' + DEFAULT_NAME;
        if (access(defaultPath.c_str(), R_OK) != 0) {
            continue;
        }
        auto result = parseWithPath(std::move(defaultPath));
        if (result.parsedConfig != nullptr) {
            return result;
        }
    }

    ALOGE("Could not parse effect configuration in any of the default locations.");
    return {nullptr, 0, ""};
}

} // namespace effectsConfig
} // namespace android
