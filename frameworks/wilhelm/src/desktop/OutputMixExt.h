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

/** \file OutputMixExt.h OutputMixExt interface */

#ifdef __cplusplus
extern "C" {
#endif
extern const SLInterfaceID SL_IID_OUTPUTMIXEXT;
#ifdef __cplusplus
}
#endif

typedef const struct SLOutputMixExtItf_ * const * SLOutputMixExtItf;

/** \brief v-table for IOutputMixExt */

struct SLOutputMixExtItf_ {
    void (*FillBuffer)(SLOutputMixExtItf self, void *pBuffer, SLuint32 size);
};

/** \brief Track describes each PCM input source to OutputMix */

typedef struct {
    struct BufferQueue_interface *mBufferQueue;
    CAudioPlayer *mAudioPlayer; ///< Mixer examines this track if non-NULL
    const void *mReader;    ///< Pointer to next frame in BufferHeader.mBuffer
    SLuint32 mAvail;        ///< Number of available bytes in the current buffer
    float mGains[STEREO_CHANNELS]; ///< Copied from CAudioPlayer::mGains
    SLuint32 mFramesMixed;  ///< Number of sample frames mixed from track; reset periodically
} Track;

extern SLresult IOutputMixExt_checkAudioPlayerSourceSink(CAudioPlayer *thiz);
extern void audioPlayerGainUpdate(CAudioPlayer *thiz);
extern void IOutputMixExt_FillBuffer(SLOutputMixExtItf self, void *pBuffer, SLuint32 size);
