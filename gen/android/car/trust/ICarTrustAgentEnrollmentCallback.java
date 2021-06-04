/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.trust;
/**
 * Callback interface for state changes during Trusted device enrollment.
 *
 * @hide
 */
public interface ICarTrustAgentEnrollmentCallback extends android.os.IInterface
{
  /** Default implementation for ICarTrustAgentEnrollmentCallback. */
  public static class Default implements android.car.trust.ICarTrustAgentEnrollmentCallback
  {
    /**
         * Communicate about failure/timeouts in the handshake process.
         */
    @Override public void onEnrollmentHandshakeFailure(android.bluetooth.BluetoothDevice device, int errorCode) throws android.os.RemoteException
    {
    }
    /**
         * Present the pairing/authentication string to the user.
         */
    @Override public void onAuthStringAvailable(android.bluetooth.BluetoothDevice device, java.lang.String authString) throws android.os.RemoteException
    {
    }
    /**
         * Escrow token was received and the Trust Agent framework has generated a corresponding handle.
         */
    @Override public void onEscrowTokenAdded(long handle) throws android.os.RemoteException
    {
    }
    /**
         * Escrow token was removed as a result of a call to
         * {@link CarTrustAgentEnrollmentManager#removeEscrowToken(long handle, int uid)}. The peer
         * device associated with this token is not trusted for authentication anymore.
         */
    @Override public void onEscrowTokenRemoved(long handle) throws android.os.RemoteException
    {
    }
    /**
         * Escrow token's active state changed.
         */
    @Override public void onEscrowTokenActiveStateChanged(long handle, boolean active) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.trust.ICarTrustAgentEnrollmentCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.car.trust.ICarTrustAgentEnrollmentCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.trust.ICarTrustAgentEnrollmentCallback interface,
     * generating a proxy if needed.
     */
    public static android.car.trust.ICarTrustAgentEnrollmentCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.trust.ICarTrustAgentEnrollmentCallback))) {
        return ((android.car.trust.ICarTrustAgentEnrollmentCallback)iin);
      }
      return new android.car.trust.ICarTrustAgentEnrollmentCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onEnrollmentHandshakeFailure:
        {
          data.enforceInterface(descriptor);
          android.bluetooth.BluetoothDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.bluetooth.BluetoothDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.onEnrollmentHandshakeFailure(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onAuthStringAvailable:
        {
          data.enforceInterface(descriptor);
          android.bluetooth.BluetoothDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.bluetooth.BluetoothDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.onAuthStringAvailable(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onEscrowTokenAdded:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.onEscrowTokenAdded(_arg0);
          return true;
        }
        case TRANSACTION_onEscrowTokenRemoved:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.onEscrowTokenRemoved(_arg0);
          return true;
        }
        case TRANSACTION_onEscrowTokenActiveStateChanged:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.onEscrowTokenActiveStateChanged(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.trust.ICarTrustAgentEnrollmentCallback
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
           * Communicate about failure/timeouts in the handshake process.
           */
      @Override public void onEnrollmentHandshakeFailure(android.bluetooth.BluetoothDevice device, int errorCode) throws android.os.RemoteException
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
          _data.writeInt(errorCode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEnrollmentHandshakeFailure, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEnrollmentHandshakeFailure(device, errorCode);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Present the pairing/authentication string to the user.
           */
      @Override public void onAuthStringAvailable(android.bluetooth.BluetoothDevice device, java.lang.String authString) throws android.os.RemoteException
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
          _data.writeString(authString);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAuthStringAvailable, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAuthStringAvailable(device, authString);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Escrow token was received and the Trust Agent framework has generated a corresponding handle.
           */
      @Override public void onEscrowTokenAdded(long handle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(handle);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEscrowTokenAdded, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEscrowTokenAdded(handle);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Escrow token was removed as a result of a call to
           * {@link CarTrustAgentEnrollmentManager#removeEscrowToken(long handle, int uid)}. The peer
           * device associated with this token is not trusted for authentication anymore.
           */
      @Override public void onEscrowTokenRemoved(long handle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(handle);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEscrowTokenRemoved, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEscrowTokenRemoved(handle);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Escrow token's active state changed.
           */
      @Override public void onEscrowTokenActiveStateChanged(long handle, boolean active) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(handle);
          _data.writeInt(((active)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEscrowTokenActiveStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEscrowTokenActiveStateChanged(handle, active);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.car.trust.ICarTrustAgentEnrollmentCallback sDefaultImpl;
    }
    static final int TRANSACTION_onEnrollmentHandshakeFailure = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onAuthStringAvailable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onEscrowTokenAdded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onEscrowTokenRemoved = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onEscrowTokenActiveStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(android.car.trust.ICarTrustAgentEnrollmentCallback impl) {
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
    public static android.car.trust.ICarTrustAgentEnrollmentCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Communicate about failure/timeouts in the handshake process.
       */
  public void onEnrollmentHandshakeFailure(android.bluetooth.BluetoothDevice device, int errorCode) throws android.os.RemoteException;
  /**
       * Present the pairing/authentication string to the user.
       */
  public void onAuthStringAvailable(android.bluetooth.BluetoothDevice device, java.lang.String authString) throws android.os.RemoteException;
  /**
       * Escrow token was received and the Trust Agent framework has generated a corresponding handle.
       */
  public void onEscrowTokenAdded(long handle) throws android.os.RemoteException;
  /**
       * Escrow token was removed as a result of a call to
       * {@link CarTrustAgentEnrollmentManager#removeEscrowToken(long handle, int uid)}. The peer
       * device associated with this token is not trusted for authentication anymore.
       */
  public void onEscrowTokenRemoved(long handle) throws android.os.RemoteException;
  /**
       * Escrow token's active state changed.
       */
  public void onEscrowTokenActiveStateChanged(long handle, boolean active) throws android.os.RemoteException;
}
