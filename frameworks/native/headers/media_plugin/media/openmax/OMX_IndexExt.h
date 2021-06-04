/*
 * Copyright (c) 2010 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/** @file OMX_IndexExt.h - OpenMax IL version 1.1.2
 * The OMX_IndexExt header file contains extensions to the definitions
 * for both applications and components .
 */

#ifndef OMX_IndexExt_h
#define OMX_IndexExt_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Each OMX header shall include all required header files to allow the
 * header to compile without errors.  The includes below are required
 * for this header file to compile successfully
 */
#include <OMX_Index.h>


/** Khronos standard extension indices.

This enum lists the current Khronos extension indices to OpenMAX IL.
*/
typedef enum OMX_INDEXEXTTYPE {

    /* Component parameters and configurations */
    OMX_IndexExtComponentStartUnused = OMX_IndexKhronosExtensions + 0x00100000,
    OMX_IndexConfigCallbackRequest,                 /**< reference: OMX_CONFIG_CALLBACKREQUESTTYPE */
    OMX_IndexConfigCommitMode,                      /**< reference: OMX_CONFIG_COMMITMODETYPE */
    OMX_IndexConfigCommit,                          /**< reference: OMX_CONFIG_COMMITTYPE */
    OMX_IndexConfigAndroidVendorExtension,          /**< reference: OMX_CONFIG_VENDOR_EXTENSIONTYPE */

    /* Port parameters and configurations */
    OMX_IndexExtPortStartUnused = OMX_IndexKhronosExtensions + 0x00200000,

    /* Audio parameters and configurations */
    OMX_IndexExtAudioStartUnused = OMX_IndexKhronosExtensions + 0x00400000,
    OMX_IndexParamAudioAndroidAc3,                  /**< reference: OMX_AUDIO_PARAM_ANDROID_AC3TYPE */
    OMX_IndexParamAudioAndroidOpus,                 /**< reference: OMX_AUDIO_PARAM_ANDROID_OPUSTYPE */
    OMX_IndexParamAudioAndroidAacPresentation,      /**< reference: OMX_AUDIO_PARAM_ANDROID_AACPRESENTATIONTYPE */
    OMX_IndexParamAudioAndroidEac3,                 /**< reference: OMX_AUDIO_PARAM_ANDROID_EAC3TYPE */
    OMX_IndexParamAudioProfileQuerySupported,       /**< reference: OMX_AUDIO_PARAM_ANDROID_PROFILETYPE */
    OMX_IndexParamAudioAndroidAacDrcPresentation,   /**< reference: OMX_AUDIO_PARAM_ANDROID_AACDRCPRESENTATIONTYPE */
    OMX_IndexParamAudioAndroidAc4,                  /**< reference: OMX_AUDIO_PARAM_ANDROID_AC4TYPE */
    OMX_IndexConfigAudioPresentation,               /**< reference: OMX_AUDIO_CONFIG_ANDROID_AUDIOPRESENTATION */
    OMX_IndexExtAudioEndUnused,

    /* Image parameters and configurations */
    OMX_IndexExtImageStartUnused = OMX_IndexKhronosExtensions + 0x00500000,

    /* Video parameters and configurations */
    OMX_IndexExtVideoStartUnused = OMX_IndexKhronosExtensions + 0x00600000,
    OMX_IndexParamNalStreamFormatSupported,         /**< reference: OMX_NALSTREAMFORMATTYPE */
    OMX_IndexParamNalStreamFormat,                  /**< reference: OMX_NALSTREAMFORMATTYPE */
    OMX_IndexParamNalStreamFormatSelect,            /**< reference: OMX_NALSTREAMFORMATTYPE */
    OMX_IndexParamVideoVp8,                         /**< reference: OMX_VIDEO_PARAM_VP8TYPE */
    OMX_IndexConfigVideoVp8ReferenceFrame,          /**< reference: OMX_VIDEO_VP8REFERENCEFRAMETYPE */
    OMX_IndexConfigVideoVp8ReferenceFrameType,      /**< reference: OMX_VIDEO_VP8REFERENCEFRAMEINFOTYPE */
    OMX_IndexParamVideoAndroidVp8Encoder,           /**< reference: OMX_VIDEO_PARAM_ANDROID_VP8ENCODERTYPE */
    OMX_IndexParamVideoHevc,                        /**< reference: OMX_VIDEO_PARAM_HEVCTYPE */
    OMX_IndexParamSliceSegments,                    /**< reference: OMX_VIDEO_SLICESEGMENTSTYPE */
    OMX_IndexConfigAndroidIntraRefresh,             /**< reference: OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE */
    OMX_IndexParamAndroidVideoTemporalLayering,     /**< reference: OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE */
    OMX_IndexConfigAndroidVideoTemporalLayering,    /**< reference: OMX_VIDEO_CONFIG_ANDROID_TEMPORALLAYERINGTYPE */
    OMX_IndexParamMaxFrameDurationForBitrateControl,/**< reference: OMX_PARAM_U32TYPE */
    OMX_IndexParamVideoVp9,                         /**< reference: OMX_VIDEO_PARAM_VP9TYPE */
    OMX_IndexParamVideoAndroidVp9Encoder,           /**< reference: OMX_VIDEO_PARAM_ANDROID_VP9ENCODERTYPE */
    OMX_IndexParamVideoAndroidImageGrid,            /**< reference: OMX_VIDEO_PARAM_ANDROID_IMAGEGRIDTYPE */
    OMX_IndexExtVideoEndUnused,

    /* Image & Video common configurations */
    OMX_IndexExtCommonStartUnused = OMX_IndexKhronosExtensions + 0x00700000,

    /* Other configurations */
    OMX_IndexExtOtherStartUnused = OMX_IndexKhronosExtensions + 0x00800000,
    OMX_IndexConfigAutoFramerateConversion,         /**< reference: OMX_CONFIG_BOOLEANTYPE */
    OMX_IndexConfigPriority,                        /**< reference: OMX_PARAM_U32TYPE */
    OMX_IndexConfigOperatingRate,                   /**< reference: OMX_PARAM_U32TYPE in Q16 format for video and in Hz for audio */
    OMX_IndexParamConsumerUsageBits,                /**< reference: OMX_PARAM_U32TYPE */
    OMX_IndexConfigLatency,                         /**< reference: OMX_PARAM_U32TYPE */
    OMX_IndexExtOtherEndUnused,

    /* Time configurations */
    OMX_IndexExtTimeStartUnused = OMX_IndexKhronosExtensions + 0x00900000,

    OMX_IndexExtMax = 0x7FFFFFFF
} OMX_INDEXEXTTYPE;

#define OMX_MAX_STRINGVALUE_SIZE OMX_MAX_STRINGNAME_SIZE
#define OMX_MAX_ANDROID_VENDOR_PARAMCOUNT 32

typedef enum OMX_ANDROID_VENDOR_VALUETYPE {
    OMX_AndroidVendorValueInt32 = 0,   /*<< int32_t value */
    OMX_AndroidVendorValueInt64,       /*<< int64_t value */
    OMX_AndroidVendorValueString,      /*<< string value */
    OMX_AndroidVendorValueEndUnused,
} OMX_ANDROID_VENDOR_VALUETYPE;

/**
 * Structure describing a single value of an Android vendor extension.
 *
 * STRUCTURE MEMBERS:
 *  cKey        : parameter value name.
 *  eValueType  : parameter value type
 *  bSet        : if false, the parameter is not set (for OMX_GetConfig) or is unset (OMX_SetConfig)
 *                if true, the parameter is set to the corresponding value below
 *  nInt64      : int64 value
 *  cString     : string value
 */
typedef struct OMX_CONFIG_ANDROID_VENDOR_PARAMTYPE {
    OMX_U8 cKey[OMX_MAX_STRINGNAME_SIZE];
    OMX_ANDROID_VENDOR_VALUETYPE eValueType;
    OMX_BOOL bSet;
    union {
        OMX_S32 nInt32;
        OMX_S64 nInt64;
        OMX_U8 cString[OMX_MAX_STRINGVALUE_SIZE];
    };
} OMX_CONFIG_ANDROID_VENDOR_PARAMTYPE;

/**
 * OMX_CONFIG_ANDROID_VENDOR_EXTENSIONTYPE is the structure for an Android vendor extension
 * supported by the component. This structure enumerates the various extension parameters and their
 * values.
 *
 * Android vendor extensions have a name and one or more parameter values - each with a string key -
 * that are set together. The values are exposed to Android applications via a string key that is
 * the concatenation of 'vendor', the extension name and the parameter key, each separated by dot
 * (.), with any trailing '.value' suffix(es) removed (though optionally allowed).
 *
 * Extension names and parameter keys are subject to the following rules:
 *   - Each SHALL contain a set of lowercase alphanumeric (underscore allowed) tags separated by
 *     dot (.) or dash (-).
 *   - The first character of the first tag, and any tag following a dot SHALL not start with a
 *     digit.
 *   - Tags 'value', 'vendor', 'omx' and 'android' (even if trailed and/or followed by any number
 *     of underscores) are prohibited in the extension name.
 *   - Tags 'vendor', 'omx' and 'android' (even if trailed and/or followed by any number
 *     of underscores) are prohibited in parameter keys.
 *   - The tag 'value' (even if trailed and/or followed by any number
 *     of underscores) is prohibited in parameter keys with the following exception:
 *     the parameter key may be exactly 'value'
 *   - The parameter key for extensions with a single parameter value SHALL be 'value'
 *   - No two extensions SHALL have the same name
 *   - No extension's name SHALL start with another extension's NAME followed by a dot (.)
 *   - No two parameters of an extension SHALL have the same key
 *
 * This config can be used with both OMX_GetConfig and OMX_SetConfig. In the OMX_GetConfig
 * case, the caller specifies nIndex and nParamSizeUsed. The component fills in cName,
 * eDir and nParamCount. Additionally, if nParamSizeUsed is not less than nParamCount, the
 * component fills out the parameter values (nParam) with the current values for each parameter
 * of the vendor extension.
 *
 * The value of nIndex goes from 0 to N-1, where N is the number of Android vendor extensions
 * supported by the component. The component does not need to report N as the caller can determine
 * N by enumerating all extensions supported by the component. The component may not support any
 * extensions. If there are no more extensions, OMX_GetParameter returns OMX_ErrorNoMore. The
 * component supplies extensions in the order it wants clients to set them.
 *
 * The component SHALL return OMX_ErrorNone for all cases where nIndex is less than N (specifically
 * even in the case of where nParamCount is greater than nParamSizeUsed).
 *
 * In the OMX_SetConfig case the field nIndex is ignored. If the component supports an Android
 * vendor extension with the name in cName, it SHALL configure the parameter values for that
 * extension according to the parameters in nParam. nParamCount is the number of valid parameters
 * in the nParam array, and nParamSizeUsed is the size of the nParam array. (nParamSizeUsed
 * SHALL be at least nParamCount) Parameters that are part of a vendor extension but are not
 * in the nParam array are assumed to be unset (this is different from not changed).
 * All parameter values SHALL have distinct keys in nParam (the component can assume that this
 * is the case. Otherwise, the actual value for the parameters that are multiply defined can
 * be any of the set values.)
 *
 * Return values in case of OMX_SetConfig:
 *   OMX_ErrorUnsupportedIndex: the component does not support the extension specified by cName
 *   OMX_ErrorUnsupportedSetting: the component does not support some or any of the parameters
 *       (names) specified in nParam
 *   OMX_ErrorBadParameter: the parameter is invalid (e.g. nParamCount is greater than
 *       nParamSizeUsed, or some parameter value has invalid type)
 *
 * STRUCTURE MEMBERS:
 *  nSize       : size of the structure in bytes
 *  nVersion    : OMX specification version information
 *  cName       : name of vendor extension
 *  nParamCount : the number of parameter values that are part of this vendor extension
 *  nParamSizeUsed : the size of nParam
 *                (must be at least 1 and at most OMX_MAX_ANDROID_VENDOR_PARAMCOUNT)
 *  param       : the parameter values
 */
typedef struct OMX_CONFIG_ANDROID_VENDOR_EXTENSIONTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nIndex;
    OMX_U8  cName[OMX_MAX_STRINGNAME_SIZE];
    OMX_DIRTYPE eDir;
    OMX_U32 nParamCount;
    OMX_U32 nParamSizeUsed;
    OMX_CONFIG_ANDROID_VENDOR_PARAMTYPE param[1];
} OMX_CONFIG_ANDROID_VENDOR_EXTENSIONTYPE;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OMX_IndexExt_h */
/* File EOF */
