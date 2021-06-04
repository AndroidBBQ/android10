/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef ANDROID_TYPE_CONVERTER_H_
#define ANDROID_TYPE_CONVERTER_H_

#include <string>
#include <string.h>

#include <vector>
#include <system/audio.h>
#include <utils/Log.h>
#include <utils/Vector.h>
#include <utils/SortedVector.h>

#include <media/AudioParameter.h>
#include "convert.h"

namespace android {

template <typename T>
struct DefaultTraits
{
    typedef T Type;
    typedef std::vector<Type> Collection;
    static void add(Collection &collection, Type value)
    {
        collection.push_back(value);
    }
};
template <typename T>
struct VectorTraits
{
    typedef T Type;
    typedef Vector<Type> Collection;
    static void add(Collection &collection, Type value)
    {
        collection.add(value);
    }
};
template <typename T>
struct SortedVectorTraits
{
    typedef T Type;
    typedef SortedVector<Type> Collection;
    static void add(Collection &collection, Type value)
    {
        collection.add(value);
    }
};

using SampleRateTraits = SortedVectorTraits<uint32_t>;
using DeviceTraits = DefaultTraits<audio_devices_t>;
struct OutputDeviceTraits : public DeviceTraits {};
struct InputDeviceTraits : public DeviceTraits {};
using ChannelTraits = SortedVectorTraits<audio_channel_mask_t>;
struct OutputChannelTraits : public ChannelTraits {};
struct InputChannelTraits : public ChannelTraits {};
struct ChannelIndexTraits : public ChannelTraits {};
using InputFlagTraits = DefaultTraits<audio_input_flags_t>;
using OutputFlagTraits = DefaultTraits<audio_output_flags_t>;
using FormatTraits = VectorTraits<audio_format_t>;
using GainModeTraits = DefaultTraits<audio_gain_mode_t>;
using StreamTraits = DefaultTraits<audio_stream_type_t>;
using AudioModeTraits = DefaultTraits<audio_mode_t>;
using AudioContentTraits = DefaultTraits<audio_content_type_t>;
using UsageTraits = DefaultTraits<audio_usage_t>;
using SourceTraits = DefaultTraits<audio_source_t>;
struct AudioFlagTraits : public DefaultTraits<audio_flags_mask_t> {};

template <class Traits>
static void collectionFromString(const std::string &str, typename Traits::Collection &collection,
                                 const char *del = AudioParameter::valueListSeparator)
{
    char *literal = strdup(str.c_str());
    for (const char *cstr = strtok(literal, del); cstr != NULL; cstr = strtok(NULL, del)) {
        typename Traits::Type value;
        if (utilities::convertTo<std::string, typename Traits::Type >(cstr, value)) {
            Traits::add(collection, value);
        }
    }
    free(literal);
}

template <class Traits>
class TypeConverter
{
public:
    static bool toString(const typename Traits::Type &value, std::string &str);

    static bool fromString(const std::string &str, typename Traits::Type &result);

    static void collectionFromString(const std::string &str,
                                     typename Traits::Collection &collection,
                                     const char *del = AudioParameter::valueListSeparator);

    static uint32_t maskFromString(
            const std::string &str, const char *del = AudioParameter::valueListSeparator);

    static void maskToString(
            uint32_t mask, std::string &str, const char *del = AudioParameter::valueListSeparator);

protected:
    struct Table {
        const char *literal;
        typename Traits::Type value;
    };

    static const Table mTable[];
};

template <class Traits>
inline bool TypeConverter<Traits>::toString(const typename Traits::Type &value, std::string &str)
{
    for (size_t i = 0; mTable[i].literal; i++) {
        if (mTable[i].value == value) {
            str = mTable[i].literal;
            return true;
        }
    }
    char result[64];
    snprintf(result, sizeof(result), "Unknown enum value %d", value);
    str = result;
    return false;
}

template <class Traits>
inline bool TypeConverter<Traits>::fromString(const std::string &str, typename Traits::Type &result)
{
    for (size_t i = 0; mTable[i].literal; i++) {
        if (strcmp(mTable[i].literal, str.c_str()) == 0) {
            ALOGV("stringToEnum() found %s", mTable[i].literal);
            result = mTable[i].value;
            return true;
        }
    }
    return false;
}

template <class Traits>
inline void TypeConverter<Traits>::collectionFromString(const std::string &str,
        typename Traits::Collection &collection,
        const char *del)
{
    char *literal = strdup(str.c_str());

    for (const char *cstr = strtok(literal, del); cstr != NULL; cstr = strtok(NULL, del)) {
        typename Traits::Type value;
        if (fromString(cstr, value)) {
            Traits::add(collection, value);
        }
    }
    free(literal);
}

template <class Traits>
inline uint32_t TypeConverter<Traits>::maskFromString(const std::string &str, const char *del)
{
    char *literal = strdup(str.c_str());
    uint32_t value = 0;
    for (const char *cstr = strtok(literal, del); cstr != NULL; cstr = strtok(NULL, del)) {
        typename Traits::Type type;
        if (fromString(cstr, type)) {
            value |= static_cast<uint32_t>(type);
        }
    }
    free(literal);
    return value;
}

template <class Traits>
inline void TypeConverter<Traits>::maskToString(uint32_t mask, std::string &str, const char *del)
{
    if (mask != 0) {
        bool first_flag = true;
        for (size_t i = 0; mTable[i].literal; i++) {
            uint32_t value = static_cast<uint32_t>(mTable[i].value);
            if (mTable[i].value != 0 && ((mask & value) == value)) {
                if (!first_flag) str += del;
                first_flag = false;
                str += mTable[i].literal;
            }
        }
    } else {
        toString(static_cast<typename Traits::Type>(0), str);
    }
}

typedef TypeConverter<OutputDeviceTraits> OutputDeviceConverter;
typedef TypeConverter<InputDeviceTraits> InputDeviceConverter;
typedef TypeConverter<OutputFlagTraits> OutputFlagConverter;
typedef TypeConverter<InputFlagTraits> InputFlagConverter;
typedef TypeConverter<FormatTraits> FormatConverter;
typedef TypeConverter<OutputChannelTraits> OutputChannelConverter;
typedef TypeConverter<InputChannelTraits> InputChannelConverter;
typedef TypeConverter<ChannelIndexTraits> ChannelIndexConverter;
typedef TypeConverter<GainModeTraits> GainModeConverter;
typedef TypeConverter<StreamTraits> StreamTypeConverter;
typedef TypeConverter<AudioModeTraits> AudioModeConverter;
typedef TypeConverter<AudioContentTraits> AudioContentTypeConverter;
typedef TypeConverter<UsageTraits> UsageTypeConverter;
typedef TypeConverter<SourceTraits> SourceTypeConverter;
typedef TypeConverter<AudioFlagTraits> AudioFlagConverter;

template<> const OutputDeviceConverter::Table OutputDeviceConverter::mTable[];
template<> const InputDeviceConverter::Table InputDeviceConverter::mTable[];
template<> const OutputFlagConverter::Table OutputFlagConverter::mTable[];
template<> const InputFlagConverter::Table InputFlagConverter::mTable[];
template<> const FormatConverter::Table FormatConverter::mTable[];
template<> const OutputChannelConverter::Table OutputChannelConverter::mTable[];
template<> const InputChannelConverter::Table InputChannelConverter::mTable[];
template<> const ChannelIndexConverter::Table ChannelIndexConverter::mTable[];
template<> const GainModeConverter::Table GainModeConverter::mTable[];
template<> const StreamTypeConverter::Table StreamTypeConverter::mTable[];
template<> const AudioModeConverter::Table AudioModeConverter::mTable[];
template<> const AudioContentTypeConverter::Table AudioContentTypeConverter::mTable[];
template<> const UsageTypeConverter::Table UsageTypeConverter::mTable[];
template<> const SourceTypeConverter::Table SourceTypeConverter::mTable[];
template<> const AudioFlagConverter::Table AudioFlagConverter::mTable[];

bool deviceFromString(const std::string& literalDevice, audio_devices_t& device);

SampleRateTraits::Collection samplingRatesFromString(
        const std::string &samplingRates, const char *del = AudioParameter::valueListSeparator);

FormatTraits::Collection formatsFromString(
        const std::string &formats, const char *del = AudioParameter::valueListSeparator);

audio_format_t formatFromString(
        const std::string &literalFormat, audio_format_t defaultFormat = AUDIO_FORMAT_DEFAULT);

audio_channel_mask_t channelMaskFromString(const std::string &literalChannels);

ChannelTraits::Collection channelMasksFromString(
        const std::string &channels, const char *del = AudioParameter::valueListSeparator);

InputChannelTraits::Collection inputChannelMasksFromString(
        const std::string &inChannels, const char *del = AudioParameter::valueListSeparator);

OutputChannelTraits::Collection outputChannelMasksFromString(
        const std::string &outChannels, const char *del = AudioParameter::valueListSeparator);

// counting enumerations
template <typename T, std::enable_if_t<std::is_same<T, audio_content_type_t>::value
                                    || std::is_same<T, audio_mode_t>::value
                                    || std::is_same<T, audio_source_t>::value
                                    || std::is_same<T, audio_stream_type_t>::value
                                    || std::is_same<T, audio_usage_t>::value
                                    , int> = 0>
static inline std::string toString(const T& value)
{
    std::string result;
    return TypeConverter<DefaultTraits<T>>::toString(value, result)
            ? result : std::to_string(static_cast<int>(value));

}

// flag enumerations
template <typename T, std::enable_if_t<std::is_same<T, audio_gain_mode_t>::value
                                    || std::is_same<T, audio_input_flags_t>::value
                                    || std::is_same<T, audio_output_flags_t>::value
                                    , int> = 0>
static inline std::string toString(const T& value)
{
    std::string result;
    TypeConverter<DefaultTraits<T>>::maskToString(value, result);
    return result;
}

static inline std::string toString(const audio_devices_t& devices)
{
    std::string result;
    if ((devices & AUDIO_DEVICE_BIT_IN) != 0) {
        InputDeviceConverter::maskToString(devices, result);
    } else {
        OutputDeviceConverter::maskToString(devices, result);
    }
    return result;
}

// TODO: Remove when FormatTraits uses DefaultTraits.
static inline std::string toString(const audio_format_t& format)
{
    std::string result;
    return TypeConverter<VectorTraits<audio_format_t>>::toString(format, result)
            ? result : std::to_string(static_cast<int>(format));
}

static inline std::string toString(const audio_attributes_t& attributes)
{
    std::ostringstream result;
    result << "{ Content type: " << toString(attributes.content_type)
           << " Usage: " << toString(attributes.usage)
           << " Source: " << toString(attributes.source)
           << std::hex << " Flags: 0x" << attributes.flags
           << std::dec << " Tags: " << attributes.tags
           << " }";

    return result.str();
}

}; // namespace android

#endif  /*ANDROID_TYPE_CONVERTER_H_*/
