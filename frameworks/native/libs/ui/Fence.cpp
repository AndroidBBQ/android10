/*
 * Copyright (C) 2012 The Android Open Source Project
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

#include <ui/Fence.h>

#define LOG_TAG "Fence"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
//#define LOG_NDEBUG 0

// We would eliminate the non-conforming zero-length array, but we can't since
// this is effectively included from the Linux kernel
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wzero-length-array"
#include <sync/sync.h>
#pragma clang diagnostic pop

#include <sys/types.h>
#include <unistd.h>
#include <utils/Log.h>
#include <utils/String8.h>
#include <utils/Trace.h>

namespace android {

const sp<Fence> Fence::NO_FENCE = sp<Fence>(new Fence);

Fence::Fence(int fenceFd) :
    mFenceFd(fenceFd) {
}

Fence::Fence(base::unique_fd fenceFd) :
    mFenceFd(std::move(fenceFd)) {
}

status_t Fence::wait(int timeout) {
    ATRACE_CALL();
    if (mFenceFd == -1) {
        return NO_ERROR;
    }
    int err = sync_wait(mFenceFd, timeout);
    return err < 0 ? -errno : status_t(NO_ERROR);
}

status_t Fence::waitForever(const char* logname) {
    ATRACE_CALL();
    if (mFenceFd == -1) {
        return NO_ERROR;
    }
    int warningTimeout = 3000;
    int err = sync_wait(mFenceFd, warningTimeout);
    if (err < 0 && errno == ETIME) {
        ALOGE("%s: fence %d didn't signal in %u ms", logname, mFenceFd.get(),
                warningTimeout);
        err = sync_wait(mFenceFd, TIMEOUT_NEVER);
    }
    return err < 0 ? -errno : status_t(NO_ERROR);
}

sp<Fence> Fence::merge(const char* name, const sp<Fence>& f1,
        const sp<Fence>& f2) {
    ATRACE_CALL();
    int result;
    // Merge the two fences.  In the case where one of the fences is not a
    // valid fence (e.g. NO_FENCE) we merge the one valid fence with itself so
    // that a new fence with the given name is created.
    if (f1->isValid() && f2->isValid()) {
        result = sync_merge(name, f1->mFenceFd, f2->mFenceFd);
    } else if (f1->isValid()) {
        result = sync_merge(name, f1->mFenceFd, f1->mFenceFd);
    } else if (f2->isValid()) {
        result = sync_merge(name, f2->mFenceFd, f2->mFenceFd);
    } else {
        return NO_FENCE;
    }
    if (result == -1) {
        status_t err = -errno;
        ALOGE("merge: sync_merge(\"%s\", %d, %d) returned an error: %s (%d)",
                name, f1->mFenceFd.get(), f2->mFenceFd.get(),
                strerror(-err), err);
        return NO_FENCE;
    }
    return sp<Fence>(new Fence(result));
}

sp<Fence> Fence::merge(const String8& name, const sp<Fence>& f1,
        const sp<Fence>& f2) {
    return merge(name.string(), f1, f2);
}

int Fence::dup() const {
    return ::dup(mFenceFd);
}

nsecs_t Fence::getSignalTime() const {
    if (mFenceFd == -1) {
        return SIGNAL_TIME_INVALID;
    }

    struct sync_file_info* finfo = sync_file_info(mFenceFd);
    if (finfo == nullptr) {
        ALOGE("sync_file_info returned NULL for fd %d", mFenceFd.get());
        return SIGNAL_TIME_INVALID;
    }
    if (finfo->status != 1) {
        sync_file_info_free(finfo);
        return SIGNAL_TIME_PENDING;
    }

    uint64_t timestamp = 0;
    struct sync_fence_info* pinfo = sync_get_fence_info(finfo);
    for (size_t i = 0; i < finfo->num_fences; i++) {
        if (pinfo[i].timestamp_ns > timestamp) {
            timestamp = pinfo[i].timestamp_ns;
        }
    }

    sync_file_info_free(finfo);
    return nsecs_t(timestamp);
}

size_t Fence::getFlattenedSize() const {
    return 4;
}

size_t Fence::getFdCount() const {
    return isValid() ? 1 : 0;
}

status_t Fence::flatten(void*& buffer, size_t& size, int*& fds, size_t& count) const {
    if (size < getFlattenedSize() || count < getFdCount()) {
        return NO_MEMORY;
    }
    // Cast to uint32_t since the size of a size_t can vary between 32- and
    // 64-bit processes
    FlattenableUtils::write(buffer, size, static_cast<uint32_t>(getFdCount()));
    if (isValid()) {
        *fds++ = mFenceFd;
        count--;
    }
    return NO_ERROR;
}

status_t Fence::unflatten(void const*& buffer, size_t& size, int const*& fds, size_t& count) {
    if (mFenceFd != -1) {
        // Don't unflatten if we already have a valid fd.
        return INVALID_OPERATION;
    }

    if (size < getFlattenedSize()) {
        return NO_MEMORY;
    }

    uint32_t numFds;
    FlattenableUtils::read(buffer, size, numFds);

    if (numFds > 1) {
        return BAD_VALUE;
    }

    if (count < numFds) {
        return NO_MEMORY;
    }

    if (numFds) {
        mFenceFd.reset(*fds++);
        count--;
    }

    return NO_ERROR;
}

} // namespace android
