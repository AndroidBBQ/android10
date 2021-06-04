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

#pragma once

#include <system/audio.h>

#include <string>
#include <vector>
#include <utils/Errors.h>

struct _xmlNode;
struct _xmlDoc;

namespace android {
namespace engineConfig {

/** Default path of audio policy usages configuration file. */
constexpr char DEFAULT_PATH[] = "/vendor/etc/audio_policy_engine_configuration.xml";

/** Directories where the effect libraries will be search for. */
constexpr const char* POLICY_USAGE_LIBRARY_PATH[] = {"/odm/etc/", "/vendor/etc/", "/system/etc/"};

using AttributesVector = std::vector<audio_attributes_t>;
using StreamVector = std::vector<audio_stream_type_t>;

struct AttributesGroup {
    std::string name;
    audio_stream_type_t stream;
    std::string volumeGroup;
    AttributesVector attributesVect;
};

using AttributesGroups = std::vector<AttributesGroup>;

struct CurvePoint {
    int index;
    int attenuationInMb;
};
using CurvePoints = std::vector<CurvePoint>;

struct VolumeCurve {
    std::string deviceCategory;
    CurvePoints curvePoints;
};
using VolumeCurves = std::vector<VolumeCurve>;

struct VolumeGroup {
    std::string name;
    int indexMin;
    int indexMax;
    VolumeCurves volumeCurves;
};
using VolumeGroups = std::vector<VolumeGroup>;

struct ProductStrategy {
    std::string name;
    AttributesGroups attributesGroups;
};

using ProductStrategies = std::vector<ProductStrategy>;

using ValuePair = std::pair<uint32_t, std::string>;
using ValuePairs = std::vector<ValuePair>;

struct CriterionType
{
    std::string name;
    bool isInclusive;
    ValuePairs valuePairs;
};

using CriterionTypes = std::vector<CriterionType>;

struct Criterion
{
    std::string name;
    std::string typeName;
    std::string defaultLiteralValue;
};

using Criteria = std::vector<Criterion>;

struct Config {
    float version;
    ProductStrategies productStrategies;
    Criteria criteria;
    CriterionTypes criterionTypes;
    VolumeGroups volumeGroups;
};

/** Result of `parse(const char*)` */
struct ParsingResult {
    /** Parsed config, nullptr if the xml lib could not load the file */
    std::unique_ptr<Config> parsedConfig;
    size_t nbSkippedElement; //< Number of skipped invalid product strategies
};

/** Parses the provided audio policy usage configuration.
 * @return audio policy usage @see Config
 */
ParsingResult parse(const char* path = DEFAULT_PATH);
android::status_t parseLegacyVolumes(VolumeGroups &volumeGroups);

} // namespace engineConfig
} // namespace android
