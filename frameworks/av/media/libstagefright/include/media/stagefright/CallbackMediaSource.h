/*
 * Copyright 2017, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CALLBACK_MEDIA_SOURCE_H_
#define CALLBACK_MEDIA_SOURCE_H_

#include <media/MediaSource.h>
#include <media/stagefright/foundation/ABase.h>

namespace android {

class IMediaSource;

// A stagefright MediaSource that wraps a binder IMediaSource.
class CallbackMediaSource : public MediaSource {
public:
    explicit CallbackMediaSource(const sp<IMediaSource> &source);
    virtual ~CallbackMediaSource();
    virtual status_t start(MetaData *params = NULL);
    virtual status_t stop();
    virtual sp<MetaData> getFormat();
    virtual status_t read(
            MediaBufferBase **buffer, const ReadOptions *options = NULL);
    virtual status_t pause();

private:
    sp<IMediaSource> mSource;

    DISALLOW_EVIL_CONSTRUCTORS(CallbackMediaSource);
};

}  // namespace android

#endif  // CALLBACK_MEDIA_SOURCE_H_
