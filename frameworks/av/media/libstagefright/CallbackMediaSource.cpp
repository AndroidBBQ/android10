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

#include <media/stagefright/CallbackMediaSource.h>
#include <media/IMediaSource.h>

namespace android {

CallbackMediaSource::CallbackMediaSource(const sp<IMediaSource> &source)
    :mSource(source) {}

CallbackMediaSource::~CallbackMediaSource() {}

status_t CallbackMediaSource::start(MetaData *params) {
    return mSource->start(params);
}

status_t CallbackMediaSource::stop() {
    return mSource->stop();
}

sp<MetaData> CallbackMediaSource::getFormat() {
    return mSource->getFormat();
}

status_t CallbackMediaSource::read(MediaBufferBase **buffer, const ReadOptions *options) {
    return mSource->read(buffer, reinterpret_cast<const ReadOptions*>(options));
}

status_t CallbackMediaSource::pause() {
    return mSource->pause();
}

}  // namespace android
