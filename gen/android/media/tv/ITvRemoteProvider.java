/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media.tv;
/**
 * {@hide}
 */
public interface ITvRemoteProvider extends android.os.IInterface
{
  /** Default implementation for ITvRemoteProvider. */
  public static class Default implements android.media.tv.ITvRemoteProvider
  {
    @Override public void setRemoteServiceInputSink(android.media.tv.ITvRemoteServiceInput tvServiceInput) throws android.os.RemoteException
    {
    }
    @Override public void onInputBridgeConnected(android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.tv.ITvRemoteProvider
  {
    private static final java.lang.String DESCRIPTOR = "android.media.tv.ITvRemoteProvider";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.tv.ITvRemoteProvider interface,
     * generating a proxy if needed.
     */
    public static android.media.tv.ITvRemoteProvider asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.tv.ITvRemoteProvider))) {
        return ((android.media.tv.ITvRemoteProvider)iin);
      }
      return new android.media.tv.ITvRemoteProvider.Stub.Proxy(obj);
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
        case TRANSACTION_setRemoteServiceInputSink:
        {
          data.enforceInterface(descriptor);
          android.media.tv.ITvRemoteServiceInput _arg0;
          _arg0 = android.media.tv.ITvRemoteServiceInput.Stub.asInterface(data.readStrongBinder());
          this.setRemoteServiceInputSink(_arg0);
          return true;
        }
        case TRANSACTION_onInputBridgeConnected:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.onInputBridgeConnected(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.tv.ITvRemoteProvider
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
      @Override public void setRemoteServiceInputSink(android.media.tv.ITvRemoteServiceInput tvServiceInput) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((tvServiceInput!=null))?(tvServiceInput.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRemoteServiceInputSink, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setRemoteServiceInputSink(tvServiceInput);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onInputBridgeConnected(android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onInputBridgeConnected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onInputBridgeConnected(token);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.tv.ITvRemoteProvider sDefaultImpl;
    }
    static final int TRANSACTION_setRemoteServiceInputSink = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onInputBridgeConnected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.media.tv.ITvRemoteProvider impl) {
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
    public static android.media.tv.ITvRemoteProvider getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void setRemoteServiceInputSink(android.media.tv.ITvRemoteServiceInput tvServiceInput) throws android.os.RemoteException;
  public void onInputBridgeConnected(android.os.IBinder token) throws android.os.RemoteException;
}
