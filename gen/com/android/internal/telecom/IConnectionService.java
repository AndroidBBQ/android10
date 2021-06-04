/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.telecom;
/**
 * Internal remote interface for connection services.
 *
 * @see android.telecom.ConnectionService
 *
 * @hide
 */
public interface IConnectionService extends android.os.IInterface
{
  /** Default implementation for IConnectionService. */
  public static class Default implements com.android.internal.telecom.IConnectionService
  {
    @Override public void addConnectionServiceAdapter(com.android.internal.telecom.IConnectionServiceAdapter adapter, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void removeConnectionServiceAdapter(com.android.internal.telecom.IConnectionServiceAdapter adapter, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void createConnection(android.telecom.PhoneAccountHandle connectionManagerPhoneAccount, java.lang.String callId, android.telecom.ConnectionRequest request, boolean isIncoming, boolean isUnknown, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void createConnectionComplete(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void createConnectionFailed(android.telecom.PhoneAccountHandle connectionManagerPhoneAccount, java.lang.String callId, android.telecom.ConnectionRequest request, boolean isIncoming, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void abort(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void answerVideo(java.lang.String callId, int videoState, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void answer(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void deflect(java.lang.String callId, android.net.Uri address, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void reject(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void rejectWithMessage(java.lang.String callId, java.lang.String message, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void disconnect(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void silence(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void hold(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void unhold(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void onCallAudioStateChanged(java.lang.String activeCallId, android.telecom.CallAudioState callAudioState, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void playDtmfTone(java.lang.String callId, char digit, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void stopDtmfTone(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void conference(java.lang.String conferenceCallId, java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void splitFromConference(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void mergeConference(java.lang.String conferenceCallId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void swapConference(java.lang.String conferenceCallId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void onPostDialContinue(java.lang.String callId, boolean proceed, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void pullExternalCall(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void sendCallEvent(java.lang.String callId, java.lang.String event, android.os.Bundle extras, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void onExtrasChanged(java.lang.String callId, android.os.Bundle extras, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void startRtt(java.lang.String callId, android.os.ParcelFileDescriptor fromInCall, android.os.ParcelFileDescriptor toInCall, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void stopRtt(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void respondToRttUpgradeRequest(java.lang.String callId, android.os.ParcelFileDescriptor fromInCall, android.os.ParcelFileDescriptor toInCall, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void connectionServiceFocusLost(android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void connectionServiceFocusGained(android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void handoverFailed(java.lang.String callId, android.telecom.ConnectionRequest request, int error, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void handoverComplete(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.telecom.IConnectionService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.telecom.IConnectionService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.telecom.IConnectionService interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.telecom.IConnectionService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.telecom.IConnectionService))) {
        return ((com.android.internal.telecom.IConnectionService)iin);
      }
      return new com.android.internal.telecom.IConnectionService.Stub.Proxy(obj);
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
        case TRANSACTION_addConnectionServiceAdapter:
        {
          data.enforceInterface(descriptor);
          com.android.internal.telecom.IConnectionServiceAdapter _arg0;
          _arg0 = com.android.internal.telecom.IConnectionServiceAdapter.Stub.asInterface(data.readStrongBinder());
          android.telecom.Logging.Session.Info _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.addConnectionServiceAdapter(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_removeConnectionServiceAdapter:
        {
          data.enforceInterface(descriptor);
          com.android.internal.telecom.IConnectionServiceAdapter _arg0;
          _arg0 = com.android.internal.telecom.IConnectionServiceAdapter.Stub.asInterface(data.readStrongBinder());
          android.telecom.Logging.Session.Info _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.removeConnectionServiceAdapter(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_createConnection:
        {
          data.enforceInterface(descriptor);
          android.telecom.PhoneAccountHandle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.telecom.ConnectionRequest _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telecom.ConnectionRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          android.telecom.Logging.Session.Info _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          this.createConnection(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          return true;
        }
        case TRANSACTION_createConnectionComplete:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.Logging.Session.Info _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.createConnectionComplete(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_createConnectionFailed:
        {
          data.enforceInterface(descriptor);
          android.telecom.PhoneAccountHandle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.telecom.ConnectionRequest _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telecom.ConnectionRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          android.telecom.Logging.Session.Info _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          this.createConnectionFailed(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_abort:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.Logging.Session.Info _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.abort(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_answerVideo:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.telecom.Logging.Session.Info _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.answerVideo(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_answer:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.Logging.Session.Info _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.answer(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_deflect:
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
          android.telecom.Logging.Session.Info _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.deflect(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_reject:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.Logging.Session.Info _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.reject(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_rejectWithMessage:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.telecom.Logging.Session.Info _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.rejectWithMessage(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_disconnect:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.Logging.Session.Info _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.disconnect(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_silence:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.Logging.Session.Info _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.silence(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_hold:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.Logging.Session.Info _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.hold(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_unhold:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.Logging.Session.Info _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.unhold(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onCallAudioStateChanged:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.CallAudioState _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.CallAudioState.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.telecom.Logging.Session.Info _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.onCallAudioStateChanged(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_playDtmfTone:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          char _arg1;
          _arg1 = (char)data.readInt();
          android.telecom.Logging.Session.Info _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.playDtmfTone(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_stopDtmfTone:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.Logging.Session.Info _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.stopDtmfTone(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_conference:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.telecom.Logging.Session.Info _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.conference(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_splitFromConference:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.Logging.Session.Info _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.splitFromConference(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_mergeConference:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.Logging.Session.Info _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.mergeConference(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_swapConference:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.Logging.Session.Info _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.swapConference(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onPostDialContinue:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          android.telecom.Logging.Session.Info _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.onPostDialContinue(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_pullExternalCall:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.Logging.Session.Info _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.pullExternalCall(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_sendCallEvent:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.telecom.Logging.Session.Info _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.sendCallEvent(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onExtrasChanged:
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
          android.telecom.Logging.Session.Info _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.onExtrasChanged(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_startRtt:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.ParcelFileDescriptor _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.os.ParcelFileDescriptor _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.telecom.Logging.Session.Info _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.startRtt(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_stopRtt:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.Logging.Session.Info _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.stopRtt(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_respondToRttUpgradeRequest:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.ParcelFileDescriptor _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.os.ParcelFileDescriptor _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.telecom.Logging.Session.Info _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.respondToRttUpgradeRequest(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_connectionServiceFocusLost:
        {
          data.enforceInterface(descriptor);
          android.telecom.Logging.Session.Info _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.connectionServiceFocusLost(_arg0);
          return true;
        }
        case TRANSACTION_connectionServiceFocusGained:
        {
          data.enforceInterface(descriptor);
          android.telecom.Logging.Session.Info _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.connectionServiceFocusGained(_arg0);
          return true;
        }
        case TRANSACTION_handoverFailed:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.ConnectionRequest _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.ConnectionRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          android.telecom.Logging.Session.Info _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.handoverFailed(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_handoverComplete:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.Logging.Session.Info _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.handoverComplete(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.telecom.IConnectionService
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
      @Override public void addConnectionServiceAdapter(com.android.internal.telecom.IConnectionServiceAdapter adapter, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((adapter!=null))?(adapter.asBinder()):(null)));
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_addConnectionServiceAdapter, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addConnectionServiceAdapter(adapter, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void removeConnectionServiceAdapter(com.android.internal.telecom.IConnectionServiceAdapter adapter, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((adapter!=null))?(adapter.asBinder()):(null)));
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeConnectionServiceAdapter, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeConnectionServiceAdapter(adapter, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void createConnection(android.telecom.PhoneAccountHandle connectionManagerPhoneAccount, java.lang.String callId, android.telecom.ConnectionRequest request, boolean isIncoming, boolean isUnknown, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((connectionManagerPhoneAccount!=null)) {
            _data.writeInt(1);
            connectionManagerPhoneAccount.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callId);
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((isIncoming)?(1):(0)));
          _data.writeInt(((isUnknown)?(1):(0)));
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_createConnection, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().createConnection(connectionManagerPhoneAccount, callId, request, isIncoming, isUnknown, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void createConnectionComplete(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_createConnectionComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().createConnectionComplete(callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void createConnectionFailed(android.telecom.PhoneAccountHandle connectionManagerPhoneAccount, java.lang.String callId, android.telecom.ConnectionRequest request, boolean isIncoming, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((connectionManagerPhoneAccount!=null)) {
            _data.writeInt(1);
            connectionManagerPhoneAccount.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callId);
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((isIncoming)?(1):(0)));
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_createConnectionFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().createConnectionFailed(connectionManagerPhoneAccount, callId, request, isIncoming, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void abort(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_abort, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().abort(callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void answerVideo(java.lang.String callId, int videoState, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeInt(videoState);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_answerVideo, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().answerVideo(callId, videoState, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void answer(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_answer, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().answer(callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void deflect(java.lang.String callId, android.net.Uri address, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
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
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_deflect, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deflect(callId, address, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void reject(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_reject, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reject(callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void rejectWithMessage(java.lang.String callId, java.lang.String message, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeString(message);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_rejectWithMessage, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().rejectWithMessage(callId, message, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void disconnect(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_disconnect, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disconnect(callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void silence(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_silence, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().silence(callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void hold(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_hold, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().hold(callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void unhold(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_unhold, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unhold(callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onCallAudioStateChanged(java.lang.String activeCallId, android.telecom.CallAudioState callAudioState, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(activeCallId);
          if ((callAudioState!=null)) {
            _data.writeInt(1);
            callAudioState.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCallAudioStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCallAudioStateChanged(activeCallId, callAudioState, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void playDtmfTone(java.lang.String callId, char digit, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeInt(((int)digit));
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_playDtmfTone, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().playDtmfTone(callId, digit, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void stopDtmfTone(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopDtmfTone, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopDtmfTone(callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void conference(java.lang.String conferenceCallId, java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(conferenceCallId);
          _data.writeString(callId);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_conference, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().conference(conferenceCallId, callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void splitFromConference(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_splitFromConference, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().splitFromConference(callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void mergeConference(java.lang.String conferenceCallId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(conferenceCallId);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_mergeConference, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().mergeConference(conferenceCallId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void swapConference(java.lang.String conferenceCallId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(conferenceCallId);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_swapConference, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().swapConference(conferenceCallId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPostDialContinue(java.lang.String callId, boolean proceed, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeInt(((proceed)?(1):(0)));
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPostDialContinue, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPostDialContinue(callId, proceed, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void pullExternalCall(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_pullExternalCall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().pullExternalCall(callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void sendCallEvent(java.lang.String callId, java.lang.String event, android.os.Bundle extras, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeString(event);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendCallEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendCallEvent(callId, event, extras, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onExtrasChanged(java.lang.String callId, android.os.Bundle extras, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
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
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onExtrasChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onExtrasChanged(callId, extras, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void startRtt(java.lang.String callId, android.os.ParcelFileDescriptor fromInCall, android.os.ParcelFileDescriptor toInCall, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((fromInCall!=null)) {
            _data.writeInt(1);
            fromInCall.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((toInCall!=null)) {
            _data.writeInt(1);
            toInCall.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startRtt, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startRtt(callId, fromInCall, toInCall, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void stopRtt(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopRtt, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopRtt(callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void respondToRttUpgradeRequest(java.lang.String callId, android.os.ParcelFileDescriptor fromInCall, android.os.ParcelFileDescriptor toInCall, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((fromInCall!=null)) {
            _data.writeInt(1);
            fromInCall.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((toInCall!=null)) {
            _data.writeInt(1);
            toInCall.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_respondToRttUpgradeRequest, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().respondToRttUpgradeRequest(callId, fromInCall, toInCall, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void connectionServiceFocusLost(android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_connectionServiceFocusLost, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().connectionServiceFocusLost(sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void connectionServiceFocusGained(android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_connectionServiceFocusGained, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().connectionServiceFocusGained(sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void handoverFailed(java.lang.String callId, android.telecom.ConnectionRequest request, int error, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(error);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_handoverFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().handoverFailed(callId, request, error, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void handoverComplete(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_handoverComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().handoverComplete(callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.telecom.IConnectionService sDefaultImpl;
    }
    static final int TRANSACTION_addConnectionServiceAdapter = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_removeConnectionServiceAdapter = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_createConnection = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_createConnectionComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_createConnectionFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_abort = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_answerVideo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_answer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_deflect = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_reject = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_rejectWithMessage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_disconnect = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_silence = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_hold = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_unhold = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_onCallAudioStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_playDtmfTone = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_stopDtmfTone = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_conference = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_splitFromConference = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_mergeConference = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_swapConference = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_onPostDialContinue = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_pullExternalCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_sendCallEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_onExtrasChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_startRtt = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_stopRtt = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_respondToRttUpgradeRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_connectionServiceFocusLost = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_connectionServiceFocusGained = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_handoverFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_handoverComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    public static boolean setDefaultImpl(com.android.internal.telecom.IConnectionService impl) {
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
    public static com.android.internal.telecom.IConnectionService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void addConnectionServiceAdapter(com.android.internal.telecom.IConnectionServiceAdapter adapter, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void removeConnectionServiceAdapter(com.android.internal.telecom.IConnectionServiceAdapter adapter, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void createConnection(android.telecom.PhoneAccountHandle connectionManagerPhoneAccount, java.lang.String callId, android.telecom.ConnectionRequest request, boolean isIncoming, boolean isUnknown, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void createConnectionComplete(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void createConnectionFailed(android.telecom.PhoneAccountHandle connectionManagerPhoneAccount, java.lang.String callId, android.telecom.ConnectionRequest request, boolean isIncoming, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void abort(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void answerVideo(java.lang.String callId, int videoState, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void answer(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void deflect(java.lang.String callId, android.net.Uri address, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void reject(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void rejectWithMessage(java.lang.String callId, java.lang.String message, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void disconnect(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void silence(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void hold(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void unhold(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void onCallAudioStateChanged(java.lang.String activeCallId, android.telecom.CallAudioState callAudioState, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void playDtmfTone(java.lang.String callId, char digit, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void stopDtmfTone(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void conference(java.lang.String conferenceCallId, java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void splitFromConference(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void mergeConference(java.lang.String conferenceCallId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void swapConference(java.lang.String conferenceCallId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void onPostDialContinue(java.lang.String callId, boolean proceed, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void pullExternalCall(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void sendCallEvent(java.lang.String callId, java.lang.String event, android.os.Bundle extras, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void onExtrasChanged(java.lang.String callId, android.os.Bundle extras, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void startRtt(java.lang.String callId, android.os.ParcelFileDescriptor fromInCall, android.os.ParcelFileDescriptor toInCall, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void stopRtt(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void respondToRttUpgradeRequest(java.lang.String callId, android.os.ParcelFileDescriptor fromInCall, android.os.ParcelFileDescriptor toInCall, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void connectionServiceFocusLost(android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void connectionServiceFocusGained(android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void handoverFailed(java.lang.String callId, android.telecom.ConnectionRequest request, int error, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void handoverComplete(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
}
