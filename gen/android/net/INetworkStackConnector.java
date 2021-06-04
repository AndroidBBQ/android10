/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net;
/** @hide */
public interface INetworkStackConnector extends android.os.IInterface
{
  /** Default implementation for INetworkStackConnector. */
  public static class Default implements android.net.INetworkStackConnector
  {
    @Override public void makeDhcpServer(java.lang.String ifName, android.net.dhcp.DhcpServingParamsParcel params, android.net.dhcp.IDhcpServerCallbacks cb) throws android.os.RemoteException
    {
    }
    @Override public void makeNetworkMonitor(android.net.Network network, java.lang.String name, android.net.INetworkMonitorCallbacks cb) throws android.os.RemoteException
    {
    }
    @Override public void makeIpClient(java.lang.String ifName, android.net.ip.IIpClientCallbacks callbacks) throws android.os.RemoteException
    {
    }
    @Override public void fetchIpMemoryStore(android.net.IIpMemoryStoreCallbacks cb) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.INetworkStackConnector
  {
    private static final java.lang.String DESCRIPTOR = "android.net.INetworkStackConnector";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.INetworkStackConnector interface,
     * generating a proxy if needed.
     */
    public static android.net.INetworkStackConnector asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.INetworkStackConnector))) {
        return ((android.net.INetworkStackConnector)iin);
      }
      return new android.net.INetworkStackConnector.Stub.Proxy(obj);
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
        case TRANSACTION_makeDhcpServer:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.net.dhcp.DhcpServingParamsParcel _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.dhcp.DhcpServingParamsParcel.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.net.dhcp.IDhcpServerCallbacks _arg2;
          _arg2 = android.net.dhcp.IDhcpServerCallbacks.Stub.asInterface(data.readStrongBinder());
          this.makeDhcpServer(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_makeNetworkMonitor:
        {
          data.enforceInterface(descriptor);
          android.net.Network _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Network.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.net.INetworkMonitorCallbacks _arg2;
          _arg2 = android.net.INetworkMonitorCallbacks.Stub.asInterface(data.readStrongBinder());
          this.makeNetworkMonitor(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_makeIpClient:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.net.ip.IIpClientCallbacks _arg1;
          _arg1 = android.net.ip.IIpClientCallbacks.Stub.asInterface(data.readStrongBinder());
          this.makeIpClient(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_fetchIpMemoryStore:
        {
          data.enforceInterface(descriptor);
          android.net.IIpMemoryStoreCallbacks _arg0;
          _arg0 = android.net.IIpMemoryStoreCallbacks.Stub.asInterface(data.readStrongBinder());
          this.fetchIpMemoryStore(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.INetworkStackConnector
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
      @Override public void makeDhcpServer(java.lang.String ifName, android.net.dhcp.DhcpServingParamsParcel params, android.net.dhcp.IDhcpServerCallbacks cb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(ifName);
          if ((params!=null)) {
            _data.writeInt(1);
            params.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((cb!=null))?(cb.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_makeDhcpServer, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().makeDhcpServer(ifName, params, cb);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void makeNetworkMonitor(android.net.Network network, java.lang.String name, android.net.INetworkMonitorCallbacks cb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((network!=null)) {
            _data.writeInt(1);
            network.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(name);
          _data.writeStrongBinder((((cb!=null))?(cb.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_makeNetworkMonitor, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().makeNetworkMonitor(network, name, cb);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void makeIpClient(java.lang.String ifName, android.net.ip.IIpClientCallbacks callbacks) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(ifName);
          _data.writeStrongBinder((((callbacks!=null))?(callbacks.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_makeIpClient, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().makeIpClient(ifName, callbacks);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void fetchIpMemoryStore(android.net.IIpMemoryStoreCallbacks cb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((cb!=null))?(cb.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_fetchIpMemoryStore, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().fetchIpMemoryStore(cb);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.INetworkStackConnector sDefaultImpl;
    }
    static final int TRANSACTION_makeDhcpServer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_makeNetworkMonitor = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_makeIpClient = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_fetchIpMemoryStore = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.net.INetworkStackConnector impl) {
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
    public static android.net.INetworkStackConnector getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void makeDhcpServer(java.lang.String ifName, android.net.dhcp.DhcpServingParamsParcel params, android.net.dhcp.IDhcpServerCallbacks cb) throws android.os.RemoteException;
  public void makeNetworkMonitor(android.net.Network network, java.lang.String name, android.net.INetworkMonitorCallbacks cb) throws android.os.RemoteException;
  public void makeIpClient(java.lang.String ifName, android.net.ip.IIpClientCallbacks callbacks) throws android.os.RemoteException;
  public void fetchIpMemoryStore(android.net.IIpMemoryStoreCallbacks cb) throws android.os.RemoteException;
}
