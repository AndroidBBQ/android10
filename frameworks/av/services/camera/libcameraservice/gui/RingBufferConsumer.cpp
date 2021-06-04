/*
 * Copyright (C) 2013 The Android Open Source Project
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
#define LOG_TAG "RingBufferConsumer"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <inttypes.h>

#include <utils/Log.h>

#include <gui/RingBufferConsumer.h>

#define BI_LOGV(x, ...) ALOGV("[%s] " x, mName.string(), ##__VA_ARGS__)
#define BI_LOGD(x, ...) ALOGD("[%s] " x, mName.string(), ##__VA_ARGS__)
#define BI_LOGI(x, ...) ALOGI("[%s] " x, mName.string(), ##__VA_ARGS__)
#define BI_LOGW(x, ...) ALOGW("[%s] " x, mName.string(), ##__VA_ARGS__)
#define BI_LOGE(x, ...) ALOGE("[%s] " x, mName.string(), ##__VA_ARGS__)

#undef assert
#define assert(x) ALOG_ASSERT((x), #x)

typedef android::RingBufferConsumer::PinnedBufferItem PinnedBufferItem;

namespace android {

RingBufferConsumer::RingBufferConsumer(const sp<IGraphicBufferConsumer>& consumer,
        uint64_t consumerUsage,
        int bufferCount) :
    ConsumerBase(consumer),
    mBufferCount(bufferCount),
    mLatestTimestamp(0)
{
    mConsumer->setConsumerUsageBits(consumerUsage);
    mConsumer->setMaxAcquiredBufferCount(bufferCount);

    assert(bufferCount > 0);
}

RingBufferConsumer::~RingBufferConsumer() {
}

void RingBufferConsumer::setName(const String8& name) {
    Mutex::Autolock _l(mMutex);
    mName = name;
    mConsumer->setConsumerName(name);
}

sp<PinnedBufferItem> RingBufferConsumer::pinSelectedBuffer(
        const RingBufferComparator& filter,
        bool waitForFence) {

    sp<PinnedBufferItem> pinnedBuffer;

    {
        List<RingBufferItem>::iterator it, end, accIt;
        BufferInfo acc, cur;
        BufferInfo* accPtr = NULL;

        Mutex::Autolock _l(mMutex);

        for (it = mBufferItemList.begin(), end = mBufferItemList.end();
             it != end;
             ++it) {

            const RingBufferItem& item = *it;

            cur.mCrop = item.mCrop;
            cur.mTransform = item.mTransform;
            cur.mScalingMode = item.mScalingMode;
            cur.mTimestamp = item.mTimestamp;
            cur.mFrameNumber = item.mFrameNumber;
            cur.mPinned = item.mPinCount > 0;

            int ret = filter.compare(accPtr, &cur);

            if (ret == 0) {
                accPtr = NULL;
            } else if (ret > 0) {
                acc = cur;
                accPtr = &acc;
                accIt = it;
            } // else acc = acc
        }

        if (!accPtr) {
            return NULL;
        }

        pinnedBuffer = new PinnedBufferItem(this, *accIt);
        pinBufferLocked(pinnedBuffer->getBufferItem());

    } // end scope of mMutex autolock

    if (waitForFence) {
        status_t err = pinnedBuffer->getBufferItem().mFence->waitForever(
                "RingBufferConsumer::pinSelectedBuffer");
        if (err != OK) {
            BI_LOGE("Failed to wait for fence of acquired buffer: %s (%d)",
                    strerror(-err), err);
        }
    }

    return pinnedBuffer;
}

status_t RingBufferConsumer::clear() {

    status_t err;
    Mutex::Autolock _l(mMutex);

    BI_LOGV("%s", __FUNCTION__);

    // Avoid annoying log warnings by returning early
    if (mBufferItemList.size() == 0) {
        return OK;
    }

    do {
        size_t pinnedFrames = 0;
        err = releaseOldestBufferLocked(&pinnedFrames);

        if (err == NO_BUFFER_AVAILABLE) {
            assert(pinnedFrames == mBufferItemList.size());
            break;
        }

        if (err == NOT_ENOUGH_DATA) {
            // Fine. Empty buffer item list.
            break;
        }

        if (err != OK) {
            BI_LOGE("Clear failed, could not release buffer");
            return err;
        }

    } while(true);

    return OK;
}

nsecs_t RingBufferConsumer::getLatestTimestamp() {
    Mutex::Autolock _l(mMutex);
    if (mBufferItemList.size() == 0) {
        return 0;
    }
    return mLatestTimestamp;
}

void RingBufferConsumer::pinBufferLocked(const BufferItem& item) {
    List<RingBufferItem>::iterator it, end;

    for (it = mBufferItemList.begin(), end = mBufferItemList.end();
         it != end;
         ++it) {

        RingBufferItem& find = *it;
        if (item.mGraphicBuffer == find.mGraphicBuffer) {
            find.mPinCount++;
            break;
        }
    }

    if (it == end) {
        BI_LOGE("Failed to pin buffer (timestamp %" PRId64 ", framenumber %" PRIu64 ")",
                 item.mTimestamp, item.mFrameNumber);
    } else {
        BI_LOGV("Pinned buffer (frame %" PRIu64 ", timestamp %" PRId64 ")",
                item.mFrameNumber, item.mTimestamp);
    }
}

status_t RingBufferConsumer::releaseOldestBufferLocked(size_t* pinnedFrames) {
    status_t err = OK;

    List<RingBufferItem>::iterator it, end, accIt;

    it = mBufferItemList.begin();
    end = mBufferItemList.end();
    accIt = end;

    if (it == end) {
        /**
         * This is fine. We really care about being able to acquire a buffer
         * successfully after this function completes, not about it releasing
         * some buffer.
         */
        BI_LOGV("%s: No buffers yet acquired, can't release anything",
              __FUNCTION__);
        return NOT_ENOUGH_DATA;
    }

    for (; it != end; ++it) {
        RingBufferItem& find = *it;

        if (find.mPinCount > 0) {
            if (pinnedFrames != NULL) {
                ++(*pinnedFrames);
            }
            // Filter out pinned frame when searching for buffer to release
            continue;
        }

        if (find.mTimestamp < accIt->mTimestamp || accIt == end) {
            accIt = it;
        }
    }

    if (accIt != end) {
        RingBufferItem& item = *accIt;

        // In case the object was never pinned, pass the acquire fence
        // back to the release fence. If the fence was already waited on,
        // it'll just be a no-op to wait on it again.

        // item.mGraphicBuffer was populated with the proper graphic-buffer
        // at acquire even if it was previously acquired
        err = addReleaseFenceLocked(item.mSlot,
                item.mGraphicBuffer, item.mFence);

        if (err != OK) {
            BI_LOGE("Failed to add release fence to buffer "
                    "(timestamp %" PRId64 ", framenumber %" PRIu64,
                    item.mTimestamp, item.mFrameNumber);
            return err;
        }

        BI_LOGV("Attempting to release buffer timestamp %" PRId64 ", frame %" PRIu64,
                item.mTimestamp, item.mFrameNumber);

        // item.mGraphicBuffer was populated with the proper graphic-buffer
        // at acquire even if it was previously acquired
        err = releaseBufferLocked(item.mSlot, item.mGraphicBuffer,
                                  EGL_NO_DISPLAY,
                                  EGL_NO_SYNC_KHR);
        if (err != OK) {
            BI_LOGE("Failed to release buffer: %s (%d)",
                    strerror(-err), err);
            return err;
        }

        BI_LOGV("Buffer timestamp %" PRId64 ", frame %" PRIu64 " evicted",
                item.mTimestamp, item.mFrameNumber);

        mBufferItemList.erase(accIt);
    } else {
        BI_LOGW("All buffers pinned, could not find any to release");
        return NO_BUFFER_AVAILABLE;

    }

    return OK;
}

void RingBufferConsumer::onFrameAvailable(const BufferItem& item) {
    status_t err;

    {
        Mutex::Autolock _l(mMutex);

        /**
         * Release oldest frame
         */
        if (mBufferItemList.size() >= (size_t)mBufferCount) {
            err = releaseOldestBufferLocked(/*pinnedFrames*/NULL);
            assert(err != NOT_ENOUGH_DATA);

            // TODO: implement the case for NO_BUFFER_AVAILABLE
            assert(err != NO_BUFFER_AVAILABLE);
            if (err != OK) {
                return;
            }
            // TODO: in unpinBuffer rerun this routine if we had buffers
            // we could've locked but didn't because there was no space
        }

        RingBufferItem& item = *mBufferItemList.insert(mBufferItemList.end(),
                                                       RingBufferItem());

        /**
         * Acquire new frame
         */
        err = acquireBufferLocked(&item, 0);
        if (err != OK) {
            if (err != NO_BUFFER_AVAILABLE) {
                BI_LOGE("Error acquiring buffer: %s (%d)", strerror(err), err);
            }

            mBufferItemList.erase(--mBufferItemList.end());
            return;
        }

        BI_LOGV("New buffer acquired (timestamp %" PRId64 "), "
                "buffer items %zu out of %d",
                item.mTimestamp,
                mBufferItemList.size(), mBufferCount);

        if (item.mTimestamp < mLatestTimestamp) {
            BI_LOGE("Timestamp  decreases from %" PRId64 " to %" PRId64,
                    mLatestTimestamp, item.mTimestamp);
        }

        mLatestTimestamp = item.mTimestamp;

        item.mGraphicBuffer = mSlots[item.mSlot].mGraphicBuffer;
    } // end of mMutex lock

    ConsumerBase::onFrameAvailable(item);
}

void RingBufferConsumer::unpinBuffer(const BufferItem& item) {
    Mutex::Autolock _l(mMutex);

    List<RingBufferItem>::iterator it, end, accIt;

    for (it = mBufferItemList.begin(), end = mBufferItemList.end();
         it != end;
         ++it) {

        RingBufferItem& find = *it;
        if (item.mGraphicBuffer == find.mGraphicBuffer) {
            status_t res = addReleaseFenceLocked(item.mSlot,
                    item.mGraphicBuffer, item.mFence);

            if (res != OK) {
                BI_LOGE("Failed to add release fence to buffer "
                        "(timestamp %" PRId64 ", framenumber %" PRIu64,
                        item.mTimestamp, item.mFrameNumber);
                return;
            }

            find.mPinCount--;
            break;
        }
    }

    if (it == end) {
        // This should never happen. If it happens, we have a bug.
        BI_LOGE("Failed to unpin buffer (timestamp %" PRId64 ", framenumber %" PRIu64 ")",
                 item.mTimestamp, item.mFrameNumber);
    } else {
        BI_LOGV("Unpinned buffer (timestamp %" PRId64 ", framenumber %" PRIu64 ")",
                 item.mTimestamp, item.mFrameNumber);
    }
}

status_t RingBufferConsumer::setDefaultBufferSize(uint32_t w, uint32_t h) {
    Mutex::Autolock _l(mMutex);
    return mConsumer->setDefaultBufferSize(w, h);
}

status_t RingBufferConsumer::setDefaultBufferFormat(uint32_t defaultFormat) {
    Mutex::Autolock _l(mMutex);
    return mConsumer->setDefaultBufferFormat(defaultFormat);
}

status_t RingBufferConsumer::setConsumerUsage(uint64_t usage) {
    Mutex::Autolock _l(mMutex);
    return mConsumer->setConsumerUsageBits(usage);
}

} // namespace android
