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

#include <media/AudioPolicy.h>

#include "TypeConverter.h"

namespace android {

#define MAKE_STRING_FROM_ENUM(string) { #string, string }
#define TERMINATOR { .literal = nullptr }

template <>
const DeviceCategoryConverter::Table DeviceCategoryConverter::mTable[] = {
    MAKE_STRING_FROM_ENUM(DEVICE_CATEGORY_HEADSET),
    MAKE_STRING_FROM_ENUM(DEVICE_CATEGORY_SPEAKER),
    MAKE_STRING_FROM_ENUM(DEVICE_CATEGORY_EARPIECE),
    MAKE_STRING_FROM_ENUM(DEVICE_CATEGORY_EXT_MEDIA),
    MAKE_STRING_FROM_ENUM(DEVICE_CATEGORY_HEARING_AID),
    TERMINATOR
};

template <>
const MixTypeConverter::Table MixTypeConverter::mTable[] = {
    MAKE_STRING_FROM_ENUM(MIX_TYPE_INVALID),
    MAKE_STRING_FROM_ENUM(MIX_TYPE_PLAYERS),
    MAKE_STRING_FROM_ENUM(MIX_TYPE_RECORDERS),
    TERMINATOR
};

template <>
const RouteFlagTypeConverter::Table RouteFlagTypeConverter::mTable[] = {
    MAKE_STRING_FROM_ENUM(MIX_ROUTE_FLAG_RENDER),
    MAKE_STRING_FROM_ENUM(MIX_ROUTE_FLAG_LOOP_BACK),
    MAKE_STRING_FROM_ENUM(MIX_ROUTE_FLAG_LOOP_BACK_AND_RENDER),
    MAKE_STRING_FROM_ENUM(MIX_ROUTE_FLAG_ALL),
    TERMINATOR
};

template <>
const RuleTypeConverter::Table RuleTypeConverter::mTable[] = {
    MAKE_STRING_FROM_ENUM(RULE_MATCH_ATTRIBUTE_USAGE),
    MAKE_STRING_FROM_ENUM(RULE_MATCH_ATTRIBUTE_CAPTURE_PRESET),
    MAKE_STRING_FROM_ENUM(RULE_MATCH_UID),
    MAKE_STRING_FROM_ENUM(RULE_EXCLUDE_ATTRIBUTE_USAGE),
    MAKE_STRING_FROM_ENUM(RULE_EXCLUDE_ATTRIBUTE_CAPTURE_PRESET),
    MAKE_STRING_FROM_ENUM(RULE_EXCLUDE_UID),
    TERMINATOR
};

template class TypeConverter<DeviceCategoryTraits>;
template class TypeConverter<MixTypeTraits>;
template class TypeConverter<RouteFlagTraits>;
template class TypeConverter<RuleTraits>;

} // namespace android
