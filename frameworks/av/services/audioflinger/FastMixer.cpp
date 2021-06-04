/*
 * Copyright (C) 2012 The Android Open Source Project
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

// <IMPORTANT_WARNING>
// Design rules for threadLoop() are given in the comments at section "Fast mixer thread" of
// StateQueue.h.  In particular, avoid library and system calls except at well-known points.
// The design rules are only for threadLoop(), and don't apply to FastMixerDumpState methods.
// </IMPORTANT_WARNING>

#define LOG_TAG "FastMixer"
//#define LOG_NDEBUG 0

#define ATRACE_TAG ATRACE_TAG_AUDIO

#include "Configuration.h"
#include <time.h>
#include <utils/Debug.h>
#include <utils/Log.h>
#include <utils/Trace.h>
#include <system/audio.h>
#ifdef FAST_THREAD_STATISTICS
#include <audio_utils/Statistics.h>
#ifdef CPU_FREQUENCY_STATISTICS
#include <cpustats/ThreadCpuUsage.h>
#endif
#endif
#include <audio_utils/channels.h>
#include <audio_utils/format.h>
#include <audio_utils/mono_blend.h>
#include <media/AudioMixer.h>
#include "FastMixer.h"
#include "TypedLogger.h"

namespace android {

/*static*/ const FastMixerState FastMixer::sInitial;

FastMixer::FastMixer(audio_io_handle_t parentIoHandle)
    : FastThread("cycle_ms", "load_us"),
    // mFastTrackNames
    // mGenerations
    mOutputSink(NULL),
    mOutputSinkGen(0),
    mMixer(NULL),
    mSinkBuffer(NULL),
    mSinkBufferSize(0),
    mSinkChannelCount(FCC_2),
    mMixerBuffer(NULL),
    mMixerBufferSize(0),
    mMixerBufferState(UNDEFINED),
    mFormat(Format_Invalid),
    mSampleRate(0),
    mFastTracksGen(0),
    mTotalNativeFramesWritten(0),
    // timestamp
    mNativeFramesWrittenButNotPresented(0),   // the = 0 is to silence the compiler
    mMasterMono(false),
    mThreadIoHandle(parentIoHandle)
{
    (void)mThreadIoHandle; // prevent unused warning, see C++17 [[maybe_unused]]

    // FIXME pass sInitial as parameter to base class constructor, and make it static local
    mPrevious = &sInitial;
    mCurrent = &sInitial;

    mDummyDumpState = &mDummyFastMixerDumpState;
    // TODO: Add channel mask to NBAIO_Format.
    // We assume that the channel mask must be a valid positional channel mask.
    mSinkChannelMask = audio_channel_out_mask_from_count(mSinkChannelCount);

    unsigned i;
    for (i = 0; i < FastMixerState::sMaxFastTracks; ++i) {
        mGenerations[i] = 0;
    }
#ifdef FAST_THREAD_STATISTICS
    mOldLoad.tv_sec = 0;
    mOldLoad.tv_nsec = 0;
#endif
}

FastMixer::~FastMixer()
{
}

FastMixerStateQueue* FastMixer::sq()
{
    return &mSQ;
}

const FastThreadState *FastMixer::poll()
{
    return mSQ.poll();
}

void FastMixer::setNBLogWriter(NBLog::Writer *logWriter)
{
    // FIXME If mMixer is set or changed prior to this, we don't inform correctly.
    //       Should cache logWriter and re-apply it at the assignment to mMixer.
    if (mMixer != NULL) {
        mMixer->setNBLogWriter(logWriter);
    }
}

void FastMixer::onIdle()
{
    mPreIdle = *(const FastMixerState *)mCurrent;
    mCurrent = &mPreIdle;
}

void FastMixer::onExit()
{
    delete mMixer;
    free(mMixerBuffer);
    free(mSinkBuffer);
}

bool FastMixer::isSubClassCommand(FastThreadState::Command command)
{
    switch ((FastMixerState::Command) command) {
    case FastMixerState::MIX:
    case FastMixerState::WRITE:
    case FastMixerState::MIX_WRITE:
        return true;
    default:
        return false;
    }
}

void FastMixer::updateMixerTrack(int index, Reason reason) {
    const FastMixerState * const current = (const FastMixerState *) mCurrent;
    const FastTrack * const fastTrack = &current->mFastTracks[index];

    // check and update generation
    if (reason == REASON_MODIFY && mGenerations[index] == fastTrack->mGeneration) {
        return; // no change on an already configured track.
    }
    mGenerations[index] = fastTrack->mGeneration;

    // mMixer == nullptr on configuration failure (check done after generation update).
    if (mMixer == nullptr) {
        return;
    }

    switch (reason) {
    case REASON_REMOVE:
        mMixer->destroy(index);
        break;
    case REASON_ADD: {
        const status_t status = mMixer->create(
                index, fastTrack->mChannelMask, fastTrack->mFormat, AUDIO_SESSION_OUTPUT_MIX);
        LOG_ALWAYS_FATAL_IF(status != NO_ERROR,
                "%s: cannot create fast track index"
                " %d, mask %#x, format %#x in AudioMixer",
                __func__, index, fastTrack->mChannelMask, fastTrack->mFormat);
    }
        [[fallthrough]];  // now fallthrough to update the newly created track.
    case REASON_MODIFY:
        mMixer->setBufferProvider(index, fastTrack->mBufferProvider);

        float vlf, vrf;
        if (fastTrack->mVolumeProvider != nullptr) {
            const gain_minifloat_packed_t vlr = fastTrack->mVolumeProvider->getVolumeLR();
            vlf = float_from_gain(gain_minifloat_unpack_left(vlr));
            vrf = float_from_gain(gain_minifloat_unpack_right(vlr));
        } else {
            vlf = vrf = AudioMixer::UNITY_GAIN_FLOAT;
        }

        // set volume to avoid ramp whenever the track is updated (or created).
        // Note: this does not distinguish from starting fresh or
        // resuming from a paused state.
        mMixer->setParameter(index, AudioMixer::VOLUME, AudioMixer::VOLUME0, &vlf);
        mMixer->setParameter(index, AudioMixer::VOLUME, AudioMixer::VOLUME1, &vrf);

        mMixer->setParameter(index, AudioMixer::RESAMPLE, AudioMixer::REMOVE, nullptr);
        mMixer->setParameter(index, AudioMixer::TRACK, AudioMixer::MAIN_BUFFER,
                (void *)mMixerBuffer);
        mMixer->setParameter(index, AudioMixer::TRACK, AudioMixer::MIXER_FORMAT,
                (void *)(uintptr_t)mMixerBufferFormat);
        mMixer->setParameter(index, AudioMixer::TRACK, AudioMixer::FORMAT,
                (void *)(uintptr_t)fastTrack->mFormat);
        mMixer->setParameter(index, AudioMixer::TRACK, AudioMixer::CHANNEL_MASK,
                (void *)(uintptr_t)fastTrack->mChannelMask);
        mMixer->setParameter(index, AudioMixer::TRACK, AudioMixer::MIXER_CHANNEL_MASK,
                (void *)(uintptr_t)mSinkChannelMask);
        mMixer->setParameter(index, AudioMixer::TRACK, AudioMixer::HAPTIC_ENABLED,
                (void *)(uintptr_t)fastTrack->mHapticPlaybackEnabled);
        mMixer->setParameter(index, AudioMixer::TRACK, AudioMixer::HAPTIC_INTENSITY,
                (void *)(uintptr_t)fastTrack->mHapticIntensity);

        mMixer->enable(index);
        break;
    default:
        LOG_ALWAYS_FATAL("%s: invalid update reason %d", __func__, reason);
    }
}

void FastMixer::onStateChange()
{
    const FastMixerState * const current = (const FastMixerState *) mCurrent;
    const FastMixerState * const previous = (const FastMixerState *) mPrevious;
    FastMixerDumpState * const dumpState = (FastMixerDumpState *) mDumpState;
    const size_t frameCount = current->mFrameCount;

    // update boottime offset, in case it has changed
    mTimestamp.mTimebaseOffset[ExtendedTimestamp::TIMEBASE_BOOTTIME] =
            mBoottimeOffset.load();

    // handle state change here, but since we want to diff the state,
    // we're prepared for previous == &sInitial the first time through
    unsigned previousTrackMask;

    // check for change in output HAL configuration
    NBAIO_Format previousFormat = mFormat;
    if (current->mOutputSinkGen != mOutputSinkGen) {
        mOutputSink = current->mOutputSink;
        mOutputSinkGen = current->mOutputSinkGen;
        mSinkChannelMask = current->mSinkChannelMask;
        mBalance.setChannelMask(mSinkChannelMask);
        if (mOutputSink == NULL) {
            mFormat = Format_Invalid;
            mSampleRate = 0;
            mSinkChannelCount = 0;
            mSinkChannelMask = AUDIO_CHANNEL_NONE;
            mAudioChannelCount = 0;
        } else {
            mFormat = mOutputSink->format();
            mSampleRate = Format_sampleRate(mFormat);
            mSinkChannelCount = Format_channelCount(mFormat);
            LOG_ALWAYS_FATAL_IF(mSinkChannelCount > AudioMixer::MAX_NUM_CHANNELS);

            if (mSinkChannelMask == AUDIO_CHANNEL_NONE) {
                mSinkChannelMask = audio_channel_out_mask_from_count(mSinkChannelCount);
            }
            mAudioChannelCount = mSinkChannelCount - audio_channel_count_from_out_mask(
                    mSinkChannelMask & AUDIO_CHANNEL_HAPTIC_ALL);
        }
        dumpState->mSampleRate = mSampleRate;
    }

    if ((!Format_isEqual(mFormat, previousFormat)) || (frameCount != previous->mFrameCount)) {
        // FIXME to avoid priority inversion, don't delete here
        delete mMixer;
        mMixer = NULL;
        free(mMixerBuffer);
        mMixerBuffer = NULL;
        free(mSinkBuffer);
        mSinkBuffer = NULL;
        if (frameCount > 0 && mSampleRate > 0) {
            // FIXME new may block for unbounded time at internal mutex of the heap
            //       implementation; it would be better to have normal mixer allocate for us
            //       to avoid blocking here and to prevent possible priority inversion
            mMixer = new AudioMixer(frameCount, mSampleRate);
            // FIXME See the other FIXME at FastMixer::setNBLogWriter()
            NBLog::thread_params_t params;
            params.frameCount = frameCount;
            params.sampleRate = mSampleRate;
            LOG_THREAD_PARAMS(params);
            const size_t mixerFrameSize = mSinkChannelCount
                    * audio_bytes_per_sample(mMixerBufferFormat);
            mMixerBufferSize = mixerFrameSize * frameCount;
            (void)posix_memalign(&mMixerBuffer, 32, mMixerBufferSize);
            const size_t sinkFrameSize = mSinkChannelCount
                    * audio_bytes_per_sample(mFormat.mFormat);
            if (sinkFrameSize > mixerFrameSize) { // need a sink buffer
                mSinkBufferSize = sinkFrameSize * frameCount;
                (void)posix_memalign(&mSinkBuffer, 32, mSinkBufferSize);
            }
            mPeriodNs = (frameCount * 1000000000LL) / mSampleRate;    // 1.00
            mUnderrunNs = (frameCount * 1750000000LL) / mSampleRate;  // 1.75
            mOverrunNs = (frameCount * 500000000LL) / mSampleRate;    // 0.50
            mForceNs = (frameCount * 950000000LL) / mSampleRate;      // 0.95
            mWarmupNsMin = (frameCount * 750000000LL) / mSampleRate;  // 0.75
            mWarmupNsMax = (frameCount * 1250000000LL) / mSampleRate; // 1.25
        } else {
            mPeriodNs = 0;
            mUnderrunNs = 0;
            mOverrunNs = 0;
            mForceNs = 0;
            mWarmupNsMin = 0;
            mWarmupNsMax = LONG_MAX;
        }
        mMixerBufferState = UNDEFINED;
        // we need to reconfigure all active tracks
        previousTrackMask = 0;
        mFastTracksGen = current->mFastTracksGen - 1;
        dumpState->mFrameCount = frameCount;
#ifdef TEE_SINK
        mTee.set(mFormat, NBAIO_Tee::TEE_FLAG_OUTPUT_THREAD);
        mTee.setId(std::string("_") + std::to_string(mThreadIoHandle) + "_F");
#endif
    } else {
        previousTrackMask = previous->mTrackMask;
    }

    // check for change in active track set
    const unsigned currentTrackMask = current->mTrackMask;
    dumpState->mTrackMask = currentTrackMask;
    dumpState->mNumTracks = popcount(currentTrackMask);
    if (current->mFastTracksGen != mFastTracksGen) {

        // process removed tracks first to avoid running out of track names
        unsigned removedTracks = previousTrackMask & ~currentTrackMask;
        while (removedTracks != 0) {
            int i = __builtin_ctz(removedTracks);
            removedTracks &= ~(1 << i);
            updateMixerTrack(i, REASON_REMOVE);
            // don't reset track dump state, since other side is ignoring it
        }

        // now process added tracks
        unsigned addedTracks = currentTrackMask & ~previousTrackMask;
        while (addedTracks != 0) {
            int i = __builtin_ctz(addedTracks);
            addedTracks &= ~(1 << i);
            updateMixerTrack(i, REASON_ADD);
        }

        // finally process (potentially) modified tracks; these use the same slot
        // but may have a different buffer provider or volume provider
        unsigned modifiedTracks = currentTrackMask & previousTrackMask;
        while (modifiedTracks != 0) {
            int i = __builtin_ctz(modifiedTracks);
            modifiedTracks &= ~(1 << i);
            updateMixerTrack(i, REASON_MODIFY);
        }

        mFastTracksGen = current->mFastTracksGen;
    }
}

void FastMixer::onWork()
{
    // TODO: pass an ID parameter to indicate which time series we want to write to in NBLog.cpp
    // Or: pass both of these into a single call with a boolean
    const FastMixerState * const current = (const FastMixerState *) mCurrent;
    FastMixerDumpState * const dumpState = (FastMixerDumpState *) mDumpState;

    if (mIsWarm) {
        // Logging timestamps for FastMixer is currently disabled to make memory room for logging
        // other statistics in FastMixer.
        // To re-enable, delete the #ifdef FASTMIXER_LOG_HIST_TS lines (and the #endif lines).
#ifdef FASTMIXER_LOG_HIST_TS
        LOG_HIST_TS();
#endif
        //ALOGD("Eric FastMixer::onWork() mIsWarm");
    } else {
        dumpState->mTimestampVerifier.discontinuity();
        // See comment in if block.
#ifdef FASTMIXER_LOG_HIST_TS
        LOG_AUDIO_STATE();
#endif
    }
    const FastMixerState::Command command = mCommand;
    const size_t frameCount = current->mFrameCount;

    if ((command & FastMixerState::MIX) && (mMixer != NULL) && mIsWarm) {
        ALOG_ASSERT(mMixerBuffer != NULL);

        // AudioMixer::mState.enabledTracks is undefined if mState.hook == process__validate,
        // so we keep a side copy of enabledTracks
        bool anyEnabledTracks = false;

        // for each track, update volume and check for underrun
        unsigned currentTrackMask = current->mTrackMask;
        while (currentTrackMask != 0) {
            int i = __builtin_ctz(currentTrackMask);
            currentTrackMask &= ~(1 << i);
            const FastTrack* fastTrack = &current->mFastTracks[i];

            const int64_t trackFramesWrittenButNotPresented =
                mNativeFramesWrittenButNotPresented;
            const int64_t trackFramesWritten = fastTrack->mBufferProvider->framesReleased();
            ExtendedTimestamp perTrackTimestamp(mTimestamp);

            // Can't provide an ExtendedTimestamp before first frame presented.
            // Also, timestamp may not go to very last frame on stop().
            if (trackFramesWritten >= trackFramesWrittenButNotPresented &&
                    perTrackTimestamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL] > 0) {
                perTrackTimestamp.mPosition[ExtendedTimestamp::LOCATION_KERNEL] =
                        trackFramesWritten - trackFramesWrittenButNotPresented;
            } else {
                perTrackTimestamp.mPosition[ExtendedTimestamp::LOCATION_KERNEL] = 0;
                perTrackTimestamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL] = -1;
            }
            perTrackTimestamp.mPosition[ExtendedTimestamp::LOCATION_SERVER] = trackFramesWritten;
            fastTrack->mBufferProvider->onTimestamp(perTrackTimestamp);

            const int name = i;
            if (fastTrack->mVolumeProvider != NULL) {
                gain_minifloat_packed_t vlr = fastTrack->mVolumeProvider->getVolumeLR();
                float vlf = float_from_gain(gain_minifloat_unpack_left(vlr));
                float vrf = float_from_gain(gain_minifloat_unpack_right(vlr));

                mMixer->setParameter(name, AudioMixer::RAMP_VOLUME, AudioMixer::VOLUME0, &vlf);
                mMixer->setParameter(name, AudioMixer::RAMP_VOLUME, AudioMixer::VOLUME1, &vrf);
            }
            // FIXME The current implementation of framesReady() for fast tracks
            // takes a tryLock, which can block
            // up to 1 ms.  If enough active tracks all blocked in sequence, this would result
            // in the overall fast mix cycle being delayed.  Should use a non-blocking FIFO.
            size_t framesReady = fastTrack->mBufferProvider->framesReady();
            if (ATRACE_ENABLED()) {
                // I wish we had formatted trace names
                char traceName[16];
                strcpy(traceName, "fRdy");
                traceName[4] = i + (i < 10 ? '0' : 'A' - 10);
                traceName[5] = '\0';
                ATRACE_INT(traceName, framesReady);
            }
            FastTrackDump *ftDump = &dumpState->mTracks[i];
            FastTrackUnderruns underruns = ftDump->mUnderruns;
            if (framesReady < frameCount) {
                if (framesReady == 0) {
                    underruns.mBitFields.mEmpty++;
                    underruns.mBitFields.mMostRecent = UNDERRUN_EMPTY;
                    mMixer->disable(name);
                } else {
                    // allow mixing partial buffer
                    underruns.mBitFields.mPartial++;
                    underruns.mBitFields.mMostRecent = UNDERRUN_PARTIAL;
                    mMixer->enable(name);
                    anyEnabledTracks = true;
                }
            } else {
                underruns.mBitFields.mFull++;
                underruns.mBitFields.mMostRecent = UNDERRUN_FULL;
                mMixer->enable(name);
                anyEnabledTracks = true;
            }
            ftDump->mUnderruns = underruns;
            ftDump->mFramesReady = framesReady;
            ftDump->mFramesWritten = trackFramesWritten;
        }

        if (anyEnabledTracks) {
            // process() is CPU-bound
            mMixer->process();
            mMixerBufferState = MIXED;
        } else if (mMixerBufferState != ZEROED) {
            mMixerBufferState = UNDEFINED;
        }

    } else if (mMixerBufferState == MIXED) {
        mMixerBufferState = UNDEFINED;
    }
    //bool didFullWrite = false;    // dumpsys could display a count of partial writes
    if ((command & FastMixerState::WRITE) && (mOutputSink != NULL) && (mMixerBuffer != NULL)) {
        if (mMixerBufferState == UNDEFINED) {
            memset(mMixerBuffer, 0, mMixerBufferSize);
            mMixerBufferState = ZEROED;
        }

        if (mMasterMono.load()) {  // memory_order_seq_cst
            mono_blend(mMixerBuffer, mMixerBufferFormat, Format_channelCount(mFormat), frameCount,
                    true /*limit*/);
        }

        // Balance must take effect after mono conversion.
        // mBalance detects zero balance within the class for speed (not needed here).
        mBalance.setBalance(mMasterBalance.load());
        mBalance.process((float *)mMixerBuffer, frameCount);

        // prepare the buffer used to write to sink
        void *buffer = mSinkBuffer != NULL ? mSinkBuffer : mMixerBuffer;
        if (mFormat.mFormat != mMixerBufferFormat) { // sink format not the same as mixer format
            memcpy_by_audio_format(buffer, mFormat.mFormat, mMixerBuffer, mMixerBufferFormat,
                    frameCount * Format_channelCount(mFormat));
        }
        if (mSinkChannelMask & AUDIO_CHANNEL_HAPTIC_ALL) {
            // When there are haptic channels, the sample data is partially interleaved.
            // Make the sample data fully interleaved here.
            adjust_channels_non_destructive(buffer, mAudioChannelCount, buffer, mSinkChannelCount,
                    audio_bytes_per_sample(mFormat.mFormat),
                    frameCount * audio_bytes_per_frame(mAudioChannelCount, mFormat.mFormat));
        }
        // if non-NULL, then duplicate write() to this non-blocking sink
#ifdef TEE_SINK
        mTee.write(buffer, frameCount);
#endif
        // FIXME write() is non-blocking and lock-free for a properly implemented NBAIO sink,
        //       but this code should be modified to handle both non-blocking and blocking sinks
        dumpState->mWriteSequence++;
        ATRACE_BEGIN("write");
        ssize_t framesWritten = mOutputSink->write(buffer, frameCount);
        ATRACE_END();
        dumpState->mWriteSequence++;
        if (framesWritten >= 0) {
            ALOG_ASSERT((size_t) framesWritten <= frameCount);
            mTotalNativeFramesWritten += framesWritten;
            dumpState->mFramesWritten = mTotalNativeFramesWritten;
            //if ((size_t) framesWritten == frameCount) {
            //    didFullWrite = true;
            //}
        } else {
            dumpState->mWriteErrors++;
        }
        mAttemptedWrite = true;
        // FIXME count # of writes blocked excessively, CPU usage, etc. for dump

        if (mIsWarm) {
            ExtendedTimestamp timestamp; // local
            status_t status = mOutputSink->getTimestamp(timestamp);
            if (status == NO_ERROR) {
                dumpState->mTimestampVerifier.add(
                        timestamp.mPosition[ExtendedTimestamp::LOCATION_KERNEL],
                        timestamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL],
                        mSampleRate);
                const int64_t totalNativeFramesPresented =
                        timestamp.mPosition[ExtendedTimestamp::LOCATION_KERNEL];
                if (totalNativeFramesPresented <= mTotalNativeFramesWritten) {
                    mNativeFramesWrittenButNotPresented =
                        mTotalNativeFramesWritten - totalNativeFramesPresented;
                    mTimestamp.mPosition[ExtendedTimestamp::LOCATION_KERNEL] =
                            timestamp.mPosition[ExtendedTimestamp::LOCATION_KERNEL];
                    mTimestamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL] =
                            timestamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL];
                    // We don't compensate for server - kernel time difference and
                    // only update latency if we have valid info.
                    const double latencyMs =
                            (double)mNativeFramesWrittenButNotPresented * 1000 / mSampleRate;
                    dumpState->mLatencyMs = latencyMs;
                    LOG_LATENCY(latencyMs);
                } else {
                    // HAL reported that more frames were presented than were written
                    mNativeFramesWrittenButNotPresented = 0;
                    status = INVALID_OPERATION;
                }
            } else {
                dumpState->mTimestampVerifier.error();
            }
            if (status == NO_ERROR) {
                mTimestamp.mTimeNs[ExtendedTimestamp::LOCATION_SERVER] =
                        mTimestamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL];
            } else {
                // fetch server time if we can't get timestamp
                mTimestamp.mTimeNs[ExtendedTimestamp::LOCATION_SERVER] =
                        systemTime(SYSTEM_TIME_MONOTONIC);
                // clear out kernel cached position as this may get rapidly stale
                // if we never get a new valid timestamp
                mTimestamp.mPosition[ExtendedTimestamp::LOCATION_KERNEL] = 0;
                mTimestamp.mTimeNs[ExtendedTimestamp::LOCATION_KERNEL] = -1;
            }
        }
    }
}

}   // namespace android
