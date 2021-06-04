/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.car.companiondevicesupport.api.internal.trust;
/** Callback for triggered trusted device events. */
public interface ITrustedDeviceCallback extends android.os.IInterface
{
  /** Default implementation for ITrustedDeviceCallback. */
  public static class Default implements com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceCallback
  {
    /** Triggered when a new device has been enrolled. */
    @Override public void onTrustedDeviceAdded(com.android.car.companiondevicesupport.api.internal.trust.TrustedDevice device) throws android.os.RemoteException
    {
    }
    /** Triggered when a new device has been unenrolled. */
    @Override public void onTrustedDeviceRemoved(com.android.car.companiondevicesupport.api.internal.trust.TrustedDevice device) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceCallback
  {
    private static final java.lang.String DESCRIPTOR = "com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceCallback interface,
     * generating a proxy if needed.
     */
    public static com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceCallback))) {
        return ((com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceCallback)iin);
      }
      return new com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onTrustedDeviceAdded:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.internal.trust.TrustedDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.car.companiondevicesupport.api.internal.trust.TrustedDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onTrustedDeviceAdded(_arg0);
          return true;
        }
        case TRANSACTION_onTrustedDeviceRemoved:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.internal.trust.TrustedDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.car.companiondevicesupport.api.internal.trust.TrustedDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onTrustedDeviceRemoved(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceCallback
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
      /** Triggered when a new device has been enrolled. */
      @Override public void onTrustedDeviceAdded(com.android.car.companiondevicesupport.api.internal.trust.TrustedDevice device) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_onTrustedDeviceAdded, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onTrustedDeviceAdded(device);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /** Triggered when a new device has been unenrolled. */
      @Override public void onTrustedDeviceRemoved(com.android.car.companiondevicesupport.api.internal.trust.TrustedDevice device) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_onTrustedDeviceRemoved, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onTrustedDeviceRemoved(device);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceCallback sDefaultImpl;
    }
    static final int TRANSACTION_onTrustedDeviceAdded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onTrustedDeviceRemoved = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceCallback impl) {
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
    public static com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /** Triggered when a new device has been enrolled. */
  public void onTrustedDeviceAdded(com.android.car.companiondevicesupport.api.internal.trust.TrustedDevice device) throws android.os.RemoteException;
  /** Triggered when a new device has been unenrolled. */
  public void onTrustedDeviceRemoved(com.android.car.companiondevicesupport.api.internal.trust.TrustedDevice device) throws android.os.RemoteException;
}
