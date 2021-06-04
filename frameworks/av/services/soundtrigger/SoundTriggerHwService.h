/*
 * Copyright (C) 2008 The Android Open Source Project
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

#ifndef ANDROID_HARDWARE_SOUNDTRIGGER_HAL_SERVICE_H
#define ANDROID_HARDWARE_SOUNDTRIGGER_HAL_SERVICE_H

#include <utils/Vector.h>
//#include <binder/AppOpsManager.h>
#include <binder/MemoryDealer.h>
#include <binder/BinderService.h>
#include <binder/IAppOpsCallback.h>
#include <soundtrigger/ISoundTriggerHwService.h>
#include <soundtrigger/ISoundTrigger.h>
#include <soundtrigger/ISoundTriggerClient.h>
#include <system/sound_trigger.h>
#include "SoundTriggerHalInterface.h"

namespace android {

class MemoryHeapBase;

class SoundTriggerHwService :
    public BinderService<SoundTriggerHwService>,
    public BnSoundTriggerHwService
{
    friend class BinderService<SoundTriggerHwService>;
public:
    class Module;
    class ModuleClient;

    static char const* getServiceName() { return "media.sound_trigger_hw"; }

                        SoundTriggerHwService();
    virtual             ~SoundTriggerHwService();

    // ISoundTriggerHwService
    virtual status_t listModules(const String16& opPackageName,
                                 struct sound_trigger_module_descriptor *modules,
                                 uint32_t *numModules);

    virtual status_t attach(const String16& opPackageName,
                            const sound_trigger_module_handle_t handle,
                            const sp<ISoundTriggerClient>& client,
                            sp<ISoundTrigger>& module);

    virtual status_t setCaptureState(bool active);

    virtual status_t    onTransact(uint32_t code, const Parcel& data,
                                   Parcel* reply, uint32_t flags);

    virtual status_t    dump(int fd, const Vector<String16>& args);

    class Model : public RefBase {
     public:

        enum {
            STATE_IDLE,
            STATE_ACTIVE
        };

        Model(sound_model_handle_t handle, audio_session_t session, audio_io_handle_t ioHandle,
              audio_devices_t device, sound_trigger_sound_model_type_t type,
              sp<ModuleClient>& moduleClient);
        ~Model() {}

        sound_model_handle_t    mHandle;
        int                     mState;
        audio_session_t         mCaptureSession;
        audio_io_handle_t       mCaptureIOHandle;
        audio_devices_t         mCaptureDevice;
        sound_trigger_sound_model_type_t mType;
        struct sound_trigger_recognition_config mConfig;
        sp<ModuleClient>        mModuleClient;
    };

    class CallbackEvent : public RefBase {
    public:
        typedef enum {
            TYPE_RECOGNITION,
            TYPE_SOUNDMODEL,
            TYPE_SERVICE_STATE,
        } event_type;
        CallbackEvent(event_type type, sp<IMemory> memory);

        virtual             ~CallbackEvent();

        void setModule(wp<Module> module) { mModule = module; }
        void setModuleClient(wp<ModuleClient> moduleClient) { mModuleClient = moduleClient; }

        event_type mType;
        sp<IMemory> mMemory;
        wp<Module> mModule;
        wp<ModuleClient> mModuleClient;
    };

    class Module : public RefBase {
    public:

       Module(const sp<SoundTriggerHwService>& service,
              const sp<SoundTriggerHalInterface>& halInterface,
              sound_trigger_module_descriptor descriptor);

       virtual ~Module();

       virtual status_t loadSoundModel(const sp<IMemory>& modelMemory,
                                       sp<ModuleClient> moduleClient,
                                       sound_model_handle_t *handle);

       virtual status_t unloadSoundModel(sound_model_handle_t handle);

       virtual status_t startRecognition(sound_model_handle_t handle,
                                         const sp<IMemory>& dataMemory);
       virtual status_t stopRecognition(sound_model_handle_t handle);
       virtual status_t getModelState(sound_model_handle_t handle);

       sp<SoundTriggerHalInterface> halInterface() const { return mHalInterface; }
       struct sound_trigger_module_descriptor descriptor() { return mDescriptor; }
       wp<SoundTriggerHwService> service() const { return mService; }
       bool isConcurrentCaptureAllowed() const { return mDescriptor.properties.concurrent_capture; }

       sp<Model> getModel(sound_model_handle_t handle);

       void setCaptureState_l(bool active);

       sp<ModuleClient> addClient(const sp<ISoundTriggerClient>& client,
                                  const String16& opPackageName);

       void detach(const sp<ModuleClient>& moduleClient);

       void onCallbackEvent(const sp<CallbackEvent>& event);

    private:

        Mutex                                  mLock;
        wp<SoundTriggerHwService>              mService;
        sp<SoundTriggerHalInterface>           mHalInterface;
        struct sound_trigger_module_descriptor mDescriptor;
        Vector< sp<ModuleClient> >             mModuleClients;
        DefaultKeyedVector< sound_model_handle_t, sp<Model> >     mModels;
        sound_trigger_service_state_t          mServiceState;
    }; // class Module

    class ModuleClient : public virtual RefBase,
                         public BnSoundTrigger,
                         public IBinder::DeathRecipient {
    public:

       ModuleClient(const sp<Module>& module,
              const sp<ISoundTriggerClient>& client,
              const String16& opPackageName);

       virtual ~ModuleClient();

       virtual void detach();

       virtual status_t loadSoundModel(const sp<IMemory>& modelMemory,
                                       sound_model_handle_t *handle);

       virtual status_t unloadSoundModel(sound_model_handle_t handle);

       virtual status_t startRecognition(sound_model_handle_t handle,
                                         const sp<IMemory>& dataMemory);
       virtual status_t stopRecognition(sound_model_handle_t handle);
       virtual status_t getModelState(sound_model_handle_t handle);

       virtual status_t dump(int fd, const Vector<String16>& args);

       virtual void onFirstRef();

       // IBinder::DeathRecipient implementation
       virtual void        binderDied(const wp<IBinder> &who);

       void onCallbackEvent(const sp<CallbackEvent>& event);

       void setCaptureState_l(bool active);

       sp<ISoundTriggerClient> client() const { return mClient; }

    private:

        mutable Mutex               mLock;
        wp<Module>                  mModule;
        sp<ISoundTriggerClient>     mClient;
        const String16              mOpPackageName;
    }; // class ModuleClient

    class CallbackThread : public Thread {
    public:

        explicit CallbackThread(const wp<SoundTriggerHwService>& service);

        virtual             ~CallbackThread();

        // Thread virtuals
        virtual bool        threadLoop();

        // RefBase
        virtual void        onFirstRef();

                void        exit();
                void        sendCallbackEvent(const sp<CallbackEvent>& event);

    private:
        wp<SoundTriggerHwService>   mService;
        Condition                   mCallbackCond;
        Mutex                       mCallbackLock;
        Vector< sp<CallbackEvent> > mEventQueue;
    };

    static void recognitionCallback(struct sound_trigger_recognition_event *event, void *cookie);
           sp<IMemory> prepareRecognitionEvent(struct sound_trigger_recognition_event *event);
           void sendRecognitionEvent(struct sound_trigger_recognition_event *event, Module *module);

    static void soundModelCallback(struct sound_trigger_model_event *event, void *cookie);
           sp<IMemory> prepareSoundModelEvent(struct sound_trigger_model_event *event);
           void sendSoundModelEvent(struct sound_trigger_model_event *event, Module *module);

           sp<IMemory> prepareServiceStateEvent(sound_trigger_service_state_t state);
           void sendServiceStateEvent(sound_trigger_service_state_t state, Module *module);
           void sendServiceStateEvent(sound_trigger_service_state_t state,
                                      ModuleClient *moduleClient);

           void sendCallbackEvent(const sp<CallbackEvent>& event);
           void onCallbackEvent(const sp<CallbackEvent>& event);

private:

    virtual void onFirstRef();

    Mutex               mServiceLock;
    volatile int32_t    mNextUniqueId;
    DefaultKeyedVector< sound_trigger_module_handle_t, sp<Module> >     mModules;
    sp<CallbackThread>  mCallbackThread;
    sp<MemoryDealer>    mMemoryDealer;
    Mutex               mMemoryDealerLock;
    bool                mCaptureState;
};

} // namespace android

#endif // ANDROID_HARDWARE_SOUNDTRIGGER_HAL_SERVICE_H
