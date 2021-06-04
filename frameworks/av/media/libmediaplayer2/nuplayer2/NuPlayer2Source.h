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

#ifndef NUPLAYER2_SOURCE_H_

#define NUPLAYER2_SOURCE_H_

#include "NuPlayer2.h"

#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MetaData.h>
#include <mediaplayer2/mediaplayer2.h>
#include <utils/Vector.h>

namespace android {

struct ABuffer;
struct AMediaCryptoWrapper;
class MediaBuffer;

struct NuPlayer2::Source : public AHandler {
    enum Flags {
        FLAG_CAN_PAUSE          = 1,
        FLAG_CAN_SEEK_BACKWARD  = 2,  // the "10 sec back button"
        FLAG_CAN_SEEK_FORWARD   = 4,  // the "10 sec forward button"
        FLAG_CAN_SEEK           = 8,  // the "seek bar"
        FLAG_DYNAMIC_DURATION   = 16,
        FLAG_SECURE             = 32, // Secure codec is required.
        FLAG_PROTECTED          = 64, // The screen needs to be protected (screenshot is disabled).
    };

    enum {
        kWhatPrepared,
        kWhatFlagsChanged,
        kWhatVideoSizeChanged,
        kWhatBufferingUpdate,
        kWhatPauseOnBufferingStart,
        kWhatResumeOnBufferingEnd,
        kWhatCacheStats,
        kWhatSubtitleData,
        kWhatTimedTextData,
        kWhatTimedMetaData,
        kWhatQueueDecoderShutdown,
        kWhatDrmNoLicense,
        // Modular DRM
        kWhatDrmInfo,
    };

    // The provides message is used to notify the player about various
    // events.
    explicit Source(const sp<AMessage> &notify)
        : mNotify(notify) {
    }

    virtual status_t getBufferingSettings(
            BufferingSettings* buffering /* nonnull */) = 0;
    virtual status_t setBufferingSettings(const BufferingSettings& buffering) = 0;

    virtual void prepareAsync(int64_t startTimeUs) = 0;

    virtual void start() = 0;
    virtual void stop() {}
    virtual void pause() {}
    virtual void resume() {}

    // Explicitly disconnect the underling data source
    virtual void disconnect() {}

    // Returns OK iff more data was available,
    // an error or ERROR_END_OF_STREAM if not.
    virtual status_t feedMoreTSData() = 0;

    // Returns non-NULL format when the specified track exists.
    // When the format has "err" set to -EWOULDBLOCK, source needs more time to get valid meta data.
    // Returns NULL if the specified track doesn't exist or is invalid;
    virtual sp<AMessage> getFormat(bool audio);

    virtual sp<MetaData> getFormatMeta(bool /* audio */) { return NULL; }
    virtual sp<MetaData> getFileFormatMeta() const { return NULL; }

    virtual status_t dequeueAccessUnit(
            bool audio, sp<ABuffer> *accessUnit) = 0;

    virtual status_t getDuration(int64_t * /* durationUs */) {
        return INVALID_OPERATION;
    }

    virtual size_t getTrackCount() const {
        return 0;
    }

    virtual sp<AMessage> getTrackInfo(size_t /* trackIndex */) const {
        return NULL;
    }

    virtual ssize_t getSelectedTrack(media_track_type /* type */) const {
        return INVALID_OPERATION;
    }

    virtual status_t selectTrack(size_t /* trackIndex */, bool /* select */, int64_t /* timeUs*/) {
        return INVALID_OPERATION;
    }

    virtual status_t seekTo(
            int64_t /* seekTimeUs */,
            MediaPlayer2SeekMode /* mode */ = MediaPlayer2SeekMode::SEEK_PREVIOUS_SYNC) {
        return INVALID_OPERATION;
    }

    virtual bool isRealTime() const {
        return false;
    }

    virtual bool isStreaming() const {
        return true;
    }

    virtual void setOffloadAudio(bool /* offload */) {}

    // Modular DRM
    virtual status_t prepareDrm(
            const uint8_t /* uuid */[16], const Vector<uint8_t> & /* drmSessionId */,
            sp<AMediaCryptoWrapper> * /* crypto */) {
        return INVALID_OPERATION;
    }

    virtual status_t releaseDrm() {
        return INVALID_OPERATION;
    }

protected:
    virtual ~Source() {}

    virtual void onMessageReceived(const sp<AMessage> &msg);

    sp<AMessage> dupNotify() const { return mNotify->dup(); }

    void notifyFlagsChanged(uint32_t flags);
    void notifyVideoSizeChanged(const sp<AMessage> &format = NULL);
    void notifyPrepared(status_t err = OK);
    // Modular DRM
    void notifyDrmInfo(const sp<ABuffer> &buffer);

private:
    sp<AMessage> mNotify;

    DISALLOW_EVIL_CONSTRUCTORS(Source);
};

}  // namespace android

#endif  // NUPLAYER2_SOURCE_H_

