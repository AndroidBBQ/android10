/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media.midi;
/** @hide */
public interface IBluetoothMidiService extends android.os.IInterface
{
  /** Default implementation for IBluetoothMidiService. */
  public static class Default implements android.media.midi.IBluetoothMidiService
  {
    @Override public android.os.IBinder addBluetoothDevice(android.bluetooth.BluetoothDevice bluetoothDevice) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.midi.IBluetoothMidiService
  {
    private static final java.lang.String DESCRIPTOR = "android.media.midi.IBluetoothMidiService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.midi.IBluetoothMidiService interface,
     * generating a proxy if needed.
     */
    public static android.media.midi.IBluetoothMidiService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.midi.IBluetoothMidiService))) {
        return ((android.media.midi.IBluetoothMidiService)iin);
      }
      return new android.media.midi.IBluetoothMidiService.Stub.Proxy(obj);
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
        case TRANSACTION_addBluetoothDevice:
        {
          data.enforceInterface(descriptor);
          android.bluetooth.BluetoothDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.bluetooth.BluetoothDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.IBinder _result = this.addBluetoothDevice(_arg0);
          reply.writeNoException();
          reply.writeStrongBinder(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.midi.IBluetoothMidiService
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
      @Override public android.os.IBinder addBluetoothDevice(android.bluetooth.BluetoothDevice bluetoothDevice) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.IBinder _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((bluetoothDevice!=null)) {
            _data.writeInt(1);
            bluetoothDevice.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_addBluetoothDevice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addBluetoothDevice(bluetoothDevice);
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
      public static android.media.midi.IBluetoothMidiService sDefaultImpl;
    }
    static final int TRANSACTION_addBluetoothDevice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.media.midi.IBluetoothMidiService impl) {
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
    public static android.media.midi.IBluetoothMidiService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public android.os.IBinder addBluetoothDevice(android.bluetooth.BluetoothDevice bluetoothDevice) throws android.os.RemoteException;
}
