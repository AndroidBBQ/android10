/*
 * Copyright (C) 2014 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "MediaFilter"

#include <inttypes.h>
#include <utils/Trace.h>

#include <binder/MemoryDealer.h>

#include <media/stagefright/BufferProducerWrapper.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>

#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MediaFilter.h>

#include <media/MediaCodecBuffer.h>

#include <gui/BufferItem.h>

#include "ColorConvert.h"
#include "GraphicBufferListener.h"
#include "IntrinsicBlurFilter.h"
#include "RSFilter.h"
#include "SaturationFilter.h"
#include "ZeroFilter.h"

#include "../include/ACodecBufferChannel.h"
#include "../include/SharedMemoryBuffer.h"

namespace android {

// parameter: number of input and output buffers
static const size_t kBufferCountActual = 4;

MediaFilter::MediaFilter()
    : mState(UNINITIALIZED),
      mGeneration(0),
      mGraphicBufferListener(NULL) {
    mBufferChannel = std::make_shared<ACodecBufferChannel>(
            new AMessage(kWhatInputBufferFilled, this),
            new AMessage(kWhatOutputBufferDrained, this));
}

MediaFilter::~MediaFilter() {
}

//////////////////// PUBLIC FUNCTIONS //////////////////////////////////////////

std::shared_ptr<BufferChannelBase> MediaFilter::getBufferChannel() {
    return mBufferChannel;
}

void MediaFilter::initiateAllocateComponent(const sp<AMessage> &msg) {
    msg->setWhat(kWhatAllocateComponent);
    msg->setTarget(this);
    msg->post();
}

void MediaFilter::initiateConfigureComponent(const sp<AMessage> &msg) {
    msg->setWhat(kWhatConfigureComponent);
    msg->setTarget(this);
    msg->post();
}

void MediaFilter::initiateCreateInputSurface() {
    (new AMessage(kWhatCreateInputSurface, this))->post();
}

void MediaFilter::initiateSetInputSurface(
        const sp<PersistentSurface> & /* surface */) {
    ALOGW("initiateSetInputSurface() unsupported");
}

void MediaFilter::initiateStart() {
    (new AMessage(kWhatStart, this))->post();
}

void MediaFilter::initiateShutdown(bool keepComponentAllocated) {
    sp<AMessage> msg = new AMessage(kWhatShutdown, this);
    msg->setInt32("keepComponentAllocated", keepComponentAllocated);
    msg->post();
}

void MediaFilter::signalFlush() {
    (new AMessage(kWhatFlush, this))->post();
}

void MediaFilter::signalResume() {
    (new AMessage(kWhatResume, this))->post();
}

// nothing to do
void MediaFilter::signalRequestIDRFrame() {
    return;
}

void MediaFilter::signalSetParameters(const sp<AMessage> &params) {
    sp<AMessage> msg = new AMessage(kWhatSetParameters, this);
    msg->setMessage("params", params);
    msg->post();
}

void MediaFilter::signalEndOfInputStream() {
    (new AMessage(kWhatSignalEndOfInputStream, this))->post();
}

void MediaFilter::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatAllocateComponent:
        {
            onAllocateComponent(msg);
            break;
        }
        case kWhatConfigureComponent:
        {
            onConfigureComponent(msg);
            break;
        }
        case kWhatStart:
        {
            onStart();
            break;
        }
        case kWhatProcessBuffers:
        {
            processBuffers();
            break;
        }
        case kWhatInputBufferFilled:
        {
            onInputBufferFilled(msg);
            break;
        }
        case kWhatOutputBufferDrained:
        {
            onOutputBufferDrained(msg);
            break;
        }
        case kWhatShutdown:
        {
            onShutdown(msg);
            break;
        }
        case kWhatFlush:
        {
            onFlush();
            break;
        }
        case kWhatResume:
        {
            // nothing to do
            break;
        }
        case kWhatSetParameters:
        {
            onSetParameters(msg);
            break;
        }
        case kWhatCreateInputSurface:
        {
            onCreateInputSurface();
            break;
        }
        case GraphicBufferListener::kWhatFrameAvailable:
        {
            onInputFrameAvailable();
            break;
        }
        case kWhatSignalEndOfInputStream:
        {
            onSignalEndOfInputStream();
            break;
        }
        default:
        {
            ALOGE("Message not handled:\n%s", msg->debugString().c_str());
            break;
        }
    }
}

//////////////////// HELPER FUNCTIONS //////////////////////////////////////////

void MediaFilter::signalProcessBuffers() {
    (new AMessage(kWhatProcessBuffers, this))->post();
}

void MediaFilter::signalError(status_t error) {
    mCallback->onError(error, ACTION_CODE_FATAL);
}

status_t MediaFilter::allocateBuffersOnPort(OMX_U32 portIndex) {
    CHECK(portIndex == kPortIndexInput || portIndex == kPortIndexOutput);
    const bool isInput = portIndex == kPortIndexInput;
    const size_t bufferSize = isInput ? mMaxInputSize : mMaxOutputSize;

    CHECK(mDealer[portIndex] == NULL);
    CHECK(mBuffers[portIndex].isEmpty());

    ALOGV("Allocating %zu buffers of size %zu on %s port",
            kBufferCountActual, bufferSize,
            isInput ? "input" : "output");

    size_t totalSize = kBufferCountActual * bufferSize;

    mDealer[portIndex] = new MemoryDealer(totalSize, "MediaFilter");

    for (size_t i = 0; i < kBufferCountActual; ++i) {
        sp<IMemory> mem = mDealer[portIndex]->allocate(bufferSize);
        CHECK(mem.get() != NULL);

        BufferInfo info;
        info.mStatus = BufferInfo::OWNED_BY_US;
        info.mBufferID = i;
        info.mGeneration = mGeneration;
        info.mOutputFlags = 0;
        info.mData = new SharedMemoryBuffer(
                isInput ? mInputFormat : mOutputFormat, mem);
        info.mData->meta()->setInt64("timeUs", 0);

        mBuffers[portIndex].push_back(info);

        if (!isInput) {
            mAvailableOutputBuffers.push(
                    &mBuffers[portIndex].editItemAt(i));
        }
    }

    std::vector<ACodecBufferChannel::BufferAndId> array(mBuffers[portIndex].size());
    for (size_t i = 0; i < mBuffers[portIndex].size(); ++i) {
        array[i] = {mBuffers[portIndex][i].mData, mBuffers[portIndex][i].mBufferID};
    }
    if (portIndex == kPortIndexInput) {
        mBufferChannel->setInputBufferArray(array);
    } else {
        mBufferChannel->setOutputBufferArray(array);
    }

    return OK;
}

MediaFilter::BufferInfo* MediaFilter::findBufferByID(
        uint32_t portIndex, IOMX::buffer_id bufferID,
        ssize_t *index) {
    for (size_t i = 0; i < mBuffers[portIndex].size(); ++i) {
        BufferInfo *info = &mBuffers[portIndex].editItemAt(i);

        if (info->mBufferID == bufferID) {
            if (index != NULL) {
                *index = i;
            }
            return info;
        }
    }

    TRESPASS();

    return NULL;
}

void MediaFilter::postFillThisBuffer(BufferInfo *info) {
    ALOGV("postFillThisBuffer on buffer %d", info->mBufferID);
    if (mPortEOS[kPortIndexInput]) {
        return;
    }

    CHECK_EQ((int)info->mStatus, (int)BufferInfo::OWNED_BY_US);

    info->mGeneration = mGeneration;

    info->mData->meta()->clear();

    sp<AMessage> reply = new AMessage(kWhatInputBufferFilled, this);
    reply->setInt32("buffer-id", info->mBufferID);

    info->mStatus = BufferInfo::OWNED_BY_UPSTREAM;

    mBufferChannel->fillThisBuffer(info->mBufferID);
}

void MediaFilter::postDrainThisBuffer(BufferInfo *info) {
    CHECK_EQ((int)info->mStatus, (int)BufferInfo::OWNED_BY_US);

    info->mGeneration = mGeneration;

    sp<AMessage> reply = new AMessage(kWhatOutputBufferDrained, this);
    reply->setInt32("buffer-id", info->mBufferID);

    mBufferChannel->drainThisBuffer(info->mBufferID, info->mOutputFlags);

    info->mStatus = BufferInfo::OWNED_BY_UPSTREAM;
}

void MediaFilter::postEOS() {
    mCallback->onEos(ERROR_END_OF_STREAM);

    ALOGV("Sent kWhatEOS.");
}

void MediaFilter::requestFillEmptyInput() {
    if (mPortEOS[kPortIndexInput]) {
        return;
    }

    for (size_t i = 0; i < mBuffers[kPortIndexInput].size(); ++i) {
        BufferInfo *info = &mBuffers[kPortIndexInput].editItemAt(i);

        if (info->mStatus == BufferInfo::OWNED_BY_US) {
            postFillThisBuffer(info);
        }
    }
}

void MediaFilter::processBuffers() {
    if (mAvailableInputBuffers.empty() || mAvailableOutputBuffers.empty()) {
        ALOGV("Skipping process (buffers unavailable)");
        return;
    }

    if (mPortEOS[kPortIndexOutput]) {
        // TODO notify caller of queueInput error when it is supported
        // in MediaCodec
        ALOGW("Tried to process a buffer after EOS.");
        return;
    }

    BufferInfo *inputInfo = mAvailableInputBuffers[0];
    mAvailableInputBuffers.removeAt(0);
    BufferInfo *outputInfo = mAvailableOutputBuffers[0];
    mAvailableOutputBuffers.removeAt(0);

    status_t err;
    err = mFilter->processBuffers(inputInfo->mData, outputInfo->mData);
    if (err != (status_t)OK) {
        outputInfo->mData->meta()->setInt32("err", err);
    }

    int64_t timeUs;
    CHECK(inputInfo->mData->meta()->findInt64("timeUs", &timeUs));
    outputInfo->mData->meta()->setInt64("timeUs", timeUs);
    outputInfo->mOutputFlags = 0;
    int32_t eos = 0;
    if (inputInfo->mData->meta()->findInt32("eos", &eos) && eos != 0) {
        outputInfo->mOutputFlags |= OMX_BUFFERFLAG_EOS;
        mPortEOS[kPortIndexOutput] = true;
        outputInfo->mData->meta()->setInt32("eos", eos);
        postEOS();
        ALOGV("Output stream saw EOS.");
    }

    ALOGV("Processed input buffer %u [%zu], output buffer %u [%zu]",
                inputInfo->mBufferID, inputInfo->mData->size(),
                outputInfo->mBufferID, outputInfo->mData->size());

    if (mGraphicBufferListener != NULL) {
        delete inputInfo;
    } else {
        postFillThisBuffer(inputInfo);
    }
    postDrainThisBuffer(outputInfo);

    // prevent any corner case where buffers could get stuck in queue
    signalProcessBuffers();
}

void MediaFilter::onAllocateComponent(const sp<AMessage> &msg) {
    CHECK_EQ(mState, UNINITIALIZED);

    CHECK(msg->findString("componentName", &mComponentName));
    const char* name = mComponentName.c_str();
    if (!strcasecmp(name, "android.filter.zerofilter")) {
        mFilter = new ZeroFilter;
    } else if (!strcasecmp(name, "android.filter.saturation")) {
        mFilter = new SaturationFilter;
    } else if (!strcasecmp(name, "android.filter.intrinsicblur")) {
        mFilter = new IntrinsicBlurFilter;
    } else if (!strcasecmp(name, "android.filter.RenderScript")) {
        mFilter = new RSFilter;
    } else {
        ALOGE("Unrecognized filter name: %s", name);
        signalError(NAME_NOT_FOUND);
        return;
    }

    // HACK - need "OMX.google" to use MediaCodec's software renderer
    mCallback->onComponentAllocated("OMX.google.MediaFilter");
    mState = INITIALIZED;
    ALOGV("Handled kWhatAllocateComponent.");
}

void MediaFilter::onConfigureComponent(const sp<AMessage> &msg) {
    // TODO: generalize to allow audio filters as well as video

    CHECK_EQ(mState, INITIALIZED);

    // get params - at least mime, width & height
    AString mime;
    CHECK(msg->findString("mime", &mime));
    if (strcasecmp(mime.c_str(), MEDIA_MIMETYPE_VIDEO_RAW)) {
        ALOGE("Bad mime: %s", mime.c_str());
        signalError(BAD_VALUE);
        return;
    }

    CHECK(msg->findInt32("width", &mWidth));
    CHECK(msg->findInt32("height", &mHeight));
    if (!msg->findInt32("stride", &mStride)) {
        mStride = mWidth;
    }
    if (!msg->findInt32("slice-height", &mSliceHeight)) {
        mSliceHeight = mHeight;
    }

    mMaxInputSize = mWidth * mHeight * 4;   // room for ARGB8888
    int32_t maxInputSize;
    if (msg->findInt32("max-input-size", &maxInputSize)
            && (size_t)maxInputSize > mMaxInputSize) {
        mMaxInputSize = maxInputSize;
    }

    if (!msg->findInt32("color-format", &mColorFormatIn)) {
        // default to OMX_COLOR_Format32bitARGB8888
        mColorFormatIn = OMX_COLOR_Format32bitARGB8888;
        msg->setInt32("color-format", mColorFormatIn);
    }
    mColorFormatOut = mColorFormatIn;

    mMaxOutputSize = mWidth * mHeight * 4;  // room for ARGB8888

    AString cacheDir;
    if (!msg->findString("cacheDir", &cacheDir)) {
        ALOGE("Failed to find cache directory in config message.");
        signalError(NAME_NOT_FOUND);
        return;
    }

    status_t err;
    err = mFilter->configure(msg);
    if (err != (status_t)OK) {
        ALOGE("Failed to configure filter component, err %d", err);
        signalError(err);
        return;
    }

    mInputFormat = new AMessage();
    mInputFormat->setString("mime", mime.c_str());
    mInputFormat->setInt32("stride", mStride);
    mInputFormat->setInt32("slice-height", mSliceHeight);
    mInputFormat->setInt32("color-format", mColorFormatIn);
    mInputFormat->setRect("crop", 0, 0, mStride, mSliceHeight);
    mInputFormat->setInt32("width", mWidth);
    mInputFormat->setInt32("height", mHeight);

    mOutputFormat = new AMessage();
    mOutputFormat->setString("mime", mime.c_str());
    mOutputFormat->setInt32("stride", mStride);
    mOutputFormat->setInt32("slice-height", mSliceHeight);
    mOutputFormat->setInt32("color-format", mColorFormatOut);
    mOutputFormat->setRect("crop", 0, 0, mStride, mSliceHeight);
    mOutputFormat->setInt32("width", mWidth);
    mOutputFormat->setInt32("height", mHeight);

    mCallback->onComponentConfigured(mInputFormat, mOutputFormat);
    mState = CONFIGURED;
    ALOGV("Handled kWhatConfigureComponent.");
}

void MediaFilter::onStart() {
    CHECK_EQ(mState, CONFIGURED);

    allocateBuffersOnPort(kPortIndexInput);

    allocateBuffersOnPort(kPortIndexOutput);

    mCallback->onStartCompleted();

    status_t err = mFilter->start();
    if (err != (status_t)OK) {
        ALOGE("Failed to start filter component, err %d", err);
        signalError(err);
        return;
    }

    mPortEOS[kPortIndexInput] = false;
    mPortEOS[kPortIndexOutput] = false;
    mInputEOSResult = OK;
    mState = STARTED;

    requestFillEmptyInput();
    ALOGV("Handled kWhatStart.");
}

void MediaFilter::onInputBufferFilled(const sp<AMessage> &msg) {
    IOMX::buffer_id bufferID;
    CHECK(msg->findInt32("buffer-id", (int32_t*)&bufferID));
    BufferInfo *info = findBufferByID(kPortIndexInput, bufferID);

    if (mState != STARTED) {
        // we're not running, so we'll just keep that buffer...
        info->mStatus = BufferInfo::OWNED_BY_US;
        return;
    }

    if (info->mGeneration != mGeneration) {
        ALOGV("Caught a stale input buffer [ID %d]", bufferID);
        // buffer is stale (taken before a flush/shutdown) - repost it
        CHECK_EQ(info->mStatus, BufferInfo::OWNED_BY_US);
        postFillThisBuffer(info);
        return;
    }

    CHECK_EQ(info->mStatus, BufferInfo::OWNED_BY_UPSTREAM);
    info->mStatus = BufferInfo::OWNED_BY_US;

    sp<MediaCodecBuffer> buffer;
    int32_t err = OK;
    bool eos = false;

    sp<RefBase> obj;
    if (!msg->findObject("buffer", &obj)) {
        // these are unfilled buffers returned by client
        CHECK(msg->findInt32("err", &err));

        if (err == OK) {
            // buffers with no errors are returned on MediaCodec.flush
            ALOGV("saw unfilled buffer (MediaCodec.flush)");
            postFillThisBuffer(info);
            return;
        } else {
            ALOGV("saw error %d instead of an input buffer", err);
            eos = true;
        }

        buffer.clear();
    } else {
        buffer = static_cast<MediaCodecBuffer *>(obj.get());
    }

    int32_t isCSD;
    if (buffer != NULL && buffer->meta()->findInt32("csd", &isCSD)
            && isCSD != 0) {
        // ignore codec-specific data buffers
        ALOGW("MediaFilter received a codec-specific data buffer");
        postFillThisBuffer(info);
        return;
    }

    int32_t tmp;
    if (buffer != NULL && buffer->meta()->findInt32("eos", &tmp) && tmp) {
        eos = true;
        err = ERROR_END_OF_STREAM;
    }

    mAvailableInputBuffers.push_back(info);
    processBuffers();

    if (eos) {
        mPortEOS[kPortIndexInput] = true;
        mInputEOSResult = err;
    }

    ALOGV("Handled kWhatInputBufferFilled. [ID %u]", bufferID);
}

void MediaFilter::onOutputBufferDrained(const sp<AMessage> &msg) {
    IOMX::buffer_id bufferID;
    CHECK(msg->findInt32("buffer-id", (int32_t*)&bufferID));
    BufferInfo *info = findBufferByID(kPortIndexOutput, bufferID);

    if (mState != STARTED) {
        // we're not running, so we'll just keep that buffer...
        info->mStatus = BufferInfo::OWNED_BY_US;
        return;
    }

    if (info->mGeneration != mGeneration) {
        ALOGV("Caught a stale output buffer [ID %d]", bufferID);
        // buffer is stale (taken before a flush/shutdown) - keep it
        CHECK_EQ(info->mStatus, BufferInfo::OWNED_BY_US);
        return;
    }

    CHECK_EQ(info->mStatus, BufferInfo::OWNED_BY_UPSTREAM);
    info->mStatus = BufferInfo::OWNED_BY_US;

    mAvailableOutputBuffers.push_back(info);

    processBuffers();

    ALOGV("Handled kWhatOutputBufferDrained. [ID %u]",
            bufferID);
}

void MediaFilter::onShutdown(const sp<AMessage> &msg) {
    mGeneration++;

    if (mState != UNINITIALIZED) {
        mFilter->reset();
    }

    int32_t keepComponentAllocated;
    CHECK(msg->findInt32("keepComponentAllocated", &keepComponentAllocated));
    if (!keepComponentAllocated || mState == UNINITIALIZED) {
        mState = UNINITIALIZED;
    } else {
        mState = INITIALIZED;
    }

    if (keepComponentAllocated) {
        mCallback->onStopCompleted();
    } else {
        mCallback->onReleaseCompleted();
    }
}

void MediaFilter::onFlush() {
    mGeneration++;

    mAvailableInputBuffers.clear();
    for (size_t i = 0; i < mBuffers[kPortIndexInput].size(); ++i) {
        BufferInfo *info = &mBuffers[kPortIndexInput].editItemAt(i);
        info->mStatus = BufferInfo::OWNED_BY_US;
    }
    mAvailableOutputBuffers.clear();
    for (size_t i = 0; i < mBuffers[kPortIndexOutput].size(); ++i) {
        BufferInfo *info = &mBuffers[kPortIndexOutput].editItemAt(i);
        info->mStatus = BufferInfo::OWNED_BY_US;
        mAvailableOutputBuffers.push_back(info);
    }

    mPortEOS[kPortIndexInput] = false;
    mPortEOS[kPortIndexOutput] = false;
    mInputEOSResult = OK;

    mCallback->onFlushCompleted();
    ALOGV("Posted kWhatFlushCompleted");

    // MediaCodec returns all input buffers after flush, so in
    // onInputBufferFilled we call postFillThisBuffer on them
}

void MediaFilter::onSetParameters(const sp<AMessage> &msg) {
    CHECK(mState != STARTED);

    status_t err = mFilter->setParameters(msg);
    if (err != (status_t)OK) {
        ALOGE("setParameters returned err %d", err);
    }
}

void MediaFilter::onCreateInputSurface() {
    CHECK(mState == CONFIGURED);

    mGraphicBufferListener = new GraphicBufferListener;

    sp<AMessage> notify = new AMessage();
    notify->setTarget(this);
    status_t err = mGraphicBufferListener->init(
            notify, mStride, mSliceHeight, kBufferCountActual);

    if (err != OK) {
        ALOGE("Failed to init mGraphicBufferListener: %d", err);
        signalError(err);
        return;
    }

    mCallback->onInputSurfaceCreated(
            nullptr, nullptr,
            new BufferProducerWrapper(
                    mGraphicBufferListener->getIGraphicBufferProducer()));
}

void MediaFilter::onInputFrameAvailable() {
    BufferItem item = mGraphicBufferListener->getBufferItem();
    sp<GraphicBuffer> buf = mGraphicBufferListener->getBuffer(item);

    // get pointer to graphic buffer
    void* bufPtr;
    buf->lock(GraphicBuffer::USAGE_SW_READ_OFTEN, &bufPtr);

    // HACK - there is no OMX_COLOR_FORMATTYPE value for RGBA, so the format
    // conversion is hardcoded until we add this.
    // TODO: check input format and convert only if necessary
    // copy RGBA graphic buffer into temporary ARGB input buffer
    BufferInfo *inputInfo = new BufferInfo;
    inputInfo->mData = new MediaCodecBuffer(
            mInputFormat, new ABuffer(buf->getWidth() * buf->getHeight() * 4));
    ALOGV("Copying surface data into temp buffer.");
    convertRGBAToARGB(
            (uint8_t*)bufPtr, buf->getWidth(), buf->getHeight(),
            buf->getStride(), inputInfo->mData->data());
    inputInfo->mBufferID = item.mSlot;
    inputInfo->mGeneration = mGeneration;
    inputInfo->mOutputFlags = 0;
    inputInfo->mStatus = BufferInfo::OWNED_BY_US;
    inputInfo->mData->meta()->setInt64("timeUs", item.mTimestamp / 1000);

    mAvailableInputBuffers.push_back(inputInfo);

    mGraphicBufferListener->releaseBuffer(item);

    signalProcessBuffers();
}

void MediaFilter::onSignalEndOfInputStream() {
    // if using input surface, need to send an EOS output buffer
    if (mGraphicBufferListener != NULL) {
        Vector<BufferInfo> *outputBufs = &mBuffers[kPortIndexOutput];
        BufferInfo* eosBuf;
        bool foundBuf = false;
        for (size_t i = 0; i < kBufferCountActual; i++) {
            eosBuf = &outputBufs->editItemAt(i);
            if (eosBuf->mStatus == BufferInfo::OWNED_BY_US) {
                foundBuf = true;
                break;
            }
        }

        if (!foundBuf) {
            ALOGE("onSignalEndOfInputStream failed to find an output buffer");
            return;
        }

        eosBuf->mOutputFlags = OMX_BUFFERFLAG_EOS;
        eosBuf->mGeneration = mGeneration;
        eosBuf->mData->setRange(0, 0);
        postDrainThisBuffer(eosBuf);
        ALOGV("Posted EOS on output buffer %u", eosBuf->mBufferID);
    }

    mPortEOS[kPortIndexOutput] = true;
    mCallback->onSignaledInputEOS(OK);

    ALOGV("Output stream saw EOS.");
}

}   // namespace android
