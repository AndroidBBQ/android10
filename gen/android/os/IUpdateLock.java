/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os;
/**
 * Direct interface to the UpdateLockService's functionality
 *
 * {@hide}
 */
public interface IUpdateLock extends android.os.IInterface
{
  /** Default implementation for IUpdateLock. */
  public static class Default implements android.os.IUpdateLock
  {
    @Override public void acquireUpdateLock(android.os.IBinder token, java.lang.String tag) throws android.os.RemoteException
    {
    }
    @Override public void releaseUpdateLock(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.IUpdateLock
  {
    private static final java.lang.String DESCRIPTOR = "android.os.IUpdateLock";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.IUpdateLock interface,
     * generating a proxy if needed.
     */
    public static android.os.IUpdateLock asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.IUpdateLock))) {
        return ((android.os.IUpdateLock)iin);
      }
      return new android.os.IUpdateLock.Stub.Proxy(obj);
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
        case TRANSACTION_acquireUpdateLock:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.acquireUpdateLock(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_releaseUpdateLock:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.releaseUpdateLock(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.os.IUpdateLock
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
      @Override public void acquireUpdateLock(android.os.IBinder token, java.lang.String tag) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeString(tag);
          boolean _status = mRemote.transact(Stub.TRANSACTION_acquireUpdateLock, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().acquireUpdateLock(token, tag);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void releaseUpdateLock(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_releaseUpdateLock, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().releaseUpdateLock(token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.os.IUpdateLock sDefaultImpl;
    }
    static final int TRANSACTION_acquireUpdateLock = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_releaseUpdateLock = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.os.IUpdateLock impl) {
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
    public static android.os.IUpdateLock getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void acquireUpdateLock(android.os.IBinder token, java.lang.String tag) throws android.os.RemoteException;
  public void releaseUpdateLock(android.os.IBinder token) throws android.os.RemoteException;
}
