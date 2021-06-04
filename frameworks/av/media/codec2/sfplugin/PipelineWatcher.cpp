/*
 * Copyright 2019 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "PipelineWatcher"

#include <numeric>

#include <log/log.h>

#include "PipelineWatcher.h"

namespace android {

PipelineWatcher &PipelineWatcher::inputDelay(uint32_t value) {
    mInputDelay = value;
    return *this;
}

PipelineWatcher &PipelineWatcher::pipelineDelay(uint32_t value) {
    mPipelineDelay = value;
    return *this;
}

PipelineWatcher &PipelineWatcher::outputDelay(uint32_t value) {
    mOutputDelay = value;
    return *this;
}

PipelineWatcher &PipelineWatcher::smoothnessFactor(uint32_t value) {
    mSmoothnessFactor = value;
    return *this;
}

void PipelineWatcher::onWorkQueued(
        uint64_t frameIndex,
        std::vector<std::shared_ptr<C2Buffer>> &&buffers,
        const Clock::time_point &queuedAt) {
    ALOGV("onWorkQueued(frameIndex=%llu, buffers(size=%zu), queuedAt=%lld)",
          (unsigned long long)frameIndex,
          buffers.size(),
          (long long)queuedAt.time_since_epoch().count());
    auto it = mFramesInPipeline.find(frameIndex);
    if (it != mFramesInPipeline.end()) {
        ALOGD("onWorkQueued: Duplicate frame index (%llu); previous entry removed",
              (unsigned long long)frameIndex);
        (void)mFramesInPipeline.erase(it);
    }
    (void)mFramesInPipeline.try_emplace(frameIndex, std::move(buffers), queuedAt);
}

std::shared_ptr<C2Buffer> PipelineWatcher::onInputBufferReleased(
        uint64_t frameIndex, size_t arrayIndex) {
    ALOGV("onInputBufferReleased(frameIndex=%llu, arrayIndex=%zu)",
          (unsigned long long)frameIndex, arrayIndex);
    auto it = mFramesInPipeline.find(frameIndex);
    if (it == mFramesInPipeline.end()) {
        ALOGD("onInputBufferReleased: frameIndex not found (%llu); ignored",
              (unsigned long long)frameIndex);
        return nullptr;
    }
    if (it->second.buffers.size() <= arrayIndex) {
        ALOGD("onInputBufferReleased: buffers at %llu: size %zu, requested index: %zu",
              (unsigned long long)frameIndex, it->second.buffers.size(), arrayIndex);
        return nullptr;
    }
    std::shared_ptr<C2Buffer> buffer(std::move(it->second.buffers[arrayIndex]));
    ALOGD_IF(!buffer, "onInputBufferReleased: buffer already released (%llu:%zu)",
             (unsigned long long)frameIndex, arrayIndex);
    return buffer;
}

void PipelineWatcher::onWorkDone(uint64_t frameIndex) {
    ALOGV("onWorkDone(frameIndex=%llu)", (unsigned long long)frameIndex);
    auto it = mFramesInPipeline.find(frameIndex);
    if (it == mFramesInPipeline.end()) {
        ALOGD("onWorkDone: frameIndex not found (%llu); ignored",
              (unsigned long long)frameIndex);
        return;
    }
    (void)mFramesInPipeline.erase(it);
}

void PipelineWatcher::flush() {
    mFramesInPipeline.clear();
}

bool PipelineWatcher::pipelineFull() const {
    if (mFramesInPipeline.size() >=
            mInputDelay + mPipelineDelay + mOutputDelay + mSmoothnessFactor) {
        ALOGV("pipelineFull: too many frames in pipeline (%zu)", mFramesInPipeline.size());
        return true;
    }
    size_t sizeWithInputReleased = std::count_if(
            mFramesInPipeline.begin(),
            mFramesInPipeline.end(),
            [](const decltype(mFramesInPipeline)::value_type &value) {
                for (const std::shared_ptr<C2Buffer> &buffer : value.second.buffers) {
                    if (buffer) {
                        return false;
                    }
                }
                return true;
            });
    if (sizeWithInputReleased >=
            mPipelineDelay + mOutputDelay + mSmoothnessFactor) {
        ALOGV("pipelineFull: too many frames in pipeline, with input released (%zu)",
              sizeWithInputReleased);
        return true;
    }

    size_t sizeWithInputsPending = mFramesInPipeline.size() - sizeWithInputReleased;
    if (sizeWithInputsPending > mPipelineDelay + mInputDelay + mSmoothnessFactor) {
        ALOGV("pipelineFull: too many inputs pending (%zu) in pipeline, with inputs released (%zu)",
              sizeWithInputsPending, sizeWithInputReleased);
        return true;
    }
    ALOGV("pipeline has room (total: %zu, input released: %zu)",
          mFramesInPipeline.size(), sizeWithInputReleased);
    return false;
}

PipelineWatcher::Clock::duration PipelineWatcher::elapsed(
        const PipelineWatcher::Clock::time_point &now, size_t n) const {
    if (mFramesInPipeline.size() <= n) {
        return Clock::duration::zero();
    }
    std::vector<Clock::duration> durations;
    for (const decltype(mFramesInPipeline)::value_type &value : mFramesInPipeline) {
        Clock::duration elapsed = now - value.second.queuedAt;
        ALOGV("elapsed: frameIndex = %llu elapsed = %lldms",
              (unsigned long long)value.first,
              std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
        durations.push_back(elapsed);
    }
    std::nth_element(durations.begin(), durations.begin() + n, durations.end(),
                     std::greater<Clock::duration>());
    return durations[n];
}

}  // namespace android
