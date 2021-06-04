/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include <binder/Status.h>

namespace android {
namespace binder {

Status Status::ok() {
    return Status();
}

Status Status::fromExceptionCode(int32_t exceptionCode) {
    if (exceptionCode == EX_TRANSACTION_FAILED) {
        return Status(exceptionCode, FAILED_TRANSACTION);
    }
    return Status(exceptionCode, OK);
}

Status Status::fromExceptionCode(int32_t exceptionCode,
                                 const String8& message) {
    if (exceptionCode == EX_TRANSACTION_FAILED) {
        return Status(exceptionCode, FAILED_TRANSACTION, message);
    }
    return Status(exceptionCode, OK, message);
}

Status Status::fromExceptionCode(int32_t exceptionCode,
                                 const char* message) {
    return fromExceptionCode(exceptionCode, String8(message));
}

Status Status::fromServiceSpecificError(int32_t serviceSpecificErrorCode) {
    return Status(EX_SERVICE_SPECIFIC, serviceSpecificErrorCode);
}

Status Status::fromServiceSpecificError(int32_t serviceSpecificErrorCode,
                                        const String8& message) {
    return Status(EX_SERVICE_SPECIFIC, serviceSpecificErrorCode, message);
}

Status Status::fromServiceSpecificError(int32_t serviceSpecificErrorCode,
                                        const char* message) {
    return fromServiceSpecificError(serviceSpecificErrorCode, String8(message));
}

Status Status::fromStatusT(status_t status) {
    Status ret;
    ret.setFromStatusT(status);
    return ret;
}

std::string Status::exceptionToString(int32_t exceptionCode) {
    switch (exceptionCode) {
        #define EXCEPTION_TO_CASE(EXCEPTION) case EXCEPTION: return #EXCEPTION;
        EXCEPTION_TO_CASE(EX_NONE)
        EXCEPTION_TO_CASE(EX_SECURITY)
        EXCEPTION_TO_CASE(EX_BAD_PARCELABLE)
        EXCEPTION_TO_CASE(EX_ILLEGAL_ARGUMENT)
        EXCEPTION_TO_CASE(EX_NULL_POINTER)
        EXCEPTION_TO_CASE(EX_ILLEGAL_STATE)
        EXCEPTION_TO_CASE(EX_NETWORK_MAIN_THREAD)
        EXCEPTION_TO_CASE(EX_UNSUPPORTED_OPERATION)
        EXCEPTION_TO_CASE(EX_SERVICE_SPECIFIC)
        EXCEPTION_TO_CASE(EX_PARCELABLE)
        EXCEPTION_TO_CASE(EX_HAS_REPLY_HEADER)
        EXCEPTION_TO_CASE(EX_TRANSACTION_FAILED)
        #undef EXCEPTION_TO_CASE
        default: return std::to_string(exceptionCode);
    }
}

Status::Status(int32_t exceptionCode, int32_t errorCode)
    : mException(exceptionCode),
      mErrorCode(errorCode) {}

Status::Status(int32_t exceptionCode, int32_t errorCode, const String8& message)
    : mException(exceptionCode),
      mErrorCode(errorCode),
      mMessage(message) {}

status_t Status::readFromParcel(const Parcel& parcel) {
    status_t status = parcel.readInt32(&mException);
    if (status != OK) {
        setFromStatusT(status);
        return status;
    }

    // Skip over fat response headers.  Not used (or propagated) in native code.
    if (mException == EX_HAS_REPLY_HEADER) {
        // Note that the header size includes the 4 byte size field.
        const size_t header_start = parcel.dataPosition();
        // Get available size before reading more
        const size_t header_avail = parcel.dataAvail();

        int32_t header_size;
        status = parcel.readInt32(&header_size);
        if (status != OK) {
            setFromStatusT(status);
            return status;
        }

        if (header_size < 0 || static_cast<size_t>(header_size) > header_avail) {
            android_errorWriteLog(0x534e4554, "132650049");
            setFromStatusT(UNKNOWN_ERROR);
            return UNKNOWN_ERROR;
        }

        parcel.setDataPosition(header_start + header_size);
        // And fat response headers are currently only used when there are no
        // exceptions, so act like there was no error.
        mException = EX_NONE;
    }

    if (mException == EX_NONE) {
        return status;
    }

    // The remote threw an exception.  Get the message back.
    String16 message;
    status = parcel.readString16(&message);
    if (status != OK) {
        setFromStatusT(status);
        return status;
    }
    mMessage = String8(message);

    // Skip over the remote stack trace data
    int32_t remote_stack_trace_header_size;
    status = parcel.readInt32(&remote_stack_trace_header_size);
    if (status != OK) {
        setFromStatusT(status);
        return status;
    }
    if (remote_stack_trace_header_size < 0 ||
        static_cast<size_t>(remote_stack_trace_header_size) > parcel.dataAvail()) {

        android_errorWriteLog(0x534e4554, "132650049");
        setFromStatusT(UNKNOWN_ERROR);
        return UNKNOWN_ERROR;
    }
    parcel.setDataPosition(parcel.dataPosition() + remote_stack_trace_header_size);

    if (mException == EX_SERVICE_SPECIFIC) {
        status = parcel.readInt32(&mErrorCode);
    } else if (mException == EX_PARCELABLE) {
        // Skip over the blob of Parcelable data
        const size_t header_start = parcel.dataPosition();
        // Get available size before reading more
        const size_t header_avail = parcel.dataAvail();

        int32_t header_size;
        status = parcel.readInt32(&header_size);
        if (status != OK) {
            setFromStatusT(status);
            return status;
        }

        if (header_size < 0 || static_cast<size_t>(header_size) > header_avail) {
            android_errorWriteLog(0x534e4554, "132650049");
            setFromStatusT(UNKNOWN_ERROR);
            return UNKNOWN_ERROR;
        }

        parcel.setDataPosition(header_start + header_size);
    }
    if (status != OK) {
        setFromStatusT(status);
        return status;
    }

    return status;
}

status_t Status::writeToParcel(Parcel* parcel) const {
    // Something really bad has happened, and we're not going to even
    // try returning rich error data.
    if (mException == EX_TRANSACTION_FAILED) {
        return mErrorCode;
    }

    status_t status = parcel->writeInt32(mException);
    if (status != OK) { return status; }
    if (mException == EX_NONE) {
        // We have no more information to write.
        return status;
    }
    status = parcel->writeString16(String16(mMessage));
    status = parcel->writeInt32(0); // Empty remote stack trace header
    if (mException == EX_SERVICE_SPECIFIC) {
        status = parcel->writeInt32(mErrorCode);
    } else if (mException == EX_PARCELABLE) {
        // Sending Parcelable blobs currently not supported
        status = parcel->writeInt32(0);
    }
    return status;
}

void Status::setException(int32_t ex, const String8& message) {
    mException = ex;
    mErrorCode = ex == EX_TRANSACTION_FAILED ? FAILED_TRANSACTION : NO_ERROR;
    mMessage.setTo(message);
}

void Status::setServiceSpecificError(int32_t errorCode, const String8& message) {
    setException(EX_SERVICE_SPECIFIC, message);
    mErrorCode = errorCode;
}

void Status::setFromStatusT(status_t status) {
    mException = (status == NO_ERROR) ? EX_NONE : EX_TRANSACTION_FAILED;
    mErrorCode = status;
    mMessage.clear();
}

String8 Status::toString8() const {
    String8 ret;
    if (mException == EX_NONE) {
        ret.append("No error");
    } else {
        ret.appendFormat("Status(%d, %s): '", mException, exceptionToString(mException).c_str());
        if (mException == EX_SERVICE_SPECIFIC ||
            mException == EX_TRANSACTION_FAILED) {
            ret.appendFormat("%d: ", mErrorCode);
        }
        ret.append(String8(mMessage));
        ret.append("'");
    }
    return ret;
}

std::stringstream& operator<< (std::stringstream& stream, const Status& s) {
    stream << s.toString8().string();
    return stream;
}

}  // namespace binder
}  // namespace android
