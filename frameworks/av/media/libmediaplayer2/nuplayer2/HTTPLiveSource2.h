/*
 * Copyright 2017 The Android Open Source Project
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

#ifndef HTTP_LIVE_SOURCE2_H_

#define HTTP_LIVE_SOURCE2_H_

#include "NuPlayer2.h"
#include "NuPlayer2Source.h"

#include "LiveSession.h"

namespace android {

struct LiveSession;

struct NuPlayer2::HTTPLiveSource2 : public NuPlayer2::Source {
    HTTPLiveSource2(
            const sp<AMessage> &notify,
            const sp<MediaHTTPService> &httpService,
            const char *url,
            const KeyedVector<String8, String8> *headers);

    virtual status_t getBufferingSettings(
            BufferingSettings* buffering /* nonnull */) override;
    virtual status_t setBufferingSettings(const BufferingSettings& buffering) override;

    virtual void prepareAsync(int64_t startTimeUs);
    virtual void start();

    virtual status_t dequeueAccessUnit(bool audio, sp<ABuffer> *accessUnit);
    virtual sp<MetaData> getFormatMeta(bool audio);
    virtual sp<AMessage> getFormat(bool audio);

    virtual status_t feedMoreTSData();
    virtual status_t getDuration(int64_t *durationUs);
    virtual size_t getTrackCount() const;
    virtual sp<AMessage> getTrackInfo(size_t trackIndex) const;
    virtual ssize_t getSelectedTrack(media_track_type /* type */) const;
    virtual status_t selectTrack(size_t trackIndex, bool select, int64_t timeUs);
    virtual status_t seekTo(
            int64_t seekTimeUs,
            MediaPlayer2SeekMode mode = MediaPlayer2SeekMode::SEEK_PREVIOUS_SYNC) override;

protected:
    virtual ~HTTPLiveSource2();

    virtual void onMessageReceived(const sp<AMessage> &msg);

private:
    enum Flags {
        // Don't log any URLs.
        kFlagIncognito = 1,
    };

    enum {
        kWhatSessionNotify,
        kWhatFetchSubtitleData,
        kWhatFetchMetaData,
    };

    sp<MediaHTTPService> mHTTPService;
    AString mURL;
    KeyedVector<String8, String8> mExtraHeaders;
    uint32_t mFlags;
    status_t mFinalResult;
    off64_t mOffset;
    sp<ALooper> mLiveLooper;
    sp<LiveSession> mLiveSession;
    int32_t mFetchSubtitleDataGeneration;
    int32_t mFetchMetaDataGeneration;
    bool mHasMetadata;
    bool mMetadataSelected;
    BufferingSettings mBufferingSettings;

    void onSessionNotify(const sp<AMessage> &msg);
    void pollForRawData(
            const sp<AMessage> &msg, int32_t currentGeneration,
            LiveSession::StreamType fetchType, int32_t pushWhat);

    DISALLOW_EVIL_CONSTRUCTORS(HTTPLiveSource2);
};

}  // namespace android

#endif  // HTTP_LIVE_SOURCE2_H_
