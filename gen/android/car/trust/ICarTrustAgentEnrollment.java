/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.trust;
/**
 * Binder interface for CarTrustAgentEnrollmentService. The service implements the functionality
 * to communicate with the remote device securely to enroll the remote device as a trusted device.
 *
 * @hide
 */
public interface ICarTrustAgentEnrollment extends android.os.IInterface
{
  /** Default implementation for ICarTrustAgentEnrollment. */
  public static class Default implements android.car.trust.ICarTrustAgentEnrollment
  {
    @Override public void startEnrollmentAdvertising() throws android.os.RemoteException
    {
    }
    @Override public void stopEnrollmentAdvertising() throws android.os.RemoteException
    {
    }
    @Override public void enrollmentHandshakeAccepted(android.bluetooth.BluetoothDevice device) throws android.os.RemoteException
    {
    }
    @Override public void terminateEnrollmentHandshake() throws android.os.RemoteException
    {
    }
    @Override public boolean isEscrowTokenActive(long handle, int uid) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void removeEscrowToken(long handle, int uid) throws android.os.RemoteException
    {
    }
    @Override public void removeAllTrustedDevices(int uid) throws android.os.RemoteException
    {
    }
    @Override public void setTrustedDeviceEnrollmentEnabled(boolean enable) throws android.os.RemoteException
    {
    }
    @Override public void setTrustedDeviceUnlockEnabled(boolean enable) throws android.os.RemoteException
    {
    }
    @Override public java.util.List<android.car.trust.TrustedDeviceInfo> getEnrolledDeviceInfosForUser(int uid) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void registerEnrollmentCallback(android.car.trust.ICarTrustAgentEnrollmentCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void unregisterEnrollmentCallback(android.car.trust.ICarTrustAgentEnrollmentCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void registerBleCallback(android.car.trust.ICarTrustAgentBleCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void unregisterBleCallback(android.car.trust.ICarTrustAgentBleCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.trust.ICarTrustAgentEnrollment
  {
    private static final java.lang.String DESCRIPTOR = "android.car.trust.ICarTrustAgentEnrollment";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.trust.ICarTrustAgentEnrollment interface,
     * generating a proxy if needed.
     */
    public static android.car.trust.ICarTrustAgentEnrollment asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.trust.ICarTrustAgentEnrollment))) {
        return ((android.car.trust.ICarTrustAgentEnrollment)iin);
      }
      return new android.car.trust.ICarTrustAgentEnrollment.Stub.Proxy(obj);
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
        case TRANSACTION_startEnrollmentAdvertising:
        {
          data.enforceInterface(descriptor);
          this.startEnrollmentAdvertising();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopEnrollmentAdvertising:
        {
          data.enforceInterface(descriptor);
          this.stopEnrollmentAdvertising();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_enrollmentHandshakeAccepted:
        {
          data.enforceInterface(descriptor);
          android.bluetooth.BluetoothDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.bluetooth.BluetoothDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.enrollmentHandshakeAccepted(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_terminateEnrollmentHandshake:
        {
          data.enforceInterface(descriptor);
          this.terminateEnrollmentHandshake();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isEscrowTokenActive:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.isEscrowTokenActive(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_removeEscrowToken:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          this.removeEscrowToken(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeAllTrustedDevices:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.removeAllTrustedDevices(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setTrustedDeviceEnrollmentEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setTrustedDeviceEnrollmentEnabled(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setTrustedDeviceUnlockEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setTrustedDeviceUnlockEnabled(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getEnrolledDeviceInfosForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<android.car.trust.TrustedDeviceInfo> _result = this.getEnrolledDeviceInfosForUser(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_registerEnrollmentCallback:
        {
          data.enforceInterface(descriptor);
          android.car.trust.ICarTrustAgentEnrollmentCallback _arg0;
          _arg0 = android.car.trust.ICarTrustAgentEnrollmentCallback.Stub.asInterface(data.readStrongBinder());
          this.registerEnrollmentCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterEnrollmentCallback:
        {
          data.enforceInterface(descriptor);
          android.car.trust.ICarTrustAgentEnrollmentCallback _arg0;
          _arg0 = android.car.trust.ICarTrustAgentEnrollmentCallback.Stub.asInterface(data.readStrongBinder());
          this.unregisterEnrollmentCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerBleCallback:
        {
          data.enforceInterface(descriptor);
          android.car.trust.ICarTrustAgentBleCallback _arg0;
          _arg0 = android.car.trust.ICarTrustAgentBleCallback.Stub.asInterface(data.readStrongBinder());
          this.registerBleCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterBleCallback:
        {
          data.enforceInterface(descriptor);
          android.car.trust.ICarTrustAgentBleCallback _arg0;
          _arg0 = android.car.trust.ICarTrustAgentBleCallback.Stub.asInterface(data.readStrongBinder());
          this.unregisterBleCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.trust.ICarTrustAgentEnrollment
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
      @Override public void startEnrollmentAdvertising() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startEnrollmentAdvertising, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startEnrollmentAdvertising();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stopEnrollmentAdvertising() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopEnrollmentAdvertising, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopEnrollmentAdvertising();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void enrollmentHandshakeAccepted(android.bluetooth.BluetoothDevice device) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((device!=null)) {
            _data.writeInt(1);
            device.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_enrollmentHandshakeAccepted, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enrollmentHandshakeAccepted(device);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void terminateEnrollmentHandshake() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_terminateEnrollmentHandshake, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().terminateEnrollmentHandshake();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isEscrowTokenActive(long handle, int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(handle);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isEscrowTokenActive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isEscrowTokenActive(handle, uid);
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
      @Override public void removeEscrowToken(long handle, int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(handle);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeEscrowToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeEscrowToken(handle, uid);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeAllTrustedDevices(int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeAllTrustedDevices, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeAllTrustedDevices(uid);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setTrustedDeviceEnrollmentEnabled(boolean enable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTrustedDeviceEnrollmentEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setTrustedDeviceEnrollmentEnabled(enable);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setTrustedDeviceUnlockEnabled(boolean enable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTrustedDeviceUnlockEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setTrustedDeviceUnlockEnabled(enable);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.util.List<android.car.trust.TrustedDeviceInfo> getEnrolledDeviceInfosForUser(int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.car.trust.TrustedDeviceInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getEnrolledDeviceInfosForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getEnrolledDeviceInfosForUser(uid);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.car.trust.TrustedDeviceInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void registerEnrollmentCallback(android.car.trust.ICarTrustAgentEnrollmentCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerEnrollmentCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerEnrollmentCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterEnrollmentCallback(android.car.trust.ICarTrustAgentEnrollmentCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterEnrollmentCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterEnrollmentCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void registerBleCallback(android.car.trust.ICarTrustAgentBleCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerBleCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerBleCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterBleCallback(android.car.trust.ICarTrustAgentBleCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterBleCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterBleCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.car.trust.ICarTrustAgentEnrollment sDefaultImpl;
    }
    static final int TRANSACTION_startEnrollmentAdvertising = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_stopEnrollmentAdvertising = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_enrollmentHandshakeAccepted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_terminateEnrollmentHandshake = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_isEscrowTokenActive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_removeEscrowToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_removeAllTrustedDevices = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_setTrustedDeviceEnrollmentEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_setTrustedDeviceUnlockEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getEnrolledDeviceInfosForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_registerEnrollmentCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_unregisterEnrollmentCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_registerBleCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_unregisterBleCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    public static boolean setDefaultImpl(android.car.trust.ICarTrustAgentEnrollment impl) {
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
    public static android.car.trust.ICarTrustAgentEnrollment getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void startEnrollmentAdvertising() throws android.os.RemoteException;
  public void stopEnrollmentAdvertising() throws android.os.RemoteException;
  public void enrollmentHandshakeAccepted(android.bluetooth.BluetoothDevice device) throws android.os.RemoteException;
  public void terminateEnrollmentHandshake() throws android.os.RemoteException;
  public boolean isEscrowTokenActive(long handle, int uid) throws android.os.RemoteException;
  public void removeEscrowToken(long handle, int uid) throws android.os.RemoteException;
  public void removeAllTrustedDevices(int uid) throws android.os.RemoteException;
  public void setTrustedDeviceEnrollmentEnabled(boolean enable) throws android.os.RemoteException;
  public void setTrustedDeviceUnlockEnabled(boolean enable) throws android.os.RemoteException;
  public java.util.List<android.car.trust.TrustedDeviceInfo> getEnrolledDeviceInfosForUser(int uid) throws android.os.RemoteException;
  public void registerEnrollmentCallback(android.car.trust.ICarTrustAgentEnrollmentCallback callback) throws android.os.RemoteException;
  public void unregisterEnrollmentCallback(android.car.trust.ICarTrustAgentEnrollmentCallback callback) throws android.os.RemoteException;
  public void registerBleCallback(android.car.trust.ICarTrustAgentBleCallback callback) throws android.os.RemoteException;
  public void unregisterBleCallback(android.car.trust.ICarTrustAgentBleCallback callback) throws android.os.RemoteException;
}
