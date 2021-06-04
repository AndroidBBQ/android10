/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.car.companiondevicesupport.api.internal.association;
/** Manager of devices associated with the car. */
public interface IAssociatedDeviceManager extends android.os.IInterface
{
  /** Default implementation for IAssociatedDeviceManager. */
  public static class Default implements com.android.car.companiondevicesupport.api.internal.association.IAssociatedDeviceManager
  {
    /**
         * Set a callback for association.
         * @param callback {@link IAssociationCallback} to set.
         */
    @Override public void setAssociationCallback(com.android.car.companiondevicesupport.api.internal.association.IAssociationCallback callback) throws android.os.RemoteException
    {
    }
    /** Clear the association callback from manager. */
    @Override public void clearAssociationCallback() throws android.os.RemoteException
    {
    }
    /** Starts the association with a new device. */
    @Override public void startAssociation() throws android.os.RemoteException
    {
    }
    /** Stops the association with current device. */
    @Override public void stopAssociation() throws android.os.RemoteException
    {
    }
    /** Returns {@link List<AssociatedDevice>} of devices associated with the given user. */
    @Override public java.util.List<com.android.car.companiondevicesupport.api.external.AssociatedDevice> getActiveUserAssociatedDevices() throws android.os.RemoteException
    {
      return null;
    }
    /** Confirms the paring code. */
    @Override public void acceptVerification() throws android.os.RemoteException
    {
    }
    /** Remove the associated device of the given identifier for the active user. */
    @Override public void removeAssociatedDevice(java.lang.String deviceId) throws android.os.RemoteException
    {
    }
    /**
         * Set a callback for associated device related events.
         *
         * @param callback {@link IDeviceAssociationCallback} to set.
         */
    @Override public void setDeviceAssociationCallback(com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback callback) throws android.os.RemoteException
    {
    }
    /** Clear the device association callback from manager. */
    @Override public void clearDeviceAssociationCallback() throws android.os.RemoteException
    {
    }
    /** Returns {@link List<CompanionDevice>} of devices currently connected. */
    @Override public java.util.List<com.android.car.companiondevicesupport.api.external.CompanionDevice> getActiveUserConnectedDevices() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Set a callback for connection events for only the currently active user's devices.
         *
         * @param callback {@link IConnectionCallback} to set.
         */
    @Override public void setConnectionCallback(com.android.car.companiondevicesupport.api.external.IConnectionCallback callback) throws android.os.RemoteException
    {
    }
    /** Clear the connection callback from manager. */
    @Override public void clearConnectionCallback() throws android.os.RemoteException
    {
    }
    /** Enable connection on the associated device with the given identifier. */
    @Override public void enableAssociatedDeviceConnection(java.lang.String deviceId) throws android.os.RemoteException
    {
    }
    /** Disable connection on the associated device with the given identifier. */
    @Override public void disableAssociatedDeviceConnection(java.lang.String deviceId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.car.companiondevicesupport.api.internal.association.IAssociatedDeviceManager
  {
    private static final java.lang.String DESCRIPTOR = "com.android.car.companiondevicesupport.api.internal.association.IAssociatedDeviceManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.car.companiondevicesupport.api.internal.association.IAssociatedDeviceManager interface,
     * generating a proxy if needed.
     */
    public static com.android.car.companiondevicesupport.api.internal.association.IAssociatedDeviceManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.car.companiondevicesupport.api.internal.association.IAssociatedDeviceManager))) {
        return ((com.android.car.companiondevicesupport.api.internal.association.IAssociatedDeviceManager)iin);
      }
      return new com.android.car.companiondevicesupport.api.internal.association.IAssociatedDeviceManager.Stub.Proxy(obj);
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
        case TRANSACTION_setAssociationCallback:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.internal.association.IAssociationCallback _arg0;
          _arg0 = com.android.car.companiondevicesupport.api.internal.association.IAssociationCallback.Stub.asInterface(data.readStrongBinder());
          this.setAssociationCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_clearAssociationCallback:
        {
          data.enforceInterface(descriptor);
          this.clearAssociationCallback();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startAssociation:
        {
          data.enforceInterface(descriptor);
          this.startAssociation();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopAssociation:
        {
          data.enforceInterface(descriptor);
          this.stopAssociation();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getActiveUserAssociatedDevices:
        {
          data.enforceInterface(descriptor);
          java.util.List<com.android.car.companiondevicesupport.api.external.AssociatedDevice> _result = this.getActiveUserAssociatedDevices();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_acceptVerification:
        {
          data.enforceInterface(descriptor);
          this.acceptVerification();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeAssociatedDevice:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.removeAssociatedDevice(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setDeviceAssociationCallback:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback _arg0;
          _arg0 = com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback.Stub.asInterface(data.readStrongBinder());
          this.setDeviceAssociationCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_clearDeviceAssociationCallback:
        {
          data.enforceInterface(descriptor);
          this.clearDeviceAssociationCallback();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getActiveUserConnectedDevices:
        {
          data.enforceInterface(descriptor);
          java.util.List<com.android.car.companiondevicesupport.api.external.CompanionDevice> _result = this.getActiveUserConnectedDevices();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_setConnectionCallback:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.external.IConnectionCallback _arg0;
          _arg0 = com.android.car.companiondevicesupport.api.external.IConnectionCallback.Stub.asInterface(data.readStrongBinder());
          this.setConnectionCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_clearConnectionCallback:
        {
          data.enforceInterface(descriptor);
          this.clearConnectionCallback();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_enableAssociatedDeviceConnection:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.enableAssociatedDeviceConnection(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_disableAssociatedDeviceConnection:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.disableAssociatedDeviceConnection(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.car.companiondevicesupport.api.internal.association.IAssociatedDeviceManager
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
           * Set a callback for association.
           * @param callback {@link IAssociationCallback} to set.
           */
      @Override public void setAssociationCallback(com.android.car.companiondevicesupport.api.internal.association.IAssociationCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAssociationCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAssociationCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Clear the association callback from manager. */
      @Override public void clearAssociationCallback() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearAssociationCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearAssociationCallback();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Starts the association with a new device. */
      @Override public void startAssociation() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_startAssociation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startAssociation();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Stops the association with current device. */
      @Override public void stopAssociation() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopAssociation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopAssociation();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Returns {@link List<AssociatedDevice>} of devices associated with the given user. */
      @Override public java.util.List<com.android.car.companiondevicesupport.api.external.AssociatedDevice> getActiveUserAssociatedDevices() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<com.android.car.companiondevicesupport.api.external.AssociatedDevice> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveUserAssociatedDevices, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActiveUserAssociatedDevices();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(com.android.car.companiondevicesupport.api.external.AssociatedDevice.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /** Confirms the paring code. */
      @Override public void acceptVerification() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_acceptVerification, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().acceptVerification();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Remove the associated device of the given identifier for the active user. */
      @Override public void removeAssociatedDevice(java.lang.String deviceId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(deviceId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeAssociatedDevice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeAssociatedDevice(deviceId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Set a callback for associated device related events.
           *
           * @param callback {@link IDeviceAssociationCallback} to set.
           */
      @Override public void setDeviceAssociationCallback(com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDeviceAssociationCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDeviceAssociationCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Clear the device association callback from manager. */
      @Override public void clearDeviceAssociationCallback() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearDeviceAssociationCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearDeviceAssociationCallback();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Returns {@link List<CompanionDevice>} of devices currently connected. */
      @Override public java.util.List<com.android.car.companiondevicesupport.api.external.CompanionDevice> getActiveUserConnectedDevices() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<com.android.car.companiondevicesupport.api.external.CompanionDevice> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveUserConnectedDevices, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActiveUserConnectedDevices();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(com.android.car.companiondevicesupport.api.external.CompanionDevice.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Set a callback for connection events for only the currently active user's devices.
           *
           * @param callback {@link IConnectionCallback} to set.
           */
      @Override public void setConnectionCallback(com.android.car.companiondevicesupport.api.external.IConnectionCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setConnectionCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setConnectionCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Clear the connection callback from manager. */
      @Override public void clearConnectionCallback() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearConnectionCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearConnectionCallback();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Enable connection on the associated device with the given identifier. */
      @Override public void enableAssociatedDeviceConnection(java.lang.String deviceId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(deviceId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableAssociatedDeviceConnection, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enableAssociatedDeviceConnection(deviceId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Disable connection on the associated device with the given identifier. */
      @Override public void disableAssociatedDeviceConnection(java.lang.String deviceId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(deviceId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableAssociatedDeviceConnection, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableAssociatedDeviceConnection(deviceId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.car.companiondevicesupport.api.internal.association.IAssociatedDeviceManager sDefaultImpl;
    }
    static final int TRANSACTION_setAssociationCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_clearAssociationCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_startAssociation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_stopAssociation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getActiveUserAssociatedDevices = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_acceptVerification = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_removeAssociatedDevice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_setDeviceAssociationCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_clearDeviceAssociationCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getActiveUserConnectedDevices = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_setConnectionCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_clearConnectionCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_enableAssociatedDeviceConnection = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_disableAssociatedDeviceConnection = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    public static boolean setDefaultImpl(com.android.car.companiondevicesupport.api.internal.association.IAssociatedDeviceManager impl) {
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
    public static com.android.car.companiondevicesupport.api.internal.association.IAssociatedDeviceManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Set a callback for association.
       * @param callback {@link IAssociationCallback} to set.
       */
  public void setAssociationCallback(com.android.car.companiondevicesupport.api.internal.association.IAssociationCallback callback) throws android.os.RemoteException;
  /** Clear the association callback from manager. */
  public void clearAssociationCallback() throws android.os.RemoteException;
  /** Starts the association with a new device. */
  public void startAssociation() throws android.os.RemoteException;
  /** Stops the association with current device. */
  public void stopAssociation() throws android.os.RemoteException;
  /** Returns {@link List<AssociatedDevice>} of devices associated with the given user. */
  public java.util.List<com.android.car.companiondevicesupport.api.external.AssociatedDevice> getActiveUserAssociatedDevices() throws android.os.RemoteException;
  /** Confirms the paring code. */
  public void acceptVerification() throws android.os.RemoteException;
  /** Remove the associated device of the given identifier for the active user. */
  public void removeAssociatedDevice(java.lang.String deviceId) throws android.os.RemoteException;
  /**
       * Set a callback for associated device related events.
       *
       * @param callback {@link IDeviceAssociationCallback} to set.
       */
  public void setDeviceAssociationCallback(com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback callback) throws android.os.RemoteException;
  /** Clear the device association callback from manager. */
  public void clearDeviceAssociationCallback() throws android.os.RemoteException;
  /** Returns {@link List<CompanionDevice>} of devices currently connected. */
  public java.util.List<com.android.car.companiondevicesupport.api.external.CompanionDevice> getActiveUserConnectedDevices() throws android.os.RemoteException;
  /**
       * Set a callback for connection events for only the currently active user's devices.
       *
       * @param callback {@link IConnectionCallback} to set.
       */
  public void setConnectionCallback(com.android.car.companiondevicesupport.api.external.IConnectionCallback callback) throws android.os.RemoteException;
  /** Clear the connection callback from manager. */
  public void clearConnectionCallback() throws android.os.RemoteException;
  /** Enable connection on the associated device with the given identifier. */
  public void enableAssociatedDeviceConnection(java.lang.String deviceId) throws android.os.RemoteException;
  /** Disable connection on the associated device with the given identifier. */
  public void disableAssociatedDeviceConnection(java.lang.String deviceId) throws android.os.RemoteException;
}
