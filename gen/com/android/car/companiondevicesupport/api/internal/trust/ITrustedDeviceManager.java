/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.car.companiondevicesupport.api.internal.trust;
/**
 * Manager of trusted devices with the car to be used by any service/activity that needs to interact
 * with trusted devices.
 */
public interface ITrustedDeviceManager extends android.os.IInterface
{
  /** Default implementation for ITrustedDeviceManager. */
  public static class Default implements com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceManager
  {
    /** Indicate the escrow token has been added for a user and corresponding handle. */
    @Override public void onEscrowTokenAdded(int userId, long handle) throws android.os.RemoteException
    {
    }
    /** Indicate the escrow token has been activated for a user and corresponding handle. */
    @Override public void onEscrowTokenActivated(int userId, long handle) throws android.os.RemoteException
    {
    }
    /** Register a new callback for trusted device events. */
    @Override public void registerTrustedDeviceCallback(com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceCallback callback) throws android.os.RemoteException
    {
    }
    /** Remove a previously registered callback. */
    @Override public void unregisterTrustedDeviceCallback(com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceCallback callback) throws android.os.RemoteException
    {
    }
    /** Register a new callback for enrollment triggered events. */
    @Override public void registerTrustedDeviceEnrollmentCallback(com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceEnrollmentCallback callback) throws android.os.RemoteException
    {
    }
    /** Remove a previously registered callback. */
    @Override public void unregisterTrustedDeviceEnrollmentCallback(com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceEnrollmentCallback callback) throws android.os.RemoteException
    {
    }
    /** Set a delegate for TrustAgent operation calls. */
    @Override public void setTrustedDeviceAgentDelegate(com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceAgentDelegate trustAgentDelegate) throws android.os.RemoteException
    {
    }
    /** Remove a prevoiusly set delegate. */
    @Override public void clearTrustedDeviceAgentDelegate(com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceAgentDelegate trustAgentDelegate) throws android.os.RemoteException
    {
    }
    /** Returns a list of trusted devices for user. */
    @Override public java.util.List<com.android.car.companiondevicesupport.api.internal.trust.TrustedDevice> getTrustedDevicesForActiveUser() throws android.os.RemoteException
    {
      return null;
    }
    /** Remove a trusted device and invalidate any credentials associated with it. */
    @Override public void removeTrustedDevice(com.android.car.companiondevicesupport.api.internal.trust.TrustedDevice trustedDevice) throws android.os.RemoteException
    {
    }
    /** Returns {@link List<CompanionDevice>} of devices currently connected. */
    @Override public java.util.List<com.android.car.companiondevicesupport.api.external.CompanionDevice> getActiveUserConnectedDevices() throws android.os.RemoteException
    {
      return null;
    }
    /** Register a new callback for associated device events. */
    @Override public void registerAssociatedDeviceCallback(com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback callback) throws android.os.RemoteException
    {
    }
    /** Remove a previously registered callback. */
    @Override public void unregisterAssociatedDeviceCallback(com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback callback) throws android.os.RemoteException
    {
    }
    /** Attempts to initiate trusted device enrollment on the phone with the given device id. */
    @Override public void initiateEnrollment(java.lang.String deviceId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceManager
  {
    private static final java.lang.String DESCRIPTOR = "com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceManager interface,
     * generating a proxy if needed.
     */
    public static com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceManager))) {
        return ((com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceManager)iin);
      }
      return new com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceManager.Stub.Proxy(obj);
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
        case TRANSACTION_onEscrowTokenAdded:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          this.onEscrowTokenAdded(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onEscrowTokenActivated:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          this.onEscrowTokenActivated(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerTrustedDeviceCallback:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceCallback _arg0;
          _arg0 = com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceCallback.Stub.asInterface(data.readStrongBinder());
          this.registerTrustedDeviceCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterTrustedDeviceCallback:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceCallback _arg0;
          _arg0 = com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceCallback.Stub.asInterface(data.readStrongBinder());
          this.unregisterTrustedDeviceCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerTrustedDeviceEnrollmentCallback:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceEnrollmentCallback _arg0;
          _arg0 = com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceEnrollmentCallback.Stub.asInterface(data.readStrongBinder());
          this.registerTrustedDeviceEnrollmentCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterTrustedDeviceEnrollmentCallback:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceEnrollmentCallback _arg0;
          _arg0 = com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceEnrollmentCallback.Stub.asInterface(data.readStrongBinder());
          this.unregisterTrustedDeviceEnrollmentCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setTrustedDeviceAgentDelegate:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceAgentDelegate _arg0;
          _arg0 = com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceAgentDelegate.Stub.asInterface(data.readStrongBinder());
          this.setTrustedDeviceAgentDelegate(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_clearTrustedDeviceAgentDelegate:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceAgentDelegate _arg0;
          _arg0 = com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceAgentDelegate.Stub.asInterface(data.readStrongBinder());
          this.clearTrustedDeviceAgentDelegate(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getTrustedDevicesForActiveUser:
        {
          data.enforceInterface(descriptor);
          java.util.List<com.android.car.companiondevicesupport.api.internal.trust.TrustedDevice> _result = this.getTrustedDevicesForActiveUser();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_removeTrustedDevice:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.internal.trust.TrustedDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.car.companiondevicesupport.api.internal.trust.TrustedDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.removeTrustedDevice(_arg0);
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
        case TRANSACTION_registerAssociatedDeviceCallback:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback _arg0;
          _arg0 = com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback.Stub.asInterface(data.readStrongBinder());
          this.registerAssociatedDeviceCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterAssociatedDeviceCallback:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback _arg0;
          _arg0 = com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback.Stub.asInterface(data.readStrongBinder());
          this.unregisterAssociatedDeviceCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_initiateEnrollment:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.initiateEnrollment(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceManager
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
      /** Indicate the escrow token has been added for a user and corresponding handle. */
      @Override public void onEscrowTokenAdded(int userId, long handle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeLong(handle);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEscrowTokenAdded, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEscrowTokenAdded(userId, handle);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Indicate the escrow token has been activated for a user and corresponding handle. */
      @Override public void onEscrowTokenActivated(int userId, long handle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          _data.writeLong(handle);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEscrowTokenActivated, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEscrowTokenActivated(userId, handle);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Register a new callback for trusted device events. */
      @Override public void registerTrustedDeviceCallback(com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerTrustedDeviceCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerTrustedDeviceCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Remove a previously registered callback. */
      @Override public void unregisterTrustedDeviceCallback(com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterTrustedDeviceCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterTrustedDeviceCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Register a new callback for enrollment triggered events. */
      @Override public void registerTrustedDeviceEnrollmentCallback(com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceEnrollmentCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerTrustedDeviceEnrollmentCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerTrustedDeviceEnrollmentCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Remove a previously registered callback. */
      @Override public void unregisterTrustedDeviceEnrollmentCallback(com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceEnrollmentCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterTrustedDeviceEnrollmentCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterTrustedDeviceEnrollmentCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Set a delegate for TrustAgent operation calls. */
      @Override public void setTrustedDeviceAgentDelegate(com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceAgentDelegate trustAgentDelegate) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((trustAgentDelegate!=null))?(trustAgentDelegate.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTrustedDeviceAgentDelegate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setTrustedDeviceAgentDelegate(trustAgentDelegate);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Remove a prevoiusly set delegate. */
      @Override public void clearTrustedDeviceAgentDelegate(com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceAgentDelegate trustAgentDelegate) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((trustAgentDelegate!=null))?(trustAgentDelegate.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearTrustedDeviceAgentDelegate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearTrustedDeviceAgentDelegate(trustAgentDelegate);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Returns a list of trusted devices for user. */
      @Override public java.util.List<com.android.car.companiondevicesupport.api.internal.trust.TrustedDevice> getTrustedDevicesForActiveUser() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<com.android.car.companiondevicesupport.api.internal.trust.TrustedDevice> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTrustedDevicesForActiveUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTrustedDevicesForActiveUser();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(com.android.car.companiondevicesupport.api.internal.trust.TrustedDevice.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /** Remove a trusted device and invalidate any credentials associated with it. */
      @Override public void removeTrustedDevice(com.android.car.companiondevicesupport.api.internal.trust.TrustedDevice trustedDevice) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((trustedDevice!=null)) {
            _data.writeInt(1);
            trustedDevice.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeTrustedDevice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeTrustedDevice(trustedDevice);
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
      /** Register a new callback for associated device events. */
      @Override public void registerAssociatedDeviceCallback(com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerAssociatedDeviceCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerAssociatedDeviceCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Remove a previously registered callback. */
      @Override public void unregisterAssociatedDeviceCallback(com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterAssociatedDeviceCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterAssociatedDeviceCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /** Attempts to initiate trusted device enrollment on the phone with the given device id. */
      @Override public void initiateEnrollment(java.lang.String deviceId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(deviceId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_initiateEnrollment, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().initiateEnrollment(deviceId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceManager sDefaultImpl;
    }
    static final int TRANSACTION_onEscrowTokenAdded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onEscrowTokenActivated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_registerTrustedDeviceCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_unregisterTrustedDeviceCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_registerTrustedDeviceEnrollmentCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_unregisterTrustedDeviceEnrollmentCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_setTrustedDeviceAgentDelegate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_clearTrustedDeviceAgentDelegate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_getTrustedDevicesForActiveUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_removeTrustedDevice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_getActiveUserConnectedDevices = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_registerAssociatedDeviceCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_unregisterAssociatedDeviceCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_initiateEnrollment = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    public static boolean setDefaultImpl(com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceManager impl) {
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
    public static com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /** Indicate the escrow token has been added for a user and corresponding handle. */
  public void onEscrowTokenAdded(int userId, long handle) throws android.os.RemoteException;
  /** Indicate the escrow token has been activated for a user and corresponding handle. */
  public void onEscrowTokenActivated(int userId, long handle) throws android.os.RemoteException;
  /** Register a new callback for trusted device events. */
  public void registerTrustedDeviceCallback(com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceCallback callback) throws android.os.RemoteException;
  /** Remove a previously registered callback. */
  public void unregisterTrustedDeviceCallback(com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceCallback callback) throws android.os.RemoteException;
  /** Register a new callback for enrollment triggered events. */
  public void registerTrustedDeviceEnrollmentCallback(com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceEnrollmentCallback callback) throws android.os.RemoteException;
  /** Remove a previously registered callback. */
  public void unregisterTrustedDeviceEnrollmentCallback(com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceEnrollmentCallback callback) throws android.os.RemoteException;
  /** Set a delegate for TrustAgent operation calls. */
  public void setTrustedDeviceAgentDelegate(com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceAgentDelegate trustAgentDelegate) throws android.os.RemoteException;
  /** Remove a prevoiusly set delegate. */
  public void clearTrustedDeviceAgentDelegate(com.android.car.companiondevicesupport.api.internal.trust.ITrustedDeviceAgentDelegate trustAgentDelegate) throws android.os.RemoteException;
  /** Returns a list of trusted devices for user. */
  public java.util.List<com.android.car.companiondevicesupport.api.internal.trust.TrustedDevice> getTrustedDevicesForActiveUser() throws android.os.RemoteException;
  /** Remove a trusted device and invalidate any credentials associated with it. */
  public void removeTrustedDevice(com.android.car.companiondevicesupport.api.internal.trust.TrustedDevice trustedDevice) throws android.os.RemoteException;
  /** Returns {@link List<CompanionDevice>} of devices currently connected. */
  public java.util.List<com.android.car.companiondevicesupport.api.external.CompanionDevice> getActiveUserConnectedDevices() throws android.os.RemoteException;
  /** Register a new callback for associated device events. */
  public void registerAssociatedDeviceCallback(com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback callback) throws android.os.RemoteException;
  /** Remove a previously registered callback. */
  public void unregisterAssociatedDeviceCallback(com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback callback) throws android.os.RemoteException;
  /** Attempts to initiate trusted device enrollment on the phone with the given device id. */
  public void initiateEnrollment(java.lang.String deviceId) throws android.os.RemoteException;
}
