/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.dhcp;
/** @hide */
public interface IDhcpServer extends android.os.IInterface
{
  /** Default implementation for IDhcpServer. */
  public static class Default implements android.net.dhcp.IDhcpServer
  {
    @Override public void start(android.net.INetworkStackStatusCallback cb) throws android.os.RemoteException
    {
    }
    @Override public void updateParams(android.net.dhcp.DhcpServingParamsParcel params, android.net.INetworkStackStatusCallback cb) throws android.os.RemoteException
    {
    }
    @Override public void stop(android.net.INetworkStackStatusCallback cb) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.dhcp.IDhcpServer
  {
    private static final java.lang.String DESCRIPTOR = "android.net.dhcp.IDhcpServer";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.dhcp.IDhcpServer interface,
     * generating a proxy if needed.
     */
    public static android.net.dhcp.IDhcpServer asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.dhcp.IDhcpServer))) {
        return ((android.net.dhcp.IDhcpServer)iin);
      }
      return new android.net.dhcp.IDhcpServer.Stub.Proxy(obj);
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
        case TRANSACTION_start:
        {
          data.enforceInterface(descriptor);
          android.net.INetworkStackStatusCallback _arg0;
          _arg0 = android.net.INetworkStackStatusCallback.Stub.asInterface(data.readStrongBinder());
          this.start(_arg0);
          return true;
        }
        case TRANSACTION_updateParams:
        {
          data.enforceInterface(descriptor);
          android.net.dhcp.DhcpServingParamsParcel _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.dhcp.DhcpServingParamsParcel.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.net.INetworkStackStatusCallback _arg1;
          _arg1 = android.net.INetworkStackStatusCallback.Stub.asInterface(data.readStrongBinder());
          this.updateParams(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_stop:
        {
          data.enforceInterface(descriptor);
          android.net.INetworkStackStatusCallback _arg0;
          _arg0 = android.net.INetworkStackStatusCallback.Stub.asInterface(data.readStrongBinder());
          this.stop(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.dhcp.IDhcpServer
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
      @Override public void start(android.net.INetworkStackStatusCallback cb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((cb!=null))?(cb.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_start, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().start(cb);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void updateParams(android.net.dhcp.DhcpServingParamsParcel params, android.net.INetworkStackStatusCallback cb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((params!=null)) {
            _data.writeInt(1);
            params.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((cb!=null))?(cb.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateParams, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateParams(params, cb);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void stop(android.net.INetworkStackStatusCallback cb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((cb!=null))?(cb.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_stop, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stop(cb);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.dhcp.IDhcpServer sDefaultImpl;
    }
    static final int TRANSACTION_start = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_updateParams = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_stop = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.net.dhcp.IDhcpServer impl) {
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
    public static android.net.dhcp.IDhcpServer getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public static final int STATUS_UNKNOWN = 0;
  public static final int STATUS_SUCCESS = 1;
  public static final int STATUS_INVALID_ARGUMENT = 2;
  public static final int STATUS_UNKNOWN_ERROR = 3;
  public void start(android.net.INetworkStackStatusCallback cb) throws android.os.RemoteException;
  public void updateParams(android.net.dhcp.DhcpServingParamsParcel params, android.net.INetworkStackStatusCallback cb) throws android.os.RemoteException;
  public void stop(android.net.INetworkStackStatusCallback cb) throws android.os.RemoteException;
}
