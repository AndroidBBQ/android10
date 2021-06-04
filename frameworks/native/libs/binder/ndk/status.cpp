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

#include <android/binder_status.h>
#include "status_internal.h"

#include <android-base/logging.h>

using ::android::status_t;
using ::android::binder::Status;

AStatus* AStatus_newOk() {
    return new AStatus();
}

AStatus* AStatus_fromExceptionCode(binder_exception_t exception) {
    return new AStatus(Status::fromExceptionCode(PruneException(exception)));
}

AStatus* AStatus_fromExceptionCodeWithMessage(binder_exception_t exception, const char* message) {
    return new AStatus(Status::fromExceptionCode(PruneException(exception), message));
}

AStatus* AStatus_fromServiceSpecificError(int32_t serviceSpecific) {
    return new AStatus(Status::fromServiceSpecificError(serviceSpecific));
}

AStatus* AStatus_fromServiceSpecificErrorWithMessage(int32_t serviceSpecific, const char* message) {
    return new AStatus(Status::fromServiceSpecificError(serviceSpecific, message));
}

AStatus* AStatus_fromStatus(binder_status_t status) {
    return new AStatus(Status::fromStatusT(PruneStatusT(status)));
}

bool AStatus_isOk(const AStatus* status) {
    return status->get()->isOk();
}

binder_exception_t AStatus_getExceptionCode(const AStatus* status) {
    return PruneException(status->get()->exceptionCode());
}

int32_t AStatus_getServiceSpecificError(const AStatus* status) {
    return status->get()->serviceSpecificErrorCode();
}

binder_status_t AStatus_getStatus(const AStatus* status) {
    return PruneStatusT(status->get()->transactionError());
}

const char* AStatus_getMessage(const AStatus* status) {
    return status->get()->exceptionMessage().c_str();
}

void AStatus_delete(AStatus* status) {
    delete status;
}

binder_status_t PruneStatusT(status_t status) {
    switch (status) {
        case ::android::OK:
            return STATUS_OK;
        case ::android::NO_MEMORY:
            return STATUS_NO_MEMORY;
        case ::android::INVALID_OPERATION:
            return STATUS_INVALID_OPERATION;
        case ::android::BAD_VALUE:
            return STATUS_BAD_VALUE;
        case ::android::BAD_TYPE:
            return STATUS_BAD_TYPE;
        case ::android::NAME_NOT_FOUND:
            return STATUS_NAME_NOT_FOUND;
        case ::android::PERMISSION_DENIED:
            return STATUS_PERMISSION_DENIED;
        case ::android::NO_INIT:
            return STATUS_NO_INIT;
        case ::android::ALREADY_EXISTS:
            return STATUS_ALREADY_EXISTS;
        case ::android::DEAD_OBJECT:
            return STATUS_DEAD_OBJECT;
        case ::android::FAILED_TRANSACTION:
            return STATUS_FAILED_TRANSACTION;
        case ::android::BAD_INDEX:
            return STATUS_BAD_INDEX;
        case ::android::NOT_ENOUGH_DATA:
            return STATUS_NOT_ENOUGH_DATA;
        case ::android::WOULD_BLOCK:
            return STATUS_WOULD_BLOCK;
        case ::android::TIMED_OUT:
            return STATUS_TIMED_OUT;
        case ::android::UNKNOWN_TRANSACTION:
            return STATUS_UNKNOWN_TRANSACTION;
        case ::android::FDS_NOT_ALLOWED:
            return STATUS_FDS_NOT_ALLOWED;
        case ::android::UNEXPECTED_NULL:
            return STATUS_UNEXPECTED_NULL;
        case ::android::UNKNOWN_ERROR:
            return STATUS_UNKNOWN_ERROR;

        default:
            LOG(WARNING) << __func__
                         << ": Unknown status_t pruned into STATUS_UNKNOWN_ERROR: " << status;
            return STATUS_UNKNOWN_ERROR;
    }
}

binder_exception_t PruneException(int32_t exception) {
    switch (exception) {
        case Status::EX_NONE:
            return EX_NONE;
        case Status::EX_SECURITY:
            return EX_SECURITY;
        case Status::EX_BAD_PARCELABLE:
            return EX_BAD_PARCELABLE;
        case Status::EX_ILLEGAL_ARGUMENT:
            return EX_ILLEGAL_ARGUMENT;
        case Status::EX_NULL_POINTER:
            return EX_NULL_POINTER;
        case Status::EX_ILLEGAL_STATE:
            return EX_ILLEGAL_STATE;
        case Status::EX_NETWORK_MAIN_THREAD:
            return EX_NETWORK_MAIN_THREAD;
        case Status::EX_UNSUPPORTED_OPERATION:
            return EX_UNSUPPORTED_OPERATION;
        case Status::EX_SERVICE_SPECIFIC:
            return EX_SERVICE_SPECIFIC;
        case Status::EX_PARCELABLE:
            return EX_PARCELABLE;
        case Status::EX_TRANSACTION_FAILED:
            return EX_TRANSACTION_FAILED;

        default:
            LOG(WARNING) << __func__
                         << ": Unknown status_t pruned into EX_TRANSACTION_FAILED: " << exception;
            return EX_TRANSACTION_FAILED;
    }
}
