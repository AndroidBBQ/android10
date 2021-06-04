/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.accessibilityservice;
/**
 * Top-level interface to an accessibility service component.
 *
 * @hide
 */
public interface IAccessibilityServiceClient extends android.os.IInterface
{
  /** Default implementation for IAccessibilityServiceClient. */
  public static class Default implements android.accessibilityservice.IAccessibilityServiceClient
  {
    @Override public void init(android.accessibilityservice.IAccessibilityServiceConnection connection, int connectionId, android.os.IBinder windowToken) throws android.os.RemoteException
    {
    }
    @Override public void onAccessibilityEvent(android.view.accessibility.AccessibilityEvent event, boolean serviceWantsEvent) throws android.os.RemoteException
    {
    }
    @Override public void onInterrupt() throws android.os.RemoteException
    {
    }
    @Override public void onGesture(int gesture) throws android.os.RemoteException
    {
    }
    @Override public void clearAccessibilityCache() throws android.os.RemoteException
    {
    }
    @Override public void onKeyEvent(android.view.KeyEvent event, int sequence) throws android.os.RemoteException
    {
    }
    @Override public void onMagnificationChanged(int displayId, android.graphics.Region region, float scale, float centerX, float centerY) throws android.os.RemoteException
    {
    }
    @Override public void onSoftKeyboardShowModeChanged(int showMode) throws android.os.RemoteException
    {
    }
    @Override public void onPerformGestureResult(int sequence, boolean completedSuccessfully) throws android.os.RemoteException
    {
    }
    @Override public void onFingerprintCapturingGesturesChanged(boolean capturing) throws android.os.RemoteException
    {
    }
    @Override public void onFingerprintGesture(int gesture) throws android.os.RemoteException
    {
    }
    @Override public void onAccessibilityButtonClicked() throws android.os.RemoteException
    {
    }
    @Override public void onAccessibilityButtonAvailabilityChanged(boolean available) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.accessibilityservice.IAccessibilityServiceClient
  {
    private static final java.lang.String DESCRIPTOR = "android.accessibilityservice.IAccessibilityServiceClient";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.accessibilityservice.IAccessibilityServiceClient interface,
     * generating a proxy if needed.
     */
    public static android.accessibilityservice.IAccessibilityServiceClient asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.accessibilityservice.IAccessibilityServiceClient))) {
        return ((android.accessibilityservice.IAccessibilityServiceClient)iin);
      }
      return new android.accessibilityservice.IAccessibilityServiceClient.Stub.Proxy(obj);
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
        case TRANSACTION_init:
        {
          data.enforceInterface(descriptor);
          android.accessibilityservice.IAccessibilityServiceConnection _arg0;
          _arg0 = android.accessibilityservice.IAccessibilityServiceConnection.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          android.os.IBinder _arg2;
          _arg2 = data.readStrongBinder();
          this.init(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onAccessibilityEvent:
        {
          data.enforceInterface(descriptor);
          android.view.accessibility.AccessibilityEvent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.accessibility.AccessibilityEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.onAccessibilityEvent(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onInterrupt:
        {
          data.enforceInterface(descriptor);
          this.onInterrupt();
          return true;
        }
        case TRANSACTION_onGesture:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onGesture(_arg0);
          return true;
        }
        case TRANSACTION_clearAccessibilityCache:
        {
          data.enforceInterface(descriptor);
          this.clearAccessibilityCache();
          return true;
        }
        case TRANSACTION_onKeyEvent:
        {
          data.enforceInterface(descriptor);
          android.view.KeyEvent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.KeyEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.onKeyEvent(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onMagnificationChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.graphics.Region _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.graphics.Region.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          float _arg2;
          _arg2 = data.readFloat();
          float _arg3;
          _arg3 = data.readFloat();
          float _arg4;
          _arg4 = data.readFloat();
          this.onMagnificationChanged(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_onSoftKeyboardShowModeChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onSoftKeyboardShowModeChanged(_arg0);
          return true;
        }
        case TRANSACTION_onPerformGestureResult:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.onPerformGestureResult(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onFingerprintCapturingGesturesChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onFingerprintCapturingGesturesChanged(_arg0);
          return true;
        }
        case TRANSACTION_onFingerprintGesture:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onFingerprintGesture(_arg0);
          return true;
        }
        case TRANSACTION_onAccessibilityButtonClicked:
        {
          data.enforceInterface(descriptor);
          this.onAccessibilityButtonClicked();
          return true;
        }
        case TRANSACTION_onAccessibilityButtonAvailabilityChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onAccessibilityButtonAvailabilityChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.accessibilityservice.IAccessibilityServiceClient
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
      @Override public void init(android.accessibilityservice.IAccessibilityServiceConnection connection, int connectionId, android.os.IBinder windowToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((connection!=null))?(connection.asBinder()):(null)));
          _data.writeInt(connectionId);
          _data.writeStrongBinder(windowToken);
          boolean _status = mRemote.transact(Stub.TRANSACTION_init, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().init(connection, connectionId, windowToken);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onAccessibilityEvent(android.view.accessibility.AccessibilityEvent event, boolean serviceWantsEvent) throws android.os.RemoteException
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
          _data.writeInt(((serviceWantsEvent)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAccessibilityEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAccessibilityEvent(event, serviceWantsEvent);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onInterrupt() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onInterrupt, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onInterrupt();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onGesture(int gesture) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(gesture);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGesture, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGesture(gesture);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void clearAccessibilityCache() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearAccessibilityCache, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearAccessibilityCache();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onKeyEvent(android.view.KeyEvent event, int sequence) throws android.os.RemoteException
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
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onKeyEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onKeyEvent(event, sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onMagnificationChanged(int displayId, android.graphics.Region region, float scale, float centerX, float centerY) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          if ((region!=null)) {
            _data.writeInt(1);
            region.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeFloat(scale);
          _data.writeFloat(centerX);
          _data.writeFloat(centerY);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onMagnificationChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onMagnificationChanged(displayId, region, scale, centerX, centerY);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSoftKeyboardShowModeChanged(int showMode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(showMode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSoftKeyboardShowModeChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSoftKeyboardShowModeChanged(showMode);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPerformGestureResult(int sequence, boolean completedSuccessfully) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sequence);
          _data.writeInt(((completedSuccessfully)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPerformGestureResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPerformGestureResult(sequence, completedSuccessfully);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onFingerprintCapturingGesturesChanged(boolean capturing) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((capturing)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onFingerprintCapturingGesturesChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onFingerprintCapturingGesturesChanged(capturing);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onFingerprintGesture(int gesture) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(gesture);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onFingerprintGesture, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onFingerprintGesture(gesture);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onAccessibilityButtonClicked() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAccessibilityButtonClicked, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAccessibilityButtonClicked();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onAccessibilityButtonAvailabilityChanged(boolean available) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((available)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAccessibilityButtonAvailabilityChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAccessibilityButtonAvailabilityChanged(available);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.accessibilityservice.IAccessibilityServiceClient sDefaultImpl;
    }
    static final int TRANSACTION_init = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onAccessibilityEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onInterrupt = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onGesture = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_clearAccessibilityCache = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onKeyEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_onMagnificationChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_onSoftKeyboardShowModeChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_onPerformGestureResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_onFingerprintCapturingGesturesChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_onFingerprintGesture = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_onAccessibilityButtonClicked = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_onAccessibilityButtonAvailabilityChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    public static boolean setDefaultImpl(android.accessibilityservice.IAccessibilityServiceClient impl) {
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
    public static android.accessibilityservice.IAccessibilityServiceClient getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void init(android.accessibilityservice.IAccessibilityServiceConnection connection, int connectionId, android.os.IBinder windowToken) throws android.os.RemoteException;
  public void onAccessibilityEvent(android.view.accessibility.AccessibilityEvent event, boolean serviceWantsEvent) throws android.os.RemoteException;
  public void onInterrupt() throws android.os.RemoteException;
  public void onGesture(int gesture) throws android.os.RemoteException;
  public void clearAccessibilityCache() throws android.os.RemoteException;
  public void onKeyEvent(android.view.KeyEvent event, int sequence) throws android.os.RemoteException;
  public void onMagnificationChanged(int displayId, android.graphics.Region region, float scale, float centerX, float centerY) throws android.os.RemoteException;
  public void onSoftKeyboardShowModeChanged(int showMode) throws android.os.RemoteException;
  public void onPerformGestureResult(int sequence, boolean completedSuccessfully) throws android.os.RemoteException;
  public void onFingerprintCapturingGesturesChanged(boolean capturing) throws android.os.RemoteException;
  public void onFingerprintGesture(int gesture) throws android.os.RemoteException;
  public void onAccessibilityButtonClicked() throws android.os.RemoteException;
  public void onAccessibilityButtonAvailabilityChanged(boolean available) throws android.os.RemoteException;
}
