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

#ifndef ANDROID_MEDIA_PERFORMANCEANALYSIS_H
#define ANDROID_MEDIA_PERFORMANCEANALYSIS_H

#include <deque>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <media/nblog/Events.h>
#include <media/nblog/ReportPerformance.h>
#include <utils/Timers.h>

namespace android {

class String8;

namespace ReportPerformance {

// TODO make this a templated class and put it in a separate file.
// The templated parameters would be bin size and low limit.
/*
 * Histogram provides a way to store numeric data in histogram format and read it as a serialized
 * string. The terms "bin" and "bucket" are used interchangeably.
 *
 * This class is not thread-safe.
 */
class Histogram {
public:
    struct Config {
        const double binSize;   // TODO template type
        const size_t numBins;
        const double low;       // TODO template type
    };

    // Histograms are constructed with fixed configuration numbers. Dynamic configuration based
    // the data is possible but complex because
    // - data points are added one by one, not processed as a batch.
    // - Histograms with different configuration parameters are tricky to aggregate, and they
    //   will need to be aggregated at the Media Metrics cloud side.
    // - not providing limits theoretically allows for infinite number of buckets.

    /**
     * \brief Creates a Histogram object.
     *
     * \param binSize the width of each bin of the histogram, must be greater than 0.
     *                Units are whatever data the caller decides to store.
     * \param numBins the number of bins desired in the histogram range, must be greater than 0.
     * \param low     the lower bound of the histogram bucket values.
     *                Units are whatever data the caller decides to store.
     *                Note that the upper bound can be calculated by the following:
     *                  upper = lower + binSize * numBins.
     */
    Histogram(double binSize, size_t numBins, double low = 0.)
        : mBinSize(binSize), mNumBins(numBins), mLow(low), mBins(mNumBins + 2) {}

    Histogram(const Config &c)
        : Histogram(c.binSize, c.numBins, c.low) {}

    /**
     * \brief Add a data point to the histogram. The value of the data point
     *        is rounded to the nearest multiple of the bin size (before accounting
     *        for the lower bound offset, which may not be a multiple of the bin size).
     *
     * \param value the value of the data point to add.
     */
    void add(double value);

    /**
     * \brief Removes all data points from the histogram.
     */
    void clear();

    /**
     * \brief Returns the total number of data points added to the histogram.
     *
     * \return the total number of data points in the histogram.
     */
    uint64_t totalCount() const;

    /**
     * \brief Serializes the histogram into a string. The format is chosen to be compatible with
     *        the histogram representation to send to the Media Metrics service.
     *
     *        The string is as follows:
     *          binSize,numBins,low,{-1|lowCount,...,binIndex|count,...,numBins|highCount}
     *
     *        - binIndex is an integer with 0 <= binIndex < numBins.
     *        - count is the number of occurrences of the (rounded) value
     *          low + binSize * bucketIndex.
     *        - lowCount is the number of (rounded) values less than low.
     *        - highCount is the number of (rounded) values greater than or equal to
     *          low + binSize * numBins.
     *        - a binIndex may be skipped if its count is 0.
     *
     * \return the histogram serialized as a string.
     */
    std::string toString() const;

    // Draw log scale sideways histogram as ASCII art and store as a std::string.
    // Empty string is returned if totalCount() == 0.
    std::string asciiArtString(size_t indent = 0) const;

private:
    // Histogram version number.
    static constexpr int kVersion = 1;

    const double mBinSize;          // Size of each bucket
    const size_t mNumBins;          // Number of buckets in range (excludes low and high)
    const double mLow;              // Lower bound of values

    // Data structure to store the actual histogram. Counts of bin values less than mLow
    // are stored in mBins[0]. Bin index i corresponds to mBins[i+1]. Counts of bin values
    // >= high are stored in mBins[mNumBins + 1].
    std::vector<uint64_t> mBins;

    uint64_t mTotalCount = 0;       // Total number of values recorded
};

// This is essentially the same as class PerformanceAnalysis, but PerformanceAnalysis
// also does some additional analyzing of data, while the purpose of this struct is
// to hold data.
struct PerformanceData {
    // TODO the Histogram::Config numbers below are for FastMixer.
    // Specify different numbers for other thread types.

    // Values based on mUnderrunNs and mOverrunNs in FastMixer.cpp for frameCount = 192
    // and mSampleRate = 48000, which correspond to 2 and 7 seconds.
    static constexpr Histogram::Config kWorkConfig = { 0.25, 20, 2.};

    // Values based on trial and error logging. Need a better way to determine
    // bin size and lower/upper limits.
    static constexpr Histogram::Config kLatencyConfig = { 2., 10, 10.};

    // Values based on trial and error logging. Need a better way to determine
    // bin size and lower/upper limits.
    static constexpr Histogram::Config kWarmupConfig = { 5., 10, 10.};

    NBLog::thread_info_t threadInfo{};
    NBLog::thread_params_t threadParams{};

    // Performance Data
    Histogram workHist{kWorkConfig};
    Histogram latencyHist{kLatencyConfig};
    Histogram warmupHist{kWarmupConfig};
    int64_t underruns = 0;
    static constexpr size_t kMaxSnapshotsToStore = 256;
    std::deque<std::pair<NBLog::Event, int64_t /*timestamp*/>> snapshots;
    int64_t overruns = 0;
    nsecs_t active = 0;
    nsecs_t start{systemTime()};

    // Reset the performance data. This does not represent a thread state change.
    // Thread info is not reset here because the data is meant to be a continuation of the thread
    // that struct PerformanceData is associated with.
    void reset() {
        workHist.clear();
        latencyHist.clear();
        warmupHist.clear();
        underruns = 0;
        overruns = 0;
        active = 0;
        start = systemTime();
    }

    // Return true if performance data has not been recorded yet, false otherwise.
    bool empty() const {
        return workHist.totalCount() == 0 && latencyHist.totalCount() == 0
                && warmupHist.totalCount() == 0 && underruns == 0 && overruns == 0
                && active == 0;
    }
};

//------------------------------------------------------------------------------

class PerformanceAnalysis;

// a map of PerformanceAnalysis instances
// The outer key is for the thread, the inner key for the source file location.
using PerformanceAnalysisMap = std::map<int, std::map<log_hash_t, PerformanceAnalysis>>;

class PerformanceAnalysis {
    // This class stores and analyzes audio processing wakeup timestamps from NBLog
    // FIXME: currently, all performance data is stored in deques. Turn these into circular
    // buffers.
    // TODO: add a mutex.
public:

    PerformanceAnalysis() {};

    friend void dump(int fd, int indent,
                     PerformanceAnalysisMap &threadPerformanceAnalysis);

    // Called in the case of an audio on/off event, e.g., EVENT_AUDIO_STATE.
    // Used to discard idle time intervals
    void handleStateChange();

    // Writes wakeup timestamp entry to log and runs analysis
    void logTsEntry(timestamp ts);

    // FIXME: make peakdetector and storeOutlierData a single function
    // Input: mOutlierData. Looks at time elapsed between outliers
    // finds significant changes in the distribution
    // writes timestamps of significant changes to mPeakTimestamps
    bool detectAndStorePeak(msInterval delta, timestamp ts);

    // stores timestamps of intervals above a threshold: these are assumed outliers.
    // writes to mOutlierData <time elapsed since previous outlier, outlier timestamp>
    bool detectAndStoreOutlier(const msInterval diffMs);

    // Generates a string of analysis of the buffer periods and prints to console
    // FIXME: move this data visualization to a separate class. Model/view/controller
    void reportPerformance(String8 *body, int author, log_hash_t hash,
                           int maxHeight = 10);

private:

    // TODO use a circular buffer for the deques and vectors below

    // stores outlier analysis:
    // <elapsed time between outliers in ms, outlier beginning timestamp>
    std::deque<std::pair<msInterval, timestamp>> mOutlierData;

    // stores each timestamp at which a peak was detected
    // a peak is a moment at which the average outlier interval changed significantly
    std::deque<timestamp> mPeakTimestamps;

    // stores buffer period histograms with timestamp of first sample
    std::deque<std::pair<timestamp, Hist>> mHists;

    // Parameters used when detecting outliers
    struct BufferPeriod {
        double    mMean = -1;          // average time between audio processing wakeups
        double    mOutlierFactor = -1; // values > mMean * mOutlierFactor are outliers
        double    mOutlier = -1;       // this is set to mMean * mOutlierFactor
        timestamp mPrevTs = -1;        // previous timestamp
    } mBufferPeriod;

    // capacity allocated to data structures
    struct MaxLength {
        size_t Hists; // number of histograms stored in memory
        size_t Outliers; // number of values stored in outlier array
        size_t Peaks; // number of values stored in peak array
        int HistTimespanMs; // maximum histogram timespan
    };
    // These values allow for 10 hours of data allowing for a glitch and a peak
    // as often as every 3 seconds
    static constexpr MaxLength kMaxLength = {.Hists = 60, .Outliers = 12000,
            .Peaks = 12000, .HistTimespanMs = 10 * kSecPerMin * kMsPerSec };

    // these variables ensure continuity while analyzing the timestamp
    // series one sample at a time.
    // TODO: change this to a running variance/mean class
    struct OutlierDistribution {
        msInterval mMean = 0;         // sample mean since previous peak
        msInterval mSd = 0;           // sample sd since previous peak
        msInterval mElapsed = 0;      // time since previous detected outlier
        const int  kMaxDeviation = 5; // standard deviations from the mean threshold
        msInterval mTypicalDiff = 0;  // global mean of outliers
        double     mN = 0;            // length of sequence since the last peak
        double     mM2 = 0;           // used to calculate sd
    } mOutlierDistribution;
};

void dump(int fd, int indent, PerformanceAnalysisMap &threadPerformanceAnalysis);
void dumpLine(int fd, int indent, const String8 &body);

}   // namespace ReportPerformance
}   // namespace android

#endif  // ANDROID_MEDIA_PERFORMANCEANALYSIS_H
