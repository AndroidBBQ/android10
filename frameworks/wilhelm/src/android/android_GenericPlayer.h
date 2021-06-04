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

#ifndef __ANDROID_GENERICPLAYER_H__
#define __ANDROID_GENERICPLAYER_H__

#include <media/stagefright/foundation/AHandler.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>

//--------------------------------------------------------------------------------------------------
/**
 * Message parameters for AHandler messages, see list in GenericPlayer::kWhatxxx
 */
#define WHATPARAM_SEEK_SEEKTIME_MS                  "seekTimeMs"
#define WHATPARAM_LOOP_LOOPING                      "looping"
#define WHATPARAM_BUFFERING_UPDATE                  "bufferingUpdate"
#define WHATPARAM_BUFFERING_UPDATETHRESHOLD_PERCENT "buffUpdateThreshold"
#define WHATPARAM_ATTACHAUXEFFECT                   "attachAuxEffect"
#define WHATPARAM_SETAUXEFFECTSENDLEVEL             "setAuxEffectSendLevel"
// Parameters for kWhatSetPlayEvents
#define WHATPARAM_SETPLAYEVENTS_FLAGS               "setPlayEventsFlags"
#define WHATPARAM_SETPLAYEVENTS_MARKER              "setPlayEventsMarker"
#define WHATPARAM_SETPLAYEVENTS_UPDATE              "setPlayEventsUpdate"
// Parameters for kWhatOneShot (see explanation at definition of kWhatOneShot below)
#define WHATPARAM_ONESHOT_GENERATION                "oneShotGeneration"

namespace android {

// abstract base class
class GenericPlayer : public AHandler
{
public:

    enum {
        kEventPrepared                = 0,
        kEventHasVideoSize            = 1,
        kEventPrefetchStatusChange    = 2,
        kEventPrefetchFillLevelUpdate = 3,
        kEventEndOfStream             = 4,
        kEventChannelCount            = 5,
        kEventPlay                    = 6, // SL_PLAYEVENT_*
        kEventErrorAfterPrepare       = 7, // error after successful prepare
    };


    explicit GenericPlayer(const AudioPlayback_Parameters* params);
    virtual ~GenericPlayer();

    void init(const notif_cbf_t cbf, void* notifUser);
    virtual void preDestroy();

    void setDataSource(const char *uri);
    void setDataSource(int fd, int64_t offset, int64_t length, bool closeAfterUse = false);

    void prepare();
    virtual void play();
    void pause();
    void stop();
    // timeMsec must be >= 0 or == ANDROID_UNKNOWN_TIME (used by StreamPlayer after discontinuity)
    void seek(int64_t timeMsec);
    void loop(bool loop);
    void setBufferingUpdateThreshold(int16_t thresholdPercent);

    void getDurationMsec(int* msec); //msec != NULL, ANDROID_UNKNOWN_TIME if unknown
    virtual void getPositionMsec(int* msec) = 0; //msec != NULL, ANDROID_UNKNOWN_TIME if unknown

    virtual void setVideoSurfaceTexture(const sp<IGraphicBufferProducer> &bufferProducer __unused)
            { }

    void setVolume(float leftVol, float rightVol);
    void attachAuxEffect(int32_t effectId);
    void setAuxEffectSendLevel(float level);

    virtual void setPlaybackRate(int32_t ratePermille);

    // Call after changing any of the IPlay settings related to SL_PLAYEVENT_*
    void setPlayEvents(int32_t eventFlags, int32_t markerPosition, int32_t positionUpdatePeriod);

protected:
    // mutex used for set vs use of volume, duration, and cache (fill, threshold) settings
    Mutex mSettingsLock;

    void resetDataLocator();
    DataLocator2 mDataLocator;
    int          mDataLocatorType;

    // Constants used to identify the messages in this player's AHandler message loop
    //   in onMessageReceived()
    enum {
        kWhatPrepare         = 0,  // start preparation
        kWhatNotif           = 1,  // send a notification to client
        kWhatPlay            = 2,  // start player
        kWhatPause           = 3,  // pause or stop player
        kWhatSeek            = 4,  // request a seek to specified position
        kWhatSeekComplete    = 5,  // seek request has completed
        kWhatLoop            = 6,  // set the player's looping status
        kWhatVolumeUpdate    = 7,  // set the channel gains to specified values
        kWhatBufferingUpdate = 8,
        kWhatBuffUpdateThres = 9,
        kWhatAttachAuxEffect = 10,
        kWhatSetAuxEffectSendLevel = 11,
        kWhatSetPlayEvents   = 12,  // process new IPlay settings related to SL_PLAYEVENT_*
        kWhatOneShot         = 13,  // deferred (non-0 timeout) handler for SL_PLAYEVENT_*
        // As used here, "one-shot" is the software equivalent of a "retriggerable monostable
        // multivibrator" from electronics.  Briefly, a one-shot is a timer that can be triggered
        // to fire at some point in the future.  It is "retriggerable" because while the timer
        // is active, it is possible to replace the current timeout value by a new value.
        // This is done by cancelling the current timer (using a generation count),
        // and then posting another timer with the new desired value.
    };

    // Send a notification to one of the event listeners
    virtual void notify(const char* event, int data1, bool async);
    virtual void notify(const char* event, int data1, int data2, bool async);

    // AHandler implementation
    virtual void onMessageReceived(const sp<AMessage> &msg);

    // Async event handlers (called from GenericPlayer's event loop)
    virtual void onPrepare();
    virtual void onNotify(const sp<AMessage> &msg);
    virtual void onPlay();
    virtual void onPause();
    virtual void onSeek(const sp<AMessage> &msg);
    virtual void onLoop(const sp<AMessage> &msg);
    virtual void onVolumeUpdate();
    virtual void onSeekComplete();
    virtual void onBufferingUpdate(const sp<AMessage> &msg);
    virtual void onSetBufferingUpdateThreshold(const sp<AMessage> &msg);
    virtual void onAttachAuxEffect(const sp<AMessage> &msg);
    virtual void onSetAuxEffectSendLevel(const sp<AMessage> &msg);
    void onSetPlayEvents(const sp<AMessage> &msg);
    void onOneShot(const sp<AMessage> &msg);

    // Convenience methods
    //   for async notifications of prefetch status and cache fill level, needs to be called
    //     with mSettingsLock locked
    void notifyStatus();
    void notifyCacheFill();
    //   for internal async notification to update state that the player is no longer seeking
    void seekComplete();
    void bufferingUpdate(int16_t fillLevelPerMille);

    // Event notification from GenericPlayer to OpenSL ES / OpenMAX AL framework
    notif_cbf_t mNotifyClient;
    void*       mNotifyUser;
    // lock to protect mNotifyClient and mNotifyUser updates
    Mutex       mNotifyClientLock;

    // Bits for mStateFlags
    enum {
        kFlagPrepared               = 1 << 0,   // use only for successful preparation
        kFlagPreparing              = 1 << 1,
        kFlagPlaying                = 1 << 2,
        kFlagBuffering              = 1 << 3,
        kFlagSeeking                = 1 << 4,   // set if we (not Stagefright) initiated a seek
        kFlagLooping                = 1 << 5,   // set if looping is enabled
        kFlagPreparedUnsuccessfully = 1 << 6,
    };

    // Only accessed from event loop, does not need a mutex
    uint32_t mStateFlags;

    sp<ALooper> mLooper;

    const AudioPlayback_Parameters mPlaybackParams;

    // protected by mSettingsLock after construction
    AndroidAudioLevels mAndroidAudioLevels;

    // protected by mSettingsLock
    int32_t mDurationMsec;
    int16_t mPlaybackRatePermille;

    CacheStatus_t mCacheStatus;
    int16_t mCacheFill; // cache fill level + played back level in permille
    int16_t mLastNotifiedCacheFill; // last cache fill level communicated to the listener
    int16_t mCacheFillNotifThreshold; // threshold in cache fill level for cache fill to be reported

    // Call any time any of the IPlay copies, current position, or play state changes, and
    // supply the latest known position or ANDROID_UNKNOWN_TIME if position is unknown to caller.
    void updateOneShot(int positionMs = ANDROID_UNKNOWN_TIME);

    // players that "render" data to present it to the user (a music player, a video player),
    // should return true, while players that only decode (hopefully faster than "real time")
    // should return false.
    virtual bool advancesPositionInRealTime() const { return true; }

private:

    // Our copy of some important IPlay member variables, except in Android units
    int32_t mEventFlags;
    int32_t mMarkerPositionMs;
    int32_t mPositionUpdatePeriodMs;

    // We need to be able to cancel any pending one-shot event(s) prior to posting
    // a new one-shot.  As AMessage does not currently support cancellation by
    // "what" category, we simulate this by keeping a generation counter for
    // one-shots.  When a one-shot event is delivered, it checks to see if it is
    // still the current one-shot.  If not, it returns immediately, thus
    // effectively cancelling itself.  Note that counter wrap-around is possible
    // but unlikely and benign.
    int32_t mOneShotGeneration;

    // Play position at time of the most recently delivered SL_PLAYEVENT_HEADATNEWPOS,
    // or ANDROID_UNKNOWN_TIME if a SL_PLAYEVENT_HEADATNEWPOS has never been delivered.
    int32_t mDeliveredNewPosMs;

    // Play position most recently observed by updateOneShot, or ANDROID_UNKNOWN_TIME
    // if the play position has never been observed.
    int32_t mObservedPositionMs;

    DISALLOW_EVIL_CONSTRUCTORS(GenericPlayer);
};

} // namespace android

extern void android_player_volumeUpdate(float *pVolumes /*[2]*/, const IVolume *volumeItf,
        unsigned channelCount, float amplFromDirectLevel, const bool *audibilityFactors /*[2]*/);

#endif /* __ANDROID_GENERICPLAYER_H__ */
