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

//#define LOG_NDEBUG 0
#define LOG_TAG "DrmManagerService(Native)"
#include <utils/Log.h>

#include <private/android_filesystem_config.h>
#include <media/MemoryLeakTrackUtil.h>

#include <errno.h>
#include <utils/threads.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <sys/stat.h>
#include "DrmManagerService.h"
#include "DrmManager.h"

#include <selinux/android.h>

using namespace android;

static int selinux_enabled;
static char *drmserver_context;
static Vector<uid_t> trustedUids;

const char *const DrmManagerService::drm_perm_labels[] = {
    "consumeRights",
    "setPlaybackStatus",
    "openDecryptSession",
    "closeDecryptSession",
    "initializeDecryptUnit",
    "decrypt",
    "finalizeDecryptUnit",
    "pread"
};

const char *DrmManagerService::get_perm_label(drm_perm_t perm) {
    unsigned int index = perm;

    if (index >= (sizeof(drm_perm_labels) / sizeof(drm_perm_labels[0]))) {
        ALOGE("SELinux: Failed to retrieve permission label(perm=%d).\n", perm);
        abort();
    }
    return drm_perm_labels[index];
}

bool DrmManagerService::selinuxIsProtectedCallAllowed(pid_t spid, const char* ssid, drm_perm_t perm) {
    if (selinux_enabled <= 0) {
        return true;
    }

    char *sctx = NULL;
    const char *selinux_class = "drmservice";
    const char *str_perm = get_perm_label(perm);

    if (ssid == NULL) {
        android_errorWriteLog(0x534e4554, "121035042");

        if (getpidcon(spid, &sctx) != 0) {
            ALOGE("SELinux: getpidcon(pid=%d) failed.\n", spid);
            return false;
        }
    }

    bool allowed = (selinux_check_access(ssid ? ssid : sctx, drmserver_context,
            selinux_class, str_perm, NULL) == 0);
    freecon(sctx);

    return allowed;
}

bool DrmManagerService::isProtectedCallAllowed(drm_perm_t perm) {
    // TODO
    // Following implementation is just for reference.
    // Each OEM manufacturer should implement/replace with their own solutions.
    IPCThreadState* ipcState = IPCThreadState::self();
    uid_t uid = ipcState->getCallingUid();
    pid_t spid = ipcState->getCallingPid();
    const char* ssid = ipcState->getCallingSid();

    for (unsigned int i = 0; i < trustedUids.size(); ++i) {
        if (trustedUids[i] == uid) {
            return selinuxIsProtectedCallAllowed(spid, ssid, perm);
        }
    }
    return false;
}

void DrmManagerService::instantiate() {
    ALOGV("instantiate");
    sp<DrmManagerService> service = new DrmManagerService();
    service->setRequestingSid(true);
    defaultServiceManager()->addService(String16("drm.drmManager"), service);

    if (0 >= trustedUids.size()) {
        // TODO
        // Following implementation is just for reference.
        // Each OEM manufacturer should implement/replace with their own solutions.

        // Add trusted uids here
        trustedUids.push(AID_MEDIA);
    }

    selinux_enabled = is_selinux_enabled();
    if (selinux_enabled > 0 && getcon(&drmserver_context) != 0) {
        ALOGE("SELinux: DrmManagerService failed to get context for DrmManagerService. Aborting.\n");
        abort();
    }

    union selinux_callback cb;
    cb.func_log = selinux_log_callback;
    selinux_set_callback(SELINUX_CB_LOG, cb);
}

DrmManagerService::DrmManagerService() :
        mDrmManager(NULL) {
    ALOGV("created");
    mDrmManager = new DrmManager();
    mDrmManager->loadPlugIns();
}

DrmManagerService::~DrmManagerService() {
    ALOGV("Destroyed");
    mDrmManager->unloadPlugIns();
    delete mDrmManager; mDrmManager = NULL;
}

int DrmManagerService::addUniqueId(bool isNative) {
    return mDrmManager->addUniqueId(isNative);
}

void DrmManagerService::removeUniqueId(int uniqueId) {
    mDrmManager->removeUniqueId(uniqueId);
}

void DrmManagerService::addClient(int uniqueId) {
    mDrmManager->addClient(uniqueId);
}

void DrmManagerService::removeClient(int uniqueId) {
    mDrmManager->removeClient(uniqueId);
}

status_t DrmManagerService::setDrmServiceListener(
            int uniqueId, const sp<IDrmServiceListener>& drmServiceListener) {
    ALOGV("Entering setDrmServiceListener");
    mDrmManager->setDrmServiceListener(uniqueId, drmServiceListener);
    return DRM_NO_ERROR;
}

DrmConstraints* DrmManagerService::getConstraints(
            int uniqueId, const String8* path, const int action) {
    ALOGV("Entering getConstraints from content");
    return mDrmManager->getConstraints(uniqueId, path, action);
}

DrmMetadata* DrmManagerService::getMetadata(int uniqueId, const String8* path) {
    ALOGV("Entering getMetadata from content");
    return mDrmManager->getMetadata(uniqueId, path);
}

bool DrmManagerService::canHandle(int uniqueId, const String8& path, const String8& mimeType) {
    ALOGV("Entering canHandle");
    return mDrmManager->canHandle(uniqueId, path, mimeType);
}

DrmInfoStatus* DrmManagerService::processDrmInfo(int uniqueId, const DrmInfo* drmInfo) {
    ALOGV("Entering processDrmInfo");
    return mDrmManager->processDrmInfo(uniqueId, drmInfo);
}

DrmInfo* DrmManagerService::acquireDrmInfo(int uniqueId, const DrmInfoRequest* drmInfoRequest) {
    ALOGV("Entering acquireDrmInfo");
    return mDrmManager->acquireDrmInfo(uniqueId, drmInfoRequest);
}

status_t DrmManagerService::saveRights(
            int uniqueId, const DrmRights& drmRights,
            const String8& rightsPath, const String8& contentPath) {
    ALOGV("Entering saveRights");
    return mDrmManager->saveRights(uniqueId, drmRights, rightsPath, contentPath);
}

String8 DrmManagerService::getOriginalMimeType(int uniqueId, const String8& path, int fd) {
    ALOGV("Entering getOriginalMimeType");
    return mDrmManager->getOriginalMimeType(uniqueId, path, fd);
}

int DrmManagerService::getDrmObjectType(
           int uniqueId, const String8& path, const String8& mimeType) {
    ALOGV("Entering getDrmObjectType");
    return mDrmManager->getDrmObjectType(uniqueId, path, mimeType);
}

int DrmManagerService::checkRightsStatus(
            int uniqueId, const String8& path, int action) {
    ALOGV("Entering checkRightsStatus");
    return mDrmManager->checkRightsStatus(uniqueId, path, action);
}

status_t DrmManagerService::consumeRights(
            int uniqueId, sp<DecryptHandle>& decryptHandle, int action, bool reserve) {
    ALOGV("Entering consumeRights");
    if (!isProtectedCallAllowed(CONSUME_RIGHTS)) {
        return DRM_ERROR_NO_PERMISSION;
    }
    return mDrmManager->consumeRights(uniqueId, decryptHandle, action, reserve);
}

status_t DrmManagerService::setPlaybackStatus(
            int uniqueId, sp<DecryptHandle>& decryptHandle, int playbackStatus, int64_t position) {
    ALOGV("Entering setPlaybackStatus");
    if (!isProtectedCallAllowed(SET_PLAYBACK_STATUS)) {
        return DRM_ERROR_NO_PERMISSION;
    }
    return mDrmManager->setPlaybackStatus(uniqueId, decryptHandle, playbackStatus, position);
}

bool DrmManagerService::validateAction(
            int uniqueId, const String8& path,
            int action, const ActionDescription& description) {
    ALOGV("Entering validateAction");
    return mDrmManager->validateAction(uniqueId, path, action, description);
}

status_t DrmManagerService::removeRights(int uniqueId, const String8& path) {
    ALOGV("Entering removeRights");
    return mDrmManager->removeRights(uniqueId, path);
}

status_t DrmManagerService::removeAllRights(int uniqueId) {
    ALOGV("Entering removeAllRights");
    return mDrmManager->removeAllRights(uniqueId);
}

int DrmManagerService::openConvertSession(int uniqueId, const String8& mimeType) {
    ALOGV("Entering openConvertSession");
    return mDrmManager->openConvertSession(uniqueId, mimeType);
}

DrmConvertedStatus* DrmManagerService::convertData(
            int uniqueId, int convertId, const DrmBuffer* inputData) {
    ALOGV("Entering convertData");
    return mDrmManager->convertData(uniqueId, convertId, inputData);
}

DrmConvertedStatus* DrmManagerService::closeConvertSession(int uniqueId, int convertId) {
    ALOGV("Entering closeConvertSession");
    return mDrmManager->closeConvertSession(uniqueId, convertId);
}

status_t DrmManagerService::getAllSupportInfo(
            int uniqueId, int* length, DrmSupportInfo** drmSupportInfoArray) {
    ALOGV("Entering getAllSupportInfo");
    return mDrmManager->getAllSupportInfo(uniqueId, length, drmSupportInfoArray);
}

sp<DecryptHandle> DrmManagerService::openDecryptSession(
            int uniqueId, int fd, off64_t offset, off64_t length, const char* mime) {
    ALOGV("Entering DrmManagerService::openDecryptSession");
    if (isProtectedCallAllowed(OPEN_DECRYPT_SESSION)) {
        return mDrmManager->openDecryptSession(uniqueId, fd, offset, length, mime);
    }

    return NULL;
}

sp<DecryptHandle> DrmManagerService::openDecryptSession(
            int uniqueId, const char* uri, const char* mime) {
    ALOGV("Entering DrmManagerService::openDecryptSession with uri");
    if (isProtectedCallAllowed(OPEN_DECRYPT_SESSION)) {
        return mDrmManager->openDecryptSession(uniqueId, uri, mime);
    }

    return NULL;
}

sp<DecryptHandle> DrmManagerService::openDecryptSession(
            int uniqueId, const DrmBuffer& buf, const String8& mimeType) {
    ALOGV("Entering DrmManagerService::openDecryptSession for streaming");
    if (isProtectedCallAllowed(OPEN_DECRYPT_SESSION)) {
        return mDrmManager->openDecryptSession(uniqueId, buf, mimeType);
    }

    return NULL;
}

status_t DrmManagerService::closeDecryptSession(int uniqueId, sp<DecryptHandle>& decryptHandle) {
    ALOGV("Entering closeDecryptSession");
    if (!isProtectedCallAllowed(CLOSE_DECRYPT_SESSION)) {
        return DRM_ERROR_NO_PERMISSION;
    }
    return mDrmManager->closeDecryptSession(uniqueId, decryptHandle);
}

status_t DrmManagerService::initializeDecryptUnit(int uniqueId, sp<DecryptHandle>& decryptHandle,
            int decryptUnitId, const DrmBuffer* headerInfo) {
    ALOGV("Entering initializeDecryptUnit");
    if (!isProtectedCallAllowed(INITIALIZE_DECRYPT_UNIT)) {
        return DRM_ERROR_NO_PERMISSION;
    }
    return mDrmManager->initializeDecryptUnit(uniqueId,decryptHandle, decryptUnitId, headerInfo);
}

status_t DrmManagerService::decrypt(
            int uniqueId, sp<DecryptHandle>& decryptHandle, int decryptUnitId,
            const DrmBuffer* encBuffer, DrmBuffer** decBuffer, DrmBuffer* IV) {
    ALOGV("Entering decrypt");
    if (!isProtectedCallAllowed(DECRYPT)) {
        return DRM_ERROR_NO_PERMISSION;
    }
    return mDrmManager->decrypt(uniqueId, decryptHandle, decryptUnitId, encBuffer, decBuffer, IV);
}

status_t DrmManagerService::finalizeDecryptUnit(
            int uniqueId, sp<DecryptHandle>& decryptHandle, int decryptUnitId) {
    ALOGV("Entering finalizeDecryptUnit");
    if (!isProtectedCallAllowed(FINALIZE_DECRYPT_UNIT)) {
        return DRM_ERROR_NO_PERMISSION;
    }
    return mDrmManager->finalizeDecryptUnit(uniqueId, decryptHandle, decryptUnitId);
}

ssize_t DrmManagerService::pread(int uniqueId, sp<DecryptHandle>& decryptHandle,
            void* buffer, ssize_t numBytes, off64_t offset) {
    ALOGV("Entering pread");
    if (!isProtectedCallAllowed(PREAD)) {
        return DRM_ERROR_NO_PERMISSION;
    }
    return mDrmManager->pread(uniqueId, decryptHandle, buffer, numBytes, offset);
}

status_t DrmManagerService::dump(int fd, const Vector<String16>& args)
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    if (checkCallingPermission(String16("android.permission.DUMP")) == false) {
        snprintf(buffer, SIZE, "Permission Denial: "
                "can't dump DrmManagerService from pid=%d, uid=%d\n",
                IPCThreadState::self()->getCallingPid(),
                IPCThreadState::self()->getCallingUid());
        result.append(buffer);
    } else {
#if DRM_MEMORY_LEAK_TRACK
        bool dumpMem = false;
        for (size_t i = 0; i < args.size(); i++) {
            if (args[i] == String16("-m")) {
                dumpMem = true;
            }
        }
        if (dumpMem) {
            result.append("\nDumping memory:\n");
            std::string s = dumpMemoryAddresses(100 /* limit */);
            result.append(s.c_str(), s.size());
        }
#else
        (void)args;
#endif
    }
    write(fd, result.string(), result.size());
    return NO_ERROR;
}

