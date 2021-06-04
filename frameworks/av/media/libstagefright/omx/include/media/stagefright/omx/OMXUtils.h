/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef OMX_UTILS_H_
#define OMX_UTILS_H_

#include <media/IOMX.h>

/***** DO NOT USE THIS INCLUDE!!! INTERAL ONLY!!! UNLESS YOU RESIDE IN media/libstagefright *****/

// OMXUtils contains omx-specific utility functions for stagefright/omx library
// TODO: move ACodec and OMXClient into this library

namespace android {

template<class T>
static void InitOMXParams(T *params) {
    memset(params, 0, sizeof(T));
    params->nSize = sizeof(T);
    params->nVersion.s.nVersionMajor = 1;
    params->nVersion.s.nVersionMinor = 0;
    params->nVersion.s.nRevision = 0;
    params->nVersion.s.nStep = 0;
}

status_t StatusFromOMXError(OMX_ERRORTYPE err);

const char *GetComponentRole(bool isEncoder, const char *mime);
status_t SetComponentRole(const sp<IOMXNode> &omxNode, const char *role);

struct DescribeColorFormat2Params;

bool IsFlexibleColorFormat(
        const sp<IOMXNode> &omxNode, uint32_t colorFormat,
        bool usingNativeBuffers, OMX_U32 *flexibleEquivalent);
bool DescribeDefaultColorFormat(DescribeColorFormat2Params &describeParams);
bool DescribeColorFormat(
        const sp<IOMXNode> &omxNode,
        DescribeColorFormat2Params &describeParams);

inline static const char *asString(MetadataBufferType i, const char *def = "??") {
    using namespace android;
    switch (i) {
        case kMetadataBufferTypeCameraSource:   return "CameraSource";
        case kMetadataBufferTypeGrallocSource:  return "GrallocSource";
        case kMetadataBufferTypeANWBuffer:      return "ANWBuffer";
        case kMetadataBufferTypeNativeHandleSource: return "NativeHandleSource";
        case kMetadataBufferTypeInvalid:        return "Invalid";
        default:                                return def;
    }
}

inline static const char *asString(IOMX::PortMode mode, const char *def = "??") {
    using namespace android;
    switch (mode) {
        case IOMX::kPortModePresetByteBuffer:   return "PresetByteBuffer";
        case IOMX::kPortModePresetANWBuffer:    return "PresetANWBuffer";
        case IOMX::kPortModePresetSecureBuffer: return "PresetSecureBuffer";
        case IOMX::kPortModeDynamicANWBuffer:   return "DynamicANWBuffer";
        case IOMX::kPortModeDynamicNativeHandle:return "DynamicNativeHandle";
        default:                                return def;
    }
}

}  // namespace android

#endif
