/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef ANDROID_SERVERS_CAMERA3_STREAMBUFFERFREEDLISTENER_H
#define ANDROID_SERVERS_CAMERA3_STREAMBUFFERFREEDLISTENER_H

#include <gui/Surface.h>
#include <utils/RefBase.h>

namespace android {

namespace camera3 {

class Camera3StreamBufferFreedListener : public virtual RefBase {
public:
    // onBufferFreed is called when a buffer is no longer being managed
    // by this stream. This will not be called in events when all
    // buffers are freed due to stream disconnection.
    //
    // The input handle may be deleted after this callback ends, so attempting
    // to dereference handle post this callback is illegal and might lead to
    // crash.
    //
    // This callback will be called while holding Camera3Stream's lock, so
    // calling into other Camera3Stream APIs within this callback will
    // lead to deadlock.
    virtual void onBufferFreed(int streamId, const native_handle_t* handle) = 0;

    virtual ~Camera3StreamBufferFreedListener() {}
};

}; //namespace camera3
}; //namespace android

#endif
