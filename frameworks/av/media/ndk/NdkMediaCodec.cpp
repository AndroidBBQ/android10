/*
 * Copyright (C) 2014 The Android Open Source Project
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

#include <inttypes.h>

//#define LOG_NDEBUG 0
#define LOG_TAG "NdkMediaCodec"

#include <media/NdkMediaCodec.h>
#include <media/NdkMediaError.h>
#include <media/NdkMediaFormatPriv.h>
#include "NdkMediaCryptoPriv.h"

#include <utils/Log.h>
#include <utils/StrongPointer.h>
#include <gui/Surface.h>

#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>

#include <media/stagefright/PersistentSurface.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/MediaErrors.h>
#include <media/MediaCodecBuffer.h>
#include <android/native_window.h>

using namespace android;


static media_status_t translate_error(status_t err) {
    if (err == OK) {
        return AMEDIA_OK;
    } else if (err == -EAGAIN) {
        return (media_status_t) AMEDIACODEC_INFO_TRY_AGAIN_LATER;
    }
    ALOGE("sf error code: %d", err);
    return AMEDIA_ERROR_UNKNOWN;
}

enum {
    kWhatActivityNotify,
    kWhatAsyncNotify,
    kWhatRequestActivityNotifications,
    kWhatStopActivityNotifications,
};

struct AMediaCodecPersistentSurface : public Surface {
    sp<PersistentSurface> mPersistentSurface;
    AMediaCodecPersistentSurface(
            const sp<IGraphicBufferProducer>& igbp,
            const sp<PersistentSurface>& ps)
            : Surface(igbp) {
        mPersistentSurface = ps;
    }
    virtual ~AMediaCodecPersistentSurface() {
        //mPersistentSurface ref will be let go off here
    }
};

class CodecHandler: public AHandler {
private:
    AMediaCodec* mCodec;
public:
    explicit CodecHandler(AMediaCodec *codec);
    virtual void onMessageReceived(const sp<AMessage> &msg);
};

typedef void (*OnCodecEvent)(AMediaCodec *codec, void *userdata);

struct AMediaCodec {
    sp<android::MediaCodec> mCodec;
    sp<ALooper> mLooper;
    sp<CodecHandler> mHandler;
    sp<AMessage> mActivityNotification;
    int32_t mGeneration;
    bool mRequestedActivityNotification;
    OnCodecEvent mCallback;
    void *mCallbackUserData;

    sp<AMessage> mAsyncNotify;
    mutable Mutex mAsyncCallbackLock;
    AMediaCodecOnAsyncNotifyCallback mAsyncCallback;
    void *mAsyncCallbackUserData;
};

CodecHandler::CodecHandler(AMediaCodec *codec) {
    mCodec = codec;
}

void CodecHandler::onMessageReceived(const sp<AMessage> &msg) {

    switch (msg->what()) {
        case kWhatRequestActivityNotifications:
        {
            if (mCodec->mRequestedActivityNotification) {
                break;
            }

            mCodec->mCodec->requestActivityNotification(mCodec->mActivityNotification);
            mCodec->mRequestedActivityNotification = true;
            break;
        }

        case kWhatActivityNotify:
        {
            {
                int32_t generation;
                msg->findInt32("generation", &generation);

                if (generation != mCodec->mGeneration) {
                    // stale
                    break;
                }

                mCodec->mRequestedActivityNotification = false;
            }

            if (mCodec->mCallback) {
                mCodec->mCallback(mCodec, mCodec->mCallbackUserData);
            }
            break;
        }

        case kWhatAsyncNotify:
        {
             int32_t cbID;
             if (!msg->findInt32("callbackID", &cbID)) {
                 ALOGE("kWhatAsyncNotify: callbackID is expected.");
                 break;
             }

             ALOGV("kWhatAsyncNotify: cbID = %d", cbID);

             switch (cbID) {
                 case MediaCodec::CB_INPUT_AVAILABLE:
                 {
                     int32_t index;
                     if (!msg->findInt32("index", &index)) {
                         ALOGE("CB_INPUT_AVAILABLE: index is expected.");
                         break;
                     }

                     Mutex::Autolock _l(mCodec->mAsyncCallbackLock);
                     if (mCodec->mAsyncCallbackUserData != NULL
                         || mCodec->mAsyncCallback.onAsyncInputAvailable != NULL) {
                         mCodec->mAsyncCallback.onAsyncInputAvailable(
                                 mCodec,
                                 mCodec->mAsyncCallbackUserData,
                                 index);
                     }

                     break;
                 }

                 case MediaCodec::CB_OUTPUT_AVAILABLE:
                 {
                     int32_t index;
                     size_t offset;
                     size_t size;
                     int64_t timeUs;
                     int32_t flags;

                     if (!msg->findInt32("index", &index)) {
                         ALOGE("CB_OUTPUT_AVAILABLE: index is expected.");
                         break;
                     }
                     if (!msg->findSize("offset", &offset)) {
                         ALOGE("CB_OUTPUT_AVAILABLE: offset is expected.");
                         break;
                     }
                     if (!msg->findSize("size", &size)) {
                         ALOGE("CB_OUTPUT_AVAILABLE: size is expected.");
                         break;
                     }
                     if (!msg->findInt64("timeUs", &timeUs)) {
                         ALOGE("CB_OUTPUT_AVAILABLE: timeUs is expected.");
                         break;
                     }
                     if (!msg->findInt32("flags", &flags)) {
                         ALOGE("CB_OUTPUT_AVAILABLE: flags is expected.");
                         break;
                     }

                     AMediaCodecBufferInfo bufferInfo = {
                         (int32_t)offset,
                         (int32_t)size,
                         timeUs,
                         (uint32_t)flags};

                     Mutex::Autolock _l(mCodec->mAsyncCallbackLock);
                     if (mCodec->mAsyncCallbackUserData != NULL
                         || mCodec->mAsyncCallback.onAsyncOutputAvailable != NULL) {
                         mCodec->mAsyncCallback.onAsyncOutputAvailable(
                                 mCodec,
                                 mCodec->mAsyncCallbackUserData,
                                 index,
                                 &bufferInfo);
                     }

                     break;
                 }

                 case MediaCodec::CB_OUTPUT_FORMAT_CHANGED:
                 {
                     sp<AMessage> format;
                     if (!msg->findMessage("format", &format)) {
                         ALOGE("CB_OUTPUT_FORMAT_CHANGED: format is expected.");
                         break;
                     }

                     // Here format is MediaCodec's internal copy of output format.
                     // Make a copy since the client might modify it.
                     sp<AMessage> copy;
                     if (format != nullptr) {
                         copy = format->dup();
                     }
                     AMediaFormat *aMediaFormat = AMediaFormat_fromMsg(&copy);

                     Mutex::Autolock _l(mCodec->mAsyncCallbackLock);
                     if (mCodec->mAsyncCallbackUserData != NULL
                         || mCodec->mAsyncCallback.onAsyncFormatChanged != NULL) {
                         mCodec->mAsyncCallback.onAsyncFormatChanged(
                                 mCodec,
                                 mCodec->mAsyncCallbackUserData,
                                 aMediaFormat);
                     }

                     break;
                 }

                 case MediaCodec::CB_ERROR:
                 {
                     status_t err;
                     int32_t actionCode;
                     AString detail;
                     if (!msg->findInt32("err", &err)) {
                         ALOGE("CB_ERROR: err is expected.");
                         break;
                     }
                     if (!msg->findInt32("action", &actionCode)) {
                         ALOGE("CB_ERROR: action is expected.");
                         break;
                     }
                     msg->findString("detail", &detail);
                     ALOGE("Decoder reported error(0x%x), actionCode(%d), detail(%s)",
                           err, actionCode, detail.c_str());

                     Mutex::Autolock _l(mCodec->mAsyncCallbackLock);
                     if (mCodec->mAsyncCallbackUserData != NULL
                         || mCodec->mAsyncCallback.onAsyncError != NULL) {
                         mCodec->mAsyncCallback.onAsyncError(
                                 mCodec,
                                 mCodec->mAsyncCallbackUserData,
                                 translate_error(err),
                                 actionCode,
                                 detail.c_str());
                     }

                     break;
                 }

                 default:
                 {
                     ALOGE("kWhatAsyncNotify: callbackID(%d) is unexpected.", cbID);
                     break;
                 }
             }
             break;
        }

        case kWhatStopActivityNotifications:
        {
            sp<AReplyToken> replyID;
            msg->senderAwaitsResponse(&replyID);

            mCodec->mGeneration++;
            mCodec->mRequestedActivityNotification = false;

            sp<AMessage> response = new AMessage;
            response->postReply(replyID);
            break;
        }

        default:
            ALOGE("shouldn't be here");
            break;
    }

}


static void requestActivityNotification(AMediaCodec *codec) {
    (new AMessage(kWhatRequestActivityNotifications, codec->mHandler))->post();
}

extern "C" {

static AMediaCodec * createAMediaCodec(const char *name, bool name_is_type, bool encoder) {
    AMediaCodec *mData = new AMediaCodec();
    mData->mLooper = new ALooper;
    mData->mLooper->setName("NDK MediaCodec_looper");
    size_t res = mData->mLooper->start(
            false,      // runOnCallingThread
            true,       // canCallJava XXX
            PRIORITY_AUDIO);
    if (res != OK) {
        ALOGE("Failed to start the looper");
        AMediaCodec_delete(mData);
        return NULL;
    }
    if (name_is_type) {
        mData->mCodec = android::MediaCodec::CreateByType(mData->mLooper, name, encoder);
    } else {
        mData->mCodec = android::MediaCodec::CreateByComponentName(mData->mLooper, name);
    }
    if (mData->mCodec == NULL) {  // failed to create codec
        AMediaCodec_delete(mData);
        return NULL;
    }
    mData->mHandler = new CodecHandler(mData);
    mData->mLooper->registerHandler(mData->mHandler);
    mData->mGeneration = 1;
    mData->mRequestedActivityNotification = false;
    mData->mCallback = NULL;

    mData->mAsyncCallback = {};
    mData->mAsyncCallbackUserData = NULL;

    return mData;
}

EXPORT
AMediaCodec* AMediaCodec_createCodecByName(const char *name) {
    return createAMediaCodec(name, false, false);
}

EXPORT
AMediaCodec* AMediaCodec_createDecoderByType(const char *mime_type) {
    return createAMediaCodec(mime_type, true, false);
}

EXPORT
AMediaCodec* AMediaCodec_createEncoderByType(const char *name) {
    return createAMediaCodec(name, true, true);
}

EXPORT
media_status_t AMediaCodec_delete(AMediaCodec *mData) {
    if (mData != NULL) {
        if (mData->mCodec != NULL) {
            mData->mCodec->release();
            mData->mCodec.clear();
        }

        if (mData->mLooper != NULL) {
            if (mData->mHandler != NULL) {
                mData->mLooper->unregisterHandler(mData->mHandler->id());
            }
            mData->mLooper->stop();
            mData->mLooper.clear();
        }
        delete mData;
    }
    return AMEDIA_OK;
}

EXPORT
media_status_t AMediaCodec_getName(
        AMediaCodec *mData,
        char** out_name) {
    if (out_name == NULL) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }

    AString compName;
    status_t err = mData->mCodec->getName(&compName);
    if (err != OK) {
        return translate_error(err);
    }
    *out_name = strdup(compName.c_str());
    return AMEDIA_OK;
}

EXPORT
void AMediaCodec_releaseName(
        AMediaCodec * /* mData */,
        char* name) {
    if (name != NULL) {
        free(name);
    }
}

EXPORT
media_status_t AMediaCodec_configure(
        AMediaCodec *mData,
        const AMediaFormat* format,
        ANativeWindow* window,
        AMediaCrypto *crypto,
        uint32_t flags) {
    sp<AMessage> nativeFormat;
    AMediaFormat_getFormat(format, &nativeFormat);
    ALOGV("configure with format: %s", nativeFormat->debugString(0).c_str());
    sp<Surface> surface = NULL;
    if (window != NULL) {
        surface = (Surface*) window;
    }

    status_t err = mData->mCodec->configure(nativeFormat, surface,
            crypto ? crypto->mCrypto : NULL, flags);
    if (err != OK) {
        ALOGE("configure: err(%d), failed with format: %s",
              err, nativeFormat->debugString(0).c_str());
    }
    return translate_error(err);
}

EXPORT
media_status_t AMediaCodec_setAsyncNotifyCallback(
        AMediaCodec *mData,
        AMediaCodecOnAsyncNotifyCallback callback,
        void *userdata) {
    if (mData->mAsyncNotify == NULL && userdata != NULL) {
        mData->mAsyncNotify = new AMessage(kWhatAsyncNotify, mData->mHandler);
        status_t err = mData->mCodec->setCallback(mData->mAsyncNotify);
        if (err != OK) {
            ALOGE("setAsyncNotifyCallback: err(%d), failed to set async callback", err);
            return translate_error(err);
        }
    }

    Mutex::Autolock _l(mData->mAsyncCallbackLock);
    mData->mAsyncCallback = callback;
    mData->mAsyncCallbackUserData = userdata;

    return AMEDIA_OK;
}


EXPORT
media_status_t AMediaCodec_releaseCrypto(AMediaCodec *mData) {
    return translate_error(mData->mCodec->releaseCrypto());
}

EXPORT
media_status_t AMediaCodec_start(AMediaCodec *mData) {
    status_t ret =  mData->mCodec->start();
    if (ret != OK) {
        return translate_error(ret);
    }
    mData->mActivityNotification = new AMessage(kWhatActivityNotify, mData->mHandler);
    mData->mActivityNotification->setInt32("generation", mData->mGeneration);
    requestActivityNotification(mData);
    return AMEDIA_OK;
}

EXPORT
media_status_t AMediaCodec_stop(AMediaCodec *mData) {
    media_status_t ret = translate_error(mData->mCodec->stop());

    sp<AMessage> msg = new AMessage(kWhatStopActivityNotifications, mData->mHandler);
    sp<AMessage> response;
    msg->postAndAwaitResponse(&response);
    mData->mActivityNotification.clear();

    return ret;
}

EXPORT
media_status_t AMediaCodec_flush(AMediaCodec *mData) {
    return translate_error(mData->mCodec->flush());
}

EXPORT
ssize_t AMediaCodec_dequeueInputBuffer(AMediaCodec *mData, int64_t timeoutUs) {
    size_t idx;
    status_t ret = mData->mCodec->dequeueInputBuffer(&idx, timeoutUs);
    requestActivityNotification(mData);
    if (ret == OK) {
        return idx;
    }
    return translate_error(ret);
}

EXPORT
uint8_t* AMediaCodec_getInputBuffer(AMediaCodec *mData, size_t idx, size_t *out_size) {
    if (mData->mAsyncNotify != NULL) {
        // Asynchronous mode
        sp<MediaCodecBuffer> abuf;
        if (mData->mCodec->getInputBuffer(idx, &abuf) != 0) {
            return NULL;
        }

        if (out_size != NULL) {
            *out_size = abuf->capacity();
        }
        return abuf->data();
    }

    android::Vector<android::sp<android::MediaCodecBuffer> > abufs;
    if (mData->mCodec->getInputBuffers(&abufs) == 0) {
        size_t n = abufs.size();
        if (idx >= n) {
            ALOGE("buffer index %zu out of range", idx);
            return NULL;
        }
        if (abufs[idx] == NULL) {
            ALOGE("buffer index %zu is NULL", idx);
            return NULL;
        }
        if (out_size != NULL) {
            *out_size = abufs[idx]->capacity();
        }
        return abufs[idx]->data();
    }
    ALOGE("couldn't get input buffers");
    return NULL;
}

EXPORT
uint8_t* AMediaCodec_getOutputBuffer(AMediaCodec *mData, size_t idx, size_t *out_size) {
    if (mData->mAsyncNotify != NULL) {
        // Asynchronous mode
        sp<MediaCodecBuffer> abuf;
        if (mData->mCodec->getOutputBuffer(idx, &abuf) != 0) {
            return NULL;
        }

        if (out_size != NULL) {
            *out_size = abuf->capacity();
        }
        return abuf->data();
    }

    android::Vector<android::sp<android::MediaCodecBuffer> > abufs;
    if (mData->mCodec->getOutputBuffers(&abufs) == 0) {
        size_t n = abufs.size();
        if (idx >= n) {
            ALOGE("buffer index %zu out of range", idx);
            return NULL;
        }
        if (out_size != NULL) {
            *out_size = abufs[idx]->capacity();
        }
        return abufs[idx]->data();
    }
    ALOGE("couldn't get output buffers");
    return NULL;
}

EXPORT
media_status_t AMediaCodec_queueInputBuffer(AMediaCodec *mData,
        size_t idx, off_t offset, size_t size, uint64_t time, uint32_t flags) {

    AString errorMsg;
    status_t ret = mData->mCodec->queueInputBuffer(idx, offset, size, time, flags, &errorMsg);
    return translate_error(ret);
}

EXPORT
ssize_t AMediaCodec_dequeueOutputBuffer(AMediaCodec *mData,
        AMediaCodecBufferInfo *info, int64_t timeoutUs) {
    size_t idx;
    size_t offset;
    size_t size;
    uint32_t flags;
    int64_t presentationTimeUs;
    status_t ret = mData->mCodec->dequeueOutputBuffer(&idx, &offset, &size, &presentationTimeUs,
            &flags, timeoutUs);
    requestActivityNotification(mData);
    switch (ret) {
        case OK:
            info->offset = offset;
            info->size = size;
            info->flags = flags;
            info->presentationTimeUs = presentationTimeUs;
            return idx;
        case -EAGAIN:
            return AMEDIACODEC_INFO_TRY_AGAIN_LATER;
        case android::INFO_FORMAT_CHANGED:
            return AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED;
        case INFO_OUTPUT_BUFFERS_CHANGED:
            return AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED;
        default:
            break;
    }
    return translate_error(ret);
}

EXPORT
AMediaFormat* AMediaCodec_getOutputFormat(AMediaCodec *mData) {
    sp<AMessage> format;
    mData->mCodec->getOutputFormat(&format);
    return AMediaFormat_fromMsg(&format);
}

EXPORT
AMediaFormat* AMediaCodec_getInputFormat(AMediaCodec *mData) {
    sp<AMessage> format;
    mData->mCodec->getInputFormat(&format);
    return AMediaFormat_fromMsg(&format);
}

EXPORT
AMediaFormat* AMediaCodec_getBufferFormat(AMediaCodec *mData, size_t index) {
    sp<AMessage> format;
    mData->mCodec->getOutputFormat(index, &format);
    return AMediaFormat_fromMsg(&format);
}

EXPORT
media_status_t AMediaCodec_releaseOutputBuffer(AMediaCodec *mData, size_t idx, bool render) {
    if (render) {
        return translate_error(mData->mCodec->renderOutputBufferAndRelease(idx));
    } else {
        return translate_error(mData->mCodec->releaseOutputBuffer(idx));
    }
}

EXPORT
media_status_t AMediaCodec_releaseOutputBufferAtTime(
        AMediaCodec *mData, size_t idx, int64_t timestampNs) {
    ALOGV("render @ %" PRId64, timestampNs);
    return translate_error(mData->mCodec->renderOutputBufferAndRelease(idx, timestampNs));
}

EXPORT
media_status_t AMediaCodec_setOutputSurface(AMediaCodec *mData, ANativeWindow* window) {
    sp<Surface> surface = NULL;
    if (window != NULL) {
        surface = (Surface*) window;
    }
    return translate_error(mData->mCodec->setSurface(surface));
}

EXPORT
media_status_t AMediaCodec_createInputSurface(AMediaCodec *mData, ANativeWindow **surface) {
    if (surface == NULL || mData == NULL) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    *surface = NULL;

    sp<IGraphicBufferProducer> igbp = NULL;
    status_t err = mData->mCodec->createInputSurface(&igbp);
    if (err != NO_ERROR) {
        return translate_error(err);
    }

    *surface = new Surface(igbp);
    ANativeWindow_acquire(*surface);
    return AMEDIA_OK;
}

EXPORT
media_status_t AMediaCodec_createPersistentInputSurface(ANativeWindow **surface) {
    if (surface == NULL) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    *surface = NULL;

    sp<PersistentSurface> ps = MediaCodec::CreatePersistentInputSurface();
    if (ps == NULL) {
        return AMEDIA_ERROR_UNKNOWN;
    }

    sp<IGraphicBufferProducer> igbp = ps->getBufferProducer();
    if (igbp == NULL) {
        return AMEDIA_ERROR_UNKNOWN;
    }

    *surface = new AMediaCodecPersistentSurface(igbp, ps);
    ANativeWindow_acquire(*surface);

    return AMEDIA_OK;
}

EXPORT
media_status_t AMediaCodec_setInputSurface(
        AMediaCodec *mData, ANativeWindow *surface) {

    if (surface == NULL || mData == NULL) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }

    AMediaCodecPersistentSurface *aMediaPersistentSurface =
            static_cast<AMediaCodecPersistentSurface *>(surface);
    if (aMediaPersistentSurface->mPersistentSurface == NULL) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }

    return translate_error(mData->mCodec->setInputSurface(
            aMediaPersistentSurface->mPersistentSurface));
}

EXPORT
media_status_t AMediaCodec_setParameters(
        AMediaCodec *mData, const AMediaFormat* params) {
    if (params == NULL || mData == NULL) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    sp<AMessage> nativeParams;
    AMediaFormat_getFormat(params, &nativeParams);
    ALOGV("setParameters: %s", nativeParams->debugString(0).c_str());

    return translate_error(mData->mCodec->setParameters(nativeParams));
}

EXPORT
media_status_t AMediaCodec_signalEndOfInputStream(AMediaCodec *mData) {

    if (mData == NULL) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }

    status_t err = mData->mCodec->signalEndOfInputStream();
    if (err == INVALID_OPERATION) {
        return AMEDIA_ERROR_INVALID_OPERATION;
    }

    return translate_error(err);

}

//EXPORT
media_status_t AMediaCodec_setNotificationCallback(AMediaCodec *mData, OnCodecEvent callback,
        void *userdata) {
    mData->mCallback = callback;
    mData->mCallbackUserData = userdata;
    return AMEDIA_OK;
}

typedef struct AMediaCodecCryptoInfo {
        int numsubsamples;
        uint8_t key[16];
        uint8_t iv[16];
        cryptoinfo_mode_t mode;
        cryptoinfo_pattern_t pattern;
        size_t *clearbytes;
        size_t *encryptedbytes;
} AMediaCodecCryptoInfo;

EXPORT
media_status_t AMediaCodec_queueSecureInputBuffer(
        AMediaCodec* codec,
        size_t idx,
        off_t offset,
        AMediaCodecCryptoInfo* crypto,
        uint64_t time,
        uint32_t flags) {

    CryptoPlugin::SubSample *subSamples = new CryptoPlugin::SubSample[crypto->numsubsamples];
    for (int i = 0; i < crypto->numsubsamples; i++) {
        subSamples[i].mNumBytesOfClearData = crypto->clearbytes[i];
        subSamples[i].mNumBytesOfEncryptedData = crypto->encryptedbytes[i];
    }

    CryptoPlugin::Pattern pattern;
    pattern.mEncryptBlocks = crypto->pattern.encryptBlocks;
    pattern.mSkipBlocks = crypto->pattern.skipBlocks;

    AString errormsg;
    status_t err  = codec->mCodec->queueSecureInputBuffer(idx,
            offset,
            subSamples,
            crypto->numsubsamples,
            crypto->key,
            crypto->iv,
            (CryptoPlugin::Mode)crypto->mode,
            pattern,
            time,
            flags,
            &errormsg);
    if (err != 0) {
        ALOGE("queSecureInputBuffer: %s", errormsg.c_str());
    }
    delete [] subSamples;
    return translate_error(err);
}

EXPORT
bool AMediaCodecActionCode_isRecoverable(int32_t actionCode) {
    return (actionCode == ACTION_CODE_RECOVERABLE);
}

EXPORT
bool AMediaCodecActionCode_isTransient(int32_t actionCode) {
    return (actionCode == ACTION_CODE_TRANSIENT);
}


EXPORT
void AMediaCodecCryptoInfo_setPattern(AMediaCodecCryptoInfo *info,
        cryptoinfo_pattern_t *pattern) {
    info->pattern.encryptBlocks = pattern->encryptBlocks;
    info->pattern.skipBlocks = pattern->skipBlocks;
}

EXPORT
AMediaCodecCryptoInfo *AMediaCodecCryptoInfo_new(
        int numsubsamples,
        uint8_t key[16],
        uint8_t iv[16],
        cryptoinfo_mode_t mode,
        size_t *clearbytes,
        size_t *encryptedbytes) {

    // size needed to store all the crypto data
    size_t cryptosize;
    // = sizeof(AMediaCodecCryptoInfo) + sizeof(size_t) * numsubsamples * 2;
    if (__builtin_mul_overflow(sizeof(size_t) * 2, numsubsamples, &cryptosize) ||
            __builtin_add_overflow(cryptosize, sizeof(AMediaCodecCryptoInfo), &cryptosize)) {
        ALOGE("crypto size overflow");
        return NULL;
    }
    AMediaCodecCryptoInfo *ret = (AMediaCodecCryptoInfo*) malloc(cryptosize);
    if (!ret) {
        ALOGE("couldn't allocate %zu bytes", cryptosize);
        return NULL;
    }
    ret->numsubsamples = numsubsamples;
    memcpy(ret->key, key, 16);
    memcpy(ret->iv, iv, 16);
    ret->mode = mode;
    ret->pattern.encryptBlocks = 0;
    ret->pattern.skipBlocks = 0;

    // clearbytes and encryptedbytes point at the actual data, which follows
    ret->clearbytes = (size_t*) (ret + 1); // point immediately after the struct
    ret->encryptedbytes = ret->clearbytes + numsubsamples; // point after the clear sizes

    memcpy(ret->clearbytes, clearbytes, numsubsamples * sizeof(size_t));
    memcpy(ret->encryptedbytes, encryptedbytes, numsubsamples * sizeof(size_t));

    return ret;
}


EXPORT
media_status_t AMediaCodecCryptoInfo_delete(AMediaCodecCryptoInfo* info) {
    free(info);
    return AMEDIA_OK;
}

EXPORT
size_t AMediaCodecCryptoInfo_getNumSubSamples(AMediaCodecCryptoInfo* ci) {
    return ci->numsubsamples;
}

EXPORT
media_status_t AMediaCodecCryptoInfo_getKey(AMediaCodecCryptoInfo* ci, uint8_t *dst) {
    if (!ci) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    if (!dst) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    memcpy(dst, ci->key, 16);
    return AMEDIA_OK;
}

EXPORT
media_status_t AMediaCodecCryptoInfo_getIV(AMediaCodecCryptoInfo* ci, uint8_t *dst) {
    if (!ci) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    if (!dst) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    memcpy(dst, ci->iv, 16);
    return AMEDIA_OK;
}

EXPORT
cryptoinfo_mode_t AMediaCodecCryptoInfo_getMode(AMediaCodecCryptoInfo* ci) {
    if (!ci) {
        return (cryptoinfo_mode_t) AMEDIA_ERROR_INVALID_OBJECT;
    }
    return ci->mode;
}

EXPORT
media_status_t AMediaCodecCryptoInfo_getClearBytes(AMediaCodecCryptoInfo* ci, size_t *dst) {
    if (!ci) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    if (!dst) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    memcpy(dst, ci->clearbytes, sizeof(size_t) * ci->numsubsamples);
    return AMEDIA_OK;
}

EXPORT
media_status_t AMediaCodecCryptoInfo_getEncryptedBytes(AMediaCodecCryptoInfo* ci, size_t *dst) {
    if (!ci) {
        return AMEDIA_ERROR_INVALID_OBJECT;
    }
    if (!dst) {
        return AMEDIA_ERROR_INVALID_PARAMETER;
    }
    memcpy(dst, ci->encryptedbytes, sizeof(size_t) * ci->numsubsamples);
    return AMEDIA_OK;
}

} // extern "C"

