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

#pragma once

#include <android/binder_status.h>

#include <binder/Status.h>
#include <utils/Errors.h>

struct AStatus {
    AStatus() {}  // ok
    explicit AStatus(::android::binder::Status&& status) : mStatus(std::move(status)) {}

    ::android::binder::Status* get() { return &mStatus; }
    const ::android::binder::Status* get() const { return &mStatus; }

   private:
    ::android::binder::Status mStatus;
};

// This collapses the statuses into the declared range.
binder_status_t PruneStatusT(android::status_t status);

// This collapses the exception into the declared range.
binder_exception_t PruneException(int32_t exception);
