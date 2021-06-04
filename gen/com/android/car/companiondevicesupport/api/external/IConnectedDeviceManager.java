/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.car.companiondevicesupport.api.external;
/** Manager of devices connected to the car. */
public interface IConnectedDeviceManager extends android.os.IInterface
{
  /** Default implementation for IConnectedDeviceManager. */
  public static class Default implements com.android.car.companiondevicesupport.api.external.IConnectedDeviceManager
  {
    /** Returns {@link List<CompanionDevice>} of devices currently connected. */
    @Override public java.util.List<com.android.car.companiondevicesupport.api.external.CompanionDevice> getActiveUserConnectedDevices() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Register a callback for manager triggered connection events for only the currently active
         * user's devices.
         *
         * @param callback {@link IConnectionCallback} to register.
         */
    @Override public void registerActiveUserConnectionCallback(com.android.car.companiondevicesupport.api.external.IConnectionCallback callback) throws android.os.RemoteException
    {
    }
    /**
         * Unregister a connection callback from manager.
         *
         * @param callback {@link IConnectionCallback} to unregister.
         */
    @Override public void unregisterConnectionCallback(com.android.car.companiondevicesupport.api.external.IConnectionCallback callback) throws android.os.RemoteException
    {
    }
    /**
         * Register a callback for a specific companionDevice and recipient.
         *
         * @param companionDevice {@link CompanionDevice} to register triggers on.
         * @param recipientId {@link ParcelUuid} to register as recipient of.
         * @param callback {@link IDeviceCallback} to register.
         */
    @Override public void registerDeviceCallback(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice, android.os.ParcelUuid recipientId, com.android.car.companiondevicesupport.api.external.IDeviceCallback callback) throws android.os.RemoteException
    {
    }
    /**
         * Unregister callback from companionDevice events.
         *
         * @param companionDevice {@link CompanionDevice} callback was registered on.
         * @param recipientId {@link ParcelUuid} callback was registered under.
         * @param callback {@link IDeviceCallback} to unregister.
         */
    @Override public void unregisterDeviceCallback(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice, android.os.ParcelUuid recipientId, com.android.car.companiondevicesupport.api.external.IDeviceCallback callback) throws android.os.RemoteException
    {
    }
    /**
         * Securely send message to a companionDevice.
         *
         * @param companionDevice {@link CompanionDevice} to send the message to.
         * @param recipientId Recipient {@link ParcelUuid}.
         * @param message Message to send.
         * @return `true` if message was able to initiate, `false` if secure channel was not available.
         */
    @Override public boolean sendMessageSecurely(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice, android.os.ParcelUuid recipientId, byte[] message) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Send an unencrypted message to a companionDevice.
         *
         * @param companionDevice {@link CompanionDevice} to send the message to.
         * @param recipientId Recipient {@link ParcelUuid}.
         * @param message Message to send.
         */
    @Override public void sendMessageUnsecurely(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice, android.os.ParcelUuid recipientId, byte[] message) throws android.os.RemoteException
    {
    }
    /**
         * Register a callback for associated devic erelated events.
         *
         * @param callback {@link IDeviceAssociationCallback} to register.
         */
    @Override public void registerDeviceAssociationCallback(com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback callback) throws android.os.RemoteException
    {
    }
    /**
         * Unregister a device association callback from manager.
         *
         * @param callback {@link IDeviceAssociationCallback} to unregister.
         */
    @Override public void unregisterDeviceAssociationCallback(com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.car.companiondevicesupport.api.external.IConnectedDeviceManager
  {
    private static final java.lang.String DESCRIPTOR = "com.android.car.companiondevicesupport.api.external.IConnectedDeviceManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.car.companiondevicesupport.api.external.IConnectedDeviceManager interface,
     * generating a proxy if needed.
     */
    public static com.android.car.companiondevicesupport.api.external.IConnectedDeviceManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.car.companiondevicesupport.api.external.IConnectedDeviceManager))) {
        return ((com.android.car.companiondevicesupport.api.external.IConnectedDeviceManager)iin);
      }
      return new com.android.car.companiondevicesupport.api.external.IConnectedDeviceManager.Stub.Proxy(obj);
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
        case TRANSACTION_getActiveUserConnectedDevices:
        {
          data.enforceInterface(descriptor);
          java.util.List<com.android.car.companiondevicesupport.api.external.CompanionDevice> _result = this.getActiveUserConnectedDevices();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_registerActiveUserConnectionCallback:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.external.IConnectionCallback _arg0;
          _arg0 = com.android.car.companiondevicesupport.api.external.IConnectionCallback.Stub.asInterface(data.readStrongBinder());
          this.registerActiveUserConnectionCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterConnectionCallback:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.external.IConnectionCallback _arg0;
          _arg0 = com.android.car.companiondevicesupport.api.external.IConnectionCallback.Stub.asInterface(data.readStrongBinder());
          this.unregisterConnectionCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerDeviceCallback:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.external.CompanionDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.car.companiondevicesupport.api.external.CompanionDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.ParcelUuid _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.ParcelUuid.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          com.android.car.companiondevicesupport.api.external.IDeviceCallback _arg2;
          _arg2 = com.android.car.companiondevicesupport.api.external.IDeviceCallback.Stub.asInterface(data.readStrongBinder());
          this.registerDeviceCallback(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterDeviceCallback:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.external.CompanionDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.car.companiondevicesupport.api.external.CompanionDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.ParcelUuid _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.ParcelUuid.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          com.android.car.companiondevicesupport.api.external.IDeviceCallback _arg2;
          _arg2 = com.android.car.companiondevicesupport.api.external.IDeviceCallback.Stub.asInterface(data.readStrongBinder());
          this.unregisterDeviceCallback(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_sendMessageSecurely:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.external.CompanionDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.car.companiondevicesupport.api.external.CompanionDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.ParcelUuid _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.ParcelUuid.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          byte[] _arg2;
          _arg2 = data.createByteArray();
          boolean _result = this.sendMessageSecurely(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_sendMessageUnsecurely:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.external.CompanionDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.car.companiondevicesupport.api.external.CompanionDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.ParcelUuid _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.ParcelUuid.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          byte[] _arg2;
          _arg2 = data.createByteArray();
          this.sendMessageUnsecurely(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerDeviceAssociationCallback:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback _arg0;
          _arg0 = com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback.Stub.asInterface(data.readStrongBinder());
          this.registerDeviceAssociationCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterDeviceAssociationCallback:
        {
          data.enforceInterface(descriptor);
          com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback _arg0;
          _arg0 = com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback.Stub.asInterface(data.readStrongBinder());
          this.unregisterDeviceAssociationCallback(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.car.companiondevicesupport.api.external.IConnectedDeviceManager
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
           * Register a callback for manager triggered connection events for only the currently active
           * user's devices.
           *
           * @param callback {@link IConnectionCallback} to register.
           */
      @Override public void registerActiveUserConnectionCallback(com.android.car.companiondevicesupport.api.external.IConnectionCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerActiveUserConnectionCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerActiveUserConnectionCallback(callback);
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
           * Unregister a connection callback from manager.
           *
           * @param callback {@link IConnectionCallback} to unregister.
           */
      @Override public void unregisterConnectionCallback(com.android.car.companiondevicesupport.api.external.IConnectionCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterConnectionCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterConnectionCallback(callback);
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
           * Register a callback for a specific companionDevice and recipient.
           *
           * @param companionDevice {@link CompanionDevice} to register triggers on.
           * @param recipientId {@link ParcelUuid} to register as recipient of.
           * @param callback {@link IDeviceCallback} to register.
           */
      @Override public void registerDeviceCallback(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice, android.os.ParcelUuid recipientId, com.android.car.companiondevicesupport.api.external.IDeviceCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((companionDevice!=null)) {
            _data.writeInt(1);
            companionDevice.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((recipientId!=null)) {
            _data.writeInt(1);
            recipientId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerDeviceCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerDeviceCallback(companionDevice, recipientId, callback);
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
           * Unregister callback from companionDevice events.
           *
           * @param companionDevice {@link CompanionDevice} callback was registered on.
           * @param recipientId {@link ParcelUuid} callback was registered under.
           * @param callback {@link IDeviceCallback} to unregister.
           */
      @Override public void unregisterDeviceCallback(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice, android.os.ParcelUuid recipientId, com.android.car.companiondevicesupport.api.external.IDeviceCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((companionDevice!=null)) {
            _data.writeInt(1);
            companionDevice.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((recipientId!=null)) {
            _data.writeInt(1);
            recipientId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterDeviceCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterDeviceCallback(companionDevice, recipientId, callback);
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
           * Securely send message to a companionDevice.
           *
           * @param companionDevice {@link CompanionDevice} to send the message to.
           * @param recipientId Recipient {@link ParcelUuid}.
           * @param message Message to send.
           * @return `true` if message was able to initiate, `false` if secure channel was not available.
           */
      @Override public boolean sendMessageSecurely(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice, android.os.ParcelUuid recipientId, byte[] message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((companionDevice!=null)) {
            _data.writeInt(1);
            companionDevice.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((recipientId!=null)) {
            _data.writeInt(1);
            recipientId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeByteArray(message);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendMessageSecurely, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().sendMessageSecurely(companionDevice, recipientId, message);
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
      /**
           * Send an unencrypted message to a companionDevice.
           *
           * @param companionDevice {@link CompanionDevice} to send the message to.
           * @param recipientId Recipient {@link ParcelUuid}.
           * @param message Message to send.
           */
      @Override public void sendMessageUnsecurely(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice, android.os.ParcelUuid recipientId, byte[] message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((companionDevice!=null)) {
            _data.writeInt(1);
            companionDevice.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((recipientId!=null)) {
            _data.writeInt(1);
            recipientId.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeByteArray(message);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendMessageUnsecurely, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendMessageUnsecurely(companionDevice, recipientId, message);
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
           * Register a callback for associated devic erelated events.
           *
           * @param callback {@link IDeviceAssociationCallback} to register.
           */
      @Override public void registerDeviceAssociationCallback(com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerDeviceAssociationCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerDeviceAssociationCallback(callback);
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
           * Unregister a device association callback from manager.
           *
           * @param callback {@link IDeviceAssociationCallback} to unregister.
           */
      @Override public void unregisterDeviceAssociationCallback(com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterDeviceAssociationCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterDeviceAssociationCallback(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.car.companiondevicesupport.api.external.IConnectedDeviceManager sDefaultImpl;
    }
    static final int TRANSACTION_getActiveUserConnectedDevices = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_registerActiveUserConnectionCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_unregisterConnectionCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_registerDeviceCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_unregisterDeviceCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_sendMessageSecurely = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_sendMessageUnsecurely = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_registerDeviceAssociationCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_unregisterDeviceAssociationCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    public static boolean setDefaultImpl(com.android.car.companiondevicesupport.api.external.IConnectedDeviceManager impl) {
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
    public static com.android.car.companiondevicesupport.api.external.IConnectedDeviceManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /** Returns {@link List<CompanionDevice>} of devices currently connected. */
  public java.util.List<com.android.car.companiondevicesupport.api.external.CompanionDevice> getActiveUserConnectedDevices() throws android.os.RemoteException;
  /**
       * Register a callback for manager triggered connection events for only the currently active
       * user's devices.
       *
       * @param callback {@link IConnectionCallback} to register.
       */
  public void registerActiveUserConnectionCallback(com.android.car.companiondevicesupport.api.external.IConnectionCallback callback) throws android.os.RemoteException;
  /**
       * Unregister a connection callback from manager.
       *
       * @param callback {@link IConnectionCallback} to unregister.
       */
  public void unregisterConnectionCallback(com.android.car.companiondevicesupport.api.external.IConnectionCallback callback) throws android.os.RemoteException;
  /**
       * Register a callback for a specific companionDevice and recipient.
       *
       * @param companionDevice {@link CompanionDevice} to register triggers on.
       * @param recipientId {@link ParcelUuid} to register as recipient of.
       * @param callback {@link IDeviceCallback} to register.
       */
  public void registerDeviceCallback(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice, android.os.ParcelUuid recipientId, com.android.car.companiondevicesupport.api.external.IDeviceCallback callback) throws android.os.RemoteException;
  /**
       * Unregister callback from companionDevice events.
       *
       * @param companionDevice {@link CompanionDevice} callback was registered on.
       * @param recipientId {@link ParcelUuid} callback was registered under.
       * @param callback {@link IDeviceCallback} to unregister.
       */
  public void unregisterDeviceCallback(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice, android.os.ParcelUuid recipientId, com.android.car.companiondevicesupport.api.external.IDeviceCallback callback) throws android.os.RemoteException;
  /**
       * Securely send message to a companionDevice.
       *
       * @param companionDevice {@link CompanionDevice} to send the message to.
       * @param recipientId Recipient {@link ParcelUuid}.
       * @param message Message to send.
       * @return `true` if message was able to initiate, `false` if secure channel was not available.
       */
  public boolean sendMessageSecurely(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice, android.os.ParcelUuid recipientId, byte[] message) throws android.os.RemoteException;
  /**
       * Send an unencrypted message to a companionDevice.
       *
       * @param companionDevice {@link CompanionDevice} to send the message to.
       * @param recipientId Recipient {@link ParcelUuid}.
       * @param message Message to send.
       */
  public void sendMessageUnsecurely(com.android.car.companiondevicesupport.api.external.CompanionDevice companionDevice, android.os.ParcelUuid recipientId, byte[] message) throws android.os.RemoteException;
  /**
       * Register a callback for associated devic erelated events.
       *
       * @param callback {@link IDeviceAssociationCallback} to register.
       */
  public void registerDeviceAssociationCallback(com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback callback) throws android.os.RemoteException;
  /**
       * Unregister a device association callback from manager.
       *
       * @param callback {@link IDeviceAssociationCallback} to unregister.
       */
  public void unregisterDeviceAssociationCallback(com.android.car.companiondevicesupport.api.external.IDeviceAssociationCallback callback) throws android.os.RemoteException;
}
