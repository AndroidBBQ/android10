/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.accessibilityservice;
/**
 * Interface given to an AccessibilitySerivce to talk to the AccessibilityManagerService.
 *
 * @hide
 */
public interface IAccessibilityServiceConnection extends android.os.IInterface
{
  /** Default implementation for IAccessibilityServiceConnection. */
  public static class Default implements android.accessibilityservice.IAccessibilityServiceConnection
  {
    @Override public void setServiceInfo(android.accessibilityservice.AccessibilityServiceInfo info) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String[] findAccessibilityNodeInfoByAccessibilityId(int accessibilityWindowId, long accessibilityNodeId, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, int flags, long threadId, android.os.Bundle arguments) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String[] findAccessibilityNodeInfosByText(int accessibilityWindowId, long accessibilityNodeId, java.lang.String text, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, long threadId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String[] findAccessibilityNodeInfosByViewId(int accessibilityWindowId, long accessibilityNodeId, java.lang.String viewId, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, long threadId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String[] findFocus(int accessibilityWindowId, long accessibilityNodeId, int focusType, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, long threadId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String[] focusSearch(int accessibilityWindowId, long accessibilityNodeId, int direction, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, long threadId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean performAccessibilityAction(int accessibilityWindowId, long accessibilityNodeId, int action, android.os.Bundle arguments, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, long threadId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.view.accessibility.AccessibilityWindowInfo getWindow(int windowId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.util.List<android.view.accessibility.AccessibilityWindowInfo> getWindows() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.accessibilityservice.AccessibilityServiceInfo getServiceInfo() throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean performGlobalAction(int action) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void disableSelf() throws android.os.RemoteException
    {
    }
    @Override public void setOnKeyEventResult(boolean handled, int sequence) throws android.os.RemoteException
    {
    }
    @Override public float getMagnificationScale(int displayId) throws android.os.RemoteException
    {
      return 0.0f;
    }
    @Override public float getMagnificationCenterX(int displayId) throws android.os.RemoteException
    {
      return 0.0f;
    }
    @Override public float getMagnificationCenterY(int displayId) throws android.os.RemoteException
    {
      return 0.0f;
    }
    @Override public android.graphics.Region getMagnificationRegion(int displayId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean resetMagnification(int displayId, boolean animate) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean setMagnificationScaleAndCenter(int displayId, float scale, float centerX, float centerY, boolean animate) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void setMagnificationCallbackEnabled(int displayId, boolean enabled) throws android.os.RemoteException
    {
    }
    @Override public boolean setSoftKeyboardShowMode(int showMode) throws android.os.RemoteException
    {
      return false;
    }
    @Override public int getSoftKeyboardShowMode() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setSoftKeyboardCallbackEnabled(boolean enabled) throws android.os.RemoteException
    {
    }
    @Override public boolean isAccessibilityButtonAvailable() throws android.os.RemoteException
    {
      return false;
    }
    @Override public void sendGesture(int sequence, android.content.pm.ParceledListSlice gestureSteps) throws android.os.RemoteException
    {
    }
    @Override public boolean isFingerprintGestureDetectionAvailable() throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.accessibilityservice.IAccessibilityServiceConnection
  {
    private static final java.lang.String DESCRIPTOR = "android.accessibilityservice.IAccessibilityServiceConnection";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.accessibilityservice.IAccessibilityServiceConnection interface,
     * generating a proxy if needed.
     */
    public static android.accessibilityservice.IAccessibilityServiceConnection asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.accessibilityservice.IAccessibilityServiceConnection))) {
        return ((android.accessibilityservice.IAccessibilityServiceConnection)iin);
      }
      return new android.accessibilityservice.IAccessibilityServiceConnection.Stub.Proxy(obj);
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
        case TRANSACTION_setServiceInfo:
        {
          data.enforceInterface(descriptor);
          android.accessibilityservice.AccessibilityServiceInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.accessibilityservice.AccessibilityServiceInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setServiceInfo(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_findAccessibilityNodeInfoByAccessibilityId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          int _arg2;
          _arg2 = data.readInt();
          android.view.accessibility.IAccessibilityInteractionConnectionCallback _arg3;
          _arg3 = android.view.accessibility.IAccessibilityInteractionConnectionCallback.Stub.asInterface(data.readStrongBinder());
          int _arg4;
          _arg4 = data.readInt();
          long _arg5;
          _arg5 = data.readLong();
          android.os.Bundle _arg6;
          if ((0!=data.readInt())) {
            _arg6 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg6 = null;
          }
          java.lang.String[] _result = this.findAccessibilityNodeInfoByAccessibilityId(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_findAccessibilityNodeInfosByText:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          android.view.accessibility.IAccessibilityInteractionConnectionCallback _arg4;
          _arg4 = android.view.accessibility.IAccessibilityInteractionConnectionCallback.Stub.asInterface(data.readStrongBinder());
          long _arg5;
          _arg5 = data.readLong();
          java.lang.String[] _result = this.findAccessibilityNodeInfosByText(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_findAccessibilityNodeInfosByViewId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          android.view.accessibility.IAccessibilityInteractionConnectionCallback _arg4;
          _arg4 = android.view.accessibility.IAccessibilityInteractionConnectionCallback.Stub.asInterface(data.readStrongBinder());
          long _arg5;
          _arg5 = data.readLong();
          java.lang.String[] _result = this.findAccessibilityNodeInfosByViewId(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_findFocus:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          android.view.accessibility.IAccessibilityInteractionConnectionCallback _arg4;
          _arg4 = android.view.accessibility.IAccessibilityInteractionConnectionCallback.Stub.asInterface(data.readStrongBinder());
          long _arg5;
          _arg5 = data.readLong();
          java.lang.String[] _result = this.findFocus(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_focusSearch:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          android.view.accessibility.IAccessibilityInteractionConnectionCallback _arg4;
          _arg4 = android.view.accessibility.IAccessibilityInteractionConnectionCallback.Stub.asInterface(data.readStrongBinder());
          long _arg5;
          _arg5 = data.readLong();
          java.lang.String[] _result = this.focusSearch(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_performAccessibilityAction:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          int _arg2;
          _arg2 = data.readInt();
          android.os.Bundle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          int _arg4;
          _arg4 = data.readInt();
          android.view.accessibility.IAccessibilityInteractionConnectionCallback _arg5;
          _arg5 = android.view.accessibility.IAccessibilityInteractionConnectionCallback.Stub.asInterface(data.readStrongBinder());
          long _arg6;
          _arg6 = data.readLong();
          boolean _result = this.performAccessibilityAction(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getWindow:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.view.accessibility.AccessibilityWindowInfo _result = this.getWindow(_arg0);
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
        case TRANSACTION_getWindows:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.view.accessibility.AccessibilityWindowInfo> _result = this.getWindows();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getServiceInfo:
        {
          data.enforceInterface(descriptor);
          android.accessibilityservice.AccessibilityServiceInfo _result = this.getServiceInfo();
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
        case TRANSACTION_performGlobalAction:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.performGlobalAction(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_disableSelf:
        {
          data.enforceInterface(descriptor);
          this.disableSelf();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setOnKeyEventResult:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          this.setOnKeyEventResult(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_getMagnificationScale:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          float _result = this.getMagnificationScale(_arg0);
          reply.writeNoException();
          reply.writeFloat(_result);
          return true;
        }
        case TRANSACTION_getMagnificationCenterX:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          float _result = this.getMagnificationCenterX(_arg0);
          reply.writeNoException();
          reply.writeFloat(_result);
          return true;
        }
        case TRANSACTION_getMagnificationCenterY:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          float _result = this.getMagnificationCenterY(_arg0);
          reply.writeNoException();
          reply.writeFloat(_result);
          return true;
        }
        case TRANSACTION_getMagnificationRegion:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.graphics.Region _result = this.getMagnificationRegion(_arg0);
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
        case TRANSACTION_resetMagnification:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          boolean _result = this.resetMagnification(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setMagnificationScaleAndCenter:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          float _arg1;
          _arg1 = data.readFloat();
          float _arg2;
          _arg2 = data.readFloat();
          float _arg3;
          _arg3 = data.readFloat();
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          boolean _result = this.setMagnificationScaleAndCenter(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setMagnificationCallbackEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setMagnificationCallbackEnabled(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setSoftKeyboardShowMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.setSoftKeyboardShowMode(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getSoftKeyboardShowMode:
        {
          data.enforceInterface(descriptor);
          int _result = this.getSoftKeyboardShowMode();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setSoftKeyboardCallbackEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setSoftKeyboardCallbackEnabled(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isAccessibilityButtonAvailable:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isAccessibilityButtonAvailable();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_sendGesture:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.content.pm.ParceledListSlice _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.sendGesture(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isFingerprintGestureDetectionAvailable:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isFingerprintGestureDetectionAvailable();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.accessibilityservice.IAccessibilityServiceConnection
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
      @Override public void setServiceInfo(android.accessibilityservice.AccessibilityServiceInfo info) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((info!=null)) {
            _data.writeInt(1);
            info.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setServiceInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setServiceInfo(info);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String[] findAccessibilityNodeInfoByAccessibilityId(int accessibilityWindowId, long accessibilityNodeId, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, int flags, long threadId, android.os.Bundle arguments) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(accessibilityWindowId);
          _data.writeLong(accessibilityNodeId);
          _data.writeInt(interactionId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(flags);
          _data.writeLong(threadId);
          if ((arguments!=null)) {
            _data.writeInt(1);
            arguments.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_findAccessibilityNodeInfoByAccessibilityId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().findAccessibilityNodeInfoByAccessibilityId(accessibilityWindowId, accessibilityNodeId, interactionId, callback, flags, threadId, arguments);
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String[] findAccessibilityNodeInfosByText(int accessibilityWindowId, long accessibilityNodeId, java.lang.String text, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, long threadId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(accessibilityWindowId);
          _data.writeLong(accessibilityNodeId);
          _data.writeString(text);
          _data.writeInt(interactionId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeLong(threadId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_findAccessibilityNodeInfosByText, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().findAccessibilityNodeInfosByText(accessibilityWindowId, accessibilityNodeId, text, interactionId, callback, threadId);
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String[] findAccessibilityNodeInfosByViewId(int accessibilityWindowId, long accessibilityNodeId, java.lang.String viewId, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, long threadId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(accessibilityWindowId);
          _data.writeLong(accessibilityNodeId);
          _data.writeString(viewId);
          _data.writeInt(interactionId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeLong(threadId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_findAccessibilityNodeInfosByViewId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().findAccessibilityNodeInfosByViewId(accessibilityWindowId, accessibilityNodeId, viewId, interactionId, callback, threadId);
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String[] findFocus(int accessibilityWindowId, long accessibilityNodeId, int focusType, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, long threadId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(accessibilityWindowId);
          _data.writeLong(accessibilityNodeId);
          _data.writeInt(focusType);
          _data.writeInt(interactionId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeLong(threadId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_findFocus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().findFocus(accessibilityWindowId, accessibilityNodeId, focusType, interactionId, callback, threadId);
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.lang.String[] focusSearch(int accessibilityWindowId, long accessibilityNodeId, int direction, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, long threadId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(accessibilityWindowId);
          _data.writeLong(accessibilityNodeId);
          _data.writeInt(direction);
          _data.writeInt(interactionId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeLong(threadId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_focusSearch, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().focusSearch(accessibilityWindowId, accessibilityNodeId, direction, interactionId, callback, threadId);
          }
          _reply.readException();
          _result = _reply.createStringArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public boolean performAccessibilityAction(int accessibilityWindowId, long accessibilityNodeId, int action, android.os.Bundle arguments, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, long threadId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(accessibilityWindowId);
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
          _data.writeLong(threadId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_performAccessibilityAction, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().performAccessibilityAction(accessibilityWindowId, accessibilityNodeId, action, arguments, interactionId, callback, threadId);
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
      @Override public android.view.accessibility.AccessibilityWindowInfo getWindow(int windowId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.view.accessibility.AccessibilityWindowInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(windowId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getWindow, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getWindow(windowId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.view.accessibility.AccessibilityWindowInfo.CREATOR.createFromParcel(_reply);
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
      @Override public java.util.List<android.view.accessibility.AccessibilityWindowInfo> getWindows() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.view.accessibility.AccessibilityWindowInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getWindows, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getWindows();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.view.accessibility.AccessibilityWindowInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.accessibilityservice.AccessibilityServiceInfo getServiceInfo() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.accessibilityservice.AccessibilityServiceInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getServiceInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getServiceInfo();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.accessibilityservice.AccessibilityServiceInfo.CREATOR.createFromParcel(_reply);
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
      @Override public boolean performGlobalAction(int action) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(action);
          boolean _status = mRemote.transact(Stub.TRANSACTION_performGlobalAction, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().performGlobalAction(action);
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
      @Override public void disableSelf() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableSelf, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableSelf();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setOnKeyEventResult(boolean handled, int sequence) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((handled)?(1):(0)));
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setOnKeyEventResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setOnKeyEventResult(handled, sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public float getMagnificationScale(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        float _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMagnificationScale, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMagnificationScale(displayId);
          }
          _reply.readException();
          _result = _reply.readFloat();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public float getMagnificationCenterX(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        float _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMagnificationCenterX, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMagnificationCenterX(displayId);
          }
          _reply.readException();
          _result = _reply.readFloat();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public float getMagnificationCenterY(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        float _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMagnificationCenterY, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMagnificationCenterY(displayId);
          }
          _reply.readException();
          _result = _reply.readFloat();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.graphics.Region getMagnificationRegion(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.graphics.Region _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMagnificationRegion, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMagnificationRegion(displayId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.graphics.Region.CREATOR.createFromParcel(_reply);
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
      @Override public boolean resetMagnification(int displayId, boolean animate) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeInt(((animate)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_resetMagnification, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().resetMagnification(displayId, animate);
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
      @Override public boolean setMagnificationScaleAndCenter(int displayId, float scale, float centerX, float centerY, boolean animate) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeFloat(scale);
          _data.writeFloat(centerX);
          _data.writeFloat(centerY);
          _data.writeInt(((animate)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMagnificationScaleAndCenter, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setMagnificationScaleAndCenter(displayId, scale, centerX, centerY, animate);
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
      @Override public void setMagnificationCallbackEnabled(int displayId, boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMagnificationCallbackEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setMagnificationCallbackEnabled(displayId, enabled);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean setSoftKeyboardShowMode(int showMode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(showMode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSoftKeyboardShowMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setSoftKeyboardShowMode(showMode);
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
      @Override public int getSoftKeyboardShowMode() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSoftKeyboardShowMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSoftKeyboardShowMode();
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setSoftKeyboardCallbackEnabled(boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSoftKeyboardCallbackEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSoftKeyboardCallbackEnabled(enabled);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isAccessibilityButtonAvailable() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isAccessibilityButtonAvailable, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isAccessibilityButtonAvailable();
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
      @Override public void sendGesture(int sequence, android.content.pm.ParceledListSlice gestureSteps) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sequence);
          if ((gestureSteps!=null)) {
            _data.writeInt(1);
            gestureSteps.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendGesture, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendGesture(sequence, gestureSteps);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isFingerprintGestureDetectionAvailable() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isFingerprintGestureDetectionAvailable, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isFingerprintGestureDetectionAvailable();
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
      public static android.accessibilityservice.IAccessibilityServiceConnection sDefaultImpl;
    }
    static final int TRANSACTION_setServiceInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_findAccessibilityNodeInfoByAccessibilityId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_findAccessibilityNodeInfosByText = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_findAccessibilityNodeInfosByViewId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_findFocus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_focusSearch = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_performAccessibilityAction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_getWindow = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_getWindows = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getServiceInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_performGlobalAction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_disableSelf = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_setOnKeyEventResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_getMagnificationScale = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_getMagnificationCenterX = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_getMagnificationCenterY = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_getMagnificationRegion = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_resetMagnification = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_setMagnificationScaleAndCenter = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_setMagnificationCallbackEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_setSoftKeyboardShowMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_getSoftKeyboardShowMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_setSoftKeyboardCallbackEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_isAccessibilityButtonAvailable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_sendGesture = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_isFingerprintGestureDetectionAvailable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    public static boolean setDefaultImpl(android.accessibilityservice.IAccessibilityServiceConnection impl) {
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
    public static android.accessibilityservice.IAccessibilityServiceConnection getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void setServiceInfo(android.accessibilityservice.AccessibilityServiceInfo info) throws android.os.RemoteException;
  public java.lang.String[] findAccessibilityNodeInfoByAccessibilityId(int accessibilityWindowId, long accessibilityNodeId, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, int flags, long threadId, android.os.Bundle arguments) throws android.os.RemoteException;
  public java.lang.String[] findAccessibilityNodeInfosByText(int accessibilityWindowId, long accessibilityNodeId, java.lang.String text, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, long threadId) throws android.os.RemoteException;
  public java.lang.String[] findAccessibilityNodeInfosByViewId(int accessibilityWindowId, long accessibilityNodeId, java.lang.String viewId, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, long threadId) throws android.os.RemoteException;
  public java.lang.String[] findFocus(int accessibilityWindowId, long accessibilityNodeId, int focusType, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, long threadId) throws android.os.RemoteException;
  public java.lang.String[] focusSearch(int accessibilityWindowId, long accessibilityNodeId, int direction, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, long threadId) throws android.os.RemoteException;
  public boolean performAccessibilityAction(int accessibilityWindowId, long accessibilityNodeId, int action, android.os.Bundle arguments, int interactionId, android.view.accessibility.IAccessibilityInteractionConnectionCallback callback, long threadId) throws android.os.RemoteException;
  public android.view.accessibility.AccessibilityWindowInfo getWindow(int windowId) throws android.os.RemoteException;
  public java.util.List<android.view.accessibility.AccessibilityWindowInfo> getWindows() throws android.os.RemoteException;
  public android.accessibilityservice.AccessibilityServiceInfo getServiceInfo() throws android.os.RemoteException;
  public boolean performGlobalAction(int action) throws android.os.RemoteException;
  public void disableSelf() throws android.os.RemoteException;
  public void setOnKeyEventResult(boolean handled, int sequence) throws android.os.RemoteException;
  public float getMagnificationScale(int displayId) throws android.os.RemoteException;
  public float getMagnificationCenterX(int displayId) throws android.os.RemoteException;
  public float getMagnificationCenterY(int displayId) throws android.os.RemoteException;
  public android.graphics.Region getMagnificationRegion(int displayId) throws android.os.RemoteException;
  public boolean resetMagnification(int displayId, boolean animate) throws android.os.RemoteException;
  public boolean setMagnificationScaleAndCenter(int displayId, float scale, float centerX, float centerY, boolean animate) throws android.os.RemoteException;
  public void setMagnificationCallbackEnabled(int displayId, boolean enabled) throws android.os.RemoteException;
  public boolean setSoftKeyboardShowMode(int showMode) throws android.os.RemoteException;
  public int getSoftKeyboardShowMode() throws android.os.RemoteException;
  public void setSoftKeyboardCallbackEnabled(boolean enabled) throws android.os.RemoteException;
  public boolean isAccessibilityButtonAvailable() throws android.os.RemoteException;
  public void sendGesture(int sequence, android.content.pm.ParceledListSlice gestureSteps) throws android.os.RemoteException;
  public boolean isFingerprintGestureDetectionAvailable() throws android.os.RemoteException;
}
