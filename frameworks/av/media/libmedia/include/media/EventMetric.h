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
#ifndef ANDROID_EVENT_METRIC_H_
#define ANDROID_EVENT_METRIC_H_

#include <media/MediaAnalyticsItem.h>
#include <utils/Timers.h>

namespace android {

// This is a simple holder for the statistics recorded in EventMetric.
struct EventStatistics {
  // The count of times the event occurred.
  int64_t count;

  // The minimum and maximum values recorded in the Record method.
  double min;
  double max;

  // The average (mean) of all values recorded.
  double mean;
  // The sum of squared devation. Variance can be calculated from
  // this value.
  //    var = sum_squared_deviation / count;
  double sum_squared_deviation;
};

// The EventMetric class is used to accumulate stats about an event over time.
// A common use case is to track clock timings for a method call or operation.
// An EventMetric can break down stats by a dimension specified by the
// application. E.g. an application may want to track counts broken out by
// error code or the size of some parameter.
//
// Example:
//
//   struct C {
//     status_t DoWork() {
//       unsigned long start_time = now();
//       status_t result;
//
//       // DO WORK and determine result;
//
//       work_event_.Record(now() - start_time, result);
//
//       return result;
//     }
//     EventMetric<status_t> work_event_;
//   };
//
//   C c;
//   c.DoWork();
//
//   std::map<int, int64_t> values;
//   metric.ExportValues(
//       [&] (int attribute_value, int64_t value) {
//            values[attribute_value] = value;
//       });
//   // Do something with the exported stat.
//
template<typename AttributeType>
class EventMetric {
 public:
  // Instantiate the counter with the given metric name and
  // attribute names. |attribute_names| must not be null.
  EventMetric(
      const std::string& metric_name,
      const std::string& attribute_name)
          : metric_name_(metric_name),
            attribute_name_(attribute_name) {}

  // Increment the count of times the operation occurred with this
  // combination of attributes.
  void Record(double value, AttributeType attribute) {
    if (values_.find(attribute) != values_.end()) {
      EventStatistics* stats = values_[attribute].get();
      // Using method of provisional means.
      double deviation = value - stats->mean;
      stats->mean = stats->mean + (deviation / stats->count);
      stats->sum_squared_deviation =
          stats->sum_squared_deviation + (deviation * (value - stats->mean));
      stats->count++;

      stats->min = stats->min < value ? stats->min : value;
      stats->max = stats->max > value ? stats->max : value;
    } else {
      std::unique_ptr<EventStatistics> stats =
          std::make_unique<EventStatistics>();
      stats->count = 1;
      stats->min = value;
      stats->max = value;
      stats->mean = value;
      stats->sum_squared_deviation = 0;
      values_[attribute] = std::move(stats);
    }
  };

  // Export the metrics to the provided |function|. Each value for Attribute
  // has a separate set of stats. As such, |function| will be called once per
  // value of Attribute.
  void ExportValues(
      std::function<void (const AttributeType&,
                          const EventStatistics&)> function) const {
    for (auto it = values_.begin(); it != values_.end(); it++) {
      function(it->first, *(it->second));
    }
  }

  const std::string& metric_name() const { return metric_name_; };

 private:
  const std::string metric_name_;
  const std::string attribute_name_;
  std::map<AttributeType, std::unique_ptr<struct EventStatistics>> values_;
};

// The EventTimer is a supporting class for EventMetric instances that are used
// to time methods. The EventTimer starts a timer when first in scope, and
// records the timing when exiting scope.
//
// Example:
//
// EventMetric<int> my_metric;
//
// {
//   EventTimer<int> my_timer(&my_metric);
//   // Set the attribute to associate with this timing.
//   my_timer.SetAttribtue(42);
//
//   // Do some work that you want to time.
//
// }  // The EventTimer destructor will record the the timing in my_metric;
//
template<typename AttributeType>
class EventTimer {
 public:
  explicit EventTimer(EventMetric<AttributeType>* metric)
      :start_time_(systemTime()), metric_(metric) {
  }

  virtual ~EventTimer() {
    if (metric_) {
      metric_->Record(ns2us(systemTime() - start_time_), attribute_);
    }
  }

  // Set the attribute to associate with this timing. E.g. this can be used to
  // record the return code from the work that was timed.
  void SetAttribute(const AttributeType& attribute) {
    attribute_ = attribute;
  }

 protected:
  // Visible for testing only.
  nsecs_t start_time_;

 private:
  EventMetric<AttributeType>* metric_;
  AttributeType attribute_;
};

}  // namespace android

#endif  // ANDROID_EVENT_METRIC_H_
