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

#ifndef MEDIA_FILTER_H_
#define MEDIA_FILTER_H_

#include <media/stagefright/CodecBase.h>

namespace android {

class ACodecBufferChannel;
struct GraphicBufferListener;
class MemoryDealer;
struct SimpleFilter;

struct MediaFilter : public CodecBase {
    MediaFilter();

    virtual std::shared_ptr<BufferChannelBase> getBufferChannel() override;
    virtual void initiateAllocateComponent(const sp<AMessage> &msg);
    virtual void initiateConfigureComponent(const sp<AMessage> &msg);
    virtual void initiateCreateInputSurface();
    virtual void initiateSetInputSurface(const sp<PersistentSurface> &surface);

    virtual void initiateStart();
    virtual void initiateShutdown(bool keepComponentAllocated = false);

    virtual void signalFlush();
    virtual void signalResume();

    virtual void signalRequestIDRFrame();
    virtual void signalSetParameters(const sp<AMessage> &msg);
    virtual void signalEndOfInputStream();

    virtual void onMessageReceived(const sp<AMessage> &msg);

protected:
    virtual ~MediaFilter();

private:
    struct BufferInfo {
        enum Status {
            OWNED_BY_US,
            OWNED_BY_UPSTREAM,
        };

        uint32_t mBufferID;
        int32_t mGeneration;
        int32_t mOutputFlags;
        Status mStatus;

        sp<MediaCodecBuffer> mData;
    };

    enum State {
      UNINITIALIZED,
      INITIALIZED,
      CONFIGURED,
      STARTED,
    };

    enum {
        kWhatInputBufferFilled       = 'inpF',
        kWhatOutputBufferDrained     = 'outD',
        kWhatShutdown                = 'shut',
        kWhatFlush                   = 'flus',
        kWhatResume                  = 'resm',
        kWhatAllocateComponent       = 'allo',
        kWhatConfigureComponent      = 'conf',
        kWhatCreateInputSurface      = 'cisf',
        kWhatSignalEndOfInputStream  = 'eois',
        kWhatStart                   = 'star',
        kWhatSetParameters           = 'setP',
        kWhatProcessBuffers          = 'proc',
    };

    enum {
        kPortIndexInput  = 0,
        kPortIndexOutput = 1
    };

    // member variables
    AString mComponentName;
    State mState;
    status_t mInputEOSResult;
    int32_t mWidth, mHeight;
    int32_t mStride, mSliceHeight;
    int32_t mColorFormatIn, mColorFormatOut;
    size_t mMaxInputSize, mMaxOutputSize;
    int32_t mGeneration;
    sp<AMessage> mInputFormat;
    sp<AMessage> mOutputFormat;

    sp<MemoryDealer> mDealer[2];
    Vector<BufferInfo> mBuffers[2];
    Vector<BufferInfo*> mAvailableInputBuffers;
    Vector<BufferInfo*> mAvailableOutputBuffers;
    bool mPortEOS[2];

    sp<SimpleFilter> mFilter;
    sp<GraphicBufferListener> mGraphicBufferListener;

    std::shared_ptr<ACodecBufferChannel> mBufferChannel;

    // helper functions
    void signalProcessBuffers();
    void signalError(status_t error);

    status_t allocateBuffersOnPort(OMX_U32 portIndex);
    BufferInfo *findBufferByID(
            uint32_t portIndex, uint32_t bufferID,
            ssize_t *index = NULL);
    void postFillThisBuffer(BufferInfo *info);
    void postDrainThisBuffer(BufferInfo *info);
    void postEOS();
    void requestFillEmptyInput();
    void processBuffers();

    void onAllocateComponent(const sp<AMessage> &msg);
    void onConfigureComponent(const sp<AMessage> &msg);
    void onStart();
    void onInputBufferFilled(const sp<AMessage> &msg);
    void onOutputBufferDrained(const sp<AMessage> &msg);
    void onShutdown(const sp<AMessage> &msg);
    void onFlush();
    void onSetParameters(const sp<AMessage> &msg);
    void onCreateInputSurface();
    void onInputFrameAvailable();
    void onSignalEndOfInputStream();

    DISALLOW_EVIL_CONSTRUCTORS(MediaFilter);
};

}  // namespace android

#endif  // MEDIA_FILTER_H_
