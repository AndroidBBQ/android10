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

#pragma once

#include <limits>
#include <sstream>
#include <string>
#include <vector>
#include <stdint.h>
#include <cmath>

namespace android
{

namespace utilities
{

/**
 * Convert a given source type to a given destination type.
 *
 * String conversion to T reads the value of the type T in the given string.
 * The function does not allow to have white spaces around the value to parse
 * and tries to parse the whole string, which means that if some bytes were not
 * read in the string, the function fails.
 * Hexadecimal representation (ie. numbers starting with 0x) is supported only
 * for integral types conversions.
 *
 * Numeric conversion to string formats the source value to decimal space.
 *
 * Vector to vector conversion calls convertTo on each element.
 *
 * @tparam srcType source type, default value is string type
 * @tparam dstType destination type
 * @param[in] input The source to convert from.
 * @param[out] result Converted value if success, undefined on failure.
 *
 * @return true if conversion was successful, false otherwise.
 */
template <typename srcType, typename dstType>
static inline bool convertTo(const srcType &input, dstType &result);

/* details namespace is here to hide implementation details to header end user. It
 * is NOT intended to be used outside. */
namespace details
{

/** Helper class to limit instantiation of templates */
template <typename T>
struct ConversionFromStringAllowed;
template <typename T>
struct ConversionToStringAllowed;

/* List of allowed types for conversion */
template <>
struct ConversionFromStringAllowed<bool> {};
template <>
struct ConversionFromStringAllowed<uint64_t> {};
template <>
struct ConversionFromStringAllowed<int64_t> {};
template <>
struct ConversionFromStringAllowed<uint32_t> {};
template <>
struct ConversionFromStringAllowed<int32_t> {};
template <>
struct ConversionFromStringAllowed<uint16_t> {};
template <>
struct ConversionFromStringAllowed<int16_t> {};
template <>
struct ConversionFromStringAllowed<float> {};
template <>
struct ConversionFromStringAllowed<double> {};

template <>
struct ConversionToStringAllowed<int64_t> {};
template <>
struct ConversionToStringAllowed<uint64_t> {};
template <>
struct ConversionToStringAllowed<uint32_t> {};
template <>
struct ConversionToStringAllowed<int32_t> {};
template <>
struct ConversionToStringAllowed<double> {};
template <>
struct ConversionToStringAllowed<float> {};

/**
 * Set the decimal precision to 10 digits.
 * Note that this setting is aligned with Android Audio Parameter
 * policy concerning float storage into string.
 */
static const uint32_t gFloatPrecision = 10;

template <typename T>
static inline bool fromString(const std::string &str, T &result)
{
    /* Check that conversion to that type is allowed.
     * If this fails, this means that this template was not intended to be used
     * with this type, thus that the result is undefined. */
    ConversionFromStringAllowed<T>();

    if (str.find_first_of(std::string("\r\n\t\v ")) != std::string::npos) {
        return false;
    }

    /* Check for a '-' in string. If type is unsigned and a - is found, the
     * parsing fails. This is made necessary because "-1" is read as 65535 for
     * uint16_t, for example */
    if (str.find('-') != std::string::npos
        && !std::numeric_limits<T>::is_signed) {
        return false;
    }

    std::stringstream ss(str);

    /* Sadly, the stream conversion does not handle hexadecimal format, thus
     * check is done manually */
    if (str.substr(0, 2) == "0x") {
        if (std::numeric_limits<T>::is_integer) {
            ss >> std::hex >> result;
        } else {
            /* Conversion undefined for non integers */
            return false;
        }
    } else {
        ss >> result;
    }

    return ss.eof() && !ss.fail() && !ss.bad();
}

template <typename T>
static inline bool toString(const T &value, std::string &str)
{
    /* Check that conversion from that type is allowed.
     * If this fails, this means that this template was not intended to be used
     * with this type, thus that the result is undefined. */
    ConversionToStringAllowed<T>();

    std::stringstream oss;
    oss.precision(gFloatPrecision);
    oss << value;
    str = oss.str();
    return !oss.fail() && !oss.bad();
}

template <typename srcType, typename dstType>
class Converter;

template <typename dstType>
class Converter<std::string, dstType>
{
public:
    static inline bool run(const std::string &str, dstType &result)
    {
        return fromString<dstType>(str, result);
    }
};

template <typename srcType>
class Converter<srcType, std::string>
{
public:
    static inline bool run(const srcType &str, std::string &result)
    {
        return toString<srcType>(str, result);
    }
};

/** Convert a vector by applying convertTo on each element.
 *
 * @tparam SrcElem Type of the src elements.
 * @tparam DstElem Type of the destination elements.
 */
template <typename SrcElem, typename DstElem>
class Converter<std::vector<SrcElem>, std::vector<DstElem> >
{
public:
    typedef const std::vector<SrcElem> Src;
    typedef std::vector<DstElem> Dst;

    static inline bool run(Src &src, Dst &dst)
    {
        typedef typename Src::const_iterator SrcIt;
        dst.clear();
        dst.reserve(src.size());
        for (SrcIt it = src.begin(); it != src.end(); ++it) {
            DstElem dstElem;
            if (not convertTo(*it, dstElem)) {
                return false;
            }
            dst.push_back(dstElem);
        }
        return true;
    }
};

} // namespace details

template <typename srcType, typename dstType>
static inline bool convertTo(const srcType &input, dstType &result)
{
    return details::Converter<srcType, dstType>::run(input, result);
}

/**
 * Specialization for int16_t of convertTo template function.
 *
 * This function follows the same paradigm than it's generic version.
 *
 * The specific implementation is made necessary because the stlport version of
 * string streams is bugged and does not fail when giving overflowed values.
 * This specialisation can be safely removed when stlport behaviour is fixed.
 *
 * @param[in]  str    the string to parse.
 * @param[out] result reference to object where to store the result.
 *
 * @return true if conversion was successful, false otherwise.
 */
template <>
inline bool convertTo<std::string, int16_t>(const std::string &str, int16_t &result)
{
    int64_t res;

    if (!convertTo<std::string, int64_t>(str, res)) {
        return false;
    }

    if (res > std::numeric_limits<int16_t>::max() || res < std::numeric_limits<int16_t>::min()) {
        return false;
    }

    result = static_cast<int16_t>(res);
    return true;
}

/**
 * Specialization for float of convertTo template function.
 *
 * This function follows the same paradigm than it's generic version and is
 * based on it but makes furthers checks on the returned value.
 *
 * The specific implementation is made necessary because the stlport conversion
 * from string to float behaves differently than GNU STL: overflow produce
 * +/-Infinity rather than an error.
 *
 * @param[in]  str    the string to parse.
 * @param[out] result reference to object where to store the result.
 *
 * @return true if conversion was successful, false otherwise.
 */
template <>
inline bool convertTo<std::string, float>(const std::string &str, float &result)
{
    if (!details::Converter<std::string, float>::run(str, result)) {
        return false;
    }

    if (std::abs(result) == std::numeric_limits<float>::infinity() ||
        result == std::numeric_limits<float>::quiet_NaN()) {
        return false;
    }

    return true;
}

/**
 * Specialization for double of convertTo template function.
 *
 * This function follows the same paradigm than it's generic version and is
 * based on it but makes furthers checks on the returned value.
 *
 * The specific implementation is made necessary because the stlport conversion
 * from string to double behaves differently than GNU STL: overflow produce
 * +/-Infinity rather than an error.
 *
 * @param[in]  str    the string to parse.
 * @param[out] result reference to object where to store the result.
 *
 * @return true if conversion was successful, false otherwise.
 */
template <>
inline bool convertTo<std::string, double>(const std::string &str, double &result)
{
    if (!details::Converter<std::string, double>::run(str, result)) {
        return false;
    }

    if (std::abs(result) == std::numeric_limits<double>::infinity() ||
        result == std::numeric_limits<double>::quiet_NaN()) {
        return false;
    }

    return true;
}

/**
 * Specialization for boolean of convertTo template function.
 *
 * This function follows the same paradigm than it's generic version.
 * This function accepts to parse boolean as "0/1" or "false/true" or
 * "FALSE/TRUE".
 * The specific implementation is made necessary because the behaviour of
 * string streams when parsing boolean values is not sufficient to fit our
 * requirements. Indeed, parsing "true" will correctly parse the value, but the
 * end of stream is not reached which makes the ss.eof() fails in the generic
 * implementation.
 *
 * @param[in]  str    the string to parse.
 * @param[out] result reference to object where to store the result.
 *
 * @return true if conversion was successful, false otherwise.
 */
template <>
inline bool convertTo<std::string, bool>(const std::string &str, bool &result)
{
    if (str == "0" || str == "FALSE" || str == "false") {
        result = false;
        return true;
    }

    if (str == "1" || str == "TRUE" || str == "true") {
        result = true;
        return true;
    }

    return false;
}

/**
 * Specialization for boolean to string of convertTo template function.
 *
 * This function follows the same paradigm than it's generic version.
 * This function arbitrarily decides to return "false/true".
 * It is compatible with the specialization from string to boolean.
 *
 * @param[in]  isSet  boolean to convert to a string.
 * @param[out] result reference to object where to store the result.
 *
 * @return true if conversion was successful, false otherwise.
 */
template <>
inline bool convertTo<bool, std::string>(const bool &isSet, std::string &result)
{
    result = isSet ? "true" : "false";
    return true;
}

/**
 * Specialization for string to string of convertTo template function.
 *
 * This function is a dummy conversion from string to string.
 * In case of clients using template as well, this implementation avoids adding extra
 * specialization to bypass the conversion from string to string.
 *
 * @param[in]  str    the string to parse.
 * @param[out] result reference to object where to store the result.
 *
 * @return true if conversion was successful, false otherwise.
 */
template <>
inline bool convertTo<std::string, std::string>(const std::string &str, std::string &result)
{
    result = str;
    return true;
}

} // namespace utilities

} // namespace android
