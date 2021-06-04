/*
**
** Copyright 2014, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/


#define LOG_TAG "AudioFlinger::PatchPanel"
//#define LOG_NDEBUG 0

#include "Configuration.h"
#include <utils/Log.h>
#include <audio_utils/primitives.h>

#include "AudioFlinger.h"
#include <media/AudioParameter.h>
#include <media/PatchBuilder.h>
#include <mediautils/ServiceUtilities.h>

// ----------------------------------------------------------------------------

// Note: the following macro is used for extremely verbose logging message.  In
// order to run with ALOG_ASSERT turned on, we need to have LOG_NDEBUG set to
// 0; but one side effect of this is to turn all LOGV's as well.  Some messages
// are so verbose that we want to suppress them even when we have ALOG_ASSERT
// turned on.  Do not uncomment the #def below unless you really know what you
// are doing and want to see all of the extremely verbose messages.
//#define VERY_VERY_VERBOSE_LOGGING
#ifdef VERY_VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
#else
#define ALOGVV(a...) do { } while(0)
#endif

namespace android {

/* List connected audio ports and their attributes */
status_t AudioFlinger::listAudioPorts(unsigned int *num_ports,
                                struct audio_port *ports)
{
    Mutex::Autolock _l(mLock);
    return mPatchPanel.listAudioPorts(num_ports, ports);
}

/* Get supported attributes for a given audio port */
status_t AudioFlinger::getAudioPort(struct audio_port *port)
{
    Mutex::Autolock _l(mLock);
    return mPatchPanel.getAudioPort(port);
}

/* Connect a patch between several source and sink ports */
status_t AudioFlinger::createAudioPatch(const struct audio_patch *patch,
                                   audio_patch_handle_t *handle)
{
    Mutex::Autolock _l(mLock);
    return mPatchPanel.createAudioPatch(patch, handle);
}

/* Disconnect a patch */
status_t AudioFlinger::releaseAudioPatch(audio_patch_handle_t handle)
{
    Mutex::Autolock _l(mLock);
    return mPatchPanel.releaseAudioPatch(handle);
}

/* List connected audio ports and they attributes */
status_t AudioFlinger::listAudioPatches(unsigned int *num_patches,
                                  struct audio_patch *patches)
{
    Mutex::Autolock _l(mLock);
    return mPatchPanel.listAudioPatches(num_patches, patches);
}

status_t AudioFlinger::PatchPanel::SoftwarePatch::getLatencyMs_l(double *latencyMs) const
{
    const auto& iter = mPatchPanel.mPatches.find(mPatchHandle);
    if (iter != mPatchPanel.mPatches.end()) {
        return iter->second.getLatencyMs(latencyMs);
    } else {
        return BAD_VALUE;
    }
}

/* List connected audio ports and their attributes */
status_t AudioFlinger::PatchPanel::listAudioPorts(unsigned int *num_ports __unused,
                                struct audio_port *ports __unused)
{
    ALOGV(__func__);
    return NO_ERROR;
}

/* Get supported attributes for a given audio port */
status_t AudioFlinger::PatchPanel::getAudioPort(struct audio_port *port __unused)
{
    ALOGV(__func__);
    return NO_ERROR;
}

/* Connect a patch between several source and sink ports */
status_t AudioFlinger::PatchPanel::createAudioPatch(const struct audio_patch *patch,
                                   audio_patch_handle_t *handle)
{
    if (handle == NULL || patch == NULL) {
        return BAD_VALUE;
    }
    ALOGV("%s() num_sources %d num_sinks %d handle %d",
            __func__, patch->num_sources, patch->num_sinks, *handle);
    status_t status = NO_ERROR;
    audio_patch_handle_t halHandle = AUDIO_PATCH_HANDLE_NONE;

    if (!audio_patch_is_valid(patch) || (patch->num_sinks == 0 && patch->num_sources != 2)) {
        return BAD_VALUE;
    }
    // limit number of sources to 1 for now or 2 sources for special cross hw module case.
    // only the audio policy manager can request a patch creation with 2 sources.
    if (patch->num_sources > 2) {
        return INVALID_OPERATION;
    }

    if (*handle != AUDIO_PATCH_HANDLE_NONE) {
        auto iter = mPatches.find(*handle);
        if (iter != mPatches.end()) {
            ALOGV("%s() removing patch handle %d", __func__, *handle);
            Patch &removedPatch = iter->second;
            // free resources owned by the removed patch if applicable
            // 1) if a software patch is present, release the playback and capture threads and
            // tracks created. This will also release the corresponding audio HAL patches
            if (removedPatch.isSoftware()) {
                removedPatch.clearConnections(this);
            }
            // 2) if the new patch and old patch source or sink are devices from different
            // hw modules,  clear the audio HAL patches now because they will not be updated
            // by call to create_audio_patch() below which will happen on a different HW module
            if (removedPatch.mHalHandle != AUDIO_PATCH_HANDLE_NONE) {
                audio_module_handle_t hwModule = AUDIO_MODULE_HANDLE_NONE;
                const struct audio_patch &oldPatch = removedPatch.mAudioPatch;
                if (oldPatch.sources[0].type == AUDIO_PORT_TYPE_DEVICE &&
                        (patch->sources[0].type != AUDIO_PORT_TYPE_DEVICE ||
                                oldPatch.sources[0].ext.device.hw_module !=
                                patch->sources[0].ext.device.hw_module)) {
                    hwModule = oldPatch.sources[0].ext.device.hw_module;
                } else if (patch->num_sinks == 0 ||
                        (oldPatch.sinks[0].type == AUDIO_PORT_TYPE_DEVICE &&
                                (patch->sinks[0].type != AUDIO_PORT_TYPE_DEVICE ||
                                        oldPatch.sinks[0].ext.device.hw_module !=
                                        patch->sinks[0].ext.device.hw_module))) {
                    // Note on (patch->num_sinks == 0): this situation should not happen as
                    // these special patches are only created by the policy manager but just
                    // in case, systematically clear the HAL patch.
                    // Note that removedPatch.mAudioPatch.num_sinks cannot be 0 here because
                    // removedPatch.mHalHandle would be AUDIO_PATCH_HANDLE_NONE in this case.
                    hwModule = oldPatch.sinks[0].ext.device.hw_module;
                }
                sp<DeviceHalInterface> hwDevice = findHwDeviceByModule(hwModule);
                if (hwDevice != 0) {
                    hwDevice->releaseAudioPatch(removedPatch.mHalHandle);
                }
            }
            mPatches.erase(iter);
            removeSoftwarePatchFromInsertedModules(*handle);
        }
    }

    Patch newPatch{*patch};
    audio_module_handle_t insertedModule = AUDIO_MODULE_HANDLE_NONE;

    switch (patch->sources[0].type) {
        case AUDIO_PORT_TYPE_DEVICE: {
            audio_module_handle_t srcModule = patch->sources[0].ext.device.hw_module;
            AudioHwDevice *audioHwDevice = findAudioHwDeviceByModule(srcModule);
            if (!audioHwDevice) {
                status = BAD_VALUE;
                goto exit;
            }
            for (unsigned int i = 0; i < patch->num_sinks; i++) {
                // support only one sink if connection to a mix or across HW modules
                if ((patch->sinks[i].type == AUDIO_PORT_TYPE_MIX ||
                                (patch->sinks[i].type == AUDIO_PORT_TYPE_DEVICE &&
                                        patch->sinks[i].ext.device.hw_module != srcModule)) &&
                        patch->num_sinks > 1) {
                    ALOGW("%s() multiple sinks for mix or across modules not supported", __func__);
                    status = INVALID_OPERATION;
                    goto exit;
                }
                // reject connection to different sink types
                if (patch->sinks[i].type != patch->sinks[0].type) {
                    ALOGW("%s() different sink types in same patch not supported", __func__);
                    status = BAD_VALUE;
                    goto exit;
                }
            }

            // manage patches requiring a software bridge
            // - special patch request with 2 sources (reuse one existing output mix) OR
            // - Device to device AND
            //    - source HW module != destination HW module OR
            //    - audio HAL does not support audio patches creation
            if ((patch->num_sources == 2) ||
                ((patch->sinks[0].type == AUDIO_PORT_TYPE_DEVICE) &&
                 ((patch->sinks[0].ext.device.hw_module != srcModule) ||
                  !audioHwDevice->supportsAudioPatches()))) {
                audio_devices_t outputDevice = patch->sinks[0].ext.device.type;
                String8 outputDeviceAddress = String8(patch->sinks[0].ext.device.address);
                if (patch->num_sources == 2) {
                    if (patch->sources[1].type != AUDIO_PORT_TYPE_MIX ||
                            (patch->num_sinks != 0 && patch->sinks[0].ext.device.hw_module !=
                                    patch->sources[1].ext.mix.hw_module)) {
                        ALOGW("%s() invalid source combination", __func__);
                        status = INVALID_OPERATION;
                        goto exit;
                    }

                    sp<ThreadBase> thread =
                            mAudioFlinger.checkPlaybackThread_l(patch->sources[1].ext.mix.handle);
                    if (thread == 0) {
                        ALOGW("%s() cannot get playback thread", __func__);
                        status = INVALID_OPERATION;
                        goto exit;
                    }
                    // existing playback thread is reused, so it is not closed when patch is cleared
                    newPatch.mPlayback.setThread(
                            reinterpret_cast<PlaybackThread*>(thread.get()), false /*closeThread*/);
                } else {
                    audio_config_t config = AUDIO_CONFIG_INITIALIZER;
                    audio_io_handle_t output = AUDIO_IO_HANDLE_NONE;
                    audio_output_flags_t flags = AUDIO_OUTPUT_FLAG_NONE;
                    if (patch->sinks[0].config_mask & AUDIO_PORT_CONFIG_SAMPLE_RATE) {
                        config.sample_rate = patch->sinks[0].sample_rate;
                    }
                    if (patch->sinks[0].config_mask & AUDIO_PORT_CONFIG_CHANNEL_MASK) {
                        config.channel_mask = patch->sinks[0].channel_mask;
                    }
                    if (patch->sinks[0].config_mask & AUDIO_PORT_CONFIG_FORMAT) {
                        config.format = patch->sinks[0].format;
                    }
                    if (patch->sinks[0].config_mask & AUDIO_PORT_CONFIG_FLAGS) {
                        flags = patch->sinks[0].flags.output;
                    }
                    sp<ThreadBase> thread = mAudioFlinger.openOutput_l(
                                                            patch->sinks[0].ext.device.hw_module,
                                                            &output,
                                                            &config,
                                                            outputDevice,
                                                            outputDeviceAddress,
                                                            flags);
                    ALOGV("mAudioFlinger.openOutput_l() returned %p", thread.get());
                    if (thread == 0) {
                        status = NO_MEMORY;
                        goto exit;
                    }
                    newPatch.mPlayback.setThread(reinterpret_cast<PlaybackThread*>(thread.get()));
                }
                audio_devices_t device = patch->sources[0].ext.device.type;
                String8 address = String8(patch->sources[0].ext.device.address);
                audio_config_t config = AUDIO_CONFIG_INITIALIZER;
                // open input stream with source device audio properties if provided or
                // default to peer output stream properties otherwise.
                if (patch->sources[0].config_mask & AUDIO_PORT_CONFIG_SAMPLE_RATE) {
                    config.sample_rate = patch->sources[0].sample_rate;
                } else {
                    config.sample_rate = newPatch.mPlayback.thread()->sampleRate();
                }
                if (patch->sources[0].config_mask & AUDIO_PORT_CONFIG_CHANNEL_MASK) {
                    config.channel_mask = patch->sources[0].channel_mask;
                } else {
                    config.channel_mask = audio_channel_in_mask_from_count(
                            newPatch.mPlayback.thread()->channelCount());
                }
                if (patch->sources[0].config_mask & AUDIO_PORT_CONFIG_FORMAT) {
                    config.format = patch->sources[0].format;
                } else {
                    config.format = newPatch.mPlayback.thread()->format();
                }
                audio_input_flags_t flags =
                        patch->sources[0].config_mask & AUDIO_PORT_CONFIG_FLAGS ?
                        patch->sources[0].flags.input : AUDIO_INPUT_FLAG_NONE;
                audio_io_handle_t input = AUDIO_IO_HANDLE_NONE;
                sp<ThreadBase> thread = mAudioFlinger.openInput_l(srcModule,
                                                                    &input,
                                                                    &config,
                                                                    device,
                                                                    address,
                                                                    AUDIO_SOURCE_MIC,
                                                                    flags,
                                                                    outputDevice,
                                                                    outputDeviceAddress);
                ALOGV("mAudioFlinger.openInput_l() returned %p inChannelMask %08x",
                      thread.get(), config.channel_mask);
                if (thread == 0) {
                    status = NO_MEMORY;
                    goto exit;
                }
                newPatch.mRecord.setThread(reinterpret_cast<RecordThread*>(thread.get()));
                status = newPatch.createConnections(this);
                if (status != NO_ERROR) {
                    goto exit;
                }
                if (audioHwDevice->isInsert()) {
                    insertedModule = audioHwDevice->handle();
                }
            } else {
                if (patch->sinks[0].type == AUDIO_PORT_TYPE_MIX) {
                    sp<ThreadBase> thread = mAudioFlinger.checkRecordThread_l(
                                                              patch->sinks[0].ext.mix.handle);
                    if (thread == 0) {
                        thread = mAudioFlinger.checkMmapThread_l(patch->sinks[0].ext.mix.handle);
                        if (thread == 0) {
                            ALOGW("%s() bad capture I/O handle %d",
                                    __func__, patch->sinks[0].ext.mix.handle);
                            status = BAD_VALUE;
                            goto exit;
                        }
                    }
                    status = thread->sendCreateAudioPatchConfigEvent(patch, &halHandle);
                    // remove stale audio patch with same input as sink if any
                    for (auto& iter : mPatches) {
                        if (iter.second.mAudioPatch.sinks[0].ext.mix.handle == thread->id()) {
                            mPatches.erase(iter.first);
                            break;
                        }
                    }
                } else {
                    sp<DeviceHalInterface> hwDevice = audioHwDevice->hwDevice();
                    status = hwDevice->createAudioPatch(patch->num_sources,
                                                        patch->sources,
                                                        patch->num_sinks,
                                                        patch->sinks,
                                                        &halHandle);
                    if (status == INVALID_OPERATION) goto exit;
                }
            }
        } break;
        case AUDIO_PORT_TYPE_MIX: {
            audio_module_handle_t srcModule =  patch->sources[0].ext.mix.hw_module;
            ssize_t index = mAudioFlinger.mAudioHwDevs.indexOfKey(srcModule);
            if (index < 0) {
                ALOGW("%s() bad src hw module %d", __func__, srcModule);
                status = BAD_VALUE;
                goto exit;
            }
            // limit to connections between devices and output streams
            audio_devices_t type = AUDIO_DEVICE_NONE;
            for (unsigned int i = 0; i < patch->num_sinks; i++) {
                if (patch->sinks[i].type != AUDIO_PORT_TYPE_DEVICE) {
                    ALOGW("%s() invalid sink type %d for mix source",
                            __func__, patch->sinks[i].type);
                    status = BAD_VALUE;
                    goto exit;
                }
                // limit to connections between sinks and sources on same HW module
                if (patch->sinks[i].ext.device.hw_module != srcModule) {
                    status = BAD_VALUE;
                    goto exit;
                }
                type |= patch->sinks[i].ext.device.type;
            }
            sp<ThreadBase> thread =
                            mAudioFlinger.checkPlaybackThread_l(patch->sources[0].ext.mix.handle);
            if (thread == 0) {
                thread = mAudioFlinger.checkMmapThread_l(patch->sources[0].ext.mix.handle);
                if (thread == 0) {
                    ALOGW("%s() bad playback I/O handle %d",
                            __func__, patch->sources[0].ext.mix.handle);
                    status = BAD_VALUE;
                    goto exit;
                }
            }
            if (thread == mAudioFlinger.primaryPlaybackThread_l()) {
                AudioParameter param = AudioParameter();
                param.addInt(String8(AudioParameter::keyRouting), (int)type);

                mAudioFlinger.broacastParametersToRecordThreads_l(param.toString());
            }

            status = thread->sendCreateAudioPatchConfigEvent(patch, &halHandle);

            // remove stale audio patch with same output as source if any
            for (auto& iter : mPatches) {
                if (iter.second.mAudioPatch.sources[0].ext.mix.handle == thread->id()) {
                    mPatches.erase(iter.first);
                    break;
                }
            }
        } break;
        default:
            status = BAD_VALUE;
            goto exit;
    }
exit:
    ALOGV("%s() status %d", __func__, status);
    if (status == NO_ERROR) {
        *handle = (audio_patch_handle_t) mAudioFlinger.nextUniqueId(AUDIO_UNIQUE_ID_USE_PATCH);
        newPatch.mHalHandle = halHandle;
        mPatches.insert(std::make_pair(*handle, std::move(newPatch)));
        if (insertedModule != AUDIO_MODULE_HANDLE_NONE) {
            addSoftwarePatchToInsertedModules(insertedModule, *handle);
        }
        ALOGV("%s() added new patch handle %d halHandle %d", __func__, *handle, halHandle);
    } else {
        newPatch.clearConnections(this);
    }
    return status;
}

AudioFlinger::PatchPanel::Patch::~Patch()
{
    ALOGE_IF(isSoftware(), "Software patch connections leaked %d %d",
            mRecord.handle(), mPlayback.handle());
}

status_t AudioFlinger::PatchPanel::Patch::createConnections(PatchPanel *panel)
{
    // create patch from source device to record thread input
    status_t status = panel->createAudioPatch(
            PatchBuilder().addSource(mAudioPatch.sources[0]).
                addSink(mRecord.thread(), { .source = AUDIO_SOURCE_MIC }).patch(),
            mRecord.handlePtr());
    if (status != NO_ERROR) {
        *mRecord.handlePtr() = AUDIO_PATCH_HANDLE_NONE;
        return status;
    }

    // create patch from playback thread output to sink device
    if (mAudioPatch.num_sinks != 0) {
        status = panel->createAudioPatch(
                PatchBuilder().addSource(mPlayback.thread()).addSink(mAudioPatch.sinks[0]).patch(),
                mPlayback.handlePtr());
        if (status != NO_ERROR) {
            *mPlayback.handlePtr() = AUDIO_PATCH_HANDLE_NONE;
            return status;
        }
    } else {
        *mPlayback.handlePtr() = AUDIO_PATCH_HANDLE_NONE;
    }

    // use a pseudo LCM between input and output framecount
    size_t playbackFrameCount = mPlayback.thread()->frameCount();
    int playbackShift = __builtin_ctz(playbackFrameCount);
    size_t recordFrameCount = mRecord.thread()->frameCount();
    int shift = __builtin_ctz(recordFrameCount);
    if (playbackShift < shift) {
        shift = playbackShift;
    }
    size_t frameCount = (playbackFrameCount * recordFrameCount) >> shift;
    ALOGV("%s() playframeCount %zu recordFrameCount %zu frameCount %zu",
            __func__, playbackFrameCount, recordFrameCount, frameCount);

    // create a special record track to capture from record thread
    uint32_t channelCount = mPlayback.thread()->channelCount();
    audio_channel_mask_t inChannelMask = audio_channel_in_mask_from_count(channelCount);
    audio_channel_mask_t outChannelMask = mPlayback.thread()->channelMask();
    uint32_t sampleRate = mPlayback.thread()->sampleRate();
    audio_format_t format = mPlayback.thread()->format();

    audio_format_t inputFormat = mRecord.thread()->format();
    if (!audio_is_linear_pcm(inputFormat)) {
        // The playbackThread format will say PCM for IEC61937 packetized stream.
        // Use recordThread format.
        format = inputFormat;
    }
    audio_input_flags_t inputFlags = mAudioPatch.sources[0].config_mask & AUDIO_PORT_CONFIG_FLAGS ?
            mAudioPatch.sources[0].flags.input : AUDIO_INPUT_FLAG_NONE;
    if (sampleRate == mRecord.thread()->sampleRate() &&
            inChannelMask == mRecord.thread()->channelMask() &&
            mRecord.thread()->fastTrackAvailable() &&
            mRecord.thread()->hasFastCapture()) {
        // Create a fast track if the record thread has fast capture to get better performance.
        // Only enable fast mode when there is no resample needed.
        inputFlags = (audio_input_flags_t) (inputFlags | AUDIO_INPUT_FLAG_FAST);
    } else {
        // Fast mode is not available in this case.
        inputFlags = (audio_input_flags_t) (inputFlags & ~AUDIO_INPUT_FLAG_FAST);
    }
    sp<RecordThread::PatchRecord> tempRecordTrack = new (std::nothrow) RecordThread::PatchRecord(
                                             mRecord.thread().get(),
                                             sampleRate,
                                             inChannelMask,
                                             format,
                                             frameCount,
                                             NULL,
                                             (size_t)0 /* bufferSize */,
                                             inputFlags);
    status = mRecord.checkTrack(tempRecordTrack.get());
    if (status != NO_ERROR) {
        return status;
    }

    audio_output_flags_t outputFlags = mAudioPatch.sinks[0].config_mask & AUDIO_PORT_CONFIG_FLAGS ?
            mAudioPatch.sinks[0].flags.output : AUDIO_OUTPUT_FLAG_NONE;
    audio_stream_type_t streamType = AUDIO_STREAM_PATCH;
    if (mAudioPatch.num_sources == 2 && mAudioPatch.sources[1].type == AUDIO_PORT_TYPE_MIX) {
        // "reuse one existing output mix" case
        streamType = mAudioPatch.sources[1].ext.mix.usecase.stream;
    }
    if (mPlayback.thread()->hasFastMixer()) {
        // Create a fast track if the playback thread has fast mixer to get better performance.
        // Note: we should have matching channel mask, sample rate, and format by the logic above.
        outputFlags = (audio_output_flags_t) (outputFlags | AUDIO_OUTPUT_FLAG_FAST);
    } else {
        outputFlags = (audio_output_flags_t) (outputFlags & ~AUDIO_OUTPUT_FLAG_FAST);
    }

    // create a special playback track to render to playback thread.
    // this track is given the same buffer as the PatchRecord buffer
    sp<PlaybackThread::PatchTrack> tempPatchTrack = new (std::nothrow) PlaybackThread::PatchTrack(
                                           mPlayback.thread().get(),
                                           streamType,
                                           sampleRate,
                                           outChannelMask,
                                           format,
                                           frameCount,
                                           tempRecordTrack->buffer(),
                                           tempRecordTrack->bufferSize(),
                                           outputFlags);
    status = mPlayback.checkTrack(tempPatchTrack.get());
    if (status != NO_ERROR) {
        return status;
    }

    // tie playback and record tracks together
    mRecord.setTrackAndPeer(tempRecordTrack, tempPatchTrack);
    mPlayback.setTrackAndPeer(tempPatchTrack, tempRecordTrack);

    // start capture and playback
    mRecord.track()->start(AudioSystem::SYNC_EVENT_NONE, AUDIO_SESSION_NONE);
    mPlayback.track()->start();

    return status;
}

void AudioFlinger::PatchPanel::Patch::clearConnections(PatchPanel *panel)
{
    ALOGV("%s() mRecord.handle %d mPlayback.handle %d",
            __func__, mRecord.handle(), mPlayback.handle());
    mRecord.stopTrack();
    mPlayback.stopTrack();
    mRecord.clearTrackPeer(); // mRecord stop is synchronous. Break PeerProxy sp<> cycle.
    mRecord.closeConnections(panel);
    mPlayback.closeConnections(panel);
}

status_t AudioFlinger::PatchPanel::Patch::getLatencyMs(double *latencyMs) const
{
    if (!isSoftware()) return INVALID_OPERATION;

    auto recordTrack = mRecord.const_track();
    if (recordTrack.get() == nullptr) return INVALID_OPERATION;

    auto playbackTrack = mPlayback.const_track();
    if (playbackTrack.get() == nullptr) return INVALID_OPERATION;

    // Latency information for tracks may be called without obtaining
    // the underlying thread lock.
    //
    // We use record server latency + playback track latency (generally smaller than the
    // reverse due to internal biases).
    //
    // TODO: is this stable enough? Consider a PatchTrack synchronized version of this.

    // For PCM tracks get server latency.
    if (audio_is_linear_pcm(recordTrack->format())) {
        double recordServerLatencyMs, playbackTrackLatencyMs;
        if (recordTrack->getServerLatencyMs(&recordServerLatencyMs) == OK
                && playbackTrack->getTrackLatencyMs(&playbackTrackLatencyMs) == OK) {
            *latencyMs = recordServerLatencyMs + playbackTrackLatencyMs;
            return OK;
        }
    }

    // See if kernel latencies are available.
    // If so, do a frame diff and time difference computation to estimate
    // the total patch latency. This requires that frame counts are reported by the
    // HAL are matched properly in the case of record overruns and playback underruns.
    ThreadBase::TrackBase::FrameTime recordFT{}, playFT{};
    recordTrack->getKernelFrameTime(&recordFT);
    playbackTrack->getKernelFrameTime(&playFT);
    if (recordFT.timeNs > 0 && playFT.timeNs > 0) {
        const int64_t frameDiff = recordFT.frames - playFT.frames;
        const int64_t timeDiffNs = recordFT.timeNs - playFT.timeNs;

        // It is possible that the patch track and patch record have a large time disparity because
        // one thread runs but another is stopped.  We arbitrarily choose the maximum timestamp
        // time difference based on how often we expect the timestamps to update in normal operation
        // (typical should be no more than 50 ms).
        //
        // If the timestamps aren't sampled close enough, the patch latency is not
        // considered valid.
        //
        // TODO: change this based on more experiments.
        constexpr int64_t maxValidTimeDiffNs = 200 * NANOS_PER_MILLISECOND;
        if (std::abs(timeDiffNs) < maxValidTimeDiffNs) {
            *latencyMs = frameDiff * 1e3 / recordTrack->sampleRate()
                   - timeDiffNs * 1e-6;
            return OK;
        }
    }

    return INVALID_OPERATION;
}

String8 AudioFlinger::PatchPanel::Patch::dump(audio_patch_handle_t myHandle) const
{
    // TODO: Consider table dump form for patches, just like tracks.
    String8 result = String8::format("Patch %d: thread %p => thread %p",
            myHandle, mRecord.const_thread().get(), mPlayback.const_thread().get());

    // add latency if it exists
    double latencyMs;
    if (getLatencyMs(&latencyMs) == OK) {
        result.appendFormat("  latency: %.2lf ms", latencyMs);
    }
    return result;
}

/* Disconnect a patch */
status_t AudioFlinger::PatchPanel::releaseAudioPatch(audio_patch_handle_t handle)
{
    ALOGV("%s handle %d", __func__, handle);
    status_t status = NO_ERROR;

    auto iter = mPatches.find(handle);
    if (iter == mPatches.end()) {
        return BAD_VALUE;
    }
    Patch &removedPatch = iter->second;
    const struct audio_patch &patch = removedPatch.mAudioPatch;

    const struct audio_port_config &src = patch.sources[0];
    switch (src.type) {
        case AUDIO_PORT_TYPE_DEVICE: {
            sp<DeviceHalInterface> hwDevice = findHwDeviceByModule(src.ext.device.hw_module);
            if (hwDevice == 0) {
                ALOGW("%s() bad src hw module %d", __func__, src.ext.device.hw_module);
                status = BAD_VALUE;
                break;
            }

            if (removedPatch.isSoftware()) {
                removedPatch.clearConnections(this);
                break;
            }

            if (patch.sinks[0].type == AUDIO_PORT_TYPE_MIX) {
                audio_io_handle_t ioHandle = patch.sinks[0].ext.mix.handle;
                sp<ThreadBase> thread = mAudioFlinger.checkRecordThread_l(ioHandle);
                if (thread == 0) {
                    thread = mAudioFlinger.checkMmapThread_l(ioHandle);
                    if (thread == 0) {
                        ALOGW("%s() bad capture I/O handle %d", __func__, ioHandle);
                        status = BAD_VALUE;
                        break;
                    }
                }
                status = thread->sendReleaseAudioPatchConfigEvent(removedPatch.mHalHandle);
            } else {
                status = hwDevice->releaseAudioPatch(removedPatch.mHalHandle);
            }
        } break;
        case AUDIO_PORT_TYPE_MIX: {
            if (findHwDeviceByModule(src.ext.mix.hw_module) == 0) {
                ALOGW("%s() bad src hw module %d", __func__, src.ext.mix.hw_module);
                status = BAD_VALUE;
                break;
            }
            audio_io_handle_t ioHandle = src.ext.mix.handle;
            sp<ThreadBase> thread = mAudioFlinger.checkPlaybackThread_l(ioHandle);
            if (thread == 0) {
                thread = mAudioFlinger.checkMmapThread_l(ioHandle);
                if (thread == 0) {
                    ALOGW("%s() bad playback I/O handle %d", __func__, ioHandle);
                    status = BAD_VALUE;
                    break;
                }
            }
            status = thread->sendReleaseAudioPatchConfigEvent(removedPatch.mHalHandle);
        } break;
        default:
            status = BAD_VALUE;
    }

    mPatches.erase(iter);
    removeSoftwarePatchFromInsertedModules(handle);
    return status;
}

/* List connected audio ports and they attributes */
status_t AudioFlinger::PatchPanel::listAudioPatches(unsigned int *num_patches __unused,
                                  struct audio_patch *patches __unused)
{
    ALOGV(__func__);
    return NO_ERROR;
}

status_t AudioFlinger::PatchPanel::getDownstreamSoftwarePatches(
        audio_io_handle_t stream,
        std::vector<AudioFlinger::PatchPanel::SoftwarePatch> *patches) const
{
    for (const auto& module : mInsertedModules) {
        if (module.second.streams.count(stream)) {
            for (const auto& patchHandle : module.second.sw_patches) {
                const auto& patch_iter = mPatches.find(patchHandle);
                if (patch_iter != mPatches.end()) {
                    const Patch &patch = patch_iter->second;
                    patches->emplace_back(*this, patchHandle,
                            patch.mPlayback.const_thread()->id(),
                            patch.mRecord.const_thread()->id());
                } else {
                    ALOGE("Stale patch handle in the cache: %d", patchHandle);
                }
            }
            return OK;
        }
    }
    // The stream is not associated with any of inserted modules.
    return BAD_VALUE;
}

void AudioFlinger::PatchPanel::notifyStreamOpened(
        AudioHwDevice *audioHwDevice, audio_io_handle_t stream)
{
    if (audioHwDevice->isInsert()) {
        mInsertedModules[audioHwDevice->handle()].streams.insert(stream);
    }
}

void AudioFlinger::PatchPanel::notifyStreamClosed(audio_io_handle_t stream)
{
    for (auto& module : mInsertedModules) {
        module.second.streams.erase(stream);
    }
}

AudioHwDevice* AudioFlinger::PatchPanel::findAudioHwDeviceByModule(audio_module_handle_t module)
{
    if (module == AUDIO_MODULE_HANDLE_NONE) return nullptr;
    ssize_t index = mAudioFlinger.mAudioHwDevs.indexOfKey(module);
    if (index < 0) {
        ALOGW("%s() bad hw module %d", __func__, module);
        return nullptr;
    }
    return mAudioFlinger.mAudioHwDevs.valueAt(index);
}

sp<DeviceHalInterface> AudioFlinger::PatchPanel::findHwDeviceByModule(audio_module_handle_t module)
{
    AudioHwDevice *audioHwDevice = findAudioHwDeviceByModule(module);
    return audioHwDevice ? audioHwDevice->hwDevice() : nullptr;
}

void AudioFlinger::PatchPanel::addSoftwarePatchToInsertedModules(
        audio_module_handle_t module, audio_patch_handle_t handle)
{
    mInsertedModules[module].sw_patches.insert(handle);
}

void AudioFlinger::PatchPanel::removeSoftwarePatchFromInsertedModules(
        audio_patch_handle_t handle)
{
    for (auto& module : mInsertedModules) {
        module.second.sw_patches.erase(handle);
    }
}

void AudioFlinger::PatchPanel::dump(int fd) const
{
    String8 patchPanelDump;
    const char *indent = "  ";

    // Only dump software patches.
    bool headerPrinted = false;
    for (const auto& iter : mPatches) {
        if (iter.second.isSoftware()) {
            if (!headerPrinted) {
                patchPanelDump += "\nSoftware patches:\n";
                headerPrinted = true;
            }
            patchPanelDump.appendFormat("%s%s\n", indent, iter.second.dump(iter.first).string());
        }
    }

    headerPrinted = false;
    for (const auto& module : mInsertedModules) {
        if (!module.second.streams.empty() || !module.second.sw_patches.empty()) {
            if (!headerPrinted) {
                patchPanelDump += "\nTracked inserted modules:\n";
                headerPrinted = true;
            }
            String8 moduleDump = String8::format("Module %d: I/O handles: ", module.first);
            for (const auto& stream : module.second.streams) {
                moduleDump.appendFormat("%d ", stream);
            }
            moduleDump.append("; SW Patches: ");
            for (const auto& patch : module.second.sw_patches) {
                moduleDump.appendFormat("%d ", patch);
            }
            patchPanelDump.appendFormat("%s%s\n", indent, moduleDump.string());
        }
    }

    if (!patchPanelDump.isEmpty()) {
        write(fd, patchPanelDump.string(), patchPanelDump.size());
    }
}

} // namespace android
