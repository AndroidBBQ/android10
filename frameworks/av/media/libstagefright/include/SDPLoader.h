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

#ifndef SDP_LOADER_H_

#define SDP_LOADER_H_

#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AHandler.h>
#include <utils/String8.h>

namespace android {

struct HTTPBase;
struct MediaHTTPService;

struct SDPLoader : public AHandler {
    enum Flags {
        // Don't log any URLs.
        kFlagIncognito = 1,
    };
    enum {
        kWhatSDPLoaded = 'sdpl'
    };
    SDPLoader(
            const sp<AMessage> &notify,
            uint32_t flags,
            const sp<MediaHTTPService> &httpService);

    void load(const char* url, const KeyedVector<String8, String8> *headers);

    void cancel();

protected:
    virtual ~SDPLoader() {}

    virtual void onMessageReceived(const sp<AMessage> &msg);

private:
    enum {
        kWhatLoad = 'load',
    };

    void onLoad(const sp<AMessage> &msg);

    sp<AMessage> mNotify;
    const char* mUrl;
    uint32_t mFlags;
    sp<ALooper> mNetLooper;
    bool mCancelled;

    sp<HTTPBase> mHTTPDataSource;

    DISALLOW_EVIL_CONSTRUCTORS(SDPLoader);
};

}  // namespace android

#endif  // SDP_LOADER_H_
