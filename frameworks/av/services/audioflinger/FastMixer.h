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

#ifndef ANDROID_AUDIO_FAST_MIXER_H
#define ANDROID_AUDIO_FAST_MIXER_H

#include <atomic>
#include <audio_utils/Balance.h>
#include "FastThread.h"
#include "StateQueue.h"
#include "FastMixerState.h"
#include "FastMixerDumpState.h"
#include "NBAIO_Tee.h"

namespace android {

class AudioMixer;

typedef StateQueue<FastMixerState> FastMixerStateQueue;

class FastMixer : public FastThread {

public:
    /** FastMixer constructor takes as param the parent MixerThread's io handle (id)
        for purposes of identification. */
    explicit FastMixer(audio_io_handle_t threadIoHandle);
    virtual ~FastMixer();

            FastMixerStateQueue* sq();

    virtual void setMasterMono(bool mono) { mMasterMono.store(mono); /* memory_order_seq_cst */ }
    virtual void setMasterBalance(float balance) { mMasterBalance.store(balance); }
    virtual float getMasterBalance() const { return mMasterBalance.load(); }
    virtual void setBoottimeOffset(int64_t boottimeOffset) {
        mBoottimeOffset.store(boottimeOffset); /* memory_order_seq_cst */
    }
private:
            FastMixerStateQueue mSQ;

    // callouts
    virtual const FastThreadState *poll();
    virtual void setNBLogWriter(NBLog::Writer *logWriter);
    virtual void onIdle();
    virtual void onExit();
    virtual bool isSubClassCommand(FastThreadState::Command command);
    virtual void onStateChange();
    virtual void onWork();

    enum Reason {
        REASON_REMOVE,
        REASON_ADD,
        REASON_MODIFY,
    };
    // called when a fast track of index has been removed, added, or modified
    void updateMixerTrack(int index, Reason reason);

    // FIXME these former local variables need comments
    static const FastMixerState sInitial;

    FastMixerState  mPreIdle;   // copy of state before we went into idle
    int             mGenerations[FastMixerState::kMaxFastTracks];
                                // last observed mFastTracks[i].mGeneration
    NBAIO_Sink*     mOutputSink;
    int             mOutputSinkGen;
    AudioMixer*     mMixer;

    // mSinkBuffer audio format is stored in format.mFormat.
    void*           mSinkBuffer;        // used for mixer output format translation
                                        // if sink format is different than mixer output.
    size_t          mSinkBufferSize;
    uint32_t        mSinkChannelCount;
    audio_channel_mask_t mSinkChannelMask;
    void*           mMixerBuffer;       // mixer output buffer.
    size_t          mMixerBufferSize;
    static constexpr audio_format_t mMixerBufferFormat = AUDIO_FORMAT_PCM_FLOAT;

    uint32_t        mAudioChannelCount; // audio channel count, excludes haptic channels.

    enum {UNDEFINED, MIXED, ZEROED} mMixerBufferState;
    NBAIO_Format    mFormat;
    unsigned        mSampleRate;
    int             mFastTracksGen;
    FastMixerDumpState mDummyFastMixerDumpState;
    int64_t         mTotalNativeFramesWritten;  // copied to dumpState->mFramesWritten

    // next 2 fields are valid only when timestampStatus == NO_ERROR
    ExtendedTimestamp mTimestamp;
    int64_t         mNativeFramesWrittenButNotPresented;

    audio_utils::Balance mBalance;

    // accessed without lock between multiple threads.
    std::atomic_bool mMasterMono;
    std::atomic<float> mMasterBalance{};
    std::atomic_int_fast64_t mBoottimeOffset;

    const audio_io_handle_t mThreadIoHandle; // parent thread id for debugging purposes
#ifdef TEE_SINK
    NBAIO_Tee       mTee;
#endif
};  // class FastMixer

}   // namespace android

#endif  // ANDROID_AUDIO_FAST_MIXER_H
