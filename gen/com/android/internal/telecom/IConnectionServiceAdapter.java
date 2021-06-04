/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.telecom;
/**
 * Internal remote callback interface for connection services.
 *
 * @see android.telecom.ConnectionServiceAdapter
 *
 * {@hide}
 */
public interface IConnectionServiceAdapter extends android.os.IInterface
{
  /** Default implementation for IConnectionServiceAdapter. */
  public static class Default implements com.android.internal.telecom.IConnectionServiceAdapter
  {
    @Override public void handleCreateConnectionComplete(java.lang.String callId, android.telecom.ConnectionRequest request, android.telecom.ParcelableConnection connection, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void setActive(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void setRinging(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void setDialing(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void setPulling(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void setDisconnected(java.lang.String callId, android.telecom.DisconnectCause disconnectCause, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void setOnHold(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void setRingbackRequested(java.lang.String callId, boolean ringing, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void setConnectionCapabilities(java.lang.String callId, int connectionCapabilities, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void setConnectionProperties(java.lang.String callId, int connectionProperties, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void setIsConferenced(java.lang.String callId, java.lang.String conferenceCallId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void setConferenceMergeFailed(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void addConferenceCall(java.lang.String callId, android.telecom.ParcelableConference conference, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void removeCall(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void onPostDialWait(java.lang.String callId, java.lang.String remaining, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void onPostDialChar(java.lang.String callId, char nextChar, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void queryRemoteConnectionServices(com.android.internal.telecom.RemoteServiceCallback callback, java.lang.String callingPackage, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void setVideoProvider(java.lang.String callId, com.android.internal.telecom.IVideoProvider videoProvider, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void setVideoState(java.lang.String callId, int videoState, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void setIsVoipAudioMode(java.lang.String callId, boolean isVoip, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void setStatusHints(java.lang.String callId, android.telecom.StatusHints statusHints, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void setAddress(java.lang.String callId, android.net.Uri address, int presentation, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void setCallerDisplayName(java.lang.String callId, java.lang.String callerDisplayName, int presentation, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void setConferenceableConnections(java.lang.String callId, java.util.List<java.lang.String> conferenceableCallIds, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void addExistingConnection(java.lang.String callId, android.telecom.ParcelableConnection connection, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void putExtras(java.lang.String callId, android.os.Bundle extras, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void removeExtras(java.lang.String callId, java.util.List<java.lang.String> keys, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void setAudioRoute(java.lang.String callId, int audioRoute, java.lang.String bluetoothAddress, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void onConnectionEvent(java.lang.String callId, java.lang.String event, android.os.Bundle extras, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void onRttInitiationSuccess(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void onRttInitiationFailure(java.lang.String callId, int reason, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void onRttSessionRemotelyTerminated(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void onRemoteRttRequest(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void onPhoneAccountChanged(java.lang.String callId, android.telecom.PhoneAccountHandle pHandle, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void onConnectionServiceFocusReleased(android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void resetConnectionTime(java.lang.String callIdi, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override public void setConferenceState(java.lang.String callId, boolean isConference, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.telecom.IConnectionServiceAdapter
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.telecom.IConnectionServiceAdapter";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.telecom.IConnectionServiceAdapter interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.telecom.IConnectionServiceAdapter asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.telecom.IConnectionServiceAdapter))) {
        return ((com.android.internal.telecom.IConnectionServiceAdapter)iin);
      }
      return new com.android.internal.telecom.IConnectionServiceAdapter.Stub.Proxy(obj);
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
        case TRANSACTION_handleCreateConnectionComplete:
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
          android.telecom.ParcelableConnection _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telecom.ParcelableConnection.CREATOR.createFromParcel(data);
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
          this.handleCreateConnectionComplete(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_setActive:
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
          this.setActive(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setRinging:
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
          this.setRinging(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setDialing:
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
          this.setDialing(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setPulling:
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
          this.setPulling(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setDisconnected:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.DisconnectCause _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.DisconnectCause.CREATOR.createFromParcel(data);
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
          this.setDisconnected(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_setOnHold:
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
          this.setOnHold(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setRingbackRequested:
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
          this.setRingbackRequested(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_setConnectionCapabilities:
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
          this.setConnectionCapabilities(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_setConnectionProperties:
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
          this.setConnectionProperties(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_setIsConferenced:
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
          this.setIsConferenced(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_setConferenceMergeFailed:
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
          this.setConferenceMergeFailed(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_addConferenceCall:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.ParcelableConference _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.ParcelableConference.CREATOR.createFromParcel(data);
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
          this.addConferenceCall(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_removeCall:
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
          this.removeCall(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onPostDialWait:
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
          this.onPostDialWait(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onPostDialChar:
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
          this.onPostDialChar(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_queryRemoteConnectionServices:
        {
          data.enforceInterface(descriptor);
          com.android.internal.telecom.RemoteServiceCallback _arg0;
          _arg0 = com.android.internal.telecom.RemoteServiceCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.telecom.Logging.Session.Info _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.queryRemoteConnectionServices(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_setVideoProvider:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          com.android.internal.telecom.IVideoProvider _arg1;
          _arg1 = com.android.internal.telecom.IVideoProvider.Stub.asInterface(data.readStrongBinder());
          android.telecom.Logging.Session.Info _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.setVideoProvider(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_setVideoState:
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
          this.setVideoState(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_setIsVoipAudioMode:
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
          this.setIsVoipAudioMode(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_setStatusHints:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.StatusHints _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.StatusHints.CREATOR.createFromParcel(data);
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
          this.setStatusHints(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_setAddress:
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
          int _arg2;
          _arg2 = data.readInt();
          android.telecom.Logging.Session.Info _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.setAddress(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_setCallerDisplayName:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          android.telecom.Logging.Session.Info _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.setCallerDisplayName(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_setConferenceableConnections:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.List<java.lang.String> _arg1;
          _arg1 = data.createStringArrayList();
          android.telecom.Logging.Session.Info _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.setConferenceableConnections(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_addExistingConnection:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.ParcelableConnection _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.ParcelableConnection.CREATOR.createFromParcel(data);
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
          this.addExistingConnection(_arg0, _arg1, _arg2);
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
          android.telecom.Logging.Session.Info _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.putExtras(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_removeExtras:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.List<java.lang.String> _arg1;
          _arg1 = data.createStringArrayList();
          android.telecom.Logging.Session.Info _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.removeExtras(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_setAudioRoute:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.telecom.Logging.Session.Info _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.setAudioRoute(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onConnectionEvent:
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
          this.onConnectionEvent(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_onRttInitiationSuccess:
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
          this.onRttInitiationSuccess(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onRttInitiationFailure:
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
          this.onRttInitiationFailure(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onRttSessionRemotelyTerminated:
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
          this.onRttSessionRemotelyTerminated(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onRemoteRttRequest:
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
          this.onRemoteRttRequest(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onPhoneAccountChanged:
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
          android.telecom.Logging.Session.Info _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.onPhoneAccountChanged(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onConnectionServiceFocusReleased:
        {
          data.enforceInterface(descriptor);
          android.telecom.Logging.Session.Info _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telecom.Logging.Session.Info.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onConnectionServiceFocusReleased(_arg0);
          return true;
        }
        case TRANSACTION_resetConnectionTime:
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
          this.resetConnectionTime(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setConferenceState:
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
          this.setConferenceState(_arg0, _arg1, _arg2);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.telecom.IConnectionServiceAdapter
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
      @Override public void handleCreateConnectionComplete(java.lang.String callId, android.telecom.ConnectionRequest request, android.telecom.ParcelableConnection connection, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
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
          if ((connection!=null)) {
            _data.writeInt(1);
            connection.writeToParcel(_data, 0);
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_handleCreateConnectionComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().handleCreateConnectionComplete(callId, request, connection, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setActive(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_setActive, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setActive(callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setRinging(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRinging, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setRinging(callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setDialing(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDialing, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDialing(callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setPulling(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPulling, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPulling(callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setDisconnected(java.lang.String callId, android.telecom.DisconnectCause disconnectCause, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((disconnectCause!=null)) {
            _data.writeInt(1);
            disconnectCause.writeToParcel(_data, 0);
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDisconnected, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDisconnected(callId, disconnectCause, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setOnHold(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_setOnHold, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setOnHold(callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setRingbackRequested(java.lang.String callId, boolean ringing, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeInt(((ringing)?(1):(0)));
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRingbackRequested, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setRingbackRequested(callId, ringing, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setConnectionCapabilities(java.lang.String callId, int connectionCapabilities, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeInt(connectionCapabilities);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setConnectionCapabilities, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setConnectionCapabilities(callId, connectionCapabilities, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setConnectionProperties(java.lang.String callId, int connectionProperties, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeInt(connectionProperties);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setConnectionProperties, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setConnectionProperties(callId, connectionProperties, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setIsConferenced(java.lang.String callId, java.lang.String conferenceCallId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeString(conferenceCallId);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setIsConferenced, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setIsConferenced(callId, conferenceCallId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setConferenceMergeFailed(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_setConferenceMergeFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setConferenceMergeFailed(callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void addConferenceCall(java.lang.String callId, android.telecom.ParcelableConference conference, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((conference!=null)) {
            _data.writeInt(1);
            conference.writeToParcel(_data, 0);
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_addConferenceCall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addConferenceCall(callId, conference, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void removeCall(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeCall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeCall(callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPostDialWait(java.lang.String callId, java.lang.String remaining, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeString(remaining);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPostDialWait, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPostDialWait(callId, remaining, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPostDialChar(java.lang.String callId, char nextChar, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeInt(((int)nextChar));
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPostDialChar, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPostDialChar(callId, nextChar, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void queryRemoteConnectionServices(com.android.internal.telecom.RemoteServiceCallback callback, java.lang.String callingPackage, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          _data.writeString(callingPackage);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryRemoteConnectionServices, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().queryRemoteConnectionServices(callback, callingPackage, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setVideoProvider(java.lang.String callId, com.android.internal.telecom.IVideoProvider videoProvider, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeStrongBinder((((videoProvider!=null))?(videoProvider.asBinder()):(null)));
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVideoProvider, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVideoProvider(callId, videoProvider, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setVideoState(java.lang.String callId, int videoState, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVideoState, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVideoState(callId, videoState, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setIsVoipAudioMode(java.lang.String callId, boolean isVoip, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeInt(((isVoip)?(1):(0)));
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setIsVoipAudioMode, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setIsVoipAudioMode(callId, isVoip, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setStatusHints(java.lang.String callId, android.telecom.StatusHints statusHints, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((statusHints!=null)) {
            _data.writeInt(1);
            statusHints.writeToParcel(_data, 0);
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_setStatusHints, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setStatusHints(callId, statusHints, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setAddress(java.lang.String callId, android.net.Uri address, int presentation, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
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
          _data.writeInt(presentation);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAddress, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAddress(callId, address, presentation, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setCallerDisplayName(java.lang.String callId, java.lang.String callerDisplayName, int presentation, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeString(callerDisplayName);
          _data.writeInt(presentation);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setCallerDisplayName, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setCallerDisplayName(callId, callerDisplayName, presentation, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setConferenceableConnections(java.lang.String callId, java.util.List<java.lang.String> conferenceableCallIds, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeStringList(conferenceableCallIds);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setConferenceableConnections, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setConferenceableConnections(callId, conferenceableCallIds, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void addExistingConnection(java.lang.String callId, android.telecom.ParcelableConnection connection, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((connection!=null)) {
            _data.writeInt(1);
            connection.writeToParcel(_data, 0);
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_addExistingConnection, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addExistingConnection(callId, connection, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void putExtras(java.lang.String callId, android.os.Bundle extras, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_putExtras, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().putExtras(callId, extras, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void removeExtras(java.lang.String callId, java.util.List<java.lang.String> keys, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeStringList(keys);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeExtras, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeExtras(callId, keys, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setAudioRoute(java.lang.String callId, int audioRoute, java.lang.String bluetoothAddress, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeInt(audioRoute);
          _data.writeString(bluetoothAddress);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAudioRoute, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAudioRoute(callId, audioRoute, bluetoothAddress, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onConnectionEvent(java.lang.String callId, java.lang.String event, android.os.Bundle extras, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_onConnectionEvent, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onConnectionEvent(callId, event, extras, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onRttInitiationSuccess(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRttInitiationSuccess, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRttInitiationSuccess(callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onRttInitiationFailure(java.lang.String callId, int reason, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeInt(reason);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRttInitiationFailure, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRttInitiationFailure(callId, reason, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onRttSessionRemotelyTerminated(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRttSessionRemotelyTerminated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRttSessionRemotelyTerminated(callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onRemoteRttRequest(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRemoteRttRequest, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRemoteRttRequest(callId, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onPhoneAccountChanged(java.lang.String callId, android.telecom.PhoneAccountHandle pHandle, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          if ((pHandle!=null)) {
            _data.writeInt(1);
            pHandle.writeToParcel(_data, 0);
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPhoneAccountChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPhoneAccountChanged(callId, pHandle, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onConnectionServiceFocusReleased(android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_onConnectionServiceFocusReleased, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onConnectionServiceFocusReleased(sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void resetConnectionTime(java.lang.String callIdi, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callIdi);
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_resetConnectionTime, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().resetConnectionTime(callIdi, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setConferenceState(java.lang.String callId, boolean isConference, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callId);
          _data.writeInt(((isConference)?(1):(0)));
          if ((sessionInfo!=null)) {
            _data.writeInt(1);
            sessionInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setConferenceState, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setConferenceState(callId, isConference, sessionInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.telecom.IConnectionServiceAdapter sDefaultImpl;
    }
    static final int TRANSACTION_handleCreateConnectionComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_setActive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_setRinging = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_setDialing = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_setPulling = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_setDisconnected = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_setOnHold = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_setRingbackRequested = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_setConnectionCapabilities = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_setConnectionProperties = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_setIsConferenced = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_setConferenceMergeFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_addConferenceCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_removeCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_onPostDialWait = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_onPostDialChar = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_queryRemoteConnectionServices = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_setVideoProvider = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_setVideoState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_setIsVoipAudioMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_setStatusHints = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_setAddress = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_setCallerDisplayName = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_setConferenceableConnections = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_addExistingConnection = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_putExtras = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_removeExtras = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_setAudioRoute = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_onConnectionEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_onRttInitiationSuccess = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_onRttInitiationFailure = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_onRttSessionRemotelyTerminated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_onRemoteRttRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_onPhoneAccountChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_onConnectionServiceFocusReleased = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_resetConnectionTime = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_setConferenceState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    public static boolean setDefaultImpl(com.android.internal.telecom.IConnectionServiceAdapter impl) {
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
    public static com.android.internal.telecom.IConnectionServiceAdapter getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void handleCreateConnectionComplete(java.lang.String callId, android.telecom.ConnectionRequest request, android.telecom.ParcelableConnection connection, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void setActive(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void setRinging(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void setDialing(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void setPulling(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void setDisconnected(java.lang.String callId, android.telecom.DisconnectCause disconnectCause, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void setOnHold(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void setRingbackRequested(java.lang.String callId, boolean ringing, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void setConnectionCapabilities(java.lang.String callId, int connectionCapabilities, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void setConnectionProperties(java.lang.String callId, int connectionProperties, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void setIsConferenced(java.lang.String callId, java.lang.String conferenceCallId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void setConferenceMergeFailed(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void addConferenceCall(java.lang.String callId, android.telecom.ParcelableConference conference, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void removeCall(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void onPostDialWait(java.lang.String callId, java.lang.String remaining, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void onPostDialChar(java.lang.String callId, char nextChar, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void queryRemoteConnectionServices(com.android.internal.telecom.RemoteServiceCallback callback, java.lang.String callingPackage, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void setVideoProvider(java.lang.String callId, com.android.internal.telecom.IVideoProvider videoProvider, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void setVideoState(java.lang.String callId, int videoState, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void setIsVoipAudioMode(java.lang.String callId, boolean isVoip, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void setStatusHints(java.lang.String callId, android.telecom.StatusHints statusHints, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void setAddress(java.lang.String callId, android.net.Uri address, int presentation, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void setCallerDisplayName(java.lang.String callId, java.lang.String callerDisplayName, int presentation, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void setConferenceableConnections(java.lang.String callId, java.util.List<java.lang.String> conferenceableCallIds, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void addExistingConnection(java.lang.String callId, android.telecom.ParcelableConnection connection, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void putExtras(java.lang.String callId, android.os.Bundle extras, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void removeExtras(java.lang.String callId, java.util.List<java.lang.String> keys, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void setAudioRoute(java.lang.String callId, int audioRoute, java.lang.String bluetoothAddress, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void onConnectionEvent(java.lang.String callId, java.lang.String event, android.os.Bundle extras, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void onRttInitiationSuccess(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void onRttInitiationFailure(java.lang.String callId, int reason, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void onRttSessionRemotelyTerminated(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void onRemoteRttRequest(java.lang.String callId, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void onPhoneAccountChanged(java.lang.String callId, android.telecom.PhoneAccountHandle pHandle, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void onConnectionServiceFocusReleased(android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void resetConnectionTime(java.lang.String callIdi, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
  public void setConferenceState(java.lang.String callId, boolean isConference, android.telecom.Logging.Session.Info sessionInfo) throws android.os.RemoteException;
}
