/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.telephony;
public interface ITelephonyRegistry extends android.os.IInterface
{
  /** Default implementation for ITelephonyRegistry. */
  public static class Default implements com.android.internal.telephony.ITelephonyRegistry
  {
    @Override public void addOnSubscriptionsChangedListener(java.lang.String pkg, com.android.internal.telephony.IOnSubscriptionsChangedListener callback) throws android.os.RemoteException
    {
    }
    @Override public void addOnOpportunisticSubscriptionsChangedListener(java.lang.String pkg, com.android.internal.telephony.IOnSubscriptionsChangedListener callback) throws android.os.RemoteException
    {
    }
    @Override public void removeOnSubscriptionsChangedListener(java.lang.String pkg, com.android.internal.telephony.IOnSubscriptionsChangedListener callback) throws android.os.RemoteException
    {
    }
    @Override public void listen(java.lang.String pkg, com.android.internal.telephony.IPhoneStateListener callback, int events, boolean notifyNow) throws android.os.RemoteException
    {
    }
    @Override public void listenForSubscriber(int subId, java.lang.String pkg, com.android.internal.telephony.IPhoneStateListener callback, int events, boolean notifyNow) throws android.os.RemoteException
    {
    }
    @Override public void notifyCallState(int state, java.lang.String incomingNumber) throws android.os.RemoteException
    {
    }
    @Override public void notifyCallStateForPhoneId(int phoneId, int subId, int state, java.lang.String incomingNumber) throws android.os.RemoteException
    {
    }
    @Override public void notifyServiceStateForPhoneId(int phoneId, int subId, android.telephony.ServiceState state) throws android.os.RemoteException
    {
    }
    @Override public void notifySignalStrengthForPhoneId(int phoneId, int subId, android.telephony.SignalStrength signalStrength) throws android.os.RemoteException
    {
    }
    @Override public void notifyMessageWaitingChangedForPhoneId(int phoneId, int subId, boolean mwi) throws android.os.RemoteException
    {
    }
    @Override public void notifyCallForwardingChanged(boolean cfi) throws android.os.RemoteException
    {
    }
    @Override public void notifyCallForwardingChangedForSubscriber(int subId, boolean cfi) throws android.os.RemoteException
    {
    }
    @Override public void notifyDataActivity(int state) throws android.os.RemoteException
    {
    }
    @Override public void notifyDataActivityForSubscriber(int subId, int state) throws android.os.RemoteException
    {
    }
    @Override public void notifyDataConnection(int state, boolean isDataConnectivityPossible, java.lang.String apn, java.lang.String apnType, android.net.LinkProperties linkProperties, android.net.NetworkCapabilities networkCapabilities, int networkType, boolean roaming) throws android.os.RemoteException
    {
    }
    @Override public void notifyDataConnectionForSubscriber(int phoneId, int subId, int state, boolean isDataConnectivityPossible, java.lang.String apn, java.lang.String apnType, android.net.LinkProperties linkProperties, android.net.NetworkCapabilities networkCapabilities, int networkType, boolean roaming) throws android.os.RemoteException
    {
    }
    @Override public void notifyDataConnectionFailed(java.lang.String apnType) throws android.os.RemoteException
    {
    }
    @Override public void notifyDataConnectionFailedForSubscriber(int phoneId, int subId, java.lang.String apnType) throws android.os.RemoteException
    {
    }
    @Override public void notifyCellLocation(android.os.Bundle cellLocation) throws android.os.RemoteException
    {
    }
    @Override public void notifyCellLocationForSubscriber(int subId, android.os.Bundle cellLocation) throws android.os.RemoteException
    {
    }
    @Override public void notifyOtaspChanged(int subId, int otaspMode) throws android.os.RemoteException
    {
    }
    @Override public void notifyCellInfo(java.util.List<android.telephony.CellInfo> cellInfo) throws android.os.RemoteException
    {
    }
    @Override public void notifyPhysicalChannelConfigurationForSubscriber(int phoneId, int subId, java.util.List<android.telephony.PhysicalChannelConfig> configs) throws android.os.RemoteException
    {
    }
    @Override public void notifyPreciseCallState(int phoneId, int subId, int ringingCallState, int foregroundCallState, int backgroundCallState) throws android.os.RemoteException
    {
    }
    @Override public void notifyDisconnectCause(int phoneId, int subId, int disconnectCause, int preciseDisconnectCause) throws android.os.RemoteException
    {
    }
    @Override public void notifyPreciseDataConnectionFailed(int phoneId, int subId, java.lang.String apnType, java.lang.String apn, int failCause) throws android.os.RemoteException
    {
    }
    @Override public void notifyCellInfoForSubscriber(int subId, java.util.List<android.telephony.CellInfo> cellInfo) throws android.os.RemoteException
    {
    }
    @Override public void notifySrvccStateChanged(int subId, int lteState) throws android.os.RemoteException
    {
    }
    @Override public void notifySimActivationStateChangedForPhoneId(int phoneId, int subId, int activationState, int activationType) throws android.os.RemoteException
    {
    }
    @Override public void notifyOemHookRawEventForSubscriber(int phoneId, int subId, byte[] rawData) throws android.os.RemoteException
    {
    }
    @Override public void notifySubscriptionInfoChanged() throws android.os.RemoteException
    {
    }
    @Override public void notifyOpportunisticSubscriptionInfoChanged() throws android.os.RemoteException
    {
    }
    @Override public void notifyCarrierNetworkChange(boolean active) throws android.os.RemoteException
    {
    }
    @Override public void notifyUserMobileDataStateChangedForPhoneId(int phoneId, int subId, boolean state) throws android.os.RemoteException
    {
    }
    @Override public void notifyPhoneCapabilityChanged(android.telephony.PhoneCapability capability) throws android.os.RemoteException
    {
    }
    @Override public void notifyActiveDataSubIdChanged(int activeDataSubId) throws android.os.RemoteException
    {
    }
    @Override public void notifyRadioPowerStateChanged(int phoneId, int subId, int state) throws android.os.RemoteException
    {
    }
    @Override public void notifyEmergencyNumberList(int phoneId, int subId) throws android.os.RemoteException
    {
    }
    @Override public void notifyCallQualityChanged(android.telephony.CallQuality callQuality, int phoneId, int subId, int callNetworkType) throws android.os.RemoteException
    {
    }
    @Override public void notifyImsDisconnectCause(int subId, android.telephony.ims.ImsReasonInfo imsReasonInfo) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.telephony.ITelephonyRegistry
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.telephony.ITelephonyRegistry";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.telephony.ITelephonyRegistry interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.telephony.ITelephonyRegistry asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.telephony.ITelephonyRegistry))) {
        return ((com.android.internal.telephony.ITelephonyRegistry)iin);
      }
      return new com.android.internal.telephony.ITelephonyRegistry.Stub.Proxy(obj);
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
        case TRANSACTION_addOnSubscriptionsChangedListener:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          com.android.internal.telephony.IOnSubscriptionsChangedListener _arg1;
          _arg1 = com.android.internal.telephony.IOnSubscriptionsChangedListener.Stub.asInterface(data.readStrongBinder());
          this.addOnSubscriptionsChangedListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addOnOpportunisticSubscriptionsChangedListener:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          com.android.internal.telephony.IOnSubscriptionsChangedListener _arg1;
          _arg1 = com.android.internal.telephony.IOnSubscriptionsChangedListener.Stub.asInterface(data.readStrongBinder());
          this.addOnOpportunisticSubscriptionsChangedListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeOnSubscriptionsChangedListener:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          com.android.internal.telephony.IOnSubscriptionsChangedListener _arg1;
          _arg1 = com.android.internal.telephony.IOnSubscriptionsChangedListener.Stub.asInterface(data.readStrongBinder());
          this.removeOnSubscriptionsChangedListener(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_listen:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          com.android.internal.telephony.IPhoneStateListener _arg1;
          _arg1 = com.android.internal.telephony.IPhoneStateListener.Stub.asInterface(data.readStrongBinder());
          int _arg2;
          _arg2 = data.readInt();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          this.listen(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_listenForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          com.android.internal.telephony.IPhoneStateListener _arg2;
          _arg2 = com.android.internal.telephony.IPhoneStateListener.Stub.asInterface(data.readStrongBinder());
          int _arg3;
          _arg3 = data.readInt();
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          this.listenForSubscriber(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyCallState:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.notifyCallState(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyCallStateForPhoneId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.notifyCallStateForPhoneId(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyServiceStateForPhoneId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.telephony.ServiceState _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telephony.ServiceState.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.notifyServiceStateForPhoneId(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifySignalStrengthForPhoneId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.telephony.SignalStrength _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telephony.SignalStrength.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.notifySignalStrengthForPhoneId(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyMessageWaitingChangedForPhoneId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.notifyMessageWaitingChangedForPhoneId(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyCallForwardingChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.notifyCallForwardingChanged(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyCallForwardingChangedForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.notifyCallForwardingChangedForSubscriber(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyDataActivity:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.notifyDataActivity(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyDataActivityForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.notifyDataActivityForSubscriber(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyDataConnection:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String _arg3;
          _arg3 = data.readString();
          android.net.LinkProperties _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.net.LinkProperties.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          android.net.NetworkCapabilities _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.net.NetworkCapabilities.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          int _arg6;
          _arg6 = data.readInt();
          boolean _arg7;
          _arg7 = (0!=data.readInt());
          this.notifyDataConnection(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyDataConnectionForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          java.lang.String _arg4;
          _arg4 = data.readString();
          java.lang.String _arg5;
          _arg5 = data.readString();
          android.net.LinkProperties _arg6;
          if ((0!=data.readInt())) {
            _arg6 = android.net.LinkProperties.CREATOR.createFromParcel(data);
          }
          else {
            _arg6 = null;
          }
          android.net.NetworkCapabilities _arg7;
          if ((0!=data.readInt())) {
            _arg7 = android.net.NetworkCapabilities.CREATOR.createFromParcel(data);
          }
          else {
            _arg7 = null;
          }
          int _arg8;
          _arg8 = data.readInt();
          boolean _arg9;
          _arg9 = (0!=data.readInt());
          this.notifyDataConnectionForSubscriber(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyDataConnectionFailed:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.notifyDataConnectionFailed(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyDataConnectionFailedForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.notifyDataConnectionFailedForSubscriber(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyCellLocation:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.notifyCellLocation(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyCellLocationForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.notifyCellLocationForSubscriber(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyOtaspChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.notifyOtaspChanged(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyCellInfo:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.telephony.CellInfo> _arg0;
          _arg0 = data.createTypedArrayList(android.telephony.CellInfo.CREATOR);
          this.notifyCellInfo(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyPhysicalChannelConfigurationForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.util.List<android.telephony.PhysicalChannelConfig> _arg2;
          _arg2 = data.createTypedArrayList(android.telephony.PhysicalChannelConfig.CREATOR);
          this.notifyPhysicalChannelConfigurationForSubscriber(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyPreciseCallState:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          this.notifyPreciseCallState(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyDisconnectCause:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          this.notifyDisconnectCause(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyPreciseDataConnectionFailed:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String _arg3;
          _arg3 = data.readString();
          int _arg4;
          _arg4 = data.readInt();
          this.notifyPreciseDataConnectionFailed(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyCellInfoForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.util.List<android.telephony.CellInfo> _arg1;
          _arg1 = data.createTypedArrayList(android.telephony.CellInfo.CREATOR);
          this.notifyCellInfoForSubscriber(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifySrvccStateChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.notifySrvccStateChanged(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifySimActivationStateChangedForPhoneId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          this.notifySimActivationStateChangedForPhoneId(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyOemHookRawEventForSubscriber:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          byte[] _arg2;
          _arg2 = data.createByteArray();
          this.notifyOemHookRawEventForSubscriber(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifySubscriptionInfoChanged:
        {
          data.enforceInterface(descriptor);
          this.notifySubscriptionInfoChanged();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyOpportunisticSubscriptionInfoChanged:
        {
          data.enforceInterface(descriptor);
          this.notifyOpportunisticSubscriptionInfoChanged();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyCarrierNetworkChange:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.notifyCarrierNetworkChange(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyUserMobileDataStateChangedForPhoneId:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.notifyUserMobileDataStateChangedForPhoneId(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyPhoneCapabilityChanged:
        {
          data.enforceInterface(descriptor);
          android.telephony.PhoneCapability _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.PhoneCapability.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.notifyPhoneCapabilityChanged(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyActiveDataSubIdChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.notifyActiveDataSubIdChanged(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyRadioPowerStateChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.notifyRadioPowerStateChanged(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyEmergencyNumberList:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.notifyEmergencyNumberList(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyCallQualityChanged:
        {
          data.enforceInterface(descriptor);
          android.telephony.CallQuality _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.CallQuality.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          this.notifyCallQualityChanged(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyImsDisconnectCause:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.ims.ImsReasonInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telephony.ims.ImsReasonInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.notifyImsDisconnectCause(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.telephony.ITelephonyRegistry
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
      @Override public void addOnSubscriptionsChangedListener(java.lang.String pkg, com.android.internal.telephony.IOnSubscriptionsChangedListener callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(pkg);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addOnSubscriptionsChangedListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addOnSubscriptionsChangedListener(pkg, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void addOnOpportunisticSubscriptionsChangedListener(java.lang.String pkg, com.android.internal.telephony.IOnSubscriptionsChangedListener callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(pkg);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addOnOpportunisticSubscriptionsChangedListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addOnOpportunisticSubscriptionsChangedListener(pkg, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeOnSubscriptionsChangedListener(java.lang.String pkg, com.android.internal.telephony.IOnSubscriptionsChangedListener callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(pkg);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeOnSubscriptionsChangedListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeOnSubscriptionsChangedListener(pkg, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void listen(java.lang.String pkg, com.android.internal.telephony.IPhoneStateListener callback, int events, boolean notifyNow) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(pkg);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(events);
          _data.writeInt(((notifyNow)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_listen, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().listen(pkg, callback, events, notifyNow);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void listenForSubscriber(int subId, java.lang.String pkg, com.android.internal.telephony.IPhoneStateListener callback, int events, boolean notifyNow) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeString(pkg);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeInt(events);
          _data.writeInt(((notifyNow)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_listenForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().listenForSubscriber(subId, pkg, callback, events, notifyNow);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyCallState(int state, java.lang.String incomingNumber) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(state);
          _data.writeString(incomingNumber);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyCallState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyCallState(state, incomingNumber);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyCallStateForPhoneId(int phoneId, int subId, int state, java.lang.String incomingNumber) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(phoneId);
          _data.writeInt(subId);
          _data.writeInt(state);
          _data.writeString(incomingNumber);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyCallStateForPhoneId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyCallStateForPhoneId(phoneId, subId, state, incomingNumber);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyServiceStateForPhoneId(int phoneId, int subId, android.telephony.ServiceState state) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(phoneId);
          _data.writeInt(subId);
          if ((state!=null)) {
            _data.writeInt(1);
            state.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyServiceStateForPhoneId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyServiceStateForPhoneId(phoneId, subId, state);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifySignalStrengthForPhoneId(int phoneId, int subId, android.telephony.SignalStrength signalStrength) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(phoneId);
          _data.writeInt(subId);
          if ((signalStrength!=null)) {
            _data.writeInt(1);
            signalStrength.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifySignalStrengthForPhoneId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifySignalStrengthForPhoneId(phoneId, subId, signalStrength);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyMessageWaitingChangedForPhoneId(int phoneId, int subId, boolean mwi) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(phoneId);
          _data.writeInt(subId);
          _data.writeInt(((mwi)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyMessageWaitingChangedForPhoneId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyMessageWaitingChangedForPhoneId(phoneId, subId, mwi);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyCallForwardingChanged(boolean cfi) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((cfi)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyCallForwardingChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyCallForwardingChanged(cfi);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyCallForwardingChangedForSubscriber(int subId, boolean cfi) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(((cfi)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyCallForwardingChangedForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyCallForwardingChangedForSubscriber(subId, cfi);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyDataActivity(int state) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(state);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyDataActivity, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyDataActivity(state);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyDataActivityForSubscriber(int subId, int state) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(state);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyDataActivityForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyDataActivityForSubscriber(subId, state);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyDataConnection(int state, boolean isDataConnectivityPossible, java.lang.String apn, java.lang.String apnType, android.net.LinkProperties linkProperties, android.net.NetworkCapabilities networkCapabilities, int networkType, boolean roaming) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(state);
          _data.writeInt(((isDataConnectivityPossible)?(1):(0)));
          _data.writeString(apn);
          _data.writeString(apnType);
          if ((linkProperties!=null)) {
            _data.writeInt(1);
            linkProperties.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((networkCapabilities!=null)) {
            _data.writeInt(1);
            networkCapabilities.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(networkType);
          _data.writeInt(((roaming)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyDataConnection, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyDataConnection(state, isDataConnectivityPossible, apn, apnType, linkProperties, networkCapabilities, networkType, roaming);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyDataConnectionForSubscriber(int phoneId, int subId, int state, boolean isDataConnectivityPossible, java.lang.String apn, java.lang.String apnType, android.net.LinkProperties linkProperties, android.net.NetworkCapabilities networkCapabilities, int networkType, boolean roaming) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(phoneId);
          _data.writeInt(subId);
          _data.writeInt(state);
          _data.writeInt(((isDataConnectivityPossible)?(1):(0)));
          _data.writeString(apn);
          _data.writeString(apnType);
          if ((linkProperties!=null)) {
            _data.writeInt(1);
            linkProperties.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((networkCapabilities!=null)) {
            _data.writeInt(1);
            networkCapabilities.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(networkType);
          _data.writeInt(((roaming)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyDataConnectionForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyDataConnectionForSubscriber(phoneId, subId, state, isDataConnectivityPossible, apn, apnType, linkProperties, networkCapabilities, networkType, roaming);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyDataConnectionFailed(java.lang.String apnType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(apnType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyDataConnectionFailed, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyDataConnectionFailed(apnType);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyDataConnectionFailedForSubscriber(int phoneId, int subId, java.lang.String apnType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(phoneId);
          _data.writeInt(subId);
          _data.writeString(apnType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyDataConnectionFailedForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyDataConnectionFailedForSubscriber(phoneId, subId, apnType);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyCellLocation(android.os.Bundle cellLocation) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((cellLocation!=null)) {
            _data.writeInt(1);
            cellLocation.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyCellLocation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyCellLocation(cellLocation);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyCellLocationForSubscriber(int subId, android.os.Bundle cellLocation) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          if ((cellLocation!=null)) {
            _data.writeInt(1);
            cellLocation.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyCellLocationForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyCellLocationForSubscriber(subId, cellLocation);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyOtaspChanged(int subId, int otaspMode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(otaspMode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyOtaspChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyOtaspChanged(subId, otaspMode);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyCellInfo(java.util.List<android.telephony.CellInfo> cellInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(cellInfo);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyCellInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyCellInfo(cellInfo);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyPhysicalChannelConfigurationForSubscriber(int phoneId, int subId, java.util.List<android.telephony.PhysicalChannelConfig> configs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(phoneId);
          _data.writeInt(subId);
          _data.writeTypedList(configs);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyPhysicalChannelConfigurationForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyPhysicalChannelConfigurationForSubscriber(phoneId, subId, configs);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyPreciseCallState(int phoneId, int subId, int ringingCallState, int foregroundCallState, int backgroundCallState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(phoneId);
          _data.writeInt(subId);
          _data.writeInt(ringingCallState);
          _data.writeInt(foregroundCallState);
          _data.writeInt(backgroundCallState);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyPreciseCallState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyPreciseCallState(phoneId, subId, ringingCallState, foregroundCallState, backgroundCallState);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyDisconnectCause(int phoneId, int subId, int disconnectCause, int preciseDisconnectCause) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(phoneId);
          _data.writeInt(subId);
          _data.writeInt(disconnectCause);
          _data.writeInt(preciseDisconnectCause);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyDisconnectCause, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyDisconnectCause(phoneId, subId, disconnectCause, preciseDisconnectCause);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyPreciseDataConnectionFailed(int phoneId, int subId, java.lang.String apnType, java.lang.String apn, int failCause) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(phoneId);
          _data.writeInt(subId);
          _data.writeString(apnType);
          _data.writeString(apn);
          _data.writeInt(failCause);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyPreciseDataConnectionFailed, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyPreciseDataConnectionFailed(phoneId, subId, apnType, apn, failCause);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyCellInfoForSubscriber(int subId, java.util.List<android.telephony.CellInfo> cellInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeTypedList(cellInfo);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyCellInfoForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyCellInfoForSubscriber(subId, cellInfo);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifySrvccStateChanged(int subId, int lteState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          _data.writeInt(lteState);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifySrvccStateChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifySrvccStateChanged(subId, lteState);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifySimActivationStateChangedForPhoneId(int phoneId, int subId, int activationState, int activationType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(phoneId);
          _data.writeInt(subId);
          _data.writeInt(activationState);
          _data.writeInt(activationType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifySimActivationStateChangedForPhoneId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifySimActivationStateChangedForPhoneId(phoneId, subId, activationState, activationType);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyOemHookRawEventForSubscriber(int phoneId, int subId, byte[] rawData) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(phoneId);
          _data.writeInt(subId);
          _data.writeByteArray(rawData);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyOemHookRawEventForSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyOemHookRawEventForSubscriber(phoneId, subId, rawData);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifySubscriptionInfoChanged() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifySubscriptionInfoChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifySubscriptionInfoChanged();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyOpportunisticSubscriptionInfoChanged() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyOpportunisticSubscriptionInfoChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyOpportunisticSubscriptionInfoChanged();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyCarrierNetworkChange(boolean active) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((active)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyCarrierNetworkChange, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyCarrierNetworkChange(active);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyUserMobileDataStateChangedForPhoneId(int phoneId, int subId, boolean state) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(phoneId);
          _data.writeInt(subId);
          _data.writeInt(((state)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyUserMobileDataStateChangedForPhoneId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyUserMobileDataStateChangedForPhoneId(phoneId, subId, state);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyPhoneCapabilityChanged(android.telephony.PhoneCapability capability) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((capability!=null)) {
            _data.writeInt(1);
            capability.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyPhoneCapabilityChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyPhoneCapabilityChanged(capability);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyActiveDataSubIdChanged(int activeDataSubId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(activeDataSubId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyActiveDataSubIdChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyActiveDataSubIdChanged(activeDataSubId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyRadioPowerStateChanged(int phoneId, int subId, int state) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(phoneId);
          _data.writeInt(subId);
          _data.writeInt(state);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyRadioPowerStateChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyRadioPowerStateChanged(phoneId, subId, state);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyEmergencyNumberList(int phoneId, int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(phoneId);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyEmergencyNumberList, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyEmergencyNumberList(phoneId, subId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyCallQualityChanged(android.telephony.CallQuality callQuality, int phoneId, int subId, int callNetworkType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((callQuality!=null)) {
            _data.writeInt(1);
            callQuality.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(phoneId);
          _data.writeInt(subId);
          _data.writeInt(callNetworkType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyCallQualityChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyCallQualityChanged(callQuality, phoneId, subId, callNetworkType);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void notifyImsDisconnectCause(int subId, android.telephony.ims.ImsReasonInfo imsReasonInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          if ((imsReasonInfo!=null)) {
            _data.writeInt(1);
            imsReasonInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyImsDisconnectCause, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyImsDisconnectCause(subId, imsReasonInfo);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.internal.telephony.ITelephonyRegistry sDefaultImpl;
    }
    static final int TRANSACTION_addOnSubscriptionsChangedListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_addOnOpportunisticSubscriptionsChangedListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_removeOnSubscriptionsChangedListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_listen = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_listenForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_notifyCallState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_notifyCallStateForPhoneId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_notifyServiceStateForPhoneId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_notifySignalStrengthForPhoneId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_notifyMessageWaitingChangedForPhoneId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_notifyCallForwardingChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_notifyCallForwardingChangedForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_notifyDataActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_notifyDataActivityForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_notifyDataConnection = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_notifyDataConnectionForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_notifyDataConnectionFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_notifyDataConnectionFailedForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_notifyCellLocation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_notifyCellLocationForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_notifyOtaspChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_notifyCellInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_notifyPhysicalChannelConfigurationForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_notifyPreciseCallState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_notifyDisconnectCause = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_notifyPreciseDataConnectionFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_notifyCellInfoForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_notifySrvccStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_notifySimActivationStateChangedForPhoneId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_notifyOemHookRawEventForSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_notifySubscriptionInfoChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_notifyOpportunisticSubscriptionInfoChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_notifyCarrierNetworkChange = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_notifyUserMobileDataStateChangedForPhoneId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_notifyPhoneCapabilityChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_notifyActiveDataSubIdChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_notifyRadioPowerStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    static final int TRANSACTION_notifyEmergencyNumberList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 37);
    static final int TRANSACTION_notifyCallQualityChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 38);
    static final int TRANSACTION_notifyImsDisconnectCause = (android.os.IBinder.FIRST_CALL_TRANSACTION + 39);
    public static boolean setDefaultImpl(com.android.internal.telephony.ITelephonyRegistry impl) {
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
    public static com.android.internal.telephony.ITelephonyRegistry getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void addOnSubscriptionsChangedListener(java.lang.String pkg, com.android.internal.telephony.IOnSubscriptionsChangedListener callback) throws android.os.RemoteException;
  public void addOnOpportunisticSubscriptionsChangedListener(java.lang.String pkg, com.android.internal.telephony.IOnSubscriptionsChangedListener callback) throws android.os.RemoteException;
  public void removeOnSubscriptionsChangedListener(java.lang.String pkg, com.android.internal.telephony.IOnSubscriptionsChangedListener callback) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ITelephonyRegistry.aidl:41:1:41:25")
  public void listen(java.lang.String pkg, com.android.internal.telephony.IPhoneStateListener callback, int events, boolean notifyNow) throws android.os.RemoteException;
  public void listenForSubscriber(int subId, java.lang.String pkg, com.android.internal.telephony.IPhoneStateListener callback, int events, boolean notifyNow) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ITelephonyRegistry.aidl:45:1:45:25")
  public void notifyCallState(int state, java.lang.String incomingNumber) throws android.os.RemoteException;
  public void notifyCallStateForPhoneId(int phoneId, int subId, int state, java.lang.String incomingNumber) throws android.os.RemoteException;
  public void notifyServiceStateForPhoneId(int phoneId, int subId, android.telephony.ServiceState state) throws android.os.RemoteException;
  public void notifySignalStrengthForPhoneId(int phoneId, int subId, android.telephony.SignalStrength signalStrength) throws android.os.RemoteException;
  public void notifyMessageWaitingChangedForPhoneId(int phoneId, int subId, boolean mwi) throws android.os.RemoteException;
  public void notifyCallForwardingChanged(boolean cfi) throws android.os.RemoteException;
  public void notifyCallForwardingChangedForSubscriber(int subId, boolean cfi) throws android.os.RemoteException;
  public void notifyDataActivity(int state) throws android.os.RemoteException;
  public void notifyDataActivityForSubscriber(int subId, int state) throws android.os.RemoteException;
  public void notifyDataConnection(int state, boolean isDataConnectivityPossible, java.lang.String apn, java.lang.String apnType, android.net.LinkProperties linkProperties, android.net.NetworkCapabilities networkCapabilities, int networkType, boolean roaming) throws android.os.RemoteException;
  public void notifyDataConnectionForSubscriber(int phoneId, int subId, int state, boolean isDataConnectivityPossible, java.lang.String apn, java.lang.String apnType, android.net.LinkProperties linkProperties, android.net.NetworkCapabilities networkCapabilities, int networkType, boolean roaming) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ITelephonyRegistry.aidl:63:1:63:25")
  public void notifyDataConnectionFailed(java.lang.String apnType) throws android.os.RemoteException;
  public void notifyDataConnectionFailedForSubscriber(int phoneId, int subId, java.lang.String apnType) throws android.os.RemoteException;
  public void notifyCellLocation(android.os.Bundle cellLocation) throws android.os.RemoteException;
  public void notifyCellLocationForSubscriber(int subId, android.os.Bundle cellLocation) throws android.os.RemoteException;
  public void notifyOtaspChanged(int subId, int otaspMode) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/internal/telephony/ITelephonyRegistry.aidl:69:1:69:25")
  public void notifyCellInfo(java.util.List<android.telephony.CellInfo> cellInfo) throws android.os.RemoteException;
  public void notifyPhysicalChannelConfigurationForSubscriber(int phoneId, int subId, java.util.List<android.telephony.PhysicalChannelConfig> configs) throws android.os.RemoteException;
  public void notifyPreciseCallState(int phoneId, int subId, int ringingCallState, int foregroundCallState, int backgroundCallState) throws android.os.RemoteException;
  public void notifyDisconnectCause(int phoneId, int subId, int disconnectCause, int preciseDisconnectCause) throws android.os.RemoteException;
  public void notifyPreciseDataConnectionFailed(int phoneId, int subId, java.lang.String apnType, java.lang.String apn, int failCause) throws android.os.RemoteException;
  public void notifyCellInfoForSubscriber(int subId, java.util.List<android.telephony.CellInfo> cellInfo) throws android.os.RemoteException;
  public void notifySrvccStateChanged(int subId, int lteState) throws android.os.RemoteException;
  public void notifySimActivationStateChangedForPhoneId(int phoneId, int subId, int activationState, int activationType) throws android.os.RemoteException;
  public void notifyOemHookRawEventForSubscriber(int phoneId, int subId, byte[] rawData) throws android.os.RemoteException;
  public void notifySubscriptionInfoChanged() throws android.os.RemoteException;
  public void notifyOpportunisticSubscriptionInfoChanged() throws android.os.RemoteException;
  public void notifyCarrierNetworkChange(boolean active) throws android.os.RemoteException;
  public void notifyUserMobileDataStateChangedForPhoneId(int phoneId, int subId, boolean state) throws android.os.RemoteException;
  public void notifyPhoneCapabilityChanged(android.telephony.PhoneCapability capability) throws android.os.RemoteException;
  public void notifyActiveDataSubIdChanged(int activeDataSubId) throws android.os.RemoteException;
  public void notifyRadioPowerStateChanged(int phoneId, int subId, int state) throws android.os.RemoteException;
  public void notifyEmergencyNumberList(int phoneId, int subId) throws android.os.RemoteException;
  public void notifyCallQualityChanged(android.telephony.CallQuality callQuality, int phoneId, int subId, int callNetworkType) throws android.os.RemoteException;
  public void notifyImsDisconnectCause(int subId, android.telephony.ims.ImsReasonInfo imsReasonInfo) throws android.os.RemoteException;
}
