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

#include "rsContext.h"
#include "rsAllocation.h"

namespace android {
namespace renderscript {

extern "C" const void * rsaAllocationGetType(RsContext con, RsAllocation va) {
    Allocation *a = static_cast<Allocation *>(va);
    a->getType()->incUserRef();

    return a->getType();
}

} // namespace renderscript
} // namespace android
