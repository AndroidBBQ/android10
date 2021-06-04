/*
 * Copyright (C) 2015 The Android Open Source Project
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

#define LOG_TAG "APM::SoundTriggerSession"
//#define LOG_NDEBUG 0

#include "SoundTriggerSession.h"


namespace android {

status_t SoundTriggerSessionCollection::acquireSession(audio_session_t session,
                                                                   audio_io_handle_t ioHandle)
{
    add(session, ioHandle);

    return NO_ERROR;
}

status_t SoundTriggerSessionCollection::releaseSession(audio_session_t session)
{
    ssize_t index = indexOfKey(session);
    if (index < 0) {
        ALOGW("releaseSession() session %d not registered", session);
        return BAD_VALUE;
    }

    removeItem(session);
    return NO_ERROR;
}

}; //namespace android
