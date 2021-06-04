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

#include "TouchVideoDevice.h"

#define LOG_TAG "TouchVideoDevice"

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>

#include <android-base/stringprintf.h>
#include <android-base/unique_fd.h>
#include <log/log.h>

using android::base::StringPrintf;
using android::base::unique_fd;

namespace android {

TouchVideoDevice::TouchVideoDevice(int fd, std::string&& name, std::string&& devicePath,
        uint32_t height, uint32_t width,
        const std::array<const int16_t*, NUM_BUFFERS>& readLocations) :
        mFd(fd), mName(std::move(name)), mPath(std::move(devicePath)),
        mHeight(height), mWidth(width),
        mReadLocations(readLocations) {
    mFrames.reserve(MAX_QUEUE_SIZE);
};

std::unique_ptr<TouchVideoDevice> TouchVideoDevice::create(std::string devicePath) {
    unique_fd fd(open(devicePath.c_str(), O_RDWR | O_NONBLOCK));
    if (fd.get() == INVALID_FD) {
        ALOGE("Could not open video device %s: %s", devicePath.c_str(), strerror(errno));
        return nullptr;
    }

    struct v4l2_capability cap;
    int result = ioctl(fd.get(), VIDIOC_QUERYCAP, &cap);
    if (result == -1) {
        ALOGE("VIDIOC_QUERYCAP failed: %s", strerror(errno));
        return nullptr;
    }
    if (!(cap.capabilities & V4L2_CAP_TOUCH)) {
        ALOGE("Capability V4L2_CAP_TOUCH is not present, can't use device for heatmap data. "
                "Make sure device specifies V4L2_CAP_TOUCH");
        return nullptr;
    }
    ALOGI("Opening video device: driver = %s, card = %s, bus_info = %s, version = %i",
            cap.driver, cap.card, cap.bus_info, cap.version);
    std::string name = reinterpret_cast<const char*>(cap.card);

    struct v4l2_input v4l2_input_struct;
    v4l2_input_struct.index = 0;
    result = ioctl(fd.get(), VIDIOC_ENUMINPUT, &v4l2_input_struct);
    if (result == -1) {
        ALOGE("VIDIOC_ENUMINPUT failed: %s", strerror(errno));
        return nullptr;
    }

    if (v4l2_input_struct.type != V4L2_INPUT_TYPE_TOUCH) {
        ALOGE("Video device does not provide touch data. "
                "Make sure device specifies V4L2_INPUT_TYPE_TOUCH.");
        return nullptr;
    }

    struct v4l2_format v4l2_fmt;
    v4l2_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    result = ioctl(fd.get(), VIDIOC_G_FMT, &v4l2_fmt);
    if (result == -1) {
        ALOGE("VIDIOC_G_FMT failed: %s", strerror(errno));
        return nullptr;
    }
    const uint32_t height = v4l2_fmt.fmt.pix.height;
    const uint32_t width = v4l2_fmt.fmt.pix.width;
    ALOGI("Frame dimensions: height = %" PRIu32 " width = %" PRIu32, height, width);

    struct v4l2_requestbuffers req = {};
    req.count = NUM_BUFFERS;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    // req.reserved is zeroed during initialization, which is required per v4l docs
    result = ioctl(fd.get(), VIDIOC_REQBUFS, &req);
    if (result == -1) {
        ALOGE("VIDIOC_REQBUFS failed: %s", strerror(errno));
        return nullptr;
    }
    if (req.count != NUM_BUFFERS) {
        ALOGE("Requested %zu buffers, but driver responded with count=%i", NUM_BUFFERS, req.count);
        return nullptr;
    }

    struct v4l2_buffer buf = {};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    // buf.reserved and buf.reserved2 are zeroed during initialization, required per v4l docs
    std::array<const int16_t*, NUM_BUFFERS> readLocations;
    for (size_t i = 0; i < NUM_BUFFERS; i++) {
        buf.index = i;
        result = ioctl(fd.get(), VIDIOC_QUERYBUF, &buf);
        if (result == -1) {
            ALOGE("VIDIOC_QUERYBUF failed: %s", strerror(errno));
            return nullptr;
        }
        if (buf.length != height * width * sizeof(int16_t)) {
            ALOGE("Unexpected value of buf.length = %i (offset = %" PRIu32 ")",
                    buf.length, buf.m.offset);
            return nullptr;
        }

        readLocations[i] = static_cast<const int16_t*>(mmap(nullptr /* start anywhere */,
                buf.length, PROT_READ /* required */, MAP_SHARED /* recommended */,
                fd.get(), buf.m.offset));
        if (readLocations[i] == MAP_FAILED) {
            ALOGE("%s: map failed: %s", __func__, strerror(errno));
            return nullptr;
        }
    }

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    result = ioctl(fd.get(), VIDIOC_STREAMON, &type);
    if (result == -1) {
        ALOGE("VIDIOC_STREAMON failed: %s", strerror(errno));
        return nullptr;
    }

    for (size_t i = 0; i < NUM_BUFFERS; i++) {
        buf.index = i;
        result = ioctl(fd.get(), VIDIOC_QBUF, &buf);
        if (result == -1) {
            ALOGE("VIDIOC_QBUF failed for buffer %zu: %s", i, strerror(errno));
            return nullptr;
        }
    }
    // Using 'new' to access a non-public constructor.
    return std::unique_ptr<TouchVideoDevice>(new TouchVideoDevice(
            fd.release(), std::move(name), std::move(devicePath), height, width, readLocations));
}

size_t TouchVideoDevice::readAndQueueFrames() {
    std::vector<TouchVideoFrame> frames = readFrames();
    const size_t numFrames = frames.size();
    if (numFrames == 0) {
        // Likely an error occurred
        return 0;
    }
    // Concatenate the vectors, then clip up to maximum size allowed
    mFrames.insert(mFrames.end(), std::make_move_iterator(frames.begin()),
            std::make_move_iterator(frames.end()));
    if (mFrames.size() > MAX_QUEUE_SIZE) {
        ALOGE("More than %zu frames have been accumulated. Dropping %zu frames", MAX_QUEUE_SIZE,
                mFrames.size() - MAX_QUEUE_SIZE);
        mFrames.erase(mFrames.begin(), mFrames.end() - MAX_QUEUE_SIZE);
    }
    return numFrames;
}

std::vector<TouchVideoFrame> TouchVideoDevice::consumeFrames() {
    std::vector<TouchVideoFrame> frames = std::move(mFrames);
    mFrames = {};
    return frames;
}

std::optional<TouchVideoFrame> TouchVideoDevice::readFrame() {
    struct v4l2_buffer buf = {};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    int result = ioctl(mFd.get(), VIDIOC_DQBUF, &buf);
    if (result == -1) {
        // EAGAIN means we've reached the end of the read buffer, so it's expected.
        if (errno != EAGAIN) {
            ALOGE("VIDIOC_DQBUF failed: %s", strerror(errno));
        }
        return std::nullopt;
    }
    if ((buf.flags & V4L2_BUF_FLAG_TIMESTAMP_MASK) != V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC) {
        // We use CLOCK_MONOTONIC for input events, so if the clocks don't match,
        // we can't compare timestamps. Just log a warning, since this is a driver issue
        ALOGW("The timestamp %ld.%ld was not acquired using CLOCK_MONOTONIC",
                buf.timestamp.tv_sec, buf.timestamp.tv_usec);
    }
    std::vector<int16_t> data(mHeight * mWidth);
    const int16_t* readFrom = mReadLocations[buf.index];
    std::copy(readFrom, readFrom + mHeight * mWidth, data.begin());
    TouchVideoFrame frame(mHeight, mWidth, std::move(data), buf.timestamp);

    result = ioctl(mFd.get(), VIDIOC_QBUF, &buf);
    if (result == -1) {
        ALOGE("VIDIOC_QBUF failed: %s", strerror(errno));
    }
    return std::make_optional(std::move(frame));
}

/*
 * This function should not be called unless buffer is ready! This must be checked with
 * select, poll, epoll, or some other similar api first.
 * The oldest frame will be at the beginning of the array.
 */
std::vector<TouchVideoFrame> TouchVideoDevice::readFrames() {
    std::vector<TouchVideoFrame> frames;
    while (true) {
        std::optional<TouchVideoFrame> frame = readFrame();
        if (!frame) {
            break;
        }
        frames.push_back(std::move(*frame));
    }
    return frames;
}

TouchVideoDevice::~TouchVideoDevice() {
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int result = ioctl(mFd.get(), VIDIOC_STREAMOFF, &type);
    if (result == -1) {
        ALOGE("VIDIOC_STREAMOFF failed: %s", strerror(errno));
    }
    for (const int16_t* buffer : mReadLocations) {
        void* bufferAddress = static_cast<void*>(const_cast<int16_t*>(buffer));
        result = munmap(bufferAddress,  mHeight * mWidth * sizeof(int16_t));
        if (result == -1) {
            ALOGE("%s: Couldn't unmap: [%s]", __func__, strerror(errno));
        }
    }
}

std::string TouchVideoDevice::dump() const {
    return StringPrintf("Video device %s (%s) : height=%" PRIu32 ", width=%" PRIu32
            ", fd=%i, hasValidFd=%s",
            mName.c_str(), mPath.c_str(), mHeight, mWidth, mFd.get(),
            hasValidFd() ? "true" : "false");
}

} // namespace android
