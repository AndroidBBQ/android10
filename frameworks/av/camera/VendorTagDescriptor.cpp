/*
 * Copyright (C) 2014 The Android Open Source Project
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

#define LOG_TAG "VendorTagDescriptor"

#include <binder/Parcel.h>
#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/Mutex.h>
#include <utils/Vector.h>
#include <utils/SortedVector.h>
#include <system/camera_metadata.h>
#include <camera_metadata_hidden.h>

#include "camera/VendorTagDescriptor.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

namespace android {

extern "C" {

static int vendor_tag_descriptor_get_tag_count(const vendor_tag_ops_t* v);
static void vendor_tag_descriptor_get_all_tags(const vendor_tag_ops_t* v, uint32_t* tagArray);
static const char* vendor_tag_descriptor_get_section_name(const vendor_tag_ops_t* v, uint32_t tag);
static const char* vendor_tag_descriptor_get_tag_name(const vendor_tag_ops_t* v, uint32_t tag);
static int vendor_tag_descriptor_get_tag_type(const vendor_tag_ops_t* v, uint32_t tag);

static int vendor_tag_descriptor_cache_get_tag_count(metadata_vendor_id_t id);
static void vendor_tag_descriptor_cache_get_all_tags(uint32_t* tagArray,
        metadata_vendor_id_t id);
static const char* vendor_tag_descriptor_cache_get_section_name(uint32_t tag,
        metadata_vendor_id_t id);
static const char* vendor_tag_descriptor_cache_get_tag_name(uint32_t tag,
        metadata_vendor_id_t id);
static int vendor_tag_descriptor_cache_get_tag_type(uint32_t tag,
        metadata_vendor_id_t id);

} /* extern "C" */


static Mutex sLock;
static sp<VendorTagDescriptor> sGlobalVendorTagDescriptor;
static sp<VendorTagDescriptorCache> sGlobalVendorTagDescriptorCache;

namespace hardware {
namespace camera2 {
namespace params {

VendorTagDescriptor::~VendorTagDescriptor() {
    size_t len = mReverseMapping.size();
    for (size_t i = 0; i < len; ++i)  {
        delete mReverseMapping[i];
    }
}

VendorTagDescriptor::VendorTagDescriptor() :
        mTagCount(0),
        mVendorOps() {
}

VendorTagDescriptor::VendorTagDescriptor(const VendorTagDescriptor& src) {
    copyFrom(src);
}

VendorTagDescriptor& VendorTagDescriptor::operator=(const VendorTagDescriptor& rhs) {
    copyFrom(rhs);
    return *this;
}

void VendorTagDescriptor::copyFrom(const VendorTagDescriptor& src) {
    if (this == &src) return;

    size_t len = mReverseMapping.size();
    for (size_t i = 0; i < len; ++i) {
        delete mReverseMapping[i];
    }
    mReverseMapping.clear();

    len = src.mReverseMapping.size();
    // Have to copy KeyedVectors inside mReverseMapping
    for (size_t i = 0; i < len; ++i) {
        KeyedVector<String8, uint32_t>* nameMapper = new KeyedVector<String8, uint32_t>();
        *nameMapper = *(src.mReverseMapping.valueAt(i));
        mReverseMapping.add(src.mReverseMapping.keyAt(i), nameMapper);
    }
    // Everything else is simple
    mTagToNameMap = src.mTagToNameMap;
    mTagToSectionMap = src.mTagToSectionMap;
    mTagToTypeMap = src.mTagToTypeMap;
    mSections = src.mSections;
    mTagCount = src.mTagCount;
    mVendorOps = src.mVendorOps;
}

status_t VendorTagDescriptor::readFromParcel(const android::Parcel* parcel) {
    status_t res = OK;
    if (parcel == NULL) {
        ALOGE("%s: parcel argument was NULL.", __FUNCTION__);
        return BAD_VALUE;
    }

    int32_t tagCount = 0;
    if ((res = parcel->readInt32(&tagCount)) != OK) {
        ALOGE("%s: could not read tag count from parcel", __FUNCTION__);
        return res;
    }

    if (tagCount < 0 || tagCount > INT32_MAX) {
        ALOGE("%s: tag count %d from vendor ops is invalid.", __FUNCTION__, tagCount);
        return BAD_VALUE;
    }

    mTagCount = tagCount;

    uint32_t tag, sectionIndex;
    uint32_t maxSectionIndex = 0;
    int32_t tagType;
    Vector<uint32_t> allTags;
    for (int32_t i = 0; i < tagCount; ++i) {
        if ((res = parcel->readInt32(reinterpret_cast<int32_t*>(&tag))) != OK) {
            ALOGE("%s: could not read tag id from parcel for index %d", __FUNCTION__, i);
            break;
        }
        if (tag < CAMERA_METADATA_VENDOR_TAG_BOUNDARY) {
            ALOGE("%s: vendor tag %d not in vendor tag section.", __FUNCTION__, tag);
            res = BAD_VALUE;
            break;
        }
        if ((res = parcel->readInt32(&tagType)) != OK) {
            ALOGE("%s: could not read tag type from parcel for tag %d", __FUNCTION__, tag);
            break;
        }
        if (tagType < 0 || tagType >= NUM_TYPES) {
            ALOGE("%s: tag type %d from vendor ops does not exist.", __FUNCTION__, tagType);
            res = BAD_VALUE;
            break;
        }
        String8 tagName = parcel->readString8();
        if (tagName.isEmpty()) {
            ALOGE("%s: parcel tag name was NULL for tag %d.", __FUNCTION__, tag);
            res = NOT_ENOUGH_DATA;
            break;
        }

        if ((res = parcel->readInt32(reinterpret_cast<int32_t*>(&sectionIndex))) != OK) {
            ALOGE("%s: could not read section index for tag %d.", __FUNCTION__, tag);
            break;
        }

        maxSectionIndex = (maxSectionIndex >= sectionIndex) ? maxSectionIndex : sectionIndex;

        allTags.add(tag);
        mTagToNameMap.add(tag, tagName);
        mTagToSectionMap.add(tag, sectionIndex);
        mTagToTypeMap.add(tag, tagType);
    }

    if (res != OK) {
        return res;
    }

    size_t sectionCount = 0;
    if (tagCount > 0) {
        if ((res = parcel->readInt32(reinterpret_cast<int32_t*>(&sectionCount))) != OK) {
            ALOGE("%s: could not read section count for.", __FUNCTION__);
            return res;
        }
        if (sectionCount < (maxSectionIndex + 1)) {
            ALOGE("%s: Incorrect number of sections defined, received %zu, needs %d.",
                    __FUNCTION__, sectionCount, (maxSectionIndex + 1));
            return BAD_VALUE;
        }
        LOG_ALWAYS_FATAL_IF(mSections.setCapacity(sectionCount) <= 0,
                "Vector capacity must be positive");
        for (size_t i = 0; i < sectionCount; ++i) {
            String8 sectionName = parcel->readString8();
            if (sectionName.isEmpty()) {
                ALOGE("%s: parcel section name was NULL for section %zu.",
                      __FUNCTION__, i);
                return NOT_ENOUGH_DATA;
            }
            mSections.add(sectionName);
        }
    }

    LOG_ALWAYS_FATAL_IF(static_cast<size_t>(tagCount) != allTags.size(),
                        "tagCount must be the same as allTags size");
    // Set up reverse mapping
    for (size_t i = 0; i < static_cast<size_t>(tagCount); ++i) {
        uint32_t tag = allTags[i];
        const String8& sectionString = mSections[mTagToSectionMap.valueFor(tag)];

        ssize_t reverseIndex = -1;
        if ((reverseIndex = mReverseMapping.indexOfKey(sectionString)) < 0) {
            KeyedVector<String8, uint32_t>* nameMapper = new KeyedVector<String8, uint32_t>();
            reverseIndex = mReverseMapping.add(sectionString, nameMapper);
        }
        mReverseMapping[reverseIndex]->add(mTagToNameMap.valueFor(tag), tag);
    }

    return res;
}

int VendorTagDescriptor::getTagCount() const {
    size_t size = mTagToNameMap.size();
    if (size == 0) {
        return VENDOR_TAG_COUNT_ERR;
    }
    return size;
}

void VendorTagDescriptor::getTagArray(uint32_t* tagArray) const {
    size_t size = mTagToNameMap.size();
    for (size_t i = 0; i < size; ++i) {
        tagArray[i] = mTagToNameMap.keyAt(i);
    }
}

const char* VendorTagDescriptor::getSectionName(uint32_t tag) const {
    ssize_t index = mTagToSectionMap.indexOfKey(tag);
    if (index < 0) {
        return VENDOR_SECTION_NAME_ERR;
    }
    return mSections[mTagToSectionMap.valueAt(index)].string();
}

const char* VendorTagDescriptor::getTagName(uint32_t tag) const {
    ssize_t index = mTagToNameMap.indexOfKey(tag);
    if (index < 0) {
        return VENDOR_TAG_NAME_ERR;
    }
    return mTagToNameMap.valueAt(index).string();
}

int VendorTagDescriptor::getTagType(uint32_t tag) const {
    ssize_t index = mTagToNameMap.indexOfKey(tag);
    if (index < 0) {
        return VENDOR_TAG_TYPE_ERR;
    }
    return mTagToTypeMap.valueFor(tag);
}

status_t VendorTagDescriptor::writeToParcel(android::Parcel* parcel) const {
    status_t res = OK;
    if (parcel == NULL) {
        ALOGE("%s: parcel argument was NULL.", __FUNCTION__);
        return BAD_VALUE;
    }

    if ((res = parcel->writeInt32(mTagCount)) != OK) {
        return res;
    }

    size_t size = mTagToNameMap.size();
    uint32_t tag, sectionIndex;
    int32_t tagType;
    for (size_t i = 0; i < size; ++i) {
        tag = mTagToNameMap.keyAt(i);
        String8 tagName = mTagToNameMap[i];
        sectionIndex = mTagToSectionMap.valueFor(tag);
        tagType = mTagToTypeMap.valueFor(tag);
        if ((res = parcel->writeInt32(tag)) != OK) break;
        if ((res = parcel->writeInt32(tagType)) != OK) break;
        if ((res = parcel->writeString8(tagName)) != OK) break;
        if ((res = parcel->writeInt32(sectionIndex)) != OK) break;
    }

    size_t numSections = mSections.size();
    if (numSections > 0) {
        if ((res = parcel->writeInt32(numSections)) != OK) return res;
        for (size_t i = 0; i < numSections; ++i) {
            if ((res = parcel->writeString8(mSections[i])) != OK) return res;
        }
    }

    return res;
}

const SortedVector<String8>* VendorTagDescriptor::getAllSectionNames() const {
    return &mSections;
}

status_t VendorTagDescriptor::lookupTag(const String8& name, const String8& section, /*out*/uint32_t* tag) const {
    ssize_t index = mReverseMapping.indexOfKey(section);
    if (index < 0) {
        ALOGE("%s: Section '%s' does not exist.", __FUNCTION__, section.string());
        return BAD_VALUE;
    }

    ssize_t nameIndex = mReverseMapping[index]->indexOfKey(name);
    if (nameIndex < 0) {
        ALOGE("%s: Tag name '%s' does not exist.", __FUNCTION__, name.string());
        return BAD_VALUE;
    }

    if (tag != NULL) {
        *tag = mReverseMapping[index]->valueAt(nameIndex);
    }
    return OK;
}

ssize_t VendorTagDescriptor::getSectionIndex(uint32_t tag) const {
    return mTagToSectionMap.valueFor(tag);
}

void VendorTagDescriptor::dump(int fd, int verbosity, int indentation) const {

    size_t size = mTagToNameMap.size();
    if (size == 0) {
        dprintf(fd, "%*sDumping configured vendor tag descriptors: None set\n",
                indentation, "");
        return;
    }

    dprintf(fd, "%*sDumping configured vendor tag descriptors: %zu entries\n",
            indentation, "", size);
    for (size_t i = 0; i < size; ++i) {
        uint32_t tag =  mTagToNameMap.keyAt(i);

        if (verbosity < 1) {
            dprintf(fd, "%*s0x%x\n", indentation + 2, "", tag);
            continue;
        }
        String8 name = mTagToNameMap.valueAt(i);
        uint32_t sectionId = mTagToSectionMap.valueFor(tag);
        String8 sectionName = mSections[sectionId];
        int type = mTagToTypeMap.valueFor(tag);
        const char* typeName = (type >= 0 && type < NUM_TYPES) ?
                camera_metadata_type_names[type] : "UNKNOWN";
        dprintf(fd, "%*s0x%x (%s) with type %d (%s) defined in section %s\n", indentation + 2,
            "", tag, name.string(), type, typeName, sectionName.string());
    }

}

status_t VendorTagDescriptorCache::writeToParcel(Parcel* parcel) const {
    status_t res = OK;
    if (parcel == NULL) {
        ALOGE("%s: parcel argument was NULL.", __FUNCTION__);
        return BAD_VALUE;
    }

    if ((res = parcel->writeInt32(mVendorMap.size())) != OK) {
        return res;
    }

    for (const auto &iter : mVendorMap) {
        if ((res = parcel->writeUint64(iter.first)) != OK) break;
        if ((res = parcel->writeParcelable(*iter.second)) != OK) break;
    }

    return res;
}


status_t VendorTagDescriptorCache::readFromParcel(const Parcel* parcel) {
    status_t res = OK;
    if (parcel == NULL) {
        ALOGE("%s: parcel argument was NULL.", __FUNCTION__);
        return BAD_VALUE;
    }

    int32_t vendorCount = 0;
    if ((res = parcel->readInt32(&vendorCount)) != OK) {
        ALOGE("%s: could not read vendor count from parcel", __FUNCTION__);
        return res;
    }

    if (vendorCount < 0 || vendorCount > INT32_MAX) {
        ALOGE("%s: vendor count %d from is invalid.", __FUNCTION__, vendorCount);
        return BAD_VALUE;
    }

    metadata_vendor_id_t id;
    for (int32_t i = 0; i < vendorCount; i++) {
        if ((res = parcel->readUint64(&id)) != OK) {
            ALOGE("%s: could not read vendor id from parcel for index %d",
                  __FUNCTION__, i);
            break;
        }
        sp<android::VendorTagDescriptor> desc = new android::VendorTagDescriptor();
        if ((res = parcel->readParcelable(desc.get())) != OK) {
            ALOGE("%s: could not read vendor tag descriptor from parcel for index %d rc = %d",
                  __FUNCTION__, i, res);
            break;
        }

        if ((res = addVendorDescriptor(id, desc)) != OK) {
            ALOGE("%s: failed to add vendor tag descriptor for index: %d ",
                  __FUNCTION__, i);
            break;
        }
    }

    return res;
}

const std::unordered_map<metadata_vendor_id_t, sp<android::VendorTagDescriptor>> &
            VendorTagDescriptorCache::getVendorIdsAndTagDescriptors() {
    return mVendorMap;
}

int VendorTagDescriptorCache::getTagCount(metadata_vendor_id_t id) const {
    int ret = 0;
    auto desc = mVendorMap.find(id);
    if (desc != mVendorMap.end()) {
        ret = desc->second->getTagCount();
    } else {
        ALOGE("%s: Vendor descriptor id is missing!", __func__);
    }

    return ret;
}

void VendorTagDescriptorCache::getTagArray(uint32_t* tagArray,
        metadata_vendor_id_t id) const {
    auto desc = mVendorMap.find(id);
    if (desc != mVendorMap.end()) {
        desc->second->getTagArray(tagArray);
    } else {
        ALOGE("%s: Vendor descriptor id is missing!", __func__);
    }
}

const char* VendorTagDescriptorCache::getSectionName(uint32_t tag,
        metadata_vendor_id_t id) const {
    const char *ret = nullptr;
    auto desc = mVendorMap.find(id);
    if (desc != mVendorMap.end()) {
        ret = desc->second->getSectionName(tag);
    } else {
        ALOGE("%s: Vendor descriptor id is missing!", __func__);
    }

    return ret;
}

const char* VendorTagDescriptorCache::getTagName(uint32_t tag,
        metadata_vendor_id_t id) const {
    const char *ret = nullptr;
    auto desc = mVendorMap.find(id);
    if (desc != mVendorMap.end()) {
        ret = desc->second->getTagName(tag);
    } else {
        ALOGE("%s: Vendor descriptor id is missing!", __func__);
    }

    return ret;
}

int VendorTagDescriptorCache::getTagType(uint32_t tag,
        metadata_vendor_id_t id) const {
    int ret = 0;
    auto desc = mVendorMap.find(id);
    if (desc != mVendorMap.end()) {
        ret = desc->second->getTagType(tag);
    } else {
        ALOGE("%s: Vendor descriptor id is missing!", __func__);
    }

    return ret;
}

void VendorTagDescriptorCache::dump(int fd, int verbosity,
        int indentation) const {
    for (const auto &desc : mVendorMap) {
        dprintf(fd, "%*sDumping vendor tag descriptors for vendor with"
                " id %" PRIu64 " \n", indentation, "", desc.first);
        desc.second->dump(fd, verbosity, indentation);
    }
}

int32_t VendorTagDescriptorCache::addVendorDescriptor(metadata_vendor_id_t id,
        sp<android::VendorTagDescriptor> desc) {
    auto entry = mVendorMap.find(id);
    if (entry != mVendorMap.end()) {
        ALOGE("%s: Vendor descriptor with same id already present!", __func__);
        return BAD_VALUE;
    }

    mVendorMap.emplace(id, desc);
    return NO_ERROR;
}

int32_t VendorTagDescriptorCache::getVendorTagDescriptor(
        metadata_vendor_id_t id, sp<android::VendorTagDescriptor> *desc /*out*/) {
    auto entry = mVendorMap.find(id);
    if (entry == mVendorMap.end()) {
        return NAME_NOT_FOUND;
    }

    *desc = entry->second;

    return NO_ERROR;
}

} // namespace params
} // namespace camera2
} // namespace hardware

status_t VendorTagDescriptor::createDescriptorFromOps(const vendor_tag_ops_t* vOps,
            /*out*/
            sp<VendorTagDescriptor>& descriptor) {
    if (vOps == NULL) {
        ALOGE("%s: vendor_tag_ops argument was NULL.", __FUNCTION__);
        return BAD_VALUE;
    }

    int tagCount = vOps->get_tag_count(vOps);
    if (tagCount < 0 || tagCount > INT32_MAX) {
        ALOGE("%s: tag count %d from vendor ops is invalid.", __FUNCTION__, tagCount);
        return BAD_VALUE;
    }

    Vector<uint32_t> tagArray;
    LOG_ALWAYS_FATAL_IF(tagArray.resize(tagCount) != tagCount,
            "%s: too many (%u) vendor tags defined.", __FUNCTION__, tagCount);

    vOps->get_all_tags(vOps, /*out*/tagArray.editArray());

    sp<VendorTagDescriptor> desc = new VendorTagDescriptor();
    desc->mTagCount = tagCount;

    SortedVector<String8> sections;
    KeyedVector<uint32_t, String8> tagToSectionMap;

    for (size_t i = 0; i < static_cast<size_t>(tagCount); ++i) {
        uint32_t tag = tagArray[i];
        if (tag < CAMERA_METADATA_VENDOR_TAG_BOUNDARY) {
            ALOGE("%s: vendor tag %d not in vendor tag section.", __FUNCTION__, tag);
            return BAD_VALUE;
        }
        const char *tagName = vOps->get_tag_name(vOps, tag);
        if (tagName == NULL) {
            ALOGE("%s: no tag name defined for vendor tag %d.", __FUNCTION__, tag);
            return BAD_VALUE;
        }
        desc->mTagToNameMap.add(tag, String8(tagName));
        const char *sectionName = vOps->get_section_name(vOps, tag);
        if (sectionName == NULL) {
            ALOGE("%s: no section name defined for vendor tag %d.", __FUNCTION__, tag);
            return BAD_VALUE;
        }

        String8 sectionString(sectionName);

        sections.add(sectionString);
        tagToSectionMap.add(tag, sectionString);

        int tagType = vOps->get_tag_type(vOps, tag);
        if (tagType < 0 || tagType >= NUM_TYPES) {
            ALOGE("%s: tag type %d from vendor ops does not exist.", __FUNCTION__, tagType);
            return BAD_VALUE;
        }
        desc->mTagToTypeMap.add(tag, tagType);
    }

    desc->mSections = sections;

    for (size_t i = 0; i < static_cast<size_t>(tagCount); ++i) {
        uint32_t tag = tagArray[i];
        const String8& sectionString = tagToSectionMap.valueFor(tag);

        // Set up tag to section index map
        ssize_t index = sections.indexOf(sectionString);
        LOG_ALWAYS_FATAL_IF(index < 0, "index %zd must be non-negative", index);
        desc->mTagToSectionMap.add(tag, static_cast<uint32_t>(index));

        // Set up reverse mapping
        ssize_t reverseIndex = -1;
        if ((reverseIndex = desc->mReverseMapping.indexOfKey(sectionString)) < 0) {
            KeyedVector<String8, uint32_t>* nameMapper = new KeyedVector<String8, uint32_t>();
            reverseIndex = desc->mReverseMapping.add(sectionString, nameMapper);
        }
        desc->mReverseMapping[reverseIndex]->add(desc->mTagToNameMap.valueFor(tag), tag);
    }

    descriptor = desc;
    return OK;
}

status_t VendorTagDescriptor::setAsGlobalVendorTagDescriptor(const sp<VendorTagDescriptor>& desc) {
    status_t res = OK;
    Mutex::Autolock al(sLock);
    sGlobalVendorTagDescriptor = desc;

    vendor_tag_ops_t* opsPtr = NULL;
    if (desc != NULL) {
        opsPtr = &(desc->mVendorOps);
        opsPtr->get_tag_count = vendor_tag_descriptor_get_tag_count;
        opsPtr->get_all_tags = vendor_tag_descriptor_get_all_tags;
        opsPtr->get_section_name = vendor_tag_descriptor_get_section_name;
        opsPtr->get_tag_name = vendor_tag_descriptor_get_tag_name;
        opsPtr->get_tag_type = vendor_tag_descriptor_get_tag_type;
    }
    if((res = set_camera_metadata_vendor_ops(opsPtr)) != OK) {
        ALOGE("%s: Could not set vendor tag descriptor, received error %s (%d)."
                , __FUNCTION__, strerror(-res), res);
    }
    return res;
}

void VendorTagDescriptor::clearGlobalVendorTagDescriptor() {
    Mutex::Autolock al(sLock);
    set_camera_metadata_vendor_ops(NULL);
    sGlobalVendorTagDescriptor.clear();
}

sp<VendorTagDescriptor> VendorTagDescriptor::getGlobalVendorTagDescriptor() {
    Mutex::Autolock al(sLock);
    return sGlobalVendorTagDescriptor;
}

status_t VendorTagDescriptorCache::setAsGlobalVendorTagCache(
        const sp<VendorTagDescriptorCache>& cache) {
    status_t res = OK;
    Mutex::Autolock al(sLock);
    sGlobalVendorTagDescriptorCache = cache;

    struct vendor_tag_cache_ops* opsPtr = NULL;
    if (cache != NULL) {
        opsPtr = &(cache->mVendorCacheOps);
        opsPtr->get_tag_count = vendor_tag_descriptor_cache_get_tag_count;
        opsPtr->get_all_tags = vendor_tag_descriptor_cache_get_all_tags;
        opsPtr->get_section_name = vendor_tag_descriptor_cache_get_section_name;
        opsPtr->get_tag_name = vendor_tag_descriptor_cache_get_tag_name;
        opsPtr->get_tag_type = vendor_tag_descriptor_cache_get_tag_type;
    }
    if((res = set_camera_metadata_vendor_cache_ops(opsPtr)) != OK) {
        ALOGE("%s: Could not set vendor tag cache, received error %s (%d)."
                , __FUNCTION__, strerror(-res), res);
    }
    return res;
}

void VendorTagDescriptorCache::clearGlobalVendorTagCache() {
    Mutex::Autolock al(sLock);
    set_camera_metadata_vendor_cache_ops(NULL);
    sGlobalVendorTagDescriptorCache.clear();
}

sp<VendorTagDescriptorCache> VendorTagDescriptorCache::getGlobalVendorTagCache() {
    Mutex::Autolock al(sLock);
    return sGlobalVendorTagDescriptorCache;
}

extern "C" {

int vendor_tag_descriptor_get_tag_count(const vendor_tag_ops_t* /*v*/) {
    Mutex::Autolock al(sLock);
    if (sGlobalVendorTagDescriptor == NULL) {
        ALOGE("%s: Vendor tag descriptor not initialized.", __FUNCTION__);
        return VENDOR_TAG_COUNT_ERR;
    }
    return sGlobalVendorTagDescriptor->getTagCount();
}

void vendor_tag_descriptor_get_all_tags(const vendor_tag_ops_t* /*v*/, uint32_t* tagArray) {
    Mutex::Autolock al(sLock);
    if (sGlobalVendorTagDescriptor == NULL) {
        ALOGE("%s: Vendor tag descriptor not initialized.", __FUNCTION__);
        return;
    }
    sGlobalVendorTagDescriptor->getTagArray(tagArray);
}

const char* vendor_tag_descriptor_get_section_name(const vendor_tag_ops_t* /*v*/, uint32_t tag) {
    Mutex::Autolock al(sLock);
    if (sGlobalVendorTagDescriptor == NULL) {
        ALOGE("%s: Vendor tag descriptor not initialized.", __FUNCTION__);
        return VENDOR_SECTION_NAME_ERR;
    }
    return sGlobalVendorTagDescriptor->getSectionName(tag);
}

const char* vendor_tag_descriptor_get_tag_name(const vendor_tag_ops_t* /*v*/, uint32_t tag) {
    Mutex::Autolock al(sLock);
    if (sGlobalVendorTagDescriptor == NULL) {
        ALOGE("%s: Vendor tag descriptor not initialized.", __FUNCTION__);
        return VENDOR_TAG_NAME_ERR;
    }
    return sGlobalVendorTagDescriptor->getTagName(tag);
}

int vendor_tag_descriptor_get_tag_type(const vendor_tag_ops_t* /*v*/, uint32_t tag) {
    Mutex::Autolock al(sLock);
    if (sGlobalVendorTagDescriptor == NULL) {
        ALOGE("%s: Vendor tag descriptor not initialized.", __FUNCTION__);
        return VENDOR_TAG_TYPE_ERR;
    }
    return sGlobalVendorTagDescriptor->getTagType(tag);
}

int vendor_tag_descriptor_cache_get_tag_count(metadata_vendor_id_t id) {
    Mutex::Autolock al(sLock);
    if (sGlobalVendorTagDescriptorCache == NULL) {
        ALOGE("%s: Vendor tag descriptor cache not initialized.", __FUNCTION__);
        return VENDOR_TAG_COUNT_ERR;
    }
    return sGlobalVendorTagDescriptorCache->getTagCount(id);
}

void vendor_tag_descriptor_cache_get_all_tags(uint32_t* tagArray,
        metadata_vendor_id_t id) {
    Mutex::Autolock al(sLock);
    if (sGlobalVendorTagDescriptorCache == NULL) {
        ALOGE("%s: Vendor tag descriptor cache not initialized.", __FUNCTION__);
    }
    sGlobalVendorTagDescriptorCache->getTagArray(tagArray, id);
}

const char* vendor_tag_descriptor_cache_get_section_name(uint32_t tag,
        metadata_vendor_id_t id) {
    Mutex::Autolock al(sLock);
    if (sGlobalVendorTagDescriptorCache == NULL) {
        ALOGE("%s: Vendor tag descriptor cache not initialized.", __FUNCTION__);
        return VENDOR_SECTION_NAME_ERR;
    }
    return sGlobalVendorTagDescriptorCache->getSectionName(tag, id);
}

const char* vendor_tag_descriptor_cache_get_tag_name(uint32_t tag,
        metadata_vendor_id_t id) {
    Mutex::Autolock al(sLock);
    if (sGlobalVendorTagDescriptorCache == NULL) {
        ALOGE("%s: Vendor tag descriptor cache not initialized.", __FUNCTION__);
        return VENDOR_TAG_NAME_ERR;
    }
    return sGlobalVendorTagDescriptorCache->getTagName(tag, id);
}

int vendor_tag_descriptor_cache_get_tag_type(uint32_t tag,
        metadata_vendor_id_t id) {
    Mutex::Autolock al(sLock);
    if (sGlobalVendorTagDescriptorCache == NULL) {
        ALOGE("%s: Vendor tag descriptor cache not initialized.", __FUNCTION__);
        return VENDOR_TAG_NAME_ERR;
    }
    return sGlobalVendorTagDescriptorCache->getTagType(tag, id);
}

} /* extern "C" */
} /* namespace android */
