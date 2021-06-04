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

#ifndef __NO_OP_DRM_MANAGER_CLIENT_IMPL_H__
#define __NO_OP_DRM_MANAGER_CLIENT_IMPL_H__

#include "DrmManagerClientImpl.h"

namespace android {

class NoOpDrmManagerClientImpl : public DrmManagerClientImpl {
public:
    NoOpDrmManagerClientImpl() { }

    void remove(int uniqueId);
    void addClient(int uniqueId);
    void removeClient(int uniqueId);
    status_t setOnInfoListener(
            int uniqueId, const sp<DrmManagerClient::OnInfoListener>& infoListener);
    DrmConstraints* getConstraints(int uniqueId, const String8* path, const int action);

    DrmMetadata* getMetadata(int uniqueId, const String8* path);
    bool canHandle(int uniqueId, const String8& path, const String8& mimeType);
    DrmInfoStatus* processDrmInfo(int uniqueId, const DrmInfo* drmInfo);
    DrmInfo* acquireDrmInfo(int uniqueId, const DrmInfoRequest* drmInfoRequest);
    status_t saveRights(int uniqueId, const DrmRights& drmRights,
            const String8& rightsPath, const String8& contentPath);
    String8 getOriginalMimeType(int uniqueId, const String8& path, int fd);
    int getDrmObjectType(int uniqueId, const String8& path, const String8& mimeType);
    int checkRightsStatus(int uniqueId, const String8& path, int action);
    status_t consumeRights(int uniqueId, sp<DecryptHandle> &decryptHandle, int action, bool reserve);
    status_t setPlaybackStatus(
            int uniqueId, sp<DecryptHandle> &decryptHandle, int playbackStatus, int64_t position);
    bool validateAction(
        int uniqueId, const String8& path, int action, const ActionDescription& description);
    status_t removeRights(int uniqueId, const String8& path);
    status_t removeAllRights(int uniqueId);
    int openConvertSession(int uniqueId, const String8& mimeType);
    DrmConvertedStatus* convertData(int uniqueId, int convertId, const DrmBuffer* inputData);
    DrmConvertedStatus* closeConvertSession(int uniqueId, int convertId);
    status_t getAllSupportInfo(int uniqueId, int* length, DrmSupportInfo** drmSupportInfoArray);
    sp<DecryptHandle> openDecryptSession(
            int uniqueId, int fd, off64_t offset, off64_t length, const char* mime);
    sp<DecryptHandle> openDecryptSession(
            int uniqueId, const char* uri, const char* mime);
    sp<DecryptHandle> openDecryptSession(int uniqueId, const DrmBuffer& buf,
            const String8& mimeType);
    status_t closeDecryptSession(int uniqueId, sp<DecryptHandle> &decryptHandle);
    status_t initializeDecryptUnit(int uniqueId, sp<DecryptHandle> &decryptHandle,
            int decryptUnitId, const DrmBuffer* headerInfo);
    status_t decrypt(int uniqueId, sp<DecryptHandle> &decryptHandle, int decryptUnitId,
            const DrmBuffer* encBuffer, DrmBuffer** decBuffer, DrmBuffer* IV);
    status_t finalizeDecryptUnit(int uniqueId, sp<DecryptHandle> &decryptHandle, int decryptUnitId);
    ssize_t pread(int uniqueId, sp<DecryptHandle> &decryptHandle,
            void* buffer, ssize_t numBytes, off64_t offset);
    status_t notify(const DrmInfoEvent& event);
};

}

#endif // __NO_OP_DRM_MANAGER_CLIENT_IMPL_H
