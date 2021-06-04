/*
 * Copyright (C) 2011 The Android Open Source Project
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

#include "TextDescriptions.h"
#include <media/stagefright/foundation/ByteUtils.h>
#include <media/stagefright/MediaErrors.h>

namespace android {

TextDescriptions::TextDescriptions() {
}

status_t TextDescriptions::getParcelOfDescriptions(
        const uint8_t *data, ssize_t size,
        uint32_t flags, int timeMs, Parcel *parcel) {
    parcel->freeData();

    if (flags & IN_BAND_TEXT_3GPP) {
        if (flags & GLOBAL_DESCRIPTIONS) {
            return extract3GPPGlobalDescriptions(data, size, parcel);
        } else if (flags & LOCAL_DESCRIPTIONS) {
            return extract3GPPLocalDescriptions(data, size, timeMs, parcel);
        }
    } else if (flags & OUT_OF_BAND_TEXT_SRT) {
        if (flags & LOCAL_DESCRIPTIONS) {
            return extractSRTLocalDescriptions(data, size, timeMs, parcel);
        }
    }

    return ERROR_UNSUPPORTED;
}

// Parse the SRT text sample, and store the timing and text sample in a Parcel.
// The Parcel will be sent to MediaPlayer.java through event, and will be
// parsed in TimedText.java.
status_t TextDescriptions::extractSRTLocalDescriptions(
        const uint8_t *data, ssize_t size, int timeMs, Parcel *parcel) {
    parcel->writeInt32(KEY_LOCAL_SETTING);
    parcel->writeInt32(KEY_START_TIME);
    parcel->writeInt32(timeMs);

    parcel->writeInt32(KEY_STRUCT_TEXT);
    // write the size of the text sample
    parcel->writeInt32(size);
    // write the text sample as a byte array
    parcel->writeInt32(size);
    parcel->write(data, size);

    return OK;
}

// Extract the local 3GPP display descriptions. 3GPP local descriptions
// are appended to the text sample if any. The descriptions could include
// information such as text styles, highlights, karaoke and so on. They
// are contained in different boxes, such as 'styl' box contains text
// styles, and 'krok' box contains karaoke timing and positions.
status_t TextDescriptions::extract3GPPLocalDescriptions(
        const uint8_t *data, ssize_t size,
        int timeMs, Parcel *parcel) {

    parcel->writeInt32(KEY_LOCAL_SETTING);

    // write start time to display this text sample
    parcel->writeInt32(KEY_START_TIME);
    parcel->writeInt32(timeMs);

    if (size < 2) {
        return OK;
    }
    ssize_t textLen = (*data) << 8 | (*(data + 1));

    if (size < textLen + 2) {
        return OK;
    }

    // write text sample length and text sample itself
    parcel->writeInt32(KEY_STRUCT_TEXT);
    parcel->writeInt32(textLen);
    parcel->writeInt32(textLen);
    parcel->write(data + 2, textLen);

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
            // 'styl' box specifies the style of the text.
            case FOURCC('s', 't', 'y', 'l'):
            {
                if (remaining < 2) {
                    return OK;
                }
                size_t dataPos = parcel->dataPosition();
                uint16_t count = U16_AT(tmpData);

                tmpData += 2;
                remaining -= 2;

                for (int i = 0; i < count; i++) {
                    if (remaining < 12) {
                        // roll back
                        parcel->setDataPosition(dataPos);
                        return OK;
                    }
                    parcel->writeInt32(KEY_STRUCT_STYLE_LIST);
                    parcel->writeInt32(KEY_START_CHAR);
                    parcel->writeInt32(U16_AT(tmpData));

                    parcel->writeInt32(KEY_END_CHAR);
                    parcel->writeInt32(U16_AT(tmpData + 2));

                    parcel->writeInt32(KEY_FONT_ID);
                    parcel->writeInt32(U16_AT(tmpData + 4));

                    parcel->writeInt32(KEY_STYLE_FLAGS);
                    parcel->writeInt32(*(tmpData + 6));

                    parcel->writeInt32(KEY_FONT_SIZE);
                    parcel->writeInt32(*(tmpData + 7));

                    parcel->writeInt32(KEY_TEXT_COLOR_RGBA);
                    uint32_t rgba = *(tmpData + 8) << 24 | *(tmpData + 9) << 16
                        | *(tmpData + 10) << 8 | *(tmpData + 11);
                    parcel->writeInt32(rgba);

                    tmpData += 12;
                    remaining -= 12;
                }

                break;
            }
            // 'krok' box. The number of highlight events is specified, and each
            // event is specified by a starting and ending char offset and an end
            // time for the event.
            case FOURCC('k', 'r', 'o', 'k'):
            {
                if (remaining < 6) {
                    return OK;
                }
                size_t dataPos = parcel->dataPosition();

                parcel->writeInt32(KEY_STRUCT_KARAOKE_LIST);

                int startTime = U32_AT(tmpData);
                uint16_t count = U16_AT(tmpData + 4);
                parcel->writeInt32(count);

                tmpData += 6;
                remaining -= 6;
                int lastEndTime = 0;

                for (int i = 0; i < count; i++) {
                    if (remaining < 8) {
                        // roll back
                        parcel->setDataPosition(dataPos);
                        return OK;
                    }
                    parcel->writeInt32(startTime + lastEndTime);

                    lastEndTime = U32_AT(tmpData);
                    parcel->writeInt32(lastEndTime);

                    parcel->writeInt32(U16_AT(tmpData + 4));
                    parcel->writeInt32(U16_AT(tmpData + 6));

                    tmpData += 8;
                    remaining -= 8;
                }

                break;
            }
            // 'hlit' box specifies highlighted text
            case FOURCC('h', 'l', 'i', 't'):
            {
                if (remaining < 4) {
                    return OK;
                }

                parcel->writeInt32(KEY_STRUCT_HIGHLIGHT_LIST);

                // the start char offset to highlight
                parcel->writeInt32(U16_AT(tmpData));
                // the last char offset to highlight
                parcel->writeInt32(U16_AT(tmpData + 2));

                tmpData += 4;
                remaining -= 4;
                break;
            }
            // 'hclr' box specifies the RGBA color: 8 bits each of
            // red, green, blue, and an alpha(transparency) value
            case FOURCC('h', 'c', 'l', 'r'):
            {
                if (remaining < 4) {
                    return OK;
                }
                parcel->writeInt32(KEY_HIGHLIGHT_COLOR_RGBA);

                uint32_t rgba = *(tmpData) << 24 | *(tmpData + 1) << 16
                    | *(tmpData + 2) << 8 | *(tmpData + 3);
                parcel->writeInt32(rgba);

                tmpData += 4;
                remaining -= 4;
                break;
            }
            // 'dlay' box specifies a delay after a scroll in and/or
            // before scroll out.
            case FOURCC('d', 'l', 'a', 'y'):
            {
                if (remaining < 4) {
                    return OK;
                }
                parcel->writeInt32(KEY_SCROLL_DELAY);

                uint32_t delay = *(tmpData) << 24 | *(tmpData + 1) << 16
                    | *(tmpData + 2) << 8 | *(tmpData + 3);
                parcel->writeInt32(delay);

                tmpData += 4;
                remaining -= 4;
                break;
            }
            // 'href' box for hyper text link
            case FOURCC('h', 'r', 'e', 'f'):
            {
                if (remaining < 5) {
                    return OK;
                }

                size_t dataPos = parcel->dataPosition();

                parcel->writeInt32(KEY_STRUCT_HYPER_TEXT_LIST);

                // the start offset of the text to be linked
                parcel->writeInt32(U16_AT(tmpData));
                // the end offset of the text
                parcel->writeInt32(U16_AT(tmpData + 2));

                // the number of bytes in the following URL
                size_t len = *(tmpData + 4);
                parcel->writeInt32(len);

                remaining -= 5;

                if (remaining  < len) {
                    parcel->setDataPosition(dataPos);
                    return OK;
                }
                // the linked-to URL
                parcel->writeInt32(len);
                parcel->write(tmpData + 5, len);

                tmpData += (5 + len);
                remaining -= len;

                if (remaining  < 1) {
                    parcel->setDataPosition(dataPos);
                    return OK;
                }

                // the number of bytes in the following "alt" string
                len = *tmpData;
                parcel->writeInt32(len);

                tmpData += 1;
                remaining -= 1;
                if (remaining  < len) {
                    parcel->setDataPosition(dataPos);
                    return OK;
                }

                // an "alt" string for user display
                parcel->writeInt32(len);
                parcel->write(tmpData, len);

                tmpData += 1;
                remaining -= 1;
                break;
            }
            // 'tbox' box to indicate the position of the text with values
            // of top, left, bottom and right
            case FOURCC('t', 'b', 'o', 'x'):
            {
                if (remaining < 8) {
                    return OK;
                }
                parcel->writeInt32(KEY_STRUCT_TEXT_POS);
                parcel->writeInt32(U16_AT(tmpData));
                parcel->writeInt32(U16_AT(tmpData + 2));
                parcel->writeInt32(U16_AT(tmpData + 4));
                parcel->writeInt32(U16_AT(tmpData + 6));

                tmpData += 8;
                remaining -= 8;
                break;
            }
            // 'blnk' to specify the char range to be blinked
            case FOURCC('b', 'l', 'n', 'k'):
            {
                if (remaining < 4) {
                    return OK;
                }

                parcel->writeInt32(KEY_STRUCT_BLINKING_TEXT_LIST);

                // start char offset
                parcel->writeInt32(U16_AT(tmpData));
                // end char offset
                parcel->writeInt32(U16_AT(tmpData + 2));

                tmpData += 4;
                remaining -= 4;
                break;
            }
            // 'twrp' box specifies text wrap behavior. If the value if 0x00,
            // then no wrap. If it's 0x01, then automatic 'soft' wrap is enabled.
            // 0x02-0xff are reserved.
            case FOURCC('t', 'w', 'r', 'p'):
            {
                if (remaining < 1) {
                    return OK;
                }
                parcel->writeInt32(KEY_WRAP_TEXT);
                parcel->writeInt32(*tmpData);

                tmpData += 1;
                remaining -= 1;
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

// To extract box 'tx3g' defined in 3GPP TS 26.245, and store it in a Parcel
status_t TextDescriptions::extract3GPPGlobalDescriptions(
        const uint8_t *data, ssize_t size, Parcel *parcel) {

    parcel->writeInt32(KEY_GLOBAL_SETTING);

    while (size >= 8) {
        ssize_t chunkSize = U32_AT(data);
        uint32_t chunkType = U32_AT(data + 4);
        const uint8_t *tmpData = data;
        tmpData += 8;
        size_t remaining = size - 8;

        if (chunkSize <= 8 || size < chunkSize) {
            return OK;
        }
        switch(chunkType) {
            case FOURCC('t', 'x', '3', 'g'):
            {
                if (remaining < 18) { // 8 just below, and another 10 a little further down
                    return OK;
                }
                tmpData += 8; // skip the first 8 bytes
                remaining -=8;
                parcel->writeInt32(KEY_DISPLAY_FLAGS);
                parcel->writeInt32(U32_AT(tmpData));

                parcel->writeInt32(KEY_STRUCT_JUSTIFICATION);
                parcel->writeInt32(tmpData[4]);
                parcel->writeInt32(tmpData[5]);

                parcel->writeInt32(KEY_BACKGROUND_COLOR_RGBA);
                uint32_t rgba = *(tmpData + 6) << 24 | *(tmpData + 7) << 16
                    | *(tmpData + 8) << 8 | *(tmpData + 9);
                parcel->writeInt32(rgba);

                tmpData += 10;
                remaining -= 10;

                if (remaining < 8) {
                    return OK;
                }
                parcel->writeInt32(KEY_STRUCT_TEXT_POS);
                parcel->writeInt32(U16_AT(tmpData));
                parcel->writeInt32(U16_AT(tmpData + 2));
                parcel->writeInt32(U16_AT(tmpData + 4));
                parcel->writeInt32(U16_AT(tmpData + 6));

                tmpData += 8;
                remaining -= 8;

                if (remaining < 12) {
                    return OK;
                }
                parcel->writeInt32(KEY_STRUCT_STYLE_LIST);
                parcel->writeInt32(KEY_START_CHAR);
                parcel->writeInt32(U16_AT(tmpData));

                parcel->writeInt32(KEY_END_CHAR);
                parcel->writeInt32(U16_AT(tmpData + 2));

                parcel->writeInt32(KEY_FONT_ID);
                parcel->writeInt32(U16_AT(tmpData + 4));

                parcel->writeInt32(KEY_STYLE_FLAGS);
                parcel->writeInt32(*(tmpData + 6));

                parcel->writeInt32(KEY_FONT_SIZE);
                parcel->writeInt32(*(tmpData + 7));

                parcel->writeInt32(KEY_TEXT_COLOR_RGBA);
                rgba = *(tmpData + 8) << 24 | *(tmpData + 9) << 16
                    | *(tmpData + 10) << 8 | *(tmpData + 11);
                parcel->writeInt32(rgba);

                tmpData += 12;
                remaining -= 12;

                if (remaining < 2) {
                    return OK;
                }

                size_t dataPos = parcel->dataPosition();

                parcel->writeInt32(KEY_STRUCT_FONT_LIST);
                uint16_t count = U16_AT(tmpData);
                parcel->writeInt32(count);

                tmpData += 2;
                remaining -= 2;

                for (int i = 0; i < count; i++) {
                    if (remaining < 3) {
                        // roll back
                        parcel->setDataPosition(dataPos);
                        return OK;
                    }
                    // font ID
                    parcel->writeInt32(U16_AT(tmpData));

                    // font name length
                    parcel->writeInt32(*(tmpData + 2));

                    size_t len = *(tmpData + 2);

                    tmpData += 3;
                    remaining -= 3;

                    if (remaining < len) {
                        // roll back
                        parcel->setDataPosition(dataPos);
                        return OK;
                    }

                    parcel->write(tmpData, len);
                    tmpData += len;
                    remaining -= len;
                }

                // there is a "DisparityBox" after this according to the spec, but we ignore it
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
