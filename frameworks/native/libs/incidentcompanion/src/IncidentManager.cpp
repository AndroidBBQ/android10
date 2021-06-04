/**
 * Copyright (c) 2016, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <android/os/IncidentManager.h>

namespace android {
namespace os {

// ============================================================
IncidentManager::IncidentReport::IncidentReport()
        :mTimestampNs(0),
         mPrivacyPolicy(0),
         mFileDescriptor(-1) {
}

IncidentManager::IncidentReport::~IncidentReport() {
    if (mFileDescriptor >= 0) {
        close(mFileDescriptor);
    }
}

status_t IncidentManager::IncidentReport::writeToParcel(Parcel* out) const {
    status_t err;

    err = out->writeInt64(mTimestampNs);
    if (err != NO_ERROR) {
        return err;
    }


    err = out->writeInt32(mPrivacyPolicy);
    if (err != NO_ERROR) {
        return err;
    }

    if (mFileDescriptor >= 0) {
        err = out->writeInt32(1);
        if (err != NO_ERROR) {
            return err;
        }

        err = out->writeDupParcelFileDescriptor(mFileDescriptor);
        if (err != NO_ERROR) {
            return err;
        }

    } else {
        err = out->writeInt32(0);
        if (err != NO_ERROR) {
            return err;
        }
    }

    return NO_ERROR;
}

status_t IncidentManager::IncidentReport::readFromParcel(const Parcel* in) {
    status_t err;
    int32_t hasField;

    err = in->readInt64(&mTimestampNs);
    if (err != NO_ERROR) {
        return err;
    }

    err = in->readInt32(&mPrivacyPolicy);
    if (err != NO_ERROR) {
        return err;
    }

    err = in->readInt32(&hasField);
    if (err != NO_ERROR) {
        return err;
    }

    if (hasField) {
        int fd = in->readParcelFileDescriptor();
        if (fd >= 0) {
            mFileDescriptor = dup(fd);
            if (mFileDescriptor < 0) {
                return -errno;
            }
        }
    }

    return NO_ERROR;
}

status_t IncidentManager::IncidentReport::setFileDescriptor(int fd) {
    if (mFileDescriptor >= 0) {
        close(mFileDescriptor);
    }
    if (fd < 0) {
        mFileDescriptor = -1;
    } else {
        mFileDescriptor = dup(fd);
        if (mFileDescriptor < 0) {
            return -errno;
        }
    }
    return NO_ERROR;
}

void IncidentManager::IncidentReport::takeFileDescriptor(int fd) {
    if (mFileDescriptor >= 0) {
        close(mFileDescriptor);
    }
    if (fd < 0) {
        mFileDescriptor = -1;
    } else {
        mFileDescriptor = fd;
    }
}

// ============================================================
IncidentManager::~IncidentManager() {
}

}
}


