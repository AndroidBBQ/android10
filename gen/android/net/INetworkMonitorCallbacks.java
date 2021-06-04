/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net;
/** @hide */
public interface INetworkMonitorCallbacks extends android.os.IInterface
{
  /** Default implementation for INetworkMonitorCallbacks. */
  public static class Default implements android.net.INetworkMonitorCallbacks
  {
    @Override public void onNetworkMonitorCreated(android.net.INetworkMonitor networkMonitor) throws android.os.RemoteException
    {
    }
    @Override public void notifyNetworkTested(int testResult, java.lang.String redirectUrl) throws android.os.RemoteException
    {
    }
    @Override public void notifyPrivateDnsConfigResolved(android.net.PrivateDnsConfigParcel config) throws android.os.RemoteException
    {
    }
    @Override public void showProvisioningNotification(java.lang.String action, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    @Override public void hideProvisioningNotification() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.INetworkMonitorCallbacks
  {
    private static final java.lang.String DESCRIPTOR = "android.net.INetworkMonitorCallbacks";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.INetworkMonitorCallbacks interface,
     * generating a proxy if needed.
     */
    public static android.net.INetworkMonitorCallbacks asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.INetworkMonitorCallbacks))) {
        return ((android.net.INetworkMonitorCallbacks)iin);
      }
      return new android.net.INetworkMonitorCallbacks.Stub.Proxy(obj);
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
        case TRANSACTION_onNetworkMonitorCreated:
        {
          data.enforceInterface(descriptor);
          android.net.INetworkMonitor _arg0;
          _arg0 = android.net.INetworkMonitor.Stub.asInterface(data.readStrongBinder());
          this.onNetworkMonitorCreated(_arg0);
          return true;
        }
        case TRANSACTION_notifyNetworkTested:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.notifyNetworkTested(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_notifyPrivateDnsConfigResolved:
        {
          data.enforceInterface(descriptor);
          android.net.PrivateDnsConfigParcel _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.PrivateDnsConfigParcel.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.notifyPrivateDnsConfigResolved(_arg0);
          return true;
        }
        case TRANSACTION_showProvisioningNotification:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.showProvisioningNotification(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_hideProvisioningNotification:
        {
          data.enforceInterface(descriptor);
          this.hideProvisioningNotification();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.INetworkMonitorCallbacks
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
      @Override public void onNetworkMonitorCreated(android.net.INetworkMonitor networkMonitor) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((networkMonitor!=null))?(networkMonitor.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNetworkMonitorCreated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNetworkMonitorCreated(networkMonitor);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void notifyNetworkTested(int testResult, java.lang.String redirectUrl) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(testResult);
          _data.writeString(redirectUrl);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyNetworkTested, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyNetworkTested(testResult, redirectUrl);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void notifyPrivateDnsConfigResolved(android.net.PrivateDnsConfigParcel config) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((config!=null)) {
            _data.writeInt(1);
            config.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyPrivateDnsConfigResolved, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyPrivateDnsConfigResolved(config);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void showProvisioningNotification(java.lang.String action, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(action);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_showProvisioningNotification, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showProvisioningNotification(action, packageName);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void hideProvisioningNotification() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hideProvisioningNotification, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().hideProvisioningNotification();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.INetworkMonitorCallbacks sDefaultImpl;
    }
    static final int TRANSACTION_onNetworkMonitorCreated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_notifyNetworkTested = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_notifyPrivateDnsConfigResolved = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_showProvisioningNotification = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_hideProvisioningNotification = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(android.net.INetworkMonitorCallbacks impl) {
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
    public static android.net.INetworkMonitorCallbacks getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onNetworkMonitorCreated(android.net.INetworkMonitor networkMonitor) throws android.os.RemoteException;
  public void notifyNetworkTested(int testResult, java.lang.String redirectUrl) throws android.os.RemoteException;
  public void notifyPrivateDnsConfigResolved(android.net.PrivateDnsConfigParcel config) throws android.os.RemoteException;
  public void showProvisioningNotification(java.lang.String action, java.lang.String packageName) throws android.os.RemoteException;
  public void hideProvisioningNotification() throws android.os.RemoteException;
}
