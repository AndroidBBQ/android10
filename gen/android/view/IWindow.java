/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view;
/**
 * API back to a client window that the Window Manager uses to inform it of
 * interesting things happening.
 *
 * {@hide}
 */
public interface IWindow extends android.os.IInterface
{
  /** Default implementation for IWindow. */
  public static class Default implements android.view.IWindow
  {
    /**
         * ===== NOTICE =====
         * The first method must remain the first method. Scripts
         * and tools rely on their transaction number to work properly.
         *//**
         * Invoked by the view server to tell a window to execute the specified
         * command. Any response from the receiver must be sent through the
         * specified file descriptor.
         */
    @Override public void executeCommand(java.lang.String command, java.lang.String parameters, android.os.ParcelFileDescriptor descriptor) throws android.os.RemoteException
    {
    }
    @Override public void resized(android.graphics.Rect frame, android.graphics.Rect overscanInsets, android.graphics.Rect contentInsets, android.graphics.Rect visibleInsets, android.graphics.Rect stableInsets, android.graphics.Rect outsets, boolean reportDraw, android.util.MergedConfiguration newMergedConfiguration, android.graphics.Rect backDropFrame, boolean forceLayout, boolean alwaysConsumeSystemBars, int displayId, android.view.DisplayCutout.ParcelableWrapper displayCutout) throws android.os.RemoteException
    {
    }
    /**
         * Called when the window location in parent display has changed. The offset will only be a
         * nonzero value if the window is on an embedded display that is re-parented to another window.
         */
    @Override public void locationInParentDisplayChanged(android.graphics.Point offset) throws android.os.RemoteException
    {
    }
    /**
         * Called when the window insets configuration has changed.
         */
    @Override public void insetsChanged(android.view.InsetsState insetsState) throws android.os.RemoteException
    {
    }
    /**
         * Called when this window retrieved control over a specified set of inset sources.
         */
    @Override public void insetsControlChanged(android.view.InsetsState insetsState, android.view.InsetsSourceControl[] activeControls) throws android.os.RemoteException
    {
    }
    @Override public void moved(int newX, int newY) throws android.os.RemoteException
    {
    }
    @Override public void dispatchAppVisibility(boolean visible) throws android.os.RemoteException
    {
    }
    @Override public void dispatchGetNewSurface() throws android.os.RemoteException
    {
    }
    /**
         * Tell the window that it is either gaining or losing focus.  Keep it up
         * to date on the current state showing navigational focus (touch mode) too.
         */
    @Override public void windowFocusChanged(boolean hasFocus, boolean inTouchMode) throws android.os.RemoteException
    {
    }
    @Override public void closeSystemDialogs(java.lang.String reason) throws android.os.RemoteException
    {
    }
    /**
         * Called for wallpaper windows when their offsets change.
         */
    @Override public void dispatchWallpaperOffsets(float x, float y, float xStep, float yStep, boolean sync) throws android.os.RemoteException
    {
    }
    @Override public void dispatchWallpaperCommand(java.lang.String action, int x, int y, int z, android.os.Bundle extras, boolean sync) throws android.os.RemoteException
    {
    }
    /**
         * Drag/drop events
         */
    @Override public void dispatchDragEvent(android.view.DragEvent event) throws android.os.RemoteException
    {
    }
    /**
         * Pointer icon events
         */
    @Override public void updatePointerIcon(float x, float y) throws android.os.RemoteException
    {
    }
    /**
         * System chrome visibility changes
         */
    @Override public void dispatchSystemUiVisibilityChanged(int seq, int globalVisibility, int localValue, int localChanges) throws android.os.RemoteException
    {
    }
    /**
         * Called for non-application windows when the enter animation has completed.
         */
    @Override public void dispatchWindowShown() throws android.os.RemoteException
    {
    }
    /**
         * Called when Keyboard Shortcuts are requested for the window.
         */
    @Override public void requestAppKeyboardShortcuts(com.android.internal.os.IResultReceiver receiver, int deviceId) throws android.os.RemoteException
    {
    }
    /**
         * Tell the window that it is either gaining or losing pointer capture.
         */
    @Override public void dispatchPointerCaptureChanged(boolean hasCapture) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.IWindow
  {
    private static final java.lang.String DESCRIPTOR = "android.view.IWindow";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.IWindow interface,
     * generating a proxy if needed.
     */
    public static android.view.IWindow asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.IWindow))) {
        return ((android.view.IWindow)iin);
      }
      return new android.view.IWindow.Stub.Proxy(obj);
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
        case TRANSACTION_executeCommand:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.ParcelFileDescriptor _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.executeCommand(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_resized:
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
          android.graphics.Rect _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          android.graphics.Rect _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          android.graphics.Rect _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          boolean _arg6;
          _arg6 = (0!=data.readInt());
          android.util.MergedConfiguration _arg7;
          if ((0!=data.readInt())) {
            _arg7 = android.util.MergedConfiguration.CREATOR.createFromParcel(data);
          }
          else {
            _arg7 = null;
          }
          android.graphics.Rect _arg8;
          if ((0!=data.readInt())) {
            _arg8 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg8 = null;
          }
          boolean _arg9;
          _arg9 = (0!=data.readInt());
          boolean _arg10;
          _arg10 = (0!=data.readInt());
          int _arg11;
          _arg11 = data.readInt();
          android.view.DisplayCutout.ParcelableWrapper _arg12;
          if ((0!=data.readInt())) {
            _arg12 = android.view.DisplayCutout.ParcelableWrapper.CREATOR.createFromParcel(data);
          }
          else {
            _arg12 = null;
          }
          this.resized(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9, _arg10, _arg11, _arg12);
          return true;
        }
        case TRANSACTION_locationInParentDisplayChanged:
        {
          data.enforceInterface(descriptor);
          android.graphics.Point _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.graphics.Point.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.locationInParentDisplayChanged(_arg0);
          return true;
        }
        case TRANSACTION_insetsChanged:
        {
          data.enforceInterface(descriptor);
          android.view.InsetsState _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.InsetsState.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.insetsChanged(_arg0);
          return true;
        }
        case TRANSACTION_insetsControlChanged:
        {
          data.enforceInterface(descriptor);
          android.view.InsetsState _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.InsetsState.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.view.InsetsSourceControl[] _arg1;
          _arg1 = data.createTypedArray(android.view.InsetsSourceControl.CREATOR);
          this.insetsControlChanged(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_moved:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.moved(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_dispatchAppVisibility:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.dispatchAppVisibility(_arg0);
          return true;
        }
        case TRANSACTION_dispatchGetNewSurface:
        {
          data.enforceInterface(descriptor);
          this.dispatchGetNewSurface();
          return true;
        }
        case TRANSACTION_windowFocusChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.windowFocusChanged(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_closeSystemDialogs:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.closeSystemDialogs(_arg0);
          return true;
        }
        case TRANSACTION_dispatchWallpaperOffsets:
        {
          data.enforceInterface(descriptor);
          float _arg0;
          _arg0 = data.readFloat();
          float _arg1;
          _arg1 = data.readFloat();
          float _arg2;
          _arg2 = data.readFloat();
          float _arg3;
          _arg3 = data.readFloat();
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          this.dispatchWallpaperOffsets(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_dispatchWallpaperCommand:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          android.os.Bundle _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          boolean _arg5;
          _arg5 = (0!=data.readInt());
          this.dispatchWallpaperCommand(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          return true;
        }
        case TRANSACTION_dispatchDragEvent:
        {
          data.enforceInterface(descriptor);
          android.view.DragEvent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.DragEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.dispatchDragEvent(_arg0);
          return true;
        }
        case TRANSACTION_updatePointerIcon:
        {
          data.enforceInterface(descriptor);
          float _arg0;
          _arg0 = data.readFloat();
          float _arg1;
          _arg1 = data.readFloat();
          this.updatePointerIcon(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_dispatchSystemUiVisibilityChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          this.dispatchSystemUiVisibilityChanged(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_dispatchWindowShown:
        {
          data.enforceInterface(descriptor);
          this.dispatchWindowShown();
          return true;
        }
        case TRANSACTION_requestAppKeyboardShortcuts:
        {
          data.enforceInterface(descriptor);
          com.android.internal.os.IResultReceiver _arg0;
          _arg0 = com.android.internal.os.IResultReceiver.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.requestAppKeyboardShortcuts(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_dispatchPointerCaptureChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.dispatchPointerCaptureChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.IWindow
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
           * ===== NOTICE =====
           * The first method must remain the first method. Scripts
           * and tools rely on their transaction number to work properly.
           *//**
           * Invoked by the view server to tell a window to execute the specified
           * command. Any response from the receiver must be sent through the
           * specified file descriptor.
           */
      @Override public void executeCommand(java.lang.String command, java.lang.String parameters, android.os.ParcelFileDescriptor descriptor) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(command);
          _data.writeString(parameters);
          if ((descriptor!=null)) {
            _data.writeInt(1);
            descriptor.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_executeCommand, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().executeCommand(command, parameters, descriptor);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void resized(android.graphics.Rect frame, android.graphics.Rect overscanInsets, android.graphics.Rect contentInsets, android.graphics.Rect visibleInsets, android.graphics.Rect stableInsets, android.graphics.Rect outsets, boolean reportDraw, android.util.MergedConfiguration newMergedConfiguration, android.graphics.Rect backDropFrame, boolean forceLayout, boolean alwaysConsumeSystemBars, int displayId, android.view.DisplayCutout.ParcelableWrapper displayCutout) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((frame!=null)) {
            _data.writeInt(1);
            frame.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((overscanInsets!=null)) {
            _data.writeInt(1);
            overscanInsets.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
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
          if ((stableInsets!=null)) {
            _data.writeInt(1);
            stableInsets.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((outsets!=null)) {
            _data.writeInt(1);
            outsets.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((reportDraw)?(1):(0)));
          if ((newMergedConfiguration!=null)) {
            _data.writeInt(1);
            newMergedConfiguration.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((backDropFrame!=null)) {
            _data.writeInt(1);
            backDropFrame.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((forceLayout)?(1):(0)));
          _data.writeInt(((alwaysConsumeSystemBars)?(1):(0)));
          _data.writeInt(displayId);
          if ((displayCutout!=null)) {
            _data.writeInt(1);
            displayCutout.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_resized, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().resized(frame, overscanInsets, contentInsets, visibleInsets, stableInsets, outsets, reportDraw, newMergedConfiguration, backDropFrame, forceLayout, alwaysConsumeSystemBars, displayId, displayCutout);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when the window location in parent display has changed. The offset will only be a
           * nonzero value if the window is on an embedded display that is re-parented to another window.
           */
      @Override public void locationInParentDisplayChanged(android.graphics.Point offset) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((offset!=null)) {
            _data.writeInt(1);
            offset.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_locationInParentDisplayChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().locationInParentDisplayChanged(offset);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when the window insets configuration has changed.
           */
      @Override public void insetsChanged(android.view.InsetsState insetsState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((insetsState!=null)) {
            _data.writeInt(1);
            insetsState.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_insetsChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().insetsChanged(insetsState);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when this window retrieved control over a specified set of inset sources.
           */
      @Override public void insetsControlChanged(android.view.InsetsState insetsState, android.view.InsetsSourceControl[] activeControls) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((insetsState!=null)) {
            _data.writeInt(1);
            insetsState.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeTypedArray(activeControls, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_insetsControlChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().insetsControlChanged(insetsState, activeControls);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void moved(int newX, int newY) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(newX);
          _data.writeInt(newY);
          boolean _status = mRemote.transact(Stub.TRANSACTION_moved, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().moved(newX, newY);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void dispatchAppVisibility(boolean visible) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((visible)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchAppVisibility, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispatchAppVisibility(visible);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void dispatchGetNewSurface() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchGetNewSurface, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispatchGetNewSurface();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Tell the window that it is either gaining or losing focus.  Keep it up
           * to date on the current state showing navigational focus (touch mode) too.
           */
      @Override public void windowFocusChanged(boolean hasFocus, boolean inTouchMode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((hasFocus)?(1):(0)));
          _data.writeInt(((inTouchMode)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_windowFocusChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().windowFocusChanged(hasFocus, inTouchMode);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void closeSystemDialogs(java.lang.String reason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(reason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_closeSystemDialogs, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().closeSystemDialogs(reason);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called for wallpaper windows when their offsets change.
           */
      @Override public void dispatchWallpaperOffsets(float x, float y, float xStep, float yStep, boolean sync) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeFloat(x);
          _data.writeFloat(y);
          _data.writeFloat(xStep);
          _data.writeFloat(yStep);
          _data.writeInt(((sync)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchWallpaperOffsets, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispatchWallpaperOffsets(x, y, xStep, yStep, sync);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void dispatchWallpaperCommand(java.lang.String action, int x, int y, int z, android.os.Bundle extras, boolean sync) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchWallpaperCommand, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispatchWallpaperCommand(action, x, y, z, extras, sync);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Drag/drop events
           */
      @Override public void dispatchDragEvent(android.view.DragEvent event) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((event!=null)) {
            _data.writeInt(1);
            event.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchDragEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispatchDragEvent(event);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Pointer icon events
           */
      @Override public void updatePointerIcon(float x, float y) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeFloat(x);
          _data.writeFloat(y);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updatePointerIcon, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updatePointerIcon(x, y);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * System chrome visibility changes
           */
      @Override public void dispatchSystemUiVisibilityChanged(int seq, int globalVisibility, int localValue, int localChanges) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(seq);
          _data.writeInt(globalVisibility);
          _data.writeInt(localValue);
          _data.writeInt(localChanges);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchSystemUiVisibilityChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispatchSystemUiVisibilityChanged(seq, globalVisibility, localValue, localChanges);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called for non-application windows when the enter animation has completed.
           */
      @Override public void dispatchWindowShown() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchWindowShown, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispatchWindowShown();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when Keyboard Shortcuts are requested for the window.
           */
      @Override public void requestAppKeyboardShortcuts(com.android.internal.os.IResultReceiver receiver, int deviceId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((receiver!=null))?(receiver.asBinder()):(null)));
          _data.writeInt(deviceId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestAppKeyboardShortcuts, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestAppKeyboardShortcuts(receiver, deviceId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Tell the window that it is either gaining or losing pointer capture.
           */
      @Override public void dispatchPointerCaptureChanged(boolean hasCapture) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((hasCapture)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchPointerCaptureChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispatchPointerCaptureChanged(hasCapture);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.view.IWindow sDefaultImpl;
    }
    static final int TRANSACTION_executeCommand = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_resized = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_locationInParentDisplayChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_insetsChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_insetsControlChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_moved = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_dispatchAppVisibility = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_dispatchGetNewSurface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_windowFocusChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_closeSystemDialogs = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_dispatchWallpaperOffsets = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_dispatchWallpaperCommand = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_dispatchDragEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_updatePointerIcon = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_dispatchSystemUiVisibilityChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_dispatchWindowShown = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_requestAppKeyboardShortcuts = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_dispatchPointerCaptureChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    public static boolean setDefaultImpl(android.view.IWindow impl) {
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
    public static android.view.IWindow getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * ===== NOTICE =====
       * The first method must remain the first method. Scripts
       * and tools rely on their transaction number to work properly.
       *//**
       * Invoked by the view server to tell a window to execute the specified
       * command. Any response from the receiver must be sent through the
       * specified file descriptor.
       */
  public void executeCommand(java.lang.String command, java.lang.String parameters, android.os.ParcelFileDescriptor descriptor) throws android.os.RemoteException;
  public void resized(android.graphics.Rect frame, android.graphics.Rect overscanInsets, android.graphics.Rect contentInsets, android.graphics.Rect visibleInsets, android.graphics.Rect stableInsets, android.graphics.Rect outsets, boolean reportDraw, android.util.MergedConfiguration newMergedConfiguration, android.graphics.Rect backDropFrame, boolean forceLayout, boolean alwaysConsumeSystemBars, int displayId, android.view.DisplayCutout.ParcelableWrapper displayCutout) throws android.os.RemoteException;
  /**
       * Called when the window location in parent display has changed. The offset will only be a
       * nonzero value if the window is on an embedded display that is re-parented to another window.
       */
  public void locationInParentDisplayChanged(android.graphics.Point offset) throws android.os.RemoteException;
  /**
       * Called when the window insets configuration has changed.
       */
  public void insetsChanged(android.view.InsetsState insetsState) throws android.os.RemoteException;
  /**
       * Called when this window retrieved control over a specified set of inset sources.
       */
  public void insetsControlChanged(android.view.InsetsState insetsState, android.view.InsetsSourceControl[] activeControls) throws android.os.RemoteException;
  public void moved(int newX, int newY) throws android.os.RemoteException;
  public void dispatchAppVisibility(boolean visible) throws android.os.RemoteException;
  public void dispatchGetNewSurface() throws android.os.RemoteException;
  /**
       * Tell the window that it is either gaining or losing focus.  Keep it up
       * to date on the current state showing navigational focus (touch mode) too.
       */
  public void windowFocusChanged(boolean hasFocus, boolean inTouchMode) throws android.os.RemoteException;
  public void closeSystemDialogs(java.lang.String reason) throws android.os.RemoteException;
  /**
       * Called for wallpaper windows when their offsets change.
       */
  public void dispatchWallpaperOffsets(float x, float y, float xStep, float yStep, boolean sync) throws android.os.RemoteException;
  public void dispatchWallpaperCommand(java.lang.String action, int x, int y, int z, android.os.Bundle extras, boolean sync) throws android.os.RemoteException;
  /**
       * Drag/drop events
       */
  public void dispatchDragEvent(android.view.DragEvent event) throws android.os.RemoteException;
  /**
       * Pointer icon events
       */
  public void updatePointerIcon(float x, float y) throws android.os.RemoteException;
  /**
       * System chrome visibility changes
       */
  public void dispatchSystemUiVisibilityChanged(int seq, int globalVisibility, int localValue, int localChanges) throws android.os.RemoteException;
  /**
       * Called for non-application windows when the enter animation has completed.
       */
  public void dispatchWindowShown() throws android.os.RemoteException;
  /**
       * Called when Keyboard Shortcuts are requested for the window.
       */
  public void requestAppKeyboardShortcuts(com.android.internal.os.IResultReceiver receiver, int deviceId) throws android.os.RemoteException;
  /**
       * Tell the window that it is either gaining or losing pointer capture.
       */
  public void dispatchPointerCaptureChanged(boolean hasCapture) throws android.os.RemoteException;
}
