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
#ifndef ANDROID_COUNTER_METRIC_H_
#define ANDROID_COUNTER_METRIC_H_

#include <functional>
#include <map>
#include <string>

#include <media/MediaAnalyticsItem.h>
#include <utils/Log.h>

namespace android {


// The CounterMetric class is used to hold counts of operations or events.
// A CounterMetric can break down counts by a dimension specified by the
// application. E.g. an application may want to track counts broken out by
// error code or the size of some parameter.
//
// Example:
//
//   CounterMetric<status_t> workCounter;
//   workCounter("workCounterName", "result_status");
//
//   status_t err = DoWork();
//
//   // Increments the number of times called with the given error code.
//   workCounter.Increment(err);
//
//   std::map<int, int64_t> values;
//    metric.ExportValues(
//        [&] (int attribute_value, int64_t value) {
//             values[attribute_value] = value;
//        });
//
//   // Do something with the exported stat.
//
template<typename AttributeType>
class CounterMetric {
 public:
  // Instantiate the counter with the given metric name and
  // attribute names. |attribute_names| must not be null.
  CounterMetric(
      const std::string& metric_name,
      const std::string& attribute_name)
          : metric_name_(metric_name),
            attribute_name_(attribute_name) {}

  // Increment the count of times the operation occurred with this
  // combination of attributes.
  void Increment(AttributeType attribute) {
    if (values_.find(attribute) == values_.end()) {
      values_[attribute] = 1;
    } else {
      values_[attribute] = values_[attribute] + 1;
    }
  };

  // Export the metrics to the provided |function|. Each value for Attribute
  // has a separate count. As such, |function| will be called once per value
  // of Attribute.
  void ExportValues(
      std::function<void (const AttributeType&,
                          const int64_t count)> function) const {
    for (auto it = values_.begin(); it != values_.end(); it++) {
      function(it->first, it->second);
    }
  }

  const std::string& metric_name() const { return metric_name_; };

 private:
  const std::string metric_name_;
  const std::string attribute_name_;
  std::map<AttributeType, int64_t> values_;
};

}  // namespace android

#endif  // ANDROID_COUNTER_METRIC_H_
