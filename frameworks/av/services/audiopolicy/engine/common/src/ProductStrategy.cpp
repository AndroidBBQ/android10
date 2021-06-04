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

#define LOG_TAG "APM::AudioPolicyEngine/ProductStrategy"
//#define LOG_NDEBUG 0

#include "ProductStrategy.h"

#include <media/TypeConverter.h>
#include <utils/String8.h>
#include <cstdint>
#include <string>

#include <log/log.h>


namespace android {

ProductStrategy::ProductStrategy(const std::string &name) :
    mName(name),
    mId(static_cast<product_strategy_t>(HandleGenerator<uint32_t>::getNextHandle()))
{
}

void ProductStrategy::addAttributes(const AudioAttributes &audioAttributes)
{
    mAttributesVector.push_back(audioAttributes);
}

std::vector<android::AudioAttributes> ProductStrategy::listAudioAttributes() const
{
    std::vector<android::AudioAttributes> androidAa;
    for (const auto &attr : mAttributesVector) {
        androidAa.push_back({attr.mVolumeGroup, attr.mStream, attr.mAttributes});
    }
    return androidAa;
}

AttributesVector ProductStrategy::getAudioAttributes() const
{
    AttributesVector attrVector;
    for (const auto &attrGroup : mAttributesVector) {
        attrVector.push_back(attrGroup.mAttributes);
    }
    if (not attrVector.empty()) {
        return attrVector;
    }
    return { AUDIO_ATTRIBUTES_INITIALIZER };
}

bool ProductStrategy::matches(const audio_attributes_t attr) const
{
    return std::find_if(begin(mAttributesVector), end(mAttributesVector),
                        [&attr](const auto &supportedAttr) {
        return AudioProductStrategy::attributesMatches(supportedAttr.mAttributes, attr);
    }) != end(mAttributesVector);
}

audio_stream_type_t ProductStrategy::getStreamTypeForAttributes(
        const audio_attributes_t &attr) const
{
    const auto iter = std::find_if(begin(mAttributesVector), end(mAttributesVector),
                                   [&attr](const auto &supportedAttr) {
        return AudioProductStrategy::attributesMatches(supportedAttr.mAttributes, attr); });
    return iter != end(mAttributesVector) ? iter->mStream : AUDIO_STREAM_DEFAULT;
}

audio_attributes_t ProductStrategy::getAttributesForStreamType(audio_stream_type_t streamType) const
{
    const auto iter = std::find_if(begin(mAttributesVector), end(mAttributesVector),
                                   [&streamType](const auto &supportedAttr) {
        return supportedAttr.mStream == streamType; });
    return iter != end(mAttributesVector) ? iter->mAttributes : AUDIO_ATTRIBUTES_INITIALIZER;
}

bool ProductStrategy::isDefault() const
{
    return std::find_if(begin(mAttributesVector), end(mAttributesVector), [](const auto &attr) {
        return attr.mAttributes == defaultAttr; }) != end(mAttributesVector);
}

StreamTypeVector ProductStrategy::getSupportedStreams() const
{
    StreamTypeVector streams;
    for (const auto &supportedAttr : mAttributesVector) {
        if (std::find(begin(streams), end(streams), supportedAttr.mStream) == end(streams) &&
                supportedAttr.mStream != AUDIO_STREAM_DEFAULT) {
            streams.push_back(supportedAttr.mStream);
        }
    }
    return streams;
}

bool ProductStrategy::supportStreamType(const audio_stream_type_t &streamType) const
{
    return std::find_if(begin(mAttributesVector), end(mAttributesVector),
                        [&streamType](const auto &supportedAttr) {
        return supportedAttr.mStream == streamType; }) != end(mAttributesVector);
}

volume_group_t ProductStrategy::getVolumeGroupForAttributes(const audio_attributes_t &attr) const
{
    for (const auto &supportedAttr : mAttributesVector) {
        if (AudioProductStrategy::attributesMatches(supportedAttr.mAttributes, attr)) {
            return supportedAttr.mVolumeGroup;
        }
    }
    return VOLUME_GROUP_NONE;
}

volume_group_t ProductStrategy::getVolumeGroupForStreamType(audio_stream_type_t stream) const
{
    for (const auto &supportedAttr : mAttributesVector) {
        if (supportedAttr.mStream == stream) {
            return supportedAttr.mVolumeGroup;
        }
    }
    return VOLUME_GROUP_NONE;
}

volume_group_t ProductStrategy::getDefaultVolumeGroup() const
{
    const auto &iter = std::find_if(begin(mAttributesVector), end(mAttributesVector),
                                    [](const auto &attr) {return attr.mAttributes == defaultAttr;});
    return iter != end(mAttributesVector) ? iter->mVolumeGroup : VOLUME_GROUP_NONE;
}

void ProductStrategy::dump(String8 *dst, int spaces) const
{
    dst->appendFormat("\n%*s-%s (id: %d)\n", spaces, "", mName.c_str(), mId);
    std::string deviceLiteral;
    if (!OutputDeviceConverter::toString(mApplicableDevices, deviceLiteral)) {
        ALOGE("%s: failed to convert device %d", __FUNCTION__, mApplicableDevices);
    }
    dst->appendFormat("%*sSelected Device: {type:%s, @:%s}\n", spaces + 2, "",
                       deviceLiteral.c_str(), mDeviceAddress.c_str());

    for (const auto &attr : mAttributesVector) {
        dst->appendFormat("%*sGroup: %d stream: %s\n", spaces + 3, "", attr.mVolumeGroup,
                          android::toString(attr.mStream).c_str());
        dst->appendFormat("%*s Attributes: ", spaces + 3, "");
        std::string attStr =
                attr.mAttributes == defaultAttr ? "{ Any }" : android::toString(attr.mAttributes);
        dst->appendFormat("%s\n", attStr.c_str());
    }
}

product_strategy_t ProductStrategyMap::getProductStrategyForAttributes(
        const audio_attributes_t &attr) const
{
    for (const auto &iter : *this) {
        if (iter.second->matches(attr)) {
            return iter.second->getId();
        }
    }
    ALOGV("%s: No matching product strategy for attributes %s, return default", __FUNCTION__,
          toString(attr).c_str());
    return getDefault();
}

audio_attributes_t ProductStrategyMap::getAttributesForStreamType(audio_stream_type_t stream) const
{
    for (const auto &iter : *this) {
        const auto strategy = iter.second;
        if (strategy->supportStreamType(stream)) {
            return strategy->getAttributesForStreamType(stream);
        }
    }
    ALOGV("%s: No product strategy for stream %s, using default", __FUNCTION__,
          toString(stream).c_str());
    return {};
}

audio_stream_type_t ProductStrategyMap::getStreamTypeForAttributes(
        const audio_attributes_t &attr) const
{
    for (const auto &iter : *this) {
        audio_stream_type_t stream = iter.second->getStreamTypeForAttributes(attr);
        if (stream != AUDIO_STREAM_DEFAULT) {
            return stream;
        }
    }
    ALOGV("%s: No product strategy for attributes %s, using default (aka MUSIC)", __FUNCTION__,
          toString(attr).c_str());
    return  AUDIO_STREAM_MUSIC;
}

product_strategy_t ProductStrategyMap::getDefault() const
{
    if (mDefaultStrategy != PRODUCT_STRATEGY_NONE) {
        return mDefaultStrategy;
    }
    for (const auto &iter : *this) {
        if (iter.second->isDefault()) {
            ALOGV("%s: using default %s", __FUNCTION__, iter.second->getName().c_str());
            return iter.second->getId();
        }
    }
    ALOGE("%s: No default product strategy defined", __FUNCTION__);
    return PRODUCT_STRATEGY_NONE;
}

audio_attributes_t ProductStrategyMap::getAttributesForProductStrategy(
        product_strategy_t strategy) const
{
    if (find(strategy) == end()) {
        ALOGE("Invalid %d strategy requested", strategy);
        return AUDIO_ATTRIBUTES_INITIALIZER;
    }
    return at(strategy)->getAudioAttributes()[0];
}

product_strategy_t ProductStrategyMap::getProductStrategyForStream(audio_stream_type_t stream) const
{
    for (const auto &iter : *this) {
        if (iter.second->supportStreamType(stream)) {
            return iter.second->getId();
        }
    }
    ALOGV("%s: No product strategy for stream %d, using default", __FUNCTION__, stream);
    return getDefault();
}


audio_devices_t ProductStrategyMap::getDeviceTypesForProductStrategy(
        product_strategy_t strategy) const
{
    if (find(strategy) == end()) {
        ALOGE("Invalid %d strategy requested, returning device for default strategy", strategy);
        product_strategy_t defaultStrategy = getDefault();
        if (defaultStrategy == PRODUCT_STRATEGY_NONE) {
            return AUDIO_DEVICE_NONE;
        }
        return at(getDefault())->getDeviceTypes();
    }
    return at(strategy)->getDeviceTypes();
}

std::string ProductStrategyMap::getDeviceAddressForProductStrategy(product_strategy_t psId) const
{
    if (find(psId) == end()) {
        ALOGE("Invalid %d strategy requested, returning device for default strategy", psId);
        product_strategy_t defaultStrategy = getDefault();
        if (defaultStrategy == PRODUCT_STRATEGY_NONE) {
            return {};
        }
        return at(getDefault())->getDeviceAddress();
    }
    return at(psId)->getDeviceAddress();
}

volume_group_t ProductStrategyMap::getVolumeGroupForAttributes(const audio_attributes_t &attr) const
{
    for (const auto &iter : *this) {
        volume_group_t group = iter.second->getVolumeGroupForAttributes(attr);
        if (group != VOLUME_GROUP_NONE) {
            return group;
        }
    }
    return getDefaultVolumeGroup();
}

volume_group_t ProductStrategyMap::getVolumeGroupForStreamType(audio_stream_type_t stream) const
{
    for (const auto &iter : *this) {
        volume_group_t group = iter.second->getVolumeGroupForStreamType(stream);
        if (group != VOLUME_GROUP_NONE) {
            return group;
        }
    }
    ALOGW("%s: no volume group for %s, using default", __func__, toString(stream).c_str());
    return getDefaultVolumeGroup();
}

volume_group_t ProductStrategyMap::getDefaultVolumeGroup() const
{
    product_strategy_t defaultStrategy = getDefault();
    if (defaultStrategy == PRODUCT_STRATEGY_NONE) {
        return VOLUME_GROUP_NONE;
    }
    return at(defaultStrategy)->getDefaultVolumeGroup();
}

void ProductStrategyMap::initialize()
{
    mDefaultStrategy = getDefault();
    ALOG_ASSERT(mDefaultStrategy != PRODUCT_STRATEGY_NONE, "No default product strategy found");
}

void ProductStrategyMap::dump(String8 *dst, int spaces) const
{
    dst->appendFormat("%*sProduct Strategies dump:", spaces, "");
    for (const auto &iter : *this) {
        iter.second->dump(dst, spaces + 2);
    }
}

}

