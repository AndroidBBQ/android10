/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.hdmi;
/**
 * Binder interface that clients running in the application process
 * will use to perform HDMI-CEC features by communicating with other devices
 * on the bus.
 *
 * @hide
 */
public interface IHdmiControlService extends android.os.IInterface
{
  /** Default implementation for IHdmiControlService. */
  public static class Default implements android.hardware.hdmi.IHdmiControlService
  {
    @Override public int[] getSupportedTypes() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.hardware.hdmi.HdmiDeviceInfo getActiveSource() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void oneTouchPlay(android.hardware.hdmi.IHdmiControlCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void queryDisplayStatus(android.hardware.hdmi.IHdmiControlCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void addHotplugEventListener(android.hardware.hdmi.IHdmiHotplugEventListener listener) throws android.os.RemoteException
    {
    }
    @Override public void removeHotplugEventListener(android.hardware.hdmi.IHdmiHotplugEventListener listener) throws android.os.RemoteException
    {
    }
    @Override public void addDeviceEventListener(android.hardware.hdmi.IHdmiDeviceEventListener listener) throws android.os.RemoteException
    {
    }
    @Override public void deviceSelect(int deviceId, android.hardware.hdmi.IHdmiControlCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void portSelect(int portId, android.hardware.hdmi.IHdmiControlCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void sendKeyEvent(int deviceType, int keyCode, boolean isPressed) throws android.os.RemoteException
    {
    }
    @Override public void sendVolumeKeyEvent(int deviceType, int keyCode, boolean isPressed) throws android.os.RemoteException
    {
    }
    @Override public java.util.List<android.hardware.hdmi.HdmiPortInfo> getPortInfo() throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean canChangeSystemAudioMode() throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean getSystemAudioMode() throws android.os.RemoteException
    {
      return false;
    }
    @Override public int getPhysicalAddress() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setSystemAudioMode(boolean enabled, android.hardware.hdmi.IHdmiControlCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void addSystemAudioModeChangeListener(android.hardware.hdmi.IHdmiSystemAudioModeChangeListener listener) throws android.os.RemoteException
    {
    }
    @Override public void removeSystemAudioModeChangeListener(android.hardware.hdmi.IHdmiSystemAudioModeChangeListener listener) throws android.os.RemoteException
    {
    }
    @Override public void setArcMode(boolean enabled) throws android.os.RemoteException
    {
    }
    @Override public void setProhibitMode(boolean enabled) throws android.os.RemoteException
    {
    }
    @Override public void setSystemAudioVolume(int oldIndex, int newIndex, int maxIndex) throws android.os.RemoteException
    {
    }
    @Override public void setSystemAudioMute(boolean mute) throws android.os.RemoteException
    {
    }
    @Override public void setInputChangeListener(android.hardware.hdmi.IHdmiInputChangeListener listener) throws android.os.RemoteException
    {
    }
    @Override public java.util.List<android.hardware.hdmi.HdmiDeviceInfo> getInputDevices() throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.util.List<android.hardware.hdmi.HdmiDeviceInfo> getDeviceList() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void powerOffRemoteDevice(int logicalAddress, int powerStatus) throws android.os.RemoteException
    {
    }
    @Override public void powerOnRemoteDevice(int logicalAddress, int powerStatus) throws android.os.RemoteException
    {
    }
    @Override public void askRemoteDeviceToBecomeActiveSource(int physicalAddress) throws android.os.RemoteException
    {
    }
    @Override public void sendVendorCommand(int deviceType, int targetAddress, byte[] params, boolean hasVendorId) throws android.os.RemoteException
    {
    }
    @Override public void addVendorCommandListener(android.hardware.hdmi.IHdmiVendorCommandListener listener, int deviceType) throws android.os.RemoteException
    {
    }
    @Override public void sendStandby(int deviceType, int deviceId) throws android.os.RemoteException
    {
    }
    @Override public void setHdmiRecordListener(android.hardware.hdmi.IHdmiRecordListener callback) throws android.os.RemoteException
    {
    }
    @Override public void startOneTouchRecord(int recorderAddress, byte[] recordSource) throws android.os.RemoteException
    {
    }
    @Override public void stopOneTouchRecord(int recorderAddress) throws android.os.RemoteException
    {
    }
    @Override public void startTimerRecording(int recorderAddress, int sourceType, byte[] recordSource) throws android.os.RemoteException
    {
    }
    @Override public void clearTimerRecording(int recorderAddress, int sourceType, byte[] recordSource) throws android.os.RemoteException
    {
    }
    @Override public void sendMhlVendorCommand(int portId, int offset, int length, byte[] data) throws android.os.RemoteException
    {
    }
    @Override public void addHdmiMhlVendorCommandListener(android.hardware.hdmi.IHdmiMhlVendorCommandListener listener) throws android.os.RemoteException
    {
    }
    @Override public void setStandbyMode(boolean isStandbyModeOn) throws android.os.RemoteException
    {
    }
    @Override public void reportAudioStatus(int deviceType, int volume, int maxVolume, boolean isMute) throws android.os.RemoteException
    {
    }
    @Override public void setSystemAudioModeOnForAudioOnlySource() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.hdmi.IHdmiControlService
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.hdmi.IHdmiControlService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.hdmi.IHdmiControlService interface,
     * generating a proxy if needed.
     */
    public static android.hardware.hdmi.IHdmiControlService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.hdmi.IHdmiControlService))) {
        return ((android.hardware.hdmi.IHdmiControlService)iin);
      }
      return new android.hardware.hdmi.IHdmiControlService.Stub.Proxy(obj);
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
        case TRANSACTION_getSupportedTypes:
        {
          data.enforceInterface(descriptor);
          int[] _result = this.getSupportedTypes();
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_getActiveSource:
        {
          data.enforceInterface(descriptor);
          android.hardware.hdmi.HdmiDeviceInfo _result = this.getActiveSource();
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
        case TRANSACTION_oneTouchPlay:
        {
          data.enforceInterface(descriptor);
          android.hardware.hdmi.IHdmiControlCallback _arg0;
          _arg0 = android.hardware.hdmi.IHdmiControlCallback.Stub.asInterface(data.readStrongBinder());
          this.oneTouchPlay(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_queryDisplayStatus:
        {
          data.enforceInterface(descriptor);
          android.hardware.hdmi.IHdmiControlCallback _arg0;
          _arg0 = android.hardware.hdmi.IHdmiControlCallback.Stub.asInterface(data.readStrongBinder());
          this.queryDisplayStatus(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addHotplugEventListener:
        {
          data.enforceInterface(descriptor);
          android.hardware.hdmi.IHdmiHotplugEventListener _arg0;
          _arg0 = android.hardware.hdmi.IHdmiHotplugEventListener.Stub.asInterface(data.readStrongBinder());
          this.addHotplugEventListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeHotplugEventListener:
        {
          data.enforceInterface(descriptor);
          android.hardware.hdmi.IHdmiHotplugEventListener _arg0;
          _arg0 = android.hardware.hdmi.IHdmiHotplugEventListener.Stub.asInterface(data.readStrongBinder());
          this.removeHotplugEventListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addDeviceEventListener:
        {
          data.enforceInterface(descriptor);
          android.hardware.hdmi.IHdmiDeviceEventListener _arg0;
          _arg0 = android.hardware.hdmi.IHdmiDeviceEventListener.Stub.asInterface(data.readStrongBinder());
          this.addDeviceEventListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_deviceSelect:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.hardware.hdmi.IHdmiControlCallback _arg1;
          _arg1 = android.hardware.hdmi.IHdmiControlCallback.Stub.asInterface(data.readStrongBinder());
          this.deviceSelect(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_portSelect:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.hardware.hdmi.IHdmiControlCallback _arg1;
          _arg1 = android.hardware.hdmi.IHdmiControlCallback.Stub.asInterface(data.readStrongBinder());
          this.portSelect(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_sendKeyEvent:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.sendKeyEvent(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_sendVolumeKeyEvent:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.sendVolumeKeyEvent(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getPortInfo:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.hardware.hdmi.HdmiPortInfo> _result = this.getPortInfo();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_canChangeSystemAudioMode:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.canChangeSystemAudioMode();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getSystemAudioMode:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.getSystemAudioMode();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getPhysicalAddress:
        {
          data.enforceInterface(descriptor);
          int _result = this.getPhysicalAddress();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setSystemAudioMode:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          android.hardware.hdmi.IHdmiControlCallback _arg1;
          _arg1 = android.hardware.hdmi.IHdmiControlCallback.Stub.asInterface(data.readStrongBinder());
          this.setSystemAudioMode(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addSystemAudioModeChangeListener:
        {
          data.enforceInterface(descriptor);
          android.hardware.hdmi.IHdmiSystemAudioModeChangeListener _arg0;
          _arg0 = android.hardware.hdmi.IHdmiSystemAudioModeChangeListener.Stub.asInterface(data.readStrongBinder());
          this.addSystemAudioModeChangeListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeSystemAudioModeChangeListener:
        {
          data.enforceInterface(descriptor);
          android.hardware.hdmi.IHdmiSystemAudioModeChangeListener _arg0;
          _arg0 = android.hardware.hdmi.IHdmiSystemAudioModeChangeListener.Stub.asInterface(data.readStrongBinder());
          this.removeSystemAudioModeChangeListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setArcMode:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setArcMode(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setProhibitMode:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setProhibitMode(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setSystemAudioVolume:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.setSystemAudioVolume(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setSystemAudioMute:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setSystemAudioMute(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setInputChangeListener:
        {
          data.enforceInterface(descriptor);
          android.hardware.hdmi.IHdmiInputChangeListener _arg0;
          _arg0 = android.hardware.hdmi.IHdmiInputChangeListener.Stub.asInterface(data.readStrongBinder());
          this.setInputChangeListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getInputDevices:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.hardware.hdmi.HdmiDeviceInfo> _result = this.getInputDevices();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getDeviceList:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.hardware.hdmi.HdmiDeviceInfo> _result = this.getDeviceList();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_powerOffRemoteDevice:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.powerOffRemoteDevice(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_powerOnRemoteDevice:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.powerOnRemoteDevice(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_askRemoteDeviceToBecomeActiveSource:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.askRemoteDeviceToBecomeActiveSource(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_sendVendorCommand:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          byte[] _arg2;
          _arg2 = data.createByteArray();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          this.sendVendorCommand(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addVendorCommandListener:
        {
          data.enforceInterface(descriptor);
          android.hardware.hdmi.IHdmiVendorCommandListener _arg0;
          _arg0 = android.hardware.hdmi.IHdmiVendorCommandListener.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.addVendorCommandListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_sendStandby:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.sendStandby(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setHdmiRecordListener:
        {
          data.enforceInterface(descriptor);
          android.hardware.hdmi.IHdmiRecordListener _arg0;
          _arg0 = android.hardware.hdmi.IHdmiRecordListener.Stub.asInterface(data.readStrongBinder());
          this.setHdmiRecordListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startOneTouchRecord:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          byte[] _arg1;
          _arg1 = data.createByteArray();
          this.startOneTouchRecord(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopOneTouchRecord:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.stopOneTouchRecord(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startTimerRecording:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          byte[] _arg2;
          _arg2 = data.createByteArray();
          this.startTimerRecording(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_clearTimerRecording:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          byte[] _arg2;
          _arg2 = data.createByteArray();
          this.clearTimerRecording(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_sendMhlVendorCommand:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          byte[] _arg3;
          _arg3 = data.createByteArray();
          this.sendMhlVendorCommand(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addHdmiMhlVendorCommandListener:
        {
          data.enforceInterface(descriptor);
          android.hardware.hdmi.IHdmiMhlVendorCommandListener _arg0;
          _arg0 = android.hardware.hdmi.IHdmiMhlVendorCommandListener.Stub.asInterface(data.readStrongBinder());
          this.addHdmiMhlVendorCommandListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setStandbyMode:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setStandbyMode(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_reportAudioStatus:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          this.reportAudioStatus(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setSystemAudioModeOnForAudioOnlySource:
        {
          data.enforceInterface(descriptor);
          this.setSystemAudioModeOnForAudioOnlySource();
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.hdmi.IHdmiControlService
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
      @Override public int[] getSupportedTypes() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSupportedTypes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSupportedTypes();
          }
          _reply.readException();
          _result = _reply.createIntArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.hardware.hdmi.HdmiDeviceInfo getActiveSource() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.hardware.hdmi.HdmiDeviceInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveSource, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActiveSource();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.hardware.hdmi.HdmiDeviceInfo.CREATOR.createFromParcel(_reply);
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
      @Override public void oneTouchPlay(android.hardware.hdmi.IHdmiControlCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_oneTouchPlay, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().oneTouchPlay(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void queryDisplayStatus(android.hardware.hdmi.IHdmiControlCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryDisplayStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().queryDisplayStatus(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void addHotplugEventListener(android.hardware.hdmi.IHdmiHotplugEventListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addHotplugEventListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addHotplugEventListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeHotplugEventListener(android.hardware.hdmi.IHdmiHotplugEventListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeHotplugEventListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeHotplugEventListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void addDeviceEventListener(android.hardware.hdmi.IHdmiDeviceEventListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addDeviceEventListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addDeviceEventListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void deviceSelect(int deviceId, android.hardware.hdmi.IHdmiControlCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(deviceId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_deviceSelect, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deviceSelect(deviceId, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void portSelect(int portId, android.hardware.hdmi.IHdmiControlCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(portId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_portSelect, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().portSelect(portId, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void sendKeyEvent(int deviceType, int keyCode, boolean isPressed) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(deviceType);
          _data.writeInt(keyCode);
          _data.writeInt(((isPressed)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendKeyEvent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendKeyEvent(deviceType, keyCode, isPressed);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void sendVolumeKeyEvent(int deviceType, int keyCode, boolean isPressed) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(deviceType);
          _data.writeInt(keyCode);
          _data.writeInt(((isPressed)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendVolumeKeyEvent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendVolumeKeyEvent(deviceType, keyCode, isPressed);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.util.List<android.hardware.hdmi.HdmiPortInfo> getPortInfo() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.hardware.hdmi.HdmiPortInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPortInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPortInfo();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.hardware.hdmi.HdmiPortInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public boolean canChangeSystemAudioMode() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_canChangeSystemAudioMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().canChangeSystemAudioMode();
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
      @Override public boolean getSystemAudioMode() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSystemAudioMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSystemAudioMode();
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
      @Override public int getPhysicalAddress() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPhysicalAddress, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPhysicalAddress();
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
      @Override public void setSystemAudioMode(boolean enabled, android.hardware.hdmi.IHdmiControlCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSystemAudioMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSystemAudioMode(enabled, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void addSystemAudioModeChangeListener(android.hardware.hdmi.IHdmiSystemAudioModeChangeListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addSystemAudioModeChangeListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addSystemAudioModeChangeListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeSystemAudioModeChangeListener(android.hardware.hdmi.IHdmiSystemAudioModeChangeListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeSystemAudioModeChangeListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeSystemAudioModeChangeListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setArcMode(boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setArcMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setArcMode(enabled);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setProhibitMode(boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setProhibitMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setProhibitMode(enabled);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setSystemAudioVolume(int oldIndex, int newIndex, int maxIndex) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(oldIndex);
          _data.writeInt(newIndex);
          _data.writeInt(maxIndex);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSystemAudioVolume, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSystemAudioVolume(oldIndex, newIndex, maxIndex);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setSystemAudioMute(boolean mute) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((mute)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSystemAudioMute, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSystemAudioMute(mute);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setInputChangeListener(android.hardware.hdmi.IHdmiInputChangeListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setInputChangeListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setInputChangeListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.util.List<android.hardware.hdmi.HdmiDeviceInfo> getInputDevices() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.hardware.hdmi.HdmiDeviceInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getInputDevices, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getInputDevices();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.hardware.hdmi.HdmiDeviceInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.util.List<android.hardware.hdmi.HdmiDeviceInfo> getDeviceList() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.hardware.hdmi.HdmiDeviceInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDeviceList, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDeviceList();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.hardware.hdmi.HdmiDeviceInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void powerOffRemoteDevice(int logicalAddress, int powerStatus) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(logicalAddress);
          _data.writeInt(powerStatus);
          boolean _status = mRemote.transact(Stub.TRANSACTION_powerOffRemoteDevice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().powerOffRemoteDevice(logicalAddress, powerStatus);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void powerOnRemoteDevice(int logicalAddress, int powerStatus) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(logicalAddress);
          _data.writeInt(powerStatus);
          boolean _status = mRemote.transact(Stub.TRANSACTION_powerOnRemoteDevice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().powerOnRemoteDevice(logicalAddress, powerStatus);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void askRemoteDeviceToBecomeActiveSource(int physicalAddress) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(physicalAddress);
          boolean _status = mRemote.transact(Stub.TRANSACTION_askRemoteDeviceToBecomeActiveSource, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().askRemoteDeviceToBecomeActiveSource(physicalAddress);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void sendVendorCommand(int deviceType, int targetAddress, byte[] params, boolean hasVendorId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(deviceType);
          _data.writeInt(targetAddress);
          _data.writeByteArray(params);
          _data.writeInt(((hasVendorId)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendVendorCommand, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendVendorCommand(deviceType, targetAddress, params, hasVendorId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void addVendorCommandListener(android.hardware.hdmi.IHdmiVendorCommandListener listener, int deviceType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          _data.writeInt(deviceType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addVendorCommandListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addVendorCommandListener(listener, deviceType);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void sendStandby(int deviceType, int deviceId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(deviceType);
          _data.writeInt(deviceId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendStandby, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendStandby(deviceType, deviceId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setHdmiRecordListener(android.hardware.hdmi.IHdmiRecordListener callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setHdmiRecordListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setHdmiRecordListener(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void startOneTouchRecord(int recorderAddress, byte[] recordSource) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(recorderAddress);
          _data.writeByteArray(recordSource);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startOneTouchRecord, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startOneTouchRecord(recorderAddress, recordSource);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stopOneTouchRecord(int recorderAddress) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(recorderAddress);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopOneTouchRecord, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopOneTouchRecord(recorderAddress);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void startTimerRecording(int recorderAddress, int sourceType, byte[] recordSource) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(recorderAddress);
          _data.writeInt(sourceType);
          _data.writeByteArray(recordSource);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startTimerRecording, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startTimerRecording(recorderAddress, sourceType, recordSource);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void clearTimerRecording(int recorderAddress, int sourceType, byte[] recordSource) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(recorderAddress);
          _data.writeInt(sourceType);
          _data.writeByteArray(recordSource);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearTimerRecording, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearTimerRecording(recorderAddress, sourceType, recordSource);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void sendMhlVendorCommand(int portId, int offset, int length, byte[] data) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(portId);
          _data.writeInt(offset);
          _data.writeInt(length);
          _data.writeByteArray(data);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendMhlVendorCommand, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendMhlVendorCommand(portId, offset, length, data);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void addHdmiMhlVendorCommandListener(android.hardware.hdmi.IHdmiMhlVendorCommandListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addHdmiMhlVendorCommandListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addHdmiMhlVendorCommandListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setStandbyMode(boolean isStandbyModeOn) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((isStandbyModeOn)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setStandbyMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setStandbyMode(isStandbyModeOn);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void reportAudioStatus(int deviceType, int volume, int maxVolume, boolean isMute) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(deviceType);
          _data.writeInt(volume);
          _data.writeInt(maxVolume);
          _data.writeInt(((isMute)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportAudioStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportAudioStatus(deviceType, volume, maxVolume, isMute);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setSystemAudioModeOnForAudioOnlySource() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSystemAudioModeOnForAudioOnlySource, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setSystemAudioModeOnForAudioOnlySource();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.hardware.hdmi.IHdmiControlService sDefaultImpl;
    }
    static final int TRANSACTION_getSupportedTypes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getActiveSource = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_oneTouchPlay = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_queryDisplayStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_addHotplugEventListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_removeHotplugEventListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_addDeviceEventListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_deviceSelect = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_portSelect = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_sendKeyEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_sendVolumeKeyEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_getPortInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_canChangeSystemAudioMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_getSystemAudioMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_getPhysicalAddress = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_setSystemAudioMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_addSystemAudioModeChangeListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_removeSystemAudioModeChangeListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_setArcMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_setProhibitMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_setSystemAudioVolume = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_setSystemAudioMute = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_setInputChangeListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_getInputDevices = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_getDeviceList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_powerOffRemoteDevice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_powerOnRemoteDevice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_askRemoteDeviceToBecomeActiveSource = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_sendVendorCommand = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_addVendorCommandListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_sendStandby = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_setHdmiRecordListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_startOneTouchRecord = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_stopOneTouchRecord = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_startTimerRecording = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_clearTimerRecording = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_sendMhlVendorCommand = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    static final int TRANSACTION_addHdmiMhlVendorCommandListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 37);
    static final int TRANSACTION_setStandbyMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 38);
    static final int TRANSACTION_reportAudioStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 39);
    static final int TRANSACTION_setSystemAudioModeOnForAudioOnlySource = (android.os.IBinder.FIRST_CALL_TRANSACTION + 40);
    public static boolean setDefaultImpl(android.hardware.hdmi.IHdmiControlService impl) {
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
    public static android.hardware.hdmi.IHdmiControlService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public int[] getSupportedTypes() throws android.os.RemoteException;
  public android.hardware.hdmi.HdmiDeviceInfo getActiveSource() throws android.os.RemoteException;
  public void oneTouchPlay(android.hardware.hdmi.IHdmiControlCallback callback) throws android.os.RemoteException;
  public void queryDisplayStatus(android.hardware.hdmi.IHdmiControlCallback callback) throws android.os.RemoteException;
  public void addHotplugEventListener(android.hardware.hdmi.IHdmiHotplugEventListener listener) throws android.os.RemoteException;
  public void removeHotplugEventListener(android.hardware.hdmi.IHdmiHotplugEventListener listener) throws android.os.RemoteException;
  public void addDeviceEventListener(android.hardware.hdmi.IHdmiDeviceEventListener listener) throws android.os.RemoteException;
  public void deviceSelect(int deviceId, android.hardware.hdmi.IHdmiControlCallback callback) throws android.os.RemoteException;
  public void portSelect(int portId, android.hardware.hdmi.IHdmiControlCallback callback) throws android.os.RemoteException;
  public void sendKeyEvent(int deviceType, int keyCode, boolean isPressed) throws android.os.RemoteException;
  public void sendVolumeKeyEvent(int deviceType, int keyCode, boolean isPressed) throws android.os.RemoteException;
  public java.util.List<android.hardware.hdmi.HdmiPortInfo> getPortInfo() throws android.os.RemoteException;
  public boolean canChangeSystemAudioMode() throws android.os.RemoteException;
  public boolean getSystemAudioMode() throws android.os.RemoteException;
  public int getPhysicalAddress() throws android.os.RemoteException;
  public void setSystemAudioMode(boolean enabled, android.hardware.hdmi.IHdmiControlCallback callback) throws android.os.RemoteException;
  public void addSystemAudioModeChangeListener(android.hardware.hdmi.IHdmiSystemAudioModeChangeListener listener) throws android.os.RemoteException;
  public void removeSystemAudioModeChangeListener(android.hardware.hdmi.IHdmiSystemAudioModeChangeListener listener) throws android.os.RemoteException;
  public void setArcMode(boolean enabled) throws android.os.RemoteException;
  public void setProhibitMode(boolean enabled) throws android.os.RemoteException;
  public void setSystemAudioVolume(int oldIndex, int newIndex, int maxIndex) throws android.os.RemoteException;
  public void setSystemAudioMute(boolean mute) throws android.os.RemoteException;
  public void setInputChangeListener(android.hardware.hdmi.IHdmiInputChangeListener listener) throws android.os.RemoteException;
  public java.util.List<android.hardware.hdmi.HdmiDeviceInfo> getInputDevices() throws android.os.RemoteException;
  public java.util.List<android.hardware.hdmi.HdmiDeviceInfo> getDeviceList() throws android.os.RemoteException;
  public void powerOffRemoteDevice(int logicalAddress, int powerStatus) throws android.os.RemoteException;
  public void powerOnRemoteDevice(int logicalAddress, int powerStatus) throws android.os.RemoteException;
  public void askRemoteDeviceToBecomeActiveSource(int physicalAddress) throws android.os.RemoteException;
  public void sendVendorCommand(int deviceType, int targetAddress, byte[] params, boolean hasVendorId) throws android.os.RemoteException;
  public void addVendorCommandListener(android.hardware.hdmi.IHdmiVendorCommandListener listener, int deviceType) throws android.os.RemoteException;
  public void sendStandby(int deviceType, int deviceId) throws android.os.RemoteException;
  public void setHdmiRecordListener(android.hardware.hdmi.IHdmiRecordListener callback) throws android.os.RemoteException;
  public void startOneTouchRecord(int recorderAddress, byte[] recordSource) throws android.os.RemoteException;
  public void stopOneTouchRecord(int recorderAddress) throws android.os.RemoteException;
  public void startTimerRecording(int recorderAddress, int sourceType, byte[] recordSource) throws android.os.RemoteException;
  public void clearTimerRecording(int recorderAddress, int sourceType, byte[] recordSource) throws android.os.RemoteException;
  public void sendMhlVendorCommand(int portId, int offset, int length, byte[] data) throws android.os.RemoteException;
  public void addHdmiMhlVendorCommandListener(android.hardware.hdmi.IHdmiMhlVendorCommandListener listener) throws android.os.RemoteException;
  public void setStandbyMode(boolean isStandbyModeOn) throws android.os.RemoteException;
  public void reportAudioStatus(int deviceType, int volume, int maxVolume, boolean isMute) throws android.os.RemoteException;
  public void setSystemAudioModeOnForAudioOnlySource() throws android.os.RemoteException;
}
