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

#ifndef __DRM_MANAGER_SERVICE_H__
#define __DRM_MANAGER_SERVICE_H__

#include <utils/RefBase.h>
#include <utils/KeyedVector.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include "IDrmManagerService.h"
#include "IDrmServiceListener.h"

namespace android {

class DrmManager;
class String8;
class Mutex;

/**
 * This is the implementation class for DRM manager service. This delegates
 * the responsibility to DrmManager.
 *
 * The instance of this class is created while starting the DRM manager service.
 *
 */
class DrmManagerService : public BnDrmManagerService {
public:
    static void instantiate();

private:
    enum drm_perm_t {
        CONSUME_RIGHTS          = 0,
        SET_PLAYBACK_STATUS     = 1,
        OPEN_DECRYPT_SESSION    = 2,
        CLOSE_DECRYPT_SESSION   = 3,
        INITIALIZE_DECRYPT_UNIT = 4,
        DECRYPT                 = 5,
        FINALIZE_DECRYPT_UNIT   = 6,
        PREAD                   = 7,
    };

    static const char *const drm_perm_labels[];

    DrmManagerService();
    virtual ~DrmManagerService();

    static const char *get_perm_label(drm_perm_t perm);

    static bool selinuxIsProtectedCallAllowed(pid_t spid, const char* ssid, drm_perm_t perm);

    static bool isProtectedCallAllowed(drm_perm_t perm);

public:
    int addUniqueId(bool isNative);

    void removeUniqueId(int uniqueId);

    void addClient(int uniqueId);

    void removeClient(int uniqueId);

    status_t setDrmServiceListener(
            int uniqueId, const sp<IDrmServiceListener>& drmServiceListener);

    DrmConstraints* getConstraints(int uniqueId, const String8* path, const int action);

    DrmMetadata* getMetadata(int uniqueId, const String8* path);

    bool canHandle(int uniqueId, const String8& path, const String8& mimeType);

    DrmInfoStatus* processDrmInfo(int uniqueId, const DrmInfo* drmInfo);

    DrmInfo* acquireDrmInfo(int uniqueId, const DrmInfoRequest* drmInforequest);

    status_t saveRights(int uniqueId, const DrmRights& drmRights,
            const String8& rightsPath, const String8& contentPath);

    String8 getOriginalMimeType(int uniqueId, const String8& path, int fd);

    int getDrmObjectType(int uniqueId, const String8& path, const String8& mimeType);

    int checkRightsStatus(int uniqueId, const String8& path,int action);

    status_t consumeRights(int uniqueId, sp<DecryptHandle>& decryptHandle, int action,
            bool reserve);

    status_t setPlaybackStatus(
            int uniqueId, sp<DecryptHandle>& decryptHandle, int playbackStatus, int64_t position);

    bool validateAction(int uniqueId, const String8& path,
            int action, const ActionDescription& description);

    status_t removeRights(int uniqueId, const String8& path);

    status_t removeAllRights(int uniqueId);

    int openConvertSession(int uniqueId, const String8& mimeType);

    DrmConvertedStatus* convertData(int uniqueId, int convertId, const DrmBuffer* inputData);

    DrmConvertedStatus* closeConvertSession(int uniqueId, int convertId);

    status_t getAllSupportInfo(int uniqueId, int* length, DrmSupportInfo** drmSupportInfoArray);

    sp<DecryptHandle> openDecryptSession(
        int uniqueId, int fd, off64_t offset, off64_t length, const char *mime);

    sp<DecryptHandle> openDecryptSession(
        int uniqueId, const char* uri, const char* mime);

    sp<DecryptHandle> openDecryptSession(int uniqueId, const DrmBuffer& buf,
            const String8& mimeType);

    status_t closeDecryptSession(int uniqueId, sp<DecryptHandle>& decryptHandle);

    status_t initializeDecryptUnit(int uniqueId, sp<DecryptHandle>& decryptHandle,
            int decryptUnitId, const DrmBuffer* headerInfo);

    status_t decrypt(int uniqueId, sp<DecryptHandle>& decryptHandle, int decryptUnitId,
            const DrmBuffer* encBuffer, DrmBuffer** decBuffer, DrmBuffer* IV);

    status_t finalizeDecryptUnit(int uniqueId, sp<DecryptHandle>& decryptHandle,
            int decryptUnitId);

    ssize_t pread(int uniqueId, sp<DecryptHandle>& decryptHandle,
            void* buffer, ssize_t numBytes, off64_t offset);

    virtual status_t dump(int fd, const Vector<String16>& args);

private:
    DrmManager* mDrmManager;
};

};

#endif /* __DRM_MANAGER_SERVICE_H__ */

