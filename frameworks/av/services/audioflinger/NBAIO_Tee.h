/*
 * Copyright (C) 2018 The Android Open Source Project
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

// Enabled with TEE_SINK in Configuration.h
#ifndef ANDROID_NBAIO_TEE_H
#define ANDROID_NBAIO_TEE_H

#ifdef TEE_SINK

#include <atomic>
#include <mutex>
#include <set>

#include <cutils/properties.h>
#include <media/nbaio/NBAIO.h>

namespace android {

/**
 * The NBAIO_Tee uses the NBAIO Pipe and PipeReader for nonblocking
 * data collection, for eventual dump to log files.
 * See https://source.android.com/devices/audio/debugging for how to
 * enable by ro.debuggable and af.tee properties.
 *
 * The write() into the NBAIO_Tee is therefore nonblocking,
 * but changing NBAIO_Tee formats with set() cannot be done during a write();
 * usually the caller already implements this mutual exclusion.
 *
 * All other calls except set() vs write() may occur at any time.
 *
 * dump() disruption is minimized to the caller since system calls are executed
 * in an asynchronous thread (when possible).
 *
 * Currently the NBAIO_Tee is "hardwired" for AudioFlinger support.
 *
 * Some AudioFlinger specific notes:
 *
 * 1) Tees capture only linear PCM data.
 * 2) Tees without any data written are considered empty and do not generate
 *    any output files.
 * 2) Once a Tee dumps data, it is considered "emptied" and new data
 *    needs to be written before another Tee file is generated.
 * 3) Tee file format is
 *    WAV integer PCM 16 bit for AUDIO_FORMAT_PCM_8_BIT, AUDIO_FORMAT_PCM_16_BIT.
 *    WAV integer PCM 32 bit for AUDIO_FORMAT_PCM_8_24_BIT, AUDIO_FORMAT_PCM_24_BIT_PACKED
 *                               AUDIO_FORMAT_PCM_32_BIT.
 *    WAV float PCM 32 bit for AUDIO_FORMAT_PCM_FLOAT.
 *
 * Input_Thread:
 * 1) Capture buffer is teed when read from the HAL, before resampling for the AudioRecord
 *    client.
 *
 * Output_Thread:
 * 1) MixerThreads will tee at the FastMixer output (if it has one) or at the
 *    NormalMixer output (if no FastMixer).
 * 2) DuplicatingThreads do not tee any mixed data. Apply a tee on the downstream OutputTrack
 *    or on the upstream playback Tracks.
 * 3) DirectThreads and OffloadThreads do not tee any data. The upstream track
 *    (if linear PCM format) may be teed to discover data.
 * 4) MmapThreads are not supported.
 *
 * Tracks:
 * 1) RecordTracks and playback Tracks tee as data is being written to or
 *    read from the shared client-server track buffer by the associated Threads.
 * 2) The mechanism is on the AudioBufferProvider release() so large static Track
 *    playback may not show any Tee data depending on when it is released.
 * 3) When a track becomes inactive, the Thread will trigger a dump.
 */

class NBAIO_Tee {
public:
    /* TEE_FLAG is used in set() and must match the flags for the af.tee property
       given in https://source.android.com/devices/audio/debugging
    */
    enum TEE_FLAG {
        TEE_FLAG_NONE = 0,
        TEE_FLAG_INPUT_THREAD = (1 << 0),  // treat as a Tee for input (Capture) Threads
        TEE_FLAG_OUTPUT_THREAD = (1 << 1), // treat as a Tee for output (Playback) Threads
        TEE_FLAG_TRACK = (1 << 2),         // treat as a Tee for tracks (Record and Playback)
    };

    NBAIO_Tee()
        : mTee(std::make_shared<NBAIO_TeeImpl>())
    {
        getRunningTees().add(mTee);
    }

    ~NBAIO_Tee() {
        getRunningTees().remove(mTee);
        dump(-1, "_DTOR"); // log any data remaining in Tee.
    }

    /**
     * \brief set is used for deferred configuration of Tee.
     *
     *  May be called anytime except concurrently with write().
     *
     * \param format NBAIO_Format used to open NBAIO pipes
     * \param flags (https://source.android.com/devices/audio/debugging)
     *              - TEE_FLAG_NONE to bypass af.tee property checks (default);
     *              - TEE_FLAG_INPUT_THREAD to check af.tee if input thread logging set;
     *              - TEE_FLAG_OUTPUT_THREAD to check af.tee if output thread logging set;
     *              - TEE_FLAG_TRACK to check af.tee if track logging set.
     * \param frames number of frames to open the NBAIO pipe (set to 0 to use default).
     *
     * \return
     *         - NO_ERROR on success (or format unchanged)
     *         - BAD_VALUE if format or flags invalid.
     *         - PERMISSION_DENIED if flags not allowed by af.tee
     */

    status_t set(const NBAIO_Format &format,
            TEE_FLAG flags = TEE_FLAG_NONE, size_t frames = 0) const {
        return mTee->set(format, flags, frames);
    }

    status_t set(uint32_t sampleRate, uint32_t channelCount, audio_format_t format,
            TEE_FLAG flags = TEE_FLAG_NONE, size_t frames = 0) const {
        return mTee->set(Format_from_SR_C(sampleRate, channelCount, format), flags, frames);
    }

    /**
     * \brief write data to the tee.
     *
     * This call is lock free (as shared pointer and NBAIO is lock free);
     * may be called simultaneous to all methods except set().
     *
     * \param buffer to write to pipe.
     * \param frameCount in frames as specified by the format passed to set()
     */

    void write(const void *buffer, size_t frameCount) const {
        mTee->write(buffer, frameCount);
    }

    /** sets Tee id string which identifies the generated file (should be unique). */
    void setId(const std::string &id) const {
        mTee->setId(id);
    }

    /**
     * \brief dump the audio content written to the Tee.
     *
     * \param fd file descriptor to write dumped filename for logging, use -1 to ignore.
     * \param reason string suffix to append to the generated file.
     */
    void dump(int fd, const std::string &reason = "") const {
        mTee->dump(fd, reason);
    }

    /**
     * \brief dump all Tees currently alive.
     *
     * \param fd file descriptor to write dumped filename for logging, use -1 to ignore.
     * \param reason string suffix to append to the generated file.
     */
    static void dumpAll(int fd, const std::string &reason = "") {
        getRunningTees().dump(fd, reason);
    }

private:

    /** The underlying implementation of the Tee - the lifetime is through
        a shared pointer so destruction of the NBAIO_Tee container may proceed
        even though dumping is occurring. */
    class NBAIO_TeeImpl {
    public:
        status_t set(const NBAIO_Format &format, TEE_FLAG flags, size_t frames) {
            static const int teeConfig = property_get_bool("ro.debuggable", false)
                   ? property_get_int32("af.tee", 0) : 0;

            // check the type of Tee
            const TEE_FLAG type = TEE_FLAG(
                    flags & (TEE_FLAG_INPUT_THREAD | TEE_FLAG_OUTPUT_THREAD | TEE_FLAG_TRACK));

            // parameter flags can't select multiple types.
            if (__builtin_popcount(type) > 1) {
                return BAD_VALUE;
            }

            // if type is set, we check to see if it is permitted by configuration.
            if (type != 0 && (type & teeConfig) == 0) {
                return PERMISSION_DENIED;
            }

            // determine number of frames for Tee
            if (frames == 0) {
                // TODO: consider varying frame count based on type.
                frames = DEFAULT_TEE_FRAMES;
            }

            // TODO: should we check minimum number of frames?

            // don't do anything if format and frames are the same.
            if (Format_isEqual(format, mFormat) && frames == mFrames) {
                return NO_ERROR;
            }

            bool enabled = false;
            auto sinksource = makeSinkSource(format, frames, &enabled);

            // enabled is set if makeSinkSource is successful.
            // Note: as mentioned in NBAIO_Tee::set(), don't call set() while write() is
            // ongoing.
            if (enabled) {
                std::lock_guard<std::mutex> _l(mLock);
                mFlags = flags;
                mFormat = format; // could get this from the Sink.
                mFrames = frames;
                mSinkSource = std::move(sinksource);
                mEnabled.store(true);
                return NO_ERROR;
            }
            return BAD_VALUE;
        }

        void setId(const std::string &id) {
            std::lock_guard<std::mutex> _l(mLock);
            mId = id;
        }

        void dump(int fd, const std::string &reason) {
            if (!mDataReady.exchange(false)) return;
            std::string suffix;
            NBAIO_SinkSource sinkSource;
            {
                std::lock_guard<std::mutex> _l(mLock);
                suffix = mId + reason;
                sinkSource = mSinkSource;
            }
            dumpTee(fd, sinkSource, suffix);
        }

        void write(const void *buffer, size_t frameCount) {
            if (!mEnabled.load() || frameCount == 0) return;
            (void)mSinkSource.first->write(buffer, frameCount);
            mDataReady.store(true);
        }

    private:
        // TRICKY: We need to keep the NBAIO_Sink and NBAIO_Source both alive at the same time
        // because PipeReader holds a naked reference (not a strong or weak pointer) to Pipe.
        using NBAIO_SinkSource = std::pair<sp<NBAIO_Sink>, sp<NBAIO_Source>>;

        static void dumpTee(int fd, const NBAIO_SinkSource& sinkSource, const std::string& suffix);

        static NBAIO_SinkSource makeSinkSource(
                const NBAIO_Format &format, size_t frames, bool *enabled);

        // 0x200000 stereo 16-bit PCM frames = 47.5 seconds at 44.1 kHz, 8 megabytes
        static constexpr size_t DEFAULT_TEE_FRAMES = 0x200000;

        // atomic status checking
        std::atomic<bool> mEnabled{false};
        std::atomic<bool> mDataReady{false};

        // locked dump information
        mutable std::mutex mLock;
        std::string mId;                                         // GUARDED_BY(mLock)
        TEE_FLAG mFlags = TEE_FLAG_NONE;                         // GUARDED_BY(mLock)
        NBAIO_Format mFormat = Format_Invalid;                   // GUARDED_BY(mLock)
        size_t mFrames = 0;                                      // GUARDED_BY(mLock)
        NBAIO_SinkSource mSinkSource;                            // GUARDED_BY(mLock)
    };

    /** RunningTees tracks current running tees for dump purposes.
        It is implemented to have minimal locked regions, to be transparent to the caller. */
    class RunningTees {
    public:
        void add(const std::shared_ptr<NBAIO_TeeImpl> &tee) {
            std::lock_guard<std::mutex> _l(mLock);
            ALOGW_IF(!mTees.emplace(tee).second,
                    "%s: %p already exists in mTees", __func__, tee.get());
        }

        void remove(const std::shared_ptr<NBAIO_TeeImpl> &tee) {
            std::lock_guard<std::mutex> _l(mLock);
            ALOGW_IF(mTees.erase(tee) != 1,
                    "%s: %p doesn't exist in mTees", __func__, tee.get());
        }

        void dump(int fd, const std::string &reason) {
            std::vector<std::shared_ptr<NBAIO_TeeImpl>> tees; // safe snapshot of tees
            {
                std::lock_guard<std::mutex> _l(mLock);
                tees.insert(tees.end(), mTees.begin(), mTees.end());
            }
            for (const auto &tee : tees) {
                tee->dump(fd, reason);
            }
        }

    private:
        std::mutex mLock;
        std::set<std::shared_ptr<NBAIO_TeeImpl>> mTees; // GUARDED_BY(mLock)
    };

    // singleton
    static RunningTees &getRunningTees() {
        static RunningTees runningTees;
        return runningTees;
    }

    // The NBAIO TeeImpl may have lifetime longer than NBAIO_Tee if
    // RunningTees::dump() is being called simultaneous to ~NBAIO_Tee().
    // This is allowed for maximum concurrency.
    const std::shared_ptr<NBAIO_TeeImpl> mTee;
}; // NBAIO_Tee

} // namespace android

#endif // TEE_SINK
#endif // !ANDROID_NBAIO_TEE_H
