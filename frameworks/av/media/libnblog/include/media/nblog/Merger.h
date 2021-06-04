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

#ifndef ANDROID_MEDIA_NBLOG_MERGER_H
#define ANDROID_MEDIA_NBLOG_MERGER_H

#include <memory>
#include <stddef.h>
#include <stdint.h>
#include <vector>

#include <audio_utils/fifo.h>
#include <media/nblog/PerformanceAnalysis.h>
#include <media/nblog/Reader.h>
#include <utils/Condition.h>
#include <utils/Mutex.h>
#include <utils/RefBase.h>
#include <utils/Thread.h>
#include <utils/Timers.h>
#include <utils/Vector.h>

namespace android {

class String16;
class String8;

namespace NBLog {

struct Shared;

// TODO update comments to reflect current functionalities

// This class is used to read data from each thread's individual FIFO in shared memory
// and write it to a single FIFO in local memory.
class Merger : public RefBase {
public:
    Merger(const void *shared, size_t size);

    ~Merger() override = default;

    void addReader(const sp<NBLog::Reader> &reader);
    // TODO add removeReader
    void merge();

    // FIXME This is returning a reference to a shared variable that needs a lock
    const std::vector<sp<Reader>>& getReaders() const;

private:
    // vector of the readers the merger is supposed to merge from.
    // every reader reads from a writer's buffer
    // FIXME Needs to be protected by a lock
    std::vector<sp<Reader>> mReaders;

    Shared * const mShared; // raw pointer to shared memory
    std::unique_ptr<audio_utils_fifo> mFifo; // FIFO itself
    std::unique_ptr<audio_utils_fifo_writer> mFifoWriter; // used to write to FIFO
};

// This class has a pointer to the FIFO in local memory which stores the merged
// data collected by NBLog::Merger from all Readers. It is used to process
// this data and write the result to PerformanceAnalysis.
class MergeReader : public Reader {
public:
    MergeReader(const void *shared, size_t size, Merger &merger);

    // process a particular snapshot of the reader
    void processSnapshot(Snapshot &snap, int author);

    // call getSnapshot of the content of the reader's buffer and process the data
    void getAndProcessSnapshot();

    // check for periodic push of performance data to media metrics, and perform
    // the send if it is time to do so.
    void checkPushToMediaMetrics();

    void dump(int fd, const Vector<String16>& args);

private:
    // FIXME Needs to be protected by a lock,
    //       because even though our use of it is read-only there may be asynchronous updates
    // The object is owned by the Merger class.
    const std::vector<sp<Reader>>& mReaders;

    // analyzes, compresses and stores the merged data
    // contains a separate instance for every author (thread), and for every source file
    // location within each author
    ReportPerformance::PerformanceAnalysisMap mThreadPerformanceAnalysis;

    // compresses and stores audio performance data from each thread's buffers.
    // first parameter is author, i.e. thread index.
    std::map<int, ReportPerformance::PerformanceData> mThreadPerformanceData;

    // how often to push data to Media Metrics
    static constexpr nsecs_t kPeriodicMediaMetricsPush = s2ns((nsecs_t)2 * 60 * 60); // 2 hours

    // handle author entry by looking up the author's name and appending it to the body
    // returns number of bytes read from fmtEntry
    void handleAuthor(const AbstractEntry &fmtEntry, String8 *body);
};

// MergeThread is a thread that contains a Merger. It works as a retriggerable one-shot:
// when triggered, it awakes for a lapse of time, during which it periodically merges; if
// retriggered, the timeout is reset.
// The thread is triggered on AudioFlinger binder activity.
class MergeThread : public Thread {
public:
    MergeThread(Merger &merger, MergeReader &mergeReader);
    ~MergeThread() override;

    // Reset timeout and activate thread to merge periodically if it's idle
    void wakeup();

    // Set timeout period until the merging thread goes idle again
    void setTimeoutUs(int time);

private:
    bool threadLoop() override;

    // the merger who actually does the work of merging the logs
    Merger&      mMerger;

    // the mergereader used to process data merged by mMerger
    MergeReader& mMergeReader;

    // mutex for the condition variable
    Mutex        mMutex;

    // condition variable to activate merging on timeout >= 0
    Condition    mCond;

    // time left until the thread blocks again (in microseconds)
    int          mTimeoutUs;

    // merging period when the thread is awake
    static const int  kThreadSleepPeriodUs = 1000000 /*1s*/;

    // initial timeout value when triggered
    static const int  kThreadWakeupPeriodUs = 3000000 /*3s*/;
};

}   // namespace NBLog
}   // namespace android

#endif  // ANDROID_MEDIA_NBLOG_MERGER_H
