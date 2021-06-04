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
#define LOG_TAG "DrmMetrics"
#include <iomanip>
#include <utility>

#include <android-base/macros.h>
#include <media/stagefright/foundation/base64.h>
#include <mediadrm/DrmMetrics.h>
#include <sys/time.h>
#include <utils/Log.h>
#include <utils/Timers.h>

#include "protos/metrics.pb.h"

using ::android::String16;
using ::android::String8;
using ::android::drm_metrics::DrmFrameworkMetrics;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::drm::V1_0::EventType;
using ::android::hardware::drm::V1_2::KeyStatusType;
using ::android::hardware::drm::V1_1::DrmMetricGroup;
using ::android::os::PersistableBundle;

namespace {

template <typename T> std::string GetAttributeName(T type);

template <> std::string GetAttributeName<KeyStatusType>(KeyStatusType type) {
    static const char *type_names[] = {"USABLE", "EXPIRED",
                                       "OUTPUT_NOT_ALLOWED", "STATUS_PENDING",
                                       "INTERNAL_ERROR"};
    if (((size_t)type) > arraysize(type_names)) {
        return "UNKNOWN_TYPE";
    }
    return type_names[(size_t)type];
}

template <> std::string GetAttributeName<EventType>(EventType type) {
    static const char *type_names[] = {"PROVISION_REQUIRED", "KEY_NEEDED",
                                       "KEY_EXPIRED", "VENDOR_DEFINED",
                                       "SESSION_RECLAIMED"};
    if (((size_t)type) > arraysize(type_names)) {
        return "UNKNOWN_TYPE";
    }
    return type_names[(size_t)type];
}

template <typename T>
void ExportCounterMetric(const android::CounterMetric<T> &counter,
                         PersistableBundle *metrics) {
    if (!metrics) {
        ALOGE("metrics was unexpectedly null.");
        return;
    }
    std::string success_count_name = counter.metric_name() + ".ok.count";
    std::string error_count_name = counter.metric_name() + ".error.count";
    std::vector<int64_t> status_values;
    counter.ExportValues(
        [&](const android::status_t status, const int64_t value) {
            if (status == android::OK) {
                metrics->putLong(android::String16(success_count_name.c_str()),
                                 value);
            } else {
                int64_t total_errors(0);
                metrics->getLong(android::String16(error_count_name.c_str()),
                                 &total_errors);
                metrics->putLong(android::String16(error_count_name.c_str()),
                                 total_errors + value);
                status_values.push_back(status);
            }
        });
    if (!status_values.empty()) {
        std::string error_list_name = counter.metric_name() + ".error.list";
        metrics->putLongVector(android::String16(error_list_name.c_str()),
                               status_values);
    }
}

template <typename T>
void ExportCounterMetricWithAttributeNames(
    const android::CounterMetric<T> &counter, PersistableBundle *metrics) {
    if (!metrics) {
        ALOGE("metrics was unexpectedly null.");
        return;
    }
    counter.ExportValues([&](const T &attribute, const int64_t value) {
        std::string name = counter.metric_name() + "." +
                           GetAttributeName(attribute) + ".count";
        metrics->putLong(android::String16(name.c_str()), value);
    });
}

template <typename T>
void ExportEventMetric(const android::EventMetric<T> &event,
                       PersistableBundle *metrics) {
    if (!metrics) {
        ALOGE("metrics was unexpectedly null.");
        return;
    }
    std::string success_count_name = event.metric_name() + ".ok.count";
    std::string error_count_name = event.metric_name() + ".error.count";
    std::string timing_name = event.metric_name() + ".ok.average_time_micros";
    std::vector<int64_t> status_values;
    event.ExportValues([&](const android::status_t &status,
                           const android::EventStatistics &value) {
        if (status == android::OK) {
            metrics->putLong(android::String16(success_count_name.c_str()),
                             value.count);
            metrics->putLong(android::String16(timing_name.c_str()),
                             value.mean);
        } else {
            int64_t total_errors(0);
            metrics->getLong(android::String16(error_count_name.c_str()),
                             &total_errors);
            metrics->putLong(android::String16(error_count_name.c_str()),
                             total_errors + value.count);
            status_values.push_back(status);
        }
    });
    if (!status_values.empty()) {
        std::string error_list_name = event.metric_name() + ".error.list";
        metrics->putLongVector(android::String16(error_list_name.c_str()),
                               status_values);
    }
}

void ExportSessionLifespans(
    const std::map<std::string, std::pair<int64_t, int64_t>> &mSessionLifespans,
    PersistableBundle *metrics) {
    if (!metrics) {
        ALOGE("metrics was unexpectedly null.");
        return;
    }

    if (mSessionLifespans.empty()) {
        return;
    }

    PersistableBundle startTimesBundle;
    PersistableBundle endTimesBundle;
    for (auto it = mSessionLifespans.begin(); it != mSessionLifespans.end();
         it++) {
        String16 key(it->first.c_str(), it->first.size());
        startTimesBundle.putLong(key, it->second.first);
        endTimesBundle.putLong(key, it->second.second);
    }
    metrics->putPersistableBundle(
        android::String16("drm.mediadrm.session_start_times_ms"),
        startTimesBundle);
    metrics->putPersistableBundle(
        android::String16("drm.mediadrm.session_end_times_ms"), endTimesBundle);
}

std::string ToHexString(const android::Vector<uint8_t> &sessionId) {
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (size_t i = 0; i < sessionId.size(); i++) {
        out << std::setw(2) << (int)(sessionId[i]);
    }
    return out.str();
}

template <typename CT>
void SetValue(const String16 &name, DrmMetricGroup::ValueType type,
              const CT &value, PersistableBundle *bundle) {
    switch (type) {
    case DrmMetricGroup::ValueType::INT64_TYPE:
        bundle->putLong(name, value.int64Value);
        break;
    case DrmMetricGroup::ValueType::DOUBLE_TYPE:
        bundle->putDouble(name, value.doubleValue);
        break;
    case DrmMetricGroup::ValueType::STRING_TYPE:
        bundle->putString(name, String16(value.stringValue.c_str()));
        break;
    default:
        ALOGE("Unexpected value type: %hhu", type);
    }
}

inline String16 MakeIndexString(unsigned int index) {
  std::string str("[");
  str.append(std::to_string(index));
  str.append("]");
  return String16(str.c_str());
}

} // namespace

namespace android {

MediaDrmMetrics::MediaDrmMetrics()
    : mOpenSessionCounter("drm.mediadrm.open_session", "status"),
      mCloseSessionCounter("drm.mediadrm.close_session", "status"),
      mGetKeyRequestTimeUs("drm.mediadrm.get_key_request", "status"),
      mProvideKeyResponseTimeUs("drm.mediadrm.provide_key_response", "status"),
      mGetProvisionRequestCounter("drm.mediadrm.get_provision_request",
                                  "status"),
      mProvideProvisionResponseCounter(
          "drm.mediadrm.provide_provision_response", "status"),
      mKeyStatusChangeCounter("drm.mediadrm.key_status_change",
                              "key_status_type"),
      mEventCounter("drm.mediadrm.event", "event_type"),
      mGetDeviceUniqueIdCounter("drm.mediadrm.get_device_unique_id", "status") {
}

void MediaDrmMetrics::SetSessionStart(
    const android::Vector<uint8_t> &sessionId) {
    std::string sessionIdHex = ToHexString(sessionId);
    mSessionLifespans[sessionIdHex] =
        std::make_pair(GetCurrentTimeMs(), (int64_t)0);
}

void MediaDrmMetrics::SetSessionEnd(const android::Vector<uint8_t> &sessionId) {
    std::string sessionIdHex = ToHexString(sessionId);
    int64_t endTimeMs = GetCurrentTimeMs();
    if (mSessionLifespans.find(sessionIdHex) != mSessionLifespans.end()) {
        mSessionLifespans[sessionIdHex] =
            std::make_pair(mSessionLifespans[sessionIdHex].first, endTimeMs);
    } else {
        mSessionLifespans[sessionIdHex] = std::make_pair((int64_t)0, endTimeMs);
    }
}

void MediaDrmMetrics::Export(PersistableBundle *metrics) {
    if (!metrics) {
        ALOGE("metrics was unexpectedly null.");
        return;
    }
    ExportCounterMetric(mOpenSessionCounter, metrics);
    ExportCounterMetric(mCloseSessionCounter, metrics);
    ExportEventMetric(mGetKeyRequestTimeUs, metrics);
    ExportEventMetric(mProvideKeyResponseTimeUs, metrics);
    ExportCounterMetric(mGetProvisionRequestCounter, metrics);
    ExportCounterMetric(mProvideProvisionResponseCounter, metrics);
    ExportCounterMetricWithAttributeNames(mKeyStatusChangeCounter, metrics);
    ExportCounterMetricWithAttributeNames(mEventCounter, metrics);
    ExportCounterMetric(mGetDeviceUniqueIdCounter, metrics);
    ExportSessionLifespans(mSessionLifespans, metrics);
}

status_t MediaDrmMetrics::GetSerializedMetrics(std::string *serializedMetrics) {

    if (!serializedMetrics) {
        ALOGE("serializedMetrics was unexpectedly null.");
        return UNEXPECTED_NULL;
    }

    DrmFrameworkMetrics metrics;

    mOpenSessionCounter.ExportValues(
        [&](const android::status_t status, const int64_t value) {
            DrmFrameworkMetrics::Counter *counter =
                metrics.add_open_session_counter();
            counter->set_count(value);
            counter->mutable_attributes()->set_error_code(status);
        });

    mCloseSessionCounter.ExportValues(
        [&](const android::status_t status, const int64_t value) {
            DrmFrameworkMetrics::Counter *counter =
                metrics.add_close_session_counter();
            counter->set_count(value);
            counter->mutable_attributes()->set_error_code(status);
        });

    mGetProvisionRequestCounter.ExportValues(
        [&](const android::status_t status, const int64_t value) {
            DrmFrameworkMetrics::Counter *counter =
                metrics.add_get_provisioning_request_counter();
            counter->set_count(value);
            counter->mutable_attributes()->set_error_code(status);
        });

    mProvideProvisionResponseCounter.ExportValues(
        [&](const android::status_t status, const int64_t value) {
            DrmFrameworkMetrics::Counter *counter =
                metrics.add_provide_provisioning_response_counter();
            counter->set_count(value);
            counter->mutable_attributes()->set_error_code(status);
        });

    mKeyStatusChangeCounter.ExportValues(
        [&](const KeyStatusType key_status_type, const int64_t value) {
            DrmFrameworkMetrics::Counter *counter =
                metrics.add_key_status_change_counter();
            counter->set_count(value);
            counter->mutable_attributes()->set_key_status_type(
                (uint32_t)key_status_type);
        });

    mEventCounter.ExportValues(
        [&](const EventType event_type, const int64_t value) {
            DrmFrameworkMetrics::Counter *counter =
                metrics.add_event_callback_counter();
            counter->set_count(value);
            counter->mutable_attributes()->set_event_type((uint32_t)event_type);
        });

    mGetDeviceUniqueIdCounter.ExportValues(
        [&](const status_t status, const int64_t value) {
            DrmFrameworkMetrics::Counter *counter =
                metrics.add_get_device_unique_id_counter();
            counter->set_count(value);
            counter->mutable_attributes()->set_error_code(status);
        });

    mGetKeyRequestTimeUs.ExportValues(
        [&](const status_t status, const EventStatistics &stats) {
            DrmFrameworkMetrics::DistributionMetric *metric =
                metrics.add_get_key_request_time_us();
            metric->set_min(stats.min);
            metric->set_max(stats.max);
            metric->set_mean(stats.mean);
            metric->set_operation_count(stats.count);
            metric->set_variance(stats.sum_squared_deviation / stats.count);
            metric->mutable_attributes()->set_error_code(status);
        });

    mProvideKeyResponseTimeUs.ExportValues(
        [&](const status_t status, const EventStatistics &stats) {
            DrmFrameworkMetrics::DistributionMetric *metric =
                metrics.add_provide_key_response_time_us();
            metric->set_min(stats.min);
            metric->set_max(stats.max);
            metric->set_mean(stats.mean);
            metric->set_operation_count(stats.count);
            metric->set_variance(stats.sum_squared_deviation / stats.count);
            metric->mutable_attributes()->set_error_code(status);
        });

    for (const auto &sessionLifespan : mSessionLifespans) {
        auto *map = metrics.mutable_session_lifetimes();

        (*map)[sessionLifespan.first].set_start_time_ms(
            sessionLifespan.second.first);
        (*map)[sessionLifespan.first].set_end_time_ms(
            sessionLifespan.second.second);
    }

    if (!metrics.SerializeToString(serializedMetrics)) {
        ALOGE("Failed to serialize metrics.");
        return UNKNOWN_ERROR;
    }

    return OK;
}

int64_t MediaDrmMetrics::GetCurrentTimeMs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((int64_t)tv.tv_sec * 1000) + ((int64_t)tv.tv_usec / 1000);
}

status_t MediaDrmMetrics::HidlMetricsToBundle(
    const hidl_vec<DrmMetricGroup> &hidlMetricGroups,
    PersistableBundle *bundleMetricGroups) {
    if (bundleMetricGroups == nullptr) {
        return UNEXPECTED_NULL;
    }
    if (hidlMetricGroups.size() == 0) {
        return OK;
    }

    int groupIndex = 0;
    std::map<String16, int> indexMap;
    for (const auto &hidlMetricGroup : hidlMetricGroups) {
        PersistableBundle bundleMetricGroup;
        for (const auto &hidlMetric : hidlMetricGroup.metrics) {
            String16 metricName(hidlMetric.name.c_str());
            PersistableBundle bundleMetric;
            // Add metric component values.
            for (const auto &value : hidlMetric.values) {
                SetValue(String16(value.componentName.c_str()), value.type,
                         value, &bundleMetric);
            }
            // Set metric attributes.
            PersistableBundle bundleMetricAttributes;
            for (const auto &attribute : hidlMetric.attributes) {
                SetValue(String16(attribute.name.c_str()), attribute.type,
                         attribute, &bundleMetricAttributes);
            }
            // Add attributes to the bundle metric.
            bundleMetric.putPersistableBundle(String16("attributes"),
                                              bundleMetricAttributes);
            // Add one layer of indirection, allowing for repeated metric names.
            PersistableBundle repeatedMetrics;
            bundleMetricGroup.getPersistableBundle(metricName,
                                                   &repeatedMetrics);
            int index = indexMap[metricName];
            repeatedMetrics.putPersistableBundle(MakeIndexString(index),
                                                 bundleMetric);
            indexMap[metricName] = ++index;

            // Add the bundle metric to the group of metrics.
            bundleMetricGroup.putPersistableBundle(metricName,
                                                   repeatedMetrics);
        }
        // Add the bundle metric group to the collection of groups.
        bundleMetricGroups->putPersistableBundle(MakeIndexString(groupIndex++),
                                                 bundleMetricGroup);
    }

    return OK;
}

} // namespace android
