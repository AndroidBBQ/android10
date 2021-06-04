/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.persistentdata;
/**
 * Internal interface through which to communicate to the
 * PersistentDataBlockService. The persistent data block allows writing
 * raw data and setting the OEM unlock enabled/disabled bit contained
 * in the partition.
 *
 * @hide
 */
public interface IPersistentDataBlockService extends android.os.IInterface
{
  /** Default implementation for IPersistentDataBlockService. */
  public static class Default implements android.service.persistentdata.IPersistentDataBlockService
  {
    @Override public int write(byte[] data) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public byte[] read() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void wipe() throws android.os.RemoteException
    {
    }
    @Override public int getDataBlockSize() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public long getMaximumDataBlockSize() throws android.os.RemoteException
    {
      return 0L;
    }
    @Override public void setOemUnlockEnabled(boolean enabled) throws android.os.RemoteException
    {
    }
    @Override public boolean getOemUnlockEnabled() throws android.os.RemoteException
    {
      return false;
    }
    @Override public int getFlashLockState() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean hasFrpCredentialHandle() throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.persistentdata.IPersistentDataBlockService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.persistentdata.IPersistentDataBlockService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.persistentdata.IPersistentDataBlockService interface,
     * generating a proxy if needed.
     */
    public static android.service.persistentdata.IPersistentDataBlockService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.persistentdata.IPersistentDataBlockService))) {
        return ((android.service.persistentdata.IPersistentDataBlockService)iin);
      }
      return new android.service.persistentdata.IPersistentDataBlockService.Stub.Proxy(obj);
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
        case TRANSACTION_write:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          int _result = this.write(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_read:
        {
          data.enforceInterface(descriptor);
          byte[] _result = this.read();
          reply.writeNoException();
          reply.writeByteArray(_result);
          return true;
        }
        case TRANSACTION_wipe:
        {
          data.enforceInterface(descriptor);
          this.wipe();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getDataBlockSize:
        {
          data.enforceInterface(descriptor);
          int _result = this.getDataBlockSize();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getMaximumDataBlockSize:
        {
          data.enforceInterface(descriptor);
          long _result = this.getMaximumDataBlockSize();
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_setOemUnlockEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setOemUnlockEnabled(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getOemUnlockEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.getOemUnlockEnabled();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getFlashLockState:
        {
          data.enforceInterface(descriptor);
          int _result = this.getFlashLockState();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_hasFrpCredentialHandle:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.hasFrpCredentialHandle();
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
    private static class Proxy implements android.service.persistentdata.IPersistentDataBlockService
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
      @Override public int write(byte[] data) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(data);
          boolean _status = mRemote.transact(Stub.TRANSACTION_write, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().write(data);
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
      @Override public byte[] read() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        byte[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_read, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().read();
          }
          _reply.readException();
          _result = _reply.createByteArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void wipe() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_wipe, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().wipe();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getDataBlockSize() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDataBlockSize, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDataBlockSize();
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
      @Override public long getMaximumDataBlockSize() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMaximumDataBlockSize, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMaximumDataBlockSize();
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setOemUnlockEnabled(boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setOemUnlockEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setOemUnlockEnabled(enabled);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean getOemUnlockEnabled() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getOemUnlockEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getOemUnlockEnabled();
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
      @Override public int getFlashLockState() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFlashLockState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFlashLockState();
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
      @Override public boolean hasFrpCredentialHandle() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasFrpCredentialHandle, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasFrpCredentialHandle();
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
      public static android.service.persistentdata.IPersistentDataBlockService sDefaultImpl;
    }
    static final int TRANSACTION_write = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_read = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_wipe = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getDataBlockSize = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getMaximumDataBlockSize = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_setOemUnlockEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_getOemUnlockEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_getFlashLockState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_hasFrpCredentialHandle = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    public static boolean setDefaultImpl(android.service.persistentdata.IPersistentDataBlockService impl) {
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
    public static android.service.persistentdata.IPersistentDataBlockService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public int write(byte[] data) throws android.os.RemoteException;
  public byte[] read() throws android.os.RemoteException;
  public void wipe() throws android.os.RemoteException;
  public int getDataBlockSize() throws android.os.RemoteException;
  public long getMaximumDataBlockSize() throws android.os.RemoteException;
  public void setOemUnlockEnabled(boolean enabled) throws android.os.RemoteException;
  public boolean getOemUnlockEnabled() throws android.os.RemoteException;
  public int getFlashLockState() throws android.os.RemoteException;
  public boolean hasFrpCredentialHandle() throws android.os.RemoteException;
}
