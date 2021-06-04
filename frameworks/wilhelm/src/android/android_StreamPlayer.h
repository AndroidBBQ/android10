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

#include <media/IStreamSource.h>
#include <binder/IServiceManager.h>
#include "android/android_GenericMediaPlayer.h"

// number of SLuint32 fields to store a buffer event message in an item, by mapping each
//   to the item key (SLuint32), the item size (SLuint32), and the item data (mask on SLuint32)
#define NB_BUFFEREVENT_ITEM_FIELDS 3

//--------------------------------------------------------------------------------------------------
namespace android {

//--------------------------------------------------------------------------------------------------
class StreamPlayer;

class StreamSourceAppProxy : public BnStreamSource {
public:
    StreamSourceAppProxy(
            IAndroidBufferQueue *androidBufferQueue,
            const sp<CallbackProtector> &callbackProtector,
            StreamPlayer *player);
    virtual ~StreamSourceAppProxy();

    // store an item structure to indicate a processed buffer
    static const SLuint32 kItemProcessed[NB_BUFFEREVENT_ITEM_FIELDS];

    // IStreamSource implementation
    virtual void setListener(const sp<IStreamListener> &listener); // mediaserver calls exactly once
    virtual void setBuffers(const Vector<sp<IMemory> > &buffers);  // mediaserver calls exactly once
    virtual void onBufferAvailable(size_t index);

    // Consumption from ABQ
    void pullFromBuffQueue();

private:
    void receivedCmd_l(IStreamListener::Command cmd, const sp<AMessage> &msg = NULL);
    void receivedBuffer_l(size_t buffIndex, size_t buffLength);

public:
    // Call at least once prior to releasing the last strong reference to this object. It causes
    // the player to release all of its resources, similar to android.media.MediaPlayer disconnect.
    void disconnect();

private:
    // protects mListener, mBuffers, mBuffersHasBeenSet, and mAvailableBuffers
    Mutex mLock;

    sp<IStreamListener> mListener;
    // array of shared memory buffers
    Vector<sp<IMemory> > mBuffers;
    bool mBuffersHasBeenSet;
    // list of available buffers in shared memory, identified by their index
    List<size_t> mAvailableBuffers;

    // the Android Buffer Queue from which data is consumed and written to shared memory
    IAndroidBufferQueue* const mAndroidBufferQueue;

    const sp<CallbackProtector> mCallbackProtector;
    const wp<StreamPlayer> mPlayer;

    DISALLOW_EVIL_CONSTRUCTORS(StreamSourceAppProxy);
};


//--------------------------------------------------------------------------------------------------
class StreamPlayer : public GenericMediaPlayer
{
public:
    StreamPlayer(const AudioPlayback_Parameters* params, bool hasVideo,
           IAndroidBufferQueue *androidBufferQueue, const sp<CallbackProtector> &callbackProtector);
    virtual ~StreamPlayer();

    // overridden from GenericPlayer
    virtual void onMessageReceived(const sp<AMessage> &msg);
    virtual void preDestroy();

    void queueRefilled();
    // Called after AndroidBufferQueue::Clear.
    // The "_l" indicates the caller still has it's (now empty) AndroidBufferQueue locked.
    void appClear_l();

protected:

    enum {
        // message to asynchronously notify mAppProxy it should try to pull from the Android
        //    Buffer Queue and push to shared memory (media server), either because the buffer queue
        //    was refilled, or because during playback, the shared memory buffers should remain
        //    filled to prevent it from draining (this can happen if the ABQ is not ready
        //    whenever a shared memory buffer becomes available)
        kWhatPullFromAbq    = 'plfq',
        kWhatStopForDestroy = 's4ds'
    };

    const sp<StreamSourceAppProxy> mAppProxy; // application proxy for the shared memory source

    // overridden from GenericMediaPlayer
    virtual void onPrepare();
    virtual void onPlay();

    void onPullFromAndroidBufferQueue();

private:
    void onStopForDestroy();

    Mutex mStopForDestroyLock;
    Condition mStopForDestroyCondition;
    bool mStopForDestroyCompleted;

    DISALLOW_EVIL_CONSTRUCTORS(StreamPlayer);
};

} // namespace android
