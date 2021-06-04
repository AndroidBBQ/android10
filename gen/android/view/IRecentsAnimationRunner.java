/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view;
/**
 * Interface that is used to callback from window manager to the process that runs a recents
 * animation to start or cancel it.
 *
 * {@hide}
 */
public interface IRecentsAnimationRunner extends android.os.IInterface
{
  /** Default implementation for IRecentsAnimationRunner. */
  public static class Default implements android.view.IRecentsAnimationRunner
  {
    /**
         * Called when the system needs to cancel the current animation. This can be due to the
         * wallpaper not drawing in time, or the handler not finishing the animation within a predefined
         * amount of time.
         *
         * @param deferredWithScreenshot If set to {@code true}, the contents of the task will be
         *                               replaced with a screenshot, such that the runner's leash is
         *                               still active. As soon as the runner doesn't need the leash
         *                               anymore, it must call
         *                               {@link IRecentsAnimationController#cleanupScreenshot).
         *
         * @see {@link RecentsAnimationController#cleanupScreenshot}
         */
    @Override public void onAnimationCanceled(boolean deferredWithScreenshot) throws android.os.RemoteException
    {
    }
    /**
         * Called when the system is ready for the handler to start animating all the visible tasks.
         *
         * @param homeContentInsets The current home app content insets
         * @param minimizedHomeBounds Specifies the bounds of the minimized home app, will be
         *                            {@code null} if the device is not currently in split screen
         */
    @Override public void onAnimationStart(android.view.IRecentsAnimationController controller, android.view.RemoteAnimationTarget[] apps, android.graphics.Rect homeContentInsets, android.graphics.Rect minimizedHomeBounds) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.IRecentsAnimationRunner
  {
    private static final java.lang.String DESCRIPTOR = "android.view.IRecentsAnimationRunner";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.IRecentsAnimationRunner interface,
     * generating a proxy if needed.
     */
    public static android.view.IRecentsAnimationRunner asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.IRecentsAnimationRunner))) {
        return ((android.view.IRecentsAnimationRunner)iin);
      }
      return new android.view.IRecentsAnimationRunner.Stub.Proxy(obj);
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
        case TRANSACTION_onAnimationCanceled:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onAnimationCanceled(_arg0);
          return true;
        }
        case TRANSACTION_onAnimationStart:
        {
          data.enforceInterface(descriptor);
          android.view.IRecentsAnimationController _arg0;
          _arg0 = android.view.IRecentsAnimationController.Stub.asInterface(data.readStrongBinder());
          android.view.RemoteAnimationTarget[] _arg1;
          _arg1 = data.createTypedArray(android.view.RemoteAnimationTarget.CREATOR);
          android.graphics.Rect _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.graphics.Rect _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.onAnimationStart(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.IRecentsAnimationRunner
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
           * Called when the system needs to cancel the current animation. This can be due to the
           * wallpaper not drawing in time, or the handler not finishing the animation within a predefined
           * amount of time.
           *
           * @param deferredWithScreenshot If set to {@code true}, the contents of the task will be
           *                               replaced with a screenshot, such that the runner's leash is
           *                               still active. As soon as the runner doesn't need the leash
           *                               anymore, it must call
           *                               {@link IRecentsAnimationController#cleanupScreenshot).
           *
           * @see {@link RecentsAnimationController#cleanupScreenshot}
           */
      @Override public void onAnimationCanceled(boolean deferredWithScreenshot) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((deferredWithScreenshot)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAnimationCanceled, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAnimationCanceled(deferredWithScreenshot);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when the system is ready for the handler to start animating all the visible tasks.
           *
           * @param homeContentInsets The current home app content insets
           * @param minimizedHomeBounds Specifies the bounds of the minimized home app, will be
           *                            {@code null} if the device is not currently in split screen
           */
      @Override public void onAnimationStart(android.view.IRecentsAnimationController controller, android.view.RemoteAnimationTarget[] apps, android.graphics.Rect homeContentInsets, android.graphics.Rect minimizedHomeBounds) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((controller!=null))?(controller.asBinder()):(null)));
          _data.writeTypedArray(apps, 0);
          if ((homeContentInsets!=null)) {
            _data.writeInt(1);
            homeContentInsets.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((minimizedHomeBounds!=null)) {
            _data.writeInt(1);
            minimizedHomeBounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAnimationStart, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAnimationStart(controller, apps, homeContentInsets, minimizedHomeBounds);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.view.IRecentsAnimationRunner sDefaultImpl;
    }
    static final int TRANSACTION_onAnimationCanceled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onAnimationStart = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.view.IRecentsAnimationRunner impl) {
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
    public static android.view.IRecentsAnimationRunner getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Called when the system needs to cancel the current animation. This can be due to the
       * wallpaper not drawing in time, or the handler not finishing the animation within a predefined
       * amount of time.
       *
       * @param deferredWithScreenshot If set to {@code true}, the contents of the task will be
       *                               replaced with a screenshot, such that the runner's leash is
       *                               still active. As soon as the runner doesn't need the leash
       *                               anymore, it must call
       *                               {@link IRecentsAnimationController#cleanupScreenshot).
       *
       * @see {@link RecentsAnimationController#cleanupScreenshot}
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IRecentsAnimationRunner.aidl:44:1:44:25")
  public void onAnimationCanceled(boolean deferredWithScreenshot) throws android.os.RemoteException;
  /**
       * Called when the system is ready for the handler to start animating all the visible tasks.
       *
       * @param homeContentInsets The current home app content insets
       * @param minimizedHomeBounds Specifies the bounds of the minimized home app, will be
       *                            {@code null} if the device is not currently in split screen
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IRecentsAnimationRunner.aidl:54:1:54:25")
  public void onAnimationStart(android.view.IRecentsAnimationController controller, android.view.RemoteAnimationTarget[] apps, android.graphics.Rect homeContentInsets, android.graphics.Rect minimizedHomeBounds) throws android.os.RemoteException;
}
