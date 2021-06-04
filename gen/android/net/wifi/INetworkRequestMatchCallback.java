/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.wifi;
/**
 * Interface for network request match callback.
 *
 * @hide
 */
public interface INetworkRequestMatchCallback extends android.os.IInterface
{
  /** Default implementation for INetworkRequestMatchCallback. */
  public static class Default implements android.net.wifi.INetworkRequestMatchCallback
  {
    @Override public void onUserSelectionCallbackRegistration(android.net.wifi.INetworkRequestUserSelectionCallback userSelectionCallback) throws android.os.RemoteException
    {
    }
    @Override public void onAbort() throws android.os.RemoteException
    {
    }
    @Override public void onMatch(java.util.List<android.net.wifi.ScanResult> scanResults) throws android.os.RemoteException
    {
    }
    @Override public void onUserSelectionConnectSuccess(android.net.wifi.WifiConfiguration wificonfiguration) throws android.os.RemoteException
    {
    }
    @Override public void onUserSelectionConnectFailure(android.net.wifi.WifiConfiguration wificonfiguration) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.wifi.INetworkRequestMatchCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.net.wifi.INetworkRequestMatchCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.wifi.INetworkRequestMatchCallback interface,
     * generating a proxy if needed.
     */
    public static android.net.wifi.INetworkRequestMatchCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.wifi.INetworkRequestMatchCallback))) {
        return ((android.net.wifi.INetworkRequestMatchCallback)iin);
      }
      return new android.net.wifi.INetworkRequestMatchCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onUserSelectionCallbackRegistration:
        {
          data.enforceInterface(descriptor);
          android.net.wifi.INetworkRequestUserSelectionCallback _arg0;
          _arg0 = android.net.wifi.INetworkRequestUserSelectionCallback.Stub.asInterface(data.readStrongBinder());
          this.onUserSelectionCallbackRegistration(_arg0);
          return true;
        }
        case TRANSACTION_onAbort:
        {
          data.enforceInterface(descriptor);
          this.onAbort();
          return true;
        }
        case TRANSACTION_onMatch:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.net.wifi.ScanResult> _arg0;
          _arg0 = data.createTypedArrayList(android.net.wifi.ScanResult.CREATOR);
          this.onMatch(_arg0);
          return true;
        }
        case TRANSACTION_onUserSelectionConnectSuccess:
        {
          data.enforceInterface(descriptor);
          android.net.wifi.WifiConfiguration _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.wifi.WifiConfiguration.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onUserSelectionConnectSuccess(_arg0);
          return true;
        }
        case TRANSACTION_onUserSelectionConnectFailure:
        {
          data.enforceInterface(descriptor);
          android.net.wifi.WifiConfiguration _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.wifi.WifiConfiguration.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onUserSelectionConnectFailure(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.wifi.INetworkRequestMatchCallback
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
      @Override public void onUserSelectionCallbackRegistration(android.net.wifi.INetworkRequestUserSelectionCallback userSelectionCallback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((userSelectionCallback!=null))?(userSelectionCallback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onUserSelectionCallbackRegistration, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onUserSelectionCallbackRegistration(userSelectionCallback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onAbort() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAbort, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAbort();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onMatch(java.util.List<android.net.wifi.ScanResult> scanResults) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(scanResults);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onMatch, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onMatch(scanResults);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onUserSelectionConnectSuccess(android.net.wifi.WifiConfiguration wificonfiguration) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((wificonfiguration!=null)) {
            _data.writeInt(1);
            wificonfiguration.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onUserSelectionConnectSuccess, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onUserSelectionConnectSuccess(wificonfiguration);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onUserSelectionConnectFailure(android.net.wifi.WifiConfiguration wificonfiguration) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((wificonfiguration!=null)) {
            _data.writeInt(1);
            wificonfiguration.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onUserSelectionConnectFailure, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onUserSelectionConnectFailure(wificonfiguration);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.wifi.INetworkRequestMatchCallback sDefaultImpl;
    }
    static final int TRANSACTION_onUserSelectionCallbackRegistration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onAbort = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onMatch = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onUserSelectionConnectSuccess = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onUserSelectionConnectFailure = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(android.net.wifi.INetworkRequestMatchCallback impl) {
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
    public static android.net.wifi.INetworkRequestMatchCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onUserSelectionCallbackRegistration(android.net.wifi.INetworkRequestUserSelectionCallback userSelectionCallback) throws android.os.RemoteException;
  public void onAbort() throws android.os.RemoteException;
  public void onMatch(java.util.List<android.net.wifi.ScanResult> scanResults) throws android.os.RemoteException;
  public void onUserSelectionConnectSuccess(android.net.wifi.WifiConfiguration wificonfiguration) throws android.os.RemoteException;
  public void onUserSelectionConnectFailure(android.net.wifi.WifiConfiguration wificonfiguration) throws android.os.RemoteException;
}
