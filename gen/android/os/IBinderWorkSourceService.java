/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os;
public interface IBinderWorkSourceService extends android.os.IInterface
{
  /** Default implementation for IBinderWorkSourceService. */
  public static class Default implements android.os.IBinderWorkSourceService
  {
    @Override public int getIncomingWorkSourceUid() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int getBinderCallingUid() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int getThreadLocalWorkSourceUid() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setWorkSourceProvider(int uid) throws android.os.RemoteException
    {
    }
    @Override public void clearWorkSourceProvider() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.IBinderWorkSourceService
  {
    private static final java.lang.String DESCRIPTOR = "android.os.IBinderWorkSourceService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.IBinderWorkSourceService interface,
     * generating a proxy if needed.
     */
    public static android.os.IBinderWorkSourceService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.IBinderWorkSourceService))) {
        return ((android.os.IBinderWorkSourceService)iin);
      }
      return new android.os.IBinderWorkSourceService.Stub.Proxy(obj);
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
        case TRANSACTION_getIncomingWorkSourceUid:
        {
          data.enforceInterface(descriptor);
          int _result = this.getIncomingWorkSourceUid();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getBinderCallingUid:
        {
          data.enforceInterface(descriptor);
          int _result = this.getBinderCallingUid();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getThreadLocalWorkSourceUid:
        {
          data.enforceInterface(descriptor);
          int _result = this.getThreadLocalWorkSourceUid();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setWorkSourceProvider:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setWorkSourceProvider(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_clearWorkSourceProvider:
        {
          data.enforceInterface(descriptor);
          this.clearWorkSourceProvider();
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.os.IBinderWorkSourceService
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
      @Override public int getIncomingWorkSourceUid() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getIncomingWorkSourceUid, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getIncomingWorkSourceUid();
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
      @Override public int getBinderCallingUid() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getBinderCallingUid, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getBinderCallingUid();
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
      @Override public int getThreadLocalWorkSourceUid() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getThreadLocalWorkSourceUid, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getThreadLocalWorkSourceUid();
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
      @Override public void setWorkSourceProvider(int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setWorkSourceProvider, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setWorkSourceProvider(uid);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void clearWorkSourceProvider() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearWorkSourceProvider, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearWorkSourceProvider();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.os.IBinderWorkSourceService sDefaultImpl;
    }
    static final int TRANSACTION_getIncomingWorkSourceUid = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getBinderCallingUid = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getThreadLocalWorkSourceUid = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_setWorkSourceProvider = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_clearWorkSourceProvider = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(android.os.IBinderWorkSourceService impl) {
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
    public static android.os.IBinderWorkSourceService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public int getIncomingWorkSourceUid() throws android.os.RemoteException;
  public int getBinderCallingUid() throws android.os.RemoteException;
  public int getThreadLocalWorkSourceUid() throws android.os.RemoteException;
  public void setWorkSourceProvider(int uid) throws android.os.RemoteException;
  public void clearWorkSourceProvider() throws android.os.RemoteException;
}
