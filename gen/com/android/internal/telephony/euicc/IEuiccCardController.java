/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.telephony.euicc;
/** @hide */
public interface IEuiccCardController extends android.os.IInterface
{
  /** Default implementation for IEuiccCardController. */
  public static class Default implements com.android.internal.telephony.euicc.IEuiccCardController
  {
    @Override public void getAllProfiles(java.lang.String callingPackage, java.lang.String cardId, com.android.internal.telephony.euicc.IGetAllProfilesCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void getProfile(java.lang.String callingPackage, java.lang.String cardId, java.lang.String iccid, com.android.internal.telephony.euicc.IGetProfileCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void disableProfile(java.lang.String callingPackage, java.lang.String cardId, java.lang.String iccid, boolean refresh, com.android.internal.telephony.euicc.IDisableProfileCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void switchToProfile(java.lang.String callingPackage, java.lang.String cardId, java.lang.String iccid, boolean refresh, com.android.internal.telephony.euicc.ISwitchToProfileCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void setNickname(java.lang.String callingPackage, java.lang.String cardId, java.lang.String iccid, java.lang.String nickname, com.android.internal.telephony.euicc.ISetNicknameCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void deleteProfile(java.lang.String callingPackage, java.lang.String cardId, java.lang.String iccid, com.android.internal.telephony.euicc.IDeleteProfileCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void resetMemory(java.lang.String callingPackage, java.lang.String cardId, int options, com.android.internal.telephony.euicc.IResetMemoryCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void getDefaultSmdpAddress(java.lang.String callingPackage, java.lang.String cardId, com.android.internal.telephony.euicc.IGetDefaultSmdpAddressCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void getSmdsAddress(java.lang.String callingPackage, java.lang.String cardId, com.android.internal.telephony.euicc.IGetSmdsAddressCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void setDefaultSmdpAddress(java.lang.String callingPackage, java.lang.String cardId, java.lang.String address, com.android.internal.telephony.euicc.ISetDefaultSmdpAddressCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void getRulesAuthTable(java.lang.String callingPackage, java.lang.String cardId, com.android.internal.telephony.euicc.IGetRulesAuthTableCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void getEuiccChallenge(java.lang.String callingPackage, java.lang.String cardId, com.android.internal.telephony.euicc.IGetEuiccChallengeCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void getEuiccInfo1(java.lang.String callingPackage, java.lang.String cardId, com.android.internal.telephony.euicc.IGetEuiccInfo1Callback callback) throws android.os.RemoteException
    {
    }
    @Override public void getEuiccInfo2(java.lang.String callingPackage, java.lang.String cardId, com.android.internal.telephony.euicc.IGetEuiccInfo2Callback callback) throws android.os.RemoteException
    {
    }
    @Override public void authenticateServer(java.lang.String callingPackage, java.lang.String cardId, java.lang.String matchingId, byte[] serverSigned1, byte[] serverSignature1, byte[] euiccCiPkIdToBeUsed, byte[] serverCertificatein, com.android.internal.telephony.euicc.IAuthenticateServerCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void prepareDownload(java.lang.String callingPackage, java.lang.String cardId, byte[] hashCc, byte[] smdpSigned2, byte[] smdpSignature2, byte[] smdpCertificate, com.android.internal.telephony.euicc.IPrepareDownloadCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void loadBoundProfilePackage(java.lang.String callingPackage, java.lang.String cardId, byte[] boundProfilePackage, com.android.internal.telephony.euicc.ILoadBoundProfilePackageCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void cancelSession(java.lang.String callingPackage, java.lang.String cardId, byte[] transactionId, int reason, com.android.internal.telephony.euicc.ICancelSessionCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void listNotifications(java.lang.String callingPackage, java.lang.String cardId, int events, com.android.internal.telephony.euicc.IListNotificationsCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void retrieveNotificationList(java.lang.String callingPackage, java.lang.String cardId, int events, com.android.internal.telephony.euicc.IRetrieveNotificationListCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void retrieveNotification(java.lang.String callingPackage, java.lang.String cardId, int seqNumber, com.android.internal.telephony.euicc.IRetrieveNotificationCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void removeNotificationFromList(java.lang.String callingPackage, java.lang.String cardId, int seqNumber, com.android.internal.telephony.euicc.IRemoveNotificationFromListCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.telephony.euicc.IEuiccCardController
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.telephony.euicc.IEuiccCardController";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.telephony.euicc.IEuiccCardController interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.telephony.euicc.IEuiccCardController asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.telephony.euicc.IEuiccCardController))) {
        return ((com.android.internal.telephony.euicc.IEuiccCardController)iin);
      }
      return new com.android.internal.telephony.euicc.IEuiccCardController.Stub.Proxy(obj);
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
        case TRANSACTION_getAllProfiles:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          com.android.internal.telephony.euicc.IGetAllProfilesCallback _arg2;
          _arg2 = com.android.internal.telephony.euicc.IGetAllProfilesCallback.Stub.asInterface(data.readStrongBinder());
          this.getAllProfiles(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_getProfile:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          com.android.internal.telephony.euicc.IGetProfileCallback _arg3;
          _arg3 = com.android.internal.telephony.euicc.IGetProfileCallback.Stub.asInterface(data.readStrongBinder());
          this.getProfile(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_disableProfile:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          com.android.internal.telephony.euicc.IDisableProfileCallback _arg4;
          _arg4 = com.android.internal.telephony.euicc.IDisableProfileCallback.Stub.asInterface(data.readStrongBinder());
          this.disableProfile(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_switchToProfile:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          com.android.internal.telephony.euicc.ISwitchToProfileCallback _arg4;
          _arg4 = com.android.internal.telephony.euicc.ISwitchToProfileCallback.Stub.asInterface(data.readStrongBinder());
          this.switchToProfile(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_setNickname:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String _arg3;
          _arg3 = data.readString();
          com.android.internal.telephony.euicc.ISetNicknameCallback _arg4;
          _arg4 = com.android.internal.telephony.euicc.ISetNicknameCallback.Stub.asInterface(data.readStrongBinder());
          this.setNickname(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_deleteProfile:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          com.android.internal.telephony.euicc.IDeleteProfileCallback _arg3;
          _arg3 = com.android.internal.telephony.euicc.IDeleteProfileCallback.Stub.asInterface(data.readStrongBinder());
          this.deleteProfile(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_resetMemory:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          com.android.internal.telephony.euicc.IResetMemoryCallback _arg3;
          _arg3 = com.android.internal.telephony.euicc.IResetMemoryCallback.Stub.asInterface(data.readStrongBinder());
          this.resetMemory(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_getDefaultSmdpAddress:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          com.android.internal.telephony.euicc.IGetDefaultSmdpAddressCallback _arg2;
          _arg2 = com.android.internal.telephony.euicc.IGetDefaultSmdpAddressCallback.Stub.asInterface(data.readStrongBinder());
          this.getDefaultSmdpAddress(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_getSmdsAddress:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          com.android.internal.telephony.euicc.IGetSmdsAddressCallback _arg2;
          _arg2 = com.android.internal.telephony.euicc.IGetSmdsAddressCallback.Stub.asInterface(data.readStrongBinder());
          this.getSmdsAddress(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_setDefaultSmdpAddress:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          com.android.internal.telephony.euicc.ISetDefaultSmdpAddressCallback _arg3;
          _arg3 = com.android.internal.telephony.euicc.ISetDefaultSmdpAddressCallback.Stub.asInterface(data.readStrongBinder());
          this.setDefaultSmdpAddress(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_getRulesAuthTable:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          com.android.internal.telephony.euicc.IGetRulesAuthTableCallback _arg2;
          _arg2 = com.android.internal.telephony.euicc.IGetRulesAuthTableCallback.Stub.asInterface(data.readStrongBinder());
          this.getRulesAuthTable(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_getEuiccChallenge:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          com.android.internal.telephony.euicc.IGetEuiccChallengeCallback _arg2;
          _arg2 = com.android.internal.telephony.euicc.IGetEuiccChallengeCallback.Stub.asInterface(data.readStrongBinder());
          this.getEuiccChallenge(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_getEuiccInfo1:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          com.android.internal.telephony.euicc.IGetEuiccInfo1Callback _arg2;
          _arg2 = com.android.internal.telephony.euicc.IGetEuiccInfo1Callback.Stub.asInterface(data.readStrongBinder());
          this.getEuiccInfo1(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_getEuiccInfo2:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          com.android.internal.telephony.euicc.IGetEuiccInfo2Callback _arg2;
          _arg2 = com.android.internal.telephony.euicc.IGetEuiccInfo2Callback.Stub.asInterface(data.readStrongBinder());
          this.getEuiccInfo2(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_authenticateServer:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          byte[] _arg3;
          _arg3 = data.createByteArray();
          byte[] _arg4;
          _arg4 = data.createByteArray();
          byte[] _arg5;
          _arg5 = data.createByteArray();
          byte[] _arg6;
          _arg6 = data.createByteArray();
          com.android.internal.telephony.euicc.IAuthenticateServerCallback _arg7;
          _arg7 = com.android.internal.telephony.euicc.IAuthenticateServerCallback.Stub.asInterface(data.readStrongBinder());
          this.authenticateServer(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7);
          return true;
        }
        case TRANSACTION_prepareDownload:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          byte[] _arg2;
          _arg2 = data.createByteArray();
          byte[] _arg3;
          _arg3 = data.createByteArray();
          byte[] _arg4;
          _arg4 = data.createByteArray();
          byte[] _arg5;
          _arg5 = data.createByteArray();
          com.android.internal.telephony.euicc.IPrepareDownloadCallback _arg6;
          _arg6 = com.android.internal.telephony.euicc.IPrepareDownloadCallback.Stub.asInterface(data.readStrongBinder());
          this.prepareDownload(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          return true;
        }
        case TRANSACTION_loadBoundProfilePackage:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          byte[] _arg2;
          _arg2 = data.createByteArray();
          com.android.internal.telephony.euicc.ILoadBoundProfilePackageCallback _arg3;
          _arg3 = com.android.internal.telephony.euicc.ILoadBoundProfilePackageCallback.Stub.asInterface(data.readStrongBinder());
          this.loadBoundProfilePackage(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_cancelSession:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          byte[] _arg2;
          _arg2 = data.createByteArray();
          int _arg3;
          _arg3 = data.readInt();
          com.android.internal.telephony.euicc.ICancelSessionCallback _arg4;
          _arg4 = com.android.internal.telephony.euicc.ICancelSessionCallback.Stub.asInterface(data.readStrongBinder());
          this.cancelSession(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_listNotifications:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          com.android.internal.telephony.euicc.IListNotificationsCallback _arg3;
          _arg3 = com.android.internal.telephony.euicc.IListNotificationsCallback.Stub.asInterface(data.readStrongBinder());
          this.listNotifications(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_retrieveNotificationList:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          com.android.internal.telephony.euicc.IRetrieveNotificationListCallback _arg3;
          _arg3 = com.android.internal.telephony.euicc.IRetrieveNotificationListCallback.Stub.asInterface(data.readStrongBinder());
          this.retrieveNotificationList(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_retrieveNotification:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          com.android.internal.telephony.euicc.IRetrieveNotificationCallback _arg3;
          _arg3 = com.android.internal.telephony.euicc.IRetrieveNotificationCallback.Stub.asInterface(data.readStrongBinder());
          this.retrieveNotification(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_removeNotificationFromList:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          com.android.internal.telephony.euicc.IRemoveNotificationFromListCallback _arg3;
          _arg3 = com.android.internal.telephony.euicc.IRemoveNotificationFromListCallback.Stub.asInterface(data.readStrongBinder());
          this.removeNotificationFromList(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.telephony.euicc.IEuiccCardController
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
      @Override public void getAllProfiles(java.lang.String callingPackage, java.lang.String cardId, com.android.internal.telephony.euicc.IGetAllProfilesCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(cardId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAllProfiles, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getAllProfiles(callingPackage, cardId, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getProfile(java.lang.String callingPackage, java.lang.String cardId, java.lang.String iccid, com.android.internal.telephony.euicc.IGetProfileCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(cardId);
          _data.writeString(iccid);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getProfile, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getProfile(callingPackage, cardId, iccid, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void disableProfile(java.lang.String callingPackage, java.lang.String cardId, java.lang.String iccid, boolean refresh, com.android.internal.telephony.euicc.IDisableProfileCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(cardId);
          _data.writeString(iccid);
          _data.writeInt(((refresh)?(1):(0)));
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableProfile, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableProfile(callingPackage, cardId, iccid, refresh, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void switchToProfile(java.lang.String callingPackage, java.lang.String cardId, java.lang.String iccid, boolean refresh, com.android.internal.telephony.euicc.ISwitchToProfileCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(cardId);
          _data.writeString(iccid);
          _data.writeInt(((refresh)?(1):(0)));
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_switchToProfile, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().switchToProfile(callingPackage, cardId, iccid, refresh, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setNickname(java.lang.String callingPackage, java.lang.String cardId, java.lang.String iccid, java.lang.String nickname, com.android.internal.telephony.euicc.ISetNicknameCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(cardId);
          _data.writeString(iccid);
          _data.writeString(nickname);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setNickname, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setNickname(callingPackage, cardId, iccid, nickname, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void deleteProfile(java.lang.String callingPackage, java.lang.String cardId, java.lang.String iccid, com.android.internal.telephony.euicc.IDeleteProfileCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(cardId);
          _data.writeString(iccid);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_deleteProfile, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deleteProfile(callingPackage, cardId, iccid, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void resetMemory(java.lang.String callingPackage, java.lang.String cardId, int options, com.android.internal.telephony.euicc.IResetMemoryCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(cardId);
          _data.writeInt(options);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_resetMemory, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().resetMemory(callingPackage, cardId, options, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getDefaultSmdpAddress(java.lang.String callingPackage, java.lang.String cardId, com.android.internal.telephony.euicc.IGetDefaultSmdpAddressCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(cardId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDefaultSmdpAddress, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getDefaultSmdpAddress(callingPackage, cardId, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getSmdsAddress(java.lang.String callingPackage, java.lang.String cardId, com.android.internal.telephony.euicc.IGetSmdsAddressCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(cardId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSmdsAddress, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getSmdsAddress(callingPackage, cardId, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setDefaultSmdpAddress(java.lang.String callingPackage, java.lang.String cardId, java.lang.String address, com.android.internal.telephony.euicc.ISetDefaultSmdpAddressCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(cardId);
          _data.writeString(address);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDefaultSmdpAddress, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDefaultSmdpAddress(callingPackage, cardId, address, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getRulesAuthTable(java.lang.String callingPackage, java.lang.String cardId, com.android.internal.telephony.euicc.IGetRulesAuthTableCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(cardId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRulesAuthTable, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getRulesAuthTable(callingPackage, cardId, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getEuiccChallenge(java.lang.String callingPackage, java.lang.String cardId, com.android.internal.telephony.euicc.IGetEuiccChallengeCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(cardId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getEuiccChallenge, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getEuiccChallenge(callingPackage, cardId, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getEuiccInfo1(java.lang.String callingPackage, java.lang.String cardId, com.android.internal.telephony.euicc.IGetEuiccInfo1Callback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(cardId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getEuiccInfo1, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getEuiccInfo1(callingPackage, cardId, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getEuiccInfo2(java.lang.String callingPackage, java.lang.String cardId, com.android.internal.telephony.euicc.IGetEuiccInfo2Callback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(cardId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getEuiccInfo2, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getEuiccInfo2(callingPackage, cardId, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void authenticateServer(java.lang.String callingPackage, java.lang.String cardId, java.lang.String matchingId, byte[] serverSigned1, byte[] serverSignature1, byte[] euiccCiPkIdToBeUsed, byte[] serverCertificatein, com.android.internal.telephony.euicc.IAuthenticateServerCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(cardId);
          _data.writeString(matchingId);
          _data.writeByteArray(serverSigned1);
          _data.writeByteArray(serverSignature1);
          _data.writeByteArray(euiccCiPkIdToBeUsed);
          _data.writeByteArray(serverCertificatein);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_authenticateServer, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().authenticateServer(callingPackage, cardId, matchingId, serverSigned1, serverSignature1, euiccCiPkIdToBeUsed, serverCertificatein, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void prepareDownload(java.lang.String callingPackage, java.lang.String cardId, byte[] hashCc, byte[] smdpSigned2, byte[] smdpSignature2, byte[] smdpCertificate, com.android.internal.telephony.euicc.IPrepareDownloadCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(cardId);
          _data.writeByteArray(hashCc);
          _data.writeByteArray(smdpSigned2);
          _data.writeByteArray(smdpSignature2);
          _data.writeByteArray(smdpCertificate);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_prepareDownload, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().prepareDownload(callingPackage, cardId, hashCc, smdpSigned2, smdpSignature2, smdpCertificate, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void loadBoundProfilePackage(java.lang.String callingPackage, java.lang.String cardId, byte[] boundProfilePackage, com.android.internal.telephony.euicc.ILoadBoundProfilePackageCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(cardId);
          _data.writeByteArray(boundProfilePackage);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_loadBoundProfilePackage, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().loadBoundProfilePackage(callingPackage, cardId, boundProfilePackage, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void cancelSession(java.lang.String callingPackage, java.lang.String cardId, byte[] transactionId, int reason, com.android.internal.telephony.euicc.ICancelSessionCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(cardId);
          _data.writeByteArray(transactionId);
          _data.writeInt(reason);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelSession, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelSession(callingPackage, cardId, transactionId, reason, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void listNotifications(java.lang.String callingPackage, java.lang.String cardId, int events, com.android.internal.telephony.euicc.IListNotificationsCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(cardId);
          _data.writeInt(events);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_listNotifications, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().listNotifications(callingPackage, cardId, events, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void retrieveNotificationList(java.lang.String callingPackage, java.lang.String cardId, int events, com.android.internal.telephony.euicc.IRetrieveNotificationListCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(cardId);
          _data.writeInt(events);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_retrieveNotificationList, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().retrieveNotificationList(callingPackage, cardId, events, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void retrieveNotification(java.lang.String callingPackage, java.lang.String cardId, int seqNumber, com.android.internal.telephony.euicc.IRetrieveNotificationCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(cardId);
          _data.writeInt(seqNumber);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_retrieveNotification, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().retrieveNotification(callingPackage, cardId, seqNumber, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void removeNotificationFromList(java.lang.String callingPackage, java.lang.String cardId, int seqNumber, com.android.internal.telephony.euicc.IRemoveNotificationFromListCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeString(cardId);
          _data.writeInt(seqNumber);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeNotificationFromList, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeNotificationFromList(callingPackage, cardId, seqNumber, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.telephony.euicc.IEuiccCardController sDefaultImpl;
    }
    static final int TRANSACTION_getAllProfiles = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getProfile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_disableProfile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_switchToProfile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_setNickname = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_deleteProfile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_resetMemory = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_getDefaultSmdpAddress = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_getSmdsAddress = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_setDefaultSmdpAddress = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_getRulesAuthTable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_getEuiccChallenge = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_getEuiccInfo1 = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_getEuiccInfo2 = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_authenticateServer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_prepareDownload = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_loadBoundProfilePackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_cancelSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_listNotifications = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_retrieveNotificationList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_retrieveNotification = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_removeNotificationFromList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    public static boolean setDefaultImpl(com.android.internal.telephony.euicc.IEuiccCardController impl) {
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
    public static com.android.internal.telephony.euicc.IEuiccCardController getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void getAllProfiles(java.lang.String callingPackage, java.lang.String cardId, com.android.internal.telephony.euicc.IGetAllProfilesCallback callback) throws android.os.RemoteException;
  public void getProfile(java.lang.String callingPackage, java.lang.String cardId, java.lang.String iccid, com.android.internal.telephony.euicc.IGetProfileCallback callback) throws android.os.RemoteException;
  public void disableProfile(java.lang.String callingPackage, java.lang.String cardId, java.lang.String iccid, boolean refresh, com.android.internal.telephony.euicc.IDisableProfileCallback callback) throws android.os.RemoteException;
  public void switchToProfile(java.lang.String callingPackage, java.lang.String cardId, java.lang.String iccid, boolean refresh, com.android.internal.telephony.euicc.ISwitchToProfileCallback callback) throws android.os.RemoteException;
  public void setNickname(java.lang.String callingPackage, java.lang.String cardId, java.lang.String iccid, java.lang.String nickname, com.android.internal.telephony.euicc.ISetNicknameCallback callback) throws android.os.RemoteException;
  public void deleteProfile(java.lang.String callingPackage, java.lang.String cardId, java.lang.String iccid, com.android.internal.telephony.euicc.IDeleteProfileCallback callback) throws android.os.RemoteException;
  public void resetMemory(java.lang.String callingPackage, java.lang.String cardId, int options, com.android.internal.telephony.euicc.IResetMemoryCallback callback) throws android.os.RemoteException;
  public void getDefaultSmdpAddress(java.lang.String callingPackage, java.lang.String cardId, com.android.internal.telephony.euicc.IGetDefaultSmdpAddressCallback callback) throws android.os.RemoteException;
  public void getSmdsAddress(java.lang.String callingPackage, java.lang.String cardId, com.android.internal.telephony.euicc.IGetSmdsAddressCallback callback) throws android.os.RemoteException;
  public void setDefaultSmdpAddress(java.lang.String callingPackage, java.lang.String cardId, java.lang.String address, com.android.internal.telephony.euicc.ISetDefaultSmdpAddressCallback callback) throws android.os.RemoteException;
  public void getRulesAuthTable(java.lang.String callingPackage, java.lang.String cardId, com.android.internal.telephony.euicc.IGetRulesAuthTableCallback callback) throws android.os.RemoteException;
  public void getEuiccChallenge(java.lang.String callingPackage, java.lang.String cardId, com.android.internal.telephony.euicc.IGetEuiccChallengeCallback callback) throws android.os.RemoteException;
  public void getEuiccInfo1(java.lang.String callingPackage, java.lang.String cardId, com.android.internal.telephony.euicc.IGetEuiccInfo1Callback callback) throws android.os.RemoteException;
  public void getEuiccInfo2(java.lang.String callingPackage, java.lang.String cardId, com.android.internal.telephony.euicc.IGetEuiccInfo2Callback callback) throws android.os.RemoteException;
  public void authenticateServer(java.lang.String callingPackage, java.lang.String cardId, java.lang.String matchingId, byte[] serverSigned1, byte[] serverSignature1, byte[] euiccCiPkIdToBeUsed, byte[] serverCertificatein, com.android.internal.telephony.euicc.IAuthenticateServerCallback callback) throws android.os.RemoteException;
  public void prepareDownload(java.lang.String callingPackage, java.lang.String cardId, byte[] hashCc, byte[] smdpSigned2, byte[] smdpSignature2, byte[] smdpCertificate, com.android.internal.telephony.euicc.IPrepareDownloadCallback callback) throws android.os.RemoteException;
  public void loadBoundProfilePackage(java.lang.String callingPackage, java.lang.String cardId, byte[] boundProfilePackage, com.android.internal.telephony.euicc.ILoadBoundProfilePackageCallback callback) throws android.os.RemoteException;
  public void cancelSession(java.lang.String callingPackage, java.lang.String cardId, byte[] transactionId, int reason, com.android.internal.telephony.euicc.ICancelSessionCallback callback) throws android.os.RemoteException;
  public void listNotifications(java.lang.String callingPackage, java.lang.String cardId, int events, com.android.internal.telephony.euicc.IListNotificationsCallback callback) throws android.os.RemoteException;
  public void retrieveNotificationList(java.lang.String callingPackage, java.lang.String cardId, int events, com.android.internal.telephony.euicc.IRetrieveNotificationListCallback callback) throws android.os.RemoteException;
  public void retrieveNotification(java.lang.String callingPackage, java.lang.String cardId, int seqNumber, com.android.internal.telephony.euicc.IRetrieveNotificationCallback callback) throws android.os.RemoteException;
  public void removeNotificationFromList(java.lang.String callingPackage, java.lang.String cardId, int seqNumber, com.android.internal.telephony.euicc.IRemoveNotificationFromListCallback callback) throws android.os.RemoteException;
}
