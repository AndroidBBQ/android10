/*
 * Copyright 2018 The Android Open Source Project
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

#define LOG_TAG "JavaVMHelper"

#include "mediaplayer2/JavaVMHelper.h"

#include <media/stagefright/foundation/ADebug.h>
#include <utils/threads.h>

#include <stdlib.h>

namespace android {

// static
std::atomic<JavaVM *> JavaVMHelper::sJavaVM(NULL);

/*
 * Makes the current thread visible to the VM.
 *
 * The JNIEnv pointer returned is only valid for the current thread, and
 * thus must be tucked into thread-local storage.
 */
static int javaAttachThread(const char* threadName, JNIEnv** pEnv) {
    JavaVMAttachArgs args;
    JavaVM* vm;
    jint result;

    vm = JavaVMHelper::getJavaVM();
    if (vm == NULL) {
        return JNI_ERR;
    }

    args.version = JNI_VERSION_1_4;
    args.name = (char*) threadName;
    args.group = NULL;

    result = vm->AttachCurrentThread(pEnv, (void*) &args);
    if (result != JNI_OK) {
        ALOGI("NOTE: attach of thread '%s' failed\n", threadName);
    }

    return result;
}

/*
 * Detach the current thread from the set visible to the VM.
 */
static int javaDetachThread(void) {
    JavaVM* vm;
    jint result;

    vm = JavaVMHelper::getJavaVM();
    if (vm == NULL) {
        return JNI_ERR;
    }

    result = vm->DetachCurrentThread();
    if (result != JNI_OK) {
        ALOGE("ERROR: thread detach failed\n");
    }
    return result;
}

/*
 * When starting a native thread that will be visible from the VM, we
 * bounce through this to get the right attach/detach action.
 * Note that this function calls free(args)
 */
static int javaThreadShell(void* args) {
    void* start = ((void**)args)[0];
    void* userData = ((void **)args)[1];
    char* name = (char*) ((void **)args)[2];        // we own this storage
    free(args);
    JNIEnv* env;
    int result;

    /* hook us into the VM */
    if (javaAttachThread(name, &env) != JNI_OK) {
        return -1;
    }

    /* start the thread running */
    result = (*(android_thread_func_t)start)(userData);

    /* unhook us */
    javaDetachThread();
    free(name);

    return result;
}

/*
 * This is invoked from androidCreateThreadEtc() via the callback
 * set with androidSetCreateThreadFunc().
 *
 * We need to create the new thread in such a way that it gets hooked
 * into the VM before it really starts executing.
 */
static int javaCreateThreadEtc(
        android_thread_func_t entryFunction,
        void* userData,
        const char* threadName,
        int32_t threadPriority,
        size_t threadStackSize,
        android_thread_id_t* threadId) {
    void** args = (void**) malloc(3 * sizeof(void*));   // javaThreadShell must free
    int result;

    LOG_ALWAYS_FATAL_IF(threadName == nullptr, "threadName not provided to javaCreateThreadEtc");

    args[0] = (void*) entryFunction;
    args[1] = userData;
    args[2] = (void*) strdup(threadName);   // javaThreadShell must free

    result = androidCreateRawThreadEtc(javaThreadShell, args,
        threadName, threadPriority, threadStackSize, threadId);
    return result;
}

// static
JNIEnv *JavaVMHelper::getJNIEnv() {
    JNIEnv *env;
    JavaVM *vm = sJavaVM.load();
    CHECK(vm != NULL);

    if (vm->GetEnv((void **)&env, JNI_VERSION_1_4) != JNI_OK) {
        return NULL;
    }

    return env;
}

//static
JavaVM *JavaVMHelper::getJavaVM() {
    return sJavaVM.load();
}

// static
void JavaVMHelper::setJavaVM(JavaVM *vm) {
    sJavaVM.store(vm);

    // Ensure that Thread(/*canCallJava*/ true) in libutils is attached to the VM.
    // This is supposed to be done by runtime, but when libutils is used with linker
    // namespace, CreateThreadFunc should be initialized separately within the namespace.
    androidSetCreateThreadFunc((android_create_thread_fn) javaCreateThreadEtc);
}

}  // namespace android
