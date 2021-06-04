/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os.cts;
public interface ISeccompIsolatedService extends android.os.IInterface
{
  /** Default implementation for ISeccompIsolatedService. */
  public static class Default implements android.os.cts.ISeccompIsolatedService
  {
    @Override public boolean installFilter() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean createThread() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean getSystemInfo() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean writeToFile(android.os.ParcelFileDescriptor fd) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean openAshmem() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean openDevFile() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void violatePolicy() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.cts.ISeccompIsolatedService
  {
    private static final java.lang.String DESCRIPTOR = "android.os.cts.ISeccompIsolatedService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.cts.ISeccompIsolatedService interface,
     * generating a proxy if needed.
     */
    public static android.os.cts.ISeccompIsolatedService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.cts.ISeccompIsolatedService))) {
        return ((android.os.cts.ISeccompIsolatedService)iin);
      }
      return new android.os.cts.ISeccompIsolatedService.Stub.Proxy(obj);
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
        case TRANSACTION_installFilter:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.installFilter();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_createThread:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.createThread();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getSystemInfo:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.getSystemInfo();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_writeToFile:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.writeToFile(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_openAshmem:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.openAshmem();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_openDevFile:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.openDevFile();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_violatePolicy:
        {
          data.enforceInterface(descriptor);
          this.violatePolicy();
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.os.cts.ISeccompIsolatedService
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
      @Override public boolean installFilter() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_installFilter, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().installFilter();
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
      @Override public boolean createThread() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createThread, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createThread();
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
      @Override public boolean getSystemInfo() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSystemInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSystemInfo();
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
      @Override public boolean writeToFile(android.os.ParcelFileDescriptor fd) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((fd!=null)) {
            _data.writeInt(1);
            fd.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_writeToFile, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().writeToFile(fd);
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
      @Override public boolean openAshmem() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_openAshmem, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().openAshmem();
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
      @Override public boolean openDevFile() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_openDevFile, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().openDevFile();
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
      @Override public void violatePolicy() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_violatePolicy, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().violatePolicy();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.os.cts.ISeccompIsolatedService sDefaultImpl;
    }
    static final int TRANSACTION_installFilter = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_createThread = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getSystemInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_writeToFile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_openAshmem = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_openDevFile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_violatePolicy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    public static boolean setDefaultImpl(android.os.cts.ISeccompIsolatedService impl) {
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
    public static android.os.cts.ISeccompIsolatedService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public boolean installFilter() throws android.os.RemoteException;
  public boolean createThread() throws android.os.RemoteException;
  public boolean getSystemInfo() throws android.os.RemoteException;
  public boolean writeToFile(android.os.ParcelFileDescriptor fd) throws android.os.RemoteException;
  public boolean openAshmem() throws android.os.RemoteException;
  public boolean openDevFile() throws android.os.RemoteException;
  public void violatePolicy() throws android.os.RemoteException;
}
