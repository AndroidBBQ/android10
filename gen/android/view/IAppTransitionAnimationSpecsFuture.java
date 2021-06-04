/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view;
/**
 * A cross-process future to fetch the specifications for app transitions.
 *
 * {@hide}
 */
public interface IAppTransitionAnimationSpecsFuture extends android.os.IInterface
{
  /** Default implementation for IAppTransitionAnimationSpecsFuture. */
  public static class Default implements android.view.IAppTransitionAnimationSpecsFuture
  {
    @Override public android.view.AppTransitionAnimationSpec[] get() throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.IAppTransitionAnimationSpecsFuture
  {
    private static final java.lang.String DESCRIPTOR = "android.view.IAppTransitionAnimationSpecsFuture";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.IAppTransitionAnimationSpecsFuture interface,
     * generating a proxy if needed.
     */
    public static android.view.IAppTransitionAnimationSpecsFuture asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.IAppTransitionAnimationSpecsFuture))) {
        return ((android.view.IAppTransitionAnimationSpecsFuture)iin);
      }
      return new android.view.IAppTransitionAnimationSpecsFuture.Stub.Proxy(obj);
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
        case TRANSACTION_get:
        {
          data.enforceInterface(descriptor);
          android.view.AppTransitionAnimationSpec[] _result = this.get();
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.IAppTransitionAnimationSpecsFuture
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
      @Override public android.view.AppTransitionAnimationSpec[] get() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.view.AppTransitionAnimationSpec[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_get, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().get();
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.view.AppTransitionAnimationSpec.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.view.IAppTransitionAnimationSpecsFuture sDefaultImpl;
    }
    static final int TRANSACTION_get = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.view.IAppTransitionAnimationSpecsFuture impl) {
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
    public static android.view.IAppTransitionAnimationSpecsFuture getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public android.view.AppTransitionAnimationSpec[] get() throws android.os.RemoteException;
}
