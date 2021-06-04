/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view;
/**
 * @hide
 */
public interface IGraphicsStatsCallback extends android.os.IInterface
{
  /** Default implementation for IGraphicsStatsCallback. */
  public static class Default implements android.view.IGraphicsStatsCallback
  {
    @Override public void onRotateGraphicsStatsBuffer() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.IGraphicsStatsCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.view.IGraphicsStatsCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.IGraphicsStatsCallback interface,
     * generating a proxy if needed.
     */
    public static android.view.IGraphicsStatsCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.IGraphicsStatsCallback))) {
        return ((android.view.IGraphicsStatsCallback)iin);
      }
      return new android.view.IGraphicsStatsCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onRotateGraphicsStatsBuffer:
        {
          data.enforceInterface(descriptor);
          this.onRotateGraphicsStatsBuffer();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.IGraphicsStatsCallback
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
      @Override public void onRotateGraphicsStatsBuffer() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRotateGraphicsStatsBuffer, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRotateGraphicsStatsBuffer();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.view.IGraphicsStatsCallback sDefaultImpl;
    }
    static final int TRANSACTION_onRotateGraphicsStatsBuffer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.view.IGraphicsStatsCallback impl) {
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
    public static android.view.IGraphicsStatsCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onRotateGraphicsStatsBuffer() throws android.os.RemoteException;
}
