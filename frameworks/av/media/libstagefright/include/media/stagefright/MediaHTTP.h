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

#ifndef MEDIA_HTTP_H_

#define MEDIA_HTTP_H_

#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/ClearMediaHTTP.h>

namespace android {

struct MediaHTTPConnection;

struct MediaHTTP : public ClearMediaHTTP {
    MediaHTTP(const sp<MediaHTTPConnection> &conn);

protected:
    virtual ~MediaHTTP();

    virtual sp<DecryptHandle> DrmInitialization(const char* mime);

private:
    sp<DecryptHandle> mDecryptHandle;
    DrmManagerClient *mDrmManagerClient;

    void clearDRMState_l();

    DISALLOW_EVIL_CONSTRUCTORS(MediaHTTP);
};

}  // namespace android

#endif  // MEDIA_HTTP_H_
