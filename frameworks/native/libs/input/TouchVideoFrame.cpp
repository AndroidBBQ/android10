/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <input/TouchVideoFrame.h>

namespace android {

TouchVideoFrame::TouchVideoFrame(uint32_t height, uint32_t width, std::vector<int16_t> data,
        const struct timeval& timestamp) :
         mHeight(height), mWidth(width),mData(std::move(data)), mTimestamp(timestamp) {
}

bool TouchVideoFrame::operator==(const TouchVideoFrame& rhs) const {
    return mHeight == rhs.mHeight
            && mWidth == rhs.mWidth
            && mData == rhs.mData
            && mTimestamp.tv_sec == rhs.mTimestamp.tv_sec
            && mTimestamp.tv_usec == rhs.mTimestamp.tv_usec;
}

uint32_t TouchVideoFrame::getHeight() const { return mHeight; }

uint32_t TouchVideoFrame::getWidth() const { return mWidth; }

const std::vector<int16_t>& TouchVideoFrame::getData() const { return mData; }

const struct timeval& TouchVideoFrame::getTimestamp() const { return mTimestamp; }

void TouchVideoFrame::rotate(int32_t orientation) {
    switch (orientation) {
        case DISPLAY_ORIENTATION_90:
            rotateQuarterTurn(false /*clockwise*/);
            break;
        case DISPLAY_ORIENTATION_180:
            rotate180();
            break;
        case DISPLAY_ORIENTATION_270:
            rotateQuarterTurn(true /*clockwise*/);
            break;
    }
}

/**
 * Rotate once clockwise by a quarter turn === rotate 90 degrees
 * Rotate once counterclockwise by a quarter turn === rotate 270 degrees
 * For a clockwise rotation:
 *     An element at position (i, j) is rotated to (j, height - i - 1)
 * For a counterclockwise rotation:
 *     An element at position (i, j) is rotated to (width - j - 1, i)
 */
void TouchVideoFrame::rotateQuarterTurn(bool clockwise) {
    std::vector<int16_t> rotated(mData.size());
    for (size_t i = 0; i < mHeight; i++) {
        for (size_t j = 0; j < mWidth; j++) {
            size_t iRotated, jRotated;
            if (clockwise) {
                iRotated = j;
                jRotated = mHeight - i - 1;
            } else {
                iRotated = mWidth - j - 1;
                jRotated = i;
            }
            size_t indexRotated = iRotated * mHeight + jRotated;
            rotated[indexRotated] = mData[i * mWidth + j];
        }
    }
    mData = std::move(rotated);
    std::swap(mHeight, mWidth);
}

/**
 * An element at position (i, j) is rotated to (height - i - 1, width - j - 1)
 * This is equivalent to moving element [i] to position [height * width - i - 1]
 * Since element at [height * width - i - 1] would move to position [i],
 * we can just swap elements [i] and [height * width - i - 1].
 */
void TouchVideoFrame::rotate180() {
    if (mData.size() == 0) {
        return;
    }
    // Just need to swap elements i and (height * width - 1 - i)
    for (size_t i = 0; i < mData.size() / 2; i++) {
        std::swap(mData[i], mData[mHeight * mWidth - 1 - i]);
    }
}

} // namespace android
