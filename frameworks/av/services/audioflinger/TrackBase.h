/*
**
** Copyright 2012, The Android Open Source Project
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

// base for record and playback
class TrackBase : public ExtendedAudioBufferProvider, public RefBase {

public:
    enum track_state {
        IDLE,
        FLUSHED,        // for PlaybackTracks only
        STOPPED,
        // next 2 states are currently used for fast tracks
        // and offloaded tracks only
        STOPPING_1,     // waiting for first underrun
        STOPPING_2,     // waiting for presentation complete
        RESUMING,       // for PlaybackTracks only
        ACTIVE,
        PAUSING,
        PAUSED,
        STARTING_1,     // for RecordTrack only
        STARTING_2,     // for RecordTrack only
    };

    // where to allocate the data buffer
    enum alloc_type {
        ALLOC_CBLK,     // allocate immediately after control block
        ALLOC_READONLY, // allocate from a separate read-only heap per thread
        ALLOC_PIPE,     // do not allocate; use the pipe buffer
        ALLOC_LOCAL,    // allocate a local buffer
        ALLOC_NONE,     // do not allocate:use the buffer passed to TrackBase constructor
    };

    enum track_type {
        TYPE_DEFAULT,
        TYPE_OUTPUT,
        TYPE_PATCH,
    };

                        TrackBase(ThreadBase *thread,
                                const sp<Client>& client,
                                const audio_attributes_t& mAttr,
                                uint32_t sampleRate,
                                audio_format_t format,
                                audio_channel_mask_t channelMask,
                                size_t frameCount,
                                void *buffer,
                                size_t bufferSize,
                                audio_session_t sessionId,
                                pid_t creatorPid,
                                uid_t uid,
                                bool isOut,
                                alloc_type alloc = ALLOC_CBLK,
                                track_type type = TYPE_DEFAULT,
                                audio_port_handle_t portId = AUDIO_PORT_HANDLE_NONE);
    virtual             ~TrackBase();
    virtual status_t    initCheck() const;

    virtual status_t    start(AudioSystem::sync_event_t event,
                             audio_session_t triggerSession) = 0;
    virtual void        stop() = 0;
            sp<IMemory> getCblk() const { return mCblkMemory; }
            audio_track_cblk_t* cblk() const { return mCblk; }
            audio_session_t sessionId() const { return mSessionId; }
            uid_t       uid() const { return mUid; }
            pid_t       creatorPid() const { return mCreatorPid; }

            audio_port_handle_t portId() const { return mPortId; }
    virtual status_t    setSyncEvent(const sp<SyncEvent>& event);

            sp<IMemory> getBuffers() const { return mBufferMemory; }
            void*       buffer() const { return mBuffer; }
            size_t      bufferSize() const { return mBufferSize; }
    virtual bool        isFastTrack() const = 0;
    virtual bool        isDirect() const = 0;
            bool        isOutputTrack() const { return (mType == TYPE_OUTPUT); }
            bool        isPatchTrack() const { return (mType == TYPE_PATCH); }
            bool        isExternalTrack() const { return !isOutputTrack() && !isPatchTrack(); }

    virtual void        invalidate() { mIsInvalid = true; }
            bool        isInvalid() const { return mIsInvalid; }

            void        terminate() { mTerminated = true; }
            bool        isTerminated() const { return mTerminated; }

    audio_attributes_t  attributes() const { return mAttr; }

#ifdef TEE_SINK
           void         dumpTee(int fd, const std::string &reason) const {
                                mTee.dump(fd, reason);
                        }
#endif

            /** returns the buffer contents size converted to time in milliseconds
             * for PCM Playback or Record streaming tracks. The return value is zero for
             * PCM static tracks and not defined for non-PCM tracks.
             *
             * This may be called without the thread lock.
             */
    virtual double      bufferLatencyMs() const {
                            return mServerProxy->framesReadySafe() * 1000 / sampleRate();
                        }

            /** returns whether the track supports server latency computation.
             * This is set in the constructor and constant throughout the track lifetime.
             */

            bool        isServerLatencySupported() const { return mServerLatencySupported; }

            /** computes the server latency for PCM Playback or Record track
             * to the device sink/source.  This is the time for the next frame in the track buffer
             * written or read from the server thread to the device source or sink.
             *
             * This may be called without the thread lock, but latencyMs and fromTrack
             * may be not be synchronized. For example PatchPanel may not obtain the
             * thread lock before calling.
             *
             * \param latencyMs on success is set to the latency in milliseconds of the
             *        next frame written/read by the server thread to/from the track buffer
             *        from the device source/sink.
             * \param fromTrack on success is set to true if latency was computed directly
             *        from the track timestamp; otherwise set to false if latency was
             *        estimated from the server timestamp.
             *        fromTrack may be nullptr or omitted if not required.
             *
             * \returns OK or INVALID_OPERATION on failure.
             */
            status_t    getServerLatencyMs(double *latencyMs, bool *fromTrack = nullptr) const {
                            if (!isServerLatencySupported()) {
                                return INVALID_OPERATION;
                            }

                            // if no thread lock is acquired, these atomics are not
                            // synchronized with each other, considered a benign race.

                            const double serverLatencyMs = mServerLatencyMs.load();
                            if (serverLatencyMs == 0.) {
                                return INVALID_OPERATION;
                            }
                            if (fromTrack != nullptr) {
                                *fromTrack = mServerLatencyFromTrack.load();
                            }
                            *latencyMs = serverLatencyMs;
                            return OK;
                        }

            /** computes the total client latency for PCM Playback or Record tracks
             * for the next client app access to the device sink/source; i.e. the
             * server latency plus the buffer latency.
             *
             * This may be called without the thread lock, but latencyMs and fromTrack
             * may be not be synchronized. For example PatchPanel may not obtain the
             * thread lock before calling.
             *
             * \param latencyMs on success is set to the latency in milliseconds of the
             *        next frame written/read by the client app to/from the track buffer
             *        from the device sink/source.
             * \param fromTrack on success is set to true if latency was computed directly
             *        from the track timestamp; otherwise set to false if latency was
             *        estimated from the server timestamp.
             *        fromTrack may be nullptr or omitted if not required.
             *
             * \returns OK or INVALID_OPERATION on failure.
             */
            status_t    getTrackLatencyMs(double *latencyMs, bool *fromTrack = nullptr) const {
                            double serverLatencyMs;
                            status_t status = getServerLatencyMs(&serverLatencyMs, fromTrack);
                            if (status == OK) {
                                *latencyMs = serverLatencyMs + bufferLatencyMs();
                            }
                            return status;
                        }

           // TODO: Consider making this external.
           struct FrameTime {
               int64_t frames;
               int64_t timeNs;
           };

           // KernelFrameTime is updated per "mix" period even for non-pcm tracks.
           void         getKernelFrameTime(FrameTime *ft) const {
                           *ft = mKernelFrameTime.load();
                        }

           audio_format_t format() const { return mFormat; }
           int id() const { return mId; }

protected:
    DISALLOW_COPY_AND_ASSIGN(TrackBase);

    // AudioBufferProvider interface
    virtual status_t getNextBuffer(AudioBufferProvider::Buffer* buffer) = 0;
    virtual void releaseBuffer(AudioBufferProvider::Buffer* buffer);

    // ExtendedAudioBufferProvider interface is only needed for Track,
    // but putting it in TrackBase avoids the complexity of virtual inheritance
    virtual size_t  framesReady() const { return SIZE_MAX; }

    uint32_t channelCount() const { return mChannelCount; }

    size_t frameSize() const { return mFrameSize; }

    audio_channel_mask_t channelMask() const { return mChannelMask; }

    virtual uint32_t sampleRate() const { return mSampleRate; }

    bool isStopped() const {
        return (mState == STOPPED || mState == FLUSHED);
    }

    // for fast tracks and offloaded tracks only
    bool isStopping() const {
        return mState == STOPPING_1 || mState == STOPPING_2;
    }
    bool isStopping_1() const {
        return mState == STOPPING_1;
    }
    bool isStopping_2() const {
        return mState == STOPPING_2;
    }

    // Upper case characters are final states.
    // Lower case characters are transitory.
    const char *getTrackStateString() const {
        if (isTerminated()) {
            return "T ";
        }
        switch (mState) {
        case IDLE:
            return "I ";
        case STOPPING_1: // for Fast and Offload
            return "s1";
        case STOPPING_2: // for Fast and Offload
            return "s2";
        case STOPPED:
            return "S ";
        case RESUMING:
            return "r ";
        case ACTIVE:
            return "A ";
        case PAUSING:
            return "p ";
        case PAUSED:
            return "P ";
        case FLUSHED:
            return "F ";
        case STARTING_1: // for RecordTrack
            return "r1";
        case STARTING_2: // for RecordTrack
            return "r2";
        default:
            return "? ";
        }
    }

    bool isOut() const { return mIsOut; }
                                    // true for Track, false for RecordTrack,
                                    // this could be a track type if needed later

    const wp<ThreadBase> mThread;
    /*const*/ sp<Client> mClient;   // see explanation at ~TrackBase() why not const
    sp<IMemory>         mCblkMemory;
    audio_track_cblk_t* mCblk;
    sp<IMemory>         mBufferMemory;  // currently non-0 for fast RecordTrack only
    void*               mBuffer;    // start of track buffer, typically in shared memory
                                    // except for OutputTrack when it is in local memory
    size_t              mBufferSize; // size of mBuffer in bytes
    // we don't really need a lock for these
    track_state         mState;
    const audio_attributes_t mAttr;
    const uint32_t      mSampleRate;    // initial sample rate only; for tracks which
                        // support dynamic rates, the current value is in control block
    const audio_format_t mFormat;
    const audio_channel_mask_t mChannelMask;
    const uint32_t      mChannelCount;
    const size_t        mFrameSize; // AudioFlinger's view of frame size in shared memory,
                                    // where for AudioTrack (but not AudioRecord),
                                    // 8-bit PCM samples are stored as 16-bit
    const size_t        mFrameCount;// size of track buffer given at createTrack() or
                                    // createRecord(), and then adjusted as needed

    const audio_session_t mSessionId;
    uid_t               mUid;
    Vector < sp<SyncEvent> >mSyncEvents;
    const bool          mIsOut;
    sp<ServerProxy>     mServerProxy;
    const int           mId;
#ifdef TEE_SINK
    NBAIO_Tee           mTee;
#endif
    bool                mTerminated;
    track_type          mType;      // must be one of TYPE_DEFAULT, TYPE_OUTPUT, TYPE_PATCH ...
    audio_io_handle_t   mThreadIoHandle; // I/O handle of the thread the track is attached to
    audio_port_handle_t mPortId; // unique ID for this track used by audio policy
    bool                mIsInvalid; // non-resettable latch, set by invalidate()

    bool                mServerLatencySupported = false;
    std::atomic<bool>   mServerLatencyFromTrack{}; // latency from track or server timestamp.
    std::atomic<double> mServerLatencyMs{};        // last latency pushed from server thread.
    std::atomic<FrameTime> mKernelFrameTime{};     // last frame time on kernel side.
    const pid_t         mCreatorPid;  // can be different from mclient->pid() for instance
                                      // when created by NuPlayer on behalf of a client
};

// PatchProxyBufferProvider interface is implemented by PatchTrack and PatchRecord.
// it provides buffer access methods that map those of a ClientProxy (see AudioTrackShared.h)
class PatchProxyBufferProvider
{
public:

    virtual ~PatchProxyBufferProvider() {}

    virtual status_t    obtainBuffer(Proxy::Buffer* buffer,
                                     const struct timespec *requested = NULL) = 0;
    virtual void        releaseBuffer(Proxy::Buffer* buffer) = 0;
};

class PatchTrackBase : public PatchProxyBufferProvider
{
public:
    using Timeout = std::optional<std::chrono::nanoseconds>;
                        PatchTrackBase(sp<ClientProxy> proxy, const ThreadBase& thread,
                                       const Timeout& timeout);
            void        setPeerTimeout(std::chrono::nanoseconds timeout);
            template <typename T>
            void        setPeerProxy(const sp<T> &proxy, bool holdReference) {
                            mPeerReferenceHold = holdReference ? proxy : nullptr;
                            mPeerProxy = proxy.get();
                        }
            void        clearPeerProxy() {
                            mPeerReferenceHold.clear();
                            mPeerProxy = nullptr;
                        }

protected:
    const sp<ClientProxy>       mProxy;
    sp<RefBase>                 mPeerReferenceHold;   // keeps mPeerProxy alive during access.
    PatchProxyBufferProvider*   mPeerProxy = nullptr;
    struct timespec             mPeerTimeout{};

};
