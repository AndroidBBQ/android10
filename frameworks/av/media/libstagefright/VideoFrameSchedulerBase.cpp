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

//#define LOG_NDEBUG 0
#define LOG_TAG "VideoFrameSchedulerBase"
#include <utils/Log.h>
#define ATRACE_TAG ATRACE_TAG_VIDEO
#include <utils/Trace.h>
#include <utils/Vector.h>

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AUtils.h>
#include <media/stagefright/VideoFrameSchedulerBase.h>

namespace android {

template<class T>
static int compare(const T *lhs, const T *rhs) {
    if (*lhs < *rhs) {
        return -1;
    } else if (*lhs > *rhs) {
        return 1;
    } else {
        return 0;
    }
}

/* ======================================================================= */
/*                                   PLL                                   */
/* ======================================================================= */

static const size_t kMinSamplesToStartPrime = 3;
static const size_t kMinSamplesToStopPrime = VideoFrameSchedulerBase::kHistorySize;
static const size_t kMinSamplesToEstimatePeriod = 3;
static const size_t kMaxSamplesToEstimatePeriod = VideoFrameSchedulerBase::kHistorySize;

static const size_t kPrecision = 12;
static const int64_t kErrorThreshold = (1 << (kPrecision * 2)) / 10;
static const int64_t kMultiplesThresholdDiv = 4;                                     // 25%
static const int64_t kReFitThresholdDiv = 100;                                       // 1%
static const nsecs_t kMaxAllowedFrameSkip = VideoFrameSchedulerBase::kNanosIn1s;     // 1 sec
static const nsecs_t kMinPeriod = VideoFrameSchedulerBase::kNanosIn1s / 120;         // 120Hz
static const nsecs_t kRefitRefreshPeriod = 10 * VideoFrameSchedulerBase::kNanosIn1s; // 10 sec

VideoFrameSchedulerBase::PLL::PLL()
    : mPeriod(-1),
      mPhase(0),
      mPrimed(false),
      mSamplesUsedForPriming(0),
      mLastTime(-1),
      mNumSamples(0) {
}

void VideoFrameSchedulerBase::PLL::reset(float fps) {
    //test();

    mSamplesUsedForPriming = 0;
    mLastTime = -1;

    // set up or reset video PLL
    if (fps <= 0.f) {
        mPeriod = -1;
        mPrimed = false;
    } else {
        ALOGV("reset at %.1f fps", fps);
        mPeriod = (nsecs_t)(1e9 / fps + 0.5);
        mPrimed = true;
    }

    restart();
}

// reset PLL but keep previous period estimate
void VideoFrameSchedulerBase::PLL::restart() {
    mNumSamples = 0;
    mPhase = -1;
}

#if 0

void VideoFrameSchedulerBase::PLL::test() {
    nsecs_t period = VideoFrameSchedulerBase::kNanosIn1s / 60;
    mTimes[0] = 0;
    mTimes[1] = period;
    mTimes[2] = period * 3;
    mTimes[3] = period * 4;
    mTimes[4] = period * 7;
    mTimes[5] = period * 8;
    mTimes[6] = period * 10;
    mTimes[7] = period * 12;
    mNumSamples = 8;
    int64_t a, b, err;
    fit(0, period * 12 / 7, 8, &a, &b, &err);
    // a = 0.8(5)+
    // b = -0.14097(2)+
    // err = 0.2750578(703)+
    ALOGD("a=%lld (%.6f), b=%lld (%.6f), err=%lld (%.6f)",
            (long long)a, (a / (float)(1 << kPrecision)),
            (long long)b, (b / (float)(1 << kPrecision)),
            (long long)err, (err / (float)(1 << (kPrecision * 2))));
}

#endif

// If overflow happens, the value is already incorrect, and no mater what value we get is OK.
// And this part of calculation is not important, so it's OK to simply disable overflow check
// instead of using double which makes code more complicated.
__attribute__((no_sanitize("integer")))
bool VideoFrameSchedulerBase::PLL::fit(
        nsecs_t phase, nsecs_t period, size_t numSamplesToUse,
        int64_t *a, int64_t *b, int64_t *err) {
    if (numSamplesToUse > mNumSamples) {
        numSamplesToUse = mNumSamples;
    }

    if ((period >> kPrecision) == 0 ) {
        ALOGW("Period is 0, or after including precision is 0 - would cause div0, returning");
        return false;
    }

    int64_t sumX = 0;
    int64_t sumXX = 0;
    int64_t sumXY = 0;
    int64_t sumYY = 0;
    int64_t sumY = 0;

    int64_t x = 0; // x usually is in [0..numSamplesToUse)
    nsecs_t lastTime;
    for (size_t i = 0; i < numSamplesToUse; i++) {
        size_t ix = (mNumSamples - numSamplesToUse + i) % kHistorySize;
        nsecs_t time = mTimes[ix];
        if (i > 0) {
            x += divRound(time - lastTime, period);
        }
        // y is usually in [-numSamplesToUse..numSamplesToUse+kRefitRefreshPeriod/kMinPeriod) << kPrecision
        //   ideally in [0..numSamplesToUse), but shifted by -numSamplesToUse during
        //   priming, and possibly shifted by up to kRefitRefreshPeriod/kMinPeriod
        //   while we are not refitting.
        int64_t y = divRound(time - phase, period >> kPrecision);
        sumX += x;
        sumY += y;
        sumXX += x * x;
        sumXY += x * y;
        sumYY += y * y;
        lastTime = time;
    }

    int64_t div   = (int64_t)numSamplesToUse * sumXX - sumX * sumX;
    if (div == 0) {
        return false;
    }

    int64_t a_nom = (int64_t)numSamplesToUse * sumXY - sumX * sumY;
    int64_t b_nom = sumXX * sumY            - sumX * sumXY;
    *a = divRound(a_nom, div);
    *b = divRound(b_nom, div);
    // don't use a and b directly as the rounding error is significant
    *err = sumYY - divRound(a_nom * sumXY + b_nom * sumY, div);
    ALOGV("fitting[%zu] a=%lld (%.6f), b=%lld (%.6f), err=%lld (%.6f)",
            numSamplesToUse,
            (long long)*a,   (*a / (float)(1 << kPrecision)),
            (long long)*b,   (*b / (float)(1 << kPrecision)),
            (long long)*err, (*err / (float)(1 << (kPrecision * 2))));
    return true;
}

void VideoFrameSchedulerBase::PLL::prime(size_t numSamplesToUse) {
    if (numSamplesToUse > mNumSamples) {
        numSamplesToUse = mNumSamples;
    }
    CHECK(numSamplesToUse >= 3);  // must have at least 3 samples

    // estimate video framerate from deltas between timestamps, and
    // 2nd order deltas
    Vector<nsecs_t> deltas;
    nsecs_t lastTime, firstTime;
    for (size_t i = 0; i < numSamplesToUse; ++i) {
        size_t index = (mNumSamples - numSamplesToUse + i) % kHistorySize;
        nsecs_t time = mTimes[index];
        if (i > 0) {
            if (time - lastTime > kMinPeriod) {
                //ALOGV("delta: %lld", (long long)(time - lastTime));
                deltas.push(time - lastTime);
            }
        } else {
            firstTime = time;
        }
        lastTime = time;
    }
    deltas.sort(compare<nsecs_t>);
    size_t numDeltas = deltas.size();
    if (numDeltas > 1) {
        nsecs_t deltaMinLimit = max(deltas[0] / kMultiplesThresholdDiv, kMinPeriod);
        nsecs_t deltaMaxLimit = deltas[numDeltas / 2] * kMultiplesThresholdDiv;
        for (size_t i = numDeltas / 2 + 1; i < numDeltas; ++i) {
            if (deltas[i] > deltaMaxLimit) {
                deltas.resize(i);
                numDeltas = i;
                break;
            }
        }
        for (size_t i = 1; i < numDeltas; ++i) {
            nsecs_t delta2nd = deltas[i] - deltas[i - 1];
            if (delta2nd >= deltaMinLimit) {
                //ALOGV("delta2: %lld", (long long)(delta2nd));
                deltas.push(delta2nd);
            }
        }
    }

    // use the one that yields the best match
    int64_t bestScore;
    for (size_t i = 0; i < deltas.size(); ++i) {
        nsecs_t delta = deltas[i];
        int64_t score = 0;
#if 1
        // simplest score: number of deltas that are near multiples
        size_t matches = 0;
        for (size_t j = 0; j < deltas.size(); ++j) {
            nsecs_t err = periodicError(deltas[j], delta);
            if (err < delta / kMultiplesThresholdDiv) {
                ++matches;
            }
        }
        score = matches;
#if 0
        // could be weighed by the (1 - normalized error)
        if (numSamplesToUse >= kMinSamplesToEstimatePeriod) {
            int64_t a, b, err;
            fit(firstTime, delta, numSamplesToUse, &a, &b, &err);
            err = (1 << (2 * kPrecision)) - err;
            score *= max(0, err);
        }
#endif
#else
        // or use the error as a negative score
        if (numSamplesToUse >= kMinSamplesToEstimatePeriod) {
            int64_t a, b, err;
            fit(firstTime, delta, numSamplesToUse, &a, &b, &err);
            score = -delta * err;
        }
#endif
        if (i == 0 || score > bestScore) {
            bestScore = score;
            mPeriod = delta;
            mPhase = firstTime;
        }
    }
    ALOGV("priming[%zu] phase:%lld period:%lld",
            numSamplesToUse, (long long)mPhase, (long long)mPeriod);
}

nsecs_t VideoFrameSchedulerBase::PLL::addSample(nsecs_t time) {
    if (mLastTime >= 0
            // if time goes backward, or we skipped rendering
            && (time > mLastTime + kMaxAllowedFrameSkip || time < mLastTime)) {
        restart();
    }

    mLastTime = time;
    mTimes[mNumSamples % kHistorySize] = time;
    ++mNumSamples;

    bool doFit = time > mRefitAt;
    if ((mPeriod <= 0 || !mPrimed) && mNumSamples >= kMinSamplesToStartPrime) {
        prime(kMinSamplesToStopPrime);
        ++mSamplesUsedForPriming;
        doFit = true;
    }
    if (mPeriod > 0 && mNumSamples >= kMinSamplesToEstimatePeriod) {
        if (mPhase < 0) {
            // initialize phase to the current render time
            mPhase = time;
            doFit = true;
        } else if (!doFit) {
            int64_t err = periodicError(time - mPhase, mPeriod);
            doFit = err > mPeriod / kReFitThresholdDiv;
        }

        if (doFit) {
            int64_t a, b, err;
            if (!fit(mPhase, mPeriod, kMaxSamplesToEstimatePeriod, &a, &b, &err)) {
                // samples are not suitable for fitting.  this means they are
                // also not suitable for priming.
                ALOGV("could not fit - keeping old period:%lld", (long long)mPeriod);
                return mPeriod;
            }

            mRefitAt = time + kRefitRefreshPeriod;

            mPhase += (mPeriod * b) >> kPrecision;
            mPeriod = (mPeriod * a) >> kPrecision;
            ALOGV("new phase:%lld period:%lld", (long long)mPhase, (long long)mPeriod);

            if (err < kErrorThreshold) {
                if (!mPrimed && mSamplesUsedForPriming >= kMinSamplesToStopPrime) {
                    mPrimed = true;
                }
            } else {
                mPrimed = false;
                mSamplesUsedForPriming = 0;
            }
        }
    }
    return mPeriod;
}

nsecs_t VideoFrameSchedulerBase::PLL::getPeriod() const {
    return mPrimed ? mPeriod : 0;
}

/* ======================================================================= */
/*                             Frame Scheduler                             */
/* ======================================================================= */

VideoFrameSchedulerBase::VideoFrameSchedulerBase()
    : mVsyncTime(0),
      mVsyncPeriod(0),
      mVsyncRefreshAt(0),
      mLastVsyncTime(-1),
      mTimeCorrection(0) {
}

void VideoFrameSchedulerBase::init(float videoFps) {
    updateVsync();

    mLastVsyncTime = -1;
    mTimeCorrection = 0;

    mPll.reset(videoFps);
}

void VideoFrameSchedulerBase::restart() {
    mLastVsyncTime = -1;
    mTimeCorrection = 0;

    mPll.restart();
}

nsecs_t VideoFrameSchedulerBase::getVsyncPeriod() {
    if (mVsyncPeriod > 0) {
        return mVsyncPeriod;
    }
    return kDefaultVsyncPeriod;
}

float VideoFrameSchedulerBase::getFrameRate() {
    nsecs_t videoPeriod = mPll.getPeriod();
    if (videoPeriod > 0) {
        return 1e9 / videoPeriod;
    }
    return 0.f;
}

nsecs_t VideoFrameSchedulerBase::schedule(nsecs_t renderTime) {
    nsecs_t origRenderTime = renderTime;

    nsecs_t now = systemTime(SYSTEM_TIME_MONOTONIC);
    if (now >= mVsyncRefreshAt) {
        updateVsync();
    }

    // without VSYNC info, there is nothing to do
    if (mVsyncPeriod == 0) {
        ALOGV("no vsync: render=%lld", (long long)renderTime);
        return renderTime;
    }

    // ensure vsync time is well before (corrected) render time
    if (mVsyncTime > renderTime - 4 * mVsyncPeriod) {
        mVsyncTime -=
            ((mVsyncTime - renderTime) / mVsyncPeriod + 5) * mVsyncPeriod;
    }

    // Video presentation takes place at the VSYNC _after_ renderTime.  Adjust renderTime
    // so this effectively becomes a rounding operation (to the _closest_ VSYNC.)
    renderTime -= mVsyncPeriod / 2;

    const nsecs_t videoPeriod = mPll.addSample(origRenderTime);
    if (videoPeriod > 0) {
        // Smooth out rendering
        size_t N = 12;
        nsecs_t fiveSixthDev =
            abs(((videoPeriod * 5 + mVsyncPeriod) % (mVsyncPeriod * 6)) - mVsyncPeriod)
                    / (mVsyncPeriod / 100);
        // use 20 samples if we are doing 5:6 ratio +- 1% (e.g. playing 50Hz on 60Hz)
        if (fiveSixthDev < 12) {  /* 12% / 6 = 2% */
            N = 20;
        }

        nsecs_t offset = 0;
        nsecs_t edgeRemainder = 0;
        for (size_t i = 1; i <= N; i++) {
            offset +=
                (renderTime + mTimeCorrection + videoPeriod * i - mVsyncTime) % mVsyncPeriod;
            edgeRemainder += (videoPeriod * i) % mVsyncPeriod;
        }
        mTimeCorrection += mVsyncPeriod / 2 - offset / (nsecs_t)N;
        renderTime += mTimeCorrection;
        nsecs_t correctionLimit = mVsyncPeriod * 3 / 5;
        edgeRemainder = abs(edgeRemainder / (nsecs_t)N - mVsyncPeriod / 2);
        if (edgeRemainder <= mVsyncPeriod / 3) {
            correctionLimit /= 2;
        }

        // estimate how many VSYNCs a frame will spend on the display
        nsecs_t nextVsyncTime =
            renderTime + mVsyncPeriod - ((renderTime - mVsyncTime) % mVsyncPeriod);
        if (mLastVsyncTime >= 0) {
            size_t minVsyncsPerFrame = videoPeriod / mVsyncPeriod;
            size_t vsyncsForLastFrame = divRound(nextVsyncTime - mLastVsyncTime, mVsyncPeriod);
            bool vsyncsPerFrameAreNearlyConstant =
                periodicError(videoPeriod, mVsyncPeriod) / (mVsyncPeriod / 20) == 0;

            if (mTimeCorrection > correctionLimit &&
                    (vsyncsPerFrameAreNearlyConstant || vsyncsForLastFrame > minVsyncsPerFrame)) {
                // remove a VSYNC
                mTimeCorrection -= mVsyncPeriod / 2;
                renderTime -= mVsyncPeriod / 2;
                nextVsyncTime -= mVsyncPeriod;
                if (vsyncsForLastFrame > 0)
                    --vsyncsForLastFrame;
            } else if (mTimeCorrection < -correctionLimit &&
                    (vsyncsPerFrameAreNearlyConstant || vsyncsForLastFrame == minVsyncsPerFrame)) {
                // add a VSYNC
                mTimeCorrection += mVsyncPeriod / 2;
                renderTime += mVsyncPeriod / 2;
                nextVsyncTime += mVsyncPeriod;
                if (vsyncsForLastFrame < ULONG_MAX)
                    ++vsyncsForLastFrame;
            } else if (mTimeCorrection < -correctionLimit * 2
                    || mTimeCorrection > correctionLimit * 2) {
                ALOGW("correction beyond limit: %lld vs %lld (vsyncs for last frame: %zu, min: %zu)"
                        " restarting. render=%lld",
                        (long long)mTimeCorrection, (long long)correctionLimit,
                        vsyncsForLastFrame, minVsyncsPerFrame, (long long)origRenderTime);
                restart();
                return origRenderTime;
            }

            ATRACE_INT("FRAME_VSYNCS", vsyncsForLastFrame);
        }
        mLastVsyncTime = nextVsyncTime;
    }

    // align rendertime to the center between VSYNC edges
    renderTime -= (renderTime - mVsyncTime) % mVsyncPeriod;
    renderTime += mVsyncPeriod / 2;
    ALOGV("adjusting render: %lld => %lld", (long long)origRenderTime, (long long)renderTime);
    ATRACE_INT("FRAME_FLIP_IN(ms)", (renderTime - now) / 1000000);
    return renderTime;
}

VideoFrameSchedulerBase::~VideoFrameSchedulerBase() {}

} // namespace android
