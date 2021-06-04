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

#ifndef A_LOOPER_H_

#define A_LOOPER_H_

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AString.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/List.h>
#include <utils/RefBase.h>
#include <utils/threads.h>

namespace android {

struct AHandler;
struct AMessage;
struct AReplyToken;

struct ALooper : public RefBase {
    typedef int32_t event_id;
    typedef int32_t handler_id;

    ALooper();

    // Takes effect in a subsequent call to start().
    void setName(const char *name);

    handler_id registerHandler(const sp<AHandler> &handler);
    void unregisterHandler(handler_id handlerID);

    status_t start(
            bool runOnCallingThread = false,
            bool canCallJava = false,
            int32_t priority = PRIORITY_DEFAULT
            );

    status_t stop();

    static int64_t GetNowUs();

    const char *getName() const {
        return mName.c_str();
    }

protected:
    virtual ~ALooper();

private:
    friend struct AMessage;       // post()

    struct Event {
        int64_t mWhenUs;
        sp<AMessage> mMessage;
    };

    Mutex mLock;
    Condition mQueueChangedCondition;

    AString mName;

    List<Event> mEventQueue;

    struct LooperThread;
    sp<LooperThread> mThread;
    bool mRunningLocally;

    // use a separate lock for reply handling, as it is always on another thread
    // use a central lock, however, to avoid creating a mutex for each reply
    Mutex mRepliesLock;
    Condition mRepliesCondition;

    // START --- methods used only by AMessage

    // posts a message on this looper with the given timeout
    void post(const sp<AMessage> &msg, int64_t delayUs);

    // creates a reply token to be used with this looper
    sp<AReplyToken> createReplyToken();
    // waits for a response for the reply token.  If status is OK, the response
    // is stored into the supplied variable.  Otherwise, it is unchanged.
    status_t awaitResponse(const sp<AReplyToken> &replyToken, sp<AMessage> *response);
    // posts a reply for a reply token.  If the reply could be successfully posted,
    // it returns OK. Otherwise, it returns an error value.
    status_t postReply(const sp<AReplyToken> &replyToken, const sp<AMessage> &msg);

    // END --- methods used only by AMessage

    bool loop();

    DISALLOW_EVIL_CONSTRUCTORS(ALooper);
};

} // namespace android

#endif  // A_LOOPER_H_
