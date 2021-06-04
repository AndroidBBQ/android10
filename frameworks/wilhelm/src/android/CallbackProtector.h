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

#include "Configuration.h"
#include "utils/threads.h"

//--------------------------------------------------------------------------------------------------
namespace android {

class CallbackProtector : public RefBase {

public:
    CallbackProtector();
    virtual ~CallbackProtector();

    /**
     * Indicates whether the CallbackProtector is non-NULL and it's safe to enter the callback.
     */
    static bool enterCbIfOk(const sp<CallbackProtector> &protector);

    /**
     * Indicates whether it's safe to enter the callback. It would typically return false
     * if the associated object (AudioTrack, AudioPlayer, MediaPlayer) is about to be destroyed.
     */
    bool enterCb();

    /**
     * This method must be paired to each call to enterCb() or enterCbIfOk(),
     * only it returned that it is safe enter the callback;
     */
    void exitCb();

    /**
     * Called to signal the associated object is about to be destroyed, so whenever a callback is
     * entered (see enterCb) it will be notified it is pointless to process the callback. This will
     * return immediately if there are no callbacks, and will block until current callbacks exit.
     */
    void requestCbExitAndWait();

    /**
     * Similar to requestCbExitAndWait, but does not wait for current callbacks to exit.
     */
    void requestCbExit();

private:
    Mutex mLock;
    Condition mCbExitedCondition;

    bool mSafeToEnterCb;

    /** Counts the number of callbacks actively locking the associated AudioPlayer */
    unsigned int mCbCount;

#ifdef USE_DEBUG
    pthread_t mCallbackThread;
    pid_t mCallbackTid;
    pthread_t mRequesterThread;
    pid_t mRequesterTid;
#endif

    // disallow "evil" constructors
    CallbackProtector(const CallbackProtector &);
    CallbackProtector &operator=(const CallbackProtector &);
};

} // namespace android
