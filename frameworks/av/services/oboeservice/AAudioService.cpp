/*
 * Copyright (C) 2016 The Android Open Source Project
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

#define LOG_TAG "AAudioService"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <iomanip>
#include <iostream>
#include <sstream>

#include <aaudio/AAudio.h>
#include <mediautils/SchedulingPolicyService.h>
#include <mediautils/ServiceUtilities.h>
#include <utils/String16.h>

#include "binding/AAudioServiceMessage.h"
#include "AAudioClientTracker.h"
#include "AAudioEndpointManager.h"
#include "AAudioService.h"
#include "AAudioServiceStreamMMAP.h"
#include "AAudioServiceStreamShared.h"
#include "binding/IAAudioService.h"

using namespace android;
using namespace aaudio;

#define MAX_STREAMS_PER_PROCESS   8

using android::AAudioService;

android::AAudioService::AAudioService()
    : BnAAudioService() {
    mAudioClient.clientUid = getuid();   // TODO consider using geteuid()
    mAudioClient.clientPid = getpid();
    mAudioClient.packageName = String16("");
    AAudioClientTracker::getInstance().setAAudioService(this);
}

AAudioService::~AAudioService() {
}

status_t AAudioService::dump(int fd, const Vector<String16>& args) {
    std::string result;

    if (!dumpAllowed()) {
        std::stringstream ss;
        ss << "Permission Denial: can't dump AAudioService from pid="
                << IPCThreadState::self()->getCallingPid() << ", uid="
                << IPCThreadState::self()->getCallingUid() << "\n";
        result = ss.str();
        ALOGW("%s", result.c_str());
    } else {
        result = "------------ AAudio Service ------------\n"
                 + mStreamTracker.dump()
                 + AAudioClientTracker::getInstance().dump()
                 + AAudioEndpointManager::getInstance().dump();
    }
    (void)write(fd, result.c_str(), result.size());
    return NO_ERROR;
}

void AAudioService::registerClient(const sp<IAAudioClient>& client) {
    pid_t pid = IPCThreadState::self()->getCallingPid();
    AAudioClientTracker::getInstance().registerClient(pid, client);
}

aaudio_handle_t AAudioService::openStream(const aaudio::AAudioStreamRequest &request,
                                          aaudio::AAudioStreamConfiguration &configurationOutput) {
    aaudio_result_t result = AAUDIO_OK;
    sp<AAudioServiceStreamBase> serviceStream;
    const AAudioStreamConfiguration &configurationInput = request.getConstantConfiguration();
    bool sharingModeMatchRequired = request.isSharingModeMatchRequired();
    aaudio_sharing_mode_t sharingMode = configurationInput.getSharingMode();

    // Enforce limit on client processes.
    pid_t pid = request.getProcessId();
    if (pid != mAudioClient.clientPid) {
        int32_t count = AAudioClientTracker::getInstance().getStreamCount(pid);
        if (count >= MAX_STREAMS_PER_PROCESS) {
            ALOGE("openStream(): exceeded max streams per process %d >= %d",
                  count,  MAX_STREAMS_PER_PROCESS);
            return AAUDIO_ERROR_UNAVAILABLE;
        }
    }

    if (sharingMode != AAUDIO_SHARING_MODE_EXCLUSIVE && sharingMode != AAUDIO_SHARING_MODE_SHARED) {
        ALOGE("openStream(): unrecognized sharing mode = %d", sharingMode);
        return AAUDIO_ERROR_ILLEGAL_ARGUMENT;
    }

    if (sharingMode == AAUDIO_SHARING_MODE_EXCLUSIVE) {
        // only trust audioserver for in service indication
        bool inService = false;
        if (mAudioClient.clientPid == IPCThreadState::self()->getCallingPid() &&
                mAudioClient.clientUid == IPCThreadState::self()->getCallingUid()) {
            inService = request.isInService();
        }
        serviceStream = new AAudioServiceStreamMMAP(*this, inService);
        result = serviceStream->open(request);
        if (result != AAUDIO_OK) {
            // Clear it so we can possibly fall back to using a shared stream.
            ALOGW("openStream(), could not open in EXCLUSIVE mode");
            serviceStream.clear();
        }
    }

    // Try SHARED if SHARED requested or if EXCLUSIVE failed.
    if (sharingMode == AAUDIO_SHARING_MODE_SHARED) {
        serviceStream =  new AAudioServiceStreamShared(*this);
        result = serviceStream->open(request);
    } else if (serviceStream.get() == nullptr && !sharingModeMatchRequired) {
        aaudio::AAudioStreamRequest modifiedRequest = request;
        // Overwrite the original EXCLUSIVE mode with SHARED.
        modifiedRequest.getConfiguration().setSharingMode(AAUDIO_SHARING_MODE_SHARED);
        serviceStream =  new AAudioServiceStreamShared(*this);
        result = serviceStream->open(modifiedRequest);
    }

    if (result != AAUDIO_OK) {
        serviceStream.clear();
        return result;
    } else {
        aaudio_handle_t handle = mStreamTracker.addStreamForHandle(serviceStream.get());
        ALOGV("openStream(): handle = 0x%08X", handle);
        serviceStream->setHandle(handle);
        pid_t pid = request.getProcessId();
        AAudioClientTracker::getInstance().registerClientStream(pid, serviceStream);
        configurationOutput.copyFrom(*serviceStream);
        return handle;
    }
}

// If a close request is pending then close the stream
bool AAudioService::releaseStream(const sp<AAudioServiceStreamBase> &serviceStream) {
    bool closed = false;
    // decrementAndRemoveStreamByHandle() uses a lock so that if there are two simultaneous closes
    // then only one will get the pointer and do the close.
    sp<AAudioServiceStreamBase> foundStream = mStreamTracker.decrementAndRemoveStreamByHandle(
            serviceStream->getHandle());
    if (foundStream.get() != nullptr) {
        foundStream->close();
        pid_t pid = foundStream->getOwnerProcessId();
        AAudioClientTracker::getInstance().unregisterClientStream(pid, foundStream);
        closed = true;
    }
    return closed;
}

aaudio_result_t AAudioService::checkForPendingClose(
        const sp<AAudioServiceStreamBase> &serviceStream,
        aaudio_result_t defaultResult) {
    return releaseStream(serviceStream) ? AAUDIO_ERROR_INVALID_STATE : defaultResult;
}

aaudio_result_t AAudioService::closeStream(aaudio_handle_t streamHandle) {
    // Check permission and ownership first.
    sp<AAudioServiceStreamBase> serviceStream = convertHandleToServiceStream(streamHandle);
    if (serviceStream.get() == nullptr) {
        ALOGE("closeStream(0x%0x), illegal stream handle", streamHandle);
        return AAUDIO_ERROR_INVALID_HANDLE;
    }

    pid_t pid = serviceStream->getOwnerProcessId();
    AAudioClientTracker::getInstance().unregisterClientStream(pid, serviceStream);

    serviceStream->markCloseNeeded();
    (void) releaseStream(serviceStream);
    return AAUDIO_OK;
}

sp<AAudioServiceStreamBase> AAudioService::convertHandleToServiceStream(
        aaudio_handle_t streamHandle) {
    sp<AAudioServiceStreamBase> serviceStream = mStreamTracker.getStreamByHandleAndIncrement(
            streamHandle);
    if (serviceStream.get() != nullptr) {
        // Only allow owner or the aaudio service to access the stream.
        const uid_t callingUserId = IPCThreadState::self()->getCallingUid();
        const uid_t ownerUserId = serviceStream->getOwnerUserId();
        bool callerOwnsIt = callingUserId == ownerUserId;
        bool serverCalling = callingUserId == mAudioClient.clientUid;
        bool serverOwnsIt = ownerUserId == mAudioClient.clientUid;
        bool allowed = callerOwnsIt || serverCalling || serverOwnsIt;
        if (!allowed) {
            ALOGE("AAudioService: calling uid %d cannot access stream 0x%08X owned by %d",
                  callingUserId, streamHandle, ownerUserId);
            // We incremented the reference count so we must check if it needs to be closed.
            checkForPendingClose(serviceStream, AAUDIO_OK);
            serviceStream.clear();
        }
    }
    return serviceStream;
}

aaudio_result_t AAudioService::getStreamDescription(
                aaudio_handle_t streamHandle,
                aaudio::AudioEndpointParcelable &parcelable) {
    sp<AAudioServiceStreamBase> serviceStream = convertHandleToServiceStream(streamHandle);
    if (serviceStream.get() == nullptr) {
        ALOGE("getStreamDescription(), illegal stream handle = 0x%0x", streamHandle);
        return AAUDIO_ERROR_INVALID_HANDLE;
    }

    aaudio_result_t result = serviceStream->getDescription(parcelable);
    // parcelable.dump();
    return checkForPendingClose(serviceStream, result);
}

aaudio_result_t AAudioService::startStream(aaudio_handle_t streamHandle) {
    sp<AAudioServiceStreamBase> serviceStream = convertHandleToServiceStream(streamHandle);
    if (serviceStream.get() == nullptr) {
        ALOGE("startStream(), illegal stream handle = 0x%0x", streamHandle);
        return AAUDIO_ERROR_INVALID_HANDLE;
    }

    aaudio_result_t result = serviceStream->start();
    return checkForPendingClose(serviceStream, result);
}

aaudio_result_t AAudioService::pauseStream(aaudio_handle_t streamHandle) {
    sp<AAudioServiceStreamBase> serviceStream = convertHandleToServiceStream(streamHandle);
    if (serviceStream.get() == nullptr) {
        ALOGE("pauseStream(), illegal stream handle = 0x%0x", streamHandle);
        return AAUDIO_ERROR_INVALID_HANDLE;
    }
    aaudio_result_t result = serviceStream->pause();
    return checkForPendingClose(serviceStream, result);
}

aaudio_result_t AAudioService::stopStream(aaudio_handle_t streamHandle) {
    sp<AAudioServiceStreamBase> serviceStream = convertHandleToServiceStream(streamHandle);
    if (serviceStream.get() == nullptr) {
        ALOGE("stopStream(), illegal stream handle = 0x%0x", streamHandle);
        return AAUDIO_ERROR_INVALID_HANDLE;
    }
    aaudio_result_t result = serviceStream->stop();
    return checkForPendingClose(serviceStream, result);
}

aaudio_result_t AAudioService::flushStream(aaudio_handle_t streamHandle) {
    sp<AAudioServiceStreamBase> serviceStream = convertHandleToServiceStream(streamHandle);
    if (serviceStream.get() == nullptr) {
        ALOGE("flushStream(), illegal stream handle = 0x%0x", streamHandle);
        return AAUDIO_ERROR_INVALID_HANDLE;
    }
    aaudio_result_t result = serviceStream->flush();
    return checkForPendingClose(serviceStream, result);
}

aaudio_result_t AAudioService::registerAudioThread(aaudio_handle_t streamHandle,
                                                   pid_t clientThreadId,
                                                   int64_t periodNanoseconds) {
    aaudio_result_t result = AAUDIO_OK;
    sp<AAudioServiceStreamBase> serviceStream = convertHandleToServiceStream(streamHandle);
    if (serviceStream.get() == nullptr) {
        ALOGE("registerAudioThread(), illegal stream handle = 0x%0x", streamHandle);
        return AAUDIO_ERROR_INVALID_HANDLE;
    }
    if (serviceStream->getRegisteredThread() != AAudioServiceStreamBase::ILLEGAL_THREAD_ID) {
        ALOGE("AAudioService::registerAudioThread(), thread already registered");
        result = AAUDIO_ERROR_INVALID_STATE;
    } else {
        const pid_t ownerPid = IPCThreadState::self()->getCallingPid(); // TODO review
        serviceStream->setRegisteredThread(clientThreadId);
        int err = android::requestPriority(ownerPid, clientThreadId,
                                           DEFAULT_AUDIO_PRIORITY, true /* isForApp */);
        if (err != 0) {
            ALOGE("AAudioService::registerAudioThread(%d) failed, errno = %d, priority = %d",
                  clientThreadId, errno, DEFAULT_AUDIO_PRIORITY);
            result = AAUDIO_ERROR_INTERNAL;
        }
    }
    return checkForPendingClose(serviceStream, result);
}

aaudio_result_t AAudioService::unregisterAudioThread(aaudio_handle_t streamHandle,
                                                     pid_t clientThreadId) {
    aaudio_result_t result = AAUDIO_OK;
    sp<AAudioServiceStreamBase> serviceStream = convertHandleToServiceStream(streamHandle);
    if (serviceStream.get() == nullptr) {
        ALOGE("%s(), illegal stream handle = 0x%0x", __func__, streamHandle);
        return AAUDIO_ERROR_INVALID_HANDLE;
    }
    if (serviceStream->getRegisteredThread() != clientThreadId) {
        ALOGE("%s(), wrong thread", __func__);
        result = AAUDIO_ERROR_ILLEGAL_ARGUMENT;
    } else {
        serviceStream->setRegisteredThread(0);
    }
    return checkForPendingClose(serviceStream, result);
}

aaudio_result_t AAudioService::startClient(aaudio_handle_t streamHandle,
                                  const android::AudioClient& client,
                                  audio_port_handle_t *clientHandle) {
    sp<AAudioServiceStreamBase> serviceStream = convertHandleToServiceStream(streamHandle);
    if (serviceStream.get() == nullptr) {
        ALOGE("%s(), illegal stream handle = 0x%0x", __func__, streamHandle);
        return AAUDIO_ERROR_INVALID_HANDLE;
    }
    aaudio_result_t result = serviceStream->startClient(client, clientHandle);
    return checkForPendingClose(serviceStream, result);
}

aaudio_result_t AAudioService::stopClient(aaudio_handle_t streamHandle,
                                          audio_port_handle_t portHandle) {
    sp<AAudioServiceStreamBase> serviceStream = convertHandleToServiceStream(streamHandle);
    if (serviceStream.get() == nullptr) {
        ALOGE("%s(), illegal stream handle = 0x%0x", __func__, streamHandle);
        return AAUDIO_ERROR_INVALID_HANDLE;
    }
    aaudio_result_t result = serviceStream->stopClient(portHandle);
    return checkForPendingClose(serviceStream, result);
}

// This is only called internally when AudioFlinger wants to tear down a stream.
// So we do not have to check permissions.
aaudio_result_t AAudioService::disconnectStreamByPortHandle(audio_port_handle_t portHandle) {
    ALOGD("%s(%d) called", __func__, portHandle);
    sp<AAudioServiceStreamBase> serviceStream =
            mStreamTracker.findStreamByPortHandleAndIncrement(portHandle);
    if (serviceStream.get() == nullptr) {
        ALOGE("%s(), could not find stream with portHandle = %d", __func__, portHandle);
        return AAUDIO_ERROR_INVALID_HANDLE;
    }
    aaudio_result_t result = serviceStream->stop();
    serviceStream->disconnect();
    return checkForPendingClose(serviceStream, result);
}
