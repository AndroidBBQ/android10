/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.inputmethod;
public interface IMultiClientInputMethod extends android.os.IInterface
{
  /** Default implementation for IMultiClientInputMethod. */
  public static class Default implements com.android.internal.inputmethod.IMultiClientInputMethod
  {
    @Override public void initialize(com.android.internal.inputmethod.IMultiClientInputMethodPrivilegedOperations privOps) throws android.os.RemoteException
    {
    }
    @Override public void addClient(int clientId, int uid, int pid, int selfReportedDisplayId) throws android.os.RemoteException
    {
    }
    @Override public void removeClient(int clientId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.inputmethod.IMultiClientInputMethod
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.inputmethod.IMultiClientInputMethod";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.inputmethod.IMultiClientInputMethod interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.inputmethod.IMultiClientInputMethod asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.inputmethod.IMultiClientInputMethod))) {
        return ((com.android.internal.inputmethod.IMultiClientInputMethod)iin);
      }
      return new com.android.internal.inputmethod.IMultiClientInputMethod.Stub.Proxy(obj);
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
        case TRANSACTION_initialize:
        {
          data.enforceInterface(descriptor);
          com.android.internal.inputmethod.IMultiClientInputMethodPrivilegedOperations _arg0;
          _arg0 = com.android.internal.inputmethod.IMultiClientInputMethodPrivilegedOperations.Stub.asInterface(data.readStrongBinder());
          this.initialize(_arg0);
          return true;
        }
        case TRANSACTION_addClient:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          this.addClient(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_removeClient:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.removeClient(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.inputmethod.IMultiClientInputMethod
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
      @Override public void initialize(com.android.internal.inputmethod.IMultiClientInputMethodPrivilegedOperations privOps) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((privOps!=null))?(privOps.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_initialize, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().initialize(privOps);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void addClient(int clientId, int uid, int pid, int selfReportedDisplayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(clientId);
          _data.writeInt(uid);
          _data.writeInt(pid);
          _data.writeInt(selfReportedDisplayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addClient, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addClient(clientId, uid, pid, selfReportedDisplayId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void removeClient(int clientId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(clientId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeClient, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeClient(clientId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.inputmethod.IMultiClientInputMethod sDefaultImpl;
    }
    static final int TRANSACTION_initialize = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_addClient = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_removeClient = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(com.android.internal.inputmethod.IMultiClientInputMethod impl) {
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
    public static com.android.internal.inputmethod.IMultiClientInputMethod getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void initialize(com.android.internal.inputmethod.IMultiClientInputMethodPrivilegedOperations privOps) throws android.os.RemoteException;
  public void addClient(int clientId, int uid, int pid, int selfReportedDisplayId) throws android.os.RemoteException;
  public void removeClient(int clientId) throws android.os.RemoteException;
}
