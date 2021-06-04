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

#ifndef AUDIO_TO_CB_RENDERER_H_
#define AUDIO_TO_CB_RENDERER_H_

#include "android/android_AudioSfDecoder.h"

//--------------------------------------------------------------------------------------------------
namespace android {

class AudioToCbRenderer : public AudioSfDecoder
{
public:

    explicit AudioToCbRenderer(const AudioPlayback_Parameters* params);
    virtual ~AudioToCbRenderer();

    void setDataPushListener(const data_push_cbf_t pushCbf, CAudioPlayer* pushUser);

protected:

    // Async event handlers (called from GenericPlayer's event loop)
    virtual void onRender();

    // overridden from AudioSfDecoder
    virtual void createAudioSink();
    virtual void updateAudioSink();
    virtual void startAudioSink();
    virtual void pauseAudioSink();

private:

    data_push_cbf_t mDecodeCbf;
    CAudioPlayer*   mDecodeUser;

private:
    DISALLOW_EVIL_CONSTRUCTORS(AudioToCbRenderer);

};

} // namespace android

#endif //AUDIO_TO_CB_RENDERER_H_
