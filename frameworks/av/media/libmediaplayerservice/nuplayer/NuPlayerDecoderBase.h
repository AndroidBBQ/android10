/*
 * Copyright (C) 2010 The Android Open Source Project
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

#ifndef NUPLAYER_DECODER_BASE_H_

#define NUPLAYER_DECODER_BASE_H_

#include "NuPlayer.h"

#include <media/stagefright/foundation/AHandler.h>

namespace android {

struct ABuffer;
struct MediaCodec;
class MediaBuffer;
class MediaCodecBuffer;
class Surface;

struct NuPlayer::DecoderBase : public AHandler {
    explicit DecoderBase(const sp<AMessage> &notify);

    void configure(const sp<AMessage> &format);
    void init();
    void setParameters(const sp<AMessage> &params);

    // Synchronous call to ensure decoder will not request or send out data.
    void pause();

    void setRenderer(const sp<Renderer> &renderer);
    virtual status_t setVideoSurface(const sp<Surface> &) { return INVALID_OPERATION; }

    void signalFlush();
    void signalResume(bool notifyComplete);
    void initiateShutdown();

    virtual sp<AMessage> getStats() const {
        return mStats;
    }

    virtual status_t releaseCrypto() {
        return INVALID_OPERATION;
    }

    enum {
        kWhatInputDiscontinuity  = 'inDi',
        kWhatVideoSizeChanged    = 'viSC',
        kWhatFlushCompleted      = 'flsC',
        kWhatShutdownCompleted   = 'shDC',
        kWhatResumeCompleted     = 'resC',
        kWhatEOS                 = 'eos ',
        kWhatError               = 'err ',
    };

protected:

    virtual ~DecoderBase();

    void stopLooper();

    virtual void onMessageReceived(const sp<AMessage> &msg);

    virtual void onConfigure(const sp<AMessage> &format) = 0;
    virtual void onSetParameters(const sp<AMessage> &params) = 0;
    virtual void onSetRenderer(const sp<Renderer> &renderer) = 0;
    virtual void onResume(bool notifyComplete) = 0;
    virtual void onFlush() = 0;
    virtual void onShutdown(bool notifyComplete) = 0;

    void onRequestInputBuffers();
    virtual bool doRequestBuffers() = 0;
    virtual void handleError(int32_t err);

    sp<AMessage> mNotify;
    int32_t mBufferGeneration;
    bool mPaused;
    sp<AMessage> mStats;

private:
    enum {
        kWhatConfigure           = 'conf',
        kWhatSetParameters       = 'setP',
        kWhatSetRenderer         = 'setR',
        kWhatPause               = 'paus',
        kWhatRequestInputBuffers = 'reqB',
        kWhatFlush               = 'flus',
        kWhatShutdown            = 'shuD',
    };

    sp<ALooper> mDecoderLooper;
    bool mRequestInputBuffersPending;

    DISALLOW_EVIL_CONSTRUCTORS(DecoderBase);
};

}  // namespace android

#endif  // NUPLAYER_DECODER_BASE_H_
