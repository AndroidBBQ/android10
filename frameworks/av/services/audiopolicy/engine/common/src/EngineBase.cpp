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

#define LOG_TAG "APM::AudioPolicyEngine/Base"
//#define LOG_NDEBUG 0

#include "EngineBase.h"
#include "EngineDefaultConfig.h"
#include <TypeConverter.h>

namespace android {
namespace audio_policy {

void EngineBase::setObserver(AudioPolicyManagerObserver *observer)
{
    ALOG_ASSERT(observer != NULL, "Invalid Audio Policy Manager observer");
    mApmObserver = observer;
}

status_t EngineBase::initCheck()
{
    return (mApmObserver != nullptr)? NO_ERROR : NO_INIT;
}

status_t EngineBase::setPhoneState(audio_mode_t state)
{
    ALOGV("setPhoneState() state %d", state);

    if (state < 0 || state >= AUDIO_MODE_CNT) {
        ALOGW("setPhoneState() invalid state %d", state);
        return BAD_VALUE;
    }

    if (state == mPhoneState ) {
        ALOGW("setPhoneState() setting same state %d", state);
        return BAD_VALUE;
    }

    // store previous phone state for management of sonification strategy below
    int oldState = mPhoneState;
    mPhoneState = state;

    if (!is_state_in_call(oldState) && is_state_in_call(state)) {
        ALOGV("  Entering call in setPhoneState()");
        switchVolumeCurve(AUDIO_STREAM_VOICE_CALL, AUDIO_STREAM_DTMF);
    } else if (is_state_in_call(oldState) && !is_state_in_call(state)) {
        ALOGV("  Exiting call in setPhoneState()");
        restoreOriginVolumeCurve(AUDIO_STREAM_DTMF);
    }
    return NO_ERROR;
}

product_strategy_t EngineBase::getProductStrategyForAttributes(const audio_attributes_t &attr) const
{
    return mProductStrategies.getProductStrategyForAttributes(attr);
}

audio_stream_type_t EngineBase::getStreamTypeForAttributes(const audio_attributes_t &attr) const
{
    return mProductStrategies.getStreamTypeForAttributes(attr);
}

audio_attributes_t EngineBase::getAttributesForStreamType(audio_stream_type_t stream) const
{
    return mProductStrategies.getAttributesForStreamType(stream);
}

product_strategy_t EngineBase::getProductStrategyForStream(audio_stream_type_t stream) const
{
    return mProductStrategies.getProductStrategyForStream(stream);
}

product_strategy_t EngineBase::getProductStrategyByName(const std::string &name) const
{
    for (const auto &iter : mProductStrategies) {
        if (iter.second->getName() == name) {
            return iter.second->getId();
        }
    }
    return PRODUCT_STRATEGY_NONE;
}

engineConfig::ParsingResult EngineBase::loadAudioPolicyEngineConfig()
{
    auto loadProductStrategies =
            [](auto& strategyConfigs, auto& productStrategies, auto& volumeGroups) {
        for (auto& strategyConfig : strategyConfigs) {
            sp<ProductStrategy> strategy = new ProductStrategy(strategyConfig.name);
            for (const auto &group : strategyConfig.attributesGroups) {
                const auto &iter = std::find_if(begin(volumeGroups), end(volumeGroups),
                                         [&group](const auto &volumeGroup) {
                        return group.volumeGroup == volumeGroup.second->getName(); });
                ALOG_ASSERT(iter != end(volumeGroups), "Invalid Volume Group Name %s",
                            group.volumeGroup.c_str());
                if (group.stream != AUDIO_STREAM_DEFAULT) {
                    iter->second->addSupportedStream(group.stream);
                }
                for (const auto &attr : group.attributesVect) {
                    strategy->addAttributes({group.stream, iter->second->getId(), attr});
                    iter->second->addSupportedAttributes(attr);
                }
            }
            product_strategy_t strategyId = strategy->getId();
            productStrategies[strategyId] = strategy;
        }
    };
    auto loadVolumeGroups = [](auto &volumeConfigs, auto &volumeGroups) {
        for (auto &volumeConfig : volumeConfigs) {
            sp<VolumeGroup> volumeGroup = new VolumeGroup(volumeConfig.name, volumeConfig.indexMin,
                                                          volumeConfig.indexMax);
            volumeGroups[volumeGroup->getId()] = volumeGroup;

            for (auto &configCurve : volumeConfig.volumeCurves) {
                device_category deviceCat = DEVICE_CATEGORY_SPEAKER;
                if (!DeviceCategoryConverter::fromString(configCurve.deviceCategory, deviceCat)) {
                    ALOGE("%s: Invalid %s", __FUNCTION__, configCurve.deviceCategory.c_str());
                    continue;
                }
                sp<VolumeCurve> curve = new VolumeCurve(deviceCat);
                for (auto &point : configCurve.curvePoints) {
                    curve->add({point.index, point.attenuationInMb});
                }
                volumeGroup->add(curve);
            }
        }
    };
    auto result = engineConfig::parse();
    if (result.parsedConfig == nullptr) {
        ALOGW("%s: No configuration found, using default matching phone experience.", __FUNCTION__);
        engineConfig::Config config = gDefaultEngineConfig;
        android::status_t ret = engineConfig::parseLegacyVolumes(config.volumeGroups);
        result = {std::make_unique<engineConfig::Config>(config),
                  static_cast<size_t>(ret == NO_ERROR ? 0 : 1)};
    }
    ALOGE_IF(result.nbSkippedElement != 0, "skipped %zu elements", result.nbSkippedElement);
    loadVolumeGroups(result.parsedConfig->volumeGroups, mVolumeGroups);
    loadProductStrategies(result.parsedConfig->productStrategies, mProductStrategies,
                          mVolumeGroups);
    mProductStrategies.initialize();
    return result;
}

StrategyVector EngineBase::getOrderedProductStrategies() const
{
    auto findByFlag = [](const auto &productStrategies, auto flag) {
        return std::find_if(begin(productStrategies), end(productStrategies),
                            [&](const auto &strategy) {
            for (const auto &attributes : strategy.second->getAudioAttributes()) {
                if ((attributes.flags & flag) == flag) {
                    return true;
                }
            }
            return false;
        });
    };
    auto strategies = mProductStrategies;
    auto enforcedAudibleStrategyIter = findByFlag(strategies, AUDIO_FLAG_AUDIBILITY_ENFORCED);

    if (getForceUse(AUDIO_POLICY_FORCE_FOR_SYSTEM) == AUDIO_POLICY_FORCE_SYSTEM_ENFORCED &&
            enforcedAudibleStrategyIter != strategies.end()) {
        auto enforcedAudibleStrategy = *enforcedAudibleStrategyIter;
        strategies.erase(enforcedAudibleStrategyIter);
        strategies.insert(begin(strategies), enforcedAudibleStrategy);
    }
    StrategyVector orderedStrategies;
    for (const auto &iter : strategies) {
        orderedStrategies.push_back(iter.second->getId());
    }
    return orderedStrategies;
}

StreamTypeVector EngineBase::getStreamTypesForProductStrategy(product_strategy_t ps) const
{
    // @TODO default music stream to control volume if no group?
    return (mProductStrategies.find(ps) != end(mProductStrategies)) ?
                mProductStrategies.at(ps)->getSupportedStreams() :
                StreamTypeVector(AUDIO_STREAM_MUSIC);
}

AttributesVector EngineBase::getAllAttributesForProductStrategy(product_strategy_t ps) const
{
    return (mProductStrategies.find(ps) != end(mProductStrategies)) ?
                mProductStrategies.at(ps)->getAudioAttributes() : AttributesVector();
}

status_t EngineBase::listAudioProductStrategies(AudioProductStrategyVector &strategies) const
{
    for (const auto &iter : mProductStrategies) {
        const auto &productStrategy = iter.second;
        strategies.push_back(
        {productStrategy->getName(), productStrategy->listAudioAttributes(),
         productStrategy->getId()});
    }
    return NO_ERROR;
}

VolumeCurves *EngineBase::getVolumeCurvesForAttributes(const audio_attributes_t &attr) const
{
    volume_group_t volGr = mProductStrategies.getVolumeGroupForAttributes(attr);
    const auto &iter = mVolumeGroups.find(volGr);
    LOG_ALWAYS_FATAL_IF(iter == std::end(mVolumeGroups), "No volume groups for %s", toString(attr).c_str());
    return mVolumeGroups.at(volGr)->getVolumeCurves();
}

VolumeCurves *EngineBase::getVolumeCurvesForStreamType(audio_stream_type_t stream) const
{
    volume_group_t volGr = mProductStrategies.getVolumeGroupForStreamType(stream);
    if (volGr == VOLUME_GROUP_NONE) {
        volGr = mProductStrategies.getDefaultVolumeGroup();
    }
    const auto &iter = mVolumeGroups.find(volGr);
    LOG_ALWAYS_FATAL_IF(iter == std::end(mVolumeGroups), "No volume groups for %s",
                toString(stream).c_str());
    return mVolumeGroups.at(volGr)->getVolumeCurves();
}

status_t EngineBase::switchVolumeCurve(audio_stream_type_t streamSrc, audio_stream_type_t streamDst)
{
    auto srcCurves = getVolumeCurvesForStreamType(streamSrc);
    auto dstCurves = getVolumeCurvesForStreamType(streamDst);

    if (srcCurves == nullptr || dstCurves == nullptr) {
        return BAD_VALUE;
    }
    return dstCurves->switchCurvesFrom(*srcCurves);
}

status_t EngineBase::restoreOriginVolumeCurve(audio_stream_type_t stream)
{
    VolumeCurves *curves = getVolumeCurvesForStreamType(stream);
    return curves != nullptr ? curves->switchCurvesFrom(*curves) : BAD_VALUE;
}

VolumeGroupVector EngineBase::getVolumeGroups() const
{
    VolumeGroupVector group;
    for (const auto &iter : mVolumeGroups) {
        group.push_back(iter.first);
    }
    return group;
}

volume_group_t EngineBase::getVolumeGroupForAttributes(const audio_attributes_t &attr) const
{
    return mProductStrategies.getVolumeGroupForAttributes(attr);
}

volume_group_t EngineBase::getVolumeGroupForStreamType(audio_stream_type_t stream) const
{
    return mProductStrategies.getVolumeGroupForStreamType(stream);
}

status_t EngineBase::listAudioVolumeGroups(AudioVolumeGroupVector &groups) const
{
    for (const auto &iter : mVolumeGroups) {
        groups.push_back({iter.second->getName(), iter.second->getId(),
                          iter.second->getSupportedAttributes(), iter.second->getStreamTypes()});
    }
    return NO_ERROR;
}

void EngineBase::dump(String8 *dst) const
{
    mProductStrategies.dump(dst, 2);
    mVolumeGroups.dump(dst, 2);
}

} // namespace audio_policy
} // namespace android
