/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef OGG_WRITER_H_

#define OGG_WRITER_H_

#include <stdio.h>

#include <media/stagefright/MediaWriter.h>
#include <utils/threads.h>

struct OggStreamState;

namespace android {

struct OggWriter : public MediaWriter {
    OggWriter(int fd);

    status_t initCheck() const;

    virtual status_t addSource(const sp<MediaSource>& source);
    virtual bool reachedEOS();
    virtual status_t start(MetaData* params = NULL);
    virtual status_t stop() { return reset(); }
    virtual status_t pause();

protected:
    ~OggWriter();

private:
    int mFd;
    bool mHaveAllCodecSpecificData;
    status_t mInitCheck;
    sp<MediaSource> mSource;
    bool mStarted = false;
    volatile bool mPaused = false;
    volatile bool mResumed = false;
    volatile bool mDone;
    volatile bool mReachedEOS;
    pthread_t mThread;
    int64_t mSampleRate;
    int64_t mEstimatedSizeBytes;
    int64_t mEstimatedDurationUs;

    static void* ThreadWrapper(void*);
    status_t threadFunc();
    bool exceedsFileSizeLimit();
    bool exceedsFileDurationLimit();
    status_t reset();

    int32_t mCurrentPacketId;
    OggStreamState* mOs = nullptr;

    OggWriter(const OggWriter&);
    OggWriter& operator=(const OggWriter&);

    status_t writeOggHeaderPackets(unsigned char *buf, size_t size);
};

}  // namespace android

#endif  // OGG_WRITER_H_
