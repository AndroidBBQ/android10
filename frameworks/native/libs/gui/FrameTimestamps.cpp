/*
* Copyright 2016 The Android Open Source Project
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

#include <gui/FrameTimestamps.h>

#define LOG_TAG "FrameEvents"

#include <android-base/stringprintf.h>
#include <cutils/compiler.h>  // For CC_[UN]LIKELY
#include <inttypes.h>
#include <utils/Log.h>

#include <algorithm>
#include <limits>
#include <numeric>

namespace android {

using base::StringAppendF;

// ============================================================================
// FrameEvents
// ============================================================================

bool FrameEvents::hasPostedInfo() const {
    return FrameEvents::isValidTimestamp(postedTime);
}

bool FrameEvents::hasRequestedPresentInfo() const {
    return FrameEvents::isValidTimestamp(requestedPresentTime);
}

bool FrameEvents::hasLatchInfo() const {
    return FrameEvents::isValidTimestamp(latchTime);
}

bool FrameEvents::hasFirstRefreshStartInfo() const {
    return FrameEvents::isValidTimestamp(firstRefreshStartTime);
}

bool FrameEvents::hasLastRefreshStartInfo() const {
    // The last refresh start time may continue to update until a new frame
    // is latched. We know we have the final value once the release info is set.
    return addReleaseCalled;
}

bool FrameEvents::hasDequeueReadyInfo() const {
    return FrameEvents::isValidTimestamp(dequeueReadyTime);
}

bool FrameEvents::hasAcquireInfo() const {
    return acquireFence->isValid();
}

bool FrameEvents::hasGpuCompositionDoneInfo() const {
    // We may not get a gpuCompositionDone in addPostComposite if
    // client/gles compositing isn't needed.
    return addPostCompositeCalled;
}

bool FrameEvents::hasDisplayPresentInfo() const {
    // We may not get a displayPresent in addPostComposite for HWC1.
    return addPostCompositeCalled;
}

bool FrameEvents::hasReleaseInfo() const {
    return addReleaseCalled;
}

void FrameEvents::checkFencesForCompletion() {
    acquireFence->getSignalTime();
    gpuCompositionDoneFence->getSignalTime();
    displayPresentFence->getSignalTime();
    releaseFence->getSignalTime();
}

static void dumpFenceTime(std::string& outString, const char* name, bool pending,
                          const FenceTime& fenceTime) {
    StringAppendF(&outString, "--- %s", name);
    nsecs_t signalTime = fenceTime.getCachedSignalTime();
    if (Fence::isValidTimestamp(signalTime)) {
        StringAppendF(&outString, "%" PRId64 "\n", signalTime);
    } else if (pending || signalTime == Fence::SIGNAL_TIME_PENDING) {
        outString.append("Pending\n");
    } else if (&fenceTime == FenceTime::NO_FENCE.get()){
        outString.append("N/A\n");
    } else {
        outString.append("Error\n");
    }
}

void FrameEvents::dump(std::string& outString) const {
    if (!valid) {
        return;
    }

    StringAppendF(&outString, "-- Frame %" PRIu64 "\n", frameNumber);
    StringAppendF(&outString, "--- Posted      \t%" PRId64 "\n", postedTime);
    StringAppendF(&outString, "--- Req. Present\t%" PRId64 "\n", requestedPresentTime);

    outString.append("--- Latched     \t");
    if (FrameEvents::isValidTimestamp(latchTime)) {
        StringAppendF(&outString, "%" PRId64 "\n", latchTime);
    } else {
        outString.append("Pending\n");
    }

    outString.append("--- Refresh (First)\t");
    if (FrameEvents::isValidTimestamp(firstRefreshStartTime)) {
        StringAppendF(&outString, "%" PRId64 "\n", firstRefreshStartTime);
    } else {
        outString.append("Pending\n");
    }

    outString.append("--- Refresh (Last)\t");
    if (FrameEvents::isValidTimestamp(lastRefreshStartTime)) {
        StringAppendF(&outString, "%" PRId64 "\n", lastRefreshStartTime);
    } else {
        outString.append("Pending\n");
    }

    dumpFenceTime(outString, "Acquire           \t",
            true, *acquireFence);
    dumpFenceTime(outString, "GPU Composite Done\t",
            !addPostCompositeCalled, *gpuCompositionDoneFence);
    dumpFenceTime(outString, "Display Present   \t",
            !addPostCompositeCalled, *displayPresentFence);

    outString.append("--- DequeueReady  \t");
    if (FrameEvents::isValidTimestamp(dequeueReadyTime)) {
        StringAppendF(&outString, "%" PRId64 "\n", dequeueReadyTime);
    } else {
        outString.append("Pending\n");
    }

    dumpFenceTime(outString, "Release           \t",
            true, *releaseFence);
}


// ============================================================================
// FrameEventHistory
// ============================================================================

namespace {

struct FrameNumberEqual {
    explicit FrameNumberEqual(uint64_t frameNumber) : mFrameNumber(frameNumber) {}
    bool operator()(const FrameEvents& frame) {
        return frame.valid && mFrameNumber == frame.frameNumber;
    }
    const uint64_t mFrameNumber;
};

}  // namespace

FrameEventHistory::~FrameEventHistory() = default;

FrameEvents* FrameEventHistory::getFrame(uint64_t frameNumber) {
    auto frame = std::find_if(
            mFrames.begin(), mFrames.end(), FrameNumberEqual(frameNumber));
    return frame == mFrames.end() ? nullptr : &(*frame);
}

FrameEvents* FrameEventHistory::getFrame(uint64_t frameNumber, size_t* iHint) {
    *iHint = std::min(*iHint, mFrames.size());
    auto hint = mFrames.begin() + *iHint;
    auto frame = std::find_if(
            hint, mFrames.end(), FrameNumberEqual(frameNumber));
    if (frame == mFrames.end()) {
        frame = std::find_if(
                mFrames.begin(), hint, FrameNumberEqual(frameNumber));
        if (frame == hint) {
            return nullptr;
        }
    }
    *iHint = static_cast<size_t>(std::distance(mFrames.begin(), frame));
    return &(*frame);
}

void FrameEventHistory::checkFencesForCompletion() {
    for (auto& frame : mFrames) {
        frame.checkFencesForCompletion();
    }
}

// Uses !|valid| as the MSB.
static bool FrameNumberLessThan(
        const FrameEvents& lhs, const FrameEvents& rhs) {
    if (lhs.valid == rhs.valid) {
        return lhs.frameNumber < rhs.frameNumber;
    }
    return lhs.valid;
}

void FrameEventHistory::dump(std::string& outString) const {
    auto earliestFrame = std::min_element(
            mFrames.begin(), mFrames.end(), &FrameNumberLessThan);
    if (!earliestFrame->valid) {
        outString.append("-- N/A\n");
        return;
    }
    for (auto frame = earliestFrame; frame != mFrames.end(); ++frame) {
        frame->dump(outString);
    }
    for (auto frame = mFrames.begin(); frame != earliestFrame; ++frame) {
        frame->dump(outString);
    }
}


// ============================================================================
// ProducerFrameEventHistory
// ============================================================================

ProducerFrameEventHistory::~ProducerFrameEventHistory() = default;

nsecs_t ProducerFrameEventHistory::snapToNextTick(
        nsecs_t timestamp, nsecs_t tickPhase, nsecs_t tickInterval) {
    nsecs_t tickOffset = (tickPhase - timestamp) % tickInterval;
    // Integer modulo rounds towards 0 and not -inf before taking the remainder,
    // so adjust the offset if it is negative.
    if (tickOffset < 0) {
        tickOffset += tickInterval;
    }
    return timestamp + tickOffset;
}

nsecs_t ProducerFrameEventHistory::getNextCompositeDeadline(
        const nsecs_t now) const{
    return snapToNextTick(
            now, mCompositorTiming.deadline, mCompositorTiming.interval);
}

void ProducerFrameEventHistory::updateAcquireFence(
        uint64_t frameNumber, std::shared_ptr<FenceTime>&& acquire) {
    FrameEvents* frame = getFrame(frameNumber, &mAcquireOffset);
    if (frame == nullptr) {
        ALOGE("updateAcquireFence: Did not find frame.");
        return;
    }

    if (acquire->isValid()) {
        mAcquireTimeline.push(acquire);
        frame->acquireFence = std::move(acquire);
    } else {
        // If there isn't an acquire fence, assume that buffer was
        // ready for the consumer when posted.
        frame->acquireFence = std::make_shared<FenceTime>(frame->postedTime);
    }
}

void ProducerFrameEventHistory::applyDelta(
        const FrameEventHistoryDelta& delta) {
    mCompositorTiming = delta.mCompositorTiming;

    for (auto& d : delta.mDeltas) {
        // Avoid out-of-bounds access.
        if (CC_UNLIKELY(d.mIndex >= mFrames.size())) {
            ALOGE("applyDelta: Bad index.");
            return;
        }

        FrameEvents& frame = mFrames[d.mIndex];

        frame.addPostCompositeCalled = d.mAddPostCompositeCalled != 0;
        frame.addReleaseCalled = d.mAddReleaseCalled != 0;

        frame.postedTime = d.mPostedTime;
        frame.requestedPresentTime = d.mRequestedPresentTime;
        frame.latchTime = d.mLatchTime;
        frame.firstRefreshStartTime = d.mFirstRefreshStartTime;
        frame.lastRefreshStartTime = d.mLastRefreshStartTime;
        frame.dequeueReadyTime = d.mDequeueReadyTime;

        if (frame.frameNumber != d.mFrameNumber) {
            // We got a new frame. Initialize some of the fields.
            frame.frameNumber = d.mFrameNumber;
            frame.acquireFence = FenceTime::NO_FENCE;
            frame.gpuCompositionDoneFence = FenceTime::NO_FENCE;
            frame.displayPresentFence = FenceTime::NO_FENCE;
            frame.releaseFence = FenceTime::NO_FENCE;
            // The consumer only sends valid frames.
            frame.valid = true;
        }

        applyFenceDelta(&mGpuCompositionDoneTimeline,
                &frame.gpuCompositionDoneFence, d.mGpuCompositionDoneFence);
        applyFenceDelta(&mPresentTimeline,
                &frame.displayPresentFence, d.mDisplayPresentFence);
        applyFenceDelta(&mReleaseTimeline,
                &frame.releaseFence, d.mReleaseFence);
    }
}

void ProducerFrameEventHistory::updateSignalTimes() {
    mAcquireTimeline.updateSignalTimes();
    mGpuCompositionDoneTimeline.updateSignalTimes();
    mPresentTimeline.updateSignalTimes();
    mReleaseTimeline.updateSignalTimes();
}

void ProducerFrameEventHistory::applyFenceDelta(FenceTimeline* timeline,
        std::shared_ptr<FenceTime>* dst, const FenceTime::Snapshot& src) const {
    if (CC_UNLIKELY(dst == nullptr || dst->get() == nullptr)) {
        ALOGE("applyFenceDelta: dst is null.");
        return;
    }

    switch (src.state) {
        case FenceTime::Snapshot::State::EMPTY:
            return;
        case FenceTime::Snapshot::State::FENCE:
            ALOGE_IF((*dst)->isValid(), "applyFenceDelta: Unexpected fence.");
            *dst = createFenceTime(src.fence);
            timeline->push(*dst);
            return;
        case FenceTime::Snapshot::State::SIGNAL_TIME:
            if ((*dst)->isValid()) {
                (*dst)->applyTrustedSnapshot(src);
            } else {
                *dst = std::make_shared<FenceTime>(src.signalTime);
            }
            return;
    }
}

std::shared_ptr<FenceTime> ProducerFrameEventHistory::createFenceTime(
        const sp<Fence>& fence) const {
    return std::make_shared<FenceTime>(fence);
}


// ============================================================================
// ConsumerFrameEventHistory
// ============================================================================

ConsumerFrameEventHistory::~ConsumerFrameEventHistory() = default;

void ConsumerFrameEventHistory::onDisconnect() {
    mCurrentConnectId++;
    mProducerWantsEvents = false;
}

void ConsumerFrameEventHistory::initializeCompositorTiming(
        const CompositorTiming& compositorTiming) {
    mCompositorTiming = compositorTiming;
}

void ConsumerFrameEventHistory::addQueue(const NewFrameEventsEntry& newEntry) {
    // Overwrite all fields of the frame with default values unless set here.
    FrameEvents newTimestamps;
    newTimestamps.connectId = mCurrentConnectId;
    newTimestamps.frameNumber = newEntry.frameNumber;
    newTimestamps.postedTime = newEntry.postedTime;
    newTimestamps.requestedPresentTime = newEntry.requestedPresentTime;
    newTimestamps.acquireFence = newEntry.acquireFence;
    newTimestamps.valid = true;
    mFrames[mQueueOffset] = newTimestamps;

    // Note: We avoid sending the acquire fence back to the caller since
    // they have the original one already, so there is no need to set the
    // acquire dirty bit.
    mFramesDirty[mQueueOffset].setDirty<FrameEvent::POSTED>();

    mQueueOffset = (mQueueOffset + 1) % mFrames.size();
}

void ConsumerFrameEventHistory::addLatch(
        uint64_t frameNumber, nsecs_t latchTime) {
    FrameEvents* frame = getFrame(frameNumber, &mCompositionOffset);
    if (frame == nullptr) {
        ALOGE_IF(mProducerWantsEvents, "addLatch: Did not find frame.");
        return;
    }
    frame->latchTime = latchTime;
    mFramesDirty[mCompositionOffset].setDirty<FrameEvent::LATCH>();
}

void ConsumerFrameEventHistory::addPreComposition(
        uint64_t frameNumber, nsecs_t refreshStartTime) {
    FrameEvents* frame = getFrame(frameNumber, &mCompositionOffset);
    if (frame == nullptr) {
        ALOGE_IF(mProducerWantsEvents,
                "addPreComposition: Did not find frame.");
        return;
    }
    frame->lastRefreshStartTime = refreshStartTime;
    mFramesDirty[mCompositionOffset].setDirty<FrameEvent::LAST_REFRESH_START>();
    if (!FrameEvents::isValidTimestamp(frame->firstRefreshStartTime)) {
        frame->firstRefreshStartTime = refreshStartTime;
        mFramesDirty[mCompositionOffset].setDirty<FrameEvent::FIRST_REFRESH_START>();
    }
}

void ConsumerFrameEventHistory::addPostComposition(uint64_t frameNumber,
        const std::shared_ptr<FenceTime>& gpuCompositionDone,
        const std::shared_ptr<FenceTime>& displayPresent,
        const CompositorTiming& compositorTiming) {
    mCompositorTiming = compositorTiming;

    FrameEvents* frame = getFrame(frameNumber, &mCompositionOffset);
    if (frame == nullptr) {
        ALOGE_IF(mProducerWantsEvents,
                "addPostComposition: Did not find frame.");
        return;
    }
    // Only get GPU and present info for the first composite.
    if (!frame->addPostCompositeCalled) {
        frame->addPostCompositeCalled = true;
        frame->gpuCompositionDoneFence = gpuCompositionDone;
        mFramesDirty[mCompositionOffset].setDirty<FrameEvent::GPU_COMPOSITION_DONE>();
        if (!frame->displayPresentFence->isValid()) {
            frame->displayPresentFence = displayPresent;
            mFramesDirty[mCompositionOffset].setDirty<FrameEvent::DISPLAY_PRESENT>();
        }
    }
}

void ConsumerFrameEventHistory::addRelease(uint64_t frameNumber,
        nsecs_t dequeueReadyTime, std::shared_ptr<FenceTime>&& release) {
    FrameEvents* frame = getFrame(frameNumber, &mReleaseOffset);
    if (frame == nullptr) {
        ALOGE_IF(mProducerWantsEvents, "addRelease: Did not find frame.");
        return;
    }
    frame->addReleaseCalled = true;
    frame->dequeueReadyTime = dequeueReadyTime;
    frame->releaseFence = std::move(release);
    mFramesDirty[mReleaseOffset].setDirty<FrameEvent::RELEASE>();
}

void ConsumerFrameEventHistory::getFrameDelta(
        FrameEventHistoryDelta* delta,
        const std::array<FrameEvents, MAX_FRAME_HISTORY>::iterator& frame) {
    mProducerWantsEvents = true;
    size_t i = static_cast<size_t>(std::distance(mFrames.begin(), frame));
    if (mFramesDirty[i].anyDirty()) {
        // Make sure only to send back deltas for the current connection
        // since the producer won't have the correct state to apply a delta
        // from a previous connection.
        if (mFrames[i].connectId == mCurrentConnectId) {
            delta->mDeltas.emplace_back(i, *frame, mFramesDirty[i]);
        }
        mFramesDirty[i].reset();
    }
}

void ConsumerFrameEventHistory::getAndResetDelta(
        FrameEventHistoryDelta* delta) {
    mProducerWantsEvents = true;
    delta->mCompositorTiming = mCompositorTiming;

    // Write these in order of frame number so that it is easy to
    // add them to a FenceTimeline in the proper order producer side.
    delta->mDeltas.reserve(mFramesDirty.size());
    auto earliestFrame = std::min_element(
            mFrames.begin(), mFrames.end(), &FrameNumberLessThan);
    for (auto frame = earliestFrame; frame != mFrames.end(); ++frame) {
        getFrameDelta(delta, frame);
    }
    for (auto frame = mFrames.begin(); frame != earliestFrame; ++frame) {
        getFrameDelta(delta, frame);
    }
}


// ============================================================================
// FrameEventsDelta
// ============================================================================

FrameEventsDelta::FrameEventsDelta(
        size_t index,
        const FrameEvents& frameTimestamps,
        const FrameEventDirtyFields& dirtyFields)
    : mIndex(index),
      mFrameNumber(frameTimestamps.frameNumber),
      mAddPostCompositeCalled(frameTimestamps.addPostCompositeCalled),
      mAddReleaseCalled(frameTimestamps.addReleaseCalled),
      mPostedTime(frameTimestamps.postedTime),
      mRequestedPresentTime(frameTimestamps.requestedPresentTime),
      mLatchTime(frameTimestamps.latchTime),
      mFirstRefreshStartTime(frameTimestamps.firstRefreshStartTime),
      mLastRefreshStartTime(frameTimestamps.lastRefreshStartTime),
      mDequeueReadyTime(frameTimestamps.dequeueReadyTime) {
    if (dirtyFields.isDirty<FrameEvent::GPU_COMPOSITION_DONE>()) {
        mGpuCompositionDoneFence =
                frameTimestamps.gpuCompositionDoneFence->getSnapshot();
    }
    if (dirtyFields.isDirty<FrameEvent::DISPLAY_PRESENT>()) {
        mDisplayPresentFence =
                frameTimestamps.displayPresentFence->getSnapshot();
    }
    if (dirtyFields.isDirty<FrameEvent::RELEASE>()) {
        mReleaseFence = frameTimestamps.releaseFence->getSnapshot();
    }
}

constexpr size_t FrameEventsDelta::minFlattenedSize() {
    return sizeof(FrameEventsDelta::mFrameNumber) +
            sizeof(uint16_t) + // mIndex
            sizeof(uint8_t) + // mAddPostCompositeCalled
            sizeof(uint8_t) + // mAddReleaseCalled
            sizeof(FrameEventsDelta::mPostedTime) +
            sizeof(FrameEventsDelta::mRequestedPresentTime) +
            sizeof(FrameEventsDelta::mLatchTime) +
            sizeof(FrameEventsDelta::mFirstRefreshStartTime) +
            sizeof(FrameEventsDelta::mLastRefreshStartTime) +
            sizeof(FrameEventsDelta::mDequeueReadyTime);
}

// Flattenable implementation
size_t FrameEventsDelta::getFlattenedSize() const {
    auto fences = allFences(this);
    return minFlattenedSize() +
            std::accumulate(fences.begin(), fences.end(), size_t(0),
                    [](size_t a, const FenceTime::Snapshot* fence) {
                            return a + fence->getFlattenedSize();
                    });
}

size_t FrameEventsDelta::getFdCount() const {
    auto fences = allFences(this);
    return std::accumulate(fences.begin(), fences.end(), size_t(0),
            [](size_t a, const FenceTime::Snapshot* fence) {
                return a + fence->getFdCount();
            });
}

status_t FrameEventsDelta::flatten(void*& buffer, size_t& size, int*& fds,
            size_t& count) const {
    if (size < getFlattenedSize() || count < getFdCount()) {
        return NO_MEMORY;
    }

    if (mIndex >= FrameEventHistory::MAX_FRAME_HISTORY ||
            mIndex > std::numeric_limits<uint16_t>::max()) {
        return BAD_VALUE;
    }

    FlattenableUtils::write(buffer, size, mFrameNumber);

    // These are static_cast to uint16_t/uint8_t for alignment.
    FlattenableUtils::write(buffer, size, static_cast<uint16_t>(mIndex));
    FlattenableUtils::write(
            buffer, size, static_cast<uint8_t>(mAddPostCompositeCalled));
    FlattenableUtils::write(
            buffer, size, static_cast<uint8_t>(mAddReleaseCalled));

    FlattenableUtils::write(buffer, size, mPostedTime);
    FlattenableUtils::write(buffer, size, mRequestedPresentTime);
    FlattenableUtils::write(buffer, size, mLatchTime);
    FlattenableUtils::write(buffer, size, mFirstRefreshStartTime);
    FlattenableUtils::write(buffer, size, mLastRefreshStartTime);
    FlattenableUtils::write(buffer, size, mDequeueReadyTime);

    // Fences
    for (auto fence : allFences(this)) {
        status_t status = fence->flatten(buffer, size, fds, count);
        if (status != NO_ERROR) {
            return status;
        }
    }
    return NO_ERROR;
}

status_t FrameEventsDelta::unflatten(void const*& buffer, size_t& size,
            int const*& fds, size_t& count) {
    if (size < minFlattenedSize()) {
        return NO_MEMORY;
    }

    FlattenableUtils::read(buffer, size, mFrameNumber);

    // These were written as uint16_t/uint8_t for alignment.
    uint16_t temp16 = 0;
    FlattenableUtils::read(buffer, size, temp16);
    mIndex = temp16;
    if (mIndex >= FrameEventHistory::MAX_FRAME_HISTORY) {
        return BAD_VALUE;
    }
    uint8_t temp8 = 0;
    FlattenableUtils::read(buffer, size, temp8);
    mAddPostCompositeCalled = static_cast<bool>(temp8);
    FlattenableUtils::read(buffer, size, temp8);
    mAddReleaseCalled = static_cast<bool>(temp8);

    FlattenableUtils::read(buffer, size, mPostedTime);
    FlattenableUtils::read(buffer, size, mRequestedPresentTime);
    FlattenableUtils::read(buffer, size, mLatchTime);
    FlattenableUtils::read(buffer, size, mFirstRefreshStartTime);
    FlattenableUtils::read(buffer, size, mLastRefreshStartTime);
    FlattenableUtils::read(buffer, size, mDequeueReadyTime);

    // Fences
    for (auto fence : allFences(this)) {
        status_t status = fence->unflatten(buffer, size, fds, count);
        if (status != NO_ERROR) {
            return status;
        }
    }
    return NO_ERROR;
}


// ============================================================================
// FrameEventHistoryDelta
// ============================================================================

FrameEventHistoryDelta& FrameEventHistoryDelta::operator=(
        FrameEventHistoryDelta&& src) noexcept {
    mCompositorTiming = src.mCompositorTiming;

    if (CC_UNLIKELY(!mDeltas.empty())) {
        ALOGE("FrameEventHistoryDelta assign clobbering history.");
    }
    mDeltas = std::move(src.mDeltas);
    return *this;
}

constexpr size_t FrameEventHistoryDelta::minFlattenedSize() {
    return sizeof(uint32_t) + // mDeltas.size()
            sizeof(mCompositorTiming);
}

size_t FrameEventHistoryDelta::getFlattenedSize() const {
    return minFlattenedSize() +
            std::accumulate(mDeltas.begin(), mDeltas.end(), size_t(0),
                    [](size_t a, const FrameEventsDelta& delta) {
                            return a + delta.getFlattenedSize();
                    });
}

size_t FrameEventHistoryDelta::getFdCount() const {
    return std::accumulate(mDeltas.begin(), mDeltas.end(), size_t(0),
            [](size_t a, const FrameEventsDelta& delta) {
                    return a + delta.getFdCount();
            });
}

status_t FrameEventHistoryDelta::flatten(
            void*& buffer, size_t& size, int*& fds, size_t& count) const {
    if (mDeltas.size() > FrameEventHistory::MAX_FRAME_HISTORY) {
        return BAD_VALUE;
    }
    if (size < getFlattenedSize()) {
        return NO_MEMORY;
    }

    FlattenableUtils::write(buffer, size, mCompositorTiming);

    FlattenableUtils::write(
            buffer, size, static_cast<uint32_t>(mDeltas.size()));
    for (auto& d : mDeltas) {
        status_t status = d.flatten(buffer, size, fds, count);
        if (status != NO_ERROR) {
            return status;
        }
    }
    return NO_ERROR;
}

status_t FrameEventHistoryDelta::unflatten(
            void const*& buffer, size_t& size, int const*& fds, size_t& count) {
    if (size < minFlattenedSize()) {
        return NO_MEMORY;
    }

    FlattenableUtils::read(buffer, size, mCompositorTiming);

    uint32_t deltaCount = 0;
    FlattenableUtils::read(buffer, size, deltaCount);
    if (deltaCount > FrameEventHistory::MAX_FRAME_HISTORY) {
        return BAD_VALUE;
    }
    mDeltas.resize(deltaCount);
    for (auto& d : mDeltas) {
        status_t status = d.unflatten(buffer, size, fds, count);
        if (status != NO_ERROR) {
            return status;
        }
    }
    return NO_ERROR;
}


} // namespace android
