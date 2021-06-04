/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.dhcp;
/** @hide */
public interface IDhcpServerCallbacks extends android.os.IInterface
{
  /** Default implementation for IDhcpServerCallbacks. */
  public static class Default implements android.net.dhcp.IDhcpServerCallbacks
  {
    @Override public void onDhcpServerCreated(int statusCode, android.net.dhcp.IDhcpServer server) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.dhcp.IDhcpServerCallbacks
  {
    private static final java.lang.String DESCRIPTOR = "android.net.dhcp.IDhcpServerCallbacks";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.dhcp.IDhcpServerCallbacks interface,
     * generating a proxy if needed.
     */
    public static android.net.dhcp.IDhcpServerCallbacks asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.dhcp.IDhcpServerCallbacks))) {
        return ((android.net.dhcp.IDhcpServerCallbacks)iin);
      }
      return new android.net.dhcp.IDhcpServerCallbacks.Stub.Proxy(obj);
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
        case TRANSACTION_onDhcpServerCreated:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.net.dhcp.IDhcpServer _arg1;
          _arg1 = android.net.dhcp.IDhcpServer.Stub.asInterface(data.readStrongBinder());
          this.onDhcpServerCreated(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.dhcp.IDhcpServerCallbacks
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
      @Override public void onDhcpServerCreated(int statusCode, android.net.dhcp.IDhcpServer server) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(statusCode);
          _data.writeStrongBinder((((server!=null))?(server.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDhcpServerCreated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDhcpServerCreated(statusCode, server);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.dhcp.IDhcpServerCallbacks sDefaultImpl;
    }
    static final int TRANSACTION_onDhcpServerCreated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.net.dhcp.IDhcpServerCallbacks impl) {
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
    public static android.net.dhcp.IDhcpServerCallbacks getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onDhcpServerCreated(int statusCode, android.net.dhcp.IDhcpServer server) throws android.os.RemoteException;
}
