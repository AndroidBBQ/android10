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

/* MetadataExtraction implementation */

#include "sles_allinclusive.h"


static SLresult IMetadataExtraction_GetItemCount(SLMetadataExtractionItf self, SLuint32 *pItemCount)
{
    SL_ENTER_INTERFACE

    if (NULL == pItemCount) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IMetadataExtraction *thiz = (IMetadataExtraction *) self;
        if (SL_OBJECTID_AUDIOPLAYER == InterfaceToObjectID(thiz)) {
#ifdef ANDROID
            result = android_audioPlayer_metadata_getItemCount((CAudioPlayer *)thiz->mThis,
                    pItemCount);
#else
            result = SL_RESULT_FEATURE_UNSUPPORTED;
#endif
        } else {
            result = SL_RESULT_PARAMETER_INVALID;
        }
    }

    SL_LEAVE_INTERFACE
}


static SLresult IMetadataExtraction_GetKeySize(SLMetadataExtractionItf self,
    SLuint32 index, SLuint32 *pKeySize)
{
    SL_ENTER_INTERFACE

    if (NULL == pKeySize) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IMetadataExtraction *thiz = (IMetadataExtraction *) self;
        if (SL_OBJECTID_AUDIOPLAYER == InterfaceToObjectID(thiz)) {
#ifdef ANDROID
            result = android_audioPlayer_metadata_getKeySize((CAudioPlayer *)thiz->mThis,
                    index, pKeySize);
#else
            result = SL_RESULT_FEATURE_UNSUPPORTED;
#endif
        } else {
            result = SL_RESULT_PARAMETER_INVALID;
        }
    }

    SL_LEAVE_INTERFACE
}


static SLresult IMetadataExtraction_GetKey(SLMetadataExtractionItf self,
    SLuint32 index, SLuint32 keySize, SLMetadataInfo *pKey)
{
    SL_ENTER_INTERFACE

    if (NULL == pKey) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IMetadataExtraction *thiz = (IMetadataExtraction *) self;
        if (SL_OBJECTID_AUDIOPLAYER == InterfaceToObjectID(thiz)) {
#ifdef ANDROID
            result = android_audioPlayer_metadata_getKey((CAudioPlayer *)thiz->mThis,
                    index, keySize, pKey);
#else
            result = SL_RESULT_FEATURE_UNSUPPORTED;
#endif
        } else {
            result = SL_RESULT_PARAMETER_INVALID;
        }
    }

    SL_LEAVE_INTERFACE
}


static SLresult IMetadataExtraction_GetValueSize(SLMetadataExtractionItf self,
    SLuint32 index, SLuint32 *pValueSize)
{
    SL_ENTER_INTERFACE

    if (NULL == pValueSize) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IMetadataExtraction *thiz = (IMetadataExtraction *) self;
        if (SL_OBJECTID_AUDIOPLAYER == InterfaceToObjectID(thiz)) {
#ifdef ANDROID
            result = android_audioPlayer_metadata_getValueSize((CAudioPlayer *)thiz->mThis,
                    index, pValueSize);
#else
            result = SL_RESULT_FEATURE_UNSUPPORTED;
#endif
        } else {
            result = SL_RESULT_PARAMETER_INVALID;
        }
    }

    SL_LEAVE_INTERFACE
}


static SLresult IMetadataExtraction_GetValue(SLMetadataExtractionItf self,
    SLuint32 index, SLuint32 valueSize, SLMetadataInfo *pValue)
{
    SL_ENTER_INTERFACE

    if (NULL == pValue) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IMetadataExtraction *thiz = (IMetadataExtraction *) self;
        if (SL_OBJECTID_AUDIOPLAYER == InterfaceToObjectID(thiz)) {
#ifdef ANDROID
            result = android_audioPlayer_metadata_getValue((CAudioPlayer *)thiz->mThis,
                    index, valueSize, pValue);
#else
            result = SL_RESULT_FEATURE_UNSUPPORTED;
#endif
        } else {
            result = SL_RESULT_PARAMETER_INVALID;
        }
    }

    SL_LEAVE_INTERFACE
}


static SLresult IMetadataExtraction_AddKeyFilter(SLMetadataExtractionItf self,
    SLuint32 keySize, const void *pKey, SLuint32 keyEncoding,
    const SLchar *pValueLangCountry, SLuint32 valueEncoding, SLuint8 filterMask)
{
    SL_ENTER_INTERFACE

    if (NULL == pKey || NULL == pValueLangCountry || (filterMask & ~(SL_METADATA_FILTER_KEY |
            SL_METADATA_FILTER_LANG | SL_METADATA_FILTER_ENCODING))) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IMetadataExtraction *thiz = (IMetadataExtraction *) self;
        interface_lock_exclusive(thiz);
        thiz->mKeySize = keySize;
        thiz->mKey = pKey;
        thiz->mKeyEncoding = keyEncoding;
        thiz->mValueLangCountry = pValueLangCountry; // should make a local copy
        thiz->mValueEncoding = valueEncoding;
        thiz->mFilterMask = filterMask;
        interface_unlock_exclusive(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IMetadataExtraction_ClearKeyFilter(SLMetadataExtractionItf self)
{
    SL_ENTER_INTERFACE

    IMetadataExtraction *thiz = (IMetadataExtraction *) self;
    thiz->mKeyFilter = 0;
    result = SL_RESULT_SUCCESS;

    SL_LEAVE_INTERFACE
}


static const struct SLMetadataExtractionItf_ IMetadataExtraction_Itf = {
    IMetadataExtraction_GetItemCount,
    IMetadataExtraction_GetKeySize,
    IMetadataExtraction_GetKey,
    IMetadataExtraction_GetValueSize,
    IMetadataExtraction_GetValue,
    IMetadataExtraction_AddKeyFilter,
    IMetadataExtraction_ClearKeyFilter
};

void IMetadataExtraction_init(void *self)
{
    IMetadataExtraction *thiz = (IMetadataExtraction *) self;
    thiz->mItf = &IMetadataExtraction_Itf;
    thiz->mKeySize = 0;
    thiz->mKey = NULL;
    thiz->mKeyEncoding = 0 /*TBD*/;
    thiz->mValueLangCountry = 0 /*TBD*/;
    thiz->mValueEncoding = 0 /*TBD*/;
    thiz->mFilterMask = 0 /*TBD*/;
    thiz->mKeyFilter = 0;
}
