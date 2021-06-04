 /*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef TEXT_DESCRIPTIONS2_H_

#define TEXT_DESCRIPTIONS2_H_

#include <binder/Parcel.h>
#include <media/stagefright/foundation/ABase.h>

#include "mediaplayer2.pb.h"

using android::media::MediaPlayer2Proto::PlayerMessage;

namespace android {

class TextDescriptions2 {
public:
    enum {
        IN_BAND_TEXT_3GPP             = 0x01,
        OUT_OF_BAND_TEXT_SRT          = 0x02,

        GLOBAL_DESCRIPTIONS           = 0x100,
        LOCAL_DESCRIPTIONS            = 0x200,
    };

    static status_t getPlayerMessageOfDescriptions(
            const uint8_t *data, ssize_t size,
            uint32_t flags, int timeMs, PlayerMessage *playerMsg);
private:
    TextDescriptions2();

    enum {
        // These keys must be in sync with the keys in TimedText.java
        KEY_DISPLAY_FLAGS                 = 1, // int
        KEY_STYLE_FLAGS                   = 2, // int
        KEY_BACKGROUND_COLOR_RGBA         = 3, // int
        KEY_HIGHLIGHT_COLOR_RGBA          = 4, // int
        KEY_SCROLL_DELAY                  = 5, // int
        KEY_WRAP_TEXT                     = 6, // int
        KEY_START_TIME                    = 7, // int
        KEY_STRUCT_BLINKING_TEXT_LIST     = 8, // List<CharPos>
        KEY_STRUCT_FONT_LIST              = 9, // List<Font>
        KEY_STRUCT_HIGHLIGHT_LIST         = 10, // List<CharPos>
        KEY_STRUCT_HYPER_TEXT_LIST        = 11, // List<HyperText>
        KEY_STRUCT_KARAOKE_LIST           = 12, // List<Karaoke>
        KEY_STRUCT_STYLE_LIST             = 13, // List<Style>
        KEY_STRUCT_TEXT_POS               = 14, // TextPos
        KEY_STRUCT_JUSTIFICATION          = 15, // Justification
        KEY_STRUCT_TEXT                   = 16, // Text

        KEY_GLOBAL_SETTING                = 101,
        KEY_LOCAL_SETTING                 = 102,
        KEY_START_CHAR                    = 103,
        KEY_END_CHAR                      = 104,
        KEY_FONT_ID                       = 105,
        KEY_FONT_SIZE                     = 106,
        KEY_TEXT_COLOR_RGBA               = 107,
    };

    static status_t extractSRTLocalDescriptions(
            const uint8_t *data, ssize_t size,
            int timeMs, PlayerMessage *playerMsg);
    static status_t extract3GPPGlobalDescriptions(
            const uint8_t *data, ssize_t size,
            PlayerMessage *playerMsg);
    static status_t extract3GPPLocalDescriptions(
            const uint8_t *data, ssize_t size,
            int timeMs, PlayerMessage *playerMsg);

    DISALLOW_EVIL_CONSTRUCTORS(TextDescriptions2);
};

}  // namespace android
#endif  // TEXT_DESCRIPTIONS2_H_
