/*
 * Copyright 2017, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "wifi-jni"

#include <ctype.h>
#include <stdlib.h>
#include <sys/klog.h>

#include <log/log.h>
#include <jni.h>
#include <nativehelper/jni_macros.h>
#include <nativehelper/JNIHelp.h>

#include "jni_helper.h"

namespace android {


static jbyteArray android_net_wifi_readKernelLogNative(JNIEnv *env, jclass cls) {
    JNIHelper helper(env);
    ALOGV("Reading kernel logs");

    int size = klogctl(/* SYSLOG_ACTION_SIZE_BUFFER */ 10, 0, 0);
    if (size < 1) {
        ALOGD("no kernel logs");
        return helper.newByteArray(0).detach();
    }

    char *buf = (char *)malloc(size);
    if (buf == NULL) {
        ALOGD("can't allocate temporary storage");
        return helper.newByteArray(0).detach();
    }

    int read = klogctl(/* SYSLOG_ACTION_READ_ALL */ 3, buf, size);
    if (read < 0) {
        ALOGD("can't read logs - %d", read);
        free(buf);
        return helper.newByteArray(0).detach();
    } else {
        ALOGV("read %d bytes", read);
    }

    if (read != size) {
        ALOGV("read %d bytes, expecting %d", read, size);
    }

    JNIObject<jbyteArray> result = helper.newByteArray(read);
    if (result.isNull()) {
        ALOGD("can't allocate array");
        free(buf);
        return result.detach();
    }

    helper.setByteArrayRegion(result, 0, read, (jbyte*)buf);
    free(buf);
    return result.detach();
}

// ----------------------------------------------------------------------------

/*
 * JNI registration.
 */
static JNINativeMethod gWifiMethods[] = {
    NATIVE_METHOD(android_net_wifi, readKernelLogNative, "()[B"),
};

/* User to register native functions */
extern "C"
jint Java_com_android_server_wifi_WifiNative_registerNatives(JNIEnv* env, jclass clazz) {
    return jniRegisterNativeMethods(env,
            "com/android/server/wifi/WifiNative", gWifiMethods, NELEM(gWifiMethods));
}

}; // namespace android
