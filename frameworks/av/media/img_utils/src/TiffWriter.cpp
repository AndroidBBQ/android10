/*
 * Copyright 2014 The Android Open Source Project
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

#define LOG_TAG "TiffWriter"

#include <img_utils/TiffHelpers.h>
#include <img_utils/TiffWriter.h>
#include <img_utils/TagDefinitions.h>

#include <assert.h>

namespace android {
namespace img_utils {

KeyedVector<uint16_t, const TagDefinition_t*> TiffWriter::buildTagMap(
            const TagDefinition_t* definitions, size_t length) {
    KeyedVector<uint16_t, const TagDefinition_t*> map;
    for(size_t i = 0; i < length; ++i) {
        map.add(definitions[i].tagId, definitions + i);
    }
    return map;
}

#define COMPARE(op) \
bool Orderable::operator op (const Orderable& orderable) const { \
    return getComparableValue() op orderable.getComparableValue(); \
}

#define ARRAY_SIZE(array) \
    (sizeof(array) / sizeof((array)[0]))

KeyedVector<uint16_t, const TagDefinition_t*> TiffWriter::sTagMaps[] = {
    buildTagMap(TIFF_EP_TAG_DEFINITIONS, ARRAY_SIZE(TIFF_EP_TAG_DEFINITIONS)),
    buildTagMap(DNG_TAG_DEFINITIONS, ARRAY_SIZE(DNG_TAG_DEFINITIONS)),
    buildTagMap(EXIF_2_3_TAG_DEFINITIONS, ARRAY_SIZE(EXIF_2_3_TAG_DEFINITIONS)),
    buildTagMap(TIFF_6_TAG_DEFINITIONS, ARRAY_SIZE(TIFF_6_TAG_DEFINITIONS))
};

TiffWriter::TiffWriter() : mTagMaps(sTagMaps), mNumTagMaps(DEFAULT_NUM_TAG_MAPS) {}

TiffWriter::TiffWriter(KeyedVector<uint16_t, const TagDefinition_t*>* enabledDefinitions,
        size_t length) : mTagMaps(enabledDefinitions), mNumTagMaps(length) {}

TiffWriter::~TiffWriter() {}

status_t TiffWriter::write(Output* out, StripSource** sources, size_t sourcesCount,
        Endianness end) {
    status_t ret = OK;
    EndianOutput endOut(out, end);

    if (mIfd == NULL) {
        ALOGE("%s: Tiff header is empty.", __FUNCTION__);
        return BAD_VALUE;
    }

    uint32_t totalSize = getTotalSize();

    KeyedVector<uint32_t, uint32_t> offsetVector;

    for (size_t i = 0; i < mNamedIfds.size(); ++i) {
        if (mNamedIfds[i]->uninitializedOffsets()) {
            uint32_t stripSize = mNamedIfds[i]->getStripSize();
            if (mNamedIfds[i]->setStripOffset(totalSize) != OK) {
                ALOGE("%s: Could not set strip offsets.", __FUNCTION__);
                return BAD_VALUE;
            }
            totalSize += stripSize;
            WORD_ALIGN(totalSize);
            offsetVector.add(mNamedIfds.keyAt(i), totalSize);
        }
    }

    size_t offVecSize = offsetVector.size();
    if (offVecSize != sourcesCount) {
        ALOGE("%s: Mismatch between number of IFDs with uninitialized strips (%zu) and"
                " sources (%zu).", __FUNCTION__, offVecSize, sourcesCount);
        return BAD_VALUE;
    }

    BAIL_ON_FAIL(writeFileHeader(endOut), ret);

    uint32_t offset = FILE_HEADER_SIZE;
    sp<TiffIfd> ifd = mIfd;
    while(ifd != NULL) {
        BAIL_ON_FAIL(ifd->writeData(offset, &endOut), ret);
        offset += ifd->getSize();
        ifd = ifd->getNextIfd();
    }

    if (LOG_NDEBUG == 0) {
        log();
    }

    for (size_t i = 0; i < offVecSize; ++i) {
        uint32_t ifdKey = offsetVector.keyAt(i);
        uint32_t sizeToWrite = mNamedIfds[ifdKey]->getStripSize();
        bool found = false;
        for (size_t j = 0; j < sourcesCount; ++j) {
            if (sources[j]->getIfd() == ifdKey) {
                if ((ret = sources[i]->writeToStream(endOut, sizeToWrite)) != OK) {
                    ALOGE("%s: Could not write to stream, received %d.", __FUNCTION__, ret);
                    return ret;
                }
                ZERO_TILL_WORD(&endOut, sizeToWrite, ret);
                found = true;
                break;
            }
        }
        if (!found) {
            ALOGE("%s: No stream for byte strips for IFD %u", __FUNCTION__, ifdKey);
            return BAD_VALUE;
        }
        assert(offsetVector[i] == endOut.getCurrentOffset());
    }

    return ret;
}

status_t TiffWriter::write(Output* out, Endianness end) {
    status_t ret = OK;
    EndianOutput endOut(out, end);

    if (mIfd == NULL) {
        ALOGE("%s: Tiff header is empty.", __FUNCTION__);
        return BAD_VALUE;
    }
    BAIL_ON_FAIL(writeFileHeader(endOut), ret);

    uint32_t offset = FILE_HEADER_SIZE;
    sp<TiffIfd> ifd = mIfd;
    while(ifd != NULL) {
        BAIL_ON_FAIL(ifd->writeData(offset, &endOut), ret);
        offset += ifd->getSize();
        ifd = ifd->getNextIfd();
    }
    return ret;
}


const TagDefinition_t* TiffWriter::lookupDefinition(uint16_t tag) const {
    const TagDefinition_t* definition = NULL;
    for (size_t i = 0; i < mNumTagMaps; ++i) {
        ssize_t index = mTagMaps[i].indexOfKey(tag);
        if (index >= 0) {
            definition = mTagMaps[i][index];
            break;
        }
    }

    if (definition == NULL) {
        ALOGE("%s: No definition exists for tag with id %x.", __FUNCTION__, tag);
    }
    return definition;
}

sp<TiffEntry> TiffWriter::getEntry(uint16_t tag, uint32_t ifd) const {
    ssize_t index = mNamedIfds.indexOfKey(ifd);
    if (index < 0) {
        ALOGE("%s: No IFD %d set for this writer.", __FUNCTION__, ifd);
        return NULL;
    }
    return mNamedIfds[index]->getEntry(tag);
}

void TiffWriter::removeEntry(uint16_t tag, uint32_t ifd) {
    ssize_t index = mNamedIfds.indexOfKey(ifd);
    if (index >= 0) {
        mNamedIfds[index]->removeEntry(tag);
    }
}

status_t TiffWriter::addEntry(const sp<TiffEntry>& entry, uint32_t ifd) {
    uint16_t tag = entry->getTag();

    const TagDefinition_t* definition = lookupDefinition(tag);

    if (definition == NULL) {
        ALOGE("%s: No definition exists for tag 0x%x.", __FUNCTION__, tag);
        return BAD_INDEX;
    }

    ssize_t index = mNamedIfds.indexOfKey(ifd);

    // Add a new IFD if necessary
    if (index < 0) {
        ALOGE("%s: No IFD %u exists.", __FUNCTION__, ifd);
        return NAME_NOT_FOUND;
    }

    sp<TiffIfd> selectedIfd  = mNamedIfds[index];
    return selectedIfd->addEntry(entry);
}

status_t TiffWriter::addStrip(uint32_t ifd) {
    ssize_t index = mNamedIfds.indexOfKey(ifd);
    if (index < 0) {
        ALOGE("%s: Ifd %u doesn't exist, cannot add strip entries.", __FUNCTION__, ifd);
        return BAD_VALUE;
    }
    sp<TiffIfd> selected = mNamedIfds[index];
    return selected->validateAndSetStripTags();
}

status_t TiffWriter::addIfd(uint32_t ifd) {
    ssize_t index = mNamedIfds.indexOfKey(ifd);
    if (index >= 0) {
        ALOGE("%s: Ifd with ID 0x%x already exists.", __FUNCTION__, ifd);
        return BAD_VALUE;
    }

    sp<TiffIfd> newIfd = new TiffIfd(ifd);
    if (mIfd == NULL) {
        mIfd = newIfd;
    } else {
        sp<TiffIfd> last = findLastIfd();
        last->setNextIfd(newIfd);
    }

    if(mNamedIfds.add(ifd, newIfd) < 0) {
        ALOGE("%s: Failed to add new IFD 0x%x.", __FUNCTION__, ifd);
        return BAD_VALUE;
    }

    return OK;
}

status_t TiffWriter::addSubIfd(uint32_t parentIfd, uint32_t ifd, SubIfdType type) {
    ssize_t index = mNamedIfds.indexOfKey(ifd);
    if (index >= 0) {
        ALOGE("%s: Ifd with ID 0x%x already exists.", __FUNCTION__, ifd);
        return BAD_VALUE;
    }

    ssize_t parentIndex = mNamedIfds.indexOfKey(parentIfd);
    if (parentIndex < 0) {
        ALOGE("%s: Parent IFD with ID 0x%x does not exist.", __FUNCTION__, parentIfd);
        return BAD_VALUE;
    }

    sp<TiffIfd> parent = mNamedIfds[parentIndex];
    sp<TiffIfd> newIfd = new TiffIfd(ifd);

    uint16_t subIfdTag;
    if (type == SUBIFD) {
        subIfdTag = TAG_SUBIFDS;
    } else if (type == GPSINFO) {
        subIfdTag = TAG_GPSINFO;
    } else {
        ALOGE("%s: Unknown SubIFD type %d.", __FUNCTION__, type);
        return BAD_VALUE;
    }

    sp<TiffEntry> subIfds = parent->getEntry(subIfdTag);
    if (subIfds == NULL) {
        if (buildEntry(subIfdTag, 1, &newIfd, &subIfds) < 0) {
            ALOGE("%s: Failed to build SubIfd entry in IFD 0x%x.", __FUNCTION__, parentIfd);
            return BAD_VALUE;
        }
    } else {
        if (type == GPSINFO) {
            ALOGE("%s: Cannot add GPSInfo SubIFD to IFD %u, one already exists.", __FUNCTION__,
                    ifd);
            return BAD_VALUE;
        }

        Vector<sp<TiffIfd> > subIfdList;
        const sp<TiffIfd>* oldIfdArray = subIfds->getData<sp<TiffIfd> >();
        if (subIfdList.appendArray(oldIfdArray, subIfds->getCount()) < 0) {
            ALOGE("%s: Failed to build SubIfd entry in IFD 0x%x.", __FUNCTION__, parentIfd);
            return BAD_VALUE;
        }

        if (subIfdList.add(newIfd) < 0) {
            ALOGE("%s: Failed to build SubIfd entry in IFD 0x%x.", __FUNCTION__, parentIfd);
            return BAD_VALUE;
        }

        uint32_t count = subIfdList.size();
        if (buildEntry(subIfdTag, count, subIfdList.array(), &subIfds) < 0) {
            ALOGE("%s: Failed to build SubIfd entry in IFD 0x%x.", __FUNCTION__, parentIfd);
            return BAD_VALUE;
        }
    }

    if (parent->addEntry(subIfds) < 0) {
        ALOGE("%s: Failed to add SubIfd entry in IFD 0x%x.", __FUNCTION__, parentIfd);
        return BAD_VALUE;
    }

    if(mNamedIfds.add(ifd, newIfd) < 0) {
        ALOGE("%s: Failed to add new IFD 0x%x.", __FUNCTION__, ifd);
        return BAD_VALUE;
    }

    return OK;
}

TagType TiffWriter::getDefaultType(uint16_t tag) const {
    const TagDefinition_t* definition = lookupDefinition(tag);
    if (definition == NULL) {
        ALOGE("%s: Could not find definition for tag %x", __FUNCTION__, tag);
        return UNKNOWN_TAGTYPE;
    }
    return definition->defaultType;
}

uint32_t TiffWriter::getDefaultCount(uint16_t tag) const {
    const TagDefinition_t* definition = lookupDefinition(tag);
    if (definition == NULL) {
        ALOGE("%s: Could not find definition for tag %x", __FUNCTION__, tag);
        return 0;
    }
    return definition->fixedCount;
}

bool TiffWriter::hasIfd(uint32_t ifd) const {
    ssize_t index = mNamedIfds.indexOfKey(ifd);
    return index >= 0;
}

bool TiffWriter::checkIfDefined(uint16_t tag) const {
    return lookupDefinition(tag) != NULL;
}

const char* TiffWriter::getTagName(uint16_t tag) const {
    const TagDefinition_t* definition = lookupDefinition(tag);
    if (definition == NULL) {
        return NULL;
    }
    return definition->tagName;
}

sp<TiffIfd> TiffWriter::findLastIfd() {
    sp<TiffIfd> ifd = mIfd;
    while(ifd != NULL) {
        sp<TiffIfd> nextIfd = ifd->getNextIfd();
        if (nextIfd == NULL) {
            break;
        }
        ifd = std::move(nextIfd);
    }
    return ifd;
}

status_t TiffWriter::writeFileHeader(EndianOutput& out) {
    status_t ret = OK;
    uint16_t endMarker = (out.getEndianness() == BIG) ? BIG_ENDIAN_MARKER : LITTLE_ENDIAN_MARKER;
    BAIL_ON_FAIL(out.write(&endMarker, 0, 1), ret);

    uint16_t tiffMarker = TIFF_FILE_MARKER;
    BAIL_ON_FAIL(out.write(&tiffMarker, 0, 1), ret);

    uint32_t offsetMarker = FILE_HEADER_SIZE;
    BAIL_ON_FAIL(out.write(&offsetMarker, 0, 1), ret);
    return ret;
}

uint32_t TiffWriter::getTotalSize() const {
    uint32_t totalSize = FILE_HEADER_SIZE;
    sp<TiffIfd> ifd = mIfd;
    while(ifd != NULL) {
        totalSize += ifd->getSize();
        ifd = ifd->getNextIfd();
    }
    return totalSize;
}

void TiffWriter::log() const {
    ALOGI("%s: TiffWriter:", __FUNCTION__);
    size_t length = mNamedIfds.size();
    for (size_t i = 0; i < length; ++i) {
        mNamedIfds[i]->log();
    }
}

} /*namespace img_utils*/
} /*namespace android*/
