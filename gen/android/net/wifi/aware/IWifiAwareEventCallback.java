/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.wifi.aware;
/**
 * Callback interface that WifiAwareManager implements
 *
 * {@hide}
 */
public interface IWifiAwareEventCallback extends android.os.IInterface
{
  /** Default implementation for IWifiAwareEventCallback. */
  public static class Default implements android.net.wifi.aware.IWifiAwareEventCallback
  {
    @Override public void onConnectSuccess(int clientId) throws android.os.RemoteException
    {
    }
    @Override public void onConnectFail(int reason) throws android.os.RemoteException
    {
    }
    @Override public void onIdentityChanged(byte[] mac) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.wifi.aware.IWifiAwareEventCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.net.wifi.aware.IWifiAwareEventCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.wifi.aware.IWifiAwareEventCallback interface,
     * generating a proxy if needed.
     */
    public static android.net.wifi.aware.IWifiAwareEventCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.wifi.aware.IWifiAwareEventCallback))) {
        return ((android.net.wifi.aware.IWifiAwareEventCallback)iin);
      }
      return new android.net.wifi.aware.IWifiAwareEventCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onConnectSuccess:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onConnectSuccess(_arg0);
          return true;
        }
        case TRANSACTION_onConnectFail:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onConnectFail(_arg0);
          return true;
        }
        case TRANSACTION_onIdentityChanged:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          this.onIdentityChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.wifi.aware.IWifiAwareEventCallback
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
      @Override public void onConnectSuccess(int clientId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(clientId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onConnectSuccess, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onConnectSuccess(clientId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onConnectFail(int reason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(reason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onConnectFail, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onConnectFail(reason);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onIdentityChanged(byte[] mac) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(mac);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onIdentityChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onIdentityChanged(mac);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.wifi.aware.IWifiAwareEventCallback sDefaultImpl;
    }
    static final int TRANSACTION_onConnectSuccess = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onConnectFail = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onIdentityChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.net.wifi.aware.IWifiAwareEventCallback impl) {
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
    public static android.net.wifi.aware.IWifiAwareEventCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onConnectSuccess(int clientId) throws android.os.RemoteException;
  public void onConnectFail(int reason) throws android.os.RemoteException;
  public void onIdentityChanged(byte[] mac) throws android.os.RemoteException;
}
