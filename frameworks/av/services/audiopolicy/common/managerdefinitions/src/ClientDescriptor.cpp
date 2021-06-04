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

#define LOG_TAG "APM_ClientDescriptor"
//#define LOG_NDEBUG 0

#include <sstream>
#include <utils/Log.h>
#include <utils/String8.h>
#include <TypeConverter.h>
#include "AudioGain.h"
#include "AudioOutputDescriptor.h"
#include "AudioPatch.h"
#include "ClientDescriptor.h"
#include "DeviceDescriptor.h"
#include "HwModule.h"
#include "IOProfile.h"

namespace android {

std::string ClientDescriptor::toShortString() const
{
    std::stringstream ss;

    ss << "PortId: " << mPortId << " SessionId: " << mSessionId << " Uid: " << mUid;
    return ss.str();
}

void ClientDescriptor::dump(String8 *dst, int spaces, int index) const
{
    dst->appendFormat("%*sClient %d:\n", spaces, "", index+1);
    dst->appendFormat("%*s- Port Id: %d Session Id: %d UID: %d\n", spaces, "",
             mPortId, mSessionId, mUid);
    dst->appendFormat("%*s- Format: %08x Sampling rate: %d Channels: %08x\n", spaces, "",
             mConfig.format, mConfig.sample_rate, mConfig.channel_mask);
    dst->appendFormat("%*s- Attributes: %s\n", spaces, "", toString(mAttributes).c_str());
    dst->appendFormat("%*s- Preferred Device Id: %08x\n", spaces, "", mPreferredDeviceId);
    dst->appendFormat("%*s- State: %s\n", spaces, "", mActive ? "Active" : "Inactive");
}

void TrackClientDescriptor::dump(String8 *dst, int spaces, int index) const
{
    ClientDescriptor::dump(dst, spaces, index);
    dst->appendFormat("%*s- Stream: %d flags: %08x\n", spaces, "", mStream, mFlags);
    dst->appendFormat("%*s- Refcount: %d\n", spaces, "", mActivityCount);
}

std::string TrackClientDescriptor::toShortString() const
{
    std::stringstream ss;

    ss << ClientDescriptor::toShortString() << " Stream: " << mStream;
    return ss.str();
}

void RecordClientDescriptor::trackEffectEnabled(const sp<EffectDescriptor> &effect, bool enabled)
{
    if (enabled) {
        mEnabledEffects.replaceValueFor(effect->mId, effect);
    } else {
        mEnabledEffects.removeItem(effect->mId);
    }
}

void RecordClientDescriptor::dump(String8 *dst, int spaces, int index) const
{
    ClientDescriptor::dump(dst, spaces, index);
    dst->appendFormat("%*s- Source: %d flags: %08x\n", spaces, "", mSource, mFlags);
    mEnabledEffects.dump(dst, spaces + 2 /*spaces*/, false /*verbose*/);
}

SourceClientDescriptor::SourceClientDescriptor(audio_port_handle_t portId, uid_t uid,
         audio_attributes_t attributes, const sp<AudioPatch>& patchDesc,
         const sp<DeviceDescriptor>& srcDevice, audio_stream_type_t stream,
         product_strategy_t strategy, VolumeSource volumeSource) :
    TrackClientDescriptor::TrackClientDescriptor(portId, uid, AUDIO_SESSION_NONE, attributes,
        AUDIO_CONFIG_BASE_INITIALIZER, AUDIO_PORT_HANDLE_NONE,
        stream, strategy, volumeSource, AUDIO_OUTPUT_FLAG_NONE, false,
        {} /* Sources do not support secondary outputs*/),
        mPatchDesc(patchDesc), mSrcDevice(srcDevice)
{
}

void SourceClientDescriptor::setSwOutput(const sp<SwAudioOutputDescriptor>& swOutput)
{
    mSwOutput = swOutput;
}

void SourceClientDescriptor::setHwOutput(const sp<HwAudioOutputDescriptor>& hwOutput)
{
    mHwOutput = hwOutput;
}

void SourceClientDescriptor::dump(String8 *dst, int spaces, int index) const
{
    TrackClientDescriptor::dump(dst, spaces, index);
    dst->appendFormat("%*s- Device:\n", spaces, "");
    mSrcDevice->dump(dst, 2, 0);
}

void SourceClientCollection::dump(String8 *dst) const
{
    dst->append("\nAudio sources:\n");
    for (size_t i = 0; i < size(); i++) {
        valueAt(i)->dump(dst, 2, i);
    }
}

}; //namespace android
