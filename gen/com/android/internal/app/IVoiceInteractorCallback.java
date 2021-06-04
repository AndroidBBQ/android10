/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.app;
/**
 * IPC interface for an application to receive callbacks from the voice system.
 */
public interface IVoiceInteractorCallback extends android.os.IInterface
{
  /** Default implementation for IVoiceInteractorCallback. */
  public static class Default implements com.android.internal.app.IVoiceInteractorCallback
  {
    @Override public void deliverConfirmationResult(com.android.internal.app.IVoiceInteractorRequest request, boolean confirmed, android.os.Bundle result) throws android.os.RemoteException
    {
    }
    @Override public void deliverPickOptionResult(com.android.internal.app.IVoiceInteractorRequest request, boolean finished, android.app.VoiceInteractor.PickOptionRequest.Option[] selections, android.os.Bundle result) throws android.os.RemoteException
    {
    }
    @Override public void deliverCompleteVoiceResult(com.android.internal.app.IVoiceInteractorRequest request, android.os.Bundle result) throws android.os.RemoteException
    {
    }
    @Override public void deliverAbortVoiceResult(com.android.internal.app.IVoiceInteractorRequest request, android.os.Bundle result) throws android.os.RemoteException
    {
    }
    @Override public void deliverCommandResult(com.android.internal.app.IVoiceInteractorRequest request, boolean finished, android.os.Bundle result) throws android.os.RemoteException
    {
    }
    @Override public void deliverCancel(com.android.internal.app.IVoiceInteractorRequest request) throws android.os.RemoteException
    {
    }
    @Override public void destroy() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.app.IVoiceInteractorCallback
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.app.IVoiceInteractorCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.app.IVoiceInteractorCallback interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.app.IVoiceInteractorCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.app.IVoiceInteractorCallback))) {
        return ((com.android.internal.app.IVoiceInteractorCallback)iin);
      }
      return new com.android.internal.app.IVoiceInteractorCallback.Stub.Proxy(obj);
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
        case TRANSACTION_deliverConfirmationResult:
        {
          data.enforceInterface(descriptor);
          com.android.internal.app.IVoiceInteractorRequest _arg0;
          _arg0 = com.android.internal.app.IVoiceInteractorRequest.Stub.asInterface(data.readStrongBinder());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.deliverConfirmationResult(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_deliverPickOptionResult:
        {
          data.enforceInterface(descriptor);
          com.android.internal.app.IVoiceInteractorRequest _arg0;
          _arg0 = com.android.internal.app.IVoiceInteractorRequest.Stub.asInterface(data.readStrongBinder());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          android.app.VoiceInteractor.PickOptionRequest.Option[] _arg2;
          _arg2 = data.createTypedArray(android.app.VoiceInteractor.PickOptionRequest.Option.CREATOR);
          android.os.Bundle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.deliverPickOptionResult(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_deliverCompleteVoiceResult:
        {
          data.enforceInterface(descriptor);
          com.android.internal.app.IVoiceInteractorRequest _arg0;
          _arg0 = com.android.internal.app.IVoiceInteractorRequest.Stub.asInterface(data.readStrongBinder());
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.deliverCompleteVoiceResult(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_deliverAbortVoiceResult:
        {
          data.enforceInterface(descriptor);
          com.android.internal.app.IVoiceInteractorRequest _arg0;
          _arg0 = com.android.internal.app.IVoiceInteractorRequest.Stub.asInterface(data.readStrongBinder());
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.deliverAbortVoiceResult(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_deliverCommandResult:
        {
          data.enforceInterface(descriptor);
          com.android.internal.app.IVoiceInteractorRequest _arg0;
          _arg0 = com.android.internal.app.IVoiceInteractorRequest.Stub.asInterface(data.readStrongBinder());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.deliverCommandResult(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_deliverCancel:
        {
          data.enforceInterface(descriptor);
          com.android.internal.app.IVoiceInteractorRequest _arg0;
          _arg0 = com.android.internal.app.IVoiceInteractorRequest.Stub.asInterface(data.readStrongBinder());
          this.deliverCancel(_arg0);
          return true;
        }
        case TRANSACTION_destroy:
        {
          data.enforceInterface(descriptor);
          this.destroy();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.app.IVoiceInteractorCallback
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
      @Override public void deliverConfirmationResult(com.android.internal.app.IVoiceInteractorRequest request, boolean confirmed, android.os.Bundle result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((request!=null))?(request.asBinder()):(null)));
          _data.writeInt(((confirmed)?(1):(0)));
          if ((result!=null)) {
            _data.writeInt(1);
            result.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_deliverConfirmationResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deliverConfirmationResult(request, confirmed, result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void deliverPickOptionResult(com.android.internal.app.IVoiceInteractorRequest request, boolean finished, android.app.VoiceInteractor.PickOptionRequest.Option[] selections, android.os.Bundle result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((request!=null))?(request.asBinder()):(null)));
          _data.writeInt(((finished)?(1):(0)));
          _data.writeTypedArray(selections, 0);
          if ((result!=null)) {
            _data.writeInt(1);
            result.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_deliverPickOptionResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deliverPickOptionResult(request, finished, selections, result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void deliverCompleteVoiceResult(com.android.internal.app.IVoiceInteractorRequest request, android.os.Bundle result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((request!=null))?(request.asBinder()):(null)));
          if ((result!=null)) {
            _data.writeInt(1);
            result.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_deliverCompleteVoiceResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deliverCompleteVoiceResult(request, result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void deliverAbortVoiceResult(com.android.internal.app.IVoiceInteractorRequest request, android.os.Bundle result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((request!=null))?(request.asBinder()):(null)));
          if ((result!=null)) {
            _data.writeInt(1);
            result.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_deliverAbortVoiceResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deliverAbortVoiceResult(request, result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void deliverCommandResult(com.android.internal.app.IVoiceInteractorRequest request, boolean finished, android.os.Bundle result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((request!=null))?(request.asBinder()):(null)));
          _data.writeInt(((finished)?(1):(0)));
          if ((result!=null)) {
            _data.writeInt(1);
            result.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_deliverCommandResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deliverCommandResult(request, finished, result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void deliverCancel(com.android.internal.app.IVoiceInteractorRequest request) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((request!=null))?(request.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_deliverCancel, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deliverCancel(request);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void destroy() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_destroy, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().destroy();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.app.IVoiceInteractorCallback sDefaultImpl;
    }
    static final int TRANSACTION_deliverConfirmationResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_deliverPickOptionResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_deliverCompleteVoiceResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_deliverAbortVoiceResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_deliverCommandResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_deliverCancel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_destroy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    public static boolean setDefaultImpl(com.android.internal.app.IVoiceInteractorCallback impl) {
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
    public static com.android.internal.app.IVoiceInteractorCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void deliverConfirmationResult(com.android.internal.app.IVoiceInteractorRequest request, boolean confirmed, android.os.Bundle result) throws android.os.RemoteException;
  public void deliverPickOptionResult(com.android.internal.app.IVoiceInteractorRequest request, boolean finished, android.app.VoiceInteractor.PickOptionRequest.Option[] selections, android.os.Bundle result) throws android.os.RemoteException;
  public void deliverCompleteVoiceResult(com.android.internal.app.IVoiceInteractorRequest request, android.os.Bundle result) throws android.os.RemoteException;
  public void deliverAbortVoiceResult(com.android.internal.app.IVoiceInteractorRequest request, android.os.Bundle result) throws android.os.RemoteException;
  public void deliverCommandResult(com.android.internal.app.IVoiceInteractorRequest request, boolean finished, android.os.Bundle result) throws android.os.RemoteException;
  public void deliverCancel(com.android.internal.app.IVoiceInteractorRequest request) throws android.os.RemoteException;
  public void destroy() throws android.os.RemoteException;
}
