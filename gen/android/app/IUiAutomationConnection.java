/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app;
/**
 * This interface contains privileged operations a shell program can perform
 * on behalf of an instrumentation that it runs. These operations require
 * special permissions which the shell user has but the instrumentation does
 * not. Running privileged operations by the shell user on behalf of an
 * instrumentation is needed for running UiTestCases.
 *
 * {@hide}
 */
public interface IUiAutomationConnection extends android.os.IInterface
{
  /** Default implementation for IUiAutomationConnection. */
  public static class Default implements android.app.IUiAutomationConnection
  {
    @Override public void connect(android.accessibilityservice.IAccessibilityServiceClient client, int flags) throws android.os.RemoteException
    {
    }
    @Override public void disconnect() throws android.os.RemoteException
    {
    }
    @Override public boolean injectInputEvent(android.view.InputEvent event, boolean sync) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void syncInputTransactions() throws android.os.RemoteException
    {
    }
    @Override public boolean setRotation(int rotation) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.graphics.Bitmap takeScreenshot(android.graphics.Rect crop, int rotation) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean clearWindowContentFrameStats(int windowId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public android.view.WindowContentFrameStats getWindowContentFrameStats(int windowId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void clearWindowAnimationFrameStats() throws android.os.RemoteException
    {
    }
    @Override public android.view.WindowAnimationFrameStats getWindowAnimationFrameStats() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void executeShellCommand(java.lang.String command, android.os.ParcelFileDescriptor sink, android.os.ParcelFileDescriptor source) throws android.os.RemoteException
    {
    }
    @Override public void grantRuntimePermission(java.lang.String packageName, java.lang.String permission, int userId) throws android.os.RemoteException
    {
    }
    @Override public void revokeRuntimePermission(java.lang.String packageName, java.lang.String permission, int userId) throws android.os.RemoteException
    {
    }
    @Override public void adoptShellPermissionIdentity(int uid, java.lang.String[] permissions) throws android.os.RemoteException
    {
    }
    @Override public void dropShellPermissionIdentity() throws android.os.RemoteException
    {
    }
    // Called from the system process.

    @Override public void shutdown() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.IUiAutomationConnection
  {
    private static final java.lang.String DESCRIPTOR = "android.app.IUiAutomationConnection";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.IUiAutomationConnection interface,
     * generating a proxy if needed.
     */
    public static android.app.IUiAutomationConnection asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.IUiAutomationConnection))) {
        return ((android.app.IUiAutomationConnection)iin);
      }
      return new android.app.IUiAutomationConnection.Stub.Proxy(obj);
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
        case TRANSACTION_connect:
        {
          data.enforceInterface(descriptor);
          android.accessibilityservice.IAccessibilityServiceClient _arg0;
          _arg0 = android.accessibilityservice.IAccessibilityServiceClient.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.connect(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_disconnect:
        {
          data.enforceInterface(descriptor);
          this.disconnect();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_injectInputEvent:
        {
          data.enforceInterface(descriptor);
          android.view.InputEvent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.InputEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          boolean _result = this.injectInputEvent(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_syncInputTransactions:
        {
          data.enforceInterface(descriptor);
          this.syncInputTransactions();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setRotation:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.setRotation(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_takeScreenshot:
        {
          data.enforceInterface(descriptor);
          android.graphics.Rect _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          android.graphics.Bitmap _result = this.takeScreenshot(_arg0, _arg1);
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
        case TRANSACTION_clearWindowContentFrameStats:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.clearWindowContentFrameStats(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getWindowContentFrameStats:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.view.WindowContentFrameStats _result = this.getWindowContentFrameStats(_arg0);
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
        case TRANSACTION_clearWindowAnimationFrameStats:
        {
          data.enforceInterface(descriptor);
          this.clearWindowAnimationFrameStats();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getWindowAnimationFrameStats:
        {
          data.enforceInterface(descriptor);
          android.view.WindowAnimationFrameStats _result = this.getWindowAnimationFrameStats();
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
        case TRANSACTION_executeShellCommand:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.ParcelFileDescriptor _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.os.ParcelFileDescriptor _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.executeShellCommand(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_grantRuntimePermission:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          this.grantRuntimePermission(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_revokeRuntimePermission:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          this.revokeRuntimePermission(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_adoptShellPermissionIdentity:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String[] _arg1;
          _arg1 = data.createStringArray();
          this.adoptShellPermissionIdentity(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_dropShellPermissionIdentity:
        {
          data.enforceInterface(descriptor);
          this.dropShellPermissionIdentity();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_shutdown:
        {
          data.enforceInterface(descriptor);
          this.shutdown();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.IUiAutomationConnection
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
      @Override public void connect(android.accessibilityservice.IAccessibilityServiceClient client, int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_connect, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().connect(client, flags);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void disconnect() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disconnect, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disconnect();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean injectInputEvent(android.view.InputEvent event, boolean sync) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((event!=null)) {
            _data.writeInt(1);
            event.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((sync)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_injectInputEvent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().injectInputEvent(event, sync);
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
      @Override public void syncInputTransactions() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_syncInputTransactions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().syncInputTransactions();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean setRotation(int rotation) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rotation);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRotation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setRotation(rotation);
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
      @Override public android.graphics.Bitmap takeScreenshot(android.graphics.Rect crop, int rotation) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.graphics.Bitmap _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((crop!=null)) {
            _data.writeInt(1);
            crop.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(rotation);
          boolean _status = mRemote.transact(Stub.TRANSACTION_takeScreenshot, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().takeScreenshot(crop, rotation);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.graphics.Bitmap.CREATOR.createFromParcel(_reply);
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
      @Override public boolean clearWindowContentFrameStats(int windowId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(windowId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearWindowContentFrameStats, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().clearWindowContentFrameStats(windowId);
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
      @Override public android.view.WindowContentFrameStats getWindowContentFrameStats(int windowId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.view.WindowContentFrameStats _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(windowId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getWindowContentFrameStats, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getWindowContentFrameStats(windowId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.view.WindowContentFrameStats.CREATOR.createFromParcel(_reply);
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
      @Override public void clearWindowAnimationFrameStats() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearWindowAnimationFrameStats, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearWindowAnimationFrameStats();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.view.WindowAnimationFrameStats getWindowAnimationFrameStats() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.view.WindowAnimationFrameStats _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getWindowAnimationFrameStats, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getWindowAnimationFrameStats();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.view.WindowAnimationFrameStats.CREATOR.createFromParcel(_reply);
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
      @Override public void executeShellCommand(java.lang.String command, android.os.ParcelFileDescriptor sink, android.os.ParcelFileDescriptor source) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(command);
          if ((sink!=null)) {
            _data.writeInt(1);
            sink.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((source!=null)) {
            _data.writeInt(1);
            source.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_executeShellCommand, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().executeShellCommand(command, sink, source);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void grantRuntimePermission(java.lang.String packageName, java.lang.String permission, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeString(permission);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_grantRuntimePermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().grantRuntimePermission(packageName, permission, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void revokeRuntimePermission(java.lang.String packageName, java.lang.String permission, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeString(permission);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_revokeRuntimePermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().revokeRuntimePermission(packageName, permission, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void adoptShellPermissionIdentity(int uid, java.lang.String[] permissions) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeStringArray(permissions);
          boolean _status = mRemote.transact(Stub.TRANSACTION_adoptShellPermissionIdentity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().adoptShellPermissionIdentity(uid, permissions);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void dropShellPermissionIdentity() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dropShellPermissionIdentity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dropShellPermissionIdentity();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Called from the system process.

      @Override public void shutdown() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_shutdown, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().shutdown();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.app.IUiAutomationConnection sDefaultImpl;
    }
    static final int TRANSACTION_connect = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_disconnect = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_injectInputEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_syncInputTransactions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_setRotation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_takeScreenshot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_clearWindowContentFrameStats = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_getWindowContentFrameStats = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_clearWindowAnimationFrameStats = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getWindowAnimationFrameStats = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_executeShellCommand = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_grantRuntimePermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_revokeRuntimePermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_adoptShellPermissionIdentity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_dropShellPermissionIdentity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_shutdown = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    public static boolean setDefaultImpl(android.app.IUiAutomationConnection impl) {
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
    public static android.app.IUiAutomationConnection getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void connect(android.accessibilityservice.IAccessibilityServiceClient client, int flags) throws android.os.RemoteException;
  public void disconnect() throws android.os.RemoteException;
  public boolean injectInputEvent(android.view.InputEvent event, boolean sync) throws android.os.RemoteException;
  public void syncInputTransactions() throws android.os.RemoteException;
  public boolean setRotation(int rotation) throws android.os.RemoteException;
  public android.graphics.Bitmap takeScreenshot(android.graphics.Rect crop, int rotation) throws android.os.RemoteException;
  public boolean clearWindowContentFrameStats(int windowId) throws android.os.RemoteException;
  public android.view.WindowContentFrameStats getWindowContentFrameStats(int windowId) throws android.os.RemoteException;
  public void clearWindowAnimationFrameStats() throws android.os.RemoteException;
  public android.view.WindowAnimationFrameStats getWindowAnimationFrameStats() throws android.os.RemoteException;
  public void executeShellCommand(java.lang.String command, android.os.ParcelFileDescriptor sink, android.os.ParcelFileDescriptor source) throws android.os.RemoteException;
  public void grantRuntimePermission(java.lang.String packageName, java.lang.String permission, int userId) throws android.os.RemoteException;
  public void revokeRuntimePermission(java.lang.String packageName, java.lang.String permission, int userId) throws android.os.RemoteException;
  public void adoptShellPermissionIdentity(int uid, java.lang.String[] permissions) throws android.os.RemoteException;
  public void dropShellPermissionIdentity() throws android.os.RemoteException;
  // Called from the system process.

  public void shutdown() throws android.os.RemoteException;
}
