/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view;
/**
 * Interface to be invoked by the controlling process when a remote animation has finished.
 *
 * @see IRemoteAnimationRunner
 * {@hide}
 */
public interface IRemoteAnimationFinishedCallback extends android.os.IInterface
{
  /** Default implementation for IRemoteAnimationFinishedCallback. */
  public static class Default implements android.view.IRemoteAnimationFinishedCallback
  {
    @Override public void onAnimationFinished() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.IRemoteAnimationFinishedCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.view.IRemoteAnimationFinishedCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.IRemoteAnimationFinishedCallback interface,
     * generating a proxy if needed.
     */
    public static android.view.IRemoteAnimationFinishedCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.IRemoteAnimationFinishedCallback))) {
        return ((android.view.IRemoteAnimationFinishedCallback)iin);
      }
      return new android.view.IRemoteAnimationFinishedCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onAnimationFinished:
        {
          data.enforceInterface(descriptor);
          this.onAnimationFinished();
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.IRemoteAnimationFinishedCallback
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
      @Override public void onAnimationFinished() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAnimationFinished, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAnimationFinished();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.view.IRemoteAnimationFinishedCallback sDefaultImpl;
    }
    static final int TRANSACTION_onAnimationFinished = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.view.IRemoteAnimationFinishedCallback impl) {
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
    public static android.view.IRemoteAnimationFinishedCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IRemoteAnimationFinishedCallback.aidl:26:1:26:25")
  public void onAnimationFinished() throws android.os.RemoteException;
}
