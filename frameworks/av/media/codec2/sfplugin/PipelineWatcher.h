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

#ifndef PIPELINE_WATCHER_H_
#define PIPELINE_WATCHER_H_

#include <chrono>
#include <map>
#include <memory>

#include <C2Work.h>

namespace android {

/**
 * PipelineWatcher watches the pipeline and infers the status of work items from
 * events.
 */
class PipelineWatcher {
public:
    typedef std::chrono::steady_clock Clock;

    PipelineWatcher()
        : mInputDelay(0),
          mPipelineDelay(0),
          mOutputDelay(0),
          mSmoothnessFactor(0) {}
    ~PipelineWatcher() = default;

    /**
     * \param value the new input delay value
     * \return  this object
     */
    PipelineWatcher &inputDelay(uint32_t value);

    /**
     * \param value the new pipeline delay value
     * \return  this object
     */
    PipelineWatcher &pipelineDelay(uint32_t value);

    /**
     * \param value the new output delay value
     * \return  this object
     */
    PipelineWatcher &outputDelay(uint32_t value);

    /**
     * \param value the new smoothness factor value
     * \return  this object
     */
    PipelineWatcher &smoothnessFactor(uint32_t value);

    /**
     * Client queued a work item to the component.
     *
     * \param frameIndex  input frame index of this work
     * \param buffers     input buffers of the queued work item
     * \param queuedAt    time when the client queued the buffer
     */
    void onWorkQueued(
            uint64_t frameIndex,
            std::vector<std::shared_ptr<C2Buffer>> &&buffers,
            const Clock::time_point &queuedAt);

    /**
     * The component released input buffers from a work item.
     *
     * \param frameIndex  input frame index
     * \param arrayIndex  index of the buffer at the original |buffers| in
     *                    onWorkQueued().
     * \return  buffers[arrayIndex]
     */
    std::shared_ptr<C2Buffer> onInputBufferReleased(
            uint64_t frameIndex, size_t arrayIndex);

    /**
     * The component finished processing a work item.
     *
     * \param frameIndex  input frame index
     */
    void onWorkDone(uint64_t frameIndex);

    /**
     * Flush the pipeline.
     */
    void flush();

    /**
     * \return  true  if pipeline does not need more work items to proceed
     *                smoothly, considering delays and smoothness factor;
     *          false otherwise.
     */
    bool pipelineFull() const;

    /**
     * Return elapsed processing time of a work item, nth from the longest
     * processing time to the shortest.
     *
     * \param now current timestamp
     * \param n   nth work item, from the longest processing time to the
     *            shortest. It's a 0-based index.
     * \return  elapsed processing time of nth work item.
     */
    Clock::duration elapsed(const Clock::time_point &now, size_t n) const;

private:
    uint32_t mInputDelay;
    uint32_t mPipelineDelay;
    uint32_t mOutputDelay;
    uint32_t mSmoothnessFactor;

    struct Frame {
        Frame(std::vector<std::shared_ptr<C2Buffer>> &&b,
              const Clock::time_point &q)
            : buffers(b),
              queuedAt(q) {}
        std::vector<std::shared_ptr<C2Buffer>> buffers;
        const Clock::time_point queuedAt;
    };
    std::map<uint64_t, Frame> mFramesInPipeline;
};

}  // namespace android

#endif  // PIPELINE_WATCHER_H_
