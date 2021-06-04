/*
 * Copyright (C) 2013 The Android Open Source Project
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

#include <malloc.h>
#include <string.h>
#include <pthread.h>

#include "RenderScript.h"
#include "rsCppStructs.h"
#include "rsCppInternal.h"

#include <dlfcn.h>
#include <unistd.h>

using android::RSC::RS;
using android::RSC::RSError;

bool RS::gInitialized = false;
bool RS::usingNative = false;
pthread_mutex_t RS::gInitMutex = PTHREAD_MUTEX_INITIALIZER;
dispatchTable* RS::dispatch = nullptr;
static int gInitError = 0;

RS::RS() {
    mContext = nullptr;
    mErrorFunc = nullptr;
    mMessageFunc = nullptr;
    mMessageRun = false;
    mInit = false;
    mCurrentError = RS_SUCCESS;

    memset(&mElements, 0, sizeof(mElements));
    memset(&mSamplers, 0, sizeof(mSamplers));
}

RS::~RS() {
    if (mInit == true) {
        mMessageRun = false;

        if (mContext) {
            finish();
            RS::dispatch->ContextDeinitToClient(mContext);

            void *res = nullptr;
            pthread_join(mMessageThreadId, &res);

            RS::dispatch->ContextDestroy(mContext);
            mContext = nullptr;
        }
    }
}

bool RS::init(const char * name, uint32_t flags) {
    return RS::init(name, flags, 0);
}

// This will only open API 19+ libRS, because that's when
// we changed libRS to extern "C" entry points.
static bool loadSO(const char* filename, int targetApi) {
    void* handle = dlopen(filename, RTLD_LAZY | RTLD_LOCAL);
    if (handle == nullptr) {
        ALOGV("couldn't dlopen %s, %s", filename, dlerror());
        return false;
    }

    if (loadSymbols(handle, *RS::dispatch, targetApi) == false) {
        ALOGV("%s init failed!", filename);
        return false;
    }
    return true;
}

static uint32_t getProp(const char *str) {
#if !defined(__LP64__) && defined(__ANDROID__)
    char buf[256];
    android::renderscript::property_get(str, buf, "0");
    return atoi(buf);
#else
    return 0;
#endif
}

bool RS::initDispatch(int targetApi) {
    pthread_mutex_lock(&gInitMutex);
    if (gInitError) {
        goto error;
    } else if (gInitialized) {
        pthread_mutex_unlock(&gInitMutex);
        return true;
    }

    RS::dispatch = new dispatchTable;

    // Attempt to load libRS, load libRSSupport on failure.
    // If property is set, proceed directly to libRSSupport.
    if (getProp("debug.rs.forcecompat") == 0) {
        usingNative = loadSO("libRS.so", targetApi);
    }
    if (usingNative == false) {
        if (loadSO("libRSSupport.so", targetApi) == false) {
            ALOGE("Failed to load libRS.so and libRSSupport.so");
            goto error;
        }
    }

    gInitialized = true;

    pthread_mutex_unlock(&gInitMutex);
    return true;

 error:
    gInitError = 1;
    pthread_mutex_unlock(&gInitMutex);
    return false;
}

bool RS::init(const char * name, uint32_t flags, int targetApi) {
    if (mInit) {
        return true;
    }
    // When using default value 0, set targetApi to RS_VERSION,
    // to preserve the behavior of existing apps.
    if (targetApi == 0) {
        targetApi = RS_VERSION;
    }

    if (initDispatch(targetApi) == false) {
        ALOGE("Couldn't initialize dispatch table");
        return false;
    }

    uint32_t nameLen = strlen(name);
    if (nameLen > PATH_MAX) {
        ALOGE("The path to the cache directory is too long");
        return false;
    }
    memcpy(mCacheDir, name, nameLen);
    // Add the null character even if the user does not.
    mCacheDir[nameLen] = 0;
    mCacheDirLen = nameLen + 1;

    RsDevice device = RS::dispatch->DeviceCreate();
    if (device == 0) {
        ALOGE("Device creation failed");
        return false;
    }

    if (flags & ~(RS_CONTEXT_SYNCHRONOUS | RS_CONTEXT_LOW_LATENCY |
                  RS_CONTEXT_LOW_POWER | RS_CONTEXT_WAIT_FOR_ATTACH)) {
        ALOGE("Invalid flags passed");
        return false;
    }

    mContext = RS::dispatch->ContextCreate(device, 0, targetApi, RS_CONTEXT_TYPE_NORMAL, flags);
    if (mContext == 0) {
        ALOGE("Context creation failed");
        return false;
    }

    pid_t mNativeMessageThreadId;

    int status = pthread_create(&mMessageThreadId, nullptr, threadProc, this);
    if (status) {
        ALOGE("Failed to start RS message thread.");
        return false;
    }
    // Wait for the message thread to be active.
    while (!mMessageRun) {
        usleep(1000);
    }

    mInit = true;

    return true;
}

void RS::throwError(RSError error, const char *errMsg) {
    if (mCurrentError == RS_SUCCESS) {
        mCurrentError = error;
        ALOGE("RS CPP error: %s", errMsg);
    } else {
        ALOGE("RS CPP error (masked by previous error): %s", errMsg);
    }
}

RSError RS::getError() {
    return mCurrentError;
}


void * RS::threadProc(void *vrsc) {
    RS *rs = static_cast<RS *>(vrsc);
    size_t rbuf_size = 256;
    void * rbuf = malloc(rbuf_size);

    RS::dispatch->ContextInitToClient(rs->mContext);
    rs->mMessageRun = true;

    while (rs->mMessageRun) {
        size_t receiveLen = 0;
        uint32_t usrID = 0;
        uint32_t subID = 0;
        RsMessageToClientType r = RS::dispatch->ContextPeekMessage(rs->mContext,
                                                                   &receiveLen, sizeof(receiveLen),
                                                                   &usrID, sizeof(usrID));

        if (receiveLen >= rbuf_size) {
            rbuf_size = receiveLen + 32;
            void *tmpBuf = realloc(rbuf, rbuf_size);
            if (tmpBuf) {
                rbuf = tmpBuf;
            } else {
                free(rbuf);
                rbuf = NULL;
            }
        }
        if (!rbuf) {
            ALOGE("RS::message handler realloc error %zu", rbuf_size);
            // No clean way to recover now?
        }
        RS::dispatch->ContextGetMessage(rs->mContext, rbuf, rbuf_size, &receiveLen, sizeof(receiveLen),
                            &subID, sizeof(subID));

        switch(r) {
        case RS_MESSAGE_TO_CLIENT_ERROR:
            ALOGE("RS Error %s", (const char *)rbuf);
            rs->throwError(RS_ERROR_RUNTIME_ERROR, "Error returned from runtime");
            if(rs->mMessageFunc != nullptr) {
                rs->mErrorFunc(usrID, (const char *)rbuf);
            }
            break;
        case RS_MESSAGE_TO_CLIENT_NONE:
        case RS_MESSAGE_TO_CLIENT_EXCEPTION:
        case RS_MESSAGE_TO_CLIENT_RESIZE:
            /*
             * Teardown. We want to avoid starving other threads during
             * teardown by yielding until the next line in the destructor can
             * execute to set mRun = false. Note that the FIFO sends an
             * empty NONE message when it reaches its destructor.
             */
            usleep(1000);
            break;
        case RS_MESSAGE_TO_CLIENT_USER:
            if(rs->mMessageFunc != nullptr) {
                rs->mMessageFunc(usrID, rbuf, receiveLen);
            } else {
                ALOGE("Received a message from the script with no message handler installed.");
            }
            break;

        default:
            ALOGE("RS unknown message type %i", r);
        }
    }

    if (rbuf) {
        free(rbuf);
    }
    ALOGV("RS Message thread exiting.");
    return nullptr;
}

void RS::setErrorHandler(ErrorHandlerFunc_t func) {
    mErrorFunc = func;
}

void RS::setMessageHandler(MessageHandlerFunc_t func) {
    mMessageFunc  = func;
}

void RS::finish() {
    RS::dispatch->ContextFinish(mContext);
}
