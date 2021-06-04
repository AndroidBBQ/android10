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

#include "rs.h"
#include "rsDevice.h"
#include "rsContext.h"
#include "rsThreadIO.h"

#include "rsgApiStructs.h"

#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
#include "rsMesh.h"
#endif

#include <sys/types.h>
#include <sys/resource.h>
#include <sched.h>

#include <sys/syscall.h>
#include <string.h>
#include <dlfcn.h>
#include <inttypes.h>
#include <unistd.h>

#ifdef RS_COMPATIBILITY_LIB
#include "rsCompatibilityLib.h"
#endif

namespace android {
namespace renderscript {

typedef bool (*HalQueryVersion)(uint32_t *version_major, uint32_t *version_minor);
typedef bool (*HalQueryHal)(android::renderscript::RsHalInitEnums entry, void **fnPtr);
typedef bool (*HalInit)(RsContext, uint32_t version_major, uint32_t version_minor);
typedef void (*HalAbort)(RsContext);


static bool LoadHalTable(Context *rsc, HalQueryHal fn, bool loadGraphics) {
    bool ret = true;

    ret &= fn(RS_HAL_CORE_SHUTDOWN, (void **)&rsc->mHal.funcs.shutdownDriver);
    ret &= fn(RS_HAL_CORE_SET_PRIORITY, (void **)&rsc->mHal.funcs.setPriority);
    ret &= fn(RS_HAL_CORE_ALLOC_RUNTIME_MEM, (void **)&rsc->mHal.funcs.allocRuntimeMem);
    ret &= fn(RS_HAL_CORE_FREE_RUNTIME_MEM, (void **)&rsc->mHal.funcs.freeRuntimeMem);
    ret &= fn(RS_HAL_CORE_FINISH, (void **)&rsc->mHal.funcs.finish);

    ret &= fn(RS_HAL_SCRIPT_INIT, (void **)&rsc->mHal.funcs.script.init);
    ret &= fn(RS_HAL_SCRIPT_INIT_INTRINSIC, (void **)&rsc->mHal.funcs.script.initIntrinsic);
    ret &= fn(RS_HAL_SCRIPT_INVOKE_FUNCTION, (void **)&rsc->mHal.funcs.script.invokeFunction);
    ret &= fn(RS_HAL_SCRIPT_INVOKE_ROOT, (void **)&rsc->mHal.funcs.script.invokeRoot);
    ret &= fn(RS_HAL_SCRIPT_INVOKE_FOR_EACH, (void **)&rsc->mHal.funcs.script.invokeForEach);
    ret &= fn(RS_HAL_SCRIPT_INVOKE_REDUCE, (void **)&rsc->mHal.funcs.script.invokeReduce);
    ret &= fn(RS_HAL_SCRIPT_INVOKE_INIT, (void **)&rsc->mHal.funcs.script.invokeInit);
    ret &= fn(RS_HAL_SCRIPT_INVOKE_FREE_CHILDREN, (void **)&rsc->mHal.funcs.script.invokeFreeChildren);
    ret &= fn(RS_HAL_SCRIPT_SET_GLOBAL_VAR, (void **)&rsc->mHal.funcs.script.setGlobalVar);
    ret &= fn(RS_HAL_SCRIPT_GET_GLOBAL_VAR, (void **)&rsc->mHal.funcs.script.getGlobalVar);
    ret &= fn(RS_HAL_SCRIPT_SET_GLOBAL_VAR_WITH_ELEMENT_DIM, (void **)&rsc->mHal.funcs.script.setGlobalVarWithElemDims);
    ret &= fn(RS_HAL_SCRIPT_SET_GLOBAL_BIND, (void **)&rsc->mHal.funcs.script.setGlobalBind);
    ret &= fn(RS_HAL_SCRIPT_SET_GLOBAL_OBJECT, (void **)&rsc->mHal.funcs.script.setGlobalObj);
    ret &= fn(RS_HAL_SCRIPT_DESTROY, (void **)&rsc->mHal.funcs.script.destroy);
    ret &= fn(RS_HAL_SCRIPT_INVOKE_FOR_EACH_MULTI, (void **)&rsc->mHal.funcs.script.invokeForEachMulti);
    ret &= fn(RS_HAL_SCRIPT_UPDATE_CACHED_OBJECT, (void **)&rsc->mHal.funcs.script.updateCachedObject);

    ret &= fn(RS_HAL_ALLOCATION_INIT, (void **)&rsc->mHal.funcs.allocation.init);
    ret &= fn(RS_HAL_ALLOCATION_INIT_OEM, (void **)&rsc->mHal.funcs.allocation.initOem);
    ret &= fn(RS_HAL_ALLOCATION_INIT_ADAPTER, (void **)&rsc->mHal.funcs.allocation.initAdapter);
    ret &= fn(RS_HAL_ALLOCATION_DESTROY, (void **)&rsc->mHal.funcs.allocation.destroy);
    ret &= fn(RS_HAL_ALLOCATION_GET_GRALLOC_BITS, (void **)&rsc->mHal.funcs.allocation.grallocBits);
    ret &= fn(RS_HAL_ALLOCATION_RESIZE, (void **)&rsc->mHal.funcs.allocation.resize);
    ret &= fn(RS_HAL_ALLOCATION_SYNC_ALL, (void **)&rsc->mHal.funcs.allocation.syncAll);
    ret &= fn(RS_HAL_ALLOCATION_MARK_DIRTY, (void **)&rsc->mHal.funcs.allocation.markDirty);
    ret &= fn(RS_HAL_ALLOCATION_SET_SURFACE, (void **)&rsc->mHal.funcs.allocation.setSurface);
    ret &= fn(RS_HAL_ALLOCATION_IO_SEND, (void **)&rsc->mHal.funcs.allocation.ioSend);
    ret &= fn(RS_HAL_ALLOCATION_IO_RECEIVE, (void **)&rsc->mHal.funcs.allocation.ioReceive);
    ret &= fn(RS_HAL_ALLOCATION_DATA_1D, (void **)&rsc->mHal.funcs.allocation.data1D);
    ret &= fn(RS_HAL_ALLOCATION_DATA_2D, (void **)&rsc->mHal.funcs.allocation.data2D);
    ret &= fn(RS_HAL_ALLOCATION_DATA_3D, (void **)&rsc->mHal.funcs.allocation.data3D);
    ret &= fn(RS_HAL_ALLOCATION_READ_1D, (void **)&rsc->mHal.funcs.allocation.read1D);
    ret &= fn(RS_HAL_ALLOCATION_READ_2D, (void **)&rsc->mHal.funcs.allocation.read2D);
    ret &= fn(RS_HAL_ALLOCATION_READ_3D, (void **)&rsc->mHal.funcs.allocation.read3D);
    ret &= fn(RS_HAL_ALLOCATION_LOCK_1D, (void **)&rsc->mHal.funcs.allocation.lock1D);
    ret &= fn(RS_HAL_ALLOCATION_UNLOCK_1D, (void **)&rsc->mHal.funcs.allocation.unlock1D);
    ret &= fn(RS_HAL_ALLOCATION_COPY_1D, (void **)&rsc->mHal.funcs.allocation.allocData1D);
    ret &= fn(RS_HAL_ALLOCATION_COPY_2D, (void **)&rsc->mHal.funcs.allocation.allocData2D);
    ret &= fn(RS_HAL_ALLOCATION_COPY_3D, (void **)&rsc->mHal.funcs.allocation.allocData3D);
    ret &= fn(RS_HAL_ALLOCATION_ELEMENT_DATA, (void **)&rsc->mHal.funcs.allocation.elementData);
    ret &= fn(RS_HAL_ALLOCATION_ELEMENT_READ, (void **)&rsc->mHal.funcs.allocation.elementRead);
    ret &= fn(RS_HAL_ALLOCATION_GENERATE_MIPMAPS, (void **)&rsc->mHal.funcs.allocation.generateMipmaps);
    ret &= fn(RS_HAL_ALLOCATION_UPDATE_CACHED_OBJECT, (void **)&rsc->mHal.funcs.allocation.updateCachedObject);
    ret &= fn(RS_HAL_ALLOCATION_ADAPTER_OFFSET, (void **)&rsc->mHal.funcs.allocation.adapterOffset);
    ret &= fn(RS_HAL_ALLOCATION_GET_POINTER, (void **)&rsc->mHal.funcs.allocation.getPointer);
#ifdef RS_COMPATIBILITY_LIB
    ret &= fn(RS_HAL_ALLOCATION_INIT_STRIDED, (void **)&rsc->mHal.funcs.allocation.initStrided);
#endif

    ret &= fn(RS_HAL_SAMPLER_INIT, (void **)&rsc->mHal.funcs.sampler.init);
    ret &= fn(RS_HAL_SAMPLER_DESTROY, (void **)&rsc->mHal.funcs.sampler.destroy);
    ret &= fn(RS_HAL_SAMPLER_UPDATE_CACHED_OBJECT, (void **)&rsc->mHal.funcs.sampler.updateCachedObject);

    ret &= fn(RS_HAL_TYPE_INIT, (void **)&rsc->mHal.funcs.type.init);
    ret &= fn(RS_HAL_TYPE_DESTROY, (void **)&rsc->mHal.funcs.type.destroy);
    ret &= fn(RS_HAL_TYPE_UPDATE_CACHED_OBJECT, (void **)&rsc->mHal.funcs.type.updateCachedObject);

    ret &= fn(RS_HAL_ELEMENT_INIT, (void **)&rsc->mHal.funcs.element.init);
    ret &= fn(RS_HAL_ELEMENT_DESTROY, (void **)&rsc->mHal.funcs.element.destroy);
    ret &= fn(RS_HAL_ELEMENT_UPDATE_CACHED_OBJECT, (void **)&rsc->mHal.funcs.element.updateCachedObject);

    ret &= fn(RS_HAL_SCRIPT_GROUP_INIT, (void **)&rsc->mHal.funcs.scriptgroup.init);
    ret &= fn(RS_HAL_SCRIPT_GROUP_DESTROY, (void **)&rsc->mHal.funcs.scriptgroup.destroy);
    ret &= fn(RS_HAL_SCRIPT_GROUP_UPDATE_CACHED_OBJECT, (void **)&rsc->mHal.funcs.scriptgroup.updateCachedObject);
    ret &= fn(RS_HAL_SCRIPT_GROUP_SET_INPUT, (void **)&rsc->mHal.funcs.scriptgroup.setInput);
    ret &= fn(RS_HAL_SCRIPT_GROUP_SET_OUTPUT, (void **)&rsc->mHal.funcs.scriptgroup.setOutput);
    ret &= fn(RS_HAL_SCRIPT_GROUP_EXECUTE, (void **)&rsc->mHal.funcs.scriptgroup.execute);


    if (loadGraphics) {
        ret &= fn(RS_HAL_GRAPHICS_INIT, (void **)&rsc->mHal.funcs.initGraphics);
        ret &= fn(RS_HAL_GRAPHICS_SHUTDOWN, (void **)&rsc->mHal.funcs.shutdownGraphics);
        ret &= fn(RS_HAL_GRAPHICS_SWAP, (void **)&rsc->mHal.funcs.swap);
        ret &= fn(RS_HAL_GRAPHICS_SET_SURFACE, (void **)&rsc->mHal.funcs.setSurface);
        ret &= fn(RS_HAL_GRAPHICS_RASTER_INIT, (void **)&rsc->mHal.funcs.raster.init);
        ret &= fn(RS_HAL_GRAPHICS_RASTER_SET_ACTIVE, (void **)&rsc->mHal.funcs.raster.setActive);
        ret &= fn(RS_HAL_GRAPHICS_RASTER_DESTROY, (void **)&rsc->mHal.funcs.raster.destroy);
        ret &= fn(RS_HAL_GRAPHICS_VERTEX_INIT, (void **)&rsc->mHal.funcs.vertex.init);
        ret &= fn(RS_HAL_GRAPHICS_VERTEX_SET_ACTIVE, (void **)&rsc->mHal.funcs.vertex.setActive);
        ret &= fn(RS_HAL_GRAPHICS_VERTEX_DESTROY, (void **)&rsc->mHal.funcs.vertex.destroy);
        ret &= fn(RS_HAL_GRAPHICS_FRAGMENT_INIT, (void **)&rsc->mHal.funcs.fragment.init);
        ret &= fn(RS_HAL_GRAPHICS_FRAGMENT_SET_ACTIVE, (void **)&rsc->mHal.funcs.fragment.setActive);
        ret &= fn(RS_HAL_GRAPHICS_FRAGMENT_DESTROY, (void **)&rsc->mHal.funcs.fragment.destroy);
        ret &= fn(RS_HAL_GRAPHICS_MESH_INIT, (void **)&rsc->mHal.funcs.mesh.init);
        ret &= fn(RS_HAL_GRAPHICS_MESH_DRAW, (void **)&rsc->mHal.funcs.mesh.draw);
        ret &= fn(RS_HAL_GRAPHICS_MESH_DESTROY, (void **)&rsc->mHal.funcs.mesh.destroy);
        ret &= fn(RS_HAL_GRAPHICS_FB_INIT, (void **)&rsc->mHal.funcs.framebuffer.init);
        ret &= fn(RS_HAL_GRAPHICS_FB_SET_ACTIVE, (void **)&rsc->mHal.funcs.framebuffer.setActive);
        ret &= fn(RS_HAL_GRAPHICS_FB_DESTROY, (void **)&rsc->mHal.funcs.framebuffer.destroy);
        ret &= fn(RS_HAL_GRAPHICS_STORE_INIT, (void **)&rsc->mHal.funcs.store.init);
        ret &= fn(RS_HAL_GRAPHICS_STORE_SET_ACTIVE, (void **)&rsc->mHal.funcs.store.setActive);
        ret &= fn(RS_HAL_GRAPHICS_STORE_DESTROY, (void **)&rsc->mHal.funcs.store.destroy);
    }

    return ret;
}

bool Context::loadRuntime(const char* filename) {
    HalQueryVersion fnQueryVersion = nullptr;
    HalQueryHal fnQueryHal = nullptr;
    HalInit fnInit = nullptr;
    HalAbort fnAbort = nullptr;


    // TODO: store the driverSO somewhere so we can dlclose later
    void *driverSO = nullptr;

    driverSO = dlopen(filename, RTLD_LAZY);
    if (driverSO == nullptr) {
        ALOGE("Failed loading RS driver: %s", dlerror());
        return false;
    }

    // Need to call dlerror() to clear buffer before using it for dlsym().
    (void) dlerror();

    fnQueryVersion = (HalQueryVersion) dlsym(driverSO, "rsdHalQueryVersion");
    fnQueryHal = (HalQueryHal) dlsym(driverSO, "rsdHalQueryHal");
    fnInit = (HalInit) dlsym(driverSO, "rsdHalInit");
    fnAbort = (HalAbort) dlsym(driverSO, "rsdHalAbort");
    uint32_t version_major = 0;
    uint32_t version_minor = 0;

    if ((fnQueryVersion == nullptr) || (fnQueryHal == nullptr) ||
        (fnInit == nullptr) || (fnAbort == nullptr)) {

        ALOGE("Failed to find hal setup entry points: %s", dlerror());
        goto error;
    }

    if (!fnQueryVersion(&version_major, &version_minor)) {
        ALOGE("Error checking RS driver version, %s", filename);
        goto error;
    }

    if (version_major != RS_HAL_VERSION) {
        ALOGE("Mismatched RS HAL versions: %s is version %u but version %u is expected",
              filename, version_major, RS_HAL_VERSION);
        goto error;
    }

    if (!LoadHalTable(this, fnQueryHal, mIsGraphicsContext)) {
        ALOGE("Error loading RS HAL table, %s", filename);
        goto error;
    }

    if (!(*fnInit)(this, 0, 0)) {
        ALOGE("Hal init failed,  %s", filename);
        goto error;
    }

    // Only map in the actual driver name if we successfully load the runtime.
    setDriverName(filename);

    return true;


error:
    if (fnAbort != nullptr) {
        fnAbort(this);
    }
    dlclose(driverSO);
    return false;
}



bool Context::loadDriver(bool forceDefault, bool forceRSoV) {
    bool loadDefault = true;

    // Provide a mechanism for dropping in a different RS driver.
#ifndef RS_COMPATIBILITY_LIB

    if (forceRSoV) {
        // If the property is set to use the RSoV driver, load it and fall back
        // to the vendor driver or the CPU reference driver if it does not load.
        if (loadRuntime("libRSDriver_RSoV.so")) {
            ALOGV("Successfully loaded the RSoV driver!");
            return true;
        }
        ALOGE("Failed to load the RSoV driver!");
    }

    if (!forceDefault && mVendorDriverName != nullptr) {
        if (loadRuntime(mVendorDriverName)) {
            ALOGV("Successfully loaded runtime: %s", mVendorDriverName);
            loadDefault = false;
        } else {
            ALOGE("Failed to load runtime %s, loading default", mVendorDriverName);
        }
    }

    if (loadDefault) {
        if (!loadRuntime("libRSDriver.so")) {
            ALOGE("Failed to load default runtime!");
            return false;
        }
    }
#else // RS_COMPATIBILITY_LIB
    if (!LoadHalTable(this, rsdHalQueryHal, false)) {
        ALOGE("Error loading RS HAL table");
        return false;
    }
    if (rsdHalInit(this, 0, 0) != true) {
        return false;
    }
#endif

    return true;
}

} // namespace renderscript
} // namespace android
