/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include "cpp/rsDispatch.h"
#include "rsFallbackAdaptation.h"

#include <log/log.h>
#include <dlfcn.h>

#undef LOG_TAG
#define LOG_TAG "RenderScript Graphics Fallback"

dispatchTable RsFallbackAdaptation::mEntryFuncs;

RsFallbackAdaptation::RsFallbackAdaptation()
{
    void* handle = dlopen("libRS_internal.so", RTLD_LAZY | RTLD_LOCAL);
    if (handle == NULL) {
        ALOGE("couldn't dlopen %s.", dlerror());
        return;
    }
    if (loadSymbols(handle, mEntryFuncs) == false) {
        // If dispatch table initialization failed, the dispatch table
        // will be reset, and calling function pointers of uninitialized
        // dispatch table will crash the application.
        ALOGE("Fallback dispatch table init failed!");
        mEntryFuncs = {};
        dlclose(handle);
    }
}

RsFallbackAdaptation& RsFallbackAdaptation::GetInstance()
{
    // This function-local-static guarantees the instance is a singleton. The
    // constructor of RsHidlAdaptation will only be called when GetInstance is
    // called for the first time.
    static RsFallbackAdaptation instance;
    return instance;
}

const dispatchTable* RsFallbackAdaptation::GetEntryFuncs()
{
    return &mEntryFuncs;
}

