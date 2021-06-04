/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.car.companiondevicesupport.api.external;
/** Callback for triggered connection events. */
public interface IConnectionCallback extends android.os.IInterface
{
  /** Default implementation for IConnectionCallback. */
  public static class Default implements com.android.car.companiondevicesupport.api.external.IConnectionCallback
  {
    /** Triggered when a new companionDevice has connected. */
    @Override public void onDeviceConnected(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice) throws android.os.RemoteException
    {
    }
    /** Triggered when a companionDevice has disconnected. */
    @Override public void onDeviceDisconnected(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.car.companiondevicesupport.api.external.IConnectionCallback
  {
    private static final java.lang.String DESCRIPTOR = "com.android.car.companiondevicesupport.api.external.IConnectionCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.car.companiondevicesupport.api.external.IConnectionCallback interface,
     * generating a proxy if needed.
     */
    public static com.android.car.companiondevicesupport.api.external.IConnectionCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.car.companiondevicesupport.api.external.IConnectionCallback))) {
        return ((com.android.car.companiondevicesupport.api.external.IConnectionCallback)iin);
      }
      return new com.android.car.companiondevicesupport.api.external.IConnectionCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onDeviceConnected:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.external.CompanionDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.car.companiondevicesupport.api.external.CompanionDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onDeviceConnected(_arg0);
          return true;
        }
        case TRANSACTION_onDeviceDisconnected:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.external.CompanionDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.car.companiondevicesupport.api.external.CompanionDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onDeviceDisconnected(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.car.companiondevicesupport.api.external.IConnectionCallback
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
      /** Triggered when a new companionDevice has connected. */
      @Override public void onDeviceConnected(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDeviceConnected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDeviceConnected(companionDevice);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /** Triggered when a companionDevice has disconnected. */
      @Override public void onDeviceDisconnected(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDeviceDisconnected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDeviceDisconnected(companionDevice);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.car.companiondevicesupport.api.external.IConnectionCallback sDefaultImpl;
    }
    static final int TRANSACTION_onDeviceConnected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onDeviceDisconnected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(com.android.car.companiondevicesupport.api.external.IConnectionCallback impl) {
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
    public static com.android.car.companiondevicesupport.api.external.IConnectionCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /** Triggered when a new companionDevice has connected. */
  public void onDeviceConnected(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice) throws android.os.RemoteException;
  /** Triggered when a companionDevice has disconnected. */
  public void onDeviceDisconnected(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice) throws android.os.RemoteException;
}
