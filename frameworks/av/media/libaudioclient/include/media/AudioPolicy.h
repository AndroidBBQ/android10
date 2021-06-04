/*
 * Copyright (C) 2014 The Android Open Source Project
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


#ifndef ANDROID_AUDIO_POLICY_H
#define ANDROID_AUDIO_POLICY_H

#include <system/audio.h>
#include <system/audio_policy.h>
#include <binder/Parcel.h>
#include <utils/String8.h>
#include <utils/Vector.h>

namespace android {

// Keep in sync with AudioMix.java, AudioMixingRule.java, AudioPolicyConfig.java
#define RULE_EXCLUSION_MASK 0x8000
#define RULE_MATCH_ATTRIBUTE_USAGE           0x1
#define RULE_MATCH_ATTRIBUTE_CAPTURE_PRESET (0x1 << 1)
#define RULE_MATCH_UID                      (0x1 << 2)
#define RULE_EXCLUDE_ATTRIBUTE_USAGE  (RULE_EXCLUSION_MASK|RULE_MATCH_ATTRIBUTE_USAGE)
#define RULE_EXCLUDE_ATTRIBUTE_CAPTURE_PRESET \
                                      (RULE_EXCLUSION_MASK|RULE_MATCH_ATTRIBUTE_CAPTURE_PRESET)
#define RULE_EXCLUDE_UID              (RULE_EXCLUSION_MASK|RULE_MATCH_UID)

#define MIX_TYPE_INVALID (-1)
#define MIX_TYPE_PLAYERS 0
#define MIX_TYPE_RECORDERS 1

// definition of the different events that can be reported on a dynamic policy from
//   AudioSystem's implementation of the AudioPolicyClient interface
// keep in sync with AudioSystem.java
#define DYNAMIC_POLICY_EVENT_MIX_STATE_UPDATE 0

#define MIX_STATE_DISABLED (-1)
#define MIX_STATE_IDLE 0
#define MIX_STATE_MIXING 1

/** Control to which device some audio is rendered */
#define MIX_ROUTE_FLAG_RENDER 0x1
/** Loop back some audio instead of rendering it */
#define MIX_ROUTE_FLAG_LOOP_BACK (0x1 << 1)
/** Loop back some audio while it is rendered */
#define MIX_ROUTE_FLAG_LOOP_BACK_AND_RENDER (MIX_ROUTE_FLAG_RENDER | MIX_ROUTE_FLAG_LOOP_BACK)
#define MIX_ROUTE_FLAG_ALL (MIX_ROUTE_FLAG_RENDER | MIX_ROUTE_FLAG_LOOP_BACK)

#define MAX_MIXES_PER_POLICY 10
#define MAX_CRITERIA_PER_MIX 20

class AudioDeviceTypeAddr {
public:
    AudioDeviceTypeAddr() {}
    AudioDeviceTypeAddr(audio_devices_t type, String8 address) :
        mType(type), mAddress(address) {}

    status_t readFromParcel(Parcel *parcel);
    status_t writeToParcel(Parcel *parcel) const;

    audio_devices_t mType;
    String8 mAddress;
};

class AudioMixMatchCriterion {
public:
    AudioMixMatchCriterion() {}
    AudioMixMatchCriterion(audio_usage_t usage, audio_source_t source, uint32_t rule);

    status_t readFromParcel(Parcel *parcel);
    status_t writeToParcel(Parcel *parcel) const;

    union {
        audio_usage_t   mUsage;
        audio_source_t  mSource;
        uid_t           mUid;
    } mValue;
    uint32_t        mRule;
};

class AudioMix {
public:
    // flag on an AudioMix indicating the activity on this mix (IDLE, MIXING)
    //   must be reported through the AudioPolicyClient interface
    static const uint32_t kCbFlagNotifyActivity = 0x1;

    AudioMix() {}
    AudioMix(Vector<AudioMixMatchCriterion> criteria, uint32_t mixType, audio_config_t format,
             uint32_t routeFlags, String8 registrationId, uint32_t flags) :
        mCriteria(criteria), mMixType(mixType), mFormat(format),
        mRouteFlags(routeFlags), mDeviceAddress(registrationId), mCbFlags(flags){}

    status_t readFromParcel(Parcel *parcel);
    status_t writeToParcel(Parcel *parcel) const;

    void setExcludeUid(uid_t uid) const;
    void setMatchUid(uid_t uid) const;
    /** returns true if this mix has a rule to match or exclude the given uid */
    bool hasUidRule(bool match, uid_t uid) const;
    /** returns true if this mix has a rule for uid match (any uid) */
    bool hasMatchUidRule() const;
    /** returns true if this mix can be used for uid-device affinity routing */
    bool isDeviceAffinityCompatible() const;

    mutable Vector<AudioMixMatchCriterion> mCriteria;
    uint32_t        mMixType;
    audio_config_t  mFormat;
    uint32_t        mRouteFlags;
    audio_devices_t mDeviceType;
    String8         mDeviceAddress;
    uint32_t        mCbFlags; // flags indicating which callbacks to use, see kCbFlag*
    /** Ignore the AUDIO_FLAG_NO_MEDIA_PROJECTION */
    bool            mAllowPrivilegedPlaybackCapture = false;
};


// definitions for audio recording configuration updates;
// keep in sync with AudioManager.java for values used from native code
#define RECORD_CONFIG_EVENT_START  0
#define RECORD_CONFIG_EVENT_STOP   1
#define RECORD_CONFIG_EVENT_UPDATE 2

static inline bool is_mix_loopback_render(uint32_t routeFlags) {
    return (routeFlags & MIX_ROUTE_FLAG_LOOP_BACK_AND_RENDER)
           == MIX_ROUTE_FLAG_LOOP_BACK_AND_RENDER;
}

}; // namespace android

#endif  // ANDROID_AUDIO_POLICY_H
