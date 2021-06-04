/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.oemlock;
/**
 * Interface for communication with the OemLockService.
 *
 * @hide
 */
public interface IOemLockService extends android.os.IInterface
{
  /** Default implementation for IOemLockService. */
  public static class Default implements android.service.oemlock.IOemLockService
  {
    @Override public java.lang.String getLockName() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setOemUnlockAllowedByCarrier(boolean allowed, byte[] signature) throws android.os.RemoteException
    {
    }
    @Override public boolean isOemUnlockAllowedByCarrier() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setOemUnlockAllowedByUser(boolean allowed) throws android.os.RemoteException
    {
    }
    @Override public boolean isOemUnlockAllowedByUser() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isOemUnlockAllowed() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isDeviceOemUnlocked() throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.oemlock.IOemLockService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.oemlock.IOemLockService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.oemlock.IOemLockService interface,
     * generating a proxy if needed.
     */
    public static android.service.oemlock.IOemLockService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.oemlock.IOemLockService))) {
        return ((android.service.oemlock.IOemLockService)iin);
      }
      return new android.service.oemlock.IOemLockService.Stub.Proxy(obj);
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
        case TRANSACTION_getLockName:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getLockName();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_setOemUnlockAllowedByCarrier:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          byte[] _arg1;
          _arg1 = data.createByteArray();
          this.setOemUnlockAllowedByCarrier(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isOemUnlockAllowedByCarrier:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isOemUnlockAllowedByCarrier();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setOemUnlockAllowedByUser:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setOemUnlockAllowedByUser(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isOemUnlockAllowedByUser:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isOemUnlockAllowedByUser();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isOemUnlockAllowed:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isOemUnlockAllowed();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isDeviceOemUnlocked:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isDeviceOemUnlocked();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.oemlock.IOemLockService
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
      @Override public java.lang.String getLockName() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLockName, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLockName();
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setOemUnlockAllowedByCarrier(boolean allowed, byte[] signature) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((allowed)?(1):(0)));
          _data.writeByteArray(signature);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setOemUnlockAllowedByCarrier, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setOemUnlockAllowedByCarrier(allowed, signature);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isOemUnlockAllowedByCarrier() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isOemUnlockAllowedByCarrier, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isOemUnlockAllowedByCarrier();
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
      @Override public void setOemUnlockAllowedByUser(boolean allowed) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((allowed)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setOemUnlockAllowedByUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setOemUnlockAllowedByUser(allowed);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isOemUnlockAllowedByUser() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isOemUnlockAllowedByUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isOemUnlockAllowedByUser();
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
      @Override public boolean isOemUnlockAllowed() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isOemUnlockAllowed, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isOemUnlockAllowed();
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
      @Override public boolean isDeviceOemUnlocked() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isDeviceOemUnlocked, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isDeviceOemUnlocked();
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
      public static android.service.oemlock.IOemLockService sDefaultImpl;
    }
    static final int TRANSACTION_getLockName = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_setOemUnlockAllowedByCarrier = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_isOemUnlockAllowedByCarrier = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_setOemUnlockAllowedByUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_isOemUnlockAllowedByUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_isOemUnlockAllowed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_isDeviceOemUnlocked = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    public static boolean setDefaultImpl(android.service.oemlock.IOemLockService impl) {
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
    public static android.service.oemlock.IOemLockService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public java.lang.String getLockName() throws android.os.RemoteException;
  public void setOemUnlockAllowedByCarrier(boolean allowed, byte[] signature) throws android.os.RemoteException;
  public boolean isOemUnlockAllowedByCarrier() throws android.os.RemoteException;
  public void setOemUnlockAllowedByUser(boolean allowed) throws android.os.RemoteException;
  public boolean isOemUnlockAllowedByUser() throws android.os.RemoteException;
  public boolean isOemUnlockAllowed() throws android.os.RemoteException;
  public boolean isDeviceOemUnlocked() throws android.os.RemoteException;
}
