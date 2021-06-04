/*
 * Copyright (C) 2010 The Android Open Source Project
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

#ifndef _I_MTP_DATABASE_H
#define _I_MTP_DATABASE_H

#include "MtpTypes.h"

namespace android {

class MtpDataPacket;
class MtpProperty;
class MtpObjectInfo;
class MtpStringBuffer;

class IMtpDatabase {
public:
    virtual ~IMtpDatabase() {}

    // Called from SendObjectInfo to reserve a database entry for the incoming file.
    virtual MtpObjectHandle         beginSendObject(const char* path,
                                            MtpObjectFormat format,
                                            MtpObjectHandle parent,
                                            MtpStorageID storage) = 0;

    // Called to report success or failure of the SendObject file transfer.
    virtual void                    endSendObject(MtpObjectHandle handle,
                                            bool succeeded) = 0;
    
    // Called to rescan a file, such as after an edit.
    virtual void                    rescanFile(const char* path,
                                            MtpObjectHandle handle,
                                            MtpObjectFormat format) = 0;

    virtual MtpObjectHandleList*    getObjectList(MtpStorageID storageID,
                                            MtpObjectFormat format,
                                            MtpObjectHandle parent) = 0;

    virtual int                     getNumObjects(MtpStorageID storageID,
                                            MtpObjectFormat format,
                                            MtpObjectHandle parent) = 0;

    // callee should delete[] the results from these
    // results can be NULL
    virtual MtpObjectFormatList*    getSupportedPlaybackFormats() = 0;
    virtual MtpObjectFormatList*    getSupportedCaptureFormats() = 0;
    virtual MtpObjectPropertyList*  getSupportedObjectProperties(MtpObjectFormat format) = 0;
    virtual MtpDevicePropertyList*  getSupportedDeviceProperties() = 0;

    virtual MtpResponseCode         getObjectPropertyValue(MtpObjectHandle handle,
                                            MtpObjectProperty property,
                                            MtpDataPacket& packet) = 0;

    virtual MtpResponseCode         setObjectPropertyValue(MtpObjectHandle handle,
                                            MtpObjectProperty property,
                                            MtpDataPacket& packet) = 0;

    virtual MtpResponseCode         getDevicePropertyValue(MtpDeviceProperty property,
                                            MtpDataPacket& packet) = 0;

    virtual MtpResponseCode         setDevicePropertyValue(MtpDeviceProperty property,
                                            MtpDataPacket& packet) = 0;

    virtual MtpResponseCode         resetDeviceProperty(MtpDeviceProperty property) = 0;

    virtual MtpResponseCode         getObjectPropertyList(MtpObjectHandle handle,
                                            uint32_t format, uint32_t property,
                                            int groupCode, int depth,
                                            MtpDataPacket& packet) = 0;

    virtual MtpResponseCode         getObjectInfo(MtpObjectHandle handle,
                                            MtpObjectInfo& info) = 0;

    virtual void*                   getThumbnail(MtpObjectHandle handle, size_t& outThumbSize) = 0;

    virtual MtpResponseCode         getObjectFilePath(MtpObjectHandle handle,
                                            MtpStringBuffer& outFilePath,
                                            int64_t& outFileLength,
                                            MtpObjectFormat& outFormat) = 0;

    virtual MtpResponseCode         beginDeleteObject(MtpObjectHandle handle) = 0;
    virtual void                    endDeleteObject(MtpObjectHandle handle, bool succeeded) = 0;

    virtual MtpObjectHandleList*    getObjectReferences(MtpObjectHandle handle) = 0;

    virtual MtpResponseCode         setObjectReferences(MtpObjectHandle handle,
                                            MtpObjectHandleList* references) = 0;

    virtual MtpProperty*            getObjectPropertyDesc(MtpObjectProperty property,
                                            MtpObjectFormat format) = 0;

    virtual MtpProperty*            getDevicePropertyDesc(MtpDeviceProperty property) = 0;

    virtual MtpResponseCode         beginMoveObject(MtpObjectHandle handle, MtpObjectHandle newParent,
                                            MtpStorageID newStorage) = 0;

    virtual void                    endMoveObject(MtpObjectHandle oldParent, MtpObjectHandle newParent,
                                            MtpStorageID oldStorage, MtpStorageID newStorage,
                                            MtpObjectHandle handle, bool succeeded) = 0;

    virtual MtpResponseCode         beginCopyObject(MtpObjectHandle handle, MtpObjectHandle newParent,
                                            MtpStorageID newStorage) = 0;
    virtual void                    endCopyObject(MtpObjectHandle handle, bool succeeded) = 0;
};

}; // namespace android

#endif // _I_MTP_DATABASE_H
