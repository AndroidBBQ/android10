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

// #define LOG_NDEBUG 0
#define LOG_TAG "media_c2_hidl_test_common"
#include <stdio.h>

#include "media_c2_hidl_test_common.h"

// Test the codecs for NullBuffer, Empty Input Buffer with(out) flags set
void testInputBuffer(
    const std::shared_ptr<android::Codec2Client::Component>& component,
    std::mutex& queueLock, std::list<std::unique_ptr<C2Work>>& workQueue,
    uint32_t flags, bool isNullBuffer) {
    std::unique_ptr<C2Work> work;
    {
        typedef std::unique_lock<std::mutex> ULock;
        ULock l(queueLock);
        if (!workQueue.empty()) {
            work.swap(workQueue.front());
            workQueue.pop_front();
        } else {
            ASSERT_TRUE(false) << "workQueue Empty at the start of test";
        }
    }
    ASSERT_NE(work, nullptr);

    work->input.flags = (C2FrameData::flags_t)flags;
    work->input.ordinal.timestamp = 0;
    work->input.ordinal.frameIndex = 0;
    work->input.buffers.clear();
    if (isNullBuffer) {
        work->input.buffers.emplace_back(nullptr);
    }
    work->worklets.clear();
    work->worklets.emplace_back(new C2Worklet);

    std::list<std::unique_ptr<C2Work>> items;
    items.push_back(std::move(work));
    ASSERT_EQ(component->queue(&items), C2_OK);
}

// Wait for all the inputs to be consumed by the plugin.
void waitOnInputConsumption(std::mutex& queueLock,
                            std::condition_variable& queueCondition,
                            std::list<std::unique_ptr<C2Work>>& workQueue,
                            size_t bufferCount) {
    typedef std::unique_lock<std::mutex> ULock;
    uint32_t queueSize;
    uint32_t maxRetry = 0;
    {
        ULock l(queueLock);
        queueSize = workQueue.size();
    }
    while ((maxRetry < MAX_RETRY) && (queueSize < bufferCount)) {
        ULock l(queueLock);
        if (queueSize != workQueue.size()) {
            queueSize = workQueue.size();
            maxRetry = 0;
        } else {
            queueCondition.wait_for(l, TIME_OUT);
            maxRetry++;
        }
    }
}

// process onWorkDone received by Listener
void workDone(
    const std::shared_ptr<android::Codec2Client::Component>& component,
    std::unique_ptr<C2Work>& work, std::list<uint64_t>& flushedIndices,
    std::mutex& queueLock, std::condition_variable& queueCondition,
    std::list<std::unique_ptr<C2Work>>& workQueue, bool& eos, bool& csd,
    uint32_t& framesReceived) {
    // handle configuration changes in work done
    if (work->worklets.front()->output.configUpdate.size() != 0) {
        ALOGV("Config Update");
        std::vector<std::unique_ptr<C2Param>> updates =
            std::move(work->worklets.front()->output.configUpdate);
        std::vector<C2Param*> configParam;
        std::vector<std::unique_ptr<C2SettingResult>> failures;
        for (size_t i = 0; i < updates.size(); ++i) {
            C2Param* param = updates[i].get();
            if (param->index() == C2StreamInitDataInfo::output::PARAM_TYPE) {
                csd = true;
            } else if ((param->index() ==
                        C2StreamSampleRateInfo::output::PARAM_TYPE) ||
                       (param->index() ==
                        C2StreamChannelCountInfo::output::PARAM_TYPE) ||
                       (param->index() ==
                        C2StreamPictureSizeInfo::output::PARAM_TYPE)) {
                configParam.push_back(param);
            }
        }
        component->config(configParam, C2_DONT_BLOCK, &failures);
        ASSERT_EQ(failures.size(), 0u);
    }
    if (work->worklets.front()->output.flags != C2FrameData::FLAG_INCOMPLETE) {
        framesReceived++;
        eos = (work->worklets.front()->output.flags &
               C2FrameData::FLAG_END_OF_STREAM) != 0;
        auto frameIndexIt = std::find(flushedIndices.begin(), flushedIndices.end(),
                                      work->input.ordinal.frameIndex.peeku());
        ALOGV("WorkDone: frameID received %d",
              (int)work->worklets.front()->output.ordinal.frameIndex.peeku());
        work->input.buffers.clear();
        work->worklets.clear();
        {
            typedef std::unique_lock<std::mutex> ULock;
            ULock l(queueLock);
            workQueue.push_back(std::move(work));
            if (!flushedIndices.empty()) {
                flushedIndices.erase(frameIndexIt);
            }
            queueCondition.notify_all();
        }
    }
}

// Return current time in micro seconds
int64_t getNowUs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (int64_t)tv.tv_usec + tv.tv_sec * 1000000ll;
}
