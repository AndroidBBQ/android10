/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.telephony;
public interface IPhoneStateListener extends android.os.IInterface
{
  /** Default implementation for IPhoneStateListener. */
  public static class Default implements com.android.internal.telephony.IPhoneStateListener
  {
    @Override public void onServiceStateChanged(android.telephony.ServiceState serviceState) throws android.os.RemoteException
    {
    }
    @Override public void onSignalStrengthChanged(int asu) throws android.os.RemoteException
    {
    }
    @Override public void onMessageWaitingIndicatorChanged(boolean mwi) throws android.os.RemoteException
    {
    }
    @Override public void onCallForwardingIndicatorChanged(boolean cfi) throws android.os.RemoteException
    {
    }
    // we use bundle here instead of CellLocation so it can get the right subclass

    @Override public void onCellLocationChanged(android.os.Bundle location) throws android.os.RemoteException
    {
    }
    @Override public void onCallStateChanged(int state, java.lang.String incomingNumber) throws android.os.RemoteException
    {
    }
    @Override public void onDataConnectionStateChanged(int state, int networkType) throws android.os.RemoteException
    {
    }
    @Override public void onDataActivity(int direction) throws android.os.RemoteException
    {
    }
    @Override public void onSignalStrengthsChanged(android.telephony.SignalStrength signalStrength) throws android.os.RemoteException
    {
    }
    @Override public void onPhysicalChannelConfigurationChanged(java.util.List<android.telephony.PhysicalChannelConfig> configs) throws android.os.RemoteException
    {
    }
    @Override public void onOtaspChanged(int otaspMode) throws android.os.RemoteException
    {
    }
    @Override public void onCellInfoChanged(java.util.List<android.telephony.CellInfo> cellInfo) throws android.os.RemoteException
    {
    }
    @Override public void onPreciseCallStateChanged(android.telephony.PreciseCallState callState) throws android.os.RemoteException
    {
    }
    @Override public void onPreciseDataConnectionStateChanged(android.telephony.PreciseDataConnectionState dataConnectionState) throws android.os.RemoteException
    {
    }
    @Override public void onDataConnectionRealTimeInfoChanged(android.telephony.DataConnectionRealTimeInfo dcRtInfo) throws android.os.RemoteException
    {
    }
    @Override public void onSrvccStateChanged(int state) throws android.os.RemoteException
    {
    }
    @Override public void onVoiceActivationStateChanged(int activationState) throws android.os.RemoteException
    {
    }
    @Override public void onDataActivationStateChanged(int activationState) throws android.os.RemoteException
    {
    }
    @Override public void onOemHookRawEvent(byte[] rawData) throws android.os.RemoteException
    {
    }
    @Override public void onCarrierNetworkChange(boolean active) throws android.os.RemoteException
    {
    }
    @Override public void onUserMobileDataStateChanged(boolean enabled) throws android.os.RemoteException
    {
    }
    @Override public void onPhoneCapabilityChanged(android.telephony.PhoneCapability capability) throws android.os.RemoteException
    {
    }
    @Override public void onActiveDataSubIdChanged(int subId) throws android.os.RemoteException
    {
    }
    @Override public void onRadioPowerStateChanged(int state) throws android.os.RemoteException
    {
    }
    @Override public void onCallAttributesChanged(android.telephony.CallAttributes callAttributes) throws android.os.RemoteException
    {
    }
    @Override public void onEmergencyNumberListChanged(java.util.Map emergencyNumberList) throws android.os.RemoteException
    {
    }
    @Override public void onCallDisconnectCauseChanged(int disconnectCause, int preciseDisconnectCause) throws android.os.RemoteException
    {
    }
    @Override public void onImsCallDisconnectCauseChanged(android.telephony.ims.ImsReasonInfo imsReasonInfo) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.telephony.IPhoneStateListener
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.telephony.IPhoneStateListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.telephony.IPhoneStateListener interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.telephony.IPhoneStateListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.telephony.IPhoneStateListener))) {
        return ((com.android.internal.telephony.IPhoneStateListener)iin);
      }
      return new com.android.internal.telephony.IPhoneStateListener.Stub.Proxy(obj);
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
        case TRANSACTION_onServiceStateChanged:
        {
          data.enforceInterface(descriptor);
          android.telephony.ServiceState _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ServiceState.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onServiceStateChanged(_arg0);
          return true;
        }
        case TRANSACTION_onSignalStrengthChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onSignalStrengthChanged(_arg0);
          return true;
        }
        case TRANSACTION_onMessageWaitingIndicatorChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onMessageWaitingIndicatorChanged(_arg0);
          return true;
        }
        case TRANSACTION_onCallForwardingIndicatorChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onCallForwardingIndicatorChanged(_arg0);
          return true;
        }
        case TRANSACTION_onCellLocationChanged:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onCellLocationChanged(_arg0);
          return true;
        }
        case TRANSACTION_onCallStateChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.onCallStateChanged(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onDataConnectionStateChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.onDataConnectionStateChanged(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onDataActivity:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onDataActivity(_arg0);
          return true;
        }
        case TRANSACTION_onSignalStrengthsChanged:
        {
          data.enforceInterface(descriptor);
          android.telephony.SignalStrength _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.SignalStrength.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onSignalStrengthsChanged(_arg0);
          return true;
        }
        case TRANSACTION_onPhysicalChannelConfigurationChanged:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.telephony.PhysicalChannelConfig> _arg0;
          _arg0 = data.createTypedArrayList(android.telephony.PhysicalChannelConfig.CREATOR);
          this.onPhysicalChannelConfigurationChanged(_arg0);
          return true;
        }
        case TRANSACTION_onOtaspChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onOtaspChanged(_arg0);
          return true;
        }
        case TRANSACTION_onCellInfoChanged:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.telephony.CellInfo> _arg0;
          _arg0 = data.createTypedArrayList(android.telephony.CellInfo.CREATOR);
          this.onCellInfoChanged(_arg0);
          return true;
        }
        case TRANSACTION_onPreciseCallStateChanged:
        {
          data.enforceInterface(descriptor);
          android.telephony.PreciseCallState _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.PreciseCallState.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onPreciseCallStateChanged(_arg0);
          return true;
        }
        case TRANSACTION_onPreciseDataConnectionStateChanged:
        {
          data.enforceInterface(descriptor);
          android.telephony.PreciseDataConnectionState _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.PreciseDataConnectionState.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onPreciseDataConnectionStateChanged(_arg0);
          return true;
        }
        case TRANSACTION_onDataConnectionRealTimeInfoChanged:
        {
          data.enforceInterface(descriptor);
          android.telephony.DataConnectionRealTimeInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.DataConnectionRealTimeInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onDataConnectionRealTimeInfoChanged(_arg0);
          return true;
        }
        case TRANSACTION_onSrvccStateChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onSrvccStateChanged(_arg0);
          return true;
        }
        case TRANSACTION_onVoiceActivationStateChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onVoiceActivationStateChanged(_arg0);
          return true;
        }
        case TRANSACTION_onDataActivationStateChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onDataActivationStateChanged(_arg0);
          return true;
        }
        case TRANSACTION_onOemHookRawEvent:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          this.onOemHookRawEvent(_arg0);
          return true;
        }
        case TRANSACTION_onCarrierNetworkChange:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onCarrierNetworkChange(_arg0);
          return true;
        }
        case TRANSACTION_onUserMobileDataStateChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onUserMobileDataStateChanged(_arg0);
          return true;
        }
        case TRANSACTION_onPhoneCapabilityChanged:
        {
          data.enforceInterface(descriptor);
          android.telephony.PhoneCapability _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.PhoneCapability.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onPhoneCapabilityChanged(_arg0);
          return true;
        }
        case TRANSACTION_onActiveDataSubIdChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onActiveDataSubIdChanged(_arg0);
          return true;
        }
        case TRANSACTION_onRadioPowerStateChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onRadioPowerStateChanged(_arg0);
          return true;
        }
        case TRANSACTION_onCallAttributesChanged:
        {
          data.enforceInterface(descriptor);
          android.telephony.CallAttributes _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.CallAttributes.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onCallAttributesChanged(_arg0);
          return true;
        }
        case TRANSACTION_onEmergencyNumberListChanged:
        {
          data.enforceInterface(descriptor);
          java.util.Map _arg0;
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _arg0 = data.readHashMap(cl);
          this.onEmergencyNumberListChanged(_arg0);
          return true;
        }
        case TRANSACTION_onCallDisconnectCauseChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.onCallDisconnectCauseChanged(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onImsCallDisconnectCauseChanged:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsReasonInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsReasonInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onImsCallDisconnectCauseChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.telephony.IPhoneStateListener
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
      @Override public void onServiceStateChanged(android.telephony.ServiceState serviceState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((serviceState!=null)) {
            _data.writeInt(1);
            serviceState.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onServiceStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onServiceStateChanged(serviceState);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSignalStrengthChanged(int asu) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(asu);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSignalStrengthChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSignalStrengthChanged(asu);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onMessageWaitingIndicatorChanged(boolean mwi) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((mwi)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onMessageWaitingIndicatorChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onMessageWaitingIndicatorChanged(mwi);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onCallForwardingIndicatorChanged(boolean cfi) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((cfi)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCallForwardingIndicatorChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCallForwardingIndicatorChanged(cfi);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // we use bundle here instead of CellLocation so it can get the right subclass

      @Override public void onCellLocationChanged(android.os.Bundle location) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((location!=null)) {
            _data.writeInt(1);
            location.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCellLocationChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCellLocationChanged(location);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onCallStateChanged(int state, java.lang.String incomingNumber) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(state);
          _data.writeString(incomingNumber);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCallStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCallStateChanged(state, incomingNumber);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDataConnectionStateChanged(int state, int networkType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(state);
          _data.writeInt(networkType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDataConnectionStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDataConnectionStateChanged(state, networkType);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDataActivity(int direction) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(direction);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDataActivity, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDataActivity(direction);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSignalStrengthsChanged(android.telephony.SignalStrength signalStrength) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((signalStrength!=null)) {
            _data.writeInt(1);
            signalStrength.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSignalStrengthsChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSignalStrengthsChanged(signalStrength);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPhysicalChannelConfigurationChanged(java.util.List<android.telephony.PhysicalChannelConfig> configs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(configs);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPhysicalChannelConfigurationChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPhysicalChannelConfigurationChanged(configs);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onOtaspChanged(int otaspMode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(otaspMode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onOtaspChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onOtaspChanged(otaspMode);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onCellInfoChanged(java.util.List<android.telephony.CellInfo> cellInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(cellInfo);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCellInfoChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCellInfoChanged(cellInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPreciseCallStateChanged(android.telephony.PreciseCallState callState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((callState!=null)) {
            _data.writeInt(1);
            callState.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPreciseCallStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPreciseCallStateChanged(callState);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPreciseDataConnectionStateChanged(android.telephony.PreciseDataConnectionState dataConnectionState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((dataConnectionState!=null)) {
            _data.writeInt(1);
            dataConnectionState.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPreciseDataConnectionStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPreciseDataConnectionStateChanged(dataConnectionState);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDataConnectionRealTimeInfoChanged(android.telephony.DataConnectionRealTimeInfo dcRtInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((dcRtInfo!=null)) {
            _data.writeInt(1);
            dcRtInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDataConnectionRealTimeInfoChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDataConnectionRealTimeInfoChanged(dcRtInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onSrvccStateChanged(int state) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(state);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSrvccStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSrvccStateChanged(state);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onVoiceActivationStateChanged(int activationState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(activationState);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onVoiceActivationStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onVoiceActivationStateChanged(activationState);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDataActivationStateChanged(int activationState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(activationState);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDataActivationStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDataActivationStateChanged(activationState);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onOemHookRawEvent(byte[] rawData) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(rawData);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onOemHookRawEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onOemHookRawEvent(rawData);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onCarrierNetworkChange(boolean active) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((active)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCarrierNetworkChange, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCarrierNetworkChange(active);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onUserMobileDataStateChanged(boolean enabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onUserMobileDataStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onUserMobileDataStateChanged(enabled);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPhoneCapabilityChanged(android.telephony.PhoneCapability capability) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((capability!=null)) {
            _data.writeInt(1);
            capability.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPhoneCapabilityChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPhoneCapabilityChanged(capability);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onActiveDataSubIdChanged(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onActiveDataSubIdChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onActiveDataSubIdChanged(subId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onRadioPowerStateChanged(int state) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(state);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRadioPowerStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRadioPowerStateChanged(state);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onCallAttributesChanged(android.telephony.CallAttributes callAttributes) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((callAttributes!=null)) {
            _data.writeInt(1);
            callAttributes.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCallAttributesChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCallAttributesChanged(callAttributes);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onEmergencyNumberListChanged(java.util.Map emergencyNumberList) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeMap(emergencyNumberList);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEmergencyNumberListChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEmergencyNumberListChanged(emergencyNumberList);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onCallDisconnectCauseChanged(int disconnectCause, int preciseDisconnectCause) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(disconnectCause);
          _data.writeInt(preciseDisconnectCause);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCallDisconnectCauseChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCallDisconnectCauseChanged(disconnectCause, preciseDisconnectCause);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onImsCallDisconnectCauseChanged(android.telephony.ims.ImsReasonInfo imsReasonInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((imsReasonInfo!=null)) {
            _data.writeInt(1);
            imsReasonInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onImsCallDisconnectCauseChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onImsCallDisconnectCauseChanged(imsReasonInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.telephony.IPhoneStateListener sDefaultImpl;
    }
    static final int TRANSACTION_onServiceStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onSignalStrengthChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onMessageWaitingIndicatorChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onCallForwardingIndicatorChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onCellLocationChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onCallStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_onDataConnectionStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_onDataActivity = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_onSignalStrengthsChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_onPhysicalChannelConfigurationChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_onOtaspChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_onCellInfoChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_onPreciseCallStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_onPreciseDataConnectionStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_onDataConnectionRealTimeInfoChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_onSrvccStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_onVoiceActivationStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_onDataActivationStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_onOemHookRawEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_onCarrierNetworkChange = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_onUserMobileDataStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_onPhoneCapabilityChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_onActiveDataSubIdChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_onRadioPowerStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_onCallAttributesChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_onEmergencyNumberListChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_onCallDisconnectCauseChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_onImsCallDisconnectCauseChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    public static boolean setDefaultImpl(com.android.internal.telephony.IPhoneStateListener impl) {
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
    public static com.android.internal.telephony.IPhoneStateListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onServiceStateChanged(android.telephony.ServiceState serviceState) throws android.os.RemoteException;
  public void onSignalStrengthChanged(int asu) throws android.os.RemoteException;
  public void onMessageWaitingIndicatorChanged(boolean mwi) throws android.os.RemoteException;
  public void onCallForwardingIndicatorChanged(boolean cfi) throws android.os.RemoteException;
  // we use bundle here instead of CellLocation so it can get the right subclass

  public void onCellLocationChanged(android.os.Bundle location) throws android.os.RemoteException;
  public void onCallStateChanged(int state, java.lang.String incomingNumber) throws android.os.RemoteException;
  public void onDataConnectionStateChanged(int state, int networkType) throws android.os.RemoteException;
  public void onDataActivity(int direction) throws android.os.RemoteException;
  public void onSignalStrengthsChanged(android.telephony.SignalStrength signalStrength) throws android.os.RemoteException;
  public void onPhysicalChannelConfigurationChanged(java.util.List<android.telephony.PhysicalChannelConfig> configs) throws android.os.RemoteException;
  public void onOtaspChanged(int otaspMode) throws android.os.RemoteException;
  public void onCellInfoChanged(java.util.List<android.telephony.CellInfo> cellInfo) throws android.os.RemoteException;
  public void onPreciseCallStateChanged(android.telephony.PreciseCallState callState) throws android.os.RemoteException;
  public void onPreciseDataConnectionStateChanged(android.telephony.PreciseDataConnectionState dataConnectionState) throws android.os.RemoteException;
  public void onDataConnectionRealTimeInfoChanged(android.telephony.DataConnectionRealTimeInfo dcRtInfo) throws android.os.RemoteException;
  public void onSrvccStateChanged(int state) throws android.os.RemoteException;
  public void onVoiceActivationStateChanged(int activationState) throws android.os.RemoteException;
  public void onDataActivationStateChanged(int activationState) throws android.os.RemoteException;
  public void onOemHookRawEvent(byte[] rawData) throws android.os.RemoteException;
  public void onCarrierNetworkChange(boolean active) throws android.os.RemoteException;
  public void onUserMobileDataStateChanged(boolean enabled) throws android.os.RemoteException;
  public void onPhoneCapabilityChanged(android.telephony.PhoneCapability capability) throws android.os.RemoteException;
  public void onActiveDataSubIdChanged(int subId) throws android.os.RemoteException;
  public void onRadioPowerStateChanged(int state) throws android.os.RemoteException;
  public void onCallAttributesChanged(android.telephony.CallAttributes callAttributes) throws android.os.RemoteException;
  public void onEmergencyNumberListChanged(java.util.Map emergencyNumberList) throws android.os.RemoteException;
  public void onCallDisconnectCauseChanged(int disconnectCause, int preciseDisconnectCause) throws android.os.RemoteException;
  public void onImsCallDisconnectCauseChanged(android.telephony.ims.ImsReasonInfo imsReasonInfo) throws android.os.RemoteException;
}
