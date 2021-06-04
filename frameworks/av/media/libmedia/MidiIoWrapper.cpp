/*
 * Copyright (C) 2014 The Android Open Source Project
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
#define LOG_TAG "MidiIoWrapper"
#include <utils/Log.h>
#include <utils/RefBase.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <media/MidiIoWrapper.h>
#include <media/MediaExtractorPluginApi.h>

static int readAt(void *handle, void *buffer, int pos, int size) {
    return ((android::MidiIoWrapper*)handle)->readAt(buffer, pos, size);
}
static int size(void *handle) {
    return ((android::MidiIoWrapper*)handle)->size();
}

namespace android {

MidiIoWrapper::MidiIoWrapper(const char *path) {
    ALOGV("MidiIoWrapper(%s)", path);
    mFd = open(path, O_RDONLY | O_LARGEFILE);
    mBase = 0;
    mLength = lseek(mFd, 0, SEEK_END);
    mDataSource = nullptr;
}

MidiIoWrapper::MidiIoWrapper(int fd, off64_t offset, int64_t size) {
    ALOGV("MidiIoWrapper(fd=%d)", fd);
    mFd = fd < 0 ? -1 : dup(fd);
    mBase = offset;
    mLength = size;
    mDataSource = nullptr;
}

class DataSourceUnwrapper : public DataSourceBase {

public:
    explicit DataSourceUnwrapper(CDataSource *csource) {
        mSource = csource;
    }

    virtual ~DataSourceUnwrapper() {}

    virtual status_t initCheck() const { return OK; }

    // Returns the number of bytes read, or -1 on failure. It's not an error if
    // this returns zero; it just means the given offset is equal to, or
    // beyond, the end of the source.
    virtual ssize_t readAt(off64_t offset, void *data, size_t size) {
        return mSource->readAt(mSource->handle, offset, data, size);
    }

    // May return ERROR_UNSUPPORTED.
    virtual status_t getSize(off64_t *size) {
        return mSource->getSize(mSource->handle, size);
    }

    virtual bool getUri(char * /*uriString*/, size_t /*bufferSize*/) {
        return false;
    }

    virtual uint32_t flags() {
        return 0;
    }

    virtual void close() {};
private:
    CDataSource *mSource;
};

MidiIoWrapper::MidiIoWrapper(CDataSource *csource) {
    ALOGV("MidiIoWrapper(CDataSource)");
    mFd = -1;
    mBase = 0;
    mDataSource = new DataSourceUnwrapper(csource);
    off64_t l;
    if (mDataSource->getSize(&l) == OK) {
        mLength = l;
    } else {
        mLength = 0;
    }
}

MidiIoWrapper::~MidiIoWrapper() {
    ALOGV("~MidiIoWrapper");
    if (mFd >= 0) {
        close(mFd);
    }
    delete mDataSource;
}

int MidiIoWrapper::readAt(void *buffer, int offset, int size) {
    ALOGV("readAt(%p, %d, %d)", buffer, offset, size);

    if (mDataSource != NULL) {
        return mDataSource->readAt(offset, buffer, size);
    }
    if (mFd < 0) {
        errno = EBADF;
        return -1; // as per failed read.
    }
    lseek(mFd, mBase + offset, SEEK_SET);
    if (offset + size > mLength) {
        size = mLength - offset;
    }
    return read(mFd, buffer, size);
}

int MidiIoWrapper::size() {
    ALOGV("size() = %d", int(mLength));
    return mLength;
}

EAS_FILE_LOCATOR MidiIoWrapper::getLocator() {
    mEasFile.handle = this;
    mEasFile.readAt = ::readAt;
    mEasFile.size = ::size;
    return &mEasFile;
}

}  // namespace android
