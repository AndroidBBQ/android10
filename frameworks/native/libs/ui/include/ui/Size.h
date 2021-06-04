/*
 * Copyright 2019 The Android Open Source Project
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

#include <algorithm>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

namespace android {
namespace ui {

// Forward declare a few things.
struct Size;
bool operator==(const Size& lhs, const Size& rhs);

/**
 * A simple value type representing a two-dimensional size
 */
struct Size {
    int32_t width;
    int32_t height;

    // Special values
    static const Size INVALID;
    static const Size EMPTY;

    // ------------------------------------------------------------------------
    // Construction
    // ------------------------------------------------------------------------

    Size() : Size(INVALID) {}
    template <typename T>
    Size(T&& w, T&& h)
          : width(Size::clamp<int32_t, T>(std::forward<T>(w))),
            height(Size::clamp<int32_t, T>(std::forward<T>(h))) {}

    // ------------------------------------------------------------------------
    // Accessors
    // ------------------------------------------------------------------------

    int32_t getWidth() const { return width; }
    int32_t getHeight() const { return height; }

    template <typename T>
    void setWidth(T&& v) {
        width = Size::clamp<int32_t, T>(std::forward<T>(v));
    }
    template <typename T>
    void setHeight(T&& v) {
        height = Size::clamp<int32_t, T>(std::forward<T>(v));
    }

    // ------------------------------------------------------------------------
    // Assignment
    // ------------------------------------------------------------------------

    void set(const Size& size) { *this = size; }
    template <typename T>
    void set(T&& w, T&& h) {
        set(Size(std::forward<T>(w), std::forward<T>(h)));
    }

    // Sets the value to INVALID
    void makeInvalid() { set(INVALID); }

    // Sets the value to EMPTY
    void clear() { set(EMPTY); }

    // ------------------------------------------------------------------------
    // Semantic checks
    // ------------------------------------------------------------------------

    // Valid means non-negative width and height
    bool isValid() const { return width >= 0 && height >= 0; }

    // Empty means zero width and height
    bool isEmpty() const { return *this == EMPTY; }

    // ------------------------------------------------------------------------
    // Clamp Helpers
    // ------------------------------------------------------------------------

    // Note: We use only features available in C++11 here for compatibility with
    // external targets which include this file directly or indirectly and which
    // themselves use C++11.

    // C++11 compatible replacement for std::remove_cv_reference_t [C++20]
    template <typename T>
    using remove_cv_reference_t =
            typename std::remove_cv<typename std::remove_reference<T>::type>::type;

    // Takes a value of type FromType, and ensures it can be represented as a value of type ToType,
    // clamping the input value to the output range if necessary.
    template <typename ToType, typename FromType>
    static Size::remove_cv_reference_t<ToType> clamp(
            typename std::enable_if<
                    std::numeric_limits<Size::remove_cv_reference_t<ToType>>::is_bounded &&
                            std::numeric_limits<Size::remove_cv_reference_t<FromType>>::is_bounded,
                    FromType&&>::type v) {
        static constexpr auto toHighest = std::numeric_limits<remove_cv_reference_t<ToType>>::max();
        static constexpr auto toLowest =
                std::numeric_limits<remove_cv_reference_t<ToType>>::lowest();
        static constexpr auto fromHighest =
                std::numeric_limits<remove_cv_reference_t<FromType>>::max();
        static constexpr auto fromLowest =
                std::numeric_limits<remove_cv_reference_t<FromType>>::lowest();

        // A clamp is needed if the range of FromType is not a subset of the range of ToType
        static constexpr bool isClampNeeded = (toLowest > fromLowest) || (toHighest < fromHighest);

        // If a clamp is not needed, the conversion is just a trivial cast.
        if (!isClampNeeded) {
            return static_cast<ToType>(v);
        }

        // Otherwise we leverage implicit conversion to safely compare values of
        // different types, to ensure we return a value clamped to the range of
        // ToType.
        return v < toLowest ? toLowest : (v > toHighest ? toHighest : static_cast<ToType>(v));
    }
};

// ------------------------------------------------------------------------
// Comparisons
// ------------------------------------------------------------------------

inline bool operator==(const Size& lhs, const Size& rhs) {
    return lhs.width == rhs.width && lhs.height == rhs.height;
}

inline bool operator!=(const Size& lhs, const Size& rhs) {
    return !operator==(lhs, rhs);
}

inline bool operator<(const Size& lhs, const Size& rhs) {
    // Orders by increasing width, then height.
    if (lhs.width != rhs.width) return lhs.width < rhs.width;
    return lhs.height < rhs.height;
}

} // namespace ui
} // namespace android
