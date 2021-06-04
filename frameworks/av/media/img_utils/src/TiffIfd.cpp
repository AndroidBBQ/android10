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

#define LOG_TAG "TiffIfd"

#include <img_utils/TagDefinitions.h>
#include <img_utils/TiffHelpers.h>
#include <img_utils/TiffIfd.h>
#include <img_utils/TiffWriter.h>

#include <utils/Log.h>

namespace android {
namespace img_utils {

TiffIfd::TiffIfd(uint32_t ifdId)
        : mNextIfd(), mIfdId(ifdId), mStripOffsetsInitialized(false) {}

TiffIfd::~TiffIfd() {}

status_t TiffIfd::addEntry(const sp<TiffEntry>& entry) {
    size_t size = mEntries.size();
    if (size >= MAX_IFD_ENTRIES) {
        ALOGW("%s: Failed to add entry for tag 0x%x to IFD %u, too many entries in IFD!",
                __FUNCTION__, entry->getTag(), mIfdId);
        return BAD_INDEX;
    }

    if (mEntries.add(entry) < 0) {
        ALOGW("%s: Failed to add entry for tag 0x%x to ifd %u.", __FUNCTION__, entry->getTag(),
                mIfdId);
        return BAD_INDEX;
    }
    return OK;
}

sp<TiffEntry> TiffIfd::getEntry(uint16_t tag) const {
    ssize_t index = mEntries.indexOfTag(tag);
    if (index < 0) {
        ALOGW("%s: No entry for tag 0x%x in ifd %u.", __FUNCTION__, tag, mIfdId);
        return NULL;
    }
    return mEntries[index];
}

void TiffIfd::removeEntry(uint16_t tag) {
    ssize_t index = mEntries.indexOfTag(tag);
    if (index >= 0) {
        mEntries.removeAt(index);
    }
}


void TiffIfd::setNextIfd(const sp<TiffIfd>& ifd) {
    mNextIfd = ifd;
}

sp<TiffIfd> TiffIfd::getNextIfd() const {
    return mNextIfd;
}

uint32_t TiffIfd::checkAndGetOffset(uint32_t offset) const {
    size_t size = mEntries.size();

    if (size > MAX_IFD_ENTRIES) {
        ALOGW("%s: Could not calculate IFD offsets, IFD %u contains too many entries.",
                __FUNCTION__, mIfdId);
        return BAD_OFFSET;
    }

    if (size <= 0) {
        ALOGW("%s: Could not calculate IFD offsets, IFD %u contains no entries.", __FUNCTION__,
                mIfdId);
        return BAD_OFFSET;
    }

    if (offset == BAD_OFFSET) {
        ALOGW("%s: Could not calculate IFD offsets, IFD %u had a bad initial offset.",
                __FUNCTION__, mIfdId);
        return BAD_OFFSET;
    }

    uint32_t ifdSize = calculateIfdSize(size);
    WORD_ALIGN(ifdSize);
    return offset + ifdSize;
}

status_t TiffIfd::writeData(uint32_t offset, /*out*/EndianOutput* out) const {
    assert((offset % TIFF_WORD_SIZE) == 0);
    status_t ret = OK;

    ALOGV("%s: IFD %u written to offset %u", __FUNCTION__, mIfdId, offset );
    uint32_t valueOffset = checkAndGetOffset(offset);
    if (valueOffset == 0) {
        return BAD_VALUE;
    }

    size_t size = mEntries.size();

    // Writer IFD header (2 bytes, number of entries).
    uint16_t header = static_cast<uint16_t>(size);
    BAIL_ON_FAIL(out->write(&header, 0, 1), ret);

    // Write tag entries
    for (size_t i = 0; i < size; ++i) {
        BAIL_ON_FAIL(mEntries[i]->writeTagInfo(valueOffset, out), ret);
        valueOffset += mEntries[i]->getSize();
    }

    // Writer IFD footer (4 bytes, offset to next IFD).
    uint32_t footer = (mNextIfd != NULL) ? offset + getSize() : 0;
    BAIL_ON_FAIL(out->write(&footer, 0, 1), ret);

    assert(out->getCurrentOffset() == offset + calculateIfdSize(size));

    // Write zeroes till word aligned
    ZERO_TILL_WORD(out, calculateIfdSize(size), ret);

    // Write values for each tag entry
    for (size_t i = 0; i < size; ++i) {
        size_t last = out->getCurrentOffset();
        // Only write values that are too large to fit in the 12-byte TIFF entry
        if (mEntries[i]->getSize() > OFFSET_SIZE) {
            BAIL_ON_FAIL(mEntries[i]->writeData(out->getCurrentOffset(), out), ret);
        }
        size_t next = out->getCurrentOffset();
        size_t diff = (next - last);
        size_t actual = mEntries[i]->getSize();
        if (diff != actual) {
            ALOGW("Sizes do not match for tag %x. Expected %zu, received %zu",
                    mEntries[i]->getTag(), actual, diff);
        }
    }

    assert(out->getCurrentOffset() == offset + getSize());

    return ret;
}

size_t TiffIfd::getSize() const {
    size_t size = mEntries.size();
    uint32_t total = calculateIfdSize(size);
    WORD_ALIGN(total);
    for (size_t i = 0; i < size; ++i) {
        total += mEntries[i]->getSize();
    }
    return total;
}

uint32_t TiffIfd::getId() const {
    return mIfdId;
}

uint32_t TiffIfd::getComparableValue() const {
    return mIfdId;
}

status_t TiffIfd::validateAndSetStripTags() {
    sp<TiffEntry> widthEntry = getEntry(TAG_IMAGEWIDTH);
    if (widthEntry == NULL) {
        ALOGE("%s: IFD %u doesn't have a ImageWidth tag set", __FUNCTION__, mIfdId);
        return BAD_VALUE;
    }

    sp<TiffEntry> heightEntry = getEntry(TAG_IMAGELENGTH);
    if (heightEntry == NULL) {
        ALOGE("%s: IFD %u doesn't have a ImageLength tag set", __FUNCTION__, mIfdId);
        return BAD_VALUE;
    }

    sp<TiffEntry> samplesEntry = getEntry(TAG_SAMPLESPERPIXEL);
    if (samplesEntry == NULL) {
        ALOGE("%s: IFD %u doesn't have a SamplesPerPixel tag set", __FUNCTION__, mIfdId);
        return BAD_VALUE;
    }

    sp<TiffEntry> bitsEntry = getEntry(TAG_BITSPERSAMPLE);
    if (bitsEntry == NULL) {
        ALOGE("%s: IFD %u doesn't have a BitsPerSample tag set", __FUNCTION__, mIfdId);
        return BAD_VALUE;
    }

    uint32_t width = *(widthEntry->getData<uint32_t>());
    uint32_t height = *(heightEntry->getData<uint32_t>());
    uint16_t bitsPerSample = *(bitsEntry->getData<uint16_t>());
    uint16_t samplesPerPixel = *(samplesEntry->getData<uint16_t>());

    if ((bitsPerSample % 8) != 0) {
        ALOGE("%s: BitsPerSample %d in IFD %u is not byte-aligned.", __FUNCTION__,
                bitsPerSample, mIfdId);
        return BAD_VALUE;
    }

    uint32_t bytesPerSample = bitsPerSample / 8;

    // Choose strip size as close to 8kb as possible without splitting rows.
    // If the row length is >8kb, each strip will only contain a single row.
    const uint32_t rowLengthBytes = bytesPerSample * samplesPerPixel * width;
    const uint32_t idealChunkSize = (1 << 13); // 8kb
    uint32_t rowsPerChunk = idealChunkSize / rowLengthBytes;
    rowsPerChunk = (rowsPerChunk == 0) ? 1 : rowsPerChunk;
    const uint32_t actualChunkSize = rowLengthBytes * rowsPerChunk;

    const uint32_t lastChunkRows = height % rowsPerChunk;
    const uint32_t lastChunkSize = lastChunkRows * rowLengthBytes;

    if (actualChunkSize > /*max strip size for TIFF/EP*/65536) {
        ALOGE("%s: Strip length too long.", __FUNCTION__);
        return BAD_VALUE;
    }

    size_t numStrips = height / rowsPerChunk;

    // Add another strip for the incomplete chunk.
    if (lastChunkRows > 0) {
        numStrips += 1;
    }

    // Put each row in it's own strip
    uint32_t rowsPerStripVal = rowsPerChunk;
    sp<TiffEntry> rowsPerStrip = TiffWriter::uncheckedBuildEntry(TAG_ROWSPERSTRIP, LONG, 1,
            UNDEFINED_ENDIAN, &rowsPerStripVal);

    if (rowsPerStrip == NULL) {
        ALOGE("%s: Could not build entry for RowsPerStrip tag.", __FUNCTION__);
        return BAD_VALUE;
    }

    Vector<uint32_t> byteCounts;

    for (size_t i = 0; i < numStrips; ++i) {
        if (lastChunkRows > 0 && i == (numStrips - 1)) {
            byteCounts.add(lastChunkSize);
        } else {
            byteCounts.add(actualChunkSize);
        }
    }

    // Set byte counts for each strip
    sp<TiffEntry> stripByteCounts = TiffWriter::uncheckedBuildEntry(TAG_STRIPBYTECOUNTS, LONG,
            static_cast<uint32_t>(numStrips), UNDEFINED_ENDIAN, byteCounts.array());

    if (stripByteCounts == NULL) {
        ALOGE("%s: Could not build entry for StripByteCounts tag.", __FUNCTION__);
        return BAD_VALUE;
    }

    Vector<uint32_t> stripOffsetsVector;
    stripOffsetsVector.resize(numStrips);

    // Set uninitialized offsets
    sp<TiffEntry> stripOffsets = TiffWriter::uncheckedBuildEntry(TAG_STRIPOFFSETS, LONG,
            static_cast<uint32_t>(numStrips), UNDEFINED_ENDIAN, stripOffsetsVector.array());

    if (stripOffsets == NULL) {
        ALOGE("%s: Could not build entry for StripOffsets tag.", __FUNCTION__);
        return BAD_VALUE;
    }

    if(addEntry(stripByteCounts) != OK) {
        ALOGE("%s: Could not add entry for StripByteCounts to IFD %u", __FUNCTION__, mIfdId);
        return BAD_VALUE;
    }

    if(addEntry(rowsPerStrip) != OK) {
        ALOGE("%s: Could not add entry for StripByteCounts to IFD %u", __FUNCTION__, mIfdId);
        return BAD_VALUE;
    }

    if(addEntry(stripOffsets) != OK) {
        ALOGE("%s: Could not add entry for StripByteCounts to IFD %u", __FUNCTION__, mIfdId);
        return BAD_VALUE;
    }

    mStripOffsetsInitialized = true;
    return OK;
}

bool TiffIfd::uninitializedOffsets() const {
    return mStripOffsetsInitialized;
}

status_t TiffIfd::setStripOffset(uint32_t offset) {

    // Get old offsets and bytecounts
    sp<TiffEntry> oldOffsets = getEntry(TAG_STRIPOFFSETS);
    if (oldOffsets == NULL) {
        ALOGE("%s: IFD %u does not contain StripOffsets entry.", __FUNCTION__, mIfdId);
        return BAD_VALUE;
    }

    sp<TiffEntry> stripByteCounts = getEntry(TAG_STRIPBYTECOUNTS);
    if (stripByteCounts == NULL) {
        ALOGE("%s: IFD %u does not contain StripByteCounts entry.", __FUNCTION__, mIfdId);
        return BAD_VALUE;
    }

    uint32_t offsetsCount = oldOffsets->getCount();
    uint32_t byteCount = stripByteCounts->getCount();
    if (offsetsCount != byteCount) {
        ALOGE("%s: StripOffsets count (%u) doesn't match StripByteCounts count (%u) in IFD %u",
            __FUNCTION__, offsetsCount, byteCount, mIfdId);
        return BAD_VALUE;
    }

    const uint32_t* stripByteCountsArray = stripByteCounts->getData<uint32_t>();

    size_t numStrips = offsetsCount;

    Vector<uint32_t> stripOffsets;

    // Calculate updated byte offsets
    for (size_t i = 0; i < numStrips; ++i) {
        stripOffsets.add(offset);
        offset += stripByteCountsArray[i];
    }

    sp<TiffEntry> newOffsets = TiffWriter::uncheckedBuildEntry(TAG_STRIPOFFSETS, LONG,
            static_cast<uint32_t>(numStrips), UNDEFINED_ENDIAN, stripOffsets.array());

    if (newOffsets == NULL) {
        ALOGE("%s: Coult not build updated offsets entry in IFD %u", __FUNCTION__, mIfdId);
        return BAD_VALUE;
    }

    if (addEntry(newOffsets) != OK) {
        ALOGE("%s: Failed to add updated offsets entry in IFD %u", __FUNCTION__, mIfdId);
        return BAD_VALUE;
    }
    return OK;
}

uint32_t TiffIfd::getStripSize() const {
    sp<TiffEntry> stripByteCounts = getEntry(TAG_STRIPBYTECOUNTS);
    if (stripByteCounts == NULL) {
        ALOGE("%s: IFD %u does not contain StripByteCounts entry.", __FUNCTION__, mIfdId);
        return BAD_VALUE;
    }

    uint32_t count = stripByteCounts->getCount();
    const uint32_t* byteCounts = stripByteCounts->getData<uint32_t>();

    uint32_t total = 0;
    for (size_t i = 0; i < static_cast<size_t>(count); ++i) {
        total += byteCounts[i];
    }
    return total;
}

String8 TiffIfd::toString() const {
    size_t s = mEntries.size();
    String8 output;
    output.appendFormat("[ifd: %x, num_entries: %zu, entries:\n", getId(), s);
    for(size_t i = 0; i < mEntries.size(); ++i) {
        output.append("\t");
        output.append(mEntries[i]->toString());
        output.append("\n");
    }
    output.append(", next_ifd: %x]", ((mNextIfd != NULL) ? mNextIfd->getId() : 0));
    return output;
}

void TiffIfd::log() const {
    size_t s = mEntries.size();
    ALOGI("[ifd: %x, num_entries: %zu, entries:\n", getId(), s);
    for(size_t i = 0; i < s; ++i) {
        ALOGI("\t%s", mEntries[i]->toString().string());
    }
    ALOGI(", next_ifd: %x]", ((mNextIfd != NULL) ? mNextIfd->getId() : 0));
}

} /*namespace img_utils*/
} /*namespace android*/
