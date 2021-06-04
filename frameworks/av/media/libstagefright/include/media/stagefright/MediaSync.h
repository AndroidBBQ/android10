/*
 * Copyright 2015 The Android Open Source Project
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

#ifndef MEDIA_SYNC_H
#define MEDIA_SYNC_H

#include <gui/IConsumerListener.h>
#include <gui/IProducerListener.h>

#include <media/AudioResamplerPublic.h>
#include <media/AVSyncSettings.h>
#include <media/stagefright/foundation/AHandler.h>

#include <utils/Condition.h>
#include <utils/KeyedVector.h>
#include <utils/Mutex.h>

namespace android {

class AudioTrack;
class BufferItem;
class Fence;
class GraphicBuffer;
class IGraphicBufferConsumer;
class IGraphicBufferProducer;
struct MediaClock;
struct VideoFrameScheduler;

// MediaSync manages media playback and its synchronization to a media clock
// source. It can be also used for video-only playback.
//
// For video playback, it requires an output surface and provides an input
// surface. It then controls the rendering of input buffers (buffer queued to
// the input surface) on the output surface to happen at the appropriate time.
//
// For audio playback, it requires an audio track and takes updates of
// information of rendered audio data to maintain media clock when audio track
// serves as media clock source. (TODO: move audio rendering from JAVA to
// native code).
//
// It can use the audio or video track as media clock source, as well as an
// external clock. (TODO: actually support external clock as media clock
// sources; use video track as media clock source for audio-and-video stream).
//
// In video-only mode, MediaSync will playback every video frame even though
// a video frame arrives late based on its timestamp and last frame's.
//
// The client needs to configure surface (for output video rendering) and audio
// track (for querying information of audio rendering) for MediaSync.
//
// Then the client needs to obtain a surface from MediaSync and render video
// frames onto that surface. Internally, the MediaSync will receive those video
// frames and render them onto the output surface at the appropriate time.
//
// The client needs to call updateQueuedAudioData() immediately after it writes
// audio data to the audio track. Such information will be used to update media
// clock.
//
class MediaSync : public AHandler {
public:
    // Create an instance of MediaSync.
    static sp<MediaSync> create();

    // Called when MediaSync is used to render video. It should be called
    // before createInputSurface().
    status_t setSurface(const sp<IGraphicBufferProducer> &output);

    // Called when audio track is used as media clock source. It should be
    // called before updateQueuedAudioData().
    status_t setAudioTrack(const sp<AudioTrack> &audioTrack);

    // Create a surface for client to render video frames. This is the surface
    // on which the client should render video frames. Those video frames will
    // be internally directed to output surface for rendering at appropriate
    // time.
    status_t createInputSurface(sp<IGraphicBufferProducer> *outBufferProducer);

    // Update just-rendered audio data size and the presentation timestamp of
    // the first frame of that audio data. It should be called immediately
    // after the client write audio data into AudioTrack.
    // This function assumes continous audio stream.
    // TODO: support gap or backwards updates.
    status_t updateQueuedAudioData(
            size_t sizeInBytes, int64_t presentationTimeUs);

    // Set the consumer name of the input queue.
    void setName(const AString &name);

    // Get the media clock used by the MediaSync so that the client can obtain
    // corresponding media time or real time via
    // MediaClock::getMediaTime() and MediaClock::getRealTimeFor().
    sp<const MediaClock> getMediaClock();

    // Flush mediasync
    void flush();

    // Set the video frame rate hint - this is used by the video FrameScheduler
    status_t setVideoFrameRateHint(float rate);

    // Get the video frame rate measurement from the FrameScheduler
    // returns -1 if there is no measurement
    float getVideoFrameRate();

    // Set the sync settings parameters.
    status_t setSyncSettings(const AVSyncSettings &syncSettings);

    // Gets the sync settings parameters.
    void getSyncSettings(AVSyncSettings *syncSettings /* nonnull */);

    // Sets the playback rate using playback settings.
    // This method can be called any time.
    status_t setPlaybackSettings(const AudioPlaybackRate &rate);

    // Gets the playback rate (playback settings parameters).
    void getPlaybackSettings(AudioPlaybackRate *rate /* nonnull */);

    // Get the play time for pending audio frames in audio sink.
    status_t getPlayTimeForPendingAudioFrames(int64_t *outTimeUs);

protected:
    virtual void onMessageReceived(const sp<AMessage> &msg);

private:
    enum {
        kWhatDrainVideo = 'dVid',
    };

    // This is a thin wrapper class that lets us listen to
    // IConsumerListener::onFrameAvailable from mInput.
    class InputListener : public BnConsumerListener,
                          public IBinder::DeathRecipient {
    public:
        InputListener(const sp<MediaSync> &sync);
        virtual ~InputListener();

        // From IConsumerListener
        virtual void onFrameAvailable(const BufferItem &item);

        // From IConsumerListener
        // We don't care about released buffers because we detach each buffer as
        // soon as we acquire it. See the comment for onBufferReleased below for
        // some clarifying notes about the name.
        virtual void onBuffersReleased() {}

        // From IConsumerListener
        // We don't care about sideband streams, since we won't relay them.
        virtual void onSidebandStreamChanged();

        // From IBinder::DeathRecipient
        virtual void binderDied(const wp<IBinder> &who);

    private:
        sp<MediaSync> mSync;
    };

    // This is a thin wrapper class that lets us listen to
    // IProducerListener::onBufferReleased from mOutput.
    class OutputListener : public BnProducerListener,
                           public IBinder::DeathRecipient {
    public:
        OutputListener(const sp<MediaSync> &sync, const sp<IGraphicBufferProducer> &output);
        virtual ~OutputListener();

        // From IProducerListener
        virtual void onBufferReleased();

        // From IBinder::DeathRecipient
        virtual void binderDied(const wp<IBinder> &who);

    private:
        sp<MediaSync> mSync;
        sp<IGraphicBufferProducer> mOutput;
    };

    // mIsAbandoned is set to true when the input or output dies.
    // Once the MediaSync has been abandoned by one side, it will disconnect
    // from the other side and not attempt to communicate with it further.
    bool mIsAbandoned;

    mutable Mutex mMutex;
    Condition mReleaseCondition;
    size_t mNumOutstandingBuffers;
    sp<IGraphicBufferConsumer> mInput;
    sp<IGraphicBufferProducer> mOutput;
    int mUsageFlagsFromOutput;
    uint32_t mMaxAcquiredBufferCount; // max acquired buffer count
    bool mReturnPendingInputFrame;    // set while we are pending before acquiring an input frame

    sp<AudioTrack> mAudioTrack;
    uint32_t mNativeSampleRateInHz;
    int64_t mNumFramesWritten;
    bool mHasAudio;

    int64_t mNextBufferItemMediaUs;
    List<BufferItem> mBufferItems;
    sp<VideoFrameScheduler> mFrameScheduler;

    // Keep track of buffers received from |mInput|. This is needed because
    // it's possible the consumer of |mOutput| could return a different
    // GraphicBuffer::handle (e.g., due to passing buffers through IPC),
    // and that could cause problem if the producer of |mInput| only
    // supports pre-registered buffers.
    KeyedVector<uint64_t, sp<GraphicBuffer> > mBuffersFromInput;

    // Keep track of buffers sent to |mOutput|. When a new output surface comes
    // in, those buffers will be returned to input and old output surface will
    // be disconnected immediately.
    KeyedVector<uint64_t, sp<GraphicBuffer> > mBuffersSentToOutput;

    sp<ALooper> mLooper;
    float mPlaybackRate;

    AudioPlaybackRate mPlaybackSettings;
    AVSyncSettings mSyncSettings;

    sp<MediaClock> mMediaClock;

    MediaSync();

    // Must be accessed through RefBase
    virtual ~MediaSync();

    int64_t getRealTime(int64_t mediaTimeUs, int64_t nowUs);
    int64_t getDurationIfPlayedAtNativeSampleRate_l(int64_t numFrames);
    int64_t getPlayedOutAudioDurationMedia_l(int64_t nowUs);

    void onDrainVideo_l();

    // This implements the onFrameAvailable callback from IConsumerListener.
    // It gets called from an InputListener.
    // During this callback, we detach the buffer from the input, and queue
    // it for rendering on the output. This call can block if there are too
    // many outstanding buffers. If it blocks, it will resume when
    // onBufferReleasedByOutput releases a buffer back to the input.
    void onFrameAvailableFromInput();

    // Send |bufferItem| to the output for rendering.
    void renderOneBufferItem_l(const BufferItem &bufferItem);

    // This implements the onBufferReleased callback from IProducerListener.
    // It gets called from an OutputListener.
    // During this callback, we detach the buffer from the output, and release
    // it to the input. A blocked onFrameAvailable call will be allowed to proceed.
    void onBufferReleasedByOutput(sp<IGraphicBufferProducer> &output);

    // Return |buffer| back to the input.
    void returnBufferToInput_l(const sp<GraphicBuffer> &buffer, const sp<Fence> &fence);

    // When this is called, the MediaSync disconnects from (i.e., abandons) its
    // input or output, and signals any waiting onFrameAvailable calls to wake
    // up. This must be called with mMutex locked.
    void onAbandoned_l(bool isInput);

    // Set the playback in a desired speed.
    // This method can be called any time.
    // |rate| is the ratio between desired speed and the normal one, and should
    // be non-negative. The meaning of rate values:
    // 1.0 -- normal playback
    // 0.0 -- stop or pause
    // larger than 1.0 -- faster than normal speed
    // between 0.0 and 1.0 -- slower than normal speed
    void updatePlaybackRate_l(float rate);

    // apply new sync settings
    void resync_l();

    // apply playback settings only - without resyncing or updating playback rate
    status_t setPlaybackSettings_l(const AudioPlaybackRate &rate);

    // helper.
    bool isPlaying() { return mPlaybackRate != 0.0; }

    DISALLOW_EVIL_CONSTRUCTORS(MediaSync);
};

} // namespace android

#endif
