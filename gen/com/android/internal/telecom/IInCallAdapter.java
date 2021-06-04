/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.telecom;
/**
 * Internal remote callback interface for in-call services.
 *
 * @see android.telecom.InCallAdapter
 *
 * {@hide}
 */
public interface IInCallAdapter extends android.os.IInterface
{
  /** Default implementation for IInCallAdapter. */
  public static class Default implements com.android.internal.telecom.IInCallAdapter
  {
    @Override public void answerCall(java.lang.String callId, int videoState) throws android.os.RemoteException
    {
    }
    @Override public void deflectCall(java.lang.String callId, android.net.Uri address) throws android.os.RemoteException
    {
    }
    @Override public void rejectCall(java.lang.String callId, boolean rejectWithMessage, java.lang.String textMessage) throws android.os.RemoteException
    {
    }
    @Override public void disconnectCall(java.lang.String callId) throws android.os.RemoteException
    {
    }
    @Override public void holdCall(java.lang.String callId) throws android.os.RemoteException
    {
    }
    @Override public void unholdCall(java.lang.String callId) throws android.os.RemoteException
    {
    }
    @Override public void mute(boolean shouldMute) throws android.os.RemoteException
    {
    }
    @Override public void setAudioRoute(int route, java.lang.String bluetoothAddress) throws android.os.RemoteException
    {
    }
    @Override public void playDtmfTone(java.lang.String callId, char digit) throws android.os.RemoteException
    {
    }
    @Override public void stopDtmfTone(java.lang.String callId) throws android.os.RemoteException
    {
    }
    @Override public void postDialContinue(java.lang.String callId, boolean proceed) throws android.os.RemoteException
    {
    }
    @Override public void phoneAccountSelected(java.lang.String callId, android.telecom.PhoneAccountHandle accountHandle, boolean setDefault) throws android.os.RemoteException
    {
    }
    @Override public void conference(java.lang.String callId, java.lang.String otherCallId) throws android.os.RemoteException
    {
    }
    @Override public void splitFromConference(java.lang.String callId) throws android.os.RemoteException
    {
    }
    @Override public void mergeConference(java.lang.String callId) throws android.os.RemoteException
    {
    }
    @Override public void swapConference(java.lang.String callId) throws android.os.RemoteException
    {
    }
    @Override public void turnOnProximitySensor() throws android.os.RemoteException
    {
    }
    @Override public void turnOffProximitySensor(boolean screenOnImmediately) throws android.os.RemoteException
    {
    }
    @Override public void pullExternalCall(java.lang.String callId) throws android.os.RemoteException
    {
    }
    @Override public void sendCallEvent(java.lang.String callId, java.lang.String event, int targetSdkVer, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    @Override public void putExtras(java.lang.String callId, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    @Override public void removeExtras(java.lang.String callId, java.util.List<java.lang.String> keys) throws android.os.RemoteException
    {
    }
    @Override public void sendRttRequest(java.lang.String callId) throws android.os.RemoteException
    {
    }
    @Override public void respondToRttRequest(java.lang.String callId, int id, boolean accept) throws android.os.RemoteException
    {
    }
    @Override public void stopRtt(java.lang.String callId) throws android.os.RemoteException
    {
    }
    @Override public void setRttMode(java.lang.String callId, int mode) throws android.os.RemoteException
    {
    }
    @Override public void handoverTo(java.lang.String callId, android.telecom.PhoneAccountHandle destAcct, int videoState, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.telecom.IInCallAdapter
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.telecom.IInCallAdapter";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.telecom.IInCallAdapter interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.telecom.IInCallAdapter asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.telecom.IInCallAdapter))) {
        return ((com.android.internal.telecom.IInCallAdapter)iin);
      }
      return new com.android.internal.telecom.IInCallAdapter.Stub.Proxy(obj);
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
        case TRANSACTION_answerCall:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.answerCall(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_deflectCall:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.net.Uri _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.deflectCall(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_rejectCall:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.rejectCall(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_disconnectCall:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.disconnectCall(_arg0);
          return true;
        }
        case TRANSACTION_holdCall:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.holdCall(_arg0);
          return true;
        }
        case TRANSACTION_unholdCall:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.unholdCall(_arg0);
          return true;
        }
        case TRANSACTION_mute:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.mute(_arg0);
          return true;
        }
        case TRANSACTION_setAudioRoute:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.setAudioRoute(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_playDtmfTone:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          char _arg1;
          _arg1 = (char)data.readInt();
          this.playDtmfTone(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_stopDtmfTone:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.stopDtmfTone(_arg0);
          return true;
        }
        case TRANSACTION_postDialContinue:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.postDialContinue(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_phoneAccountSelected:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.PhoneAccountHandle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.phoneAccountSelected(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_conference:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.conference(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_splitFromConference:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.splitFromConference(_arg0);
          return true;
        }
        case TRANSACTION_mergeConference:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.mergeConference(_arg0);
          return true;
        }
        case TRANSACTION_swapConference:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.swapConference(_arg0);
          return true;
        }
        case TRANSACTION_turnOnProximitySensor:
        {
          data.enforceInterface(descriptor);
          this.turnOnProximitySensor();
          return true;
        }
        case TRANSACTION_turnOffProximitySensor:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.turnOffProximitySensor(_arg0);
          return true;
        }
        case TRANSACTION_pullExternalCall:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.pullExternalCall(_arg0);
          return true;
        }
        case TRANSACTION_sendCallEvent:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          android.os.Bundle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.sendCallEvent(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_putExtras:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.putExtras(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_removeExtras:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.List<java.lang.String> _arg1;
          _arg1 = data.createStringArrayList();
          this.removeExtras(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_sendRttRequest:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.sendRttRequest(_arg0);
          return true;
        }
        case TRANSACTION_respondToRttRequest:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.respondToRttRequest(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_stopRtt:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.stopRtt(_arg0);
          return true;
        }
        case TRANSACTION_setRttMode:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.setRttMode(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_handoverTo:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.PhoneAccountHandle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          android.os.Bundle _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.handoverTo(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.telecom.IInCallAdapter
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
      @Override public void answerCall(java.lang.String callId, int videoState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeInt(videoState);
          boolean _status = mRemote.transact(Stub.TRANSACTION_answerCall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().answerCall(callId, videoState);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void deflectCall(java.lang.String callId, android.net.Uri address) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((address!=null)) {
            _data.writeInt(1);
            address.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_deflectCall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deflectCall(callId, address);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void rejectCall(java.lang.String callId, boolean rejectWithMessage, java.lang.String textMessage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeInt(((rejectWithMessage)?(1):(0)));
          _data.writeString(textMessage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_rejectCall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().rejectCall(callId, rejectWithMessage, textMessage);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void disconnectCall(java.lang.String callId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disconnectCall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disconnectCall(callId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void holdCall(java.lang.String callId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_holdCall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().holdCall(callId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void unholdCall(java.lang.String callId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unholdCall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unholdCall(callId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void mute(boolean shouldMute) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((shouldMute)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_mute, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().mute(shouldMute);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setAudioRoute(int route, java.lang.String bluetoothAddress) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(route);
          _data.writeString(bluetoothAddress);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAudioRoute, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAudioRoute(route, bluetoothAddress);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void playDtmfTone(java.lang.String callId, char digit) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeInt(((int)digit));
          boolean _status = mRemote.transact(Stub.TRANSACTION_playDtmfTone, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().playDtmfTone(callId, digit);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void stopDtmfTone(java.lang.String callId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopDtmfTone, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopDtmfTone(callId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void postDialContinue(java.lang.String callId, boolean proceed) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeInt(((proceed)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_postDialContinue, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().postDialContinue(callId, proceed);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void phoneAccountSelected(java.lang.String callId, android.telecom.PhoneAccountHandle accountHandle, boolean setDefault) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((accountHandle!=null)) {
            _data.writeInt(1);
            accountHandle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((setDefault)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_phoneAccountSelected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().phoneAccountSelected(callId, accountHandle, setDefault);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void conference(java.lang.String callId, java.lang.String otherCallId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeString(otherCallId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_conference, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().conference(callId, otherCallId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void splitFromConference(java.lang.String callId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_splitFromConference, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().splitFromConference(callId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void mergeConference(java.lang.String callId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_mergeConference, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().mergeConference(callId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void swapConference(java.lang.String callId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_swapConference, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().swapConference(callId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void turnOnProximitySensor() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_turnOnProximitySensor, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().turnOnProximitySensor();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void turnOffProximitySensor(boolean screenOnImmediately) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((screenOnImmediately)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_turnOffProximitySensor, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().turnOffProximitySensor(screenOnImmediately);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void pullExternalCall(java.lang.String callId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_pullExternalCall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().pullExternalCall(callId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void sendCallEvent(java.lang.String callId, java.lang.String event, int targetSdkVer, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeString(event);
          _data.writeInt(targetSdkVer);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendCallEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendCallEvent(callId, event, targetSdkVer, extras);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void putExtras(java.lang.String callId, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_putExtras, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().putExtras(callId, extras);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void removeExtras(java.lang.String callId, java.util.List<java.lang.String> keys) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeStringList(keys);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeExtras, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeExtras(callId, keys);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void sendRttRequest(java.lang.String callId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendRttRequest, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendRttRequest(callId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void respondToRttRequest(java.lang.String callId, int id, boolean accept) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeInt(id);
          _data.writeInt(((accept)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_respondToRttRequest, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().respondToRttRequest(callId, id, accept);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void stopRtt(java.lang.String callId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopRtt, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopRtt(callId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setRttMode(java.lang.String callId, int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRttMode, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setRttMode(callId, mode);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void handoverTo(java.lang.String callId, android.telecom.PhoneAccountHandle destAcct, int videoState, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((destAcct!=null)) {
            _data.writeInt(1);
            destAcct.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(videoState);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_handoverTo, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().handoverTo(callId, destAcct, videoState, extras);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.telecom.IInCallAdapter sDefaultImpl;
    }
    static final int TRANSACTION_answerCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_deflectCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_rejectCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_disconnectCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_holdCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_unholdCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_mute = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_setAudioRoute = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_playDtmfTone = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_stopDtmfTone = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_postDialContinue = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_phoneAccountSelected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_conference = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_splitFromConference = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_mergeConference = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_swapConference = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_turnOnProximitySensor = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_turnOffProximitySensor = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_pullExternalCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_sendCallEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_putExtras = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_removeExtras = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_sendRttRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_respondToRttRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_stopRtt = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_setRttMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_handoverTo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    public static boolean setDefaultImpl(com.android.internal.telecom.IInCallAdapter impl) {
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
    public static com.android.internal.telecom.IInCallAdapter getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void answerCall(java.lang.String callId, int videoState) throws android.os.RemoteException;
  public void deflectCall(java.lang.String callId, android.net.Uri address) throws android.os.RemoteException;
  public void rejectCall(java.lang.String callId, boolean rejectWithMessage, java.lang.String textMessage) throws android.os.RemoteException;
  public void disconnectCall(java.lang.String callId) throws android.os.RemoteException;
  public void holdCall(java.lang.String callId) throws android.os.RemoteException;
  public void unholdCall(java.lang.String callId) throws android.os.RemoteException;
  public void mute(boolean shouldMute) throws android.os.RemoteException;
  public void setAudioRoute(int route, java.lang.String bluetoothAddress) throws android.os.RemoteException;
  public void playDtmfTone(java.lang.String callId, char digit) throws android.os.RemoteException;
  public void stopDtmfTone(java.lang.String callId) throws android.os.RemoteException;
  public void postDialContinue(java.lang.String callId, boolean proceed) throws android.os.RemoteException;
  public void phoneAccountSelected(java.lang.String callId, android.telecom.PhoneAccountHandle accountHandle, boolean setDefault) throws android.os.RemoteException;
  public void conference(java.lang.String callId, java.lang.String otherCallId) throws android.os.RemoteException;
  public void splitFromConference(java.lang.String callId) throws android.os.RemoteException;
  public void mergeConference(java.lang.String callId) throws android.os.RemoteException;
  public void swapConference(java.lang.String callId) throws android.os.RemoteException;
  public void turnOnProximitySensor() throws android.os.RemoteException;
  public void turnOffProximitySensor(boolean screenOnImmediately) throws android.os.RemoteException;
  public void pullExternalCall(java.lang.String callId) throws android.os.RemoteException;
  public void sendCallEvent(java.lang.String callId, java.lang.String event, int targetSdkVer, android.os.Bundle extras) throws android.os.RemoteException;
  public void putExtras(java.lang.String callId, android.os.Bundle extras) throws android.os.RemoteException;
  public void removeExtras(java.lang.String callId, java.util.List<java.lang.String> keys) throws android.os.RemoteException;
  public void sendRttRequest(java.lang.String callId) throws android.os.RemoteException;
  public void respondToRttRequest(java.lang.String callId, int id, boolean accept) throws android.os.RemoteException;
  public void stopRtt(java.lang.String callId) throws android.os.RemoteException;
  public void setRttMode(java.lang.String callId, int mode) throws android.os.RemoteException;
  public void handoverTo(java.lang.String callId, android.telecom.PhoneAccountHandle destAcct, int videoState, android.os.Bundle extras) throws android.os.RemoteException;
}
