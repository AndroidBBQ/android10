/*
 * Copyright 2014 The Android Open Source Project
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

#ifndef A_UTILS_H_

#define A_UTILS_H_

/* ============================ math templates ============================ */

/* T must be integer type, den must not be 0 */
template<class T>
inline static const T divRound(const T &num, const T &den) {
    if ((num >= 0) ^ (den >= 0)) {
        return (num - den / 2) / den;
    } else {
        return (num + den / 2) / den;
    }
}

/* == ceil(num / den). T must be integer type, den must not be 0 */
template<class T>
inline static const T divUp(const T &num, const T &den) {
    if (den < 0) {
        return (num < 0 ? num + den + 1 : num) / den;
    } else {
        return (num < 0 ? num : num + den - 1) / den;
    }
}

/* == ceil(num / den) * den. T must be integer type, alignment must be positive power of 2 */
template<class T, class U>
inline static const T align(const T &num, const U &den) {
    return (num + (T)(den - 1)) & (T)~(den - 1);
}

template<class T>
inline static T abs(const T &a) {
    return a < 0 ? -a : a;
}

template<class T>
inline static const T &min(const T &a, const T &b) {
    return a < b ? a : b;
}

template<class T>
inline static const T &max(const T &a, const T &b) {
    return a > b ? a : b;
}

template<class T>
void ENSURE_UNSIGNED_TYPE() {
    T TYPE_MUST_BE_UNSIGNED[(T)-1 < 0 ? -1 : 0] __unused;
}

// needle is in range [hayStart, hayStart + haySize)
template<class T, class U>
__attribute__((no_sanitize("integer")))
inline static bool isInRange(const T &hayStart, const U &haySize, const T &needle) {
    ENSURE_UNSIGNED_TYPE<U>();
    return (T)(hayStart + haySize) >= hayStart && needle >= hayStart && (U)(needle - hayStart) < haySize;
}

// [needleStart, needleStart + needleSize) is in range [hayStart, hayStart + haySize)
template<class T, class U>
__attribute__((no_sanitize("integer")))
inline static bool isInRange(
        const T &hayStart, const U &haySize, const T &needleStart, const U &needleSize) {
    ENSURE_UNSIGNED_TYPE<U>();
    return isInRange(hayStart, haySize, needleStart)
            && (T)(needleStart + needleSize) >= needleStart
            && (U)(needleStart + needleSize - hayStart) <= haySize;
}

/* T must be integer type, period must be positive */
template<class T>
inline static T periodicError(const T &val, const T &period) {
    T err = abs(val) % period;
    return (err < (period / 2)) ? err : (period - err);
}

#endif  // A_UTILS_H_
