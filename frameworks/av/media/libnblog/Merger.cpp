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

#define LOG_TAG "NBLog"
//#define LOG_NDEBUG 0

#include <memory>
#include <queue>
#include <stddef.h>
#include <stdint.h>
#include <vector>

#include <audio_utils/fifo.h>
#include <json/json.h>
#include <media/nblog/Merger.h>
#include <media/nblog/PerformanceAnalysis.h>
#include <media/nblog/ReportPerformance.h>
#include <media/nblog/Reader.h>
#include <media/nblog/Timeline.h>
#include <utils/Condition.h>
#include <utils/Log.h>
#include <utils/Mutex.h>
#include <utils/RefBase.h>
#include <utils/String16.h>
#include <utils/String8.h>
#include <utils/Thread.h>
#include <utils/Timers.h>
#include <utils/Vector.h>

namespace android {
namespace NBLog {

Merger::Merger(const void *shared, size_t size):
      mShared((Shared *) shared),
      mFifo(mShared != NULL ?
        new audio_utils_fifo(size, sizeof(uint8_t),
            mShared->mBuffer, mShared->mRear, NULL /*throttlesFront*/) : NULL),
      mFifoWriter(mFifo != NULL ? new audio_utils_fifo_writer(*mFifo) : NULL)
{
}

void Merger::addReader(const sp<Reader> &reader)
{
    // FIXME This is called by binder thread in MediaLogService::registerWriter
    //       but the access to shared variable mReaders is not yet protected by a lock.
    mReaders.push_back(reader);
}

// items placed in priority queue during merge
// composed by a timestamp and the index of the snapshot where the timestamp came from
struct MergeItem
{
    int64_t ts;
    int index;
    MergeItem(int64_t ts, int index): ts(ts), index(index) {}
};

bool operator>(const struct MergeItem &i1, const struct MergeItem &i2)
{
    return i1.ts > i2.ts || (i1.ts == i2.ts && i1.index > i2.index);
}

// Merge registered readers, sorted by timestamp, and write data to a single FIFO in local memory
void Merger::merge()
{
    if (true) return; // Merging is not necessary at the moment, so this is to disable it
                      // and bypass compiler warnings about member variables not being used.
    const int nLogs = mReaders.size();
    std::vector<std::unique_ptr<Snapshot>> snapshots(nLogs);
    std::vector<EntryIterator> offsets;
    offsets.reserve(nLogs);
    for (int i = 0; i < nLogs; ++i) {
        snapshots[i] = mReaders[i]->getSnapshot();
        offsets.push_back(snapshots[i]->begin());
    }
    // initialize offsets
    // TODO custom heap implementation could allow to update top, improving performance
    // for bursty buffers
    std::priority_queue<MergeItem, std::vector<MergeItem>, std::greater<MergeItem>> timestamps;
    for (int i = 0; i < nLogs; ++i)
    {
        if (offsets[i] != snapshots[i]->end()) {
            std::unique_ptr<AbstractEntry> abstractEntry = AbstractEntry::buildEntry(offsets[i]);
            if (abstractEntry == nullptr) {
                continue;
            }
            timestamps.emplace(abstractEntry->timestamp(), i);
        }
    }

    while (!timestamps.empty()) {
        int index = timestamps.top().index;     // find minimum timestamp
        // copy it to the log, increasing offset
        offsets[index] = AbstractEntry::buildEntry(offsets[index])->
            copyWithAuthor(mFifoWriter, index);
        // update data structures
        timestamps.pop();
        if (offsets[index] != snapshots[index]->end()) {
            int64_t ts = AbstractEntry::buildEntry(offsets[index])->timestamp();
            timestamps.emplace(ts, index);
        }
    }
}

const std::vector<sp<Reader>>& Merger::getReaders() const
{
    //AutoMutex _l(mLock);
    return mReaders;
}

// ---------------------------------------------------------------------------

MergeReader::MergeReader(const void *shared, size_t size, Merger &merger)
    : Reader(shared, size, "MergeReader"), mReaders(merger.getReaders())
{
}

// Takes raw content of the local merger FIFO, processes log entries, and
// writes the data to a map of class PerformanceAnalysis, based on their thread ID.
void MergeReader::processSnapshot(Snapshot &snapshot, int author)
{
    ReportPerformance::PerformanceData& data = mThreadPerformanceData[author];
    // We don't do "auto it" because it reduces readability in this case.
    for (EntryIterator it = snapshot.begin(); it != snapshot.end(); ++it) {
        switch (it->type) {
        case EVENT_HISTOGRAM_ENTRY_TS: {
            const HistTsEntry payload = it.payload<HistTsEntry>();
            // TODO: hash for histogram ts and audio state need to match
            // and correspond to audio production source file location
            mThreadPerformanceAnalysis[author][0 /*hash*/].logTsEntry(payload.ts);
        } break;
        case EVENT_AUDIO_STATE: {
            mThreadPerformanceAnalysis[author][0 /*hash*/].handleStateChange();
        } break;
        case EVENT_THREAD_INFO: {
            const thread_info_t info = it.payload<thread_info_t>();
            data.threadInfo = info;
        } break;
        case EVENT_THREAD_PARAMS: {
            const thread_params_t params = it.payload<thread_params_t>();
            data.threadParams = params;
        } break;
        case EVENT_LATENCY: {
            const double latencyMs = it.payload<double>();
            data.latencyHist.add(latencyMs);
        } break;
        case EVENT_WORK_TIME: {
            const int64_t monotonicNs = it.payload<int64_t>();
            const double monotonicMs = monotonicNs * 1e-6;
            data.workHist.add(monotonicMs);
            data.active += monotonicNs;
        } break;
        case EVENT_WARMUP_TIME: {
            const double timeMs = it.payload<double>();
            data.warmupHist.add(timeMs);
        } break;
        case EVENT_UNDERRUN: {
            const int64_t ts = it.payload<int64_t>();
            data.underruns++;
            data.snapshots.emplace_front(EVENT_UNDERRUN, ts);
            // TODO have a data structure to automatically handle resizing
            if (data.snapshots.size() > ReportPerformance::PerformanceData::kMaxSnapshotsToStore) {
                data.snapshots.pop_back();
            }
        } break;
        case EVENT_OVERRUN: {
            const int64_t ts = it.payload<int64_t>();
            data.overruns++;
            data.snapshots.emplace_front(EVENT_UNDERRUN, ts);
            // TODO have a data structure to automatically handle resizing
            if (data.snapshots.size() > ReportPerformance::PerformanceData::kMaxSnapshotsToStore) {
                data.snapshots.pop_back();
            }
        } break;
        case EVENT_RESERVED:
        case EVENT_UPPER_BOUND:
            ALOGW("warning: unexpected event %d", it->type);
            break;
        default:
            break;
        }
    }
}

void MergeReader::getAndProcessSnapshot()
{
    // get a snapshot of each reader and process them
    // TODO insert lock here
    const size_t nLogs = mReaders.size();
    std::vector<std::unique_ptr<Snapshot>> snapshots(nLogs);
    for (size_t i = 0; i < nLogs; i++) {
        snapshots[i] = mReaders[i]->getSnapshot();
    }
    // TODO unlock lock here
    for (size_t i = 0; i < nLogs; i++) {
        if (snapshots[i] != nullptr) {
            processSnapshot(*(snapshots[i]), i);
        }
    }
    checkPushToMediaMetrics();
}

void MergeReader::checkPushToMediaMetrics()
{
    const nsecs_t now = systemTime();
    for (auto& item : mThreadPerformanceData) {
        ReportPerformance::PerformanceData& data = item.second;
        if (now - data.start >= kPeriodicMediaMetricsPush) {
            (void)ReportPerformance::sendToMediaMetrics(data);
            data.reset();   // data is persistent per thread
        }
    }
}

void MergeReader::dump(int fd, const Vector<String16>& args)
{
    // TODO: add a mutex around media.log dump
    // Options for dumpsys
    bool pa = false, json = false, plots = false, retro = false;
    for (const auto &arg : args) {
        if (arg == String16("--pa")) {
            pa = true;
        } else if (arg == String16("--json")) {
            json = true;
        } else if (arg == String16("--plots")) {
            plots = true;
        } else if (arg == String16("--retro")) {
            retro = true;
        }
    }
    if (pa) {
        ReportPerformance::dump(fd, 0 /*indent*/, mThreadPerformanceAnalysis);
    }
    if (json) {
        ReportPerformance::dumpJson(fd, mThreadPerformanceData);
    }
    if (plots) {
        ReportPerformance::dumpPlots(fd, mThreadPerformanceData);
    }
    if (retro) {
        ReportPerformance::dumpRetro(fd, mThreadPerformanceData);
    }
}

void MergeReader::handleAuthor(const AbstractEntry &entry, String8 *body)
{
    int author = entry.author();
    if (author == -1) {
        return;
    }
    // FIXME Needs a lock
    const char* name = mReaders[author]->name().c_str();
    body->appendFormat("%s: ", name);
}

// ---------------------------------------------------------------------------

MergeThread::MergeThread(Merger &merger, MergeReader &mergeReader)
    : mMerger(merger),
      mMergeReader(mergeReader),
      mTimeoutUs(0)
{
}

MergeThread::~MergeThread()
{
    // set exit flag, set timeout to 0 to force threadLoop to exit and wait for the thread to join
    requestExit();
    setTimeoutUs(0);
    join();
}

bool MergeThread::threadLoop()
{
    bool doMerge;
    {
        AutoMutex _l(mMutex);
        // If mTimeoutUs is negative, wait on the condition variable until it's positive.
        // If it's positive, merge. The minimum period between waking the condition variable
        // is handled in AudioFlinger::MediaLogNotifier::threadLoop().
        mCond.wait(mMutex);
        doMerge = mTimeoutUs > 0;
        mTimeoutUs -= kThreadSleepPeriodUs;
    }
    if (doMerge) {
        // Merge data from all the readers
        mMerger.merge();
        // Process the data collected by mMerger and write it to PerformanceAnalysis
        // FIXME: decide whether to call getAndProcessSnapshot every time
        // or whether to have a separate thread that calls it with a lower frequency
        mMergeReader.getAndProcessSnapshot();
    }
    return true;
}

void MergeThread::wakeup()
{
    setTimeoutUs(kThreadWakeupPeriodUs);
}

void MergeThread::setTimeoutUs(int time)
{
    AutoMutex _l(mMutex);
    mTimeoutUs = time;
    mCond.signal();
}

}   // namespace NBLog
}   // namespace android
