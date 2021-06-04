/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view.accessibility;
/**
 * Interface for interaction between the AccessibilityManagerService
 * and the ViewRoot in a given window.
 *
 * @hide
 */
public interface IAccessibilityInteractionConnection extends android.os.IInterface
{
  /** Default implementation for IAccessibilityInteractionConnection. */
  public static class Default implements android.view.accessibility.IAccessibilityInteractionConnection
  {
    @Override public void findAccessibilityNodeInfoByAccessibilityId(long accessibilityNodeId, android.graphics.Region bounds, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, int flags, int interrogatingPid, long interrogatingTid, android.view.MagnificationSpec spec, android.os.Bundle arguments) throws android.os.RemoteException
    {
    }
    @Override public void findAccessibilityNodeInfosByViewId(long accessibilityNodeId, java.lang.String viewId, android.graphics.Region bounds, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, int flags, int interrogatingPid, long interrogatingTid, android.view.MagnificationSpec spec) throws android.os.RemoteException
    {
    }
    @Override public void findAccessibilityNodeInfosByText(long accessibilityNodeId, java.lang.String text, android.graphics.Region bounds, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, int flags, int interrogatingPid, long interrogatingTid, android.view.MagnificationSpec spec) throws android.os.RemoteException
    {
    }
    @Override public void findFocus(long accessibilityNodeId, int focusType, android.graphics.Region bounds, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, int flags, int interrogatingPid, long interrogatingTid, android.view.MagnificationSpec spec) throws android.os.RemoteException
    {
    }
    @Override public void focusSearch(long accessibilityNodeId, int direction, android.graphics.Region bounds, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, int flags, int interrogatingPid, long interrogatingTid, android.view.MagnificationSpec spec) throws android.os.RemoteException
    {
    }
    @Override public void performAccessibilityAction(long accessibilityNodeId, int action, android.os.Bundle arguments, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, int flags, int interrogatingPid, long interrogatingTid) throws android.os.RemoteException
    {
    }
    @Override public void clearAccessibilityFocus() throws android.os.RemoteException
    {
    }
    @Override public void notifyOutsideTouch() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.accessibility.IAccessibilityInteractionConnection
  {
    private static final java.lang.String DESCRIPTOR = "android.view.accessibility.IAccessibilityInteractionConnection";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.accessibility.IAccessibilityInteractionConnection interface,
     * generating a proxy if needed.
     */
    public static android.view.accessibility.IAccessibilityInteractionConnection asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.accessibility.IAccessibilityInteractionConnection))) {
        return ((android.view.accessibility.IAccessibilityInteractionConnection)iin);
      }
      return new android.view.accessibility.IAccessibilityInteractionConnection.Stub.Proxy(obj);
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
        case TRANSACTION_findAccessibilityNodeInfoByAccessibilityId:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          android.graphics.Region _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.graphics.Region.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          android.view.accessibility.IAccessibilityInteractionConnectionCallback _arg3;
          _arg3 = android.view.accessibility.IAccessibilityInteractionConnectionCallback.Stub.asInterface(data.readStrongBinder());
          int _arg4;
          _arg4 = data.readInt();
          int _arg5;
          _arg5 = data.readInt();
          long _arg6;
          _arg6 = data.readLong();
          android.view.MagnificationSpec _arg7;
          if ((0!=data.readInt())) {
            _arg7 = android.view.MagnificationSpec.CREATOR.createFromParcel(data);
          }
          else {
            _arg7 = null;
          }
          android.os.Bundle _arg8;
          if ((0!=data.readInt())) {
            _arg8 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg8 = null;
          }
          this.findAccessibilityNodeInfoByAccessibilityId(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8);
          return true;
        }
        case TRANSACTION_findAccessibilityNodeInfosByViewId:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.graphics.Region _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.graphics.Region.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          android.view.accessibility.IAccessibilityInteractionConnectionCallback _arg4;
          _arg4 = android.view.accessibility.IAccessibilityInteractionConnectionCallback.Stub.asInterface(data.readStrongBinder());
          int _arg5;
          _arg5 = data.readInt();
          int _arg6;
          _arg6 = data.readInt();
          long _arg7;
          _arg7 = data.readLong();
          android.view.MagnificationSpec _arg8;
          if ((0!=data.readInt())) {
            _arg8 = android.view.MagnificationSpec.CREATOR.createFromParcel(data);
          }
          else {
            _arg8 = null;
          }
          this.findAccessibilityNodeInfosByViewId(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8);
          return true;
        }
        case TRANSACTION_findAccessibilityNodeInfosByText:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.graphics.Region _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.graphics.Region.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          android.view.accessibility.IAccessibilityInteractionConnectionCallback _arg4;
          _arg4 = android.view.accessibility.IAccessibilityInteractionConnectionCallback.Stub.asInterface(data.readStrongBinder());
          int _arg5;
          _arg5 = data.readInt();
          int _arg6;
          _arg6 = data.readInt();
          long _arg7;
          _arg7 = data.readLong();
          android.view.MagnificationSpec _arg8;
          if ((0!=data.readInt())) {
            _arg8 = android.view.MagnificationSpec.CREATOR.createFromParcel(data);
          }
          else {
            _arg8 = null;
          }
          this.findAccessibilityNodeInfosByText(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8);
          return true;
        }
        case TRANSACTION_findFocus:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          android.graphics.Region _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.graphics.Region.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          android.view.accessibility.IAccessibilityInteractionConnectionCallback _arg4;
          _arg4 = android.view.accessibility.IAccessibilityInteractionConnectionCallback.Stub.asInterface(data.readStrongBinder());
          int _arg5;
          _arg5 = data.readInt();
          int _arg6;
          _arg6 = data.readInt();
          long _arg7;
          _arg7 = data.readLong();
          android.view.MagnificationSpec _arg8;
          if ((0!=data.readInt())) {
            _arg8 = android.view.MagnificationSpec.CREATOR.createFromParcel(data);
          }
          else {
            _arg8 = null;
          }
          this.findFocus(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8);
          return true;
        }
        case TRANSACTION_focusSearch:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          android.graphics.Region _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.graphics.Region.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          android.view.accessibility.IAccessibilityInteractionConnectionCallback _arg4;
          _arg4 = android.view.accessibility.IAccessibilityInteractionConnectionCallback.Stub.asInterface(data.readStrongBinder());
          int _arg5;
          _arg5 = data.readInt();
          int _arg6;
          _arg6 = data.readInt();
          long _arg7;
          _arg7 = data.readLong();
          android.view.MagnificationSpec _arg8;
          if ((0!=data.readInt())) {
            _arg8 = android.view.MagnificationSpec.CREATOR.createFromParcel(data);
          }
          else {
            _arg8 = null;
          }
          this.focusSearch(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8);
          return true;
        }
        case TRANSACTION_performAccessibilityAction:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          android.view.accessibility.IAccessibilityInteractionConnectionCallback _arg4;
          _arg4 = android.view.accessibility.IAccessibilityInteractionConnectionCallback.Stub.asInterface(data.readStrongBinder());
          int _arg5;
          _arg5 = data.readInt();
          int _arg6;
          _arg6 = data.readInt();
          long _arg7;
          _arg7 = data.readLong();
          this.performAccessibilityAction(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7);
          return true;
        }
        case TRANSACTION_clearAccessibilityFocus:
        {
          data.enforceInterface(descriptor);
          this.clearAccessibilityFocus();
          return true;
        }
        case TRANSACTION_notifyOutsideTouch:
        {
          data.enforceInterface(descriptor);
          this.notifyOutsideTouch();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.accessibility.IAccessibilityInteractionConnection
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
      @Override public void findAccessibilityNodeInfoByAccessibilityId(long accessibilityNodeId, android.graphics.Region bounds, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, int flags, int interrogatingPid, long interrogatingTid, android.view.MagnificationSpec spec, android.os.Bundle arguments) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(accessibilityNodeId);
          if ((bounds!=null)) {
            _data.writeInt(1);
            bounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(interactionId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(flags);
          _data.writeInt(interrogatingPid);
          _data.writeLong(interrogatingTid);
          if ((spec!=null)) {
            _data.writeInt(1);
            spec.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((arguments!=null)) {
            _data.writeInt(1);
            arguments.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_findAccessibilityNodeInfoByAccessibilityId, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().findAccessibilityNodeInfoByAccessibilityId(accessibilityNodeId, bounds, interactionId, callback, flags, interrogatingPid, interrogatingTid, spec, arguments);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void findAccessibilityNodeInfosByViewId(long accessibilityNodeId, java.lang.String viewId, android.graphics.Region bounds, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, int flags, int interrogatingPid, long interrogatingTid, android.view.MagnificationSpec spec) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(accessibilityNodeId);
          _data.writeString(viewId);
          if ((bounds!=null)) {
            _data.writeInt(1);
            bounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(interactionId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(flags);
          _data.writeInt(interrogatingPid);
          _data.writeLong(interrogatingTid);
          if ((spec!=null)) {
            _data.writeInt(1);
            spec.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_findAccessibilityNodeInfosByViewId, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().findAccessibilityNodeInfosByViewId(accessibilityNodeId, viewId, bounds, interactionId, callback, flags, interrogatingPid, interrogatingTid, spec);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void findAccessibilityNodeInfosByText(long accessibilityNodeId, java.lang.String text, android.graphics.Region bounds, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, int flags, int interrogatingPid, long interrogatingTid, android.view.MagnificationSpec spec) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(accessibilityNodeId);
          _data.writeString(text);
          if ((bounds!=null)) {
            _data.writeInt(1);
            bounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(interactionId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(flags);
          _data.writeInt(interrogatingPid);
          _data.writeLong(interrogatingTid);
          if ((spec!=null)) {
            _data.writeInt(1);
            spec.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_findAccessibilityNodeInfosByText, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().findAccessibilityNodeInfosByText(accessibilityNodeId, text, bounds, interactionId, callback, flags, interrogatingPid, interrogatingTid, spec);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void findFocus(long accessibilityNodeId, int focusType, android.graphics.Region bounds, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, int flags, int interrogatingPid, long interrogatingTid, android.view.MagnificationSpec spec) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(accessibilityNodeId);
          _data.writeInt(focusType);
          if ((bounds!=null)) {
            _data.writeInt(1);
            bounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(interactionId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(flags);
          _data.writeInt(interrogatingPid);
          _data.writeLong(interrogatingTid);
          if ((spec!=null)) {
            _data.writeInt(1);
            spec.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_findFocus, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().findFocus(accessibilityNodeId, focusType, bounds, interactionId, callback, flags, interrogatingPid, interrogatingTid, spec);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void focusSearch(long accessibilityNodeId, int direction, android.graphics.Region bounds, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, int flags, int interrogatingPid, long interrogatingTid, android.view.MagnificationSpec spec) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(accessibilityNodeId);
          _data.writeInt(direction);
          if ((bounds!=null)) {
            _data.writeInt(1);
            bounds.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(interactionId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(flags);
          _data.writeInt(interrogatingPid);
          _data.writeLong(interrogatingTid);
          if ((spec!=null)) {
            _data.writeInt(1);
            spec.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_focusSearch, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().focusSearch(accessibilityNodeId, direction, bounds, interactionId, callback, flags, interrogatingPid, interrogatingTid, spec);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void performAccessibilityAction(long accessibilityNodeId, int action, android.os.Bundle arguments, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, int flags, int interrogatingPid, long interrogatingTid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(accessibilityNodeId);
          _data.writeInt(action);
          if ((arguments!=null)) {
            _data.writeInt(1);
            arguments.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(interactionId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(flags);
          _data.writeInt(interrogatingPid);
          _data.writeLong(interrogatingTid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_performAccessibilityAction, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().performAccessibilityAction(accessibilityNodeId, action, arguments, interactionId, callback, flags, interrogatingPid, interrogatingTid);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void clearAccessibilityFocus() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearAccessibilityFocus, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearAccessibilityFocus();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void notifyOutsideTouch() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyOutsideTouch, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyOutsideTouch();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.view.accessibility.IAccessibilityInteractionConnection sDefaultImpl;
    }
    static final int TRANSACTION_findAccessibilityNodeInfoByAccessibilityId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_findAccessibilityNodeInfosByViewId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_findAccessibilityNodeInfosByText = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_findFocus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_focusSearch = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_performAccessibilityAction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_clearAccessibilityFocus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_notifyOutsideTouch = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    public static boolean setDefaultImpl(android.view.accessibility.IAccessibilityInteractionConnection impl) {
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
    public static android.view.accessibility.IAccessibilityInteractionConnection getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void findAccessibilityNodeInfoByAccessibilityId(long accessibilityNodeId, android.graphics.Region bounds, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, int flags, int interrogatingPid, long interrogatingTid, android.view.MagnificationSpec spec, android.os.Bundle arguments) throws android.os.RemoteException;
  public void findAccessibilityNodeInfosByViewId(long accessibilityNodeId, java.lang.String viewId, android.graphics.Region bounds, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, int flags, int interrogatingPid, long interrogatingTid, android.view.MagnificationSpec spec) throws android.os.RemoteException;
  public void findAccessibilityNodeInfosByText(long accessibilityNodeId, java.lang.String text, android.graphics.Region bounds, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, int flags, int interrogatingPid, long interrogatingTid, android.view.MagnificationSpec spec) throws android.os.RemoteException;
  public void findFocus(long accessibilityNodeId, int focusType, android.graphics.Region bounds, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, int flags, int interrogatingPid, long interrogatingTid, android.view.MagnificationSpec spec) throws android.os.RemoteException;
  public void focusSearch(long accessibilityNodeId, int direction, android.graphics.Region bounds, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, int flags, int interrogatingPid, long interrogatingTid, android.view.MagnificationSpec spec) throws android.os.RemoteException;
  public void performAccessibilityAction(long accessibilityNodeId, int action, android.os.Bundle arguments, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, int flags, int interrogatingPid, long interrogatingTid) throws android.os.RemoteException;
  public void clearAccessibilityFocus() throws android.os.RemoteException;
  public void notifyOutsideTouch() throws android.os.RemoteException;
}
