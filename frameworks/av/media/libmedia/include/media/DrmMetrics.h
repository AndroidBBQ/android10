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

#ifndef DRM_METRICS_H_
#define DRM_METRICS_H_

#include <map>

#include <android/hardware/drm/1.0/types.h>
#include <android/hardware/drm/1.1/types.h>
#include <android/hardware/drm/1.2/types.h>
#include <binder/PersistableBundle.h>
#include <media/CounterMetric.h>
#include <media/EventMetric.h>

namespace android {

/**
 * This class contains the definition of metrics captured within MediaDrm.
 * It also contains a method for exporting all of the metrics to a
 * PersistableBundle.
 */
class MediaDrmMetrics {
 public:
  explicit MediaDrmMetrics();
  virtual ~MediaDrmMetrics() {};
  // Count of openSession calls.
  CounterMetric<status_t> mOpenSessionCounter;
  // Count of closeSession calls.
  CounterMetric<status_t> mCloseSessionCounter;
  // Count and timing of getKeyRequest calls.
  EventMetric<status_t> mGetKeyRequestTimeUs;
  // Count and timing of provideKeyResponse calls.
  EventMetric<status_t> mProvideKeyResponseTimeUs;
  // Count of getProvisionRequest calls.
  CounterMetric<status_t> mGetProvisionRequestCounter;
  // Count of provideProvisionResponse calls.
  CounterMetric<status_t> mProvideProvisionResponseCounter;

  // Count of key status events broken out by status type.
  CounterMetric<::android::hardware::drm::V1_2::KeyStatusType>
      mKeyStatusChangeCounter;
  // Count of events broken out by event type
  CounterMetric<::android::hardware::drm::V1_0::EventType> mEventCounter;

  // Count getPropertyByteArray calls to retrieve the device unique id.
  CounterMetric<status_t> mGetDeviceUniqueIdCounter;

  // Adds a session start time record.
  void SetSessionStart(const Vector<uint8_t>& sessionId);

  // Adds a session end time record.
  void SetSessionEnd(const Vector<uint8_t>& sessionId);

  // The app package name is the application package name that is using the
  // instance. The app package name is held here for convenience. It is not
  // serialized or exported with the metrics.
  void SetAppPackageName(const String8& appPackageName) { mAppPackageName = appPackageName; }
  const String8& GetAppPackageName() { return mAppPackageName; }

  // Export the metrics to a PersistableBundle.
  void Export(os::PersistableBundle* metricsBundle);

  // Get the serialized metrics. Metrics are formatted as a serialized
  // DrmFrameworkMetrics proto. If there is a failure serializing the metrics,
  // this returns an error. The parameter |serlializedMetrics| is owned by the
  // caller and must not be null.
  status_t GetSerializedMetrics(std::string* serializedMetrics);

  // Converts the DRM plugin metrics to a PersistableBundle. All of the metrics
  // found in |pluginMetrics| are added to the |metricsBundle| parameter.
  // |pluginBundle| is owned by the caller and must not be null.
  //
  // Each item in the pluginMetrics vector is added as a new PersistableBundle. E.g.
  // DrmMetricGroup {
  //   metrics[0] {
  //     name: "buf_copy"
  //     attributes[0] {
  //       name: "size"
  //       type: INT64_TYPE
  //       int64Value: 1024
  //     }
  //     values[0] {
  //       componentName: "operation_count"
  //       type: INT64_TYPE
  //       int64Value: 75
  //     }
  //     values[1] {
  //       component_name: "average_time_seconds"
  //       type: DOUBLE_TYPE
  //       doubleValue: 0.00000042
  //     }
  //   }
  // }
  //
  // becomes
  //
  // metricsBundle {
  //   "0": (PersistableBundle) {
  //     "attributes" : (PersistableBundle) {
  //       "size" : (int64) 1024
  //     }
  //     "operation_count" : (int64) 75
  //     "average_time_seconds" : (double) 0.00000042
  //   }
  //
  static status_t HidlMetricsToBundle(
          const hardware::hidl_vec<hardware::drm::V1_1::DrmMetricGroup>& pluginMetrics,
          os::PersistableBundle* metricsBundle);

 protected:
  // This is visible for testing only.
  virtual int64_t GetCurrentTimeMs();

 private:
  // Session lifetimes. A pair of values representing the milliseconds since
  // epoch, UTC. The first value is the start time, the second is the end time.
  std::map<std::string, std::pair<int64_t, int64_t>> mSessionLifespans;

  String8 mAppPackageName;
};

}  // namespace android

#endif  // DRM_METRICS_H_
