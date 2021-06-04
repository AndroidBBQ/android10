/*
 * Copyright (C) 2005 The Android Open Source Project
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

#ifndef ANDROID_IPC_THREAD_STATE_H
#define ANDROID_IPC_THREAD_STATE_H

#include <utils/Errors.h>
#include <binder/Parcel.h>
#include <binder/ProcessState.h>
#include <utils/Vector.h>

#if defined(_WIN32)
typedef  int  uid_t;
#endif

// ---------------------------------------------------------------------------
namespace android {

class IPCThreadStateBase;

class IPCThreadState
{
public:
    static  IPCThreadState*     self();
    static  IPCThreadState*     selfOrNull();  // self(), but won't instantiate
    
            sp<ProcessState>    process();
            
            status_t            clearLastError();

            pid_t               getCallingPid() const;
            // nullptr if unavailable
            //
            // this can't be restored once it's cleared, and it does not return the
            // context of the current process when not in a binder call.
            const char*         getCallingSid() const;
            uid_t               getCallingUid() const;

            void                setStrictModePolicy(int32_t policy);
            int32_t             getStrictModePolicy() const;

            // See Binder#setCallingWorkSourceUid in Binder.java.
            int64_t             setCallingWorkSourceUid(uid_t uid);
            // Internal only. Use setCallingWorkSourceUid(uid) instead.
            int64_t             setCallingWorkSourceUidWithoutPropagation(uid_t uid);
            // See Binder#getCallingWorkSourceUid in Binder.java.
            uid_t               getCallingWorkSourceUid() const;
            // See Binder#clearCallingWorkSource in Binder.java.
            int64_t             clearCallingWorkSource();
            // See Binder#restoreCallingWorkSource in Binder.java.
            void                restoreCallingWorkSource(int64_t token);
            void                clearPropagateWorkSource();
            bool                shouldPropagateWorkSource() const;

            void                setLastTransactionBinderFlags(int32_t flags);
            int32_t             getLastTransactionBinderFlags() const;

            int64_t             clearCallingIdentity();
            // Restores PID/UID (not SID)
            void                restoreCallingIdentity(int64_t token);
            
            int                 setupPolling(int* fd);
            status_t            handlePolledCommands();
            void                flushCommands();

            void                joinThreadPool(bool isMain = true);
            
            // Stop the local process.
            void                stopProcess(bool immediate = true);
            
            status_t            transact(int32_t handle,
                                         uint32_t code, const Parcel& data,
                                         Parcel* reply, uint32_t flags);

            void                incStrongHandle(int32_t handle, BpBinder *proxy);
            void                decStrongHandle(int32_t handle);
            void                incWeakHandle(int32_t handle, BpBinder *proxy);
            void                decWeakHandle(int32_t handle);
            status_t            attemptIncStrongHandle(int32_t handle);
    static  void                expungeHandle(int32_t handle, IBinder* binder);
            status_t            requestDeathNotification(   int32_t handle,
                                                            BpBinder* proxy); 
            status_t            clearDeathNotification( int32_t handle,
                                                        BpBinder* proxy); 

    static  void                shutdown();

    // Call this to disable switching threads to background scheduling when
    // receiving incoming IPC calls.  This is specifically here for the
    // Android system process, since it expects to have background apps calling
    // in to it but doesn't want to acquire locks in its services while in
    // the background.
    static  void                disableBackgroundScheduling(bool disable);
            bool                backgroundSchedulingDisabled();

            // Call blocks until the number of executing binder threads is less than
            // the maximum number of binder threads threads allowed for this process.
            void                blockUntilThreadAvailable();


            // Is this thread currently serving a binder call. This method
            // returns true if while traversing backwards from the function call
            // stack for this thread, we encounter a function serving a binder
            // call before encountering a hwbinder call / hitting the end of the
            // call stack.
            // Eg: If thread T1 went through the following call pattern
            //     1) T1 receives and executes hwbinder call H1.
            //     2) While handling H1, T1 makes binder call B1.
            //     3) The handler of B1, calls into T1 with a callback B2.
            // If isServingCall() is called during H1 before 3), this method
            // will return false, else true.
            //
            //  ----
            // | B2 | ---> While callback B2 is being handled, during 3).
            //  ----
            // | H1 | ---> While H1 is being handled.
            //  ----
            // Fig: Thread Call stack while handling B2
            //
            // This is since after 3), while traversing the thread call stack,
            // we hit a binder call before a hwbinder call / end of stack. This
            // method may be typically used to determine whether to use
            // hardware::IPCThreadState methods or IPCThreadState methods to
            // infer information about thread state.
            bool                isServingCall() const;

            // The work source represents the UID of the process we should attribute the transaction
            // to. We use -1 to specify that the work source was not set using #setWorkSource.
            //
            // This constant needs to be kept in sync with Binder.UNSET_WORKSOURCE from the Java
            // side.
            static const int32_t kUnsetWorkSource = -1;

private:
                                IPCThreadState();
                                ~IPCThreadState();

            status_t            sendReply(const Parcel& reply, uint32_t flags);
            status_t            waitForResponse(Parcel *reply,
                                                status_t *acquireResult=nullptr);
            status_t            talkWithDriver(bool doReceive=true);
            status_t            writeTransactionData(int32_t cmd,
                                                     uint32_t binderFlags,
                                                     int32_t handle,
                                                     uint32_t code,
                                                     const Parcel& data,
                                                     status_t* statusBuffer);
            status_t            getAndExecuteCommand();
            status_t            executeCommand(int32_t command);
            void                processPendingDerefs();
            void                processPostWriteDerefs();

            void                clearCaller();

    static  void                threadDestructor(void *st);
    static  void                freeBuffer(Parcel* parcel,
                                           const uint8_t* data, size_t dataSize,
                                           const binder_size_t* objects, size_t objectsSize,
                                           void* cookie);
    
    const   sp<ProcessState>    mProcess;
            Vector<BBinder*>    mPendingStrongDerefs;
            Vector<RefBase::weakref_type*> mPendingWeakDerefs;
            Vector<RefBase*>    mPostWriteStrongDerefs;
            Vector<RefBase::weakref_type*> mPostWriteWeakDerefs;
            Parcel              mIn;
            Parcel              mOut;
            status_t            mLastError;
            pid_t               mCallingPid;
            const char*         mCallingSid;
            uid_t               mCallingUid;
            // The UID of the process who is responsible for this transaction.
            // This is used for resource attribution.
            int32_t             mWorkSource;
            // Whether the work source should be propagated.
            bool                mPropagateWorkSource;
            int32_t             mStrictModePolicy;
            int32_t             mLastTransactionBinderFlags;
            IPCThreadStateBase  *mIPCThreadStateBase;

            ProcessState::CallRestriction mCallRestriction;
};

}; // namespace android

// ---------------------------------------------------------------------------

#endif // ANDROID_IPC_THREAD_STATE_H
