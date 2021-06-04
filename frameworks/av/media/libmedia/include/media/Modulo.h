/*
 * Copyright 2015 The Android Open Source Project
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

#ifndef ANDROID_MODULO_H
#define ANDROID_MODULO_H

namespace android {

// Modulo class is used for intentionally wrapping variables such as
// counters and timers.
//
// It may also be used for variables whose computation depends on the
// associativity of addition or subtraction.
//
// Features:
// 1) Modulo checks type sizes before performing operations to ensure
//    that the wrap points match. This is critical for safe modular arithmetic.
// 2) Modulo returns Modulo types from arithmetic operations, thereby
//    avoiding unintentional use in a non-modular computation.  A Modulo
//    type is converted to its base non-Modulo type through the value() function.
// 3) Modulo separates out overflowable types from non-overflowable types.
//    A signed overflow is technically undefined in C and C++.
//    Modulo types do not participate in sanitization.
// 4) Modulo comparisons are based on signed differences to account for wrap;
//    this is not the same as the direct comparison of values.
// 5) Safe use of binary arithmetic operations relies on conversions of
//    signed operands to unsigned operands (which are modular arithmetic safe).
//    Conversions which are implementation-defined are assumed to use 2's complement
//    representation. (See A, B, C, D from the ISO/IEC FDIS 14882
//    Information technology — Programming languages — C++).
//
// A: ISO/IEC 14882:2011(E) p84 section 4.7 Integral conversions
// (2) If the destination type is unsigned, the resulting value is the least unsigned
// integer congruent to the source integer (modulo 2^n where n is the number of bits
// used to represent the unsigned type). [ Note: In a two’s complement representation,
// this conversion is conceptual and there is no change in the bit pattern (if there
// is no truncation). — end note ]
// (3) If the destination type is signed, the value is unchanged if it can be represented
// in the destination type (and bit-field width); otherwise, the value is
// implementation-defined.
//
// B: ISO/IEC 14882:2011(E) p88 section 5 Expressions
// (9) Many binary operators that expect operands of arithmetic or enumeration type
// cause conversions and yield result types in a similar way. The purpose is to
// yield a common type, which is also the type of the result. This pattern is called
// the usual arithmetic conversions, which are defined as follows:
// [...]
// Otherwise, if both operands have signed integer types or both have unsigned
// integer types, the operand with the type of lesser integer conversion rank shall be
// converted to the type of the operand with greater rank.
// — Otherwise, if the operand that has unsigned integer type has rank greater than
// or equal to the rank of the type of the other operand, the operand with signed
// integer type shall be converted to the type of the operand with unsigned integer type.
//
// C: ISO/IEC 14882:2011(E) p86 section 4.13 Integer conversion rank
// [...] The rank of long long int shall be greater than the rank of long int,
// which shall be greater than the rank of int, which shall be greater than the
// rank of short int, which shall be greater than the rank of signed char.
// — The rank of any unsigned integer type shall equal the rank of the corresponding
// signed integer type.
//
// D: ISO/IEC 14882:2011(E) p75 section 3.9.1 Fundamental types
// [...] Unsigned integers, declared unsigned, shall obey the laws of arithmetic modulo
// 2^n where n is the number of bits in the value representation of that particular
// size of integer.
//
// Note:
// Other libraries do exist for safe integer operations which can detect the
// possibility of overflow (SafeInt from MS and safe-iop in android).
// Signed safe computation is also possible from the art header safe_math.h.

template <typename T> class Modulo {
    T mValue;

public:
    typedef typename std::make_signed<T>::type signedT;
    typedef typename std::make_unsigned<T>::type unsignedT;

    Modulo() { } // intentionally uninitialized data
    Modulo(const T &value) { mValue = value; }
    const T & value() const { return mValue; } // not assignable
    signedT signedValue() const { return mValue; }
    unsignedT unsignedValue() const { return mValue; }
    void getValue(T *value) const { *value = mValue; } // more type safe than value()

    // modular operations valid only if size of T <= size of S.
    template <typename S>
    __attribute__((no_sanitize("integer")))
    Modulo<T> operator +=(const Modulo<S> &other) {
        static_assert(sizeof(T) <= sizeof(S), "argument size mismatch");
        mValue += other.unsignedValue();
        return *this;
    }

    template <typename S>
    __attribute__((no_sanitize("integer")))
    Modulo<T> operator -=(const Modulo<S> &other) {
        static_assert(sizeof(T) <= sizeof(S), "argument size mismatch");
        mValue -= other.unsignedValue();
        return *this;
    }

    // modular operations resulting in a value valid only at the smaller of the two
    // Modulo base type sizes, but we only allow equal sizes to avoid confusion.
    template <typename S>
    __attribute__((no_sanitize("integer")))
    const Modulo<T> operator +(const Modulo<S> &other) const {
        static_assert(sizeof(T) == sizeof(S), "argument size mismatch");
        return Modulo<T>(mValue + other.unsignedValue());
    }

    template <typename S>
    __attribute__((no_sanitize("integer")))
    const Modulo<T> operator -(const Modulo<S> &other) const {
        static_assert(sizeof(T) == sizeof(S), "argument size mismatch");
        return Modulo<T>(mValue - other.unsignedValue());
    }

    // modular operations that should be checked only at the smaller of
    // the two type sizes, but we only allow equal sizes to avoid confusion.
    //
    // Caution: These relational and comparison operations are not equivalent to
    // the base type operations.
    template <typename S>
    __attribute__((no_sanitize("integer")))
    bool operator >(const Modulo<S> &other) const {
        static_assert(sizeof(T) == sizeof(S), "argument size mismatch");
        return static_cast<signedT>(mValue - other.unsignedValue()) > 0;
    }

    template <typename S>
    __attribute__((no_sanitize("integer")))
    bool operator >=(const Modulo<S> &other) const {
        static_assert(sizeof(T) == sizeof(S), "argument size mismatch");
        return static_cast<signedT>(mValue - other.unsignedValue()) >= 0;
    }

    template <typename S>
    __attribute__((no_sanitize("integer")))
    bool operator ==(const Modulo<S> &other) const {
        static_assert(sizeof(T) == sizeof(S), "argument size mismatch");
        return static_cast<signedT>(mValue - other.unsignedValue()) == 0;
    }

    template <typename S>
    __attribute__((no_sanitize("integer")))
    bool operator <=(const Modulo<S> &other) const {
        static_assert(sizeof(T) == sizeof(S), "argument size mismatch");
        return static_cast<signedT>(mValue - other.unsignedValue()) <= 0;
    }

    template <typename S>
    __attribute__((no_sanitize("integer")))
    bool operator <(const Modulo<S> &other) const {
        static_assert(sizeof(T) == sizeof(S), "argument size mismatch");
        return static_cast<signedT>(mValue - other.unsignedValue()) < 0;
    }


    // modular operations with a non-Modulo type allowed with wrapping
    // because there should be no confusion as to the meaning.
    template <typename S>
    __attribute__((no_sanitize("integer")))
    Modulo<T> operator +=(const S &other) {
        mValue += unsignedT(other);
        return *this;
    }

    template <typename S>
    __attribute__((no_sanitize("integer")))
    Modulo<T> operator -=(const S &other) {
        mValue -= unsignedT(other);
        return *this;
    }

    // modular operations with a non-Modulo type allowed with wrapping,
    // but we restrict this only when size of T is greater than or equal to
    // the size of S to avoid confusion with the nature of overflow.
    //
    // Use of this follows left-associative style.
    //
    // Note: a Modulo type may be promoted by using "differences" off of
    // a larger sized type, but we do not automate this.
    template <typename S>
    __attribute__((no_sanitize("integer")))
    const Modulo<T> operator +(const S &other) const {
        static_assert(sizeof(T) >= sizeof(S), "argument size mismatch");
        return Modulo<T>(mValue + unsignedT(other));
    }

    template <typename S>
    __attribute__((no_sanitize("integer")))
    const Modulo<T> operator -(const S &other) const {
        static_assert(sizeof(T) >= sizeof(S), "argument size mismatch");
        return Modulo<T>(mValue - unsignedT(other));
    }

    // multiply is intentionally omitted, but it is a common operator in
    // modular arithmetic.

    // shift operations are intentionally omitted, but perhaps useful.
    // For example, left-shifting a negative number is undefined in C++11.
};

} // namespace android

#endif /* ANDROID_MODULO_H */
