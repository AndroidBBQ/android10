/*
 * Copyright (C) 2012 The Android Open Source Project
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

// #define LOG_NDEBUG 0

#define LOG_TAG "Camera2-Metadata"
#include <utils/Log.h>
#include <utils/Errors.h>

#include <binder/Parcel.h>
#include <camera/CameraMetadata.h>

namespace android {

#define ALIGN_TO(val, alignment) \
    (((uintptr_t)(val) + ((alignment) - 1)) & ~((alignment) - 1))

typedef Parcel::WritableBlob WritableBlob;
typedef Parcel::ReadableBlob ReadableBlob;

CameraMetadata::CameraMetadata() :
        mBuffer(NULL), mLocked(false) {
}

CameraMetadata::CameraMetadata(size_t entryCapacity, size_t dataCapacity) :
        mLocked(false)
{
    mBuffer = allocate_camera_metadata(entryCapacity, dataCapacity);
}

CameraMetadata::CameraMetadata(const CameraMetadata &other) :
        mLocked(false) {
    mBuffer = clone_camera_metadata(other.mBuffer);
}

CameraMetadata::CameraMetadata(camera_metadata_t *buffer) :
        mBuffer(NULL), mLocked(false) {
    acquire(buffer);
}

CameraMetadata &CameraMetadata::operator=(const CameraMetadata &other) {
    return operator=(other.mBuffer);
}

CameraMetadata &CameraMetadata::operator=(const camera_metadata_t *buffer) {
    if (mLocked) {
        ALOGE("%s: Assignment to a locked CameraMetadata!", __FUNCTION__);
        return *this;
    }

    if (CC_LIKELY(buffer != mBuffer)) {
        camera_metadata_t *newBuffer = clone_camera_metadata(buffer);
        clear();
        mBuffer = newBuffer;
    }
    return *this;
}

CameraMetadata::~CameraMetadata() {
    mLocked = false;
    clear();
}

const camera_metadata_t* CameraMetadata::getAndLock() const {
    mLocked = true;
    return mBuffer;
}

status_t CameraMetadata::unlock(const camera_metadata_t *buffer) const {
    if (!mLocked) {
        ALOGE("%s: Can't unlock a non-locked CameraMetadata!", __FUNCTION__);
        return INVALID_OPERATION;
    }
    if (buffer != mBuffer) {
        ALOGE("%s: Can't unlock CameraMetadata with wrong pointer!",
                __FUNCTION__);
        return BAD_VALUE;
    }
    mLocked = false;
    return OK;
}

camera_metadata_t* CameraMetadata::release() {
    if (mLocked) {
        ALOGE("%s: CameraMetadata is locked", __FUNCTION__);
        return NULL;
    }
    camera_metadata_t *released = mBuffer;
    mBuffer = NULL;
    return released;
}

void CameraMetadata::clear() {
    if (mLocked) {
        ALOGE("%s: CameraMetadata is locked", __FUNCTION__);
        return;
    }
    if (mBuffer) {
        free_camera_metadata(mBuffer);
        mBuffer = NULL;
    }
}

void CameraMetadata::acquire(camera_metadata_t *buffer) {
    if (mLocked) {
        ALOGE("%s: CameraMetadata is locked", __FUNCTION__);
        return;
    }
    clear();
    mBuffer = buffer;

    ALOGE_IF(validate_camera_metadata_structure(mBuffer, /*size*/NULL) != OK,
             "%s: Failed to validate metadata structure %p",
             __FUNCTION__, buffer);
}

void CameraMetadata::acquire(CameraMetadata &other) {
    if (mLocked) {
        ALOGE("%s: CameraMetadata is locked", __FUNCTION__);
        return;
    }
    acquire(other.release());
}

status_t CameraMetadata::append(const CameraMetadata &other) {
    return append(other.mBuffer);
}

status_t CameraMetadata::append(const camera_metadata_t* other) {
    if (mLocked) {
        ALOGE("%s: CameraMetadata is locked", __FUNCTION__);
        return INVALID_OPERATION;
    }
    size_t extraEntries = get_camera_metadata_entry_count(other);
    size_t extraData = get_camera_metadata_data_count(other);
    resizeIfNeeded(extraEntries, extraData);

    return append_camera_metadata(mBuffer, other);
}

size_t CameraMetadata::entryCount() const {
    return (mBuffer == NULL) ? 0 :
            get_camera_metadata_entry_count(mBuffer);
}

bool CameraMetadata::isEmpty() const {
    return entryCount() == 0;
}

status_t CameraMetadata::sort() {
    if (mLocked) {
        ALOGE("%s: CameraMetadata is locked", __FUNCTION__);
        return INVALID_OPERATION;
    }
    return sort_camera_metadata(mBuffer);
}

status_t CameraMetadata::checkType(uint32_t tag, uint8_t expectedType) {
    int tagType = get_local_camera_metadata_tag_type(tag, mBuffer);
    if ( CC_UNLIKELY(tagType == -1)) {
        ALOGE("Update metadata entry: Unknown tag %d", tag);
        return INVALID_OPERATION;
    }
    if ( CC_UNLIKELY(tagType != expectedType) ) {
        ALOGE("Mismatched tag type when updating entry %s (%d) of type %s; "
                "got type %s data instead ",
                get_local_camera_metadata_tag_name(tag, mBuffer), tag,
                camera_metadata_type_names[tagType],
                camera_metadata_type_names[expectedType]);
        return INVALID_OPERATION;
    }
    return OK;
}

status_t CameraMetadata::update(uint32_t tag,
        const int32_t *data, size_t data_count) {
    status_t res;
    if (mLocked) {
        ALOGE("%s: CameraMetadata is locked", __FUNCTION__);
        return INVALID_OPERATION;
    }
    if ( (res = checkType(tag, TYPE_INT32)) != OK) {
        return res;
    }
    return updateImpl(tag, (const void*)data, data_count);
}

status_t CameraMetadata::update(uint32_t tag,
        const uint8_t *data, size_t data_count) {
    status_t res;
    if (mLocked) {
        ALOGE("%s: CameraMetadata is locked", __FUNCTION__);
        return INVALID_OPERATION;
    }
    if ( (res = checkType(tag, TYPE_BYTE)) != OK) {
        return res;
    }
    return updateImpl(tag, (const void*)data, data_count);
}

status_t CameraMetadata::update(uint32_t tag,
        const float *data, size_t data_count) {
    status_t res;
    if (mLocked) {
        ALOGE("%s: CameraMetadata is locked", __FUNCTION__);
        return INVALID_OPERATION;
    }
    if ( (res = checkType(tag, TYPE_FLOAT)) != OK) {
        return res;
    }
    return updateImpl(tag, (const void*)data, data_count);
}

status_t CameraMetadata::update(uint32_t tag,
        const int64_t *data, size_t data_count) {
    status_t res;
    if (mLocked) {
        ALOGE("%s: CameraMetadata is locked", __FUNCTION__);
        return INVALID_OPERATION;
    }
    if ( (res = checkType(tag, TYPE_INT64)) != OK) {
        return res;
    }
    return updateImpl(tag, (const void*)data, data_count);
}

status_t CameraMetadata::update(uint32_t tag,
        const double *data, size_t data_count) {
    status_t res;
    if (mLocked) {
        ALOGE("%s: CameraMetadata is locked", __FUNCTION__);
        return INVALID_OPERATION;
    }
    if ( (res = checkType(tag, TYPE_DOUBLE)) != OK) {
        return res;
    }
    return updateImpl(tag, (const void*)data, data_count);
}

status_t CameraMetadata::update(uint32_t tag,
        const camera_metadata_rational_t *data, size_t data_count) {
    status_t res;
    if (mLocked) {
        ALOGE("%s: CameraMetadata is locked", __FUNCTION__);
        return INVALID_OPERATION;
    }
    if ( (res = checkType(tag, TYPE_RATIONAL)) != OK) {
        return res;
    }
    return updateImpl(tag, (const void*)data, data_count);
}

status_t CameraMetadata::update(uint32_t tag,
        const String8 &string) {
    status_t res;
    if (mLocked) {
        ALOGE("%s: CameraMetadata is locked", __FUNCTION__);
        return INVALID_OPERATION;
    }
    if ( (res = checkType(tag, TYPE_BYTE)) != OK) {
        return res;
    }
    // string.size() doesn't count the null termination character.
    return updateImpl(tag, (const void*)string.string(), string.size() + 1);
}

status_t CameraMetadata::update(const camera_metadata_ro_entry &entry) {
    status_t res;
    if (mLocked) {
        ALOGE("%s: CameraMetadata is locked", __FUNCTION__);
        return INVALID_OPERATION;
    }
    if ( (res = checkType(entry.tag, entry.type)) != OK) {
        return res;
    }
    return updateImpl(entry.tag, (const void*)entry.data.u8, entry.count);
}

status_t CameraMetadata::updateImpl(uint32_t tag, const void *data,
        size_t data_count) {
    status_t res;
    if (mLocked) {
        ALOGE("%s: CameraMetadata is locked", __FUNCTION__);
        return INVALID_OPERATION;
    }
    int type = get_local_camera_metadata_tag_type(tag, mBuffer);
    if (type == -1) {
        ALOGE("%s: Tag %d not found", __FUNCTION__, tag);
        return BAD_VALUE;
    }
    // Safety check - ensure that data isn't pointing to this metadata, since
    // that would get invalidated if a resize is needed
    size_t bufferSize = get_camera_metadata_size(mBuffer);
    uintptr_t bufAddr = reinterpret_cast<uintptr_t>(mBuffer);
    uintptr_t dataAddr = reinterpret_cast<uintptr_t>(data);
    if (dataAddr > bufAddr && dataAddr < (bufAddr + bufferSize)) {
        ALOGE("%s: Update attempted with data from the same metadata buffer!",
                __FUNCTION__);
        return INVALID_OPERATION;
    }

    size_t data_size = calculate_camera_metadata_entry_data_size(type,
            data_count);

    res = resizeIfNeeded(1, data_size);

    if (res == OK) {
        camera_metadata_entry_t entry;
        res = find_camera_metadata_entry(mBuffer, tag, &entry);
        if (res == NAME_NOT_FOUND) {
            res = add_camera_metadata_entry(mBuffer,
                    tag, data, data_count);
        } else if (res == OK) {
            res = update_camera_metadata_entry(mBuffer,
                    entry.index, data, data_count, NULL);
        }
    }

    if (res != OK) {
        ALOGE("%s: Unable to update metadata entry %s.%s (%x): %s (%d)",
                __FUNCTION__, get_local_camera_metadata_section_name(tag, mBuffer),
                get_local_camera_metadata_tag_name(tag, mBuffer), tag,
                strerror(-res), res);
    }

    IF_ALOGV() {
        ALOGE_IF(validate_camera_metadata_structure(mBuffer, /*size*/NULL) !=
                 OK,

                 "%s: Failed to validate metadata structure after update %p",
                 __FUNCTION__, mBuffer);
    }

    return res;
}

bool CameraMetadata::exists(uint32_t tag) const {
    camera_metadata_ro_entry entry;
    return find_camera_metadata_ro_entry(mBuffer, tag, &entry) == 0;
}

camera_metadata_entry_t CameraMetadata::find(uint32_t tag) {
    status_t res;
    camera_metadata_entry entry;
    if (mLocked) {
        ALOGE("%s: CameraMetadata is locked", __FUNCTION__);
        entry.count = 0;
        return entry;
    }
    res = find_camera_metadata_entry(mBuffer, tag, &entry);
    if (CC_UNLIKELY( res != OK )) {
        entry.count = 0;
        entry.data.u8 = NULL;
    }
    return entry;
}

camera_metadata_ro_entry_t CameraMetadata::find(uint32_t tag) const {
    status_t res;
    camera_metadata_ro_entry entry;
    res = find_camera_metadata_ro_entry(mBuffer, tag, &entry);
    if (CC_UNLIKELY( res != OK )) {
        entry.count = 0;
        entry.data.u8 = NULL;
    }
    return entry;
}

status_t CameraMetadata::erase(uint32_t tag) {
    camera_metadata_entry_t entry;
    status_t res;
    if (mLocked) {
        ALOGE("%s: CameraMetadata is locked", __FUNCTION__);
        return INVALID_OPERATION;
    }
    res = find_camera_metadata_entry(mBuffer, tag, &entry);
    if (res == NAME_NOT_FOUND) {
        return OK;
    } else if (res != OK) {
        ALOGE("%s: Error looking for entry %s.%s (%x): %s %d",
                __FUNCTION__,
                get_local_camera_metadata_section_name(tag, mBuffer),
                get_local_camera_metadata_tag_name(tag, mBuffer),
                tag, strerror(-res), res);
        return res;
    }
    res = delete_camera_metadata_entry(mBuffer, entry.index);
    if (res != OK) {
        ALOGE("%s: Error deleting entry %s.%s (%x): %s %d",
                __FUNCTION__,
                get_local_camera_metadata_section_name(tag, mBuffer),
                get_local_camera_metadata_tag_name(tag, mBuffer),
                tag, strerror(-res), res);
    }
    return res;
}

status_t CameraMetadata::removePermissionEntries(metadata_vendor_id_t vendorId,
        std::vector<int32_t> *tagsRemoved) {
    uint32_t tagCount = 0;
    std::vector<uint32_t> tagsToRemove;

    if (tagsRemoved == nullptr) {
        return BAD_VALUE;
    }

    sp<VendorTagDescriptor> vTags = VendorTagDescriptor::getGlobalVendorTagDescriptor();
    if ((nullptr == vTags.get()) || (0 >= vTags->getTagCount())) {
        sp<VendorTagDescriptorCache> cache =
            VendorTagDescriptorCache::getGlobalVendorTagCache();
        if (cache.get()) {
            cache->getVendorTagDescriptor(vendorId, &vTags);
        }
    }

    if ((nullptr != vTags.get()) && (vTags->getTagCount() > 0)) {
        tagCount = vTags->getTagCount();
        uint32_t *vendorTags = new uint32_t[tagCount];
        if (nullptr == vendorTags) {
            return NO_MEMORY;
        }
        vTags->getTagArray(vendorTags);

        tagsToRemove.reserve(tagCount);
        tagsToRemove.insert(tagsToRemove.begin(), vendorTags, vendorTags + tagCount);

        delete [] vendorTags;
        tagCount = 0;
    }

    auto tagsNeedingPermission = get_camera_metadata_permission_needed(&tagCount);
    if (tagCount > 0) {
        tagsToRemove.reserve(tagsToRemove.capacity() + tagCount);
        tagsToRemove.insert(tagsToRemove.end(), tagsNeedingPermission,
                tagsNeedingPermission + tagCount);
    }

    tagsRemoved->reserve(tagsToRemove.size());
    for (const auto &it : tagsToRemove) {
        if (exists(it)) {
            auto rc = erase(it);
            if (NO_ERROR != rc) {
                ALOGE("%s: Failed to erase tag: %x", __func__, it);
                return rc;
            }
            tagsRemoved->push_back(it);
        }
    }

    // Update the available characterstics accordingly
    if (exists(ANDROID_REQUEST_AVAILABLE_CHARACTERISTICS_KEYS)) {
        std::vector<uint32_t> currentKeys;

        std::sort(tagsRemoved->begin(), tagsRemoved->end());
        auto keys = find(ANDROID_REQUEST_AVAILABLE_CHARACTERISTICS_KEYS);
        currentKeys.reserve(keys.count);
        currentKeys.insert(currentKeys.end(), keys.data.i32, keys.data.i32 + keys.count);
        std::sort(currentKeys.begin(), currentKeys.end());

        std::vector<int32_t> newKeys(keys.count);
        auto end = std::set_difference(currentKeys.begin(), currentKeys.end(), tagsRemoved->begin(),
                tagsRemoved->end(), newKeys.begin());
        newKeys.resize(end - newKeys.begin());

        update(ANDROID_REQUEST_AVAILABLE_CHARACTERISTICS_KEYS, newKeys.data(), newKeys.size());
    }

    return NO_ERROR;
}

void CameraMetadata::dump(int fd, int verbosity, int indentation) const {
    dump_indented_camera_metadata(mBuffer, fd, verbosity, indentation);
}

status_t CameraMetadata::resizeIfNeeded(size_t extraEntries, size_t extraData) {
    if (mBuffer == NULL) {
        mBuffer = allocate_camera_metadata(extraEntries * 2, extraData * 2);
        if (mBuffer == NULL) {
            ALOGE("%s: Can't allocate larger metadata buffer", __FUNCTION__);
            return NO_MEMORY;
        }
    } else {
        size_t currentEntryCount = get_camera_metadata_entry_count(mBuffer);
        size_t currentEntryCap = get_camera_metadata_entry_capacity(mBuffer);
        size_t newEntryCount = currentEntryCount +
                extraEntries;
        newEntryCount = (newEntryCount > currentEntryCap) ?
                newEntryCount * 2 : currentEntryCap;

        size_t currentDataCount = get_camera_metadata_data_count(mBuffer);
        size_t currentDataCap = get_camera_metadata_data_capacity(mBuffer);
        size_t newDataCount = currentDataCount +
                extraData;
        newDataCount = (newDataCount > currentDataCap) ?
                newDataCount * 2 : currentDataCap;

        if (newEntryCount > currentEntryCap ||
                newDataCount > currentDataCap) {
            camera_metadata_t *oldBuffer = mBuffer;
            mBuffer = allocate_camera_metadata(newEntryCount,
                    newDataCount);
            if (mBuffer == NULL) {
                ALOGE("%s: Can't allocate larger metadata buffer", __FUNCTION__);
                return NO_MEMORY;
            }
            append_camera_metadata(mBuffer, oldBuffer);
            free_camera_metadata(oldBuffer);
        }
    }
    return OK;
}

status_t CameraMetadata::readFromParcel(const Parcel& data,
                                        camera_metadata_t** out) {

    status_t err = OK;

    camera_metadata_t* metadata = NULL;

    if (out) {
        *out = NULL;
    }

    // See CameraMetadata::writeToParcel for parcel data layout diagram and explanation.
    // arg0 = blobSize (int32)
    int32_t blobSizeTmp = -1;
    if ((err = data.readInt32(&blobSizeTmp)) != OK) {
        ALOGE("%s: Failed to read metadata size (error %d %s)",
              __FUNCTION__, err, strerror(-err));
        return err;
    }
    const size_t blobSize = static_cast<size_t>(blobSizeTmp);
    const size_t alignment = get_camera_metadata_alignment();

    // Special case: zero blob size means zero sized (NULL) metadata.
    if (blobSize == 0) {
        ALOGV("%s: Read 0-sized metadata", __FUNCTION__);
        return OK;
    }

    if (blobSize <= alignment) {
        ALOGE("%s: metadata blob is malformed, blobSize(%zu) should be larger than alignment(%zu)",
                __FUNCTION__, blobSize, alignment);
        return BAD_VALUE;
    }

    const size_t metadataSize = blobSize - alignment;

    // NOTE: this doesn't make sense to me. shouldn't the blob
    // know how big it is? why do we have to specify the size
    // to Parcel::readBlob ?
    ReadableBlob blob;
    // arg1 = metadata (blob)
    do {
        if ((err = data.readBlob(blobSize, &blob)) != OK) {
            ALOGE("%s: Failed to read metadata blob (sized %zu). Possible "
                  " serialization bug. Error %d %s",
                  __FUNCTION__, blobSize, err, strerror(-err));
            break;
        }

        // arg2 = offset (blob)
        // Must be after blob since we don't know offset until after writeBlob.
        int32_t offsetTmp;
        if ((err = data.readInt32(&offsetTmp)) != OK) {
            ALOGE("%s: Failed to read metadata offsetTmp (error %d %s)",
                  __FUNCTION__, err, strerror(-err));
            break;
        }
        const size_t offset = static_cast<size_t>(offsetTmp);
        if (offset >= alignment) {
            ALOGE("%s: metadata offset(%zu) should be less than alignment(%zu)",
                    __FUNCTION__, blobSize, alignment);
            err = BAD_VALUE;
            break;
        }

        const uintptr_t metadataStart = reinterpret_cast<uintptr_t>(blob.data()) + offset;
        const camera_metadata_t* tmp =
                       reinterpret_cast<const camera_metadata_t*>(metadataStart);
        ALOGV("%s: alignment is: %zu, metadata start: %p, offset: %zu",
                __FUNCTION__, alignment, tmp, offset);
        metadata = allocate_copy_camera_metadata_checked(tmp, metadataSize);
        if (metadata == NULL) {
            // We consider that allocation only fails if the validation
            // also failed, therefore the readFromParcel was a failure.
            ALOGE("%s: metadata allocation and copy failed", __FUNCTION__);
            err = BAD_VALUE;
        }
    } while(0);
    blob.release();

    if (out) {
        ALOGV("%s: Set out metadata to %p", __FUNCTION__, metadata);
        *out = metadata;
    } else if (metadata != NULL) {
        ALOGV("%s: Freed camera metadata at %p", __FUNCTION__, metadata);
        free_camera_metadata(metadata);
    }

    return err;
}

status_t CameraMetadata::writeToParcel(Parcel& data,
                                       const camera_metadata_t* metadata) {
    status_t res = OK;

    /**
     * Below is the camera metadata parcel layout:
     *
     * |--------------------------------------------|
     * |             arg0: blobSize                 |
     * |              (length = 4)                  |
     * |--------------------------------------------|<--Skip the rest if blobSize == 0.
     * |                                            |
     * |                                            |
     * |              arg1: blob                    |
     * | (length = variable, see arg1 layout below) |
     * |                                            |
     * |                                            |
     * |--------------------------------------------|
     * |              arg2: offset                  |
     * |              (length = 4)                  |
     * |--------------------------------------------|
     */

    // arg0 = blobSize (int32)
    if (metadata == NULL) {
        // Write zero blobSize for null metadata.
        return data.writeInt32(0);
    }

    /**
     * Always make the blob size sufficiently larger, as we need put alignment
     * padding and metadata into the blob. Since we don't know the alignment
     * offset before writeBlob. Then write the metadata to aligned offset.
     */
    const size_t metadataSize = get_camera_metadata_compact_size(metadata);
    const size_t alignment = get_camera_metadata_alignment();
    const size_t blobSize = metadataSize + alignment;
    res = data.writeInt32(static_cast<int32_t>(blobSize));
    if (res != OK) {
        return res;
    }

    size_t offset = 0;
    /**
     * arg1 = metadata (blob).
     *
     * The blob size is the sum of front padding size, metadata size and back padding
     * size, which is equal to metadataSize + alignment.
     *
     * The blob layout is:
     * |------------------------------------|<----Start address of the blob (unaligned).
     * |           front padding            |
     * |          (size = offset)           |
     * |------------------------------------|<----Aligned start address of metadata.
     * |                                    |
     * |                                    |
     * |            metadata                |
     * |       (size = metadataSize)        |
     * |                                    |
     * |                                    |
     * |------------------------------------|
     * |           back padding             |
     * |     (size = alignment - offset)    |
     * |------------------------------------|<----End address of blob.
     *                                            (Blob start address + blob size).
     */
    WritableBlob blob;
    do {
        res = data.writeBlob(blobSize, false, &blob);
        if (res != OK) {
            break;
        }
        const uintptr_t metadataStart = ALIGN_TO(blob.data(), alignment);
        offset = metadataStart - reinterpret_cast<uintptr_t>(blob.data());
        ALOGV("%s: alignment is: %zu, metadata start: %p, offset: %zu",
                __FUNCTION__, alignment,
                reinterpret_cast<const void *>(metadataStart), offset);
        copy_camera_metadata(reinterpret_cast<void*>(metadataStart), metadataSize, metadata);

        // Not too big of a problem since receiving side does hard validation
        // Don't check the size since the compact size could be larger
        if (validate_camera_metadata_structure(metadata, /*size*/NULL) != OK) {
            ALOGW("%s: Failed to validate metadata %p before writing blob",
                   __FUNCTION__, metadata);
        }

    } while(false);
    blob.release();

    // arg2 = offset (int32)
    res = data.writeInt32(static_cast<int32_t>(offset));

    return res;
}

status_t CameraMetadata::readFromParcel(const Parcel *parcel) {

    ALOGV("%s: parcel = %p", __FUNCTION__, parcel);

    status_t res = OK;

    if (parcel == NULL) {
        ALOGE("%s: parcel is null", __FUNCTION__);
        return BAD_VALUE;
    }

    if (mLocked) {
        ALOGE("%s: CameraMetadata is locked", __FUNCTION__);
        return INVALID_OPERATION;
    }

    camera_metadata *buffer = NULL;
    // TODO: reading should return a status code, in case validation fails
    res = CameraMetadata::readFromParcel(*parcel, &buffer);

    if (res != NO_ERROR) {
        ALOGE("%s: Failed to read from parcel. Metadata is unchanged.",
              __FUNCTION__);
        return res;
    }

    clear();
    mBuffer = buffer;

    return OK;
}

status_t CameraMetadata::writeToParcel(Parcel *parcel) const {

    ALOGV("%s: parcel = %p", __FUNCTION__, parcel);

    if (parcel == NULL) {
        ALOGE("%s: parcel is null", __FUNCTION__);
        return BAD_VALUE;
    }

    return CameraMetadata::writeToParcel(*parcel, mBuffer);
}

void CameraMetadata::swap(CameraMetadata& other) {
    if (mLocked) {
        ALOGE("%s: CameraMetadata is locked", __FUNCTION__);
        return;
    } else if (other.mLocked) {
        ALOGE("%s: Other CameraMetadata is locked", __FUNCTION__);
        return;
    }

    camera_metadata* thisBuf = mBuffer;
    camera_metadata* otherBuf = other.mBuffer;

    other.mBuffer = thisBuf;
    mBuffer = otherBuf;
}

status_t CameraMetadata::getTagFromName(const char *name,
        const VendorTagDescriptor* vTags, uint32_t *tag) {

    if (name == nullptr || tag == nullptr) return BAD_VALUE;

    size_t nameLength = strlen(name);

    const SortedVector<String8> *vendorSections;
    size_t vendorSectionCount = 0;

    if (vTags != NULL) {
        vendorSections = vTags->getAllSectionNames();
        vendorSectionCount = vendorSections->size();
    }

    // First, find the section by the longest string match
    const char *section = NULL;
    size_t sectionIndex = 0;
    size_t sectionLength = 0;
    size_t totalSectionCount = ANDROID_SECTION_COUNT + vendorSectionCount;
    for (size_t i = 0; i < totalSectionCount; ++i) {

        const char *str = (i < ANDROID_SECTION_COUNT) ? camera_metadata_section_names[i] :
                (*vendorSections)[i - ANDROID_SECTION_COUNT].string();

        ALOGV("%s: Trying to match against section '%s'", __FUNCTION__, str);

        if (strstr(name, str) == name) { // name begins with the section name
            size_t strLength = strlen(str);

            ALOGV("%s: Name begins with section name", __FUNCTION__);

            // section name is the longest we've found so far
            if (section == NULL || sectionLength < strLength) {
                section = str;
                sectionIndex = i;
                sectionLength = strLength;

                ALOGV("%s: Found new best section (%s)", __FUNCTION__, section);
            }
        }
    }

    // TODO: Make above get_camera_metadata_section_from_name ?

    if (section == NULL) {
        return NAME_NOT_FOUND;
    } else {
        ALOGV("%s: Found matched section '%s' (%zu)",
              __FUNCTION__, section, sectionIndex);
    }

    // Get the tag name component of the name
    const char *nameTagName = name + sectionLength + 1; // x.y.z -> z
    if (sectionLength + 1 >= nameLength) {
        return BAD_VALUE;
    }

    // Match rest of name against the tag names in that section only
    uint32_t candidateTag = 0;
    if (sectionIndex < ANDROID_SECTION_COUNT) {
        // Match built-in tags (typically android.*)
        uint32_t tagBegin, tagEnd; // [tagBegin, tagEnd)
        tagBegin = camera_metadata_section_bounds[sectionIndex][0];
        tagEnd = camera_metadata_section_bounds[sectionIndex][1];

        for (candidateTag = tagBegin; candidateTag < tagEnd; ++candidateTag) {
            const char *tagName = get_camera_metadata_tag_name(candidateTag);

            if (strcmp(nameTagName, tagName) == 0) {
                ALOGV("%s: Found matched tag '%s' (%d)",
                      __FUNCTION__, tagName, candidateTag);
                break;
            }
        }

        if (candidateTag == tagEnd) {
            return NAME_NOT_FOUND;
        }
    } else if (vTags != NULL) {
        // Match vendor tags (typically com.*)
        const String8 sectionName(section);
        const String8 tagName(nameTagName);

        status_t res = OK;
        if ((res = vTags->lookupTag(tagName, sectionName, &candidateTag)) != OK) {
            return NAME_NOT_FOUND;
        }
    }

    *tag = candidateTag;
    return OK;
}


}; // namespace android
