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

using android::renderscript::Context;
using android::renderscript::Device;
using android::renderscript::ObjectBase;

extern "C" RsContext rsContextCreateVendor(RsDevice vdev, uint32_t version, uint32_t sdkVersion,
                                           RsContextType ct, uint32_t flags,
                                           const char* vendorDriverName) {
    Device * dev = static_cast<Device *>(vdev);
    Context *rsc = Context::createContext(dev, nullptr, ct, flags, vendorDriverName);
    if (rsc) {
        rsc->setTargetSdkVersion(sdkVersion);
    }
    return rsc;
}

extern "C" RsContext rsContextCreate(RsDevice vdev, uint32_t version, uint32_t sdkVersion,
                                      RsContextType ct, uint32_t flags) {
    return rsContextCreateVendor(vdev, version, sdkVersion, ct, flags, nullptr);
}

extern "C" void rsaContextSetNativeLibDir(RsContext con, char *libDir, size_t length) {
#ifdef RS_COMPATIBILITY_LIB
    Context *rsc = static_cast<Context *>(con);
    rsc->setNativeLibDir(libDir, length);
#endif
}

// TODO: Figure out better naming schemes for all the rs* functions.
// Currently they share the same names as the NDK counterparts, and that is
// causing lots of confusion.
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
extern "C" RsContext rsContextCreateGL(RsDevice vdev, uint32_t version,
                                       uint32_t sdkVersion, RsSurfaceConfig sc,
                                       uint32_t dpi) {
    //ALOGV("rsContextCreateGL dev=%p", vdev);
    Device * dev = static_cast<Device *>(vdev);
    Context *rsc = Context::createContext(dev, &sc);
    if (rsc) {
        rsc->setTargetSdkVersion(sdkVersion);
        rsc->setDPI(dpi);
    }
    //ALOGV("%p rsContextCreateGL ret", rsc);
    return rsc;
}
#endif

// Only to be called at a3d load time, before object is visible to user
// not thread safe
extern "C" void rsaGetName(RsContext con, void * obj, const char **name) {
    ObjectBase *ob = static_cast<ObjectBase *>(obj);
    (*name) = ob->getName();
}
