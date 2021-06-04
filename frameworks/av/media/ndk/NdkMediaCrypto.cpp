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

//#define LOG_NDEBUG 0
#define LOG_TAG "NdkMediaCrypto"


#include <media/NdkMediaCrypto.h>
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormatPriv.h>


#include <cutils/properties.h>
#include <utils/Log.h>
#include <utils/StrongPointer.h>
#include <binder/IServiceManager.h>
#include <media/ICrypto.h>
#include <media/IMediaDrmService.h>
#include <android_util_Binder.h>

#include <jni.h>

using namespace android;

static sp<ICrypto> makeCrypto() {
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder = sm->getService(String16("media.drm"));

    sp<IMediaDrmService> service = interface_cast<IMediaDrmService>(binder);
    if (service == NULL) {
        return NULL;
    }

    sp<ICrypto> crypto = service->makeCrypto();
    if (crypto == NULL || (crypto->initCheck() != OK && crypto->initCheck() != NO_INIT)) {
        return NULL;
    }
    return crypto;
}

struct AMediaCrypto {
    sp<ICrypto> mCrypto;
};


extern "C" {


EXPORT
bool AMediaCrypto_isCryptoSchemeSupported(const AMediaUUID uuid) {
    sp<ICrypto> crypto = makeCrypto();
    if (crypto == NULL) {
        return false;
    }
    return crypto->isCryptoSchemeSupported(uuid);
}

EXPORT
bool AMediaCrypto_requiresSecureDecoderComponent(const char *mime) {
    sp<ICrypto> crypto = makeCrypto();
    if (crypto == NULL) {
        return false;
    }
    return crypto->requiresSecureDecoderComponent(mime);
}

EXPORT
AMediaCrypto* AMediaCrypto_new(const AMediaUUID uuid, const void *data, size_t datasize) {

    sp<ICrypto> tmp = makeCrypto();
    if (tmp == NULL) {
        return NULL;
    }

    if (tmp->createPlugin(uuid, data, datasize) != 0) {
        return NULL;
    }

    AMediaCrypto *crypto = new AMediaCrypto();
    crypto->mCrypto = tmp;

    return crypto;
}

EXPORT
void AMediaCrypto_delete(AMediaCrypto* crypto) {
    delete crypto;
}



} // extern "C"

