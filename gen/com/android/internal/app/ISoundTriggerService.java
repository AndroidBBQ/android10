/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.app;
/**
 * Service interface for a generic sound recognition model.
 * @hide
 */
public interface ISoundTriggerService extends android.os.IInterface
{
  /** Default implementation for ISoundTriggerService. */
  public static class Default implements com.android.internal.app.ISoundTriggerService
  {
    @Override public android.hardware.soundtrigger.SoundTrigger.GenericSoundModel getSoundModel(android.os.ParcelUuid soundModelId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void updateSoundModel(android.hardware.soundtrigger.SoundTrigger.GenericSoundModel soundModel) throws android.os.RemoteException
    {
    }
    @Override public void deleteSoundModel(android.os.ParcelUuid soundModelId) throws android.os.RemoteException
    {
    }
    @Override public int startRecognition(android.os.ParcelUuid soundModelId, android.hardware.soundtrigger.IRecognitionStatusCallback callback, android.hardware.soundtrigger.SoundTrigger.RecognitionConfig config) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int stopRecognition(android.os.ParcelUuid soundModelId, android.hardware.soundtrigger.IRecognitionStatusCallback callback) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int loadGenericSoundModel(android.hardware.soundtrigger.SoundTrigger.GenericSoundModel soundModel) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int loadKeyphraseSoundModel(android.hardware.soundtrigger.SoundTrigger.KeyphraseSoundModel soundModel) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int startRecognitionForService(android.os.ParcelUuid soundModelId, android.os.Bundle params, android.content.ComponentName callbackIntent, android.hardware.soundtrigger.SoundTrigger.RecognitionConfig config) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int stopRecognitionForService(android.os.ParcelUuid soundModelId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int unloadSoundModel(android.os.ParcelUuid soundModelId) throws android.os.RemoteException
    {
      return 0;
    }
    /** For both ...Intent and ...Service based usage */
    @Override public boolean isRecognitionActive(android.os.ParcelUuid parcelUuid) throws android.os.RemoteException
    {
      return false;
    }
    @Override public int getModelState(android.os.ParcelUuid soundModelId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.app.ISoundTriggerService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.app.ISoundTriggerService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.app.ISoundTriggerService interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.app.ISoundTriggerService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.app.ISoundTriggerService))) {
        return ((com.android.internal.app.ISoundTriggerService)iin);
      }
      return new com.android.internal.app.ISoundTriggerService.Stub.Proxy(obj);
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
        case TRANSACTION_getSoundModel:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelUuid _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelUuid.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.hardware.soundtrigger.SoundTrigger.GenericSoundModel _result = this.getSoundModel(_arg0);
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
        case TRANSACTION_updateSoundModel:
        {
          data.enforceInterface(descriptor);
          android.hardware.soundtrigger.SoundTrigger.GenericSoundModel _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.soundtrigger.SoundTrigger.GenericSoundModel.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.updateSoundModel(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_deleteSoundModel:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelUuid _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelUuid.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.deleteSoundModel(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startRecognition:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelUuid _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelUuid.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.hardware.soundtrigger.IRecognitionStatusCallback _arg1;
          _arg1 = android.hardware.soundtrigger.IRecognitionStatusCallback.Stub.asInterface(data.readStrongBinder());
          android.hardware.soundtrigger.SoundTrigger.RecognitionConfig _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.hardware.soundtrigger.SoundTrigger.RecognitionConfig.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _result = this.startRecognition(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_stopRecognition:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelUuid _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelUuid.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.hardware.soundtrigger.IRecognitionStatusCallback _arg1;
          _arg1 = android.hardware.soundtrigger.IRecognitionStatusCallback.Stub.asInterface(data.readStrongBinder());
          int _result = this.stopRecognition(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_loadGenericSoundModel:
        {
          data.enforceInterface(descriptor);
          android.hardware.soundtrigger.SoundTrigger.GenericSoundModel _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.soundtrigger.SoundTrigger.GenericSoundModel.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _result = this.loadGenericSoundModel(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_loadKeyphraseSoundModel:
        {
          data.enforceInterface(descriptor);
          android.hardware.soundtrigger.SoundTrigger.KeyphraseSoundModel _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.soundtrigger.SoundTrigger.KeyphraseSoundModel.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _result = this.loadKeyphraseSoundModel(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_startRecognitionForService:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelUuid _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelUuid.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.content.ComponentName _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.hardware.soundtrigger.SoundTrigger.RecognitionConfig _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.hardware.soundtrigger.SoundTrigger.RecognitionConfig.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          int _result = this.startRecognitionForService(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_stopRecognitionForService:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelUuid _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelUuid.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _result = this.stopRecognitionForService(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_unloadSoundModel:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelUuid _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelUuid.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _result = this.unloadSoundModel(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_isRecognitionActive:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelUuid _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelUuid.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.isRecognitionActive(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getModelState:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelUuid _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelUuid.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _result = this.getModelState(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.app.ISoundTriggerService
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
      @Override public android.hardware.soundtrigger.SoundTrigger.GenericSoundModel getSoundModel(android.os.ParcelUuid soundModelId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.hardware.soundtrigger.SoundTrigger.GenericSoundModel _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((soundModelId!=null)) {
            _data.writeInt(1);
            soundModelId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSoundModel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSoundModel(soundModelId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.hardware.soundtrigger.SoundTrigger.GenericSoundModel.CREATOR.createFromParcel(_reply);
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
      @Override public void updateSoundModel(android.hardware.soundtrigger.SoundTrigger.GenericSoundModel soundModel) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((soundModel!=null)) {
            _data.writeInt(1);
            soundModel.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateSoundModel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateSoundModel(soundModel);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void deleteSoundModel(android.os.ParcelUuid soundModelId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((soundModelId!=null)) {
            _data.writeInt(1);
            soundModelId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_deleteSoundModel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deleteSoundModel(soundModelId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int startRecognition(android.os.ParcelUuid soundModelId, android.hardware.soundtrigger.IRecognitionStatusCallback callback, android.hardware.soundtrigger.SoundTrigger.RecognitionConfig config) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((soundModelId!=null)) {
            _data.writeInt(1);
            soundModelId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          if ((config!=null)) {
            _data.writeInt(1);
            config.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startRecognition, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startRecognition(soundModelId, callback, config);
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
      @Override public int stopRecognition(android.os.ParcelUuid soundModelId, android.hardware.soundtrigger.IRecognitionStatusCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((soundModelId!=null)) {
            _data.writeInt(1);
            soundModelId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopRecognition, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().stopRecognition(soundModelId, callback);
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
      @Override public int loadGenericSoundModel(android.hardware.soundtrigger.SoundTrigger.GenericSoundModel soundModel) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((soundModel!=null)) {
            _data.writeInt(1);
            soundModel.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_loadGenericSoundModel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().loadGenericSoundModel(soundModel);
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
      @Override public int loadKeyphraseSoundModel(android.hardware.soundtrigger.SoundTrigger.KeyphraseSoundModel soundModel) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((soundModel!=null)) {
            _data.writeInt(1);
            soundModel.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_loadKeyphraseSoundModel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().loadKeyphraseSoundModel(soundModel);
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
      @Override public int startRecognitionForService(android.os.ParcelUuid soundModelId, android.os.Bundle params, android.content.ComponentName callbackIntent, android.hardware.soundtrigger.SoundTrigger.RecognitionConfig config) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((soundModelId!=null)) {
            _data.writeInt(1);
            soundModelId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((params!=null)) {
            _data.writeInt(1);
            params.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((callbackIntent!=null)) {
            _data.writeInt(1);
            callbackIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((config!=null)) {
            _data.writeInt(1);
            config.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startRecognitionForService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startRecognitionForService(soundModelId, params, callbackIntent, config);
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
      @Override public int stopRecognitionForService(android.os.ParcelUuid soundModelId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((soundModelId!=null)) {
            _data.writeInt(1);
            soundModelId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopRecognitionForService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().stopRecognitionForService(soundModelId);
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
      @Override public int unloadSoundModel(android.os.ParcelUuid soundModelId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((soundModelId!=null)) {
            _data.writeInt(1);
            soundModelId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_unloadSoundModel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().unloadSoundModel(soundModelId);
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
      /** For both ...Intent and ...Service based usage */
      @Override public boolean isRecognitionActive(android.os.ParcelUuid parcelUuid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((parcelUuid!=null)) {
            _data.writeInt(1);
            parcelUuid.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_isRecognitionActive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isRecognitionActive(parcelUuid);
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
      @Override public int getModelState(android.os.ParcelUuid soundModelId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((soundModelId!=null)) {
            _data.writeInt(1);
            soundModelId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getModelState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getModelState(soundModelId);
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
      public static com.android.internal.app.ISoundTriggerService sDefaultImpl;
    }
    static final int TRANSACTION_getSoundModel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_updateSoundModel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_deleteSoundModel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_startRecognition = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_stopRecognition = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_loadGenericSoundModel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_loadKeyphraseSoundModel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_startRecognitionForService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_stopRecognitionForService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_unloadSoundModel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_isRecognitionActive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_getModelState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    public static boolean setDefaultImpl(com.android.internal.app.ISoundTriggerService impl) {
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
    public static com.android.internal.app.ISoundTriggerService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public android.hardware.soundtrigger.SoundTrigger.GenericSoundModel getSoundModel(android.os.ParcelUuid soundModelId) throws android.os.RemoteException;
  public void updateSoundModel(android.hardware.soundtrigger.SoundTrigger.GenericSoundModel soundModel) throws android.os.RemoteException;
  public void deleteSoundModel(android.os.ParcelUuid soundModelId) throws android.os.RemoteException;
  public int startRecognition(android.os.ParcelUuid soundModelId, android.hardware.soundtrigger.IRecognitionStatusCallback callback, android.hardware.soundtrigger.SoundTrigger.RecognitionConfig config) throws android.os.RemoteException;
  public int stopRecognition(android.os.ParcelUuid soundModelId, android.hardware.soundtrigger.IRecognitionStatusCallback callback) throws android.os.RemoteException;
  public int loadGenericSoundModel(android.hardware.soundtrigger.SoundTrigger.GenericSoundModel soundModel) throws android.os.RemoteException;
  public int loadKeyphraseSoundModel(android.hardware.soundtrigger.SoundTrigger.KeyphraseSoundModel soundModel) throws android.os.RemoteException;
  public int startRecognitionForService(android.os.ParcelUuid soundModelId, android.os.Bundle params, android.content.ComponentName callbackIntent, android.hardware.soundtrigger.SoundTrigger.RecognitionConfig config) throws android.os.RemoteException;
  public int stopRecognitionForService(android.os.ParcelUuid soundModelId) throws android.os.RemoteException;
  public int unloadSoundModel(android.os.ParcelUuid soundModelId) throws android.os.RemoteException;
  /** For both ...Intent and ...Service based usage */
  public boolean isRecognitionActive(android.os.ParcelUuid parcelUuid) throws android.os.RemoteException;
  public int getModelState(android.os.ParcelUuid soundModelId) throws android.os.RemoteException;
}
