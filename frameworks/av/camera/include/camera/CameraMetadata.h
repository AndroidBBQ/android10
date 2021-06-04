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

#ifndef ANDROID_CLIENT_CAMERA2_CAMERAMETADATA_CPP
#define ANDROID_CLIENT_CAMERA2_CAMERAMETADATA_CPP

#include "system/camera_metadata.h"

#include <utils/String8.h>
#include <utils/Vector.h>
#include <binder/Parcelable.h>
#include <camera/VendorTagDescriptor.h>

namespace android {

class VendorTagDescriptor;

/**
 * A convenience wrapper around the C-based camera_metadata_t library.
 */
class CameraMetadata: public Parcelable {
  public:
    /** Creates an empty object; best used when expecting to acquire contents
     * from elsewhere */
    CameraMetadata();
    /** Creates an object with space for entryCapacity entries, with
     * dataCapacity extra storage */
    CameraMetadata(size_t entryCapacity, size_t dataCapacity = 10);

    ~CameraMetadata();

    /** Takes ownership of passed-in buffer */
    CameraMetadata(camera_metadata_t *buffer);
    /** Clones the metadata */
    CameraMetadata(const CameraMetadata &other);

    /**
     * Assignment clones metadata buffer.
     */
    CameraMetadata &operator=(const CameraMetadata &other);
    CameraMetadata &operator=(const camera_metadata_t *buffer);

    /**
     * Get reference to the underlying metadata buffer. Ownership remains with
     * the CameraMetadata object, but non-const CameraMetadata methods will not
     * work until unlock() is called. Note that the lock has nothing to do with
     * thread-safety, it simply prevents the camera_metadata_t pointer returned
     * here from being accidentally invalidated by CameraMetadata operations.
     */
    const camera_metadata_t* getAndLock() const;

    /**
     * Unlock the CameraMetadata for use again. After this unlock, the pointer
     * given from getAndLock() may no longer be used. The pointer passed out
     * from getAndLock must be provided to guarantee that the right object is
     * being unlocked.
     */
    status_t unlock(const camera_metadata_t *buffer) const;

    /**
     * Release a raw metadata buffer to the caller. After this call,
     * CameraMetadata no longer references the buffer, and the caller takes
     * responsibility for freeing the raw metadata buffer (using
     * free_camera_metadata()), or for handing it to another CameraMetadata
     * instance.
     */
    camera_metadata_t* release();

    /**
     * Clear the metadata buffer and free all storage used by it
     */
    void clear();

    /**
     * Acquire a raw metadata buffer from the caller. After this call,
     * the caller no longer owns the raw buffer, and must not free or manipulate it.
     * If CameraMetadata already contains metadata, it is freed.
     */
    void acquire(camera_metadata_t* buffer);

    /**
     * Acquires raw buffer from other CameraMetadata object. After the call, the argument
     * object no longer has any metadata.
     */
    void acquire(CameraMetadata &other);

    /**
     * Append metadata from another CameraMetadata object.
     */
    status_t append(const CameraMetadata &other);

    /**
     * Append metadata from a raw camera_metadata buffer
     */
    status_t append(const camera_metadata* other);

    /**
     * Number of metadata entries.
     */
    size_t entryCount() const;

    /**
     * Is the buffer empty (no entires)
     */
    bool isEmpty() const;

    /**
     * Sort metadata buffer for faster find
     */
    status_t sort();

    /**
     * Update metadata entry. Will create entry if it doesn't exist already, and
     * will reallocate the buffer if insufficient space exists. Overloaded for
     * the various types of valid data.
     */
    status_t update(uint32_t tag,
            const uint8_t *data, size_t data_count);
    status_t update(uint32_t tag,
            const int32_t *data, size_t data_count);
    status_t update(uint32_t tag,
            const float *data, size_t data_count);
    status_t update(uint32_t tag,
            const int64_t *data, size_t data_count);
    status_t update(uint32_t tag,
            const double *data, size_t data_count);
    status_t update(uint32_t tag,
            const camera_metadata_rational_t *data, size_t data_count);
    status_t update(uint32_t tag,
            const String8 &string);
    status_t update(const camera_metadata_ro_entry &entry);


    template<typename T>
    status_t update(uint32_t tag, Vector<T> data) {
        return update(tag, data.array(), data.size());
    }

    /**
     * Check if a metadata entry exists for a given tag id
     *
     */
    bool exists(uint32_t tag) const;

    /**
     * Get metadata entry by tag id
     */
    camera_metadata_entry find(uint32_t tag);

    /**
     * Get metadata entry by tag id, with no editing
     */
    camera_metadata_ro_entry find(uint32_t tag) const;

    /**
     * Delete metadata entry by tag
     */
    status_t erase(uint32_t tag);

    /**
     * Remove metadata entries that need additional permissions.
     */
    status_t removePermissionEntries(metadata_vendor_id_t vendorId,
            std::vector<int32_t> *tagsRemoved /*out*/);

    /**
     * Swap the underlying camera metadata between this and the other
     * metadata object.
     */
    void swap(CameraMetadata &other);

    /**
     * Dump contents into FD for debugging. The verbosity levels are
     * 0: Tag entry information only, no data values
     * 1: Level 0 plus at most 16 data values per entry
     * 2: All information
     *
     * The indentation parameter sets the number of spaces to add to the start
     * each line of output.
     */
    void dump(int fd, int verbosity = 1, int indentation = 0) const;

    /**
     * Serialization over Binder
     */

    // Metadata object is unchanged when reading from parcel fails.
    virtual status_t readFromParcel(const Parcel *parcel) override;
    virtual status_t writeToParcel(Parcel *parcel) const override;

    /**
      * Caller becomes the owner of the new metadata
      * 'const Parcel' doesnt prevent us from calling the read functions.
      *  which is interesting since it changes the internal state
      *
      * NULL can be returned when no metadata was sent, OR if there was an issue
      * unpacking the serialized data (i.e. bad parcel or invalid structure).
      */
    static status_t readFromParcel(const Parcel &parcel,
                                   camera_metadata_t** out);
    /**
      * Caller retains ownership of metadata
      * - Write 2 (int32 + blob) args in the current position
      */
    static status_t writeToParcel(Parcel &parcel,
                                  const camera_metadata_t* metadata);

    /**
     * Find tag id for a given tag name, also checking vendor tags if available.
     * On success, returns OK and writes the tag id into tag.
     *
     * This is a slow method.
     */
    static status_t getTagFromName(const char *name,
            const VendorTagDescriptor* vTags, uint32_t *tag);

  private:
    camera_metadata_t *mBuffer;
    mutable bool       mLocked;

    /**
     * Check if tag has a given type
     */
    status_t checkType(uint32_t tag, uint8_t expectedType);

    /**
     * Base update entry method
     */
    status_t updateImpl(uint32_t tag, const void *data, size_t data_count);

    /**
     * Resize metadata buffer if needed by reallocating it and copying it over.
     */
    status_t resizeIfNeeded(size_t extraEntries, size_t extraData);

};

namespace hardware {
namespace camera2 {
namespace impl {
using ::android::CameraMetadata;
typedef CameraMetadata CameraMetadataNative;
}
}
}

} // namespace android

#endif
