/*
 * Copyright (C) 2010 The Android Open Source Project
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

#ifndef THROTTLED_SOURCE_H_

#define THROTTLED_SOURCE_H_

#include <media/DataSource.h>
#include <utils/threads.h>

namespace android {

struct ThrottledSource : public DataSource {
    ThrottledSource(
            const sp<DataSource> &source,
            int32_t bandwidthLimitBytesPerSecond);

    // implementation of readAt() that sleeps to achieve the desired max throughput
    virtual ssize_t readAt(off64_t offset, void *data, size_t size);

    // returns an empty string to prevent callers from using the Uri to construct a new datasource
    virtual String8 getUri() {
        return String8();
    }

    // following methods all call through to the wrapped DataSource's methods

    status_t initCheck() const {
        return mSource->initCheck();
    }

    virtual status_t getSize(off64_t *size) {
        return mSource->getSize(size);
    }

    virtual uint32_t flags() {
        return mSource->flags();
    }

    virtual status_t reconnectAtOffset(off64_t offset) {
        return mSource->reconnectAtOffset(offset);
    }

    virtual sp<DecryptHandle> DrmInitialization(const char *mime = NULL) {
        return mSource->DrmInitialization(mime);
    }

    virtual String8 getMIMEType() const {
        return mSource->getMIMEType();
    }

private:
    Mutex mLock;

    sp<DataSource> mSource;
    int32_t mBandwidthLimitBytesPerSecond;
    int64_t mStartTimeUs;
    size_t mTotalTransferred;

    ThrottledSource(const ThrottledSource &);
    ThrottledSource &operator=(const ThrottledSource &);
};

}  // namespace android

#endif  // THROTTLED_SOURCE_H_
