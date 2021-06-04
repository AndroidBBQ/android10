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

#ifndef C2ENUM_H_
#define C2ENUM_H_

#include <C2Param.h>
#include <_C2MacroUtils.h>

#include <utility>
#include <vector>

/** \file
 * Tools for easier enum support.
 */

/// \cond INTERNAL

/* ---------------------------- UTILITIES FOR ENUMERATION REFLECTION ---------------------------- */

/**
 * Utility class that allows ignoring enum value assignment (e.g. both '(_C2EnumConst)kValue = x'
 * and '(_C2EnumConst)kValue' will eval to kValue.
 */
template<typename T>
class _C2EnumConst {
public:
    // implicit conversion from T
    inline _C2EnumConst(T value) : _mValue(value) {}
    // implicit conversion to T
    inline operator T() { return _mValue; }
    // implicit conversion to C2Value::Primitive
    inline operator C2Value::Primitive() { return (T)_mValue; }
    // ignore assignment and return T here to avoid implicit conversion to T later
    inline T &operator =(T value __unused) { return _mValue; }
private:
    T _mValue;
};

/// mapper to get name of enum
/// \note this will contain any initialization, which we will remove when converting to lower-case
#define _C2_GET_ENUM_NAME(x, y) #x
/// mapper to get value of enum
#define _C2_GET_ENUM_VALUE(x, type) (_C2EnumConst<type>)x

/// \endcond

class _C2EnumUtils {
    static C2String camelCaseToDashed(C2String name);

    static std::vector<C2String> sanitizeEnumValueNames(
            const std::vector<C2StringLiteral> names,
            C2StringLiteral _prefix = nullptr);

    friend class C2UtilTest_EnumUtilsTest_Test;

public:
    // this may not be used...
    static C2_HIDE std::vector<C2String> parseEnumValuesFromString(C2StringLiteral value);

    template<typename T>
    static C2_HIDE C2FieldDescriptor::NamedValuesType sanitizeEnumValues(
            std::vector<T> values,
            std::vector<C2StringLiteral> names,
            C2StringLiteral prefix = nullptr) {
        C2FieldDescriptor::NamedValuesType namedValues;
        std::vector<C2String> sanitizedNames = sanitizeEnumValueNames(names, prefix);
        for (size_t i = 0; i < values.size() && i < sanitizedNames.size(); ++i) {
            namedValues.emplace_back(sanitizedNames[i], values[i]);
        }
        return namedValues;
    }

    template<typename E>
    static C2_HIDE C2FieldDescriptor::NamedValuesType customEnumValues(
            std::vector<std::pair<C2StringLiteral, E>> items) {
        C2FieldDescriptor::NamedValuesType namedValues;
        for (auto &item : items) {
            namedValues.emplace_back(item.first, item.second);
        }
        return namedValues;
    }
};

#define DEFINE_C2_ENUM_VALUE_AUTO_HELPER(name, type, prefix, ...) \
    _DEFINE_C2_ENUM_VALUE_AUTO_HELPER(__C2_GENERATE_GLOBAL_VARS__, name, type, prefix, \
            ##__VA_ARGS__)
#define _DEFINE_C2_ENUM_VALUE_AUTO_HELPER(enabled, name, type, prefix, ...) \
    __DEFINE_C2_ENUM_VALUE_AUTO_HELPER(enabled, name, type, prefix, ##__VA_ARGS__)
#define __DEFINE_C2_ENUM_VALUE_AUTO_HELPER(enabled, name, type, prefix, ...) \
    ___DEFINE_C2_ENUM_VALUE_AUTO_HELPER##enabled(name, type, prefix, ##__VA_ARGS__)
#define ___DEFINE_C2_ENUM_VALUE_AUTO_HELPER(name, type, prefix, ...) \
template<> \
C2FieldDescriptor::NamedValuesType C2FieldDescriptor::namedValuesFor(const name &r __unused) { \
    return _C2EnumUtils::sanitizeEnumValues( \
            std::vector<C2Value::Primitive> { _C2_MAP(_C2_GET_ENUM_VALUE, type, __VA_ARGS__) }, \
            { _C2_MAP(_C2_GET_ENUM_NAME, type, __VA_ARGS__) }, \
            prefix); \
}
#define ___DEFINE_C2_ENUM_VALUE_AUTO_HELPER__C2_GENERATE_GLOBAL_VARS__(name, type, prefix, ...)

#define DEFINE_C2_ENUM_VALUE_CUSTOM_HELPER(name, names) \
    _DEFINE_C2_ENUM_VALUE_CUSTOM_HELPER(__C2_GENERATE_GLOBAL_VARS__, name, names)
#define _DEFINE_C2_ENUM_VALUE_CUSTOM_HELPER(enabled, name, names) \
    __DEFINE_C2_ENUM_VALUE_CUSTOM_HELPER(enabled, name, names)
#define __DEFINE_C2_ENUM_VALUE_CUSTOM_HELPER(enabled, name, names) \
    ___DEFINE_C2_ENUM_VALUE_CUSTOM_HELPER##enabled(name, names)
#define ___DEFINE_C2_ENUM_VALUE_CUSTOM_HELPER(name, names) \
template<> \
C2FieldDescriptor::NamedValuesType C2FieldDescriptor::namedValuesFor(const name &r __unused) { \
    return _C2EnumUtils::customEnumValues( \
            std::vector<std::pair<C2StringLiteral, name>> names); \
}
#define ___DEFINE_C2_ENUM_VALUE_CUSTOM_HELPER__C2_GENERATE_GLOBAL_VARS__(name, names)

/**
 * Defines an enum type with the default named value mapper. The default mapper
 * finds and removes the longest common prefix across all of the enum value names, and
 * replaces camel-case separators with dashes ('-').
 *
 * This macro must be used in the global scope and namespace.
 *
 *  ~~~~~~~~~~~~~ (.cpp)
 *  C2ENUM(c2_enum_t, uint32_t,
 *    C2_VALUE1,
 *    C2_VALUE2 = 5,
 *    C2_VALUE3 = C2_VALUE1 + 1)
 *  // named values are: C2_VALUE1 => "1", C2_VALUE2 => "2", ...
 *  // longest common prefix is "C2_VALUE"
 *  ~~~~~~~~~~~~~
 *
 * \param name name of the enum type (This can be an inner class enum.)
 * \param type underlying type
 */
#define C2ENUM(name, type, ...) \
enum name : type { __VA_ARGS__ }; \
DEFINE_C2_ENUM_VALUE_AUTO_HELPER(name, type, nullptr, __VA_ARGS__)

/**
 * Defines an enum type with the default named value mapper but custom prefix. The default
 * mapper removes the prefix from all of the enum value names (if present), and
 * inserts dashes at camel-case separators (lowHigh becomes low-high) and also replaces
 * non-leading underscores with dashes ('-').
 *
 * This macro must be used in the global scope and namespace.
 *
 *  ~~~~~~~~~~~~~ (.cpp)
 *  C2ENUM_CUSTOM_PREFIX(c2_enum_t, uint32_t, "C2_",
 *    C2_VALUE1,
 *    C2_VALUE2 = 5,
 *    C2_VALUE3 = C2_VALUE1 + 1)
 *  // named values are: C2_VALUE1 => "VALUE1", C2_VALUE2 => "VALUE2", ...
 *  ~~~~~~~~~~~~~
 *
 * \param name name of the enum type (This can be an inner class enum.)
 * \param type underlying type
 * \param prefix prefix to remove
 */
#define C2ENUM_CUSTOM_PREFIX(name, type, prefix, ...) \
enum name : type { __VA_ARGS__ }; \
DEFINE_C2_ENUM_VALUE_AUTO_HELPER(name, type, prefix, __VA_ARGS__)

/**
 * Defines an enum type with custom names.
 *
 * This macro must be used in the global scope and namespace.
 *
 *  ~~~~~~~~~~~~~ (.cpp)
 *  C2ENUM_CUSTOM_NAMES(SomeStruct::c2_enum_t, uint32_t, ({
 *      { "One", SomeStruct::C2_VALUE1 },
 *      { "Two", SomeStruct::C2_VALUE2 },
 *      { "Three", SomeStruct::C2_VALUE3 } }),
 *    C2_VALUE1,
 *    C2_VALUE2 = 5,
 *    C2_VALUE3 = C2_VALUE1 + 1)
 *
 *  // named values are: C2_VALUE1 => "One", C2_VALUE2 => "Two", ...
 *  ~~~~~~~~~~~~~
 */
#define C2ENUM_CUSTOM_NAMES(name, type, names, ...) \
enum name : type { __VA_ARGS__ }; \
DEFINE_C2_ENUM_VALUE_CUSTOM_HELPER(name, names)

/**
 * Make enums usable as their integral types.
 *
 * Note: this makes them not usable in printf()
 */
template<class E>
struct C2EasyEnum {
    using U = typename std::underlying_type<E>::type;
    E value;
    // define conversion functions
    inline constexpr operator E() const { return value; }
    inline constexpr C2EasyEnum(E value_) : value(value_) { }
    inline constexpr C2EasyEnum(U value_) : value(E(value_)) { }
    inline constexpr C2EasyEnum() = default;
};

// make C2EasyEnum behave like a regular enum

namespace std {
    template<typename E>
    struct underlying_type<C2EasyEnum<E>> {
        typedef typename underlying_type<E>::type type;
    };

    template<typename E>
    struct is_enum<C2EasyEnum<E>> {
        constexpr static bool value = true;
    };
}

#endif  // C2ENUM_H_

