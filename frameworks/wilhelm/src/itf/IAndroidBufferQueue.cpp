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

/* AndroidBufferQueue implementation */

//#define USE_LOG SLAndroidLogLevel_Verbose

#include "sles_allinclusive.h"
// for AAC ADTS verification on enqueue:
#include "android/include/AacBqToPcmCbRenderer.h"

/**
 * Determine the state of the audio player or media player associated with a buffer queue.
 *  Note that PLAYSTATE and RECORDSTATE values are equivalent (where PLAYING == RECORDING).
 */

static SLuint32 getAssociatedState(IAndroidBufferQueue *thiz)
{
    SLuint32 state;
    switch (InterfaceToObjectID(thiz)) {
      case XA_OBJECTID_MEDIAPLAYER:
        state = ((CMediaPlayer *) thiz->mThis)->mPlay.mState;
        break;
      case SL_OBJECTID_AUDIOPLAYER:
        state = ((CAudioPlayer *) thiz->mThis)->mPlay.mState;
        break;
      default:
        // unreachable, but just in case we will assume it is stopped
        assert(SL_BOOLEAN_FALSE);
        state = SL_PLAYSTATE_STOPPED;
        break;
    }
    return state;
}


/**
 * parse and set the items associated with the given buffer, based on the buffer type,
 * which determines the set of authorized items and format
 */
static SLresult setItems(SLuint32 dataLength,
        const SLAndroidBufferItem *pItems, SLuint32 itemsLength,
        SLuint16 bufferType, AdvancedBufferHeader *pBuff, bool *pEOS)
{
    // reset item structure based on type
    switch (bufferType) {
      case kAndroidBufferTypeMpeg2Ts:
        pBuff->mItems.mTsCmdData.mTsCmdCode = ANDROID_MP2TSEVENT_NONE;
        pBuff->mItems.mTsCmdData.mPts = 0;
        break;
      case kAndroidBufferTypeAacadts:
        pBuff->mItems.mAdtsCmdData.mAdtsCmdCode = ANDROID_ADTSEVENT_NONE;
        break;
      case kAndroidBufferTypeInvalid:
      default:
        // shouldn't happen, but just in case clear out the item structure
        memset(&pBuff->mItems, 0, sizeof(AdvancedBufferItems));
        return SL_RESULT_INTERNAL_ERROR;
    }

    // process all items in the array; if no items then we break out of loop immediately
    while (itemsLength > 0) {

        // remaining length must be large enough for one full item without any associated data
        if (itemsLength < sizeof(SLAndroidBufferItem)) {
            SL_LOGE("Partial item at end of array");
            return SL_RESULT_PARAMETER_INVALID;
        }
        itemsLength -= sizeof(SLAndroidBufferItem);

        // remaining length must be large enough for data with current item and alignment padding
        SLuint32 itemDataSizeWithAlignmentPadding = (pItems->itemSize + 3) & ~3;
        if (itemsLength < itemDataSizeWithAlignmentPadding) {
            SL_LOGE("Partial item data at end of array");
            return SL_RESULT_PARAMETER_INVALID;
        }
        itemsLength -= itemDataSizeWithAlignmentPadding;

        // parse item data based on type
        switch (bufferType) {

          case kAndroidBufferTypeMpeg2Ts: {
            switch (pItems->itemKey) {

              case SL_ANDROID_ITEMKEY_EOS:
                pBuff->mItems.mTsCmdData.mTsCmdCode |= ANDROID_MP2TSEVENT_EOS;
                //SL_LOGD("Found EOS event=%d", pBuff->mItems.mTsCmdData.mTsCmdCode);
                if (pItems->itemSize != 0) {
                    SL_LOGE("Invalid item parameter size %u for EOS", pItems->itemSize);
                    return SL_RESULT_PARAMETER_INVALID;
                }
                break;

              case SL_ANDROID_ITEMKEY_DISCONTINUITY:
                if (pItems->itemSize == 0) {
                    pBuff->mItems.mTsCmdData.mTsCmdCode |= ANDROID_MP2TSEVENT_DISCONTINUITY;
                    //SL_LOGD("Found DISCONTINUITYevent=%d", pBuff->mItems.mTsCmdData.mTsCmdCode);
                } else if (pItems->itemSize == sizeof(SLAuint64)) {
                    pBuff->mItems.mTsCmdData.mTsCmdCode |= ANDROID_MP2TSEVENT_DISCON_NEWPTS;
                    pBuff->mItems.mTsCmdData.mPts = *((SLAuint64*)pItems->itemData);
                    //SL_LOGD("Found PTS=%lld", pBuff->mItems.mTsCmdData.mPts);
                } else {
                    SL_LOGE("Invalid item parameter size %u for MPEG-2 PTS", pItems->itemSize);
                    return SL_RESULT_PARAMETER_INVALID;
                }
                break;

              case SL_ANDROID_ITEMKEY_FORMAT_CHANGE:
                // distinguish between a "full" format change and one where it says what changed
                if (pItems->itemSize == 0) {
                    SL_LOGV("Received format change with no data == full format change");
                    pBuff->mItems.mTsCmdData.mTsCmdCode |= ANDROID_MP2TSEVENT_FORMAT_CHANGE_FULL;
                } else if (pItems->itemSize == sizeof(SLuint32)) {
                    XAuint32 formatData = *((XAuint32*)pItems->itemData);
                    // intentionally only supporting video change when reading which specific
                    //    stream has changed, interpret other changes as full change
                    if (formatData == XA_ANDROID_FORMATCHANGE_ITEMDATA_VIDEO) {
                        pBuff->mItems.mTsCmdData.mTsCmdCode |=
                                ANDROID_MP2TSEVENT_FORMAT_CHANGE_VIDEO;
                        SL_LOGV("Received video format change");
                    } else {
                        // note that we don't support specifying
                        //    ANDROID_MP2TSEVENT_FORMAT_CHANGE_FULL by having all bits of
                        //    the data mask set, we default to it with unsupported masks
                        SL_LOGE("Received format change with unsupported data, ignoring data");
                        pBuff->mItems.mTsCmdData.mTsCmdCode |=
                                ANDROID_MP2TSEVENT_FORMAT_CHANGE_FULL;
                    }
                } else {
                    SL_LOGE("Received format change with invalid data size, ignoring data");
                    pBuff->mItems.mTsCmdData.mTsCmdCode |= ANDROID_MP2TSEVENT_FORMAT_CHANGE_FULL;
                }
                break;

              default:
                // unknown item key
                SL_LOGE("Unknown item key %u with size %u", pItems->itemKey, pItems->itemSize);
                return SL_RESULT_PARAMETER_INVALID;

            }// switch (pItems->itemKey)
          } break;

          case kAndroidBufferTypeAacadts: {
            switch (pItems->itemKey) {

              case SL_ANDROID_ITEMKEY_EOS:
                pBuff->mItems.mAdtsCmdData.mAdtsCmdCode |= ANDROID_ADTSEVENT_EOS;
                if (pItems->itemSize != 0) {
                    SL_LOGE("Invalid item parameter size %u for EOS", pItems->itemSize);
                    return SL_RESULT_PARAMETER_INVALID;
                }
                break;

              default:
                // unknown item key
                SL_LOGE("Unknown item key %u with size %u", pItems->itemKey, pItems->itemSize);
                return SL_RESULT_PARAMETER_INVALID;

            }// switch (pItems->itemKey)
          } break;

          case kAndroidBufferTypeInvalid:
          default:
            // not reachable as we checked this earlier
            return SL_RESULT_INTERNAL_ERROR;

        }// switch (bufferType)

        // skip past this item, including data with alignment padding
        pItems = (SLAndroidBufferItem *) ((char *) pItems +
                sizeof(SLAndroidBufferItem) + itemDataSizeWithAlignmentPadding);
    }

    // now check for invalid combinations of items
    switch (bufferType) {

      case kAndroidBufferTypeMpeg2Ts: {
        // supported Mpeg2Ts commands are mutually exclusive
        switch (pBuff->mItems.mTsCmdData.mTsCmdCode) {
          // single items are allowed
          case ANDROID_MP2TSEVENT_EOS:
            if (dataLength > 0) {
                SL_LOGE("Can't enqueue non-zero data with EOS");
                return SL_RESULT_PRECONDITIONS_VIOLATED;
            }
            *pEOS = true;
            break;
          case ANDROID_MP2TSEVENT_NONE:
          case ANDROID_MP2TSEVENT_DISCONTINUITY:
          case ANDROID_MP2TSEVENT_DISCON_NEWPTS:
          case ANDROID_MP2TSEVENT_FORMAT_CHANGE_FULL:
          case ANDROID_MP2TSEVENT_FORMAT_CHANGE_VIDEO:
            break;
          // no combinations are allowed
          default:
            SL_LOGE("Invalid combination of items");
            return SL_RESULT_PARAMETER_INVALID;
        }
      } break;

      case kAndroidBufferTypeAacadts: {
        // only one item supported, and thus no combination check needed
        if (pBuff->mItems.mAdtsCmdData.mAdtsCmdCode == ANDROID_ADTSEVENT_EOS) {
            if (dataLength > 0) {
                SL_LOGE("Can't enqueue non-zero data with EOS");
                return SL_RESULT_PRECONDITIONS_VIOLATED;
            }
            *pEOS = true;
        }
      } break;

      case kAndroidBufferTypeInvalid:
      default:
        // not reachable as we checked this earlier
        return SL_RESULT_INTERNAL_ERROR;
    }

    return SL_RESULT_SUCCESS;
}


static SLresult IAndroidBufferQueue_RegisterCallback(SLAndroidBufferQueueItf self,
        slAndroidBufferQueueCallback callback, void *pContext)
{
    SL_ENTER_INTERFACE

    IAndroidBufferQueue *thiz = (IAndroidBufferQueue *) self;

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


static SLresult IAndroidBufferQueue_Clear(SLAndroidBufferQueueItf self)
{
    SL_ENTER_INTERFACE
    result = SL_RESULT_SUCCESS;

    IAndroidBufferQueue *thiz = (IAndroidBufferQueue *) self;

    interface_lock_exclusive(thiz);

    // reset the queue pointers
    thiz->mFront = &thiz->mBufferArray[0];
    thiz->mRear = &thiz->mBufferArray[0];
    // reset the queue state
    thiz->mState.count = 0;
    thiz->mState.index = 0;

    // object-specific behavior for a clear
    switch (InterfaceToObjectID(thiz)) {
    case SL_OBJECTID_AUDIOPLAYER:
        android_audioPlayer_androidBufferQueue_clear_l((CAudioPlayer*) thiz->mThis);
        break;
    case XA_OBJECTID_MEDIAPLAYER:
        android_Player_androidBufferQueue_clear_l((CMediaPlayer*) thiz->mThis);
        break;
    default:
        result = SL_RESULT_PARAMETER_INVALID;
    }

    interface_unlock_exclusive(thiz);

    SL_LEAVE_INTERFACE
}


static SLresult IAndroidBufferQueue_Enqueue(SLAndroidBufferQueueItf self,
        void *pBufferContext,
        void *pData,
        SLuint32 dataLength,
        const SLAndroidBufferItem *pItems,
        SLuint32 itemsLength)
{
    SL_ENTER_INTERFACE
    SL_LOGD("IAndroidBufferQueue_Enqueue pData=%p dataLength=%d", pData, dataLength);

    if ((dataLength > 0) && (NULL == pData)) {
        SL_LOGE("Enqueue failure: non-zero data length %u but NULL data pointer", dataLength);
        result = SL_RESULT_PARAMETER_INVALID;
    } else if ((itemsLength > 0) && (NULL == pItems)) {
        SL_LOGE("Enqueue failure: non-zero items length %u but NULL items pointer", itemsLength);
        result = SL_RESULT_PARAMETER_INVALID;
    } else if ((0 == dataLength) && (0 == itemsLength)) {
        // no data and no msg
        SL_LOGE("Enqueue failure: trying to enqueue buffer with no data and no items.");
        result = SL_RESULT_PARAMETER_INVALID;
    // Note that a non-NULL data pointer with zero data length is allowed.
    // We track that data pointer as it moves through the queue
    // to assist the application in accounting for data buffers.
    // A non-NULL items pointer with zero items length is also allowed, but has no value.
    } else {
        IAndroidBufferQueue *thiz = (IAndroidBufferQueue *) self;

        // buffer size check, can be done outside of lock because buffer type can't change
        switch (thiz->mBufferType) {
          case kAndroidBufferTypeMpeg2Ts:
            if (dataLength % MPEG2_TS_PACKET_SIZE == 0) {
                // The downstream Stagefright MPEG-2 TS parser is sensitive to format errors,
                // so do a quick sanity check beforehand on the first packet of the buffer.
                // We don't check all the packets to avoid thrashing the data cache.
                if ((dataLength > 0) && (*(SLuint8 *)pData != MPEG2_TS_PACKET_SYNC)) {
                    SL_LOGE("Error enqueueing MPEG-2 TS data: incorrect packet sync");
                    result = SL_RESULT_CONTENT_CORRUPTED;
                    SL_LEAVE_INTERFACE
                }
                break;
            }
            SL_LOGE("Error enqueueing MPEG-2 TS data: size must be a multiple of %d (packet size)",
                    MPEG2_TS_PACKET_SIZE);
            result = SL_RESULT_PARAMETER_INVALID;
            SL_LEAVE_INTERFACE
            break;
          case kAndroidBufferTypeAacadts:
            // zero dataLength is permitted in case of EOS command only
            if (dataLength > 0) {
                result = android::AacBqToPcmCbRenderer::validateBufferStartEndOnFrameBoundaries(
                    pData, dataLength);
                if (SL_RESULT_SUCCESS != result) {
                    SL_LOGE("Error enqueueing ADTS data: data must start and end on frame "
                            "boundaries");
                    SL_LEAVE_INTERFACE
                }
            }
            break;
          case kAndroidBufferTypeInvalid:
          default:
            result = SL_RESULT_PARAMETER_INVALID;
            SL_LEAVE_INTERFACE
        }

        interface_lock_exclusive(thiz);

        AdvancedBufferHeader *oldRear = thiz->mRear, *newRear;
        if ((newRear = oldRear + 1) == &thiz->mBufferArray[thiz->mNumBuffers + 1]) {
            newRear = thiz->mBufferArray;
        }
        if (thiz->mEOS) {
            SL_LOGE("Can't enqueue after EOS");
            result = SL_RESULT_PRECONDITIONS_VIOLATED;
        } else if (newRear == thiz->mFront) {
            result = SL_RESULT_BUFFER_INSUFFICIENT;
        } else {
            // set oldRear->mItems based on items
            result = setItems(dataLength, pItems, itemsLength, thiz->mBufferType, oldRear,
                    &thiz->mEOS);
            if (SL_RESULT_SUCCESS == result) {
                oldRear->mDataBuffer = pData;
                oldRear->mDataSize = dataLength;
                oldRear->mDataSizeConsumed = 0;
                oldRear->mBufferContext = pBufferContext;
                //oldRear->mBufferState = TBD;
                thiz->mRear = newRear;
                ++thiz->mState.count;
            }
        }
        // set enqueue attribute if state is PLAYING and the first buffer is enqueued
        interface_unlock_exclusive_attributes(thiz, ((SL_RESULT_SUCCESS == result) &&
                (1 == thiz->mState.count) && (SL_PLAYSTATE_PLAYING == getAssociatedState(thiz))) ?
                        ATTR_ABQ_ENQUEUE : ATTR_NONE);
    }

    SL_LEAVE_INTERFACE
}


static SLresult IAndroidBufferQueue_GetState(SLAndroidBufferQueueItf self,
        SLAndroidBufferQueueState *pState)
{
    SL_ENTER_INTERFACE

    // Note that GetState while a Clear is pending is equivalent to GetState before the Clear

    if (NULL == pState) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IAndroidBufferQueue *thiz = (IAndroidBufferQueue *) self;

        interface_lock_shared(thiz);

        pState->count = thiz->mState.count;
        pState->index = thiz->mState.index;

        interface_unlock_shared(thiz);

        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IAndroidBufferQueue_SetCallbackEventsMask(SLAndroidBufferQueueItf self,
        SLuint32 eventFlags)
{
    SL_ENTER_INTERFACE

    IAndroidBufferQueue *thiz = (IAndroidBufferQueue *) self;
    interface_lock_exclusive(thiz);
    // FIXME only supporting SL_ANDROIDBUFFERQUEUEEVENT_PROCESSED in this implementation
    if (!(~(SL_ANDROIDBUFFERQUEUEEVENT_PROCESSED /* | others TBD */ ) & eventFlags)) {
        thiz->mCallbackEventsMask = eventFlags;
        result = SL_RESULT_SUCCESS;
    } else {
        result = SL_RESULT_FEATURE_UNSUPPORTED;
    }
    interface_unlock_exclusive(thiz);

    SL_LEAVE_INTERFACE
}


static SLresult IAndroidBufferQueue_GetCallbackEventsMask(SLAndroidBufferQueueItf self,
        SLuint32 *pEventFlags)
{
    SL_ENTER_INTERFACE

    if (NULL == pEventFlags) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IAndroidBufferQueue *thiz = (IAndroidBufferQueue *) self;
        interface_lock_shared(thiz);
        SLuint32 callbackEventsMask = thiz->mCallbackEventsMask;
        interface_unlock_shared(thiz);
        *pEventFlags = callbackEventsMask;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static const struct SLAndroidBufferQueueItf_ IAndroidBufferQueue_Itf = {
    IAndroidBufferQueue_RegisterCallback,
    IAndroidBufferQueue_Clear,
    IAndroidBufferQueue_Enqueue,
    IAndroidBufferQueue_GetState,
    IAndroidBufferQueue_SetCallbackEventsMask,
    IAndroidBufferQueue_GetCallbackEventsMask
};


void IAndroidBufferQueue_init(void *self)
{
    IAndroidBufferQueue *thiz = (IAndroidBufferQueue *) self;
    thiz->mItf = &IAndroidBufferQueue_Itf;

    thiz->mState.count = 0;
    thiz->mState.index = 0;

    thiz->mCallback = NULL;
    thiz->mContext = NULL;
    thiz->mCallbackEventsMask = SL_ANDROIDBUFFERQUEUEEVENT_PROCESSED;

    thiz->mBufferType = kAndroidBufferTypeInvalid;
    thiz->mBufferArray = NULL;
    thiz->mFront = NULL;
    thiz->mRear = NULL;
    thiz->mEOS = false;
}


void IAndroidBufferQueue_deinit(void *self)
{
    IAndroidBufferQueue *thiz = (IAndroidBufferQueue *) self;
    if (NULL != thiz->mBufferArray) {
        free(thiz->mBufferArray);
        thiz->mBufferArray = NULL;
    }
}


#if 0
// Dump the contents of an IAndroidBufferQueue to the log.  This is for debugging only,
// and is not a documented API.  The associated object is locked throughout for atomicity,
// but the log entries may be interspersed with unrelated logs.

void IAndroidBufferQueue_log(IAndroidBufferQueue *thiz)
{
    interface_lock_shared(thiz);
    SL_LOGI("IAndroidBufferQueue %p:", thiz);
    SL_LOGI("  mState.count=%u mState.index=%u mCallback=%p mContext=%p",
            thiz->mState.count, thiz->mState.index, thiz->mCallback, thiz->mContext);
    const char *bufferTypeString;
    switch (thiz->mBufferType) {
    case kAndroidBufferTypeInvalid:
        bufferTypeString = "kAndroidBufferTypeInvalid";
        break;
    case kAndroidBufferTypeMpeg2Ts:
        bufferTypeString = "kAndroidBufferTypeMpeg2Ts";
        break;
    case kAndroidBufferTypeAacadts:
        bufferTypeString = "kAndroidBufferTypeAacadts";
        break;
    default:
        bufferTypeString = "unknown";
        break;
    }
    SL_LOGI("  mCallbackEventsMask=0x%x, mBufferType=0x%x (%s), mEOS=%s",
            thiz->mCallbackEventsMask,
            thiz->mBufferType, bufferTypeString,
            thiz->mEOS ? "true" : "false");
    SL_LOGI("  mBufferArray=%p, mFront=%p (%u), mRear=%p (%u)",
            thiz->mBufferArray,
            thiz->mFront, thiz->mFront - thiz->mBufferArray,
            thiz->mRear, thiz->mRear - thiz->mBufferArray);
    SL_LOGI("  index mDataBuffer mDataSize mDataSizeConsumed mBufferContext mItems");
    const AdvancedBufferHeader *hdr;
    for (hdr = thiz->mFront; hdr != thiz->mRear; ) {
        SLuint32 i = hdr - thiz->mBufferArray;
        char itemString[32];
        switch (thiz->mBufferType) {
        case kAndroidBufferTypeMpeg2Ts:
            switch (hdr->mItems.mTsCmdData.mTsCmdCode) {
            case ANDROID_MP2TSEVENT_NONE:
                strcpy(itemString, "NONE");
                break;
            case ANDROID_MP2TSEVENT_EOS:
                strcpy(itemString, "EOS");
                break;
            case ANDROID_MP2TSEVENT_DISCONTINUITY:
                strcpy(itemString, "DISCONTINUITY");
                break;
            case ANDROID_MP2TSEVENT_DISCON_NEWPTS:
                snprintf(itemString, sizeof(itemString), "NEWPTS %llu",
                        hdr->mItems.mTsCmdData.mPts);
                break;
            case ANDROID_MP2TSEVENT_FORMAT_CHANGE:
                strcpy(itemString, "FORMAT_CHANGE");
                break;
            default:
                snprintf(itemString, sizeof(itemString), "0x%x", hdr->mItems.mTsCmdData.mTsCmdCode);
                break;
            }
            break;
        case kAndroidBufferTypeAacadts:
            switch (hdr->mItems.mAdtsCmdData.mAdtsCmdCode) {
            case ANDROID_ADTSEVENT_NONE:
                strcpy(itemString, "NONE");
                break;
            case ANDROID_ADTSEVENT_EOS:
                strcpy(itemString, "EOS");
                break;
            default:
                snprintf(itemString, sizeof(itemString), "0x%x",
                        hdr->mItems.mAdtsCmdData.mAdtsCmdCode);
                break;
            }
            break;
        default:
            strcpy(itemString, "");
            break;
        }
        SL_LOGI("  %5u %11p %9u %17u %14p %s",
                i, hdr->mDataBuffer, hdr->mDataSize, hdr->mDataSizeConsumed,
                hdr->mBufferContext, itemString);
                // mBufferState
        if (++hdr == &thiz->mBufferArray[thiz->mNumBuffers + 1]) {
            hdr = thiz->mBufferArray;
        }
    }
    interface_unlock_shared(thiz);
}

#endif
