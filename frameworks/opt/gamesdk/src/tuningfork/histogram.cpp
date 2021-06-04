/*
 * Copyright 2018 The Android Open Source Project
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

#include "histogram.h"

#include <sstream>
#include <cmath>

#define LOG_TAG "TuningFork"
#include "Log.h"

#include "clearcutserializer.h"

namespace tuningfork {

Histogram::Histogram(float start_ms, float end_ms, int num_buckets_between)
    : start_ms_(start_ms), end_ms_(end_ms),
      bucket_dt_ms_((end_ms_ - start_ms_) / num_buckets_between),
      num_buckets_(num_buckets_between + 2),
      buckets_(num_buckets_), auto_range_(start_ms_ == 0 && end_ms_ == 0), count_(0) {
    std::fill(buckets_.begin(), buckets_.end(), 0);
    if (auto_range_)
        samples_.reserve(num_buckets_);
    else if (bucket_dt_ms_ <= 0)
        ALOGE("Histogram end needs to be larger than histogram begin");
}

Histogram::Histogram(const Settings::Histogram &hs)
    : Histogram(hs.bucket_min, hs.bucket_max, hs.n_buckets) {
}

void Histogram::Add(Sample dt_ms) {
    if (auto_range_) {
        samples_.push_back(dt_ms);
        if (samples_.size() == samples_.capacity()) {
            CalcBucketsFromSamples();
        }
    } else {
        int i = (dt_ms - start_ms_) / bucket_dt_ms_;
        if (i < 0)
            buckets_[0]++;
        else if (i + 1 >= num_buckets_)
            buckets_[num_buckets_ - 1]++;
        else
            buckets_[i + 1]++;
        ++count_;
    }
}

void Histogram::CalcBucketsFromSamples() {
    Sample min_dt = std::numeric_limits<Sample>::max();
    Sample max_dt = std::numeric_limits<Sample>::min();
    Sample sum = 0;
    Sample sum2 = 0;
    for (Sample d: samples_) {
        if (d < min_dt) min_dt = d;
        if (d > max_dt) max_dt = d;
        sum += d;
        sum2 += d * d;
    }
    size_t n = samples_.size();
    Sample mean = sum / n;
    Sample var = sum2 / n - mean * mean;
    if (var < 0) var = 0; // Can be negative due to rounding errors
    Sample stddev = sqrt(var);
    start_ms_ = std::max(mean - kAutoSizeNumStdDev * stddev, 0.0);
    end_ms_ = mean + kAutoSizeNumStdDev * stddev;
    bucket_dt_ms_ = (end_ms_ - start_ms_) / (num_buckets_ - 2);
    if (bucket_dt_ms_ < kAutoSizeMinBucketSizeMs) {
        bucket_dt_ms_ = kAutoSizeMinBucketSizeMs;
        Sample w = bucket_dt_ms_ * (num_buckets_ - 2);
        start_ms_ = mean - w / 2;
        end_ms_ = mean + w / 2;
    }
    auto_range_ = false;
    for (Sample d: samples_) {
        Add(d);
    }
}

std::string Histogram::ToJSON() const {
    std::stringstream str;
    str.precision(2);
    str << std::fixed;
    if (auto_range_) {
        str << "{\"pmax\":[],\"cnts\":[]}";
    } else {
        str << "{\"pmax\":[";
        Sample x = start_ms_;
        for (int i = 0; i < num_buckets_ - 1; ++i) {
            str << x << ",";
            x += bucket_dt_ms_;
        }
        str << "99999],\"cnts\":[";
        for (int i = 0; i < num_buckets_ - 1; ++i) {
            str << buckets_[i] << ",";
        }
        if (num_buckets_ > 0)
            str << buckets_.back();
        str << "]}";
    }
    return str.str();
}

void Histogram::Clear(bool autorange) {
    std::fill(buckets_.begin(), buckets_.end(), 0);
    samples_.clear();
    auto_range_ = autorange;
    count_ = 0;
}

} // namespace tuningfork
