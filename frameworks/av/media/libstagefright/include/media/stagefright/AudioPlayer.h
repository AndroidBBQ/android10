/*
 * Copyright (C) 2009 The Android Open Source Project
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

#ifndef AUDIO_PLAYER_H_

#define AUDIO_PLAYER_H_

#include <media/MediaSource.h>
#include <media/MediaPlayerInterface.h>
#include <media/stagefright/MediaBuffer.h>
#include <utils/threads.h>

namespace android {

struct AudioPlaybackRate;
class AudioTrack;
struct AwesomePlayer;

class AudioPlayer {
public:
    enum {
        REACHED_EOS,
        SEEK_COMPLETE
    };

    enum {
        ALLOW_DEEP_BUFFERING = 0x01,
        USE_OFFLOAD = 0x02,
        HAS_VIDEO   = 0x1000,
        IS_STREAMING = 0x2000

    };

    AudioPlayer(const sp<MediaPlayerBase::AudioSink> &audioSink,
                uint32_t flags = 0);

    virtual ~AudioPlayer();

    // Caller retains ownership of "source".
    void setSource(const sp<MediaSource> &source);

    status_t start(bool sourceAlreadyStarted = false);

    void pause(bool playPendingSamples = false);
    status_t resume();

    status_t seekTo(int64_t time_us);

    bool isSeeking();
    bool reachedEOS(status_t *finalStatus);

    status_t setPlaybackRate(const AudioPlaybackRate &rate);
    status_t getPlaybackRate(AudioPlaybackRate *rate /* nonnull */);

private:
    sp<MediaSource> mSource;
    sp<AudioTrack> mAudioTrack;

    MediaBufferBase *mInputBuffer;

    int mSampleRate;
    int64_t mLatencyUs;
    size_t mFrameSize;

    Mutex mLock;
    int64_t mNumFramesPlayed;
    int64_t mNumFramesPlayedSysTimeUs;

    int64_t mPositionTimeMediaUs;
    int64_t mPositionTimeRealUs;

    bool mSeeking;
    bool mReachedEOS;
    status_t mFinalStatus;
    int64_t mSeekTimeUs;

    bool mStarted;

    bool mIsFirstBuffer;
    status_t mFirstBufferResult;
    MediaBufferBase *mFirstBuffer;

    sp<MediaPlayerBase::AudioSink> mAudioSink;

    bool mPlaying;
    int64_t mStartPosUs;
    const uint32_t mCreateFlags;

    static void AudioCallback(int event, void *user, void *info);
    void AudioCallback(int event, void *info);

    static size_t AudioSinkCallback(
            MediaPlayerBase::AudioSink *audioSink,
            void *data, size_t size, void *me,
            MediaPlayerBase::AudioSink::cb_event_t event);

    size_t fillBuffer(void *data, size_t size);

    void reset();

    int64_t getOutputPlayPositionUs_l();

    bool allowDeepBuffering() const { return (mCreateFlags & ALLOW_DEEP_BUFFERING) != 0; }
    bool useOffload() const { return (mCreateFlags & USE_OFFLOAD) != 0; }

    AudioPlayer(const AudioPlayer &);
    AudioPlayer &operator=(const AudioPlayer &);
};

}  // namespace android

#endif  // AUDIO_PLAYER_H_
