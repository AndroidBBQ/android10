/*
**
** Copyright 2014, The Android Open Source Project
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


// PatchPanel is concealed within AudioFlinger, their lifetimes are the same.
class PatchPanel {
public:
    class SoftwarePatch {
      public:
        SoftwarePatch(const PatchPanel &patchPanel, audio_patch_handle_t patchHandle,
                audio_io_handle_t playbackThreadHandle, audio_io_handle_t recordThreadHandle)
                : mPatchPanel(patchPanel), mPatchHandle(patchHandle),
                  mPlaybackThreadHandle(playbackThreadHandle),
                  mRecordThreadHandle(recordThreadHandle) {}
        SoftwarePatch(const SoftwarePatch&) = default;
        SoftwarePatch& operator=(const SoftwarePatch&) = default;

        // Must be called under AudioFlinger::mLock
        status_t getLatencyMs_l(double *latencyMs) const;
        audio_patch_handle_t getPatchHandle() const { return mPatchHandle; };
        audio_io_handle_t getPlaybackThreadHandle() const { return mPlaybackThreadHandle; };
        audio_io_handle_t getRecordThreadHandle() const { return mRecordThreadHandle; };
      private:
        const PatchPanel &mPatchPanel;
        const audio_patch_handle_t mPatchHandle;
        const audio_io_handle_t mPlaybackThreadHandle;
        const audio_io_handle_t mRecordThreadHandle;
    };

    explicit PatchPanel(AudioFlinger* audioFlinger) : mAudioFlinger(*audioFlinger) {}

    /* List connected audio ports and their attributes */
    status_t listAudioPorts(unsigned int *num_ports,
                                    struct audio_port *ports);

    /* Get supported attributes for a given audio port */
    status_t getAudioPort(struct audio_port *port);

    /* Create a patch between several source and sink ports */
    status_t createAudioPatch(const struct audio_patch *patch,
                                       audio_patch_handle_t *handle);

    /* Release a patch */
    status_t releaseAudioPatch(audio_patch_handle_t handle);

    /* List connected audio devices and they attributes */
    status_t listAudioPatches(unsigned int *num_patches,
                                      struct audio_patch *patches);

    // Retrieves all currently estrablished software patches for a stream
    // opened on an intermediate module.
    status_t getDownstreamSoftwarePatches(audio_io_handle_t stream,
            std::vector<SoftwarePatch> *patches) const;

    // Notifies patch panel about all opened and closed streams.
    void notifyStreamOpened(AudioHwDevice *audioHwDevice, audio_io_handle_t stream);
    void notifyStreamClosed(audio_io_handle_t stream);

    void dump(int fd) const;

private:
    template<typename ThreadType, typename TrackType>
    class Endpoint {
    public:
        Endpoint() = default;
        Endpoint(const Endpoint&) = delete;
        Endpoint& operator=(const Endpoint&) = delete;
        Endpoint(Endpoint&& other) noexcept { swap(other); }
        Endpoint& operator=(Endpoint&& other) noexcept {
            swap(other);
            return *this;
        }
        ~Endpoint() {
            ALOGE_IF(mHandle != AUDIO_PATCH_HANDLE_NONE,
                    "A non empty Patch Endpoint leaked, handle %d", mHandle);
        }

        status_t checkTrack(TrackType *trackOrNull) const {
            if (trackOrNull == nullptr) return NO_MEMORY;
            return trackOrNull->initCheck();
        }
        audio_patch_handle_t handle() const { return mHandle; }
        sp<ThreadType> thread() { return mThread; }
        sp<TrackType> track() { return mTrack; }
        sp<const ThreadType> const_thread() const { return mThread; }
        sp<const TrackType> const_track() const { return mTrack; }

        void closeConnections(PatchPanel *panel) {
            if (mHandle != AUDIO_PATCH_HANDLE_NONE) {
                panel->releaseAudioPatch(mHandle);
                mHandle = AUDIO_PATCH_HANDLE_NONE;
            }
            if (mThread != 0) {
                if (mTrack != 0) {
                    mThread->deletePatchTrack(mTrack);
                }
                if (mCloseThread) {
                    panel->mAudioFlinger.closeThreadInternal_l(mThread);
                }
            }
        }
        audio_patch_handle_t* handlePtr() { return &mHandle; }
        void setThread(const sp<ThreadType>& thread, bool closeThread = true) {
            mThread = thread;
            mCloseThread = closeThread;
        }
        template <typename T>
        void setTrackAndPeer(const sp<TrackType>& track, const sp<T> &peer) {
            mTrack = track;
            mThread->addPatchTrack(mTrack);
            mTrack->setPeerProxy(peer, true /* holdReference */);
        }
        void clearTrackPeer() { if (mTrack) mTrack->clearPeerProxy(); }
        void stopTrack() { if (mTrack) mTrack->stop(); }

        void swap(Endpoint &other) noexcept {
            using std::swap;
            swap(mThread, other.mThread);
            swap(mCloseThread, other.mCloseThread);
            swap(mHandle, other.mHandle);
            swap(mTrack, other.mTrack);
        }

        friend void swap(Endpoint &a, Endpoint &b) noexcept {
            a.swap(b);
        }

    private:
        sp<ThreadType> mThread;
        bool mCloseThread = true;
        audio_patch_handle_t mHandle = AUDIO_PATCH_HANDLE_NONE;
        sp<TrackType> mTrack;
    };

    class Patch {
    public:
        explicit Patch(const struct audio_patch &patch) : mAudioPatch(patch) {}
        ~Patch();
        Patch(const Patch&) = delete;
        Patch(Patch&&) = default;
        Patch& operator=(const Patch&) = delete;
        Patch& operator=(Patch&&) = default;

        status_t createConnections(PatchPanel *panel);
        void clearConnections(PatchPanel *panel);
        bool isSoftware() const {
            return mRecord.handle() != AUDIO_PATCH_HANDLE_NONE ||
                    mPlayback.handle() != AUDIO_PATCH_HANDLE_NONE; }

        // returns the latency of the patch (from record to playback).
        status_t getLatencyMs(double *latencyMs) const;

        String8 dump(audio_patch_handle_t myHandle) const;

        // Note that audio_patch::id is only unique within a HAL module
        struct audio_patch              mAudioPatch;
        // handle for audio HAL patch handle present only when the audio HAL version is >= 3.0
        audio_patch_handle_t            mHalHandle = AUDIO_PATCH_HANDLE_NONE;
        // below members are used by a software audio patch connecting a source device from a
        // given audio HW module to a sink device on an other audio HW module.
        // the objects are created by createConnections() and released by clearConnections()
        // playback thread is created if no existing playback thread can be used
        // connects playback thread output to sink device
        Endpoint<PlaybackThread, PlaybackThread::PatchTrack> mPlayback;
        // connects source device to record thread input
        Endpoint<RecordThread, RecordThread::PatchRecord> mRecord;
    };

    AudioHwDevice* findAudioHwDeviceByModule(audio_module_handle_t module);
    sp<DeviceHalInterface> findHwDeviceByModule(audio_module_handle_t module);
    void addSoftwarePatchToInsertedModules(
            audio_module_handle_t module, audio_patch_handle_t handle);
    void removeSoftwarePatchFromInsertedModules(audio_patch_handle_t handle);

    AudioFlinger &mAudioFlinger;
    std::map<audio_patch_handle_t, Patch> mPatches;

    // This map allows going from a thread to "downstream" software patches
    // when a processing module inserted in between. Example:
    //
    //  from map value.streams                               map key
    //  [Mixer thread] --> [Virtual output device] --> [Processing module] ---\
    //       [Harware module] <-- [Physical output device] <-- [S/W Patch] <--/
    //                                                 from map value.sw_patches
    //
    // This allows the mixer thread to look up the threads of the software patch
    // for propagating timing info, parameters, etc.
    //
    // The current assumptions are:
    //   1) The processing module acts as a mixer with several outputs which
    //      represent differently downmixed and / or encoded versions of the same
    //      mixed stream. There is no 1:1 correspondence between the input streams
    //      and the software patches, but rather a N:N correspondence between
    //      a group of streams and a group of patches.
    //   2) There are only a couple of inserted processing modules in the system,
    //      so when looking for a stream or patch handle we can iterate over
    //      all modules.
    struct ModuleConnections {
        std::set<audio_io_handle_t> streams;
        std::set<audio_patch_handle_t> sw_patches;
    };
    std::map<audio_module_handle_t, ModuleConnections> mInsertedModules;
};
