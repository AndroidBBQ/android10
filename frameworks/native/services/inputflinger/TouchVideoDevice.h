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

#ifndef _INPUTFLINGER_TOUCH_VIDEO_DEVICE_H
#define _INPUTFLINGER_TOUCH_VIDEO_DEVICE_H

#include <array>
#include <android-base/unique_fd.h>
#include <input/TouchVideoFrame.h>
#include <optional>
#include <stdint.h>
#include <string>
#include <vector>

namespace android {

/**
 * Represents a video device that uses v4l2 api to report touch heatmap data.
 */
class TouchVideoDevice {
public:
    /**
     * Create a new TouchVideoDevice for the path provided.
     * Return nullptr upon failure.
     */
    static std::unique_ptr<TouchVideoDevice> create(std::string devicePath);
    ~TouchVideoDevice();

    bool hasValidFd() const { return mFd.get() != INVALID_FD; }
    /**
     * Obtain the file descriptor associated with this video device.
     * Could be used for adding to epoll.
     */
    int getFd() const { return mFd.get(); }
    /**
     * Get the name of this video device.
     */
    const std::string& getName() const { return mName; }
    /**
     * Get the file path of this video device.
     */
    const std::string& getPath() const { return mPath; }
    /**
     * Get the height of the heatmap frame
     */
    uint32_t getHeight() const { return mHeight; }
    /**
     * Get the width of the heatmap frame
     */
    uint32_t getWidth() const { return mWidth; }
    /**
     * Direct read of the frame. Stores the frame into internal buffer.
     * Return the number of frames that were successfully read.
     *
     * This function should not be called unless buffer is ready!
     * This must be checked with select, poll, epoll, or similar api first.
     * If epoll indicates that there is data ready to read, but this function
     * returns zero, then it is likely an error occurred.
     */
    size_t readAndQueueFrames();
    /**
     * Return all of the queued frames, and erase them from the local buffer.
     * The returned frames are in the order that they were received from the
     * v4l2 device, with the oldest frame at the index 0.
     */
    std::vector<TouchVideoFrame> consumeFrames();
    /**
     * Get string representation of this video device.
     */
    std::string dump() const;

private:
    android::base::unique_fd mFd;
    std::string mName;
    std::string mPath;

    uint32_t mHeight;
    uint32_t mWidth;

    static constexpr int INVALID_FD = -1;
    /**
     * How many buffers to request for heatmap.
     * The kernel driver will be allocating these buffers for us,
     * and will provide memory locations to read these from.
     */
    static constexpr size_t NUM_BUFFERS = 3;
    std::array<const int16_t*, NUM_BUFFERS> mReadLocations;
    /**
     * How many buffers to keep for the internal queue. When the internal buffer
     * exceeds this capacity, oldest frames will be dropped.
     */
    static constexpr size_t MAX_QUEUE_SIZE = 10;
    std::vector<TouchVideoFrame> mFrames;

    /**
     * The constructor is private because opening a v4l2 device requires many checks.
     * To get a new TouchVideoDevice, use 'create' instead.
     */
    explicit TouchVideoDevice(int fd, std::string&& name, std::string&& devicePath,
            uint32_t height, uint32_t width,
            const std::array<const int16_t*, NUM_BUFFERS>& readLocations);
    /**
     * Read all currently available frames.
     */
    std::vector<TouchVideoFrame> readFrames();
    /**
     * Read a single frame. May return nullopt if no data is currently available for reading.
     */
    std::optional<TouchVideoFrame> readFrame();
};
} // namespace android
#endif //_INPUTFLINGER_TOUCH_VIDEO_DEVICE_H
