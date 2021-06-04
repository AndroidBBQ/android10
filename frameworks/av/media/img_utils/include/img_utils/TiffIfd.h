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

#ifndef IMG_UTILS_TIFF_IFD_H
#define IMG_UTILS_TIFF_IFD_H

#include <img_utils/TiffWritable.h>
#include <img_utils/TiffEntry.h>
#include <img_utils/Output.h>
#include <img_utils/SortedEntryVector.h>

#include <cutils/compiler.h>
#include <utils/Errors.h>
#include <utils/String8.h>
#include <utils/SortedVector.h>
#include <utils/StrongPointer.h>
#include <stdint.h>

namespace android {
namespace img_utils {

/**
 * This class holds a single TIFF Image File Directory (IFD) structure.
 *
 * This maps to the TIFF IFD structure that is logically composed of:
 * - A 2-byte field listing the number of entries.
 * - A list of 12-byte TIFF entries.
 * - A 4-byte offset to the next IFD.
 */
class ANDROID_API TiffIfd : public TiffWritable {
    public:
        explicit TiffIfd(uint32_t ifdId);
        virtual ~TiffIfd();

        /**
         * Add a TiffEntry to this IFD or replace an existing entry with the
         * same tag ID.  No validation is done.
         *
         * Returns OK on success, or a negative error code on failure.
         */
        virtual status_t addEntry(const sp<TiffEntry>& entry);

        /**
         * Set the pointer to the next IFD.  This is used to create a linked
         * list of IFDs as defined by the TIFF 6.0 spec., and is not included
         * when calculating the size of IFD and entries for the getSize()
         * method (unlike SubIFDs).
         */
        virtual void setNextIfd(const sp<TiffIfd>& ifd);

        /**
         * Get the pointer to the next IFD, or NULL if none exists.
         */
        virtual sp<TiffIfd> getNextIfd() const;

        /**
         * Write the IFD data.  This includes the IFD header, entries, footer,
         * and the corresponding values for each entry (recursively including
         * sub-IFDs).  The written amount should end on a word boundary, and
         * the given offset should be word aligned.
         *
         * Returns OK on success, or a negative error code on failure.
         */
        virtual status_t writeData(uint32_t offset, /*out*/EndianOutput* out) const;

        /**
         * Get the size of the IFD. This includes the IFD header, entries, footer,
         * and the corresponding values for each entry (recursively including
         * any sub-IFDs).
         */
        virtual size_t getSize() const;

        /**
         * Get the id of this IFD.
         */
        virtual uint32_t getId() const;

        /**
         * Get an entry with the given tag ID.
         *
         * Returns a strong pointer to the entry if it exists, or an empty strong
         * pointer.
         */
        virtual sp<TiffEntry> getEntry(uint16_t tag) const;

        /**
         * Remove the entry with the given tag ID if it exists.
         */
        virtual void removeEntry(uint16_t tag);

        /**
         * Convenience method to validate and set strip-related image tags.
         *
         * This sets all strip related tags, but leaves offset values unitialized.
         * setStripOffsets must be called with the desired offset before writing.
         * The strip tag values are calculated from the existing tags for image
         * dimensions and pixel type set in the IFD.
         *
         * Does not handle planar image configurations (PlanarConfiguration != 1).
         *
         * Returns OK on success, or a negative error code.
         */
        virtual status_t validateAndSetStripTags();

        /**
         * Returns true if validateAndSetStripTags has been called, but not setStripOffsets.
         */
        virtual bool uninitializedOffsets() const;

        /**
         * Convenience method to set beginning offset for strips.
         *
         * Call this to update the strip offsets before calling writeData.
         *
         * Returns OK on success, or a negative error code.
         */
        virtual status_t setStripOffset(uint32_t offset);

        /**
         * Get the total size of the strips in bytes.
         *
         * This sums the byte count at each strip offset, and returns
         * the total count of bytes stored in strips for this IFD.
         */
        virtual uint32_t getStripSize() const;

        /**
         * Get a formatted string representing this IFD.
         */
        virtual String8 toString() const;

        /**
         * Print a formatted string representing this IFD to logcat.
         */
        void log() const;

        /**
         * Get value used to determine sort order.
         */
        virtual uint32_t getComparableValue() const;

    protected:
        virtual uint32_t checkAndGetOffset(uint32_t offset) const;
        SortedEntryVector mEntries;
        sp<TiffIfd> mNextIfd;
        uint32_t mIfdId;
        bool mStripOffsetsInitialized;
};

} /*namespace img_utils*/
} /*namespace android*/

#endif /*IMG_UTILS_TIFF_IFD_H*/
