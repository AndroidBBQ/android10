/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.watchdog;
/**
 * @hide
 */
public interface IExplicitHealthCheckService extends android.os.IInterface
{
  /** Default implementation for IExplicitHealthCheckService. */
  public static class Default implements android.service.watchdog.IExplicitHealthCheckService
  {
    @Override public void setCallback(android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void request(java.lang.String packageName) throws android.os.RemoteException
    {
    }
    @Override public void cancel(java.lang.String packageName) throws android.os.RemoteException
    {
    }
    @Override public void getSupportedPackages(android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void getRequestedPackages(android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.watchdog.IExplicitHealthCheckService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.watchdog.IExplicitHealthCheckService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.watchdog.IExplicitHealthCheckService interface,
     * generating a proxy if needed.
     */
    public static android.service.watchdog.IExplicitHealthCheckService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.watchdog.IExplicitHealthCheckService))) {
        return ((android.service.watchdog.IExplicitHealthCheckService)iin);
      }
      return new android.service.watchdog.IExplicitHealthCheckService.Stub.Proxy(obj);
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
        case TRANSACTION_setCallback:
        {
          data.enforceInterface(descriptor);
          android.os.RemoteCallback _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setCallback(_arg0);
          return true;
        }
        case TRANSACTION_request:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.request(_arg0);
          return true;
        }
        case TRANSACTION_cancel:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.cancel(_arg0);
          return true;
        }
        case TRANSACTION_getSupportedPackages:
        {
          data.enforceInterface(descriptor);
          android.os.RemoteCallback _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.getSupportedPackages(_arg0);
          return true;
        }
        case TRANSACTION_getRequestedPackages:
        {
          data.enforceInterface(descriptor);
          android.os.RemoteCallback _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.getRequestedPackages(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.watchdog.IExplicitHealthCheckService
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
      @Override public void setCallback(android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setCallback, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setCallback(callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void request(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_request, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().request(packageName);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void cancel(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancel, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancel(packageName);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getSupportedPackages(android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSupportedPackages, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getSupportedPackages(callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getRequestedPackages(android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRequestedPackages, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getRequestedPackages(callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.watchdog.IExplicitHealthCheckService sDefaultImpl;
    }
    static final int TRANSACTION_setCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_request = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_cancel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getSupportedPackages = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getRequestedPackages = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(android.service.watchdog.IExplicitHealthCheckService impl) {
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
    public static android.service.watchdog.IExplicitHealthCheckService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void setCallback(android.os.RemoteCallback callback) throws android.os.RemoteException;
  public void request(java.lang.String packageName) throws android.os.RemoteException;
  public void cancel(java.lang.String packageName) throws android.os.RemoteException;
  public void getSupportedPackages(android.os.RemoteCallback callback) throws android.os.RemoteException;
  public void getRequestedPackages(android.os.RemoteCallback callback) throws android.os.RemoteException;
}
