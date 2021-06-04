/*
 * Copyright (C) 2013-2018 The Android Open Source Project
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

#define LOG_TAG "Camera2-ZslProcessor"
#define ATRACE_TAG ATRACE_TAG_CAMERA
//#define LOG_NDEBUG 0
//#define LOG_NNDEBUG 0

#ifdef LOG_NNDEBUG
#define ALOGVV(...) ALOGV(__VA_ARGS__)
#else
#define ALOGVV(...) if (0) ALOGV(__VA_ARGS__)
#endif

#include <inttypes.h>

#include <utils/Log.h>
#include <utils/Trace.h>
#include <gui/Surface.h>

#include "common/CameraDeviceBase.h"
#include "api1/Camera2Client.h"
#include "api1/client2/CaptureSequencer.h"
#include "api1/client2/ZslProcessor.h"
#include "device3/Camera3Device.h"

typedef android::RingBufferConsumer::PinnedBufferItem PinnedBufferItem;

namespace android {
namespace camera2 {

namespace {
struct TimestampFinder : public RingBufferConsumer::RingBufferComparator {
    typedef RingBufferConsumer::BufferInfo BufferInfo;

    enum {
        SELECT_I1 = -1,
        SELECT_I2 = 1,
        SELECT_NEITHER = 0,
    };

    explicit TimestampFinder(nsecs_t timestamp) : mTimestamp(timestamp) {}
    ~TimestampFinder() {}

    template <typename T>
    static void swap(T& a, T& b) {
        T tmp = a;
        a = b;
        b = tmp;
    }

    /**
     * Try to find the best candidate for a ZSL buffer.
     * Match priority from best to worst:
     *  1) Timestamps match.
     *  2) Timestamp is closest to the needle (and lower).
     *  3) Timestamp is closest to the needle (and higher).
     *
     */
    virtual int compare(const BufferInfo *i1,
                        const BufferInfo *i2) const {
        // Try to select non-null object first.
        if (i1 == NULL) {
            return SELECT_I2;
        } else if (i2 == NULL) {
            return SELECT_I1;
        }

        // Best result: timestamp is identical
        if (i1->mTimestamp == mTimestamp) {
            return SELECT_I1;
        } else if (i2->mTimestamp == mTimestamp) {
            return SELECT_I2;
        }

        const BufferInfo* infoPtrs[2] = {
            i1,
            i2
        };
        int infoSelectors[2] = {
            SELECT_I1,
            SELECT_I2
        };

        // Order i1,i2 so that always i1.timestamp < i2.timestamp
        if (i1->mTimestamp > i2->mTimestamp) {
            swap(infoPtrs[0], infoPtrs[1]);
            swap(infoSelectors[0], infoSelectors[1]);
        }

        // Second best: closest (lower) timestamp
        if (infoPtrs[1]->mTimestamp < mTimestamp) {
            return infoSelectors[1];
        } else if (infoPtrs[0]->mTimestamp < mTimestamp) {
            return infoSelectors[0];
        }

        // Worst: closest (higher) timestamp
        return infoSelectors[0];

        /**
         * The above cases should cover all the possibilities,
         * and we get an 'empty' result only if the ring buffer
         * was empty itself
         */
    }

    const nsecs_t mTimestamp;
}; // struct TimestampFinder
} // namespace anonymous

ZslProcessor::ZslProcessor(
    sp<Camera2Client> client,
    wp<CaptureSequencer> sequencer):
        Thread(false),
        mLatestClearedBufferTimestamp(0),
        mState(RUNNING),
        mClient(client),
        mSequencer(sequencer),
        mId(client->getCameraId()),
        mZslStreamId(NO_STREAM),
        mInputStreamId(NO_STREAM),
        mFrameListHead(0),
        mHasFocuser(false),
        mInputBuffer(nullptr),
        mProducer(nullptr),
        mInputProducer(nullptr),
        mInputProducerSlot(-1),
        mBuffersToDetach(0) {
    // Initialize buffer queue and frame list based on pipeline max depth.
    size_t pipelineMaxDepth = kDefaultMaxPipelineDepth;
    if (client != 0) {
        sp<Camera3Device> device =
        static_cast<Camera3Device*>(client->getCameraDevice().get());
        if (device != 0) {
            camera_metadata_ro_entry_t entry =
                device->info().find(ANDROID_REQUEST_PIPELINE_MAX_DEPTH);
            if (entry.count == 1) {
                pipelineMaxDepth = entry.data.u8[0];
            } else {
                ALOGW("%s: Unable to find the android.request.pipelineMaxDepth,"
                        " use default pipeline max depth %d", __FUNCTION__,
                        kDefaultMaxPipelineDepth);
            }

            entry = device->info().find(ANDROID_LENS_INFO_MINIMUM_FOCUS_DISTANCE);
            if (entry.count > 0 && entry.data.f[0] != 0.) {
                mHasFocuser = true;
            }
        }
    }

    ALOGV("%s: Initialize buffer queue and frame list depth based on max pipeline depth (%zu)",
          __FUNCTION__, pipelineMaxDepth);
    // Need to keep buffer queue longer than metadata queue because sometimes buffer arrives
    // earlier than metadata which causes the buffer corresponding to oldest metadata being
    // removed.
    mFrameListDepth = pipelineMaxDepth;
    mBufferQueueDepth = mFrameListDepth + 1;

    mZslQueue.insertAt(0, mBufferQueueDepth);
    mFrameList.insertAt(0, mFrameListDepth);
    sp<CaptureSequencer> captureSequencer = mSequencer.promote();
    if (captureSequencer != 0) captureSequencer->setZslProcessor(this);
}

ZslProcessor::~ZslProcessor() {
    ALOGV("%s: Exit", __FUNCTION__);
    deleteStream();
}

void ZslProcessor::onResultAvailable(const CaptureResult &result) {
    ATRACE_CALL();
    ALOGV("%s:", __FUNCTION__);
    Mutex::Autolock l(mInputMutex);
    camera_metadata_ro_entry_t entry;
    entry = result.mMetadata.find(ANDROID_SENSOR_TIMESTAMP);
    nsecs_t timestamp = entry.data.i64[0];
    if (entry.count == 0) {
        ALOGE("%s: metadata doesn't have timestamp, skip this result", __FUNCTION__);
        return;
    }

    entry = result.mMetadata.find(ANDROID_REQUEST_FRAME_COUNT);
    if (entry.count == 0) {
        ALOGE("%s: metadata doesn't have frame number, skip this result", __FUNCTION__);
        return;
    }
    int32_t frameNumber = entry.data.i32[0];

    ALOGVV("Got preview metadata for frame %d with timestamp %" PRId64, frameNumber, timestamp);

    if (mState != RUNNING) return;

    // Corresponding buffer has been cleared. No need to push into mFrameList
    if (timestamp <= mLatestClearedBufferTimestamp) return;

    mFrameList.editItemAt(mFrameListHead) = result.mMetadata;
    mFrameListHead = (mFrameListHead + 1) % mFrameListDepth;
}

status_t ZslProcessor::updateStream(const Parameters &params) {
    ATRACE_CALL();
    ALOGV("%s: Configuring ZSL streams", __FUNCTION__);
    status_t res;

    Mutex::Autolock l(mInputMutex);

    sp<Camera2Client> client = mClient.promote();
    if (client == 0) {
        ALOGE("%s: Camera %d: Client does not exist", __FUNCTION__, mId);
        return INVALID_OPERATION;
    }
    sp<Camera3Device> device =
        static_cast<Camera3Device*>(client->getCameraDevice().get());
    if (device == 0) {
        ALOGE("%s: Camera %d: Device does not exist", __FUNCTION__, mId);
        return INVALID_OPERATION;
    }

    if (mInputStreamId == NO_STREAM) {
        res = device->createInputStream(params.fastInfo.maxZslSize.width,
            params.fastInfo.maxZslSize.height, HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED,
            &mInputStreamId);
        if (res != OK) {
            ALOGE("%s: Camera %d: Can't create input stream: "
                    "%s (%d)", __FUNCTION__, client->getCameraId(),
                    strerror(-res), res);
            return res;
        }
    }

    if (mZslStreamId == NO_STREAM) {
        // Create stream for HAL production
        // TODO: Sort out better way to select resolution for ZSL

        sp<IGraphicBufferProducer> producer;
        sp<IGraphicBufferConsumer> consumer;
        BufferQueue::createBufferQueue(&producer, &consumer);
        mProducer = new RingBufferConsumer(consumer, GRALLOC_USAGE_HW_CAMERA_ZSL,
            mBufferQueueDepth);
        mProducer->setName(String8("Camera2-ZslRingBufferConsumer"));
        sp<Surface> outSurface = new Surface(producer);

        res = device->createStream(outSurface, params.fastInfo.maxZslSize.width,
            params.fastInfo.maxZslSize.height, HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED,
            HAL_DATASPACE_UNKNOWN, CAMERA3_STREAM_ROTATION_0, &mZslStreamId,
            String8());
        if (res != OK) {
            ALOGE("%s: Camera %d: Can't create ZSL stream: "
                    "%s (%d)", __FUNCTION__, client->getCameraId(),
                    strerror(-res), res);
            return res;
        }
    }

    client->registerFrameListener(Camera2Client::kPreviewRequestIdStart,
            Camera2Client::kPreviewRequestIdEnd,
            this,
            /*sendPartials*/false);

    return OK;
}

status_t ZslProcessor::deleteStream() {
    ATRACE_CALL();
    status_t res;
    sp<Camera3Device> device = nullptr;
    sp<Camera2Client> client = nullptr;

    Mutex::Autolock l(mInputMutex);

    if ((mZslStreamId != NO_STREAM) || (mInputStreamId != NO_STREAM)) {
        client = mClient.promote();
        if (client == 0) {
            ALOGE("%s: Camera %d: Client does not exist", __FUNCTION__, mId);
            return INVALID_OPERATION;
        }

        device =
            reinterpret_cast<Camera3Device*>(client->getCameraDevice().get());
        if (device == 0) {
            ALOGE("%s: Camera %d: Device does not exist", __FUNCTION__, mId);
            return INVALID_OPERATION;
        }
    }

    if (mZslStreamId != NO_STREAM) {
        res = device->deleteStream(mZslStreamId);
        if (res != OK) {
            ALOGE("%s: Camera %d: Cannot delete ZSL output stream %d: "
                    "%s (%d)", __FUNCTION__, client->getCameraId(),
                    mZslStreamId, strerror(-res), res);
            return res;
        }

        mZslStreamId = NO_STREAM;
    }
    if (mInputStreamId != NO_STREAM) {
        res = device->deleteStream(mInputStreamId);
        if (res != OK) {
            ALOGE("%s: Camera %d: Cannot delete input stream %d: "
                    "%s (%d)", __FUNCTION__, client->getCameraId(),
                    mInputStreamId, strerror(-res), res);
            return res;
        }

        mInputStreamId = NO_STREAM;
    }

    if (nullptr != mInputProducer.get()) {
        mInputProducer->disconnect(NATIVE_WINDOW_API_CPU);
        mInputProducer.clear();
    }

    return OK;
}

int ZslProcessor::getStreamId() const {
    Mutex::Autolock l(mInputMutex);
    return mZslStreamId;
}

status_t ZslProcessor::updateRequestWithDefaultStillRequest(CameraMetadata &request) const {
    sp<Camera2Client> client = mClient.promote();
    if (client == 0) {
        ALOGE("%s: Camera %d: Client does not exist", __FUNCTION__, mId);
        return INVALID_OPERATION;
    }
    sp<Camera3Device> device =
        static_cast<Camera3Device*>(client->getCameraDevice().get());
    if (device == 0) {
        ALOGE("%s: Camera %d: Device does not exist", __FUNCTION__, mId);
        return INVALID_OPERATION;
    }

    CameraMetadata stillTemplate;
    device->createDefaultRequest(CAMERA3_TEMPLATE_STILL_CAPTURE, &stillTemplate);

    // Find some of the post-processing tags, and assign the value from template to the request.
    // Only check the aberration mode and noise reduction mode for now, as they are very important
    // for image quality.
    uint32_t postProcessingTags[] = {
            ANDROID_NOISE_REDUCTION_MODE,
            ANDROID_COLOR_CORRECTION_ABERRATION_MODE,
            ANDROID_COLOR_CORRECTION_MODE,
            ANDROID_TONEMAP_MODE,
            ANDROID_SHADING_MODE,
            ANDROID_HOT_PIXEL_MODE,
            ANDROID_EDGE_MODE
    };

    camera_metadata_entry_t entry;
    for (size_t i = 0; i < sizeof(postProcessingTags) / sizeof(uint32_t); i++) {
        entry = stillTemplate.find(postProcessingTags[i]);
        if (entry.count > 0) {
            request.update(postProcessingTags[i], entry.data.u8, 1);
        }
    }

    return OK;
}

void ZslProcessor::notifyInputReleased() {
    Mutex::Autolock l(mInputMutex);

    mBuffersToDetach++;
    mBuffersToDetachSignal.signal();
}

void ZslProcessor::doNotifyInputReleasedLocked() {
    assert(nullptr != mInputBuffer.get());
    assert(nullptr != mInputProducer.get());

    sp<GraphicBuffer> gb;
    sp<Fence> fence;
    auto rc = mInputProducer->detachNextBuffer(&gb, &fence);
    if (NO_ERROR != rc) {
        ALOGE("%s: Failed to detach buffer from input producer: %d",
            __FUNCTION__, rc);
        return;
    }

    BufferItem &item = mInputBuffer->getBufferItem();
    sp<GraphicBuffer> inputBuffer = item.mGraphicBuffer;
    if (gb->handle != inputBuffer->handle) {
        ALOGE("%s: Input mismatch, expected buffer %p received %p", __FUNCTION__,
            inputBuffer->handle, gb->handle);
        return;
    }

    mInputBuffer.clear();
    ALOGV("%s: Memory optimization, clearing ZSL queue",
          __FUNCTION__);
    clearZslResultQueueLocked();

    // Required so we accept more ZSL requests
    mState = RUNNING;
}

void ZslProcessor::InputProducerListener::onBufferReleased() {
    sp<ZslProcessor> parent = mParent.promote();
    if (nullptr != parent.get()) {
        parent->notifyInputReleased();
    }
}

status_t ZslProcessor::pushToReprocess(int32_t requestId) {
    ALOGV("%s: Send in reprocess request with id %d",
            __FUNCTION__, requestId);
    Mutex::Autolock l(mInputMutex);
    status_t res;
    sp<Camera2Client> client = mClient.promote();

    if (client == 0) {
        ALOGE("%s: Camera %d: Client does not exist", __FUNCTION__, mId);
        return INVALID_OPERATION;
    }

    IF_ALOGV() {
        dumpZslQueue(-1);
    }

    size_t metadataIdx;
    nsecs_t candidateTimestamp = getCandidateTimestampLocked(&metadataIdx);

    if (candidateTimestamp == -1) {
        ALOGV("%s: Could not find good candidate for ZSL reprocessing",
              __FUNCTION__);
        return NOT_ENOUGH_DATA;
    } else {
        ALOGV("%s: Found good ZSL candidate idx: %u",
            __FUNCTION__, (unsigned int) metadataIdx);
    }

    if (nullptr == mInputProducer.get()) {
        res = client->getCameraDevice()->getInputBufferProducer(
            &mInputProducer);
        if (res != OK) {
            ALOGE("%s: Camera %d: Unable to retrieve input producer: "
                    "%s (%d)", __FUNCTION__, client->getCameraId(),
                    strerror(-res), res);
            return res;
        }

        IGraphicBufferProducer::QueueBufferOutput output;
        res = mInputProducer->connect(new InputProducerListener(this),
            NATIVE_WINDOW_API_CPU, false, &output);
        if (res != OK) {
            ALOGE("%s: Camera %d: Unable to connect to input producer: "
                    "%s (%d)", __FUNCTION__, client->getCameraId(),
                    strerror(-res), res);
            return res;
        }
    }

    res = enqueueInputBufferByTimestamp(candidateTimestamp,
        /*actualTimestamp*/NULL);
    if (res == NO_BUFFER_AVAILABLE) {
        ALOGV("%s: No ZSL buffers yet", __FUNCTION__);
        return NOT_ENOUGH_DATA;
    } else if (res != OK) {
        ALOGE("%s: Unable to push buffer for reprocessing: %s (%d)",
                __FUNCTION__, strerror(-res), res);
        return res;
    }

    {
        CameraMetadata request = mFrameList[metadataIdx];

        // Verify that the frame is reasonable for reprocessing

        camera_metadata_entry_t entry;
        entry = request.find(ANDROID_CONTROL_AE_STATE);
        if (entry.count == 0) {
            ALOGE("%s: ZSL queue frame has no AE state field!",
                    __FUNCTION__);
            return BAD_VALUE;
        }
        if (entry.data.u8[0] != ANDROID_CONTROL_AE_STATE_CONVERGED &&
                entry.data.u8[0] != ANDROID_CONTROL_AE_STATE_LOCKED) {
            ALOGV("%s: ZSL queue frame AE state is %d, need full capture",
                    __FUNCTION__, entry.data.u8[0]);
            return NOT_ENOUGH_DATA;
        }

        uint8_t requestType = ANDROID_REQUEST_TYPE_REPROCESS;
        res = request.update(ANDROID_REQUEST_TYPE,
                &requestType, 1);
        if (res != OK) {
            ALOGE("%s: Unable to update request type",
                  __FUNCTION__);
            return INVALID_OPERATION;
        }

        int32_t inputStreams[1] =
                { mInputStreamId };
        res = request.update(ANDROID_REQUEST_INPUT_STREAMS,
                inputStreams, 1);
        if (res != OK) {
            ALOGE("%s: Unable to update request input streams",
                  __FUNCTION__);
            return INVALID_OPERATION;
        }

        uint8_t captureIntent =
                static_cast<uint8_t>(ANDROID_CONTROL_CAPTURE_INTENT_STILL_CAPTURE);
        res = request.update(ANDROID_CONTROL_CAPTURE_INTENT,
                &captureIntent, 1);
        if (res != OK ) {
            ALOGE("%s: Unable to update request capture intent",
                  __FUNCTION__);
            return INVALID_OPERATION;
        }

        // TODO: Shouldn't we also update the latest preview frame?
        int32_t outputStreams[1] =
                { client->getCaptureStreamId() };
        res = request.update(ANDROID_REQUEST_OUTPUT_STREAMS,
                outputStreams, 1);
        if (res != OK) {
            ALOGE("%s: Unable to update request output streams",
                  __FUNCTION__);
            return INVALID_OPERATION;
        }

        res = request.update(ANDROID_REQUEST_ID,
                &requestId, 1);
        if (res != OK ) {
            ALOGE("%s: Unable to update frame to a reprocess request",
                  __FUNCTION__);
            return INVALID_OPERATION;
        }

        res = client->stopStream();
        if (res != OK) {
            ALOGE("%s: Camera %d: Unable to stop preview for ZSL capture: "
                "%s (%d)",
                __FUNCTION__, client->getCameraId(), strerror(-res), res);
            return INVALID_OPERATION;
        }

        // Update JPEG settings
        {
            SharedParameters::Lock l(client->getParameters());
            res = l.mParameters.updateRequestJpeg(&request);
            if (res != OK) {
                ALOGE("%s: Camera %d: Unable to update JPEG entries of ZSL "
                        "capture request: %s (%d)", __FUNCTION__,
                        client->getCameraId(),
                        strerror(-res), res);
                return res;
            }
        }

        // Update post-processing settings
        res = updateRequestWithDefaultStillRequest(request);
        if (res != OK) {
            ALOGW("%s: Unable to update post-processing tags, the reprocessed image quality "
                    "may be compromised", __FUNCTION__);
        }

        mLatestCapturedRequest = request;
        res = client->getCameraDevice()->capture(request);
        if (res != OK ) {
            ALOGE("%s: Unable to send ZSL reprocess request to capture: %s"
                  " (%d)", __FUNCTION__, strerror(-res), res);
            return res;
        }

        mState = LOCKED;
    }

    return OK;
}

status_t ZslProcessor::enqueueInputBufferByTimestamp(
        nsecs_t timestamp,
        nsecs_t* actualTimestamp) {

    TimestampFinder timestampFinder = TimestampFinder(timestamp);

    mInputBuffer = mProducer->pinSelectedBuffer(timestampFinder,
        /*waitForFence*/false);

    if (nullptr == mInputBuffer.get()) {
        ALOGE("%s: No ZSL buffers were available yet", __FUNCTION__);
        return NO_BUFFER_AVAILABLE;
    }

    nsecs_t actual = mInputBuffer->getBufferItem().mTimestamp;

    if (actual != timestamp) {
        // TODO: This is problematic, the metadata queue timestamp should
        //       usually have a corresponding ZSL buffer with the same timestamp.
        //       If this is not the case, then it is possible that we will use
        //       a ZSL buffer from a different request, which can result in
        //       side effects during the reprocess pass.
        ALOGW("%s: ZSL buffer candidate search didn't find an exact match --"
              " requested timestamp = %" PRId64 ", actual timestamp = %" PRId64,
              __FUNCTION__, timestamp, actual);
    }

    if (nullptr != actualTimestamp) {
        *actualTimestamp = actual;
    }

    BufferItem &item = mInputBuffer->getBufferItem();
    auto rc = mInputProducer->attachBuffer(&mInputProducerSlot,
        item.mGraphicBuffer);
    if (OK != rc) {
        ALOGE("%s: Failed to attach input ZSL buffer to producer: %d",
            __FUNCTION__, rc);
        return rc;
    }

    IGraphicBufferProducer::QueueBufferOutput output;
    IGraphicBufferProducer::QueueBufferInput input(item.mTimestamp,
            item.mIsAutoTimestamp, item.mDataSpace, item.mCrop,
            item.mScalingMode, item.mTransform, item.mFence);
    rc = mInputProducer->queueBuffer(mInputProducerSlot, input, &output);
    if (OK != rc) {
        ALOGE("%s: Failed to queue ZSL buffer to producer: %d",
            __FUNCTION__, rc);
        return rc;
    }

    return rc;
}

status_t ZslProcessor::clearInputRingBufferLocked(nsecs_t* latestTimestamp) {

    if (nullptr != latestTimestamp) {
        *latestTimestamp = mProducer->getLatestTimestamp();
    }
    mInputBuffer.clear();

    return mProducer->clear();
}

status_t ZslProcessor::clearZslQueue() {
    Mutex::Autolock l(mInputMutex);
    // If in middle of capture, can't clear out queue
    if (mState == LOCKED) return OK;

    return clearZslQueueLocked();
}

status_t ZslProcessor::clearZslQueueLocked() {
    if (NO_STREAM != mZslStreamId) {
        // clear result metadata list first.
        clearZslResultQueueLocked();
        return clearInputRingBufferLocked(&mLatestClearedBufferTimestamp);
    }
    return OK;
}

void ZslProcessor::clearZslResultQueueLocked() {
    mFrameList.clear();
    mFrameListHead = 0;
    mFrameList.insertAt(0, mFrameListDepth);
}

void ZslProcessor::dump(int fd, const Vector<String16>& /*args*/) const {
    Mutex::Autolock l(mInputMutex);
    if (!mLatestCapturedRequest.isEmpty()) {
        String8 result("    Latest ZSL capture request:\n");
        write(fd, result.string(), result.size());
        mLatestCapturedRequest.dump(fd, 2, 6);
    } else {
        String8 result("    Latest ZSL capture request: none yet\n");
        write(fd, result.string(), result.size());
    }
    dumpZslQueue(fd);
}

bool ZslProcessor::threadLoop() {
    Mutex::Autolock l(mInputMutex);

    if (mBuffersToDetach == 0) {
        status_t res = mBuffersToDetachSignal.waitRelative(mInputMutex, kWaitDuration);
        if (res == TIMED_OUT) return true;
    }
    while (mBuffersToDetach > 0) {
        doNotifyInputReleasedLocked();
        mBuffersToDetach--;
    }

    return true;
}

void ZslProcessor::dumpZslQueue(int fd) const {
    String8 header("ZSL queue contents:");
    String8 indent("    ");
    ALOGV("%s", header.string());
    if (fd != -1) {
        header = indent + header + "\n";
        write(fd, header.string(), header.size());
    }
    for (size_t i = 0; i < mZslQueue.size(); i++) {
        const ZslPair &queueEntry = mZslQueue[i];
        nsecs_t bufferTimestamp = queueEntry.buffer.mTimestamp;
        camera_metadata_ro_entry_t entry;
        nsecs_t frameTimestamp = 0;
        int frameAeState = -1;
        if (!queueEntry.frame.isEmpty()) {
            entry = queueEntry.frame.find(ANDROID_SENSOR_TIMESTAMP);
            if (entry.count > 0) frameTimestamp = entry.data.i64[0];
            entry = queueEntry.frame.find(ANDROID_CONTROL_AE_STATE);
            if (entry.count > 0) frameAeState = entry.data.u8[0];
        }
        String8 result =
                String8::format("   %zu: b: %" PRId64 "\tf: %" PRId64 ", AE state: %d", i,
                        bufferTimestamp, frameTimestamp, frameAeState);
        ALOGV("%s", result.string());
        if (fd != -1) {
            result = indent + result + "\n";
            write(fd, result.string(), result.size());
        }

    }
}

bool ZslProcessor::isFixedFocusMode(uint8_t afMode) const {
    switch (afMode) {
        case ANDROID_CONTROL_AF_MODE_AUTO:
        case ANDROID_CONTROL_AF_MODE_CONTINUOUS_VIDEO:
        case ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE:
        case ANDROID_CONTROL_AF_MODE_MACRO:
            return false;
            break;
        case ANDROID_CONTROL_AF_MODE_OFF:
        case ANDROID_CONTROL_AF_MODE_EDOF:
            return true;
        default:
            ALOGE("%s: unknown focus mode %d", __FUNCTION__, afMode);
            return false;
    }
}

nsecs_t ZslProcessor::getCandidateTimestampLocked(size_t* metadataIdx) const {
    /**
     * Find the smallest timestamp we know about so far
     * - ensure that aeState is either converged or locked
     */

    size_t idx = 0;
    nsecs_t minTimestamp = -1;

    size_t emptyCount = mFrameList.size();

    for (size_t j = 0; j < mFrameList.size(); j++) {
        const CameraMetadata &frame = mFrameList[j];
        if (!frame.isEmpty()) {

            emptyCount--;

            camera_metadata_ro_entry_t entry;
            entry = frame.find(ANDROID_SENSOR_TIMESTAMP);
            if (entry.count == 0) {
                ALOGE("%s: Can't find timestamp in frame!",
                        __FUNCTION__);
                continue;
            }
            nsecs_t frameTimestamp = entry.data.i64[0];
            if (minTimestamp > frameTimestamp || minTimestamp == -1) {

                entry = frame.find(ANDROID_CONTROL_AE_STATE);

                if (entry.count == 0) {
                    /**
                     * This is most likely a HAL bug. The aeState field is
                     * mandatory, so it should always be in a metadata packet.
                     */
                    ALOGW("%s: ZSL queue frame has no AE state field!",
                            __FUNCTION__);
                    continue;
                }
                if (entry.data.u8[0] != ANDROID_CONTROL_AE_STATE_CONVERGED &&
                        entry.data.u8[0] != ANDROID_CONTROL_AE_STATE_LOCKED) {
                    ALOGVV("%s: ZSL queue frame AE state is %d, need "
                           "full capture",  __FUNCTION__, entry.data.u8[0]);
                    continue;
                }

                entry = frame.find(ANDROID_CONTROL_AF_MODE);
                if (entry.count == 0) {
                    ALOGW("%s: ZSL queue frame has no AF mode field!",
                            __FUNCTION__);
                    continue;
                }
                // Check AF state if device has focuser and focus mode isn't fixed
                if (mHasFocuser) {
                    uint8_t afMode = entry.data.u8[0];
                    if (!isFixedFocusMode(afMode)) {
                        // Make sure the candidate frame has good focus.
                        entry = frame.find(ANDROID_CONTROL_AF_STATE);
                        if (entry.count == 0) {
                            ALOGW("%s: ZSL queue frame has no AF state field!",
                                    __FUNCTION__);
                            continue;
                        }
                        uint8_t afState = entry.data.u8[0];
                        if (afState != ANDROID_CONTROL_AF_STATE_PASSIVE_FOCUSED &&
                                afState != ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED &&
                                afState != ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED) {
                            ALOGVV("%s: ZSL queue frame AF state is %d is not good for capture,"
                                    " skip it", __FUNCTION__, afState);
                            continue;
                        }
                    }
                }

                minTimestamp = frameTimestamp;
                idx = j;
            }

            ALOGVV("%s: Saw timestamp %" PRId64, __FUNCTION__, frameTimestamp);
        }
    }

    if (emptyCount == mFrameList.size()) {
        /**
         * This could be mildly bad and means our ZSL was triggered before
         * there were any frames yet received by the camera framework.
         *
         * This is a fairly corner case which can happen under:
         * + a user presses the shutter button real fast when the camera starts
         *     (startPreview followed immediately by takePicture).
         * + burst capture case (hitting shutter button as fast possible)
         *
         * If this happens in steady case (preview running for a while, call
         *     a single takePicture) then this might be a fwk bug.
         */
        ALOGW("%s: ZSL queue has no metadata frames", __FUNCTION__);
    }

    ALOGV("%s: Candidate timestamp %" PRId64 " (idx %zu), empty frames: %zu",
          __FUNCTION__, minTimestamp, idx, emptyCount);

    if (metadataIdx) {
        *metadataIdx = idx;
    }

    return minTimestamp;
}

}; // namespace camera2
}; // namespace android
