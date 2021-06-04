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
#include "rsFileA3D.h"

using android::Asset;
using android::renderscript::Context;
using android::renderscript::FileA3D;
using android::renderscript::ObjectBase;
using android::renderscript::rsuCopyString;

RsObjectBase rsaFileA3DGetEntryByIndex(RsContext con, uint32_t index, RsFile file) {
    FileA3D *fa3d = static_cast<FileA3D *>(file);
    if (!fa3d) {
        ALOGE("Can't load entry. No valid file");
        return nullptr;
    }

    ObjectBase *obj = fa3d->initializeFromEntry(index);
    //ALOGV("Returning object with name %s", obj->getName());

    return obj;
}


void rsaFileA3DGetNumIndexEntries(RsContext con, int32_t *numEntries, RsFile file) {
    FileA3D *fa3d = static_cast<FileA3D *>(file);

    if (fa3d) {
        *numEntries = fa3d->getNumIndexEntries();
    } else {
        *numEntries = 0;
    }
}

void rsaFileA3DGetIndexEntries(RsContext con, RsFileIndexEntry *fileEntries, uint32_t numEntries, RsFile file) {
    FileA3D *fa3d = static_cast<FileA3D *>(file);

    if (!fa3d) {
        ALOGE("Can't load index entries. No valid file");
        return;
    }

    uint32_t numFileEntries = fa3d->getNumIndexEntries();
    if (numFileEntries != numEntries || numEntries == 0 || fileEntries == nullptr) {
        ALOGE("Can't load index entries. Invalid number requested");
        return;
    }

    for (uint32_t i = 0; i < numFileEntries; i ++) {
        const FileA3D::A3DIndexEntry *entry = fa3d->getIndexEntry(i);
        fileEntries[i].classID = entry->getType();
        fileEntries[i].objectName = rsuCopyString(entry->getObjectName());
    }
}

RsFile rsaFileA3DCreateFromMemory(RsContext con, const void *data, uint32_t len) {
    if (data == nullptr) {
        ALOGE("File load failed. Asset stream is nullptr");
        return nullptr;
    }

    Context *rsc = static_cast<Context *>(con);
    FileA3D *fa3d = new FileA3D(rsc);
    fa3d->incUserRef();

    fa3d->load(data, len);
    return fa3d;
}

RsFile rsaFileA3DCreateFromAsset(RsContext con, void *_asset) {
    ALOGE("Calling deprecated %s API", __FUNCTION__);
    return nullptr;
}

RsFile rsaFileA3DCreateFromFile(RsContext con, const char *path) {
    if (path == nullptr) {
        ALOGE("File load failed. Path is nullptr");
        return nullptr;
    }

    Context *rsc = static_cast<Context *>(con);
    FileA3D *fa3d = nullptr;

    FILE *f = fopen(path, "rbe");
    if (f) {
        fa3d = new FileA3D(rsc);
        fa3d->incUserRef();
        fa3d->load(f);
        fclose(f);
    } else {
        ALOGE("Could not open file %s", path);
    }

    return fa3d;
}
