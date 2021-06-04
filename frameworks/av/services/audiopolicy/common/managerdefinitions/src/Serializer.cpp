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

#define LOG_TAG "APM::Serializer"
//#define LOG_NDEBUG 0

#include <memory>
#include <string>
#include <utility>

#include <hidl/Status.h>
#include <libxml/parser.h>
#include <libxml/xinclude.h>
#include <media/convert.h>
#include <utils/Log.h>
#include <utils/StrongPointer.h>
#include <utils/Errors.h>
#include <utils/RefBase.h>
#include "Serializer.h"
#include "TypeConverter.h"

namespace android {

namespace {

// TODO(mnaganov): Consider finding an alternative for using HIDL code.
using hardware::Return;
using hardware::Status;
using utilities::convertTo;

template<typename E, typename C>
struct AndroidCollectionTraits {
    typedef sp<E> Element;
    typedef C Collection;
    typedef void* PtrSerializingCtx;

    static status_t addElementToCollection(const Element &element, Collection *collection) {
        return collection->add(element) >= 0 ? NO_ERROR : BAD_VALUE;
    }
};

template<typename C>
struct StdCollectionTraits {
    typedef C Collection;
    typedef typename C::value_type Element;
    typedef void* PtrSerializingCtx;

    static status_t addElementToCollection(const Element &element, Collection *collection) {
        auto pair = collection->insert(element);
        return pair.second ? NO_ERROR : BAD_VALUE;
    }
};

struct AudioGainTraits : public AndroidCollectionTraits<AudioGain, AudioGains>
{
    static constexpr const char *tag = "gain";
    static constexpr const char *collectionTag = "gains";

    struct Attributes
    {
        /** gain modes supported, e.g. AUDIO_GAIN_MODE_CHANNELS. */
        static constexpr const char *mode = "mode";
        /** controlled channels, needed if mode AUDIO_GAIN_MODE_CHANNELS. */
        static constexpr const char *channelMask = "channel_mask";
        static constexpr const char *minValueMB = "minValueMB"; /**< min value in millibel. */
        static constexpr const char *maxValueMB = "maxValueMB"; /**< max value in millibel. */
        /** default value in millibel. */
        static constexpr const char *defaultValueMB = "defaultValueMB";
        static constexpr const char *stepValueMB = "stepValueMB"; /**< step value in millibel. */
        /** needed if mode AUDIO_GAIN_MODE_RAMP. */
        static constexpr const char *minRampMs = "minRampMs";
        /** needed if mode AUDIO_GAIN_MODE_RAMP. */
        static constexpr const char *maxRampMs = "maxRampMs";
        /** needed to allow use setPortGain instead of setStreamVolume. */
        static constexpr const char *useForVolume = "useForVolume";

    };

    static Return<Element> deserialize(const xmlNode *cur, PtrSerializingCtx serializingContext);
    // No children
};

// A profile section contains a name,  one audio format and the list of supported sampling rates
// and channel masks for this format
struct AudioProfileTraits : public AndroidCollectionTraits<AudioProfile, AudioProfileVector>
{
    static constexpr const char *tag = "profile";
    static constexpr const char *collectionTag = "profiles";

    struct Attributes
    {
        static constexpr const char *samplingRates = "samplingRates";
        static constexpr const char *format = "format";
        static constexpr const char *channelMasks = "channelMasks";
    };

    static Return<Element> deserialize(const xmlNode *cur, PtrSerializingCtx serializingContext);
};

struct MixPortTraits : public AndroidCollectionTraits<IOProfile, IOProfileCollection>
{
    static constexpr const char *tag = "mixPort";
    static constexpr const char *collectionTag = "mixPorts";

    struct Attributes
    {
        static constexpr const char *name = "name";
        static constexpr const char *role = "role";
        static constexpr const char *roleSource = "source"; /**< <attribute role source value>. */
        static constexpr const char *flags = "flags";
        static constexpr const char *maxOpenCount = "maxOpenCount";
        static constexpr const char *maxActiveCount = "maxActiveCount";
    };

    static Return<Element> deserialize(const xmlNode *cur, PtrSerializingCtx serializingContext);
    // Children: GainTraits
};

struct DevicePortTraits : public AndroidCollectionTraits<DeviceDescriptor, DeviceVector>
{
    static constexpr const char *tag = "devicePort";
    static constexpr const char *collectionTag = "devicePorts";

    struct Attributes
    {
        /**  <device tag name>: any string without space. */
        static constexpr const char *tagName = "tagName";
        static constexpr const char *type = "type"; /**< <device type>. */
        static constexpr const char *role = "role"; /**< <device role: sink or source>. */
        static constexpr const char *roleSource = "source"; /**< <attribute role source value>. */
        /** optional: device address, char string less than 64. */
        static constexpr const char *address = "address";
        /** optional: the list of encoded audio formats that are known to be supported. */
        static constexpr const char *encodedFormats = "encodedFormats";
    };

    static Return<Element> deserialize(const xmlNode *cur, PtrSerializingCtx serializingContext);
    // Children: GainTraits (optional)
};

struct RouteTraits : public AndroidCollectionTraits<AudioRoute, AudioRouteVector>
{
    static constexpr const char *tag = "route";
    static constexpr const char *collectionTag = "routes";

    struct Attributes
    {
        static constexpr const char *type = "type"; /**< <route type>: mix or mux. */
        static constexpr const char *typeMix = "mix"; /**< type attribute mix value. */
        static constexpr const char *sink = "sink"; /**< <sink: involved in this route>. */
        /** sources: all source that can be involved in this route. */
        static constexpr const char *sources = "sources";
    };

    typedef HwModule *PtrSerializingCtx;

    static Return<Element> deserialize(const xmlNode *cur, PtrSerializingCtx serializingContext);
};

struct ModuleTraits : public AndroidCollectionTraits<HwModule, HwModuleCollection>
{
    static constexpr const char *tag = "module";
    static constexpr const char *collectionTag = "modules";

    static constexpr const char *childAttachedDevicesTag = "attachedDevices";
    static constexpr const char *childAttachedDeviceTag = "item";
    static constexpr const char *childDefaultOutputDeviceTag = "defaultOutputDevice";

    struct Attributes
    {
        static constexpr const char *name = "name";
        static constexpr const char *version = "halVersion";
    };

    typedef AudioPolicyConfig *PtrSerializingCtx;

    static Return<Element> deserialize(const xmlNode *cur, PtrSerializingCtx serializingContext);
    // Children: mixPortTraits, devicePortTraits, and routeTraits
    // Need to call deserialize on each child
};

struct GlobalConfigTraits
{
    static constexpr const char *tag = "globalConfiguration";

    struct Attributes
    {
        static constexpr const char *speakerDrcEnabled = "speaker_drc_enabled";
    };

    static status_t deserialize(const xmlNode *root, AudioPolicyConfig *config);
};

struct SurroundSoundTraits
{
    static constexpr const char *tag = "surroundSound";

    static status_t deserialize(const xmlNode *root, AudioPolicyConfig *config);
    // Children: SurroundSoundFormatTraits
};

struct SurroundSoundFormatTraits : public StdCollectionTraits<AudioPolicyConfig::SurroundFormats>
{
    static constexpr const char *tag = "format";
    static constexpr const char *collectionTag = "formats";

    struct Attributes
    {
        static constexpr const char *name = "name";
        static constexpr const char *subformats = "subformats";
    };

    static Return<Element> deserialize(const xmlNode *cur, PtrSerializingCtx serializingContext);
};

class PolicySerializer
{
public:
    PolicySerializer() : mVersion{std::to_string(gMajor) + "." + std::to_string(gMinor)}
    {
        ALOGV("%s: Version=%s Root=%s", __func__, mVersion.c_str(), rootName);
    }
    status_t deserialize(const char *configFile, AudioPolicyConfig *config);

private:
    static constexpr const char *rootName = "audioPolicyConfiguration";
    static constexpr const char *versionAttribute = "version";
    static constexpr uint32_t gMajor = 1; /**< the major number of the policy xml format version. */
    static constexpr uint32_t gMinor = 0; /**< the minor number of the policy xml format version. */

    typedef AudioPolicyConfig Element;

    const std::string mVersion;

    // Children: ModulesTraits, VolumeTraits, SurroundSoundTraits (optional)
};

template <class T>
constexpr void (*xmlDeleter)(T* t);
template <>
constexpr auto xmlDeleter<xmlDoc> = xmlFreeDoc;
// http://b/111067277 - Add back constexpr when we switch to C++17.
template <>
auto xmlDeleter<xmlChar> = [](xmlChar *s) { xmlFree(s); };

/** @return a unique_ptr with the correct deleter for the libxml2 object. */
template <class T>
constexpr auto make_xmlUnique(T *t) {
    // Wrap deleter in lambda to enable empty base optimization
    auto deleter = [](T *t) { xmlDeleter<T>(t); };
    return std::unique_ptr<T, decltype(deleter)>{t, deleter};
}

std::string getXmlAttribute(const xmlNode *cur, const char *attribute)
{
    auto xmlValue = make_xmlUnique(xmlGetProp(cur, reinterpret_cast<const xmlChar*>(attribute)));
    if (xmlValue == nullptr) {
        return "";
    }
    std::string value(reinterpret_cast<const char*>(xmlValue.get()));
    return value;
}

template <class Trait>
const xmlNode* getReference(const xmlNode *cur, const std::string &refName)
{
    for (; cur != NULL; cur = cur->next) {
        if (!xmlStrcmp(cur->name, reinterpret_cast<const xmlChar*>(Trait::collectionTag))) {
            for (const xmlNode *child = cur->children; child != NULL; child = child->next) {
                if ((!xmlStrcmp(child->name,
                                        reinterpret_cast<const xmlChar*>(Trait::referenceTag)))) {
                    std::string name = getXmlAttribute(child, Trait::Attributes::referenceName);
                    if (refName == name) {
                        return child;
                    }
                }
            }
        }
    }
    return NULL;
}

template <class Trait>
status_t deserializeCollection(const xmlNode *cur,
        typename Trait::Collection *collection,
        typename Trait::PtrSerializingCtx serializingContext)
{
    for (cur = cur->xmlChildrenNode; cur != NULL; cur = cur->next) {
        const xmlNode *child = NULL;
        if (!xmlStrcmp(cur->name, reinterpret_cast<const xmlChar*>(Trait::collectionTag))) {
            child = cur->xmlChildrenNode;
        } else if (!xmlStrcmp(cur->name, reinterpret_cast<const xmlChar*>(Trait::tag))) {
            child = cur;
        }
        for (; child != NULL; child = child->next) {
            if (!xmlStrcmp(child->name, reinterpret_cast<const xmlChar*>(Trait::tag))) {
                auto element = Trait::deserialize(child, serializingContext);
                if (element.isOk()) {
                    status_t status = Trait::addElementToCollection(element, collection);
                    if (status != NO_ERROR) {
                        ALOGE("%s: could not add element to %s collection", __func__,
                            Trait::collectionTag);
                        return status;
                    }
                } else {
                    return BAD_VALUE;
                }
            }
        }
        if (!xmlStrcmp(cur->name, reinterpret_cast<const xmlChar*>(Trait::tag))) {
            return NO_ERROR;
        }
    }
    return NO_ERROR;
}

Return<AudioGainTraits::Element> AudioGainTraits::deserialize(const xmlNode *cur,
        PtrSerializingCtx /*serializingContext*/)
{
    static uint32_t index = 0;
    Element gain = new AudioGain(index++, true);

    std::string mode = getXmlAttribute(cur, Attributes::mode);
    if (!mode.empty()) {
        gain->setMode(GainModeConverter::maskFromString(mode));
    }

    std::string channelsLiteral = getXmlAttribute(cur, Attributes::channelMask);
    if (!channelsLiteral.empty()) {
        gain->setChannelMask(channelMaskFromString(channelsLiteral));
    }

    std::string minValueMBLiteral = getXmlAttribute(cur, Attributes::minValueMB);
    int32_t minValueMB;
    if (!minValueMBLiteral.empty() && convertTo(minValueMBLiteral, minValueMB)) {
        gain->setMinValueInMb(minValueMB);
    }

    std::string maxValueMBLiteral = getXmlAttribute(cur, Attributes::maxValueMB);
    int32_t maxValueMB;
    if (!maxValueMBLiteral.empty() && convertTo(maxValueMBLiteral, maxValueMB)) {
        gain->setMaxValueInMb(maxValueMB);
    }

    std::string defaultValueMBLiteral = getXmlAttribute(cur, Attributes::defaultValueMB);
    int32_t defaultValueMB;
    if (!defaultValueMBLiteral.empty() && convertTo(defaultValueMBLiteral, defaultValueMB)) {
        gain->setDefaultValueInMb(defaultValueMB);
    }

    std::string stepValueMBLiteral = getXmlAttribute(cur, Attributes::stepValueMB);
    uint32_t stepValueMB;
    if (!stepValueMBLiteral.empty() && convertTo(stepValueMBLiteral, stepValueMB)) {
        gain->setStepValueInMb(stepValueMB);
    }

    std::string minRampMsLiteral = getXmlAttribute(cur, Attributes::minRampMs);
    uint32_t minRampMs;
    if (!minRampMsLiteral.empty() && convertTo(minRampMsLiteral, minRampMs)) {
        gain->setMinRampInMs(minRampMs);
    }

    std::string maxRampMsLiteral = getXmlAttribute(cur, Attributes::maxRampMs);
    uint32_t maxRampMs;
    if (!maxRampMsLiteral.empty() && convertTo(maxRampMsLiteral, maxRampMs)) {
        gain->setMaxRampInMs(maxRampMs);
    }
    std::string useForVolumeLiteral = getXmlAttribute(cur, Attributes::useForVolume);
    bool useForVolume = false;
    if (!useForVolumeLiteral.empty() && convertTo(useForVolumeLiteral, useForVolume)) {
        gain->setUseForVolume(useForVolume);
    }
    ALOGV("%s: adding new gain mode %08x channel mask %08x min mB %d max mB %d UseForVolume: %d",
          __func__, gain->getMode(), gain->getChannelMask(), gain->getMinValueInMb(),
          gain->getMaxValueInMb(), useForVolume);

    if (gain->getMode() != 0) {
        return gain;
    } else {
        return Status::fromStatusT(BAD_VALUE);
    }
}

Return<AudioProfileTraits::Element> AudioProfileTraits::deserialize(const xmlNode *cur,
        PtrSerializingCtx /*serializingContext*/)
{
    std::string samplingRates = getXmlAttribute(cur, Attributes::samplingRates);
    std::string format = getXmlAttribute(cur, Attributes::format);
    std::string channels = getXmlAttribute(cur, Attributes::channelMasks);

    Element profile = new AudioProfile(formatFromString(format, gDynamicFormat),
            channelMasksFromString(channels, ","),
            samplingRatesFromString(samplingRates, ","));

    profile->setDynamicFormat(profile->getFormat() == gDynamicFormat);
    profile->setDynamicChannels(profile->getChannels().isEmpty());
    profile->setDynamicRate(profile->getSampleRates().isEmpty());

    return profile;
}

Return<MixPortTraits::Element> MixPortTraits::deserialize(const xmlNode *child,
        PtrSerializingCtx /*serializingContext*/)
{
    std::string name = getXmlAttribute(child, Attributes::name);
    if (name.empty()) {
        ALOGE("%s: No %s found", __func__, Attributes::name);
        return Status::fromStatusT(BAD_VALUE);
    }
    ALOGV("%s: %s %s=%s", __func__, tag, Attributes::name, name.c_str());
    std::string role = getXmlAttribute(child, Attributes::role);
    if (role.empty()) {
        ALOGE("%s: No %s found", __func__, Attributes::role);
        return Status::fromStatusT(BAD_VALUE);
    }
    ALOGV("%s: Role=%s", __func__, role.c_str());
    audio_port_role_t portRole = (role == Attributes::roleSource) ?
            AUDIO_PORT_ROLE_SOURCE : AUDIO_PORT_ROLE_SINK;

    Element mixPort = new IOProfile(String8(name.c_str()), portRole);

    AudioProfileTraits::Collection profiles;
    status_t status = deserializeCollection<AudioProfileTraits>(child, &profiles, NULL);
    if (status != NO_ERROR) {
        return Status::fromStatusT(status);
    }
    if (profiles.isEmpty()) {
        profiles.add(AudioProfile::createFullDynamic());
    }
    mixPort->setAudioProfiles(profiles);

    std::string flags = getXmlAttribute(child, Attributes::flags);
    if (!flags.empty()) {
        // Source role
        if (portRole == AUDIO_PORT_ROLE_SOURCE) {
            mixPort->setFlags(OutputFlagConverter::maskFromString(flags));
        } else {
            // Sink role
            mixPort->setFlags(InputFlagConverter::maskFromString(flags));
        }
    }
    std::string maxOpenCount = getXmlAttribute(child, Attributes::maxOpenCount);
    if (!maxOpenCount.empty()) {
        convertTo(maxOpenCount, mixPort->maxOpenCount);
    }
    std::string maxActiveCount = getXmlAttribute(child, Attributes::maxActiveCount);
    if (!maxActiveCount.empty()) {
        convertTo(maxActiveCount, mixPort->maxActiveCount);
    }
    // Deserialize children
    AudioGainTraits::Collection gains;
    status = deserializeCollection<AudioGainTraits>(child, &gains, NULL);
    if (status != NO_ERROR) {
        return Status::fromStatusT(status);
    }
    mixPort->setGains(gains);

    return mixPort;
}

Return<DevicePortTraits::Element> DevicePortTraits::deserialize(const xmlNode *cur,
        PtrSerializingCtx /*serializingContext*/)
{
    std::string name = getXmlAttribute(cur, Attributes::tagName);
    if (name.empty()) {
        ALOGE("%s: No %s found", __func__, Attributes::tagName);
        return Status::fromStatusT(BAD_VALUE);
    }
    ALOGV("%s: %s %s=%s", __func__, tag, Attributes::tagName, name.c_str());
    std::string typeName = getXmlAttribute(cur, Attributes::type);
    if (typeName.empty()) {
        ALOGE("%s: no type for %s", __func__, name.c_str());
        return Status::fromStatusT(BAD_VALUE);
    }
    ALOGV("%s: %s %s=%s", __func__, tag, Attributes::type, typeName.c_str());
    std::string role = getXmlAttribute(cur, Attributes::role);
    if (role.empty()) {
        ALOGE("%s: No %s found", __func__, Attributes::role);
        return Status::fromStatusT(BAD_VALUE);
    }
    ALOGV("%s: %s %s=%s", __func__, tag, Attributes::role, role.c_str());
    audio_port_role_t portRole = (role == Attributes::roleSource) ?
                AUDIO_PORT_ROLE_SOURCE : AUDIO_PORT_ROLE_SINK;

    audio_devices_t type = AUDIO_DEVICE_NONE;
    if (!deviceFromString(typeName, type) ||
            (!audio_is_input_device(type) && portRole == AUDIO_PORT_ROLE_SOURCE) ||
            (!audio_is_output_devices(type) && portRole == AUDIO_PORT_ROLE_SINK)) {
        ALOGW("%s: bad type %08x", __func__, type);
        return Status::fromStatusT(BAD_VALUE);
    }
    std::string encodedFormatsLiteral = getXmlAttribute(cur, Attributes::encodedFormats);
    ALOGV("%s: %s %s=%s", __func__, tag, Attributes::encodedFormats, encodedFormatsLiteral.c_str());
    FormatVector encodedFormats;
    if (!encodedFormatsLiteral.empty()) {
        encodedFormats = formatsFromString(encodedFormatsLiteral, " ");
    }
    Element deviceDesc = new DeviceDescriptor(type, encodedFormats, String8(name.c_str()));

    std::string address = getXmlAttribute(cur, Attributes::address);
    if (!address.empty()) {
        ALOGV("%s: address=%s for %s", __func__, address.c_str(), name.c_str());
        deviceDesc->setAddress(String8(address.c_str()));
    }

    AudioProfileTraits::Collection profiles;
    status_t status = deserializeCollection<AudioProfileTraits>(cur, &profiles, NULL);
    if (status != NO_ERROR) {
        return Status::fromStatusT(status);
    }
    if (profiles.isEmpty()) {
        profiles.add(AudioProfile::createFullDynamic());
    }
    deviceDesc->setAudioProfiles(profiles);

    // Deserialize AudioGain children
    status = deserializeCollection<AudioGainTraits>(cur, &deviceDesc->mGains, NULL);
    if (status != NO_ERROR) {
        return Status::fromStatusT(status);
    }
    ALOGV("%s: adding device tag %s type %08x address %s", __func__,
          deviceDesc->getName().string(), type, deviceDesc->address().string());
    return deviceDesc;
}

Return<RouteTraits::Element> RouteTraits::deserialize(const xmlNode *cur, PtrSerializingCtx ctx)
{
    std::string type = getXmlAttribute(cur, Attributes::type);
    if (type.empty()) {
        ALOGE("%s: No %s found", __func__, Attributes::type);
        return Status::fromStatusT(BAD_VALUE);
    }
    audio_route_type_t routeType = (type == Attributes::typeMix) ?
                AUDIO_ROUTE_MIX : AUDIO_ROUTE_MUX;

    ALOGV("%s: %s %s=%s", __func__, tag, Attributes::type, type.c_str());
    Element route = new AudioRoute(routeType);

    std::string sinkAttr = getXmlAttribute(cur, Attributes::sink);
    if (sinkAttr.empty()) {
        ALOGE("%s: No %s found", __func__, Attributes::sink);
        return Status::fromStatusT(BAD_VALUE);
    }
    // Convert Sink name to port pointer
    sp<AudioPort> sink = ctx->findPortByTagName(String8(sinkAttr.c_str()));
    if (sink == NULL) {
        ALOGE("%s: no sink found with name=%s", __func__, sinkAttr.c_str());
        return Status::fromStatusT(BAD_VALUE);
    }
    route->setSink(sink);

    std::string sourcesAttr = getXmlAttribute(cur, Attributes::sources);
    if (sourcesAttr.empty()) {
        ALOGE("%s: No %s found", __func__, Attributes::sources);
        return Status::fromStatusT(BAD_VALUE);
    }
    // Tokenize and Convert Sources name to port pointer
    AudioPortVector sources;
    std::unique_ptr<char[]> sourcesLiteral{strndup(
                sourcesAttr.c_str(), strlen(sourcesAttr.c_str()))};
    char *devTag = strtok(sourcesLiteral.get(), ",");
    while (devTag != NULL) {
        if (strlen(devTag) != 0) {
            sp<AudioPort> source = ctx->findPortByTagName(String8(devTag));
            if (source == NULL) {
                ALOGE("%s: no source found with name=%s", __func__, devTag);
                return Status::fromStatusT(BAD_VALUE);
            }
            sources.add(source);
        }
        devTag = strtok(NULL, ",");
    }

    sink->addRoute(route);
    for (size_t i = 0; i < sources.size(); i++) {
        sp<AudioPort> source = sources.itemAt(i);
        source->addRoute(route);
    }
    route->setSources(sources);
    return route;
}

Return<ModuleTraits::Element> ModuleTraits::deserialize(const xmlNode *cur, PtrSerializingCtx ctx)
{
    std::string name = getXmlAttribute(cur, Attributes::name);
    if (name.empty()) {
        ALOGE("%s: No %s found", __func__, Attributes::name);
        return Status::fromStatusT(BAD_VALUE);
    }
    uint32_t versionMajor = 0, versionMinor = 0;
    std::string versionLiteral = getXmlAttribute(cur, Attributes::version);
    if (!versionLiteral.empty()) {
        sscanf(versionLiteral.c_str(), "%u.%u", &versionMajor, &versionMinor);
        ALOGV("%s: mHalVersion = major %u minor %u",  __func__,
              versionMajor, versionMajor);
    }

    ALOGV("%s: %s %s=%s", __func__, tag, Attributes::name, name.c_str());

    Element module = new HwModule(name.c_str(), versionMajor, versionMinor);

    // Deserialize childrens: Audio Mix Port, Audio Device Ports (Source/Sink), Audio Routes
    MixPortTraits::Collection mixPorts;
    status_t status = deserializeCollection<MixPortTraits>(cur, &mixPorts, NULL);
    if (status != NO_ERROR) {
        return Status::fromStatusT(status);
    }
    module->setProfiles(mixPorts);

    DevicePortTraits::Collection devicePorts;
    status = deserializeCollection<DevicePortTraits>(cur, &devicePorts, NULL);
    if (status != NO_ERROR) {
        return Status::fromStatusT(status);
    }
    module->setDeclaredDevices(devicePorts);

    RouteTraits::Collection routes;
    status = deserializeCollection<RouteTraits>(cur, &routes, module.get());
    if (status != NO_ERROR) {
        return Status::fromStatusT(status);
    }
    module->setRoutes(routes);

    for (const xmlNode *children = cur->xmlChildrenNode; children != NULL;
         children = children->next) {
        if (!xmlStrcmp(children->name, reinterpret_cast<const xmlChar*>(childAttachedDevicesTag))) {
            ALOGV("%s: %s %s found", __func__, tag, childAttachedDevicesTag);
            for (const xmlNode *child = children->xmlChildrenNode; child != NULL;
                 child = child->next) {
                if (!xmlStrcmp(child->name,
                                reinterpret_cast<const xmlChar*>(childAttachedDeviceTag))) {
                    auto attachedDevice = make_xmlUnique(xmlNodeListGetString(
                                    child->doc, child->xmlChildrenNode, 1));
                    if (attachedDevice != nullptr) {
                        ALOGV("%s: %s %s=%s", __func__, tag, childAttachedDeviceTag,
                                reinterpret_cast<const char*>(attachedDevice.get()));
                        sp<DeviceDescriptor> device = module->getDeclaredDevices().
                                getDeviceFromTagName(String8(reinterpret_cast<const char*>(
                                                        attachedDevice.get())));
                        ctx->addAvailableDevice(device);
                    }
                }
            }
        }
        if (!xmlStrcmp(children->name,
                        reinterpret_cast<const xmlChar*>(childDefaultOutputDeviceTag))) {
            auto defaultOutputDevice = make_xmlUnique(xmlNodeListGetString(
                            children->doc, children->xmlChildrenNode, 1));
            if (defaultOutputDevice != nullptr) {
                ALOGV("%s: %s %s=%s", __func__, tag, childDefaultOutputDeviceTag,
                        reinterpret_cast<const char*>(defaultOutputDevice.get()));
                sp<DeviceDescriptor> device = module->getDeclaredDevices().getDeviceFromTagName(
                        String8(reinterpret_cast<const char*>(defaultOutputDevice.get())));
                if (device != 0 && ctx->getDefaultOutputDevice() == 0) {
                    ctx->setDefaultOutputDevice(device);
                    ALOGV("%s: default is %08x",
                            __func__, ctx->getDefaultOutputDevice()->type());
                }
            }
        }
    }
    return module;
}

status_t GlobalConfigTraits::deserialize(const xmlNode *root, AudioPolicyConfig *config)
{
    for (const xmlNode *cur = root->xmlChildrenNode; cur != NULL; cur = cur->next) {
        if (!xmlStrcmp(cur->name, reinterpret_cast<const xmlChar*>(tag))) {
            std::string speakerDrcEnabled =
                    getXmlAttribute(cur, Attributes::speakerDrcEnabled);
            bool isSpeakerDrcEnabled;
            if (!speakerDrcEnabled.empty() &&
                    convertTo<std::string, bool>(speakerDrcEnabled, isSpeakerDrcEnabled)) {
                config->setSpeakerDrcEnabled(isSpeakerDrcEnabled);
            }
            return NO_ERROR;
        }
    }
    return NO_ERROR;
}

status_t SurroundSoundTraits::deserialize(const xmlNode *root, AudioPolicyConfig *config)
{
    config->setDefaultSurroundFormats();

    for (const xmlNode *cur = root->xmlChildrenNode; cur != NULL; cur = cur->next) {
        if (!xmlStrcmp(cur->name, reinterpret_cast<const xmlChar*>(tag))) {
            AudioPolicyConfig::SurroundFormats formats;
            status_t status = deserializeCollection<SurroundSoundFormatTraits>(
                    cur, &formats, nullptr);
            if (status == NO_ERROR) {
                config->setSurroundFormats(formats);
            }
            return NO_ERROR;
        }
    }
    return NO_ERROR;
}

Return<SurroundSoundFormatTraits::Element> SurroundSoundFormatTraits::deserialize(
        const xmlNode *cur, PtrSerializingCtx /*serializingContext*/)
{
    std::string formatLiteral = getXmlAttribute(cur, Attributes::name);
    if (formatLiteral.empty()) {
        ALOGE("%s: No %s found for a surround format", __func__, Attributes::name);
        return Status::fromStatusT(BAD_VALUE);
    }
    audio_format_t format = formatFromString(formatLiteral);
    if (format == AUDIO_FORMAT_DEFAULT) {
        ALOGE("%s: Unrecognized format %s", __func__, formatLiteral.c_str());
        return Status::fromStatusT(BAD_VALUE);
    }
    Element pair = std::make_pair(format, Collection::mapped_type{});

    std::string subformatsLiteral = getXmlAttribute(cur, Attributes::subformats);
    if (subformatsLiteral.empty()) return pair;
    FormatVector subformats = formatsFromString(subformatsLiteral, " ");
    for (const auto& subformat : subformats) {
        auto result = pair.second.insert(subformat);
        if (!result.second) {
            ALOGE("%s: could not add subformat %x to collection", __func__, subformat);
            return Status::fromStatusT(BAD_VALUE);
        }
    }
    return pair;
}

status_t PolicySerializer::deserialize(const char *configFile, AudioPolicyConfig *config)
{
    auto doc = make_xmlUnique(xmlParseFile(configFile));
    if (doc == nullptr) {
        ALOGE("%s: Could not parse %s document.", __func__, configFile);
        return BAD_VALUE;
    }
    xmlNodePtr root = xmlDocGetRootElement(doc.get());
    if (root == NULL) {
        ALOGE("%s: Could not parse %s document: empty.", __func__, configFile);
        return BAD_VALUE;
    }
    if (xmlXIncludeProcess(doc.get()) < 0) {
        ALOGE("%s: libxml failed to resolve XIncludes on %s document.", __func__, configFile);
    }

    if (xmlStrcmp(root->name, reinterpret_cast<const xmlChar*>(rootName)))  {
        ALOGE("%s: No %s root element found in xml data %s.", __func__, rootName,
                reinterpret_cast<const char*>(root->name));
        return BAD_VALUE;
    }

    std::string version = getXmlAttribute(root, versionAttribute);
    if (version.empty()) {
        ALOGE("%s: No version found in root node %s", __func__, rootName);
        return BAD_VALUE;
    }
    if (version != mVersion) {
        ALOGE("%s: Version does not match; expect %s got %s", __func__, mVersion.c_str(),
              version.c_str());
        return BAD_VALUE;
    }
    // Lets deserialize children
    // Modules
    ModuleTraits::Collection modules;
    status_t status = deserializeCollection<ModuleTraits>(root, &modules, config);
    if (status != NO_ERROR) {
        return status;
    }
    config->setHwModules(modules);

    // Global Configuration
    GlobalConfigTraits::deserialize(root, config);

    // Surround configuration
    SurroundSoundTraits::deserialize(root, config);

    return android::OK;
}

}  // namespace

status_t deserializeAudioPolicyFile(const char *fileName, AudioPolicyConfig *config)
{
    PolicySerializer serializer;
    return serializer.deserialize(fileName, config);
}

} // namespace android
