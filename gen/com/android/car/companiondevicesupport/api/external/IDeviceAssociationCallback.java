/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.car.companiondevicesupport.api.external;
/** Callback for triggered associated device related events. */
public interface IDeviceAssociationCallback extends android.os.IInterface
{
  /** Default implementation for IDeviceAssociationCallback. */
  public static class Default implements com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback
  {
    /** Triggered when an associated device has been added */
    @Override public void onAssociatedDeviceAdded(com.android.car.companiondevicesupport.api.external.AssociatedDevice device) throws android.os.RemoteException
    {
    }
    /** Triggered when an associated device has been removed.  */
    @Override public void onAssociatedDeviceRemoved(com.android.car.companiondevicesupport.api.external.AssociatedDevice device) throws android.os.RemoteException
    {
    }
    /** Triggered when an associated device has been updated. */
    @Override public void onAssociatedDeviceUpdated(com.android.car.companiondevicesupport.api.external.AssociatedDevice device) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback
  {
    private static final java.lang.String DESCRIPTOR = "com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback interface,
     * generating a proxy if needed.
     */
    public static com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback))) {
        return ((com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback)iin);
      }
      return new com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onAssociatedDeviceAdded:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.external.AssociatedDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.car.companiondevicesupport.api.external.AssociatedDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onAssociatedDeviceAdded(_arg0);
          return true;
        }
        case TRANSACTION_onAssociatedDeviceRemoved:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.external.AssociatedDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.car.companiondevicesupport.api.external.AssociatedDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onAssociatedDeviceRemoved(_arg0);
          return true;
        }
        case TRANSACTION_onAssociatedDeviceUpdated:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.external.AssociatedDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.car.companiondevicesupport.api.external.AssociatedDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onAssociatedDeviceUpdated(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback
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
      /** Triggered when an associated device has been added */
      @Override public void onAssociatedDeviceAdded(com.android.car.companiondevicesupport.api.external.AssociatedDevice device) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAssociatedDeviceAdded, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAssociatedDeviceAdded(device);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /** Triggered when an associated device has been removed.  */
      @Override public void onAssociatedDeviceRemoved(com.android.car.companiondevicesupport.api.external.AssociatedDevice device) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAssociatedDeviceRemoved, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAssociatedDeviceRemoved(device);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /** Triggered when an associated device has been updated. */
      @Override public void onAssociatedDeviceUpdated(com.android.car.companiondevicesupport.api.external.AssociatedDevice device) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_onAssociatedDeviceUpdated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onAssociatedDeviceUpdated(device);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback sDefaultImpl;
    }
    static final int TRANSACTION_onAssociatedDeviceAdded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onAssociatedDeviceRemoved = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onAssociatedDeviceUpdated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback impl) {
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
    public static com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /** Triggered when an associated device has been added */
  public void onAssociatedDeviceAdded(com.android.car.companiondevicesupport.api.external.AssociatedDevice device) throws android.os.RemoteException;
  /** Triggered when an associated device has been removed.  */
  public void onAssociatedDeviceRemoved(com.android.car.companiondevicesupport.api.external.AssociatedDevice device) throws android.os.RemoteException;
  /** Triggered when an associated device has been updated. */
  public void onAssociatedDeviceUpdated(com.android.car.companiondevicesupport.api.external.AssociatedDevice device) throws android.os.RemoteException;
}
