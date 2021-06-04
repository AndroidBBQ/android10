/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.trust;
/**
 * Callback interface for BLE connection state changes during trusted device enrollment.
 *
 * @hide
 */
public interface ICarTrustAgentBleCallback extends android.os.IInterface
{
  /** Default implementation for ICarTrustAgentBleCallback. */
  public static class Default implements android.car.trust.ICarTrustAgentBleCallback
  {
    /**
         * Called when the GATT server is started and BLE is successfully advertising for enrollment.
         */
    @Override public void onEnrollmentAdvertisingStarted() throws android.os.RemoteException
    {
    }
    /**
         * Called when the BLE enrollment advertisement fails to start.
         */
    @Override public void onEnrollmentAdvertisingFailed() throws android.os.RemoteException
    {
    }
    /**
         * Called when a remote device is connected on BLE.
         */
    @Override public void onBleEnrollmentDeviceConnected(android.bluetooth.BluetoothDevice device) throws android.os.RemoteException
    {
    }
    /**
         * Called when a remote device is disconnected on BLE.
         */
    @Override public void onBleEnrollmentDeviceDisconnected(android.bluetooth.BluetoothDevice device) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.trust.ICarTrustAgentBleCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.car.trust.ICarTrustAgentBleCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.trust.ICarTrustAgentBleCallback interface,
     * generating a proxy if needed.
     */
    public static android.car.trust.ICarTrustAgentBleCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.trust.ICarTrustAgentBleCallback))) {
        return ((android.car.trust.ICarTrustAgentBleCallback)iin);
      }
      return new android.car.trust.ICarTrustAgentBleCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onEnrollmentAdvertisingStarted:
        {
          data.enforceInterface(descriptor);
          this.onEnrollmentAdvertisingStarted();
          return true;
        }
        case TRANSACTION_onEnrollmentAdvertisingFailed:
        {
          data.enforceInterface(descriptor);
          this.onEnrollmentAdvertisingFailed();
          return true;
        }
        case TRANSACTION_onBleEnrollmentDeviceConnected:
        {
          data.enforceInterface(descriptor);
          android.bluetooth.BluetoothDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.bluetooth.BluetoothDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onBleEnrollmentDeviceConnected(_arg0);
          return true;
        }
        case TRANSACTION_onBleEnrollmentDeviceDisconnected:
        {
          data.enforceInterface(descriptor);
          android.bluetooth.BluetoothDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.bluetooth.BluetoothDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onBleEnrollmentDeviceDisconnected(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.trust.ICarTrustAgentBleCallback
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
      /**
           * Called when the GATT server is started and BLE is successfully advertising for enrollment.
           */
      @Override public void onEnrollmentAdvertisingStarted() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEnrollmentAdvertisingStarted, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEnrollmentAdvertisingStarted();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when the BLE enrollment advertisement fails to start.
           */
      @Override public void onEnrollmentAdvertisingFailed() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEnrollmentAdvertisingFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEnrollmentAdvertisingFailed();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when a remote device is connected on BLE.
           */
      @Override public void onBleEnrollmentDeviceConnected(android.bluetooth.BluetoothDevice device) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((device!=null)) {
            _data.writeInt(1);
            device.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onBleEnrollmentDeviceConnected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onBleEnrollmentDeviceConnected(device);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when a remote device is disconnected on BLE.
           */
      @Override public void onBleEnrollmentDeviceDisconnected(android.bluetooth.BluetoothDevice device) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((device!=null)) {
            _data.writeInt(1);
            device.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onBleEnrollmentDeviceDisconnected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onBleEnrollmentDeviceDisconnected(device);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.car.trust.ICarTrustAgentBleCallback sDefaultImpl;
    }
    static final int TRANSACTION_onEnrollmentAdvertisingStarted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onEnrollmentAdvertisingFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onBleEnrollmentDeviceConnected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onBleEnrollmentDeviceDisconnected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.car.trust.ICarTrustAgentBleCallback impl) {
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
    public static android.car.trust.ICarTrustAgentBleCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Called when the GATT server is started and BLE is successfully advertising for enrollment.
       */
  public void onEnrollmentAdvertisingStarted() throws android.os.RemoteException;
  /**
       * Called when the BLE enrollment advertisement fails to start.
       */
  public void onEnrollmentAdvertisingFailed() throws android.os.RemoteException;
  /**
       * Called when a remote device is connected on BLE.
       */
  public void onBleEnrollmentDeviceConnected(android.bluetooth.BluetoothDevice device) throws android.os.RemoteException;
  /**
       * Called when a remote device is disconnected on BLE.
       */
  public void onBleEnrollmentDeviceDisconnected(android.bluetooth.BluetoothDevice device) throws android.os.RemoteException;
}
