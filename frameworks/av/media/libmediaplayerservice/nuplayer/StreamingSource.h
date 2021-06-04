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

#ifndef STREAMING_SOURCE_H_

#define STREAMING_SOURCE_H_

#include "NuPlayer.h"
#include "NuPlayerSource.h"

namespace android {

struct ABuffer;
struct ATSParser;
struct AnotherPacketSource;

struct NuPlayer::StreamingSource : public NuPlayer::Source {
    StreamingSource(
            const sp<AMessage> &notify,
            const sp<IStreamSource> &source);

    virtual status_t getBufferingSettings(
            BufferingSettings* buffering /* nonnull */) override;
    virtual status_t setBufferingSettings(const BufferingSettings& buffering) override;

    virtual void prepareAsync();
    virtual void start();

    virtual status_t feedMoreTSData();

    virtual status_t dequeueAccessUnit(bool audio, sp<ABuffer> *accessUnit);

    virtual bool isRealTime() const;

protected:
    virtual ~StreamingSource();

    virtual void onMessageReceived(const sp<AMessage> &msg);

    virtual sp<AMessage> getFormat(bool audio);

private:
    enum {
        kWhatReadBuffer,
    };
    sp<IStreamSource> mSource;
    status_t mFinalResult;
    sp<NuPlayerStreamListener> mStreamListener;
    sp<ATSParser> mTSParser;

    bool mBuffering;
    Mutex mBufferingLock;
    sp<ALooper> mLooper;

    void setError(status_t err);
    sp<AnotherPacketSource> getSource(bool audio);
    bool haveSufficientDataOnAllTracks();
    status_t postReadBuffer();
    void onReadBuffer();

    DISALLOW_EVIL_CONSTRUCTORS(StreamingSource);
};

}  // namespace android

#endif  // STREAMING_SOURCE_H_
