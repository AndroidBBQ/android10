/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media.midi;
/** @hide */
public interface IMidiManager extends android.os.IInterface
{
  /** Default implementation for IMidiManager. */
  public static class Default implements android.media.midi.IMidiManager
  {
    @Override public android.media.midi.MidiDeviceInfo[] getDevices() throws android.os.RemoteException
    {
      return null;
    }
    // for device creation & removal notifications

    @Override public void registerListener(android.os.IBinder clientToken, android.media.midi.IMidiDeviceListener listener) throws android.os.RemoteException
    {
    }
    @Override public void unregisterListener(android.os.IBinder clientToken, android.media.midi.IMidiDeviceListener listener) throws android.os.RemoteException
    {
    }
    @Override public void openDevice(android.os.IBinder clientToken, android.media.midi.MidiDeviceInfo device, android.media.midi.IMidiDeviceOpenCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void openBluetoothDevice(android.os.IBinder clientToken, android.bluetooth.BluetoothDevice bluetoothDevice, android.media.midi.IMidiDeviceOpenCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void closeDevice(android.os.IBinder clientToken, android.os.IBinder deviceToken) throws android.os.RemoteException
    {
    }
    // for registering built-in MIDI devices

    @Override public android.media.midi.MidiDeviceInfo registerDeviceServer(android.media.midi.IMidiDeviceServer server, int numInputPorts, int numOutputPorts, java.lang.String[] inputPortNames, java.lang.String[] outputPortNames, android.os.Bundle properties, int type) throws android.os.RemoteException
    {
      return null;
    }
    // for unregistering built-in MIDI devices

    @Override public void unregisterDeviceServer(android.media.midi.IMidiDeviceServer server) throws android.os.RemoteException
    {
    }
    // used by MidiDeviceService to access the MidiDeviceInfo that was created based on its
    // manifest's meta-data

    @Override public android.media.midi.MidiDeviceInfo getServiceDeviceInfo(java.lang.String packageName, java.lang.String className) throws android.os.RemoteException
    {
      return null;
    }
    // used for client's to retrieve a device's MidiDeviceStatus

    @Override public android.media.midi.MidiDeviceStatus getDeviceStatus(android.media.midi.MidiDeviceInfo deviceInfo) throws android.os.RemoteException
    {
      return null;
    }
    // used by MIDI devices to report their status
    // the token is used by MidiService for death notification

    @Override public void setDeviceStatus(android.media.midi.IMidiDeviceServer server, android.media.midi.MidiDeviceStatus status) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.midi.IMidiManager
  {
    private static final java.lang.String DESCRIPTOR = "android.media.midi.IMidiManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.midi.IMidiManager interface,
     * generating a proxy if needed.
     */
    public static android.media.midi.IMidiManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.midi.IMidiManager))) {
        return ((android.media.midi.IMidiManager)iin);
      }
      return new android.media.midi.IMidiManager.Stub.Proxy(obj);
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
        case TRANSACTION_getDevices:
        {
          data.enforceInterface(descriptor);
          android.media.midi.MidiDeviceInfo[] _result = this.getDevices();
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_registerListener:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.media.midi.IMidiDeviceListener _arg1;
          _arg1 = android.media.midi.IMidiDeviceListener.Stub.asInterface(data.readStrongBinder());
          this.registerListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterListener:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.media.midi.IMidiDeviceListener _arg1;
          _arg1 = android.media.midi.IMidiDeviceListener.Stub.asInterface(data.readStrongBinder());
          this.unregisterListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_openDevice:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.media.midi.MidiDeviceInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.media.midi.MidiDeviceInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.media.midi.IMidiDeviceOpenCallback _arg2;
          _arg2 = android.media.midi.IMidiDeviceOpenCallback.Stub.asInterface(data.readStrongBinder());
          this.openDevice(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_openBluetoothDevice:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.bluetooth.BluetoothDevice _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.bluetooth.BluetoothDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.media.midi.IMidiDeviceOpenCallback _arg2;
          _arg2 = android.media.midi.IMidiDeviceOpenCallback.Stub.asInterface(data.readStrongBinder());
          this.openBluetoothDevice(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_closeDevice:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          this.closeDevice(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerDeviceServer:
        {
          data.enforceInterface(descriptor);
          android.media.midi.IMidiDeviceServer _arg0;
          _arg0 = android.media.midi.IMidiDeviceServer.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String[] _arg3;
          _arg3 = data.createStringArray();
          java.lang.String[] _arg4;
          _arg4 = data.createStringArray();
          android.os.Bundle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          int _arg6;
          _arg6 = data.readInt();
          android.media.midi.MidiDeviceInfo _result = this.registerDeviceServer(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
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
        case TRANSACTION_unregisterDeviceServer:
        {
          data.enforceInterface(descriptor);
          android.media.midi.IMidiDeviceServer _arg0;
          _arg0 = android.media.midi.IMidiDeviceServer.Stub.asInterface(data.readStrongBinder());
          this.unregisterDeviceServer(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getServiceDeviceInfo:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.media.midi.MidiDeviceInfo _result = this.getServiceDeviceInfo(_arg0, _arg1);
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
        case TRANSACTION_getDeviceStatus:
        {
          data.enforceInterface(descriptor);
          android.media.midi.MidiDeviceInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.media.midi.MidiDeviceInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.media.midi.MidiDeviceStatus _result = this.getDeviceStatus(_arg0);
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
        case TRANSACTION_setDeviceStatus:
        {
          data.enforceInterface(descriptor);
          android.media.midi.IMidiDeviceServer _arg0;
          _arg0 = android.media.midi.IMidiDeviceServer.Stub.asInterface(data.readStrongBinder());
          android.media.midi.MidiDeviceStatus _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.media.midi.MidiDeviceStatus.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.setDeviceStatus(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.midi.IMidiManager
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
      @Override public android.media.midi.MidiDeviceInfo[] getDevices() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.media.midi.MidiDeviceInfo[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDevices, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDevices();
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.media.midi.MidiDeviceInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // for device creation & removal notifications

      @Override public void registerListener(android.os.IBinder clientToken, android.media.midi.IMidiDeviceListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(clientToken);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerListener(clientToken, listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterListener(android.os.IBinder clientToken, android.media.midi.IMidiDeviceListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(clientToken);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterListener(clientToken, listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void openDevice(android.os.IBinder clientToken, android.media.midi.MidiDeviceInfo device, android.media.midi.IMidiDeviceOpenCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(clientToken);
          if ((device!=null)) {
            _data.writeInt(1);
            device.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_openDevice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().openDevice(clientToken, device, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void openBluetoothDevice(android.os.IBinder clientToken, android.bluetooth.BluetoothDevice bluetoothDevice, android.media.midi.IMidiDeviceOpenCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(clientToken);
          if ((bluetoothDevice!=null)) {
            _data.writeInt(1);
            bluetoothDevice.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_openBluetoothDevice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().openBluetoothDevice(clientToken, bluetoothDevice, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void closeDevice(android.os.IBinder clientToken, android.os.IBinder deviceToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(clientToken);
          _data.writeStrongBinder(deviceToken);
          boolean _status = mRemote.transact(Stub.TRANSACTION_closeDevice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().closeDevice(clientToken, deviceToken);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // for registering built-in MIDI devices

      @Override public android.media.midi.MidiDeviceInfo registerDeviceServer(android.media.midi.IMidiDeviceServer server, int numInputPorts, int numOutputPorts, java.lang.String[] inputPortNames, java.lang.String[] outputPortNames, android.os.Bundle properties, int type) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.media.midi.MidiDeviceInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((server!=null))?(server.asBinder()):(null)));
          _data.writeInt(numInputPorts);
          _data.writeInt(numOutputPorts);
          _data.writeStringArray(inputPortNames);
          _data.writeStringArray(outputPortNames);
          if ((properties!=null)) {
            _data.writeInt(1);
            properties.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(type);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerDeviceServer, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().registerDeviceServer(server, numInputPorts, numOutputPorts, inputPortNames, outputPortNames, properties, type);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.media.midi.MidiDeviceInfo.CREATOR.createFromParcel(_reply);
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
      // for unregistering built-in MIDI devices

      @Override public void unregisterDeviceServer(android.media.midi.IMidiDeviceServer server) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((server!=null))?(server.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterDeviceServer, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterDeviceServer(server);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // used by MidiDeviceService to access the MidiDeviceInfo that was created based on its
      // manifest's meta-data

      @Override public android.media.midi.MidiDeviceInfo getServiceDeviceInfo(java.lang.String packageName, java.lang.String className) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.media.midi.MidiDeviceInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeString(className);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getServiceDeviceInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getServiceDeviceInfo(packageName, className);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.media.midi.MidiDeviceInfo.CREATOR.createFromParcel(_reply);
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
      // used for client's to retrieve a device's MidiDeviceStatus

      @Override public android.media.midi.MidiDeviceStatus getDeviceStatus(android.media.midi.MidiDeviceInfo deviceInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.media.midi.MidiDeviceStatus _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((deviceInfo!=null)) {
            _data.writeInt(1);
            deviceInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDeviceStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDeviceStatus(deviceInfo);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.media.midi.MidiDeviceStatus.CREATOR.createFromParcel(_reply);
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
      // used by MIDI devices to report their status
      // the token is used by MidiService for death notification

      @Override public void setDeviceStatus(android.media.midi.IMidiDeviceServer server, android.media.midi.MidiDeviceStatus status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((server!=null))?(server.asBinder()):(null)));
          if ((status!=null)) {
            _data.writeInt(1);
            status.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDeviceStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDeviceStatus(server, status);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.media.midi.IMidiManager sDefaultImpl;
    }
    static final int TRANSACTION_getDevices = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_registerListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_unregisterListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_openDevice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_openBluetoothDevice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_closeDevice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_registerDeviceServer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_unregisterDeviceServer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_getServiceDeviceInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getDeviceStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_setDeviceStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    public static boolean setDefaultImpl(android.media.midi.IMidiManager impl) {
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
    public static android.media.midi.IMidiManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public android.media.midi.MidiDeviceInfo[] getDevices() throws android.os.RemoteException;
  // for device creation & removal notifications

  public void registerListener(android.os.IBinder clientToken, android.media.midi.IMidiDeviceListener listener) throws android.os.RemoteException;
  public void unregisterListener(android.os.IBinder clientToken, android.media.midi.IMidiDeviceListener listener) throws android.os.RemoteException;
  public void openDevice(android.os.IBinder clientToken, android.media.midi.MidiDeviceInfo device, android.media.midi.IMidiDeviceOpenCallback callback) throws android.os.RemoteException;
  public void openBluetoothDevice(android.os.IBinder clientToken, android.bluetooth.BluetoothDevice bluetoothDevice, android.media.midi.IMidiDeviceOpenCallback callback) throws android.os.RemoteException;
  public void closeDevice(android.os.IBinder clientToken, android.os.IBinder deviceToken) throws android.os.RemoteException;
  // for registering built-in MIDI devices

  public android.media.midi.MidiDeviceInfo registerDeviceServer(android.media.midi.IMidiDeviceServer server, int numInputPorts, int numOutputPorts, java.lang.String[] inputPortNames, java.lang.String[] outputPortNames, android.os.Bundle properties, int type) throws android.os.RemoteException;
  // for unregistering built-in MIDI devices

  public void unregisterDeviceServer(android.media.midi.IMidiDeviceServer server) throws android.os.RemoteException;
  // used by MidiDeviceService to access the MidiDeviceInfo that was created based on its
  // manifest's meta-data

  public android.media.midi.MidiDeviceInfo getServiceDeviceInfo(java.lang.String packageName, java.lang.String className) throws android.os.RemoteException;
  // used for client's to retrieve a device's MidiDeviceStatus

  public android.media.midi.MidiDeviceStatus getDeviceStatus(android.media.midi.MidiDeviceInfo deviceInfo) throws android.os.RemoteException;
  // used by MIDI devices to report their status
  // the token is used by MidiService for death notification

  public void setDeviceStatus(android.media.midi.IMidiDeviceServer server, android.media.midi.MidiDeviceStatus status) throws android.os.RemoteException;
}
