/*
 * Copyright 2015 The Android Open Source Project
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

#include <binder/ProcessInfoService.h>
#include <binder/IServiceManager.h>

#include <utils/Log.h>
#include <utils/String16.h>

namespace android {

ProcessInfoService::ProcessInfoService() {
    updateBinderLocked();
}

status_t ProcessInfoService::getProcessStatesImpl(size_t length, /*in*/ int32_t* pids,
        /*out*/ int32_t* states) {
    status_t err = NO_ERROR;
    sp<IProcessInfoService> pis;
    mProcessInfoLock.lock();
    pis = mProcessInfoService;
    mProcessInfoLock.unlock();

    for (int i = 0; i < BINDER_ATTEMPT_LIMIT; i++) {

        if (pis != nullptr) {
            err = pis->getProcessStatesFromPids(length, /*in*/ pids, /*out*/ states);
            if (err == NO_ERROR) return NO_ERROR; // success
            if (IInterface::asBinder(pis)->isBinderAlive()) return err;
        }
        sleep(1);

        mProcessInfoLock.lock();
        if (pis == mProcessInfoService) {
            updateBinderLocked();
        }
        pis = mProcessInfoService;
        mProcessInfoLock.unlock();
    }

    ALOGW("%s: Could not retrieve process states from ProcessInfoService after %d retries.",
            __FUNCTION__, BINDER_ATTEMPT_LIMIT);

    return TIMED_OUT;
}

status_t ProcessInfoService::getProcessStatesScoresImpl(size_t length,
        /*in*/ int32_t* pids, /*out*/ int32_t* states,
        /*out*/ int32_t *scores) {
    status_t err = NO_ERROR;
    sp<IProcessInfoService> pis;
    mProcessInfoLock.lock();
    pis = mProcessInfoService;
    mProcessInfoLock.unlock();

    for (int i = 0; i < BINDER_ATTEMPT_LIMIT; i++) {

        if (pis != nullptr) {
            err = pis->getProcessStatesAndOomScoresFromPids(length,
                    /*in*/ pids, /*out*/ states, /*out*/ scores);
            if (err == NO_ERROR) return NO_ERROR; // success
            if (IInterface::asBinder(pis)->isBinderAlive()) return err;
        }
        sleep(1);

        mProcessInfoLock.lock();
        if (pis == mProcessInfoService) {
            updateBinderLocked();
        }
        pis = mProcessInfoService;
        mProcessInfoLock.unlock();
    }

    ALOGW("%s: Could not retrieve process states and scores "
            "from ProcessInfoService after %d retries.", __FUNCTION__,
            BINDER_ATTEMPT_LIMIT);

    return TIMED_OUT;
}

void ProcessInfoService::updateBinderLocked() {
    const sp<IServiceManager> sm(defaultServiceManager());
    if (sm != nullptr) {
        const String16 name("processinfo");
        mProcessInfoService = interface_cast<IProcessInfoService>(sm->checkService(name));
    }
}

ANDROID_SINGLETON_STATIC_INSTANCE(ProcessInfoService);

}; // namespace android
