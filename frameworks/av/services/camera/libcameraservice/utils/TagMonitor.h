/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef ANDROID_SERVERS_CAMERA_TAGMONITOR_H
#define ANDROID_SERVERS_CAMERA_TAGMONITOR_H

#include <vector>
#include <atomic>
#include <mutex>
#include <unordered_map>

#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/Timers.h>

#include <media/RingBuffer.h>
#include <system/camera_metadata.h>
#include <system/camera_vendor_tags.h>
#include <camera/CameraMetadata.h>

namespace android {

/**
 * A monitor for camera metadata values.
 * Tracks changes to specified metadata values over time, keeping a circular
 * buffer log that can be dumped at will. */
class TagMonitor {
  public:

    // Monitor argument
    static const String16 kMonitorOption;

    enum eventSource {
        REQUEST,
        RESULT
    };

    TagMonitor();

    void initialize(metadata_vendor_id_t id) { mVendorTagId = id; }

    // Parse tag name list (comma-separated) and if valid, enable monitoring
    // If invalid, do nothing.
    // Recognizes "3a" as a shortcut for enabling tracking 3A state, mode, and
    // triggers
    void parseTagsToMonitor(String8 tagNames);

    // Disable monitoring; does not clear the event log
    void disableMonitoring();

    // Scan through the metadata and update the monitoring information
    void monitorMetadata(eventSource source, int64_t frameNumber,
            nsecs_t timestamp, const CameraMetadata& metadata,
            const std::unordered_map<std::string, CameraMetadata>& physicalMetadata);

    // Dump current event log to the provided fd
    void dumpMonitoredMetadata(int fd);

  private:

    static void printData(int fd, const uint8_t *data_ptr, uint32_t tag,
            int type, int count, int indentation);

    void monitorSingleMetadata(TagMonitor::eventSource source, int64_t frameNumber,
            nsecs_t timestamp, const std::string& cameraId, uint32_t tag,
            const CameraMetadata& metadata);

    std::atomic<bool> mMonitoringEnabled;
    std::mutex mMonitorMutex;

    // Current tags to monitor and record changes to
    std::vector<uint32_t> mMonitoredTagList;

    // Latest-seen values of tracked tags
    CameraMetadata mLastMonitoredRequestValues;
    CameraMetadata mLastMonitoredResultValues;

    std::unordered_map<std::string, CameraMetadata> mLastMonitoredPhysicalRequestKeys;
    std::unordered_map<std::string, CameraMetadata> mLastMonitoredPhysicalResultKeys;

    /**
     * A monitoring event
     * Stores a new metadata field value and the timestamp at which it changed.
     * Copies the source metadata value array and frees it on destruct.
     */
    struct MonitorEvent {
        template<typename T>
        MonitorEvent(eventSource src, uint32_t frameNumber, nsecs_t timestamp,
                const T &newValue, const std::string& cameraId);
        ~MonitorEvent();

        eventSource source;
        uint32_t frameNumber;
        nsecs_t timestamp;
        uint32_t tag;
        uint8_t type;
        std::vector<uint8_t> newData;
        std::string cameraId;
    };

    // A ring buffer for tracking the last kMaxMonitorEvents metadata changes
    static const int kMaxMonitorEvents = 100;
    RingBuffer<MonitorEvent> mMonitoringEvents;

    // 3A fields to use with the "3a" option
    static const char *k3aTags;
    metadata_vendor_id_t mVendorTagId;
};

} // namespace android

#endif
