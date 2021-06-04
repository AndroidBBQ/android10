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

#ifndef ANDROID_SERVERS_CAMERA_CAMERA2_STREAMINGPROCESSOR_H
#define ANDROID_SERVERS_CAMERA_CAMERA2_STREAMINGPROCESSOR_H

#include <utils/Mutex.h>
#include <utils/String16.h>
#include <gui/BufferItemConsumer.h>

#include "camera/CameraMetadata.h"

namespace android {

class Camera2Client;
class CameraDeviceBase;
class IMemory;

namespace camera2 {

struct Parameters;
class Camera2Heap;

/**
 * Management and processing for preview and recording streams
 */
class StreamingProcessor : public virtual VirtualLightRefBase {
  public:
    explicit StreamingProcessor(sp<Camera2Client> client);
    ~StreamingProcessor();

    status_t setPreviewWindow(const sp<Surface>& window);
    status_t setRecordingWindow(const sp<Surface>& window);

    bool haveValidPreviewWindow() const;
    bool haveValidRecordingWindow() const;

    status_t updatePreviewRequest(const Parameters &params);
    status_t updatePreviewStream(const Parameters &params);
    status_t deletePreviewStream();
    int getPreviewStreamId() const;

    status_t updateRecordingRequest(const Parameters &params);
    // If needsUpdate is set to true, a updateRecordingStream call with params will recreate
    // recording stream
    status_t recordingStreamNeedsUpdate(const Parameters &params, bool *needsUpdate);
    status_t updateRecordingStream(const Parameters &params);
    status_t deleteRecordingStream();
    int getRecordingStreamId() const;

    enum StreamType {
        NONE,
        PREVIEW,
        RECORD
    };
    status_t startStream(StreamType type,
            const Vector<int32_t> &outputStreams);

    // Toggle between paused and unpaused. Stream must be started first.
    status_t togglePauseStream(bool pause);

    status_t stopStream();

    // Returns the request ID for the currently streaming request
    // Returns 0 if there is no active request.
    status_t getActiveRequestId() const;
    status_t incrementStreamingIds();

    status_t dump(int fd, const Vector<String16>& args);

  private:
    mutable Mutex mMutex;

    enum {
        NO_STREAM = -1
    };

    wp<Camera2Client> mClient;
    wp<CameraDeviceBase> mDevice;
    int mId;

    StreamType mActiveRequest;
    bool mPaused;

    Vector<int32_t> mActiveStreamIds;

    // Preview-related members
    int32_t mPreviewRequestId;
    int mPreviewStreamId;
    CameraMetadata mPreviewRequest;
    sp<Surface> mPreviewWindow;

    int32_t mRecordingRequestId;
    int mRecordingStreamId;
    sp<Surface>  mRecordingWindow;
    CameraMetadata mRecordingRequest;
};


}; // namespace camera2
}; // namespace android

#endif
