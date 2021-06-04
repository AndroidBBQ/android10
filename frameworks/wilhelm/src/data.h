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

#ifndef WILHELM_SRC_DATA_H
#define WILHELM_SRC_DATA_H

/* Our own merged version of SLDataSource and SLDataSink */

typedef union {
    SLuint32 mLocatorType;
    SLDataLocator_Address mAddress;
    SLDataLocator_BufferQueue mBufferQueue;
    SLDataLocator_IODevice mIODevice;
    SLDataLocator_MIDIBufferQueue mMIDIBufferQueue;
    SLDataLocator_OutputMix mOutputMix;
    SLDataLocator_URI mURI;
    XADataLocator_NativeDisplay mNativeDisplay;
#ifdef ANDROID
    SLDataLocator_AndroidFD mFD;
    SLDataLocator_AndroidBufferQueue mABQ;
#endif
} DataLocator;

typedef union {
    SLuint32 mFormatType;
    SLDataFormat_PCM mPCM;
    SLAndroidDataFormat_PCM_EX mPCMEx;
    SLDataFormat_MIME mMIME;
    XADataFormat_RawImage mRawImage;
} DataFormat;

typedef struct {
    union {
        SLDataSource mSource;
        SLDataSink mSink;
        struct {
            DataLocator *pLocator;
            DataFormat *pFormat;
        } mNeutral;
    } u;
    DataLocator mLocator;
    DataFormat mFormat;
} DataLocatorFormat;

#define SL_DATALOCATOR_NULL 0   // application specified a NULL value for pLocator
                                // (not a valid value for mLocatorType)
#define XA_DATALOCATOR_NULL SL_DATALOCATOR_NULL
#define SL_DATAFORMAT_NULL 0    // application specified a NULL value for pFormat
                                // (not a valid value for mLocatorType)
#define XA_DATAFORMAT_NULL SL_DATAFORMAT_NULL

// bit masks used to configure the allowed data locators for a given data source or data sink
#define DATALOCATOR_MASK_NONE            0L
#define DATALOCATOR_MASK_NULL            (1L << SL_DATALOCATOR_NULL)
#define DATALOCATOR_MASK_URI             (1L << SL_DATALOCATOR_URI)
#define DATALOCATOR_MASK_ADDRESS         (1L << SL_DATALOCATOR_ADDRESS)
#define DATALOCATOR_MASK_IODEVICE        (1L << SL_DATALOCATOR_IODEVICE)
#define DATALOCATOR_MASK_OUTPUTMIX       (1L << SL_DATALOCATOR_OUTPUTMIX)
#define DATALOCATOR_MASK_NATIVEDISPLAY   (1L << XA_DATALOCATOR_NATIVEDISPLAY)
#define DATALOCATOR_MASK_BUFFERQUEUE     (1L << SL_DATALOCATOR_BUFFERQUEUE)
#define DATALOCATOR_MASK_MIDIBUFFERQUEUE (1L << SL_DATALOCATOR_MIDIBUFFERQUEUE)
#define DATALOCATOR_MASK_ANDROIDFD                \
                 (0x100L << (SL_DATALOCATOR_ANDROIDFD - SL_DATALOCATOR_ANDROIDFD))
#define DATALOCATOR_MASK_ANDROIDSIMPLEBUFFERQUEUE \
                 (0x100L << (SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE - SL_DATALOCATOR_ANDROIDFD))
#define DATALOCATOR_MASK_ANDROIDBUFFERQUEUE       \
                 (0x100L << (SL_DATALOCATOR_ANDROIDBUFFERQUEUE - SL_DATALOCATOR_ANDROIDFD))
#define DATALOCATOR_MASK_ALL             0x7FFL

// bit masks used to configure the allowed data formats for a given data source or data sink
#define DATAFORMAT_MASK_NONE             0L
#define DATAFORMAT_MASK_NULL             (1L << SL_DATAFORMAT_NULL)
#define DATAFORMAT_MASK_MIME             (1L << SL_DATAFORMAT_MIME)
#define DATAFORMAT_MASK_PCM              (1L << SL_DATAFORMAT_PCM)
#define DATAFORMAT_MASK_PCM_EX           (1L << SL_ANDROID_DATAFORMAT_PCM_EX)
#define DATAFORMAT_MASK_RAWIMAGE         (1L << XA_DATAFORMAT_RAWIMAGE)
#define DATAFORMAT_MASK_ALL              0xFL

extern SLresult checkDataSource(const char *name, const SLDataSource *pDataSrc,
        DataLocatorFormat *myDataSourceLocator, SLuint32 allowedDataLocatorMask,
        SLuint32 allowedDataFormatMask);
extern SLresult checkDataSink(const char *name, const SLDataSink *pDataSink,
        DataLocatorFormat *myDataSinkLocator, SLuint32 allowedDataLocatorMask,
        SLuint32 allowedDataFormatMask);
extern SLresult checkSourceSinkVsInterfacesCompatibility(
        const DataLocatorFormat *pSrcDataLocatorFormat,
        const DataLocatorFormat *pSinkDataLocatorFormat,
        const ClassTable *clazz, unsigned requiredMask);
extern void freeDataLocatorFormat(DataLocatorFormat *dlf);


/* For stream information storage */
typedef struct {
    XAuint32 domain;
    union {
        XAMediaContainerInformation containerInfo;
        XAVideoStreamInformation videoInfo;
        XAAudioStreamInformation audioInfo;
        XAImageStreamInformation imageInfo;
        XATimedTextStreamInformation textInfo;
        XAMIDIStreamInformation midiInfo;
        XAVendorStreamInformation vendorInfo;
    };
} StreamInfo;

// FIXME a terrible hack until OpenMAX AL spec is updated
#define XA_DOMAINTYPE_CONTAINER 0

#endif // WILHELM_SRC_DATA_H