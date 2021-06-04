/*
**
** Copyright 2018, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef ANDROID_MEDIAPLAYER2AUDIOOUTPUT_H
#define ANDROID_MEDIAPLAYER2AUDIOOUTPUT_H

#include <mediaplayer2/MediaPlayer2Interface.h>
#include <mediaplayer2/JAudioTrack.h>
#include <mediaplayer2/JObjectHolder.h>

#include <utility>
#include <utils/String16.h>
#include <utils/Vector.h>

#include "jni.h"

namespace android {

class AudioTrack;

class MediaPlayer2AudioOutput : public MediaPlayer2Interface::AudioSink
{
    class CallbackData;

public:
    MediaPlayer2AudioOutput(int32_t sessionId,
                            uid_t uid,
                            int pid,
                            const jobject attributes);
    virtual ~MediaPlayer2AudioOutput();

    virtual bool ready() const {
        return mJAudioTrack != nullptr;
    }
    virtual ssize_t bufferSize() const;
    virtual ssize_t frameCount() const;
    virtual ssize_t channelCount() const;
    virtual ssize_t frameSize() const;
    virtual uint32_t latency() const;
    virtual float msecsPerFrame() const;
    virtual status_t getPosition(uint32_t *position) const;
    virtual status_t getTimestamp(AudioTimestamp &ts) const;
    virtual int64_t getPlayedOutDurationUs(int64_t nowUs) const;
    virtual status_t getFramesWritten(uint32_t *frameswritten) const;
    virtual int32_t getSessionId() const;
    virtual void setSessionId(const int32_t id);
    virtual uint32_t getSampleRate() const;
    virtual int64_t getBufferDurationInUs() const;

    virtual status_t open(
            uint32_t sampleRate, int channelCount, audio_channel_mask_t channelMask,
            audio_format_t format,
            AudioCallback cb, void *cookie,
            audio_output_flags_t flags = AUDIO_OUTPUT_FLAG_NONE,
            const audio_offload_info_t *offloadInfo = NULL,
            uint32_t suggestedFrameCount = 0);

    virtual status_t start();
    virtual ssize_t write(const void* buffer, size_t size, bool blocking = true);
    virtual void stop();
    virtual void flush();
    virtual void pause();
    virtual void close();
    void setAudioAttributes(const jobject attributes);
    virtual audio_stream_type_t getAudioStreamType() const;

    void setVolume(float volume);
    virtual status_t setPlaybackRate(const AudioPlaybackRate& rate);
    virtual status_t getPlaybackRate(AudioPlaybackRate* rate /* nonnull */);

    status_t setAuxEffectSendLevel(float level);
    status_t attachAuxEffect(int effectId);
    virtual status_t dump(int fd, const Vector<String16>& args) const;

    static bool isOnEmulator();
    static int getMinBufferCount();
    virtual bool needsTrailingPadding() {
        return true;
        // TODO: return correct value.
        //return mNextOutput == NULL;
    }
    // AudioRouting
    virtual status_t setPreferredDevice(jobject device);
    virtual jobject getRoutedDevice();
    virtual status_t addAudioDeviceCallback(jobject routingDelegate);
    virtual status_t removeAudioDeviceCallback(jobject listener);

private:
    static void setMinBufferCount();
    static void CallbackWrapper(int event, void *me, void *info);
    void deleteRecycledTrack_l();
    void close_l();
    status_t updateTrack_l();

    sp<JAudioTrack>         mJAudioTrack;
    AudioCallback           mCallback;
    void *                  mCallbackCookie;
    CallbackData *          mCallbackData;
    sp<JObjectHolder>       mAttributes;
    float                   mVolume;
    AudioPlaybackRate       mPlaybackRate;
    uint32_t                mSampleRateHz; // sample rate of the content, as set in open()
    float                   mMsecsPerFrame;
    size_t                  mFrameSize;
    int32_t                 mSessionId;
    uid_t                   mUid;
    int                     mPid;
    float                   mSendLevel;
    int                     mAuxEffectId;
    audio_output_flags_t    mFlags;
    sp<JObjectHolder>       mPreferredDevice;
    mutable Mutex           mLock;

    // <listener, routingDelegate>
    Vector<std::pair<sp<JObjectHolder>, sp<JObjectHolder>>> mRoutingDelegates;

    // static variables below not protected by mutex
    static bool             mIsOnEmulator;
    static int              mMinBufferCount;  // 12 for emulator; otherwise 4

    // CallbackData is what is passed to the AudioTrack as the "user" data.
    // We need to be able to target this to a different Output on the fly,
    // so we can't use the Output itself for this.
    class CallbackData {
        friend MediaPlayer2AudioOutput;
    public:
        explicit CallbackData(MediaPlayer2AudioOutput *cookie) {
            mData = cookie;
            mSwitching = false;
        }
        MediaPlayer2AudioOutput *getOutput() const {
            return mData;
        }
        void setOutput(MediaPlayer2AudioOutput* newcookie) {
            mData = newcookie;
        }
        // lock/unlock are used by the callback before accessing the payload of this object
        void lock() const {
            mLock.lock();
        }
        void unlock() const {
            mLock.unlock();
        }

        // tryBeginTrackSwitch/endTrackSwitch are used when the CallbackData is handed over
        // to the next sink.

        // tryBeginTrackSwitch() returns true only if it obtains the lock.
        bool tryBeginTrackSwitch() {
            LOG_ALWAYS_FATAL_IF(mSwitching, "tryBeginTrackSwitch() already called");
            if (mLock.tryLock() != OK) {
                return false;
            }
            mSwitching = true;
            return true;
        }
        void endTrackSwitch() {
            if (mSwitching) {
                mLock.unlock();
            }
            mSwitching = false;
        }

    private:
        MediaPlayer2AudioOutput *mData;
        mutable Mutex mLock; // a recursive mutex might make this unnecessary.
        bool mSwitching;
        DISALLOW_EVIL_CONSTRUCTORS(CallbackData);
    };
};

}; // namespace android

#endif // ANDROID_MEDIAPLAYER2AUDIOOUTPUT_H
