/*
 * Copyright 2018 The Android Open Source Project
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

#ifndef ANDROID_DATASOURCEDESC_H
#define ANDROID_DATASOURCEDESC_H

#include <media/stagefright/foundation/ABase.h>
#include <utils/RefBase.h>
#include <utils/KeyedVector.h>
#include <utils/String8.h>

namespace android {

class DataSource;
struct MediaHTTPService;

// A binder interface for implementing a stagefright DataSource remotely.
struct DataSourceDesc : public RefBase {
public:
    // intentionally less than INT64_MAX
    // keep consistent with JAVA code
    static const int64_t kMaxTimeMs = 0x7ffffffffffffffll / 1000;
    static const int64_t kMaxTimeUs = kMaxTimeMs * 1000;

    enum {
        /* No data source has been set yet */
        TYPE_NONE     = 0,
        /* data source is type of MediaDataSource */
        TYPE_CALLBACK = 1,
        /* data source is type of FileDescriptor */
        TYPE_FD       = 2,
        /* data source is type of Url */
        TYPE_URL      = 3,
    };

    DataSourceDesc();

    int mType;

    sp<MediaHTTPService> mHttpService;
    String8 mUrl;
    KeyedVector<String8, String8> mHeaders;

    int mFD;
    int64_t mFDOffset;
    int64_t mFDLength;

    sp<DataSource> mCallbackSource;

    int64_t mId;
    int64_t mStartPositionMs;
    int64_t mEndPositionMs;

private:
    DISALLOW_EVIL_CONSTRUCTORS(DataSourceDesc);
};

}; // namespace android

#endif // ANDROID_DATASOURCEDESC_H
