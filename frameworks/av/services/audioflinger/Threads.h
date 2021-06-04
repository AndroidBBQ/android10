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

class ThreadBase : public Thread {
public:

#include "TrackBase.h"

    enum type_t {
        MIXER,              // Thread class is MixerThread
        DIRECT,             // Thread class is DirectOutputThread
        DUPLICATING,        // Thread class is DuplicatingThread
        RECORD,             // Thread class is RecordThread
        OFFLOAD,            // Thread class is OffloadThread
        MMAP                // control thread for MMAP stream
        // If you add any values here, also update ThreadBase::threadTypeToString()
    };

    static const char *threadTypeToString(type_t type);

    ThreadBase(const sp<AudioFlinger>& audioFlinger, audio_io_handle_t id,
                audio_devices_t outDevice, audio_devices_t inDevice, type_t type,
                bool systemReady);
    virtual             ~ThreadBase();

    virtual status_t    readyToRun();

    void clearPowerManager();

    // base for record and playback
    enum {
        CFG_EVENT_IO,
        CFG_EVENT_PRIO,
        CFG_EVENT_SET_PARAMETER,
        CFG_EVENT_CREATE_AUDIO_PATCH,
        CFG_EVENT_RELEASE_AUDIO_PATCH,
    };

    class ConfigEventData: public RefBase {
    public:
        virtual ~ConfigEventData() {}

        virtual  void dump(char *buffer, size_t size) = 0;
    protected:
        ConfigEventData() {}
    };

    // Config event sequence by client if status needed (e.g binder thread calling setParameters()):
    //  1. create SetParameterConfigEvent. This sets mWaitStatus in config event
    //  2. Lock mLock
    //  3. Call sendConfigEvent_l(): Append to mConfigEvents and mWaitWorkCV.signal
    //  4. sendConfigEvent_l() reads status from event->mStatus;
    //  5. sendConfigEvent_l() returns status
    //  6. Unlock
    //
    // Parameter sequence by server: threadLoop calling processConfigEvents_l():
    // 1. Lock mLock
    // 2. If there is an entry in mConfigEvents proceed ...
    // 3. Read first entry in mConfigEvents
    // 4. Remove first entry from mConfigEvents
    // 5. Process
    // 6. Set event->mStatus
    // 7. event->mCond.signal
    // 8. Unlock

    class ConfigEvent: public RefBase {
    public:
        virtual ~ConfigEvent() {}

        void dump(char *buffer, size_t size) { mData->dump(buffer, size); }

        const int mType; // event type e.g. CFG_EVENT_IO
        Mutex mLock;     // mutex associated with mCond
        Condition mCond; // condition for status return
        status_t mStatus; // status communicated to sender
        bool mWaitStatus; // true if sender is waiting for status
        bool mRequiresSystemReady; // true if must wait for system ready to enter event queue
        sp<ConfigEventData> mData;     // event specific parameter data

    protected:
        explicit ConfigEvent(int type, bool requiresSystemReady = false) :
            mType(type), mStatus(NO_ERROR), mWaitStatus(false),
            mRequiresSystemReady(requiresSystemReady), mData(NULL) {}
    };

    class IoConfigEventData : public ConfigEventData {
    public:
        IoConfigEventData(audio_io_config_event event, pid_t pid,
                          audio_port_handle_t portId) :
            mEvent(event), mPid(pid), mPortId(portId) {}

        virtual  void dump(char *buffer, size_t size) {
            snprintf(buffer, size, "IO event: event %d\n", mEvent);
        }

        const audio_io_config_event mEvent;
        const pid_t                 mPid;
        const audio_port_handle_t   mPortId;
    };

    class IoConfigEvent : public ConfigEvent {
    public:
        IoConfigEvent(audio_io_config_event event, pid_t pid, audio_port_handle_t portId) :
            ConfigEvent(CFG_EVENT_IO) {
            mData = new IoConfigEventData(event, pid, portId);
        }
        virtual ~IoConfigEvent() {}
    };

    class PrioConfigEventData : public ConfigEventData {
    public:
        PrioConfigEventData(pid_t pid, pid_t tid, int32_t prio, bool forApp) :
            mPid(pid), mTid(tid), mPrio(prio), mForApp(forApp) {}

        virtual  void dump(char *buffer, size_t size) {
            snprintf(buffer, size, "Prio event: pid %d, tid %d, prio %d, for app? %d\n",
                    mPid, mTid, mPrio, mForApp);
        }

        const pid_t mPid;
        const pid_t mTid;
        const int32_t mPrio;
        const bool mForApp;
    };

    class PrioConfigEvent : public ConfigEvent {
    public:
        PrioConfigEvent(pid_t pid, pid_t tid, int32_t prio, bool forApp) :
            ConfigEvent(CFG_EVENT_PRIO, true) {
            mData = new PrioConfigEventData(pid, tid, prio, forApp);
        }
        virtual ~PrioConfigEvent() {}
    };

    class SetParameterConfigEventData : public ConfigEventData {
    public:
        explicit SetParameterConfigEventData(String8 keyValuePairs) :
            mKeyValuePairs(keyValuePairs) {}

        virtual  void dump(char *buffer, size_t size) {
            snprintf(buffer, size, "KeyValue: %s\n", mKeyValuePairs.string());
        }

        const String8 mKeyValuePairs;
    };

    class SetParameterConfigEvent : public ConfigEvent {
    public:
        explicit SetParameterConfigEvent(String8 keyValuePairs) :
            ConfigEvent(CFG_EVENT_SET_PARAMETER) {
            mData = new SetParameterConfigEventData(keyValuePairs);
            mWaitStatus = true;
        }
        virtual ~SetParameterConfigEvent() {}
    };

    class CreateAudioPatchConfigEventData : public ConfigEventData {
    public:
        CreateAudioPatchConfigEventData(const struct audio_patch patch,
                                        audio_patch_handle_t handle) :
            mPatch(patch), mHandle(handle) {}

        virtual  void dump(char *buffer, size_t size) {
            snprintf(buffer, size, "Patch handle: %u\n", mHandle);
        }

        const struct audio_patch mPatch;
        audio_patch_handle_t mHandle;
    };

    class CreateAudioPatchConfigEvent : public ConfigEvent {
    public:
        CreateAudioPatchConfigEvent(const struct audio_patch patch,
                                    audio_patch_handle_t handle) :
            ConfigEvent(CFG_EVENT_CREATE_AUDIO_PATCH) {
            mData = new CreateAudioPatchConfigEventData(patch, handle);
            mWaitStatus = true;
        }
        virtual ~CreateAudioPatchConfigEvent() {}
    };

    class ReleaseAudioPatchConfigEventData : public ConfigEventData {
    public:
        explicit ReleaseAudioPatchConfigEventData(const audio_patch_handle_t handle) :
            mHandle(handle) {}

        virtual  void dump(char *buffer, size_t size) {
            snprintf(buffer, size, "Patch handle: %u\n", mHandle);
        }

        audio_patch_handle_t mHandle;
    };

    class ReleaseAudioPatchConfigEvent : public ConfigEvent {
    public:
        explicit ReleaseAudioPatchConfigEvent(const audio_patch_handle_t handle) :
            ConfigEvent(CFG_EVENT_RELEASE_AUDIO_PATCH) {
            mData = new ReleaseAudioPatchConfigEventData(handle);
            mWaitStatus = true;
        }
        virtual ~ReleaseAudioPatchConfigEvent() {}
    };

    class PMDeathRecipient : public IBinder::DeathRecipient {
    public:
        explicit    PMDeathRecipient(const wp<ThreadBase>& thread) : mThread(thread) {}
        virtual     ~PMDeathRecipient() {}

        // IBinder::DeathRecipient
        virtual     void        binderDied(const wp<IBinder>& who);

    private:
        DISALLOW_COPY_AND_ASSIGN(PMDeathRecipient);

        wp<ThreadBase> mThread;
    };

    virtual     status_t    initCheck() const = 0;

                // static externally-visible
                type_t      type() const { return mType; }
                bool isDuplicating() const { return (mType == DUPLICATING); }

                audio_io_handle_t id() const { return mId;}

                // dynamic externally-visible
                uint32_t    sampleRate() const { return mSampleRate; }
                audio_channel_mask_t channelMask() const { return mChannelMask; }
                audio_format_t format() const { return mHALFormat; }
                uint32_t channelCount() const { return mChannelCount; }
                // Called by AudioFlinger::frameCount(audio_io_handle_t output) and effects,
                // and returns the [normal mix] buffer's frame count.
    virtual     size_t      frameCount() const = 0;

                // Return's the HAL's frame count i.e. fast mixer buffer size.
                size_t      frameCountHAL() const { return mFrameCount; }

                size_t      frameSize() const { return mFrameSize; }

    // Should be "virtual status_t requestExitAndWait()" and override same
    // method in Thread, but Thread::requestExitAndWait() is not yet virtual.
                void        exit();
    virtual     bool        checkForNewParameter_l(const String8& keyValuePair,
                                                    status_t& status) = 0;
    virtual     status_t    setParameters(const String8& keyValuePairs);
    virtual     String8     getParameters(const String8& keys) = 0;
    virtual     void        ioConfigChanged(audio_io_config_event event, pid_t pid = 0,
                                        audio_port_handle_t portId = AUDIO_PORT_HANDLE_NONE) = 0;
                // sendConfigEvent_l() must be called with ThreadBase::mLock held
                // Can temporarily release the lock if waiting for a reply from
                // processConfigEvents_l().
                status_t    sendConfigEvent_l(sp<ConfigEvent>& event);
                void        sendIoConfigEvent(audio_io_config_event event, pid_t pid = 0,
                                              audio_port_handle_t portId = AUDIO_PORT_HANDLE_NONE);
                void        sendIoConfigEvent_l(audio_io_config_event event, pid_t pid = 0,
                                            audio_port_handle_t portId = AUDIO_PORT_HANDLE_NONE);
                void        sendPrioConfigEvent(pid_t pid, pid_t tid, int32_t prio, bool forApp);
                void        sendPrioConfigEvent_l(pid_t pid, pid_t tid, int32_t prio, bool forApp);
                status_t    sendSetParameterConfigEvent_l(const String8& keyValuePair);
                status_t    sendCreateAudioPatchConfigEvent(const struct audio_patch *patch,
                                                            audio_patch_handle_t *handle);
                status_t    sendReleaseAudioPatchConfigEvent(audio_patch_handle_t handle);
                void        processConfigEvents_l();
    virtual     void        cacheParameters_l() = 0;
    virtual     status_t    createAudioPatch_l(const struct audio_patch *patch,
                                               audio_patch_handle_t *handle) = 0;
    virtual     status_t    releaseAudioPatch_l(const audio_patch_handle_t handle) = 0;
    virtual     void        toAudioPortConfig(struct audio_port_config *config) = 0;


                // see note at declaration of mStandby, mOutDevice and mInDevice
                bool        standby() const { return mStandby; }
                audio_devices_t outDevice() const { return mOutDevice; }
                audio_devices_t inDevice() const { return mInDevice; }
                audio_devices_t getDevice() const { return isOutput() ? mOutDevice : mInDevice; }

    virtual     bool        isOutput() const = 0;

    virtual     sp<StreamHalInterface> stream() const = 0;

                sp<EffectHandle> createEffect_l(
                                    const sp<AudioFlinger::Client>& client,
                                    const sp<IEffectClient>& effectClient,
                                    int32_t priority,
                                    audio_session_t sessionId,
                                    effect_descriptor_t *desc,
                                    int *enabled,
                                    status_t *status /*non-NULL*/,
                                    bool pinned);

                // return values for hasAudioSession (bit field)
                enum effect_state {
                    EFFECT_SESSION = 0x1,   // the audio session corresponds to at least one
                                            // effect
                    TRACK_SESSION = 0x2,    // the audio session corresponds to at least one
                                            // track
                    FAST_SESSION = 0x4      // the audio session corresponds to at least one
                                            // fast track
                };

                // get effect chain corresponding to session Id.
                sp<EffectChain> getEffectChain(audio_session_t sessionId);
                // same as getEffectChain() but must be called with ThreadBase mutex locked
                sp<EffectChain> getEffectChain_l(audio_session_t sessionId) const;
                std::vector<int> getEffectIds_l(audio_session_t sessionId);
                // add an effect chain to the chain list (mEffectChains)
    virtual     status_t addEffectChain_l(const sp<EffectChain>& chain) = 0;
                // remove an effect chain from the chain list (mEffectChains)
    virtual     size_t removeEffectChain_l(const sp<EffectChain>& chain) = 0;
                // lock all effect chains Mutexes. Must be called before releasing the
                // ThreadBase mutex before processing the mixer and effects. This guarantees the
                // integrity of the chains during the process.
                // Also sets the parameter 'effectChains' to current value of mEffectChains.
                void lockEffectChains_l(Vector< sp<EffectChain> >& effectChains);
                // unlock effect chains after process
                void unlockEffectChains(const Vector< sp<EffectChain> >& effectChains);
                // get a copy of mEffectChains vector
                Vector< sp<EffectChain> > getEffectChains_l() const { return mEffectChains; };
                // set audio mode to all effect chains
                void setMode(audio_mode_t mode);
                // get effect module with corresponding ID on specified audio session
                sp<AudioFlinger::EffectModule> getEffect(audio_session_t sessionId, int effectId);
                sp<AudioFlinger::EffectModule> getEffect_l(audio_session_t sessionId, int effectId);
                // add and effect module. Also creates the effect chain is none exists for
                // the effects audio session. Only called in a context of moving an effect
                // from one thread to another
                status_t addEffect_l(const sp< EffectModule>& effect);
                // remove and effect module. Also removes the effect chain is this was the last
                // effect
                void removeEffect_l(const sp< EffectModule>& effect, bool release = false);
                // disconnect an effect handle from module and destroy module if last handle
                void disconnectEffectHandle(EffectHandle *handle, bool unpinIfLast);
                // detach all tracks connected to an auxiliary effect
    virtual     void detachAuxEffect_l(int effectId __unused) {}
                // returns a combination of:
                // - EFFECT_SESSION if effects on this audio session exist in one chain
                // - TRACK_SESSION if tracks on this audio session exist
                // - FAST_SESSION if fast tracks on this audio session exist
    virtual     uint32_t hasAudioSession_l(audio_session_t sessionId) const = 0;
                uint32_t hasAudioSession(audio_session_t sessionId) const {
                    Mutex::Autolock _l(mLock);
                    return hasAudioSession_l(sessionId);
                }

                template <typename T>
                uint32_t hasAudioSession_l(audio_session_t sessionId, const T& tracks) const {
                    uint32_t result = 0;
                    if (getEffectChain_l(sessionId) != 0) {
                        result = EFFECT_SESSION;
                    }
                    for (size_t i = 0; i < tracks.size(); ++i) {
                        const sp<TrackBase>& track = tracks[i];
                        if (sessionId == track->sessionId()
                                && !track->isInvalid()       // not yet removed from tracks.
                                && !track->isTerminated()) {
                            result |= TRACK_SESSION;
                            if (track->isFastTrack()) {
                                result |= FAST_SESSION;  // caution, only represents first track.
                            }
                            break;
                        }
                    }
                    return result;
                }

                // the value returned by default implementation is not important as the
                // strategy is only meaningful for PlaybackThread which implements this method
                virtual uint32_t getStrategyForSession_l(audio_session_t sessionId __unused)
                        { return 0; }

                // check if some effects must be suspended/restored when an effect is enabled
                // or disabled
                void checkSuspendOnEffectEnabled(const sp<EffectModule>& effect,
                                                 bool enabled,
                                                 audio_session_t sessionId =
                                                        AUDIO_SESSION_OUTPUT_MIX);
                void checkSuspendOnEffectEnabled_l(const sp<EffectModule>& effect,
                                                   bool enabled,
                                                   audio_session_t sessionId =
                                                        AUDIO_SESSION_OUTPUT_MIX);

                virtual status_t    setSyncEvent(const sp<SyncEvent>& event) = 0;
                virtual bool        isValidSyncEvent(const sp<SyncEvent>& event) const = 0;

                // Return a reference to a per-thread heap which can be used to allocate IMemory
                // objects that will be read-only to client processes, read/write to mediaserver,
                // and shared by all client processes of the thread.
                // The heap is per-thread rather than common across all threads, because
                // clients can't be trusted not to modify the offset of the IMemory they receive.
                // If a thread does not have such a heap, this method returns 0.
                virtual sp<MemoryDealer>    readOnlyHeap() const { return 0; }

                virtual sp<IMemory> pipeMemory() const { return 0; }

                        void systemReady();

                // checkEffectCompatibility_l() must be called with ThreadBase::mLock held
                virtual status_t    checkEffectCompatibility_l(const effect_descriptor_t *desc,
                                                               audio_session_t sessionId) = 0;

                        void        broadcast_l();

                virtual bool        isTimestampCorrectionEnabled() const { return false; }

                bool                isMsdDevice() const { return mIsMsdDevice; }

                void                dump(int fd, const Vector<String16>& args);

                // deliver stats to mediametrics.
                void                sendStatistics(bool force);

    mutable     Mutex                   mLock;

protected:

                // entry describing an effect being suspended in mSuspendedSessions keyed vector
                class SuspendedSessionDesc : public RefBase {
                public:
                    SuspendedSessionDesc() : mRefCount(0) {}

                    int mRefCount;          // number of active suspend requests
                    effect_uuid_t mType;    // effect type UUID
                };

                void        acquireWakeLock();
                virtual void acquireWakeLock_l();
                void        releaseWakeLock();
                void        releaseWakeLock_l();
                void        updateWakeLockUids_l(const SortedVector<uid_t> &uids);
                void        getPowerManager_l();
                // suspend or restore effects of the specified type (or all if type is NULL)
                // on a given session. The number of suspend requests is counted and restore
                // occurs when all suspend requests are cancelled.
                void setEffectSuspended_l(const effect_uuid_t *type,
                                          bool suspend,
                                          audio_session_t sessionId);
                // updated mSuspendedSessions when an effect is suspended or restored
                void        updateSuspendedSessions_l(const effect_uuid_t *type,
                                                      bool suspend,
                                                      audio_session_t sessionId);
                // check if some effects must be suspended when an effect chain is added
                void checkSuspendOnAddEffectChain_l(const sp<EffectChain>& chain);

                // sends the metadata of the active tracks to the HAL
    virtual     void        updateMetadata_l() = 0;

                String16 getWakeLockTag();

    virtual     void        preExit() { }
    virtual     void        setMasterMono_l(bool mono __unused) { }
    virtual     bool        requireMonoBlend() { return false; }

                            // called within the threadLoop to obtain timestamp from the HAL.
    virtual     status_t    threadloop_getHalTimestamp_l(
                                    ExtendedTimestamp *timestamp __unused) const {
                                return INVALID_OPERATION;
                            }

    virtual     void        dumpInternals_l(int fd __unused, const Vector<String16>& args __unused)
                            { }
    virtual     void        dumpTracks_l(int fd __unused, const Vector<String16>& args __unused) { }


    friend class AudioFlinger;      // for mEffectChains

                const type_t            mType;

                // Used by parameters, config events, addTrack_l, exit
                Condition               mWaitWorkCV;

                const sp<AudioFlinger>  mAudioFlinger;

                // updated by PlaybackThread::readOutputParameters_l() or
                // RecordThread::readInputParameters_l()
                uint32_t                mSampleRate;
                size_t                  mFrameCount;       // output HAL, direct output, record
                audio_channel_mask_t    mChannelMask;
                uint32_t                mChannelCount;
                size_t                  mFrameSize;
                // not HAL frame size, this is for output sink (to pipe to fast mixer)
                audio_format_t          mFormat;           // Source format for Recording and
                                                           // Sink format for Playback.
                                                           // Sink format may be different than
                                                           // HAL format if Fastmixer is used.
                audio_format_t          mHALFormat;
                size_t                  mBufferSize;       // HAL buffer size for read() or write()

                Vector< sp<ConfigEvent> >     mConfigEvents;
                Vector< sp<ConfigEvent> >     mPendingConfigEvents; // events awaiting system ready

                // These fields are written and read by thread itself without lock or barrier,
                // and read by other threads without lock or barrier via standby(), outDevice()
                // and inDevice().
                // Because of the absence of a lock or barrier, any other thread that reads
                // these fields must use the information in isolation, or be prepared to deal
                // with possibility that it might be inconsistent with other information.
                bool                    mStandby;     // Whether thread is currently in standby.
                audio_devices_t         mOutDevice;   // output device
                audio_devices_t         mInDevice;    // input device
                audio_devices_t         mPrevOutDevice;   // previous output device
                audio_devices_t         mPrevInDevice;    // previous input device
                struct audio_patch      mPatch;
                /**
                 * @brief mDeviceId  current device port unique identifier
                 */
                audio_port_handle_t     mDeviceId = AUDIO_PORT_HANDLE_NONE;
                audio_source_t          mAudioSource;

                const audio_io_handle_t mId;
                Vector< sp<EffectChain> > mEffectChains;

                static const int        kThreadNameLength = 16; // prctl(PR_SET_NAME) limit
                char                    mThreadName[kThreadNameLength]; // guaranteed NUL-terminated
                sp<IPowerManager>       mPowerManager;
                sp<IBinder>             mWakeLockToken;
                const sp<PMDeathRecipient> mDeathRecipient;
                // list of suspended effects per session and per type. The first (outer) vector is
                // keyed by session ID, the second (inner) by type UUID timeLow field
                // Updated by updateSuspendedSessions_l() only.
                KeyedVector< audio_session_t, KeyedVector< int, sp<SuspendedSessionDesc> > >
                                        mSuspendedSessions;
                // TODO: add comment and adjust size as needed
                static const size_t     kLogSize = 4 * 1024;
                sp<NBLog::Writer>       mNBLogWriter;
                bool                    mSystemReady;
                ExtendedTimestamp       mTimestamp;
                TimestampVerifier< // For timestamp statistics.
                        int64_t /* frame count */, int64_t /* time ns */> mTimestampVerifier;
                audio_devices_t         mTimestampCorrectedDevices = AUDIO_DEVICE_NONE;

                // ThreadLoop statistics per iteration.
                int64_t                 mLastIoBeginNs = -1;
                int64_t                 mLastIoEndNs = -1;

                // This should be read under ThreadBase lock (if not on the threadLoop thread).
                audio_utils::Statistics<double> mIoJitterMs{0.995 /* alpha */};
                audio_utils::Statistics<double> mProcessTimeMs{0.995 /* alpha */};
                audio_utils::Statistics<double> mLatencyMs{0.995 /* alpha */};

                // Save the last count when we delivered statistics to mediametrics.
                int64_t                 mLastRecordedTimestampVerifierN = 0;
                int64_t                 mLastRecordedTimeNs = 0;  // BOOTTIME to include suspend.

                bool                    mIsMsdDevice = false;
                // A condition that must be evaluated by the thread loop has changed and
                // we must not wait for async write callback in the thread loop before evaluating it
                bool                    mSignalPending;

#ifdef TEE_SINK
                NBAIO_Tee               mTee;
#endif
                // ActiveTracks is a sorted vector of track type T representing the
                // active tracks of threadLoop() to be considered by the locked prepare portion.
                // ActiveTracks should be accessed with the ThreadBase lock held.
                //
                // During processing and I/O, the threadLoop does not hold the lock;
                // hence it does not directly use ActiveTracks.  Care should be taken
                // to hold local strong references or defer removal of tracks
                // if the threadLoop may still be accessing those tracks due to mix, etc.
                //
                // This class updates power information appropriately.
                //

                template <typename T>
                class ActiveTracks {
                public:
                    explicit ActiveTracks(SimpleLog *localLog = nullptr)
                        : mActiveTracksGeneration(0)
                        , mLastActiveTracksGeneration(0)
                        , mLocalLog(localLog)
                    { }

                    ~ActiveTracks() {
                        ALOGW_IF(!mActiveTracks.isEmpty(),
                                "ActiveTracks should be empty in destructor");
                    }
                    // returns the last track added (even though it may have been
                    // subsequently removed from ActiveTracks).
                    //
                    // Used for DirectOutputThread to ensure a flush is called when transitioning
                    // to a new track (even though it may be on the same session).
                    // Used for OffloadThread to ensure that volume and mixer state is
                    // taken from the latest track added.
                    //
                    // The latest track is saved with a weak pointer to prevent keeping an
                    // otherwise useless track alive. Thus the function will return nullptr
                    // if the latest track has subsequently been removed and destroyed.
                    sp<T> getLatest() {
                        return mLatestActiveTrack.promote();
                    }

                    // SortedVector methods
                    ssize_t         add(const sp<T> &track);
                    ssize_t         remove(const sp<T> &track);
                    size_t          size() const {
                        return mActiveTracks.size();
                    }
                    bool            isEmpty() const {
                        return mActiveTracks.isEmpty();
                    }
                    ssize_t         indexOf(const sp<T>& item) {
                        return mActiveTracks.indexOf(item);
                    }
                    sp<T>           operator[](size_t index) const {
                        return mActiveTracks[index];
                    }
                    typename SortedVector<sp<T>>::iterator begin() {
                        return mActiveTracks.begin();
                    }
                    typename SortedVector<sp<T>>::iterator end() {
                        return mActiveTracks.end();
                    }

                    // Due to Binder recursion optimization, clear() and updatePowerState()
                    // cannot be called from a Binder thread because they may call back into
                    // the original calling process (system server) for BatteryNotifier
                    // (which requires a Java environment that may not be present).
                    // Hence, call clear() and updatePowerState() only from the
                    // ThreadBase thread.
                    void            clear();
                    // periodically called in the threadLoop() to update power state uids.
                    void            updatePowerState(sp<ThreadBase> thread, bool force = false);

                    /** @return true if one or move active tracks was added or removed since the
                     *          last time this function was called or the vector was created. */
                    bool            readAndClearHasChanged();

                private:
                    void            logTrack(const char *funcName, const sp<T> &track) const;

                    SortedVector<uid_t> getWakeLockUids() {
                        SortedVector<uid_t> wakeLockUids;
                        for (const sp<T> &track : mActiveTracks) {
                            wakeLockUids.add(track->uid());
                        }
                        return wakeLockUids; // moved by underlying SharedBuffer
                    }

                    std::map<uid_t, std::pair<ssize_t /* previous */, ssize_t /* current */>>
                                        mBatteryCounter;
                    SortedVector<sp<T>> mActiveTracks;
                    int                 mActiveTracksGeneration;
                    int                 mLastActiveTracksGeneration;
                    wp<T>               mLatestActiveTrack; // latest track added to ActiveTracks
                    SimpleLog * const   mLocalLog;
                    // If the vector has changed since last call to readAndClearHasChanged
                    bool                mHasChanged = false;
                };

                SimpleLog mLocalLog;

private:
                void dumpBase_l(int fd, const Vector<String16>& args);
                void dumpEffectChains_l(int fd, const Vector<String16>& args);
};

class VolumeInterface {
 public:

    virtual ~VolumeInterface() {}

    virtual void        setMasterVolume(float value) = 0;
    virtual void        setMasterMute(bool muted) = 0;
    virtual void        setStreamVolume(audio_stream_type_t stream, float value) = 0;
    virtual void        setStreamMute(audio_stream_type_t stream, bool muted) = 0;
    virtual float       streamVolume(audio_stream_type_t stream) const = 0;

};

// --- PlaybackThread ---
class PlaybackThread : public ThreadBase, public StreamOutHalInterfaceCallback,
    public VolumeInterface {
public:

#include "PlaybackTracks.h"

    enum mixer_state {
        MIXER_IDLE,             // no active tracks
        MIXER_TRACKS_ENABLED,   // at least one active track, but no track has any data ready
        MIXER_TRACKS_READY,      // at least one active track, and at least one track has data
        MIXER_DRAIN_TRACK,      // drain currently playing track
        MIXER_DRAIN_ALL,        // fully drain the hardware
        // standby mode does not have an enum value
        // suspend by audio policy manager is orthogonal to mixer state
    };

    // retry count before removing active track in case of underrun on offloaded thread:
    // we need to make sure that AudioTrack client has enough time to send large buffers
    //FIXME may be more appropriate if expressed in time units. Need to revise how underrun is
    // handled for offloaded tracks
    static const int8_t kMaxTrackRetriesOffload = 20;
    static const int8_t kMaxTrackStartupRetriesOffload = 100;
    static const int8_t kMaxTrackStopRetriesOffload = 2;
    static constexpr uint32_t kMaxTracksPerUid = 40;
    static constexpr size_t kMaxTracks = 256;

    // Maximum delay (in nanoseconds) for upcoming buffers in suspend mode, otherwise
    // if delay is greater, the estimated time for timeLoopNextNs is reset.
    // This allows for catch-up to be done for small delays, while resetting the estimate
    // for initial conditions or large delays.
    static const nsecs_t kMaxNextBufferDelayNs = 100000000;

    PlaybackThread(const sp<AudioFlinger>& audioFlinger, AudioStreamOut* output,
                   audio_io_handle_t id, audio_devices_t device, type_t type, bool systemReady);
    virtual             ~PlaybackThread();

    // Thread virtuals
    virtual     bool        threadLoop();

    // RefBase
    virtual     void        onFirstRef();

    virtual     status_t    checkEffectCompatibility_l(const effect_descriptor_t *desc,
                                                       audio_session_t sessionId);

protected:
    // Code snippets that were lifted up out of threadLoop()
    virtual     void        threadLoop_mix() = 0;
    virtual     void        threadLoop_sleepTime() = 0;
    virtual     ssize_t     threadLoop_write();
    virtual     void        threadLoop_drain();
    virtual     void        threadLoop_standby();
    virtual     void        threadLoop_exit();
    virtual     void        threadLoop_removeTracks(const Vector< sp<Track> >& tracksToRemove);

                // prepareTracks_l reads and writes mActiveTracks, and returns
                // the pending set of tracks to remove via Vector 'tracksToRemove'.  The caller
                // is responsible for clearing or destroying this Vector later on, when it
                // is safe to do so. That will drop the final ref count and destroy the tracks.
    virtual     mixer_state prepareTracks_l(Vector< sp<Track> > *tracksToRemove) = 0;
                void        removeTracks_l(const Vector< sp<Track> >& tracksToRemove);
                status_t    handleVoipVolume_l(float *volume);

    // StreamOutHalInterfaceCallback implementation
    virtual     void        onWriteReady();
    virtual     void        onDrainReady();
    virtual     void        onError();

                void        resetWriteBlocked(uint32_t sequence);
                void        resetDraining(uint32_t sequence);

    virtual     bool        waitingAsyncCallback();
    virtual     bool        waitingAsyncCallback_l();
    virtual     bool        shouldStandby_l();
    virtual     void        onAddNewTrack_l();
                void        onAsyncError(); // error reported by AsyncCallbackThread

    // ThreadBase virtuals
    virtual     void        preExit();

    virtual     bool        keepWakeLock() const { return true; }
    virtual     void        acquireWakeLock_l() {
                                ThreadBase::acquireWakeLock_l();
                                mActiveTracks.updatePowerState(this, true /* force */);
                            }

                void        dumpInternals_l(int fd, const Vector<String16>& args) override;
                void        dumpTracks_l(int fd, const Vector<String16>& args) override;

public:

    virtual     status_t    initCheck() const { return (mOutput == NULL) ? NO_INIT : NO_ERROR; }

                // return estimated latency in milliseconds, as reported by HAL
                uint32_t    latency() const;
                // same, but lock must already be held
                uint32_t    latency_l() const;

                // VolumeInterface
    virtual     void        setMasterVolume(float value);
    virtual     void        setMasterBalance(float balance);
    virtual     void        setMasterMute(bool muted);
    virtual     void        setStreamVolume(audio_stream_type_t stream, float value);
    virtual     void        setStreamMute(audio_stream_type_t stream, bool muted);
    virtual     float       streamVolume(audio_stream_type_t stream) const;

                void        setVolumeForOutput_l(float left, float right) const;

                sp<Track>   createTrack_l(
                                const sp<AudioFlinger::Client>& client,
                                audio_stream_type_t streamType,
                                const audio_attributes_t& attr,
                                uint32_t *sampleRate,
                                audio_format_t format,
                                audio_channel_mask_t channelMask,
                                size_t *pFrameCount,
                                size_t *pNotificationFrameCount,
                                uint32_t notificationsPerBuffer,
                                float speed,
                                const sp<IMemory>& sharedBuffer,
                                audio_session_t sessionId,
                                audio_output_flags_t *flags,
                                pid_t creatorPid,
                                pid_t tid,
                                uid_t uid,
                                status_t *status /*non-NULL*/,
                                audio_port_handle_t portId);

                AudioStreamOut* getOutput() const;
                AudioStreamOut* clearOutput();
                virtual sp<StreamHalInterface> stream() const;

                // a very large number of suspend() will eventually wraparound, but unlikely
                void        suspend() { (void) android_atomic_inc(&mSuspended); }
                void        restore()
                                {
                                    // if restore() is done without suspend(), get back into
                                    // range so that the next suspend() will operate correctly
                                    if (android_atomic_dec(&mSuspended) <= 0) {
                                        android_atomic_release_store(0, &mSuspended);
                                    }
                                }
                bool        isSuspended() const
                                { return android_atomic_acquire_load(&mSuspended) > 0; }

    virtual     String8     getParameters(const String8& keys);
    virtual     void        ioConfigChanged(audio_io_config_event event, pid_t pid = 0,
                                            audio_port_handle_t portId = AUDIO_PORT_HANDLE_NONE);
                status_t    getRenderPosition(uint32_t *halFrames, uint32_t *dspFrames);
                // Consider also removing and passing an explicit mMainBuffer initialization
                // parameter to AF::PlaybackThread::Track::Track().
                effect_buffer_t *sinkBuffer() const {
                    return reinterpret_cast<effect_buffer_t *>(mSinkBuffer); };

    virtual     void detachAuxEffect_l(int effectId);
                status_t attachAuxEffect(const sp<AudioFlinger::PlaybackThread::Track>& track,
                        int EffectId);
                status_t attachAuxEffect_l(const sp<AudioFlinger::PlaybackThread::Track>& track,
                        int EffectId);

                virtual status_t addEffectChain_l(const sp<EffectChain>& chain);
                virtual size_t removeEffectChain_l(const sp<EffectChain>& chain);
                        uint32_t hasAudioSession_l(audio_session_t sessionId) const override {
                            return ThreadBase::hasAudioSession_l(sessionId, mTracks);
                        }
                virtual uint32_t getStrategyForSession_l(audio_session_t sessionId);


                virtual status_t setSyncEvent(const sp<SyncEvent>& event);
                virtual bool     isValidSyncEvent(const sp<SyncEvent>& event) const;

                // called with AudioFlinger lock held
                        bool     invalidateTracks_l(audio_stream_type_t streamType);
                virtual void     invalidateTracks(audio_stream_type_t streamType);

    virtual     size_t      frameCount() const { return mNormalFrameCount; }

                status_t    getTimestamp_l(AudioTimestamp& timestamp);

                void        addPatchTrack(const sp<PatchTrack>& track);
                void        deletePatchTrack(const sp<PatchTrack>& track);

    virtual     void        toAudioPortConfig(struct audio_port_config *config);

                // Return the asynchronous signal wait time.
    virtual     int64_t     computeWaitTimeNs_l() const { return INT64_MAX; }

    virtual     bool        isOutput() const override { return true; }

                // returns true if the track is allowed to be added to the thread.
    virtual     bool        isTrackAllowed_l(
                                    audio_channel_mask_t channelMask __unused,
                                    audio_format_t format __unused,
                                    audio_session_t sessionId __unused,
                                    uid_t uid) const {
                                return trackCountForUid_l(uid) < PlaybackThread::kMaxTracksPerUid
                                       && mTracks.size() < PlaybackThread::kMaxTracks;
                            }

                bool        isTimestampCorrectionEnabled() const override {
                                const audio_devices_t device =
                                        mOutDevice & mTimestampCorrectedDevices;
                                return audio_is_output_devices(device) && popcount(device) > 0;
                            }
protected:
    // updated by readOutputParameters_l()
    size_t                          mNormalFrameCount;  // normal mixer and effects

    bool                            mThreadThrottle;     // throttle the thread processing
    uint32_t                        mThreadThrottleTimeMs; // throttle time for MIXER threads
    uint32_t                        mThreadThrottleEndMs;  // notify once per throttling
    uint32_t                        mHalfBufferMs;       // half the buffer size in milliseconds

    void*                           mSinkBuffer;         // frame size aligned sink buffer

    // TODO:
    // Rearrange the buffer info into a struct/class with
    // clear, copy, construction, destruction methods.
    //
    // mSinkBuffer also has associated with it:
    //
    // mSinkBufferSize: Sink Buffer Size
    // mFormat: Sink Buffer Format

    // Mixer Buffer (mMixerBuffer*)
    //
    // In the case of floating point or multichannel data, which is not in the
    // sink format, it is required to accumulate in a higher precision or greater channel count
    // buffer before downmixing or data conversion to the sink buffer.

    // Set to "true" to enable the Mixer Buffer otherwise mixer output goes to sink buffer.
    bool                            mMixerBufferEnabled;

    // Storage, 32 byte aligned (may make this alignment a requirement later).
    // Due to constraints on mNormalFrameCount, the buffer size is a multiple of 16 frames.
    void*                           mMixerBuffer;

    // Size of mMixerBuffer in bytes: mNormalFrameCount * #channels * sampsize.
    size_t                          mMixerBufferSize;

    // The audio format of mMixerBuffer. Set to AUDIO_FORMAT_PCM_(FLOAT|16_BIT) only.
    audio_format_t                  mMixerBufferFormat;

    // An internal flag set to true by MixerThread::prepareTracks_l()
    // when mMixerBuffer contains valid data after mixing.
    bool                            mMixerBufferValid;

    // Effects Buffer (mEffectsBuffer*)
    //
    // In the case of effects data, which is not in the sink format,
    // it is required to accumulate in a different buffer before data conversion
    // to the sink buffer.

    // Set to "true" to enable the Effects Buffer otherwise effects output goes to sink buffer.
    bool                            mEffectBufferEnabled;

    // Storage, 32 byte aligned (may make this alignment a requirement later).
    // Due to constraints on mNormalFrameCount, the buffer size is a multiple of 16 frames.
    void*                           mEffectBuffer;

    // Size of mEffectsBuffer in bytes: mNormalFrameCount * #channels * sampsize.
    size_t                          mEffectBufferSize;

    // The audio format of mEffectsBuffer. Set to AUDIO_FORMAT_PCM_16_BIT only.
    audio_format_t                  mEffectBufferFormat;

    // An internal flag set to true by MixerThread::prepareTracks_l()
    // when mEffectsBuffer contains valid data after mixing.
    //
    // When this is set, all mixer data is routed into the effects buffer
    // for any processing (including output processing).
    bool                            mEffectBufferValid;

    // suspend count, > 0 means suspended.  While suspended, the thread continues to pull from
    // tracks and mix, but doesn't write to HAL.  A2DP and SCO HAL implementations can't handle
    // concurrent use of both of them, so Audio Policy Service suspends one of the threads to
    // workaround that restriction.
    // 'volatile' means accessed via atomic operations and no lock.
    volatile int32_t                mSuspended;

    int64_t                         mBytesWritten;
    int64_t                         mFramesWritten; // not reset on standby
    int64_t                         mSuspendedFrames; // not reset on standby

    // mHapticChannelMask and mHapticChannelCount will only be valid when the thread support
    // haptic playback.
    audio_channel_mask_t            mHapticChannelMask = AUDIO_CHANNEL_NONE;
    uint32_t                        mHapticChannelCount = 0;
private:
    // mMasterMute is in both PlaybackThread and in AudioFlinger.  When a
    // PlaybackThread needs to find out if master-muted, it checks it's local
    // copy rather than the one in AudioFlinger.  This optimization saves a lock.
    bool                            mMasterMute;
                void        setMasterMute_l(bool muted) { mMasterMute = muted; }
protected:
    ActiveTracks<Track>     mActiveTracks;

    // Time to sleep between cycles when:
    virtual uint32_t        activeSleepTimeUs() const;      // mixer state MIXER_TRACKS_ENABLED
    virtual uint32_t        idleSleepTimeUs() const = 0;    // mixer state MIXER_IDLE
    virtual uint32_t        suspendSleepTimeUs() const = 0; // audio policy manager suspended us
    // No sleep when mixer state == MIXER_TRACKS_READY; relies on audio HAL stream->write()
    // No sleep in standby mode; waits on a condition

    // Code snippets that are temporarily lifted up out of threadLoop() until the merge
                void        checkSilentMode_l();

    // Non-trivial for DUPLICATING only
    virtual     void        saveOutputTracks() { }
    virtual     void        clearOutputTracks() { }

    // Cache various calculated values, at threadLoop() entry and after a parameter change
    virtual     void        cacheParameters_l();

    virtual     uint32_t    correctLatency_l(uint32_t latency) const;

    virtual     status_t    createAudioPatch_l(const struct audio_patch *patch,
                                   audio_patch_handle_t *handle);
    virtual     status_t    releaseAudioPatch_l(const audio_patch_handle_t handle);

                bool        usesHwAvSync() const { return (mType == DIRECT) && (mOutput != NULL)
                                    && mHwSupportsPause
                                    && (mOutput->flags & AUDIO_OUTPUT_FLAG_HW_AV_SYNC); }

                uint32_t    trackCountForUid_l(uid_t uid) const;

private:

    friend class AudioFlinger;      // for numerous

    DISALLOW_COPY_AND_ASSIGN(PlaybackThread);

    status_t    addTrack_l(const sp<Track>& track);
    bool        destroyTrack_l(const sp<Track>& track);
    void        removeTrack_l(const sp<Track>& track);

    void        readOutputParameters_l();
    void        updateMetadata_l() final;
    virtual void sendMetadataToBackend_l(const StreamOutHalInterface::SourceMetadata& metadata);

    // The Tracks class manages tracks added and removed from the Thread.
    template <typename T>
    class Tracks {
    public:
        Tracks(bool saveDeletedTrackIds) :
            mSaveDeletedTrackIds(saveDeletedTrackIds) { }

        // SortedVector methods
        ssize_t         add(const sp<T> &track) {
            const ssize_t index = mTracks.add(track);
            LOG_ALWAYS_FATAL_IF(index < 0, "cannot add track");
            return index;
        }
        ssize_t         remove(const sp<T> &track);
        size_t          size() const {
            return mTracks.size();
        }
        bool            isEmpty() const {
            return mTracks.isEmpty();
        }
        ssize_t         indexOf(const sp<T> &item) {
            return mTracks.indexOf(item);
        }
        sp<T>           operator[](size_t index) const {
            return mTracks[index];
        }
        typename SortedVector<sp<T>>::iterator begin() {
            return mTracks.begin();
        }
        typename SortedVector<sp<T>>::iterator end() {
            return mTracks.end();
        }

        size_t          processDeletedTrackIds(std::function<void(int)> f) {
            for (const int trackId : mDeletedTrackIds) {
                f(trackId);
            }
            return mDeletedTrackIds.size();
        }

        void            clearDeletedTrackIds() { mDeletedTrackIds.clear(); }

    private:
        // Tracks pending deletion for MIXER type threads
        const bool mSaveDeletedTrackIds; // true to enable tracking
        std::set<int> mDeletedTrackIds;

        SortedVector<sp<T>> mTracks; // wrapped SortedVector.
    };

    Tracks<Track>                   mTracks;

    stream_type_t                   mStreamTypes[AUDIO_STREAM_CNT];
    AudioStreamOut                  *mOutput;

    float                           mMasterVolume;
    std::atomic<float>              mMasterBalance{};
    audio_utils::Balance            mBalance;
    int                             mNumWrites;
    int                             mNumDelayedWrites;
    bool                            mInWrite;

    // FIXME rename these former local variables of threadLoop to standard "m" names
    nsecs_t                         mStandbyTimeNs;
    size_t                          mSinkBufferSize;

    // cached copies of activeSleepTimeUs() and idleSleepTimeUs() made by cacheParameters_l()
    uint32_t                        mActiveSleepTimeUs;
    uint32_t                        mIdleSleepTimeUs;

    uint32_t                        mSleepTimeUs;

    // mixer status returned by prepareTracks_l()
    mixer_state                     mMixerStatus; // current cycle
                                                  // previous cycle when in prepareTracks_l()
    mixer_state                     mMixerStatusIgnoringFastTracks;
                                                  // FIXME or a separate ready state per track

    // FIXME move these declarations into the specific sub-class that needs them
    // MIXER only
    uint32_t                        sleepTimeShift;

    // same as AudioFlinger::mStandbyTimeInNsecs except for DIRECT which uses a shorter value
    nsecs_t                         mStandbyDelayNs;

    // MIXER only
    nsecs_t                         maxPeriod;

    // DUPLICATING only
    uint32_t                        writeFrames;

    size_t                          mBytesRemaining;
    size_t                          mCurrentWriteLength;
    bool                            mUseAsyncWrite;
    // mWriteAckSequence contains current write sequence on bits 31-1. The write sequence is
    // incremented each time a write(), a flush() or a standby() occurs.
    // Bit 0 is set when a write blocks and indicates a callback is expected.
    // Bit 0 is reset by the async callback thread calling resetWriteBlocked(). Out of sequence
    // callbacks are ignored.
    uint32_t                        mWriteAckSequence;
    // mDrainSequence contains current drain sequence on bits 31-1. The drain sequence is
    // incremented each time a drain is requested or a flush() or standby() occurs.
    // Bit 0 is set when the drain() command is called at the HAL and indicates a callback is
    // expected.
    // Bit 0 is reset by the async callback thread calling resetDraining(). Out of sequence
    // callbacks are ignored.
    uint32_t                        mDrainSequence;
    sp<AsyncCallbackThread>         mCallbackThread;

private:
    // The HAL output sink is treated as non-blocking, but current implementation is blocking
    sp<NBAIO_Sink>          mOutputSink;
    // If a fast mixer is present, the blocking pipe sink, otherwise clear
    sp<NBAIO_Sink>          mPipeSink;
    // The current sink for the normal mixer to write it's (sub)mix, mOutputSink or mPipeSink
    sp<NBAIO_Sink>          mNormalSink;
    uint32_t                mScreenState;   // cached copy of gScreenState
    // TODO: add comment and adjust size as needed
    static const size_t     kFastMixerLogSize = 8 * 1024;
    sp<NBLog::Writer>       mFastMixerNBLogWriter;

    // Downstream patch latency, available if mDownstreamLatencyStatMs.getN() > 0.
    audio_utils::Statistics<double> mDownstreamLatencyStatMs{0.999};

public:
    virtual     bool        hasFastMixer() const = 0;
    virtual     FastTrackUnderruns getFastTrackUnderruns(size_t fastIndex __unused) const
                                { FastTrackUnderruns dummy; return dummy; }

protected:
                // accessed by both binder threads and within threadLoop(), lock on mutex needed
                unsigned    mFastTrackAvailMask;    // bit i set if fast track [i] is available
                bool        mHwSupportsPause;
                bool        mHwPaused;
                bool        mFlushPending;
                // volumes last sent to audio HAL with stream->setVolume()
                float mLeftVolFloat;
                float mRightVolFloat;
};

class MixerThread : public PlaybackThread {
public:
    MixerThread(const sp<AudioFlinger>& audioFlinger,
                AudioStreamOut* output,
                audio_io_handle_t id,
                audio_devices_t device,
                bool systemReady,
                type_t type = MIXER);
    virtual             ~MixerThread();

    // Thread virtuals

    virtual     bool        checkForNewParameter_l(const String8& keyValuePair,
                                                   status_t& status);

    virtual     bool        isTrackAllowed_l(
                                    audio_channel_mask_t channelMask, audio_format_t format,
                                    audio_session_t sessionId, uid_t uid) const override;
protected:
    virtual     mixer_state prepareTracks_l(Vector< sp<Track> > *tracksToRemove);
    virtual     uint32_t    idleSleepTimeUs() const;
    virtual     uint32_t    suspendSleepTimeUs() const;
    virtual     void        cacheParameters_l();

    virtual void acquireWakeLock_l() {
        PlaybackThread::acquireWakeLock_l();
        if (hasFastMixer()) {
            mFastMixer->setBoottimeOffset(
                    mTimestamp.mTimebaseOffset[ExtendedTimestamp::TIMEBASE_BOOTTIME]);
        }
    }

                void        dumpInternals_l(int fd, const Vector<String16>& args) override;

    // threadLoop snippets
    virtual     ssize_t     threadLoop_write();
    virtual     void        threadLoop_standby();
    virtual     void        threadLoop_mix();
    virtual     void        threadLoop_sleepTime();
    virtual     uint32_t    correctLatency_l(uint32_t latency) const;

    virtual     status_t    createAudioPatch_l(const struct audio_patch *patch,
                                   audio_patch_handle_t *handle);
    virtual     status_t    releaseAudioPatch_l(const audio_patch_handle_t handle);

                AudioMixer* mAudioMixer;    // normal mixer
private:
                // one-time initialization, no locks required
                sp<FastMixer>     mFastMixer;     // non-0 if there is also a fast mixer
                sp<AudioWatchdog> mAudioWatchdog; // non-0 if there is an audio watchdog thread

                // contents are not guaranteed to be consistent, no locks required
                FastMixerDumpState mFastMixerDumpState;
#ifdef STATE_QUEUE_DUMP
                StateQueueObserverDump mStateQueueObserverDump;
                StateQueueMutatorDump  mStateQueueMutatorDump;
#endif
                AudioWatchdogDump mAudioWatchdogDump;

                // accessible only within the threadLoop(), no locks required
                //          mFastMixer->sq()    // for mutating and pushing state
                int32_t     mFastMixerFutex;    // for cold idle

                std::atomic_bool mMasterMono;
public:
    virtual     bool        hasFastMixer() const { return mFastMixer != 0; }
    virtual     FastTrackUnderruns getFastTrackUnderruns(size_t fastIndex) const {
                              ALOG_ASSERT(fastIndex < FastMixerState::sMaxFastTracks);
                              return mFastMixerDumpState.mTracks[fastIndex].mUnderruns;
                            }

                status_t    threadloop_getHalTimestamp_l(
                                    ExtendedTimestamp *timestamp) const override {
                                if (mNormalSink.get() != nullptr) {
                                    return mNormalSink->getTimestamp(*timestamp);
                                }
                                return INVALID_OPERATION;
                            }

protected:
    virtual     void       setMasterMono_l(bool mono) {
                               mMasterMono.store(mono);
                               if (mFastMixer != nullptr) { /* hasFastMixer() */
                                   mFastMixer->setMasterMono(mMasterMono);
                               }
                           }
                // the FastMixer performs mono blend if it exists.
                // Blending with limiter is not idempotent,
                // and blending without limiter is idempotent but inefficient to do twice.
    virtual     bool       requireMonoBlend() { return mMasterMono.load() && !hasFastMixer(); }

                void       setMasterBalance(float balance) override {
                               mMasterBalance.store(balance);
                               if (hasFastMixer()) {
                                   mFastMixer->setMasterBalance(balance);
                               }
                           }
};

class DirectOutputThread : public PlaybackThread {
public:

    DirectOutputThread(const sp<AudioFlinger>& audioFlinger, AudioStreamOut* output,
                       audio_io_handle_t id, audio_devices_t device, bool systemReady)
        : DirectOutputThread(audioFlinger, output, id, device, DIRECT, systemReady) { }

    virtual                 ~DirectOutputThread();

                status_t    selectPresentation(int presentationId, int programId);

    // Thread virtuals

    virtual     bool        checkForNewParameter_l(const String8& keyValuePair,
                                                   status_t& status);

    virtual     void        flushHw_l();

                void        setMasterBalance(float balance) override;

protected:
    virtual     uint32_t    activeSleepTimeUs() const;
    virtual     uint32_t    idleSleepTimeUs() const;
    virtual     uint32_t    suspendSleepTimeUs() const;
    virtual     void        cacheParameters_l();

                void        dumpInternals_l(int fd, const Vector<String16>& args) override;

    // threadLoop snippets
    virtual     mixer_state prepareTracks_l(Vector< sp<Track> > *tracksToRemove);
    virtual     void        threadLoop_mix();
    virtual     void        threadLoop_sleepTime();
    virtual     void        threadLoop_exit();
    virtual     bool        shouldStandby_l();

    virtual     void        onAddNewTrack_l();

    bool mVolumeShaperActive = false;

    DirectOutputThread(const sp<AudioFlinger>& audioFlinger, AudioStreamOut* output,
                        audio_io_handle_t id, audio_devices_t device, ThreadBase::type_t type,
                        bool systemReady);
    void processVolume_l(Track *track, bool lastTrack);

    // prepareTracks_l() tells threadLoop_mix() the name of the single active track
    sp<Track>               mActiveTrack;

    wp<Track>               mPreviousTrack;         // used to detect track switch

    // This must be initialized for initial condition of mMasterBalance = 0 (disabled).
    float                   mMasterBalanceLeft = 1.f;
    float                   mMasterBalanceRight = 1.f;

public:
    virtual     bool        hasFastMixer() const { return false; }

    virtual     int64_t     computeWaitTimeNs_l() const override;

    status_t    threadloop_getHalTimestamp_l(ExtendedTimestamp *timestamp) const override {
                    // For DIRECT and OFFLOAD threads, query the output sink directly.
                    if (mOutput != nullptr) {
                        uint64_t uposition64;
                        struct timespec time;
                        if (mOutput->getPresentationPosition(
                                &uposition64, &time) == OK) {
                            timestamp->mPosition[ExtendedTimestamp::LOCATION_KERNEL]
                                    = (int64_t)uposition64;
                            timestamp->mTimeNs[ExtendedTimestamp::LOCATION_KERNEL]
                                    = audio_utils_ns_from_timespec(&time);
                            return NO_ERROR;
                        }
                    }
                    return INVALID_OPERATION;
                }
};

class OffloadThread : public DirectOutputThread {
public:

    OffloadThread(const sp<AudioFlinger>& audioFlinger, AudioStreamOut* output,
                        audio_io_handle_t id, uint32_t device, bool systemReady);
    virtual                 ~OffloadThread() {};
    virtual     void        flushHw_l();

protected:
    // threadLoop snippets
    virtual     mixer_state prepareTracks_l(Vector< sp<Track> > *tracksToRemove);
    virtual     void        threadLoop_exit();

    virtual     bool        waitingAsyncCallback();
    virtual     bool        waitingAsyncCallback_l();
    virtual     void        invalidateTracks(audio_stream_type_t streamType);

    virtual     bool        keepWakeLock() const { return (mKeepWakeLock || (mDrainSequence & 1)); }

private:
    size_t      mPausedWriteLength;     // length in bytes of write interrupted by pause
    size_t      mPausedBytesRemaining;  // bytes still waiting in mixbuffer after resume
    bool        mKeepWakeLock;          // keep wake lock while waiting for write callback
    uint64_t    mOffloadUnderrunPosition; // Current frame position for offloaded playback
                                          // used and valid only during underrun.  ~0 if
                                          // no underrun has occurred during playback and
                                          // is not reset on standby.
};

class AsyncCallbackThread : public Thread {
public:

    explicit AsyncCallbackThread(const wp<PlaybackThread>& playbackThread);

    virtual             ~AsyncCallbackThread();

    // Thread virtuals
    virtual bool        threadLoop();

    // RefBase
    virtual void        onFirstRef();

            void        exit();
            void        setWriteBlocked(uint32_t sequence);
            void        resetWriteBlocked();
            void        setDraining(uint32_t sequence);
            void        resetDraining();
            void        setAsyncError();

private:
    const wp<PlaybackThread>   mPlaybackThread;
    // mWriteAckSequence corresponds to the last write sequence passed by the offload thread via
    // setWriteBlocked(). The sequence is shifted one bit to the left and the lsb is used
    // to indicate that the callback has been received via resetWriteBlocked()
    uint32_t                   mWriteAckSequence;
    // mDrainSequence corresponds to the last drain sequence passed by the offload thread via
    // setDraining(). The sequence is shifted one bit to the left and the lsb is used
    // to indicate that the callback has been received via resetDraining()
    uint32_t                   mDrainSequence;
    Condition                  mWaitWorkCV;
    Mutex                      mLock;
    bool                       mAsyncError;
};

class DuplicatingThread : public MixerThread {
public:
    DuplicatingThread(const sp<AudioFlinger>& audioFlinger, MixerThread* mainThread,
                      audio_io_handle_t id, bool systemReady);
    virtual                 ~DuplicatingThread();

    // Thread virtuals
                void        addOutputTrack(MixerThread* thread);
                void        removeOutputTrack(MixerThread* thread);
                uint32_t    waitTimeMs() const { return mWaitTimeMs; }

                void        sendMetadataToBackend_l(
                        const StreamOutHalInterface::SourceMetadata& metadata) override;
protected:
    virtual     uint32_t    activeSleepTimeUs() const;
                void        dumpInternals_l(int fd, const Vector<String16>& args) override;

private:
                bool        outputsReady(const SortedVector< sp<OutputTrack> > &outputTracks);
protected:
    // threadLoop snippets
    virtual     void        threadLoop_mix();
    virtual     void        threadLoop_sleepTime();
    virtual     ssize_t     threadLoop_write();
    virtual     void        threadLoop_standby();
    virtual     void        cacheParameters_l();

private:
    // called from threadLoop, addOutputTrack, removeOutputTrack
    virtual     void        updateWaitTime_l();
protected:
    virtual     void        saveOutputTracks();
    virtual     void        clearOutputTracks();
private:

                uint32_t    mWaitTimeMs;
    SortedVector < sp<OutputTrack> >  outputTracks;
    SortedVector < sp<OutputTrack> >  mOutputTracks;
public:
    virtual     bool        hasFastMixer() const { return false; }
                status_t    threadloop_getHalTimestamp_l(
                                    ExtendedTimestamp *timestamp) const override {
        if (mOutputTracks.size() > 0) {
            // forward the first OutputTrack's kernel information for timestamp.
            const ExtendedTimestamp trackTimestamp =
                    mOutputTracks[0]->getClientProxyTimestamp();
            if (trackTimestamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL] > 0) {
                timestamp->mTimeNs[ExtendedTimestamp::LOCATION_KERNEL] =
                        trackTimestamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL];
                timestamp->mPosition[ExtendedTimestamp::LOCATION_KERNEL] =
                        trackTimestamp.mPosition[ExtendedTimestamp::LOCATION_KERNEL];
                return OK;  // discard server timestamp - that's ignored.
            }
        }
        return INVALID_OPERATION;
    }
};

// record thread
class RecordThread : public ThreadBase
{
public:

    class RecordTrack;

    /* The ResamplerBufferProvider is used to retrieve recorded input data from the
     * RecordThread.  It maintains local state on the relative position of the read
     * position of the RecordTrack compared with the RecordThread.
     */
    class ResamplerBufferProvider : public AudioBufferProvider
    {
    public:
        explicit ResamplerBufferProvider(RecordTrack* recordTrack) :
            mRecordTrack(recordTrack),
            mRsmpInUnrel(0), mRsmpInFront(0) { }
        virtual ~ResamplerBufferProvider() { }

        // called to set the ResamplerBufferProvider to head of the RecordThread data buffer,
        // skipping any previous data read from the hal.
        virtual void reset();

        /* Synchronizes RecordTrack position with the RecordThread.
         * Calculates available frames and handle overruns if the RecordThread
         * has advanced faster than the ResamplerBufferProvider has retrieved data.
         * TODO: why not do this for every getNextBuffer?
         *
         * Parameters
         * framesAvailable:  pointer to optional output size_t to store record track
         *                   frames available.
         *      hasOverrun:  pointer to optional boolean, returns true if track has overrun.
         */

        virtual void sync(size_t *framesAvailable = NULL, bool *hasOverrun = NULL);

        // AudioBufferProvider interface
        virtual status_t    getNextBuffer(AudioBufferProvider::Buffer* buffer);
        virtual void        releaseBuffer(AudioBufferProvider::Buffer* buffer);
    private:
        RecordTrack * const mRecordTrack;
        size_t              mRsmpInUnrel;   // unreleased frames remaining from
                                            // most recent getNextBuffer
                                            // for debug only
        int32_t             mRsmpInFront;   // next available frame
                                            // rolling counter that is never cleared
    };

#include "RecordTracks.h"

            RecordThread(const sp<AudioFlinger>& audioFlinger,
                    AudioStreamIn *input,
                    audio_io_handle_t id,
                    audio_devices_t outDevice,
                    audio_devices_t inDevice,
                    bool systemReady
                    );
            virtual     ~RecordThread();

    // no addTrack_l ?
    void        destroyTrack_l(const sp<RecordTrack>& track);
    void        removeTrack_l(const sp<RecordTrack>& track);

    // Thread virtuals
    virtual bool        threadLoop();
    virtual void        preExit();

    // RefBase
    virtual void        onFirstRef();

    virtual status_t    initCheck() const { return (mInput == NULL) ? NO_INIT : NO_ERROR; }

    virtual sp<MemoryDealer>    readOnlyHeap() const { return mReadOnlyHeap; }

    virtual sp<IMemory> pipeMemory() const { return mPipeMemory; }

            sp<AudioFlinger::RecordThread::RecordTrack>  createRecordTrack_l(
                    const sp<AudioFlinger::Client>& client,
                    const audio_attributes_t& attr,
                    uint32_t *pSampleRate,
                    audio_format_t format,
                    audio_channel_mask_t channelMask,
                    size_t *pFrameCount,
                    audio_session_t sessionId,
                    size_t *pNotificationFrameCount,
                    pid_t creatorPid,
                    uid_t uid,
                    audio_input_flags_t *flags,
                    pid_t tid,
                    status_t *status /*non-NULL*/,
                    audio_port_handle_t portId,
                    const String16& opPackageName);

            status_t    start(RecordTrack* recordTrack,
                              AudioSystem::sync_event_t event,
                              audio_session_t triggerSession);

            // ask the thread to stop the specified track, and
            // return true if the caller should then do it's part of the stopping process
            bool        stop(RecordTrack* recordTrack);

            AudioStreamIn* clearInput();
            virtual sp<StreamHalInterface> stream() const;


    virtual bool        checkForNewParameter_l(const String8& keyValuePair,
                                               status_t& status);
    virtual void        cacheParameters_l() {}
    virtual String8     getParameters(const String8& keys);
    virtual void        ioConfigChanged(audio_io_config_event event, pid_t pid = 0,
                                        audio_port_handle_t portId = AUDIO_PORT_HANDLE_NONE);
    virtual status_t    createAudioPatch_l(const struct audio_patch *patch,
                                           audio_patch_handle_t *handle);
    virtual status_t    releaseAudioPatch_l(const audio_patch_handle_t handle);

            void        addPatchTrack(const sp<PatchRecord>& record);
            void        deletePatchTrack(const sp<PatchRecord>& record);

            void        readInputParameters_l();
    virtual uint32_t    getInputFramesLost();

    virtual status_t addEffectChain_l(const sp<EffectChain>& chain);
    virtual size_t removeEffectChain_l(const sp<EffectChain>& chain);
            uint32_t hasAudioSession_l(audio_session_t sessionId) const override {
                         return ThreadBase::hasAudioSession_l(sessionId, mTracks);
                     }

            // Return the set of unique session IDs across all tracks.
            // The keys are the session IDs, and the associated values are meaningless.
            // FIXME replace by Set [and implement Bag/Multiset for other uses].
            KeyedVector<audio_session_t, bool> sessionIds() const;

    virtual status_t setSyncEvent(const sp<SyncEvent>& event);
    virtual bool     isValidSyncEvent(const sp<SyncEvent>& event) const;

    static void syncStartEventCallback(const wp<SyncEvent>& event);

    virtual size_t      frameCount() const { return mFrameCount; }
            bool        hasFastCapture() const { return mFastCapture != 0; }
    virtual void        toAudioPortConfig(struct audio_port_config *config);

    virtual status_t    checkEffectCompatibility_l(const effect_descriptor_t *desc,
                                                   audio_session_t sessionId);

    virtual void        acquireWakeLock_l() {
                            ThreadBase::acquireWakeLock_l();
                            mActiveTracks.updatePowerState(this, true /* force */);
                        }
    virtual bool        isOutput() const override { return false; }

            void        checkBtNrec();

            // Sets the UID records silence
            void        setRecordSilenced(uid_t uid, bool silenced);

            status_t    getActiveMicrophones(std::vector<media::MicrophoneInfo>* activeMicrophones);

            status_t    setPreferredMicrophoneDirection(audio_microphone_direction_t direction);
            status_t    setPreferredMicrophoneFieldDimension(float zoom);

            void        updateMetadata_l() override;

            bool        fastTrackAvailable() const { return mFastTrackAvail; }

            bool        isTimestampCorrectionEnabled() const override {
                            // checks popcount for exactly one device.
                            return audio_is_input_device(
                                    mInDevice & mTimestampCorrectedDevices);
                        }

protected:
            void        dumpInternals_l(int fd, const Vector<String16>& args) override;
            void        dumpTracks_l(int fd, const Vector<String16>& args) override;

private:
            // Enter standby if not already in standby, and set mStandby flag
            void    standbyIfNotAlreadyInStandby();

            // Call the HAL standby method unconditionally, and don't change mStandby flag
            void    inputStandBy();

            void    checkBtNrec_l();

            AudioStreamIn                       *mInput;
            SortedVector < sp<RecordTrack> >    mTracks;
            // mActiveTracks has dual roles:  it indicates the current active track(s), and
            // is used together with mStartStopCond to indicate start()/stop() progress
            ActiveTracks<RecordTrack>           mActiveTracks;

            Condition                           mStartStopCond;

            // resampler converts input at HAL Hz to output at AudioRecord client Hz
            void                               *mRsmpInBuffer;  // size = mRsmpInFramesOA
            size_t                              mRsmpInFrames;  // size of resampler input in frames
            size_t                              mRsmpInFramesP2;// size rounded up to a power-of-2
            size_t                              mRsmpInFramesOA;// mRsmpInFramesP2 + over-allocation

            // rolling index that is never cleared
            int32_t                             mRsmpInRear;    // last filled frame + 1

            // For dumpsys
            const sp<MemoryDealer>              mReadOnlyHeap;

            // one-time initialization, no locks required
            sp<FastCapture>                     mFastCapture;   // non-0 if there is also
                                                                // a fast capture

            // FIXME audio watchdog thread

            // contents are not guaranteed to be consistent, no locks required
            FastCaptureDumpState                mFastCaptureDumpState;
#ifdef STATE_QUEUE_DUMP
            // FIXME StateQueue observer and mutator dump fields
#endif
            // FIXME audio watchdog dump

            // accessible only within the threadLoop(), no locks required
            //          mFastCapture->sq()      // for mutating and pushing state
            int32_t     mFastCaptureFutex;      // for cold idle

            // The HAL input source is treated as non-blocking,
            // but current implementation is blocking
            sp<NBAIO_Source>                    mInputSource;
            // The source for the normal capture thread to read from: mInputSource or mPipeSource
            sp<NBAIO_Source>                    mNormalSource;
            // If a fast capture is present, the non-blocking pipe sink written to by fast capture,
            // otherwise clear
            sp<NBAIO_Sink>                      mPipeSink;
            // If a fast capture is present, the non-blocking pipe source read by normal thread,
            // otherwise clear
            sp<NBAIO_Source>                    mPipeSource;
            // Depth of pipe from fast capture to normal thread and fast clients, always power of 2
            size_t                              mPipeFramesP2;
            // If a fast capture is present, the Pipe as IMemory, otherwise clear
            sp<IMemory>                         mPipeMemory;

            // TODO: add comment and adjust size as needed
            static const size_t                 kFastCaptureLogSize = 4 * 1024;
            sp<NBLog::Writer>                   mFastCaptureNBLogWriter;

            bool                                mFastTrackAvail;    // true if fast track available
            // common state to all record threads
            std::atomic_bool                    mBtNrecSuspended;

            int64_t                             mFramesRead = 0;    // continuous running counter.
};

class MmapThread : public ThreadBase
{
 public:

#include "MmapTracks.h"

    MmapThread(const sp<AudioFlinger>& audioFlinger, audio_io_handle_t id,
                      AudioHwDevice *hwDev, sp<StreamHalInterface> stream,
                      audio_devices_t outDevice, audio_devices_t inDevice, bool systemReady);
    virtual     ~MmapThread();

    virtual     void        configure(const audio_attributes_t *attr,
                                      audio_stream_type_t streamType,
                                      audio_session_t sessionId,
                                      const sp<MmapStreamCallback>& callback,
                                      audio_port_handle_t deviceId,
                                      audio_port_handle_t portId);

                void        disconnect();

    // MmapStreamInterface
    status_t createMmapBuffer(int32_t minSizeFrames,
                                      struct audio_mmap_buffer_info *info);
    status_t getMmapPosition(struct audio_mmap_position *position);
    status_t start(const AudioClient& client, audio_port_handle_t *handle);
    status_t stop(audio_port_handle_t handle);
    status_t standby();

    // RefBase
    virtual     void        onFirstRef();

    // Thread virtuals
    virtual     bool        threadLoop();

    virtual     void        threadLoop_exit();
    virtual     void        threadLoop_standby();
    virtual     bool        shouldStandby_l() { return false; }
    virtual     status_t    exitStandby();

    virtual     status_t    initCheck() const { return (mHalStream == 0) ? NO_INIT : NO_ERROR; }
    virtual     size_t      frameCount() const { return mFrameCount; }
    virtual     bool        checkForNewParameter_l(const String8& keyValuePair,
                                                    status_t& status);
    virtual     String8     getParameters(const String8& keys);
    virtual     void        ioConfigChanged(audio_io_config_event event, pid_t pid = 0,
                                            audio_port_handle_t portId = AUDIO_PORT_HANDLE_NONE);
                void        readHalParameters_l();
    virtual     void        cacheParameters_l() {}
    virtual     status_t    createAudioPatch_l(const struct audio_patch *patch,
                                               audio_patch_handle_t *handle);
    virtual     status_t    releaseAudioPatch_l(const audio_patch_handle_t handle);
    virtual     void        toAudioPortConfig(struct audio_port_config *config);

    virtual     sp<StreamHalInterface> stream() const { return mHalStream; }
    virtual     status_t    addEffectChain_l(const sp<EffectChain>& chain);
    virtual     size_t      removeEffectChain_l(const sp<EffectChain>& chain);
    virtual     status_t    checkEffectCompatibility_l(const effect_descriptor_t *desc,
                                                               audio_session_t sessionId);

                uint32_t    hasAudioSession_l(audio_session_t sessionId) const override {
                                // Note: using mActiveTracks as no mTracks here.
                                return ThreadBase::hasAudioSession_l(sessionId, mActiveTracks);
                            }
    virtual     status_t    setSyncEvent(const sp<SyncEvent>& event);
    virtual     bool        isValidSyncEvent(const sp<SyncEvent>& event) const;

    virtual     void        checkSilentMode_l() {}
    virtual     void        processVolume_l() {}
                void        checkInvalidTracks_l();

    virtual     audio_stream_type_t streamType() { return AUDIO_STREAM_DEFAULT; }

    virtual     void        invalidateTracks(audio_stream_type_t streamType __unused) {}

                // Sets the UID records silence
    virtual     void        setRecordSilenced(uid_t uid __unused, bool silenced __unused) {}

 protected:
                void        dumpInternals_l(int fd, const Vector<String16>& args) override;
                void        dumpTracks_l(int fd, const Vector<String16>& args) override;

                audio_attributes_t      mAttr;
                audio_session_t         mSessionId;
                audio_port_handle_t     mPortId;

                wp<MmapStreamCallback>  mCallback;
                sp<StreamHalInterface>  mHalStream;
                sp<DeviceHalInterface>  mHalDevice;
                AudioHwDevice* const    mAudioHwDev;
                ActiveTracks<MmapTrack> mActiveTracks;
                float                   mHalVolFloat;

                int32_t                 mNoCallbackWarningCount;
     static     constexpr int32_t       kMaxNoCallbackWarnings = 5;
};

class MmapPlaybackThread : public MmapThread, public VolumeInterface
{

public:
    MmapPlaybackThread(const sp<AudioFlinger>& audioFlinger, audio_io_handle_t id,
                      AudioHwDevice *hwDev, AudioStreamOut *output,
                      audio_devices_t outDevice, audio_devices_t inDevice, bool systemReady);
    virtual     ~MmapPlaybackThread() {}

    virtual     void        configure(const audio_attributes_t *attr,
                                      audio_stream_type_t streamType,
                                      audio_session_t sessionId,
                                      const sp<MmapStreamCallback>& callback,
                                      audio_port_handle_t deviceId,
                                      audio_port_handle_t portId);

                AudioStreamOut* clearOutput();

                // VolumeInterface
    virtual     void        setMasterVolume(float value);
    virtual     void        setMasterMute(bool muted);
    virtual     void        setStreamVolume(audio_stream_type_t stream, float value);
    virtual     void        setStreamMute(audio_stream_type_t stream, bool muted);
    virtual     float       streamVolume(audio_stream_type_t stream) const;

                void        setMasterMute_l(bool muted) { mMasterMute = muted; }

    virtual     void        invalidateTracks(audio_stream_type_t streamType);

    virtual     audio_stream_type_t streamType() { return mStreamType; }
    virtual     void        checkSilentMode_l();
                void        processVolume_l() override;

    virtual     bool        isOutput() const override { return true; }

                void        updateMetadata_l() override;

    virtual     void        toAudioPortConfig(struct audio_port_config *config);

protected:
                void        dumpInternals_l(int fd, const Vector<String16>& args) override;

                audio_stream_type_t         mStreamType;
                float                       mMasterVolume;
                float                       mStreamVolume;
                bool                        mMasterMute;
                bool                        mStreamMute;
                AudioStreamOut*             mOutput;
};

class MmapCaptureThread : public MmapThread
{

public:
    MmapCaptureThread(const sp<AudioFlinger>& audioFlinger, audio_io_handle_t id,
                      AudioHwDevice *hwDev, AudioStreamIn *input,
                      audio_devices_t outDevice, audio_devices_t inDevice, bool systemReady);
    virtual     ~MmapCaptureThread() {}

                AudioStreamIn* clearInput();

                status_t       exitStandby() override;
    virtual     bool           isOutput() const override { return false; }

                void           updateMetadata_l() override;
                void           processVolume_l() override;
                void           setRecordSilenced(uid_t uid, bool silenced) override;

    virtual     void           toAudioPortConfig(struct audio_port_config *config);

protected:

                AudioStreamIn*  mInput;
};
