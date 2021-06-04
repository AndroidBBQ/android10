/*
 * Copyright (C) 2015 The Android Open Source Project
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

#define LOG_TAG "APM::AudioPatch"
//#define LOG_NDEBUG 0

#include "AudioPatch.h"
#include "AudioGain.h"
#include "TypeConverter.h"

#include <log/log.h>
#include <utils/String8.h>

namespace android {

AudioPatch::AudioPatch(const struct audio_patch *patch, uid_t uid) :
    mHandle(HandleGenerator<audio_patch_handle_t>::getNextHandle()),
    mPatch(*patch),
    mUid(uid),
    mAfPatchHandle(AUDIO_PATCH_HANDLE_NONE)
{
}

static void dumpPatchEndpoints(
        String8 *dst, int spaces, const char *prefix, int count, const audio_port_config *cfgs)
{
    for (int i = 0; i < count; ++i) {
        const audio_port_config &cfg = cfgs[i];
        dst->appendFormat("%*s  [%s %d] ", spaces, "", prefix, i + 1);
        if (cfg.type == AUDIO_PORT_TYPE_DEVICE) {
            dst->appendFormat("Device ID %d %s", cfg.id, toString(cfg.ext.device.type).c_str());
        } else {
            dst->appendFormat("Mix ID %d I/O handle %d", cfg.id, cfg.ext.mix.handle);
        }
        dst->append("\n");
    }
}

void AudioPatch::dump(String8 *dst, int spaces, int index) const
{
    dst->appendFormat("%*sPatch %d: owner uid %4d, handle %2d, af handle %2d\n",
            spaces, "", index + 1, mUid, mHandle, mAfPatchHandle);
    dumpPatchEndpoints(dst, spaces, "src ", mPatch.num_sources, mPatch.sources);
    dumpPatchEndpoints(dst, spaces, "sink", mPatch.num_sinks, mPatch.sinks);
}

status_t AudioPatchCollection::addAudioPatch(audio_patch_handle_t handle,
                                             const sp<AudioPatch>& patch)
{
    ssize_t index = indexOfKey(handle);

    if (index >= 0) {
        ALOGW("addAudioPatch() patch %d already in", handle);
        return ALREADY_EXISTS;
    }
    add(handle, patch);
    ALOGV("addAudioPatch() handle %d af handle %d num_sources %d num_sinks %d source handle %d"
            "sink handle %d",
          handle, patch->mAfPatchHandle, patch->mPatch.num_sources, patch->mPatch.num_sinks,
          patch->mPatch.sources[0].id, patch->mPatch.sinks[0].id);
    return NO_ERROR;
}

status_t AudioPatchCollection::removeAudioPatch(audio_patch_handle_t handle)
{
    ssize_t index = indexOfKey(handle);

    if (index < 0) {
        ALOGW("removeAudioPatch() patch %d not in", handle);
        return ALREADY_EXISTS;
    }
    ALOGV("removeAudioPatch() handle %d af handle %d", handle, valueAt(index)->mAfPatchHandle);
    removeItemsAt(index);
    return NO_ERROR;
}

status_t AudioPatchCollection::listAudioPatches(unsigned int *num_patches,
                                                struct audio_patch *patches) const
{
    if (num_patches == NULL || (*num_patches != 0 && patches == NULL)) {
        return BAD_VALUE;
    }
    ALOGV("listAudioPatches() num_patches %d patches %p available patches %zu",
          *num_patches, patches, size());
    if (patches == NULL) {
        *num_patches = 0;
    }

    size_t patchesWritten = 0;
    size_t patchesMax = *num_patches;
    *num_patches = 0;
    for (size_t patchIndex = 0; patchIndex < size(); patchIndex++) {
        // do not report patches with AUDIO_DEVICE_IN_STUB as source or
        // AUDIO_DEVICE_OUT_STUB as sink as those devices are used by stub HALs by convention
        const sp<AudioPatch> patch = valueAt(patchIndex);
        bool skip = false;
        for (size_t srcIndex = 0; srcIndex < patch->mPatch.num_sources && !skip; srcIndex++) {
            if (patch->mPatch.sources[srcIndex].type == AUDIO_PORT_TYPE_DEVICE &&
                    patch->mPatch.sources[srcIndex].ext.device.type == AUDIO_DEVICE_IN_STUB) {
                skip = true;
            }
        }
        for (size_t sinkIndex = 0; sinkIndex < patch->mPatch.num_sinks && !skip; sinkIndex++) {
            if (patch->mPatch.sinks[sinkIndex].type == AUDIO_PORT_TYPE_DEVICE &&
                    patch->mPatch.sinks[sinkIndex].ext.device.type == AUDIO_DEVICE_OUT_STUB) {
                skip = true;
            }
        }
        if (skip) {
            continue; // to next audio patch
        }
        if (patchesWritten < patchesMax) {
            patches[patchesWritten] = patch->mPatch;
            patches[patchesWritten++].id = patch->mHandle;
        }
        (*num_patches)++;
        ALOGV("listAudioPatches() patch %zu num_sources %d num_sinks %d",
              patchIndex, patch->mPatch.num_sources, patch->mPatch.num_sinks);
    }

    ALOGV("listAudioPatches() got %zu patches needed %d", patchesWritten, *num_patches);
    return NO_ERROR;
}

void AudioPatchCollection::dump(String8 *dst) const
{
    dst->append("\nAudio Patches:\n");
    for (size_t i = 0; i < size(); i++) {
        valueAt(i)->dump(dst, 2, i);
    }
}

} // namespace android
