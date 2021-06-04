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

#ifndef C2UTILS_MACRO_UTILS_H_
#define C2UTILS_MACRO_UTILS_H_

/** \file
 * Macro utilities for the utils library used by Codec2 implementations.
 */

/// \if 0

/* --------------------------------- VARIABLE ARGUMENT COUNTING --------------------------------- */

// remove empty arguments - _C2_ARG() expands to '', while _C2_ARG(x) expands to ', x'
// _C2_ARGn(...) does the same for n arguments
#define _C2_ARG(...) , ##__VA_ARGS__
#define _C2_ARG2(_1, _2) _C2_ARG(_1) _C2_ARG(_2)
#define _C2_ARG4(_1, _2, _3, _4) _C2_ARG2(_1, _2) _C2_ARG2(_3, _4)
#define _C2_ARG8(_1, _2, _3, _4, _5, _6, _7, _8) _C2_ARG4(_1, _2, _3, _4) _C2_ARG4(_5, _6, _7, _8)
#define _C2_ARG16(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16) \
        _C2_ARG8(_1, _2, _3, _4, _5, _6, _7, _8) _C2_ARG8(_9, _10, _11, _12, _13, _14, _15, _16)

// return the 65th argument
#define _C2_ARGC_3(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
        _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, \
        _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, \
        _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, ...) _64

/// \endif

/**
 * Returns the number of arguments.
 */
// We do this by prepending 1 and appending 65 designed values such that the 65th element
// will be the number of arguments.
#define _C2_ARGC(...) _C2_ARGC_1(0, ##__VA_ARGS__, \
        64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, \
        42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, \
        20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

/// \if 0

// step 1. remove empty arguments - this is needed to allow trailing comma in enum definitions
// (NOTE: we don't know which argument will have this trailing comma so we have to try all)
#define _C2_ARGC_1(_, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
        _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, \
        _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, \
        _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, ...) \
    _C2_ARGC_2(_ _C2_ARG(_0) \
    _C2_ARG16(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16) \
    _C2_ARG16(_17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32) \
    _C2_ARG16(_33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48) \
    _C2_ARG16(_49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64), \
    ##__VA_ARGS__)

// step 2. this is needed as removed arguments cannot be passed directly as empty into a macro
#define _C2_ARGC_2(...) _C2_ARGC_3(__VA_ARGS__)

/// \endif

/* -------------------------------- VARIABLE ARGUMENT CONVERSION -------------------------------- */

/// \if 0

// macros that convert _1, _2, _3, ... to fn(_1, arg), fn(_2, arg), fn(_3, arg), ...
#define _C2_MAP_64(fn, arg, head, ...) fn(head, arg), _C2_MAP_63(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_63(fn, arg, head, ...) fn(head, arg), _C2_MAP_62(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_62(fn, arg, head, ...) fn(head, arg), _C2_MAP_61(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_61(fn, arg, head, ...) fn(head, arg), _C2_MAP_60(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_60(fn, arg, head, ...) fn(head, arg), _C2_MAP_59(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_59(fn, arg, head, ...) fn(head, arg), _C2_MAP_58(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_58(fn, arg, head, ...) fn(head, arg), _C2_MAP_57(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_57(fn, arg, head, ...) fn(head, arg), _C2_MAP_56(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_56(fn, arg, head, ...) fn(head, arg), _C2_MAP_55(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_55(fn, arg, head, ...) fn(head, arg), _C2_MAP_54(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_54(fn, arg, head, ...) fn(head, arg), _C2_MAP_53(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_53(fn, arg, head, ...) fn(head, arg), _C2_MAP_52(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_52(fn, arg, head, ...) fn(head, arg), _C2_MAP_51(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_51(fn, arg, head, ...) fn(head, arg), _C2_MAP_50(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_50(fn, arg, head, ...) fn(head, arg), _C2_MAP_49(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_49(fn, arg, head, ...) fn(head, arg), _C2_MAP_48(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_48(fn, arg, head, ...) fn(head, arg), _C2_MAP_47(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_47(fn, arg, head, ...) fn(head, arg), _C2_MAP_46(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_46(fn, arg, head, ...) fn(head, arg), _C2_MAP_45(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_45(fn, arg, head, ...) fn(head, arg), _C2_MAP_44(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_44(fn, arg, head, ...) fn(head, arg), _C2_MAP_43(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_43(fn, arg, head, ...) fn(head, arg), _C2_MAP_42(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_42(fn, arg, head, ...) fn(head, arg), _C2_MAP_41(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_41(fn, arg, head, ...) fn(head, arg), _C2_MAP_40(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_40(fn, arg, head, ...) fn(head, arg), _C2_MAP_39(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_39(fn, arg, head, ...) fn(head, arg), _C2_MAP_38(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_38(fn, arg, head, ...) fn(head, arg), _C2_MAP_37(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_37(fn, arg, head, ...) fn(head, arg), _C2_MAP_36(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_36(fn, arg, head, ...) fn(head, arg), _C2_MAP_35(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_35(fn, arg, head, ...) fn(head, arg), _C2_MAP_34(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_34(fn, arg, head, ...) fn(head, arg), _C2_MAP_33(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_33(fn, arg, head, ...) fn(head, arg), _C2_MAP_32(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_32(fn, arg, head, ...) fn(head, arg), _C2_MAP_31(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_31(fn, arg, head, ...) fn(head, arg), _C2_MAP_30(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_30(fn, arg, head, ...) fn(head, arg), _C2_MAP_29(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_29(fn, arg, head, ...) fn(head, arg), _C2_MAP_28(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_28(fn, arg, head, ...) fn(head, arg), _C2_MAP_27(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_27(fn, arg, head, ...) fn(head, arg), _C2_MAP_26(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_26(fn, arg, head, ...) fn(head, arg), _C2_MAP_25(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_25(fn, arg, head, ...) fn(head, arg), _C2_MAP_24(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_24(fn, arg, head, ...) fn(head, arg), _C2_MAP_23(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_23(fn, arg, head, ...) fn(head, arg), _C2_MAP_22(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_22(fn, arg, head, ...) fn(head, arg), _C2_MAP_21(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_21(fn, arg, head, ...) fn(head, arg), _C2_MAP_20(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_20(fn, arg, head, ...) fn(head, arg), _C2_MAP_19(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_19(fn, arg, head, ...) fn(head, arg), _C2_MAP_18(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_18(fn, arg, head, ...) fn(head, arg), _C2_MAP_17(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_17(fn, arg, head, ...) fn(head, arg), _C2_MAP_16(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_16(fn, arg, head, ...) fn(head, arg), _C2_MAP_15(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_15(fn, arg, head, ...) fn(head, arg), _C2_MAP_14(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_14(fn, arg, head, ...) fn(head, arg), _C2_MAP_13(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_13(fn, arg, head, ...) fn(head, arg), _C2_MAP_12(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_12(fn, arg, head, ...) fn(head, arg), _C2_MAP_11(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_11(fn, arg, head, ...) fn(head, arg), _C2_MAP_10(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_10(fn, arg, head, ...) fn(head, arg), _C2_MAP_9(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_9(fn, arg, head, ...) fn(head, arg), _C2_MAP_8(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_8(fn, arg, head, ...) fn(head, arg), _C2_MAP_7(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_7(fn, arg, head, ...) fn(head, arg), _C2_MAP_6(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_6(fn, arg, head, ...) fn(head, arg), _C2_MAP_5(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_5(fn, arg, head, ...) fn(head, arg), _C2_MAP_4(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_4(fn, arg, head, ...) fn(head, arg), _C2_MAP_3(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_3(fn, arg, head, ...) fn(head, arg), _C2_MAP_2(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_2(fn, arg, head, ...) fn(head, arg), _C2_MAP_1(fn, arg, ##__VA_ARGS__)
#define _C2_MAP_1(fn, arg, head, ...) fn(head, arg)

/// \endif

/**
 * Maps each argument using another macro x -> fn(x, arg)
 */
// use wrapper to call the proper mapper based on the number of arguments
#define _C2_MAP(fn, arg, ...) _C2_MAP__(_C2_ARGC(__VA_ARGS__), fn, arg, ##__VA_ARGS__)

/// \if 0

// evaluate _n so it becomes a number
#define _C2_MAP__(_n, fn, arg, ...) _C2_MAP_(_n, fn, arg, __VA_ARGS__)
// call the proper mapper
#define _C2_MAP_(_n, fn, arg, ...) _C2_MAP_##_n (fn, arg, __VA_ARGS__)

/// \endif

#endif  // C2UTILS_MACRO_UTILS_H_
