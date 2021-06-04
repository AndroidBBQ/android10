/*
 * Copyright 2018 The Android Open Source Project
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

#ifndef ANDROID_CODEC2_MAPPER_H_
#define ANDROID_CODEC2_MAPPER_H_

#include <C2Config.h>

#include <media/stagefright/foundation/ColorUtils.h>

#include <memory>

namespace android {

    /**
     * Utility class to map Codec 2.0 values to android values.
     */
    struct C2Mapper {
        struct ProfileLevelMapper {
            virtual bool mapProfile(C2Config::profile_t, int32_t*) = 0;
            virtual bool mapProfile(int32_t, C2Config::profile_t*) = 0;
            virtual bool mapLevel(C2Config::level_t, int32_t*) = 0;
            virtual bool mapLevel(int32_t, C2Config::level_t*) = 0;
            virtual ~ProfileLevelMapper() = default;
        };

        static std::shared_ptr<ProfileLevelMapper>
        GetProfileLevelMapper(std::string mediaType);

        static std::shared_ptr<ProfileLevelMapper>
        GetHdrProfileLevelMapper(std::string mediaType, bool isHdr10Plus = false);

        // convert between bitrates
        static bool map(C2Config::bitrate_mode_t, int32_t*);
        static bool map(int32_t, C2Config::bitrate_mode_t*);

        // convert between pcm encodings
        static bool map(C2Config::pcm_encoding_t, int32_t*);
        static bool map(int32_t, C2Config::pcm_encoding_t*);

        // convert between picture types
        static bool map(C2Config::picture_type_t, int32_t*);
        static bool map(int32_t, C2Config::picture_type_t*);

        // convert between color aspects
        static bool map(C2Color::range_t, int32_t*);
        static bool map(int32_t, C2Color::range_t*);
        static bool map(C2Color::primaries_t, C2Color::matrix_t, int32_t*);
        static bool map(int32_t, C2Color::primaries_t*, C2Color::matrix_t*);
        static bool map(C2Color::transfer_t, int32_t*);
        static bool map(int32_t, C2Color::transfer_t*);

        static bool map(
                C2Color::range_t, C2Color::primaries_t, C2Color::matrix_t, C2Color::transfer_t,
                uint32_t *dataSpace);

        static bool map(C2Color::range_t, ColorAspects::Range*);
        static bool map(ColorAspects::Range, C2Color::range_t*);
        static bool map(C2Color::primaries_t, ColorAspects::Primaries*);
        static bool map(ColorAspects::Primaries, C2Color::primaries_t*);
        static bool map(C2Color::matrix_t, ColorAspects::MatrixCoeffs*);
        static bool map(ColorAspects::MatrixCoeffs, C2Color::matrix_t*);
        static bool map(C2Color::transfer_t, ColorAspects::Transfer*);
        static bool map(ColorAspects::Transfer, C2Color::transfer_t*);
    };
}

#endif  // ANDROID_CODEC2_MAPPER_H_
