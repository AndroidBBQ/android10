/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.wifi.rtt;
/**
 * @hide
 */
public interface IWifiRttManager extends android.os.IInterface
{
  /** Default implementation for IWifiRttManager. */
  public static class Default implements android.net.wifi.rtt.IWifiRttManager
  {
    @Override public boolean isAvailable() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void startRanging(android.os.IBinder binder, java.lang.String callingPackage, android.os.WorkSource workSource, android.net.wifi.rtt.RangingRequest request, android.net.wifi.rtt.IRttCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void cancelRanging(android.os.WorkSource workSource) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.wifi.rtt.IWifiRttManager
  {
    private static final java.lang.String DESCRIPTOR = "android.net.wifi.rtt.IWifiRttManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.wifi.rtt.IWifiRttManager interface,
     * generating a proxy if needed.
     */
    public static android.net.wifi.rtt.IWifiRttManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.wifi.rtt.IWifiRttManager))) {
        return ((android.net.wifi.rtt.IWifiRttManager)iin);
      }
      return new android.net.wifi.rtt.IWifiRttManager.Stub.Proxy(obj);
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
        case TRANSACTION_isAvailable:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isAvailable();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_startRanging:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.WorkSource _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.WorkSource.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.net.wifi.rtt.RangingRequest _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.net.wifi.rtt.RangingRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          android.net.wifi.rtt.IRttCallback _arg4;
          _arg4 = android.net.wifi.rtt.IRttCallback.Stub.asInterface(data.readStrongBinder());
          this.startRanging(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_cancelRanging:
        {
          data.enforceInterface(descriptor);
          android.os.WorkSource _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.WorkSource.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.cancelRanging(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.wifi.rtt.IWifiRttManager
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
      @Override public boolean isAvailable() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isAvailable, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isAvailable();
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
      @Override public void startRanging(android.os.IBinder binder, java.lang.String callingPackage, android.os.WorkSource workSource, android.net.wifi.rtt.RangingRequest request, android.net.wifi.rtt.IRttCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(binder);
          _data.writeString(callingPackage);
          if ((workSource!=null)) {
            _data.writeInt(1);
            workSource.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startRanging, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startRanging(binder, callingPackage, workSource, request, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void cancelRanging(android.os.WorkSource workSource) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((workSource!=null)) {
            _data.writeInt(1);
            workSource.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelRanging, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelRanging(workSource);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.net.wifi.rtt.IWifiRttManager sDefaultImpl;
    }
    static final int TRANSACTION_isAvailable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_startRanging = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_cancelRanging = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.net.wifi.rtt.IWifiRttManager impl) {
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
    public static android.net.wifi.rtt.IWifiRttManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public boolean isAvailable() throws android.os.RemoteException;
  public void startRanging(android.os.IBinder binder, java.lang.String callingPackage, android.os.WorkSource workSource, android.net.wifi.rtt.RangingRequest request, android.net.wifi.rtt.IRttCallback callback) throws android.os.RemoteException;
  public void cancelRanging(android.os.WorkSource workSource) throws android.os.RemoteException;
}
