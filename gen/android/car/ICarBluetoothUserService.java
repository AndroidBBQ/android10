/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car;
/** @hide */
public interface ICarBluetoothUserService extends android.os.IInterface
{
  /** Default implementation for ICarBluetoothUserService. */
  public static class Default implements android.car.ICarBluetoothUserService
  {
    // Bluetooth related APIs

    @Override public void setupBluetoothConnectionProxies() throws android.os.RemoteException
    {
    }
    @Override public void closeBluetoothConnectionProxies() throws android.os.RemoteException
    {
    }
    @Override public boolean isBluetoothConnectionProxyAvailable(int profile) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean bluetoothConnectToProfile(int profile, android.bluetooth.BluetoothDevice device) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean bluetoothDisconnectFromProfile(int profile, android.bluetooth.BluetoothDevice device) throws android.os.RemoteException
    {
      return false;
    }
    @Override public int getProfilePriority(int profile, android.bluetooth.BluetoothDevice device) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setProfilePriority(int profile, android.bluetooth.BluetoothDevice device, int priority) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.ICarBluetoothUserService
  {
    private static final java.lang.String DESCRIPTOR = "android.car.ICarBluetoothUserService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.ICarBluetoothUserService interface,
     * generating a proxy if needed.
     */
    public static android.car.ICarBluetoothUserService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.ICarBluetoothUserService))) {
        return ((android.car.ICarBluetoothUserService)iin);
      }
      return new android.car.ICarBluetoothUserService.Stub.Proxy(obj);
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
        case TRANSACTION_setupBluetoothConnectionProxies:
        {
          data.enforceInterface(descriptor);
          this.setupBluetoothConnectionProxies();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_closeBluetoothConnectionProxies:
        {
          data.enforceInterface(descriptor);
          this.closeBluetoothConnectionProxies();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isBluetoothConnectionProxyAvailable:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isBluetoothConnectionProxyAvailable(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_bluetoothConnectToProfile:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.bluetooth.BluetoothDevice _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.bluetooth.BluetoothDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _result = this.bluetoothConnectToProfile(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_bluetoothDisconnectFromProfile:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.bluetooth.BluetoothDevice _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.bluetooth.BluetoothDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _result = this.bluetoothDisconnectFromProfile(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getProfilePriority:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.bluetooth.BluetoothDevice _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.bluetooth.BluetoothDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _result = this.getProfilePriority(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setProfilePriority:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.bluetooth.BluetoothDevice _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.bluetooth.BluetoothDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          this.setProfilePriority(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.ICarBluetoothUserService
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
      // Bluetooth related APIs

      @Override public void setupBluetoothConnectionProxies() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setupBluetoothConnectionProxies, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setupBluetoothConnectionProxies();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void closeBluetoothConnectionProxies() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_closeBluetoothConnectionProxies, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().closeBluetoothConnectionProxies();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isBluetoothConnectionProxyAvailable(int profile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(profile);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isBluetoothConnectionProxyAvailable, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isBluetoothConnectionProxyAvailable(profile);
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
      @Override public boolean bluetoothConnectToProfile(int profile, android.bluetooth.BluetoothDevice device) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(profile);
          if ((device!=null)) {
            _data.writeInt(1);
            device.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_bluetoothConnectToProfile, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().bluetoothConnectToProfile(profile, device);
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
      @Override public boolean bluetoothDisconnectFromProfile(int profile, android.bluetooth.BluetoothDevice device) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(profile);
          if ((device!=null)) {
            _data.writeInt(1);
            device.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_bluetoothDisconnectFromProfile, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().bluetoothDisconnectFromProfile(profile, device);
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
      @Override public int getProfilePriority(int profile, android.bluetooth.BluetoothDevice device) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(profile);
          if ((device!=null)) {
            _data.writeInt(1);
            device.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getProfilePriority, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getProfilePriority(profile, device);
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
      @Override public void setProfilePriority(int profile, android.bluetooth.BluetoothDevice device, int priority) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(profile);
          if ((device!=null)) {
            _data.writeInt(1);
            device.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(priority);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setProfilePriority, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setProfilePriority(profile, device, priority);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.car.ICarBluetoothUserService sDefaultImpl;
    }
    static final int TRANSACTION_setupBluetoothConnectionProxies = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_closeBluetoothConnectionProxies = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_isBluetoothConnectionProxyAvailable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_bluetoothConnectToProfile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_bluetoothDisconnectFromProfile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getProfilePriority = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_setProfilePriority = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    public static boolean setDefaultImpl(android.car.ICarBluetoothUserService impl) {
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
    public static android.car.ICarBluetoothUserService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // Bluetooth related APIs

  public void setupBluetoothConnectionProxies() throws android.os.RemoteException;
  public void closeBluetoothConnectionProxies() throws android.os.RemoteException;
  public boolean isBluetoothConnectionProxyAvailable(int profile) throws android.os.RemoteException;
  public boolean bluetoothConnectToProfile(int profile, android.bluetooth.BluetoothDevice device) throws android.os.RemoteException;
  public boolean bluetoothDisconnectFromProfile(int profile, android.bluetooth.BluetoothDevice device) throws android.os.RemoteException;
  public int getProfilePriority(int profile, android.bluetooth.BluetoothDevice device) throws android.os.RemoteException;
  public void setProfilePriority(int profile, android.bluetooth.BluetoothDevice device, int priority) throws android.os.RemoteException;
}
