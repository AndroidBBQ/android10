/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view;
/**
 * System private per-application interface to the window manager.
 *
 * {@hide}
 */
public interface IWindowSession extends android.os.IInterface
{
  /** Default implementation for IWindowSession. */
  public static class Default implements android.view.IWindowSession
  {
    @Override public int addToDisplay(android.view.IWindow window, int seq, android.view.WindowManager.LayoutParams attrs, int viewVisibility, int layerStackId, android.graphics.Rect outFrame, android.graphics.Rect outContentInsets, android.graphics.Rect outStableInsets, android.graphics.Rect outOutsets, android.view.DisplayCutout.ParcelableWrapper displayCutout, android.view.InputChannel outInputChannel, android.view.InsetsState insetsState) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int addToDisplayWithoutInputChannel(android.view.IWindow window, int seq, android.view.WindowManager.LayoutParams attrs, int viewVisibility, int layerStackId, android.graphics.Rect outContentInsets, android.graphics.Rect outStableInsets, android.view.InsetsState insetsState) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void remove(android.view.IWindow window) throws android.os.RemoteException
    {
    }
    /**
         * Change the parameters of a window.  You supply the
         * new parameters, it returns the new frame of the window on screen (the
         * position should be ignored) and surface of the window.  The surface
         * will be invalid if the window is currently hidden, else you can use it
         * to draw the window's contents.
         * 
         * @param window The window being modified.
         * @param seq Ordering sequence number.
         * @param attrs If non-null, new attributes to apply to the window.
         * @param requestedWidth The width the window wants to be.
         * @param requestedHeight The height the window wants to be.
         * @param viewVisibility Window root view's visibility.
         * @param flags Request flags: {@link WindowManagerGlobal#RELAYOUT_INSETS_PENDING},
         * {@link WindowManagerGlobal#RELAYOUT_DEFER_SURFACE_DESTROY}.
         * @param frameNumber A frame number in which changes requested in this layout will be rendered.
         * @param outFrame Rect in which is placed the new position/size on
         * screen.
         * @param outOverscanInsets Rect in which is placed the offsets from
         * <var>outFrame</var> in which the content of the window are inside
         * of the display's overlay region.
         * @param outContentInsets Rect in which is placed the offsets from
         * <var>outFrame</var> in which the content of the window should be
         * placed.  This can be used to modify the window layout to ensure its
         * contents are visible to the user, taking into account system windows
         * like the status bar or a soft keyboard.
         * @param outVisibleInsets Rect in which is placed the offsets from
         * <var>outFrame</var> in which the window is actually completely visible
         * to the user.  This can be used to temporarily scroll the window's
         * contents to make sure the user can see it.  This is different than
         * <var>outContentInsets</var> in that these insets change transiently,
         * so complex relayout of the window should not happen based on them.
         * @param outOutsets Rect in which is placed the dead area of the screen that we would like to
         * treat as real display. Example of such area is a chin in some models of wearable devices.
         * @param outBackdropFrame Rect which is used draw the resizing background during a resize
         * operation.
         * @param outMergedConfiguration New config container that holds global, override and merged
         * config for window, if it is now becoming visible and the merged configuration has changed
         * since it was last displayed.
         * @param outSurface Object in which is placed the new display surface.
         * @param insetsState The current insets state in the system.
         *
         * @return int Result flags: {@link WindowManagerGlobal#RELAYOUT_SHOW_FOCUS},
         * {@link WindowManagerGlobal#RELAYOUT_FIRST_TIME}.
         */
    @Override public int relayout(android.view.IWindow window, int seq, android.view.WindowManager.LayoutParams attrs, int requestedWidth, int requestedHeight, int viewVisibility, int flags, long frameNumber, android.graphics.Rect outFrame, android.graphics.Rect outOverscanInsets, android.graphics.Rect outContentInsets, android.graphics.Rect outVisibleInsets, android.graphics.Rect outStableInsets, android.graphics.Rect outOutsets, android.graphics.Rect outBackdropFrame, android.view.DisplayCutout.ParcelableWrapper displayCutout, android.util.MergedConfiguration outMergedConfiguration, android.view.SurfaceControl outSurfaceControl, android.view.InsetsState insetsState) throws android.os.RemoteException
    {
      return 0;
    }
    /*
         * Notify the window manager that an application is relaunching and
         * windows should be prepared for replacement.
         *
         * @param appToken The application
         * @param childrenOnly Whether to only prepare child windows for replacement
         * (for example when main windows are being reused via preservation).
         */
    @Override public void prepareToReplaceWindows(android.os.IBinder appToken, boolean childrenOnly) throws android.os.RemoteException
    {
    }
    /**
         * Called by a client to report that it ran out of graphics memory.
         */
    @Override public boolean outOfMemory(android.view.IWindow window) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Give the window manager a hint of the part of the window that is
         * completely transparent, allowing it to work with the surface flinger
         * to optimize compositing of this part of the window.
         */
    @Override public void setTransparentRegion(android.view.IWindow window, android.graphics.Region region) throws android.os.RemoteException
    {
    }
    /**
         * Tell the window manager about the content and visible insets of the
         * given window, which can be used to adjust the <var>outContentInsets</var>
         * and <var>outVisibleInsets</var> values returned by
         * {@link #relayout relayout()} for windows behind this one.
         *
         * @param touchableInsets Controls which part of the window inside of its
         * frame can receive pointer events, as defined by
         * {@link android.view.ViewTreeObserver.InternalInsetsInfo}.
         */
    @Override public void setInsets(android.view.IWindow window, int touchableInsets, android.graphics.Rect contentInsets, android.graphics.Rect visibleInsets, android.graphics.Region touchableRegion) throws android.os.RemoteException
    {
    }
    /**
         * Return the current display size in which the window is being laid out,
         * accounting for screen decorations around it.
         */
    @Override public void getDisplayFrame(android.view.IWindow window, android.graphics.Rect outDisplayFrame) throws android.os.RemoteException
    {
    }
    @Override public void finishDrawing(android.view.IWindow window) throws android.os.RemoteException
    {
    }
    @Override public void setInTouchMode(boolean showFocus) throws android.os.RemoteException
    {
    }
    @Override public boolean getInTouchMode() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean performHapticFeedback(int effectId, boolean always) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Initiate the drag operation itself
         *
         * @param window Window which initiates drag operation.
         * @param flags See {@code View#startDragAndDrop}
         * @param surface Surface containing drag shadow image
         * @param touchSource See {@code InputDevice#getSource()}
         * @param touchX X coordinate of last touch point
         * @param touchY Y coordinate of last touch point
         * @param thumbCenterX X coordinate for the position within the shadow image that should be
         *         underneath the touch point during the drag and drop operation.
         * @param thumbCenterY Y coordinate for the position within the shadow image that should be
         *         underneath the touch point during the drag and drop operation.
         * @param data Data transferred by drag and drop
         * @return Token of drag operation which will be passed to cancelDragAndDrop.
         */
    @Override public android.os.IBinder performDrag(android.view.IWindow window, int flags, android.view.SurfaceControl surface, int touchSource, float touchX, float touchY, float thumbCenterX, float thumbCenterY, android.content.ClipData data) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Report the result of a drop action targeted to the given window.
         * consumed is 'true' when the drop was accepted by a valid recipient,
         * 'false' otherwise.
         */
    @Override public void reportDropResult(android.view.IWindow window, boolean consumed) throws android.os.RemoteException
    {
    }
    /**
         * Cancel the current drag operation.
         * skipAnimation is 'true' when it should skip the drag cancel animation which brings the drag
         * shadow image back to the drag start position.
         */
    @Override public void cancelDragAndDrop(android.os.IBinder dragToken, boolean skipAnimation) throws android.os.RemoteException
    {
    }
    /**
         * Tell the OS that we've just dragged into a View that is willing to accept the drop
         */
    @Override public void dragRecipientEntered(android.view.IWindow window) throws android.os.RemoteException
    {
    }
    /**
         * Tell the OS that we've just dragged *off* of a View that was willing to accept the drop
         */
    @Override public void dragRecipientExited(android.view.IWindow window) throws android.os.RemoteException
    {
    }
    /**
         * For windows with the wallpaper behind them, and the wallpaper is
         * larger than the screen, set the offset within the screen.
         * For multi screen launcher type applications, xstep and ystep indicate
         * how big the increment is from one screen to another.
         */
    @Override public void setWallpaperPosition(android.os.IBinder windowToken, float x, float y, float xstep, float ystep) throws android.os.RemoteException
    {
    }
    @Override public void wallpaperOffsetsComplete(android.os.IBinder window) throws android.os.RemoteException
    {
    }
    /**
         * Apply a raw offset to the wallpaper service when shown behind this window.
         */
    @Override public void setWallpaperDisplayOffset(android.os.IBinder windowToken, int x, int y) throws android.os.RemoteException
    {
    }
    @Override public android.os.Bundle sendWallpaperCommand(android.os.IBinder window, java.lang.String action, int x, int y, int z, android.os.Bundle extras, boolean sync) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void wallpaperCommandComplete(android.os.IBinder window, android.os.Bundle result) throws android.os.RemoteException
    {
    }
    /**
         * Notifies that a rectangle on the screen has been requested.
         */
    @Override public void onRectangleOnScreenRequested(android.os.IBinder token, android.graphics.Rect rectangle) throws android.os.RemoteException
    {
    }
    @Override public android.view.IWindowId getWindowId(android.os.IBinder window) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * When the system is dozing in a low-power partially suspended state, pokes a short
         * lived wake lock and ensures that the display is ready to accept the next frame
         * of content drawn in the window.
         *
         * This mechanism is bound to the window rather than to the display manager or the
         * power manager so that the system can ensure that the window is actually visible
         * and prevent runaway applications from draining the battery.  This is similar to how
         * FLAG_KEEP_SCREEN_ON works.
         *
         * This method is synchronous because it may need to acquire a wake lock before returning.
         * The assumption is that this method will be called rather infrequently.
         */
    @Override public void pokeDrawLock(android.os.IBinder window) throws android.os.RemoteException
    {
    }
    /**
         * Starts a task window move with {startX, startY} as starting point. The amount of move
         * will be the offset between {startX, startY} and the new cursor position.
         *
         * Returns true if the move started successfully; false otherwise.
         */
    @Override public boolean startMovingTask(android.view.IWindow window, float startX, float startY) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void finishMovingTask(android.view.IWindow window) throws android.os.RemoteException
    {
    }
    @Override public void updatePointerIcon(android.view.IWindow window) throws android.os.RemoteException
    {
    }
    /**
         * Reparent the top layers for a display to the requested SurfaceControl. The display that is
         * going to be re-parented (the displayId passed in) needs to have been created by the same
         * process that is requesting the re-parent. This is to ensure clients can't just re-parent
         * display content info to any SurfaceControl, as this would be a security issue.
         *
         * @param window The window which owns the SurfaceControl. This indicates the z-order of the
         *               windows of this display against the windows on the parent display.
         * @param sc The SurfaceControl that the top level layers for the display should be re-parented
         *           to.
         * @param displayId The id of the display to be re-parented.
         */
    @Override public void reparentDisplayContent(android.view.IWindow window, android.view.SurfaceControl sc, int displayId) throws android.os.RemoteException
    {
    }
    /**
         * Update the location of a child display in its parent window. This enables windows in the
         * child display to compute the global transformation matrix.
         *
         * @param window The parent window of the display.
         * @param x The x coordinate in the parent window.
         * @param y The y coordinate in the parent window.
         * @param displayId The id of the display to be notified.
         */
    @Override public void updateDisplayContentLocation(android.view.IWindow window, int x, int y, int displayId) throws android.os.RemoteException
    {
    }
    /**
         * Update a tap exclude region identified by provided id in the window. Touches on this region
         * will neither be dispatched to this window nor change the focus to this window. Passing an
         * invalid region will remove the area from the exclude region of this window.
         */
    @Override public void updateTapExcludeRegion(android.view.IWindow window, int regionId, android.graphics.Region region) throws android.os.RemoteException
    {
    }
    /**
         * Called when the client has changed the local insets state, and now the server should reflect
         * that new state.
         */
    @Override public void insetsModified(android.view.IWindow window, android.view.InsetsState state) throws android.os.RemoteException
    {
    }
    /**
         * Called when the system gesture exclusion has changed.
         */
    @Override public void reportSystemGestureExclusionChanged(android.view.IWindow window, java.util.List<android.graphics.Rect> exclusionRects) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.IWindowSession
  {
    private static final java.lang.String DESCRIPTOR = "android.view.IWindowSession";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.IWindowSession interface,
     * generating a proxy if needed.
     */
    public static android.view.IWindowSession asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.IWindowSession))) {
        return ((android.view.IWindowSession)iin);
      }
      return new android.view.IWindowSession.Stub.Proxy(obj);
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
        case TRANSACTION_addToDisplay:
        {
          data.enforceInterface(descriptor);
          android.view.IWindow _arg0;
          _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          android.view.WindowManager.LayoutParams _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.view.WindowManager.LayoutParams.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          android.graphics.Rect _arg5;
          _arg5 = new android.graphics.Rect();
          android.graphics.Rect _arg6;
          _arg6 = new android.graphics.Rect();
          android.graphics.Rect _arg7;
          _arg7 = new android.graphics.Rect();
          android.graphics.Rect _arg8;
          _arg8 = new android.graphics.Rect();
          android.view.DisplayCutout.ParcelableWrapper _arg9;
          _arg9 = new android.view.DisplayCutout.ParcelableWrapper();
          android.view.InputChannel _arg10;
          _arg10 = new android.view.InputChannel();
          android.view.InsetsState _arg11;
          _arg11 = new android.view.InsetsState();
          int _result = this.addToDisplay(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9, _arg10, _arg11);
          reply.writeNoException();
          reply.writeInt(_result);
          if ((_arg5!=null)) {
            reply.writeInt(1);
            _arg5.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          if ((_arg6!=null)) {
            reply.writeInt(1);
            _arg6.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          if ((_arg7!=null)) {
            reply.writeInt(1);
            _arg7.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          if ((_arg8!=null)) {
            reply.writeInt(1);
            _arg8.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          if ((_arg9!=null)) {
            reply.writeInt(1);
            _arg9.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          if ((_arg10!=null)) {
            reply.writeInt(1);
            _arg10.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          if ((_arg11!=null)) {
            reply.writeInt(1);
            _arg11.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_addToDisplayWithoutInputChannel:
        {
          data.enforceInterface(descriptor);
          android.view.IWindow _arg0;
          _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          android.view.WindowManager.LayoutParams _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.view.WindowManager.LayoutParams.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          android.graphics.Rect _arg5;
          _arg5 = new android.graphics.Rect();
          android.graphics.Rect _arg6;
          _arg6 = new android.graphics.Rect();
          android.view.InsetsState _arg7;
          _arg7 = new android.view.InsetsState();
          int _result = this.addToDisplayWithoutInputChannel(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7);
          reply.writeNoException();
          reply.writeInt(_result);
          if ((_arg5!=null)) {
            reply.writeInt(1);
            _arg5.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          if ((_arg6!=null)) {
            reply.writeInt(1);
            _arg6.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          if ((_arg7!=null)) {
            reply.writeInt(1);
            _arg7.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_remove:
        {
          data.enforceInterface(descriptor);
          android.view.IWindow _arg0;
          _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
          this.remove(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_relayout:
        {
          data.enforceInterface(descriptor);
          android.view.IWindow _arg0;
          _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          android.view.WindowManager.LayoutParams _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.view.WindowManager.LayoutParams.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          int _arg5;
          _arg5 = data.readInt();
          int _arg6;
          _arg6 = data.readInt();
          long _arg7;
          _arg7 = data.readLong();
          android.graphics.Rect _arg8;
          _arg8 = new android.graphics.Rect();
          android.graphics.Rect _arg9;
          _arg9 = new android.graphics.Rect();
          android.graphics.Rect _arg10;
          _arg10 = new android.graphics.Rect();
          android.graphics.Rect _arg11;
          _arg11 = new android.graphics.Rect();
          android.graphics.Rect _arg12;
          _arg12 = new android.graphics.Rect();
          android.graphics.Rect _arg13;
          _arg13 = new android.graphics.Rect();
          android.graphics.Rect _arg14;
          _arg14 = new android.graphics.Rect();
          android.view.DisplayCutout.ParcelableWrapper _arg15;
          _arg15 = new android.view.DisplayCutout.ParcelableWrapper();
          android.util.MergedConfiguration _arg16;
          _arg16 = new android.util.MergedConfiguration();
          android.view.SurfaceControl _arg17;
          _arg17 = new android.view.SurfaceControl();
          android.view.InsetsState _arg18;
          _arg18 = new android.view.InsetsState();
          int _result = this.relayout(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9, _arg10, _arg11, _arg12, _arg13, _arg14, _arg15, _arg16, _arg17, _arg18);
          reply.writeNoException();
          reply.writeInt(_result);
          if ((_arg8!=null)) {
            reply.writeInt(1);
            _arg8.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          if ((_arg9!=null)) {
            reply.writeInt(1);
            _arg9.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          if ((_arg10!=null)) {
            reply.writeInt(1);
            _arg10.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          if ((_arg11!=null)) {
            reply.writeInt(1);
            _arg11.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          if ((_arg12!=null)) {
            reply.writeInt(1);
            _arg12.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          if ((_arg13!=null)) {
            reply.writeInt(1);
            _arg13.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          if ((_arg14!=null)) {
            reply.writeInt(1);
            _arg14.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          if ((_arg15!=null)) {
            reply.writeInt(1);
            _arg15.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          if ((_arg16!=null)) {
            reply.writeInt(1);
            _arg16.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          if ((_arg17!=null)) {
            reply.writeInt(1);
            _arg17.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          if ((_arg18!=null)) {
            reply.writeInt(1);
            _arg18.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_prepareToReplaceWindows:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.prepareToReplaceWindows(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_outOfMemory:
        {
          data.enforceInterface(descriptor);
          android.view.IWindow _arg0;
          _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.outOfMemory(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setTransparentRegion:
        {
          data.enforceInterface(descriptor);
          android.view.IWindow _arg0;
          _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
          android.graphics.Region _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.graphics.Region.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.setTransparentRegion(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setInsets:
        {
          data.enforceInterface(descriptor);
          android.view.IWindow _arg0;
          _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
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
          android.graphics.Region _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.graphics.Region.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          this.setInsets(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getDisplayFrame:
        {
          data.enforceInterface(descriptor);
          android.view.IWindow _arg0;
          _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
          android.graphics.Rect _arg1;
          _arg1 = new android.graphics.Rect();
          this.getDisplayFrame(_arg0, _arg1);
          reply.writeNoException();
          if ((_arg1!=null)) {
            reply.writeInt(1);
            _arg1.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_finishDrawing:
        {
          data.enforceInterface(descriptor);
          android.view.IWindow _arg0;
          _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
          this.finishDrawing(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setInTouchMode:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setInTouchMode(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getInTouchMode:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.getInTouchMode();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_performHapticFeedback:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          boolean _result = this.performHapticFeedback(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_performDrag:
        {
          data.enforceInterface(descriptor);
          android.view.IWindow _arg0;
          _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          android.view.SurfaceControl _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.view.SurfaceControl.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          float _arg4;
          _arg4 = data.readFloat();
          float _arg5;
          _arg5 = data.readFloat();
          float _arg6;
          _arg6 = data.readFloat();
          float _arg7;
          _arg7 = data.readFloat();
          android.content.ClipData _arg8;
          if ((0!=data.readInt())) {
            _arg8 = android.content.ClipData.CREATOR.createFromParcel(data);
          }
          else {
            _arg8 = null;
          }
          android.os.IBinder _result = this.performDrag(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8);
          reply.writeNoException();
          reply.writeStrongBinder(_result);
          return true;
        }
        case TRANSACTION_reportDropResult:
        {
          data.enforceInterface(descriptor);
          android.view.IWindow _arg0;
          _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.reportDropResult(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_cancelDragAndDrop:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.cancelDragAndDrop(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_dragRecipientEntered:
        {
          data.enforceInterface(descriptor);
          android.view.IWindow _arg0;
          _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
          this.dragRecipientEntered(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_dragRecipientExited:
        {
          data.enforceInterface(descriptor);
          android.view.IWindow _arg0;
          _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
          this.dragRecipientExited(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setWallpaperPosition:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          float _arg1;
          _arg1 = data.readFloat();
          float _arg2;
          _arg2 = data.readFloat();
          float _arg3;
          _arg3 = data.readFloat();
          float _arg4;
          _arg4 = data.readFloat();
          this.setWallpaperPosition(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_wallpaperOffsetsComplete:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.wallpaperOffsetsComplete(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setWallpaperDisplayOffset:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.setWallpaperDisplayOffset(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_sendWallpaperCommand:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          android.os.Bundle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          boolean _arg6;
          _arg6 = (0!=data.readInt());
          android.os.Bundle _result = this.sendWallpaperCommand(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
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
        case TRANSACTION_wallpaperCommandComplete:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.wallpaperCommandComplete(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onRectangleOnScreenRequested:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.graphics.Rect _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.onRectangleOnScreenRequested(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getWindowId:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.view.IWindowId _result = this.getWindowId(_arg0);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_pokeDrawLock:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          this.pokeDrawLock(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startMovingTask:
        {
          data.enforceInterface(descriptor);
          android.view.IWindow _arg0;
          _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
          float _arg1;
          _arg1 = data.readFloat();
          float _arg2;
          _arg2 = data.readFloat();
          boolean _result = this.startMovingTask(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_finishMovingTask:
        {
          data.enforceInterface(descriptor);
          android.view.IWindow _arg0;
          _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
          this.finishMovingTask(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_updatePointerIcon:
        {
          data.enforceInterface(descriptor);
          android.view.IWindow _arg0;
          _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
          this.updatePointerIcon(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_reparentDisplayContent:
        {
          data.enforceInterface(descriptor);
          android.view.IWindow _arg0;
          _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
          android.view.SurfaceControl _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.view.SurfaceControl.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          this.reparentDisplayContent(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_updateDisplayContentLocation:
        {
          data.enforceInterface(descriptor);
          android.view.IWindow _arg0;
          _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          this.updateDisplayContentLocation(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_updateTapExcludeRegion:
        {
          data.enforceInterface(descriptor);
          android.view.IWindow _arg0;
          _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          android.graphics.Region _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.graphics.Region.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.updateTapExcludeRegion(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_insetsModified:
        {
          data.enforceInterface(descriptor);
          android.view.IWindow _arg0;
          _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
          android.view.InsetsState _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.view.InsetsState.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.insetsModified(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_reportSystemGestureExclusionChanged:
        {
          data.enforceInterface(descriptor);
          android.view.IWindow _arg0;
          _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
          java.util.List<android.graphics.Rect> _arg1;
          _arg1 = data.createTypedArrayList(android.graphics.Rect.CREATOR);
          this.reportSystemGestureExclusionChanged(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.IWindowSession
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
      @Override public int addToDisplay(android.view.IWindow window, int seq, android.view.WindowManager.LayoutParams attrs, int viewVisibility, int layerStackId, android.graphics.Rect outFrame, android.graphics.Rect outContentInsets, android.graphics.Rect outStableInsets, android.graphics.Rect outOutsets, android.view.DisplayCutout.ParcelableWrapper displayCutout, android.view.InputChannel outInputChannel, android.view.InsetsState insetsState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((window!=null))?(window.asBinder()):(null)));
          _data.writeInt(seq);
          if ((attrs!=null)) {
            _data.writeInt(1);
            attrs.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(viewVisibility);
          _data.writeInt(layerStackId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addToDisplay, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addToDisplay(window, seq, attrs, viewVisibility, layerStackId, outFrame, outContentInsets, outStableInsets, outOutsets, displayCutout, outInputChannel, insetsState);
          }
          _reply.readException();
          _result = _reply.readInt();
          if ((0!=_reply.readInt())) {
            outFrame.readFromParcel(_reply);
          }
          if ((0!=_reply.readInt())) {
            outContentInsets.readFromParcel(_reply);
          }
          if ((0!=_reply.readInt())) {
            outStableInsets.readFromParcel(_reply);
          }
          if ((0!=_reply.readInt())) {
            outOutsets.readFromParcel(_reply);
          }
          if ((0!=_reply.readInt())) {
            displayCutout.readFromParcel(_reply);
          }
          if ((0!=_reply.readInt())) {
            outInputChannel.readFromParcel(_reply);
          }
          if ((0!=_reply.readInt())) {
            insetsState.readFromParcel(_reply);
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int addToDisplayWithoutInputChannel(android.view.IWindow window, int seq, android.view.WindowManager.LayoutParams attrs, int viewVisibility, int layerStackId, android.graphics.Rect outContentInsets, android.graphics.Rect outStableInsets, android.view.InsetsState insetsState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((window!=null))?(window.asBinder()):(null)));
          _data.writeInt(seq);
          if ((attrs!=null)) {
            _data.writeInt(1);
            attrs.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(viewVisibility);
          _data.writeInt(layerStackId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addToDisplayWithoutInputChannel, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addToDisplayWithoutInputChannel(window, seq, attrs, viewVisibility, layerStackId, outContentInsets, outStableInsets, insetsState);
          }
          _reply.readException();
          _result = _reply.readInt();
          if ((0!=_reply.readInt())) {
            outContentInsets.readFromParcel(_reply);
          }
          if ((0!=_reply.readInt())) {
            outStableInsets.readFromParcel(_reply);
          }
          if ((0!=_reply.readInt())) {
            insetsState.readFromParcel(_reply);
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void remove(android.view.IWindow window) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((window!=null))?(window.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_remove, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().remove(window);
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
           * Change the parameters of a window.  You supply the
           * new parameters, it returns the new frame of the window on screen (the
           * position should be ignored) and surface of the window.  The surface
           * will be invalid if the window is currently hidden, else you can use it
           * to draw the window's contents.
           * 
           * @param window The window being modified.
           * @param seq Ordering sequence number.
           * @param attrs If non-null, new attributes to apply to the window.
           * @param requestedWidth The width the window wants to be.
           * @param requestedHeight The height the window wants to be.
           * @param viewVisibility Window root view's visibility.
           * @param flags Request flags: {@link WindowManagerGlobal#RELAYOUT_INSETS_PENDING},
           * {@link WindowManagerGlobal#RELAYOUT_DEFER_SURFACE_DESTROY}.
           * @param frameNumber A frame number in which changes requested in this layout will be rendered.
           * @param outFrame Rect in which is placed the new position/size on
           * screen.
           * @param outOverscanInsets Rect in which is placed the offsets from
           * <var>outFrame</var> in which the content of the window are inside
           * of the display's overlay region.
           * @param outContentInsets Rect in which is placed the offsets from
           * <var>outFrame</var> in which the content of the window should be
           * placed.  This can be used to modify the window layout to ensure its
           * contents are visible to the user, taking into account system windows
           * like the status bar or a soft keyboard.
           * @param outVisibleInsets Rect in which is placed the offsets from
           * <var>outFrame</var> in which the window is actually completely visible
           * to the user.  This can be used to temporarily scroll the window's
           * contents to make sure the user can see it.  This is different than
           * <var>outContentInsets</var> in that these insets change transiently,
           * so complex relayout of the window should not happen based on them.
           * @param outOutsets Rect in which is placed the dead area of the screen that we would like to
           * treat as real display. Example of such area is a chin in some models of wearable devices.
           * @param outBackdropFrame Rect which is used draw the resizing background during a resize
           * operation.
           * @param outMergedConfiguration New config container that holds global, override and merged
           * config for window, if it is now becoming visible and the merged configuration has changed
           * since it was last displayed.
           * @param outSurface Object in which is placed the new display surface.
           * @param insetsState The current insets state in the system.
           *
           * @return int Result flags: {@link WindowManagerGlobal#RELAYOUT_SHOW_FOCUS},
           * {@link WindowManagerGlobal#RELAYOUT_FIRST_TIME}.
           */
      @Override public int relayout(android.view.IWindow window, int seq, android.view.WindowManager.LayoutParams attrs, int requestedWidth, int requestedHeight, int viewVisibility, int flags, long frameNumber, android.graphics.Rect outFrame, android.graphics.Rect outOverscanInsets, android.graphics.Rect outContentInsets, android.graphics.Rect outVisibleInsets, android.graphics.Rect outStableInsets, android.graphics.Rect outOutsets, android.graphics.Rect outBackdropFrame, android.view.DisplayCutout.ParcelableWrapper displayCutout, android.util.MergedConfiguration outMergedConfiguration, android.view.SurfaceControl outSurfaceControl, android.view.InsetsState insetsState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((window!=null))?(window.asBinder()):(null)));
          _data.writeInt(seq);
          if ((attrs!=null)) {
            _data.writeInt(1);
            attrs.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(requestedWidth);
          _data.writeInt(requestedHeight);
          _data.writeInt(viewVisibility);
          _data.writeInt(flags);
          _data.writeLong(frameNumber);
          boolean _status = mRemote.transact(Stub.TRANSACTION_relayout, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().relayout(window, seq, attrs, requestedWidth, requestedHeight, viewVisibility, flags, frameNumber, outFrame, outOverscanInsets, outContentInsets, outVisibleInsets, outStableInsets, outOutsets, outBackdropFrame, displayCutout, outMergedConfiguration, outSurfaceControl, insetsState);
          }
          _reply.readException();
          _result = _reply.readInt();
          if ((0!=_reply.readInt())) {
            outFrame.readFromParcel(_reply);
          }
          if ((0!=_reply.readInt())) {
            outOverscanInsets.readFromParcel(_reply);
          }
          if ((0!=_reply.readInt())) {
            outContentInsets.readFromParcel(_reply);
          }
          if ((0!=_reply.readInt())) {
            outVisibleInsets.readFromParcel(_reply);
          }
          if ((0!=_reply.readInt())) {
            outStableInsets.readFromParcel(_reply);
          }
          if ((0!=_reply.readInt())) {
            outOutsets.readFromParcel(_reply);
          }
          if ((0!=_reply.readInt())) {
            outBackdropFrame.readFromParcel(_reply);
          }
          if ((0!=_reply.readInt())) {
            displayCutout.readFromParcel(_reply);
          }
          if ((0!=_reply.readInt())) {
            outMergedConfiguration.readFromParcel(_reply);
          }
          if ((0!=_reply.readInt())) {
            outSurfaceControl.readFromParcel(_reply);
          }
          if ((0!=_reply.readInt())) {
            insetsState.readFromParcel(_reply);
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /*
           * Notify the window manager that an application is relaunching and
           * windows should be prepared for replacement.
           *
           * @param appToken The application
           * @param childrenOnly Whether to only prepare child windows for replacement
           * (for example when main windows are being reused via preservation).
           */
      @Override public void prepareToReplaceWindows(android.os.IBinder appToken, boolean childrenOnly) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(appToken);
          _data.writeInt(((childrenOnly)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_prepareToReplaceWindows, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().prepareToReplaceWindows(appToken, childrenOnly);
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
           * Called by a client to report that it ran out of graphics memory.
           */
      @Override public boolean outOfMemory(android.view.IWindow window) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((window!=null))?(window.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_outOfMemory, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().outOfMemory(window);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Give the window manager a hint of the part of the window that is
           * completely transparent, allowing it to work with the surface flinger
           * to optimize compositing of this part of the window.
           */
      @Override public void setTransparentRegion(android.view.IWindow window, android.graphics.Region region) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((window!=null))?(window.asBinder()):(null)));
          if ((region!=null)) {
            _data.writeInt(1);
            region.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTransparentRegion, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setTransparentRegion(window, region);
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
           * Tell the window manager about the content and visible insets of the
           * given window, which can be used to adjust the <var>outContentInsets</var>
           * and <var>outVisibleInsets</var> values returned by
           * {@link #relayout relayout()} for windows behind this one.
           *
           * @param touchableInsets Controls which part of the window inside of its
           * frame can receive pointer events, as defined by
           * {@link android.view.ViewTreeObserver.InternalInsetsInfo}.
           */
      @Override public void setInsets(android.view.IWindow window, int touchableInsets, android.graphics.Rect contentInsets, android.graphics.Rect visibleInsets, android.graphics.Region touchableRegion) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((window!=null))?(window.asBinder()):(null)));
          _data.writeInt(touchableInsets);
          if ((contentInsets!=null)) {
            _data.writeInt(1);
            contentInsets.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((visibleInsets!=null)) {
            _data.writeInt(1);
            visibleInsets.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((touchableRegion!=null)) {
            _data.writeInt(1);
            touchableRegion.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setInsets, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setInsets(window, touchableInsets, contentInsets, visibleInsets, touchableRegion);
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
           * Return the current display size in which the window is being laid out,
           * accounting for screen decorations around it.
           */
      @Override public void getDisplayFrame(android.view.IWindow window, android.graphics.Rect outDisplayFrame) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((window!=null))?(window.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDisplayFrame, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getDisplayFrame(window, outDisplayFrame);
            return;
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            outDisplayFrame.readFromParcel(_reply);
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void finishDrawing(android.view.IWindow window) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((window!=null))?(window.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_finishDrawing, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().finishDrawing(window);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setInTouchMode(boolean showFocus) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((showFocus)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setInTouchMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setInTouchMode(showFocus);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean getInTouchMode() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getInTouchMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getInTouchMode();
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public boolean performHapticFeedback(int effectId, boolean always) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(effectId);
          _data.writeInt(((always)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_performHapticFeedback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().performHapticFeedback(effectId, always);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Initiate the drag operation itself
           *
           * @param window Window which initiates drag operation.
           * @param flags See {@code View#startDragAndDrop}
           * @param surface Surface containing drag shadow image
           * @param touchSource See {@code InputDevice#getSource()}
           * @param touchX X coordinate of last touch point
           * @param touchY Y coordinate of last touch point
           * @param thumbCenterX X coordinate for the position within the shadow image that should be
           *         underneath the touch point during the drag and drop operation.
           * @param thumbCenterY Y coordinate for the position within the shadow image that should be
           *         underneath the touch point during the drag and drop operation.
           * @param data Data transferred by drag and drop
           * @return Token of drag operation which will be passed to cancelDragAndDrop.
           */
      @Override public android.os.IBinder performDrag(android.view.IWindow window, int flags, android.view.SurfaceControl surface, int touchSource, float touchX, float touchY, float thumbCenterX, float thumbCenterY, android.content.ClipData data) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.IBinder _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((window!=null))?(window.asBinder()):(null)));
          _data.writeInt(flags);
          if ((surface!=null)) {
            _data.writeInt(1);
            surface.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(touchSource);
          _data.writeFloat(touchX);
          _data.writeFloat(touchY);
          _data.writeFloat(thumbCenterX);
          _data.writeFloat(thumbCenterY);
          if ((data!=null)) {
            _data.writeInt(1);
            data.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_performDrag, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().performDrag(window, flags, surface, touchSource, touchX, touchY, thumbCenterX, thumbCenterY, data);
          }
          _reply.readException();
          _result = _reply.readStrongBinder();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Report the result of a drop action targeted to the given window.
           * consumed is 'true' when the drop was accepted by a valid recipient,
           * 'false' otherwise.
           */
      @Override public void reportDropResult(android.view.IWindow window, boolean consumed) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((window!=null))?(window.asBinder()):(null)));
          _data.writeInt(((consumed)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportDropResult, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportDropResult(window, consumed);
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
           * Cancel the current drag operation.
           * skipAnimation is 'true' when it should skip the drag cancel animation which brings the drag
           * shadow image back to the drag start position.
           */
      @Override public void cancelDragAndDrop(android.os.IBinder dragToken, boolean skipAnimation) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(dragToken);
          _data.writeInt(((skipAnimation)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelDragAndDrop, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelDragAndDrop(dragToken, skipAnimation);
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
           * Tell the OS that we've just dragged into a View that is willing to accept the drop
           */
      @Override public void dragRecipientEntered(android.view.IWindow window) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((window!=null))?(window.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_dragRecipientEntered, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dragRecipientEntered(window);
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
           * Tell the OS that we've just dragged *off* of a View that was willing to accept the drop
           */
      @Override public void dragRecipientExited(android.view.IWindow window) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((window!=null))?(window.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_dragRecipientExited, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dragRecipientExited(window);
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
           * For windows with the wallpaper behind them, and the wallpaper is
           * larger than the screen, set the offset within the screen.
           * For multi screen launcher type applications, xstep and ystep indicate
           * how big the increment is from one screen to another.
           */
      @Override public void setWallpaperPosition(android.os.IBinder windowToken, float x, float y, float xstep, float ystep) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(windowToken);
          _data.writeFloat(x);
          _data.writeFloat(y);
          _data.writeFloat(xstep);
          _data.writeFloat(ystep);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setWallpaperPosition, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setWallpaperPosition(windowToken, x, y, xstep, ystep);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void wallpaperOffsetsComplete(android.os.IBinder window) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(window);
          boolean _status = mRemote.transact(Stub.TRANSACTION_wallpaperOffsetsComplete, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().wallpaperOffsetsComplete(window);
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
           * Apply a raw offset to the wallpaper service when shown behind this window.
           */
      @Override public void setWallpaperDisplayOffset(android.os.IBinder windowToken, int x, int y) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(windowToken);
          _data.writeInt(x);
          _data.writeInt(y);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setWallpaperDisplayOffset, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setWallpaperDisplayOffset(windowToken, x, y);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.os.Bundle sendWallpaperCommand(android.os.IBinder window, java.lang.String action, int x, int y, int z, android.os.Bundle extras, boolean sync) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.Bundle _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(window);
          _data.writeString(action);
          _data.writeInt(x);
          _data.writeInt(y);
          _data.writeInt(z);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((sync)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendWallpaperCommand, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().sendWallpaperCommand(window, action, x, y, z, extras, sync);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.Bundle.CREATOR.createFromParcel(_reply);
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
      @Override public void wallpaperCommandComplete(android.os.IBinder window, android.os.Bundle result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(window);
          if ((result!=null)) {
            _data.writeInt(1);
            result.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_wallpaperCommandComplete, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().wallpaperCommandComplete(window, result);
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
           * Notifies that a rectangle on the screen has been requested.
           */
      @Override public void onRectangleOnScreenRequested(android.os.IBinder token, android.graphics.Rect rectangle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          if ((rectangle!=null)) {
            _data.writeInt(1);
            rectangle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRectangleOnScreenRequested, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRectangleOnScreenRequested(token, rectangle);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.view.IWindowId getWindowId(android.os.IBinder window) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.view.IWindowId _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(window);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getWindowId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getWindowId(window);
          }
          _reply.readException();
          _result = android.view.IWindowId.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * When the system is dozing in a low-power partially suspended state, pokes a short
           * lived wake lock and ensures that the display is ready to accept the next frame
           * of content drawn in the window.
           *
           * This mechanism is bound to the window rather than to the display manager or the
           * power manager so that the system can ensure that the window is actually visible
           * and prevent runaway applications from draining the battery.  This is similar to how
           * FLAG_KEEP_SCREEN_ON works.
           *
           * This method is synchronous because it may need to acquire a wake lock before returning.
           * The assumption is that this method will be called rather infrequently.
           */
      @Override public void pokeDrawLock(android.os.IBinder window) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(window);
          boolean _status = mRemote.transact(Stub.TRANSACTION_pokeDrawLock, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().pokeDrawLock(window);
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
           * Starts a task window move with {startX, startY} as starting point. The amount of move
           * will be the offset between {startX, startY} and the new cursor position.
           *
           * Returns true if the move started successfully; false otherwise.
           */
      @Override public boolean startMovingTask(android.view.IWindow window, float startX, float startY) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((window!=null))?(window.asBinder()):(null)));
          _data.writeFloat(startX);
          _data.writeFloat(startY);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startMovingTask, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startMovingTask(window, startX, startY);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void finishMovingTask(android.view.IWindow window) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((window!=null))?(window.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_finishMovingTask, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().finishMovingTask(window);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void updatePointerIcon(android.view.IWindow window) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((window!=null))?(window.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_updatePointerIcon, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updatePointerIcon(window);
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
           * Reparent the top layers for a display to the requested SurfaceControl. The display that is
           * going to be re-parented (the displayId passed in) needs to have been created by the same
           * process that is requesting the re-parent. This is to ensure clients can't just re-parent
           * display content info to any SurfaceControl, as this would be a security issue.
           *
           * @param window The window which owns the SurfaceControl. This indicates the z-order of the
           *               windows of this display against the windows on the parent display.
           * @param sc The SurfaceControl that the top level layers for the display should be re-parented
           *           to.
           * @param displayId The id of the display to be re-parented.
           */
      @Override public void reparentDisplayContent(android.view.IWindow window, android.view.SurfaceControl sc, int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((window!=null))?(window.asBinder()):(null)));
          if ((sc!=null)) {
            _data.writeInt(1);
            sc.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reparentDisplayContent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reparentDisplayContent(window, sc, displayId);
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
           * Update the location of a child display in its parent window. This enables windows in the
           * child display to compute the global transformation matrix.
           *
           * @param window The parent window of the display.
           * @param x The x coordinate in the parent window.
           * @param y The y coordinate in the parent window.
           * @param displayId The id of the display to be notified.
           */
      @Override public void updateDisplayContentLocation(android.view.IWindow window, int x, int y, int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((window!=null))?(window.asBinder()):(null)));
          _data.writeInt(x);
          _data.writeInt(y);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateDisplayContentLocation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateDisplayContentLocation(window, x, y, displayId);
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
           * Update a tap exclude region identified by provided id in the window. Touches on this region
           * will neither be dispatched to this window nor change the focus to this window. Passing an
           * invalid region will remove the area from the exclude region of this window.
           */
      @Override public void updateTapExcludeRegion(android.view.IWindow window, int regionId, android.graphics.Region region) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((window!=null))?(window.asBinder()):(null)));
          _data.writeInt(regionId);
          if ((region!=null)) {
            _data.writeInt(1);
            region.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateTapExcludeRegion, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateTapExcludeRegion(window, regionId, region);
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
           * Called when the client has changed the local insets state, and now the server should reflect
           * that new state.
           */
      @Override public void insetsModified(android.view.IWindow window, android.view.InsetsState state) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((window!=null))?(window.asBinder()):(null)));
          if ((state!=null)) {
            _data.writeInt(1);
            state.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_insetsModified, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().insetsModified(window, state);
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
           * Called when the system gesture exclusion has changed.
           */
      @Override public void reportSystemGestureExclusionChanged(android.view.IWindow window, java.util.List<android.graphics.Rect> exclusionRects) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((window!=null))?(window.asBinder()):(null)));
          _data.writeTypedList(exclusionRects);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportSystemGestureExclusionChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportSystemGestureExclusionChanged(window, exclusionRects);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.view.IWindowSession sDefaultImpl;
    }
    static final int TRANSACTION_addToDisplay = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_addToDisplayWithoutInputChannel = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_remove = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_relayout = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_prepareToReplaceWindows = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_outOfMemory = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_setTransparentRegion = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_setInsets = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_getDisplayFrame = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_finishDrawing = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_setInTouchMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_getInTouchMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_performHapticFeedback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_performDrag = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_reportDropResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_cancelDragAndDrop = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_dragRecipientEntered = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_dragRecipientExited = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_setWallpaperPosition = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_wallpaperOffsetsComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_setWallpaperDisplayOffset = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_sendWallpaperCommand = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_wallpaperCommandComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_onRectangleOnScreenRequested = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_getWindowId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_pokeDrawLock = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_startMovingTask = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_finishMovingTask = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_updatePointerIcon = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_reparentDisplayContent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_updateDisplayContentLocation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_updateTapExcludeRegion = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_insetsModified = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_reportSystemGestureExclusionChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    public static boolean setDefaultImpl(android.view.IWindowSession impl) {
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
    public static android.view.IWindowSession getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public int addToDisplay(android.view.IWindow window, int seq, android.view.WindowManager.LayoutParams attrs, int viewVisibility, int layerStackId, android.graphics.Rect outFrame, android.graphics.Rect outContentInsets, android.graphics.Rect outStableInsets, android.graphics.Rect outOutsets, android.view.DisplayCutout.ParcelableWrapper displayCutout, android.view.InputChannel outInputChannel, android.view.InsetsState insetsState) throws android.os.RemoteException;
  public int addToDisplayWithoutInputChannel(android.view.IWindow window, int seq, android.view.WindowManager.LayoutParams attrs, int viewVisibility, int layerStackId, android.graphics.Rect outContentInsets, android.graphics.Rect outStableInsets, android.view.InsetsState insetsState) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowSession.aidl:51:1:51:25")
  public void remove(android.view.IWindow window) throws android.os.RemoteException;
  /**
       * Change the parameters of a window.  You supply the
       * new parameters, it returns the new frame of the window on screen (the
       * position should be ignored) and surface of the window.  The surface
       * will be invalid if the window is currently hidden, else you can use it
       * to draw the window's contents.
       * 
       * @param window The window being modified.
       * @param seq Ordering sequence number.
       * @param attrs If non-null, new attributes to apply to the window.
       * @param requestedWidth The width the window wants to be.
       * @param requestedHeight The height the window wants to be.
       * @param viewVisibility Window root view's visibility.
       * @param flags Request flags: {@link WindowManagerGlobal#RELAYOUT_INSETS_PENDING},
       * {@link WindowManagerGlobal#RELAYOUT_DEFER_SURFACE_DESTROY}.
       * @param frameNumber A frame number in which changes requested in this layout will be rendered.
       * @param outFrame Rect in which is placed the new position/size on
       * screen.
       * @param outOverscanInsets Rect in which is placed the offsets from
       * <var>outFrame</var> in which the content of the window are inside
       * of the display's overlay region.
       * @param outContentInsets Rect in which is placed the offsets from
       * <var>outFrame</var> in which the content of the window should be
       * placed.  This can be used to modify the window layout to ensure its
       * contents are visible to the user, taking into account system windows
       * like the status bar or a soft keyboard.
       * @param outVisibleInsets Rect in which is placed the offsets from
       * <var>outFrame</var> in which the window is actually completely visible
       * to the user.  This can be used to temporarily scroll the window's
       * contents to make sure the user can see it.  This is different than
       * <var>outContentInsets</var> in that these insets change transiently,
       * so complex relayout of the window should not happen based on them.
       * @param outOutsets Rect in which is placed the dead area of the screen that we would like to
       * treat as real display. Example of such area is a chin in some models of wearable devices.
       * @param outBackdropFrame Rect which is used draw the resizing background during a resize
       * operation.
       * @param outMergedConfiguration New config container that holds global, override and merged
       * config for window, if it is now becoming visible and the merged configuration has changed
       * since it was last displayed.
       * @param outSurface Object in which is placed the new display surface.
       * @param insetsState The current insets state in the system.
       *
       * @return int Result flags: {@link WindowManagerGlobal#RELAYOUT_SHOW_FOCUS},
       * {@link WindowManagerGlobal#RELAYOUT_FIRST_TIME}.
       */
  public int relayout(android.view.IWindow window, int seq, android.view.WindowManager.LayoutParams attrs, int requestedWidth, int requestedHeight, int viewVisibility, int flags, long frameNumber, android.graphics.Rect outFrame, android.graphics.Rect outOverscanInsets, android.graphics.Rect outContentInsets, android.graphics.Rect outVisibleInsets, android.graphics.Rect outStableInsets, android.graphics.Rect outOutsets, android.graphics.Rect outBackdropFrame, android.view.DisplayCutout.ParcelableWrapper displayCutout, android.util.MergedConfiguration outMergedConfiguration, android.view.SurfaceControl outSurfaceControl, android.view.InsetsState insetsState) throws android.os.RemoteException;
  /*
       * Notify the window manager that an application is relaunching and
       * windows should be prepared for replacement.
       *
       * @param appToken The application
       * @param childrenOnly Whether to only prepare child windows for replacement
       * (for example when main windows are being reused via preservation).
       */
  public void prepareToReplaceWindows(android.os.IBinder appToken, boolean childrenOnly) throws android.os.RemoteException;
  /**
       * Called by a client to report that it ran out of graphics memory.
       */
  public boolean outOfMemory(android.view.IWindow window) throws android.os.RemoteException;
  /**
       * Give the window manager a hint of the part of the window that is
       * completely transparent, allowing it to work with the surface flinger
       * to optimize compositing of this part of the window.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowSession.aidl:128:1:128:25")
  public void setTransparentRegion(android.view.IWindow window, android.graphics.Region region) throws android.os.RemoteException;
  /**
       * Tell the window manager about the content and visible insets of the
       * given window, which can be used to adjust the <var>outContentInsets</var>
       * and <var>outVisibleInsets</var> values returned by
       * {@link #relayout relayout()} for windows behind this one.
       *
       * @param touchableInsets Controls which part of the window inside of its
       * frame can receive pointer events, as defined by
       * {@link android.view.ViewTreeObserver.InternalInsetsInfo}.
       */
  public void setInsets(android.view.IWindow window, int touchableInsets, android.graphics.Rect contentInsets, android.graphics.Rect visibleInsets, android.graphics.Region touchableRegion) throws android.os.RemoteException;
  /**
       * Return the current display size in which the window is being laid out,
       * accounting for screen decorations around it.
       */
  public void getDisplayFrame(android.view.IWindow window, android.graphics.Rect outDisplayFrame) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowSession.aidl:150:1:150:25")
  public void finishDrawing(android.view.IWindow window) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowSession.aidl:153:1:153:25")
  public void setInTouchMode(boolean showFocus) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowSession.aidl:155:1:155:25")
  public boolean getInTouchMode() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowSession.aidl:158:1:158:25")
  public boolean performHapticFeedback(int effectId, boolean always) throws android.os.RemoteException;
  /**
       * Initiate the drag operation itself
       *
       * @param window Window which initiates drag operation.
       * @param flags See {@code View#startDragAndDrop}
       * @param surface Surface containing drag shadow image
       * @param touchSource See {@code InputDevice#getSource()}
       * @param touchX X coordinate of last touch point
       * @param touchY Y coordinate of last touch point
       * @param thumbCenterX X coordinate for the position within the shadow image that should be
       *         underneath the touch point during the drag and drop operation.
       * @param thumbCenterY Y coordinate for the position within the shadow image that should be
       *         underneath the touch point during the drag and drop operation.
       * @param data Data transferred by drag and drop
       * @return Token of drag operation which will be passed to cancelDragAndDrop.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowSession.aidl:177:1:177:25")
  public android.os.IBinder performDrag(android.view.IWindow window, int flags, android.view.SurfaceControl surface, int touchSource, float touchX, float touchY, float thumbCenterX, float thumbCenterY, android.content.ClipData data) throws android.os.RemoteException;
  /**
       * Report the result of a drop action targeted to the given window.
       * consumed is 'true' when the drop was accepted by a valid recipient,
       * 'false' otherwise.
       */
  public void reportDropResult(android.view.IWindow window, boolean consumed) throws android.os.RemoteException;
  /**
       * Cancel the current drag operation.
       * skipAnimation is 'true' when it should skip the drag cancel animation which brings the drag
       * shadow image back to the drag start position.
       */
  public void cancelDragAndDrop(android.os.IBinder dragToken, boolean skipAnimation) throws android.os.RemoteException;
  /**
       * Tell the OS that we've just dragged into a View that is willing to accept the drop
       */
  public void dragRecipientEntered(android.view.IWindow window) throws android.os.RemoteException;
  /**
       * Tell the OS that we've just dragged *off* of a View that was willing to accept the drop
       */
  public void dragRecipientExited(android.view.IWindow window) throws android.os.RemoteException;
  /**
       * For windows with the wallpaper behind them, and the wallpaper is
       * larger than the screen, set the offset within the screen.
       * For multi screen launcher type applications, xstep and ystep indicate
       * how big the increment is from one screen to another.
       */
  public void setWallpaperPosition(android.os.IBinder windowToken, float x, float y, float xstep, float ystep) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowSession.aidl:213:1:213:25")
  public void wallpaperOffsetsComplete(android.os.IBinder window) throws android.os.RemoteException;
  /**
       * Apply a raw offset to the wallpaper service when shown behind this window.
       */
  public void setWallpaperDisplayOffset(android.os.IBinder windowToken, int x, int y) throws android.os.RemoteException;
  public android.os.Bundle sendWallpaperCommand(android.os.IBinder window, java.lang.String action, int x, int y, int z, android.os.Bundle extras, boolean sync) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/IWindowSession.aidl:224:1:224:25")
  public void wallpaperCommandComplete(android.os.IBinder window, android.os.Bundle result) throws android.os.RemoteException;
  /**
       * Notifies that a rectangle on the screen has been requested.
       */
  public void onRectangleOnScreenRequested(android.os.IBinder token, android.graphics.Rect rectangle) throws android.os.RemoteException;
  public android.view.IWindowId getWindowId(android.os.IBinder window) throws android.os.RemoteException;
  /**
       * When the system is dozing in a low-power partially suspended state, pokes a short
       * lived wake lock and ensures that the display is ready to accept the next frame
       * of content drawn in the window.
       *
       * This mechanism is bound to the window rather than to the display manager or the
       * power manager so that the system can ensure that the window is actually visible
       * and prevent runaway applications from draining the battery.  This is similar to how
       * FLAG_KEEP_SCREEN_ON works.
       *
       * This method is synchronous because it may need to acquire a wake lock before returning.
       * The assumption is that this method will be called rather infrequently.
       */
  public void pokeDrawLock(android.os.IBinder window) throws android.os.RemoteException;
  /**
       * Starts a task window move with {startX, startY} as starting point. The amount of move
       * will be the offset between {startX, startY} and the new cursor position.
       *
       * Returns true if the move started successfully; false otherwise.
       */
  public boolean startMovingTask(android.view.IWindow window, float startX, float startY) throws android.os.RemoteException;
  public void finishMovingTask(android.view.IWindow window) throws android.os.RemoteException;
  public void updatePointerIcon(android.view.IWindow window) throws android.os.RemoteException;
  /**
       * Reparent the top layers for a display to the requested SurfaceControl. The display that is
       * going to be re-parented (the displayId passed in) needs to have been created by the same
       * process that is requesting the re-parent. This is to ensure clients can't just re-parent
       * display content info to any SurfaceControl, as this would be a security issue.
       *
       * @param window The window which owns the SurfaceControl. This indicates the z-order of the
       *               windows of this display against the windows on the parent display.
       * @param sc The SurfaceControl that the top level layers for the display should be re-parented
       *           to.
       * @param displayId The id of the display to be re-parented.
       */
  public void reparentDisplayContent(android.view.IWindow window, android.view.SurfaceControl sc, int displayId) throws android.os.RemoteException;
  /**
       * Update the location of a child display in its parent window. This enables windows in the
       * child display to compute the global transformation matrix.
       *
       * @param window The parent window of the display.
       * @param x The x coordinate in the parent window.
       * @param y The y coordinate in the parent window.
       * @param displayId The id of the display to be notified.
       */
  public void updateDisplayContentLocation(android.view.IWindow window, int x, int y, int displayId) throws android.os.RemoteException;
  /**
       * Update a tap exclude region identified by provided id in the window. Touches on this region
       * will neither be dispatched to this window nor change the focus to this window. Passing an
       * invalid region will remove the area from the exclude region of this window.
       */
  public void updateTapExcludeRegion(android.view.IWindow window, int regionId, android.graphics.Region region) throws android.os.RemoteException;
  /**
       * Called when the client has changed the local insets state, and now the server should reflect
       * that new state.
       */
  public void insetsModified(android.view.IWindow window, android.view.InsetsState state) throws android.os.RemoteException;
  /**
       * Called when the system gesture exclusion has changed.
       */
  public void reportSystemGestureExclusionChanged(android.view.IWindow window, java.util.List<android.graphics.Rect> exclusionRects) throws android.os.RemoteException;
}
