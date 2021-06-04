/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view;
/**
 * Interface that is used to callback from window manager to the process that runs a remote
 * animation to start or cancel it.
 *
 * {@hide}
 */
public interface IRemoteAnimationRunner extends android.os.IInterface
{
  /** Default implementation for IRemoteAnimationRunner. */
  public static class Default implements android.view.IRemoteAnimationRunner
  {
    /**
         * Called when the process needs to start the remote animation.
         *
         * @param apps The list of apps to animate.
         * @param finishedCallback The callback to invoke when the animation is finished.
         */
    @Override public void onAnimationStart(android.view.RemoteAnimationTarget[] apps, android.view.IRemoteAnimationFinishedCallback finishedCallback) throws android.os.RemoteException
    {
    }
    /**
         * Called when the animation was cancelled. From this point on, any updates onto the leashes
         * won't have any effect anymore.
         */
    @Override public void onAnimationCancelled() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.IRemoteAnimationRunner
  {
    private static final java.lang.String DESCRIPTOR = "android.view.IRemoteAnimationRunner";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.IRemoteAnimationRunner interface,
     * generating a proxy if needed.
     */
    public static android.view.IRemoteAnimationRunner asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.IRemoteAnimationRunner))) {
        return ((android.view.IRemoteAnimationRunner)iin);
      }
      return new android.view.IRemoteAnimationRunner.Stub.Proxy(obj);
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
        case TRANSACTION_onAnimationStart:
        {
          data.enforceInterface(descriptor);
          android.view.RemoteAnimationTarget[] _arg0;
          _arg0 = data.createTypedArray(android.view.RemoteAnimationTarget.CREATOR);
          android.view.IRemoteAnimationFinishedCallback _arg1;
          _arg1 = android.view.IRemoteAnimationFinishedCallback.Stub.asInterface(data.readStrongBinder());
          this.onAnimationStart(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onAnimationCancelled:
        {
          data.enforceInterface(descriptor);
          this.onAnimationCancelled();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.IRemoteAnimationRunner
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
      /**
           * Called when the process needs to start the remote animation.
           *
           * @param apps The list of apps to animate.
           * @param finishedCallback The callback to invoke when the animation is finished.
           */
      @Override public void onAnimationStart(android.view.RemoteAnimationTarget[] apps, android.view.IRemoteAnimationFinishedCallback finishedCallback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedArray(apps, 0);
          _data.writeStrongBinder((((finishedCallback!=null))?(finishedCallback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAnimationStart, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAnimationStart(apps, finishedCallback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when the animation was cancelled. From this point on, any updates onto the leashes
           * won't have any effect anymore.
           */
      @Override public void onAnimationCancelled() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAnimationCancelled, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAnimationCancelled();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.view.IRemoteAnimationRunner sDefaultImpl;
    }
    static final int TRANSACTION_onAnimationStart = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onAnimationCancelled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.view.IRemoteAnimationRunner impl) {
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
    public static android.view.IRemoteAnimationRunner getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Called when the process needs to start the remote animation.
       *
       * @param apps The list of apps to animate.
       * @param finishedCallback The callback to invoke when the animation is finished.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IRemoteAnimationRunner.aidl:36:1:36:25")
  public void onAnimationStart(android.view.RemoteAnimationTarget[] apps, android.view.IRemoteAnimationFinishedCallback finishedCallback) throws android.os.RemoteException;
  /**
       * Called when the animation was cancelled. From this point on, any updates onto the leashes
       * won't have any effect anymore.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IRemoteAnimationRunner.aidl:44:1:44:25")
  public void onAnimationCancelled() throws android.os.RemoteException;
}
