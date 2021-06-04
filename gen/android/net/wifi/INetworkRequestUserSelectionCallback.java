/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.wifi;
/**
 * Interface for providing user selection in response to
 * network request match callback.
 * @hide
 */
public interface INetworkRequestUserSelectionCallback extends android.os.IInterface
{
  /** Default implementation for INetworkRequestUserSelectionCallback. */
  public static class Default implements android.net.wifi.INetworkRequestUserSelectionCallback
  {
    @Override public void select(android.net.wifi.WifiConfiguration wificonfiguration) throws android.os.RemoteException
    {
    }
    @Override public void reject() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.wifi.INetworkRequestUserSelectionCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.net.wifi.INetworkRequestUserSelectionCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.wifi.INetworkRequestUserSelectionCallback interface,
     * generating a proxy if needed.
     */
    public static android.net.wifi.INetworkRequestUserSelectionCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.wifi.INetworkRequestUserSelectionCallback))) {
        return ((android.net.wifi.INetworkRequestUserSelectionCallback)iin);
      }
      return new android.net.wifi.INetworkRequestUserSelectionCallback.Stub.Proxy(obj);
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
        case TRANSACTION_select:
        {
          data.enforceInterface(descriptor);
          android.net.wifi.WifiConfiguration _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.wifi.WifiConfiguration.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.select(_arg0);
          return true;
        }
        case TRANSACTION_reject:
        {
          data.enforceInterface(descriptor);
          this.reject();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.wifi.INetworkRequestUserSelectionCallback
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
      @Override public void select(android.net.wifi.WifiConfiguration wificonfiguration) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_select, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().select(wificonfiguration);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void reject() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reject, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reject();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.wifi.INetworkRequestUserSelectionCallback sDefaultImpl;
    }
    static final int TRANSACTION_select = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_reject = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.net.wifi.INetworkRequestUserSelectionCallback impl) {
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
    public static android.net.wifi.INetworkRequestUserSelectionCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void select(android.net.wifi.WifiConfiguration wificonfiguration) throws android.os.RemoteException;
  public void reject() throws android.os.RemoteException;
}
