/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.telephony.euicc;
/** @hide */
public interface IEuiccController extends android.os.IInterface
{
  /** Default implementation for IEuiccController. */
  public static class Default implements com.android.internal.telephony.euicc.IEuiccController
  {
    @Override public void continueOperation(int cardId, android.content.Intent resolutionIntent, android.os.Bundle resolutionExtras) throws android.os.RemoteException
    {
    }
    @Override public void getDownloadableSubscriptionMetadata(int cardId, android.telephony.euicc.DownloadableSubscription subscription, java.lang.String callingPackage, android.app.PendingIntent callbackIntent) throws android.os.RemoteException
    {
    }
    @Override public void getDefaultDownloadableSubscriptionList(int cardId, java.lang.String callingPackage, android.app.PendingIntent callbackIntent) throws android.os.RemoteException
    {
    }
    @Override public java.lang.String getEid(int cardId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int getOtaStatus(int cardId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void downloadSubscription(int cardId, android.telephony.euicc.DownloadableSubscription subscription, boolean switchAfterDownload, java.lang.String callingPackage, android.os.Bundle resolvedBundle, android.app.PendingIntent callbackIntent) throws android.os.RemoteException
    {
    }
    @Override public android.telephony.euicc.EuiccInfo getEuiccInfo(int cardId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void deleteSubscription(int cardId, int subscriptionId, java.lang.String callingPackage, android.app.PendingIntent callbackIntent) throws android.os.RemoteException
    {
    }
    @Override public void switchToSubscription(int cardId, int subscriptionId, java.lang.String callingPackage, android.app.PendingIntent callbackIntent) throws android.os.RemoteException
    {
    }
    @Override public void updateSubscriptionNickname(int cardId, int subscriptionId, java.lang.String nickname, java.lang.String callingPackage, android.app.PendingIntent callbackIntent) throws android.os.RemoteException
    {
    }
    @Override public void eraseSubscriptions(int cardId, android.app.PendingIntent callbackIntent) throws android.os.RemoteException
    {
    }
    @Override public void retainSubscriptionsForFactoryReset(int cardId, android.app.PendingIntent callbackIntent) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.telephony.euicc.IEuiccController
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.telephony.euicc.IEuiccController";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.telephony.euicc.IEuiccController interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.telephony.euicc.IEuiccController asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.telephony.euicc.IEuiccController))) {
        return ((com.android.internal.telephony.euicc.IEuiccController)iin);
      }
      return new com.android.internal.telephony.euicc.IEuiccController.Stub.Proxy(obj);
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
        case TRANSACTION_continueOperation:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.content.Intent _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.Intent.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.os.Bundle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.continueOperation(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_getDownloadableSubscriptionMetadata:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.euicc.DownloadableSubscription _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telephony.euicc.DownloadableSubscription.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.app.PendingIntent _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.getDownloadableSubscriptionMetadata(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_getDefaultDownloadableSubscriptionList:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.app.PendingIntent _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.getDefaultDownloadableSubscriptionList(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_getEid:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _result = this.getEid(_arg0, _arg1);
          reply.writeNoException();
          reply.writeString(_result);
          return true;
        }
        case TRANSACTION_getOtaStatus:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.getOtaStatus(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_downloadSubscription:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.euicc.DownloadableSubscription _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telephony.euicc.DownloadableSubscription.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          java.lang.String _arg3;
          _arg3 = data.readString();
          android.os.Bundle _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          android.app.PendingIntent _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          this.downloadSubscription(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          return true;
        }
        case TRANSACTION_getEuiccInfo:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.telephony.euicc.EuiccInfo _result = this.getEuiccInfo(_arg0);
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
        case TRANSACTION_deleteSubscription:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.app.PendingIntent _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.deleteSubscription(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_switchToSubscription:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.app.PendingIntent _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.switchToSubscription(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_updateSubscriptionNickname:
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
          android.app.PendingIntent _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          this.updateSubscriptionNickname(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_eraseSubscriptions:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.app.PendingIntent _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.eraseSubscriptions(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_retainSubscriptionsForFactoryReset:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.app.PendingIntent _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.retainSubscriptionsForFactoryReset(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.telephony.euicc.IEuiccController
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
      @Override public void continueOperation(int cardId, android.content.Intent resolutionIntent, android.os.Bundle resolutionExtras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(cardId);
          if ((resolutionIntent!=null)) {
            _data.writeInt(1);
            resolutionIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((resolutionExtras!=null)) {
            _data.writeInt(1);
            resolutionExtras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_continueOperation, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().continueOperation(cardId, resolutionIntent, resolutionExtras);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getDownloadableSubscriptionMetadata(int cardId, android.telephony.euicc.DownloadableSubscription subscription, java.lang.String callingPackage, android.app.PendingIntent callbackIntent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(cardId);
          if ((subscription!=null)) {
            _data.writeInt(1);
            subscription.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          if ((callbackIntent!=null)) {
            _data.writeInt(1);
            callbackIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDownloadableSubscriptionMetadata, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getDownloadableSubscriptionMetadata(cardId, subscription, callingPackage, callbackIntent);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getDefaultDownloadableSubscriptionList(int cardId, java.lang.String callingPackage, android.app.PendingIntent callbackIntent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(cardId);
          _data.writeString(callingPackage);
          if ((callbackIntent!=null)) {
            _data.writeInt(1);
            callbackIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDefaultDownloadableSubscriptionList, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getDefaultDownloadableSubscriptionList(cardId, callingPackage, callbackIntent);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public java.lang.String getEid(int cardId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.lang.String _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(cardId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getEid, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getEid(cardId, callingPackage);
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
      @Override public int getOtaStatus(int cardId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(cardId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getOtaStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getOtaStatus(cardId);
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
      @Override public void downloadSubscription(int cardId, android.telephony.euicc.DownloadableSubscription subscription, boolean switchAfterDownload, java.lang.String callingPackage, android.os.Bundle resolvedBundle, android.app.PendingIntent callbackIntent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(cardId);
          if ((subscription!=null)) {
            _data.writeInt(1);
            subscription.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((switchAfterDownload)?(1):(0)));
          _data.writeString(callingPackage);
          if ((resolvedBundle!=null)) {
            _data.writeInt(1);
            resolvedBundle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((callbackIntent!=null)) {
            _data.writeInt(1);
            callbackIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_downloadSubscription, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().downloadSubscription(cardId, subscription, switchAfterDownload, callingPackage, resolvedBundle, callbackIntent);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public android.telephony.euicc.EuiccInfo getEuiccInfo(int cardId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.telephony.euicc.EuiccInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(cardId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getEuiccInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getEuiccInfo(cardId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.telephony.euicc.EuiccInfo.CREATOR.createFromParcel(_reply);
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
      @Override public void deleteSubscription(int cardId, int subscriptionId, java.lang.String callingPackage, android.app.PendingIntent callbackIntent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(cardId);
          _data.writeInt(subscriptionId);
          _data.writeString(callingPackage);
          if ((callbackIntent!=null)) {
            _data.writeInt(1);
            callbackIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_deleteSubscription, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deleteSubscription(cardId, subscriptionId, callingPackage, callbackIntent);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void switchToSubscription(int cardId, int subscriptionId, java.lang.String callingPackage, android.app.PendingIntent callbackIntent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(cardId);
          _data.writeInt(subscriptionId);
          _data.writeString(callingPackage);
          if ((callbackIntent!=null)) {
            _data.writeInt(1);
            callbackIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_switchToSubscription, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().switchToSubscription(cardId, subscriptionId, callingPackage, callbackIntent);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void updateSubscriptionNickname(int cardId, int subscriptionId, java.lang.String nickname, java.lang.String callingPackage, android.app.PendingIntent callbackIntent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(cardId);
          _data.writeInt(subscriptionId);
          _data.writeString(nickname);
          _data.writeString(callingPackage);
          if ((callbackIntent!=null)) {
            _data.writeInt(1);
            callbackIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateSubscriptionNickname, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateSubscriptionNickname(cardId, subscriptionId, nickname, callingPackage, callbackIntent);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void eraseSubscriptions(int cardId, android.app.PendingIntent callbackIntent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(cardId);
          if ((callbackIntent!=null)) {
            _data.writeInt(1);
            callbackIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_eraseSubscriptions, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().eraseSubscriptions(cardId, callbackIntent);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void retainSubscriptionsForFactoryReset(int cardId, android.app.PendingIntent callbackIntent) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(cardId);
          if ((callbackIntent!=null)) {
            _data.writeInt(1);
            callbackIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_retainSubscriptionsForFactoryReset, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().retainSubscriptionsForFactoryReset(cardId, callbackIntent);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.telephony.euicc.IEuiccController sDefaultImpl;
    }
    static final int TRANSACTION_continueOperation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getDownloadableSubscriptionMetadata = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getDefaultDownloadableSubscriptionList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getEid = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getOtaStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_downloadSubscription = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_getEuiccInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_deleteSubscription = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_switchToSubscription = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_updateSubscriptionNickname = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_eraseSubscriptions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_retainSubscriptionsForFactoryReset = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    public static boolean setDefaultImpl(com.android.internal.telephony.euicc.IEuiccController impl) {
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
    public static com.android.internal.telephony.euicc.IEuiccController getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void continueOperation(int cardId, android.content.Intent resolutionIntent, android.os.Bundle resolutionExtras) throws android.os.RemoteException;
  public void getDownloadableSubscriptionMetadata(int cardId, android.telephony.euicc.DownloadableSubscription subscription, java.lang.String callingPackage, android.app.PendingIntent callbackIntent) throws android.os.RemoteException;
  public void getDefaultDownloadableSubscriptionList(int cardId, java.lang.String callingPackage, android.app.PendingIntent callbackIntent) throws android.os.RemoteException;
  public java.lang.String getEid(int cardId, java.lang.String callingPackage) throws android.os.RemoteException;
  public int getOtaStatus(int cardId) throws android.os.RemoteException;
  public void downloadSubscription(int cardId, android.telephony.euicc.DownloadableSubscription subscription, boolean switchAfterDownload, java.lang.String callingPackage, android.os.Bundle resolvedBundle, android.app.PendingIntent callbackIntent) throws android.os.RemoteException;
  public android.telephony.euicc.EuiccInfo getEuiccInfo(int cardId) throws android.os.RemoteException;
  public void deleteSubscription(int cardId, int subscriptionId, java.lang.String callingPackage, android.app.PendingIntent callbackIntent) throws android.os.RemoteException;
  public void switchToSubscription(int cardId, int subscriptionId, java.lang.String callingPackage, android.app.PendingIntent callbackIntent) throws android.os.RemoteException;
  public void updateSubscriptionNickname(int cardId, int subscriptionId, java.lang.String nickname, java.lang.String callingPackage, android.app.PendingIntent callbackIntent) throws android.os.RemoteException;
  public void eraseSubscriptions(int cardId, android.app.PendingIntent callbackIntent) throws android.os.RemoteException;
  public void retainSubscriptionsForFactoryReset(int cardId, android.app.PendingIntent callbackIntent) throws android.os.RemoteException;
}
