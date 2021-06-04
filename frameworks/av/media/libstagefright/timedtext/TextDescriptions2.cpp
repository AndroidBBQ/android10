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

#include "TextDescriptions2.h"
#include <media/stagefright/foundation/ByteUtils.h>
#include <media/stagefright/MediaErrors.h>

namespace android {

TextDescriptions2::TextDescriptions2() {
}

status_t TextDescriptions2::getPlayerMessageOfDescriptions(
        const uint8_t *data, ssize_t size,
        uint32_t flags, int timeMs, PlayerMessage *playerMsg) {
    if (flags & IN_BAND_TEXT_3GPP) {
        if (flags & GLOBAL_DESCRIPTIONS) {
            return extract3GPPGlobalDescriptions(data, size, playerMsg);
        } else if (flags & LOCAL_DESCRIPTIONS) {
            return extract3GPPLocalDescriptions(data, size, timeMs, playerMsg);
        }
    } else if (flags & OUT_OF_BAND_TEXT_SRT) {
        if (flags & LOCAL_DESCRIPTIONS) {
            return extractSRTLocalDescriptions(data, size, timeMs, playerMsg);
        }
    }

    return ERROR_UNSUPPORTED;
}

// Parse the SRT text sample, and store the timing and text sample in a PlayerMessage.
// The PlayerMessage will be sent to MediaPlayer2.java through event, and will be
// parsed in TimedText.java.
status_t TextDescriptions2::extractSRTLocalDescriptions(
        const uint8_t *data, ssize_t size, int timeMs, PlayerMessage *playerMsg) {
    playerMsg->add_values()->set_int32_value(KEY_LOCAL_SETTING);
    playerMsg->add_values()->set_int32_value(KEY_START_TIME);
    playerMsg->add_values()->set_int32_value(timeMs);

    playerMsg->add_values()->set_int32_value(KEY_STRUCT_TEXT);
    playerMsg->add_values()->set_bytes_value(data, size);

    return OK;
}

// Extract the local 3GPP display descriptions. 3GPP local descriptions
// are appended to the text sample if any.
status_t TextDescriptions2::extract3GPPLocalDescriptions(
        const uint8_t *data, ssize_t size,
        int timeMs, PlayerMessage *playerMsg) {

    playerMsg->add_values()->set_int32_value(KEY_LOCAL_SETTING);

    // write start time to display this text sample
    playerMsg->add_values()->set_int32_value(KEY_START_TIME);
    playerMsg->add_values()->set_int32_value(timeMs);

    if (size < 2) {
        return OK;
    }
    ssize_t textLen = (*data) << 8 | (*(data + 1));

    if (size < textLen + 2) {
        return OK;
    }

    // write text sample length and text sample itself
    playerMsg->add_values()->set_int32_value(KEY_STRUCT_TEXT);
    playerMsg->add_values()->set_bytes_value(data + 2, textLen);

    if (size > textLen + 2) {
        data += (textLen + 2);
        size -= (textLen + 2);
    } else {
        return OK;
    }

    while (size >= 8) {
        const uint8_t *tmpData = data;
        ssize_t chunkSize = U32_AT(tmpData);      // size includes size and type
        uint32_t chunkType = U32_AT(tmpData + 4);

        if (chunkSize <= 8 || chunkSize > size) {
            return OK;
        }

        size_t remaining = chunkSize - 8;

        tmpData += 8;

        switch(chunkType) {
            // 'tbox' box to indicate the position of the text with values
            // of top, left, bottom and right
            case FOURCC('t', 'b', 'o', 'x'):
            {
                if (remaining < 8) {
                    return OK;
                }
                playerMsg->add_values()->set_int32_value(KEY_STRUCT_TEXT_POS);
                playerMsg->add_values()->set_int32_value(U16_AT(tmpData));
                playerMsg->add_values()->set_int32_value(U16_AT(tmpData + 2));
                playerMsg->add_values()->set_int32_value(U16_AT(tmpData + 4));
                playerMsg->add_values()->set_int32_value(U16_AT(tmpData + 6));

                tmpData += 8;
                remaining -= 8;
                break;
            }
            default:
            {
                break;
            }
        }

        data += chunkSize;
        size -= chunkSize;
    }

    return OK;
}

// To extract box 'tx3g' defined in 3GPP TS 26.245, and store it in a PlayerMessage
status_t TextDescriptions2::extract3GPPGlobalDescriptions(
        const uint8_t *data, ssize_t size, PlayerMessage *playerMsg) {

    playerMsg->add_values()->set_int32_value(KEY_GLOBAL_SETTING);

    while (size >= 8) {
        ssize_t chunkSize = U32_AT(data);
        uint32_t chunkType = U32_AT(data + 4);
        const uint8_t *tmpData = data;
        tmpData += 8;
        size_t remaining = size - 8;

        if (size < chunkSize) {
            return OK;
        }
        switch(chunkType) {
            case FOURCC('t', 'x', '3', 'g'):
            {
                if (remaining < 18) {
                    return OK;
                }
                // Skip DISPLAY_FLAGS, STRUCT_JUSTIFICATION, and BACKGROUND_COLOR_RGBA
                tmpData += 18;
                remaining -= 18;

                if (remaining < 8) {
                    return OK;
                }
                playerMsg->add_values()->set_int32_value(KEY_STRUCT_TEXT_POS);
                playerMsg->add_values()->set_int32_value(U16_AT(tmpData));
                playerMsg->add_values()->set_int32_value(U16_AT(tmpData + 2));
                playerMsg->add_values()->set_int32_value(U16_AT(tmpData + 4));
                playerMsg->add_values()->set_int32_value(U16_AT(tmpData + 6));

                tmpData += 8;
                remaining -= 18;
                // Ignore remaining data.
                break;
            }
            default:
            {
                break;
            }
        }

        data += chunkSize;
        size -= chunkSize;
    }

    return OK;
}

}  // namespace android
