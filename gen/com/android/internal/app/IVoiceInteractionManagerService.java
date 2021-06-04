/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.app;
public interface IVoiceInteractionManagerService extends android.os.IInterface
{
  /** Default implementation for IVoiceInteractionManagerService. */
  public static class Default implements com.android.internal.app.IVoiceInteractionManagerService
  {
    @Override public void showSession(android.service.voice.IVoiceInteractionService service, android.os.Bundle sessionArgs, int flags) throws android.os.RemoteException
    {
    }
    @Override public boolean deliverNewSession(android.os.IBinder token, android.service.voice.IVoiceInteractionSession session, com.android.internal.app.IVoiceInteractor interactor) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean showSessionFromSession(android.os.IBinder token, android.os.Bundle sessionArgs, int flags) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean hideSessionFromSession(android.os.IBinder token) throws android.os.RemoteException
    {
      return false;
    }
    @Override public int startVoiceActivity(android.os.IBinder token, android.content.Intent intent, java.lang.String resolvedType) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int startAssistantActivity(android.os.IBinder token, android.content.Intent intent, java.lang.String resolvedType) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setKeepAwake(android.os.IBinder token, boolean keepAwake) throws android.os.RemoteException
    {
    }
    @Override public void closeSystemDialogs(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public void finish(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public void setDisabledShowContext(int flags) throws android.os.RemoteException
    {
    }
    @Override public int getDisabledShowContext() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int getUserDisabledShowContext() throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Gets the registered Sound model for keyphrase detection for the current user.
         * May be null if no matching sound model exists.
         *
         * @param keyphraseId The unique identifier for the keyphrase.
         * @param bcp47Locale The BCP47 language tag  for the keyphrase's locale.
         */
    @Override public android.hardware.soundtrigger.SoundTrigger.KeyphraseSoundModel getKeyphraseSoundModel(int keyphraseId, java.lang.String bcp47Locale) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Add/Update the given keyphrase sound model.
         */
    @Override public int updateKeyphraseSoundModel(android.hardware.soundtrigger.SoundTrigger.KeyphraseSoundModel model) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Deletes the given keyphrase sound model for the current user.
         *
         * @param keyphraseId The unique identifier for the keyphrase.
         * @param bcp47Locale The BCP47 language tag  for the keyphrase's locale.
         */
    @Override public int deleteKeyphraseSoundModel(int keyphraseId, java.lang.String bcp47Locale) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Gets the properties of the DSP hardware on this device, null if not present.
         */
    @Override public android.hardware.soundtrigger.SoundTrigger.ModuleProperties getDspModuleProperties(android.service.voice.IVoiceInteractionService service) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Indicates if there's a keyphrase sound model available for the given keyphrase ID.
         * This performs the check for the current user.
         *
         * @param service The current VoiceInteractionService.
         * @param keyphraseId The unique identifier for the keyphrase.
         * @param bcp47Locale The BCP47 language tag  for the keyphrase's locale.
         */
    @Override public boolean isEnrolledForKeyphrase(android.service.voice.IVoiceInteractionService service, int keyphraseId, java.lang.String bcp47Locale) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Starts a recognition for the given keyphrase.
         */
    @Override public int startRecognition(android.service.voice.IVoiceInteractionService service, int keyphraseId, java.lang.String bcp47Locale, android.hardware.soundtrigger.IRecognitionStatusCallback callback, android.hardware.soundtrigger.SoundTrigger.RecognitionConfig recognitionConfig) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Stops a recognition for the given keyphrase.
         */
    @Override public int stopRecognition(android.service.voice.IVoiceInteractionService service, int keyphraseId, android.hardware.soundtrigger.IRecognitionStatusCallback callback) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * @return the component name for the currently active voice interaction service
         */
    @Override public android.content.ComponentName getActiveServiceComponentName() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Shows the session for the currently active service. Used to start a new session from system
         * affordances.
         *
         * @param args the bundle to pass as arguments to the voice interaction session
         * @param sourceFlags flags indicating the source of this show
         * @param showCallback optional callback to be notified when the session was shown
         * @param activityToken optional token of activity that needs to be on top
         */
    @Override public boolean showSessionForActiveService(android.os.Bundle args, int sourceFlags, com.android.internal.app.IVoiceInteractionSessionShowCallback showCallback, android.os.IBinder activityToken) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Hides the session from the active service, if it is showing.
         */
    @Override public void hideCurrentSession() throws android.os.RemoteException
    {
    }
    /**
         * Notifies the active service that a launch was requested from the Keyguard. This will only
         * be called if {@link #activeServiceSupportsLaunchFromKeyguard()} returns true.
         */
    @Override public void launchVoiceAssistFromKeyguard() throws android.os.RemoteException
    {
    }
    /**
         * Indicates whether there is a voice session running (but not necessarily showing).
         */
    @Override public boolean isSessionRunning() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Indicates whether the currently active voice interaction service is capable of handling the
         * assist gesture.
         */
    @Override public boolean activeServiceSupportsAssist() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Indicates whether the currently active voice interaction service is capable of being launched
         * from the lockscreen.
         */
    @Override public boolean activeServiceSupportsLaunchFromKeyguard() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Called when the lockscreen got shown.
         */
    @Override public void onLockscreenShown() throws android.os.RemoteException
    {
    }
    /**
         * Register a voice interaction listener.
         */
    @Override public void registerVoiceInteractionSessionListener(com.android.internal.app.IVoiceInteractionSessionListener listener) throws android.os.RemoteException
    {
    }
    /**
         * Checks the availability of a set of voice actions for the current active voice service.
         * Returns all supported voice actions.
         */
    @Override public void getActiveServiceSupportedActions(java.util.List<java.lang.String> voiceActions, com.android.internal.app.IVoiceActionCheckCallback callback) throws android.os.RemoteException
    {
    }
    /**
         * Provide hints for showing UI.
         */
    @Override public void setUiHints(android.service.voice.IVoiceInteractionService service, android.os.Bundle hints) throws android.os.RemoteException
    {
    }
    /**
         * Requests a list of supported actions from a specific activity.
         */
    @Override public void requestDirectActions(android.os.IBinder token, int taskId, android.os.IBinder assistToken, android.os.RemoteCallback cancellationCallback, android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    /**
         * Requests performing an action from a specific activity.
         */
    @Override public void performDirectAction(android.os.IBinder token, java.lang.String actionId, android.os.Bundle arguments, int taskId, android.os.IBinder assistToken, android.os.RemoteCallback cancellationCallback, android.os.RemoteCallback resultCallback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.app.IVoiceInteractionManagerService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.app.IVoiceInteractionManagerService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.app.IVoiceInteractionManagerService interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.app.IVoiceInteractionManagerService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.app.IVoiceInteractionManagerService))) {
        return ((com.android.internal.app.IVoiceInteractionManagerService)iin);
      }
      return new com.android.internal.app.IVoiceInteractionManagerService.Stub.Proxy(obj);
    }
    @Override public android.os.IBinder asBinder()
    {
      return this;
    }
    @Override public boolean onTransact(int code, android.os.Parcel data, android.os.Parcel reply, int flags) throws android.os.RemoteException
    {
      java.lang.String descriptor = DESCRIPTOR;
      switch (code)
      {
        case INTERFACE_TRANSACTION:
        {
          reply.writeString(descriptor);
          return true;
        }
        case TRANSACTION_showSession:
        {
          data.enforceInterface(descriptor);
          android.service.voice.IVoiceInteractionService _arg0;
          _arg0 = android.service.voice.IVoiceInteractionService.Stub.asInterface(data.readStrongBinder());
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          this.showSession(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_deliverNewSession:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.service.voice.IVoiceInteractionSession _arg1;
          _arg1 = android.service.voice.IVoiceInteractionSession.Stub.asInterface(data.readStrongBinder());
          com.android.internal.app.IVoiceInteractor _arg2;
          _arg2 = com.android.internal.app.IVoiceInteractor.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.deliverNewSession(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_showSessionFromSession:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          boolean _result = this.showSessionFromSession(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_hideSessionFromSession:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _result = this.hideSessionFromSession(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_startVoiceActivity:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.content.Intent _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _result = this.startVoiceActivity(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_startAssistantActivity:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.content.Intent _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _result = this.startAssistantActivity(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setKeepAwake:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setKeepAwake(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_closeSystemDialogs:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.closeSystemDialogs(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_finish:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.finish(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setDisabledShowContext:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setDisabledShowContext(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getDisabledShowContext:
        {
          data.enforceInterface(descriptor);
          int _result = this.getDisabledShowContext();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getUserDisabledShowContext:
        {
          data.enforceInterface(descriptor);
          int _result = this.getUserDisabledShowContext();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getKeyphraseSoundModel:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.hardware.soundtrigger.SoundTrigger.KeyphraseSoundModel _result = this.getKeyphraseSoundModel(_arg0, _arg1);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_updateKeyphraseSoundModel:
        {
          data.enforceInterface(descriptor);
          android.hardware.soundtrigger.SoundTrigger.KeyphraseSoundModel _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.soundtrigger.SoundTrigger.KeyphraseSoundModel.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _result = this.updateKeyphraseSoundModel(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_deleteKeyphraseSoundModel:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.deleteKeyphraseSoundModel(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getDspModuleProperties:
        {
          data.enforceInterface(descriptor);
          android.service.voice.IVoiceInteractionService _arg0;
          _arg0 = android.service.voice.IVoiceInteractionService.Stub.asInterface(data.readStrongBinder());
          android.hardware.soundtrigger.SoundTrigger.ModuleProperties _result = this.getDspModuleProperties(_arg0);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_isEnrolledForKeyphrase:
        {
          data.enforceInterface(descriptor);
          android.service.voice.IVoiceInteractionService _arg0;
          _arg0 = android.service.voice.IVoiceInteractionService.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _result = this.isEnrolledForKeyphrase(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_startRecognition:
        {
          data.enforceInterface(descriptor);
          android.service.voice.IVoiceInteractionService _arg0;
          _arg0 = android.service.voice.IVoiceInteractionService.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.hardware.soundtrigger.IRecognitionStatusCallback _arg3;
          _arg3 = android.hardware.soundtrigger.IRecognitionStatusCallback.Stub.asInterface(data.readStrongBinder());
          android.hardware.soundtrigger.SoundTrigger.RecognitionConfig _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.hardware.soundtrigger.SoundTrigger.RecognitionConfig.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          int _result = this.startRecognition(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_stopRecognition:
        {
          data.enforceInterface(descriptor);
          android.service.voice.IVoiceInteractionService _arg0;
          _arg0 = android.service.voice.IVoiceInteractionService.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          android.hardware.soundtrigger.IRecognitionStatusCallback _arg2;
          _arg2 = android.hardware.soundtrigger.IRecognitionStatusCallback.Stub.asInterface(data.readStrongBinder());
          int _result = this.stopRecognition(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getActiveServiceComponentName:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _result = this.getActiveServiceComponentName();
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_showSessionForActiveService:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          com.android.internal.app.IVoiceInteractionSessionShowCallback _arg2;
          _arg2 = com.android.internal.app.IVoiceInteractionSessionShowCallback.Stub.asInterface(data.readStrongBinder());
          android.os.IBinder _arg3;
          _arg3 = data.readStrongBinder();
          boolean _result = this.showSessionForActiveService(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_hideCurrentSession:
        {
          data.enforceInterface(descriptor);
          this.hideCurrentSession();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_launchVoiceAssistFromKeyguard:
        {
          data.enforceInterface(descriptor);
          this.launchVoiceAssistFromKeyguard();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isSessionRunning:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isSessionRunning();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_activeServiceSupportsAssist:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.activeServiceSupportsAssist();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_activeServiceSupportsLaunchFromKeyguard:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.activeServiceSupportsLaunchFromKeyguard();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_onLockscreenShown:
        {
          data.enforceInterface(descriptor);
          this.onLockscreenShown();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerVoiceInteractionSessionListener:
        {
          data.enforceInterface(descriptor);
          com.android.internal.app.IVoiceInteractionSessionListener _arg0;
          _arg0 = com.android.internal.app.IVoiceInteractionSessionListener.Stub.asInterface(data.readStrongBinder());
          this.registerVoiceInteractionSessionListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getActiveServiceSupportedActions:
        {
          data.enforceInterface(descriptor);
          java.util.List<java.lang.String> _arg0;
          _arg0 = data.createStringArrayList();
          com.android.internal.app.IVoiceActionCheckCallback _arg1;
          _arg1 = com.android.internal.app.IVoiceActionCheckCallback.Stub.asInterface(data.readStrongBinder());
          this.getActiveServiceSupportedActions(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setUiHints:
        {
          data.enforceInterface(descriptor);
          android.service.voice.IVoiceInteractionService _arg0;
          _arg0 = android.service.voice.IVoiceInteractionService.Stub.asInterface(data.readStrongBinder());
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.setUiHints(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_requestDirectActions:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          android.os.IBinder _arg2;
          _arg2 = data.readStrongBinder();
          android.os.RemoteCallback _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          android.os.RemoteCallback _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          this.requestDirectActions(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_performDirectAction:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          android.os.IBinder _arg4;
          _arg4 = data.readStrongBinder();
          android.os.RemoteCallback _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          android.os.RemoteCallback _arg6;
          if ((0!=data.readInt())) {
            _arg6 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg6 = null;
          }
          this.performDirectAction(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.app.IVoiceInteractionManagerService
    {
      private android.os.IBinder mRemote;
      Proxy(android.os.IBinder remote)
      {
        mRemote = remote;
      }
      @Override public android.os.IBinder asBinder()
      {
        return mRemote;
      }
      public java.lang.String getInterfaceDescriptor()
      {
        return DESCRIPTOR;
      }
      @Override public void showSession(android.service.voice.IVoiceInteractionService service, android.os.Bundle sessionArgs, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((service!=null))?(service.asBinder()):(null)));
          if ((sessionArgs!=null)) {
            _data.writeInt(1);
            sessionArgs.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_showSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showSession(service, sessionArgs, flags);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean deliverNewSession(android.os.IBinder token, android.service.voice.IVoiceInteractionSession session, com.android.internal.app.IVoiceInteractor interactor) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeStrongBinder((((session!=null))?(session.asBinder()):(null)));
          _data.writeStrongBinder((((interactor!=null))?(interactor.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_deliverNewSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().deliverNewSession(token, session, interactor);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public boolean showSessionFromSession(android.os.IBinder token, android.os.Bundle sessionArgs, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          if ((sessionArgs!=null)) {
            _data.writeInt(1);
            sessionArgs.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_showSessionFromSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().showSessionFromSession(token, sessionArgs, flags);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public boolean hideSessionFromSession(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hideSessionFromSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hideSessionFromSession(token);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int startVoiceActivity(android.os.IBinder token, android.content.Intent intent, java.lang.String resolvedType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(resolvedType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startVoiceActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startVoiceActivity(token, intent, resolvedType);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int startAssistantActivity(android.os.IBinder token, android.content.Intent intent, java.lang.String resolvedType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(resolvedType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startAssistantActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startAssistantActivity(token, intent, resolvedType);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setKeepAwake(android.os.IBinder token, boolean keepAwake) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(((keepAwake)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setKeepAwake, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setKeepAwake(token, keepAwake);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void closeSystemDialogs(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_closeSystemDialogs, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().closeSystemDialogs(token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void finish(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_finish, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().finish(token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setDisabledShowContext(int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDisabledShowContext, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDisabledShowContext(flags);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getDisabledShowContext() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDisabledShowContext, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDisabledShowContext();
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int getUserDisabledShowContext() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getUserDisabledShowContext, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getUserDisabledShowContext();
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Gets the registered Sound model for keyphrase detection for the current user.
           * May be null if no matching sound model exists.
           *
           * @param keyphraseId The unique identifier for the keyphrase.
           * @param bcp47Locale The BCP47 language tag  for the keyphrase's locale.
           */
      @Override public android.hardware.soundtrigger.SoundTrigger.KeyphraseSoundModel getKeyphraseSoundModel(int keyphraseId, java.lang.String bcp47Locale) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.hardware.soundtrigger.SoundTrigger.KeyphraseSoundModel _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(keyphraseId);
          _data.writeString(bcp47Locale);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getKeyphraseSoundModel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getKeyphraseSoundModel(keyphraseId, bcp47Locale);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.hardware.soundtrigger.SoundTrigger.KeyphraseSoundModel.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Add/Update the given keyphrase sound model.
           */
      @Override public int updateKeyphraseSoundModel(android.hardware.soundtrigger.SoundTrigger.KeyphraseSoundModel model) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((model!=null)) {
            _data.writeInt(1);
            model.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateKeyphraseSoundModel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().updateKeyphraseSoundModel(model);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Deletes the given keyphrase sound model for the current user.
           *
           * @param keyphraseId The unique identifier for the keyphrase.
           * @param bcp47Locale The BCP47 language tag  for the keyphrase's locale.
           */
      @Override public int deleteKeyphraseSoundModel(int keyphraseId, java.lang.String bcp47Locale) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(keyphraseId);
          _data.writeString(bcp47Locale);
          boolean _status = mRemote.transact(Stub.TRANSACTION_deleteKeyphraseSoundModel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().deleteKeyphraseSoundModel(keyphraseId, bcp47Locale);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Gets the properties of the DSP hardware on this device, null if not present.
           */
      @Override public android.hardware.soundtrigger.SoundTrigger.ModuleProperties getDspModuleProperties(android.service.voice.IVoiceInteractionService service) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.hardware.soundtrigger.SoundTrigger.ModuleProperties _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((service!=null))?(service.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDspModuleProperties, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDspModuleProperties(service);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.hardware.soundtrigger.SoundTrigger.ModuleProperties.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Indicates if there's a keyphrase sound model available for the given keyphrase ID.
           * This performs the check for the current user.
           *
           * @param service The current VoiceInteractionService.
           * @param keyphraseId The unique identifier for the keyphrase.
           * @param bcp47Locale The BCP47 language tag  for the keyphrase's locale.
           */
      @Override public boolean isEnrolledForKeyphrase(android.service.voice.IVoiceInteractionService service, int keyphraseId, java.lang.String bcp47Locale) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((service!=null))?(service.asBinder()):(null)));
          _data.writeInt(keyphraseId);
          _data.writeString(bcp47Locale);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isEnrolledForKeyphrase, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isEnrolledForKeyphrase(service, keyphraseId, bcp47Locale);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Starts a recognition for the given keyphrase.
           */
      @Override public int startRecognition(android.service.voice.IVoiceInteractionService service, int keyphraseId, java.lang.String bcp47Locale, android.hardware.soundtrigger.IRecognitionStatusCallback callback, android.hardware.soundtrigger.SoundTrigger.RecognitionConfig recognitionConfig) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((service!=null))?(service.asBinder()):(null)));
          _data.writeInt(keyphraseId);
          _data.writeString(bcp47Locale);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          if ((recognitionConfig!=null)) {
            _data.writeInt(1);
            recognitionConfig.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startRecognition, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startRecognition(service, keyphraseId, bcp47Locale, callback, recognitionConfig);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Stops a recognition for the given keyphrase.
           */
      @Override public int stopRecognition(android.service.voice.IVoiceInteractionService service, int keyphraseId, android.hardware.soundtrigger.IRecognitionStatusCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((service!=null))?(service.asBinder()):(null)));
          _data.writeInt(keyphraseId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopRecognition, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().stopRecognition(service, keyphraseId, callback);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * @return the component name for the currently active voice interaction service
           */
      @Override public android.content.ComponentName getActiveServiceComponentName() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.ComponentName _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveServiceComponentName, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActiveServiceComponentName();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.ComponentName.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Shows the session for the currently active service. Used to start a new session from system
           * affordances.
           *
           * @param args the bundle to pass as arguments to the voice interaction session
           * @param sourceFlags flags indicating the source of this show
           * @param showCallback optional callback to be notified when the session was shown
           * @param activityToken optional token of activity that needs to be on top
           */
      @Override public boolean showSessionForActiveService(android.os.Bundle args, int sourceFlags, com.android.internal.app.IVoiceInteractionSessionShowCallback showCallback, android.os.IBinder activityToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((args!=null)) {
            _data.writeInt(1);
            args.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(sourceFlags);
          _data.writeStrongBinder((((showCallback!=null))?(showCallback.asBinder()):(null)));
          _data.writeStrongBinder(activityToken);
          boolean _status = mRemote.transact(Stub.TRANSACTION_showSessionForActiveService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().showSessionForActiveService(args, sourceFlags, showCallback, activityToken);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Hides the session from the active service, if it is showing.
           */
      @Override public void hideCurrentSession() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hideCurrentSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().hideCurrentSession();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Notifies the active service that a launch was requested from the Keyguard. This will only
           * be called if {@link #activeServiceSupportsLaunchFromKeyguard()} returns true.
           */
      @Override public void launchVoiceAssistFromKeyguard() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_launchVoiceAssistFromKeyguard, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().launchVoiceAssistFromKeyguard();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Indicates whether there is a voice session running (but not necessarily showing).
           */
      @Override public boolean isSessionRunning() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isSessionRunning, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isSessionRunning();
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Indicates whether the currently active voice interaction service is capable of handling the
           * assist gesture.
           */
      @Override public boolean activeServiceSupportsAssist() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_activeServiceSupportsAssist, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().activeServiceSupportsAssist();
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Indicates whether the currently active voice interaction service is capable of being launched
           * from the lockscreen.
           */
      @Override public boolean activeServiceSupportsLaunchFromKeyguard() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_activeServiceSupportsLaunchFromKeyguard, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().activeServiceSupportsLaunchFromKeyguard();
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Called when the lockscreen got shown.
           */
      @Override public void onLockscreenShown() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onLockscreenShown, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onLockscreenShown();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Register a voice interaction listener.
           */
      @Override public void registerVoiceInteractionSessionListener(com.android.internal.app.IVoiceInteractionSessionListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerVoiceInteractionSessionListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerVoiceInteractionSessionListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Checks the availability of a set of voice actions for the current active voice service.
           * Returns all supported voice actions.
           */
      @Override public void getActiveServiceSupportedActions(java.util.List<java.lang.String> voiceActions, com.android.internal.app.IVoiceActionCheckCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStringList(voiceActions);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveServiceSupportedActions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getActiveServiceSupportedActions(voiceActions, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Provide hints for showing UI.
           */
      @Override public void setUiHints(android.service.voice.IVoiceInteractionService service, android.os.Bundle hints) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((service!=null))?(service.asBinder()):(null)));
          if ((hints!=null)) {
            _data.writeInt(1);
            hints.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setUiHints, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setUiHints(service, hints);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Requests a list of supported actions from a specific activity.
           */
      @Override public void requestDirectActions(android.os.IBinder token, int taskId, android.os.IBinder assistToken, android.os.RemoteCallback cancellationCallback, android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(taskId);
          _data.writeStrongBinder(assistToken);
          if ((cancellationCallback!=null)) {
            _data.writeInt(1);
            cancellationCallback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestDirectActions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestDirectActions(token, taskId, assistToken, cancellationCallback, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Requests performing an action from a specific activity.
           */
      @Override public void performDirectAction(android.os.IBinder token, java.lang.String actionId, android.os.Bundle arguments, int taskId, android.os.IBinder assistToken, android.os.RemoteCallback cancellationCallback, android.os.RemoteCallback resultCallback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeString(actionId);
          if ((arguments!=null)) {
            _data.writeInt(1);
            arguments.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(taskId);
          _data.writeStrongBinder(assistToken);
          if ((cancellationCallback!=null)) {
            _data.writeInt(1);
            cancellationCallback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((resultCallback!=null)) {
            _data.writeInt(1);
            resultCallback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_performDirectAction, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().performDirectAction(token, actionId, arguments, taskId, assistToken, cancellationCallback, resultCallback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.internal.app.IVoiceInteractionManagerService sDefaultImpl;
    }
    static final int TRANSACTION_showSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_deliverNewSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_showSessionFromSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_hideSessionFromSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_startVoiceActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_startAssistantActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_setKeepAwake = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_closeSystemDialogs = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_finish = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_setDisabledShowContext = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_getDisabledShowContext = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_getUserDisabledShowContext = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_getKeyphraseSoundModel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_updateKeyphraseSoundModel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_deleteKeyphraseSoundModel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_getDspModuleProperties = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_isEnrolledForKeyphrase = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_startRecognition = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_stopRecognition = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_getActiveServiceComponentName = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_showSessionForActiveService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_hideCurrentSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_launchVoiceAssistFromKeyguard = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_isSessionRunning = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_activeServiceSupportsAssist = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_activeServiceSupportsLaunchFromKeyguard = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_onLockscreenShown = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_registerVoiceInteractionSessionListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_getActiveServiceSupportedActions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_setUiHints = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_requestDirectActions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_performDirectAction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    public static boolean setDefaultImpl(com.android.internal.app.IVoiceInteractionManagerService impl) {
      // Only one user of this interface can use this function
      // at a time. This is a heuristic to detect if two different
      // users in the same process use this function.
      if (Stub.Proxy.sDefaultImpl != null) {
        throw new IllegalStateException("setDefaultImpl() called twice");
      }
      if (impl != null) {
        Stub.Proxy.sDefaultImpl = impl;
        return true;
      }
      return false;
    }
    public static com.android.internal.app.IVoiceInteractionManagerService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void showSession(android.service.voice.IVoiceInteractionService service, android.os.Bundle sessionArgs, int flags) throws android.os.RemoteException;
  public boolean deliverNewSession(android.os.IBinder token, android.service.voice.IVoiceInteractionSession session, com.android.internal.app.IVoiceInteractor interactor) throws android.os.RemoteException;
  public boolean showSessionFromSession(android.os.IBinder token, android.os.Bundle sessionArgs, int flags) throws android.os.RemoteException;
  public boolean hideSessionFromSession(android.os.IBinder token) throws android.os.RemoteException;
  public int startVoiceActivity(android.os.IBinder token, android.content.Intent intent, java.lang.String resolvedType) throws android.os.RemoteException;
  public int startAssistantActivity(android.os.IBinder token, android.content.Intent intent, java.lang.String resolvedType) throws android.os.RemoteException;
  public void setKeepAwake(android.os.IBinder token, boolean keepAwake) throws android.os.RemoteException;
  public void closeSystemDialogs(android.os.IBinder token) throws android.os.RemoteException;
  public void finish(android.os.IBinder token) throws android.os.RemoteException;
  public void setDisabledShowContext(int flags) throws android.os.RemoteException;
  public int getDisabledShowContext() throws android.os.RemoteException;
  public int getUserDisabledShowContext() throws android.os.RemoteException;
  /**
       * Gets the registered Sound model for keyphrase detection for the current user.
       * May be null if no matching sound model exists.
       *
       * @param keyphraseId The unique identifier for the keyphrase.
       * @param bcp47Locale The BCP47 language tag  for the keyphrase's locale.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/app/IVoiceInteractionManagerService.aidl:55:1:55:25")
  public android.hardware.soundtrigger.SoundTrigger.KeyphraseSoundModel getKeyphraseSoundModel(int keyphraseId, java.lang.String bcp47Locale) throws android.os.RemoteException;
  /**
       * Add/Update the given keyphrase sound model.
       */
  public int updateKeyphraseSoundModel(android.hardware.soundtrigger.SoundTrigger.KeyphraseSoundModel model) throws android.os.RemoteException;
  /**
       * Deletes the given keyphrase sound model for the current user.
       *
       * @param keyphraseId The unique identifier for the keyphrase.
       * @param bcp47Locale The BCP47 language tag  for the keyphrase's locale.
       */
  public int deleteKeyphraseSoundModel(int keyphraseId, java.lang.String bcp47Locale) throws android.os.RemoteException;
  /**
       * Gets the properties of the DSP hardware on this device, null if not present.
       */
  public android.hardware.soundtrigger.SoundTrigger.ModuleProperties getDspModuleProperties(android.service.voice.IVoiceInteractionService service) throws android.os.RemoteException;
  /**
       * Indicates if there's a keyphrase sound model available for the given keyphrase ID.
       * This performs the check for the current user.
       *
       * @param service The current VoiceInteractionService.
       * @param keyphraseId The unique identifier for the keyphrase.
       * @param bcp47Locale The BCP47 language tag  for the keyphrase's locale.
       */
  public boolean isEnrolledForKeyphrase(android.service.voice.IVoiceInteractionService service, int keyphraseId, java.lang.String bcp47Locale) throws android.os.RemoteException;
  /**
       * Starts a recognition for the given keyphrase.
       */
  public int startRecognition(android.service.voice.IVoiceInteractionService service, int keyphraseId, java.lang.String bcp47Locale, android.hardware.soundtrigger.IRecognitionStatusCallback callback, android.hardware.soundtrigger.SoundTrigger.RecognitionConfig recognitionConfig) throws android.os.RemoteException;
  /**
       * Stops a recognition for the given keyphrase.
       */
  public int stopRecognition(android.service.voice.IVoiceInteractionService service, int keyphraseId, android.hardware.soundtrigger.IRecognitionStatusCallback callback) throws android.os.RemoteException;
  /**
       * @return the component name for the currently active voice interaction service
       */
  public android.content.ComponentName getActiveServiceComponentName() throws android.os.RemoteException;
  /**
       * Shows the session for the currently active service. Used to start a new session from system
       * affordances.
       *
       * @param args the bundle to pass as arguments to the voice interaction session
       * @param sourceFlags flags indicating the source of this show
       * @param showCallback optional callback to be notified when the session was shown
       * @param activityToken optional token of activity that needs to be on top
       */
  public boolean showSessionForActiveService(android.os.Bundle args, int sourceFlags, com.android.internal.app.IVoiceInteractionSessionShowCallback showCallback, android.os.IBinder activityToken) throws android.os.RemoteException;
  /**
       * Hides the session from the active service, if it is showing.
       */
  public void hideCurrentSession() throws android.os.RemoteException;
  /**
       * Notifies the active service that a launch was requested from the Keyguard. This will only
       * be called if {@link #activeServiceSupportsLaunchFromKeyguard()} returns true.
       */
  public void launchVoiceAssistFromKeyguard() throws android.os.RemoteException;
  /**
       * Indicates whether there is a voice session running (but not necessarily showing).
       */
  public boolean isSessionRunning() throws android.os.RemoteException;
  /**
       * Indicates whether the currently active voice interaction service is capable of handling the
       * assist gesture.
       */
  public boolean activeServiceSupportsAssist() throws android.os.RemoteException;
  /**
       * Indicates whether the currently active voice interaction service is capable of being launched
       * from the lockscreen.
       */
  public boolean activeServiceSupportsLaunchFromKeyguard() throws android.os.RemoteException;
  /**
       * Called when the lockscreen got shown.
       */
  public void onLockscreenShown() throws android.os.RemoteException;
  /**
       * Register a voice interaction listener.
       */
  public void registerVoiceInteractionSessionListener(com.android.internal.app.IVoiceInteractionSessionListener listener) throws android.os.RemoteException;
  /**
       * Checks the availability of a set of voice actions for the current active voice service.
       * Returns all supported voice actions.
       */
  public void getActiveServiceSupportedActions(java.util.List<java.lang.String> voiceActions, com.android.internal.app.IVoiceActionCheckCallback callback) throws android.os.RemoteException;
  /**
       * Provide hints for showing UI.
       */
  public void setUiHints(android.service.voice.IVoiceInteractionService service, android.os.Bundle hints) throws android.os.RemoteException;
  /**
       * Requests a list of supported actions from a specific activity.
       */
  public void requestDirectActions(android.os.IBinder token, int taskId, android.os.IBinder assistToken, android.os.RemoteCallback cancellationCallback, android.os.RemoteCallback callback) throws android.os.RemoteException;
  /**
       * Requests performing an action from a specific activity.
       */
  public void performDirectAction(android.os.IBinder token, java.lang.String actionId, android.os.Bundle arguments, int taskId, android.os.IBinder assistToken, android.os.RemoteCallback cancellationCallback, android.os.RemoteCallback resultCallback) throws android.os.RemoteException;
}
