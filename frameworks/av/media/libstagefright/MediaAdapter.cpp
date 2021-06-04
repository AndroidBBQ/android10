/*
 * Copyright (C) 2013 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "MediaAdapter"
#include <utils/Log.h>

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaAdapter.h>
#include <media/stagefright/MediaBuffer.h>

namespace android {

MediaAdapter::MediaAdapter(const sp<MetaData> &meta)
    : mCurrentMediaBuffer(NULL),
      mStarted(false),
      mOutputFormat(meta) {
}

MediaAdapter::~MediaAdapter() {
    Mutex::Autolock autoLock(mAdapterLock);
    mOutputFormat.clear();
    CHECK(mCurrentMediaBuffer == NULL);
}

status_t MediaAdapter::start(MetaData * /* params */) {
    Mutex::Autolock autoLock(mAdapterLock);
    if (!mStarted) {
        mStarted = true;
    }
    return OK;
}

status_t MediaAdapter::stop() {
    MediaBuffer *currentBuffer = NULL;
    {
        Mutex::Autolock autoLock(mAdapterLock);
        if (mStarted) {
            mStarted = false;
            // If stop() happens immediately after a pushBuffer(), we should
            // clean up the mCurrentMediaBuffer. But need to release without
            // the lock as signalBufferReturned() will acquire the lock.
            currentBuffer = mCurrentMediaBuffer;
            mCurrentMediaBuffer = NULL;

            // While read() is still waiting, we should signal it to finish.
            mBufferReadCond.signal();
        }
    }
    if (currentBuffer != NULL) {
        currentBuffer->release();
        currentBuffer = NULL;
    }
    return OK;
}

sp<MetaData> MediaAdapter::getFormat() {
    Mutex::Autolock autoLock(mAdapterLock);
    return mOutputFormat;
}

void MediaAdapter::signalBufferReturned(MediaBufferBase *buffer) {
    Mutex::Autolock autoLock(mAdapterLock);
    CHECK(buffer != NULL);
    buffer->setObserver(0);
    buffer->release();
    ALOGV("buffer returned %p", buffer);
    mBufferReturnedCond.signal();
}

status_t MediaAdapter::read(
            MediaBufferBase **buffer, const ReadOptions * /* options */) {
    Mutex::Autolock autoLock(mAdapterLock);
    if (!mStarted) {
        ALOGV("Read before even started!");
        return ERROR_END_OF_STREAM;
    }

    while (mCurrentMediaBuffer == NULL && mStarted) {
        ALOGV("waiting @ read()");
        mBufferReadCond.wait(mAdapterLock);
    }

    if (!mStarted) {
        ALOGV("read interrupted after stop");
        CHECK(mCurrentMediaBuffer == NULL);
        return ERROR_END_OF_STREAM;
    }

    CHECK(mCurrentMediaBuffer != NULL);

    *buffer = mCurrentMediaBuffer;
    mCurrentMediaBuffer = NULL;

    return OK;
}

status_t MediaAdapter::pushBuffer(MediaBuffer *buffer) {
    if (buffer == NULL) {
        ALOGE("pushBuffer get an NULL buffer");
        return -EINVAL;
    }

    Mutex::Autolock autoLock(mAdapterLock);
    if (!mStarted) {
        ALOGE("pushBuffer called before start");
        return INVALID_OPERATION;
    }
    mCurrentMediaBuffer = buffer;
    mCurrentMediaBuffer->setObserver(this);
    mBufferReadCond.signal();

    ALOGV("wait for the buffer returned @ pushBuffer! %p", buffer);
    mBufferReturnedCond.wait(mAdapterLock);

    return OK;
}

}  // namespace android

