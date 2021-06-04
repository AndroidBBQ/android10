/*
 * Copyright (C) 2011 The Android Open Source Project
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

//#define USE_LOG SLAndroidLogLevel_Verbose

#include "sles_allinclusive.h"
#include "android/BufferQueueSource.h"

#include <media/stagefright/foundation/ADebug.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace android {


const SLuint32 BufferQueueSource::kItemProcessed[NB_BUFFEREVENT_ITEM_FIELDS] = {
        SL_ANDROID_ITEMKEY_BUFFERQUEUEEVENT, // item key
        sizeof(SLuint32),                    // item size
        SL_ANDROIDBUFFERQUEUEEVENT_PROCESSED // item data
};


BufferQueueSource::BufferQueueSource(IAndroidBufferQueue *androidBufferQueue) :
          mAndroidBufferQueueSource(androidBufferQueue),
          mStreamToBqOffset(0),
          mEosReached(false)
{
}


BufferQueueSource::~BufferQueueSource() {
    SL_LOGD("BufferQueueSource::~BufferQueueSource");
}


//--------------------------------------------------------------------------
status_t BufferQueueSource::initCheck() const {
    return mAndroidBufferQueueSource != NULL ? OK : NO_INIT;
}

ssize_t BufferQueueSource::readAt(off64_t offset, void *data, size_t size) {
    SL_LOGD("BufferQueueSource::readAt(offset=%lld, data=%p, size=%d)", offset, data, size);

    if (mEosReached) {
        // once EOS has been received from the buffer queue, you can't read anymore
        return 0;
    }

    ssize_t readSize;
    slAndroidBufferQueueCallback callback = NULL;
    void* pBufferContext, *pBufferData, *callbackPContext;
    uint32_t dataSize, dataUsed;

    interface_lock_exclusive(mAndroidBufferQueueSource);

    if (mAndroidBufferQueueSource->mState.count == 0) {
        readSize = 0;
    } else {
        assert(mAndroidBufferQueueSource->mFront != mAndroidBufferQueueSource->mRear);

        AdvancedBufferHeader *oldFront = mAndroidBufferQueueSource->mFront;
        AdvancedBufferHeader *newFront = &oldFront[1];

        // where to read from
        char *pSrc = NULL;
        // can this read operation cause us to call the buffer queue callback
        // (either because there was a command with no data, or all the data has been consumed)
        bool queueCallbackCandidate = false;

        // consume events when starting to read data from a buffer for the first time
        if (oldFront->mDataSizeConsumed == 0) {
            if (oldFront->mItems.mAdtsCmdData.mAdtsCmdCode & ANDROID_ADTSEVENT_EOS) {
                mEosReached = true;
                // EOS has no associated data
                queueCallbackCandidate = true;
            }
            oldFront->mItems.mAdtsCmdData.mAdtsCmdCode = ANDROID_ADTSEVENT_NONE;
        }

        //assert(mStreamToBqOffset <= offset);
        CHECK_LE(mStreamToBqOffset, offset);

        if (offset + (off64_t) size <= mStreamToBqOffset + oldFront->mDataSize) {
            pSrc = ((char*)oldFront->mDataBuffer) + (offset - mStreamToBqOffset);

            if (offset - mStreamToBqOffset + size == oldFront->mDataSize) {
                // consumed buffer entirely
                oldFront->mDataSizeConsumed = oldFront->mDataSize;
                mStreamToBqOffset += oldFront->mDataSize;
                queueCallbackCandidate = true;

                // move queue to next buffer
                if (newFront == &mAndroidBufferQueueSource->
                        mBufferArray[mAndroidBufferQueueSource->mNumBuffers + 1]) {
                    // reached the end, circle back
                    newFront = mAndroidBufferQueueSource->mBufferArray;
                }
                mAndroidBufferQueueSource->mFront = newFront;
                // update the queue state
                mAndroidBufferQueueSource->mState.count--;
                mAndroidBufferQueueSource->mState.index++;
                SL_LOGV("BufferQueueSource moving to next buffer");
            }
        }

        // consume data: copy to given destination
        if (NULL != pSrc) {
            memcpy(data, pSrc, size);
            readSize = size;
        } else {
            readSize = 0;
        }

        if (queueCallbackCandidate) {
            // data has been consumed, and the buffer queue state has been updated
            // we will notify the client if applicable
            if (mAndroidBufferQueueSource->mCallbackEventsMask &
                    SL_ANDROIDBUFFERQUEUEEVENT_PROCESSED) {
                callback = mAndroidBufferQueueSource->mCallback;
                // save callback data while under lock
                callbackPContext = mAndroidBufferQueueSource->mContext;
                pBufferContext = (void *)oldFront->mBufferContext;
                pBufferData    = (void *)oldFront->mDataBuffer;
                dataSize       = oldFront->mDataSize;
                dataUsed       = oldFront->mDataSizeConsumed;
            }
        }
    }

    interface_unlock_exclusive(mAndroidBufferQueueSource);

    // notify client
    if (NULL != callback) {
        SLresult result = (*callback)(&mAndroidBufferQueueSource->mItf, callbackPContext,
                pBufferContext, pBufferData, dataSize, dataUsed,
                // no messages during playback other than marking the buffer as processed
                (const SLAndroidBufferItem*)(&kItemProcessed) /* pItems */,
                NB_BUFFEREVENT_ITEM_FIELDS * sizeof(SLuint32) /* itemsLength */ );
        if (SL_RESULT_SUCCESS != result) {
            // Reserved for future use
            SL_LOGW("Unsuccessful result %d returned from AndroidBufferQueueCallback", result);
        }
    }

    return readSize;
}


status_t BufferQueueSource::getSize(off64_t *size) {
    SL_LOGD("BufferQueueSource::getSize()");
    // we're streaming, we don't know how much there is
    *size = 0;
    return ERROR_UNSUPPORTED;
}

}  // namespace android
