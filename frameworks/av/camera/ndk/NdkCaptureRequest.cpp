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

//#define LOG_NDEBUG 0
#define LOG_TAG "NdkCaptureRequest"
#define ATRACE_TAG ATRACE_TAG_CAMERA

#include <utils/Log.h>
#include <utils/Trace.h>

#include <camera/NdkCaptureRequest.h>
#include "impl/ACameraMetadata.h"
#include "impl/ACaptureRequest.h"

EXPORT
camera_status_t ACameraOutputTarget_create(
        ACameraWindowType* window, ACameraOutputTarget** out) {
    ATRACE_CALL();
    if (window == nullptr) {
        ALOGE("%s: Error: input window is null", __FUNCTION__);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }
    *out = new ACameraOutputTarget(window);
    return ACAMERA_OK;
}

EXPORT
void ACameraOutputTarget_free(ACameraOutputTarget* target) {
    ATRACE_CALL();
    if (target != nullptr) {
        delete target;
    }
    return;
}

EXPORT
camera_status_t ACaptureRequest_addTarget(
        ACaptureRequest* req, const ACameraOutputTarget* target) {
    ATRACE_CALL();
    if (req == nullptr || req->targets == nullptr || target == nullptr) {
        void* req_targets;
        if (req != nullptr)
            req_targets = req->targets;
        else
            req_targets = nullptr;
        ALOGE("%s: Error: invalid input: req %p, req-targets %p, target %p",
                __FUNCTION__, req, req_targets, target);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }
    auto pair = req->targets->mOutputs.insert(*target);
    if (!pair.second) {
        ALOGW("%s: target %p already exists!", __FUNCTION__, target);
    }
    return ACAMERA_OK;
}

EXPORT
camera_status_t ACaptureRequest_removeTarget(
        ACaptureRequest* req, const ACameraOutputTarget* target) {
    ATRACE_CALL();
    if (req == nullptr || req->targets == nullptr || target == nullptr) {
        void* req_targets;
        if (req != nullptr)
            req_targets = req->targets;
        else
            req_targets = nullptr;
        ALOGE("%s: Error: invalid input: req %p, req-targets %p, target %p",
                __FUNCTION__, req, req_targets, target);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }
    req->targets->mOutputs.erase(*target);
    return ACAMERA_OK;
}

EXPORT
camera_status_t ACaptureRequest_getConstEntry(
        const ACaptureRequest* req, uint32_t tag, ACameraMetadata_const_entry* entry) {
    ATRACE_CALL();
    if (req == nullptr || entry == nullptr) {
        ALOGE("%s: invalid argument! req 0x%p, tag 0x%x, entry 0x%p",
               __FUNCTION__, req, tag, entry);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }
    return req->settings->getConstEntry(tag, entry);
}

EXPORT
camera_status_t ACaptureRequest_getConstEntry_physicalCamera(
        const ACaptureRequest* req, const char* physicalId,
        uint32_t tag, ACameraMetadata_const_entry* entry) {
    ATRACE_CALL();
    if (req == nullptr || entry == nullptr || physicalId == nullptr) {
        ALOGE("%s: invalid argument! req %p, tag 0x%x, entry %p, physicalId %p",
               __FUNCTION__, req, tag, entry, physicalId);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }

    const auto& physicalSettings = req->physicalSettings.find(physicalId);
    if (physicalSettings == req->physicalSettings.end()) {
        ALOGE("%s: Failed to find metadata for physical camera id  %s",
                __FUNCTION__, physicalId);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }

    return physicalSettings->second->getConstEntry(tag, entry);
}

EXPORT
camera_status_t ACaptureRequest_getAllTags(
        const ACaptureRequest* req, /*out*/int32_t* numTags, /*out*/const uint32_t** tags) {
    ATRACE_CALL();
    if (req == nullptr || numTags == nullptr || tags == nullptr) {
        ALOGE("%s: invalid argument! request %p, numTags %p, tags %p",
               __FUNCTION__, req, numTags, tags);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }
    return req->settings->getTags(numTags, tags);
}

#define SET_ENTRY(NAME,NDK_TYPE)                                                        \
EXPORT                                                                                  \
camera_status_t ACaptureRequest_setEntry_##NAME(                                        \
        ACaptureRequest* req, uint32_t tag, uint32_t count, const NDK_TYPE* data) {     \
    ATRACE_CALL();                                                                      \
    if (req == nullptr || (count > 0 && data == nullptr)) {                             \
        ALOGE("%s: invalid argument! req %p, tag 0x%x, count %d, data 0x%p",            \
               __FUNCTION__, req, tag, count, data);                                    \
        return ACAMERA_ERROR_INVALID_PARAMETER;                                         \
    }                                                                                   \
    return req->settings->update(tag, count, data);                                     \
}

SET_ENTRY(u8,uint8_t)
SET_ENTRY(i32,int32_t)
SET_ENTRY(float,float)
SET_ENTRY(double,double)
SET_ENTRY(i64,int64_t)
SET_ENTRY(rational,ACameraMetadata_rational)

#undef SET_ENTRY

#define SET_PHYSICAL_ENTRY(NAME,NDK_TYPE)                                               \
EXPORT                                                                                  \
camera_status_t ACaptureRequest_setEntry_physicalCamera_##NAME(                         \
        ACaptureRequest* req, const char* physicalId, uint32_t tag,                     \
        uint32_t count, const NDK_TYPE* data) {                                         \
    ATRACE_CALL();                                                                      \
    if (req == nullptr || (count > 0 && data == nullptr) || physicalId == nullptr) {    \
        ALOGE("%s: invalid argument! req %p, tag 0x%x, count %d, data 0x%p, physicalId %p", \
               __FUNCTION__, req, tag, count, data, physicalId);                        \
        return ACAMERA_ERROR_INVALID_PARAMETER;                                         \
    }                                                                                   \
    if (req->physicalSettings.find(physicalId) == req->physicalSettings.end()) {        \
        ALOGE("%s: Failed to find metadata for physical camera id %s",                  \
            __FUNCTION__, physicalId);                                                  \
      return ACAMERA_ERROR_INVALID_PARAMETER;                                           \
    }                                                                                   \
    return req->physicalSettings[physicalId]->update(tag, count, data);                 \
}

SET_PHYSICAL_ENTRY(u8,uint8_t)
SET_PHYSICAL_ENTRY(i32,int32_t)
SET_PHYSICAL_ENTRY(float,float)
SET_PHYSICAL_ENTRY(double,double)
SET_PHYSICAL_ENTRY(i64,int64_t)
SET_PHYSICAL_ENTRY(rational,ACameraMetadata_rational)

#undef SET_PHYSICAL_ENTRY

EXPORT
void ACaptureRequest_free(ACaptureRequest* request) {
    ATRACE_CALL();
    if (request == nullptr) {
        return;
    }
    request->settings.clear();
    request->physicalSettings.clear();
    delete request->targets;
    delete request;
    return;
}

EXPORT
camera_status_t ACaptureRequest_setUserContext(
        ACaptureRequest* request, void* context) {
    if (request == nullptr) {
        ALOGE("%s: invalid argument! request is NULL", __FUNCTION__);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }
    return request->setContext(context);
}

EXPORT
camera_status_t ACaptureRequest_getUserContext(
        const ACaptureRequest* request, /*out*/void** context) {
    if (request == nullptr || context == nullptr) {
        ALOGE("%s: invalid argument! request %p, context %p",
                __FUNCTION__, request, context);
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }
    return request->getContext(context);
}

EXPORT
ACaptureRequest* ACaptureRequest_copy(const ACaptureRequest* src) {
    ATRACE_CALL();
    if (src == nullptr) {
        ALOGE("%s: src is null!", __FUNCTION__);
        return nullptr;
    }

    ACaptureRequest* pRequest = new ACaptureRequest();
    pRequest->settings = new ACameraMetadata(*(src->settings));
    for (const auto& entry : src->physicalSettings) {
        pRequest->physicalSettings[entry.first] = new ACameraMetadata(*(entry.second));
    }
    pRequest->targets  = new ACameraOutputTargets();
    *(pRequest->targets)  = *(src->targets);
    pRequest->context = src->context;
    return pRequest;
}
