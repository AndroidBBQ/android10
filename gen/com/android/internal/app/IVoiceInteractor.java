/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.app;
/**
 * IPC interface for an application to perform calls through a VoiceInteractor.
 */
public interface IVoiceInteractor extends android.os.IInterface
{
  /** Default implementation for IVoiceInteractor. */
  public static class Default implements com.android.internal.app.IVoiceInteractor
  {
    @Override public com.android.internal.app.IVoiceInteractorRequest startConfirmation(java.lang.String callingPackage, com.android.internal.app.IVoiceInteractorCallback callback, android.app.VoiceInteractor.Prompt prompt, android.os.Bundle extras) throws android.os.RemoteException
    {
      return null;
    }
    @Override public com.android.internal.app.IVoiceInteractorRequest startPickOption(java.lang.String callingPackage, com.android.internal.app.IVoiceInteractorCallback callback, android.app.VoiceInteractor.Prompt prompt, android.app.VoiceInteractor.PickOptionRequest.Option[] options, android.os.Bundle extras) throws android.os.RemoteException
    {
      return null;
    }
    @Override public com.android.internal.app.IVoiceInteractorRequest startCompleteVoice(java.lang.String callingPackage, com.android.internal.app.IVoiceInteractorCallback callback, android.app.VoiceInteractor.Prompt prompt, android.os.Bundle extras) throws android.os.RemoteException
    {
      return null;
    }
    @Override public com.android.internal.app.IVoiceInteractorRequest startAbortVoice(java.lang.String callingPackage, com.android.internal.app.IVoiceInteractorCallback callback, android.app.VoiceInteractor.Prompt prompt, android.os.Bundle extras) throws android.os.RemoteException
    {
      return null;
    }
    @Override public com.android.internal.app.IVoiceInteractorRequest startCommand(java.lang.String callingPackage, com.android.internal.app.IVoiceInteractorCallback callback, java.lang.String command, android.os.Bundle extras) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean[] supportsCommands(java.lang.String callingPackage, java.lang.String[] commands) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void notifyDirectActionsChanged(int taskId, android.os.IBinder assistToken) throws android.os.RemoteException
    {
    }
    @Override public void setKillCallback(android.os.ICancellationSignal callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.app.IVoiceInteractor
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.app.IVoiceInteractor";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.app.IVoiceInteractor interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.app.IVoiceInteractor asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.app.IVoiceInteractor))) {
        return ((com.android.internal.app.IVoiceInteractor)iin);
      }
      return new com.android.internal.app.IVoiceInteractor.Stub.Proxy(obj);
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
        case TRANSACTION_startConfirmation:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          com.android.internal.app.IVoiceInteractorCallback _arg1;
          _arg1 = com.android.internal.app.IVoiceInteractorCallback.Stub.asInterface(data.readStrongBinder());
          android.app.VoiceInteractor.Prompt _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.app.VoiceInteractor.Prompt.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.os.Bundle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          com.android.internal.app.IVoiceInteractorRequest _result = this.startConfirmation(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_startPickOption:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          com.android.internal.app.IVoiceInteractorCallback _arg1;
          _arg1 = com.android.internal.app.IVoiceInteractorCallback.Stub.asInterface(data.readStrongBinder());
          android.app.VoiceInteractor.Prompt _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.app.VoiceInteractor.Prompt.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.app.VoiceInteractor.PickOptionRequest.Option[] _arg3;
          _arg3 = data.createTypedArray(android.app.VoiceInteractor.PickOptionRequest.Option.CREATOR);
          android.os.Bundle _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          com.android.internal.app.IVoiceInteractorRequest _result = this.startPickOption(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_startCompleteVoice:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          com.android.internal.app.IVoiceInteractorCallback _arg1;
          _arg1 = com.android.internal.app.IVoiceInteractorCallback.Stub.asInterface(data.readStrongBinder());
          android.app.VoiceInteractor.Prompt _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.app.VoiceInteractor.Prompt.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.os.Bundle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          com.android.internal.app.IVoiceInteractorRequest _result = this.startCompleteVoice(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_startAbortVoice:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          com.android.internal.app.IVoiceInteractorCallback _arg1;
          _arg1 = com.android.internal.app.IVoiceInteractorCallback.Stub.asInterface(data.readStrongBinder());
          android.app.VoiceInteractor.Prompt _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.app.VoiceInteractor.Prompt.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.os.Bundle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          com.android.internal.app.IVoiceInteractorRequest _result = this.startAbortVoice(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_startCommand:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          com.android.internal.app.IVoiceInteractorCallback _arg1;
          _arg1 = com.android.internal.app.IVoiceInteractorCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.os.Bundle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          com.android.internal.app.IVoiceInteractorRequest _result = this.startCommand(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_supportsCommands:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String[] _arg1;
          _arg1 = data.createStringArray();
          boolean[] _result = this.supportsCommands(_arg0, _arg1);
          reply.writeNoException();
          reply.writeBooleanArray(_result);
          return true;
        }
        case TRANSACTION_notifyDirectActionsChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          this.notifyDirectActionsChanged(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setKillCallback:
        {
          data.enforceInterface(descriptor);
          android.os.ICancellationSignal _arg0;
          _arg0 = android.os.ICancellationSignal.Stub.asInterface(data.readStrongBinder());
          this.setKillCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.app.IVoiceInteractor
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
      @Override public com.android.internal.app.IVoiceInteractorRequest startConfirmation(java.lang.String callingPackage, com.android.internal.app.IVoiceInteractorCallback callback, android.app.VoiceInteractor.Prompt prompt, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.internal.app.IVoiceInteractorRequest _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          if ((prompt!=null)) {
            _data.writeInt(1);
            prompt.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startConfirmation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startConfirmation(callingPackage, callback, prompt, extras);
          }
          _reply.readException();
          _result = com.android.internal.app.IVoiceInteractorRequest.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public com.android.internal.app.IVoiceInteractorRequest startPickOption(java.lang.String callingPackage, com.android.internal.app.IVoiceInteractorCallback callback, android.app.VoiceInteractor.Prompt prompt, android.app.VoiceInteractor.PickOptionRequest.Option[] options, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.internal.app.IVoiceInteractorRequest _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          if ((prompt!=null)) {
            _data.writeInt(1);
            prompt.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeTypedArray(options, 0);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startPickOption, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startPickOption(callingPackage, callback, prompt, options, extras);
          }
          _reply.readException();
          _result = com.android.internal.app.IVoiceInteractorRequest.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public com.android.internal.app.IVoiceInteractorRequest startCompleteVoice(java.lang.String callingPackage, com.android.internal.app.IVoiceInteractorCallback callback, android.app.VoiceInteractor.Prompt prompt, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.internal.app.IVoiceInteractorRequest _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          if ((prompt!=null)) {
            _data.writeInt(1);
            prompt.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startCompleteVoice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startCompleteVoice(callingPackage, callback, prompt, extras);
          }
          _reply.readException();
          _result = com.android.internal.app.IVoiceInteractorRequest.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public com.android.internal.app.IVoiceInteractorRequest startAbortVoice(java.lang.String callingPackage, com.android.internal.app.IVoiceInteractorCallback callback, android.app.VoiceInteractor.Prompt prompt, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.internal.app.IVoiceInteractorRequest _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          if ((prompt!=null)) {
            _data.writeInt(1);
            prompt.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startAbortVoice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startAbortVoice(callingPackage, callback, prompt, extras);
          }
          _reply.readException();
          _result = com.android.internal.app.IVoiceInteractorRequest.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public com.android.internal.app.IVoiceInteractorRequest startCommand(java.lang.String callingPackage, com.android.internal.app.IVoiceInteractorCallback callback, java.lang.String command, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.internal.app.IVoiceInteractorRequest _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeString(command);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startCommand, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startCommand(callingPackage, callback, command, extras);
          }
          _reply.readException();
          _result = com.android.internal.app.IVoiceInteractorRequest.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public boolean[] supportsCommands(java.lang.String callingPackage, java.lang.String[] commands) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeStringArray(commands);
          boolean _status = mRemote.transact(Stub.TRANSACTION_supportsCommands, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().supportsCommands(callingPackage, commands);
          }
          _reply.readException();
          _result = _reply.createBooleanArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void notifyDirectActionsChanged(int taskId, android.os.IBinder assistToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(taskId);
          _data.writeStrongBinder(assistToken);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyDirectActionsChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyDirectActionsChanged(taskId, assistToken);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setKillCallback(android.os.ICancellationSignal callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setKillCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setKillCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.internal.app.IVoiceInteractor sDefaultImpl;
    }
    static final int TRANSACTION_startConfirmation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_startPickOption = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_startCompleteVoice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_startAbortVoice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_startCommand = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_supportsCommands = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_notifyDirectActionsChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_setKillCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    public static boolean setDefaultImpl(com.android.internal.app.IVoiceInteractor impl) {
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
    public static com.android.internal.app.IVoiceInteractor getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public com.android.internal.app.IVoiceInteractorRequest startConfirmation(java.lang.String callingPackage, com.android.internal.app.IVoiceInteractorCallback callback, android.app.VoiceInteractor.Prompt prompt, android.os.Bundle extras) throws android.os.RemoteException;
  public com.android.internal.app.IVoiceInteractorRequest startPickOption(java.lang.String callingPackage, com.android.internal.app.IVoiceInteractorCallback callback, android.app.VoiceInteractor.Prompt prompt, android.app.VoiceInteractor.PickOptionRequest.Option[] options, android.os.Bundle extras) throws android.os.RemoteException;
  public com.android.internal.app.IVoiceInteractorRequest startCompleteVoice(java.lang.String callingPackage, com.android.internal.app.IVoiceInteractorCallback callback, android.app.VoiceInteractor.Prompt prompt, android.os.Bundle extras) throws android.os.RemoteException;
  public com.android.internal.app.IVoiceInteractorRequest startAbortVoice(java.lang.String callingPackage, com.android.internal.app.IVoiceInteractorCallback callback, android.app.VoiceInteractor.Prompt prompt, android.os.Bundle extras) throws android.os.RemoteException;
  public com.android.internal.app.IVoiceInteractorRequest startCommand(java.lang.String callingPackage, com.android.internal.app.IVoiceInteractorCallback callback, java.lang.String command, android.os.Bundle extras) throws android.os.RemoteException;
  public boolean[] supportsCommands(java.lang.String callingPackage, java.lang.String[] commands) throws android.os.RemoteException;
  public void notifyDirectActionsChanged(int taskId, android.os.IBinder assistToken) throws android.os.RemoteException;
  public void setKillCallback(android.os.ICancellationSignal callback) throws android.os.RemoteException;
}
