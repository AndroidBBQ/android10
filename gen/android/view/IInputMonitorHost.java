/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view;
/**
 * @hide
 */
public interface IInputMonitorHost extends android.os.IInterface
{
  /** Default implementation for IInputMonitorHost. */
  public static class Default implements android.view.IInputMonitorHost
  {
    @Override public void pilferPointers() throws android.os.RemoteException
    {
    }
    @Override public void dispose() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.IInputMonitorHost
  {
    private static final java.lang.String DESCRIPTOR = "android.view.IInputMonitorHost";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.IInputMonitorHost interface,
     * generating a proxy if needed.
     */
    public static android.view.IInputMonitorHost asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.IInputMonitorHost))) {
        return ((android.view.IInputMonitorHost)iin);
      }
      return new android.view.IInputMonitorHost.Stub.Proxy(obj);
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
        case TRANSACTION_pilferPointers:
        {
          data.enforceInterface(descriptor);
          this.pilferPointers();
          return true;
        }
        case TRANSACTION_dispose:
        {
          data.enforceInterface(descriptor);
          this.dispose();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.IInputMonitorHost
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
      @Override public void pilferPointers() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_pilferPointers, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().pilferPointers();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void dispose() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispose, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispose();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.view.IInputMonitorHost sDefaultImpl;
    }
    static final int TRANSACTION_pilferPointers = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_dispose = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.view.IInputMonitorHost impl) {
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
    public static android.view.IInputMonitorHost getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void pilferPointers() throws android.os.RemoteException;
  public void dispose() throws android.os.RemoteException;
}
