/*
 * Copyright (C) 2009 The Android Open Source Project
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

#ifndef FILE_SOURCE_H_

#define FILE_SOURCE_H_

#include <stdio.h>

#include <media/stagefright/ClearFileSource.h>
#include <media/stagefright/MediaErrors.h>
#include <utils/threads.h>
#include <drm/DrmManagerClient.h>

namespace android {

class FileSource : public ClearFileSource {
public:
    FileSource(const char *filename);
    // FileSource takes ownership and will close the fd
    FileSource(int fd, int64_t offset, int64_t length);

    virtual ssize_t readAt(off64_t offset, void *data, size_t size);

    virtual sp<DecryptHandle> DrmInitialization(const char *mime);

    static bool requiresDrm(int fd, int64_t offset, int64_t length, const char *mime);

protected:
    virtual ~FileSource();

private:
    /*for DRM*/
    sp<DecryptHandle> mDecryptHandle;
    DrmManagerClient *mDrmManagerClient;
    int64_t mDrmBufOffset;
    ssize_t mDrmBufSize;
    unsigned char *mDrmBuf;

    ssize_t readAtDRM_l(off64_t offset, void *data, size_t size);

    FileSource(const FileSource &);
    FileSource &operator=(const FileSource &);
};

}  // namespace android

#endif  // FILE_SOURCE_H_

