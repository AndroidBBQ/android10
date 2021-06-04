/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view;
/**
 * Listener for changes to the pinned stack made by the WindowManager.
 *
 * @hide
 */
public interface IPinnedStackListener extends android.os.IInterface
{
  /** Default implementation for IPinnedStackListener. */
  public static class Default implements android.view.IPinnedStackListener
  {
    /**
         * Called when the listener is registered and provides an interface to call back to the pinned
         * stack controller to update the controller of the pinned stack state.
         */
    @Override public void onListenerRegistered(android.view.IPinnedStackController controller) throws android.os.RemoteException
    {
    }
    /**
         * Called when the window manager has detected a change that would cause the movement bounds
         * to be changed (ie. after configuration change, aspect ratio change, etc). It then provides
         * the components that allow the listener to calculate the movement bounds itself. The
         * {@param normalBounds} are also the default bounds that the PiP would be entered in its
         * current state with the aspect ratio applied.  The {@param animatingBounds} are provided
         * to indicate the current target bounds of the pinned stack (the final bounds if animating,
         * the current bounds if not), which may be helpful in calculating dependent animation bounds.
         *
         * The {@param displayRotation} is provided so that the client can verify when making certain
         * calls that it will not provide stale information based on an old display rotation (ie. if
         * the WM has changed in the mean time but the client has not received onMovementBoundsChanged).
         */
    @Override public void onMovementBoundsChanged(android.graphics.Rect insetBounds, android.graphics.Rect normalBounds, android.graphics.Rect animatingBounds, boolean fromImeAdjustment, boolean fromShelfAdjustment, int displayRotation) throws android.os.RemoteException
    {
    }
    /**
         * Called when window manager decides to adjust the pinned stack bounds because of the IME, or
         * when the listener is first registered to allow the listener to synchronized its state with
         * the controller.  This call will always be followed by a onMovementBoundsChanged() call
         * with fromImeAdjustement set to {@code true}.
         */
    @Override public void onImeVisibilityChanged(boolean imeVisible, int imeHeight) throws android.os.RemoteException
    {
    }
    /**
         * Called when window manager decides to adjust the pinned stack bounds because of the shelf, or
         * when the listener is first registered to allow the listener to synchronized its state with
         * the controller.  This call will always be followed by a onMovementBoundsChanged() call
         * with fromShelfAdjustment set to {@code true}.
         */
    @Override public void onShelfVisibilityChanged(boolean shelfVisible, int shelfHeight) throws android.os.RemoteException
    {
    }
    /**
         * Called when window manager decides to adjust the minimized state, or when the listener
         * is first registered to allow the listener to synchronized its state with the controller.
         */
    @Override public void onMinimizedStateChanged(boolean isMinimized) throws android.os.RemoteException
    {
    }
    /**
         * Called when the set of actions for the current PiP activity changes, or when the listener
         * is first registered to allow the listener to synchronized its state with the controller.
         */
    @Override public void onActionsChanged(android.content.pm.ParceledListSlice actions) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.IPinnedStackListener
  {
    private static final java.lang.String DESCRIPTOR = "android.view.IPinnedStackListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.IPinnedStackListener interface,
     * generating a proxy if needed.
     */
    public static android.view.IPinnedStackListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.IPinnedStackListener))) {
        return ((android.view.IPinnedStackListener)iin);
      }
      return new android.view.IPinnedStackListener.Stub.Proxy(obj);
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
        case TRANSACTION_onListenerRegistered:
        {
          data.enforceInterface(descriptor);
          android.view.IPinnedStackController _arg0;
          _arg0 = android.view.IPinnedStackController.Stub.asInterface(data.readStrongBinder());
          this.onListenerRegistered(_arg0);
          return true;
        }
        case TRANSACTION_onMovementBoundsChanged:
        {
          data.enforceInterface(descriptor);
          android.graphics.Rect _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.graphics.Rect _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.graphics.Rect _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          int _arg5;
          _arg5 = data.readInt();
          this.onMovementBoundsChanged(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          return true;
        }
        case TRANSACTION_onImeVisibilityChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          this.onImeVisibilityChanged(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onShelfVisibilityChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          this.onShelfVisibilityChanged(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onMinimizedStateChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onMinimizedStateChanged(_arg0);
          return true;
        }
        case TRANSACTION_onActionsChanged:
        {
          data.enforceInterface(descriptor);
          android.content.pm.ParceledListSlice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onActionsChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.IPinnedStackListener
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
           * Called when the listener is registered and provides an interface to call back to the pinned
           * stack controller to update the controller of the pinned stack state.
           */
      @Override public void onListenerRegistered(android.view.IPinnedStackController controller) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((controller!=null))?(controller.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onListenerRegistered, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onListenerRegistered(controller);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when the window manager has detected a change that would cause the movement bounds
           * to be changed (ie. after configuration change, aspect ratio change, etc). It then provides
           * the components that allow the listener to calculate the movement bounds itself. The
           * {@param normalBounds} are also the default bounds that the PiP would be entered in its
           * current state with the aspect ratio applied.  The {@param animatingBounds} are provided
           * to indicate the current target bounds of the pinned stack (the final bounds if animating,
           * the current bounds if not), which may be helpful in calculating dependent animation bounds.
           *
           * The {@param displayRotation} is provided so that the client can verify when making certain
           * calls that it will not provide stale information based on an old display rotation (ie. if
           * the WM has changed in the mean time but the client has not received onMovementBoundsChanged).
           */
      @Override public void onMovementBoundsChanged(android.graphics.Rect insetBounds, android.graphics.Rect normalBounds, android.graphics.Rect animatingBounds, boolean fromImeAdjustment, boolean fromShelfAdjustment, int displayRotation) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((insetBounds!=null)) {
            _data.writeInt(1);
            insetBounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((normalBounds!=null)) {
            _data.writeInt(1);
            normalBounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((animatingBounds!=null)) {
            _data.writeInt(1);
            animatingBounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((fromImeAdjustment)?(1):(0)));
          _data.writeInt(((fromShelfAdjustment)?(1):(0)));
          _data.writeInt(displayRotation);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onMovementBoundsChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onMovementBoundsChanged(insetBounds, normalBounds, animatingBounds, fromImeAdjustment, fromShelfAdjustment, displayRotation);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when window manager decides to adjust the pinned stack bounds because of the IME, or
           * when the listener is first registered to allow the listener to synchronized its state with
           * the controller.  This call will always be followed by a onMovementBoundsChanged() call
           * with fromImeAdjustement set to {@code true}.
           */
      @Override public void onImeVisibilityChanged(boolean imeVisible, int imeHeight) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((imeVisible)?(1):(0)));
          _data.writeInt(imeHeight);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onImeVisibilityChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onImeVisibilityChanged(imeVisible, imeHeight);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when window manager decides to adjust the pinned stack bounds because of the shelf, or
           * when the listener is first registered to allow the listener to synchronized its state with
           * the controller.  This call will always be followed by a onMovementBoundsChanged() call
           * with fromShelfAdjustment set to {@code true}.
           */
      @Override public void onShelfVisibilityChanged(boolean shelfVisible, int shelfHeight) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((shelfVisible)?(1):(0)));
          _data.writeInt(shelfHeight);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onShelfVisibilityChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onShelfVisibilityChanged(shelfVisible, shelfHeight);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when window manager decides to adjust the minimized state, or when the listener
           * is first registered to allow the listener to synchronized its state with the controller.
           */
      @Override public void onMinimizedStateChanged(boolean isMinimized) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((isMinimized)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onMinimizedStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onMinimizedStateChanged(isMinimized);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when the set of actions for the current PiP activity changes, or when the listener
           * is first registered to allow the listener to synchronized its state with the controller.
           */
      @Override public void onActionsChanged(android.content.pm.ParceledListSlice actions) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((actions!=null)) {
            _data.writeInt(1);
            actions.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onActionsChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onActionsChanged(actions);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.view.IPinnedStackListener sDefaultImpl;
    }
    static final int TRANSACTION_onListenerRegistered = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onMovementBoundsChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onImeVisibilityChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onShelfVisibilityChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onMinimizedStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onActionsChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(android.view.IPinnedStackListener impl) {
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
    public static android.view.IPinnedStackListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Called when the listener is registered and provides an interface to call back to the pinned
       * stack controller to update the controller of the pinned stack state.
       */
  public void onListenerRegistered(android.view.IPinnedStackController controller) throws android.os.RemoteException;
  /**
       * Called when the window manager has detected a change that would cause the movement bounds
       * to be changed (ie. after configuration change, aspect ratio change, etc). It then provides
       * the components that allow the listener to calculate the movement bounds itself. The
       * {@param normalBounds} are also the default bounds that the PiP would be entered in its
       * current state with the aspect ratio applied.  The {@param animatingBounds} are provided
       * to indicate the current target bounds of the pinned stack (the final bounds if animating,
       * the current bounds if not), which may be helpful in calculating dependent animation bounds.
       *
       * The {@param displayRotation} is provided so that the client can verify when making certain
       * calls that it will not provide stale information based on an old display rotation (ie. if
       * the WM has changed in the mean time but the client has not received onMovementBoundsChanged).
       */
  public void onMovementBoundsChanged(android.graphics.Rect insetBounds, android.graphics.Rect normalBounds, android.graphics.Rect animatingBounds, boolean fromImeAdjustment, boolean fromShelfAdjustment, int displayRotation) throws android.os.RemoteException;
  /**
       * Called when window manager decides to adjust the pinned stack bounds because of the IME, or
       * when the listener is first registered to allow the listener to synchronized its state with
       * the controller.  This call will always be followed by a onMovementBoundsChanged() call
       * with fromImeAdjustement set to {@code true}.
       */
  public void onImeVisibilityChanged(boolean imeVisible, int imeHeight) throws android.os.RemoteException;
  /**
       * Called when window manager decides to adjust the pinned stack bounds because of the shelf, or
       * when the listener is first registered to allow the listener to synchronized its state with
       * the controller.  This call will always be followed by a onMovementBoundsChanged() call
       * with fromShelfAdjustment set to {@code true}.
       */
  public void onShelfVisibilityChanged(boolean shelfVisible, int shelfHeight) throws android.os.RemoteException;
  /**
       * Called when window manager decides to adjust the minimized state, or when the listener
       * is first registered to allow the listener to synchronized its state with the controller.
       */
  public void onMinimizedStateChanged(boolean isMinimized) throws android.os.RemoteException;
  /**
       * Called when the set of actions for the current PiP activity changes, or when the listener
       * is first registered to allow the listener to synchronized its state with the controller.
       */
  public void onActionsChanged(android.content.pm.ParceledListSlice actions) throws android.os.RemoteException;
}
