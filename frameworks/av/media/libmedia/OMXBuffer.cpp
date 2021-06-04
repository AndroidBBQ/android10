/*
 * Copyright 2016, The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "OMXBuffer"

#include <media/stagefright/foundation/ADebug.h>
#include <media/MediaCodecBuffer.h>
#include <media/OMXBuffer.h>
#include <binder/IMemory.h>
#include <binder/Parcel.h>
#include <ui/GraphicBuffer.h>
#include <utils/NativeHandle.h>

namespace android {

//static
OMXBuffer OMXBuffer::sPreset(static_cast<sp<MediaCodecBuffer> >(NULL));

OMXBuffer::OMXBuffer()
    : mBufferType(kBufferTypeInvalid) {
}

OMXBuffer::OMXBuffer(const sp<MediaCodecBuffer>& codecBuffer)
    : mBufferType(kBufferTypePreset),
      mRangeOffset(codecBuffer != NULL ? codecBuffer->offset() : 0),
      mRangeLength(codecBuffer != NULL ? codecBuffer->size() : 0) {
}

OMXBuffer::OMXBuffer(OMX_U32 rangeOffset, OMX_U32 rangeLength)
    : mBufferType(kBufferTypePreset),
      mRangeOffset(rangeOffset),
      mRangeLength(rangeLength) {
}

OMXBuffer::OMXBuffer(const sp<IMemory> &mem)
    : mBufferType(kBufferTypeSharedMem),
      mMem(mem) {
}

OMXBuffer::OMXBuffer(const sp<GraphicBuffer> &gbuf)
    : mBufferType(kBufferTypeANWBuffer),
      mGraphicBuffer(gbuf) {
}

OMXBuffer::OMXBuffer(const sp<NativeHandle> &handle)
    : mBufferType(kBufferTypeNativeHandle),
      mNativeHandle(handle) {
}

OMXBuffer::OMXBuffer(const hidl_memory &hidlMemory)
    : mBufferType(kBufferTypeHidlMemory),
      mHidlMemory(hidlMemory) {
}

OMXBuffer::~OMXBuffer() {
}

status_t OMXBuffer::writeToParcel(Parcel *parcel) const {
    CHECK(mBufferType != kBufferTypeHidlMemory);
    parcel->writeInt32(mBufferType);

    switch(mBufferType) {
        case kBufferTypePreset:
        {
            status_t err = parcel->writeUint32(mRangeOffset);
            if (err != OK) {
                return err;
            }
            return parcel->writeUint32(mRangeLength);
        }

        case kBufferTypeSharedMem:
        {
            return parcel->writeStrongBinder(IInterface::asBinder(mMem));
        }

        case kBufferTypeANWBuffer:
        {
            if (mGraphicBuffer == NULL) {
                return parcel->writeBool(false);
            }
            status_t err = parcel->writeBool(true);
            if (err != OK) {
                return err;
            }
            return parcel->write(*mGraphicBuffer);
        }

        case kBufferTypeNativeHandle:
        {
            return parcel->writeNativeHandle(mNativeHandle->handle());
        }

        default:
            return BAD_VALUE;
    }
    return BAD_VALUE;
}

status_t OMXBuffer::readFromParcel(const Parcel *parcel) {
    BufferType bufferType = (BufferType) parcel->readInt32();
    CHECK(bufferType != kBufferTypeHidlMemory);

    switch(bufferType) {
        case kBufferTypePreset:
        {
            status_t err = parcel->readUint32(&mRangeOffset);
            if (err != OK) {
                return err;
            }
            err = parcel->readUint32(&mRangeLength);
            if (err != OK) {
                return err;
            }
            break;
        }

        case kBufferTypeSharedMem:
        {
            mMem = interface_cast<IMemory>(parcel->readStrongBinder());
            break;
        }

        case kBufferTypeANWBuffer:
        {
            bool notNull;
            status_t err = parcel->readBool(&notNull);
            if (err != OK) {
                return err;
            }
            if (notNull) {
                sp<GraphicBuffer> buffer = new GraphicBuffer();
                status_t err = parcel->read(*buffer);
                if (err != OK) {
                    return err;
                }
                mGraphicBuffer = buffer;
            } else {
                mGraphicBuffer = nullptr;
            }
            break;
        }

        case kBufferTypeNativeHandle:
        {
            sp<NativeHandle> handle = NativeHandle::create(
                    parcel->readNativeHandle(), true /* ownsHandle */);

            mNativeHandle = handle;
            break;
        }

        default:
            return BAD_VALUE;
    }

    mBufferType = bufferType;
    return OK;
}

OMXBuffer& OMXBuffer::operator=(OMXBuffer&& source) noexcept {
    mBufferType = std::move(source.mBufferType);
    mRangeOffset = std::move(source.mRangeOffset);
    mRangeLength = std::move(source.mRangeLength);
    mMem = std::move(source.mMem);
    mGraphicBuffer = std::move(source.mGraphicBuffer);
    mNativeHandle = std::move(source.mNativeHandle);
    mHidlMemory = std::move(source.mHidlMemory);
    return *this;
}

} // namespace android




