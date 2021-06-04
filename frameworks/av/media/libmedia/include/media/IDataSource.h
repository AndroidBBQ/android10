/*
 * Copyright 2015 The Android Open Source Project
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

#ifndef ANDROID_IDATASOURCE_H
#define ANDROID_IDATASOURCE_H

#include <binder/IInterface.h>
#include <media/stagefright/foundation/ABase.h>
#include <utils/Errors.h>
#include <utils/String8.h>

namespace android {

class IMemory;
class DecryptHandle;

// A binder interface for implementing a stagefright DataSource remotely.
class IDataSource : public IInterface {
public:
    DECLARE_META_INTERFACE(DataSource);

    // Get the memory that readAt writes into.
    virtual sp<IMemory> getIMemory() = 0;
    // Read up to |size| bytes into the memory returned by getIMemory(). Returns
    // the number of bytes read, or negative value on error (eg.
    // ERROR_END_OF_STREAM indicating EOS. This is needed by CallbackDataSource
    // to properly handle reading of last chunk). |size| must not be larger than
    // the buffer.
    virtual ssize_t readAt(off64_t offset, size_t size) = 0;
    // Get the size, or -1 if the size is unknown.
    virtual status_t getSize(off64_t* size) = 0;
    // This should be called before deleting |this|. The other methods may
    // return errors if they're called after calling close().
    virtual void close() = 0;
    // Get the flags of the source.
    // Refer to DataSource:Flags for the definition of the flags.
    virtual uint32_t getFlags() = 0;
    // get a description of the source, e.g. the url or filename it is based on
    virtual String8 toString() = 0;
    // Initialize DRM and return a DecryptHandle.
    virtual sp<DecryptHandle> DrmInitialization(const char *mime) = 0;

private:
    DISALLOW_EVIL_CONSTRUCTORS(IDataSource);
};

// ----------------------------------------------------------------------------

class BnDataSource : public BnInterface<IDataSource> {
public:
    virtual status_t onTransact(uint32_t code,
                                const Parcel& data,
                                Parcel* reply,
                                uint32_t flags = 0);
};

}; // namespace android

#endif // ANDROID_IDATASOURCE_H
