/*
 * Copyright 2017 The Android Open Source Project
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

#ifndef ANDROID_MEDIAPLAYER2INTERFACE_H
#define ANDROID_MEDIAPLAYER2INTERFACE_H

#ifdef __cplusplus

#include <sys/types.h>
#include <utils/Errors.h>
#include <utils/String8.h>
#include <utils/RefBase.h>
#include <jni.h>

#include <media/AVSyncSettings.h>
#include <media/AudioResamplerPublic.h>
#include <media/AudioSystem.h>
#include <media/AudioTimestamp.h>
#include <media/BufferingSettings.h>
#include <media/stagefright/foundation/AHandler.h>
#include <mediaplayer2/MediaPlayer2Types.h>

#include "jni.h"
#include "mediaplayer2.pb.h"

using android::media::MediaPlayer2Proto::PlayerMessage;

// Fwd decl to make sure everyone agrees that the scope of struct sockaddr_in is
// global, and not in android::
struct sockaddr_in;

namespace android {

struct DataSourceDesc;
class Parcel;
struct ANativeWindowWrapper;

#define DEFAULT_AUDIOSINK_BUFFERSIZE 1200
#define DEFAULT_AUDIOSINK_SAMPLERATE 44100

// when the channel mask isn't known, use the channel count to derive a mask in AudioSink::open()
#define CHANNEL_MASK_USE_CHANNEL_ORDER 0

// duration below which we do not allow deep audio buffering
#define AUDIO_SINK_MIN_DEEP_BUFFER_DURATION_US 5000000

class MediaPlayer2InterfaceListener: public RefBase
{
public:
    virtual void notify(int64_t srcId, int msg, int ext1, int ext2,
           const PlayerMessage *obj) = 0;
};

class MediaPlayer2Interface : public AHandler {
public:
    // AudioSink: abstraction layer for audio output
    class AudioSink : public RefBase {
    public:
        enum cb_event_t {
            CB_EVENT_FILL_BUFFER,   // Request to write more data to buffer.
            CB_EVENT_STREAM_END,    // Sent after all the buffers queued in AF and HW are played
                                    // back (after stop is called)
            CB_EVENT_TEAR_DOWN      // The AudioTrack was invalidated due to use case change:
                                    // Need to re-evaluate offloading options
        };

        // Callback returns the number of bytes actually written to the buffer.
        typedef size_t (*AudioCallback)(
                AudioSink *audioSink, void *buffer, size_t size, void *cookie, cb_event_t event);

        virtual ~AudioSink() {}
        virtual bool ready() const = 0; // audio output is open and ready
        virtual ssize_t bufferSize() const = 0;
        virtual ssize_t frameCount() const = 0;
        virtual ssize_t channelCount() const = 0;
        virtual ssize_t frameSize() const = 0;
        virtual uint32_t latency() const = 0;
        virtual float msecsPerFrame() const = 0;
        virtual status_t getPosition(uint32_t *position) const = 0;
        virtual status_t getTimestamp(AudioTimestamp &ts) const = 0;
        virtual int64_t getPlayedOutDurationUs(int64_t nowUs) const = 0;
        virtual status_t getFramesWritten(uint32_t *frameswritten) const = 0;
        virtual int32_t getSessionId() const = 0;
        virtual audio_stream_type_t getAudioStreamType() const = 0;
        virtual uint32_t getSampleRate() const = 0;
        virtual int64_t getBufferDurationInUs() const = 0;

        // If no callback is specified, use the "write" API below to submit
        // audio data.
        virtual status_t open(
                uint32_t sampleRate, int channelCount, audio_channel_mask_t channelMask,
                audio_format_t format=AUDIO_FORMAT_PCM_16_BIT,
                AudioCallback cb = NULL,
                void *cookie = NULL,
                audio_output_flags_t flags = AUDIO_OUTPUT_FLAG_NONE,
                const audio_offload_info_t *offloadInfo = NULL,
                uint32_t suggestedFrameCount = 0) = 0;

        virtual status_t start() = 0;

        /* Input parameter |size| is in byte units stored in |buffer|.
         * Data is copied over and actual number of bytes written (>= 0)
         * is returned, or no data is copied and a negative status code
         * is returned (even when |blocking| is true).
         * When |blocking| is false, AudioSink will immediately return after
         * part of or full |buffer| is copied over.
         * When |blocking| is true, AudioSink will wait to copy the entire
         * buffer, unless an error occurs or the copy operation is
         * prematurely stopped.
         */
        virtual ssize_t write(const void* buffer, size_t size, bool blocking = true) = 0;

        virtual void stop() = 0;
        virtual void flush() = 0;
        virtual void pause() = 0;
        virtual void close() = 0;

        virtual status_t setPlaybackRate(const AudioPlaybackRate& rate) = 0;
        virtual status_t getPlaybackRate(AudioPlaybackRate* rate /* nonnull */) = 0;
        virtual bool needsTrailingPadding() {
            return true;
        }

        virtual status_t setParameters(const String8& /* keyValuePairs */) {
            return NO_ERROR;
        }
        virtual String8 getParameters(const String8& /* keys */) {
            return String8::empty();
        }

        // AudioRouting
        virtual status_t    setPreferredDevice(jobject device);
        virtual jobject     getRoutedDevice();
        virtual status_t    addAudioDeviceCallback(jobject routingDelegate);
        virtual status_t    removeAudioDeviceCallback(jobject listener);
    };

    MediaPlayer2Interface() : mListener(NULL) { }
    virtual ~MediaPlayer2Interface() { }
    virtual status_t initCheck() = 0;

    virtual void setAudioSink(const sp<AudioSink>& audioSink) {
        mAudioSink = audioSink;
    }

    virtual status_t setDataSource(const sp<DataSourceDesc> &dsd) = 0;

    virtual status_t prepareNextDataSource(const sp<DataSourceDesc> &dsd) = 0;

    virtual status_t playNextDataSource(int64_t srcId) = 0;

    // pass the buffered native window to the media player service
    virtual status_t setVideoSurfaceTexture(const sp<ANativeWindowWrapper>& nww) = 0;

    virtual status_t getBufferingSettings(BufferingSettings* buffering /* nonnull */) {
        *buffering = BufferingSettings();
        return OK;
    }
    virtual status_t setBufferingSettings(const BufferingSettings& /* buffering */) {
        return OK;
    }

    virtual status_t prepareAsync() = 0;
    virtual status_t start() = 0;
    virtual status_t pause() = 0;
    virtual bool isPlaying() = 0;
    virtual status_t setPlaybackSettings(const AudioPlaybackRate& rate) {
        // by default, players only support setting rate to the default
        if (!isAudioPlaybackRateEqual(rate, AUDIO_PLAYBACK_RATE_DEFAULT)) {
            return BAD_VALUE;
        }
        return OK;
    }
    virtual status_t getPlaybackSettings(AudioPlaybackRate* rate /* nonnull */) {
        *rate = AUDIO_PLAYBACK_RATE_DEFAULT;
        return OK;
    }
    virtual status_t setSyncSettings(const AVSyncSettings& sync, float /* videoFps */) {
        // By default, players only support setting sync source to default; all other sync
        // settings are ignored. There is no requirement for getters to return set values.
        if (sync.mSource != AVSYNC_SOURCE_DEFAULT) {
            return BAD_VALUE;
        }
        return OK;
    }
    virtual status_t getSyncSettings(
            AVSyncSettings* sync /* nonnull */, float* videoFps /* nonnull */) {
        *sync = AVSyncSettings();
        *videoFps = -1.f;
        return OK;
    }
    virtual status_t seekTo(
            int64_t msec, MediaPlayer2SeekMode mode = MediaPlayer2SeekMode::SEEK_PREVIOUS_SYNC) = 0;
    virtual status_t getCurrentPosition(int64_t *msec) = 0;
    virtual status_t getDuration(int64_t *msec) = 0;
    virtual status_t reset() = 0;
    virtual status_t notifyAt(int64_t /* mediaTimeUs */) {
        return INVALID_OPERATION;
    }
    virtual status_t setLooping(int loop) = 0;
    virtual status_t setParameter(int key, const Parcel &request) = 0;
    virtual status_t getParameter(int key, Parcel *reply) = 0;

    virtual status_t getMetrics(char **buffer, size_t *length) = 0;

    // Invoke a generic method on the player by using opaque parcels
    // for the request and reply.
    //
    // @param request Parcel that is positioned at the start of the
    //                data sent by the java layer.
    // @param[out] reply Parcel to hold the reply data. Cannot be null.
    // @return OK if the call was successful.
    virtual status_t invoke(const PlayerMessage &request, PlayerMessage *reply) = 0;

    void setListener(const sp<MediaPlayer2InterfaceListener> &listener) {
        Mutex::Autolock autoLock(mListenerLock);
        mListener = listener;
    }

    void sendEvent(int64_t srcId, int msg, int ext1=0, int ext2=0, const PlayerMessage *obj=NULL) {
        sp<MediaPlayer2InterfaceListener> listener;
        {
            Mutex::Autolock autoLock(mListenerLock);
            listener = mListener;
        }

        if (listener) {
            listener->notify(srcId, msg, ext1, ext2, obj);
        }
    }

    virtual status_t dump(int /* fd */, const Vector<String16>& /* args */) const {
        return INVALID_OPERATION;
    }

    virtual void onMessageReceived(const sp<AMessage> & /* msg */) override { }

    // Modular DRM
    virtual status_t prepareDrm(int64_t /*srcId*/, const uint8_t /* uuid */[16],
                                const Vector<uint8_t>& /* drmSessionId */) {
        return INVALID_OPERATION;
    }
    virtual status_t releaseDrm(int64_t /*srcId*/) {
        return INVALID_OPERATION;
    }

protected:
    sp<AudioSink> mAudioSink;

private:
    Mutex mListenerLock;
    sp<MediaPlayer2InterfaceListener> mListener;
};

}; // namespace android

#endif // __cplusplus


#endif // ANDROID_MEDIAPLAYER2INTERFACE_H
