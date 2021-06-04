/*
 * Copyright (C) 2010 The Android Open Source Project
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

#define LOG_TAG "Surface"

#include <gui/view/Surface.h>

#include <binder/Parcel.h>

#include <utils/Log.h>

#include <gui/IGraphicBufferProducer.h>

namespace android {
namespace view {

status_t Surface::writeToParcel(Parcel* parcel) const {
    return writeToParcel(parcel, false);
}

status_t Surface::writeToParcel(Parcel* parcel, bool nameAlreadyWritten) const {
    if (parcel == nullptr) return BAD_VALUE;

    status_t res = OK;

    if (!nameAlreadyWritten) {
        res = parcel->writeString16(name);
        if (res != OK) return res;

        /* isSingleBuffered defaults to no */
        res = parcel->writeInt32(0);
        if (res != OK) return res;
    }

    return IGraphicBufferProducer::exportToParcel(graphicBufferProducer, parcel);
}

status_t Surface::readFromParcel(const Parcel* parcel) {
    return readFromParcel(parcel, false);
}

status_t Surface::readFromParcel(const Parcel* parcel, bool nameAlreadyRead) {
    if (parcel == nullptr) return BAD_VALUE;

    status_t res = OK;
    if (!nameAlreadyRead) {
        name = readMaybeEmptyString16(parcel);
        // Discard this for now
        int isSingleBuffered;
        res = parcel->readInt32(&isSingleBuffered);
        if (res != OK) {
            ALOGE("Can't read isSingleBuffered");
            return res;
        }
    }

    graphicBufferProducer = IGraphicBufferProducer::createFromParcel(parcel);
    return OK;
}

String16 Surface::readMaybeEmptyString16(const Parcel* parcel) {
    size_t len;
    const char16_t* str = parcel->readString16Inplace(&len);
    if (str != nullptr) {
        return String16(str, len);
    } else {
        return String16();
    }
}

} // namespace view
} // namespace android
