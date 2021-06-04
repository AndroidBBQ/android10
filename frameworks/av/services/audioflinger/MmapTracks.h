/*
**
** Copyright 2017, The Android Open Source Project
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

#ifndef INCLUDING_FROM_AUDIOFLINGER_H
    #error This header file should only be included from AudioFlinger.h
#endif

// playback track
class MmapTrack : public TrackBase {
public:
                MmapTrack(ThreadBase *thread,
                            const audio_attributes_t& attr,
                            uint32_t sampleRate,
                            audio_format_t format,
                            audio_channel_mask_t channelMask,
                            audio_session_t sessionId,
                            bool isOut,
                            uid_t uid,
                            pid_t pid,
                            pid_t creatorPid,
                            audio_port_handle_t portId = AUDIO_PORT_HANDLE_NONE);
    virtual             ~MmapTrack();

                        // TrackBase virtual
    virtual status_t    initCheck() const;
    virtual status_t    start(AudioSystem::sync_event_t event,
                              audio_session_t triggerSession);
    virtual void        stop();
    virtual bool        isFastTrack() const { return false; }
            bool        isDirect() const override { return true; }

            void        appendDumpHeader(String8& result);
            void        appendDump(String8& result, bool active);

                        // protected by MMapThread::mLock
            void        setSilenced_l(bool silenced) { mSilenced = silenced;
                                                       mSilencedNotified = false;}
                        // protected by MMapThread::mLock
            bool        isSilenced_l() const { return mSilenced; }
                        // protected by MMapThread::mLock
            bool        getAndSetSilencedNotified_l() { bool silencedNotified = mSilencedNotified;
                                                        mSilencedNotified = true;
                                                        return silencedNotified; }
private:
    friend class MmapThread;

    DISALLOW_COPY_AND_ASSIGN(MmapTrack);

    // AudioBufferProvider interface
    virtual status_t getNextBuffer(AudioBufferProvider::Buffer* buffer);
    // releaseBuffer() not overridden

    // ExtendedAudioBufferProvider interface
    virtual size_t framesReady() const;
    virtual int64_t framesReleased() const;
    virtual void onTimestamp(const ExtendedTimestamp &timestamp);

    pid_t mPid;
    bool  mSilenced;            // protected by MMapThread::mLock
    bool  mSilencedNotified;    // protected by MMapThread::mLock
};  // end of Track

