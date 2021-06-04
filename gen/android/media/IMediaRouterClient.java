/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media;
/**
 * {@hide}
 */
public interface IMediaRouterClient extends android.os.IInterface
{
  /** Default implementation for IMediaRouterClient. */
  public static class Default implements android.media.IMediaRouterClient
  {
    @Override public void onStateChanged() throws android.os.RemoteException
    {
    }
    @Override public void onRestoreRoute() throws android.os.RemoteException
    {
    }
    @Override public void onSelectedRouteChanged(java.lang.String routeId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.IMediaRouterClient
  {
    private static final java.lang.String DESCRIPTOR = "android.media.IMediaRouterClient";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.IMediaRouterClient interface,
     * generating a proxy if needed.
     */
    public static android.media.IMediaRouterClient asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.IMediaRouterClient))) {
        return ((android.media.IMediaRouterClient)iin);
      }
      return new android.media.IMediaRouterClient.Stub.Proxy(obj);
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
        case TRANSACTION_onStateChanged:
        {
          data.enforceInterface(descriptor);
          this.onStateChanged();
          return true;
        }
        case TRANSACTION_onRestoreRoute:
        {
          data.enforceInterface(descriptor);
          this.onRestoreRoute();
          return true;
        }
        case TRANSACTION_onSelectedRouteChanged:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.onSelectedRouteChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.IMediaRouterClient
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
      @Override public void onStateChanged() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onStateChanged();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onRestoreRoute() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRestoreRoute, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRestoreRoute();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSelectedRouteChanged(java.lang.String routeId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(routeId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSelectedRouteChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSelectedRouteChanged(routeId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.IMediaRouterClient sDefaultImpl;
    }
    static final int TRANSACTION_onStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onRestoreRoute = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onSelectedRouteChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.media.IMediaRouterClient impl) {
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
    public static android.media.IMediaRouterClient getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onStateChanged() throws android.os.RemoteException;
  public void onRestoreRoute() throws android.os.RemoteException;
  public void onSelectedRouteChanged(java.lang.String routeId) throws android.os.RemoteException;
}
