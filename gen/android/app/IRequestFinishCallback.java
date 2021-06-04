/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app;
/**
 * This callback allows ActivityTaskManager to ask the calling Activity
 * to finish in response to a call to onBackPressedOnTaskRoot.
 *
 * {@hide}
 */
public interface IRequestFinishCallback extends android.os.IInterface
{
  /** Default implementation for IRequestFinishCallback. */
  public static class Default implements android.app.IRequestFinishCallback
  {
    @Override public void requestFinish() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.IRequestFinishCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.app.IRequestFinishCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.IRequestFinishCallback interface,
     * generating a proxy if needed.
     */
    public static android.app.IRequestFinishCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.IRequestFinishCallback))) {
        return ((android.app.IRequestFinishCallback)iin);
      }
      return new android.app.IRequestFinishCallback.Stub.Proxy(obj);
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
        case TRANSACTION_requestFinish:
        {
          data.enforceInterface(descriptor);
          this.requestFinish();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.IRequestFinishCallback
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
      @Override public void requestFinish() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestFinish, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestFinish();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.app.IRequestFinishCallback sDefaultImpl;
    }
    static final int TRANSACTION_requestFinish = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.app.IRequestFinishCallback impl) {
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
    public static android.app.IRequestFinishCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void requestFinish() throws android.os.RemoteException;
}
