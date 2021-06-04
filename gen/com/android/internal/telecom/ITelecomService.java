/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.telecom;
/**
 * Interface used to interact with Telecom. Mostly this is used by TelephonyManager for passing
 * commands that were previously handled by ITelephony.
 * {@hide}
 */
public interface ITelecomService extends android.os.IInterface
{
  /** Default implementation for ITelecomService. */
  public static class Default implements com.android.internal.telecom.ITelecomService
  {
    /**
         * Brings the in-call screen to the foreground if there is an active call.
         *
         * @param showDialpad if true, make the dialpad visible initially.
         */
    @Override public void showInCallScreen(boolean showDialpad, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    /**
         * @see TelecomServiceImpl#getDefaultOutgoingPhoneAccount
         */
    @Override public android.telecom.PhoneAccountHandle getDefaultOutgoingPhoneAccount(java.lang.String uriScheme, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @see TelecomServiceImpl#getUserSelectedOutgoingPhoneAccount
         */
    @Override public android.telecom.PhoneAccountHandle getUserSelectedOutgoingPhoneAccount(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @see TelecomServiceImpl#setUserSelectedOutgoingPhoneAccount
         */
    @Override public void setUserSelectedOutgoingPhoneAccount(android.telecom.PhoneAccountHandle account) throws android.os.RemoteException
    {
    }
    /**
         * @see TelecomServiceImpl#getCallCapablePhoneAccounts
         */
    @Override public java.util.List<android.telecom.PhoneAccountHandle> getCallCapablePhoneAccounts(boolean includeDisabledAccounts, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @see TelecomServiceImpl#getSelfManagedPhoneAccounts
         */
    @Override public java.util.List<android.telecom.PhoneAccountHandle> getSelfManagedPhoneAccounts(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @see TelecomManager#getPhoneAccountsSupportingScheme
         */
    @Override public java.util.List<android.telecom.PhoneAccountHandle> getPhoneAccountsSupportingScheme(java.lang.String uriScheme, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @see TelecomManager#getPhoneAccountsForPackage
         */
    @Override public java.util.List<android.telecom.PhoneAccountHandle> getPhoneAccountsForPackage(java.lang.String packageName) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @see TelecomManager#getPhoneAccount
         */
    @Override public android.telecom.PhoneAccount getPhoneAccount(android.telecom.PhoneAccountHandle account) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @see TelecomManager#getAllPhoneAccountsCount
         */
    @Override public int getAllPhoneAccountsCount() throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * @see TelecomManager#getAllPhoneAccounts
         */
    @Override public java.util.List<android.telecom.PhoneAccount> getAllPhoneAccounts() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @see TelecomManager#getAllPhoneAccountHandles
         */
    @Override public java.util.List<android.telecom.PhoneAccountHandle> getAllPhoneAccountHandles() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @see TelecomServiceImpl#getSimCallManager
         */
    @Override public android.telecom.PhoneAccountHandle getSimCallManager(int subId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @see TelecomServiceImpl#getSimCallManagerForUser
         */
    @Override public android.telecom.PhoneAccountHandle getSimCallManagerForUser(int userId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @see TelecomServiceImpl#registerPhoneAccount
         */
    @Override public void registerPhoneAccount(android.telecom.PhoneAccount metadata) throws android.os.RemoteException
    {
    }
    /**
         * @see TelecomServiceImpl#unregisterPhoneAccount
         */
    @Override public void unregisterPhoneAccount(android.telecom.PhoneAccountHandle account) throws android.os.RemoteException
    {
    }
    /**
         * @see TelecomServiceImpl#clearAccounts
         */
    @Override public void clearAccounts(java.lang.String packageName) throws android.os.RemoteException
    {
    }
    /**
         * @see TelecomServiceImpl#isVoiceMailNumber
         */
    @Override public boolean isVoiceMailNumber(android.telecom.PhoneAccountHandle accountHandle, java.lang.String number, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * @see TelecomServiceImpl#getVoiceMailNumber
         */
    @Override public java.lang.String getVoiceMailNumber(android.telecom.PhoneAccountHandle accountHandle, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @see TelecomServiceImpl#getLine1Number
         */
    @Override public java.lang.String getLine1Number(android.telecom.PhoneAccountHandle accountHandle, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @see TelecomServiceImpl#getDefaultPhoneApp
         */
    @Override public android.content.ComponentName getDefaultPhoneApp() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @see TelecomServiceImpl#getDefaultDialerPackage
         */
    @Override public java.lang.String getDefaultDialerPackage() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @see TelecomServiceImpl#getSystemDialerPackage
         */
    @Override public java.lang.String getSystemDialerPackage() throws android.os.RemoteException
    {
      return null;
    }
    /**
        * @see TelecomServiceImpl#dumpCallAnalytics
        */
    @Override public android.telecom.TelecomAnalytics dumpCallAnalytics() throws android.os.RemoteException
    {
      return null;
    }
    //
    // Internal system apis relating to call management.
    //
    /**
         * @see TelecomServiceImpl#silenceRinger
         */
    @Override public void silenceRinger(java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    /**
         * @see TelecomServiceImpl#isInCall
         */
    @Override public boolean isInCall(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * @see TelecomServiceImpl#isInManagedCall
         */
    @Override public boolean isInManagedCall(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * @see TelecomServiceImpl#isRinging
         */
    @Override public boolean isRinging(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * @see TelecomServiceImpl#getCallState
         */
    @Override public int getCallState() throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * @see TelecomServiceImpl#endCall
         */
    @Override public boolean endCall(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * @see TelecomServiceImpl#acceptRingingCall
         */
    @Override public void acceptRingingCall(java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    /**
         * @see TelecomServiceImpl#acceptRingingCallWithVideoState(int)
         */
    @Override public void acceptRingingCallWithVideoState(java.lang.String callingPackage, int videoState) throws android.os.RemoteException
    {
    }
    /**
         * @see TelecomServiceImpl#cancelMissedCallsNotification
         */
    @Override public void cancelMissedCallsNotification(java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    /**
         * @see TelecomServiceImpl#handleMmi
         */
    @Override public boolean handlePinMmi(java.lang.String dialString, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * @see TelecomServiceImpl#handleMmi
         */
    @Override public boolean handlePinMmiForPhoneAccount(android.telecom.PhoneAccountHandle accountHandle, java.lang.String dialString, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * @see TelecomServiceImpl#getAdnUriForPhoneAccount
         */
    @Override public android.net.Uri getAdnUriForPhoneAccount(android.telecom.PhoneAccountHandle accountHandle, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @see TelecomServiceImpl#isTtySupported
         */
    @Override public boolean isTtySupported(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * @see TelecomServiceImpl#getCurrentTtyMode
         */
    @Override public int getCurrentTtyMode(java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * @see TelecomServiceImpl#addNewIncomingCall
         */
    @Override public void addNewIncomingCall(android.telecom.PhoneAccountHandle phoneAccount, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    /**
         * @see TelecomServiceImpl#addNewUnknownCall
         */
    @Override public void addNewUnknownCall(android.telecom.PhoneAccountHandle phoneAccount, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    /**
         * @see TelecomServiceImpl#placeCall
         */
    @Override public void placeCall(android.net.Uri handle, android.os.Bundle extras, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    /**
         * @see TelecomServiceImpl#enablePhoneAccount
         */
    @Override public boolean enablePhoneAccount(android.telecom.PhoneAccountHandle accountHandle, boolean isEnabled) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * @see TelecomServiceImpl#setDefaultDialer
         */
    @Override public boolean setDefaultDialer(java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    /**
        * @see TelecomServiceImpl#createManageBlockedNumbersIntent
        **/
    @Override public android.content.Intent createManageBlockedNumbersIntent() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * @see TelecomServiceImpl#isIncomingCallPermitted
         */
    @Override public boolean isIncomingCallPermitted(android.telecom.PhoneAccountHandle phoneAccountHandle) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * @see TelecomServiceImpl#isOutgoingCallPermitted
         */
    @Override public boolean isOutgoingCallPermitted(android.telecom.PhoneAccountHandle phoneAccountHandle) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * @see TelecomServiceImpl#waitOnHandler
         */
    @Override public void waitOnHandlers() throws android.os.RemoteException
    {
    }
    /**
         * @see TelecomServiceImpl#acceptHandover
         */
    @Override public void acceptHandover(android.net.Uri srcAddr, int videoState, android.telecom.PhoneAccountHandle destAcct) throws android.os.RemoteException
    {
    }
    /**
         * @see TelecomServiceImpl#isInEmergencyCall
         */
    @Override public boolean isInEmergencyCall() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * @see TelecomServiceImpl#handleCallIntent
         */
    @Override public void handleCallIntent(android.content.Intent intent) throws android.os.RemoteException
    {
    }
    @Override public void setTestDefaultCallRedirectionApp(java.lang.String packageName) throws android.os.RemoteException
    {
    }
    @Override public void setTestPhoneAcctSuggestionComponent(java.lang.String flattenedComponentName) throws android.os.RemoteException
    {
    }
    @Override public void setTestDefaultCallScreeningApp(java.lang.String packageName) throws android.os.RemoteException
    {
    }
    @Override public void addOrRemoveTestCallCompanionApp(java.lang.String packageName, boolean isAdded) throws android.os.RemoteException
    {
    }
    @Override public void setTestAutoModeApp(java.lang.String packageName) throws android.os.RemoteException
    {
    }
    /**
         * @see TelecomServiceImpl#setTestDefaultDialer
         */
    @Override public void setTestDefaultDialer(java.lang.String packageName) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.telecom.ITelecomService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.telecom.ITelecomService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.telecom.ITelecomService interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.telecom.ITelecomService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.telecom.ITelecomService))) {
        return ((com.android.internal.telecom.ITelecomService)iin);
      }
      return new com.android.internal.telecom.ITelecomService.Stub.Proxy(obj);
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
        case TRANSACTION_showInCallScreen:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.showInCallScreen(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getDefaultOutgoingPhoneAccount:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.telecom.PhoneAccountHandle _result = this.getDefaultOutgoingPhoneAccount(_arg0, _arg1);
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
        case TRANSACTION_getUserSelectedOutgoingPhoneAccount:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.telecom.PhoneAccountHandle _result = this.getUserSelectedOutgoingPhoneAccount(_arg0);
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
        case TRANSACTION_setUserSelectedOutgoingPhoneAccount:
        {
          data.enforceInterface(descriptor);
          android.telecom.PhoneAccountHandle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setUserSelectedOutgoingPhoneAccount(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getCallCapablePhoneAccounts:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.util.List<android.telecom.PhoneAccountHandle> _result = this.getCallCapablePhoneAccounts(_arg0, _arg1);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getSelfManagedPhoneAccounts:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.List<android.telecom.PhoneAccountHandle> _result = this.getSelfManagedPhoneAccounts(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getPhoneAccountsSupportingScheme:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.util.List<android.telecom.PhoneAccountHandle> _result = this.getPhoneAccountsSupportingScheme(_arg0, _arg1);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getPhoneAccountsForPackage:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.List<android.telecom.PhoneAccountHandle> _result = this.getPhoneAccountsForPackage(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getPhoneAccount:
        {
          data.enforceInterface(descriptor);
          android.telecom.PhoneAccountHandle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.telecom.PhoneAccount _result = this.getPhoneAccount(_arg0);
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
        case TRANSACTION_getAllPhoneAccountsCount:
        {
          data.enforceInterface(descriptor);
          int _result = this.getAllPhoneAccountsCount();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getAllPhoneAccounts:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.telecom.PhoneAccount> _result = this.getAllPhoneAccounts();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getAllPhoneAccountHandles:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.telecom.PhoneAccountHandle> _result = this.getAllPhoneAccountHandles();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getSimCallManager:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telecom.PhoneAccountHandle _result = this.getSimCallManager(_arg0);
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
        case TRANSACTION_getSimCallManagerForUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telecom.PhoneAccountHandle _result = this.getSimCallManagerForUser(_arg0);
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
        case TRANSACTION_registerPhoneAccount:
        {
          data.enforceInterface(descriptor);
          android.telecom.PhoneAccount _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telecom.PhoneAccount.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.registerPhoneAccount(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterPhoneAccount:
        {
          data.enforceInterface(descriptor);
          android.telecom.PhoneAccountHandle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.unregisterPhoneAccount(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_clearAccounts:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.clearAccounts(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isVoiceMailNumber:
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
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _result = this.isVoiceMailNumber(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getVoiceMailNumber:
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
          java.lang.String _result = this.getVoiceMailNumber(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getLine1Number:
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
          java.lang.String _result = this.getLine1Number(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getDefaultPhoneApp:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _result = this.getDefaultPhoneApp();
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
        case TRANSACTION_getDefaultDialerPackage:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getDefaultDialerPackage();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getSystemDialerPackage:
        {
          data.enforceInterface(descriptor);
          java.lang.String _result = this.getSystemDialerPackage();
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_dumpCallAnalytics:
        {
          data.enforceInterface(descriptor);
          android.telecom.TelecomAnalytics _result = this.dumpCallAnalytics();
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
        case TRANSACTION_silenceRinger:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.silenceRinger(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isInCall:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.isInCall(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isInManagedCall:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.isInManagedCall(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isRinging:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.isRinging(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getCallState:
        {
          data.enforceInterface(descriptor);
          int _result = this.getCallState();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_endCall:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.endCall(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_acceptRingingCall:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.acceptRingingCall(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_acceptRingingCallWithVideoState:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.acceptRingingCallWithVideoState(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_cancelMissedCallsNotification:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.cancelMissedCallsNotification(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_handlePinMmi:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.handlePinMmi(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_handlePinMmiForPhoneAccount:
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
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _result = this.handlePinMmiForPhoneAccount(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getAdnUriForPhoneAccount:
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
          android.net.Uri _result = this.getAdnUriForPhoneAccount(_arg0, _arg1);
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
        case TRANSACTION_isTtySupported:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.isTtySupported(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getCurrentTtyMode:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _result = this.getCurrentTtyMode(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_addNewIncomingCall:
        {
          data.enforceInterface(descriptor);
          android.telecom.PhoneAccountHandle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.addNewIncomingCall(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addNewUnknownCall:
        {
          data.enforceInterface(descriptor);
          android.telecom.PhoneAccountHandle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.addNewUnknownCall(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_placeCall:
        {
          data.enforceInterface(descriptor);
          android.net.Uri _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.placeCall(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_enablePhoneAccount:
        {
          data.enforceInterface(descriptor);
          android.telecom.PhoneAccountHandle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          boolean _result = this.enablePhoneAccount(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setDefaultDialer:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.setDefaultDialer(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_createManageBlockedNumbersIntent:
        {
          data.enforceInterface(descriptor);
          android.content.Intent _result = this.createManageBlockedNumbersIntent();
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
        case TRANSACTION_isIncomingCallPermitted:
        {
          data.enforceInterface(descriptor);
          android.telecom.PhoneAccountHandle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.isIncomingCallPermitted(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isOutgoingCallPermitted:
        {
          data.enforceInterface(descriptor);
          android.telecom.PhoneAccountHandle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.isOutgoingCallPermitted(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_waitOnHandlers:
        {
          data.enforceInterface(descriptor);
          this.waitOnHandlers();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_acceptHandover:
        {
          data.enforceInterface(descriptor);
          android.net.Uri _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          android.telecom.PhoneAccountHandle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.acceptHandover(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isInEmergencyCall:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isInEmergencyCall();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_handleCallIntent:
        {
          data.enforceInterface(descriptor);
          android.content.Intent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.handleCallIntent(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setTestDefaultCallRedirectionApp:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.setTestDefaultCallRedirectionApp(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setTestPhoneAcctSuggestionComponent:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.setTestPhoneAcctSuggestionComponent(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setTestDefaultCallScreeningApp:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.setTestDefaultCallScreeningApp(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addOrRemoveTestCallCompanionApp:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.addOrRemoveTestCallCompanionApp(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setTestAutoModeApp:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.setTestAutoModeApp(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setTestDefaultDialer:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.setTestDefaultDialer(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.telecom.ITelecomService
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
           * Brings the in-call screen to the foreground if there is an active call.
           *
           * @param showDialpad if true, make the dialpad visible initially.
           */
      @Override public void showInCallScreen(boolean showDialpad, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((showDialpad)?(1):(0)));
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_showInCallScreen, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().showInCallScreen(showDialpad, callingPackage);
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
           * @see TelecomServiceImpl#getDefaultOutgoingPhoneAccount
           */
      @Override public android.telecom.PhoneAccountHandle getDefaultOutgoingPhoneAccount(java.lang.String uriScheme, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telecom.PhoneAccountHandle _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(uriScheme);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDefaultOutgoingPhoneAccount, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDefaultOutgoingPhoneAccount(uriScheme, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(_reply);
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
           * @see TelecomServiceImpl#getUserSelectedOutgoingPhoneAccount
           */
      @Override public android.telecom.PhoneAccountHandle getUserSelectedOutgoingPhoneAccount(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telecom.PhoneAccountHandle _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getUserSelectedOutgoingPhoneAccount, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getUserSelectedOutgoingPhoneAccount(callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(_reply);
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
           * @see TelecomServiceImpl#setUserSelectedOutgoingPhoneAccount
           */
      @Override public void setUserSelectedOutgoingPhoneAccount(android.telecom.PhoneAccountHandle account) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setUserSelectedOutgoingPhoneAccount, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setUserSelectedOutgoingPhoneAccount(account);
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
           * @see TelecomServiceImpl#getCallCapablePhoneAccounts
           */
      @Override public java.util.List<android.telecom.PhoneAccountHandle> getCallCapablePhoneAccounts(boolean includeDisabledAccounts, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.telecom.PhoneAccountHandle> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((includeDisabledAccounts)?(1):(0)));
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCallCapablePhoneAccounts, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCallCapablePhoneAccounts(includeDisabledAccounts, callingPackage);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.telecom.PhoneAccountHandle.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * @see TelecomServiceImpl#getSelfManagedPhoneAccounts
           */
      @Override public java.util.List<android.telecom.PhoneAccountHandle> getSelfManagedPhoneAccounts(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.telecom.PhoneAccountHandle> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSelfManagedPhoneAccounts, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSelfManagedPhoneAccounts(callingPackage);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.telecom.PhoneAccountHandle.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * @see TelecomManager#getPhoneAccountsSupportingScheme
           */
      @Override public java.util.List<android.telecom.PhoneAccountHandle> getPhoneAccountsSupportingScheme(java.lang.String uriScheme, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.telecom.PhoneAccountHandle> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(uriScheme);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPhoneAccountsSupportingScheme, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPhoneAccountsSupportingScheme(uriScheme, callingPackage);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.telecom.PhoneAccountHandle.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * @see TelecomManager#getPhoneAccountsForPackage
           */
      @Override public java.util.List<android.telecom.PhoneAccountHandle> getPhoneAccountsForPackage(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.telecom.PhoneAccountHandle> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPhoneAccountsForPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPhoneAccountsForPackage(packageName);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.telecom.PhoneAccountHandle.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * @see TelecomManager#getPhoneAccount
           */
      @Override public android.telecom.PhoneAccount getPhoneAccount(android.telecom.PhoneAccountHandle account) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telecom.PhoneAccount _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPhoneAccount, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPhoneAccount(account);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telecom.PhoneAccount.CREATOR.createFromParcel(_reply);
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
           * @see TelecomManager#getAllPhoneAccountsCount
           */
      @Override public int getAllPhoneAccountsCount() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAllPhoneAccountsCount, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAllPhoneAccountsCount();
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
           * @see TelecomManager#getAllPhoneAccounts
           */
      @Override public java.util.List<android.telecom.PhoneAccount> getAllPhoneAccounts() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.telecom.PhoneAccount> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAllPhoneAccounts, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAllPhoneAccounts();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.telecom.PhoneAccount.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * @see TelecomManager#getAllPhoneAccountHandles
           */
      @Override public java.util.List<android.telecom.PhoneAccountHandle> getAllPhoneAccountHandles() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.telecom.PhoneAccountHandle> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAllPhoneAccountHandles, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAllPhoneAccountHandles();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.telecom.PhoneAccountHandle.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * @see TelecomServiceImpl#getSimCallManager
           */
      @Override public android.telecom.PhoneAccountHandle getSimCallManager(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telecom.PhoneAccountHandle _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSimCallManager, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSimCallManager(subId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(_reply);
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
           * @see TelecomServiceImpl#getSimCallManagerForUser
           */
      @Override public android.telecom.PhoneAccountHandle getSimCallManagerForUser(int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telecom.PhoneAccountHandle _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSimCallManagerForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSimCallManagerForUser(userId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(_reply);
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
           * @see TelecomServiceImpl#registerPhoneAccount
           */
      @Override public void registerPhoneAccount(android.telecom.PhoneAccount metadata) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((metadata!=null)) {
            _data.writeInt(1);
            metadata.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerPhoneAccount, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerPhoneAccount(metadata);
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
           * @see TelecomServiceImpl#unregisterPhoneAccount
           */
      @Override public void unregisterPhoneAccount(android.telecom.PhoneAccountHandle account) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((account!=null)) {
            _data.writeInt(1);
            account.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterPhoneAccount, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterPhoneAccount(account);
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
           * @see TelecomServiceImpl#clearAccounts
           */
      @Override public void clearAccounts(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearAccounts, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearAccounts(packageName);
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
           * @see TelecomServiceImpl#isVoiceMailNumber
           */
      @Override public boolean isVoiceMailNumber(android.telecom.PhoneAccountHandle accountHandle, java.lang.String number, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((accountHandle!=null)) {
            _data.writeInt(1);
            accountHandle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(number);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isVoiceMailNumber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isVoiceMailNumber(accountHandle, number, callingPackage);
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
           * @see TelecomServiceImpl#getVoiceMailNumber
           */
      @Override public java.lang.String getVoiceMailNumber(android.telecom.PhoneAccountHandle accountHandle, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((accountHandle!=null)) {
            _data.writeInt(1);
            accountHandle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getVoiceMailNumber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getVoiceMailNumber(accountHandle, callingPackage);
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
      /**
           * @see TelecomServiceImpl#getLine1Number
           */
      @Override public java.lang.String getLine1Number(android.telecom.PhoneAccountHandle accountHandle, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((accountHandle!=null)) {
            _data.writeInt(1);
            accountHandle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getLine1Number, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getLine1Number(accountHandle, callingPackage);
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
      /**
           * @see TelecomServiceImpl#getDefaultPhoneApp
           */
      @Override public android.content.ComponentName getDefaultPhoneApp() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.ComponentName _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDefaultPhoneApp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDefaultPhoneApp();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.ComponentName.CREATOR.createFromParcel(_reply);
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
           * @see TelecomServiceImpl#getDefaultDialerPackage
           */
      @Override public java.lang.String getDefaultDialerPackage() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDefaultDialerPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDefaultDialerPackage();
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
      /**
           * @see TelecomServiceImpl#getSystemDialerPackage
           */
      @Override public java.lang.String getSystemDialerPackage() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSystemDialerPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSystemDialerPackage();
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
      /**
          * @see TelecomServiceImpl#dumpCallAnalytics
          */
      @Override public android.telecom.TelecomAnalytics dumpCallAnalytics() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telecom.TelecomAnalytics _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_dumpCallAnalytics, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().dumpCallAnalytics();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telecom.TelecomAnalytics.CREATOR.createFromParcel(_reply);
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
      //
      // Internal system apis relating to call management.
      //
      /**
           * @see TelecomServiceImpl#silenceRinger
           */
      @Override public void silenceRinger(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_silenceRinger, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().silenceRinger(callingPackage);
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
           * @see TelecomServiceImpl#isInCall
           */
      @Override public boolean isInCall(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isInCall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isInCall(callingPackage);
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
           * @see TelecomServiceImpl#isInManagedCall
           */
      @Override public boolean isInManagedCall(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isInManagedCall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isInManagedCall(callingPackage);
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
           * @see TelecomServiceImpl#isRinging
           */
      @Override public boolean isRinging(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isRinging, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isRinging(callingPackage);
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
           * @see TelecomServiceImpl#getCallState
           */
      @Override public int getCallState() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCallState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCallState();
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
           * @see TelecomServiceImpl#endCall
           */
      @Override public boolean endCall(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_endCall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().endCall(callingPackage);
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
           * @see TelecomServiceImpl#acceptRingingCall
           */
      @Override public void acceptRingingCall(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_acceptRingingCall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().acceptRingingCall(callingPackage);
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
           * @see TelecomServiceImpl#acceptRingingCallWithVideoState(int)
           */
      @Override public void acceptRingingCallWithVideoState(java.lang.String callingPackage, int videoState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(videoState);
          boolean _status = mRemote.transact(Stub.TRANSACTION_acceptRingingCallWithVideoState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().acceptRingingCallWithVideoState(callingPackage, videoState);
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
           * @see TelecomServiceImpl#cancelMissedCallsNotification
           */
      @Override public void cancelMissedCallsNotification(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelMissedCallsNotification, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelMissedCallsNotification(callingPackage);
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
           * @see TelecomServiceImpl#handleMmi
           */
      @Override public boolean handlePinMmi(java.lang.String dialString, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(dialString);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_handlePinMmi, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().handlePinMmi(dialString, callingPackage);
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
           * @see TelecomServiceImpl#handleMmi
           */
      @Override public boolean handlePinMmiForPhoneAccount(android.telecom.PhoneAccountHandle accountHandle, java.lang.String dialString, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((accountHandle!=null)) {
            _data.writeInt(1);
            accountHandle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(dialString);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_handlePinMmiForPhoneAccount, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().handlePinMmiForPhoneAccount(accountHandle, dialString, callingPackage);
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
           * @see TelecomServiceImpl#getAdnUriForPhoneAccount
           */
      @Override public android.net.Uri getAdnUriForPhoneAccount(android.telecom.PhoneAccountHandle accountHandle, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.Uri _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((accountHandle!=null)) {
            _data.writeInt(1);
            accountHandle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAdnUriForPhoneAccount, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAdnUriForPhoneAccount(accountHandle, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.Uri.CREATOR.createFromParcel(_reply);
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
           * @see TelecomServiceImpl#isTtySupported
           */
      @Override public boolean isTtySupported(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isTtySupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isTtySupported(callingPackage);
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
           * @see TelecomServiceImpl#getCurrentTtyMode
           */
      @Override public int getCurrentTtyMode(java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentTtyMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentTtyMode(callingPackage);
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
           * @see TelecomServiceImpl#addNewIncomingCall
           */
      @Override public void addNewIncomingCall(android.telecom.PhoneAccountHandle phoneAccount, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((phoneAccount!=null)) {
            _data.writeInt(1);
            phoneAccount.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_addNewIncomingCall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addNewIncomingCall(phoneAccount, extras);
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
           * @see TelecomServiceImpl#addNewUnknownCall
           */
      @Override public void addNewUnknownCall(android.telecom.PhoneAccountHandle phoneAccount, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((phoneAccount!=null)) {
            _data.writeInt(1);
            phoneAccount.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_addNewUnknownCall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addNewUnknownCall(phoneAccount, extras);
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
           * @see TelecomServiceImpl#placeCall
           */
      @Override public void placeCall(android.net.Uri handle, android.os.Bundle extras, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((handle!=null)) {
            _data.writeInt(1);
            handle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_placeCall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().placeCall(handle, extras, callingPackage);
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
           * @see TelecomServiceImpl#enablePhoneAccount
           */
      @Override public boolean enablePhoneAccount(android.telecom.PhoneAccountHandle accountHandle, boolean isEnabled) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((accountHandle!=null)) {
            _data.writeInt(1);
            accountHandle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((isEnabled)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_enablePhoneAccount, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().enablePhoneAccount(accountHandle, isEnabled);
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
           * @see TelecomServiceImpl#setDefaultDialer
           */
      @Override public boolean setDefaultDialer(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDefaultDialer, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setDefaultDialer(packageName);
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
          * @see TelecomServiceImpl#createManageBlockedNumbersIntent
          **/
      @Override public android.content.Intent createManageBlockedNumbersIntent() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.Intent _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createManageBlockedNumbersIntent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createManageBlockedNumbersIntent();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.Intent.CREATOR.createFromParcel(_reply);
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
           * @see TelecomServiceImpl#isIncomingCallPermitted
           */
      @Override public boolean isIncomingCallPermitted(android.telecom.PhoneAccountHandle phoneAccountHandle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((phoneAccountHandle!=null)) {
            _data.writeInt(1);
            phoneAccountHandle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_isIncomingCallPermitted, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isIncomingCallPermitted(phoneAccountHandle);
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
           * @see TelecomServiceImpl#isOutgoingCallPermitted
           */
      @Override public boolean isOutgoingCallPermitted(android.telecom.PhoneAccountHandle phoneAccountHandle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((phoneAccountHandle!=null)) {
            _data.writeInt(1);
            phoneAccountHandle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_isOutgoingCallPermitted, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isOutgoingCallPermitted(phoneAccountHandle);
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
           * @see TelecomServiceImpl#waitOnHandler
           */
      @Override public void waitOnHandlers() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_waitOnHandlers, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().waitOnHandlers();
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
           * @see TelecomServiceImpl#acceptHandover
           */
      @Override public void acceptHandover(android.net.Uri srcAddr, int videoState, android.telecom.PhoneAccountHandle destAcct) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((srcAddr!=null)) {
            _data.writeInt(1);
            srcAddr.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(videoState);
          if ((destAcct!=null)) {
            _data.writeInt(1);
            destAcct.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_acceptHandover, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().acceptHandover(srcAddr, videoState, destAcct);
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
           * @see TelecomServiceImpl#isInEmergencyCall
           */
      @Override public boolean isInEmergencyCall() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isInEmergencyCall, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isInEmergencyCall();
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
           * @see TelecomServiceImpl#handleCallIntent
           */
      @Override public void handleCallIntent(android.content.Intent intent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((intent!=null)) {
            _data.writeInt(1);
            intent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_handleCallIntent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().handleCallIntent(intent);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setTestDefaultCallRedirectionApp(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTestDefaultCallRedirectionApp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setTestDefaultCallRedirectionApp(packageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setTestPhoneAcctSuggestionComponent(java.lang.String flattenedComponentName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(flattenedComponentName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTestPhoneAcctSuggestionComponent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setTestPhoneAcctSuggestionComponent(flattenedComponentName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setTestDefaultCallScreeningApp(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTestDefaultCallScreeningApp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setTestDefaultCallScreeningApp(packageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void addOrRemoveTestCallCompanionApp(java.lang.String packageName, boolean isAdded) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(((isAdded)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addOrRemoveTestCallCompanionApp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addOrRemoveTestCallCompanionApp(packageName, isAdded);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setTestAutoModeApp(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTestAutoModeApp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setTestAutoModeApp(packageName);
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
           * @see TelecomServiceImpl#setTestDefaultDialer
           */
      @Override public void setTestDefaultDialer(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setTestDefaultDialer, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setTestDefaultDialer(packageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.internal.telecom.ITelecomService sDefaultImpl;
    }
    static final int TRANSACTION_showInCallScreen = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getDefaultOutgoingPhoneAccount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getUserSelectedOutgoingPhoneAccount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_setUserSelectedOutgoingPhoneAccount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getCallCapablePhoneAccounts = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getSelfManagedPhoneAccounts = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_getPhoneAccountsSupportingScheme = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_getPhoneAccountsForPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_getPhoneAccount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getAllPhoneAccountsCount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_getAllPhoneAccounts = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_getAllPhoneAccountHandles = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_getSimCallManager = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_getSimCallManagerForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_registerPhoneAccount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_unregisterPhoneAccount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_clearAccounts = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_isVoiceMailNumber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_getVoiceMailNumber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_getLine1Number = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_getDefaultPhoneApp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_getDefaultDialerPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_getSystemDialerPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_dumpCallAnalytics = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_silenceRinger = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_isInCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_isInManagedCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_isRinging = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_getCallState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_endCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_acceptRingingCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_acceptRingingCallWithVideoState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_cancelMissedCallsNotification = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_handlePinMmi = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_handlePinMmiForPhoneAccount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_getAdnUriForPhoneAccount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    static final int TRANSACTION_isTtySupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 36);
    static final int TRANSACTION_getCurrentTtyMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 37);
    static final int TRANSACTION_addNewIncomingCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 38);
    static final int TRANSACTION_addNewUnknownCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 39);
    static final int TRANSACTION_placeCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 40);
    static final int TRANSACTION_enablePhoneAccount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 41);
    static final int TRANSACTION_setDefaultDialer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 42);
    static final int TRANSACTION_createManageBlockedNumbersIntent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 43);
    static final int TRANSACTION_isIncomingCallPermitted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 44);
    static final int TRANSACTION_isOutgoingCallPermitted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 45);
    static final int TRANSACTION_waitOnHandlers = (android.os.IBinder.FIRST_CALL_TRANSACTION + 46);
    static final int TRANSACTION_acceptHandover = (android.os.IBinder.FIRST_CALL_TRANSACTION + 47);
    static final int TRANSACTION_isInEmergencyCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 48);
    static final int TRANSACTION_handleCallIntent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 49);
    static final int TRANSACTION_setTestDefaultCallRedirectionApp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 50);
    static final int TRANSACTION_setTestPhoneAcctSuggestionComponent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 51);
    static final int TRANSACTION_setTestDefaultCallScreeningApp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 52);
    static final int TRANSACTION_addOrRemoveTestCallCompanionApp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 53);
    static final int TRANSACTION_setTestAutoModeApp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 54);
    static final int TRANSACTION_setTestDefaultDialer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 55);
    public static boolean setDefaultImpl(com.android.internal.telecom.ITelecomService impl) {
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
    public static com.android.internal.telecom.ITelecomService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Brings the in-call screen to the foreground if there is an active call.
       *
       * @param showDialpad if true, make the dialpad visible initially.
       */
  public void showInCallScreen(boolean showDialpad, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#getDefaultOutgoingPhoneAccount
       */
  public android.telecom.PhoneAccountHandle getDefaultOutgoingPhoneAccount(java.lang.String uriScheme, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#getUserSelectedOutgoingPhoneAccount
       */
  public android.telecom.PhoneAccountHandle getUserSelectedOutgoingPhoneAccount(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#setUserSelectedOutgoingPhoneAccount
       */
  public void setUserSelectedOutgoingPhoneAccount(android.telecom.PhoneAccountHandle account) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#getCallCapablePhoneAccounts
       */
  public java.util.List<android.telecom.PhoneAccountHandle> getCallCapablePhoneAccounts(boolean includeDisabledAccounts, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#getSelfManagedPhoneAccounts
       */
  public java.util.List<android.telecom.PhoneAccountHandle> getSelfManagedPhoneAccounts(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see TelecomManager#getPhoneAccountsSupportingScheme
       */
  public java.util.List<android.telecom.PhoneAccountHandle> getPhoneAccountsSupportingScheme(java.lang.String uriScheme, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see TelecomManager#getPhoneAccountsForPackage
       */
  public java.util.List<android.telecom.PhoneAccountHandle> getPhoneAccountsForPackage(java.lang.String packageName) throws android.os.RemoteException;
  /**
       * @see TelecomManager#getPhoneAccount
       */
  public android.telecom.PhoneAccount getPhoneAccount(android.telecom.PhoneAccountHandle account) throws android.os.RemoteException;
  /**
       * @see TelecomManager#getAllPhoneAccountsCount
       */
  public int getAllPhoneAccountsCount() throws android.os.RemoteException;
  /**
       * @see TelecomManager#getAllPhoneAccounts
       */
  public java.util.List<android.telecom.PhoneAccount> getAllPhoneAccounts() throws android.os.RemoteException;
  /**
       * @see TelecomManager#getAllPhoneAccountHandles
       */
  public java.util.List<android.telecom.PhoneAccountHandle> getAllPhoneAccountHandles() throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#getSimCallManager
       */
  public android.telecom.PhoneAccountHandle getSimCallManager(int subId) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#getSimCallManagerForUser
       */
  public android.telecom.PhoneAccountHandle getSimCallManagerForUser(int userId) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#registerPhoneAccount
       */
  public void registerPhoneAccount(android.telecom.PhoneAccount metadata) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#unregisterPhoneAccount
       */
  public void unregisterPhoneAccount(android.telecom.PhoneAccountHandle account) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#clearAccounts
       */
  public void clearAccounts(java.lang.String packageName) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#isVoiceMailNumber
       */
  public boolean isVoiceMailNumber(android.telecom.PhoneAccountHandle accountHandle, java.lang.String number, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#getVoiceMailNumber
       */
  public java.lang.String getVoiceMailNumber(android.telecom.PhoneAccountHandle accountHandle, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#getLine1Number
       */
  public java.lang.String getLine1Number(android.telecom.PhoneAccountHandle accountHandle, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#getDefaultPhoneApp
       */
  public android.content.ComponentName getDefaultPhoneApp() throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#getDefaultDialerPackage
       */
  public java.lang.String getDefaultDialerPackage() throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#getSystemDialerPackage
       */
  public java.lang.String getSystemDialerPackage() throws android.os.RemoteException;
  /**
      * @see TelecomServiceImpl#dumpCallAnalytics
      */
  public android.telecom.TelecomAnalytics dumpCallAnalytics() throws android.os.RemoteException;
  //
  // Internal system apis relating to call management.
  //
  /**
       * @see TelecomServiceImpl#silenceRinger
       */
  public void silenceRinger(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#isInCall
       */
  public boolean isInCall(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#isInManagedCall
       */
  public boolean isInManagedCall(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#isRinging
       */
  public boolean isRinging(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#getCallState
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telecomm/java/com/android/internal/telecom/ITelecomService.aidl:185:1:185:25")
  public int getCallState() throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#endCall
       */
  public boolean endCall(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#acceptRingingCall
       */
  public void acceptRingingCall(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#acceptRingingCallWithVideoState(int)
       */
  public void acceptRingingCallWithVideoState(java.lang.String callingPackage, int videoState) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#cancelMissedCallsNotification
       */
  public void cancelMissedCallsNotification(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#handleMmi
       */
  public boolean handlePinMmi(java.lang.String dialString, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#handleMmi
       */
  public boolean handlePinMmiForPhoneAccount(android.telecom.PhoneAccountHandle accountHandle, java.lang.String dialString, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#getAdnUriForPhoneAccount
       */
  public android.net.Uri getAdnUriForPhoneAccount(android.telecom.PhoneAccountHandle accountHandle, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#isTtySupported
       */
  public boolean isTtySupported(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#getCurrentTtyMode
       */
  public int getCurrentTtyMode(java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#addNewIncomingCall
       */
  public void addNewIncomingCall(android.telecom.PhoneAccountHandle phoneAccount, android.os.Bundle extras) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#addNewUnknownCall
       */
  public void addNewUnknownCall(android.telecom.PhoneAccountHandle phoneAccount, android.os.Bundle extras) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#placeCall
       */
  public void placeCall(android.net.Uri handle, android.os.Bundle extras, java.lang.String callingPackage) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#enablePhoneAccount
       */
  public boolean enablePhoneAccount(android.telecom.PhoneAccountHandle accountHandle, boolean isEnabled) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#setDefaultDialer
       */
  public boolean setDefaultDialer(java.lang.String packageName) throws android.os.RemoteException;
  /**
      * @see TelecomServiceImpl#createManageBlockedNumbersIntent
      **/
  public android.content.Intent createManageBlockedNumbersIntent() throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#isIncomingCallPermitted
       */
  public boolean isIncomingCallPermitted(android.telecom.PhoneAccountHandle phoneAccountHandle) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#isOutgoingCallPermitted
       */
  public boolean isOutgoingCallPermitted(android.telecom.PhoneAccountHandle phoneAccountHandle) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#waitOnHandler
       */
  public void waitOnHandlers() throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#acceptHandover
       */
  public void acceptHandover(android.net.Uri srcAddr, int videoState, android.telecom.PhoneAccountHandle destAcct) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#isInEmergencyCall
       */
  public boolean isInEmergencyCall() throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#handleCallIntent
       */
  public void handleCallIntent(android.content.Intent intent) throws android.os.RemoteException;
  public void setTestDefaultCallRedirectionApp(java.lang.String packageName) throws android.os.RemoteException;
  public void setTestPhoneAcctSuggestionComponent(java.lang.String flattenedComponentName) throws android.os.RemoteException;
  public void setTestDefaultCallScreeningApp(java.lang.String packageName) throws android.os.RemoteException;
  public void addOrRemoveTestCallCompanionApp(java.lang.String packageName, boolean isAdded) throws android.os.RemoteException;
  public void setTestAutoModeApp(java.lang.String packageName) throws android.os.RemoteException;
  /**
       * @see TelecomServiceImpl#setTestDefaultDialer
       */
  public void setTestDefaultDialer(java.lang.String packageName) throws android.os.RemoteException;
}
