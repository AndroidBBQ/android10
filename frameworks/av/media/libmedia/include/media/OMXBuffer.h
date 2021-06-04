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

#ifndef _OMXBUFFER_H_
#define _OMXBUFFER_H_

#include <cutils/native_handle.h>
#include <media/IOMX.h>
#include <utils/StrongPointer.h>
#include <hidl/HidlSupport.h>

namespace android {

class OMXBuffer;

// This is needed temporarily for the OMX HIDL transition.
namespace hardware { namespace media { namespace omx {
namespace V1_0 {
    struct CodecBuffer;
namespace implementation {
    inline bool wrapAs(::android::hardware::media::omx::V1_0::CodecBuffer* t,
            ::android::OMXBuffer const& l);
    inline bool convertTo(::android::OMXBuffer* l,
            ::android::hardware::media::omx::V1_0::CodecBuffer const& t);
}
namespace utils {
    inline bool wrapAs(::android::hardware::media::omx::V1_0::CodecBuffer* t,
            ::android::OMXBuffer const& l);
    inline bool convertTo(::android::OMXBuffer* l,
            ::android::hardware::media::omx::V1_0::CodecBuffer const& t);
}
}}}}

class GraphicBuffer;
class IMemory;
class MediaCodecBuffer;
class NativeHandle;
struct OMXNodeInstance;
using hardware::hidl_memory;

// TODO: After complete HIDL transition, this class would be replaced by
// CodecBuffer.
class OMXBuffer {
public:
    // sPreset is used in places where we are referring to a pre-registered
    // buffer on a port. It has type kBufferTypePreset and mRangeLength of 0.
    static OMXBuffer sPreset;

    // Default constructor, constructs a buffer of type kBufferTypeInvalid.
    OMXBuffer();

    // Constructs a buffer of type kBufferTypePreset with mRangeOffset set to
    // |codecBuffer|'s offset and mRangeLength set to |codecBuffer|'s size (or 0
    // if |codecBuffer| is NULL).
    OMXBuffer(const sp<MediaCodecBuffer> &codecBuffer);

    // Constructs a buffer of type kBufferTypePreset with specified mRangeOffset
    // and mRangeLength.
    OMXBuffer(OMX_U32 rangeOffset, OMX_U32 rangeLength);

    // Constructs a buffer of type kBufferTypeSharedMem.
    OMXBuffer(const sp<IMemory> &mem);

    // Constructs a buffer of type kBufferTypeANWBuffer.
    OMXBuffer(const sp<GraphicBuffer> &gbuf);

    // Constructs a buffer of type kBufferTypeNativeHandle.
    OMXBuffer(const sp<NativeHandle> &handle);

    // Constructs a buffer of type kBufferTypeHidlMemory.
    OMXBuffer(const hidl_memory &hidlMemory);

    // Parcelling/Un-parcelling.
    status_t writeToParcel(Parcel *parcel) const;
    status_t readFromParcel(const Parcel *parcel);

    ~OMXBuffer();

private:
    friend struct OMXNodeInstance;
    friend struct C2OMXNode;

    // This is needed temporarily for OMX HIDL transition.
    friend inline bool (::android::hardware::media::omx::V1_0::implementation::
            wrapAs)(::android::hardware::media::omx::V1_0::CodecBuffer* t,
            OMXBuffer const& l);
    friend inline bool (::android::hardware::media::omx::V1_0::implementation::
            convertTo)(OMXBuffer* l,
            ::android::hardware::media::omx::V1_0::CodecBuffer const& t);
    friend inline bool (::android::hardware::media::omx::V1_0::utils::
            wrapAs)(::android::hardware::media::omx::V1_0::CodecBuffer* t,
            OMXBuffer const& l);
    friend inline bool (::android::hardware::media::omx::V1_0::utils::
            convertTo)(OMXBuffer* l,
            ::android::hardware::media::omx::V1_0::CodecBuffer const& t);

    enum BufferType {
        kBufferTypeInvalid = 0,
        kBufferTypePreset,
        kBufferTypeSharedMem,
        kBufferTypeANWBuffer, // Use only for non-Treble
        kBufferTypeNativeHandle,
        kBufferTypeHidlMemory // Mapped to CodecBuffer::Type::SHARED_MEM.
    };

    BufferType mBufferType;

    // kBufferTypePreset
    // If the port is operating in byte buffer mode, mRangeLength is the valid
    // range length. Otherwise the range info should also be ignored.
    OMX_U32 mRangeOffset;
    OMX_U32 mRangeLength;

    // kBufferTypeSharedMem
    sp<IMemory> mMem;

    // kBufferTypeANWBuffer
    sp<GraphicBuffer> mGraphicBuffer;

    // kBufferTypeNativeHandle
    sp<NativeHandle> mNativeHandle;

    // kBufferTypeHidlMemory
    hidl_memory mHidlMemory;

    // Move assignment
    OMXBuffer &operator=(OMXBuffer&&) noexcept;

    // Deleted copy constructor and assignment.
    OMXBuffer(const OMXBuffer&) = delete;
    OMXBuffer& operator=(const OMXBuffer&) = delete;
};

}  // namespace android

#endif  // _OMXBUFFER_H_
