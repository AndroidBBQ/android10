/*
 * Copyright (C) 2014 The Android Open Source Project
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

#define LOG_TAG "FastMixerDumpState"
//#define LOG_NDEBUG 0

#include "Configuration.h"
#ifdef FAST_THREAD_STATISTICS
#include <audio_utils/Statistics.h>
#ifdef CPU_FREQUENCY_STATISTICS
#include <cpustats/ThreadCpuUsage.h>
#endif
#endif
#include <utils/Debug.h>
#include <utils/Log.h>
#include "FastMixerDumpState.h"

namespace android {

FastMixerDumpState::FastMixerDumpState() : FastThreadDumpState(),
    mWriteSequence(0), mFramesWritten(0),
    mNumTracks(0), mWriteErrors(0),
    mSampleRate(0), mFrameCount(0),
    mTrackMask(0)
{
}

FastMixerDumpState::~FastMixerDumpState()
{
}

// helper function called by qsort()
static int compare_uint32_t(const void *pa, const void *pb)
{
    uint32_t a = *(const uint32_t *)pa;
    uint32_t b = *(const uint32_t *)pb;
    if (a < b) {
        return -1;
    } else if (a > b) {
        return 1;
    } else {
        return 0;
    }
}

void FastMixerDumpState::dump(int fd) const
{
    if (mCommand == FastMixerState::INITIAL) {
        dprintf(fd, "  FastMixer not initialized\n");
        return;
    }
    double measuredWarmupMs = (mMeasuredWarmupTs.tv_sec * 1000.0) +
            (mMeasuredWarmupTs.tv_nsec / 1000000.0);
    double mixPeriodSec = (double) mFrameCount / mSampleRate;
    dprintf(fd, "  FastMixer command=%s writeSequence=%u framesWritten=%u\n"
                "            numTracks=%u writeErrors=%u underruns=%u overruns=%u\n"
                "            sampleRate=%u frameCount=%zu measuredWarmup=%.3g ms, warmupCycles=%u\n"
                "            mixPeriod=%.2f ms latency=%.2f ms\n",
                FastMixerState::commandToString(mCommand), mWriteSequence, mFramesWritten,
                mNumTracks, mWriteErrors, mUnderruns, mOverruns,
                mSampleRate, mFrameCount, measuredWarmupMs, mWarmupCycles,
                mixPeriodSec * 1e3, mLatencyMs);
    dprintf(fd, "  FastMixer Timestamp stats: %s\n", mTimestampVerifier.toString().c_str());
#ifdef FAST_THREAD_STATISTICS
    // find the interval of valid samples
    const uint32_t bounds = mBounds;
    const uint32_t newestOpen = bounds & 0xFFFF;
    uint32_t oldestClosed = bounds >> 16;

    //uint32_t n = (newestOpen - oldestClosed) & 0xFFFF;
    uint32_t n;
    __builtin_sub_overflow(newestOpen, oldestClosed, &n);
    n &= 0xFFFF;

    if (n > mSamplingN) {
        ALOGE("too many samples %u", n);
        n = mSamplingN;
    }
    // statistics for monotonic (wall clock) time, thread raw CPU load in time, CPU clock frequency,
    // and adjusted CPU load in MHz normalized for CPU clock frequency
    audio_utils::Statistics<double> wall, loadNs;
#ifdef CPU_FREQUENCY_STATISTICS
    audio_utils::Statistics<double> kHz, loadMHz;
    uint32_t previousCpukHz = 0;
#endif
    // Assuming a normal distribution for cycle times, three standard deviations on either side of
    // the mean account for 99.73% of the population.  So if we take each tail to be 1/1000 of the
    // sample set, we get 99.8% combined, or close to three standard deviations.
    static const uint32_t kTailDenominator = 1000;
    uint32_t *tail = n >= kTailDenominator ? new uint32_t[n] : NULL;
    // loop over all the samples
    for (uint32_t j = 0; j < n; ++j) {
        size_t i = oldestClosed++ & (mSamplingN - 1);
        uint32_t wallNs = mMonotonicNs[i];
        if (tail != NULL) {
            tail[j] = wallNs;
        }
        wall.add(wallNs);
        uint32_t sampleLoadNs = mLoadNs[i];
        loadNs.add(sampleLoadNs);
#ifdef CPU_FREQUENCY_STATISTICS
        uint32_t sampleCpukHz = mCpukHz[i];
        // skip bad kHz samples
        if ((sampleCpukHz & ~0xF) != 0) {
            kHz.add(sampleCpukHz >> 4);
            if (sampleCpukHz == previousCpukHz) {
                double megacycles = (double) sampleLoadNs * (double) (sampleCpukHz >> 4) * 1e-12;
                double adjMHz = megacycles / mixPeriodSec;  // _not_ wallNs * 1e9
                loadMHz.add(adjMHz);
            }
        }
        previousCpukHz = sampleCpukHz;
#endif
    }
    if (n) {
        dprintf(fd, "  Simple moving statistics over last %.1f seconds:\n",
                    wall.getN() * mixPeriodSec);
        dprintf(fd, "    wall clock time in ms per mix cycle:\n"
                    "      mean=%.2f min=%.2f max=%.2f stddev=%.2f\n",
                    wall.getMean()*1e-6, wall.getMin()*1e-6, wall.getMax()*1e-6,
                    wall.getStdDev()*1e-6);
        dprintf(fd, "    raw CPU load in us per mix cycle:\n"
                    "      mean=%.0f min=%.0f max=%.0f stddev=%.0f\n",
                    loadNs.getMean()*1e-3, loadNs.getMin()*1e-3, loadNs.getMax()*1e-3,
                    loadNs.getStdDev()*1e-3);
    } else {
        dprintf(fd, "  No FastMixer statistics available currently\n");
    }
#ifdef CPU_FREQUENCY_STATISTICS
    dprintf(fd, "  CPU clock frequency in MHz:\n"
                "    mean=%.0f min=%.0f max=%.0f stddev=%.0f\n",
                kHz.getMean()*1e-3, kHz.getMin()*1e-3, kHz.getMax()*1e-3, kHz.getStdDev()*1e-3);
    dprintf(fd, "  adjusted CPU load in MHz (i.e. normalized for CPU clock frequency):\n"
                "    mean=%.1f min=%.1f max=%.1f stddev=%.1f\n",
                loadMHz.getMean(), loadMHz.getMin(), loadMHz.getMax(), loadMHz.getStdDev());
#endif
    if (tail != NULL) {
        qsort(tail, n, sizeof(uint32_t), compare_uint32_t);
        // assume same number of tail samples on each side, left and right
        uint32_t count = n / kTailDenominator;
        audio_utils::Statistics<double> left, right;
        for (uint32_t i = 0; i < count; ++i) {
            left.add(tail[i]);
            right.add(tail[n - (i + 1)]);
        }
        dprintf(fd, "  Distribution of mix cycle times in ms for the tails "
                    "(> ~3 stddev outliers):\n"
                    "    left tail: mean=%.2f min=%.2f max=%.2f stddev=%.2f\n"
                    "    right tail: mean=%.2f min=%.2f max=%.2f stddev=%.2f\n",
                    left.getMean()*1e-6, left.getMin()*1e-6, left.getMax()*1e-6, left.getStdDev()*1e-6,
                    right.getMean()*1e-6, right.getMin()*1e-6, right.getMax()*1e-6,
                    right.getStdDev()*1e-6);
        delete[] tail;
    }
#endif
    // The active track mask and track states are updated non-atomically.
    // So if we relied on isActive to decide whether to display,
    // then we might display an obsolete track or omit an active track.
    // Instead we always display all tracks, with an indication
    // of whether we think the track is active.
    uint32_t trackMask = mTrackMask;
    dprintf(fd, "  Fast tracks: sMaxFastTracks=%u activeMask=%#x\n",
            FastMixerState::sMaxFastTracks, trackMask);
    dprintf(fd, "  Index Active Full Partial Empty  Recent Ready    Written\n");
    for (uint32_t i = 0; i < FastMixerState::sMaxFastTracks; ++i, trackMask >>= 1) {
        bool isActive = trackMask & 1;
        const FastTrackDump *ftDump = &mTracks[i];
        const FastTrackUnderruns& underruns = ftDump->mUnderruns;
        const char *mostRecent;
        switch (underruns.mBitFields.mMostRecent) {
        case UNDERRUN_FULL:
            mostRecent = "full";
            break;
        case UNDERRUN_PARTIAL:
            mostRecent = "partial";
            break;
        case UNDERRUN_EMPTY:
            mostRecent = "empty";
            break;
        default:
            mostRecent = "?";
            break;
        }
        dprintf(fd, "  %5u %6s %4u %7u %5u %7s %5zu %10lld\n",
                i, isActive ? "yes" : "no",
                (underruns.mBitFields.mFull) & UNDERRUN_MASK,
                (underruns.mBitFields.mPartial) & UNDERRUN_MASK,
                (underruns.mBitFields.mEmpty) & UNDERRUN_MASK,
                mostRecent, ftDump->mFramesReady,
                (long long)ftDump->mFramesWritten);
    }
}

}   // android
