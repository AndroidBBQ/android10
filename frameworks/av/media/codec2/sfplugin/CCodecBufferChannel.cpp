/*
 * Copyright 2017, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "CCodecBufferChannel"
#include <utils/Log.h>

#include <numeric>

#include <C2AllocatorGralloc.h>
#include <C2PlatformSupport.h>
#include <C2BlockInternal.h>
#include <C2Config.h>
#include <C2Debug.h>

#include <android/hardware/cas/native/1.0/IDescrambler.h>
#include <android-base/stringprintf.h>
#include <binder/MemoryDealer.h>
#include <gui/Surface.h>
#include <media/openmax/OMX_Core.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ALookup.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AUtils.h>
#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/MediaCodecConstants.h>
#include <media/MediaCodecBuffer.h>
#include <system/window.h>

#include "CCodecBufferChannel.h"
#include "Codec2Buffer.h"
#include "SkipCutBuffer.h"

namespace android {

using android::base::StringPrintf;
using hardware::hidl_handle;
using hardware::hidl_string;
using hardware::hidl_vec;
using namespace hardware::cas::V1_0;
using namespace hardware::cas::native::V1_0;

using CasStatus = hardware::cas::V1_0::Status;

namespace {

constexpr size_t kSmoothnessFactor = 4;
constexpr size_t kRenderingDepth = 3;

// This is for keeping IGBP's buffer dropping logic in legacy mode other
// than making it non-blocking. Do not change this value.
const static size_t kDequeueTimeoutNs = 0;

}  // namespace

CCodecBufferChannel::QueueGuard::QueueGuard(
        CCodecBufferChannel::QueueSync &sync) : mSync(sync) {
    Mutex::Autolock l(mSync.mGuardLock);
    // At this point it's guaranteed that mSync is not under state transition,
    // as we are holding its mutex.

    Mutexed<CCodecBufferChannel::QueueSync::Counter>::Locked count(mSync.mCount);
    if (count->value == -1) {
        mRunning = false;
    } else {
        ++count->value;
        mRunning = true;
    }
}

CCodecBufferChannel::QueueGuard::~QueueGuard() {
    if (mRunning) {
        // We are not holding mGuardLock at this point so that QueueSync::stop() can
        // keep holding the lock until mCount reaches zero.
        Mutexed<CCodecBufferChannel::QueueSync::Counter>::Locked count(mSync.mCount);
        --count->value;
        count->cond.broadcast();
    }
}

void CCodecBufferChannel::QueueSync::start() {
    Mutex::Autolock l(mGuardLock);
    // If stopped, it goes to running state; otherwise no-op.
    Mutexed<Counter>::Locked count(mCount);
    if (count->value == -1) {
        count->value = 0;
    }
}

void CCodecBufferChannel::QueueSync::stop() {
    Mutex::Autolock l(mGuardLock);
    Mutexed<Counter>::Locked count(mCount);
    if (count->value == -1) {
        // no-op
        return;
    }
    // Holding mGuardLock here blocks creation of additional QueueGuard objects, so
    // mCount can only decrement. In other words, threads that acquired the lock
    // are allowed to finish execution but additional threads trying to acquire
    // the lock at this point will block, and then get QueueGuard at STOPPED
    // state.
    while (count->value != 0) {
        count.waitForCondition(count->cond);
    }
    count->value = -1;
}

// CCodecBufferChannel::ReorderStash

CCodecBufferChannel::ReorderStash::ReorderStash() {
    clear();
}

void CCodecBufferChannel::ReorderStash::clear() {
    mPending.clear();
    mStash.clear();
    mDepth = 0;
    mKey = C2Config::ORDINAL;
}

void CCodecBufferChannel::ReorderStash::flush() {
    mPending.clear();
    mStash.clear();
}

void CCodecBufferChannel::ReorderStash::setDepth(uint32_t depth) {
    mPending.splice(mPending.end(), mStash);
    mDepth = depth;
}

void CCodecBufferChannel::ReorderStash::setKey(C2Config::ordinal_key_t key) {
    mPending.splice(mPending.end(), mStash);
    mKey = key;
}

bool CCodecBufferChannel::ReorderStash::pop(Entry *entry) {
    if (mPending.empty()) {
        return false;
    }
    entry->buffer     = mPending.front().buffer;
    entry->timestamp  = mPending.front().timestamp;
    entry->flags      = mPending.front().flags;
    entry->ordinal    = mPending.front().ordinal;
    mPending.pop_front();
    return true;
}

void CCodecBufferChannel::ReorderStash::emplace(
        const std::shared_ptr<C2Buffer> &buffer,
        int64_t timestamp,
        int32_t flags,
        const C2WorkOrdinalStruct &ordinal) {
    bool eos = flags & MediaCodec::BUFFER_FLAG_EOS;
    if (!buffer && eos) {
        // TRICKY: we may be violating ordering of the stash here. Because we
        // don't expect any more emplace() calls after this, the ordering should
        // not matter.
        mStash.emplace_back(buffer, timestamp, flags, ordinal);
    } else {
        flags = flags & ~MediaCodec::BUFFER_FLAG_EOS;
        auto it = mStash.begin();
        for (; it != mStash.end(); ++it) {
            if (less(ordinal, it->ordinal)) {
                break;
            }
        }
        mStash.emplace(it, buffer, timestamp, flags, ordinal);
        if (eos) {
            mStash.back().flags = mStash.back().flags | MediaCodec::BUFFER_FLAG_EOS;
        }
    }
    while (!mStash.empty() && mStash.size() > mDepth) {
        mPending.push_back(mStash.front());
        mStash.pop_front();
    }
}

void CCodecBufferChannel::ReorderStash::defer(
        const CCodecBufferChannel::ReorderStash::Entry &entry) {
    mPending.push_front(entry);
}

bool CCodecBufferChannel::ReorderStash::hasPending() const {
    return !mPending.empty();
}

bool CCodecBufferChannel::ReorderStash::less(
        const C2WorkOrdinalStruct &o1, const C2WorkOrdinalStruct &o2) {
    switch (mKey) {
        case C2Config::ORDINAL:   return o1.frameIndex < o2.frameIndex;
        case C2Config::TIMESTAMP: return o1.timestamp < o2.timestamp;
        case C2Config::CUSTOM:    return o1.customOrdinal < o2.customOrdinal;
        default:
            ALOGD("Unrecognized key; default to timestamp");
            return o1.frameIndex < o2.frameIndex;
    }
}

// Input

CCodecBufferChannel::Input::Input() : extraBuffers("extra") {}

// CCodecBufferChannel

CCodecBufferChannel::CCodecBufferChannel(
        const std::shared_ptr<CCodecCallback> &callback)
    : mHeapSeqNum(-1),
      mCCodecCallback(callback),
      mFrameIndex(0u),
      mFirstValidFrameIndex(0u),
      mMetaMode(MODE_NONE),
      mInputMetEos(false) {
    mOutputSurface.lock()->maxDequeueBuffers = kSmoothnessFactor + kRenderingDepth;
    {
        Mutexed<Input>::Locked input(mInput);
        input->buffers.reset(new DummyInputBuffers(""));
        input->extraBuffers.flush();
        input->inputDelay = 0u;
        input->pipelineDelay = 0u;
        input->numSlots = kSmoothnessFactor;
        input->numExtraSlots = 0u;
    }
    {
        Mutexed<Output>::Locked output(mOutput);
        output->outputDelay = 0u;
        output->numSlots = kSmoothnessFactor;
    }
}

CCodecBufferChannel::~CCodecBufferChannel() {
    if (mCrypto != nullptr && mDealer != nullptr && mHeapSeqNum >= 0) {
        mCrypto->unsetHeap(mHeapSeqNum);
    }
}

void CCodecBufferChannel::setComponent(
        const std::shared_ptr<Codec2Client::Component> &component) {
    mComponent = component;
    mComponentName = component->getName() + StringPrintf("#%d", int(uintptr_t(component.get()) % 997));
    mName = mComponentName.c_str();
}

status_t CCodecBufferChannel::setInputSurface(
        const std::shared_ptr<InputSurfaceWrapper> &surface) {
    ALOGV("[%s] setInputSurface", mName);
    mInputSurface = surface;
    return mInputSurface->connect(mComponent);
}

status_t CCodecBufferChannel::signalEndOfInputStream() {
    if (mInputSurface == nullptr) {
        return INVALID_OPERATION;
    }
    return mInputSurface->signalEndOfInputStream();
}

status_t CCodecBufferChannel::queueInputBufferInternal(sp<MediaCodecBuffer> buffer) {
    int64_t timeUs;
    CHECK(buffer->meta()->findInt64("timeUs", &timeUs));

    if (mInputMetEos) {
        ALOGD("[%s] buffers after EOS ignored (%lld us)", mName, (long long)timeUs);
        return OK;
    }

    int32_t flags = 0;
    int32_t tmp = 0;
    bool eos = false;
    if (buffer->meta()->findInt32("eos", &tmp) && tmp) {
        eos = true;
        mInputMetEos = true;
        ALOGV("[%s] input EOS", mName);
    }
    if (buffer->meta()->findInt32("csd", &tmp) && tmp) {
        flags |= C2FrameData::FLAG_CODEC_CONFIG;
    }
    ALOGV("[%s] queueInputBuffer: buffer->size() = %zu", mName, buffer->size());
    std::unique_ptr<C2Work> work(new C2Work);
    work->input.ordinal.timestamp = timeUs;
    work->input.ordinal.frameIndex = mFrameIndex++;
    // WORKAROUND: until codecs support handling work after EOS and max output sizing, use timestamp
    // manipulation to achieve image encoding via video codec, and to constrain encoded output.
    // Keep client timestamp in customOrdinal
    work->input.ordinal.customOrdinal = timeUs;
    work->input.buffers.clear();

    uint64_t queuedFrameIndex = work->input.ordinal.frameIndex.peeku();
    std::vector<std::shared_ptr<C2Buffer>> queuedBuffers;
    sp<Codec2Buffer> copy;

    if (buffer->size() > 0u) {
        Mutexed<Input>::Locked input(mInput);
        std::shared_ptr<C2Buffer> c2buffer;
        if (!input->buffers->releaseBuffer(buffer, &c2buffer, false)) {
            return -ENOENT;
        }
        // TODO: we want to delay copying buffers.
        if (input->extraBuffers.numComponentBuffers() < input->numExtraSlots) {
            copy = input->buffers->cloneAndReleaseBuffer(buffer);
            if (copy != nullptr) {
                (void)input->extraBuffers.assignSlot(copy);
                if (!input->extraBuffers.releaseSlot(copy, &c2buffer, false)) {
                    return UNKNOWN_ERROR;
                }
                bool released = input->buffers->releaseBuffer(buffer, nullptr, true);
                ALOGV("[%s] queueInputBuffer: buffer copied; %sreleased",
                      mName, released ? "" : "not ");
                buffer.clear();
            } else {
                ALOGW("[%s] queueInputBuffer: failed to copy a buffer; this may cause input "
                      "buffer starvation on component.", mName);
            }
        }
        work->input.buffers.push_back(c2buffer);
        queuedBuffers.push_back(c2buffer);
    } else if (eos) {
        flags |= C2FrameData::FLAG_END_OF_STREAM;
    }
    work->input.flags = (C2FrameData::flags_t)flags;
    // TODO: fill info's

    work->input.configUpdate = std::move(mParamsToBeSet);
    work->worklets.clear();
    work->worklets.emplace_back(new C2Worklet);

    std::list<std::unique_ptr<C2Work>> items;
    items.push_back(std::move(work));
    mPipelineWatcher.lock()->onWorkQueued(
            queuedFrameIndex,
            std::move(queuedBuffers),
            PipelineWatcher::Clock::now());
    c2_status_t err = mComponent->queue(&items);
    if (err != C2_OK) {
        mPipelineWatcher.lock()->onWorkDone(queuedFrameIndex);
    }

    if (err == C2_OK && eos && buffer->size() > 0u) {
        work.reset(new C2Work);
        work->input.ordinal.timestamp = timeUs;
        work->input.ordinal.frameIndex = mFrameIndex++;
        // WORKAROUND: keep client timestamp in customOrdinal
        work->input.ordinal.customOrdinal = timeUs;
        work->input.buffers.clear();
        work->input.flags = C2FrameData::FLAG_END_OF_STREAM;
        work->worklets.emplace_back(new C2Worklet);

        queuedFrameIndex = work->input.ordinal.frameIndex.peeku();
        queuedBuffers.clear();

        items.clear();
        items.push_back(std::move(work));

        mPipelineWatcher.lock()->onWorkQueued(
                queuedFrameIndex,
                std::move(queuedBuffers),
                PipelineWatcher::Clock::now());
        err = mComponent->queue(&items);
        if (err != C2_OK) {
            mPipelineWatcher.lock()->onWorkDone(queuedFrameIndex);
        }
    }
    if (err == C2_OK) {
        Mutexed<Input>::Locked input(mInput);
        bool released = false;
        if (buffer) {
            released = input->buffers->releaseBuffer(buffer, nullptr, true);
        } else if (copy) {
            released = input->extraBuffers.releaseSlot(copy, nullptr, true);
        }
        ALOGV("[%s] queueInputBuffer: buffer%s %sreleased",
              mName, (buffer == nullptr) ? "(copy)" : "", released ? "" : "not ");
    }

    feedInputBufferIfAvailableInternal();
    return err;
}

status_t CCodecBufferChannel::setParameters(std::vector<std::unique_ptr<C2Param>> &params) {
    QueueGuard guard(mSync);
    if (!guard.isRunning()) {
        ALOGD("[%s] setParameters is only supported in the running state.", mName);
        return -ENOSYS;
    }
    mParamsToBeSet.insert(mParamsToBeSet.end(),
                          std::make_move_iterator(params.begin()),
                          std::make_move_iterator(params.end()));
    params.clear();
    return OK;
}

status_t CCodecBufferChannel::queueInputBuffer(const sp<MediaCodecBuffer> &buffer) {
    QueueGuard guard(mSync);
    if (!guard.isRunning()) {
        ALOGD("[%s] No more buffers should be queued at current state.", mName);
        return -ENOSYS;
    }
    return queueInputBufferInternal(buffer);
}

status_t CCodecBufferChannel::queueSecureInputBuffer(
        const sp<MediaCodecBuffer> &buffer, bool secure, const uint8_t *key,
        const uint8_t *iv, CryptoPlugin::Mode mode, CryptoPlugin::Pattern pattern,
        const CryptoPlugin::SubSample *subSamples, size_t numSubSamples,
        AString *errorDetailMsg) {
    QueueGuard guard(mSync);
    if (!guard.isRunning()) {
        ALOGD("[%s] No more buffers should be queued at current state.", mName);
        return -ENOSYS;
    }

    if (!hasCryptoOrDescrambler()) {
        return -ENOSYS;
    }
    sp<EncryptedLinearBlockBuffer> encryptedBuffer((EncryptedLinearBlockBuffer *)buffer.get());

    ssize_t result = -1;
    ssize_t codecDataOffset = 0;
    if (mCrypto != nullptr) {
        ICrypto::DestinationBuffer destination;
        if (secure) {
            destination.mType = ICrypto::kDestinationTypeNativeHandle;
            destination.mHandle = encryptedBuffer->handle();
        } else {
            destination.mType = ICrypto::kDestinationTypeSharedMemory;
            destination.mSharedMemory = mDecryptDestination;
        }
        ICrypto::SourceBuffer source;
        encryptedBuffer->fillSourceBuffer(&source);
        result = mCrypto->decrypt(
                key, iv, mode, pattern, source, buffer->offset(),
                subSamples, numSubSamples, destination, errorDetailMsg);
        if (result < 0) {
            return result;
        }
        if (destination.mType == ICrypto::kDestinationTypeSharedMemory) {
            encryptedBuffer->copyDecryptedContent(mDecryptDestination, result);
        }
    } else {
        // Here we cast CryptoPlugin::SubSample to hardware::cas::native::V1_0::SubSample
        // directly, the structure definitions should match as checked in DescramblerImpl.cpp.
        hidl_vec<SubSample> hidlSubSamples;
        hidlSubSamples.setToExternal((SubSample *)subSamples, numSubSamples, false /*own*/);

        hardware::cas::native::V1_0::SharedBuffer srcBuffer;
        encryptedBuffer->fillSourceBuffer(&srcBuffer);

        DestinationBuffer dstBuffer;
        if (secure) {
            dstBuffer.type = BufferType::NATIVE_HANDLE;
            dstBuffer.secureMemory = hidl_handle(encryptedBuffer->handle());
        } else {
            dstBuffer.type = BufferType::SHARED_MEMORY;
            dstBuffer.nonsecureMemory = srcBuffer;
        }

        CasStatus status = CasStatus::OK;
        hidl_string detailedError;
        ScramblingControl sctrl = ScramblingControl::UNSCRAMBLED;

        if (key != nullptr) {
            sctrl = (ScramblingControl)key[0];
            // Adjust for the PES offset
            codecDataOffset = key[2] | (key[3] << 8);
        }

        auto returnVoid = mDescrambler->descramble(
                sctrl,
                hidlSubSamples,
                srcBuffer,
                0,
                dstBuffer,
                0,
                [&status, &result, &detailedError] (
                        CasStatus _status, uint32_t _bytesWritten,
                        const hidl_string& _detailedError) {
                    status = _status;
                    result = (ssize_t)_bytesWritten;
                    detailedError = _detailedError;
                });

        if (!returnVoid.isOk() || status != CasStatus::OK || result < 0) {
            ALOGI("[%s] descramble failed, trans=%s, status=%d, result=%zd",
                    mName, returnVoid.description().c_str(), status, result);
            return UNKNOWN_ERROR;
        }

        if (result < codecDataOffset) {
            ALOGD("invalid codec data offset: %zd, result %zd", codecDataOffset, result);
            return BAD_VALUE;
        }

        ALOGV("[%s] descramble succeeded, %zd bytes", mName, result);

        if (dstBuffer.type == BufferType::SHARED_MEMORY) {
            encryptedBuffer->copyDecryptedContentFromMemory(result);
        }
    }

    buffer->setRange(codecDataOffset, result - codecDataOffset);
    return queueInputBufferInternal(buffer);
}

void CCodecBufferChannel::feedInputBufferIfAvailable() {
    QueueGuard guard(mSync);
    if (!guard.isRunning()) {
        ALOGV("[%s] We're not running --- no input buffer reported", mName);
        return;
    }
    feedInputBufferIfAvailableInternal();
}

void CCodecBufferChannel::feedInputBufferIfAvailableInternal() {
    if (mInputMetEos ||
           mReorderStash.lock()->hasPending() ||
           mPipelineWatcher.lock()->pipelineFull()) {
        return;
    } else {
        Mutexed<Output>::Locked output(mOutput);
        if (output->buffers->numClientBuffers() >= output->numSlots) {
            return;
        }
    }
    size_t numInputSlots = mInput.lock()->numSlots;
    for (size_t i = 0; i < numInputSlots; ++i) {
        sp<MediaCodecBuffer> inBuffer;
        size_t index;
        {
            Mutexed<Input>::Locked input(mInput);
            if (input->buffers->numClientBuffers() >= input->numSlots) {
                return;
            }
            if (!input->buffers->requestNewBuffer(&index, &inBuffer)) {
                ALOGV("[%s] no new buffer available", mName);
                break;
            }
        }
        ALOGV("[%s] new input index = %zu [%p]", mName, index, inBuffer.get());
        mCallback->onInputBufferAvailable(index, inBuffer);
    }
}

status_t CCodecBufferChannel::renderOutputBuffer(
        const sp<MediaCodecBuffer> &buffer, int64_t timestampNs) {
    ALOGV("[%s] renderOutputBuffer: %p", mName, buffer.get());
    std::shared_ptr<C2Buffer> c2Buffer;
    bool released = false;
    {
        Mutexed<Output>::Locked output(mOutput);
        if (output->buffers) {
            released = output->buffers->releaseBuffer(buffer, &c2Buffer);
        }
    }
    // NOTE: some apps try to releaseOutputBuffer() with timestamp and/or render
    //       set to true.
    sendOutputBuffers();
    // input buffer feeding may have been gated by pending output buffers
    feedInputBufferIfAvailable();
    if (!c2Buffer) {
        if (released) {
            std::call_once(mRenderWarningFlag, [this] {
                ALOGW("[%s] The app is calling releaseOutputBuffer() with "
                      "timestamp or render=true with non-video buffers. Apps should "
                      "call releaseOutputBuffer() with render=false for those.",
                      mName);
            });
        }
        return INVALID_OPERATION;
    }

#if 0
    const std::vector<std::shared_ptr<const C2Info>> infoParams = c2Buffer->info();
    ALOGV("[%s] queuing gfx buffer with %zu infos", mName, infoParams.size());
    for (const std::shared_ptr<const C2Info> &info : infoParams) {
        AString res;
        for (size_t ix = 0; ix + 3 < info->size(); ix += 4) {
            if (ix) res.append(", ");
            res.append(*((int32_t*)info.get() + (ix / 4)));
        }
        ALOGV("  [%s]", res.c_str());
    }
#endif
    std::shared_ptr<const C2StreamRotationInfo::output> rotation =
        std::static_pointer_cast<const C2StreamRotationInfo::output>(
                c2Buffer->getInfo(C2StreamRotationInfo::output::PARAM_TYPE));
    bool flip = rotation && (rotation->flip & 1);
    uint32_t quarters = ((rotation ? rotation->value : 0) / 90) & 3;
    uint32_t transform = 0;
    switch (quarters) {
        case 0: // no rotation
            transform = flip ? HAL_TRANSFORM_FLIP_H : 0;
            break;
        case 1: // 90 degrees counter-clockwise
            transform = flip ? (HAL_TRANSFORM_FLIP_V | HAL_TRANSFORM_ROT_90)
                    : HAL_TRANSFORM_ROT_270;
            break;
        case 2: // 180 degrees
            transform = flip ? HAL_TRANSFORM_FLIP_V : HAL_TRANSFORM_ROT_180;
            break;
        case 3: // 90 degrees clockwise
            transform = flip ? (HAL_TRANSFORM_FLIP_H | HAL_TRANSFORM_ROT_90)
                    : HAL_TRANSFORM_ROT_90;
            break;
    }

    std::shared_ptr<const C2StreamSurfaceScalingInfo::output> surfaceScaling =
        std::static_pointer_cast<const C2StreamSurfaceScalingInfo::output>(
                c2Buffer->getInfo(C2StreamSurfaceScalingInfo::output::PARAM_TYPE));
    uint32_t videoScalingMode = NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW;
    if (surfaceScaling) {
        videoScalingMode = surfaceScaling->value;
    }

    // Use dataspace from format as it has the default aspects already applied
    android_dataspace_t dataSpace = HAL_DATASPACE_UNKNOWN; // this is 0
    (void)buffer->format()->findInt32("android._dataspace", (int32_t *)&dataSpace);

    // HDR static info
    std::shared_ptr<const C2StreamHdrStaticInfo::output> hdrStaticInfo =
        std::static_pointer_cast<const C2StreamHdrStaticInfo::output>(
                c2Buffer->getInfo(C2StreamHdrStaticInfo::output::PARAM_TYPE));

    // HDR10 plus info
    std::shared_ptr<const C2StreamHdr10PlusInfo::output> hdr10PlusInfo =
        std::static_pointer_cast<const C2StreamHdr10PlusInfo::output>(
                c2Buffer->getInfo(C2StreamHdr10PlusInfo::output::PARAM_TYPE));

    {
        Mutexed<OutputSurface>::Locked output(mOutputSurface);
        if (output->surface == nullptr) {
            ALOGI("[%s] cannot render buffer without surface", mName);
            return OK;
        }
    }

    std::vector<C2ConstGraphicBlock> blocks = c2Buffer->data().graphicBlocks();
    if (blocks.size() != 1u) {
        ALOGD("[%s] expected 1 graphic block, but got %zu", mName, blocks.size());
        return UNKNOWN_ERROR;
    }
    const C2ConstGraphicBlock &block = blocks.front();

    // TODO: revisit this after C2Fence implementation.
    android::IGraphicBufferProducer::QueueBufferInput qbi(
            timestampNs,
            false, // droppable
            dataSpace,
            Rect(blocks.front().crop().left,
                 blocks.front().crop().top,
                 blocks.front().crop().right(),
                 blocks.front().crop().bottom()),
            videoScalingMode,
            transform,
            Fence::NO_FENCE, 0);
    if (hdrStaticInfo || hdr10PlusInfo) {
        HdrMetadata hdr;
        if (hdrStaticInfo) {
            struct android_smpte2086_metadata smpte2086_meta = {
                .displayPrimaryRed = {
                    hdrStaticInfo->mastering.red.x, hdrStaticInfo->mastering.red.y
                },
                .displayPrimaryGreen = {
                    hdrStaticInfo->mastering.green.x, hdrStaticInfo->mastering.green.y
                },
                .displayPrimaryBlue = {
                    hdrStaticInfo->mastering.blue.x, hdrStaticInfo->mastering.blue.y
                },
                .whitePoint = {
                    hdrStaticInfo->mastering.white.x, hdrStaticInfo->mastering.white.y
                },
                .maxLuminance = hdrStaticInfo->mastering.maxLuminance,
                .minLuminance = hdrStaticInfo->mastering.minLuminance,
            };

            struct android_cta861_3_metadata cta861_meta = {
                .maxContentLightLevel = hdrStaticInfo->maxCll,
                .maxFrameAverageLightLevel = hdrStaticInfo->maxFall,
            };

            hdr.validTypes = HdrMetadata::SMPTE2086 | HdrMetadata::CTA861_3;
            hdr.smpte2086 = smpte2086_meta;
            hdr.cta8613 = cta861_meta;
        }
        if (hdr10PlusInfo) {
            hdr.validTypes |= HdrMetadata::HDR10PLUS;
            hdr.hdr10plus.assign(
                    hdr10PlusInfo->m.value,
                    hdr10PlusInfo->m.value + hdr10PlusInfo->flexCount());
        }
        qbi.setHdrMetadata(hdr);
    }
    // we don't have dirty regions
    qbi.setSurfaceDamage(Region::INVALID_REGION);
    android::IGraphicBufferProducer::QueueBufferOutput qbo;
    status_t result = mComponent->queueToOutputSurface(block, qbi, &qbo);
    if (result != OK) {
        ALOGI("[%s] queueBuffer failed: %d", mName, result);
        return result;
    }
    ALOGV("[%s] queue buffer successful", mName);

    int64_t mediaTimeUs = 0;
    (void)buffer->meta()->findInt64("timeUs", &mediaTimeUs);
    mCCodecCallback->onOutputFramesRendered(mediaTimeUs, timestampNs);

    return OK;
}

status_t CCodecBufferChannel::discardBuffer(const sp<MediaCodecBuffer> &buffer) {
    ALOGV("[%s] discardBuffer: %p", mName, buffer.get());
    bool released = false;
    {
        Mutexed<Input>::Locked input(mInput);
        if (input->buffers && input->buffers->releaseBuffer(buffer, nullptr, true)) {
            released = true;
        }
    }
    {
        Mutexed<Output>::Locked output(mOutput);
        if (output->buffers && output->buffers->releaseBuffer(buffer, nullptr)) {
            released = true;
        }
    }
    if (released) {
        sendOutputBuffers();
        feedInputBufferIfAvailable();
    } else {
        ALOGD("[%s] MediaCodec discarded an unknown buffer", mName);
    }
    return OK;
}

void CCodecBufferChannel::getInputBufferArray(Vector<sp<MediaCodecBuffer>> *array) {
    array->clear();
    Mutexed<Input>::Locked input(mInput);

    if (!input->buffers->isArrayMode()) {
        input->buffers = input->buffers->toArrayMode(input->numSlots);
    }

    input->buffers->getArray(array);
}

void CCodecBufferChannel::getOutputBufferArray(Vector<sp<MediaCodecBuffer>> *array) {
    array->clear();
    Mutexed<Output>::Locked output(mOutput);

    if (!output->buffers->isArrayMode()) {
        output->buffers = output->buffers->toArrayMode(output->numSlots);
    }

    output->buffers->getArray(array);
}

status_t CCodecBufferChannel::start(
        const sp<AMessage> &inputFormat, const sp<AMessage> &outputFormat) {
    C2StreamBufferTypeSetting::input iStreamFormat(0u);
    C2StreamBufferTypeSetting::output oStreamFormat(0u);
    C2PortReorderBufferDepthTuning::output reorderDepth;
    C2PortReorderKeySetting::output reorderKey;
    C2PortActualDelayTuning::input inputDelay(0);
    C2PortActualDelayTuning::output outputDelay(0);
    C2ActualPipelineDelayTuning pipelineDelay(0);

    c2_status_t err = mComponent->query(
            {
                &iStreamFormat,
                &oStreamFormat,
                &reorderDepth,
                &reorderKey,
                &inputDelay,
                &pipelineDelay,
                &outputDelay,
            },
            {},
            C2_DONT_BLOCK,
            nullptr);
    if (err == C2_BAD_INDEX) {
        if (!iStreamFormat || !oStreamFormat) {
            return UNKNOWN_ERROR;
        }
    } else if (err != C2_OK) {
        return UNKNOWN_ERROR;
    }

    {
        Mutexed<ReorderStash>::Locked reorder(mReorderStash);
        reorder->clear();
        if (reorderDepth) {
            reorder->setDepth(reorderDepth.value);
        }
        if (reorderKey) {
            reorder->setKey(reorderKey.value);
        }
    }

    uint32_t inputDelayValue = inputDelay ? inputDelay.value : 0;
    uint32_t pipelineDelayValue = pipelineDelay ? pipelineDelay.value : 0;
    uint32_t outputDelayValue = outputDelay ? outputDelay.value : 0;

    size_t numInputSlots = inputDelayValue + pipelineDelayValue + kSmoothnessFactor;
    size_t numOutputSlots = outputDelayValue + kSmoothnessFactor;

    // TODO: get this from input format
    bool secure = mComponent->getName().find(".secure") != std::string::npos;

    std::shared_ptr<C2AllocatorStore> allocatorStore = GetCodec2PlatformAllocatorStore();
    int poolMask = property_get_int32(
            "debug.stagefright.c2-poolmask",
            1 << C2PlatformAllocatorStore::ION |
            1 << C2PlatformAllocatorStore::BUFFERQUEUE);

    if (inputFormat != nullptr) {
        bool graphic = (iStreamFormat.value == C2BufferData::GRAPHIC);
        std::shared_ptr<C2BlockPool> pool;
        {
            Mutexed<BlockPools>::Locked pools(mBlockPools);

            // set default allocator ID.
            pools->inputAllocatorId = (graphic) ? C2PlatformAllocatorStore::GRALLOC
                                                : C2PlatformAllocatorStore::ION;

            // query C2PortAllocatorsTuning::input from component. If an allocator ID is obtained
            // from component, create the input block pool with given ID. Otherwise, use default IDs.
            std::vector<std::unique_ptr<C2Param>> params;
            err = mComponent->query({ },
                                    { C2PortAllocatorsTuning::input::PARAM_TYPE },
                                    C2_DONT_BLOCK,
                                    &params);
            if ((err != C2_OK && err != C2_BAD_INDEX) || params.size() != 1) {
                ALOGD("[%s] Query input allocators returned %zu params => %s (%u)",
                        mName, params.size(), asString(err), err);
            } else if (err == C2_OK && params.size() == 1) {
                C2PortAllocatorsTuning::input *inputAllocators =
                    C2PortAllocatorsTuning::input::From(params[0].get());
                if (inputAllocators && inputAllocators->flexCount() > 0) {
                    std::shared_ptr<C2Allocator> allocator;
                    // verify allocator IDs and resolve default allocator
                    allocatorStore->fetchAllocator(inputAllocators->m.values[0], &allocator);
                    if (allocator) {
                        pools->inputAllocatorId = allocator->getId();
                    } else {
                        ALOGD("[%s] component requested invalid input allocator ID %u",
                                mName, inputAllocators->m.values[0]);
                    }
                }
            }

            // TODO: use C2Component wrapper to associate this pool with ourselves
            if ((poolMask >> pools->inputAllocatorId) & 1) {
                err = CreateCodec2BlockPool(pools->inputAllocatorId, nullptr, &pool);
                ALOGD("[%s] Created input block pool with allocatorID %u => poolID %llu - %s (%d)",
                        mName, pools->inputAllocatorId,
                        (unsigned long long)(pool ? pool->getLocalId() : 111000111),
                        asString(err), err);
            } else {
                err = C2_NOT_FOUND;
            }
            if (err != C2_OK) {
                C2BlockPool::local_id_t inputPoolId =
                    graphic ? C2BlockPool::BASIC_GRAPHIC : C2BlockPool::BASIC_LINEAR;
                err = GetCodec2BlockPool(inputPoolId, nullptr, &pool);
                ALOGD("[%s] Using basic input block pool with poolID %llu => got %llu - %s (%d)",
                        mName, (unsigned long long)inputPoolId,
                        (unsigned long long)(pool ? pool->getLocalId() : 111000111),
                        asString(err), err);
                if (err != C2_OK) {
                    return NO_MEMORY;
                }
            }
            pools->inputPool = pool;
        }

        bool forceArrayMode = false;
        Mutexed<Input>::Locked input(mInput);
        input->inputDelay = inputDelayValue;
        input->pipelineDelay = pipelineDelayValue;
        input->numSlots = numInputSlots;
        input->extraBuffers.flush();
        input->numExtraSlots = 0u;
        if (graphic) {
            if (mInputSurface) {
                input->buffers.reset(new DummyInputBuffers(mName));
            } else if (mMetaMode == MODE_ANW) {
                input->buffers.reset(new GraphicMetadataInputBuffers(mName));
                // This is to ensure buffers do not get released prematurely.
                // TODO: handle this without going into array mode
                forceArrayMode = true;
            } else {
                input->buffers.reset(new GraphicInputBuffers(numInputSlots, mName));
            }
        } else {
            if (hasCryptoOrDescrambler()) {
                int32_t capacity = kLinearBufferSize;
                (void)inputFormat->findInt32(KEY_MAX_INPUT_SIZE, &capacity);
                if ((size_t)capacity > kMaxLinearBufferSize) {
                    ALOGD("client requested %d, capped to %zu", capacity, kMaxLinearBufferSize);
                    capacity = kMaxLinearBufferSize;
                }
                if (mDealer == nullptr) {
                    mDealer = new MemoryDealer(
                            align(capacity, MemoryDealer::getAllocationAlignment())
                                * (numInputSlots + 1),
                            "EncryptedLinearInputBuffers");
                    mDecryptDestination = mDealer->allocate((size_t)capacity);
                }
                if (mCrypto != nullptr && mHeapSeqNum < 0) {
                    mHeapSeqNum = mCrypto->setHeap(mDealer->getMemoryHeap());
                } else {
                    mHeapSeqNum = -1;
                }
                input->buffers.reset(new EncryptedLinearInputBuffers(
                        secure, mDealer, mCrypto, mHeapSeqNum, (size_t)capacity,
                        numInputSlots, mName));
                forceArrayMode = true;
            } else {
                input->buffers.reset(new LinearInputBuffers(mName));
            }
        }
        input->buffers->setFormat(inputFormat);

        if (err == C2_OK) {
            input->buffers->setPool(pool);
        } else {
            // TODO: error
        }

        if (forceArrayMode) {
            input->buffers = input->buffers->toArrayMode(numInputSlots);
        }
    }

    if (outputFormat != nullptr) {
        sp<IGraphicBufferProducer> outputSurface;
        uint32_t outputGeneration;
        {
            Mutexed<OutputSurface>::Locked output(mOutputSurface);
            output->maxDequeueBuffers = numOutputSlots +
                    reorderDepth.value + kRenderingDepth;
            if (!secure) {
                output->maxDequeueBuffers += numInputSlots;
            }
            outputSurface = output->surface ?
                    output->surface->getIGraphicBufferProducer() : nullptr;
            if (outputSurface) {
                output->surface->setMaxDequeuedBufferCount(output->maxDequeueBuffers);
            }
            outputGeneration = output->generation;
        }

        bool graphic = (oStreamFormat.value == C2BufferData::GRAPHIC);
        C2BlockPool::local_id_t outputPoolId_;

        {
            Mutexed<BlockPools>::Locked pools(mBlockPools);

            // set default allocator ID.
            pools->outputAllocatorId = (graphic) ? C2PlatformAllocatorStore::GRALLOC
                                                 : C2PlatformAllocatorStore::ION;

            // query C2PortAllocatorsTuning::output from component, or use default allocator if
            // unsuccessful.
            std::vector<std::unique_ptr<C2Param>> params;
            err = mComponent->query({ },
                                    { C2PortAllocatorsTuning::output::PARAM_TYPE },
                                    C2_DONT_BLOCK,
                                    &params);
            if ((err != C2_OK && err != C2_BAD_INDEX) || params.size() != 1) {
                ALOGD("[%s] Query output allocators returned %zu params => %s (%u)",
                        mName, params.size(), asString(err), err);
            } else if (err == C2_OK && params.size() == 1) {
                C2PortAllocatorsTuning::output *outputAllocators =
                    C2PortAllocatorsTuning::output::From(params[0].get());
                if (outputAllocators && outputAllocators->flexCount() > 0) {
                    std::shared_ptr<C2Allocator> allocator;
                    // verify allocator IDs and resolve default allocator
                    allocatorStore->fetchAllocator(outputAllocators->m.values[0], &allocator);
                    if (allocator) {
                        pools->outputAllocatorId = allocator->getId();
                    } else {
                        ALOGD("[%s] component requested invalid output allocator ID %u",
                                mName, outputAllocators->m.values[0]);
                    }
                }
            }

            // use bufferqueue if outputting to a surface.
            // query C2PortSurfaceAllocatorTuning::output from component, or use default allocator
            // if unsuccessful.
            if (outputSurface) {
                params.clear();
                err = mComponent->query({ },
                                        { C2PortSurfaceAllocatorTuning::output::PARAM_TYPE },
                                        C2_DONT_BLOCK,
                                        &params);
                if ((err != C2_OK && err != C2_BAD_INDEX) || params.size() != 1) {
                    ALOGD("[%s] Query output surface allocator returned %zu params => %s (%u)",
                            mName, params.size(), asString(err), err);
                } else if (err == C2_OK && params.size() == 1) {
                    C2PortSurfaceAllocatorTuning::output *surfaceAllocator =
                        C2PortSurfaceAllocatorTuning::output::From(params[0].get());
                    if (surfaceAllocator) {
                        std::shared_ptr<C2Allocator> allocator;
                        // verify allocator IDs and resolve default allocator
                        allocatorStore->fetchAllocator(surfaceAllocator->value, &allocator);
                        if (allocator) {
                            pools->outputAllocatorId = allocator->getId();
                        } else {
                            ALOGD("[%s] component requested invalid surface output allocator ID %u",
                                    mName, surfaceAllocator->value);
                            err = C2_BAD_VALUE;
                        }
                    }
                }
                if (pools->outputAllocatorId == C2PlatformAllocatorStore::GRALLOC
                        && err != C2_OK
                        && ((poolMask >> C2PlatformAllocatorStore::BUFFERQUEUE) & 1)) {
                    pools->outputAllocatorId = C2PlatformAllocatorStore::BUFFERQUEUE;
                }
            }

            if ((poolMask >> pools->outputAllocatorId) & 1) {
                err = mComponent->createBlockPool(
                        pools->outputAllocatorId, &pools->outputPoolId, &pools->outputPoolIntf);
                ALOGI("[%s] Created output block pool with allocatorID %u => poolID %llu - %s",
                        mName, pools->outputAllocatorId,
                        (unsigned long long)pools->outputPoolId,
                        asString(err));
            } else {
                err = C2_NOT_FOUND;
            }
            if (err != C2_OK) {
                // use basic pool instead
                pools->outputPoolId =
                    graphic ? C2BlockPool::BASIC_GRAPHIC : C2BlockPool::BASIC_LINEAR;
            }

            // Configure output block pool ID as parameter C2PortBlockPoolsTuning::output to
            // component.
            std::unique_ptr<C2PortBlockPoolsTuning::output> poolIdsTuning =
                    C2PortBlockPoolsTuning::output::AllocUnique({ pools->outputPoolId });

            std::vector<std::unique_ptr<C2SettingResult>> failures;
            err = mComponent->config({ poolIdsTuning.get() }, C2_MAY_BLOCK, &failures);
            ALOGD("[%s] Configured output block pool ids %llu => %s",
                    mName, (unsigned long long)poolIdsTuning->m.values[0], asString(err));
            outputPoolId_ = pools->outputPoolId;
        }

        Mutexed<Output>::Locked output(mOutput);
        output->outputDelay = outputDelayValue;
        output->numSlots = numOutputSlots;
        if (graphic) {
            if (outputSurface) {
                output->buffers.reset(new GraphicOutputBuffers(mName));
            } else {
                output->buffers.reset(new RawGraphicOutputBuffers(numOutputSlots, mName));
            }
        } else {
            output->buffers.reset(new LinearOutputBuffers(mName));
        }
        output->buffers->setFormat(outputFormat);


        // Try to set output surface to created block pool if given.
        if (outputSurface) {
            mComponent->setOutputSurface(
                    outputPoolId_,
                    outputSurface,
                    outputGeneration);
        }

        if (oStreamFormat.value == C2BufferData::LINEAR
                && mComponentName.find("c2.qti.") == std::string::npos) {
            // WORKAROUND: if we're using early CSD workaround we convert to
            //             array mode, to appease apps assuming the output
            //             buffers to be of the same size.
            output->buffers = output->buffers->toArrayMode(numOutputSlots);

            int32_t channelCount;
            int32_t sampleRate;
            if (outputFormat->findInt32(KEY_CHANNEL_COUNT, &channelCount)
                    && outputFormat->findInt32(KEY_SAMPLE_RATE, &sampleRate)) {
                int32_t delay = 0;
                int32_t padding = 0;;
                if (!outputFormat->findInt32("encoder-delay", &delay)) {
                    delay = 0;
                }
                if (!outputFormat->findInt32("encoder-padding", &padding)) {
                    padding = 0;
                }
                if (delay || padding) {
                    // We need write access to the buffers, and we're already in
                    // array mode.
                    output->buffers->initSkipCutBuffer(delay, padding, sampleRate, channelCount);
                }
            }
        }
    }

    // Set up pipeline control. This has to be done after mInputBuffers and
    // mOutputBuffers are initialized to make sure that lingering callbacks
    // about buffers from the previous generation do not interfere with the
    // newly initialized pipeline capacity.

    {
        Mutexed<PipelineWatcher>::Locked watcher(mPipelineWatcher);
        watcher->inputDelay(inputDelayValue)
                .pipelineDelay(pipelineDelayValue)
                .outputDelay(outputDelayValue)
                .smoothnessFactor(kSmoothnessFactor);
        watcher->flush();
    }

    mInputMetEos = false;
    mSync.start();
    return OK;
}

status_t CCodecBufferChannel::requestInitialInputBuffers() {
    if (mInputSurface) {
        return OK;
    }

    C2StreamBufferTypeSetting::output oStreamFormat(0u);
    c2_status_t err = mComponent->query({ &oStreamFormat }, {}, C2_DONT_BLOCK, nullptr);
    if (err != C2_OK) {
        return UNKNOWN_ERROR;
    }
    size_t numInputSlots = mInput.lock()->numSlots;
    std::vector<sp<MediaCodecBuffer>> toBeQueued;
    for (size_t i = 0; i < numInputSlots; ++i) {
        size_t index;
        sp<MediaCodecBuffer> buffer;
        {
            Mutexed<Input>::Locked input(mInput);
            if (!input->buffers->requestNewBuffer(&index, &buffer)) {
                if (i == 0) {
                    ALOGW("[%s] start: cannot allocate memory at all", mName);
                    return NO_MEMORY;
                } else {
                    ALOGV("[%s] start: cannot allocate memory, only %zu buffers allocated",
                            mName, i);
                }
                break;
            }
        }
        if (buffer) {
            Mutexed<std::list<sp<ABuffer>>>::Locked configs(mFlushedConfigs);
            ALOGV("[%s] input buffer %zu available", mName, index);
            bool post = true;
            if (!configs->empty()) {
                sp<ABuffer> config = configs->front();
                configs->pop_front();
                if (buffer->capacity() >= config->size()) {
                    memcpy(buffer->base(), config->data(), config->size());
                    buffer->setRange(0, config->size());
                    buffer->meta()->clear();
                    buffer->meta()->setInt64("timeUs", 0);
                    buffer->meta()->setInt32("csd", 1);
                    post = false;
                } else {
                    ALOGD("[%s] buffer capacity too small for the config (%zu < %zu)",
                            mName, buffer->capacity(), config->size());
                }
            } else if (oStreamFormat.value == C2BufferData::LINEAR && i == 0
                    && mComponentName.find("c2.qti.") == std::string::npos) {
                // WORKAROUND: Some apps expect CSD available without queueing
                //             any input. Queue an empty buffer to get the CSD.
                buffer->setRange(0, 0);
                buffer->meta()->clear();
                buffer->meta()->setInt64("timeUs", 0);
                post = false;
            }
            if (post) {
                mCallback->onInputBufferAvailable(index, buffer);
            } else {
                toBeQueued.emplace_back(buffer);
            }
        }
    }
    for (const sp<MediaCodecBuffer> &buffer : toBeQueued) {
        if (queueInputBufferInternal(buffer) != OK) {
            ALOGV("[%s] Error while queueing initial buffers", mName);
        }
    }
    return OK;
}

void CCodecBufferChannel::stop() {
    mSync.stop();
    mFirstValidFrameIndex = mFrameIndex.load(std::memory_order_relaxed);
    if (mInputSurface != nullptr) {
        mInputSurface.reset();
    }
}

void CCodecBufferChannel::flush(const std::list<std::unique_ptr<C2Work>> &flushedWork) {
    ALOGV("[%s] flush", mName);
    {
        Mutexed<std::list<sp<ABuffer>>>::Locked configs(mFlushedConfigs);
        for (const std::unique_ptr<C2Work> &work : flushedWork) {
            if (!(work->input.flags & C2FrameData::FLAG_CODEC_CONFIG)) {
                continue;
            }
            if (work->input.buffers.empty()
                    || work->input.buffers.front()->data().linearBlocks().empty()) {
                ALOGD("[%s] no linear codec config data found", mName);
                continue;
            }
            C2ReadView view =
                    work->input.buffers.front()->data().linearBlocks().front().map().get();
            if (view.error() != C2_OK) {
                ALOGD("[%s] failed to map flushed codec config data: %d", mName, view.error());
                continue;
            }
            configs->push_back(ABuffer::CreateAsCopy(view.data(), view.capacity()));
            ALOGV("[%s] stashed flushed codec config data (size=%u)", mName, view.capacity());
        }
    }
    {
        Mutexed<Input>::Locked input(mInput);
        input->buffers->flush();
        input->extraBuffers.flush();
    }
    {
        Mutexed<Output>::Locked output(mOutput);
        output->buffers->flush(flushedWork);
    }
    mReorderStash.lock()->flush();
    mPipelineWatcher.lock()->flush();
}

void CCodecBufferChannel::onWorkDone(
        std::unique_ptr<C2Work> work, const sp<AMessage> &outputFormat,
        const C2StreamInitDataInfo::output *initData) {
    if (handleWork(std::move(work), outputFormat, initData)) {
        feedInputBufferIfAvailable();
    }
}

void CCodecBufferChannel::onInputBufferDone(
        uint64_t frameIndex, size_t arrayIndex) {
    if (mInputSurface) {
        return;
    }
    std::shared_ptr<C2Buffer> buffer =
            mPipelineWatcher.lock()->onInputBufferReleased(frameIndex, arrayIndex);
    bool newInputSlotAvailable;
    {
        Mutexed<Input>::Locked input(mInput);
        newInputSlotAvailable = input->buffers->expireComponentBuffer(buffer);
        if (!newInputSlotAvailable) {
            (void)input->extraBuffers.expireComponentBuffer(buffer);
        }
    }
    if (newInputSlotAvailable) {
        feedInputBufferIfAvailable();
    }
}

bool CCodecBufferChannel::handleWork(
        std::unique_ptr<C2Work> work,
        const sp<AMessage> &outputFormat,
        const C2StreamInitDataInfo::output *initData) {
    if (outputFormat != nullptr) {
        Mutexed<Output>::Locked output(mOutput);
        ALOGD("[%s] onWorkDone: output format changed to %s",
                mName, outputFormat->debugString().c_str());
        output->buffers->setFormat(outputFormat);

        AString mediaType;
        if (outputFormat->findString(KEY_MIME, &mediaType)
                && mediaType == MIMETYPE_AUDIO_RAW) {
            int32_t channelCount;
            int32_t sampleRate;
            if (outputFormat->findInt32(KEY_CHANNEL_COUNT, &channelCount)
                    && outputFormat->findInt32(KEY_SAMPLE_RATE, &sampleRate)) {
                output->buffers->updateSkipCutBuffer(sampleRate, channelCount);
            }
        }
    }

    if ((work->input.ordinal.frameIndex - mFirstValidFrameIndex.load()).peek() < 0) {
        // Discard frames from previous generation.
        ALOGD("[%s] Discard frames from previous generation.", mName);
        return false;
    }

    if (mInputSurface == nullptr && (work->worklets.size() != 1u
            || !work->worklets.front()
            || !(work->worklets.front()->output.flags & C2FrameData::FLAG_INCOMPLETE))) {
        mPipelineWatcher.lock()->onWorkDone(work->input.ordinal.frameIndex.peeku());
    }

    if (work->result == C2_NOT_FOUND) {
        ALOGD("[%s] flushed work; ignored.", mName);
        return true;
    }

    if (work->result != C2_OK) {
        ALOGD("[%s] work failed to complete: %d", mName, work->result);
        mCCodecCallback->onError(work->result, ACTION_CODE_FATAL);
        return false;
    }

    // NOTE: MediaCodec usage supposedly have only one worklet
    if (work->worklets.size() != 1u) {
        ALOGI("[%s] onWorkDone: incorrect number of worklets: %zu",
                mName, work->worklets.size());
        mCCodecCallback->onError(UNKNOWN_ERROR, ACTION_CODE_FATAL);
        return false;
    }

    const std::unique_ptr<C2Worklet> &worklet = work->worklets.front();

    std::shared_ptr<C2Buffer> buffer;
    // NOTE: MediaCodec usage supposedly have only one output stream.
    if (worklet->output.buffers.size() > 1u) {
        ALOGI("[%s] onWorkDone: incorrect number of output buffers: %zu",
                mName, worklet->output.buffers.size());
        mCCodecCallback->onError(UNKNOWN_ERROR, ACTION_CODE_FATAL);
        return false;
    } else if (worklet->output.buffers.size() == 1u) {
        buffer = worklet->output.buffers[0];
        if (!buffer) {
            ALOGD("[%s] onWorkDone: nullptr found in buffers; ignored.", mName);
        }
    }

    std::optional<uint32_t> newInputDelay, newPipelineDelay;
    while (!worklet->output.configUpdate.empty()) {
        std::unique_ptr<C2Param> param;
        worklet->output.configUpdate.back().swap(param);
        worklet->output.configUpdate.pop_back();
        switch (param->coreIndex().coreIndex()) {
            case C2PortReorderBufferDepthTuning::CORE_INDEX: {
                C2PortReorderBufferDepthTuning::output reorderDepth;
                if (reorderDepth.updateFrom(*param)) {
                    bool secure = mComponent->getName().find(".secure") != std::string::npos;
                    mReorderStash.lock()->setDepth(reorderDepth.value);
                    ALOGV("[%s] onWorkDone: updated reorder depth to %u",
                          mName, reorderDepth.value);
                    size_t numOutputSlots = mOutput.lock()->numSlots;
                    size_t numInputSlots = mInput.lock()->numSlots;
                    Mutexed<OutputSurface>::Locked output(mOutputSurface);
                    output->maxDequeueBuffers = numOutputSlots +
                            reorderDepth.value + kRenderingDepth;
                    if (!secure) {
                        output->maxDequeueBuffers += numInputSlots;
                    }
                    if (output->surface) {
                        output->surface->setMaxDequeuedBufferCount(output->maxDequeueBuffers);
                    }
                } else {
                    ALOGD("[%s] onWorkDone: failed to read reorder depth", mName);
                }
                break;
            }
            case C2PortReorderKeySetting::CORE_INDEX: {
                C2PortReorderKeySetting::output reorderKey;
                if (reorderKey.updateFrom(*param)) {
                    mReorderStash.lock()->setKey(reorderKey.value);
                    ALOGV("[%s] onWorkDone: updated reorder key to %u",
                          mName, reorderKey.value);
                } else {
                    ALOGD("[%s] onWorkDone: failed to read reorder key", mName);
                }
                break;
            }
            case C2PortActualDelayTuning::CORE_INDEX: {
                if (param->isGlobal()) {
                    C2ActualPipelineDelayTuning pipelineDelay;
                    if (pipelineDelay.updateFrom(*param)) {
                        ALOGV("[%s] onWorkDone: updating pipeline delay %u",
                              mName, pipelineDelay.value);
                        newPipelineDelay = pipelineDelay.value;
                        (void)mPipelineWatcher.lock()->pipelineDelay(pipelineDelay.value);
                    }
                }
                if (param->forInput()) {
                    C2PortActualDelayTuning::input inputDelay;
                    if (inputDelay.updateFrom(*param)) {
                        ALOGV("[%s] onWorkDone: updating input delay %u",
                              mName, inputDelay.value);
                        newInputDelay = inputDelay.value;
                        (void)mPipelineWatcher.lock()->inputDelay(inputDelay.value);
                    }
                }
                if (param->forOutput()) {
                    C2PortActualDelayTuning::output outputDelay;
                    if (outputDelay.updateFrom(*param)) {
                        ALOGV("[%s] onWorkDone: updating output delay %u",
                              mName, outputDelay.value);
                        bool secure = mComponent->getName().find(".secure") != std::string::npos;
                        (void)mPipelineWatcher.lock()->outputDelay(outputDelay.value);

                        bool outputBuffersChanged = false;
                        size_t numOutputSlots = 0;
                        size_t numInputSlots = mInput.lock()->numSlots;
                        {
                            Mutexed<Output>::Locked output(mOutput);
                            output->outputDelay = outputDelay.value;
                            numOutputSlots = outputDelay.value + kSmoothnessFactor;
                            if (output->numSlots < numOutputSlots) {
                                output->numSlots = numOutputSlots;
                                if (output->buffers->isArrayMode()) {
                                    OutputBuffersArray *array =
                                        (OutputBuffersArray *)output->buffers.get();
                                    ALOGV("[%s] onWorkDone: growing output buffer array to %zu",
                                          mName, numOutputSlots);
                                    array->grow(numOutputSlots);
                                    outputBuffersChanged = true;
                                }
                            }
                            numOutputSlots = output->numSlots;
                        }

                        if (outputBuffersChanged) {
                            mCCodecCallback->onOutputBuffersChanged();
                        }

                        uint32_t depth = mReorderStash.lock()->depth();
                        Mutexed<OutputSurface>::Locked output(mOutputSurface);
                        output->maxDequeueBuffers = numOutputSlots + depth + kRenderingDepth;
                        if (!secure) {
                            output->maxDequeueBuffers += numInputSlots;
                        }
                        if (output->surface) {
                            output->surface->setMaxDequeuedBufferCount(output->maxDequeueBuffers);
                        }
                    }
                }
                break;
            }
            default:
                ALOGV("[%s] onWorkDone: unrecognized config update (%08X)",
                      mName, param->index());
                break;
        }
    }
    if (newInputDelay || newPipelineDelay) {
        Mutexed<Input>::Locked input(mInput);
        size_t newNumSlots =
            newInputDelay.value_or(input->inputDelay) +
            newPipelineDelay.value_or(input->pipelineDelay) +
            kSmoothnessFactor;
        if (input->buffers->isArrayMode()) {
            if (input->numSlots >= newNumSlots) {
                input->numExtraSlots = 0;
            } else {
                input->numExtraSlots = newNumSlots - input->numSlots;
            }
            ALOGV("[%s] onWorkDone: updated number of extra slots to %zu (input array mode)",
                  mName, input->numExtraSlots);
        } else {
            input->numSlots = newNumSlots;
        }
    }

    int32_t flags = 0;
    if (worklet->output.flags & C2FrameData::FLAG_END_OF_STREAM) {
        flags |= MediaCodec::BUFFER_FLAG_EOS;
        ALOGV("[%s] onWorkDone: output EOS", mName);
    }

    sp<MediaCodecBuffer> outBuffer;
    size_t index;

    // WORKAROUND: adjust output timestamp based on client input timestamp and codec
    // input timestamp. Codec output timestamp (in the timestamp field) shall correspond to
    // the codec input timestamp, but client output timestamp should (reported in timeUs)
    // shall correspond to the client input timesamp (in customOrdinal). By using the
    // delta between the two, this allows for some timestamp deviation - e.g. if one input
    // produces multiple output.
    c2_cntr64_t timestamp =
        worklet->output.ordinal.timestamp + work->input.ordinal.customOrdinal
                - work->input.ordinal.timestamp;
    if (mInputSurface != nullptr) {
        // When using input surface we need to restore the original input timestamp.
        timestamp = work->input.ordinal.customOrdinal;
    }
    ALOGV("[%s] onWorkDone: input %lld, codec %lld => output %lld => %lld",
          mName,
          work->input.ordinal.customOrdinal.peekll(),
          work->input.ordinal.timestamp.peekll(),
          worklet->output.ordinal.timestamp.peekll(),
          timestamp.peekll());

    if (initData != nullptr) {
        Mutexed<Output>::Locked output(mOutput);
        if (output->buffers->registerCsd(initData, &index, &outBuffer) == OK) {
            outBuffer->meta()->setInt64("timeUs", timestamp.peek());
            outBuffer->meta()->setInt32("flags", MediaCodec::BUFFER_FLAG_CODECCONFIG);
            ALOGV("[%s] onWorkDone: csd index = %zu [%p]", mName, index, outBuffer.get());

            output.unlock();
            mCallback->onOutputBufferAvailable(index, outBuffer);
        } else {
            ALOGD("[%s] onWorkDone: unable to register csd", mName);
            output.unlock();
            mCCodecCallback->onError(UNKNOWN_ERROR, ACTION_CODE_FATAL);
            return false;
        }
    }

    if (!buffer && !flags) {
        ALOGV("[%s] onWorkDone: Not reporting output buffer (%lld)",
              mName, work->input.ordinal.frameIndex.peekull());
        return true;
    }

    if (buffer) {
        for (const std::shared_ptr<const C2Info> &info : buffer->info()) {
            // TODO: properly translate these to metadata
            switch (info->coreIndex().coreIndex()) {
                case C2StreamPictureTypeMaskInfo::CORE_INDEX:
                    if (((C2StreamPictureTypeMaskInfo *)info.get())->value & C2Config::SYNC_FRAME) {
                        flags |= MediaCodec::BUFFER_FLAG_SYNCFRAME;
                    }
                    break;
                default:
                    break;
            }
        }
    }

    {
        Mutexed<ReorderStash>::Locked reorder(mReorderStash);
        reorder->emplace(buffer, timestamp.peek(), flags, worklet->output.ordinal);
        if (flags & MediaCodec::BUFFER_FLAG_EOS) {
            // Flush reorder stash
            reorder->setDepth(0);
        }
    }
    sendOutputBuffers();
    return true;
}

void CCodecBufferChannel::sendOutputBuffers() {
    ReorderStash::Entry entry;
    sp<MediaCodecBuffer> outBuffer;
    size_t index;

    while (true) {
        Mutexed<ReorderStash>::Locked reorder(mReorderStash);
        if (!reorder->hasPending()) {
            break;
        }
        if (!reorder->pop(&entry)) {
            break;
        }

        Mutexed<Output>::Locked output(mOutput);
        status_t err = output->buffers->registerBuffer(entry.buffer, &index, &outBuffer);
        if (err != OK) {
            bool outputBuffersChanged = false;
            if (err != WOULD_BLOCK) {
                if (!output->buffers->isArrayMode()) {
                    output->buffers = output->buffers->toArrayMode(output->numSlots);
                }
                OutputBuffersArray *array = (OutputBuffersArray *)output->buffers.get();
                array->realloc(entry.buffer);
                outputBuffersChanged = true;
            }
            ALOGV("[%s] sendOutputBuffers: unable to register output buffer", mName);
            reorder->defer(entry);

            output.unlock();
            reorder.unlock();

            if (outputBuffersChanged) {
                mCCodecCallback->onOutputBuffersChanged();
            }
            return;
        }
        output.unlock();
        reorder.unlock();

        outBuffer->meta()->setInt64("timeUs", entry.timestamp);
        outBuffer->meta()->setInt32("flags", entry.flags);
        ALOGV("[%s] sendOutputBuffers: out buffer index = %zu [%p] => %p + %zu (%lld)",
                mName, index, outBuffer.get(), outBuffer->data(), outBuffer->size(),
                (long long)entry.timestamp);
        mCallback->onOutputBufferAvailable(index, outBuffer);
    }
}

status_t CCodecBufferChannel::setSurface(const sp<Surface> &newSurface) {
    static std::atomic_uint32_t surfaceGeneration{0};
    uint32_t generation = (getpid() << 10) |
            ((surfaceGeneration.fetch_add(1, std::memory_order_relaxed) + 1)
                & ((1 << 10) - 1));

    sp<IGraphicBufferProducer> producer;
    if (newSurface) {
        newSurface->setScalingMode(NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
        newSurface->setDequeueTimeout(kDequeueTimeoutNs);
        newSurface->setMaxDequeuedBufferCount(mOutputSurface.lock()->maxDequeueBuffers);
        producer = newSurface->getIGraphicBufferProducer();
        producer->setGenerationNumber(generation);
    } else {
        ALOGE("[%s] setting output surface to null", mName);
        return INVALID_OPERATION;
    }

    std::shared_ptr<Codec2Client::Configurable> outputPoolIntf;
    C2BlockPool::local_id_t outputPoolId;
    {
        Mutexed<BlockPools>::Locked pools(mBlockPools);
        outputPoolId = pools->outputPoolId;
        outputPoolIntf = pools->outputPoolIntf;
    }

    if (outputPoolIntf) {
        if (mComponent->setOutputSurface(
                outputPoolId,
                producer,
                generation) != C2_OK) {
            ALOGI("[%s] setSurface: component setOutputSurface failed", mName);
            return INVALID_OPERATION;
        }
    }

    {
        Mutexed<OutputSurface>::Locked output(mOutputSurface);
        output->surface = newSurface;
        output->generation = generation;
    }

    return OK;
}

PipelineWatcher::Clock::duration CCodecBufferChannel::elapsed() {
    // When client pushed EOS, we want all the work to be done quickly.
    // Otherwise, component may have stalled work due to input starvation up to
    // the sum of the delay in the pipeline.
    size_t n = 0;
    if (!mInputMetEos) {
        size_t outputDelay = mOutput.lock()->outputDelay;
        Mutexed<Input>::Locked input(mInput);
        n = input->inputDelay + input->pipelineDelay + outputDelay;
    }
    return mPipelineWatcher.lock()->elapsed(PipelineWatcher::Clock::now(), n);
}

void CCodecBufferChannel::setMetaMode(MetaMode mode) {
    mMetaMode = mode;
}

status_t toStatusT(c2_status_t c2s, c2_operation_t c2op) {
    // C2_OK is always translated to OK.
    if (c2s == C2_OK) {
        return OK;
    }

    // Operation-dependent translation
    // TODO: Add as necessary
    switch (c2op) {
    case C2_OPERATION_Component_start:
        switch (c2s) {
        case C2_NO_MEMORY:
            return NO_MEMORY;
        default:
            return UNKNOWN_ERROR;
        }
    default:
        break;
    }

    // Backup operation-agnostic translation
    switch (c2s) {
    case C2_BAD_INDEX:
        return BAD_INDEX;
    case C2_BAD_VALUE:
        return BAD_VALUE;
    case C2_BLOCKING:
        return WOULD_BLOCK;
    case C2_DUPLICATE:
        return ALREADY_EXISTS;
    case C2_NO_INIT:
        return NO_INIT;
    case C2_NO_MEMORY:
        return NO_MEMORY;
    case C2_NOT_FOUND:
        return NAME_NOT_FOUND;
    case C2_TIMED_OUT:
        return TIMED_OUT;
    case C2_BAD_STATE:
    case C2_CANCELED:
    case C2_CANNOT_DO:
    case C2_CORRUPTED:
    case C2_OMITTED:
    case C2_REFUSED:
        return UNKNOWN_ERROR;
    default:
        return -static_cast<status_t>(c2s);
    }
}

}  // namespace android
