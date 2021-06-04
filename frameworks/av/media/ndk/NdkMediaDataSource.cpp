/*
 * Copyright (C) 2018 The Android Open Source Project
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
#define LOG_TAG "NdkMediaDataSource"

#include "NdkMediaDataSourcePriv.h"

#include <inttypes.h>
#include <jni.h>
#include <unistd.h>

#include <android_runtime/AndroidRuntime.h>
#include <android_util_Binder.h>
#include <cutils/properties.h>
#include <utils/Log.h>
#include <utils/StrongPointer.h>
#include <media/IMediaHTTPService.h>
#include <media/NdkMediaError.h>
#include <media/NdkMediaDataSource.h>
#include <media/stagefright/DataSourceFactory.h>
#include <media/stagefright/InterfaceUtils.h>
#include <mediaplayer2/JavaVMHelper.h>
#include <mediaplayer2/JMedia2HTTPService.h>

#include "../../libstagefright/include/HTTPBase.h"
#include "../../libstagefright/include/NuCachedSource2.h"
#include "NdkMediaDataSourceCallbacksPriv.h"


using namespace android;

struct AMediaDataSource {
    void *userdata;
    AMediaDataSourceReadAt readAt;
    AMediaDataSourceGetSize getSize;
    AMediaDataSourceClose close;
    AMediaDataSourceGetAvailableSize getAvailableSize;
    sp<DataSource> mImpl;
    uint32_t mFlags;
};

NdkDataSource::NdkDataSource(AMediaDataSource *dataSource)
    : mDataSource(AMediaDataSource_new()) {
    AMediaDataSource_setReadAt(mDataSource, dataSource->readAt);
    AMediaDataSource_setGetSize(mDataSource, dataSource->getSize);
    AMediaDataSource_setClose(mDataSource, dataSource->close);
    AMediaDataSource_setUserdata(mDataSource, dataSource->userdata);
    AMediaDataSource_setGetAvailableSize(mDataSource, dataSource->getAvailableSize);
    mDataSource->mImpl = dataSource->mImpl;
    mDataSource->mFlags = dataSource->mFlags;
}

NdkDataSource::~NdkDataSource() {
    AMediaDataSource_delete(mDataSource);
}

status_t NdkDataSource::initCheck() const {
    return OK;
}

uint32_t NdkDataSource::flags() {
    return mDataSource->mFlags;
}

ssize_t NdkDataSource::readAt(off64_t offset, void *data, size_t size) {
    Mutex::Autolock l(mLock);
    if (mDataSource->readAt == NULL || mDataSource->userdata == NULL) {
        return -1;
    }
    return mDataSource->readAt(mDataSource->userdata, offset, data, size);
}

status_t NdkDataSource::getSize(off64_t *size) {
    Mutex::Autolock l(mLock);
    if (mDataSource->getSize == NULL || mDataSource->userdata == NULL) {
        return NO_INIT;
    }
    if (size != NULL) {
        *size = mDataSource->getSize(mDataSource->userdata);
    }
    return OK;
}

String8 NdkDataSource::toString() {
    return String8::format("NdkDataSource(pid %d, uid %d)", getpid(), getuid());
}

String8 NdkDataSource::getMIMEType() const {
    return String8("application/octet-stream");
}

void NdkDataSource::close() {
    if (mDataSource->close != NULL && mDataSource->userdata != NULL) {
        mDataSource->close(mDataSource->userdata);
    }
}

status_t NdkDataSource::getAvailableSize(off64_t offset, off64_t *sizeptr) {
    off64_t size = -1;
    if (mDataSource->getAvailableSize != NULL
            && mDataSource->userdata != NULL
            && sizeptr != NULL) {
        size = mDataSource->getAvailableSize(mDataSource->userdata, offset);
        *sizeptr = size;
    }
    return size >= 0 ? OK : UNKNOWN_ERROR;
}

static sp<MediaHTTPService> createMediaHttpServiceFromJavaObj(JNIEnv *env, jobject obj, int version) {
    if (obj == NULL) {
        return NULL;
    }
    switch (version) {
        case 1:
            return interface_cast<IMediaHTTPService>(ibinderForJavaObject(env, obj));
        case 2:
            return new JMedia2HTTPService(env, obj);
        default:
            return NULL;
    }
}

static sp<MediaHTTPService> createMediaHttpServiceTemplate(
        JNIEnv *env,
        const char *uri,
        const char *clazz,
        const char *method,
        const char *signature,
        int version) {
    jobject service = NULL;
    if (env == NULL) {
        ALOGE("http service must be created from Java thread");
        return NULL;
    }

    jclass mediahttpclass = env->FindClass(clazz);
    if (mediahttpclass == NULL) {
        ALOGE("can't find Media(2)HttpService");
        env->ExceptionClear();
        return NULL;
    }

    jmethodID mediaHttpCreateMethod = env->GetStaticMethodID(mediahttpclass, method, signature);
    if (mediaHttpCreateMethod == NULL) {
        ALOGE("can't find method");
        env->ExceptionClear();
        return NULL;
    }

    jstring juri = env->NewStringUTF(uri);

    service = env->CallStaticObjectMethod(mediahttpclass, mediaHttpCreateMethod, juri);
    env->DeleteLocalRef(juri);

    env->ExceptionClear();
    sp<MediaHTTPService> httpService = createMediaHttpServiceFromJavaObj(env, service, version);
    return httpService;

}

sp<MediaHTTPService> createMediaHttpService(const char *uri, int version) {

    JNIEnv *env;
    const char *clazz, *method, *signature;

    switch (version) {
        case 1:
            env = AndroidRuntime::getJNIEnv();
            clazz = "android/media/MediaHTTPService";
            method = "createHttpServiceBinderIfNecessary";
            signature = "(Ljava/lang/String;)Landroid/os/IBinder;";
            break;
        case 2:
            env = JavaVMHelper::getJNIEnv();
            clazz = "android/media/Media2HTTPService";
            method = "createHTTPService";
            signature = "(Ljava/lang/String;)Landroid/media/Media2HTTPService;";
            break;
        default:
            return NULL;
    }

    return createMediaHttpServiceTemplate(env, uri, clazz, method, signature, version);

}

extern "C" {

EXPORT
AMediaDataSource* AMediaDataSource_new() {
    AMediaDataSource *mSource = new AMediaDataSource();
    mSource->userdata = NULL;
    mSource->readAt = NULL;
    mSource->getSize = NULL;
    mSource->close = NULL;
    return mSource;
}

EXPORT
AMediaDataSource* AMediaDataSource_newUri(
        const char *uri,
        int numheaders,
        const char * const *key_values) {

    sp<MediaHTTPService> service = createMediaHttpService(uri, /* version = */ 1);
    KeyedVector<String8, String8> headers;
    for (int i = 0; i < numheaders; ++i) {
        String8 key8(key_values[i * 2]);
        String8 value8(key_values[i * 2 + 1]);
        headers.add(key8, value8);
    }

    sp<DataSource> source = DataSourceFactory::CreateFromURI(service, uri, &headers);
    if (source == NULL) {
        ALOGE("AMediaDataSource_newUri source is null");
        return NULL;
    }
    ALOGI("AMediaDataSource_newUri source %s flags %u", source->toString().c_str(), source->flags());
    AMediaDataSource* aSource = convertDataSourceToAMediaDataSource(source);
    aSource->mImpl = source;
    aSource->mFlags = source->flags();
    return aSource;
}

EXPORT
void AMediaDataSource_delete(AMediaDataSource *mSource) {
    ALOGV("dtor");
    if (mSource != NULL) {
        delete mSource;
    }
}

EXPORT
void AMediaDataSource_setUserdata(AMediaDataSource *mSource, void *userdata) {
    mSource->userdata = userdata;
}

EXPORT
void AMediaDataSource_setReadAt(AMediaDataSource *mSource, AMediaDataSourceReadAt readAt) {
    mSource->readAt = readAt;
}

EXPORT
void AMediaDataSource_setGetSize(AMediaDataSource *mSource, AMediaDataSourceGetSize getSize) {
    mSource->getSize = getSize;
}

EXPORT
void AMediaDataSource_setClose(AMediaDataSource *mSource, AMediaDataSourceClose close) {
    mSource->close = close;
}

EXPORT
void AMediaDataSource_close(AMediaDataSource *mSource) {
    return mSource->close(mSource->userdata);
}

EXPORT
void AMediaDataSource_setGetAvailableSize(AMediaDataSource *mSource,
        AMediaDataSourceGetAvailableSize getAvailableSize) {
    mSource->getAvailableSize = getAvailableSize;
}

} // extern "C"

