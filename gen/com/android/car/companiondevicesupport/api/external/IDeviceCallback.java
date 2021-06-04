/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.car.companiondevicesupport.api.external;
/** Triggered companionDevice events for a connected companionDevice. */
public interface IDeviceCallback extends android.os.IInterface
{
  /** Default implementation for IDeviceCallback. */
  public static class Default implements com.android.car.companiondevicesupport.api.external.IDeviceCallback
  {
    /**
         * Triggered when secure channel has been established on a companionDevice. Encrypted messaging
         * now available.
         */
    @Override public void onSecureChannelEstablished(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice) throws android.os.RemoteException
    {
    }
    /** Triggered when a new message is received from a companionDevice. */
    @Override public void onMessageReceived(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice, byte[] message) throws android.os.RemoteException
    {
    }
    /** Triggered when an error has occurred for a companionDevice. */
    @Override public void onDeviceError(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice, int error) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.car.companiondevicesupport.api.external.IDeviceCallback
  {
    private static final java.lang.String DESCRIPTOR = "com.android.car.companiondevicesupport.api.external.IDeviceCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.car.companiondevicesupport.api.external.IDeviceCallback interface,
     * generating a proxy if needed.
     */
    public static com.android.car.companiondevicesupport.api.external.IDeviceCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.car.companiondevicesupport.api.external.IDeviceCallback))) {
        return ((com.android.car.companiondevicesupport.api.external.IDeviceCallback)iin);
      }
      return new com.android.car.companiondevicesupport.api.external.IDeviceCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onSecureChannelEstablished:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.external.CompanionDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.car.companiondevicesupport.api.external.CompanionDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onSecureChannelEstablished(_arg0);
          return true;
        }
        case TRANSACTION_onMessageReceived:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.external.CompanionDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.car.companiondevicesupport.api.external.CompanionDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          byte[] _arg1;
          _arg1 = data.createByteArray();
          this.onMessageReceived(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onDeviceError:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.external.CompanionDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.car.companiondevicesupport.api.external.CompanionDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.onDeviceError(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.car.companiondevicesupport.api.external.IDeviceCallback
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
           * Triggered when secure channel has been established on a companionDevice. Encrypted messaging
           * now available.
           */
      @Override public void onSecureChannelEstablished(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((companionDevice!=null)) {
            _data.writeInt(1);
            companionDevice.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSecureChannelEstablished, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSecureChannelEstablished(companionDevice);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /** Triggered when a new message is received from a companionDevice. */
      @Override public void onMessageReceived(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice, byte[] message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((companionDevice!=null)) {
            _data.writeInt(1);
            companionDevice.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeByteArray(message);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onMessageReceived, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onMessageReceived(companionDevice, message);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /** Triggered when an error has occurred for a companionDevice. */
      @Override public void onDeviceError(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice, int error) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((companionDevice!=null)) {
            _data.writeInt(1);
            companionDevice.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(error);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDeviceError, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDeviceError(companionDevice, error);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.car.companiondevicesupport.api.external.IDeviceCallback sDefaultImpl;
    }
    static final int TRANSACTION_onSecureChannelEstablished = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onMessageReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onDeviceError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(com.android.car.companiondevicesupport.api.external.IDeviceCallback impl) {
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
    public static com.android.car.companiondevicesupport.api.external.IDeviceCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Triggered when secure channel has been established on a companionDevice. Encrypted messaging
       * now available.
       */
  public void onSecureChannelEstablished(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice) throws android.os.RemoteException;
  /** Triggered when a new message is received from a companionDevice. */
  public void onMessageReceived(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice, byte[] message) throws android.os.RemoteException;
  /** Triggered when an error has occurred for a companionDevice. */
  public void onDeviceError(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice, int error) throws android.os.RemoteException;
}
