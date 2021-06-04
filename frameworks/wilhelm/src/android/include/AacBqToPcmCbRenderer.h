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

#ifndef AAC_BQ_TO_PCM_H_
#define AAC_BQ_TO_PCM_H_

#include "android/android_AudioToCbRenderer.h"
#include "android/BufferQueueSource.h"
#include "android/include/AacAdtsExtractor.h"

//--------------------------------------------------------------------------------------------------
namespace android {

// Class to receive AAC ADTS data through an Android Buffer Queue, decode it and output
// the PCM samples through a registered callback (just like its superclass, AudioToCbRenderer).
// The implementation mainly overrides AudioSfDecoder::onPrepare() for the specificities
// of the data source creation, but all other behavior remains the same (e.g. PCM format metadata)

class AacBqToPcmCbRenderer : public AudioToCbRenderer
{
public:

    AacBqToPcmCbRenderer(const AudioPlayback_Parameters* params,
            IAndroidBufferQueue *androidBufferQueue);
    virtual ~AacBqToPcmCbRenderer();

    // verifies the given memory starts and ends on ADTS frame boundaries.
    // This is for instance used whenever ADTS data is being enqueued through an
    // SL / XA AndroidBufferQueue interface so only parseable ADTS data goes in
    // the buffer queue, and no ADTS frame is stored across two buffers.
    static SLresult validateBufferStartEndOnFrameBoundaries(void* data, size_t size);

protected:

    // Async event handlers (called from GenericPlayer's event loop)
    virtual void onPrepare();


private:
    const sp<BufferQueueSource> mBqSource;

private:
    DISALLOW_EVIL_CONSTRUCTORS(AacBqToPcmCbRenderer);

};

} // namespace android

#endif //AAC_BQ_TO_PCM_H_
