/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.telephony;
public interface IOns extends android.os.IInterface
{
  /** Default implementation for IOns. */
  public static class Default implements com.android.internal.telephony.IOns
  {
    /**
        * Enable or disable Opportunistic Network service.
        *
        * This method should be called to enable or disable
        * OpportunisticNetwork service on the device.
        *
        * <p>
        * Requires Permission:
        *   {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}
        * Or the calling app has carrier privileges. @see #hasCarrierPrivileges
        *
        * @param enable enable(True) or disable(False)
        * @param callingPackage caller's package name
        * @return returns true if successfully set.
        */
    @Override public boolean setEnable(boolean enable, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * is Opportunistic Network service enabled
         *
         * This method should be called to determine if the Opportunistic Network service is enabled
        *
        * <p>
        * Requires Permission:
        *   {@link android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE}
        * Or the calling app has carrier privileges. @see #hasCarrierPrivileges
        *
        * @param callingPackage caller's package name
        */
    @Override public boolean isEnabled(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Set preferred opportunistic data subscription id.
         *
         * <p>Requires that the calling app has carrier privileges on both primary and
         * secondary subscriptions (see
         * {@link #hasCarrierPrivileges}), or has permission
         * {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}.
         *
         * @param subId which opportunistic subscription
         * {@link SubscriptionManager#getOpportunisticSubscriptions} is preferred for cellular data.
         * Pass {@link SubscriptionManager#DEFAULT_SUBSCRIPTION_ID} to unset the preference
         * @param needValidation whether validation is needed before switch happens.
         * @param callback callback upon request completion.
         * @param callingPackage caller's package name
         *
         */
    @Override public void setPreferredDataSubscriptionId(int subId, boolean needValidation, com.android.internal.telephony.ISetOpportunisticDataCallback callbackStub, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    /**
         * Get preferred opportunistic data subscription Id
         *
         * <p>Requires that the calling app has carrier privileges (see {@link #hasCarrierPrivileges}),
         * or has permission {@link android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE}.
         * @return subId preferred opportunistic subscription id or
         * {@link SubscriptionManager#DEFAULT_SUBSCRIPTION_ID} if there are no preferred
         * subscription id
         *
         */
    @Override public int getPreferredDataSubscriptionId(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Update availability of a list of networks in the current location.
         *
         * This api should be called if the caller is aware of the availability of a network
         * at the current location. This information will be used by OpportunisticNetwork service
         * to decide to attach to the network. If an empty list is passed,
         * it is assumed that no network is available.
         * Requires that the calling app has carrier privileges on both primary and
         * secondary subscriptions (see
         * {@link #hasCarrierPrivileges}), or has permission
         * {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}.
         *  @param availableNetworks is a list of available network information.
         *  @param callingPackage caller's package name
         *  @param callback callback upon request completion.
         *
         */
    @Override public void updateAvailableNetworks(java.util.List<android.telephony.AvailableNetworkInfo> availableNetworks, com.android.internal.telephony.IUpdateAvailableNetworksCallback callbackStub, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.telephony.IOns
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.telephony.IOns";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.telephony.IOns interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.telephony.IOns asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.telephony.IOns))) {
        return ((com.android.internal.telephony.IOns)iin);
      }
      return new com.android.internal.telephony.IOns.Stub.Proxy(obj);
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
        case TRANSACTION_setEnable:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.setEnable(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isEnabled:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.isEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setPreferredDataSubscriptionId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          com.android.internal.telephony.ISetOpportunisticDataCallback _arg2;
          _arg2 = com.android.internal.telephony.ISetOpportunisticDataCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.setPreferredDataSubscriptionId(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getPreferredDataSubscriptionId:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _result = this.getPreferredDataSubscriptionId(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_updateAvailableNetworks:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.telephony.AvailableNetworkInfo> _arg0;
          _arg0 = data.createTypedArrayList(android.telephony.AvailableNetworkInfo.CREATOR);
          com.android.internal.telephony.IUpdateAvailableNetworksCallback _arg1;
          _arg1 = com.android.internal.telephony.IUpdateAvailableNetworksCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.updateAvailableNetworks(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.telephony.IOns
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
          * Enable or disable Opportunistic Network service.
          *
          * This method should be called to enable or disable
          * OpportunisticNetwork service on the device.
          *
          * <p>
          * Requires Permission:
          *   {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}
          * Or the calling app has carrier privileges. @see #hasCarrierPrivileges
          *
          * @param enable enable(True) or disable(False)
          * @param callingPackage caller's package name
          * @return returns true if successfully set.
          */
      @Override public boolean setEnable(boolean enable, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enable)?(1):(0)));
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setEnable, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setEnable(enable, callingPackage);
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
           * is Opportunistic Network service enabled
           *
           * This method should be called to determine if the Opportunistic Network service is enabled
          *
          * <p>
          * Requires Permission:
          *   {@link android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE}
          * Or the calling app has carrier privileges. @see #hasCarrierPrivileges
          *
          * @param callingPackage caller's package name
          */
      @Override public boolean isEnabled(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isEnabled(callingPackage);
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
           * Set preferred opportunistic data subscription id.
           *
           * <p>Requires that the calling app has carrier privileges on both primary and
           * secondary subscriptions (see
           * {@link #hasCarrierPrivileges}), or has permission
           * {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}.
           *
           * @param subId which opportunistic subscription
           * {@link SubscriptionManager#getOpportunisticSubscriptions} is preferred for cellular data.
           * Pass {@link SubscriptionManager#DEFAULT_SUBSCRIPTION_ID} to unset the preference
           * @param needValidation whether validation is needed before switch happens.
           * @param callback callback upon request completion.
           * @param callingPackage caller's package name
           *
           */
      @Override public void setPreferredDataSubscriptionId(int subId, boolean needValidation, com.android.internal.telephony.ISetOpportunisticDataCallback callbackStub, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(((needValidation)?(1):(0)));
          _data.writeStrongBinder((((callbackStub!=null))?(callbackStub.asBinder()):(null)));
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPreferredDataSubscriptionId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPreferredDataSubscriptionId(subId, needValidation, callbackStub, callingPackage);
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
           * Get preferred opportunistic data subscription Id
           *
           * <p>Requires that the calling app has carrier privileges (see {@link #hasCarrierPrivileges}),
           * or has permission {@link android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE}.
           * @return subId preferred opportunistic subscription id or
           * {@link SubscriptionManager#DEFAULT_SUBSCRIPTION_ID} if there are no preferred
           * subscription id
           *
           */
      @Override public int getPreferredDataSubscriptionId(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPreferredDataSubscriptionId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPreferredDataSubscriptionId(callingPackage);
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
      /**
           * Update availability of a list of networks in the current location.
           *
           * This api should be called if the caller is aware of the availability of a network
           * at the current location. This information will be used by OpportunisticNetwork service
           * to decide to attach to the network. If an empty list is passed,
           * it is assumed that no network is available.
           * Requires that the calling app has carrier privileges on both primary and
           * secondary subscriptions (see
           * {@link #hasCarrierPrivileges}), or has permission
           * {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}.
           *  @param availableNetworks is a list of available network information.
           *  @param callingPackage caller's package name
           *  @param callback callback upon request completion.
           *
           */
      @Override public void updateAvailableNetworks(java.util.List<android.telephony.AvailableNetworkInfo> availableNetworks, com.android.internal.telephony.IUpdateAvailableNetworksCallback callbackStub, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(availableNetworks);
          _data.writeStrongBinder((((callbackStub!=null))?(callbackStub.asBinder()):(null)));
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateAvailableNetworks, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateAvailableNetworks(availableNetworks, callbackStub, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.internal.telephony.IOns sDefaultImpl;
    }
    static final int TRANSACTION_setEnable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_isEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_setPreferredDataSubscriptionId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getPreferredDataSubscriptionId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_updateAvailableNetworks = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    public static boolean setDefaultImpl(com.android.internal.telephony.IOns impl) {
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
    public static com.android.internal.telephony.IOns getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
      * Enable or disable Opportunistic Network service.
      *
      * This method should be called to enable or disable
      * OpportunisticNetwork service on the device.
      *
      * <p>
      * Requires Permission:
      *   {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}
      * Or the calling app has carrier privileges. @see #hasCarrierPrivileges
      *
      * @param enable enable(True) or disable(False)
      * @param callingPackage caller's package name
      * @return returns true if successfully set.
      */
  public boolean setEnable(boolean enable, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * is Opportunistic Network service enabled
       *
       * This method should be called to determine if the Opportunistic Network service is enabled
      *
      * <p>
      * Requires Permission:
      *   {@link android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE}
      * Or the calling app has carrier privileges. @see #hasCarrierPrivileges
      *
      * @param callingPackage caller's package name
      */
  public boolean isEnabled(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Set preferred opportunistic data subscription id.
       *
       * <p>Requires that the calling app has carrier privileges on both primary and
       * secondary subscriptions (see
       * {@link #hasCarrierPrivileges}), or has permission
       * {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}.
       *
       * @param subId which opportunistic subscription
       * {@link SubscriptionManager#getOpportunisticSubscriptions} is preferred for cellular data.
       * Pass {@link SubscriptionManager#DEFAULT_SUBSCRIPTION_ID} to unset the preference
       * @param needValidation whether validation is needed before switch happens.
       * @param callback callback upon request completion.
       * @param callingPackage caller's package name
       *
       */
  public void setPreferredDataSubscriptionId(int subId, boolean needValidation, com.android.internal.telephony.ISetOpportunisticDataCallback callbackStub, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Get preferred opportunistic data subscription Id
       *
       * <p>Requires that the calling app has carrier privileges (see {@link #hasCarrierPrivileges}),
       * or has permission {@link android.Manifest.permission#READ_PHONE_STATE READ_PHONE_STATE}.
       * @return subId preferred opportunistic subscription id or
       * {@link SubscriptionManager#DEFAULT_SUBSCRIPTION_ID} if there are no preferred
       * subscription id
       *
       */
  public int getPreferredDataSubscriptionId(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Update availability of a list of networks in the current location.
       *
       * This api should be called if the caller is aware of the availability of a network
       * at the current location. This information will be used by OpportunisticNetwork service
       * to decide to attach to the network. If an empty list is passed,
       * it is assumed that no network is available.
       * Requires that the calling app has carrier privileges on both primary and
       * secondary subscriptions (see
       * {@link #hasCarrierPrivileges}), or has permission
       * {@link android.Manifest.permission#MODIFY_PHONE_STATE MODIFY_PHONE_STATE}.
       *  @param availableNetworks is a list of available network information.
       *  @param callingPackage caller's package name
       *  @param callback callback upon request completion.
       *
       */
  public void updateAvailableNetworks(java.util.List<android.telephony.AvailableNetworkInfo> availableNetworks, com.android.internal.telephony.IUpdateAvailableNetworksCallback callbackStub, java.lang.String callingPackage) throws android.os.RemoteException;
}
