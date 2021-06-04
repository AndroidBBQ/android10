/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view.accessibility;
/**
 * Interface implemented by the AccessibilityManagerService called by
 * the AccessibilityManagers.
 *
 * @hide
 */
public interface IAccessibilityManager extends android.os.IInterface
{
  /** Default implementation for IAccessibilityManager. */
  public static class Default implements android.view.accessibility.IAccessibilityManager
  {
    @Override public void interrupt(int userId) throws android.os.RemoteException
    {
    }
    @Override public void sendAccessibilityEvent(android.view.accessibility.AccessibilityEvent uiEvent, int userId) throws android.os.RemoteException
    {
    }
    @Override public long addClient(android.view.accessibility.IAccessibilityManagerClient client, int userId) throws android.os.RemoteException
    {
      return 0L;
    }
    @Override public java.util.List<android.accessibilityservice.AccessibilityServiceInfo> getInstalledAccessibilityServiceList(int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.util.List<android.accessibilityservice.AccessibilityServiceInfo> getEnabledAccessibilityServiceList(int feedbackType, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int addAccessibilityInteractionConnection(android.view.IWindow windowToken, android.view.accessibility.IAccessibilityInteractionConnection connection, java.lang.String packageName, int userId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void removeAccessibilityInteractionConnection(android.view.IWindow windowToken) throws android.os.RemoteException
    {
    }
    @Override public void setPictureInPictureActionReplacingConnection(android.view.accessibility.IAccessibilityInteractionConnection connection) throws android.os.RemoteException
    {
    }
    @Override public void registerUiTestAutomationService(android.os.IBinder owner, android.accessibilityservice.IAccessibilityServiceClient client, android.accessibilityservice.AccessibilityServiceInfo info, int flags) throws android.os.RemoteException
    {
    }
    @Override public void unregisterUiTestAutomationService(android.accessibilityservice.IAccessibilityServiceClient client) throws android.os.RemoteException
    {
    }
    @Override public void temporaryEnableAccessibilityStateUntilKeyguardRemoved(android.content.ComponentName service, boolean touchExplorationEnabled) throws android.os.RemoteException
    {
    }
    @Override public android.os.IBinder getWindowToken(int windowId, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void notifyAccessibilityButtonClicked(int displayId) throws android.os.RemoteException
    {
    }
    @Override public void notifyAccessibilityButtonVisibilityChanged(boolean available) throws android.os.RemoteException
    {
    }
    // Requires Manifest.permission.MANAGE_ACCESSIBILITY

    @Override public void performAccessibilityShortcut() throws android.os.RemoteException
    {
    }
    // Requires Manifest.permission.MANAGE_ACCESSIBILITY

    @Override public java.lang.String getAccessibilityShortcutService() throws android.os.RemoteException
    {
      return null;
    }
    // System process only

    @Override public boolean sendFingerprintGesture(int gestureKeyCode) throws android.os.RemoteException
    {
      return false;
    }
    // System process only

    @Override public int getAccessibilityWindowId(android.os.IBinder windowToken) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public long getRecommendedTimeoutMillis() throws android.os.RemoteException
    {
      return 0L;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.accessibility.IAccessibilityManager
  {
    private static final java.lang.String DESCRIPTOR = "android.view.accessibility.IAccessibilityManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.accessibility.IAccessibilityManager interface,
     * generating a proxy if needed.
     */
    public static android.view.accessibility.IAccessibilityManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.accessibility.IAccessibilityManager))) {
        return ((android.view.accessibility.IAccessibilityManager)iin);
      }
      return new android.view.accessibility.IAccessibilityManager.Stub.Proxy(obj);
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
        case TRANSACTION_interrupt:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.interrupt(_arg0);
          return true;
        }
        case TRANSACTION_sendAccessibilityEvent:
        {
          data.enforceInterface(descriptor);
          android.view.accessibility.AccessibilityEvent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.accessibility.AccessibilityEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.sendAccessibilityEvent(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_addClient:
        {
          data.enforceInterface(descriptor);
          android.view.accessibility.IAccessibilityManagerClient _arg0;
          _arg0 = android.view.accessibility.IAccessibilityManagerClient.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          long _result = this.addClient(_arg0, _arg1);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_getInstalledAccessibilityServiceList:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<android.accessibilityservice.AccessibilityServiceInfo> _result = this.getInstalledAccessibilityServiceList(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getEnabledAccessibilityServiceList:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.util.List<android.accessibilityservice.AccessibilityServiceInfo> _result = this.getEnabledAccessibilityServiceList(_arg0, _arg1);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_addAccessibilityInteractionConnection:
        {
          data.enforceInterface(descriptor);
          android.view.IWindow _arg0;
          _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
          android.view.accessibility.IAccessibilityInteractionConnection _arg1;
          _arg1 = android.view.accessibility.IAccessibilityInteractionConnection.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          int _result = this.addAccessibilityInteractionConnection(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_removeAccessibilityInteractionConnection:
        {
          data.enforceInterface(descriptor);
          android.view.IWindow _arg0;
          _arg0 = android.view.IWindow.Stub.asInterface(data.readStrongBinder());
          this.removeAccessibilityInteractionConnection(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setPictureInPictureActionReplacingConnection:
        {
          data.enforceInterface(descriptor);
          android.view.accessibility.IAccessibilityInteractionConnection _arg0;
          _arg0 = android.view.accessibility.IAccessibilityInteractionConnection.Stub.asInterface(data.readStrongBinder());
          this.setPictureInPictureActionReplacingConnection(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerUiTestAutomationService:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.accessibilityservice.IAccessibilityServiceClient _arg1;
          _arg1 = android.accessibilityservice.IAccessibilityServiceClient.Stub.asInterface(data.readStrongBinder());
          android.accessibilityservice.AccessibilityServiceInfo _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.accessibilityservice.AccessibilityServiceInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          this.registerUiTestAutomationService(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterUiTestAutomationService:
        {
          data.enforceInterface(descriptor);
          android.accessibilityservice.IAccessibilityServiceClient _arg0;
          _arg0 = android.accessibilityservice.IAccessibilityServiceClient.Stub.asInterface(data.readStrongBinder());
          this.unregisterUiTestAutomationService(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_temporaryEnableAccessibilityStateUntilKeyguardRemoved:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.temporaryEnableAccessibilityStateUntilKeyguardRemoved(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getWindowToken:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.os.IBinder _result = this.getWindowToken(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStrongBinder(_result);
          return true;
        }
        case TRANSACTION_notifyAccessibilityButtonClicked:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.notifyAccessibilityButtonClicked(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyAccessibilityButtonVisibilityChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.notifyAccessibilityButtonVisibilityChanged(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_performAccessibilityShortcut:
        {
          data.enforceInterface(descriptor);
          this.performAccessibilityShortcut();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getAccessibilityShortcutService:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getAccessibilityShortcutService();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_sendFingerprintGesture:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.sendFingerprintGesture(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getAccessibilityWindowId:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _result = this.getAccessibilityWindowId(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getRecommendedTimeoutMillis:
        {
          data.enforceInterface(descriptor);
          long _result = this.getRecommendedTimeoutMillis();
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.accessibility.IAccessibilityManager
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
      @Override public void interrupt(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_interrupt, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().interrupt(userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void sendAccessibilityEvent(android.view.accessibility.AccessibilityEvent uiEvent, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((uiEvent!=null)) {
            _data.writeInt(1);
            uiEvent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendAccessibilityEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendAccessibilityEvent(uiEvent, userId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public long addClient(android.view.accessibility.IAccessibilityManagerClient client, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addClient, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addClient(client, userId);
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.util.List<android.accessibilityservice.AccessibilityServiceInfo> getInstalledAccessibilityServiceList(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.accessibilityservice.AccessibilityServiceInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getInstalledAccessibilityServiceList, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getInstalledAccessibilityServiceList(userId);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.accessibilityservice.AccessibilityServiceInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.util.List<android.accessibilityservice.AccessibilityServiceInfo> getEnabledAccessibilityServiceList(int feedbackType, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.accessibilityservice.AccessibilityServiceInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(feedbackType);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getEnabledAccessibilityServiceList, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getEnabledAccessibilityServiceList(feedbackType, userId);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.accessibilityservice.AccessibilityServiceInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int addAccessibilityInteractionConnection(android.view.IWindow windowToken, android.view.accessibility.IAccessibilityInteractionConnection connection, java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((windowToken!=null))?(windowToken.asBinder()):(null)));
          _data.writeStrongBinder((((connection!=null))?(connection.asBinder()):(null)));
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addAccessibilityInteractionConnection, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addAccessibilityInteractionConnection(windowToken, connection, packageName, userId);
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
      @Override public void removeAccessibilityInteractionConnection(android.view.IWindow windowToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((windowToken!=null))?(windowToken.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeAccessibilityInteractionConnection, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeAccessibilityInteractionConnection(windowToken);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setPictureInPictureActionReplacingConnection(android.view.accessibility.IAccessibilityInteractionConnection connection) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((connection!=null))?(connection.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPictureInPictureActionReplacingConnection, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPictureInPictureActionReplacingConnection(connection);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void registerUiTestAutomationService(android.os.IBinder owner, android.accessibilityservice.IAccessibilityServiceClient client, android.accessibilityservice.AccessibilityServiceInfo info, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(owner);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          if ((info!=null)) {
            _data.writeInt(1);
            info.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerUiTestAutomationService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerUiTestAutomationService(owner, client, info, flags);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterUiTestAutomationService(android.accessibilityservice.IAccessibilityServiceClient client) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterUiTestAutomationService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterUiTestAutomationService(client);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void temporaryEnableAccessibilityStateUntilKeyguardRemoved(android.content.ComponentName service, boolean touchExplorationEnabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((service!=null)) {
            _data.writeInt(1);
            service.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((touchExplorationEnabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_temporaryEnableAccessibilityStateUntilKeyguardRemoved, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().temporaryEnableAccessibilityStateUntilKeyguardRemoved(service, touchExplorationEnabled);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.os.IBinder getWindowToken(int windowId, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.IBinder _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(windowId);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getWindowToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getWindowToken(windowId, userId);
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
      @Override public void notifyAccessibilityButtonClicked(int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyAccessibilityButtonClicked, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyAccessibilityButtonClicked(displayId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyAccessibilityButtonVisibilityChanged(boolean available) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((available)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyAccessibilityButtonVisibilityChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyAccessibilityButtonVisibilityChanged(available);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Requires Manifest.permission.MANAGE_ACCESSIBILITY

      @Override public void performAccessibilityShortcut() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_performAccessibilityShortcut, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().performAccessibilityShortcut();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Requires Manifest.permission.MANAGE_ACCESSIBILITY

      @Override public java.lang.String getAccessibilityShortcutService() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAccessibilityShortcutService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAccessibilityShortcutService();
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // System process only

      @Override public boolean sendFingerprintGesture(int gestureKeyCode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(gestureKeyCode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendFingerprintGesture, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().sendFingerprintGesture(gestureKeyCode);
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
      // System process only

      @Override public int getAccessibilityWindowId(android.os.IBinder windowToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(windowToken);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAccessibilityWindowId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAccessibilityWindowId(windowToken);
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
      @Override public long getRecommendedTimeoutMillis() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRecommendedTimeoutMillis, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRecommendedTimeoutMillis();
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.view.accessibility.IAccessibilityManager sDefaultImpl;
    }
    static final int TRANSACTION_interrupt = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_sendAccessibilityEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_addClient = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getInstalledAccessibilityServiceList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getEnabledAccessibilityServiceList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_addAccessibilityInteractionConnection = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_removeAccessibilityInteractionConnection = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_setPictureInPictureActionReplacingConnection = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_registerUiTestAutomationService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_unregisterUiTestAutomationService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_temporaryEnableAccessibilityStateUntilKeyguardRemoved = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_getWindowToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_notifyAccessibilityButtonClicked = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_notifyAccessibilityButtonVisibilityChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_performAccessibilityShortcut = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_getAccessibilityShortcutService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_sendFingerprintGesture = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_getAccessibilityWindowId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_getRecommendedTimeoutMillis = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    public static boolean setDefaultImpl(android.view.accessibility.IAccessibilityManager impl) {
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
    public static android.view.accessibility.IAccessibilityManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void interrupt(int userId) throws android.os.RemoteException;
  public void sendAccessibilityEvent(android.view.accessibility.AccessibilityEvent uiEvent, int userId) throws android.os.RemoteException;
  public long addClient(android.view.accessibility.IAccessibilityManagerClient client, int userId) throws android.os.RemoteException;
  public java.util.List<android.accessibilityservice.AccessibilityServiceInfo> getInstalledAccessibilityServiceList(int userId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/view/accessibility/IAccessibilityManager.aidl:45:1:45:25")
  public java.util.List<android.accessibilityservice.AccessibilityServiceInfo> getEnabledAccessibilityServiceList(int feedbackType, int userId) throws android.os.RemoteException;
  public int addAccessibilityInteractionConnection(android.view.IWindow windowToken, android.view.accessibility.IAccessibilityInteractionConnection connection, java.lang.String packageName, int userId) throws android.os.RemoteException;
  public void removeAccessibilityInteractionConnection(android.view.IWindow windowToken) throws android.os.RemoteException;
  public void setPictureInPictureActionReplacingConnection(android.view.accessibility.IAccessibilityInteractionConnection connection) throws android.os.RemoteException;
  public void registerUiTestAutomationService(android.os.IBinder owner, android.accessibilityservice.IAccessibilityServiceClient client, android.accessibilityservice.AccessibilityServiceInfo info, int flags) throws android.os.RemoteException;
  public void unregisterUiTestAutomationService(android.accessibilityservice.IAccessibilityServiceClient client) throws android.os.RemoteException;
  public void temporaryEnableAccessibilityStateUntilKeyguardRemoved(android.content.ComponentName service, boolean touchExplorationEnabled) throws android.os.RemoteException;
  public android.os.IBinder getWindowToken(int windowId, int userId) throws android.os.RemoteException;
  public void notifyAccessibilityButtonClicked(int displayId) throws android.os.RemoteException;
  public void notifyAccessibilityButtonVisibilityChanged(boolean available) throws android.os.RemoteException;
  // Requires Manifest.permission.MANAGE_ACCESSIBILITY

  public void performAccessibilityShortcut() throws android.os.RemoteException;
  // Requires Manifest.permission.MANAGE_ACCESSIBILITY

  public java.lang.String getAccessibilityShortcutService() throws android.os.RemoteException;
  // System process only

  public boolean sendFingerprintGesture(int gestureKeyCode) throws android.os.RemoteException;
  // System process only

  public int getAccessibilityWindowId(android.os.IBinder windowToken) throws android.os.RemoteException;
  public long getRecommendedTimeoutMillis() throws android.os.RemoteException;
}
