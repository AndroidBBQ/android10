/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef STAGEFRIGHT_FOUNDATION_FILE_DESCRIPTOR_H_
#define STAGEFRIGHT_FOUNDATION_FILE_DESCRIPTOR_H_

#include <memory>

namespace android {

/**
 * FileDescriptor is a utility class for managing file descriptors in a scoped way.
 *
 * usage:
 *
 * status_t function(int fd) {
 *   FileDescriptor::Autoclose managedFd(fd);
 *   if (error_condition)
 *     return ERROR;
 *   next_function(managedFd.release());
 * }
 */
struct FileDescriptor {
    // created this class with minimal methods. more methods can be added here to manage
    // a shared file descriptor object.

    /**
     * A locally scoped managed file descriptor object. This object is not shareable/copiable and
     * is not thread safe.
     */
    struct Autoclose {
        // created this class with minimal methods
        /**
         * Creates a locally scoped file descriptor holder object taking ownership of the passed in
         * file descriptor.
         */
        Autoclose(int fd)
            : mFd(fd) {

        }

        ~Autoclose() {
            if (isValid()) {
                ::close(mFd);
                mFd = kInvalidFileDescriptor;
            }
        }

        /**
         * Releases the managed file descriptor from the holder. This invalidates the (remaining)
         * file descriptor in this object.
         */
        int release() {
            int managedFd = mFd;
            mFd = kInvalidFileDescriptor;
            return managedFd;
        }

        /**
         * Checks whether the managed file descriptor is valid
         */
        bool isValid() const {
            return mFd >= 0;
        }

    private:
        // not yet needed

        /**
         * Returns the managed file descriptor from this object without releasing the ownership.
         * The returned file descriptor has the same lifecycle as the managed file descriptor
         * in this object. Therefore, care must be taken that it is not closed, and that this
         * object keeps managing the returned file descriptor for the duration of its use.
         */
        int get() const {
            return mFd;
        }

    private:
        int mFd;

        enum {
            kInvalidFileDescriptor = -1,
        };

        DISALLOW_EVIL_CONSTRUCTORS(Autoclose);
    };

private:
    std::shared_ptr<Autoclose> mSharedFd;
};

}  // namespace android

#endif  // STAGEFRIGHT_FOUNDATION_FLAGGED_H_

