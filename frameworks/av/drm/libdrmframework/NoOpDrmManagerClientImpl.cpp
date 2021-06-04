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

#include "NoOpDrmManagerClientImpl.h"

namespace android {

void NoOpDrmManagerClientImpl::remove(int /* uniqueId */) {
}

void NoOpDrmManagerClientImpl::addClient(int /* uniqueId */) {
}

void NoOpDrmManagerClientImpl::removeClient(
            int /* uniqueId */) {
}

status_t NoOpDrmManagerClientImpl::setOnInfoListener(
            int /* uniqueId */,
            const sp<DrmManagerClient::OnInfoListener>& /* infoListener */) {
    return UNKNOWN_ERROR;
}

DrmConstraints* NoOpDrmManagerClientImpl::getConstraints(
            int /* uniqueId */,
            const String8* /* path */,
            const int /* action */) {
    return NULL;
}

DrmMetadata* NoOpDrmManagerClientImpl::getMetadata(
            int /* uniqueId */,
            const String8* /* path */) {
    return NULL;
}

bool NoOpDrmManagerClientImpl::canHandle(
            int /* uniqueId */,
            const String8& /* path */,
            const String8& /* mimeType */) {
    return false;
}

DrmInfoStatus* NoOpDrmManagerClientImpl::processDrmInfo(
            int /* uniqueId */,
            const DrmInfo* /* drmInfo */) {
    return NULL;
}

DrmInfo* NoOpDrmManagerClientImpl::acquireDrmInfo(
            int /* uniqueId */,
            const DrmInfoRequest* /* drmInfoRequest */) {
    return NULL;
}

status_t NoOpDrmManagerClientImpl::saveRights(
            int /* uniqueId */,
            const DrmRights& /* drmRights */,
            const String8& /* rightsPath */,
            const String8& /* contentPath */) {
    return UNKNOWN_ERROR;
}

String8 NoOpDrmManagerClientImpl::getOriginalMimeType(
            int /* uniqueId */,
            const String8& /* path */,
            int /* fd */) {
    return String8();
}

int NoOpDrmManagerClientImpl::getDrmObjectType(
            int /* uniqueId */,
            const String8& /* path */,
            const String8& /* mimeType */) {
    return -1;
}

int NoOpDrmManagerClientImpl::checkRightsStatus(
            int /* uniqueId */,
            const String8& /* path */,
            int /* action */) {
    return -1;
}

status_t NoOpDrmManagerClientImpl::consumeRights(
            int /* uniqueId */,
            sp<DecryptHandle> &/* decryptHandle */,
            int /* action */,
            bool /* reserve */) {
    return UNKNOWN_ERROR;
}

status_t NoOpDrmManagerClientImpl::setPlaybackStatus(
            int /* uniqueId */,
            sp<DecryptHandle> &/* decryptHandle */,
            int /* playbackStatus */,
            int64_t /* position */) {
    return UNKNOWN_ERROR;
}

bool NoOpDrmManagerClientImpl::validateAction(
            int /* uniqueId */,
            const String8& /* path */,
            int /* action */,
            const ActionDescription& /* description */) {
    return false;
}

status_t NoOpDrmManagerClientImpl::removeRights(
            int /* uniqueId */,
            const String8& /* path */) {
    return UNKNOWN_ERROR;
}

status_t NoOpDrmManagerClientImpl::removeAllRights(
            int /* uniqueId */) {
    return UNKNOWN_ERROR;
}

int NoOpDrmManagerClientImpl::openConvertSession(
            int /* uniqueId */,
            const String8& /* mimeType */) {
    return -1;
}

DrmConvertedStatus* NoOpDrmManagerClientImpl::convertData(
            int /* uniqueId */,
            int /* convertId */,
            const DrmBuffer* /* inputData */) {
    return NULL;
}

DrmConvertedStatus* NoOpDrmManagerClientImpl::closeConvertSession(
            int /* uniqueId */,
            int /* convertId */) {
    return NULL;
}

status_t NoOpDrmManagerClientImpl::getAllSupportInfo(
            int /* uniqueId */,
            int* /* length */,
            DrmSupportInfo** /* drmSupportInfoArray */) {
    return UNKNOWN_ERROR;
}

sp<DecryptHandle> NoOpDrmManagerClientImpl::openDecryptSession(
            int /* uniqueId */,
            int /* fd */,
            off64_t /* offset */,
            off64_t /* length */,
            const char* /* mime */) {
    return NULL;
}

sp<DecryptHandle> NoOpDrmManagerClientImpl::openDecryptSession(
            int /* uniqueId */,
            const char* /* uri */,
            const char* /* mime */) {
    return NULL;
}

sp<DecryptHandle> NoOpDrmManagerClientImpl::openDecryptSession(
            int /* uniqueId */,
            const DrmBuffer& /* buf */,
            const String8& /* mimeType */) {
    return NULL;
}

status_t NoOpDrmManagerClientImpl::closeDecryptSession(
            int /* uniqueId */,
            sp<DecryptHandle> &/* decryptHandle */) {
    return UNKNOWN_ERROR;
}

status_t NoOpDrmManagerClientImpl::initializeDecryptUnit(
            int /* uniqueId */,
            sp<DecryptHandle> &/* decryptHandle */,
            int /* decryptUnitId */,
            const DrmBuffer* /* headerInfo */) {
    return UNKNOWN_ERROR;
}

status_t NoOpDrmManagerClientImpl::decrypt(
            int /* uniqueId */,
            sp<DecryptHandle> &/* decryptHandle */,
            int /* decryptUnitId */,
            const DrmBuffer* /* encBuffer */,
            DrmBuffer** /* decBuffer */,
            DrmBuffer* /* IV */) {
    return UNKNOWN_ERROR;
}

status_t NoOpDrmManagerClientImpl::finalizeDecryptUnit(
            int /* uniqueId */,
            sp<DecryptHandle> &/* decryptHandle */,
            int /* decryptUnitId */) {
    return UNKNOWN_ERROR;
}

ssize_t NoOpDrmManagerClientImpl::pread(
            int /* uniqueId */,
            sp<DecryptHandle> &/* decryptHandle */,
            void* /* buffer */,
            ssize_t /* numBytes */,
            off64_t /* offset */) {
    return -1;
}

status_t NoOpDrmManagerClientImpl::notify(
            const DrmInfoEvent& /* event */) {
    return UNKNOWN_ERROR;
}

}
