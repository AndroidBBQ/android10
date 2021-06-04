/*
 * Copyright (C) 2006 The Android Open Source Project
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

#include <binder/IActivityManager.h>
#include <binder/IBinder.h>
#include <binder/IServiceManager.h>

#include "ActivityManager.h"

namespace android {

// Perform ContentProvider.openFile() on the given URI, returning
// the resulting native file descriptor.  Returns < 0 on error.
int openContentProviderFile(const String16& uri)
{
    int fd = -1;

    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder = sm->getService(String16("activity"));
    sp<IActivityManager> am = interface_cast<IActivityManager>(binder);
    if (am != NULL) {
        fd = am->openContentUri(uri);
    }

    return fd;
}

} /* namespace android */
