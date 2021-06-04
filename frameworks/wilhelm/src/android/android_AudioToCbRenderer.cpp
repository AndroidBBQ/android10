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

//#define USE_LOG SLAndroidLogLevel_Verbose

#include "sles_allinclusive.h"
#include "android/android_AudioToCbRenderer.h"

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaBuffer.h>

namespace android {

//--------------------------------------------------------------------------------------------------
AudioToCbRenderer::AudioToCbRenderer(const AudioPlayback_Parameters* params) :
        AudioSfDecoder(params),
        mDecodeCbf(NULL),
        mDecodeUser(NULL)
{
    SL_LOGD("AudioToCbRenderer::AudioToCbRenderer()");

}


AudioToCbRenderer::~AudioToCbRenderer() {
    SL_LOGD("AudioToCbRenderer::~AudioToCbRenderer()");

}

void AudioToCbRenderer::setDataPushListener(const data_push_cbf_t pushCbf, CAudioPlayer* pushUser) {
    mDecodeCbf = pushCbf;
    mDecodeUser = pushUser;
}

//--------------------------------------------------
// Event handlers

void AudioToCbRenderer::onRender() {
    SL_LOGV("AudioToCbRenderer::onRender");

    Mutex::Autolock _l(mBufferSourceLock);

    if (NULL == mDecodeBuffer) {
        // nothing to render, move along
        //SL_LOGV("AudioToCbRenderer::onRender NULL buffer, exiting");
        return;
    }

    if (mStateFlags & kFlagPlaying) {
        if (NULL != mDecodeCbf) {
            size_t full = mDecodeBuffer->range_length();
            size_t consumed = 0;
            size_t offset = 0;
            while (offset < full) {
                consumed = mDecodeCbf(
                        (const uint8_t *)mDecodeBuffer->data()
                                + offset + mDecodeBuffer->range_offset(),
                        mDecodeBuffer->range_length() - offset,
                        mDecodeUser);
                offset += consumed;
                //SL_LOGV("consumed=%u, offset=%u, full=%u", consumed, offset, full);
                if (consumed == 0) {
                    // decoded data is not being consumed, skip this buffer
                    break;
                }
            }
        }
        (new AMessage(kWhatDecode, this))->post();
    }

    mDecodeBuffer->release();
    mDecodeBuffer = NULL;

    updateOneShot();
}


//--------------------------------------------------
// Audio output
void AudioToCbRenderer::createAudioSink() {
    SL_LOGD("AudioToCbRenderer::createAudioSink()");
}


void AudioToCbRenderer::updateAudioSink() {
    SL_LOGD("AudioToCbRenderer::updateAudioSink()");
}


void AudioToCbRenderer::startAudioSink() {
    SL_LOGD("AudioToCbRenderer::startAudioSink()");
}


void AudioToCbRenderer::pauseAudioSink() {
    SL_LOGD("AudioToCbRenderer::pauseAudioSink()");
}

} // namespace android
