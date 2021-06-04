/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view;
/**
 * Passed to the {@link IRecentsAnimationRunner} in order for the runner to control to let the
 * runner control certain aspects of the recents animation, and to notify window manager when the
 * animation has completed.
 *
 * {@hide}
 */
public interface IRecentsAnimationController extends android.os.IInterface
{
  /** Default implementation for IRecentsAnimationController. */
  public static class Default implements android.view.IRecentsAnimationController
  {
    /**
         * Takes a screenshot of the task associated with the given {@param taskId}. Only valid for the
         * current set of task ids provided to the handler.
         */
    @Override public android.app.ActivityManager.TaskSnapshot screenshotTask(int taskId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Notifies to the system that the animation into Recents should end, and all leashes associated
         * with remote animation targets should be relinquished. If {@param moveHomeToTop} is true, then
         * the home activity should be moved to the top. Otherwise, the home activity is hidden and the
         * user is returned to the app.
         * @param sendUserLeaveHint If set to true, {@link Activity#onUserLeaving} will be sent to the
         *                          top resumed app, false otherwise.
         */
    @Override public void finish(boolean moveHomeToTop, boolean sendUserLeaveHint) throws android.os.RemoteException
    {
    }
    /**
         * Called by the handler to indicate that the recents animation input consumer should be
         * enabled. This is currently used to work around an issue where registering an input consumer
         * mid-animation causes the existing motion event chain to be canceled. Instead, the caller
         * may register the recents animation input consumer prior to starting the recents animation
         * and then enable it mid-animation to start receiving touch events.
         */
    @Override public void setInputConsumerEnabled(boolean enabled) throws android.os.RemoteException
    {
    }
    /**
        * Informs the system whether the animation targets passed into
        * IRecentsAnimationRunner.onAnimationStart are currently behind the system bars. If they are,
        * they can control the SystemUI flags, otherwise the SystemUI flags from home activity will be
        * taken.
        */
    @Override public void setAnimationTargetsBehindSystemBars(boolean behindSystemBars) throws android.os.RemoteException
    {
    }
    /**
         * Informs the system that the primary split-screen stack should be minimized.
         */
    @Override public void setSplitScreenMinimized(boolean minimized) throws android.os.RemoteException
    {
    }
    /**
         * Hides the current input method if one is showing.
         */
    @Override public void hideCurrentInputMethod() throws android.os.RemoteException
    {
    }
    /**
         * This call is deprecated, use #setDeferCancelUntilNextTransition() instead
         * TODO(138144750): Remove this method once there are no callers
         * @deprecated
         */
    @Override public void setCancelWithDeferredScreenshot(boolean screenshot) throws android.os.RemoteException
    {
    }
    /**
         * Clean up the screenshot of previous task which was created during recents animation that
         * was cancelled by a stack order change.
         *
         * @see {@link IRecentsAnimationRunner#onAnimationCanceled}
         */
    @Override public void cleanupScreenshot() throws android.os.RemoteException
    {
    }
    /**
         * Set a state for controller whether would like to cancel recents animations with deferred
         * task screenshot presentation.
         *
         * When we cancel the recents animation due to a stack order change, we can't just cancel it
         * immediately as it would lead to a flicker in Launcher if we just remove the task from the
         * leash. Instead we screenshot the previous task and replace the child of the leash with the
         * screenshot, so that Launcher can still control the leash lifecycle & make the next app
         * transition animate smoothly without flickering.
         *
         * @param defer When set {@code true}, means that the recents animation will defer canceling the
         *              animation when a stack order change is triggered until the subsequent app
         *              transition start and skip previous task's animation.
         *              When set to {@code false}, means that the recents animation will be canceled
         *              immediately when the stack order changes.
         * @param screenshot When set {@code true}, means that the system will take previous task's
         *                   screenshot and replace the contents of the leash with it when the next app
         *                   transition starting. The runner must call #cleanupScreenshot() to end the
         *                   recents animation.
         *                   When set to {@code false}, means that the system will simply wait for the
         *                   next app transition start to immediately cancel the recents animation. This
         *                   can be useful when you want an immediate transition into a state where the
         *                   task is shown in the home/recents activity (without waiting for a
         *                   screenshot).
         *
         * @see #cleanupScreenshot()
         * @see IRecentsAnimationRunner#onCancelled
         */
    @Override public void setDeferCancelUntilNextTransition(boolean defer, boolean screenshot) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.IRecentsAnimationController
  {
    private static final java.lang.String DESCRIPTOR = "android.view.IRecentsAnimationController";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.IRecentsAnimationController interface,
     * generating a proxy if needed.
     */
    public static android.view.IRecentsAnimationController asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.IRecentsAnimationController))) {
        return ((android.view.IRecentsAnimationController)iin);
      }
      return new android.view.IRecentsAnimationController.Stub.Proxy(obj);
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
        case TRANSACTION_screenshotTask:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.app.ActivityManager.TaskSnapshot _result = this.screenshotTask(_arg0);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_finish:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.finish(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setInputConsumerEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setInputConsumerEnabled(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setAnimationTargetsBehindSystemBars:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setAnimationTargetsBehindSystemBars(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setSplitScreenMinimized:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setSplitScreenMinimized(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_hideCurrentInputMethod:
        {
          data.enforceInterface(descriptor);
          this.hideCurrentInputMethod();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setCancelWithDeferredScreenshot:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setCancelWithDeferredScreenshot(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_cleanupScreenshot:
        {
          data.enforceInterface(descriptor);
          this.cleanupScreenshot();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setDeferCancelUntilNextTransition:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setDeferCancelUntilNextTransition(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.IRecentsAnimationController
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
           * Takes a screenshot of the task associated with the given {@param taskId}. Only valid for the
           * current set of task ids provided to the handler.
           */
      @Override public android.app.ActivityManager.TaskSnapshot screenshotTask(int taskId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.ActivityManager.TaskSnapshot _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(taskId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_screenshotTask, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().screenshotTask(taskId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.app.ActivityManager.TaskSnapshot.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Notifies to the system that the animation into Recents should end, and all leashes associated
           * with remote animation targets should be relinquished. If {@param moveHomeToTop} is true, then
           * the home activity should be moved to the top. Otherwise, the home activity is hidden and the
           * user is returned to the app.
           * @param sendUserLeaveHint If set to true, {@link Activity#onUserLeaving} will be sent to the
           *                          top resumed app, false otherwise.
           */
      @Override public void finish(boolean moveHomeToTop, boolean sendUserLeaveHint) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((moveHomeToTop)?(1):(0)));
          _data.writeInt(((sendUserLeaveHint)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_finish, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().finish(moveHomeToTop, sendUserLeaveHint);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Called by the handler to indicate that the recents animation input consumer should be
           * enabled. This is currently used to work around an issue where registering an input consumer
           * mid-animation causes the existing motion event chain to be canceled. Instead, the caller
           * may register the recents animation input consumer prior to starting the recents animation
           * and then enable it mid-animation to start receiving touch events.
           */
      @Override public void setInputConsumerEnabled(boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setInputConsumerEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setInputConsumerEnabled(enabled);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
          * Informs the system whether the animation targets passed into
          * IRecentsAnimationRunner.onAnimationStart are currently behind the system bars. If they are,
          * they can control the SystemUI flags, otherwise the SystemUI flags from home activity will be
          * taken.
          */
      @Override public void setAnimationTargetsBehindSystemBars(boolean behindSystemBars) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((behindSystemBars)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAnimationTargetsBehindSystemBars, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAnimationTargetsBehindSystemBars(behindSystemBars);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Informs the system that the primary split-screen stack should be minimized.
           */
      @Override public void setSplitScreenMinimized(boolean minimized) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((minimized)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSplitScreenMinimized, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSplitScreenMinimized(minimized);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Hides the current input method if one is showing.
           */
      @Override public void hideCurrentInputMethod() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hideCurrentInputMethod, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().hideCurrentInputMethod();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * This call is deprecated, use #setDeferCancelUntilNextTransition() instead
           * TODO(138144750): Remove this method once there are no callers
           * @deprecated
           */
      @Override public void setCancelWithDeferredScreenshot(boolean screenshot) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((screenshot)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setCancelWithDeferredScreenshot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setCancelWithDeferredScreenshot(screenshot);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Clean up the screenshot of previous task which was created during recents animation that
           * was cancelled by a stack order change.
           *
           * @see {@link IRecentsAnimationRunner#onAnimationCanceled}
           */
      @Override public void cleanupScreenshot() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cleanupScreenshot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cleanupScreenshot();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Set a state for controller whether would like to cancel recents animations with deferred
           * task screenshot presentation.
           *
           * When we cancel the recents animation due to a stack order change, we can't just cancel it
           * immediately as it would lead to a flicker in Launcher if we just remove the task from the
           * leash. Instead we screenshot the previous task and replace the child of the leash with the
           * screenshot, so that Launcher can still control the leash lifecycle & make the next app
           * transition animate smoothly without flickering.
           *
           * @param defer When set {@code true}, means that the recents animation will defer canceling the
           *              animation when a stack order change is triggered until the subsequent app
           *              transition start and skip previous task's animation.
           *              When set to {@code false}, means that the recents animation will be canceled
           *              immediately when the stack order changes.
           * @param screenshot When set {@code true}, means that the system will take previous task's
           *                   screenshot and replace the contents of the leash with it when the next app
           *                   transition starting. The runner must call #cleanupScreenshot() to end the
           *                   recents animation.
           *                   When set to {@code false}, means that the system will simply wait for the
           *                   next app transition start to immediately cancel the recents animation. This
           *                   can be useful when you want an immediate transition into a state where the
           *                   task is shown in the home/recents activity (without waiting for a
           *                   screenshot).
           *
           * @see #cleanupScreenshot()
           * @see IRecentsAnimationRunner#onCancelled
           */
      @Override public void setDeferCancelUntilNextTransition(boolean defer, boolean screenshot) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((defer)?(1):(0)));
          _data.writeInt(((screenshot)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDeferCancelUntilNextTransition, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDeferCancelUntilNextTransition(defer, screenshot);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.view.IRecentsAnimationController sDefaultImpl;
    }
    static final int TRANSACTION_screenshotTask = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_finish = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_setInputConsumerEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_setAnimationTargetsBehindSystemBars = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_setSplitScreenMinimized = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_hideCurrentInputMethod = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_setCancelWithDeferredScreenshot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_cleanupScreenshot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_setDeferCancelUntilNextTransition = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    public static boolean setDefaultImpl(android.view.IRecentsAnimationController impl) {
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
    public static android.view.IRecentsAnimationController getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Takes a screenshot of the task associated with the given {@param taskId}. Only valid for the
       * current set of task ids provided to the handler.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IRecentsAnimationController.aidl:36:1:36:25")
  public android.app.ActivityManager.TaskSnapshot screenshotTask(int taskId) throws android.os.RemoteException;
  /**
       * Notifies to the system that the animation into Recents should end, and all leashes associated
       * with remote animation targets should be relinquished. If {@param moveHomeToTop} is true, then
       * the home activity should be moved to the top. Otherwise, the home activity is hidden and the
       * user is returned to the app.
       * @param sendUserLeaveHint If set to true, {@link Activity#onUserLeaving} will be sent to the
       *                          top resumed app, false otherwise.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IRecentsAnimationController.aidl:47:1:47:25")
  public void finish(boolean moveHomeToTop, boolean sendUserLeaveHint) throws android.os.RemoteException;
  /**
       * Called by the handler to indicate that the recents animation input consumer should be
       * enabled. This is currently used to work around an issue where registering an input consumer
       * mid-animation causes the existing motion event chain to be canceled. Instead, the caller
       * may register the recents animation input consumer prior to starting the recents animation
       * and then enable it mid-animation to start receiving touch events.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IRecentsAnimationController.aidl:57:1:57:25")
  public void setInputConsumerEnabled(boolean enabled) throws android.os.RemoteException;
  /**
      * Informs the system whether the animation targets passed into
      * IRecentsAnimationRunner.onAnimationStart are currently behind the system bars. If they are,
      * they can control the SystemUI flags, otherwise the SystemUI flags from home activity will be
      * taken.
      */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IRecentsAnimationController.aidl:66:1:66:25")
  public void setAnimationTargetsBehindSystemBars(boolean behindSystemBars) throws android.os.RemoteException;
  /**
       * Informs the system that the primary split-screen stack should be minimized.
       */
  public void setSplitScreenMinimized(boolean minimized) throws android.os.RemoteException;
  /**
       * Hides the current input method if one is showing.
       */
  public void hideCurrentInputMethod() throws android.os.RemoteException;
  /**
       * This call is deprecated, use #setDeferCancelUntilNextTransition() instead
       * TODO(138144750): Remove this method once there are no callers
       * @deprecated
       */
  public void setCancelWithDeferredScreenshot(boolean screenshot) throws android.os.RemoteException;
  /**
       * Clean up the screenshot of previous task which was created during recents animation that
       * was cancelled by a stack order change.
       *
       * @see {@link IRecentsAnimationRunner#onAnimationCanceled}
       */
  public void cleanupScreenshot() throws android.os.RemoteException;
  /**
       * Set a state for controller whether would like to cancel recents animations with deferred
       * task screenshot presentation.
       *
       * When we cancel the recents animation due to a stack order change, we can't just cancel it
       * immediately as it would lead to a flicker in Launcher if we just remove the task from the
       * leash. Instead we screenshot the previous task and replace the child of the leash with the
       * screenshot, so that Launcher can still control the leash lifecycle & make the next app
       * transition animate smoothly without flickering.
       *
       * @param defer When set {@code true}, means that the recents animation will defer canceling the
       *              animation when a stack order change is triggered until the subsequent app
       *              transition start and skip previous task's animation.
       *              When set to {@code false}, means that the recents animation will be canceled
       *              immediately when the stack order changes.
       * @param screenshot When set {@code true}, means that the system will take previous task's
       *                   screenshot and replace the contents of the leash with it when the next app
       *                   transition starting. The runner must call #cleanupScreenshot() to end the
       *                   recents animation.
       *                   When set to {@code false}, means that the system will simply wait for the
       *                   next app transition start to immediately cancel the recents animation. This
       *                   can be useful when you want an immediate transition into a state where the
       *                   task is shown in the home/recents activity (without waiting for a
       *                   screenshot).
       *
       * @see #cleanupScreenshot()
       * @see IRecentsAnimationRunner#onCancelled
       */
  public void setDeferCancelUntilNextTransition(boolean defer, boolean screenshot) throws android.os.RemoteException;
}
