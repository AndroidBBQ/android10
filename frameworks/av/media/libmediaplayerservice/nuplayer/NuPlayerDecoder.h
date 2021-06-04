/*
 * Copyright 2014 The Android Open Source Project
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

#ifndef NUPLAYER_DECODER_H_
#define NUPLAYER_DECODER_H_

#include "NuPlayer.h"

#include "NuPlayerDecoderBase.h"

namespace android {

class MediaCodecBuffer;

struct NuPlayer::Decoder : public DecoderBase {
    Decoder(const sp<AMessage> &notify,
            const sp<Source> &source,
            pid_t pid,
            uid_t uid,
            const sp<Renderer> &renderer = NULL,
            const sp<Surface> &surface = NULL,
            const sp<CCDecoder> &ccDecoder = NULL);

    virtual sp<AMessage> getStats() const;

    // sets the output surface of video decoders.
    virtual status_t setVideoSurface(const sp<Surface> &surface);

    virtual status_t releaseCrypto();

protected:
    virtual ~Decoder();

    virtual void onMessageReceived(const sp<AMessage> &msg);

    virtual void onConfigure(const sp<AMessage> &format);
    virtual void onSetParameters(const sp<AMessage> &params);
    virtual void onSetRenderer(const sp<Renderer> &renderer);
    virtual void onResume(bool notifyComplete);
    virtual void onFlush();
    virtual void onShutdown(bool notifyComplete);
    virtual bool doRequestBuffers();

private:
    enum {
        kWhatCodecNotify         = 'cdcN',
        kWhatRenderBuffer        = 'rndr',
        kWhatSetVideoSurface     = 'sSur',
        kWhatAudioOutputFormatChanged = 'aofc',
        kWhatDrmReleaseCrypto    = 'rDrm',
    };

    enum {
        kMaxNumVideoTemporalLayers = 32,
    };

    sp<Surface> mSurface;

    sp<Source> mSource;
    sp<Renderer> mRenderer;
    sp<CCDecoder> mCCDecoder;

    sp<AMessage> mInputFormat;
    sp<AMessage> mOutputFormat;
    sp<MediaCodec> mCodec;
    sp<ALooper> mCodecLooper;

    List<sp<AMessage> > mPendingInputMessages;

    Vector<sp<MediaCodecBuffer> > mInputBuffers;
    Vector<sp<MediaCodecBuffer> > mOutputBuffers;
    Vector<sp<ABuffer> > mCSDsForCurrentFormat;
    Vector<sp<ABuffer> > mCSDsToSubmit;
    Vector<bool> mInputBufferIsDequeued;
    Vector<MediaBuffer *> mMediaBuffers;
    Vector<size_t> mDequeuedInputBuffers;

    const pid_t mPid;
    const uid_t mUid;
    int64_t mSkipRenderingUntilMediaTimeUs;
    int64_t mNumFramesTotal;
    int64_t mNumInputFramesDropped;
    int64_t mNumOutputFramesDropped;
    int32_t mVideoWidth;
    int32_t mVideoHeight;
    bool mIsAudio;
    bool mIsVideoAVC;
    bool mIsSecure;
    bool mIsEncrypted;
    bool mIsEncryptedObservedEarlier;
    bool mFormatChangePending;
    bool mTimeChangePending;
    float mFrameRateTotal;
    float mPlaybackSpeed;
    int32_t mNumVideoTemporalLayerTotal;
    int32_t mNumVideoTemporalLayerAllowed;
    int32_t mCurrentMaxVideoTemporalLayerId;
    float mVideoTemporalLayerAggregateFps[kMaxNumVideoTemporalLayers];

    bool mResumePending;
    AString mComponentName;

    void handleError(int32_t err);
    bool handleAnInputBuffer(size_t index);
    bool handleAnOutputBuffer(
            size_t index,
            size_t offset,
            size_t size,
            int64_t timeUs,
            int32_t flags);
    void handleOutputFormatChange(const sp<AMessage> &format);

    void releaseAndResetMediaBuffers();
    void requestCodecNotification();
    bool isStaleReply(const sp<AMessage> &msg);

    void doFlush(bool notifyComplete);
    status_t fetchInputData(sp<AMessage> &reply);
    bool onInputBufferFetched(const sp<AMessage> &msg);
    void onRenderBuffer(const sp<AMessage> &msg);

    bool supportsSeamlessFormatChange(const sp<AMessage> &to) const;
    bool supportsSeamlessAudioFormatChange(const sp<AMessage> &targetFormat) const;
    void rememberCodecSpecificData(const sp<AMessage> &format);
    bool isDiscontinuityPending() const;
    void finishHandleDiscontinuity(bool flushOnTimeChange);

    void notifyResumeCompleteIfNecessary();

    void onReleaseCrypto(const sp<AMessage>& msg);

    DISALLOW_EVIL_CONSTRUCTORS(Decoder);
};

}  // namespace android

#endif  // NUPLAYER_DECODER_H_
