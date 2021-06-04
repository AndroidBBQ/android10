/*
 * Copyright 2017 The Android Open Source Project
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

#ifndef MEDIA_HTTP_CONNECTION_H_

#define MEDIA_HTTP_CONNECTION_H_

#include <media/stagefright/foundation/ABase.h>
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <utils/String8.h>

namespace android {

struct MediaHTTPConnection : public virtual RefBase {
    MediaHTTPConnection() {}

    virtual bool connect(
            const char *uri, const KeyedVector<String8, String8> *headers) = 0;

    virtual void disconnect() = 0;
    virtual ssize_t readAt(off64_t offset, void *data, size_t size) = 0;
    virtual off64_t getSize() = 0;
    virtual status_t getMIMEType(String8 *mimeType) = 0;
    virtual status_t getUri(String8 *uri) = 0;

private:
    DISALLOW_EVIL_CONSTRUCTORS(MediaHTTPConnection);
};

}  // namespace android

#endif  // MEDIA_HTTP_CONNECTION_H_
