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

#ifndef ANDROID_SERVERS_CAMERA_CAMERA2_ZSLPROCESSOR_H
#define ANDROID_SERVERS_CAMERA_CAMERA2_ZSLPROCESSOR_H

#include <utils/Thread.h>
#include <utils/String16.h>
#include <utils/Vector.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>
#include <gui/BufferItem.h>
#include <gui/BufferItemConsumer.h>
#include <gui/RingBufferConsumer.h>
#include <gui/IProducerListener.h>
#include <camera/CameraMetadata.h>

#include "api1/client2/FrameProcessor.h"

namespace android {

class Camera2Client;

namespace camera2 {

class CaptureSequencer;
struct Parameters;

/***
 * ZSL queue processing for HALv3.0 or newer
 */
class ZslProcessor :
            virtual public Thread,
            virtual public FrameProcessor::FilteredListener {
  public:
    ZslProcessor(sp<Camera2Client> client, wp<CaptureSequencer> sequencer);
    ~ZslProcessor();

    // From FrameProcessor::FilteredListener
    virtual void onResultAvailable(const CaptureResult &result);

    /**
     ****************************************
     * ZslProcessorInterface implementation *
     ****************************************
     */

    // Update the streams by recreating them if the size/format has changed
    status_t updateStream(const Parameters &params);

    // Delete the underlying CameraDevice streams
    status_t deleteStream();

    // Get ID for use with android.request.outputStreams / inputStreams
    int getStreamId() const;

    /**
     * Submits a ZSL capture request (id = requestId)
     *
     * An appropriate ZSL buffer is selected by the closest timestamp,
     * then we push that buffer to be reprocessed by the HAL.
     * A capture request is created and submitted on behalf of the client.
     */
    status_t pushToReprocess(int32_t requestId);

    // Flush the ZSL buffer queue, freeing up all the buffers
    status_t clearZslQueue();

    void dump(int fd, const Vector<String16>& args) const;

  private:

    class InputProducerListener : public BnProducerListener {
    public:
        InputProducerListener(wp<ZslProcessor> parent) : mParent(parent) {}
        virtual void onBufferReleased();
        virtual bool needsReleaseNotify() { return true; }

    private:
        wp<ZslProcessor> mParent;
    };

    static const nsecs_t kWaitDuration = 10000000; // 10 ms
    nsecs_t mLatestClearedBufferTimestamp;

    enum {
        RUNNING,
        LOCKED
    } mState;

    enum { NO_BUFFER_AVAILABLE = BufferQueue::NO_BUFFER_AVAILABLE };

    wp<Camera2Client> mClient;
    wp<CaptureSequencer> mSequencer;

    const int mId;

    mutable Mutex mInputMutex;

    enum {
        NO_STREAM = -1
    };

    int mZslStreamId;
    int mInputStreamId;

    struct ZslPair {
        BufferItem buffer;
        CameraMetadata frame;
    };

    static const int32_t kDefaultMaxPipelineDepth = 4;
    size_t mBufferQueueDepth;
    size_t mFrameListDepth;
    Vector<CameraMetadata> mFrameList;
    size_t mFrameListHead;

    ZslPair mNextPair;

    Vector<ZslPair> mZslQueue;

    CameraMetadata mLatestCapturedRequest;

    bool mHasFocuser;

    // Input buffer queued into HAL
    sp<RingBufferConsumer::PinnedBufferItem> mInputBuffer;
    sp<RingBufferConsumer>                   mProducer;
    sp<IGraphicBufferProducer>               mInputProducer;
    int                                      mInputProducerSlot;

    Condition                                mBuffersToDetachSignal;
    int                                      mBuffersToDetach;

    virtual bool threadLoop();

    status_t clearZslQueueLocked();

    void clearZslResultQueueLocked();

    void dumpZslQueue(int id) const;

    nsecs_t getCandidateTimestampLocked(size_t* metadataIdx) const;

    status_t enqueueInputBufferByTimestamp( nsecs_t timestamp,
        nsecs_t* actualTimestamp);
    status_t clearInputRingBufferLocked(nsecs_t* latestTimestamp);
    void notifyInputReleased();
    void doNotifyInputReleasedLocked();

    bool isFixedFocusMode(uint8_t afMode) const;

    // Update the post-processing metadata with the default still capture request template
    status_t updateRequestWithDefaultStillRequest(CameraMetadata &request) const;
};


}; //namespace camera2
}; //namespace android

#endif
