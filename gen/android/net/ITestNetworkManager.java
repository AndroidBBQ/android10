/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net;
/**
 * Interface that allows for creation and management of test-only networks.
 *
 * @hide
 */
public interface ITestNetworkManager extends android.os.IInterface
{
  /** Default implementation for ITestNetworkManager. */
  public static class Default implements android.net.ITestNetworkManager
  {
    @Override public android.net.TestNetworkInterface createTunInterface(android.net.LinkAddress[] linkAddrs) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.net.TestNetworkInterface createTapInterface() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setupTestNetwork(java.lang.String iface, android.os.IBinder binder) throws android.os.RemoteException
    {
    }
    @Override public void teardownTestNetwork(int netId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.ITestNetworkManager
  {
    private static final java.lang.String DESCRIPTOR = "android.net.ITestNetworkManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.ITestNetworkManager interface,
     * generating a proxy if needed.
     */
    public static android.net.ITestNetworkManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.ITestNetworkManager))) {
        return ((android.net.ITestNetworkManager)iin);
      }
      return new android.net.ITestNetworkManager.Stub.Proxy(obj);
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
        case TRANSACTION_createTunInterface:
        {
          data.enforceInterface(descriptor);
          android.net.LinkAddress[] _arg0;
          _arg0 = data.createTypedArray(android.net.LinkAddress.CREATOR);
          android.net.TestNetworkInterface _result = this.createTunInterface(_arg0);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_createTapInterface:
        {
          data.enforceInterface(descriptor);
          android.net.TestNetworkInterface _result = this.createTapInterface();
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_setupTestNetwork:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          this.setupTestNetwork(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_teardownTestNetwork:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.teardownTestNetwork(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.ITestNetworkManager
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
      @Override public android.net.TestNetworkInterface createTunInterface(android.net.LinkAddress[] linkAddrs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.TestNetworkInterface _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedArray(linkAddrs, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createTunInterface, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createTunInterface(linkAddrs);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.TestNetworkInterface.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.net.TestNetworkInterface createTapInterface() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.TestNetworkInterface _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createTapInterface, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createTapInterface();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.TestNetworkInterface.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setupTestNetwork(java.lang.String iface, android.os.IBinder binder) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iface);
          _data.writeStrongBinder(binder);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setupTestNetwork, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setupTestNetwork(iface, binder);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void teardownTestNetwork(int netId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(netId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_teardownTestNetwork, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().teardownTestNetwork(netId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.net.ITestNetworkManager sDefaultImpl;
    }
    static final int TRANSACTION_createTunInterface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_createTapInterface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_setupTestNetwork = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_teardownTestNetwork = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.net.ITestNetworkManager impl) {
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
    public static android.net.ITestNetworkManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public android.net.TestNetworkInterface createTunInterface(android.net.LinkAddress[] linkAddrs) throws android.os.RemoteException;
  public android.net.TestNetworkInterface createTapInterface() throws android.os.RemoteException;
  public void setupTestNetwork(java.lang.String iface, android.os.IBinder binder) throws android.os.RemoteException;
  public void teardownTestNetwork(int netId) throws android.os.RemoteException;
}
