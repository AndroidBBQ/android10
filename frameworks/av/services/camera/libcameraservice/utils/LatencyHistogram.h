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

#ifndef ANDROID_SERVERS_CAMERA_LATENCY_HISTOGRAM_H_
#define ANDROID_SERVERS_CAMERA_LATENCY_HISTOGRAM_H_

#include <vector>

#include <utils/Timers.h>
#include <utils/Mutex.h>

namespace android {

// Histogram for camera latency characteristic
class CameraLatencyHistogram {
public:
    CameraLatencyHistogram() = delete;
    CameraLatencyHistogram(int32_t binSizeMs, int32_t binCount=10);
    void add(nsecs_t start, nsecs_t end);
    void reset();

    void dump(int fd, const char* name) const;
    void log(const char* format, ...);
private:
    int32_t mBinSizeMs;
    int32_t mBinCount;
    std::vector<int64_t> mBins;
    uint64_t mTotalCount;

    void formatHistogramText(String8& lineBins, String8& lineBinCounts) const;
}; // class CameraLatencyHistogram

}; // namespace android

#endif // ANDROID_SERVERS_CAMERA_LATENCY_HISTOGRAM_H_
