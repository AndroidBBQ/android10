/*
 * Copyright (C) 2016 The Android Open Source Project
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


#include "BufLog.h"
#define LOG_TAG "BufLog"
//#define LOG_NDEBUG 0

#include <errno.h>
#include "log/log.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <audio_utils/string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

// ------------------------------
// BufLogSingleton
// ------------------------------
pthread_once_t onceControl = PTHREAD_ONCE_INIT;

BufLog *BufLogSingleton::mInstance = NULL;

void BufLogSingleton::initOnce() {
    mInstance = new BufLog();
    ALOGW("=====================================\n" \
            "Warning: BUFLOG is defined in some part of your code.\n" \
            "This will create large audio dumps in %s.\n" \
            "=====================================\n", BUFLOG_BASE_PATH);
}

BufLog *BufLogSingleton::instance() {
    pthread_once(&onceControl, initOnce);
    return mInstance;
}

bool BufLogSingleton::instanceExists() {
    return mInstance != NULL;
}

// ------------------------------
// BufLog
// ------------------------------

BufLog::BufLog() {
    memset(mStreams, 0, sizeof(mStreams));
}

BufLog::~BufLog() {
    android::Mutex::Autolock autoLock(mLock);

    for (unsigned int id = 0; id < BUFLOG_MAXSTREAMS; id++) {
        BufLogStream *pBLStream = mStreams[id];
        if (pBLStream != NULL) {
            delete pBLStream ;
            mStreams[id] = NULL;
        }
    }
}

size_t BufLog::write(int streamid, const char *tag, int format, int channels,
        int samplingRate, size_t maxBytes, const void *buf, size_t size) {
    unsigned int id = streamid % BUFLOG_MAXSTREAMS;
    android::Mutex::Autolock autoLock(mLock);

    BufLogStream *pBLStream = mStreams[id];

    if (pBLStream == NULL) {
        pBLStream = mStreams[id] = new BufLogStream(id, tag, format, channels,
                samplingRate, maxBytes);
        ALOG_ASSERT(pBLStream != NULL, "BufLogStream Failed to be created");
    }

    return pBLStream->write(buf, size);
}

void BufLog::reset() {
    android::Mutex::Autolock autoLock(mLock);
    ALOGV("Resetting all BufLogs");
    int count = 0;

    for (unsigned int id = 0; id < BUFLOG_MAXSTREAMS; id++) {
        BufLogStream *pBLStream = mStreams[id];
        if (pBLStream != NULL) {
            delete pBLStream;
            mStreams[id] = NULL;
            count++;
        }
    }
    ALOGV("Reset %d BufLogs", count);
}

// ------------------------------
// BufLogStream
// ------------------------------

BufLogStream::BufLogStream(unsigned int id,
        const char *tag,
        unsigned int format,
        unsigned int channels,
        unsigned int samplingRate,
        size_t maxBytes = 0) : mId(id), mFormat(format), mChannels(channels),
                mSamplingRate(samplingRate), mMaxBytes(maxBytes) {
    mByteCount = 0;
    mPaused = false;
    if (tag != NULL) {
        (void)audio_utils_strlcpy(mTag, tag);
    } else {
        mTag[0] = 0;
    }
    ALOGV("Creating BufLogStream id:%d tag:%s format:%#x ch:%d sr:%d maxbytes:%zu", mId, mTag,
            mFormat, mChannels, mSamplingRate, mMaxBytes);

    //open file (s), info about tag, format, etc.
    //timestamp
    char timeStr[16];   //size 16: format %Y%m%d%H%M%S 14 chars + string null terminator
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    strftime(timeStr, sizeof(timeStr), "%Y%m%d%H%M%S", &tm);
    char logPath[BUFLOG_MAX_PATH_SIZE];
    snprintf(logPath, BUFLOG_MAX_PATH_SIZE, "%s/%s_%d_%s_%d_%d_%d.raw", BUFLOG_BASE_PATH, timeStr,
            mId, mTag, mFormat, mChannels, mSamplingRate);
    ALOGV("data output: %s", logPath);

    mFile = fopen(logPath, "wb");
    if (mFile != NULL) {
        ALOGV("Success creating file at: %p", mFile);
    } else {
        ALOGE("Error: could not create file BufLogStream %s", strerror(errno));
    }
}

void BufLogStream::closeStream_l() {
    ALOGV("Closing BufLogStream id:%d tag:%s", mId, mTag);
    if (mFile != NULL) {
        fclose(mFile);
        mFile = NULL;
    }
}

BufLogStream::~BufLogStream() {
    ALOGV("Destroying BufLogStream id:%d tag:%s", mId, mTag);
    android::Mutex::Autolock autoLock(mLock);
    closeStream_l();
}

size_t BufLogStream::write(const void *buf, size_t size) {

    size_t bytes = 0;
    if (!mPaused && mFile != NULL) {
        if (size > 0 && buf != NULL) {
            android::Mutex::Autolock autoLock(mLock);
            if (mMaxBytes > 0) {
                size = MIN(size, mMaxBytes - mByteCount);
            }
            bytes = fwrite(buf, 1, size, mFile);
            mByteCount += bytes;
            if (mMaxBytes > 0 && mMaxBytes == mByteCount) {
                closeStream_l();
            }
        }
        ALOGV("wrote %zu/%zu bytes to BufLogStream %d tag:%s. Total Bytes: %zu", bytes, size, mId,
                mTag, mByteCount);
    } else {
        ALOGV("Warning: trying to write to %s BufLogStream id:%d tag:%s",
                mPaused ? "paused" : "closed", mId, mTag);
    }
    return bytes;
}

bool BufLogStream::setPause(bool pause) {
    bool old = mPaused;
    mPaused = pause;
    return old;
}

void BufLogStream::finalize() {
    android::Mutex::Autolock autoLock(mLock);
    closeStream_l();
}
