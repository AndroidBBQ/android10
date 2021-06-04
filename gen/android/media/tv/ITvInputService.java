/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media.tv;
/**
 * Top-level interface to a TV input component (implemented in a Service).
 * @hide
 */
public interface ITvInputService extends android.os.IInterface
{
  /** Default implementation for ITvInputService. */
  public static class Default implements android.media.tv.ITvInputService
  {
    @Override public void registerCallback(android.media.tv.ITvInputServiceCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void unregisterCallback(android.media.tv.ITvInputServiceCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void createSession(android.view.InputChannel channel, android.media.tv.ITvInputSessionCallback callback, java.lang.String inputId) throws android.os.RemoteException
    {
    }
    @Override public void createRecordingSession(android.media.tv.ITvInputSessionCallback callback, java.lang.String inputId) throws android.os.RemoteException
    {
    }
    // For hardware TvInputService

    @Override public void notifyHardwareAdded(android.media.tv.TvInputHardwareInfo hardwareInfo) throws android.os.RemoteException
    {
    }
    @Override public void notifyHardwareRemoved(android.media.tv.TvInputHardwareInfo hardwareInfo) throws android.os.RemoteException
    {
    }
    @Override public void notifyHdmiDeviceAdded(android.hardware.hdmi.HdmiDeviceInfo deviceInfo) throws android.os.RemoteException
    {
    }
    @Override public void notifyHdmiDeviceRemoved(android.hardware.hdmi.HdmiDeviceInfo deviceInfo) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.tv.ITvInputService
  {
    private static final java.lang.String DESCRIPTOR = "android.media.tv.ITvInputService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.tv.ITvInputService interface,
     * generating a proxy if needed.
     */
    public static android.media.tv.ITvInputService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.tv.ITvInputService))) {
        return ((android.media.tv.ITvInputService)iin);
      }
      return new android.media.tv.ITvInputService.Stub.Proxy(obj);
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
        case TRANSACTION_registerCallback:
        {
          data.enforceInterface(descriptor);
          android.media.tv.ITvInputServiceCallback _arg0;
          _arg0 = android.media.tv.ITvInputServiceCallback.Stub.asInterface(data.readStrongBinder());
          this.registerCallback(_arg0);
          return true;
        }
        case TRANSACTION_unregisterCallback:
        {
          data.enforceInterface(descriptor);
          android.media.tv.ITvInputServiceCallback _arg0;
          _arg0 = android.media.tv.ITvInputServiceCallback.Stub.asInterface(data.readStrongBinder());
          this.unregisterCallback(_arg0);
          return true;
        }
        case TRANSACTION_createSession:
        {
          data.enforceInterface(descriptor);
          android.view.InputChannel _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.InputChannel.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.media.tv.ITvInputSessionCallback _arg1;
          _arg1 = android.media.tv.ITvInputSessionCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.createSession(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_createRecordingSession:
        {
          data.enforceInterface(descriptor);
          android.media.tv.ITvInputSessionCallback _arg0;
          _arg0 = android.media.tv.ITvInputSessionCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.createRecordingSession(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_notifyHardwareAdded:
        {
          data.enforceInterface(descriptor);
          android.media.tv.TvInputHardwareInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.tv.TvInputHardwareInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.notifyHardwareAdded(_arg0);
          return true;
        }
        case TRANSACTION_notifyHardwareRemoved:
        {
          data.enforceInterface(descriptor);
          android.media.tv.TvInputHardwareInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.tv.TvInputHardwareInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.notifyHardwareRemoved(_arg0);
          return true;
        }
        case TRANSACTION_notifyHdmiDeviceAdded:
        {
          data.enforceInterface(descriptor);
          android.hardware.hdmi.HdmiDeviceInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.hdmi.HdmiDeviceInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.notifyHdmiDeviceAdded(_arg0);
          return true;
        }
        case TRANSACTION_notifyHdmiDeviceRemoved:
        {
          data.enforceInterface(descriptor);
          android.hardware.hdmi.HdmiDeviceInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.hdmi.HdmiDeviceInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.notifyHdmiDeviceRemoved(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.tv.ITvInputService
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
      @Override public void registerCallback(android.media.tv.ITvInputServiceCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerCallback, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerCallback(callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void unregisterCallback(android.media.tv.ITvInputServiceCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterCallback, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterCallback(callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void createSession(android.view.InputChannel channel, android.media.tv.ITvInputSessionCallback callback, java.lang.String inputId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((channel!=null)) {
            _data.writeInt(1);
            channel.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeString(inputId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createSession, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().createSession(channel, callback, inputId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void createRecordingSession(android.media.tv.ITvInputSessionCallback callback, java.lang.String inputId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeString(inputId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createRecordingSession, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().createRecordingSession(callback, inputId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // For hardware TvInputService

      @Override public void notifyHardwareAdded(android.media.tv.TvInputHardwareInfo hardwareInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((hardwareInfo!=null)) {
            _data.writeInt(1);
            hardwareInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyHardwareAdded, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyHardwareAdded(hardwareInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void notifyHardwareRemoved(android.media.tv.TvInputHardwareInfo hardwareInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((hardwareInfo!=null)) {
            _data.writeInt(1);
            hardwareInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyHardwareRemoved, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyHardwareRemoved(hardwareInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void notifyHdmiDeviceAdded(android.hardware.hdmi.HdmiDeviceInfo deviceInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((deviceInfo!=null)) {
            _data.writeInt(1);
            deviceInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyHdmiDeviceAdded, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyHdmiDeviceAdded(deviceInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void notifyHdmiDeviceRemoved(android.hardware.hdmi.HdmiDeviceInfo deviceInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((deviceInfo!=null)) {
            _data.writeInt(1);
            deviceInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyHdmiDeviceRemoved, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyHdmiDeviceRemoved(deviceInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.media.tv.ITvInputService sDefaultImpl;
    }
    static final int TRANSACTION_registerCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_unregisterCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_createSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_createRecordingSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_notifyHardwareAdded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_notifyHardwareRemoved = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_notifyHdmiDeviceAdded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_notifyHdmiDeviceRemoved = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    public static boolean setDefaultImpl(android.media.tv.ITvInputService impl) {
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
    public static android.media.tv.ITvInputService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void registerCallback(android.media.tv.ITvInputServiceCallback callback) throws android.os.RemoteException;
  public void unregisterCallback(android.media.tv.ITvInputServiceCallback callback) throws android.os.RemoteException;
  public void createSession(android.view.InputChannel channel, android.media.tv.ITvInputSessionCallback callback, java.lang.String inputId) throws android.os.RemoteException;
  public void createRecordingSession(android.media.tv.ITvInputSessionCallback callback, java.lang.String inputId) throws android.os.RemoteException;
  // For hardware TvInputService

  public void notifyHardwareAdded(android.media.tv.TvInputHardwareInfo hardwareInfo) throws android.os.RemoteException;
  public void notifyHardwareRemoved(android.media.tv.TvInputHardwareInfo hardwareInfo) throws android.os.RemoteException;
  public void notifyHdmiDeviceAdded(android.hardware.hdmi.HdmiDeviceInfo deviceInfo) throws android.os.RemoteException;
  public void notifyHdmiDeviceRemoved(android.hardware.hdmi.HdmiDeviceInfo deviceInfo) throws android.os.RemoteException;
}
