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

/* OutputMixExt implementation */

#include "sles_allinclusive.h"
#include <math.h>


// OutputMixExt is used by SDL, but is not specific to or dependent on SDL


// stereo is a frame consisting of a pair of 16-bit PCM samples

typedef struct {
    short left;
    short right;
} stereo;


/** \brief Summary of the gain, as an optimization for the mixer */

typedef enum {
    GAIN_MUTE  = 0,  // mValue == 0.0f within epsilon
    GAIN_UNITY = 1,  // mValue == 1.0f within epsilon
    GAIN_OTHER = 2   // 0.0f < mValue < 1.0f
} Summary;


/** \brief Check whether a track has any data for us to read */

static SLboolean track_check(Track *track)
{
    assert(NULL != track);
    SLboolean trackHasData = SL_BOOLEAN_FALSE;

    CAudioPlayer *audioPlayer = track->mAudioPlayer;
    if (NULL != audioPlayer) {

        // track is initialized

        // FIXME This lock could block and result in stuttering;
        // a trylock with retry or lockless solution would be ideal
        object_lock_exclusive(&audioPlayer->mObject);
        assert(audioPlayer->mTrack == track);

        SLuint32 framesMixed = track->mFramesMixed;
        if (0 != framesMixed) {
            track->mFramesMixed = 0;
            audioPlayer->mPlay.mFramesSinceLastSeek += framesMixed;
            audioPlayer->mPlay.mFramesSincePositionUpdate += framesMixed;
        }

        SLboolean doBroadcast = SL_BOOLEAN_FALSE;
        const BufferHeader *oldFront;

        if (audioPlayer->mBufferQueue.mClearRequested) {
            // application thread(s) that call BufferQueue::Clear while mixer is active
            // will block synchronously until mixer acknowledges the Clear request
            audioPlayer->mBufferQueue.mFront = &audioPlayer->mBufferQueue.mArray[0];
            audioPlayer->mBufferQueue.mRear = &audioPlayer->mBufferQueue.mArray[0];
            audioPlayer->mBufferQueue.mState.count = 0;
            audioPlayer->mBufferQueue.mState.playIndex = 0;
            audioPlayer->mBufferQueue.mClearRequested = SL_BOOLEAN_FALSE;
            track->mReader = NULL;
            track->mAvail = 0;
            doBroadcast = SL_BOOLEAN_TRUE;
        }

        if (audioPlayer->mDestroyRequested) {
            // an application thread that calls Object::Destroy while mixer is active will block
            // synchronously in the PreDestroy hook until mixer acknowledges the Destroy request
            COutputMix *outputMix = CAudioPlayer_GetOutputMix(audioPlayer);
            unsigned i = track - outputMix->mOutputMixExt.mTracks;
            assert( /* 0 <= i && */ i < MAX_TRACK);
            unsigned mask = 1 << i;
            track->mAudioPlayer = NULL;
            assert(outputMix->mOutputMixExt.mActiveMask & mask);
            outputMix->mOutputMixExt.mActiveMask &= ~mask;
            audioPlayer->mTrack = NULL;
            audioPlayer->mDestroyRequested = SL_BOOLEAN_FALSE;
            doBroadcast = SL_BOOLEAN_TRUE;
            goto broadcast;
        }

        switch (audioPlayer->mPlay.mState) {

        case SL_PLAYSTATE_PLAYING:  // continue playing current track data
            if (0 < track->mAvail) {
                trackHasData = SL_BOOLEAN_TRUE;
                break;
            }

            // try to get another buffer from queue
            oldFront = audioPlayer->mBufferQueue.mFront;
            if (oldFront != audioPlayer->mBufferQueue.mRear) {
                assert(0 < audioPlayer->mBufferQueue.mState.count);
                track->mReader = oldFront->mBuffer;
                track->mAvail = oldFront->mSize;
                // note that the buffer stays on the queue while we are reading
                audioPlayer->mPlay.mState = SL_PLAYSTATE_PLAYING;
                trackHasData = SL_BOOLEAN_TRUE;
            } else {
                // no buffers on queue, so playable but not playing
                // NTH should be able to call a desperation callback when completely starved,
                // or call less often than every buffer based on high/low water-marks
            }

            // copy gains from audio player to track
            track->mGains[0] = audioPlayer->mGains[0];
            track->mGains[1] = audioPlayer->mGains[1];
            break;

        case SL_PLAYSTATE_STOPPING: // application thread(s) called Play::SetPlayState(STOPPED)
            audioPlayer->mPlay.mPosition = (SLmillisecond) 0;
            audioPlayer->mPlay.mFramesSinceLastSeek = 0;
            audioPlayer->mPlay.mFramesSincePositionUpdate = 0;
            audioPlayer->mPlay.mLastSeekPosition = 0;
            audioPlayer->mPlay.mState = SL_PLAYSTATE_STOPPED;
            // stop cancels a pending seek
            audioPlayer->mSeek.mPos = SL_TIME_UNKNOWN;
            oldFront = audioPlayer->mBufferQueue.mFront;
            if (oldFront != audioPlayer->mBufferQueue.mRear) {
                assert(0 < audioPlayer->mBufferQueue.mState.count);
                track->mReader = oldFront->mBuffer;
                track->mAvail = oldFront->mSize;
            }
            doBroadcast = SL_BOOLEAN_TRUE;
            break;

        case SL_PLAYSTATE_STOPPED:  // idle
        case SL_PLAYSTATE_PAUSED:   // idle
            break;

        default:
            assert(SL_BOOLEAN_FALSE);
            break;
        }

broadcast:
        if (doBroadcast) {
            object_cond_broadcast(&audioPlayer->mObject);
        }

        object_unlock_exclusive(&audioPlayer->mObject);

    }

    return trackHasData;

}


/** \brief This is the track mixer: fill the specified 16-bit stereo PCM buffer */

void IOutputMixExt_FillBuffer(SLOutputMixExtItf self, void *pBuffer, SLuint32 size)
{
    SL_ENTER_INTERFACE_VOID

    // Force to be a multiple of a frame, assumes stereo 16-bit PCM
    size &= ~3;
    SLboolean mixBufferHasData = SL_BOOLEAN_FALSE;
    IOutputMixExt *thiz = (IOutputMixExt *) self;
    IObject *thisObject = thiz->mThis;
    // This lock should never block, except when the application destroys the output mix object
    object_lock_exclusive(thisObject);
    unsigned activeMask;
    // If the output mix is marked for destruction, then acknowledge the request
    if (thiz->mDestroyRequested) {
        IEngine *thisEngine = &thisObject->mEngine->mEngine;
        interface_lock_exclusive(thisEngine);
        assert(&thisEngine->mOutputMix->mObject == thisObject);
        thisEngine->mOutputMix = NULL;
        // Note we don't attempt to connect another output mix, even if there is one
        interface_unlock_exclusive(thisEngine);
        // Acknowledge the destroy request, and notify the pre-destroy hook
        thiz->mDestroyRequested = SL_BOOLEAN_FALSE;
        object_cond_broadcast(thisObject);
        activeMask = 0;
    } else {
        activeMask = thiz->mActiveMask;
    }
    while (activeMask) {
        unsigned i = ctz(activeMask);
        assert(MAX_TRACK > i);
        activeMask &= ~(1 << i);
        Track *track = &thiz->mTracks[i];

        // track is allocated

        if (!track_check(track)) {
            continue;
        }

        // track is playing
        void *dstWriter = pBuffer;
        unsigned desired = size;
        SLboolean trackContributedToMix = SL_BOOLEAN_FALSE;
        float gains[STEREO_CHANNELS];
        Summary summaries[STEREO_CHANNELS];
        unsigned channel;
        for (channel = 0; channel < STEREO_CHANNELS; ++channel) {
            float gain = track->mGains[channel];
            gains[channel] = gain;
            Summary summary;
            if (gain <= 0.001) {
                summary = GAIN_MUTE;
            } else if (gain >= 0.999) {
                summary = GAIN_UNITY;
            } else {
                summary = GAIN_OTHER;
            }
            summaries[channel] = summary;
        }
        while (desired > 0) {
            unsigned actual = desired;
            if (track->mAvail < actual) {
                actual = track->mAvail;
            }
            // force actual to be a frame multiple
            if (actual > 0) {
                assert(NULL != track->mReader);
                stereo *mixBuffer = (stereo *) dstWriter;
                const stereo *source = (const stereo *) track->mReader;
                unsigned j;
                if (GAIN_MUTE != summaries[0] || GAIN_MUTE != summaries[1]) {
                    if (mixBufferHasData) {
                        // apply gain during add
                        if (GAIN_UNITY != summaries[0] || GAIN_UNITY != summaries[1]) {
                            for (j = 0; j < actual; j += sizeof(stereo), ++mixBuffer, ++source) {
                                mixBuffer->left += (short) (source->left * track->mGains[0]);
                                mixBuffer->right += (short) (source->right * track->mGains[1]);
                            }
                        // no gain adjustment needed, so do a simple add
                        } else {
                            for (j = 0; j < actual; j += sizeof(stereo), ++mixBuffer, ++source) {
                                mixBuffer->left += source->left;
                                mixBuffer->right += source->right;
                            }
                        }
                    } else {
                        // apply gain during copy
                        if (GAIN_UNITY != summaries[0] || GAIN_UNITY != summaries[1]) {
                            for (j = 0; j < actual; j += sizeof(stereo), ++mixBuffer, ++source) {
                                mixBuffer->left = (short) (source->left * track->mGains[0]);
                                mixBuffer->right = (short) (source->right * track->mGains[1]);
                            }
                        // no gain adjustment needed, so do a simple copy
                        } else {
                            memcpy(dstWriter, track->mReader, actual);
                        }
                    }
                    trackContributedToMix = SL_BOOLEAN_TRUE;
                }
                dstWriter = (char *) dstWriter + actual;
                desired -= actual;
                track->mReader = (char *) track->mReader + actual;
                track->mAvail -= actual;
                if (track->mAvail == 0) {
                    IBufferQueue *bufferQueue = &track->mAudioPlayer->mBufferQueue;
                    interface_lock_exclusive(bufferQueue);
                    const BufferHeader *oldFront, *newFront, *rear;
                    oldFront = bufferQueue->mFront;
                    rear = bufferQueue->mRear;
                    // a buffer stays on queue while playing, so it better still be there
                    assert(oldFront != rear);
                    newFront = oldFront;
                    if (++newFront == &bufferQueue->mArray[bufferQueue->mNumBuffers + 1]) {
                        newFront = bufferQueue->mArray;
                    }
                    bufferQueue->mFront = (BufferHeader *) newFront;
                    assert(0 < bufferQueue->mState.count);
                    --bufferQueue->mState.count;
                    if (newFront != rear) {
                        // we don't acknowledge application requests between buffers
                        // within the same mixer frame
                        assert(0 < bufferQueue->mState.count);
                        track->mReader = newFront->mBuffer;
                        track->mAvail = newFront->mSize;
                    }
                    // else we would set play state to playable but not playing during next mixer
                    // frame if the queue is still empty at that time
                    ++bufferQueue->mState.playIndex;
                    slBufferQueueCallback callback = bufferQueue->mCallback;
                    void *context = bufferQueue->mContext;
                    interface_unlock_exclusive(bufferQueue);
                    // The callback function is called on each buffer completion
                    if (NULL != callback) {
                        (*callback)((SLBufferQueueItf) bufferQueue, context);
                        // Maybe it enqueued another buffer, or maybe it didn't.
                        // We will find out later during the next mixer frame.
                    }
                }
                // no lock, but safe because noone else updates this field
                track->mFramesMixed += actual >> 2;    // sizeof(short) * STEREO_CHANNELS
                continue;
            }
            // we need more data: desired > 0 but actual == 0
            if (track_check(track)) {
                continue;
            }
            // underflow: clear out rest of partial buffer (NTH synthesize comfort noise)
            if (!mixBufferHasData && trackContributedToMix) {
                memset(dstWriter, 0, actual);
            }
            break;
        }
        if (trackContributedToMix) {
            mixBufferHasData = SL_BOOLEAN_TRUE;
        }
    }
    object_unlock_exclusive(thisObject);
    // No active tracks, so output silence
    if (!mixBufferHasData) {
        memset(pBuffer, 0, size);
    }

    SL_LEAVE_INTERFACE_VOID
}


static const struct SLOutputMixExtItf_ IOutputMixExt_Itf = {
    IOutputMixExt_FillBuffer
};

void IOutputMixExt_init(void *self)
{
    IOutputMixExt *thiz = (IOutputMixExt *) self;
    thiz->mItf = &IOutputMixExt_Itf;
    thiz->mActiveMask = 0;
    Track *track = &thiz->mTracks[0];
    unsigned i;
    for (i = 0; i < MAX_TRACK; ++i, ++track) {
        track->mAudioPlayer = NULL;
    }
    thiz->mDestroyRequested = SL_BOOLEAN_FALSE;
}


/** \brief Called by Engine::CreateAudioPlayer to allocate a track */

SLresult IOutputMixExt_checkAudioPlayerSourceSink(CAudioPlayer *thiz)
{
    thiz->mTrack = NULL;

    // check the source for compatibility
    switch (thiz->mDataSource.mLocator.mLocatorType) {
    case SL_DATALOCATOR_BUFFERQUEUE:
#ifdef ANDROID
    case SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE:
#endif
        switch (thiz->mDataSource.mFormat.mFormatType) {
        case SL_DATAFORMAT_PCM:
#ifdef USE_SDL
            // SDL is hard-coded to 44.1 kHz, and there is no sample rate converter
            if (SL_SAMPLINGRATE_44_1 != thiz->mDataSource.mFormat.mPCM.samplesPerSec)
                return SL_RESULT_CONTENT_UNSUPPORTED;
#endif
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    // check the sink for compatibility
    const SLDataSink *pAudioSnk = &thiz->mDataSink.u.mSink;
    Track *track = NULL;
    switch (*(SLuint32 *)pAudioSnk->pLocator) {
    case SL_DATALOCATOR_OUTPUTMIX:
        {
        // pAudioSnk->pFormat is ignored
        IOutputMixExt *omExt = &((COutputMix *) ((SLDataLocator_OutputMix *)
            pAudioSnk->pLocator)->outputMix)->mOutputMixExt;
        // allocate an entry within OutputMix for this track
        interface_lock_exclusive(omExt);
        unsigned availMask = ~omExt->mActiveMask;
        if (!availMask) {
            interface_unlock_exclusive(omExt);
            // All track slots full in output mix
            return SL_RESULT_MEMORY_FAILURE;
        }
        unsigned i = ctz(availMask);
        assert(MAX_TRACK > i);
        omExt->mActiveMask |= 1 << i;
        track = &omExt->mTracks[i];
        track->mAudioPlayer = NULL;    // only field that is accessed before full initialization
        interface_unlock_exclusive(omExt);
        thiz->mTrack = track;
        thiz->mGains[0] = 1.0f;
        thiz->mGains[1] = 1.0f;
        thiz->mDestroyRequested = SL_BOOLEAN_FALSE;
        }
        break;
    default:
        return SL_RESULT_CONTENT_UNSUPPORTED;
    }

    assert(NULL != track);
    track->mBufferQueue = &thiz->mBufferQueue;
    track->mAudioPlayer = thiz;
    track->mReader = NULL;
    track->mAvail = 0;
    track->mGains[0] = 1.0f;
    track->mGains[1] = 1.0f;
    track->mFramesMixed = 0;
    return SL_RESULT_SUCCESS;
}


/** \brief Called when a gain-related field (mute, solo, volume, stereo position, etc.) updated */

void audioPlayerGainUpdate(CAudioPlayer *audioPlayer)
{
    SLboolean mute = audioPlayer->mVolume.mMute;
    SLuint8 muteMask = audioPlayer->mMuteMask;
    SLuint8 soloMask = audioPlayer->mSoloMask;
    SLmillibel level = audioPlayer->mVolume.mLevel;
    SLboolean enableStereoPosition = audioPlayer->mVolume.mEnableStereoPosition;
    SLpermille stereoPosition = audioPlayer->mVolume.mStereoPosition;

    if (soloMask) {
        muteMask |= ~soloMask;
    }
    if (mute || !(~muteMask & 3)) {
        audioPlayer->mGains[0] = 0.0f;
        audioPlayer->mGains[1] = 0.0f;
    } else {
        float playerGain = powf(10.0f, level / 2000.0f);
        unsigned channel;
        for (channel = 0; channel < STEREO_CHANNELS; ++channel) {
            float gain;
            if (muteMask & (1 << channel)) {
                gain = 0.0f;
            } else {
                gain = playerGain;
                if (enableStereoPosition) {
                    switch (channel) {
                    case 0:
                        if (stereoPosition > 0) {
                            gain *= (1000 - stereoPosition) / 1000.0f;
                        }
                        break;
                    case 1:
                        if (stereoPosition < 0) {
                            gain *= (1000 + stereoPosition) / 1000.0f;
                        }
                        break;
                    default:
                        assert(SL_BOOLEAN_FALSE);
                        break;
                    }
                }
            }
            audioPlayer->mGains[channel] = gain;
        }
    }
}
