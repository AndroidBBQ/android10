/*
 * Copyright 2018, The Android Open Source Project
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

#ifndef ANDROID_HARDWARE_MEDIA_C2_V1_0_INPUT_SURFACE_CONNECTION_H
#define ANDROID_HARDWARE_MEDIA_C2_V1_0_INPUT_SURFACE_CONNECTION_H

#include <memory>

#include <C2Component.h>
#include <media/stagefright/bqhelper/GraphicBufferSource.h>
#include <media/stagefright/codec2/1.0/InputSurfaceConnection.h>

namespace android {
namespace hardware {
namespace media {
namespace c2 {
namespace V1_0 {
namespace implementation {

// TODO: inherit from IInputSurfaceConnection
class InputSurfaceConnection : public RefBase {
public:
    virtual ~InputSurfaceConnection();

    // From IInputSurfaceConnection
    void disconnect();

private:
    friend class InputSurface;

    // For InputSurface
    InputSurfaceConnection(
            const sp<GraphicBufferSource> &source, const std::shared_ptr<C2Component> &comp);
    bool init();

    InputSurfaceConnection() = delete;

    class Impl;

    sp<GraphicBufferSource> mSource;
    sp<Impl> mImpl;

    DISALLOW_EVIL_CONSTRUCTORS(InputSurfaceConnection);
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace c2
}  // namespace media
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_MEDIA_C2_V1_0_INPUT_SURFACE_CONNECTION_H
