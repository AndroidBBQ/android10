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

#define LOG_TAG "ReportPerformance"
//#define LOG_NDEBUG 0

#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <sys/prctl.h>
#include <sys/time.h>
#include <utility>
#include <json/json.h>
#include <media/MediaAnalyticsItem.h>
#include <media/nblog/Events.h>
#include <media/nblog/PerformanceAnalysis.h>
#include <media/nblog/ReportPerformance.h>
#include <utils/Log.h>
#include <utils/String8.h>

namespace android {
namespace ReportPerformance {

static std::unique_ptr<Json::Value> dumpToJson(const PerformanceData& data)
{
    std::unique_ptr<Json::Value> rootPtr = std::make_unique<Json::Value>(Json::objectValue);
    Json::Value& root = *rootPtr;
    root["ioHandle"] = data.threadInfo.id;
    root["type"] = NBLog::threadTypeToString(data.threadInfo.type);
    root["frameCount"] = (Json::Value::Int)data.threadParams.frameCount;
    root["sampleRate"] = (Json::Value::Int)data.threadParams.sampleRate;
    root["workMsHist"] = data.workHist.toString();
    root["latencyMsHist"] = data.latencyHist.toString();
    root["warmupMsHist"] = data.warmupHist.toString();
    root["underruns"] = (Json::Value::Int64)data.underruns;
    root["overruns"] = (Json::Value::Int64)data.overruns;
    root["activeMs"] = (Json::Value::Int64)ns2ms(data.active);
    root["durationMs"] = (Json::Value::Int64)ns2ms(systemTime() - data.start);
    return rootPtr;
}

static std::string dumpHistogramsToString(const PerformanceData& data)
{
    std::stringstream ss;
    ss << "==========================================\n";
    ss << "Thread type=" << NBLog::threadTypeToString(data.threadInfo.type)
            << " handle=" << data.threadInfo.id
            << " sampleRate=" << data.threadParams.sampleRate
            << " frameCount=" << data.threadParams.frameCount << "\n";
    ss << "  Thread work times in ms:\n" << data.workHist.asciiArtString(4 /*indent*/);
    ss << "  Thread latencies in ms:\n" << data.latencyHist.asciiArtString(4 /*indent*/);
    ss << "  Thread warmup times in ms:\n" << data.warmupHist.asciiArtString(4 /*indent*/);
    return ss.str();
}

void dumpJson(int fd, const std::map<int, PerformanceData>& threadDataMap)
{
    if (fd < 0) {
        return;
    }

    Json::Value root(Json::arrayValue);
    for (const auto& item : threadDataMap) {
        const ReportPerformance::PerformanceData& data = item.second;
        // Skip threads that do not have performance data recorded yet.
        if (data.empty()) {
            continue;
        }
        std::unique_ptr<Json::Value> dataJson = ReportPerformance::dumpToJson(data);
        if (dataJson == nullptr) {
            continue;
        }
        (*dataJson)["threadNum"] = item.first;
        root.append(*dataJson);
    }
    Json::StyledWriter writer;
    std::string rootStr = writer.write(root);
    write(fd, rootStr.c_str(), rootStr.size());
}

void dumpPlots(int fd, const std::map<int, PerformanceData>& threadDataMap)
{
    if (fd < 0) {
        return;
    }

    for (const auto &item : threadDataMap) {
        const ReportPerformance::PerformanceData& data = item.second;
        if (data.empty()) {
            continue;
        }
        std::string hists = ReportPerformance::dumpHistogramsToString(data);
        write(fd, hists.c_str(), hists.size());
    }
}

static std::string dumpRetroString(const PerformanceData& data, int64_t now)
{
    std::stringstream ss;
    ss << NBLog::threadTypeToString(data.threadInfo.type) << "," << data.threadInfo.id << "\n";
    for (const auto &item : data.snapshots) {
        // TODO use an enum to string conversion method. One good idea:
        // https://stackoverflow.com/a/238157
        if (item.first == NBLog::EVENT_UNDERRUN) {
            ss << "EVENT_UNDERRUN,";
        } else if (item.first == NBLog::EVENT_OVERRUN) {
            ss << "EVENT_OVERRUN,";
        }
        ss << now - item.second << "\n";
    }
    ss << "\n";
    return ss.str();
}

void dumpRetro(int fd, const std::map<int, PerformanceData>& threadDataMap)
{
    if (fd < 0) {
        return;
    }

    const nsecs_t now = systemTime();
    for (const auto &item : threadDataMap) {
        const ReportPerformance::PerformanceData& data = item.second;
        if (data.snapshots.empty()) {
            continue;
        }
        const std::string retroStr = dumpRetroString(data, now);
        write(fd, retroStr.c_str(), retroStr.size());
    }
}

bool sendToMediaMetrics(const PerformanceData& data)
{
    // See documentation for these metrics here:
    // docs.google.com/document/d/11--6dyOXVOpacYQLZiaOY5QVtQjUyqNx2zT9cCzLKYE/edit?usp=sharing
    static constexpr char kThreadType[] = "android.media.audiothread.type";
    static constexpr char kThreadFrameCount[] = "android.media.audiothread.framecount";
    static constexpr char kThreadSampleRate[] = "android.media.audiothread.samplerate";
    static constexpr char kThreadWorkHist[] = "android.media.audiothread.workMs.hist";
    static constexpr char kThreadLatencyHist[] = "android.media.audiothread.latencyMs.hist";
    static constexpr char kThreadWarmupHist[] = "android.media.audiothread.warmupMs.hist";
    static constexpr char kThreadUnderruns[] = "android.media.audiothread.underruns";
    static constexpr char kThreadOverruns[] = "android.media.audiothread.overruns";
    static constexpr char kThreadActive[] = "android.media.audiothread.activeMs";
    static constexpr char kThreadDuration[] = "android.media.audiothread.durationMs";

    // Currently, we only allow FastMixer thread data to be sent to Media Metrics.
    if (data.threadInfo.type != NBLog::FASTMIXER) {
        return false;
    }

    std::unique_ptr<MediaAnalyticsItem> item(MediaAnalyticsItem::create("audiothread"));

    const Histogram &workHist = data.workHist;
    if (workHist.totalCount() > 0) {
        item->setCString(kThreadWorkHist, workHist.toString().c_str());
    }

    const Histogram &latencyHist = data.latencyHist;
    if (latencyHist.totalCount() > 0) {
        item->setCString(kThreadLatencyHist, latencyHist.toString().c_str());
    }

    const Histogram &warmupHist = data.warmupHist;
    if (warmupHist.totalCount() > 0) {
        item->setCString(kThreadWarmupHist, warmupHist.toString().c_str());
    }

    if (data.underruns > 0) {
        item->setInt64(kThreadUnderruns, data.underruns);
    }

    if (data.overruns > 0) {
        item->setInt64(kThreadOverruns, data.overruns);
    }

    // Send to Media Metrics if the record is not empty.
    // The thread and time info are added inside the if statement because
    // we want to send them only if there are performance metrics to send.
    if (item->count() > 0) {
        // Add thread info fields.
        const char * const typeString = NBLog::threadTypeToString(data.threadInfo.type);
        item->setCString(kThreadType, typeString);
        item->setInt32(kThreadFrameCount, data.threadParams.frameCount);
        item->setInt32(kThreadSampleRate, data.threadParams.sampleRate);
        // Add time info fields.
        item->setInt64(kThreadActive, data.active / 1000000);
        item->setInt64(kThreadDuration, (systemTime() - data.start) / 1000000);
        return item->selfrecord();
    }
    return false;
}

//------------------------------------------------------------------------------

// TODO: use a function like this to extract logic from writeToFile
// https://stackoverflow.com/a/9279620

// Writes outlier intervals, timestamps, and histograms spanning long time intervals to file.
// TODO: write data in binary format
void writeToFile(const std::deque<std::pair<timestamp, Hist>> &hists,
                 const std::deque<std::pair<msInterval, timestamp>> &outlierData,
                 const std::deque<timestamp> &peakTimestamps,
                 const char * directory, bool append, int author, log_hash_t hash) {

    // TODO: remove old files, implement rotating files as in AudioFlinger.cpp

    if (outlierData.empty() && hists.empty() && peakTimestamps.empty()) {
        ALOGW("No data, returning.");
        return;
    }

    std::stringstream outlierName;
    std::stringstream histogramName;
    std::stringstream peakName;

    // get current time
    char currTime[16]; //YYYYMMDDHHMMSS + '\0' + one unused
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm tm;
    localtime_r(&tv.tv_sec, &tm);
    strftime(currTime, sizeof(currTime), "%Y%m%d%H%M%S", &tm);

    // generate file names
    std::stringstream common;
    common << author << "_" << hash << "_" << currTime << ".csv";

    histogramName << directory << "histograms_" << common.str();
    outlierName << directory << "outliers_" << common.str();
    peakName << directory << "peaks_" << common.str();

    std::ofstream hfs;
    hfs.open(histogramName.str(), append ? std::ios::app : std::ios::trunc);
    if (!hfs.is_open()) {
        ALOGW("couldn't open file %s", histogramName.str().c_str());
        return;
    }
    // each histogram is written as a line where the first value is the timestamp and
    // subsequent values are pairs of buckets and counts. Each value is separated
    // by a comma, and each histogram is separated by a newline.
    for (auto hist = hists.begin(); hist != hists.end(); ++hist) {
        hfs << hist->first << ", ";
        for (auto bucket = hist->second.begin(); bucket != hist->second.end(); ++bucket) {
            hfs << bucket->first / static_cast<double>(kJiffyPerMs)
                << ", " << bucket->second;
            if (std::next(bucket) != end(hist->second)) {
                hfs << ", ";
            }
        }
        if (std::next(hist) != end(hists)) {
            hfs << "\n";
        }
    }
    hfs.close();

    std::ofstream ofs;
    ofs.open(outlierName.str(), append ? std::ios::app : std::ios::trunc);
    if (!ofs.is_open()) {
        ALOGW("couldn't open file %s", outlierName.str().c_str());
        return;
    }
    // outliers are written as pairs separated by newlines, where each
    // pair's values are separated by a comma
    for (const auto &outlier : outlierData) {
        ofs << outlier.first << ", " << outlier.second << "\n";
    }
    ofs.close();

    std::ofstream pfs;
    pfs.open(peakName.str(), append ? std::ios::app : std::ios::trunc);
    if (!pfs.is_open()) {
        ALOGW("couldn't open file %s", peakName.str().c_str());
        return;
    }
    // peaks are simply timestamps separated by commas
    for (auto peak = peakTimestamps.begin(); peak != peakTimestamps.end(); ++peak) {
        pfs << *peak;
        if (std::next(peak) != end(peakTimestamps)) {
            pfs << ", ";
        }
    }
    pfs.close();
}

}   // namespace ReportPerformance
}   // namespace android
