/*
 * Copyright (C) 2017 The Android Open Source Project
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


#define LOG_TAG "PerformanceAnalysis"
// #define LOG_NDEBUG 0
// #define WRITE_TO_FILE

#include <algorithm>
#include <climits>
#include <deque>
#include <iomanip>
#include <math.h>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>
#include <time.h>
#include <new>
#include <audio_utils/LogPlot.h>
#include <audio_utils/roundup.h>
#include <media/nblog/NBLog.h>
#include <media/nblog/PerformanceAnalysis.h>
#include <media/nblog/ReportPerformance.h>
#include <utils/Log.h>
#include <utils/String8.h>
#include <utils/Timers.h>

#include <queue>
#include <utility>

namespace android {
namespace ReportPerformance {

void Histogram::add(double value)
{
    if (mBinSize <= 0 || mBins.size() < 2) {
        return;
    }
    // TODO Handle domain and range error exceptions?
    const int unboundedIndex = lround((value - mLow) / mBinSize) + 1;
    // std::clamp is introduced in C++17
    //const int index = std::clamp(unboundedIndex, 0, (int)(mBins.size() - 1));
    const int index = std::max(0, std::min((int)(mBins.size() - 1), unboundedIndex));
    mBins[index]++;
    mTotalCount++;
}

void Histogram::clear()
{
    std::fill(mBins.begin(), mBins.end(), 0);
    mTotalCount = 0;
}

uint64_t Histogram::totalCount() const
{
    return mTotalCount;
}

std::string Histogram::toString() const {
    std::stringstream ss;
    static constexpr char kDivider = '|';
    ss << kVersion << "," << mBinSize << "," << mNumBins << "," << mLow << ",{";
    bool first = true;
    for (size_t i = 0; i < mBins.size(); i++) {
        if (mBins[i] != 0) {
            if (!first) {
                ss << ",";
            }
            ss << static_cast<int>(i) - 1 << kDivider << mBins[i];
            first = false;
        }
    }
    ss << "}";

    return ss.str();
}

std::string Histogram::asciiArtString(size_t indent) const {
    if (totalCount() == 0 || mBinSize <= 0 || mBins.size() < 2) {
        return "";
    }

    static constexpr char kMarker = '-';
    // One increment is considered one step of a bin's height.
    static constexpr size_t kMarkersPerIncrement = 2;
    static constexpr size_t kMaxIncrements = 64 + 1;
    static constexpr size_t kMaxNumberWidth = 7;
    static const std::string kMarkers(kMarkersPerIncrement * kMaxIncrements, kMarker);
    static const std::string kSpaces(kMarkersPerIncrement * kMaxIncrements, ' ');
    // get the last n characters of s, or the whole string if it is shorter
    auto getTail = [](const size_t n, const std::string &s) {
        return s.c_str() + s.size() - std::min(n, s.size());
    };

    // Since totalCount() > 0, mBins is not empty and maxCount > 0.
    const unsigned maxCount = *std::max_element(mBins.begin(), mBins.end());
    const size_t maxIncrements = log2(maxCount) + 1;

    std::stringstream ss;

    // Non-zero bins must exist at this point because totalCount() > 0.
    size_t firstNonZeroBin = 0;
    // If firstNonZeroBin reaches mBins.size() - 1, then it must be a nonzero bin.
    for (; firstNonZeroBin < mBins.size() - 1 && mBins[firstNonZeroBin] == 0; firstNonZeroBin++) {}
    const size_t firstBinToPrint = firstNonZeroBin == 0 ? 0 : firstNonZeroBin - 1;

    size_t lastNonZeroBin = mBins.size() - 1;
    // If lastNonZeroBin reaches 0, then it must be a nonzero bin.
    for (; lastNonZeroBin > 0 && mBins[lastNonZeroBin] == 0; lastNonZeroBin--) {}
    const size_t lastBinToPrint = lastNonZeroBin == mBins.size() - 1 ? lastNonZeroBin
            : lastNonZeroBin + 1;

    for (size_t bin = firstBinToPrint; bin <= lastBinToPrint; bin++) {
        ss << std::setw(indent + kMaxNumberWidth);
        if (bin == 0) {
            ss << "<";
        } else if (bin == mBins.size() - 1) {
            ss << ">";
        } else {
            ss << mLow + (bin - 1) * mBinSize;
        }
        ss << " |";
        size_t increments = 0;
        const uint64_t binCount = mBins[bin];
        if (binCount > 0) {
            increments = log2(binCount) + 1;
            ss << getTail(increments * kMarkersPerIncrement, kMarkers);
        }
        ss << getTail((maxIncrements - increments + 1) * kMarkersPerIncrement, kSpaces)
                << binCount << "\n";
    }
    ss << "\n";

    return ss.str();
}

//------------------------------------------------------------------------------

// Given an audio processing wakeup timestamp, buckets the time interval
// since the previous timestamp into a histogram, searches for
// outliers, analyzes the outlier series for unexpectedly
// small or large values and stores these as peaks
void PerformanceAnalysis::logTsEntry(timestamp ts) {
    // after a state change, start a new series and do not
    // record time intervals in-between
    if (mBufferPeriod.mPrevTs == 0) {
        mBufferPeriod.mPrevTs = ts;
        return;
    }

    // calculate time interval between current and previous timestamp
    const msInterval diffMs = static_cast<msInterval>(
        deltaMs(mBufferPeriod.mPrevTs, ts));

    const int diffJiffy = deltaJiffy(mBufferPeriod.mPrevTs, ts);

    // old versus new weight ratio when updating the buffer period mean
    static constexpr double exponentialWeight = 0.999;
    // update buffer period mean with exponential weighting
    mBufferPeriod.mMean = (mBufferPeriod.mMean < 0) ? diffMs :
            exponentialWeight * mBufferPeriod.mMean + (1.0 - exponentialWeight) * diffMs;
    // set mOutlierFactor to a smaller value for the fastmixer thread
    const int kFastMixerMax = 10;
    // NormalMixer times vary much more than FastMixer times.
    // TODO: mOutlierFactor values are set empirically based on what appears to be
    // an outlier. Learn these values from the data.
    mBufferPeriod.mOutlierFactor = mBufferPeriod.mMean < kFastMixerMax ? 1.8 : 2.0;
    // set outlier threshold
    mBufferPeriod.mOutlier = mBufferPeriod.mMean * mBufferPeriod.mOutlierFactor;

    // Check whether the time interval between the current timestamp
    // and the previous one is long enough to count as an outlier
    const bool isOutlier = detectAndStoreOutlier(diffMs);
    // If an outlier was found, check whether it was a peak
    if (isOutlier) {
        /*bool isPeak =*/ detectAndStorePeak(
            mOutlierData[0].first, mOutlierData[0].second);
        // TODO: decide whether to insert a new empty histogram if a peak
        // TODO: remove isPeak if unused to avoid "unused variable" error
        // occurred at the current timestamp
    }

    // Insert a histogram to mHists if it is empty, or
    // close the current histogram and insert a new empty one if
    // if the current histogram has spanned its maximum time interval.
    if (mHists.empty() ||
        deltaMs(mHists[0].first, ts) >= kMaxLength.HistTimespanMs) {
        mHists.emplace_front(ts, std::map<int, int>());
        // When memory is full, delete oldest histogram
        // TODO: use a circular buffer
        if (mHists.size() >= kMaxLength.Hists) {
            mHists.resize(kMaxLength.Hists);
        }
    }
    // add current time intervals to histogram
    ++mHists[0].second[diffJiffy];
    // update previous timestamp
    mBufferPeriod.mPrevTs = ts;
}


// forces short-term histogram storage to avoid adding idle audio time interval
// to buffer period data
void PerformanceAnalysis::handleStateChange() {
    mBufferPeriod.mPrevTs = 0;
    return;
}


// Checks whether the time interval between two outliers is far enough from
// a typical delta to be considered a peak.
// looks for changes in distribution (peaks), which can be either positive or negative.
// The function sets the mean to the starting value and sigma to 0, and updates
// them as long as no peak is detected. When a value is more than 'threshold'
// standard deviations from the mean, a peak is detected and the mean and sigma
// are set to the peak value and 0.
bool PerformanceAnalysis::detectAndStorePeak(msInterval diff, timestamp ts) {
    bool isPeak = false;
    if (mOutlierData.empty()) {
        return false;
    }
    // Update mean of the distribution
    // TypicalDiff is used to check whether a value is unusually large
    // when we cannot use standard deviations from the mean because the sd is set to 0.
    mOutlierDistribution.mTypicalDiff = (mOutlierDistribution.mTypicalDiff *
            (mOutlierData.size() - 1) + diff) / mOutlierData.size();

    // Initialize short-term mean at start of program
    if (mOutlierDistribution.mMean == 0) {
        mOutlierDistribution.mMean = diff;
    }
    // Update length of current sequence of outliers
    mOutlierDistribution.mN++;

    // Check whether a large deviation from the mean occurred.
    // If the standard deviation has been reset to zero, the comparison is
    // instead to the mean of the full mOutlierInterval sequence.
    if ((fabs(diff - mOutlierDistribution.mMean) <
            mOutlierDistribution.kMaxDeviation * mOutlierDistribution.mSd) ||
            (mOutlierDistribution.mSd == 0 &&
            fabs(diff - mOutlierDistribution.mMean) <
            mOutlierDistribution.mTypicalDiff)) {
        // update the mean and sd using online algorithm
        // https://en.wikipedia.org/wiki/
        // Algorithms_for_calculating_variance#Online_algorithm
        mOutlierDistribution.mN++;
        const double kDelta = diff - mOutlierDistribution.mMean;
        mOutlierDistribution.mMean += kDelta / mOutlierDistribution.mN;
        const double kDelta2 = diff - mOutlierDistribution.mMean;
        mOutlierDistribution.mM2 += kDelta * kDelta2;
        mOutlierDistribution.mSd = (mOutlierDistribution.mN < 2) ? 0 :
                sqrt(mOutlierDistribution.mM2 / (mOutlierDistribution.mN - 1));
    } else {
        // new value is far from the mean:
        // store peak timestamp and reset mean, sd, and short-term sequence
        isPeak = true;
        mPeakTimestamps.emplace_front(ts);
        // if mPeaks has reached capacity, delete oldest data
        // Note: this means that mOutlierDistribution values do not exactly
        // match the data we have in mPeakTimestamps, but this is not an issue
        // in practice for estimating future peaks.
        // TODO: turn this into a circular buffer
        if (mPeakTimestamps.size() >= kMaxLength.Peaks) {
            mPeakTimestamps.resize(kMaxLength.Peaks);
        }
        mOutlierDistribution.mMean = 0;
        mOutlierDistribution.mSd = 0;
        mOutlierDistribution.mN = 0;
        mOutlierDistribution.mM2 = 0;
    }
    return isPeak;
}


// Determines whether the difference between a timestamp and the previous
// one is beyond a threshold. If yes, stores the timestamp as an outlier
// and writes to mOutlierdata in the following format:
// Time elapsed since previous outlier: Timestamp of start of outlier
// e.g. timestamps (ms) 1, 4, 5, 16, 18, 28 will produce pairs (4, 5), (13, 18).
// TODO: learn what timestamp sequences correlate with glitches instead of
// manually designing a heuristic.
bool PerformanceAnalysis::detectAndStoreOutlier(const msInterval diffMs) {
    bool isOutlier = false;
    if (diffMs >= mBufferPeriod.mOutlier) {
        isOutlier = true;
        mOutlierData.emplace_front(
                mOutlierDistribution.mElapsed, mBufferPeriod.mPrevTs);
        // Remove oldest value if the vector is full
        // TODO: turn this into a circular buffer
        // TODO: make sure kShortHistSize is large enough that that data will never be lost
        // before being written to file or to a FIFO
        if (mOutlierData.size() >= kMaxLength.Outliers) {
            mOutlierData.resize(kMaxLength.Outliers);
        }
        mOutlierDistribution.mElapsed = 0;
    }
    mOutlierDistribution.mElapsed += diffMs;
    return isOutlier;
}

// rounds value to precision based on log-distance from mean
__attribute__((no_sanitize("signed-integer-overflow")))
inline double logRound(double x, double mean) {
    // Larger values decrease range of high resolution and prevent overflow
    // of a histogram on the console.
    // The following formula adjusts kBase based on the buffer period length.
    // Different threads have buffer periods ranging from 2 to 40. The
    // formula below maps buffer period 2 to kBase = ~1, 4 to ~2, 20 to ~3, 40 to ~4.
    // TODO: tighten this for higher means, the data still overflows
    const double kBase = log(mean) / log(2.2);
    const double power = floor(
        log(abs(x - mean) / mean) / log(kBase)) + 2;
    // do not round values close to the mean
    if (power < 1) {
        return x;
    }
    const int factor = static_cast<int>(pow(10, power));
    return (static_cast<int>(x) * factor) / factor;
}

// TODO Make it return a std::string instead of modifying body
// TODO: move this to ReportPerformance, probably make it a friend function
// of PerformanceAnalysis
void PerformanceAnalysis::reportPerformance(String8 *body, int author, log_hash_t hash,
                                            int maxHeight) {
    if (mHists.empty() || body == nullptr) {
        return;
    }

    // ms of active audio in displayed histogram
    double elapsedMs = 0;
    // starting timestamp of histogram
    timestamp startingTs = mHists[0].first;

    // histogram which stores .1 precision ms counts instead of Jiffy multiple counts
    std::map<double, int> buckets;
    for (const auto &shortHist: mHists) {
        for (const auto &countPair : shortHist.second) {
            const double ms = static_cast<double>(countPair.first) / kJiffyPerMs;
            buckets[logRound(ms, mBufferPeriod.mMean)] += countPair.second;
            elapsedMs += ms * countPair.second;
        }
    }

    static const int SIZE = 128;
    char title[SIZE];
    snprintf(title, sizeof(title), "\n%s %3.2f %s\n%s%d, %lld, %lld\n",
            "Occurrences in", (elapsedMs / kMsPerSec), "seconds of audio:",
            "Thread, hash, starting timestamp: ", author,
            static_cast<long long>(hash), static_cast<long long>(startingTs));
    static const char * const kLabel = "ms";

    body->appendFormat("%s",
            audio_utils_plot_histogram(buckets, title, kLabel, maxHeight).c_str());

    // Now report glitches
    body->appendFormat("\ntime elapsed between glitches and glitch timestamps:\n");
    for (const auto &outlier: mOutlierData) {
        body->appendFormat("%lld: %lld\n", static_cast<long long>(outlier.first),
                           static_cast<long long>(outlier.second));
    }
}

//------------------------------------------------------------------------------

// writes summary of performance into specified file descriptor
void dump(int fd, int indent, PerformanceAnalysisMap &threadPerformanceAnalysis) {
    String8 body;
#ifdef WRITE_TO_FILE
    const char* const kDirectory = "/data/misc/audioserver/";
#endif
    for (auto & thread : threadPerformanceAnalysis) {
        for (auto & hash: thread.second) {
            PerformanceAnalysis& curr = hash.second;
            // write performance data to console
            curr.reportPerformance(&body, thread.first, hash.first);
            if (!body.isEmpty()) {
                dumpLine(fd, indent, body);
                body.clear();
            }
#ifdef WRITE_TO_FILE
            // write to file. Enable by uncommenting macro at top of file.
            writeToFile(curr.mHists, curr.mOutlierData, curr.mPeakTimestamps,
                        kDirectory, false, thread.first, hash.first);
#endif
        }
    }
}


// Writes a string into specified file descriptor
void dumpLine(int fd, int indent, const String8 &body) {
    dprintf(fd, "%.*s%s \n", indent, "", body.string());
}

} // namespace ReportPerformance
}   // namespace android
