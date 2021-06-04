
/*
 * Copyright (C) 2009 The Android Open Source Project
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

#include "rsMesh.h"
#include "rsAnimation.h"
#include "rs.h"

#include <inttypes.h>

namespace android {
namespace renderscript {

FileA3D::FileA3D(Context *rsc) : ObjectBase(rsc) {
    mAlloc = nullptr;
    mData = nullptr;
    mWriteStream = nullptr;
    mReadStream = nullptr;

    mMajorVersion = 0;
    mMinorVersion = 1;
    mDataSize = 0;
}

FileA3D::~FileA3D() {
    for (size_t i = 0; i < mIndex.size(); i ++) {
        delete mIndex[i];
    }
    for (size_t i = 0; i < mWriteIndex.size(); i ++) {
        delete mWriteIndex[i];
    }
    if (mWriteStream) {
        delete mWriteStream;
    }
    if (mReadStream) {
        delete mReadStream;
    }
    if (mAlloc) {
        free(mAlloc);
    }
}

void FileA3D::parseHeader(IStream *headerStream) {
    mMajorVersion = headerStream->loadU32();
    mMinorVersion = headerStream->loadU32();
    uint32_t flags = headerStream->loadU32();
    mUse64BitOffsets = (flags & 1) != 0;

    uint32_t numIndexEntries = headerStream->loadU32();
    for (uint32_t i = 0; i < numIndexEntries; i ++) {
        A3DIndexEntry *entry = new A3DIndexEntry();
        entry->mObjectName = headerStream->loadString();

        //ALOGV("Header data, entry name = %s", entry->mObjectName.string());
        entry->mType = (RsA3DClassID)headerStream->loadU32();
        if (mUse64BitOffsets){
            entry->mOffset = headerStream->loadOffset();
            entry->mLength = headerStream->loadOffset();
        } else {
            entry->mOffset = headerStream->loadU32();
            entry->mLength = headerStream->loadU32();
        }
        entry->mRsObj = nullptr;
        mIndex.push_back(entry);
    }
}

bool FileA3D::load(Asset *asset) {
    return false;
}

bool FileA3D::load(const void *data, size_t length) {
    const uint8_t *localData = (const uint8_t *)data;

    size_t lengthRemaining = length;
    size_t magicStrLen = 12;
    if ((length < magicStrLen) ||
        memcmp(data, "Android3D_ff", magicStrLen)) {
        return false;
    }

    localData += magicStrLen;
    lengthRemaining -= magicStrLen;

    // Next we get our header size
    uint64_t headerSize = 0;
    if (lengthRemaining < sizeof(headerSize)) {
        return false;
    }

    memcpy(&headerSize, localData, sizeof(headerSize));
    localData += sizeof(headerSize);
    lengthRemaining -= sizeof(headerSize);

    if (lengthRemaining < headerSize) {
        return false;
    }

    // Now open the stream to parse the header
    IStream headerStream(localData, false);
    parseHeader(&headerStream);

    localData += headerSize;
    lengthRemaining -= headerSize;

    if (lengthRemaining < sizeof(mDataSize)) {
        return false;
    }

    // Read the size of the data
    memcpy(&mDataSize, localData, sizeof(mDataSize));
    localData += sizeof(mDataSize);
    lengthRemaining -= sizeof(mDataSize);

    if (lengthRemaining < mDataSize) {
        return false;
    }

    // We should know enough to read the file in at this point.
    mData = (uint8_t *)localData;
    mReadStream = new IStream(mData, mUse64BitOffsets);

    return true;
}

bool FileA3D::load(FILE *f) {
    char magicString[12];
    size_t len;

    ALOGV("file open 1");
    len = fread(magicString, 1, 12, f);
    if ((len != 12) ||
        memcmp(magicString, "Android3D_ff", 12)) {
        return false;
    }

    // Next thing is the size of the header
    uint64_t headerSize = 0;
    len = fread(&headerSize, 1, sizeof(headerSize), f);
    if (len != sizeof(headerSize) || headerSize == 0) {
        return false;
    }

    uint8_t *headerData = (uint8_t *)malloc(headerSize);
    if (!headerData) {
        return false;
    }

    len = fread(headerData, 1, headerSize, f);
    if (len != headerSize) {
        free(headerData);
        return false;
    }

    // Now open the stream to parse the header
    IStream headerStream(headerData, false);
    parseHeader(&headerStream);

    free(headerData);

    // Next thing is the size of the header
    len = fread(&mDataSize, 1, sizeof(mDataSize), f);
    if (len != sizeof(mDataSize) || mDataSize == 0) {
        return false;
    }

    ALOGV("file open size = %" PRIi64, mDataSize);

    // We should know enough to read the file in at this point.
    mAlloc = malloc(mDataSize);
    if (!mAlloc) {
        return false;
    }
    mData = (uint8_t *)mAlloc;
    len = fread(mAlloc, 1, mDataSize, f);
    if (len != mDataSize) {
        return false;
    }

    mReadStream = new IStream(mData, mUse64BitOffsets);

    ALOGV("Header is read an stream initialized");
    return true;
}

size_t FileA3D::getNumIndexEntries() const {
    return mIndex.size();
}

FileA3D::A3DIndexEntry::~A3DIndexEntry() {
    delete[] mObjectName;
}

const FileA3D::A3DIndexEntry *FileA3D::getIndexEntry(size_t index) const {
    if (index < mIndex.size()) {
        return mIndex[index];
    }
    return nullptr;
}

ObjectBase *FileA3D::initializeFromEntry(size_t index) {
    if (index >= mIndex.size()) {
        return nullptr;
    }

    FileA3D::A3DIndexEntry *entry = mIndex[index];
    if (!entry) {
        return nullptr;
    }

    if (entry->mRsObj) {
        entry->mRsObj->incUserRef();
        return entry->mRsObj;
    }

    // Seek to the beginning of object
    mReadStream->reset(entry->mOffset);
    switch (entry->mType) {
        case RS_A3D_CLASS_ID_UNKNOWN:
            return nullptr;
        case RS_A3D_CLASS_ID_MESH:
            entry->mRsObj = Mesh::createFromStream(mRSC, mReadStream);
            break;
        case RS_A3D_CLASS_ID_TYPE:
            entry->mRsObj = Type::createFromStream(mRSC, mReadStream);
            break;
        case RS_A3D_CLASS_ID_ELEMENT:
            entry->mRsObj = Element::createFromStream(mRSC, mReadStream);
            break;
        case RS_A3D_CLASS_ID_ALLOCATION:
            entry->mRsObj = Allocation::createFromStream(mRSC, mReadStream);
            break;
        case RS_A3D_CLASS_ID_PROGRAM_VERTEX:
            //entry->mRsObj = ProgramVertex::createFromStream(mRSC, mReadStream);
            break;
        case RS_A3D_CLASS_ID_PROGRAM_RASTER:
            //entry->mRsObj = ProgramRaster::createFromStream(mRSC, mReadStream);
            break;
        case RS_A3D_CLASS_ID_PROGRAM_FRAGMENT:
            //entry->mRsObj = ProgramFragment::createFromStream(mRSC, mReadStream);
            break;
        case RS_A3D_CLASS_ID_PROGRAM_STORE:
            //entry->mRsObj = ProgramStore::createFromStream(mRSC, mReadStream);
            break;
        case RS_A3D_CLASS_ID_SAMPLER:
            //entry->mRsObj = Sampler::createFromStream(mRSC, mReadStream);
            break;
        case RS_A3D_CLASS_ID_ANIMATION:
            //entry->mRsObj = Animation::createFromStream(mRSC, mReadStream);
            break;
        case RS_A3D_CLASS_ID_ADAPTER_1D:
            //entry->mRsObj = Adapter1D::createFromStream(mRSC, mReadStream);
            break;
        case RS_A3D_CLASS_ID_ADAPTER_2D:
            //entry->mRsObj = Adapter2D::createFromStream(mRSC, mReadStream);
            break;
        case RS_A3D_CLASS_ID_SCRIPT_C:
            break;
        case RS_A3D_CLASS_ID_SCRIPT_KERNEL_ID:
            break;
        case RS_A3D_CLASS_ID_SCRIPT_INVOKE_ID:
            break;
        case RS_A3D_CLASS_ID_SCRIPT_FIELD_ID:
            break;
        case RS_A3D_CLASS_ID_SCRIPT_METHOD_ID:
            break;
        case RS_A3D_CLASS_ID_SCRIPT_GROUP:
            break;
        case RS_A3D_CLASS_ID_CLOSURE:
            break;
        case RS_A3D_CLASS_ID_SCRIPT_GROUP2:
            break;
    }
    if (entry->mRsObj) {
        entry->mRsObj->incUserRef();
    }
    return entry->mRsObj;
}

bool FileA3D::writeFile(const char *filename) {
    if (!mWriteStream) {
        ALOGE("No objects to write\n");
        return false;
    }
    if (mWriteStream->getPos() == 0) {
        ALOGE("No objects to write\n");
        return false;
    }

    FILE *writeHandle = fopen(filename, "wbe");
    if (!writeHandle) {
        ALOGE("Couldn't open the file for writing\n");
        return false;
    }

    // Open a new stream to make writing the header easier
    OStream headerStream(5*1024, false);
    headerStream.addU32(mMajorVersion);
    headerStream.addU32(mMinorVersion);
    uint32_t is64Bit = 0;
    headerStream.addU32(is64Bit);

    uint32_t writeIndexSize = mWriteIndex.size();
    headerStream.addU32(writeIndexSize);
    for (uint32_t i = 0; i < writeIndexSize; i ++) {
        headerStream.addString(mWriteIndex[i]->mObjectName);
        headerStream.addU32((uint32_t)mWriteIndex[i]->mType);
        if (mUse64BitOffsets){
            headerStream.addOffset(mWriteIndex[i]->mOffset);
            headerStream.addOffset(mWriteIndex[i]->mLength);
        } else {
            uint32_t offset = (uint32_t)mWriteIndex[i]->mOffset;
            headerStream.addU32(offset);
            offset = (uint32_t)mWriteIndex[i]->mLength;
            headerStream.addU32(offset);
        }
    }

    // Write our magic string so we know we are reading the right file
    fwrite(A3D_MAGIC_KEY, sizeof(char), strlen(A3D_MAGIC_KEY), writeHandle);

    // Store the size of the header to make it easier to parse when we read it
    uint64_t headerSize = headerStream.getPos();
    fwrite(&headerSize, sizeof(headerSize), 1, writeHandle);

    // Now write our header
    fwrite(headerStream.getPtr(), sizeof(uint8_t), headerStream.getPos(), writeHandle);

    // Now write the size of the data part of the file for easier parsing later
    uint64_t fileDataSize = mWriteStream->getPos();
    fwrite(&fileDataSize, sizeof(fileDataSize), 1, writeHandle);

    fwrite(mWriteStream->getPtr(), sizeof(uint8_t), mWriteStream->getPos(), writeHandle);

    int status = fclose(writeHandle);

    if (status != 0) {
        ALOGE("Couldn't close file\n");
        return false;
    }

    return true;
}

void FileA3D::appendToFile(Context *con, ObjectBase *obj) {
    if (!obj) {
        return;
    }
    if (!mWriteStream) {
        const uint64_t initialStreamSize = 256*1024;
        mWriteStream = new OStream(initialStreamSize, false);
    }
    A3DIndexEntry *indexEntry = new A3DIndexEntry();
    indexEntry->mObjectName = rsuCopyString(obj->getName());
    indexEntry->mType = obj->getClassId();
    indexEntry->mOffset = mWriteStream->getPos();
    indexEntry->mRsObj = obj;
    mWriteIndex.push_back(indexEntry);
    obj->serialize(con, mWriteStream);
    indexEntry->mLength = mWriteStream->getPos() - indexEntry->mOffset;
    mWriteStream->align(4);
}

} // namespace renderscript
} // namespace android
