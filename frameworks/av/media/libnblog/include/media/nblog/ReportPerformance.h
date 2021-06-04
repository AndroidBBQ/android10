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

#ifndef ANDROID_MEDIA_REPORTPERFORMANCE_H
#define ANDROID_MEDIA_REPORTPERFORMANCE_H

#include <deque>
#include <map>
#include <vector>

namespace android {
namespace ReportPerformance {

struct PerformanceData;

// Dumps performance data in a JSON format.
void dumpJson(int fd, const std::map<int, PerformanceData>& threadDataMap);

//Dumps performance data as visualized plots.
void dumpPlots(int fd, const std::map<int, PerformanceData>& threadDataMap);

// Dumps snapshots at important events in the past.
void dumpRetro(int fd, const std::map<int, PerformanceData>& threadDataMap);

// Send one thread's data to media metrics, if the performance data is nontrivial (i.e. not
// all zero values). Return true if data was sent, false if there is nothing to write
// or an error occurred while writing.
bool sendToMediaMetrics(const PerformanceData& data);

//------------------------------------------------------------------------------

constexpr int kMsPerSec = 1000;
constexpr int kSecPerMin = 60;

constexpr int kJiffyPerMs = 10; // time unit for histogram as a multiple of milliseconds

// stores a histogram: key: observed buffer period (multiple of jiffy). value: count
using Hist = std::map<int, int>;

using msInterval = double;
using jiffyInterval = double;

using timestamp = int64_t;

using log_hash_t = uint64_t;

static inline int deltaMs(int64_t ns1, int64_t ns2) {
    return (ns2 - ns1) / (1000 * 1000);
}

static inline int deltaJiffy(int64_t ns1, int64_t ns2) {
    return (kJiffyPerMs * (ns2 - ns1)) / (1000 * 1000);
}

static inline uint32_t log2(uint32_t x) {
    // This works for x > 0
    return 31 - __builtin_clz(x);
}

// Writes outlier intervals, timestamps, peaks timestamps, and histograms to a file.
void writeToFile(const std::deque<std::pair<timestamp, Hist>> &hists,
                 const std::deque<std::pair<msInterval, timestamp>> &outlierData,
                 const std::deque<timestamp> &peakTimestamps,
                 const char * kDirectory, bool append, int author, log_hash_t hash);

}   // namespace ReportPerformance
}   // namespace android

#endif  // ANDROID_MEDIA_REPORTPERFORMANCE_H
