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

#include "rsDevice.h"
#include "rsContext.h"
#include "rsThreadIO.h"
#include "rsgApiStructs.h"
#include "rsgApiFuncDecl.h"
#include "rsFifo.h"

namespace android {
namespace renderscript {

extern "C" RsDevice rsDeviceCreate() {
    Device * d = new Device();
    return d;
}

extern "C" void rsDeviceDestroy(RsDevice dev) {
    // A Device should be destroyed in the destructor of the associated Context.
    // Keep this empty function here even after calls to nDeviceDestroy() in
    // RenderScript.helpDestroy() have been removed. This is necessary to keep
    // existing apps running.
}

extern "C" void rsDeviceSetConfig(RsDevice dev, RsDeviceParam p, int32_t value) {
    Device * d = static_cast<Device *>(dev);
    if (p == RS_DEVICE_PARAM_FORCE_SOFTWARE_GL) {
        d->mForceSW = value != 0;
        return;
    }
    rsAssert(0);
}

} // namespace renderscript
} // namespace android
