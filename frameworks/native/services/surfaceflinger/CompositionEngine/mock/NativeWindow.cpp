
/*
 * Copyright 2019 The Android Open Source Project
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

#include "compositionengine/mock/NativeWindow.h"
#include <log/log.h>

namespace android::compositionengine::mock {

static int forwardSetSwapInterval(ANativeWindow* window, int interval) {
    return static_cast<NativeWindow*>(window)->setSwapInterval(interval);
}

static int forwardDequeueBuffer(ANativeWindow* window, ANativeWindowBuffer** buffer, int* fenceFd) {
    return static_cast<NativeWindow*>(window)->dequeueBuffer(buffer, fenceFd);
}

static int forwardCancelBuffer(ANativeWindow* window, ANativeWindowBuffer* buffer, int fenceFd) {
    return static_cast<NativeWindow*>(window)->cancelBuffer(buffer, fenceFd);
}

static int forwardQueueBuffer(ANativeWindow* window, ANativeWindowBuffer* buffer, int fenceFd) {
    return static_cast<NativeWindow*>(window)->queueBuffer(buffer, fenceFd);
}

static int forwardQuery(const ANativeWindow* window, int what, int* value) {
    return static_cast<const NativeWindow*>(window)->query(what, value);
}

static int forwardPerform(ANativeWindow* window, int operation, ...) {
    va_list args;
    va_start(args, operation);
    int result = NO_ERROR;
    switch (operation) {
        case NATIVE_WINDOW_API_CONNECT: {
            int api = va_arg(args, int);
            result = static_cast<NativeWindow*>(window)->connect(api);
            break;
        }
        case NATIVE_WINDOW_SET_BUFFERS_FORMAT: {
            PixelFormat format = va_arg(args, PixelFormat);
            result = static_cast<NativeWindow*>(window)->setBuffersFormat(format);
            break;
        }
        case NATIVE_WINDOW_SET_BUFFERS_DATASPACE: {
            ui::Dataspace dataspace = static_cast<ui::Dataspace>(va_arg(args, int));
            result = static_cast<NativeWindow*>(window)->setBuffersDataSpace(dataspace);
            break;
        }
        case NATIVE_WINDOW_SET_USAGE: {
            // Note: Intentionally widens usage from 32 to 64 bits so we
            // just have one implementation.
            uint64_t usage = va_arg(args, uint32_t);
            result = static_cast<NativeWindow*>(window)->setUsage(usage);
            break;
        }
        case NATIVE_WINDOW_SET_USAGE64: {
            uint64_t usage = va_arg(args, uint64_t);
            result = static_cast<NativeWindow*>(window)->setUsage(usage);
            break;
        }
        case NATIVE_WINDOW_API_DISCONNECT: {
            int api = va_arg(args, int);
            result = static_cast<NativeWindow*>(window)->disconnect(api);
            break;
        }
        default:
            LOG_ALWAYS_FATAL("Unexpected operation %d", operation);
            break;
    }

    va_end(args);
    return result;
}

static int forwardDequeueBufferDeprecated(ANativeWindow* window, ANativeWindowBuffer** buffer) {
    int ignoredFenceFd = -1;
    return static_cast<NativeWindow*>(window)->dequeueBuffer(buffer, &ignoredFenceFd);
}

static int forwardCancelBufferDeprecated(ANativeWindow* window, ANativeWindowBuffer* buffer) {
    return static_cast<NativeWindow*>(window)->cancelBuffer(buffer, -1);
}

static int forwardLockBufferDeprecated(ANativeWindow* window, ANativeWindowBuffer* buffer) {
    return static_cast<NativeWindow*>(window)->lockBuffer_DEPRECATED(buffer);
}

static int forwardQueueBufferDeprecated(ANativeWindow* window, ANativeWindowBuffer* buffer) {
    return static_cast<NativeWindow*>(window)->queueBuffer(buffer, -1);
}

NativeWindow::NativeWindow() {
    ANativeWindow::setSwapInterval = &forwardSetSwapInterval;
    ANativeWindow::dequeueBuffer = &forwardDequeueBuffer;
    ANativeWindow::cancelBuffer = &forwardCancelBuffer;
    ANativeWindow::queueBuffer = &forwardQueueBuffer;
    ANativeWindow::query = &forwardQuery;
    ANativeWindow::perform = &forwardPerform;

    ANativeWindow::dequeueBuffer_DEPRECATED = &forwardDequeueBufferDeprecated;
    ANativeWindow::cancelBuffer_DEPRECATED = &forwardCancelBufferDeprecated;
    ANativeWindow::lockBuffer_DEPRECATED = &forwardLockBufferDeprecated;
    ANativeWindow::queueBuffer_DEPRECATED = &forwardQueueBufferDeprecated;
}
NativeWindow::~NativeWindow() = default;

} // namespace android::compositionengine::mock
