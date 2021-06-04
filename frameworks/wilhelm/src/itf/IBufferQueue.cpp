/*
 * Copyright (C) 2010 The Android Open Source Project
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

/* BufferQueue implementation */

#include "sles_allinclusive.h"


/** Determine the state of the audio player or audio recorder associated with a buffer queue.
 *  Note that PLAYSTATE and RECORDSTATE values are equivalent (where PLAYING == RECORDING).
 */

static SLuint32 getAssociatedState(IBufferQueue *thiz)
{
    SLuint32 state;
    switch (InterfaceToObjectID(thiz)) {
    case SL_OBJECTID_AUDIOPLAYER:
        state = ((CAudioPlayer *) thiz->mThis)->mPlay.mState;
        break;
    case SL_OBJECTID_AUDIORECORDER:
        state = ((CAudioRecorder *) thiz->mThis)->mRecord.mState;
        break;
    default:
        // unreachable, but just in case we will assume it is stopped
        assert(SL_BOOLEAN_FALSE);
        state = SL_PLAYSTATE_STOPPED;
        break;
    }
    return state;
}


SLresult IBufferQueue_Enqueue(SLBufferQueueItf self, const void *pBuffer, SLuint32 size)
{
    SL_ENTER_INTERFACE

    // Note that Enqueue while a Clear is pending is equivalent to Enqueue followed by Clear

    if (NULL == pBuffer || 0 == size) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IBufferQueue *thiz = (IBufferQueue *) self;
        interface_lock_exclusive(thiz);
        BufferHeader *oldRear = thiz->mRear, *newRear;
        if ((newRear = oldRear + 1) == &thiz->mArray[thiz->mNumBuffers + 1]) {
            newRear = thiz->mArray;
        }
        if (newRear == thiz->mFront) {
            result = SL_RESULT_BUFFER_INSUFFICIENT;
        } else {
            oldRear->mBuffer = pBuffer;
            oldRear->mSize = size;
            thiz->mRear = newRear;
            ++thiz->mState.count;
            result = SL_RESULT_SUCCESS;
        }
        // set enqueue attribute if state is PLAYING and the first buffer is enqueued
        interface_unlock_exclusive_attributes(thiz, ((SL_RESULT_SUCCESS == result) &&
            (1 == thiz->mState.count) && (SL_PLAYSTATE_PLAYING == getAssociatedState(thiz))) ?
            ATTR_BQ_ENQUEUE : ATTR_NONE);
    }
    SL_LEAVE_INTERFACE
}


SLresult IBufferQueue_Clear(SLBufferQueueItf self)
{
    SL_ENTER_INTERFACE

    result = SL_RESULT_SUCCESS;
    IBufferQueue *thiz = (IBufferQueue *) self;
    interface_lock_exclusive(thiz);

#ifdef ANDROID
    if (SL_OBJECTID_AUDIOPLAYER == InterfaceToObjectID(thiz)) {
        CAudioPlayer *audioPlayer = (CAudioPlayer *) thiz->mThis;
        // flush associated audio player
        result = android_audioPlayer_bufferQueue_onClear(audioPlayer);
    }
    // flush our buffers
    if (SL_RESULT_SUCCESS == result) {
        thiz->mFront = &thiz->mArray[0];
        thiz->mRear = &thiz->mArray[0];
        thiz->mState.count = 0;
        thiz->mState.playIndex = 0;
        thiz->mSizeConsumed = 0;
        thiz->mCallbackPending = false;
    }
#endif

#ifdef USE_OUTPUTMIXEXT
    // mixer might be reading from the front buffer, so tread carefully here
    // NTH asynchronous cancel instead of blocking until mixer acknowledges
    thiz->mClearRequested = SL_BOOLEAN_TRUE;
    do {
        interface_cond_wait(thiz);
    } while (thiz->mClearRequested);
#endif

    interface_unlock_exclusive(thiz);

    // there is a danger that a buffer is still held by the callback thread
    // after we leave IBufferQueue_Clear().  This buffer will not be written
    // into anymore, but it is possible that it will be returned via callback.
    SL_LEAVE_INTERFACE
}


static SLresult IBufferQueue_GetState(SLBufferQueueItf self, SLBufferQueueState *pState)
{
    SL_ENTER_INTERFACE

    // Note that GetState while a Clear is pending is equivalent to GetState before the Clear

    if (NULL == pState) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IBufferQueue *thiz = (IBufferQueue *) self;
        SLBufferQueueState state;
        interface_lock_shared(thiz);
#ifdef __cplusplus // FIXME Is this a compiler bug?
        state.count = thiz->mState.count;
        state.playIndex = thiz->mState.playIndex;
#else
        state = thiz->mState;
#endif
        interface_unlock_shared(thiz);
        *pState = state;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


SLresult IBufferQueue_RegisterCallback(SLBufferQueueItf self,
    slBufferQueueCallback callback, void *pContext)
{
    SL_ENTER_INTERFACE

    IBufferQueue *thiz = (IBufferQueue *) self;
    interface_lock_exclusive(thiz);
    // verify pre-condition that media object is in the SL_PLAYSTATE_STOPPED state
    if (SL_PLAYSTATE_STOPPED == getAssociatedState(thiz)) {
        thiz->mCallback = callback;
        thiz->mContext = pContext;
        result = SL_RESULT_SUCCESS;
    } else {
        result = SL_RESULT_PRECONDITIONS_VIOLATED;
    }
    interface_unlock_exclusive(thiz);

    SL_LEAVE_INTERFACE
}


static const struct SLBufferQueueItf_ IBufferQueue_Itf = {
    IBufferQueue_Enqueue,
    IBufferQueue_Clear,
    IBufferQueue_GetState,
    IBufferQueue_RegisterCallback
};

void IBufferQueue_init(void *self)
{
    IBufferQueue *thiz = (IBufferQueue *) self;
    thiz->mItf = &IBufferQueue_Itf;
    thiz->mState.count = 0;
    thiz->mState.playIndex = 0;
    thiz->mCallback = NULL;
    thiz->mContext = NULL;
    thiz->mNumBuffers = 0;
    thiz->mClearRequested = SL_BOOLEAN_FALSE;
    thiz->mArray = NULL;
    thiz->mFront = NULL;
    thiz->mRear = NULL;
#ifdef ANDROID
    thiz->mSizeConsumed = 0;
    thiz->mCallbackPending = false;
#endif
    BufferHeader *bufferHeader = thiz->mTypical;
    unsigned i;
    for (i = 0; i < BUFFER_HEADER_TYPICAL+1; ++i, ++bufferHeader) {
        bufferHeader->mBuffer = NULL;
        bufferHeader->mSize = 0;
    }
}


/** \brief Interface deinitialization hook called by IObject::Destroy.
 *  Free the buffer queue, if it was larger than typical.
 */

void IBufferQueue_deinit(void *self)
{
    IBufferQueue *thiz = (IBufferQueue *) self;
    if ((NULL != thiz->mArray) && (thiz->mArray != thiz->mTypical)) {
        free(thiz->mArray);
        thiz->mArray = NULL;
    }
}
