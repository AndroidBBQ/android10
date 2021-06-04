/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.input;
/** @hide */
public interface IInputManager extends android.os.IInterface
{
  /** Default implementation for IInputManager. */
  public static class Default implements android.hardware.input.IInputManager
  {
    // Gets input device information.

    @Override public android.view.InputDevice getInputDevice(int deviceId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int[] getInputDeviceIds() throws android.os.RemoteException
    {
      return null;
    }
    // Enable/disable input device.

    @Override public boolean isInputDeviceEnabled(int deviceId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void enableInputDevice(int deviceId) throws android.os.RemoteException
    {
    }
    @Override public void disableInputDevice(int deviceId) throws android.os.RemoteException
    {
    }
    // Reports whether the hardware supports the given keys; returns true if successful

    @Override public boolean hasKeys(int deviceId, int sourceMask, int[] keyCodes, boolean[] keyExists) throws android.os.RemoteException
    {
      return false;
    }
    // Temporarily changes the pointer speed.

    @Override public void tryPointerSpeed(int speed) throws android.os.RemoteException
    {
    }
    // Injects an input event into the system.  To inject into windows owned by other
    // applications, the caller must have the INJECT_EVENTS permission.

    @Override public boolean injectInputEvent(android.view.InputEvent ev, int mode) throws android.os.RemoteException
    {
      return false;
    }
    // Calibrate input device position

    @Override public android.hardware.input.TouchCalibration getTouchCalibrationForInputDevice(java.lang.String inputDeviceDescriptor, int rotation) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setTouchCalibrationForInputDevice(java.lang.String inputDeviceDescriptor, int rotation, android.hardware.input.TouchCalibration calibration) throws android.os.RemoteException
    {
    }
    // Keyboard layouts configuration.

    @Override public android.hardware.input.KeyboardLayout[] getKeyboardLayouts() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.hardware.input.KeyboardLayout[] getKeyboardLayoutsForInputDevice(android.hardware.input.InputDeviceIdentifier identifier) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.hardware.input.KeyboardLayout getKeyboardLayout(java.lang.String keyboardLayoutDescriptor) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.lang.String getCurrentKeyboardLayoutForInputDevice(android.hardware.input.InputDeviceIdentifier identifier) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setCurrentKeyboardLayoutForInputDevice(android.hardware.input.InputDeviceIdentifier identifier, java.lang.String keyboardLayoutDescriptor) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String[] getEnabledKeyboardLayoutsForInputDevice(android.hardware.input.InputDeviceIdentifier identifier) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void addKeyboardLayoutForInputDevice(android.hardware.input.InputDeviceIdentifier identifier, java.lang.String keyboardLayoutDescriptor) throws android.os.RemoteException
    {
    }
    @Override public void removeKeyboardLayoutForInputDevice(android.hardware.input.InputDeviceIdentifier identifier, java.lang.String keyboardLayoutDescriptor) throws android.os.RemoteException
    {
    }
    // Registers an input devices changed listener.

    @Override public void registerInputDevicesChangedListener(android.hardware.input.IInputDevicesChangedListener listener) throws android.os.RemoteException
    {
    }
    // Queries whether the device is currently in tablet mode

    @Override public int isInTabletMode() throws android.os.RemoteException
    {
      return 0;
    }
    // Registers a tablet mode change listener

    @Override public void registerTabletModeChangedListener(android.hardware.input.ITabletModeChangedListener listener) throws android.os.RemoteException
    {
    }
    // Input device vibrator control.

    @Override public void vibrate(int deviceId, long[] pattern, int repeat, android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public void cancelVibrate(int deviceId, android.os.IBinder token) throws android.os.RemoteException
    {
    }
    @Override public void setPointerIconType(int typeId) throws android.os.RemoteException
    {
    }
    @Override public void setCustomPointerIcon(android.view.PointerIcon icon) throws android.os.RemoteException
    {
    }
    @Override public void requestPointerCapture(android.os.IBinder windowToken, boolean enabled) throws android.os.RemoteException
    {
    }
    /** Create an input monitor for gestures. */
    @Override public android.view.InputMonitor monitorGestureInput(java.lang.String name, int displayId) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.input.IInputManager
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.input.IInputManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.input.IInputManager interface,
     * generating a proxy if needed.
     */
    public static android.hardware.input.IInputManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.input.IInputManager))) {
        return ((android.hardware.input.IInputManager)iin);
      }
      return new android.hardware.input.IInputManager.Stub.Proxy(obj);
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
        case TRANSACTION_getInputDevice:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.view.InputDevice _result = this.getInputDevice(_arg0);
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
        case TRANSACTION_getInputDeviceIds:
        {
          data.enforceInterface(descriptor);
          int[] _result = this.getInputDeviceIds();
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_isInputDeviceEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isInputDeviceEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_enableInputDevice:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.enableInputDevice(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_disableInputDevice:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.disableInputDevice(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_hasKeys:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int[] _arg2;
          _arg2 = data.createIntArray();
          boolean[] _arg3;
          int _arg3_length = data.readInt();
          if ((_arg3_length<0)) {
            _arg3 = null;
          }
          else {
            _arg3 = new boolean[_arg3_length];
          }
          boolean _result = this.hasKeys(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          reply.writeBooleanArray(_arg3);
          return true;
        }
        case TRANSACTION_tryPointerSpeed:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.tryPointerSpeed(_arg0);
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
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.injectInputEvent(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getTouchCalibrationForInputDevice:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.hardware.input.TouchCalibration _result = this.getTouchCalibrationForInputDevice(_arg0, _arg1);
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
        case TRANSACTION_setTouchCalibrationForInputDevice:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.hardware.input.TouchCalibration _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.hardware.input.TouchCalibration.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.setTouchCalibrationForInputDevice(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getKeyboardLayouts:
        {
          data.enforceInterface(descriptor);
          android.hardware.input.KeyboardLayout[] _result = this.getKeyboardLayouts();
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_getKeyboardLayoutsForInputDevice:
        {
          data.enforceInterface(descriptor);
          android.hardware.input.InputDeviceIdentifier _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.input.InputDeviceIdentifier.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.hardware.input.KeyboardLayout[] _result = this.getKeyboardLayoutsForInputDevice(_arg0);
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_getKeyboardLayout:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.hardware.input.KeyboardLayout _result = this.getKeyboardLayout(_arg0);
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
        case TRANSACTION_getCurrentKeyboardLayoutForInputDevice:
        {
          data.enforceInterface(descriptor);
          android.hardware.input.InputDeviceIdentifier _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.input.InputDeviceIdentifier.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _result = this.getCurrentKeyboardLayoutForInputDevice(_arg0);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_setCurrentKeyboardLayoutForInputDevice:
        {
          data.enforceInterface(descriptor);
          android.hardware.input.InputDeviceIdentifier _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.input.InputDeviceIdentifier.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.setCurrentKeyboardLayoutForInputDevice(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getEnabledKeyboardLayoutsForInputDevice:
        {
          data.enforceInterface(descriptor);
          android.hardware.input.InputDeviceIdentifier _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.input.InputDeviceIdentifier.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String[] _result = this.getEnabledKeyboardLayoutsForInputDevice(_arg0);
          reply.writeNoException();
          reply.writeStringArray(_result);
          return true;
        }
        case TRANSACTION_addKeyboardLayoutForInputDevice:
        {
          data.enforceInterface(descriptor);
          android.hardware.input.InputDeviceIdentifier _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.input.InputDeviceIdentifier.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.addKeyboardLayoutForInputDevice(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeKeyboardLayoutForInputDevice:
        {
          data.enforceInterface(descriptor);
          android.hardware.input.InputDeviceIdentifier _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.input.InputDeviceIdentifier.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.removeKeyboardLayoutForInputDevice(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerInputDevicesChangedListener:
        {
          data.enforceInterface(descriptor);
          android.hardware.input.IInputDevicesChangedListener _arg0;
          _arg0 = android.hardware.input.IInputDevicesChangedListener.Stub.asInterface(data.readStrongBinder());
          this.registerInputDevicesChangedListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isInTabletMode:
        {
          data.enforceInterface(descriptor);
          int _result = this.isInTabletMode();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_registerTabletModeChangedListener:
        {
          data.enforceInterface(descriptor);
          android.hardware.input.ITabletModeChangedListener _arg0;
          _arg0 = android.hardware.input.ITabletModeChangedListener.Stub.asInterface(data.readStrongBinder());
          this.registerTabletModeChangedListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_vibrate:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long[] _arg1;
          _arg1 = data.createLongArray();
          int _arg2;
          _arg2 = data.readInt();
          android.os.IBinder _arg3;
          _arg3 = data.readStrongBinder();
          this.vibrate(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_cancelVibrate:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          this.cancelVibrate(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setPointerIconType:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setPointerIconType(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setCustomPointerIcon:
        {
          data.enforceInterface(descriptor);
          android.view.PointerIcon _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.PointerIcon.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setCustomPointerIcon(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_requestPointerCapture:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.requestPointerCapture(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_monitorGestureInput:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.view.InputMonitor _result = this.monitorGestureInput(_arg0, _arg1);
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
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.input.IInputManager
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
      // Gets input device information.

      @Override public android.view.InputDevice getInputDevice(int deviceId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.view.InputDevice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(deviceId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getInputDevice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getInputDevice(deviceId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.view.InputDevice.CREATOR.createFromParcel(_reply);
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
      @Override public int[] getInputDeviceIds() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getInputDeviceIds, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getInputDeviceIds();
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
      // Enable/disable input device.

      @Override public boolean isInputDeviceEnabled(int deviceId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(deviceId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isInputDeviceEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isInputDeviceEnabled(deviceId);
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
      @Override public void enableInputDevice(int deviceId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(deviceId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableInputDevice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enableInputDevice(deviceId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void disableInputDevice(int deviceId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(deviceId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableInputDevice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableInputDevice(deviceId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Reports whether the hardware supports the given keys; returns true if successful

      @Override public boolean hasKeys(int deviceId, int sourceMask, int[] keyCodes, boolean[] keyExists) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(deviceId);
          _data.writeInt(sourceMask);
          _data.writeIntArray(keyCodes);
          if ((keyExists==null)) {
            _data.writeInt(-1);
          }
          else {
            _data.writeInt(keyExists.length);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasKeys, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasKeys(deviceId, sourceMask, keyCodes, keyExists);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
          _reply.readBooleanArray(keyExists);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // Temporarily changes the pointer speed.

      @Override public void tryPointerSpeed(int speed) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(speed);
          boolean _status = mRemote.transact(Stub.TRANSACTION_tryPointerSpeed, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().tryPointerSpeed(speed);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Injects an input event into the system.  To inject into windows owned by other
      // applications, the caller must have the INJECT_EVENTS permission.

      @Override public boolean injectInputEvent(android.view.InputEvent ev, int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((ev!=null)) {
            _data.writeInt(1);
            ev.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_injectInputEvent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().injectInputEvent(ev, mode);
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
      // Calibrate input device position

      @Override public android.hardware.input.TouchCalibration getTouchCalibrationForInputDevice(java.lang.String inputDeviceDescriptor, int rotation) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.hardware.input.TouchCalibration _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(inputDeviceDescriptor);
          _data.writeInt(rotation);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTouchCalibrationForInputDevice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTouchCalibrationForInputDevice(inputDeviceDescriptor, rotation);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.hardware.input.TouchCalibration.CREATOR.createFromParcel(_reply);
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
      @Override public void setTouchCalibrationForInputDevice(java.lang.String inputDeviceDescriptor, int rotation, android.hardware.input.TouchCalibration calibration) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(inputDeviceDescriptor);
          _data.writeInt(rotation);
          if ((calibration!=null)) {
            _data.writeInt(1);
            calibration.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTouchCalibrationForInputDevice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setTouchCalibrationForInputDevice(inputDeviceDescriptor, rotation, calibration);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Keyboard layouts configuration.

      @Override public android.hardware.input.KeyboardLayout[] getKeyboardLayouts() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.hardware.input.KeyboardLayout[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getKeyboardLayouts, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getKeyboardLayouts();
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.hardware.input.KeyboardLayout.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.hardware.input.KeyboardLayout[] getKeyboardLayoutsForInputDevice(android.hardware.input.InputDeviceIdentifier identifier) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.hardware.input.KeyboardLayout[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((identifier!=null)) {
            _data.writeInt(1);
            identifier.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getKeyboardLayoutsForInputDevice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getKeyboardLayoutsForInputDevice(identifier);
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.hardware.input.KeyboardLayout.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.hardware.input.KeyboardLayout getKeyboardLayout(java.lang.String keyboardLayoutDescriptor) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.hardware.input.KeyboardLayout _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(keyboardLayoutDescriptor);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getKeyboardLayout, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getKeyboardLayout(keyboardLayoutDescriptor);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.hardware.input.KeyboardLayout.CREATOR.createFromParcel(_reply);
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
      @Override public java.lang.String getCurrentKeyboardLayoutForInputDevice(android.hardware.input.InputDeviceIdentifier identifier) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((identifier!=null)) {
            _data.writeInt(1);
            identifier.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentKeyboardLayoutForInputDevice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentKeyboardLayoutForInputDevice(identifier);
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
      @Override public void setCurrentKeyboardLayoutForInputDevice(android.hardware.input.InputDeviceIdentifier identifier, java.lang.String keyboardLayoutDescriptor) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((identifier!=null)) {
            _data.writeInt(1);
            identifier.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(keyboardLayoutDescriptor);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setCurrentKeyboardLayoutForInputDevice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setCurrentKeyboardLayoutForInputDevice(identifier, keyboardLayoutDescriptor);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.lang.String[] getEnabledKeyboardLayoutsForInputDevice(android.hardware.input.InputDeviceIdentifier identifier) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((identifier!=null)) {
            _data.writeInt(1);
            identifier.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getEnabledKeyboardLayoutsForInputDevice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getEnabledKeyboardLayoutsForInputDevice(identifier);
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
      @Override public void addKeyboardLayoutForInputDevice(android.hardware.input.InputDeviceIdentifier identifier, java.lang.String keyboardLayoutDescriptor) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((identifier!=null)) {
            _data.writeInt(1);
            identifier.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(keyboardLayoutDescriptor);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addKeyboardLayoutForInputDevice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addKeyboardLayoutForInputDevice(identifier, keyboardLayoutDescriptor);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeKeyboardLayoutForInputDevice(android.hardware.input.InputDeviceIdentifier identifier, java.lang.String keyboardLayoutDescriptor) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((identifier!=null)) {
            _data.writeInt(1);
            identifier.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(keyboardLayoutDescriptor);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeKeyboardLayoutForInputDevice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeKeyboardLayoutForInputDevice(identifier, keyboardLayoutDescriptor);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Registers an input devices changed listener.

      @Override public void registerInputDevicesChangedListener(android.hardware.input.IInputDevicesChangedListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerInputDevicesChangedListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerInputDevicesChangedListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Queries whether the device is currently in tablet mode

      @Override public int isInTabletMode() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isInTabletMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isInTabletMode();
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
      // Registers a tablet mode change listener

      @Override public void registerTabletModeChangedListener(android.hardware.input.ITabletModeChangedListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerTabletModeChangedListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerTabletModeChangedListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Input device vibrator control.

      @Override public void vibrate(int deviceId, long[] pattern, int repeat, android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(deviceId);
          _data.writeLongArray(pattern);
          _data.writeInt(repeat);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_vibrate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().vibrate(deviceId, pattern, repeat, token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void cancelVibrate(int deviceId, android.os.IBinder token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(deviceId);
          _data.writeStrongBinder(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelVibrate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelVibrate(deviceId, token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setPointerIconType(int typeId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(typeId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPointerIconType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPointerIconType(typeId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setCustomPointerIcon(android.view.PointerIcon icon) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((icon!=null)) {
            _data.writeInt(1);
            icon.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setCustomPointerIcon, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setCustomPointerIcon(icon);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void requestPointerCapture(android.os.IBinder windowToken, boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(windowToken);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestPointerCapture, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestPointerCapture(windowToken, enabled);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Create an input monitor for gestures. */
      @Override public android.view.InputMonitor monitorGestureInput(java.lang.String name, int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.view.InputMonitor _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(name);
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_monitorGestureInput, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().monitorGestureInput(name, displayId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.view.InputMonitor.CREATOR.createFromParcel(_reply);
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
      public static android.hardware.input.IInputManager sDefaultImpl;
    }
    static final int TRANSACTION_getInputDevice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getInputDeviceIds = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_isInputDeviceEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_enableInputDevice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_disableInputDevice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_hasKeys = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_tryPointerSpeed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_injectInputEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_getTouchCalibrationForInputDevice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_setTouchCalibrationForInputDevice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_getKeyboardLayouts = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_getKeyboardLayoutsForInputDevice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_getKeyboardLayout = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_getCurrentKeyboardLayoutForInputDevice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_setCurrentKeyboardLayoutForInputDevice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_getEnabledKeyboardLayoutsForInputDevice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_addKeyboardLayoutForInputDevice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_removeKeyboardLayoutForInputDevice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_registerInputDevicesChangedListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_isInTabletMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_registerTabletModeChangedListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_vibrate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_cancelVibrate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_setPointerIconType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_setCustomPointerIcon = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_requestPointerCapture = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_monitorGestureInput = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    public static boolean setDefaultImpl(android.hardware.input.IInputManager impl) {
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
    public static android.hardware.input.IInputManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // Gets input device information.

  public android.view.InputDevice getInputDevice(int deviceId) throws android.os.RemoteException;
  public int[] getInputDeviceIds() throws android.os.RemoteException;
  // Enable/disable input device.

  public boolean isInputDeviceEnabled(int deviceId) throws android.os.RemoteException;
  public void enableInputDevice(int deviceId) throws android.os.RemoteException;
  public void disableInputDevice(int deviceId) throws android.os.RemoteException;
  // Reports whether the hardware supports the given keys; returns true if successful

  public boolean hasKeys(int deviceId, int sourceMask, int[] keyCodes, boolean[] keyExists) throws android.os.RemoteException;
  // Temporarily changes the pointer speed.

  public void tryPointerSpeed(int speed) throws android.os.RemoteException;
  // Injects an input event into the system.  To inject into windows owned by other
  // applications, the caller must have the INJECT_EVENTS permission.

  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/hardware/input/IInputManager.aidl:50:1:50:25")
  public boolean injectInputEvent(android.view.InputEvent ev, int mode) throws android.os.RemoteException;
  // Calibrate input device position

  public android.hardware.input.TouchCalibration getTouchCalibrationForInputDevice(java.lang.String inputDeviceDescriptor, int rotation) throws android.os.RemoteException;
  public void setTouchCalibrationForInputDevice(java.lang.String inputDeviceDescriptor, int rotation, android.hardware.input.TouchCalibration calibration) throws android.os.RemoteException;
  // Keyboard layouts configuration.

  public android.hardware.input.KeyboardLayout[] getKeyboardLayouts() throws android.os.RemoteException;
  public android.hardware.input.KeyboardLayout[] getKeyboardLayoutsForInputDevice(android.hardware.input.InputDeviceIdentifier identifier) throws android.os.RemoteException;
  public android.hardware.input.KeyboardLayout getKeyboardLayout(java.lang.String keyboardLayoutDescriptor) throws android.os.RemoteException;
  public java.lang.String getCurrentKeyboardLayoutForInputDevice(android.hardware.input.InputDeviceIdentifier identifier) throws android.os.RemoteException;
  public void setCurrentKeyboardLayoutForInputDevice(android.hardware.input.InputDeviceIdentifier identifier, java.lang.String keyboardLayoutDescriptor) throws android.os.RemoteException;
  public java.lang.String[] getEnabledKeyboardLayoutsForInputDevice(android.hardware.input.InputDeviceIdentifier identifier) throws android.os.RemoteException;
  public void addKeyboardLayoutForInputDevice(android.hardware.input.InputDeviceIdentifier identifier, java.lang.String keyboardLayoutDescriptor) throws android.os.RemoteException;
  public void removeKeyboardLayoutForInputDevice(android.hardware.input.InputDeviceIdentifier identifier, java.lang.String keyboardLayoutDescriptor) throws android.os.RemoteException;
  // Registers an input devices changed listener.

  public void registerInputDevicesChangedListener(android.hardware.input.IInputDevicesChangedListener listener) throws android.os.RemoteException;
  // Queries whether the device is currently in tablet mode

  public int isInTabletMode() throws android.os.RemoteException;
  // Registers a tablet mode change listener

  public void registerTabletModeChangedListener(android.hardware.input.ITabletModeChangedListener listener) throws android.os.RemoteException;
  // Input device vibrator control.

  public void vibrate(int deviceId, long[] pattern, int repeat, android.os.IBinder token) throws android.os.RemoteException;
  public void cancelVibrate(int deviceId, android.os.IBinder token) throws android.os.RemoteException;
  public void setPointerIconType(int typeId) throws android.os.RemoteException;
  public void setCustomPointerIcon(android.view.PointerIcon icon) throws android.os.RemoteException;
  public void requestPointerCapture(android.os.IBinder windowToken, boolean enabled) throws android.os.RemoteException;
  /** Create an input monitor for gestures. */
  public android.view.InputMonitor monitorGestureInput(java.lang.String name, int displayId) throws android.os.RemoteException;
}
