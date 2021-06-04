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

/** Data locator, data format, data source, and data sink support */

#include "sles_allinclusive.h"
#ifdef ANDROID  // FIXME This file should be portable
#include "android/channels.h"
#include "data.h"
#endif

/** \brief Check a data locator and make local deep copy */

static SLresult checkDataLocator(const char *name, void *pLocator, DataLocator *pDataLocator,
        SLuint32 allowedDataLocatorMask)
{
    assert(NULL != name && NULL != pDataLocator);
    SLresult result = SL_RESULT_SUCCESS;

    SLuint32 locatorType;
    if (NULL == pLocator) {
        pDataLocator->mLocatorType = locatorType = SL_DATALOCATOR_NULL;
    } else {
        locatorType = *(SLuint32 *)pLocator;
        switch (locatorType) {

        case SL_DATALOCATOR_ADDRESS:
            pDataLocator->mAddress = *(SLDataLocator_Address *)pLocator;
            // if length is greater than zero, then the address must be non-NULL
            if ((0 < pDataLocator->mAddress.length) && (NULL == pDataLocator->mAddress.pAddress)) {
                SL_LOGE("%s: pAddress=NULL", name);
                result = SL_RESULT_PARAMETER_INVALID;
            }
            break;

        case SL_DATALOCATOR_BUFFERQUEUE:
#ifdef ANDROID
        // This is an alias that is _not_ converted; the rest of the code must check for both
        // locator types. That's because it is only an alias for audio players, not audio recorder
        // objects so we have to remember the distinction.
        case SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE:
#endif
            pDataLocator->mBufferQueue = *(SLDataLocator_BufferQueue *)pLocator;
            // number of buffers must be specified, there is no default value, and can't be too big
            if (!((1 <= pDataLocator->mBufferQueue.numBuffers) &&
                (pDataLocator->mBufferQueue.numBuffers <= 255))) {
                SL_LOGE("%s: numBuffers=%u", name, pDataLocator->mBufferQueue.numBuffers);
                result = SL_RESULT_PARAMETER_INVALID;
            }
            break;

        case SL_DATALOCATOR_IODEVICE:
            {
            pDataLocator->mIODevice = *(SLDataLocator_IODevice *)pLocator;
            SLuint32 deviceType = pDataLocator->mIODevice.deviceType;
            SLObjectItf device = pDataLocator->mIODevice.device;
            if (NULL != device) {
                pDataLocator->mIODevice.deviceID = 0;
                SLuint32 expectedObjectID;
                switch (deviceType) {
                case SL_IODEVICE_LEDARRAY:
                    expectedObjectID = SL_OBJECTID_LEDDEVICE;
                    break;
                case SL_IODEVICE_VIBRA:
                    expectedObjectID = SL_OBJECTID_VIBRADEVICE;
                    break;
                case XA_IODEVICE_CAMERA:
                    expectedObjectID = XA_OBJECTID_CAMERADEVICE;
                    break;
                case XA_IODEVICE_RADIO:
                    expectedObjectID = XA_OBJECTID_RADIODEVICE;
                    break;
                // audio input and audio output cannot be specified via objects
                case SL_IODEVICE_AUDIOINPUT:
                // case SL_IODEVICE_AUDIOOUTPUT:   // does not exist in 1.0.1, added in 1.1
                default:
                    SL_LOGE("%s: deviceType=%u", name, deviceType);
                    pDataLocator->mIODevice.device = NULL;
                    expectedObjectID = 0;
                    result = SL_RESULT_PARAMETER_INVALID;
                }
                if (result == SL_RESULT_SUCCESS) {
                    // check that device has the correct object ID and is realized,
                    // and acquire a strong reference to it
                    result = AcquireStrongRef((IObject *) device, expectedObjectID);
                    if (SL_RESULT_SUCCESS != result) {
                        SL_LOGE("%s: locatorType=IODEVICE, but device field %p has wrong " \
                            "object ID or is not realized", name, device);
                        pDataLocator->mIODevice.device = NULL;
                    }
                }
            } else {
                SLuint32 deviceID = pDataLocator->mIODevice.deviceID;
                switch (deviceType) {
                case SL_IODEVICE_LEDARRAY:
                    if (SL_DEFAULTDEVICEID_LED != deviceID) {
                        SL_LOGE("%s: invalid LED deviceID=%u", name, deviceID);
                        result = SL_RESULT_PARAMETER_INVALID;
                    }
                    break;
                case SL_IODEVICE_VIBRA:
                    if (SL_DEFAULTDEVICEID_VIBRA != deviceID) {
                        SL_LOGE("%s: invalid vibra deviceID=%u", name, deviceID);
                        result = SL_RESULT_PARAMETER_INVALID;
                    }
                    break;
                case SL_IODEVICE_AUDIOINPUT:
                    if (SL_DEFAULTDEVICEID_AUDIOINPUT != deviceID) {
                        SL_LOGE("%s: invalid audio input deviceID=%u", name, deviceID);
                        result = SL_RESULT_PARAMETER_INVALID;
                    }
                    break;
                case XA_IODEVICE_RADIO:
                    // no default device ID for radio; see Khronos bug XXXX
                    break;
                case XA_IODEVICE_CAMERA:
                    if (XA_DEFAULTDEVICEID_CAMERA != deviceID) {
                        SL_LOGE("%s: invalid audio input deviceID=%u", name, deviceID);
                        result = XA_RESULT_PARAMETER_INVALID;
                    }
                    break;
                // case SL_IODEVICE_AUDIOOUTPUT:
                    // does not exist in 1.0.1, added in 1.1
                    // break;
                default:
                    SL_LOGE("%s: deviceType=%u is invalid", name, deviceType);
                    result = SL_RESULT_PARAMETER_INVALID;
                }
            }
            }
            break;

        case SL_DATALOCATOR_MIDIBUFFERQUEUE:
            pDataLocator->mMIDIBufferQueue = *(SLDataLocator_MIDIBufferQueue *)pLocator;
            if (0 == pDataLocator->mMIDIBufferQueue.tpqn) {
                pDataLocator->mMIDIBufferQueue.tpqn = 192;
            }
            // number of buffers must be specified, there is no default value, and can't be too big
            if (!((1 <= pDataLocator->mMIDIBufferQueue.numBuffers) &&
                (pDataLocator->mMIDIBufferQueue.numBuffers <= 255))) {
                SL_LOGE("%s: SLDataLocator_MIDIBufferQueue.numBuffers=%d", name,
                        pDataLocator->mMIDIBufferQueue.numBuffers);
                result = SL_RESULT_PARAMETER_INVALID;
            }
            break;

        case SL_DATALOCATOR_OUTPUTMIX:
            pDataLocator->mOutputMix = *(SLDataLocator_OutputMix *)pLocator;
            // check that output mix object has the correct object ID and is realized,
            // and acquire a strong reference to it
            result = AcquireStrongRef((IObject *) pDataLocator->mOutputMix.outputMix,
                SL_OBJECTID_OUTPUTMIX);
            if (SL_RESULT_SUCCESS != result) {
                SL_LOGE("%s: locatorType=SL_DATALOCATOR_OUTPUTMIX, but outputMix field %p does " \
                    "not refer to an SL_OBJECTID_OUTPUTMIX or the output mix is not realized", \
                    name, pDataLocator->mOutputMix.outputMix);
                pDataLocator->mOutputMix.outputMix = NULL;
            }
            break;

        case XA_DATALOCATOR_NATIVEDISPLAY:
            pDataLocator->mNativeDisplay = *(XADataLocator_NativeDisplay *)pLocator;
            // hWindow is NDK C ANativeWindow * and hDisplay must be NULL
            if (pDataLocator->mNativeDisplay.hWindow == NULL) {
                SL_LOGE("%s: hWindow must be non-NULL ANativeWindow *", name);
                result = SL_RESULT_PARAMETER_INVALID;
            }
            if (pDataLocator->mNativeDisplay.hDisplay != NULL) {
                SL_LOGE("%s: hDisplay must be NULL, but is %p", name,
                        pDataLocator->mNativeDisplay.hDisplay);
                result = SL_RESULT_PARAMETER_INVALID;
            }
            break;

        case SL_DATALOCATOR_URI:
            {
            pDataLocator->mURI = *(SLDataLocator_URI *)pLocator;
            if (NULL == pDataLocator->mURI.URI) {
                SL_LOGE("%s: invalid URI=NULL", name);
                result = SL_RESULT_PARAMETER_INVALID;
            } else {
                // NTH verify URI address for validity
                size_t len = strlen((const char *) pDataLocator->mURI.URI);
                SLchar *myURI = (SLchar *) malloc(len + 1);
                if (NULL == myURI) {
                    result = SL_RESULT_MEMORY_FAILURE;
                } else {
                    memcpy(myURI, pDataLocator->mURI.URI, len + 1);
                    // Verify that another thread didn't change the NUL-terminator after we used it
                    // to determine length of string to copy. It's OK if the string became shorter.
                    if ('\0' != myURI[len]) {
                        free(myURI);
                        myURI = NULL;
                        result = SL_RESULT_PARAMETER_INVALID;
                    }
                }
                pDataLocator->mURI.URI = myURI;
            }
            }
            break;

#ifdef ANDROID
        case SL_DATALOCATOR_ANDROIDFD:
        {
            pDataLocator->mFD = *(SLDataLocator_AndroidFD *)pLocator;
            SL_LOGV("%s: fd=%d offset=%lld length=%lld", name, pDataLocator->mFD.fd,
                    pDataLocator->mFD.offset, pDataLocator->mFD.length);
            // NTH check against process fd limit
            if (0 > pDataLocator->mFD.fd) {
                SL_LOGE("%s: fd=%d\n", name, pDataLocator->mFD.fd);
                result = SL_RESULT_PARAMETER_INVALID;
            }
            break;
        }
        case SL_DATALOCATOR_ANDROIDBUFFERQUEUE:
        {
            pDataLocator->mABQ = *(SLDataLocator_AndroidBufferQueue*)pLocator;
            // number of buffers must be specified, there is no default value, and can't be too big
            if (!((1 <= pDataLocator->mBufferQueue.numBuffers) &&
                    (pDataLocator->mBufferQueue.numBuffers <= 255))) {
                SL_LOGE("%s: numBuffers=%u", name, pDataLocator->mABQ.numBuffers);
                result = SL_RESULT_PARAMETER_INVALID;
            }
            break;
        }
#endif

        case SL_DATALOCATOR_NULL:   // a NULL pointer is allowed, but not a pointer to NULL
        default:
            SL_LOGE("%s: locatorType=%u", name, locatorType);
            result = SL_RESULT_PARAMETER_INVALID;
        }

        // Verify that another thread didn't change the locatorType field after we used it
        // to determine sizeof struct to copy.
        if ((SL_RESULT_SUCCESS == result) && (locatorType != pDataLocator->mLocatorType)) {
            SL_LOGE("%s: locatorType changed from %u to %u", name, locatorType,
                    pDataLocator->mLocatorType);
            result = SL_RESULT_PRECONDITIONS_VIOLATED;
        }

    }

    // Verify that the data locator type is allowed in this context
    if (SL_RESULT_SUCCESS == result) {
        SLuint32 actualMask;
        switch (locatorType) {
        case SL_DATALOCATOR_NULL:
        case SL_DATALOCATOR_URI:
        case SL_DATALOCATOR_ADDRESS:
        case SL_DATALOCATOR_IODEVICE:
        case SL_DATALOCATOR_OUTPUTMIX:
        case XA_DATALOCATOR_NATIVEDISPLAY:
        case SL_DATALOCATOR_BUFFERQUEUE:
        case SL_DATALOCATOR_MIDIBUFFERQUEUE:
            actualMask = 1L << locatorType;
            break;
#ifdef ANDROID
        case SL_DATALOCATOR_ANDROIDFD:
        case SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE:
        case SL_DATALOCATOR_ANDROIDBUFFERQUEUE:
            actualMask = 0x100L << (locatorType - SL_DATALOCATOR_ANDROIDFD);
            break;
#endif
        default:
            assert(false);
            actualMask = 0L;
            break;
        }
        if (!(allowedDataLocatorMask & actualMask)) {
            SL_LOGE("%s: data locator type 0x%x not allowed", name, locatorType);
            result = SL_RESULT_CONTENT_UNSUPPORTED;
        }
    }

    return result;
}


/** \brief Free the local deep copy of a data locator */

static void freeDataLocator(DataLocator *pDataLocator)
{
    switch (pDataLocator->mLocatorType) {
    case SL_DATALOCATOR_NULL:
    case SL_DATALOCATOR_ADDRESS:
    case SL_DATALOCATOR_BUFFERQUEUE:
    case SL_DATALOCATOR_MIDIBUFFERQUEUE:
    case XA_DATALOCATOR_NATIVEDISPLAY:
        break;
    case SL_DATALOCATOR_URI:
        if (NULL != pDataLocator->mURI.URI) {
            free(pDataLocator->mURI.URI);
            pDataLocator->mURI.URI = NULL;
        }
        pDataLocator->mURI.URI = NULL;
        break;
    case SL_DATALOCATOR_IODEVICE:
        if (NULL != pDataLocator->mIODevice.device) {
            ReleaseStrongRef((IObject *) pDataLocator->mIODevice.device);
            pDataLocator->mIODevice.device = NULL;
        }
        break;
    case SL_DATALOCATOR_OUTPUTMIX:
        if (NULL != pDataLocator->mOutputMix.outputMix) {
            ReleaseStrongRef((IObject *) pDataLocator->mOutputMix.outputMix);
            pDataLocator->mOutputMix.outputMix = NULL;
        }
        break;
#ifdef ANDROID
    case SL_DATALOCATOR_ANDROIDBUFFERQUEUE:
    case SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE:
    case SL_DATALOCATOR_ANDROIDFD:
        break;
#endif
    default:
        // an invalid data locator is caught earlier when making the copy
        assert(false);
        break;
    }
}


/** \brief Check a data format and make local deep copy */

static SLresult checkDataFormat(const char *name, void *pFormat, DataFormat *pDataFormat,
        SLuint32 allowedDataFormatMask, SLboolean isOutputFormat)
{
    assert(NULL != name && NULL != pDataFormat);
    SLresult result = SL_RESULT_SUCCESS;
    const SLuint32 *df_representation = NULL; // pointer to representation field, if it exists
    SLuint32 formatType;
    if (NULL == pFormat) {
        pDataFormat->mFormatType = formatType = SL_DATAFORMAT_NULL;
    } else {
        formatType = *(SLuint32 *)pFormat;
        switch (formatType) {
        case SL_ANDROID_DATAFORMAT_PCM_EX:
            pDataFormat->mPCMEx.representation =
                    ((SLAndroidDataFormat_PCM_EX *)pFormat)->representation;
            switch (pDataFormat->mPCMEx.representation) {
            case SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT:
            case SL_ANDROID_PCM_REPRESENTATION_UNSIGNED_INT:
            case SL_ANDROID_PCM_REPRESENTATION_FLOAT:
                df_representation = &pDataFormat->mPCMEx.representation;
                break;
            default:
                SL_LOGE("%s: unsupported representation: %d", name,
                        pDataFormat->mPCMEx.representation);
                result = SL_RESULT_PARAMETER_INVALID;
                break;
            }
            // SL_ANDROID_DATAFORMAT_PCM_EX - fall through to next test.
            FALLTHROUGH_INTENDED;
        case SL_DATAFORMAT_PCM:
            pDataFormat->mPCM = *(SLDataFormat_PCM *)pFormat;
            do {
                if (pDataFormat->mPCM.numChannels == 0) {
                    result = SL_RESULT_PARAMETER_INVALID;
                } else if (pDataFormat->mPCM.numChannels > SL_ANDROID_SPEAKER_COUNT_MAX) {
                    result = SL_RESULT_CONTENT_UNSUPPORTED;
                }
                if (SL_RESULT_SUCCESS != result) {
                    SL_LOGE("%s: numChannels=%u", name, (unsigned) pDataFormat->mPCM.numChannels);
                    break;
                }

                // check the sampling rate
                if (pDataFormat->mPCM.samplesPerSec == 0) {
                    result = SL_RESULT_PARAMETER_INVALID;
                } else if (pDataFormat->mPCM.samplesPerSec < SL_SAMPLINGRATE_8 ||
                        pDataFormat->mPCM.samplesPerSec > SL_SAMPLINGRATE_192) {
                    result = SL_RESULT_CONTENT_UNSUPPORTED;
                }
                if (SL_RESULT_SUCCESS != result) {
                    SL_LOGE("%s: samplesPerSec=%u", name, pDataFormat->mPCM.samplesPerSec);
                    break;
                }

                // check the container bit depth and representation
                switch (pDataFormat->mPCM.containerSize) {
                case 8:
                    if (df_representation != NULL &&
                            *df_representation != SL_ANDROID_PCM_REPRESENTATION_UNSIGNED_INT) {
                        result = SL_RESULT_PARAMETER_INVALID;
                    }
                    break;
                case 16:
                case 24:
                    if (df_representation != NULL &&
                            *df_representation != SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT) {
                        result = SL_RESULT_PARAMETER_INVALID;
                    }
                    break;
                case 32:
                    if (df_representation != NULL
                            && *df_representation != SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT
                            && *df_representation != SL_ANDROID_PCM_REPRESENTATION_FLOAT) {
                        result = SL_RESULT_PARAMETER_INVALID;
                    }
                    break;
                default:
                    result = SL_RESULT_PARAMETER_INVALID;
                    break;
                }
                if (SL_RESULT_SUCCESS != result) {
                    SL_LOGE("%s: containerSize=%u", name, pDataFormat->mPCM.containerSize);
                    break;
                }

                // sample size cannot be zero, and container size cannot be less than sample size
                if (pDataFormat->mPCM.bitsPerSample == 0 ||
                        pDataFormat->mPCM.containerSize < pDataFormat->mPCM.bitsPerSample) {
                    result = SL_RESULT_PARAMETER_INVALID;
                }
                if (SL_RESULT_SUCCESS != result) {
                    SL_LOGE("%s: containerSize=%u, bitsPerSample=%u", name,
                            (unsigned) pDataFormat->mPCM.containerSize,
                            (unsigned) pDataFormat->mPCM.bitsPerSample);
                    break;
                }

                // check the channel mask
                SL_LOGV("%s: Requested channel mask of 0x%x for %d channel audio",
                    name,
                    pDataFormat->mPCM.channelMask,
                    pDataFormat->mPCM.numChannels);

                if (pDataFormat->mPCM.channelMask == 0) {
                    // We can derive the channel mask from the channel count,
                    // but issue a warning--the automatic mask generation
                    // makes a lot of assumptions that may or may not be what
                    // the app was expecting.
                    SLuint32 mask = isOutputFormat
                            ? sles_channel_out_mask_from_count(pDataFormat->mPCM.numChannels)
                            : sles_channel_in_mask_from_count(pDataFormat->mPCM.numChannels);
                    if (mask == SL_ANDROID_UNKNOWN_CHANNELMASK) {
                        SL_LOGE("No channel mask specified and no default mapping for"
                                "requested speaker count of %u", pDataFormat->mPCM.numChannels);
                        result = SL_RESULT_PARAMETER_INVALID;
                    } else {
                        pDataFormat->mPCM.channelMask = mask;
                        SL_LOGW("No channel mask specified; Using mask %#x based on requested"
                                        "speaker count of %u",
                                pDataFormat->mPCM.channelMask,
                                pDataFormat->mPCM.numChannels);
                    }
                }

                SLuint32 mask = pDataFormat->mPCM.channelMask;
                SLuint32 count = sles_channel_count_from_mask(mask);
                if (count != pDataFormat->mPCM.numChannels) {
                    SL_LOGE("%s: requested %d channels but mask (0x%x) has %d channel bits set",
                        name,
                        pDataFormat->mPCM.numChannels,
                        mask,
                        count);
                    result = SL_RESULT_PARAMETER_INVALID;
                    break;
                }

                SL_LOGV("%s: final channel mask is 0x%x", name, pDataFormat->mPCM.channelMask);

                // check the endianness / byte order
                switch (pDataFormat->mPCM.endianness) {
                case SL_BYTEORDER_LITTLEENDIAN:
                case SL_BYTEORDER_BIGENDIAN:
                    break;
                // native is proposed but not yet in spec
                default:
                    result = SL_RESULT_PARAMETER_INVALID;
                    break;
                }
                if (SL_RESULT_SUCCESS != result) {
                    SL_LOGE("%s: endianness=%u", name, (unsigned) pDataFormat->mPCM.endianness);
                    break;
                }

                // here if all checks passed successfully

            } while(0);
            break;

        case SL_DATAFORMAT_MIME:
            pDataFormat->mMIME = *(SLDataFormat_MIME *)pFormat;
            if (NULL != pDataFormat->mMIME.mimeType) {
                // NTH check address for validity
                size_t len = strlen((const char *) pDataFormat->mMIME.mimeType);
                SLchar *myMIME = (SLchar *) malloc(len + 1);
                if (NULL == myMIME) {
                    result = SL_RESULT_MEMORY_FAILURE;
                } else {
                    memcpy(myMIME, pDataFormat->mMIME.mimeType, len + 1);
                    // make sure MIME string was not modified asynchronously
                    if ('\0' != myMIME[len]) {
                        free(myMIME);
                        myMIME = NULL;
                        result = SL_RESULT_PRECONDITIONS_VIOLATED;
                    }
                }
                pDataFormat->mMIME.mimeType = myMIME;
            }
            break;

        case XA_DATAFORMAT_RAWIMAGE:
            pDataFormat->mRawImage = *(XADataFormat_RawImage *)pFormat;
            switch (pDataFormat->mRawImage.colorFormat) {
            case XA_COLORFORMAT_MONOCHROME:
            case XA_COLORFORMAT_8BITRGB332:
            case XA_COLORFORMAT_12BITRGB444:
            case XA_COLORFORMAT_16BITARGB4444:
            case XA_COLORFORMAT_16BITARGB1555:
            case XA_COLORFORMAT_16BITRGB565:
            case XA_COLORFORMAT_16BITBGR565:
            case XA_COLORFORMAT_18BITRGB666:
            case XA_COLORFORMAT_18BITARGB1665:
            case XA_COLORFORMAT_19BITARGB1666:
            case XA_COLORFORMAT_24BITRGB888:
            case XA_COLORFORMAT_24BITBGR888:
            case XA_COLORFORMAT_24BITARGB1887:
            case XA_COLORFORMAT_25BITARGB1888:
            case XA_COLORFORMAT_32BITBGRA8888:
            case XA_COLORFORMAT_32BITARGB8888:
            case XA_COLORFORMAT_YUV411PLANAR:
            case XA_COLORFORMAT_YUV420PLANAR:
            case XA_COLORFORMAT_YUV420SEMIPLANAR:
            case XA_COLORFORMAT_YUV422PLANAR:
            case XA_COLORFORMAT_YUV422SEMIPLANAR:
            case XA_COLORFORMAT_YCBYCR:
            case XA_COLORFORMAT_YCRYCB:
            case XA_COLORFORMAT_CBYCRY:
            case XA_COLORFORMAT_CRYCBY:
            case XA_COLORFORMAT_YUV444INTERLEAVED:
            case XA_COLORFORMAT_RAWBAYER8BIT:
            case XA_COLORFORMAT_RAWBAYER10BIT:
            case XA_COLORFORMAT_RAWBAYER8BITCOMPRESSED:
            case XA_COLORFORMAT_L2:
            case XA_COLORFORMAT_L4:
            case XA_COLORFORMAT_L8:
            case XA_COLORFORMAT_L16:
            case XA_COLORFORMAT_L24:
            case XA_COLORFORMAT_L32:
            case XA_COLORFORMAT_18BITBGR666:
            case XA_COLORFORMAT_24BITARGB6666:
            case XA_COLORFORMAT_24BITABGR6666:
                break;
            case XA_COLORFORMAT_UNUSED:
            default:
                result = XA_RESULT_PARAMETER_INVALID;
                SL_LOGE("%s: unsupported color format %d", name,
                    pDataFormat->mRawImage.colorFormat);
                break;
            }
            // no checks for height, width, or stride
            break;

        default:
            result = SL_RESULT_PARAMETER_INVALID;
            SL_LOGE("%s: formatType=%u", name, (unsigned) formatType);
            break;

        }

        // make sure format type was not modified asynchronously
        if ((SL_RESULT_SUCCESS == result) && (formatType != pDataFormat->mFormatType)) {
            SL_LOGE("%s: formatType changed from %u to %u", name, formatType,
                    pDataFormat->mFormatType);
            result = SL_RESULT_PRECONDITIONS_VIOLATED;
        }

    }

    // Verify that the data format type is allowed in this context
    if (SL_RESULT_SUCCESS == result) {
        SLuint32 actualMask;
        switch (formatType) {
        case SL_DATAFORMAT_NULL:
        case SL_DATAFORMAT_MIME:
        case SL_DATAFORMAT_PCM:
        case SL_ANDROID_DATAFORMAT_PCM_EX:
        case XA_DATAFORMAT_RAWIMAGE:
            actualMask = 1L << formatType;
            break;
        default:
            assert(false);
            actualMask = 0L;
            break;
        }
        if (!(allowedDataFormatMask & actualMask)) {
            SL_LOGE("%s: data format %d not allowed", name, formatType);
            result = SL_RESULT_CONTENT_UNSUPPORTED;
        }
    }

    return result;
}


/** \brief Check interface ID compatibility with respect to a particular source
 *         and sink data locator format
 */

SLresult checkSourceSinkVsInterfacesCompatibility(const DataLocatorFormat *pSrcDataLocatorFormat,
        const DataLocatorFormat *pSinkDataLocatorFormat,
        const ClassTable *clazz, unsigned requiredMask) {
    int index;
    switch (pSrcDataLocatorFormat->mLocator.mLocatorType) {
    case SL_DATALOCATOR_URI:
#ifdef ANDROID
    case SL_DATALOCATOR_ANDROIDFD:
#endif
        // URIs and FD can be sources when "playing" to an OutputMix or a Buffer Queue for decode
        // so we don't prevent the retrieval of the BufferQueue interfaces for those sources
        switch (pSinkDataLocatorFormat->mLocator.mLocatorType) {
        case SL_DATALOCATOR_BUFFERQUEUE:
#ifdef ANDROID
        case SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE:
#endif
            break;
        default:
            // can't require SLBufferQueueItf or its alias SLAndroidSimpleBufferQueueItf
            // if the data sink is not a buffer queue
            index = clazz->mMPH_to_index[MPH_BUFFERQUEUE];
#ifdef ANDROID
            assert(index == clazz->mMPH_to_index[MPH_ANDROIDSIMPLEBUFFERQUEUE]);
#endif
            if (0 <= index) {
                if (requiredMask & (1 << index)) {
                    SL_LOGE("can't require SL_IID_BUFFERQUEUE "
#ifdef ANDROID
                            "or SL_IID_ANDROIDSIMPLEBUFFERQUEUE "
#endif
                            "with a non-buffer queue data sink");
                    return SL_RESULT_FEATURE_UNSUPPORTED;
                }
            }
            break;
        }
        break;

    case SL_DATALOCATOR_BUFFERQUEUE:
#ifdef ANDROID
    case SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE:
#endif
        // can't require SLSeekItf if data source is a buffer queue
        index = clazz->mMPH_to_index[MPH_SEEK];
        if (0 <= index) {
            if (requiredMask & (1 << index)) {
                SL_LOGE("can't require SL_IID_SEEK with a buffer queue data source");
                return SL_RESULT_FEATURE_UNSUPPORTED;
            }
        }
        // can't require SLMuteSoloItf if data source is a mono buffer queue
        index = clazz->mMPH_to_index[MPH_MUTESOLO];
        if (0 <= index) {
            if ((requiredMask & (1 << index)) &&
                    (SL_DATAFORMAT_PCM == pSrcDataLocatorFormat->mFormat.mFormatType) &&
                    (1 == pSrcDataLocatorFormat->mFormat.mPCM.numChannels)) {
                SL_LOGE("can't require SL_IID_MUTESOLO with a mono buffer queue data source");
                return SL_RESULT_FEATURE_UNSUPPORTED;
            }
        }
        break;

#ifdef ANDROID
    case SL_DATALOCATOR_ANDROIDBUFFERQUEUE:
        // can't require SLSeekItf if data source is an Android buffer queue
        index = clazz->mMPH_to_index[MPH_SEEK];
        if (0 <= index) {
            if (requiredMask & (1 << index)) {
                SL_LOGE("can't require SL_IID_SEEK with a SL_DATALOCATOR_ANDROIDBUFFERQUEUE "\
                        "source");
                return SL_RESULT_FEATURE_UNSUPPORTED;
            }
        }
        switch (pSinkDataLocatorFormat->mLocator.mLocatorType) {
        // for use-case AAC decode from SLAndroidBufferQueueItf with AAC ADTS data
        case SL_DATALOCATOR_BUFFERQUEUE:
        case SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE:
            break;
        // for use-case audio playback from SLAndroidBufferQueueItf with MP2TS data
        case SL_DATALOCATOR_OUTPUTMIX:
            break;
        default:
            SL_LOGE("Invalid sink for SL_DATALOCATOR_ANDROIDBUFFERQUEUE source");
            return SL_RESULT_FEATURE_UNSUPPORTED;
            break;
        }
        break;
#endif
    case SL_DATALOCATOR_ADDRESS:
    case SL_DATALOCATOR_MIDIBUFFERQUEUE:
    case XA_DATALOCATOR_NATIVEDISPLAY:
        // any special checks here???
    default:
        // can't require SLBufferQueueItf or its alias SLAndroidSimpleBufferQueueItf
        // if the data source is not a buffer queue
        index = clazz->mMPH_to_index[MPH_BUFFERQUEUE];
#ifdef ANDROID
        assert(index == clazz->mMPH_to_index[MPH_ANDROIDSIMPLEBUFFERQUEUE]);
#endif
        if (0 <= index) {
            if (requiredMask & (1 << index)) {
                SL_LOGE("can't require SL_IID_BUFFERQUEUE "
#ifdef ANDROID
                        "or SL_IID_ANDROIDSIMPLEBUFFERQUEUE "
#endif
                        "with a non-buffer queue data source");
                return SL_RESULT_FEATURE_UNSUPPORTED;
            }
        }
        break;
    }
    return SL_RESULT_SUCCESS;
}


/** \brief Free the local deep copy of a data format */

static void freeDataFormat(DataFormat *pDataFormat)
{
    switch (pDataFormat->mFormatType) {
    case SL_DATAFORMAT_MIME:
        if (NULL != pDataFormat->mMIME.mimeType) {
            free(pDataFormat->mMIME.mimeType);
            pDataFormat->mMIME.mimeType = NULL;
        }
        break;
    case SL_ANDROID_DATAFORMAT_PCM_EX:
    case SL_DATAFORMAT_PCM:
    case XA_DATAFORMAT_RAWIMAGE:
    case SL_DATAFORMAT_NULL:
        break;
    default:
        // an invalid data format is caught earlier during the copy
        assert(false);
        break;
    }
}


/** \brief Check a data source and make local deep copy */

SLresult checkDataSource(const char *name, const SLDataSource *pDataSrc,
        DataLocatorFormat *pDataLocatorFormat, SLuint32 allowedDataLocatorMask,
        SLuint32 allowedDataFormatMask)
{
    assert(NULL != name && NULL != pDataLocatorFormat);
    pDataLocatorFormat->u.mSource.pLocator = &pDataLocatorFormat->mLocator;
    pDataLocatorFormat->u.mSource.pFormat = &pDataLocatorFormat->mFormat;

    if (NULL == pDataSrc) {
        pDataLocatorFormat->mLocator.mLocatorType = SL_DATALOCATOR_NULL;
        pDataLocatorFormat->mFormat.mFormatType = SL_DATAFORMAT_NULL;
        if ((allowedDataLocatorMask & DATALOCATOR_MASK_NULL) &&
                (allowedDataFormatMask & DATAFORMAT_MASK_NULL)) {
            return SL_RESULT_SUCCESS;
        }
        SL_LOGE("%s: data source cannot be NULL", name);
        return SL_RESULT_PARAMETER_INVALID;
    }
    SLDataSource myDataSrc = *pDataSrc;
    SLresult result;
    result = checkDataLocator(name, myDataSrc.pLocator, &pDataLocatorFormat->mLocator,
            allowedDataLocatorMask);
    if (SL_RESULT_SUCCESS != result) {
        return result;
    }

    switch (pDataLocatorFormat->mLocator.mLocatorType) {
    case SL_DATALOCATOR_URI:
        allowedDataFormatMask &= DATAFORMAT_MASK_MIME;
        break;
    case SL_DATALOCATOR_ADDRESS:
    case SL_DATALOCATOR_BUFFERQUEUE:
        allowedDataFormatMask &= DATAFORMAT_MASK_PCM | DATAFORMAT_MASK_PCM_EX;
        break;
    // Per the spec, the pFormat field is ignored in some cases
    case SL_DATALOCATOR_IODEVICE:
        myDataSrc.pFormat = NULL;
        FALLTHROUGH_INTENDED;
    case SL_DATALOCATOR_NULL:
    case SL_DATALOCATOR_MIDIBUFFERQUEUE:
        allowedDataFormatMask &= DATAFORMAT_MASK_NULL;
        break;
    case SL_DATALOCATOR_OUTPUTMIX:
    case XA_DATALOCATOR_NATIVEDISPLAY:
        allowedDataFormatMask = DATAFORMAT_MASK_NONE;
        break;
#ifdef ANDROID
    case SL_DATALOCATOR_ANDROIDFD:
        allowedDataFormatMask &= DATAFORMAT_MASK_MIME;
        break;
    case SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE:
        allowedDataFormatMask &= DATAFORMAT_MASK_PCM | DATAFORMAT_MASK_PCM_EX;
        break;
    case SL_DATALOCATOR_ANDROIDBUFFERQUEUE:
        allowedDataFormatMask &= DATAFORMAT_MASK_MIME;;
        break;
#endif
    default:
        // invalid data locator type is caught earlier
        assert(false);
        allowedDataFormatMask = DATAFORMAT_MASK_NONE;
        break;
    }

    result = checkDataFormat(name,
                             myDataSrc.pFormat,
                             &pDataLocatorFormat->mFormat,
                             allowedDataFormatMask,
                             SL_BOOLEAN_TRUE /*isOutputFormat*/);
    if (SL_RESULT_SUCCESS != result) {
        freeDataLocator(&pDataLocatorFormat->mLocator);
        return result;
    }

    return SL_RESULT_SUCCESS;
}


/** \brief Check a data sink and make local deep copy */

SLresult checkDataSink(const char *name, const SLDataSink *pDataSink,
        DataLocatorFormat *pDataLocatorFormat, SLuint32 allowedDataLocatorMask,
        SLuint32 allowedDataFormatMask)
{
    assert(NULL != name && NULL != pDataLocatorFormat);
    pDataLocatorFormat->u.mSink.pLocator = &pDataLocatorFormat->mLocator;
    pDataLocatorFormat->u.mSink.pFormat = &pDataLocatorFormat->mFormat;

    if (NULL == pDataSink) {
        pDataLocatorFormat->mLocator.mLocatorType = SL_DATALOCATOR_NULL;
        pDataLocatorFormat->mFormat.mFormatType = SL_DATAFORMAT_NULL;
        if ((allowedDataLocatorMask & DATALOCATOR_MASK_NULL) &&
                (allowedDataFormatMask & DATAFORMAT_MASK_NULL)) {
            return SL_RESULT_SUCCESS;
        }
        SL_LOGE("%s: data sink cannot be NULL", name);
        return SL_RESULT_PARAMETER_INVALID;
    }
    SLDataSink myDataSink = *pDataSink;
    SLresult result;
    result = checkDataLocator(name, myDataSink.pLocator, &pDataLocatorFormat->mLocator,
            allowedDataLocatorMask);
    if (SL_RESULT_SUCCESS != result) {
        return result;
    }

    switch (pDataLocatorFormat->mLocator.mLocatorType) {
    case SL_DATALOCATOR_URI:
        allowedDataFormatMask &= DATAFORMAT_MASK_MIME;
        break;
    case SL_DATALOCATOR_ADDRESS:
    case SL_DATALOCATOR_BUFFERQUEUE:
        allowedDataFormatMask &= DATAFORMAT_MASK_PCM | DATAFORMAT_MASK_PCM_EX;
        break;
    // Per the spec, the pFormat field is ignored in some cases
    case SL_DATALOCATOR_IODEVICE:
    case SL_DATALOCATOR_OUTPUTMIX:
    case XA_DATALOCATOR_NATIVEDISPLAY:
        myDataSink.pFormat = NULL;
        FALLTHROUGH_INTENDED;
    case SL_DATALOCATOR_NULL:
    case SL_DATALOCATOR_MIDIBUFFERQUEUE:
        allowedDataFormatMask &= DATAFORMAT_MASK_NULL;
        break;
#ifdef ANDROID
    case SL_DATALOCATOR_ANDROIDFD:
        allowedDataFormatMask = DATAFORMAT_MASK_NONE;
        break;
    case SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE:
        allowedDataFormatMask &= DATAFORMAT_MASK_PCM | DATAFORMAT_MASK_PCM_EX;
        break;
    case SL_DATALOCATOR_ANDROIDBUFFERQUEUE:
        allowedDataFormatMask = DATAFORMAT_MASK_NONE;
        break;
#endif
    default:
        // invalid data locator type is caught earlier
        assert(false);
        allowedDataFormatMask = DATAFORMAT_MASK_NONE;
        break;
    }

    result = checkDataFormat(name,
            myDataSink.pFormat,
            &pDataLocatorFormat->mFormat,
            allowedDataFormatMask,
            SL_BOOLEAN_FALSE /*isOutputFormat*/);

    if (SL_RESULT_SUCCESS != result) {
        freeDataLocator(&pDataLocatorFormat->mLocator);
        return result;
    }

    return SL_RESULT_SUCCESS;
}


/** \brief Free the local deep copy of a data locator format */

void freeDataLocatorFormat(DataLocatorFormat *dlf)
{
    assert(NULL != dlf);
    freeDataLocator(&dlf->mLocator);
    freeDataFormat(&dlf->mFormat);
}
