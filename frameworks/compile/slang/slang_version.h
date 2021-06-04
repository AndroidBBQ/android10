/*
 * Copyright 2011-2012, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _FRAMEWORKS_COMPILE_SLANG_SLANG_VERSION_H_  // NOLINT
#define _FRAMEWORKS_COMPILE_SLANG_SLANG_VERSION_H_

#include <climits>

#define RS_DEVELOPMENT_API UINT_MAX

// API levels used by the standard Android SDK.
// MR -> Maintenance Release
// HC -> Honeycomb
// ICS -> Ice Cream Sandwich
// JB -> Jelly Bean
// KK -> KitKat
// M -> Marshmallow
enum SlangTargetAPI {
  SLANG_MINIMUM_TARGET_API = 11,
  SLANG_HC_TARGET_API = 11,
  SLANG_HC_MR1_TARGET_API = 12,
  SLANG_HC_MR2_TARGET_API = 13,
  SLANG_ICS_TARGET_API = 14,
  SLANG_ICS_MR1_TARGET_API = 15,
  SLANG_JB_TARGET_API = 16,
  SLANG_JB_MR1_TARGET_API = 17,
  SLANG_JB_MR2_TARGET_API = 18,
  SLANG_KK_TARGET_API = 19,
  SLANG_L_TARGET_API = 21,
  SLANG_M_TARGET_API = 23,
  SLANG_N_TARGET_API = 24,
  SLANG_N_MR1_TARGET_API = 25,
  SLANG_O_TARGET_API = 26,
  SLANG_O_MR1_TARGET_API = 27,
  SLANG_P_TARGET_API = 28,
  SLANG_MAXIMUM_TARGET_API = RS_VERSION,
  SLANG_DEVELOPMENT_TARGET_API = RS_DEVELOPMENT_API
};
// Note that RS_VERSION is defined at build time (see Android.mk for details).

// API levels where particular features exist.
// . Open (feature appears at a particular level and continues to exist):
//     SLANG_FEAT_FOO_API
// . Closed (feature exists only through a particular range of API levels):
//     SLANG_FEAT_BAR_API_MIN, SLANG_FEAT_BAR_API_MAX
enum SlangFeatureAPI {
  SLANG_FEATURE_GENERAL_REDUCTION_API = SLANG_N_TARGET_API,
  SLANG_FEATURE_GENERAL_REDUCTION_HALTER_API = SLANG_DEVELOPMENT_TARGET_API,
  SLANG_FEATURE_SINGLE_SOURCE_API = SLANG_N_TARGET_API,
};

// SlangVersion refers to the released compiler version, for which
// certain behaviors could change.
//
// The SlangVersion is recorded in the generated bitcode.  A bitcode
// consumer (for example: runtime, driver, bcc) is expected to use
// this version number as follows:
//
//   If version number is at least $VERSION, then I can assume that
//   $PROPERTY holds.
//
// However, a bitcode consumer is not obligated to act on this
// information (for example, the consumer may be too old to know about
// that version number).  So slang must not change its behavior for
// $VERSION in such a way that a backend needs to know about $VERSION
// in order to behave properly.
//
// For example:
//
//   If version number is at least N_STRUCT_EXPLICIT_PADDING, then I
//   can assume that no field of any struct is followed by implicit
//   padding.
//
//   bcc can take advantage of this (by turning off analyses and
//   transformations that are needed because of implicit padding), but
//   slang must still generate code that works properly with a bcc
//   that is too old to know about the N_STRUCT_EXPLICIT_PADDING
//   guarantee.
//
// Note that we CANNOT say
//
//   If version number is at least $VERSION_LO but lower than
//   $VERSION_HI, then I can assume that $PROPERTY holds.
//
// because a bitcode consumer might know about $VERSION_LO (where it
// could start taking advantage of $PROPERTY) without knowing about
// $VERSION_HI (where it would have to stop taking advantage of
// $PROPERTY).
namespace SlangVersion {
enum {
  LEGACY = 0,
  ICS = 1400,
  JB = 1600,
  JB_MR1 = 1700,
  JB_MR2 = 1800,
  KK = 1900,
  KK_P1 = 1901,
  L = 2100,
  M = 2300,
  M_RS_OBJECT = 2310,
  N = 2400,
  N_STRUCT_EXPLICIT_PADDING = 2410,
  CURRENT = N_STRUCT_EXPLICIT_PADDING
};
}  // namespace SlangVersion

#endif  // _FRAMEWORKS_COMPILE_SLANG_SLANG_VERSION_H_  NOLINT
