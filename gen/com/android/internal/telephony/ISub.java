/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.telephony;
public interface ISub extends android.os.IInterface
{
  /** Default implementation for ISub. */
  public static class Default implements com.android.internal.telephony.ISub
  {
    /**
         * @param callingPackage The package maing the call.
         * @return a list of all subscriptions in the database, this includes
         * all subscriptions that have been seen.
         */
    @Override public java.util.List<android.telephony.SubscriptionInfo> getAllSubInfoList(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @param callingPackage The package maing the call.
         * @return the count of all subscriptions in the database, this includes
         * all subscriptions that have been seen.
         */
    @Override public int getAllSubInfoCount(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Get the active SubscriptionInfo with the subId key
         * @param subId The unique SubscriptionInfo key in database
         * @param callingPackage The package maing the call.
         * @return SubscriptionInfo, maybe null if its not active
         */
    @Override public android.telephony.SubscriptionInfo getActiveSubscriptionInfo(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Get the active SubscriptionInfo associated with the iccId
         * @param iccId the IccId of SIM card
         * @param callingPackage The package maing the call.
         * @return SubscriptionInfo, maybe null if its not active
         */
    @Override public android.telephony.SubscriptionInfo getActiveSubscriptionInfoForIccId(java.lang.String iccId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Get the active SubscriptionInfo associated with the slotIndex
         * @param slotIndex the slot which the subscription is inserted
         * @param callingPackage The package making the call.
         * @return SubscriptionInfo, null for Remote-SIMs or non-active slotIndex.
         */
    @Override public android.telephony.SubscriptionInfo getActiveSubscriptionInfoForSimSlotIndex(int slotIndex, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Get the SubscriptionInfo(s) of the active subscriptions. The records will be sorted
         * by {@link SubscriptionInfo#getSimSlotIndex} then by {@link SubscriptionInfo#getSubscriptionId}.
         *
         * @param callingPackage The package maing the call.
         * @return Sorted list of the currently {@link SubscriptionInfo} records available on the device.
         * <ul>
         * <li>
         * If null is returned the current state is unknown but if a {@link OnSubscriptionsChangedListener}
         * has been registered {@link OnSubscriptionsChangedListener#onSubscriptionsChanged} will be
         * invoked in the future.
         * </li>
         * <li>
         * If the list is empty then there are no {@link SubscriptionInfo} records currently available.
         * </li>
         * <li>
         * if the list is non-empty the list is sorted by {@link SubscriptionInfo#getSimSlotIndex}
         * then by {@link SubscriptionInfo#getSubscriptionId}.
         * </li>
         * </ul>
         */
    @Override public java.util.List<android.telephony.SubscriptionInfo> getActiveSubscriptionInfoList(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @param callingPackage The package making the call.
         * @return the number of active subscriptions
         */
    @Override public int getActiveSubInfoCount(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * @return the maximum number of subscriptions this device will support at any one time.
         */
    @Override public int getActiveSubInfoCountMax() throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * @see android.telephony.SubscriptionManager#getAvailableSubscriptionInfoList
         */
    @Override public java.util.List<android.telephony.SubscriptionInfo> getAvailableSubscriptionInfoList(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @see android.telephony.SubscriptionManager#getAccessibleSubscriptionInfoList
         */
    @Override public java.util.List<android.telephony.SubscriptionInfo> getAccessibleSubscriptionInfoList(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @see android.telephony.SubscriptionManager#requestEmbeddedSubscriptionInfoListRefresh
         */
    @Override public void requestEmbeddedSubscriptionInfoListRefresh(int cardId) throws android.os.RemoteException
    {
    }
    /**
         * Add a new SubscriptionInfo to subinfo database if needed
         * @param iccId the IccId of the SIM card
         * @param slotIndex the slot which the SIM is inserted
         * @return the URL of the newly created row or the updated row
         */
    @Override public int addSubInfoRecord(java.lang.String iccId, int slotIndex) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Add a new subscription info record, if needed
         * @param uniqueId This is the unique identifier for the subscription within the specific
         *                 subscription type.
         * @param displayName human-readable name of the device the subscription corresponds to.
         * @param slotIndex the slot assigned to this device
         * @param subscriptionType the type of subscription to be added.
         * @return 0 if success, < 0 on error.
         */
    @Override public int addSubInfo(java.lang.String uniqueId, java.lang.String displayName, int slotIndex, int subscriptionType) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Remove subscription info record for the given device.
         * @param uniqueId This is the unique identifier for the subscription within the specific
         *                      subscription type.
         * @param subscriptionType the type of subscription to be removed
         * @return 0 if success, < 0 on error.
         */
    @Override public int removeSubInfo(java.lang.String uniqueId, int subscriptionType) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Set SIM icon tint color by simInfo index
         * @param tint the icon tint color of the SIM
         * @param subId the unique SubscriptionInfo index in database
         * @return the number of records updated
         */
    @Override public int setIconTint(int tint, int subId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Set display name by simInfo index with name source
         * @param displayName the display name of SIM card
         * @param subId the unique SubscriptionInfo index in database
         * @param nameSource, 0: DEFAULT_SOURCE, 1: SIM_SOURCE, 2: USER_INPUT
         * @return the number of records updated
         */
    @Override public int setDisplayNameUsingSrc(java.lang.String displayName, int subId, int nameSource) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Set phone number by subId
         * @param number the phone number of the SIM
         * @param subId the unique SubscriptionInfo index in database
         * @return the number of records updated
         */
    @Override public int setDisplayNumber(java.lang.String number, int subId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Set data roaming by simInfo index
         * @param roaming 0:Don't allow data when roaming, 1:Allow data when roaming
         * @param subId the unique SubscriptionInfo index in database
         * @return the number of records updated
         */
    @Override public int setDataRoaming(int roaming, int subId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Switch to a certain subscription
         *
         * @param opportunistic whether it’s opportunistic subscription.
         * @param subId the unique SubscriptionInfo index in database
         * @return the number of records updated
         */
    @Override public int setOpportunistic(boolean opportunistic, int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Inform SubscriptionManager that subscriptions in the list are bundled
         * as a group. Typically it's a primary subscription and an opportunistic
         * subscription. It should only affect multi-SIM scenarios where primary
         * and opportunistic subscriptions can be activated together.
         * Being in the same group means they might be activated or deactivated
         * together, some of them may be invisible to the users, etc.
         *
         * Caller will either have {@link android.Manifest.permission.MODIFY_PHONE_STATE}
         * permission or can manage all subscriptions in the list, according to their
         * acess rules.
         *
         * @param subIdList list of subId that will be in the same group
         * @return groupUUID a UUID assigned to the subscription group. It returns
         * null if fails.
         *
         */
    @Override public android.os.ParcelUuid createSubscriptionGroup(int[] subIdList, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Set which subscription is preferred for cellular data. It's
         * designed to overwrite default data subscription temporarily.
         *
         * @param subId which subscription is preferred to for cellular data.
         * @param needValidation whether validation is needed before switching.
         * @param callback callback upon request completion.
         *
         * @hide
         *
         */
    @Override public void setPreferredDataSubscriptionId(int subId, boolean needValidation, com.android.internal.telephony.ISetOpportunisticDataCallback callback) throws android.os.RemoteException
    {
    }
    /**
         * Get which subscription is preferred for cellular data.
         *
         * @hide
         *
         */
    @Override public int getPreferredDataSubscriptionId() throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Get User downloaded Profiles.
         *
         * Return opportunistic subscriptions that can be visible to the caller.
         * @return the list of opportunistic subscription info. If none exists, an empty list.
         */
    @Override public java.util.List<android.telephony.SubscriptionInfo> getOpportunisticSubscriptions(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void removeSubscriptionsFromGroup(int[] subIdList, android.os.ParcelUuid groupUuid, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public void addSubscriptionsIntoGroup(int[] subIdList, android.os.ParcelUuid groupUuid, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public java.util.List<android.telephony.SubscriptionInfo> getSubscriptionsInGroup(android.os.ParcelUuid groupUuid, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int getSlotIndex(int subId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int[] getSubId(int slotIndex) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int getDefaultSubId() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int clearSubInfo() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int getPhoneId(int subId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Get the default data subscription
         * @return Id of the data subscription
         */
    @Override public int getDefaultDataSubId() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setDefaultDataSubId(int subId) throws android.os.RemoteException
    {
    }
    @Override public int getDefaultVoiceSubId() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setDefaultVoiceSubId(int subId) throws android.os.RemoteException
    {
    }
    @Override public int getDefaultSmsSubId() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setDefaultSmsSubId(int subId) throws android.os.RemoteException
    {
    }
    @Override public int[] getActiveSubIdList(boolean visibleOnly) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int setSubscriptionProperty(int subId, java.lang.String propKey, java.lang.String propValue) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public java.lang.String getSubscriptionProperty(int subId, java.lang.String propKey, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public boolean setSubscriptionEnabled(boolean enable, int subId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isSubscriptionEnabled(int subId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public int getEnabledSubscriptionId(int slotIndex) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Get the SIM state for the slot index
         * @return SIM state as the ordinal of IccCardConstants.State
         */
    @Override public int getSimStateForSlotIndex(int slotIndex) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public boolean isActiveSubId(int subId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean setAlwaysAllowMmsData(int subId, boolean alwaysAllow) throws android.os.RemoteException
    {
      return false;
    }
    @Override public int getActiveDataSubscriptionId() throws android.os.RemoteException
    {
      return 0;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.telephony.ISub
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.telephony.ISub";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.telephony.ISub interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.telephony.ISub asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.telephony.ISub))) {
        return ((com.android.internal.telephony.ISub)iin);
      }
      return new com.android.internal.telephony.ISub.Stub.Proxy(obj);
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
        case TRANSACTION_getAllSubInfoList:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.List<android.telephony.SubscriptionInfo> _result = this.getAllSubInfoList(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getAllSubInfoCount:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _result = this.getAllSubInfoCount(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getActiveSubscriptionInfo:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.telephony.SubscriptionInfo _result = this.getActiveSubscriptionInfo(_arg0, _arg1);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_getActiveSubscriptionInfoForIccId:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.telephony.SubscriptionInfo _result = this.getActiveSubscriptionInfoForIccId(_arg0, _arg1);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_getActiveSubscriptionInfoForSimSlotIndex:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.telephony.SubscriptionInfo _result = this.getActiveSubscriptionInfoForSimSlotIndex(_arg0, _arg1);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_getActiveSubscriptionInfoList:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.List<android.telephony.SubscriptionInfo> _result = this.getActiveSubscriptionInfoList(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getActiveSubInfoCount:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _result = this.getActiveSubInfoCount(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getActiveSubInfoCountMax:
        {
          data.enforceInterface(descriptor);
          int _result = this.getActiveSubInfoCountMax();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getAvailableSubscriptionInfoList:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.List<android.telephony.SubscriptionInfo> _result = this.getAvailableSubscriptionInfoList(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getAccessibleSubscriptionInfoList:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.List<android.telephony.SubscriptionInfo> _result = this.getAccessibleSubscriptionInfoList(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_requestEmbeddedSubscriptionInfoListRefresh:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.requestEmbeddedSubscriptionInfoListRefresh(_arg0);
          return true;
        }
        case TRANSACTION_addSubInfoRecord:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.addSubInfoRecord(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_addSubInfo:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          int _result = this.addSubInfo(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_removeSubInfo:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.removeSubInfo(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setIconTint:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.setIconTint(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setDisplayNameUsingSrc:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _result = this.setDisplayNameUsingSrc(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setDisplayNumber:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.setDisplayNumber(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setDataRoaming:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.setDataRoaming(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setOpportunistic:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _result = this.setOpportunistic(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_createSubscriptionGroup:
        {
          data.enforceInterface(descriptor);
          int[] _arg0;
          _arg0 = data.createIntArray();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.ParcelUuid _result = this.createSubscriptionGroup(_arg0, _arg1);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
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
          this.setPreferredDataSubscriptionId(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getPreferredDataSubscriptionId:
        {
          data.enforceInterface(descriptor);
          int _result = this.getPreferredDataSubscriptionId();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getOpportunisticSubscriptions:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.List<android.telephony.SubscriptionInfo> _result = this.getOpportunisticSubscriptions(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_removeSubscriptionsFromGroup:
        {
          data.enforceInterface(descriptor);
          int[] _arg0;
          _arg0 = data.createIntArray();
          android.os.ParcelUuid _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.ParcelUuid.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.removeSubscriptionsFromGroup(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addSubscriptionsIntoGroup:
        {
          data.enforceInterface(descriptor);
          int[] _arg0;
          _arg0 = data.createIntArray();
          android.os.ParcelUuid _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.ParcelUuid.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.addSubscriptionsIntoGroup(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getSubscriptionsInGroup:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelUuid _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelUuid.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.util.List<android.telephony.SubscriptionInfo> _result = this.getSubscriptionsInGroup(_arg0, _arg1);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getSlotIndex:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getSlotIndex(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getSubId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int[] _result = this.getSubId(_arg0);
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_getDefaultSubId:
        {
          data.enforceInterface(descriptor);
          int _result = this.getDefaultSubId();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_clearSubInfo:
        {
          data.enforceInterface(descriptor);
          int _result = this.clearSubInfo();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getPhoneId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getPhoneId(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getDefaultDataSubId:
        {
          data.enforceInterface(descriptor);
          int _result = this.getDefaultDataSubId();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setDefaultDataSubId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setDefaultDataSubId(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getDefaultVoiceSubId:
        {
          data.enforceInterface(descriptor);
          int _result = this.getDefaultVoiceSubId();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setDefaultVoiceSubId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setDefaultVoiceSubId(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getDefaultSmsSubId:
        {
          data.enforceInterface(descriptor);
          int _result = this.getDefaultSmsSubId();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setDefaultSmsSubId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setDefaultSmsSubId(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getActiveSubIdList:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int[] _result = this.getActiveSubIdList(_arg0);
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_setSubscriptionProperty:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _result = this.setSubscriptionProperty(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getSubscriptionProperty:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String _result = this.getSubscriptionProperty(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_setSubscriptionEnabled:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.setSubscriptionEnabled(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isSubscriptionEnabled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.isSubscriptionEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getEnabledSubscriptionId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getEnabledSubscriptionId(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getSimStateForSlotIndex:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getSimStateForSlotIndex(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_isActiveSubId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.isActiveSubId(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setAlwaysAllowMmsData:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          boolean _result = this.setAlwaysAllowMmsData(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getActiveDataSubscriptionId:
        {
          data.enforceInterface(descriptor);
          int _result = this.getActiveDataSubscriptionId();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.telephony.ISub
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
           * @param callingPackage The package maing the call.
           * @return a list of all subscriptions in the database, this includes
           * all subscriptions that have been seen.
           */
      @Override public java.util.List<android.telephony.SubscriptionInfo> getAllSubInfoList(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.telephony.SubscriptionInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAllSubInfoList, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAllSubInfoList(callingPackage);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.telephony.SubscriptionInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * @param callingPackage The package maing the call.
           * @return the count of all subscriptions in the database, this includes
           * all subscriptions that have been seen.
           */
      @Override public int getAllSubInfoCount(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAllSubInfoCount, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAllSubInfoCount(callingPackage);
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
           * Get the active SubscriptionInfo with the subId key
           * @param subId The unique SubscriptionInfo key in database
           * @param callingPackage The package maing the call.
           * @return SubscriptionInfo, maybe null if its not active
           */
      @Override public android.telephony.SubscriptionInfo getActiveSubscriptionInfo(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.SubscriptionInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveSubscriptionInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActiveSubscriptionInfo(subId, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telephony.SubscriptionInfo.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Get the active SubscriptionInfo associated with the iccId
           * @param iccId the IccId of SIM card
           * @param callingPackage The package maing the call.
           * @return SubscriptionInfo, maybe null if its not active
           */
      @Override public android.telephony.SubscriptionInfo getActiveSubscriptionInfoForIccId(java.lang.String iccId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.SubscriptionInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iccId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveSubscriptionInfoForIccId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActiveSubscriptionInfoForIccId(iccId, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telephony.SubscriptionInfo.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Get the active SubscriptionInfo associated with the slotIndex
           * @param slotIndex the slot which the subscription is inserted
           * @param callingPackage The package making the call.
           * @return SubscriptionInfo, null for Remote-SIMs or non-active slotIndex.
           */
      @Override public android.telephony.SubscriptionInfo getActiveSubscriptionInfoForSimSlotIndex(int slotIndex, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.SubscriptionInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotIndex);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveSubscriptionInfoForSimSlotIndex, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActiveSubscriptionInfoForSimSlotIndex(slotIndex, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telephony.SubscriptionInfo.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Get the SubscriptionInfo(s) of the active subscriptions. The records will be sorted
           * by {@link SubscriptionInfo#getSimSlotIndex} then by {@link SubscriptionInfo#getSubscriptionId}.
           *
           * @param callingPackage The package maing the call.
           * @return Sorted list of the currently {@link SubscriptionInfo} records available on the device.
           * <ul>
           * <li>
           * If null is returned the current state is unknown but if a {@link OnSubscriptionsChangedListener}
           * has been registered {@link OnSubscriptionsChangedListener#onSubscriptionsChanged} will be
           * invoked in the future.
           * </li>
           * <li>
           * If the list is empty then there are no {@link SubscriptionInfo} records currently available.
           * </li>
           * <li>
           * if the list is non-empty the list is sorted by {@link SubscriptionInfo#getSimSlotIndex}
           * then by {@link SubscriptionInfo#getSubscriptionId}.
           * </li>
           * </ul>
           */
      @Override public java.util.List<android.telephony.SubscriptionInfo> getActiveSubscriptionInfoList(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.telephony.SubscriptionInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveSubscriptionInfoList, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActiveSubscriptionInfoList(callingPackage);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.telephony.SubscriptionInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * @param callingPackage The package making the call.
           * @return the number of active subscriptions
           */
      @Override public int getActiveSubInfoCount(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveSubInfoCount, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActiveSubInfoCount(callingPackage);
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
           * @return the maximum number of subscriptions this device will support at any one time.
           */
      @Override public int getActiveSubInfoCountMax() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveSubInfoCountMax, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActiveSubInfoCountMax();
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
           * @see android.telephony.SubscriptionManager#getAvailableSubscriptionInfoList
           */
      @Override public java.util.List<android.telephony.SubscriptionInfo> getAvailableSubscriptionInfoList(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.telephony.SubscriptionInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAvailableSubscriptionInfoList, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAvailableSubscriptionInfoList(callingPackage);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.telephony.SubscriptionInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * @see android.telephony.SubscriptionManager#getAccessibleSubscriptionInfoList
           */
      @Override public java.util.List<android.telephony.SubscriptionInfo> getAccessibleSubscriptionInfoList(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.telephony.SubscriptionInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAccessibleSubscriptionInfoList, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAccessibleSubscriptionInfoList(callingPackage);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.telephony.SubscriptionInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * @see android.telephony.SubscriptionManager#requestEmbeddedSubscriptionInfoListRefresh
           */
      @Override public void requestEmbeddedSubscriptionInfoListRefresh(int cardId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(cardId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestEmbeddedSubscriptionInfoListRefresh, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestEmbeddedSubscriptionInfoListRefresh(cardId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Add a new SubscriptionInfo to subinfo database if needed
           * @param iccId the IccId of the SIM card
           * @param slotIndex the slot which the SIM is inserted
           * @return the URL of the newly created row or the updated row
           */
      @Override public int addSubInfoRecord(java.lang.String iccId, int slotIndex) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iccId);
          _data.writeInt(slotIndex);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addSubInfoRecord, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addSubInfoRecord(iccId, slotIndex);
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
           * Add a new subscription info record, if needed
           * @param uniqueId This is the unique identifier for the subscription within the specific
           *                 subscription type.
           * @param displayName human-readable name of the device the subscription corresponds to.
           * @param slotIndex the slot assigned to this device
           * @param subscriptionType the type of subscription to be added.
           * @return 0 if success, < 0 on error.
           */
      @Override public int addSubInfo(java.lang.String uniqueId, java.lang.String displayName, int slotIndex, int subscriptionType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(uniqueId);
          _data.writeString(displayName);
          _data.writeInt(slotIndex);
          _data.writeInt(subscriptionType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addSubInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addSubInfo(uniqueId, displayName, slotIndex, subscriptionType);
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
           * Remove subscription info record for the given device.
           * @param uniqueId This is the unique identifier for the subscription within the specific
           *                      subscription type.
           * @param subscriptionType the type of subscription to be removed
           * @return 0 if success, < 0 on error.
           */
      @Override public int removeSubInfo(java.lang.String uniqueId, int subscriptionType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(uniqueId);
          _data.writeInt(subscriptionType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeSubInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().removeSubInfo(uniqueId, subscriptionType);
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
           * Set SIM icon tint color by simInfo index
           * @param tint the icon tint color of the SIM
           * @param subId the unique SubscriptionInfo index in database
           * @return the number of records updated
           */
      @Override public int setIconTint(int tint, int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(tint);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setIconTint, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setIconTint(tint, subId);
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
           * Set display name by simInfo index with name source
           * @param displayName the display name of SIM card
           * @param subId the unique SubscriptionInfo index in database
           * @param nameSource, 0: DEFAULT_SOURCE, 1: SIM_SOURCE, 2: USER_INPUT
           * @return the number of records updated
           */
      @Override public int setDisplayNameUsingSrc(java.lang.String displayName, int subId, int nameSource) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(displayName);
          _data.writeInt(subId);
          _data.writeInt(nameSource);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDisplayNameUsingSrc, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setDisplayNameUsingSrc(displayName, subId, nameSource);
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
           * Set phone number by subId
           * @param number the phone number of the SIM
           * @param subId the unique SubscriptionInfo index in database
           * @return the number of records updated
           */
      @Override public int setDisplayNumber(java.lang.String number, int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(number);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDisplayNumber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setDisplayNumber(number, subId);
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
           * Set data roaming by simInfo index
           * @param roaming 0:Don't allow data when roaming, 1:Allow data when roaming
           * @param subId the unique SubscriptionInfo index in database
           * @return the number of records updated
           */
      @Override public int setDataRoaming(int roaming, int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(roaming);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDataRoaming, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setDataRoaming(roaming, subId);
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
           * Switch to a certain subscription
           *
           * @param opportunistic whether it’s opportunistic subscription.
           * @param subId the unique SubscriptionInfo index in database
           * @return the number of records updated
           */
      @Override public int setOpportunistic(boolean opportunistic, int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((opportunistic)?(1):(0)));
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setOpportunistic, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setOpportunistic(opportunistic, subId, callingPackage);
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
           * Inform SubscriptionManager that subscriptions in the list are bundled
           * as a group. Typically it's a primary subscription and an opportunistic
           * subscription. It should only affect multi-SIM scenarios where primary
           * and opportunistic subscriptions can be activated together.
           * Being in the same group means they might be activated or deactivated
           * together, some of them may be invisible to the users, etc.
           *
           * Caller will either have {@link android.Manifest.permission.MODIFY_PHONE_STATE}
           * permission or can manage all subscriptions in the list, according to their
           * acess rules.
           *
           * @param subIdList list of subId that will be in the same group
           * @return groupUUID a UUID assigned to the subscription group. It returns
           * null if fails.
           *
           */
      @Override public android.os.ParcelUuid createSubscriptionGroup(int[] subIdList, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.ParcelUuid _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeIntArray(subIdList);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createSubscriptionGroup, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createSubscriptionGroup(subIdList, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.ParcelUuid.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Set which subscription is preferred for cellular data. It's
           * designed to overwrite default data subscription temporarily.
           *
           * @param subId which subscription is preferred to for cellular data.
           * @param needValidation whether validation is needed before switching.
           * @param callback callback upon request completion.
           *
           * @hide
           *
           */
      @Override public void setPreferredDataSubscriptionId(int subId, boolean needValidation, com.android.internal.telephony.ISetOpportunisticDataCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(((needValidation)?(1):(0)));
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPreferredDataSubscriptionId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPreferredDataSubscriptionId(subId, needValidation, callback);
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
           * Get which subscription is preferred for cellular data.
           *
           * @hide
           *
           */
      @Override public int getPreferredDataSubscriptionId() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPreferredDataSubscriptionId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPreferredDataSubscriptionId();
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
           * Get User downloaded Profiles.
           *
           * Return opportunistic subscriptions that can be visible to the caller.
           * @return the list of opportunistic subscription info. If none exists, an empty list.
           */
      @Override public java.util.List<android.telephony.SubscriptionInfo> getOpportunisticSubscriptions(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.telephony.SubscriptionInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getOpportunisticSubscriptions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getOpportunisticSubscriptions(callingPackage);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.telephony.SubscriptionInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void removeSubscriptionsFromGroup(int[] subIdList, android.os.ParcelUuid groupUuid, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeIntArray(subIdList);
          if ((groupUuid!=null)) {
            _data.writeInt(1);
            groupUuid.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeSubscriptionsFromGroup, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeSubscriptionsFromGroup(subIdList, groupUuid, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void addSubscriptionsIntoGroup(int[] subIdList, android.os.ParcelUuid groupUuid, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeIntArray(subIdList);
          if ((groupUuid!=null)) {
            _data.writeInt(1);
            groupUuid.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addSubscriptionsIntoGroup, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addSubscriptionsIntoGroup(subIdList, groupUuid, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.util.List<android.telephony.SubscriptionInfo> getSubscriptionsInGroup(android.os.ParcelUuid groupUuid, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.telephony.SubscriptionInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((groupUuid!=null)) {
            _data.writeInt(1);
            groupUuid.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSubscriptionsInGroup, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSubscriptionsInGroup(groupUuid, callingPackage);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.telephony.SubscriptionInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int getSlotIndex(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSlotIndex, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSlotIndex(subId);
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
      @Override public int[] getSubId(int slotIndex) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotIndex);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSubId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSubId(slotIndex);
          }
          _reply.readException();
          _result = _reply.createIntArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int getDefaultSubId() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDefaultSubId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDefaultSubId();
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
      @Override public int clearSubInfo() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearSubInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().clearSubInfo();
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
      @Override public int getPhoneId(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPhoneId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPhoneId(subId);
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
           * Get the default data subscription
           * @return Id of the data subscription
           */
      @Override public int getDefaultDataSubId() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDefaultDataSubId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDefaultDataSubId();
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
      @Override public void setDefaultDataSubId(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDefaultDataSubId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDefaultDataSubId(subId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getDefaultVoiceSubId() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDefaultVoiceSubId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDefaultVoiceSubId();
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
      @Override public void setDefaultVoiceSubId(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDefaultVoiceSubId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDefaultVoiceSubId(subId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getDefaultSmsSubId() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDefaultSmsSubId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDefaultSmsSubId();
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
      @Override public void setDefaultSmsSubId(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDefaultSmsSubId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDefaultSmsSubId(subId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int[] getActiveSubIdList(boolean visibleOnly) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((visibleOnly)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveSubIdList, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActiveSubIdList(visibleOnly);
          }
          _reply.readException();
          _result = _reply.createIntArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int setSubscriptionProperty(int subId, java.lang.String propKey, java.lang.String propValue) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(propKey);
          _data.writeString(propValue);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSubscriptionProperty, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setSubscriptionProperty(subId, propKey, propValue);
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
      @Override public java.lang.String getSubscriptionProperty(int subId, java.lang.String propKey, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(propKey);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSubscriptionProperty, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSubscriptionProperty(subId, propKey, callingPackage);
          }
          _reply.readException();
          _result = _reply.readString();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public boolean setSubscriptionEnabled(boolean enable, int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enable)?(1):(0)));
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setSubscriptionEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setSubscriptionEnabled(enable, subId);
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
      @Override public boolean isSubscriptionEnabled(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isSubscriptionEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isSubscriptionEnabled(subId);
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
      @Override public int getEnabledSubscriptionId(int slotIndex) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotIndex);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getEnabledSubscriptionId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getEnabledSubscriptionId(slotIndex);
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
           * Get the SIM state for the slot index
           * @return SIM state as the ordinal of IccCardConstants.State
           */
      @Override public int getSimStateForSlotIndex(int slotIndex) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(slotIndex);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSimStateForSlotIndex, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSimStateForSlotIndex(slotIndex);
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
      @Override public boolean isActiveSubId(int subId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isActiveSubId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isActiveSubId(subId, callingPackage);
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
      @Override public boolean setAlwaysAllowMmsData(int subId, boolean alwaysAllow) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(((alwaysAllow)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAlwaysAllowMmsData, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setAlwaysAllowMmsData(subId, alwaysAllow);
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
      @Override public int getActiveDataSubscriptionId() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getActiveDataSubscriptionId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getActiveDataSubscriptionId();
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
      public static com.android.internal.telephony.ISub sDefaultImpl;
    }
    static final int TRANSACTION_getAllSubInfoList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getAllSubInfoCount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getActiveSubscriptionInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getActiveSubscriptionInfoForIccId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getActiveSubscriptionInfoForSimSlotIndex = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getActiveSubscriptionInfoList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_getActiveSubInfoCount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_getActiveSubInfoCountMax = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_getAvailableSubscriptionInfoList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getAccessibleSubscriptionInfoList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_requestEmbeddedSubscriptionInfoListRefresh = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_addSubInfoRecord = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_addSubInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_removeSubInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_setIconTint = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_setDisplayNameUsingSrc = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_setDisplayNumber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_setDataRoaming = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_setOpportunistic = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_createSubscriptionGroup = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_setPreferredDataSubscriptionId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_getPreferredDataSubscriptionId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_getOpportunisticSubscriptions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_removeSubscriptionsFromGroup = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_addSubscriptionsIntoGroup = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_getSubscriptionsInGroup = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_getSlotIndex = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_getSubId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_getDefaultSubId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_clearSubInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_getPhoneId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_getDefaultDataSubId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_setDefaultDataSubId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_getDefaultVoiceSubId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_setDefaultVoiceSubId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_getDefaultSmsSubId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_setDefaultSmsSubId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    static final int TRANSACTION_getActiveSubIdList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 37);
    static final int TRANSACTION_setSubscriptionProperty = (android.os.IBinder.FIRST_CALL_TRANSACTION + 38);
    static final int TRANSACTION_getSubscriptionProperty = (android.os.IBinder.FIRST_CALL_TRANSACTION + 39);
    static final int TRANSACTION_setSubscriptionEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 40);
    static final int TRANSACTION_isSubscriptionEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 41);
    static final int TRANSACTION_getEnabledSubscriptionId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 42);
    static final int TRANSACTION_getSimStateForSlotIndex = (android.os.IBinder.FIRST_CALL_TRANSACTION + 43);
    static final int TRANSACTION_isActiveSubId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 44);
    static final int TRANSACTION_setAlwaysAllowMmsData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 45);
    static final int TRANSACTION_getActiveDataSubscriptionId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 46);
    public static boolean setDefaultImpl(com.android.internal.telephony.ISub impl) {
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
    public static com.android.internal.telephony.ISub getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * @param callingPackage The package maing the call.
       * @return a list of all subscriptions in the database, this includes
       * all subscriptions that have been seen.
       */
  public java.util.List<android.telephony.SubscriptionInfo> getAllSubInfoList(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @param callingPackage The package maing the call.
       * @return the count of all subscriptions in the database, this includes
       * all subscriptions that have been seen.
       */
  public int getAllSubInfoCount(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Get the active SubscriptionInfo with the subId key
       * @param subId The unique SubscriptionInfo key in database
       * @param callingPackage The package maing the call.
       * @return SubscriptionInfo, maybe null if its not active
       */
  public android.telephony.SubscriptionInfo getActiveSubscriptionInfo(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Get the active SubscriptionInfo associated with the iccId
       * @param iccId the IccId of SIM card
       * @param callingPackage The package maing the call.
       * @return SubscriptionInfo, maybe null if its not active
       */
  public android.telephony.SubscriptionInfo getActiveSubscriptionInfoForIccId(java.lang.String iccId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Get the active SubscriptionInfo associated with the slotIndex
       * @param slotIndex the slot which the subscription is inserted
       * @param callingPackage The package making the call.
       * @return SubscriptionInfo, null for Remote-SIMs or non-active slotIndex.
       */
  public android.telephony.SubscriptionInfo getActiveSubscriptionInfoForSimSlotIndex(int slotIndex, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Get the SubscriptionInfo(s) of the active subscriptions. The records will be sorted
       * by {@link SubscriptionInfo#getSimSlotIndex} then by {@link SubscriptionInfo#getSubscriptionId}.
       *
       * @param callingPackage The package maing the call.
       * @return Sorted list of the currently {@link SubscriptionInfo} records available on the device.
       * <ul>
       * <li>
       * If null is returned the current state is unknown but if a {@link OnSubscriptionsChangedListener}
       * has been registered {@link OnSubscriptionsChangedListener#onSubscriptionsChanged} will be
       * invoked in the future.
       * </li>
       * <li>
       * If the list is empty then there are no {@link SubscriptionInfo} records currently available.
       * </li>
       * <li>
       * if the list is non-empty the list is sorted by {@link SubscriptionInfo#getSimSlotIndex}
       * then by {@link SubscriptionInfo#getSubscriptionId}.
       * </li>
       * </ul>
       */
  public java.util.List<android.telephony.SubscriptionInfo> getActiveSubscriptionInfoList(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @param callingPackage The package making the call.
       * @return the number of active subscriptions
       */
  public int getActiveSubInfoCount(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @return the maximum number of subscriptions this device will support at any one time.
       */
  public int getActiveSubInfoCountMax() throws android.os.RemoteException;
  /**
       * @see android.telephony.SubscriptionManager#getAvailableSubscriptionInfoList
       */
  public java.util.List<android.telephony.SubscriptionInfo> getAvailableSubscriptionInfoList(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see android.telephony.SubscriptionManager#getAccessibleSubscriptionInfoList
       */
  public java.util.List<android.telephony.SubscriptionInfo> getAccessibleSubscriptionInfoList(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see android.telephony.SubscriptionManager#requestEmbeddedSubscriptionInfoListRefresh
       */
  public void requestEmbeddedSubscriptionInfoListRefresh(int cardId) throws android.os.RemoteException;
  /**
       * Add a new SubscriptionInfo to subinfo database if needed
       * @param iccId the IccId of the SIM card
       * @param slotIndex the slot which the SIM is inserted
       * @return the URL of the newly created row or the updated row
       */
  public int addSubInfoRecord(java.lang.String iccId, int slotIndex) throws android.os.RemoteException;
  /**
       * Add a new subscription info record, if needed
       * @param uniqueId This is the unique identifier for the subscription within the specific
       *                 subscription type.
       * @param displayName human-readable name of the device the subscription corresponds to.
       * @param slotIndex the slot assigned to this device
       * @param subscriptionType the type of subscription to be added.
       * @return 0 if success, < 0 on error.
       */
  public int addSubInfo(java.lang.String uniqueId, java.lang.String displayName, int slotIndex, int subscriptionType) throws android.os.RemoteException;
  /**
       * Remove subscription info record for the given device.
       * @param uniqueId This is the unique identifier for the subscription within the specific
       *                      subscription type.
       * @param subscriptionType the type of subscription to be removed
       * @return 0 if success, < 0 on error.
       */
  public int removeSubInfo(java.lang.String uniqueId, int subscriptionType) throws android.os.RemoteException;
  /**
       * Set SIM icon tint color by simInfo index
       * @param tint the icon tint color of the SIM
       * @param subId the unique SubscriptionInfo index in database
       * @return the number of records updated
       */
  public int setIconTint(int tint, int subId) throws android.os.RemoteException;
  /**
       * Set display name by simInfo index with name source
       * @param displayName the display name of SIM card
       * @param subId the unique SubscriptionInfo index in database
       * @param nameSource, 0: DEFAULT_SOURCE, 1: SIM_SOURCE, 2: USER_INPUT
       * @return the number of records updated
       */
  public int setDisplayNameUsingSrc(java.lang.String displayName, int subId, int nameSource) throws android.os.RemoteException;
  /**
       * Set phone number by subId
       * @param number the phone number of the SIM
       * @param subId the unique SubscriptionInfo index in database
       * @return the number of records updated
       */
  public int setDisplayNumber(java.lang.String number, int subId) throws android.os.RemoteException;
  /**
       * Set data roaming by simInfo index
       * @param roaming 0:Don't allow data when roaming, 1:Allow data when roaming
       * @param subId the unique SubscriptionInfo index in database
       * @return the number of records updated
       */
  public int setDataRoaming(int roaming, int subId) throws android.os.RemoteException;
  /**
       * Switch to a certain subscription
       *
       * @param opportunistic whether it’s opportunistic subscription.
       * @param subId the unique SubscriptionInfo index in database
       * @return the number of records updated
       */
  public int setOpportunistic(boolean opportunistic, int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Inform SubscriptionManager that subscriptions in the list are bundled
       * as a group. Typically it's a primary subscription and an opportunistic
       * subscription. It should only affect multi-SIM scenarios where primary
       * and opportunistic subscriptions can be activated together.
       * Being in the same group means they might be activated or deactivated
       * together, some of them may be invisible to the users, etc.
       *
       * Caller will either have {@link android.Manifest.permission.MODIFY_PHONE_STATE}
       * permission or can manage all subscriptions in the list, according to their
       * acess rules.
       *
       * @param subIdList list of subId that will be in the same group
       * @return groupUUID a UUID assigned to the subscription group. It returns
       * null if fails.
       *
       */
  public android.os.ParcelUuid createSubscriptionGroup(int[] subIdList, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * Set which subscription is preferred for cellular data. It's
       * designed to overwrite default data subscription temporarily.
       *
       * @param subId which subscription is preferred to for cellular data.
       * @param needValidation whether validation is needed before switching.
       * @param callback callback upon request completion.
       *
       * @hide
       *
       */
  public void setPreferredDataSubscriptionId(int subId, boolean needValidation, com.android.internal.telephony.ISetOpportunisticDataCallback callback) throws android.os.RemoteException;
  /**
       * Get which subscription is preferred for cellular data.
       *
       * @hide
       *
       */
  public int getPreferredDataSubscriptionId() throws android.os.RemoteException;
  /**
       * Get User downloaded Profiles.
       *
       * Return opportunistic subscriptions that can be visible to the caller.
       * @return the list of opportunistic subscription info. If none exists, an empty list.
       */
  public java.util.List<android.telephony.SubscriptionInfo> getOpportunisticSubscriptions(java.lang.String callingPackage) throws android.os.RemoteException;
  public void removeSubscriptionsFromGroup(int[] subIdList, android.os.ParcelUuid groupUuid, java.lang.String callingPackage) throws android.os.RemoteException;
  public void addSubscriptionsIntoGroup(int[] subIdList, android.os.ParcelUuid groupUuid, java.lang.String callingPackage) throws android.os.RemoteException;
  public java.util.List<android.telephony.SubscriptionInfo> getSubscriptionsInGroup(android.os.ParcelUuid groupUuid, java.lang.String callingPackage) throws android.os.RemoteException;
  public int getSlotIndex(int subId) throws android.os.RemoteException;
  public int[] getSubId(int slotIndex) throws android.os.RemoteException;
  public int getDefaultSubId() throws android.os.RemoteException;
  public int clearSubInfo() throws android.os.RemoteException;
  public int getPhoneId(int subId) throws android.os.RemoteException;
  /**
       * Get the default data subscription
       * @return Id of the data subscription
       */
  public int getDefaultDataSubId() throws android.os.RemoteException;
  public void setDefaultDataSubId(int subId) throws android.os.RemoteException;
  public int getDefaultVoiceSubId() throws android.os.RemoteException;
  public void setDefaultVoiceSubId(int subId) throws android.os.RemoteException;
  public int getDefaultSmsSubId() throws android.os.RemoteException;
  public void setDefaultSmsSubId(int subId) throws android.os.RemoteException;
  public int[] getActiveSubIdList(boolean visibleOnly) throws android.os.RemoteException;
  public int setSubscriptionProperty(int subId, java.lang.String propKey, java.lang.String propValue) throws android.os.RemoteException;
  public java.lang.String getSubscriptionProperty(int subId, java.lang.String propKey, java.lang.String callingPackage) throws android.os.RemoteException;
  public boolean setSubscriptionEnabled(boolean enable, int subId) throws android.os.RemoteException;
  public boolean isSubscriptionEnabled(int subId) throws android.os.RemoteException;
  public int getEnabledSubscriptionId(int slotIndex) throws android.os.RemoteException;
  /**
       * Get the SIM state for the slot index
       * @return SIM state as the ordinal of IccCardConstants.State
       */
  public int getSimStateForSlotIndex(int slotIndex) throws android.os.RemoteException;
  public boolean isActiveSubId(int subId, java.lang.String callingPackage) throws android.os.RemoteException;
  public boolean setAlwaysAllowMmsData(int subId, boolean alwaysAllow) throws android.os.RemoteException;
  public int getActiveDataSubscriptionId() throws android.os.RemoteException;
}
