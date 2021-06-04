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

#ifndef STAGEFRIGHT_FOUNDATION_TYPE_TRAITS_H_
#define STAGEFRIGHT_FOUNDATION_TYPE_TRAITS_H_

#include <type_traits>

#undef HIDE
#define HIDE __attribute__((visibility("hidden")))

namespace android {

/**
 * std::is_signed, is_unsigned and is_integral does not consider enums even though the standard
 * considers them integral. Create modified versions of these here. Also create a wrapper around
 * std::underlying_type that does not require checking if the type is an enum.
 */

/**
 * Type support utility class to check if a type is an integral type or an enum.
 */
template<typename T>
struct HIDE is_integral_or_enum
    : std::integral_constant<bool, std::is_integral<T>::value || std::is_enum<T>::value> { };

/**
 * Type support utility class to get the underlying std::is_integral supported type for a type.
 * This returns the underlying type for enums, and the same type for types covered by
 * std::is_integral.
 *
 * This is also used as a conditional to return an alternate type if the template param is not
 * an integral or enum type (as in underlying_integral_type<T, TypeIfNotEnumOrIntegral>::type).
 */
template<typename T,
        typename U=typename std::enable_if<is_integral_or_enum<T>::value>::type,
        bool=std::is_enum<T>::value,
        bool=std::is_integral<T>::value>
struct HIDE underlying_integral_type {
    static_assert(!std::is_enum<T>::value, "T should not be enum here");
    static_assert(!std::is_integral<T>::value, "T should not be integral here");
    typedef U type;
};

/** Specialization for enums. */
template<typename T, typename U>
struct HIDE underlying_integral_type<T, U, true, false> {
    static_assert(std::is_enum<T>::value, "T should be enum here");
    static_assert(!std::is_integral<T>::value, "T should not be integral here");
    typedef typename std::underlying_type<T>::type type;
};

/** Specialization for non-enum std-integral types. */
template<typename T, typename U>
struct HIDE underlying_integral_type<T, U, false, true> {
    static_assert(!std::is_enum<T>::value, "T should not be enum here");
    static_assert(std::is_integral<T>::value, "T should be integral here");
    typedef T type;
};

/**
 * Type support utility class to check if the underlying integral type is signed.
 */
template<typename T>
struct HIDE is_signed_integral
    : std::integral_constant<bool, std::is_signed<
            typename underlying_integral_type<T, unsigned>::type>::value> { };

/**
 * Type support utility class to check if the underlying integral type is unsigned.
 */
template<typename T>
struct HIDE is_unsigned_integral
    : std::integral_constant<bool, std::is_unsigned<
            typename underlying_integral_type<T, signed>::type>::value> {
};

/**
 * Type support relationship query template.
 *
 * If T occurs as one of the types in Us with the same const-volatile qualifications, provides the
 * member constant |value| equal to true. Otherwise value is false.
 */
template<typename T, typename ...Us>
struct HIDE is_one_of;

/// \if 0
/**
 * Template specialization when first type matches the searched type.
 */
template<typename T, typename ...Us>
struct HIDE is_one_of<T, T, Us...> : std::true_type {};

/**
 * Template specialization when first type does not match the searched type.
 */
template<typename T, typename U, typename ...Us>
struct HIDE is_one_of<T, U, Us...> : is_one_of<T, Us...> {};

/**
 * Template specialization when there are no types to search.
 */
template<typename T>
struct HIDE is_one_of<T> : std::false_type {};
/// \endif

/**
 * Type support relationship query template.
 *
 * If all types in Us are unique, provides the member constant |value| equal to true.
 * Otherwise value is false.
 */
template<typename ...Us>
struct HIDE are_unique;

/// \if 0
/**
 * Template specialization when there are no types.
 */
template<>
struct HIDE are_unique<> : std::true_type {};

/**
 * Template specialization when there is at least one type to check.
 */
template<typename T, typename ...Us>
struct HIDE are_unique<T, Us...>
    : std::integral_constant<bool, are_unique<Us...>::value && !is_one_of<T, Us...>::value> {};
/// \endif

/// \if 0
template<size_t Base, typename T, typename ...Us>
struct HIDE _find_first_impl;

/**
 * Template specialization when there are no types to search.
 */
template<size_t Base, typename T>
struct HIDE _find_first_impl<Base, T> : std::integral_constant<size_t, 0> {};

/**
 * Template specialization when T is the first type in Us.
 */
template<size_t Base, typename T, typename ...Us>
struct HIDE _find_first_impl<Base, T, T, Us...> : std::integral_constant<size_t, Base> {};

/**
 * Template specialization when T is not the first type in Us.
 */
template<size_t Base, typename T, typename U, typename ...Us>
struct HIDE _find_first_impl<Base, T, U, Us...>
    : std::integral_constant<size_t, _find_first_impl<Base + 1, T, Us...>::value> {};

/// \endif

/**
 * Type support relationship query template.
 *
 * If T occurs in Us, index is the 1-based left-most index of T in Us. Otherwise, index is 0.
 */
template<typename T, typename ...Us>
struct HIDE find_first {
    static constexpr size_t index = _find_first_impl<1, T, Us...>::value;
};

/// \if 0
/**
 * Helper class for find_first_convertible_to template.
 *
 * Adds a base index.
 */
template<size_t Base, typename T, typename ...Us>
struct HIDE _find_first_convertible_to_helper;

/**
 * Template specialization for when there are more types to consider
 */
template<size_t Base, typename T, typename U, typename ...Us>
struct HIDE _find_first_convertible_to_helper<Base, T, U, Us...> {
    static constexpr size_t index =
        std::is_convertible<T, U>::value ? Base :
                _find_first_convertible_to_helper<Base + 1, T, Us...>::index;
    typedef typename std::conditional<
        std::is_convertible<T, U>::value, U,
        typename _find_first_convertible_to_helper<Base + 1, T, Us...>::type>::type type;
};

/**
 * Template specialization for when there are no more types to consider
 */
template<size_t Base, typename T>
struct HIDE _find_first_convertible_to_helper<Base, T> {
    static constexpr size_t index = 0;
    typedef void type;
};

/// \endif

/**
 * Type support template that returns the type that T can be implicitly converted into, and its
 * index, from a list of other types (Us).
 *
 * Returns index of 0 and type of void if there are no convertible types.
 *
 * \param T type that is converted
 * \param Us types into which the conversion is considered
 */
template<typename T, typename ...Us>
struct HIDE find_first_convertible_to : public _find_first_convertible_to_helper<1, T, Us...> { };

}  // namespace android

#endif  // STAGEFRIGHT_FOUNDATION_TYPE_TRAITS_H_

