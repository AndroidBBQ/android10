/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.ims.aidl;
/**
 * A listener type for receiving notification on IMS call session events.
 * When an event is generated for an {@link IImsCallSession}, the application is notified
 * by having one of the methods called on the {@link IImsCallSessionListener}.
 * {@hide}
 */
public interface IImsCallSessionListener extends android.os.IInterface
{
  /** Default implementation for IImsCallSessionListener. */
  public static class Default implements android.telephony.ims.aidl.IImsCallSessionListener
  {
    /**
         * Notifies the result of the basic session operation (setup / terminate).
         */
    @Override public void callSessionProgressing(android.telephony.ims.ImsStreamMediaProfile profile) throws android.os.RemoteException
    {
    }
    @Override public void callSessionInitiated(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
    {
    }
    @Override public void callSessionInitiatedFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException
    {
    }
    @Override public void callSessionTerminated(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException
    {
    }
    /**
         * Notifies the result of the call hold/resume operation.
         */
    @Override public void callSessionHeld(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
    {
    }
    @Override public void callSessionHoldFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException
    {
    }
    @Override public void callSessionHoldReceived(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
    {
    }
    @Override public void callSessionResumed(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
    {
    }
    @Override public void callSessionResumeFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException
    {
    }
    @Override public void callSessionResumeReceived(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
    {
    }
    /**
         * Notifies the result of call merge operation.
         */
    @Override public void callSessionMergeStarted(com.android.ims.internal.IImsCallSession newSession, android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
    {
    }
    @Override public void callSessionMergeComplete(com.android.ims.internal.IImsCallSession session) throws android.os.RemoteException
    {
    }
    @Override public void callSessionMergeFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException
    {
    }
    /**
         * Notifies the result of call upgrade / downgrade or any other call updates.
         */
    @Override public void callSessionUpdated(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
    {
    }
    @Override public void callSessionUpdateFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException
    {
    }
    @Override public void callSessionUpdateReceived(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
    {
    }
    /**
         * Notifies the result of conference extension.
         */
    @Override public void callSessionConferenceExtended(com.android.ims.internal.IImsCallSession newSession, android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
    {
    }
    @Override public void callSessionConferenceExtendFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException
    {
    }
    @Override public void callSessionConferenceExtendReceived(com.android.ims.internal.IImsCallSession newSession, android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
    {
    }
    /**
         * Notifies the result of the participant invitation / removal to/from the conference session.
         */
    @Override public void callSessionInviteParticipantsRequestDelivered() throws android.os.RemoteException
    {
    }
    @Override public void callSessionInviteParticipantsRequestFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException
    {
    }
    @Override public void callSessionRemoveParticipantsRequestDelivered() throws android.os.RemoteException
    {
    }
    @Override public void callSessionRemoveParticipantsRequestFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException
    {
    }
    /**
         * Notifies the changes of the conference info. in the conference session.
         */
    @Override public void callSessionConferenceStateUpdated(android.telephony.ims.ImsConferenceState state) throws android.os.RemoteException
    {
    }
    /**
         * Notifies the incoming USSD message.
         */
    @Override public void callSessionUssdMessageReceived(int mode, java.lang.String ussdMessage) throws android.os.RemoteException
    {
    }
    /**
         * Notifies of handover information for this call
         */
    @Override public void callSessionHandover(int srcAccessTech, int targetAccessTech, android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException
    {
    }
    @Override public void callSessionHandoverFailed(int srcAccessTech, int targetAccessTech, android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException
    {
    }
    @Override public void callSessionMayHandover(int srcAccessTech, int targetAccessTech) throws android.os.RemoteException
    {
    }
    /**
         * Notifies the TTY mode change by remote party.
         * @param mode one of the following:
         * - {@link com.android.internal.telephony.Phone#TTY_MODE_OFF}
         * - {@link com.android.internal.telephony.Phone#TTY_MODE_FULL}
         * - {@link com.android.internal.telephony.Phone#TTY_MODE_HCO}
         * - {@link com.android.internal.telephony.Phone#TTY_MODE_VCO}
         */
    @Override public void callSessionTtyModeReceived(int mode) throws android.os.RemoteException
    {
    }
    /**
         * Notifies of a change to the multiparty state for this {@code ImsCallSession}.
         *
         * @param session The call session.
         * @param isMultiParty {@code true} if the session became multiparty, {@code false} otherwise.
         */
    @Override public void callSessionMultipartyStateChanged(boolean isMultiParty) throws android.os.RemoteException
    {
    }
    /**
         * Notifies the supplementary service information for the current session.
         */
    @Override public void callSessionSuppServiceReceived(android.telephony.ims.ImsSuppServiceNotification suppSrvNotification) throws android.os.RemoteException
    {
    }
    /**
         * Device received RTT modify request from Remote UE
         * @param session ImsCallProfile with updated attribute
         */
    @Override public void callSessionRttModifyRequestReceived(android.telephony.ims.ImsCallProfile callProfile) throws android.os.RemoteException
    {
    }
    /**
         * Device issued RTT modify request and inturn received response
         * from Remote UE
         * @param status Will be one of the following values from:
         * - {@link Connection.RttModifyStatus}
         */
    @Override public void callSessionRttModifyResponseReceived(int status) throws android.os.RemoteException
    {
    }
    /**
         * While in call, device received RTT message from Remote UE
         * @param rttMessage Received RTT message
         */
    @Override public void callSessionRttMessageReceived(java.lang.String rttMessage) throws android.os.RemoteException
    {
    }
    /**
         * While in call, there has been a change in RTT audio indicator.
         * @param profile updated ImsStreamMediaProfile
         */
    @Override public void callSessionRttAudioIndicatorChanged(android.telephony.ims.ImsStreamMediaProfile profile) throws android.os.RemoteException
    {
    }
    /**
         * Notifies of a change to the call quality.
         * @param callQuality then updated call quality
         */
    @Override public void callQualityChanged(android.telephony.CallQuality callQuality) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.ims.aidl.IImsCallSessionListener
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.ims.aidl.IImsCallSessionListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.ims.aidl.IImsCallSessionListener interface,
     * generating a proxy if needed.
     */
    public static android.telephony.ims.aidl.IImsCallSessionListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.ims.aidl.IImsCallSessionListener))) {
        return ((android.telephony.ims.aidl.IImsCallSessionListener)iin);
      }
      return new android.telephony.ims.aidl.IImsCallSessionListener.Stub.Proxy(obj);
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
        case TRANSACTION_callSessionProgressing:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsStreamMediaProfile _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsStreamMediaProfile.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.callSessionProgressing(_arg0);
          return true;
        }
        case TRANSACTION_callSessionInitiated:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsCallProfile _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsCallProfile.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.callSessionInitiated(_arg0);
          return true;
        }
        case TRANSACTION_callSessionInitiatedFailed:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsReasonInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsReasonInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.callSessionInitiatedFailed(_arg0);
          return true;
        }
        case TRANSACTION_callSessionTerminated:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsReasonInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsReasonInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.callSessionTerminated(_arg0);
          return true;
        }
        case TRANSACTION_callSessionHeld:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsCallProfile _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsCallProfile.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.callSessionHeld(_arg0);
          return true;
        }
        case TRANSACTION_callSessionHoldFailed:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsReasonInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsReasonInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.callSessionHoldFailed(_arg0);
          return true;
        }
        case TRANSACTION_callSessionHoldReceived:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsCallProfile _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsCallProfile.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.callSessionHoldReceived(_arg0);
          return true;
        }
        case TRANSACTION_callSessionResumed:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsCallProfile _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsCallProfile.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.callSessionResumed(_arg0);
          return true;
        }
        case TRANSACTION_callSessionResumeFailed:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsReasonInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsReasonInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.callSessionResumeFailed(_arg0);
          return true;
        }
        case TRANSACTION_callSessionResumeReceived:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsCallProfile _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsCallProfile.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.callSessionResumeReceived(_arg0);
          return true;
        }
        case TRANSACTION_callSessionMergeStarted:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.IImsCallSession _arg0;
          _arg0 = com.android.ims.internal.IImsCallSession.Stub.asInterface(data.readStrongBinder());
          android.telephony.ims.ImsCallProfile _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telephony.ims.ImsCallProfile.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.callSessionMergeStarted(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_callSessionMergeComplete:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.IImsCallSession _arg0;
          _arg0 = com.android.ims.internal.IImsCallSession.Stub.asInterface(data.readStrongBinder());
          this.callSessionMergeComplete(_arg0);
          return true;
        }
        case TRANSACTION_callSessionMergeFailed:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsReasonInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsReasonInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.callSessionMergeFailed(_arg0);
          return true;
        }
        case TRANSACTION_callSessionUpdated:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsCallProfile _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsCallProfile.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.callSessionUpdated(_arg0);
          return true;
        }
        case TRANSACTION_callSessionUpdateFailed:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsReasonInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsReasonInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.callSessionUpdateFailed(_arg0);
          return true;
        }
        case TRANSACTION_callSessionUpdateReceived:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsCallProfile _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsCallProfile.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.callSessionUpdateReceived(_arg0);
          return true;
        }
        case TRANSACTION_callSessionConferenceExtended:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.IImsCallSession _arg0;
          _arg0 = com.android.ims.internal.IImsCallSession.Stub.asInterface(data.readStrongBinder());
          android.telephony.ims.ImsCallProfile _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telephony.ims.ImsCallProfile.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.callSessionConferenceExtended(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_callSessionConferenceExtendFailed:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsReasonInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsReasonInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.callSessionConferenceExtendFailed(_arg0);
          return true;
        }
        case TRANSACTION_callSessionConferenceExtendReceived:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.IImsCallSession _arg0;
          _arg0 = com.android.ims.internal.IImsCallSession.Stub.asInterface(data.readStrongBinder());
          android.telephony.ims.ImsCallProfile _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telephony.ims.ImsCallProfile.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.callSessionConferenceExtendReceived(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_callSessionInviteParticipantsRequestDelivered:
        {
          data.enforceInterface(descriptor);
          this.callSessionInviteParticipantsRequestDelivered();
          return true;
        }
        case TRANSACTION_callSessionInviteParticipantsRequestFailed:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsReasonInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsReasonInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.callSessionInviteParticipantsRequestFailed(_arg0);
          return true;
        }
        case TRANSACTION_callSessionRemoveParticipantsRequestDelivered:
        {
          data.enforceInterface(descriptor);
          this.callSessionRemoveParticipantsRequestDelivered();
          return true;
        }
        case TRANSACTION_callSessionRemoveParticipantsRequestFailed:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsReasonInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsReasonInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.callSessionRemoveParticipantsRequestFailed(_arg0);
          return true;
        }
        case TRANSACTION_callSessionConferenceStateUpdated:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsConferenceState _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsConferenceState.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.callSessionConferenceStateUpdated(_arg0);
          return true;
        }
        case TRANSACTION_callSessionUssdMessageReceived:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.callSessionUssdMessageReceived(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_callSessionHandover:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.telephony.ims.ImsReasonInfo _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telephony.ims.ImsReasonInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.callSessionHandover(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_callSessionHandoverFailed:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.telephony.ims.ImsReasonInfo _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telephony.ims.ImsReasonInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.callSessionHandoverFailed(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_callSessionMayHandover:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.callSessionMayHandover(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_callSessionTtyModeReceived:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.callSessionTtyModeReceived(_arg0);
          return true;
        }
        case TRANSACTION_callSessionMultipartyStateChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.callSessionMultipartyStateChanged(_arg0);
          return true;
        }
        case TRANSACTION_callSessionSuppServiceReceived:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsSuppServiceNotification _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsSuppServiceNotification.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.callSessionSuppServiceReceived(_arg0);
          return true;
        }
        case TRANSACTION_callSessionRttModifyRequestReceived:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsCallProfile _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsCallProfile.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.callSessionRttModifyRequestReceived(_arg0);
          return true;
        }
        case TRANSACTION_callSessionRttModifyResponseReceived:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.callSessionRttModifyResponseReceived(_arg0);
          return true;
        }
        case TRANSACTION_callSessionRttMessageReceived:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.callSessionRttMessageReceived(_arg0);
          return true;
        }
        case TRANSACTION_callSessionRttAudioIndicatorChanged:
        {
          data.enforceInterface(descriptor);
          android.telephony.ims.ImsStreamMediaProfile _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.ims.ImsStreamMediaProfile.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.callSessionRttAudioIndicatorChanged(_arg0);
          return true;
        }
        case TRANSACTION_callQualityChanged:
        {
          data.enforceInterface(descriptor);
          android.telephony.CallQuality _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.CallQuality.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.callQualityChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.ims.aidl.IImsCallSessionListener
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
           * Notifies the result of the basic session operation (setup / terminate).
           */
      @Override public void callSessionProgressing(android.telephony.ims.ImsStreamMediaProfile profile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((profile!=null)) {
            _data.writeInt(1);
            profile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionProgressing, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionProgressing(profile);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void callSessionInitiated(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((profile!=null)) {
            _data.writeInt(1);
            profile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionInitiated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionInitiated(profile);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void callSessionInitiatedFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((reasonInfo!=null)) {
            _data.writeInt(1);
            reasonInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionInitiatedFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionInitiatedFailed(reasonInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void callSessionTerminated(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((reasonInfo!=null)) {
            _data.writeInt(1);
            reasonInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionTerminated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionTerminated(reasonInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the result of the call hold/resume operation.
           */
      @Override public void callSessionHeld(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((profile!=null)) {
            _data.writeInt(1);
            profile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionHeld, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionHeld(profile);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void callSessionHoldFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((reasonInfo!=null)) {
            _data.writeInt(1);
            reasonInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionHoldFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionHoldFailed(reasonInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void callSessionHoldReceived(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((profile!=null)) {
            _data.writeInt(1);
            profile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionHoldReceived, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionHoldReceived(profile);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void callSessionResumed(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((profile!=null)) {
            _data.writeInt(1);
            profile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionResumed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionResumed(profile);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void callSessionResumeFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((reasonInfo!=null)) {
            _data.writeInt(1);
            reasonInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionResumeFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionResumeFailed(reasonInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void callSessionResumeReceived(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((profile!=null)) {
            _data.writeInt(1);
            profile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionResumeReceived, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionResumeReceived(profile);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the result of call merge operation.
           */
      @Override public void callSessionMergeStarted(com.android.ims.internal.IImsCallSession newSession, android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((newSession!=null))?(newSession.asBinder()):(null)));
          if ((profile!=null)) {
            _data.writeInt(1);
            profile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionMergeStarted, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionMergeStarted(newSession, profile);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void callSessionMergeComplete(com.android.ims.internal.IImsCallSession session) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((session!=null))?(session.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionMergeComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionMergeComplete(session);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void callSessionMergeFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((reasonInfo!=null)) {
            _data.writeInt(1);
            reasonInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionMergeFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionMergeFailed(reasonInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the result of call upgrade / downgrade or any other call updates.
           */
      @Override public void callSessionUpdated(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((profile!=null)) {
            _data.writeInt(1);
            profile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionUpdated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionUpdated(profile);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void callSessionUpdateFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((reasonInfo!=null)) {
            _data.writeInt(1);
            reasonInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionUpdateFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionUpdateFailed(reasonInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void callSessionUpdateReceived(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((profile!=null)) {
            _data.writeInt(1);
            profile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionUpdateReceived, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionUpdateReceived(profile);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the result of conference extension.
           */
      @Override public void callSessionConferenceExtended(com.android.ims.internal.IImsCallSession newSession, android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((newSession!=null))?(newSession.asBinder()):(null)));
          if ((profile!=null)) {
            _data.writeInt(1);
            profile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionConferenceExtended, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionConferenceExtended(newSession, profile);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void callSessionConferenceExtendFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((reasonInfo!=null)) {
            _data.writeInt(1);
            reasonInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionConferenceExtendFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionConferenceExtendFailed(reasonInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void callSessionConferenceExtendReceived(com.android.ims.internal.IImsCallSession newSession, android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((newSession!=null))?(newSession.asBinder()):(null)));
          if ((profile!=null)) {
            _data.writeInt(1);
            profile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionConferenceExtendReceived, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionConferenceExtendReceived(newSession, profile);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the result of the participant invitation / removal to/from the conference session.
           */
      @Override public void callSessionInviteParticipantsRequestDelivered() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionInviteParticipantsRequestDelivered, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionInviteParticipantsRequestDelivered();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void callSessionInviteParticipantsRequestFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((reasonInfo!=null)) {
            _data.writeInt(1);
            reasonInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionInviteParticipantsRequestFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionInviteParticipantsRequestFailed(reasonInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void callSessionRemoveParticipantsRequestDelivered() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionRemoveParticipantsRequestDelivered, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionRemoveParticipantsRequestDelivered();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void callSessionRemoveParticipantsRequestFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((reasonInfo!=null)) {
            _data.writeInt(1);
            reasonInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionRemoveParticipantsRequestFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionRemoveParticipantsRequestFailed(reasonInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the changes of the conference info. in the conference session.
           */
      @Override public void callSessionConferenceStateUpdated(android.telephony.ims.ImsConferenceState state) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((state!=null)) {
            _data.writeInt(1);
            state.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionConferenceStateUpdated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionConferenceStateUpdated(state);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the incoming USSD message.
           */
      @Override public void callSessionUssdMessageReceived(int mode, java.lang.String ussdMessage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(mode);
          _data.writeString(ussdMessage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionUssdMessageReceived, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionUssdMessageReceived(mode, ussdMessage);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies of handover information for this call
           */
      @Override public void callSessionHandover(int srcAccessTech, int targetAccessTech, android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(srcAccessTech);
          _data.writeInt(targetAccessTech);
          if ((reasonInfo!=null)) {
            _data.writeInt(1);
            reasonInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionHandover, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionHandover(srcAccessTech, targetAccessTech, reasonInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void callSessionHandoverFailed(int srcAccessTech, int targetAccessTech, android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(srcAccessTech);
          _data.writeInt(targetAccessTech);
          if ((reasonInfo!=null)) {
            _data.writeInt(1);
            reasonInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionHandoverFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionHandoverFailed(srcAccessTech, targetAccessTech, reasonInfo);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void callSessionMayHandover(int srcAccessTech, int targetAccessTech) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(srcAccessTech);
          _data.writeInt(targetAccessTech);
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionMayHandover, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionMayHandover(srcAccessTech, targetAccessTech);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the TTY mode change by remote party.
           * @param mode one of the following:
           * - {@link com.android.internal.telephony.Phone#TTY_MODE_OFF}
           * - {@link com.android.internal.telephony.Phone#TTY_MODE_FULL}
           * - {@link com.android.internal.telephony.Phone#TTY_MODE_HCO}
           * - {@link com.android.internal.telephony.Phone#TTY_MODE_VCO}
           */
      @Override public void callSessionTtyModeReceived(int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionTtyModeReceived, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionTtyModeReceived(mode);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies of a change to the multiparty state for this {@code ImsCallSession}.
           *
           * @param session The call session.
           * @param isMultiParty {@code true} if the session became multiparty, {@code false} otherwise.
           */
      @Override public void callSessionMultipartyStateChanged(boolean isMultiParty) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((isMultiParty)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionMultipartyStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionMultipartyStateChanged(isMultiParty);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the supplementary service information for the current session.
           */
      @Override public void callSessionSuppServiceReceived(android.telephony.ims.ImsSuppServiceNotification suppSrvNotification) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((suppSrvNotification!=null)) {
            _data.writeInt(1);
            suppSrvNotification.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionSuppServiceReceived, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionSuppServiceReceived(suppSrvNotification);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Device received RTT modify request from Remote UE
           * @param session ImsCallProfile with updated attribute
           */
      @Override public void callSessionRttModifyRequestReceived(android.telephony.ims.ImsCallProfile callProfile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((callProfile!=null)) {
            _data.writeInt(1);
            callProfile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionRttModifyRequestReceived, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionRttModifyRequestReceived(callProfile);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Device issued RTT modify request and inturn received response
           * from Remote UE
           * @param status Will be one of the following values from:
           * - {@link Connection.RttModifyStatus}
           */
      @Override public void callSessionRttModifyResponseReceived(int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionRttModifyResponseReceived, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionRttModifyResponseReceived(status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * While in call, device received RTT message from Remote UE
           * @param rttMessage Received RTT message
           */
      @Override public void callSessionRttMessageReceived(java.lang.String rttMessage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(rttMessage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionRttMessageReceived, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionRttMessageReceived(rttMessage);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * While in call, there has been a change in RTT audio indicator.
           * @param profile updated ImsStreamMediaProfile
           */
      @Override public void callSessionRttAudioIndicatorChanged(android.telephony.ims.ImsStreamMediaProfile profile) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((profile!=null)) {
            _data.writeInt(1);
            profile.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callSessionRttAudioIndicatorChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callSessionRttAudioIndicatorChanged(profile);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies of a change to the call quality.
           * @param callQuality then updated call quality
           */
      @Override public void callQualityChanged(android.telephony.CallQuality callQuality) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((callQuality!=null)) {
            _data.writeInt(1);
            callQuality.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_callQualityChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().callQualityChanged(callQuality);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.telephony.ims.aidl.IImsCallSessionListener sDefaultImpl;
    }
    static final int TRANSACTION_callSessionProgressing = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_callSessionInitiated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_callSessionInitiatedFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_callSessionTerminated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_callSessionHeld = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_callSessionHoldFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_callSessionHoldReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_callSessionResumed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_callSessionResumeFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_callSessionResumeReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_callSessionMergeStarted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_callSessionMergeComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_callSessionMergeFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_callSessionUpdated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_callSessionUpdateFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_callSessionUpdateReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_callSessionConferenceExtended = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_callSessionConferenceExtendFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_callSessionConferenceExtendReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_callSessionInviteParticipantsRequestDelivered = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_callSessionInviteParticipantsRequestFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_callSessionRemoveParticipantsRequestDelivered = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_callSessionRemoveParticipantsRequestFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_callSessionConferenceStateUpdated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_callSessionUssdMessageReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_callSessionHandover = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_callSessionHandoverFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_callSessionMayHandover = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_callSessionTtyModeReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_callSessionMultipartyStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_callSessionSuppServiceReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_callSessionRttModifyRequestReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_callSessionRttModifyResponseReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_callSessionRttMessageReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_callSessionRttAudioIndicatorChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_callQualityChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    public static boolean setDefaultImpl(android.telephony.ims.aidl.IImsCallSessionListener impl) {
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
    public static android.telephony.ims.aidl.IImsCallSessionListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Notifies the result of the basic session operation (setup / terminate).
       */
  public void callSessionProgressing(android.telephony.ims.ImsStreamMediaProfile profile) throws android.os.RemoteException;
  public void callSessionInitiated(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException;
  public void callSessionInitiatedFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException;
  public void callSessionTerminated(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException;
  /**
       * Notifies the result of the call hold/resume operation.
       */
  public void callSessionHeld(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException;
  public void callSessionHoldFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException;
  public void callSessionHoldReceived(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException;
  public void callSessionResumed(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException;
  public void callSessionResumeFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException;
  public void callSessionResumeReceived(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException;
  /**
       * Notifies the result of call merge operation.
       */
  public void callSessionMergeStarted(com.android.ims.internal.IImsCallSession newSession, android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException;
  public void callSessionMergeComplete(com.android.ims.internal.IImsCallSession session) throws android.os.RemoteException;
  public void callSessionMergeFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException;
  /**
       * Notifies the result of call upgrade / downgrade or any other call updates.
       */
  public void callSessionUpdated(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException;
  public void callSessionUpdateFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException;
  public void callSessionUpdateReceived(android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException;
  /**
       * Notifies the result of conference extension.
       */
  public void callSessionConferenceExtended(com.android.ims.internal.IImsCallSession newSession, android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException;
  public void callSessionConferenceExtendFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException;
  public void callSessionConferenceExtendReceived(com.android.ims.internal.IImsCallSession newSession, android.telephony.ims.ImsCallProfile profile) throws android.os.RemoteException;
  /**
       * Notifies the result of the participant invitation / removal to/from the conference session.
       */
  public void callSessionInviteParticipantsRequestDelivered() throws android.os.RemoteException;
  public void callSessionInviteParticipantsRequestFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException;
  public void callSessionRemoveParticipantsRequestDelivered() throws android.os.RemoteException;
  public void callSessionRemoveParticipantsRequestFailed(android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException;
  /**
       * Notifies the changes of the conference info. in the conference session.
       */
  public void callSessionConferenceStateUpdated(android.telephony.ims.ImsConferenceState state) throws android.os.RemoteException;
  /**
       * Notifies the incoming USSD message.
       */
  public void callSessionUssdMessageReceived(int mode, java.lang.String ussdMessage) throws android.os.RemoteException;
  /**
       * Notifies of handover information for this call
       */
  public void callSessionHandover(int srcAccessTech, int targetAccessTech, android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException;
  public void callSessionHandoverFailed(int srcAccessTech, int targetAccessTech, android.telephony.ims.ImsReasonInfo reasonInfo) throws android.os.RemoteException;
  public void callSessionMayHandover(int srcAccessTech, int targetAccessTech) throws android.os.RemoteException;
  /**
       * Notifies the TTY mode change by remote party.
       * @param mode one of the following:
       * - {@link com.android.internal.telephony.Phone#TTY_MODE_OFF}
       * - {@link com.android.internal.telephony.Phone#TTY_MODE_FULL}
       * - {@link com.android.internal.telephony.Phone#TTY_MODE_HCO}
       * - {@link com.android.internal.telephony.Phone#TTY_MODE_VCO}
       */
  public void callSessionTtyModeReceived(int mode) throws android.os.RemoteException;
  /**
       * Notifies of a change to the multiparty state for this {@code ImsCallSession}.
       *
       * @param session The call session.
       * @param isMultiParty {@code true} if the session became multiparty, {@code false} otherwise.
       */
  public void callSessionMultipartyStateChanged(boolean isMultiParty) throws android.os.RemoteException;
  /**
       * Notifies the supplementary service information for the current session.
       */
  public void callSessionSuppServiceReceived(android.telephony.ims.ImsSuppServiceNotification suppSrvNotification) throws android.os.RemoteException;
  /**
       * Device received RTT modify request from Remote UE
       * @param session ImsCallProfile with updated attribute
       */
  public void callSessionRttModifyRequestReceived(android.telephony.ims.ImsCallProfile callProfile) throws android.os.RemoteException;
  /**
       * Device issued RTT modify request and inturn received response
       * from Remote UE
       * @param status Will be one of the following values from:
       * - {@link Connection.RttModifyStatus}
       */
  public void callSessionRttModifyResponseReceived(int status) throws android.os.RemoteException;
  /**
       * While in call, device received RTT message from Remote UE
       * @param rttMessage Received RTT message
       */
  public void callSessionRttMessageReceived(java.lang.String rttMessage) throws android.os.RemoteException;
  /**
       * While in call, there has been a change in RTT audio indicator.
       * @param profile updated ImsStreamMediaProfile
       */
  public void callSessionRttAudioIndicatorChanged(android.telephony.ims.ImsStreamMediaProfile profile) throws android.os.RemoteException;
  /**
       * Notifies of a change to the call quality.
       * @param callQuality then updated call quality
       */
  public void callQualityChanged(android.telephony.CallQuality callQuality) throws android.os.RemoteException;
}
