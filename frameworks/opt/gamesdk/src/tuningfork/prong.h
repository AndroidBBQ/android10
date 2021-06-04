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

#pragma once

#include "tuningfork_internal.h"
#include "histogram.h"

#include <inttypes.h>
#include <vector>
#include <map>
#include <string>
#include <memory>

namespace tuningfork {

typedef ProtobufSerialization SerializedAnnotation;

// A prong holds a histogram for a given annotation and instrument key
class Prong {
public:
    InstrumentationKey instrumentation_key_;
    SerializedAnnotation annotation_;
    Histogram histogram_;
    TimePoint last_time_ns_;

    Prong(InstrumentationKey instrumentation_key = 0,
          const SerializedAnnotation &annotation = {},
          const Settings::Histogram& histogram_settings = {})
        : instrumentation_key_(instrumentation_key), annotation_(annotation),
          last_time_ns_(std::chrono::steady_clock::time_point::min()),
          histogram_(histogram_settings) {}

    void Tick(TimePoint t_ns) {
        if (last_time_ns_ != std::chrono::steady_clock::time_point::min())
            Trace(t_ns - last_time_ns_);
        last_time_ns_ = t_ns;
    }

    void Trace(Duration dt_ns) {
        // The histogram stores millisecond values as doubles
        histogram_.Add(
            double(std::chrono::duration_cast<std::chrono::nanoseconds>(dt_ns).count()) / 1000000);
    }

    void Clear() {
        last_time_ns_ = std::chrono::steady_clock::time_point::min();
        histogram_.Clear();
    }

    size_t Count() const {
        return histogram_.Count();
    }

    friend class ClearcutSerializer;
};

// Simple fixed-size cache
class ProngCache {
    std::vector<std::unique_ptr<Prong>> prongs_;
public:
    ProngCache(size_t size, int max_num_instrumentation_keys,
               const std::vector<Settings::Histogram>& histogram_settings,
               const std::function<SerializedAnnotation(uint64_t)>& seralizeId);

    Prong *Get(uint64_t compound_id);

    void Clear();

    friend class ClearcutSerializer;

};

} // namespace tuningfork {
