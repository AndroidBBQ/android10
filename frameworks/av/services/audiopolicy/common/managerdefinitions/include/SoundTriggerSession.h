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

#pragma once

#include <system/audio.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>

namespace android {

class SoundTriggerSessionCollection : public DefaultKeyedVector<audio_session_t, audio_io_handle_t>
{
public:
    status_t releaseSession(audio_session_t session);

    status_t acquireSession(audio_session_t session, audio_io_handle_t ioHandle);
};

} // namespace android
