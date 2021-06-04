/*
 * Copyright 2015 The Android Open Source Project
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

#ifndef ANDROID_LINEAR_MAP_H
#define ANDROID_LINEAR_MAP_H

#include <stdint.h>

namespace android {

/*
A general purpose lookup utility that defines a mapping between X and Y as a
continuous set of line segments with shared (x, y) end-points.
The (x, y) points must be added in order, monotonically increasing in both x and y;
a log warning is emitted if this does not happen (See general usage notes below).

A limited history of (x, y) points is kept for space reasons (See general usage notes).

In AudioFlinger, we use the LinearMap to associate track frames to
sink frames.  When we want to obtain a client track timestamp, we first
get a timestamp from the sink.  The sink timestamp's position (mPosition)
corresponds to the sink frames written. We use LinearMap to figure out which track frame
the sink frame corresponds to. This allows us to substitute a track frame for the
the sink frame (keeping the mTime identical) and return that timestamp back to the client.

The method findX() can be used to retrieve an x value from a given y value and is
used for timestamps, similarly for findY() which is provided for completeness.

We update the (track frame, sink frame) points in the LinearMap each time we write data
to the sink by the AudioFlinger PlaybackThread (MixerThread).


AudioFlinger Timestamp Notes:

1) Example: Obtaining a track timestamp during playback.  In this case, the LinearMap
looks something like this:

Track Frame    Sink Frame
(track start)
0              50000  (track starts here, the sink may already be running)
1000           51000
2000           52000

When we request a track timestamp, we call the sink getTimestamp() and get for example
mPosition = 51020.  Using the LinearMap, we find we have played to track frame 1020.
We substitute the sink mPosition of 51020 with the track position 1020,
and return that timestamp to the app.

2) Example: Obtaining a track timestamp duing pause. In this case, the LinearMap
looks something like this:

Track Frame    Sink Frame
... (some time has gone by)
15000          30000
16000          31000
17000          32000
(pause here)
(suppose we call sink getTimestamp() here and get sink mPosition = 31100; that means
        we have played to track frame 16100.  The track timestamp mPosition will
        continue to advance until the sink timestamp returns a value of mPosition
        greater than 32000, corresponding to track frame 17000 when the pause was called).
17000          33000
17000          34000
...

3) If the track underruns, it appears as if a pause was called on that track.

4) If there is an underrun in the HAL layer, then it may be possible that
the sink getTimestamp() will return a value greater than the number of frames written
(it should always be less). This should be rare, if not impossible by some
HAL implementations of the sink getTimestamp. In that case, timing is lost
and we will return the most recent track frame written.

5) When called with no points in the map, findX() returns the start value (default 0).
This is consistent with starting after a stop() or flush().

6) Resuming after Track standby will be similar to coming out of pause, as the HAL ensures
framesWritten() and getTimestamp() are contiguous for non-offloaded/direct tracks.

7) LinearMap works for different speeds and sample rates as it uses
linear interpolation. Since AudioFlinger only updates speed and sample rate
exactly at the sample points pushed into the LinearMap, the returned values
from findX() and findY() are accurate regardless of how many speed or sample
rate changes are made, so long as the coordinate looked up is within the
sample history.

General usage notes:

1) In order for the LinearMap to work reliably, you cannot look backwards more
than the size of its circular buffer history, set upon creation (typically 16).
If you look back further, the position is extrapolated either from a passed in
extrapolation parameter or from the oldest line segment.

2) Points must monotonically increase in x and y. The increment between adjacent
points cannot be greater than signed 32 bits. Wrap in the x, y coordinates are supported,
since we use differences in our computation.

3) If the frame data is discontinuous (due to stop or flush) call reset() to clear
the sample counter.

4) If (x, y) are not strictly monotonic increasing, i.e. (x2 > x1) and (y2 > y1),
then one or both of the inverses y = f(x) or x = g(y) may have multiple solutions.
In that case, the most recent solution is returned by findX() or findY().  We
do not warn if (x2 == x1) or (y2 == y1), but we do logcat warn if (x2 < x1) or
(y2 < y1).

5) Due to rounding it is possible x != findX(findY(x)) or y != findY(findX(y))
even when the inverse exists. Nevertheless, the values should be close.

*/

template <typename T>
class LinearMap {
public:
    // This enumeration describes the reliability of the findX() or findY() estimation
    // in descending order.
    enum FindMethod {
        FIND_METHOD_INTERPOLATION,           // High reliability (errors due to rounding)
        FIND_METHOD_FORWARD_EXTRAPOLATION,   // Reliability based on no future speed changes
        FIND_METHOD_BACKWARD_EXTRAPOLATION,  // Reliability based on prior estimated speed
        FIND_METHOD_START_VALUE,             // No samples in history, using start value
    };

    explicit LinearMap(size_t size)
            : mSize(size),
              mPos(0), // a circular buffer, so could start anywhere. the first sample is at 1.
              mSamples(0),
              // mStepValid(false),      // only valid if mSamples > 1
              // mExtrapolateTail(false), // only valid if mSamples > 0
              mX(new T[size]),
              mY(new T[size]) { }

    ~LinearMap() {
        delete[] mX;
        delete[] mY;
    }

    // Add a new sample point to the linear map.
    //
    // The difference between the new sample and the previous sample
    // in the x or y coordinate must be less than INT32_MAX for purposes
    // of the linear interpolation or extrapolation.
    //
    // The value should be monotonic increasing (e.g. diff >= 0);
    // logcat warnings are issued if they are not.
    __attribute__((no_sanitize("integer")))
    void push(T x, T y) {
        // Assumption: we assume x, y are monotonic increasing values,
        // which (can) wrap in precision no less than 32 bits and have
        // "step" or differences between adjacent points less than 32 bits.

        if (mSamples > 0) {
            const bool lastStepValid = mStepValid;
            int32_t xdiff;
            int32_t ydiff;
            // check difference assumption here
            mStepValid = checkedDiff(&xdiff, x, mX[mPos], "x")
                    & /* bitwise AND to always warn for ydiff, though logical AND is also OK */
                    checkedDiff(&ydiff, y, mY[mPos], "y");

            // Optimization: do not add a new sample if the line segment would
            // simply extend the previous line segment.  This extends the useful
            // history by removing redundant points.
            if (mSamples > 1 && mStepValid && lastStepValid) {
                const size_t prev = previousPosition();
                const int32_t xdiff2 = x - mX[prev];
                const int32_t ydiff2 = y - mY[prev];

                // if both current step and previous step are valid (non-negative and
                // less than INT32_MAX for precision greater than 4 bytes)
                // then the sum of the two steps is valid when the
                // int32_t difference is non-negative.
                if (xdiff2 >= 0 && ydiff2 >= 0
                        && (int64_t)xdiff2 * ydiff == (int64_t)ydiff2 * xdiff) {
                    // ALOGD("reusing sample! (%u, %u) sample depth %zd", x, y, mSamples);
                    mX[mPos] = x;
                    mY[mPos] = y;
                    return;
                }
            }
        }
        if (++mPos >= mSize) {
            mPos = 0;
        }
        if (mSamples < mSize) {
            mExtrapolateTail = false;
            ++mSamples;
        } else {
            // we enable extrapolation beyond the oldest sample
            // if the sample buffers are completely full and we
            // no longer know the full history.
            mExtrapolateTail = true;
        }
        mX[mPos] = x;
        mY[mPos] = y;
    }

    // clear all samples from the circular array
    void reset() {
        // no need to reset mPos, we use a circular buffer.
        // computed values such as mStepValid are set after a subsequent push().
        mSamples = 0;
    }

    // returns true if LinearMap contains at least one sample.
    bool hasData() const {
        return mSamples != 0;
    }

    // find the corresponding X point from a Y point.
    // See findU for details.
    __attribute__((no_sanitize("integer")))
    T findX(T y, FindMethod *method = NULL, double extrapolation = 0.0, T startValue = 0) const {
        return findU(y, mX, mY, method, extrapolation, startValue);
    }

    // find the corresponding Y point from a X point.
    // See findU for details.
    __attribute__((no_sanitize("integer")))
    T findY(T x, FindMethod *method = NULL, double extrapolation = 0.0, T startValue = 0) const {
        return findU(x, mY, mX, method, extrapolation, startValue);
    }

protected:

    // returns false if the diff is out of int32_t bounds or negative.
    __attribute__((no_sanitize("integer")))
    static inline bool checkedDiff(int32_t *diff, T x2, T x1, const char *coord) {
        if (sizeof(T) >= 8) {
            const int64_t diff64 = x2 - x1;
            *diff = (int32_t)diff64;  // intentionally lose precision
            if (diff64 > INT32_MAX) {
                ALOGW("LinearMap: %s overflow diff(%lld) from %llu - %llu exceeds INT32_MAX",
                        coord, (long long)diff64,
                        (unsigned long long)x2, (unsigned long long)x1);
                return false;
            } else if (diff64 < 0) {
                ALOGW("LinearMap: %s negative diff(%lld) from %llu - %llu",
                        coord, (long long)diff64,
                        (unsigned long long)x2, (unsigned long long)x1);
                return false;
            }
            return true;
        }
        // for 32 bit integers we cannot detect overflow (it
        // shows up as a negative difference).
        *diff = x2 - x1;
        if (*diff < 0) {
            ALOGW("LinearMap: %s negative diff(%d) from %u - %u",
                    coord, *diff, (unsigned)x2, (unsigned)x1);
            return false;
        }
        return true;
    }

    // Returns the previous position in the mSamples array
    // going backwards back steps.
    //
    // Parameters:
    //   back: number of backward steps, cannot be less than zero or greater than mSamples.
    //
    __attribute__((no_sanitize("integer")))
    size_t previousPosition(ssize_t back = 1) const {
        LOG_ALWAYS_FATAL_IF(back < 0 || (size_t)back > mSamples, "Invalid back(%zd)", back);
        ssize_t position = mPos - back;
        if (position < 0) position += mSize;
        return (size_t)position;
    }

    // A generic implementation of finding the "other coordinate" with coordinates
    // (u, v) = (x, y) or (u, v) = (y, x).
    //
    // Parameters:
    //   uArray: the u axis samples.
    //   vArray: the v axis samples.
    //   method: [out] how the returned value was computed.
    //   extrapolation: the slope used when extrapolating from the
    //     first sample value or the last sample value in the history.
    //     If mExtrapolateTail is set, the slope of the last line segment
    //     is used if the extrapolation parameter is zero to continue the tail of history.
    //     At this time, we do not use a different value for forward extrapolation from the
    //     head of history from backward extrapolation from the tail of history.
    //     TODO: back extrapolation value could be stored along with mX, mY in history.
    //   startValue: used only when there are no samples in history. One can detect
    //     whether there are samples in history by the method hasData().
    //
    __attribute__((no_sanitize("integer")))
    T findU(T v, T *uArray, T *vArray, FindMethod *method,
            double extrapolation, T startValue) const {
        if (mSamples == 0) {
            if (method != NULL) {
                *method = FIND_METHOD_START_VALUE;
            }
            return startValue;  // nothing yet
        }
        ssize_t previous = 0;
        int32_t diff = 0;
        for (ssize_t i = 0; i < (ssize_t)mSamples; ++i) {
            size_t current = previousPosition(i);

            // Assumption: even though the type "T" may have precision greater
            // than 32 bits, the difference between adjacent points is limited to 32 bits.
            diff = v - vArray[current];
            if (diff >= 0 ||
                    (i == (ssize_t)mSamples - 1 && mExtrapolateTail && extrapolation == 0.0)) {
                // ALOGD("depth = %zd out of %zd", i, limit);
                if (i == 0) {
                    if (method != NULL) {
                        *method = FIND_METHOD_FORWARD_EXTRAPOLATION;
                    }
                    return uArray[current] + diff * extrapolation;
                }
                // interpolate / extrapolate: For this computation, we
                // must use differentials here otherwise we have inconsistent
                // values on modulo wrap. previous is always valid here since
                // i > 0.  we also perform rounding with the assumption
                // that uStep, vStep, and diff are non-negative.
                int32_t uStep = uArray[previous] - uArray[current]; // non-negative
                int32_t vStep = vArray[previous] - vArray[current]; // positive
                T u = uStep <= 0 || vStep <= 0 ?  // we do not permit negative ustep or vstep
                        uArray[current]
                      : ((int64_t)diff * uStep + (vStep >> 1)) / vStep + uArray[current];
                // ALOGD("u:%u  diff:%d  uStep:%d  vStep:%d  u_current:%d",
                //         u, diff, uStep, vStep, uArray[current]);
                if (method != NULL) {
                    *method = (diff >= 0) ?
                            FIND_METHOD_INTERPOLATION : FIND_METHOD_BACKWARD_EXTRAPOLATION;
                }
                return u;
            }
            previous = current;
        }
        // previous is always valid here.
        if (method != NULL) {
            *method = FIND_METHOD_BACKWARD_EXTRAPOLATION;
        }
        return uArray[previous] + diff * extrapolation;
    }

private:
    const size_t    mSize;      // Size of mX and mY arrays (history).
    size_t          mPos;       // Index in mX and mY of last pushed data;
                                // (incremented after push) [0, mSize - 1].
    size_t          mSamples;   // Number of valid samples in the array [0, mSize].
    bool            mStepValid; // Last sample step was valid (non-negative)
    bool            mExtrapolateTail; // extrapolate tail using oldest line segment
    T * const       mX;         // History of X values as a circular array.
    T * const       mY;         // History of Y values as a circular array.
};

} // namespace android

#endif // ANDROID_LINEAR_MAP_H
