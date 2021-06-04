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

#ifndef C2_H_
#define C2_H_

#include <errno.h>

#include <string>

/** nanoseconds with arbitrary origin. */
typedef int64_t c2_nsecs_t;

/** \mainpage Codec2
 *
 * Codec2 is a generic frame-based data processing API.
 *
 * The media subsystem accesses components via the \ref API.
 */

/** \ingroup API
 *
 * The Codec2 API defines the operation of data processing components and their interaction with
 * the rest of the system.
 *
 * Coding Conventions
 *
 * Mitigating Binary Compatibility.
 *
 * While full binary compatibility is not a goal of the API (due to our use of STL), we try to
 * mitigate binary breaks by adhering to the following conventions:
 *
 * - at most one vtable with placeholder virtual methods
 * - all optional/placeholder virtual methods returning a c2_status_t, with C2_OMITTED not requiring
 *   any update to input/output arguments.
 * - limiting symbol export of inline methods
 * - use of pimpl (or shared-pimpl)
 *
 * Naming
 *
 * - all classes and types prefix with C2
 * - classes for internal use prefix with _C2
 * - enum values in global namespace prefix with C2_ all caps
 * - enum values inside classes have no C2_ prefix as class already has it
 * - supporting two kinds of enum naming: all-caps and kCamelCase
 * \todo revisit kCamelCase for param-type
 *
 * Aspects
 *
 * Aspects define certain common behavior across a group of objects.
 * - classes whose name matches _C2.*Aspect
 * - only protected constructors
 * - no desctructor and copiable
 * - all methods are inline or static (this is opposite of the interface paradigm where all methods
 *   are virtual, which would not work due to the at most one vtable rule.)
 * - only private variables (this prevents subclasses interfering with the aspects.)
 */

/// \defgroup types Common Types
/// @{

/**
 * C2String: basic string implementation
 */
typedef std::string C2String;

/**
 * C2StringLiteral: basic string literal implementation.
 * \note these are never owned by any object, and can only refer to C string literals.
 */
typedef const char *C2StringLiteral;

/**
 * c2_status_t: status codes used.
 */
enum c2_status_t : int32_t {
/*
 * Use POSIX errno constants.
 */
    C2_OK        = 0,            ///< operation completed successfully

    // bad input
    C2_BAD_VALUE = EINVAL,       ///< argument has invalid value (user error)
    C2_BAD_INDEX = ENXIO,        ///< argument uses invalid index (user error)
    C2_CANNOT_DO = ENOTSUP,      ///< argument/index is valid but not possible

    // bad sequencing of events
    C2_DUPLICATE = EEXIST,       ///< object already exists
    C2_NOT_FOUND = ENOENT,       ///< object not found
    C2_BAD_STATE = EPERM,        ///< operation is not permitted in the current state
    C2_BLOCKING  = EWOULDBLOCK,  ///< operation would block but blocking is not permitted
    C2_CANCELED  = EINTR,        ///< operation interrupted/canceled

    // bad environment
    C2_NO_MEMORY = ENOMEM,       ///< not enough memory to complete operation
    C2_REFUSED   = EACCES,       ///< missing permission to complete operation

    C2_TIMED_OUT = ETIMEDOUT,    ///< operation did not complete within timeout

    // bad versioning
    C2_OMITTED   = ENOSYS,       ///< operation is not implemented/supported (optional only)

    // unknown fatal
    C2_CORRUPTED = EFAULT,       ///< some unexpected error prevented the operation
    C2_NO_INIT   = ENODEV,       ///< status has not been initialized
};

/**
 * Type that describes the desired blocking behavior for variable blocking calls. Blocking in this
 * API is used in a somewhat modified meaning such that operations that merely update variables
 * protected by mutexes are still considered "non-blocking" (always used in quotes).
 */
enum c2_blocking_t : int32_t {
    /**
     * The operation SHALL be "non-blocking". This means that it shall not perform any file
     * operations, or call/wait on other processes. It may use a protected region as long as the
     * mutex is never used to protect code that is otherwise "may block".
     */
    C2_DONT_BLOCK = false,
    /**
     * The operation MAY be temporarily blocking.
     */
    C2_MAY_BLOCK = true,
};

/// @}

/// \defgroup utils Utilities
/// @{

#define C2_DO_NOT_COPY(type) \
    type& operator=(const type &) = delete; \
    type(const type &) = delete; \

#define C2_DEFAULT_MOVE(type) \
    type& operator=(type &&) = default; \
    type(type &&) = default; \

#define C2_ALLOW_OVERFLOW __attribute__((no_sanitize("integer")))
#define C2_CONST    __attribute__((const))
#define C2_HIDE     __attribute__((visibility("hidden")))
#define C2_INLINE   inline C2_HIDE
#define C2_INTERNAL __attribute__((internal_linkage))
#define C2_PACK     __attribute__((aligned(4)))
#define C2_PURE     __attribute__((pure))

#define DEFINE_OTHER_COMPARISON_OPERATORS(type) \
    inline bool operator!=(const type &other) const { return !(*this == other); } \
    inline bool operator<=(const type &other) const { return (*this == other) || (*this < other); } \
    inline bool operator>=(const type &other) const { return !(*this < other); } \
    inline bool operator>(const type &other) const { return !(*this < other) && !(*this == other); }

#define DEFINE_FIELD_BASED_COMPARISON_OPERATORS(type, field) \
    inline bool operator<(const type &other) const { return field < other.field; } \
    inline bool operator==(const type &other) const { return field == other.field; } \
    DEFINE_OTHER_COMPARISON_OPERATORS(type)

#define DEFINE_FIELD_AND_MASK_BASED_COMPARISON_OPERATORS(type, field, mask) \
    inline bool operator<(const type &other) const { \
        return (field & mask) < (other.field & (mask)); \
    } \
    inline bool operator==(const type &other) const { \
        return (field & mask) == (other.field & (mask)); \
    } \
    DEFINE_OTHER_COMPARISON_OPERATORS(type)

#define DEFINE_ENUM_OPERATORS(etype) \
    inline constexpr etype operator|(etype a, etype b) { return (etype)(std::underlying_type<etype>::type(a) | std::underlying_type<etype>::type(b)); } \
    inline constexpr etype &operator|=(etype &a, etype b) { a = (etype)(std::underlying_type<etype>::type(a) | std::underlying_type<etype>::type(b)); return a; } \
    inline constexpr etype operator&(etype a, etype b) { return (etype)(std::underlying_type<etype>::type(a) & std::underlying_type<etype>::type(b)); } \
    inline constexpr etype &operator&=(etype &a, etype b) { a = (etype)(std::underlying_type<etype>::type(a) & std::underlying_type<etype>::type(b)); return a; } \
    inline constexpr etype operator^(etype a, etype b) { return (etype)(std::underlying_type<etype>::type(a) ^ std::underlying_type<etype>::type(b)); } \
    inline constexpr etype &operator^=(etype &a, etype b) { a = (etype)(std::underlying_type<etype>::type(a) ^ std::underlying_type<etype>::type(b)); return a; } \
    inline constexpr etype operator~(etype a) { return (etype)(~std::underlying_type<etype>::type(a)); }

template<typename T, typename B>
class C2_HIDE c2_cntr_t;

/// \cond INTERNAL

/// \defgroup utils_internal
/// @{

template<typename T>
struct C2_HIDE _c2_cntr_compat_helper {
    template<typename U, typename E=typename std::enable_if<std::is_integral<U>::value>::type>
    C2_ALLOW_OVERFLOW
    inline static constexpr T get(const U &value) {
        return T(value);
    }

    template<typename U, typename E=typename std::enable_if<(sizeof(U) >= sizeof(T))>::type>
    C2_ALLOW_OVERFLOW
    inline static constexpr T get(const c2_cntr_t<U, void> &value) {
        return T(value.mValue);
    }
};

/// @}

/// \endcond

/**
 * Integral counter type.
 *
 * This is basically an unsigned integral type that is NEVER checked for overflow/underflow - and
 * comparison operators are redefined.
 *
 * \note Comparison of counter types is not fully transitive, e.g.
 * it could be that a > b > c but a !> c.
 * std::less<>, greater<>, less_equal<> and greater_equal<> specializations yield total ordering,
 * but may not match semantic ordering of the values.
 *
 * Technically: counter types represent integer values: A * 2^N + value, where A can be arbitrary.
 * This makes addition, subtraction, multiplication (as well as bitwise operations) well defined.
 * However, division is in general not well defined, as the result may depend on A. This is also
 * true for logical operators and boolean conversion.
 *
 * Even though well defined, bitwise operators are not implemented for counter types as they are not
 * meaningful.
 */
template<typename T, typename B=typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value>::type>
class C2_HIDE c2_cntr_t {
    using compat = _c2_cntr_compat_helper<T>;

    T mValue;
    constexpr static T HALF_RANGE = T(~0) ^ (T(~0) >> 1);

    template<typename U>
    friend struct _c2_cntr_compat_helper;
public:

    /**
     * Default constructor. Initialized counter to 0.
     */
    inline constexpr c2_cntr_t() : mValue(T(0)) {}

    /**
     * Construct from a compatible type.
     */
    template<typename U>
    inline constexpr c2_cntr_t(const U &value) : mValue(compat::get(value)) {}

    /**
     * Peek as underlying signed type.
     */
    C2_ALLOW_OVERFLOW
    inline constexpr typename std::make_signed<T>::type peek() const {
        return static_cast<typename std::make_signed<T>::type>(mValue);
    }

    /**
     * Peek as underlying unsigned type.
     */
    inline constexpr T peeku() const {
        return mValue;
    }

    /**
     * Peek as long long - e.g. for printing.
     */
    C2_ALLOW_OVERFLOW
    inline constexpr long long peekll() const {
        return (long long)mValue;
    }

    /**
     * Peek as unsigned long long - e.g. for printing.
     */
    C2_ALLOW_OVERFLOW
    inline constexpr unsigned long long peekull() const {
        return (unsigned long long)mValue;
    }

    /**
     * Convert to a smaller counter type. This is always safe.
     */
    template<typename U, typename E=typename std::enable_if<(sizeof(U) < sizeof(T))>::type>
    inline constexpr operator c2_cntr_t<U>() {
        return c2_cntr_t<U>(mValue);
    }

    /**
     * Arithmetic operators
     */

#define DEFINE_C2_CNTR_BINARY_OP(attrib, op, op_assign) \
    template<typename U> \
    attrib inline c2_cntr_t<T>& operator op_assign(const U &value) { \
        mValue op_assign compat::get(value); \
        return *this; \
    } \
    \
    template<typename U, typename E=decltype(compat::get(U(0)))> \
    attrib inline constexpr c2_cntr_t<T> operator op(const U &value) const { \
        return c2_cntr_t<T>(mValue op compat::get(value)); \
    } \
    \
    template<typename U, typename E=typename std::enable_if<(sizeof(U) < sizeof(T))>::type> \
    attrib inline constexpr c2_cntr_t<U> operator op(const c2_cntr_t<U> &value) const { \
        return c2_cntr_t<U>(U(mValue) op value.peeku()); \
    }

#define DEFINE_C2_CNTR_UNARY_OP(attrib, op) \
    attrib inline constexpr c2_cntr_t<T> operator op() const { \
        return c2_cntr_t<T>(op mValue); \
    }

#define DEFINE_C2_CNTR_CREMENT_OP(attrib, op) \
    attrib inline c2_cntr_t<T> &operator op() { \
        op mValue; \
        return *this; \
    } \
    attrib inline c2_cntr_t<T> operator op(int) { \
        return c2_cntr_t<T, void>(mValue op); \
    }

    DEFINE_C2_CNTR_BINARY_OP(C2_ALLOW_OVERFLOW, +, +=)
    DEFINE_C2_CNTR_BINARY_OP(C2_ALLOW_OVERFLOW, -, -=)
    DEFINE_C2_CNTR_BINARY_OP(C2_ALLOW_OVERFLOW, *, *=)

    DEFINE_C2_CNTR_UNARY_OP(C2_ALLOW_OVERFLOW, -)
    DEFINE_C2_CNTR_UNARY_OP(C2_ALLOW_OVERFLOW, +)

    DEFINE_C2_CNTR_CREMENT_OP(C2_ALLOW_OVERFLOW, ++)
    DEFINE_C2_CNTR_CREMENT_OP(C2_ALLOW_OVERFLOW, --)

    template<typename U, typename E=typename std::enable_if<std::is_unsigned<U>::value>::type>
    C2_ALLOW_OVERFLOW
    inline constexpr c2_cntr_t<T> operator<<(const U &value) const {
        return c2_cntr_t<T>(mValue << value);
    }

    template<typename U, typename E=typename std::enable_if<std::is_unsigned<U>::value>::type>
    C2_ALLOW_OVERFLOW
    inline c2_cntr_t<T> &operator<<=(const U &value) {
        mValue <<= value;
        return *this;
    }

    /**
     * Comparison operators
     */
    C2_ALLOW_OVERFLOW
    inline constexpr bool operator<=(const c2_cntr_t<T> &other) const {
        return T(other.mValue - mValue) < HALF_RANGE;
    }

    C2_ALLOW_OVERFLOW
    inline constexpr bool operator>=(const c2_cntr_t<T> &other) const {
        return T(mValue - other.mValue) < HALF_RANGE;
    }

    inline constexpr bool operator==(const c2_cntr_t<T> &other) const {
        return mValue == other.mValue;
    }

    inline constexpr bool operator!=(const c2_cntr_t<T> &other) const {
        return !(*this == other);
    }

    inline constexpr bool operator<(const c2_cntr_t<T> &other) const {
        return *this <= other && *this != other;
    }

    inline constexpr bool operator>(const c2_cntr_t<T> &other) const {
        return *this >= other && *this != other;
    }
};

template<typename U, typename T, typename E=typename std::enable_if<std::is_integral<U>::value>::type>
inline constexpr c2_cntr_t<T> operator+(const U &a, const c2_cntr_t<T> &b) {
    return b + a;
}

template<typename U, typename T, typename E=typename std::enable_if<std::is_integral<U>::value>::type>
inline constexpr c2_cntr_t<T> operator-(const U &a, const c2_cntr_t<T> &b) {
    return c2_cntr_t<T>(a) - b;
}

template<typename U, typename T, typename E=typename std::enable_if<std::is_integral<U>::value>::type>
inline constexpr c2_cntr_t<T> operator*(const U &a, const c2_cntr_t<T> &b) {
    return b * a;
}

typedef c2_cntr_t<uint32_t> c2_cntr32_t; /** 32-bit counter type */
typedef c2_cntr_t<uint64_t> c2_cntr64_t; /** 64-bit counter type */

/// \cond INTERNAL

/// \defgroup utils_internal
/// @{

template<typename... T> struct c2_types;

/** specialization for a single type */
template<typename T>
struct c2_types<T> {
    typedef typename std::decay<T>::type wide_type;
    typedef wide_type narrow_type;
    typedef wide_type min_type; // type for min(T...)
};

/** specialization for two types */
template<typename T, typename U>
struct c2_types<T, U> {
    static_assert(std::is_floating_point<T>::value == std::is_floating_point<U>::value,
                  "mixing floating point and non-floating point types is disallowed");
    static_assert(std::is_signed<T>::value == std::is_signed<U>::value,
                  "mixing signed and unsigned types is disallowed");

    typedef typename std::decay<
            decltype(true ? std::declval<T>() : std::declval<U>())>::type wide_type;
    typedef typename std::decay<
            typename std::conditional<sizeof(T) < sizeof(U), T, U>::type>::type narrow_type;
    typedef typename std::conditional<
            std::is_signed<T>::value, wide_type, narrow_type>::type min_type;
};

/// @}

/// \endcond

/**
 * Type support utility class. Only supports similar classes, such as:
 * - all floating point
 * - all unsigned/all signed
 * - all pointer
 */
template<typename T, typename U, typename... V>
struct c2_types<T, U, V...> {
    /** Common type that accommodates all template parameter types. */
    typedef typename c2_types<typename c2_types<T, U>::wide_type, V...>::wide_type wide_type;
    /** Narrowest type of the template parameter types. */
    typedef typename c2_types<typename c2_types<T, U>::narrow_type, V...>::narrow_type narrow_type;
    /** Type that accommodates the minimum value for any input for the template parameter types. */
    typedef typename c2_types<typename c2_types<T, U>::min_type, V...>::min_type min_type;
};

/**
 *  \ingroup utils_internal
 * specialization for two values */
template<typename T, typename U>
inline constexpr typename c2_types<T, U>::wide_type c2_max(const T a, const U b) {
    typedef typename c2_types<T, U>::wide_type wide_type;
    return ({ wide_type a_(a), b_(b); a_ > b_ ? a_ : b_; });
}

/**
 * Finds the maximum value of a list of "similarly typed" values.
 *
 * This is an extension to std::max where the types do not have to be identical, and the smallest
 * resulting type is used that accommodates the argument types.
 *
 * \note Value types must be similar, e.g. all floating point, all pointers, all signed, or all
 * unsigned.
 *
 * @return the largest of the input arguments.
 */
template<typename T, typename U, typename... V>
constexpr typename c2_types<T, U, V...>::wide_type c2_max(const T a, const U b, const V ... c) {
    typedef typename c2_types<T, U, V...>::wide_type wide_type;
    return ({ wide_type a_(a), b_(c2_max(b, c...)); a_ > b_ ? a_ : b_; });
}

/**
 *  \ingroup utils_internal
 * specialization for two values */
template<typename T, typename U>
inline constexpr typename c2_types<T, U>::min_type c2_min(const T a, const U b) {
    typedef typename c2_types<T, U>::wide_type wide_type;
    return ({
        wide_type a_(a), b_(b);
        static_cast<typename c2_types<T, U>::min_type>(a_ < b_ ? a_ : b_);
    });
}

/**
 * Finds the minimum value of a list of "similarly typed" values.
 *
 * This is an extension to std::min where the types do not have to be identical, and the smallest
 * resulting type is used that accommodates the argument types.
 *
 * \note Value types must be similar, e.g. all floating point, all pointers, all signed, or all
 * unsigned.
 *
 * @return the smallest of the input arguments.
 */
template<typename T, typename U, typename... V>
constexpr typename c2_types<T, U, V...>::min_type c2_min(const T a, const U b, const V ... c) {
    typedef typename c2_types<U, V...>::min_type rest_type;
    typedef typename c2_types<T, rest_type>::wide_type wide_type;
    return ({
        wide_type a_(a), b_(c2_min(b, c...));
        static_cast<typename c2_types<T, rest_type>::min_type>(a_ < b_ ? a_ : b_);
    });
}

/**
 *  \ingroup utils_internal
 */
template<typename T, typename U, typename V>
inline constexpr typename c2_types<T, V>::wide_type c2_clamp(const T a, const U b, const V c) {
    typedef typename c2_types<T, U, V>::wide_type wide_type;
    return ({
        wide_type a_(a), b_(b), c_(c);
        static_cast<typename c2_types<T, V>::wide_type>(b_ < a_ ? a_ : b_ > c_ ? c_ : b_);
    });
}

/// @}

#include <functional>
template<typename T>
struct std::less<::c2_cntr_t<T>> {
    constexpr bool operator()(const ::c2_cntr_t<T> &lh, const ::c2_cntr_t<T> &rh) const {
        return lh.peeku() < rh.peeku();
    }
};
template<typename T>
struct std::less_equal<::c2_cntr_t<T>> {
    constexpr bool operator()(const ::c2_cntr_t<T> &lh, const ::c2_cntr_t<T> &rh) const {
        return lh.peeku() <= rh.peeku();
    }
};
template<typename T>
struct std::greater<::c2_cntr_t<T>> {
    constexpr bool operator()(const ::c2_cntr_t<T> &lh, const ::c2_cntr_t<T> &rh) const {
        return lh.peeku() > rh.peeku();
    }
};
template<typename T>
struct std::greater_equal<::c2_cntr_t<T>> {
    constexpr bool operator()(const ::c2_cntr_t<T> &lh, const ::c2_cntr_t<T> &rh) const {
        return lh.peeku() >= rh.peeku();
    }
};

#endif  // C2_H_
