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

/** \brief libsndfile integration */

#include "sles_allinclusive.h"


/** \brief Called by SndFile.c:audioPlayerTransportUpdate after a play state change or seek,
 *  and by IOutputMixExt::FillBuffer after each buffer is consumed.
 */

void SndFile_Callback(SLBufferQueueItf caller, void *pContext)
{
    CAudioPlayer *thisAP = (CAudioPlayer *) pContext;
    object_lock_peek(&thisAP->mObject);
    SLuint32 state = thisAP->mPlay.mState;
    object_unlock_peek(&thisAP->mObject);
    if (SL_PLAYSTATE_PLAYING != state) {
        return;
    }
    struct SndFile *thiz = &thisAP->mSndFile;
    SLresult result;
    pthread_mutex_lock(&thiz->mMutex);
    if (thiz->mEOF) {
        pthread_mutex_unlock(&thiz->mMutex);
        return;
    }
    short *pBuffer = &thiz->mBuffer[thiz->mWhich * SndFile_BUFSIZE];
    if (++thiz->mWhich >= SndFile_NUMBUFS) {
        thiz->mWhich = 0;
    }
    sf_count_t count;
    count = sf_read_short(thiz->mSNDFILE, pBuffer, (sf_count_t) SndFile_BUFSIZE);
    pthread_mutex_unlock(&thiz->mMutex);
    bool headAtNewPos = false;
    object_lock_exclusive(&thisAP->mObject);
    slPlayCallback callback = thisAP->mPlay.mCallback;
    void *context = thisAP->mPlay.mContext;
    // make a copy of sample rate so we are absolutely sure we will not divide by zero
    SLuint32 sampleRateMilliHz = thisAP->mSampleRateMilliHz;
    if (UNKNOWN_SAMPLERATE != sampleRateMilliHz) {
        // this will overflow after 49 days, but no fix possible as it's part of the API
        thisAP->mPlay.mPosition = (SLuint32) (((long long) thisAP->mPlay.mFramesSinceLastSeek *
            1000000LL) / sampleRateMilliHz) + thisAP->mPlay.mLastSeekPosition;
        // make a good faith effort for the mean time between "head at new position" callbacks to
        // occur at the requested update period, but there will be jitter
        SLuint32 frameUpdatePeriod = thisAP->mPlay.mFrameUpdatePeriod;
        if ((0 != frameUpdatePeriod) &&
            (thisAP->mPlay.mFramesSincePositionUpdate >= frameUpdatePeriod) &&
            (SL_PLAYEVENT_HEADATNEWPOS & thisAP->mPlay.mEventFlags)) {
            // if we overrun a requested update period, then reset the clock modulo the
            // update period so that it appears to the application as one or more lost callbacks,
            // but no additional jitter
            if ((thisAP->mPlay.mFramesSincePositionUpdate -= thisAP->mPlay.mFrameUpdatePeriod) >=
                    frameUpdatePeriod) {
                thisAP->mPlay.mFramesSincePositionUpdate %= frameUpdatePeriod;
            }
            headAtNewPos = true;
        }
    }
    if (0 < count) {
        object_unlock_exclusive(&thisAP->mObject);
        SLuint32 size = (SLuint32) (count * sizeof(short));
        result = IBufferQueue_Enqueue(caller, pBuffer, size);
        // not much we can do if the Enqueue fails, so we'll just drop the decoded data
        if (SL_RESULT_SUCCESS != result) {
            SL_LOGE("enqueue failed 0x%x", result);
        }
    } else {
        thisAP->mPlay.mState = SL_PLAYSTATE_PAUSED;
        thiz->mEOF = SL_BOOLEAN_TRUE;
        // this would result in a non-monotonically increasing position, so don't do it
        // thisAP->mPlay.mPosition = thisAP->mPlay.mDuration;
        object_unlock_exclusive_attributes(&thisAP->mObject, ATTR_TRANSPORT);
    }
    // callbacks are called with mutex unlocked
    if (NULL != callback) {
        if (headAtNewPos) {
            (*callback)(&thisAP->mPlay.mItf, context, SL_PLAYEVENT_HEADATNEWPOS);
        }
    }
}


/** \brief Check whether the supplied libsndfile format is supported by us */

SLboolean SndFile_IsSupported(const SF_INFO *sfinfo)
{
    switch (sfinfo->format & SF_FORMAT_TYPEMASK) {
    case SF_FORMAT_WAV:
        break;
    default:
        return SL_BOOLEAN_FALSE;
    }
    switch (sfinfo->format & SF_FORMAT_SUBMASK) {
    case SF_FORMAT_PCM_U8:
    case SF_FORMAT_PCM_16:
        break;
    default:
        return SL_BOOLEAN_FALSE;
    }
    switch (sfinfo->samplerate) {
    case 11025:
    case 22050:
    case 44100:
        break;
    default:
        return SL_BOOLEAN_FALSE;
    }
    switch (sfinfo->channels) {
    case 1:
    case 2:
        break;
    default:
        return SL_BOOLEAN_FALSE;
    }
    return SL_BOOLEAN_TRUE;
}


/** \brief Check whether the partially-constructed AudioPlayer is compatible with libsndfile */

SLresult SndFile_checkAudioPlayerSourceSink(CAudioPlayer *thiz)
{
    const SLDataSource *pAudioSrc = &thiz->mDataSource.u.mSource;
    SLuint32 locatorType = *(SLuint32 *)pAudioSrc->pLocator;
    SLuint32 formatType = *(SLuint32 *)pAudioSrc->pFormat;
    switch (locatorType) {
    case SL_DATALOCATOR_BUFFERQUEUE:
#ifdef ANDROID
    case SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE:
#endif
        break;
    case SL_DATALOCATOR_URI:
        {
        SLDataLocator_URI *dl_uri = (SLDataLocator_URI *) pAudioSrc->pLocator;
        SLchar *uri = dl_uri->URI;
        if (NULL == uri) {
            return SL_RESULT_PARAMETER_INVALID;
        }
        if (!strncmp((const char *) uri, "file:///", 8)) {
            uri += 8;
        }
        switch (formatType) {
        case SL_DATAFORMAT_NULL:    // OK to omit the data format
        case SL_DATAFORMAT_MIME:    // we ignore a MIME type if specified
            break;
        case SL_DATAFORMAT_PCM:
        case XA_DATAFORMAT_RAWIMAGE:
            return SL_RESULT_CONTENT_UNSUPPORTED;
        default:
            // an invalid data format is detected earlier during the deep copy
            assert(false);
            return SL_RESULT_INTERNAL_ERROR;
        }
        thiz->mSndFile.mPathname = uri;
        thiz->mBufferQueue.mNumBuffers = SndFile_NUMBUFS;
        }
        break;
    default:
        return SL_RESULT_CONTENT_UNSUPPORTED;
    }
    thiz->mSndFile.mWhich = 0;
    thiz->mSndFile.mSNDFILE = NULL;
    // thiz->mSndFile.mMutex is initialized only when there is a valid mSNDFILE
    thiz->mSndFile.mEOF = SL_BOOLEAN_FALSE;

    return SL_RESULT_SUCCESS;
}


/** \brief Called with mutex unlocked for marker and position updates, and play state change */

void audioPlayerTransportUpdate(CAudioPlayer *audioPlayer)
{

    if (NULL != audioPlayer->mSndFile.mSNDFILE) {

        object_lock_exclusive(&audioPlayer->mObject);
        SLboolean empty = 0 == audioPlayer->mBufferQueue.mState.count;
        // FIXME a made-up number that should depend on player state and prefetch status
        audioPlayer->mPrefetchStatus.mLevel = 1000;
        SLmillisecond pos = audioPlayer->mSeek.mPos;
        if (SL_TIME_UNKNOWN != pos) {
            audioPlayer->mSeek.mPos = SL_TIME_UNKNOWN;
            // trim seek position to the current known duration
            if (pos > audioPlayer->mPlay.mDuration) {
                pos = audioPlayer->mPlay.mDuration;
            }
            audioPlayer->mPlay.mLastSeekPosition = pos;
            audioPlayer->mPlay.mFramesSinceLastSeek = 0;
            // seek postpones the next head at new position callback
            audioPlayer->mPlay.mFramesSincePositionUpdate = 0;
        }
        object_unlock_exclusive(&audioPlayer->mObject);

        if (SL_TIME_UNKNOWN != pos) {

            // discard any enqueued buffers for the old position
            IBufferQueue_Clear(&audioPlayer->mBufferQueue.mItf);
            empty = SL_BOOLEAN_TRUE;

            pthread_mutex_lock(&audioPlayer->mSndFile.mMutex);
            // FIXME why void?
            (void) sf_seek(audioPlayer->mSndFile.mSNDFILE, (sf_count_t) (((long long) pos *
                audioPlayer->mSndFile.mSfInfo.samplerate) / 1000LL), SEEK_SET);
            audioPlayer->mSndFile.mEOF = SL_BOOLEAN_FALSE;
            audioPlayer->mSndFile.mWhich = 0;
            pthread_mutex_unlock(&audioPlayer->mSndFile.mMutex);

        }

        // FIXME only on seek or play state change (STOPPED, PAUSED) -> PLAYING
        if (empty) {
            SndFile_Callback(&audioPlayer->mBufferQueue.mItf, audioPlayer);
        }

    }

}


/** \brief Called by CAudioPlayer_Realize */

SLresult SndFile_Realize(CAudioPlayer *thiz)
{
    SLresult result = SL_RESULT_SUCCESS;
    if (NULL != thiz->mSndFile.mPathname) {
        thiz->mSndFile.mSfInfo.format = 0;
        thiz->mSndFile.mSNDFILE = sf_open(
            (const char *) thiz->mSndFile.mPathname, SFM_READ, &thiz->mSndFile.mSfInfo);
        if (NULL == thiz->mSndFile.mSNDFILE) {
            result = SL_RESULT_CONTENT_NOT_FOUND;
        } else if (!SndFile_IsSupported(&thiz->mSndFile.mSfInfo)) {
            sf_close(thiz->mSndFile.mSNDFILE);
            thiz->mSndFile.mSNDFILE = NULL;
            result = SL_RESULT_CONTENT_UNSUPPORTED;
        } else {
            int ok;
            ok = pthread_mutex_init(&thiz->mSndFile.mMutex, (const pthread_mutexattr_t *) NULL);
            assert(0 == ok);
            SLBufferQueueItf bufferQueue = &thiz->mBufferQueue.mItf;
            IBufferQueue *thisBQ = (IBufferQueue *) bufferQueue;
            IBufferQueue_RegisterCallback(&thisBQ->mItf, SndFile_Callback, thiz);
            thiz->mPrefetchStatus.mStatus = SL_PREFETCHSTATUS_SUFFICIENTDATA;
            // this is the initial duration; will update when a new maximum position is detected
            thiz->mPlay.mDuration = (SLmillisecond) (((long long) thiz->mSndFile.mSfInfo.frames *
                1000LL) / thiz->mSndFile.mSfInfo.samplerate);
            thiz->mNumChannels = thiz->mSndFile.mSfInfo.channels;
            thiz->mSampleRateMilliHz = thiz->mSndFile.mSfInfo.samplerate * 1000;
#ifdef USE_OUTPUTMIXEXT
            thiz->mPlay.mFrameUpdatePeriod = ((long long) thiz->mPlay.mPositionUpdatePeriod *
                (long long) thiz->mSampleRateMilliHz) / 1000000LL;
#endif
        }
    }
    return result;
}


/** \brief Called by CAudioPlayer_Destroy */

void SndFile_Destroy(CAudioPlayer *thiz)
{
    if (NULL != thiz->mSndFile.mSNDFILE) {
        sf_close(thiz->mSndFile.mSNDFILE);
        thiz->mSndFile.mSNDFILE = NULL;
        int ok;
        ok = pthread_mutex_destroy(&thiz->mSndFile.mMutex);
        assert(0 == ok);
    }
}
