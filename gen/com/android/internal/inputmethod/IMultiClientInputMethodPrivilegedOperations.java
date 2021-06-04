/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.inputmethod;
/**
 * Defines priviledged operations that only the current MSIMS is allowed to call.
 * Actual operations are implemented and handled by MultiClientInputMethodManagerService.
 */
public interface IMultiClientInputMethodPrivilegedOperations extends android.os.IInterface
{
  /** Default implementation for IMultiClientInputMethodPrivilegedOperations. */
  public static class Default implements com.android.internal.inputmethod.IMultiClientInputMethodPrivilegedOperations
  {
    @Override public android.os.IBinder createInputMethodWindowToken(int displayId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void deleteInputMethodWindowToken(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public void acceptClient(int clientId, com.android.internal.view.IInputMethodSession session, com.android.internal.inputmethod.IMultiClientInputMethodSession multiClientSession, android.view.InputChannel writeChannel) throws android.os.RemoteException
    {
    }
    @Override public void reportImeWindowTarget(int clientId, int targetWindowHandle, android.os.IBinder imeWindowToken) throws android.os.RemoteException
    {
    }
    @Override public boolean isUidAllowedOnDisplay(int displayId, int uid) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setActive(int clientId, boolean active) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.inputmethod.IMultiClientInputMethodPrivilegedOperations
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.inputmethod.IMultiClientInputMethodPrivilegedOperations";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.inputmethod.IMultiClientInputMethodPrivilegedOperations interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.inputmethod.IMultiClientInputMethodPrivilegedOperations asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.inputmethod.IMultiClientInputMethodPrivilegedOperations))) {
        return ((com.android.internal.inputmethod.IMultiClientInputMethodPrivilegedOperations)iin);
      }
      return new com.android.internal.inputmethod.IMultiClientInputMethodPrivilegedOperations.Stub.Proxy(obj);
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
        case TRANSACTION_createInputMethodWindowToken:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.IBinder _result = this.createInputMethodWindowToken(_arg0);
          reply.writeNoException();
          reply.writeStrongBinder(_result);
          return true;
        }
        case TRANSACTION_deleteInputMethodWindowToken:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.deleteInputMethodWindowToken(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_acceptClient:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          com.android.internal.view.IInputMethodSession _arg1;
          _arg1 = com.android.internal.view.IInputMethodSession.Stub.asInterface(data.readStrongBinder());
          com.android.internal.inputmethod.IMultiClientInputMethodSession _arg2;
          _arg2 = com.android.internal.inputmethod.IMultiClientInputMethodSession.Stub.asInterface(data.readStrongBinder());
          android.view.InputChannel _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.view.InputChannel.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.acceptClient(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_reportImeWindowTarget:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.os.IBinder _arg2;
          _arg2 = data.readStrongBinder();
          this.reportImeWindowTarget(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isUidAllowedOnDisplay:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.isUidAllowedOnDisplay(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setActive:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setActive(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.inputmethod.IMultiClientInputMethodPrivilegedOperations
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
      @Override public android.os.IBinder createInputMethodWindowToken(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.IBinder _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createInputMethodWindowToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createInputMethodWindowToken(displayId);
          }
          _reply.readException();
          _result = _reply.readStrongBinder();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void deleteInputMethodWindowToken(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_deleteInputMethodWindowToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deleteInputMethodWindowToken(token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void acceptClient(int clientId, com.android.internal.view.IInputMethodSession session, com.android.internal.inputmethod.IMultiClientInputMethodSession multiClientSession, android.view.InputChannel writeChannel) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(clientId);
          _data.writeStrongBinder((((session!=null))?(session.asBinder()):(null)));
          _data.writeStrongBinder((((multiClientSession!=null))?(multiClientSession.asBinder()):(null)));
          if ((writeChannel!=null)) {
            _data.writeInt(1);
            writeChannel.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_acceptClient, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().acceptClient(clientId, session, multiClientSession, writeChannel);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void reportImeWindowTarget(int clientId, int targetWindowHandle, android.os.IBinder imeWindowToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(clientId);
          _data.writeInt(targetWindowHandle);
          _data.writeStrongBinder(imeWindowToken);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportImeWindowTarget, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportImeWindowTarget(clientId, targetWindowHandle, imeWindowToken);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isUidAllowedOnDisplay(int displayId, int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isUidAllowedOnDisplay, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isUidAllowedOnDisplay(displayId, uid);
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
      @Override public void setActive(int clientId, boolean active) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(clientId);
          _data.writeInt(((active)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setActive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setActive(clientId, active);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.internal.inputmethod.IMultiClientInputMethodPrivilegedOperations sDefaultImpl;
    }
    static final int TRANSACTION_createInputMethodWindowToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_deleteInputMethodWindowToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_acceptClient = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_reportImeWindowTarget = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_isUidAllowedOnDisplay = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_setActive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(com.android.internal.inputmethod.IMultiClientInputMethodPrivilegedOperations impl) {
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
    public static com.android.internal.inputmethod.IMultiClientInputMethodPrivilegedOperations getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public android.os.IBinder createInputMethodWindowToken(int displayId) throws android.os.RemoteException;
  public void deleteInputMethodWindowToken(android.os.IBinder token) throws android.os.RemoteException;
  public void acceptClient(int clientId, com.android.internal.view.IInputMethodSession session, com.android.internal.inputmethod.IMultiClientInputMethodSession multiClientSession, android.view.InputChannel writeChannel) throws android.os.RemoteException;
  public void reportImeWindowTarget(int clientId, int targetWindowHandle, android.os.IBinder imeWindowToken) throws android.os.RemoteException;
  public boolean isUidAllowedOnDisplay(int displayId, int uid) throws android.os.RemoteException;
  public void setActive(int clientId, boolean active) throws android.os.RemoteException;
}
