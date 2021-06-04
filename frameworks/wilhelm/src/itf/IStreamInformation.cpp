/*
 * Copyright (C) 2011 The Android Open Source Project
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

/* StreamInformation implementation */

#include "sles_allinclusive.h"

static XAresult IStreamInformation_QueryMediaContainerInformation( XAStreamInformationItf self,
        XAMediaContainerInformation * info /* [out] */)
{
    XA_ENTER_INTERFACE

    if (NULL == info) {
        result = XA_RESULT_PARAMETER_INVALID;
        XA_LEAVE_INTERFACE
    }

#ifdef ANDROID
    IStreamInformation *thiz = (IStreamInformation *) self;
    interface_lock_shared(thiz);
    // always storing container info at index 0, as per spec
    *info = thiz->mStreamInfoTable.itemAt(0).containerInfo;
    interface_unlock_shared(thiz);
    // even though the pointer to the media container info is returned, the values aren't set
    //  for the actual container in this version, they are simply initialized to defaults
    //  (see IStreamInformation_init)
    result = XA_RESULT_SUCCESS;
#else
    SL_LOGE("QueryMediaContainerInformation is unsupported");
    memset(info, 0, sizeof(XAMediaContainerInformation));
    result = XA_RESULT_FEATURE_UNSUPPORTED;
#endif

    XA_LEAVE_INTERFACE
}


static XAresult IStreamInformation_QueryStreamType( XAStreamInformationItf self,
        XAuint32 streamIndex, /* [in] */
        XAuint32 *domain)     /* [out] */
{
    XA_ENTER_INTERFACE

    if (NULL == domain) {
        result = XA_RESULT_PARAMETER_INVALID;
        XA_LEAVE_INTERFACE;
    }

#ifndef ANDROID
    *domain = XA_DOMAINTYPE_UNKNOWN;
#else
    if (0 == streamIndex) {
        // stream 0 is reserved for the container
        result = XA_RESULT_PARAMETER_INVALID;
        *domain = XA_DOMAINTYPE_UNKNOWN;
    } else {
        IStreamInformation *thiz = (IStreamInformation *) self;

        interface_lock_shared(thiz);

        XAuint32 nbStreams = thiz->mStreamInfoTable.itemAt(0).containerInfo.numStreams;
        // streams in the container are numbered 1..nbStreams
        if (streamIndex <= nbStreams) {
            result = XA_RESULT_SUCCESS;
            *domain = thiz->mStreamInfoTable.itemAt(streamIndex).domain;
        } else {
            SL_LOGE("Querying stream type for stream %d, only %d streams available",
                    streamIndex, nbStreams);
            result = XA_RESULT_PARAMETER_INVALID;
        }

        interface_unlock_shared(thiz);
    }
#endif

    XA_LEAVE_INTERFACE
}


static XAresult IStreamInformation_QueryStreamInformation( XAStreamInformationItf self,
        XAuint32 streamIndex, /* [in] */
        void * info)          /* [out] */
{
    XA_ENTER_INTERFACE

    if (NULL == info) {
        result = XA_RESULT_PARAMETER_INVALID;
    } else {

#ifndef ANDROID
        result = XA_RESULT_FEATURE_UNSUPPORTED;
#else

        IStreamInformation *thiz = (IStreamInformation *) self;

        interface_lock_shared(thiz);

        XAuint32 nbStreams = thiz->mStreamInfoTable.itemAt(0).containerInfo.numStreams;
        // stream 0 is the container, and other streams in the container are numbered 1..nbStreams
        if (streamIndex <= nbStreams) {
            result = XA_RESULT_SUCCESS;
            const StreamInfo& streamInfo = thiz->mStreamInfoTable.itemAt((size_t)streamIndex);

            switch (streamInfo.domain) {
            case XA_DOMAINTYPE_CONTAINER:
                *(XAMediaContainerInformation *)info = streamInfo.containerInfo;
                break;
            case XA_DOMAINTYPE_AUDIO:
                *(XAAudioStreamInformation *)info = streamInfo.audioInfo;
                break;
            case XA_DOMAINTYPE_VIDEO:
                *(XAVideoStreamInformation *)info = streamInfo.videoInfo;
                break;
            case XA_DOMAINTYPE_IMAGE:
                *(XAImageStreamInformation *)info = streamInfo.imageInfo;
                break;
            case XA_DOMAINTYPE_TIMEDTEXT:
                *(XATimedTextStreamInformation *)info = streamInfo.textInfo;
                break;
            case XA_DOMAINTYPE_MIDI:
                *(XAMIDIStreamInformation *)info = streamInfo.midiInfo;
                break;
            case XA_DOMAINTYPE_VENDOR:
                *(XAVendorStreamInformation *)info = streamInfo.vendorInfo;
                break;
            default:
                SL_LOGE("StreamInformation::QueryStreamInformation index %u has "
                        "unknown domain %u", streamIndex, streamInfo.domain);
                result = XA_RESULT_INTERNAL_ERROR;
                break;
            }

        } else {
            SL_LOGE("Querying stream type for stream %d, only %d streams available",
                    streamIndex, nbStreams);
            result = XA_RESULT_PARAMETER_INVALID;
        }

        interface_unlock_shared(thiz);
#endif

    }

    XA_LEAVE_INTERFACE
}


static XAresult IStreamInformation_QueryStreamName( XAStreamInformationItf self,
        XAuint32 streamIndex, /* [in] */
        XAuint16 * pNameSize, /* [in/out] */
        XAchar * pName)       /* [out] */
{
    XA_ENTER_INTERFACE

    if (NULL == pNameSize || streamIndex == 0) {
        result = XA_RESULT_PARAMETER_INVALID;
    } else {
#ifdef ANDROID
        IStreamInformation *thiz = (IStreamInformation *) self;
        interface_lock_shared(thiz);

        XAuint32 nbStreams = thiz->mStreamInfoTable.itemAt(0).containerInfo.numStreams;
        // streams in the container are numbered 1..nbStreams
        if (streamIndex <= nbStreams) {
            char streamName[16];        // large enough for the fixed format in next line
            snprintf(streamName, sizeof(streamName), "stream%u", streamIndex);
            size_t actualNameLength = strlen(streamName);
            if (NULL == pName) {
                // application is querying the name length in order to allocate a buffer
                result = XA_RESULT_SUCCESS;
            } else {
                SLuint16 availableNameLength = *pNameSize;
                if (actualNameLength > availableNameLength) {
                    memcpy(pName, streamName, availableNameLength);
                    result = XA_RESULT_BUFFER_INSUFFICIENT;
                } else if (actualNameLength == availableNameLength) {
                    memcpy(pName, streamName, availableNameLength);
                    result = XA_RESULT_SUCCESS;
                } else { // actualNameLength < availableNameLength
                    memcpy(pName, streamName, actualNameLength + 1);
                    result = XA_RESULT_SUCCESS;
                }
            }
            *pNameSize = actualNameLength;
        } else {
            result = XA_RESULT_PARAMETER_INVALID;
        }

        interface_unlock_shared(thiz);
#else
        SL_LOGE("unsupported XAStreamInformationItf function");
        result = XA_RESULT_FEATURE_UNSUPPORTED;
#endif
    }

    XA_LEAVE_INTERFACE
}


static XAresult IStreamInformation_RegisterStreamChangeCallback( XAStreamInformationItf self,
        xaStreamEventChangeCallback callback, /* [in] */
        void * pContext)                      /* [in] */
{
    XA_ENTER_INTERFACE

    IStreamInformation *thiz = (IStreamInformation *) self;

    interface_lock_exclusive(thiz);

    thiz->mCallback = callback;
    thiz->mContext = pContext;
    result = SL_RESULT_SUCCESS;

    interface_unlock_exclusive(thiz);

    XA_LEAVE_INTERFACE
}


static XAresult IStreamInformation_QueryActiveStreams( XAStreamInformationItf self,
        XAuint32 *numStreams,      /* [in/out] */
        XAboolean *activeStreams)  /* [out] */
{
    XA_ENTER_INTERFACE

    if (NULL == numStreams) {
        result = XA_RESULT_PARAMETER_INVALID;
        XA_LEAVE_INTERFACE;
    }

#ifdef ANDROID
    IStreamInformation *thiz = (IStreamInformation *) self;
    interface_lock_shared(thiz);

    result = XA_RESULT_SUCCESS;
    *numStreams = thiz->mStreamInfoTable.itemAt(0).containerInfo.numStreams;
    // FIXME not setting activeStreams

    interface_unlock_shared(thiz);
#else
    result = SL_RESULT_FEATURE_UNSUPPORTED;
#endif

    XA_LEAVE_INTERFACE
}


static XAresult IStreamInformation_SetActiveStream( XAStreamInformationItf self,
        XAuint32   streamNum, /* [in] */
        XAboolean  active,    /* [in] */
        XAboolean  commitNow) /* [in] */
{
    XA_ENTER_INTERFACE

    SL_LOGE("unsupported XAStreamInformationItf function");
    result = XA_RESULT_FEATURE_UNSUPPORTED;

    XA_LEAVE_INTERFACE
}


static const struct XAStreamInformationItf_ IStreamInformation_Itf = {
    IStreamInformation_QueryMediaContainerInformation,
    IStreamInformation_QueryStreamType,
    IStreamInformation_QueryStreamInformation,
    IStreamInformation_QueryStreamName,
    IStreamInformation_RegisterStreamChangeCallback,
    IStreamInformation_QueryActiveStreams,
    IStreamInformation_SetActiveStream
};


void IStreamInformation_init(void *self)
{
    SL_LOGV("IStreamInformation_init\n");
    IStreamInformation *thiz = (IStreamInformation *) self;
    thiz->mItf = &IStreamInformation_Itf;

    thiz->mCallback = NULL;
    thiz->mContext = NULL;

    for (int i=0 ; i < NB_SUPPORTED_STREAMS ; i++) {
        thiz->mActiveStreams[i] = XA_BOOLEAN_FALSE;
    }

#ifdef ANDROID
    // placement new constructor for C++ field within C struct
    (void) new (&thiz->mStreamInfoTable) android::Vector<StreamInfo>();
    // initialize container info
    StreamInfo contInf;
    contInf.domain = XA_DOMAINTYPE_CONTAINER;
    contInf.containerInfo.containerType = XA_CONTAINERTYPE_UNSPECIFIED;
    contInf.containerInfo.mediaDuration = XA_TIME_UNKNOWN;
    // FIXME shouldn't this be 1 ?
    contInf.containerInfo.numStreams = 0;
    // always storing container info at index 0, as per spec: here, the table was still empty
    thiz->mStreamInfoTable.add(contInf);
#endif
}


void IStreamInformation_deinit(void *self) {
#ifdef ANDROID
    IStreamInformation *thiz = (IStreamInformation *) self;
    // explicit destructor
    thiz->mStreamInfoTable.~Vector<StreamInfo>();
#endif
}
