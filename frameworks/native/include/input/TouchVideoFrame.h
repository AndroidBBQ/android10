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

#ifndef _LIBINPUT_TOUCHVIDEOFRAME_H
#define _LIBINPUT_TOUCHVIDEOFRAME_H

#include <stdint.h>
#include <sys/time.h>
#include <ui/DisplayInfo.h>
#include <vector>

namespace android {

/**
 * Represents data from a single scan of the touchscreen device.
 * Similar in concept to a video frame, but the touch strength is used as
 * the values instead.
 */
class TouchVideoFrame {
public:
    TouchVideoFrame(uint32_t height, uint32_t width, std::vector<int16_t> data,
            const struct timeval& timestamp);

    bool operator==(const TouchVideoFrame& rhs) const;

    /**
     * Height of the frame
     */
    uint32_t getHeight() const;
    /**
     * Width of the frame
     */
    uint32_t getWidth() const;
    /**
     * The touch strength data.
     * The array is a 2-D row-major matrix, with dimensions (height, width).
     * Total size of the array should equal getHeight() * getWidth().
     * Data is allowed to be negative.
     */
    const std::vector<int16_t>& getData() const;
    /**
     * Time at which the heatmap was taken.
     */
    const struct timeval& getTimestamp() const;

    /**
     * Rotate the video frame.
     * The rotation value is an enum from ui/DisplayInfo.h
     */
    void rotate(int32_t orientation);

private:
    uint32_t mHeight;
    uint32_t mWidth;
    std::vector<int16_t> mData;
    struct timeval mTimestamp;

    /**
     * Common method for 90 degree and 270 degree rotation
     */
    void rotateQuarterTurn(bool clockwise);
    void rotate180();
};

} // namespace android

#endif // _LIBINPUT_TOUCHVIDEOFRAME_H
