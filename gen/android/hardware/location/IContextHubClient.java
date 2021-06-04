/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.location;
/**
 * @hide
 */
public interface IContextHubClient extends android.os.IInterface
{
  /** Default implementation for IContextHubClient. */
  public static class Default implements android.hardware.location.IContextHubClient
  {
    // Sends a message to a nanoapp

    @Override public int sendMessageToNanoApp(android.hardware.location.NanoAppMessage message) throws android.os.RemoteException
    {
      return 0;
    }
    // Closes the connection with the Context Hub

    @Override public void close() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.location.IContextHubClient
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.location.IContextHubClient";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.location.IContextHubClient interface,
     * generating a proxy if needed.
     */
    public static android.hardware.location.IContextHubClient asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.location.IContextHubClient))) {
        return ((android.hardware.location.IContextHubClient)iin);
      }
      return new android.hardware.location.IContextHubClient.Stub.Proxy(obj);
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
        case TRANSACTION_sendMessageToNanoApp:
        {
          data.enforceInterface(descriptor);
          android.hardware.location.NanoAppMessage _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.location.NanoAppMessage.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _result = this.sendMessageToNanoApp(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_close:
        {
          data.enforceInterface(descriptor);
          this.close();
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.location.IContextHubClient
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
      // Sends a message to a nanoapp

      @Override public int sendMessageToNanoApp(android.hardware.location.NanoAppMessage message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((message!=null)) {
            _data.writeInt(1);
            message.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendMessageToNanoApp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().sendMessageToNanoApp(message);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // Closes the connection with the Context Hub

      @Override public void close() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_close, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().close();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.hardware.location.IContextHubClient sDefaultImpl;
    }
    static final int TRANSACTION_sendMessageToNanoApp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_close = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.hardware.location.IContextHubClient impl) {
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
    public static android.hardware.location.IContextHubClient getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // Sends a message to a nanoapp

  public int sendMessageToNanoApp(android.hardware.location.NanoAppMessage message) throws android.os.RemoteException;
  // Closes the connection with the Context Hub

  public void close() throws android.os.RemoteException;
}
