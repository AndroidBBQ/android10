/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.wifi.p2p;
/**
 * Interface that WifiP2pService implements
 *
 * {@hide}
 */
public interface IWifiP2pManager extends android.os.IInterface
{
  /** Default implementation for IWifiP2pManager. */
  public static class Default implements android.net.wifi.p2p.IWifiP2pManager
  {
    @Override public android.os.Messenger getMessenger(android.os.IBinder binder) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.os.Messenger getP2pStateMachineMessenger() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void close(android.os.IBinder binder) throws android.os.RemoteException
    {
    }
    @Override public void setMiracastMode(int mode) throws android.os.RemoteException
    {
    }
    @Override public void checkConfigureWifiDisplayPermission() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.wifi.p2p.IWifiP2pManager
  {
    private static final java.lang.String DESCRIPTOR = "android.net.wifi.p2p.IWifiP2pManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.wifi.p2p.IWifiP2pManager interface,
     * generating a proxy if needed.
     */
    public static android.net.wifi.p2p.IWifiP2pManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.wifi.p2p.IWifiP2pManager))) {
        return ((android.net.wifi.p2p.IWifiP2pManager)iin);
      }
      return new android.net.wifi.p2p.IWifiP2pManager.Stub.Proxy(obj);
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
        case TRANSACTION_getMessenger:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.os.Messenger _result = this.getMessenger(_arg0);
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
        case TRANSACTION_getP2pStateMachineMessenger:
        {
          data.enforceInterface(descriptor);
          android.os.Messenger _result = this.getP2pStateMachineMessenger();
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
        case TRANSACTION_close:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.close(_arg0);
          return true;
        }
        case TRANSACTION_setMiracastMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setMiracastMode(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_checkConfigureWifiDisplayPermission:
        {
          data.enforceInterface(descriptor);
          this.checkConfigureWifiDisplayPermission();
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.wifi.p2p.IWifiP2pManager
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
      @Override public android.os.Messenger getMessenger(android.os.IBinder binder) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.Messenger _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(binder);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMessenger, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMessenger(binder);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.Messenger.CREATOR.createFromParcel(_reply);
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
      @Override public android.os.Messenger getP2pStateMachineMessenger() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.Messenger _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getP2pStateMachineMessenger, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getP2pStateMachineMessenger();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.Messenger.CREATOR.createFromParcel(_reply);
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
      @Override public void close(android.os.IBinder binder) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(binder);
          boolean _status = mRemote.transact(Stub.TRANSACTION_close, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().close(binder);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setMiracastMode(int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMiracastMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setMiracastMode(mode);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void checkConfigureWifiDisplayPermission() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_checkConfigureWifiDisplayPermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().checkConfigureWifiDisplayPermission();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.net.wifi.p2p.IWifiP2pManager sDefaultImpl;
    }
    static final int TRANSACTION_getMessenger = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getP2pStateMachineMessenger = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_close = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_setMiracastMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_checkConfigureWifiDisplayPermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(android.net.wifi.p2p.IWifiP2pManager impl) {
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
    public static android.net.wifi.p2p.IWifiP2pManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public android.os.Messenger getMessenger(android.os.IBinder binder) throws android.os.RemoteException;
  public android.os.Messenger getP2pStateMachineMessenger() throws android.os.RemoteException;
  public void close(android.os.IBinder binder) throws android.os.RemoteException;
  public void setMiracastMode(int mode) throws android.os.RemoteException;
  public void checkConfigureWifiDisplayPermission() throws android.os.RemoteException;
}
