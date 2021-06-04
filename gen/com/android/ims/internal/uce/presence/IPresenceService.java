/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.ims.internal.uce.presence;
/** IPresenceService
{@hide} */
public interface IPresenceService extends android.os.IInterface
{
  /** Default implementation for IPresenceService. */
  public static class Default implements com.android.ims.internal.uce.presence.IPresenceService
  {
    /**
         * Gets the version of the Presence service implementation.
         * The verion information is received in getVersionCb callback.
         * @param presenceServiceHdl returned in createPresenceService().
         * @return StatusCode, status of the request placed.
         */
    @Override public com.android.ims.internal.uce.common.StatusCode getVersion(int presenceServiceHdl) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Adds a listener to the Presence service.
         * @param presenceServiceHdl returned in createPresenceService().
         * @param presenceServiceListener IPresenceListener Object.
         * @param presenceServiceListenerHdl wrapper for client's listener handle to be stored.
         *
         * The service will fill UceLong.mUceLong with presenceListenerHandle.
         *
         * @return StatusCode, status of the request placed
         */
    @Override public com.android.ims.internal.uce.common.StatusCode addListener(int presenceServiceHdl, com.android.ims.internal.uce.presence.IPresenceListener presenceServiceListener, com.android.ims.internal.uce.common.UceLong presenceServiceListenerHdl) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Removes a listener from the Presence service.
         * @param presenceServiceHdl returned in createPresenceService().
         * @param presenceServiceListenerHdl provided in createPresenceService() or Addlistener().
         * @return StatusCode, status of the request placed.
         */
    @Override public com.android.ims.internal.uce.common.StatusCode removeListener(int presenceServiceHdl, com.android.ims.internal.uce.common.UceLong presenceServiceListenerHdl) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Re-enables the Presence service if it is in the Blocked state due to receiving a SIP
         * response 489 Bad event.
         * The application must call this API before calling any presence API after receiving a SIP
         * response 489 Bad event.
         * The status of this request is notified in cmdStatus callback.
         *
         * @param presenceServiceHdl returned in createPresenceService().
         * @param userData, userData provided by client to identify the request/API call, it
         *                  is returned in the cmdStatus() callback for client to match response
         *                  with original request.
         * @return StatusCode, status of the request placed.
         */
    @Override public com.android.ims.internal.uce.common.StatusCode reenableService(int presenceServiceHdl, int userData) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Sends a request to publish current device capabilities.
         * The network response is notifed in sipResponseReceived() callback.
         * @param presenceServiceHdl returned in createPresenceService().
         * @param myCapInfo PresCapInfo object.
         * @param userData, userData provided by client to identify the request/API call, it
         *                  is returned in the cmdStatus() callback for client to match response
         *                  with original request.
         * @return StatusCode, status of the request placed.
         */
    @Override public com.android.ims.internal.uce.common.StatusCode publishMyCap(int presenceServiceHdl, com.android.ims.internal.uce.presence.PresCapInfo myCapInfo, int userData) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Retrieves the capability information for a single contact. Clients receive the requested
         * information via the listener callback function capInfoReceived() callback.
         *
         * @param presenceServiceHdl returned in createPresenceService().
         * @param remoteUri remote contact URI
         * @param userData, userData provided by client to identify the request/API call, it
         *                  is returned in the cmdStatus() callback for client to match response
         *                  with original request.
         * @return StatusCode, status of the request placed.
         */
    @Override public com.android.ims.internal.uce.common.StatusCode getContactCap(int presenceServiceHdl, java.lang.String remoteUri, int userData) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Retrieves the capability information for a list of contacts. Clients receive the requested
         * information via the listener callback function listCapInfoReceived() callback.
         *
         * @param presenceServiceHdl returned in createPresenceService().
         * @param remoteUriList list of remote contact URI's.
         * @param userData, userData provided by client to identify the request/API call, it
         *                  is returned in the cmdStatus() callback for client to match response
         *                  with original request.
         * @return StatusCode, status of the request placed.
         */
    @Override public com.android.ims.internal.uce.common.StatusCode getContactListCap(int presenceServiceHdl, java.lang.String[] remoteUriList, int userData) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Sets the mapping between a new feature tag and the corresponding service tuple information
         * to be included in the published document.
         * The staus of this call is received in cmdStatus callback.
         *
         * @param presenceServiceHdl returned in createPresenceService().
         * @param featureTag to be supported
         * @param PresServiceInfo service information describing the featureTag.
         * @param userData, userData provided by client to identify the request/API call, it
         *                  is returned in the cmdStatus() callback for client to match response
         *                  with original request.
         * @return StatusCode, status of the request placed.
         */
    @Override public com.android.ims.internal.uce.common.StatusCode setNewFeatureTag(int presenceServiceHdl, java.lang.String featureTag, com.android.ims.internal.uce.presence.PresServiceInfo serviceInfo, int userData) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.ims.internal.uce.presence.IPresenceService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.ims.internal.uce.presence.IPresenceService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.ims.internal.uce.presence.IPresenceService interface,
     * generating a proxy if needed.
     */
    public static com.android.ims.internal.uce.presence.IPresenceService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.ims.internal.uce.presence.IPresenceService))) {
        return ((com.android.ims.internal.uce.presence.IPresenceService)iin);
      }
      return new com.android.ims.internal.uce.presence.IPresenceService.Stub.Proxy(obj);
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
        case TRANSACTION_getVersion:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          com.android.ims.internal.uce.common.StatusCode _result = this.getVersion(_arg0);
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
        case TRANSACTION_addListener:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          com.android.ims.internal.uce.presence.IPresenceListener _arg1;
          _arg1 = com.android.ims.internal.uce.presence.IPresenceListener.Stub.asInterface(data.readStrongBinder());
          com.android.ims.internal.uce.common.UceLong _arg2;
          if ((0!=data.readInt())) {
            _arg2 = com.android.ims.internal.uce.common.UceLong.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          com.android.ims.internal.uce.common.StatusCode _result = this.addListener(_arg0, _arg1, _arg2);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          if ((_arg2!=null)) {
            reply.writeInt(1);
            _arg2.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_removeListener:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          com.android.ims.internal.uce.common.UceLong _arg1;
          if ((0!=data.readInt())) {
            _arg1 = com.android.ims.internal.uce.common.UceLong.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          com.android.ims.internal.uce.common.StatusCode _result = this.removeListener(_arg0, _arg1);
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
        case TRANSACTION_reenableService:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          com.android.ims.internal.uce.common.StatusCode _result = this.reenableService(_arg0, _arg1);
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
        case TRANSACTION_publishMyCap:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          com.android.ims.internal.uce.presence.PresCapInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = com.android.ims.internal.uce.presence.PresCapInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          com.android.ims.internal.uce.common.StatusCode _result = this.publishMyCap(_arg0, _arg1, _arg2);
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
        case TRANSACTION_getContactCap:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          com.android.ims.internal.uce.common.StatusCode _result = this.getContactCap(_arg0, _arg1, _arg2);
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
        case TRANSACTION_getContactListCap:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String[] _arg1;
          _arg1 = data.createStringArray();
          int _arg2;
          _arg2 = data.readInt();
          com.android.ims.internal.uce.common.StatusCode _result = this.getContactListCap(_arg0, _arg1, _arg2);
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
        case TRANSACTION_setNewFeatureTag:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          com.android.ims.internal.uce.presence.PresServiceInfo _arg2;
          if ((0!=data.readInt())) {
            _arg2 = com.android.ims.internal.uce.presence.PresServiceInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          com.android.ims.internal.uce.common.StatusCode _result = this.setNewFeatureTag(_arg0, _arg1, _arg2, _arg3);
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
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.ims.internal.uce.presence.IPresenceService
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
           * Gets the version of the Presence service implementation.
           * The verion information is received in getVersionCb callback.
           * @param presenceServiceHdl returned in createPresenceService().
           * @return StatusCode, status of the request placed.
           */
      @Override public com.android.ims.internal.uce.common.StatusCode getVersion(int presenceServiceHdl) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.uce.common.StatusCode _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(presenceServiceHdl);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getVersion, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getVersion(presenceServiceHdl);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = com.android.ims.internal.uce.common.StatusCode.CREATOR.createFromParcel(_reply);
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
           * Adds a listener to the Presence service.
           * @param presenceServiceHdl returned in createPresenceService().
           * @param presenceServiceListener IPresenceListener Object.
           * @param presenceServiceListenerHdl wrapper for client's listener handle to be stored.
           *
           * The service will fill UceLong.mUceLong with presenceListenerHandle.
           *
           * @return StatusCode, status of the request placed
           */
      @Override public com.android.ims.internal.uce.common.StatusCode addListener(int presenceServiceHdl, com.android.ims.internal.uce.presence.IPresenceListener presenceServiceListener, com.android.ims.internal.uce.common.UceLong presenceServiceListenerHdl) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.uce.common.StatusCode _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(presenceServiceHdl);
          _data.writeStrongBinder((((presenceServiceListener!=null))?(presenceServiceListener.asBinder()):(null)));
          if ((presenceServiceListenerHdl!=null)) {
            _data.writeInt(1);
            presenceServiceListenerHdl.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_addListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addListener(presenceServiceHdl, presenceServiceListener, presenceServiceListenerHdl);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = com.android.ims.internal.uce.common.StatusCode.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
          if ((0!=_reply.readInt())) {
            presenceServiceListenerHdl.readFromParcel(_reply);
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Removes a listener from the Presence service.
           * @param presenceServiceHdl returned in createPresenceService().
           * @param presenceServiceListenerHdl provided in createPresenceService() or Addlistener().
           * @return StatusCode, status of the request placed.
           */
      @Override public com.android.ims.internal.uce.common.StatusCode removeListener(int presenceServiceHdl, com.android.ims.internal.uce.common.UceLong presenceServiceListenerHdl) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.uce.common.StatusCode _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(presenceServiceHdl);
          if ((presenceServiceListenerHdl!=null)) {
            _data.writeInt(1);
            presenceServiceListenerHdl.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().removeListener(presenceServiceHdl, presenceServiceListenerHdl);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = com.android.ims.internal.uce.common.StatusCode.CREATOR.createFromParcel(_reply);
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
           * Re-enables the Presence service if it is in the Blocked state due to receiving a SIP
           * response 489 Bad event.
           * The application must call this API before calling any presence API after receiving a SIP
           * response 489 Bad event.
           * The status of this request is notified in cmdStatus callback.
           *
           * @param presenceServiceHdl returned in createPresenceService().
           * @param userData, userData provided by client to identify the request/API call, it
           *                  is returned in the cmdStatus() callback for client to match response
           *                  with original request.
           * @return StatusCode, status of the request placed.
           */
      @Override public com.android.ims.internal.uce.common.StatusCode reenableService(int presenceServiceHdl, int userData) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.uce.common.StatusCode _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(presenceServiceHdl);
          _data.writeInt(userData);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reenableService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().reenableService(presenceServiceHdl, userData);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = com.android.ims.internal.uce.common.StatusCode.CREATOR.createFromParcel(_reply);
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
           * Sends a request to publish current device capabilities.
           * The network response is notifed in sipResponseReceived() callback.
           * @param presenceServiceHdl returned in createPresenceService().
           * @param myCapInfo PresCapInfo object.
           * @param userData, userData provided by client to identify the request/API call, it
           *                  is returned in the cmdStatus() callback for client to match response
           *                  with original request.
           * @return StatusCode, status of the request placed.
           */
      @Override public com.android.ims.internal.uce.common.StatusCode publishMyCap(int presenceServiceHdl, com.android.ims.internal.uce.presence.PresCapInfo myCapInfo, int userData) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.uce.common.StatusCode _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(presenceServiceHdl);
          if ((myCapInfo!=null)) {
            _data.writeInt(1);
            myCapInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userData);
          boolean _status = mRemote.transact(Stub.TRANSACTION_publishMyCap, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().publishMyCap(presenceServiceHdl, myCapInfo, userData);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = com.android.ims.internal.uce.common.StatusCode.CREATOR.createFromParcel(_reply);
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
           * Retrieves the capability information for a single contact. Clients receive the requested
           * information via the listener callback function capInfoReceived() callback.
           *
           * @param presenceServiceHdl returned in createPresenceService().
           * @param remoteUri remote contact URI
           * @param userData, userData provided by client to identify the request/API call, it
           *                  is returned in the cmdStatus() callback for client to match response
           *                  with original request.
           * @return StatusCode, status of the request placed.
           */
      @Override public com.android.ims.internal.uce.common.StatusCode getContactCap(int presenceServiceHdl, java.lang.String remoteUri, int userData) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.uce.common.StatusCode _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(presenceServiceHdl);
          _data.writeString(remoteUri);
          _data.writeInt(userData);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getContactCap, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getContactCap(presenceServiceHdl, remoteUri, userData);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = com.android.ims.internal.uce.common.StatusCode.CREATOR.createFromParcel(_reply);
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
           * Retrieves the capability information for a list of contacts. Clients receive the requested
           * information via the listener callback function listCapInfoReceived() callback.
           *
           * @param presenceServiceHdl returned in createPresenceService().
           * @param remoteUriList list of remote contact URI's.
           * @param userData, userData provided by client to identify the request/API call, it
           *                  is returned in the cmdStatus() callback for client to match response
           *                  with original request.
           * @return StatusCode, status of the request placed.
           */
      @Override public com.android.ims.internal.uce.common.StatusCode getContactListCap(int presenceServiceHdl, java.lang.String[] remoteUriList, int userData) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.uce.common.StatusCode _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(presenceServiceHdl);
          _data.writeStringArray(remoteUriList);
          _data.writeInt(userData);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getContactListCap, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getContactListCap(presenceServiceHdl, remoteUriList, userData);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = com.android.ims.internal.uce.common.StatusCode.CREATOR.createFromParcel(_reply);
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
           * Sets the mapping between a new feature tag and the corresponding service tuple information
           * to be included in the published document.
           * The staus of this call is received in cmdStatus callback.
           *
           * @param presenceServiceHdl returned in createPresenceService().
           * @param featureTag to be supported
           * @param PresServiceInfo service information describing the featureTag.
           * @param userData, userData provided by client to identify the request/API call, it
           *                  is returned in the cmdStatus() callback for client to match response
           *                  with original request.
           * @return StatusCode, status of the request placed.
           */
      @Override public com.android.ims.internal.uce.common.StatusCode setNewFeatureTag(int presenceServiceHdl, java.lang.String featureTag, com.android.ims.internal.uce.presence.PresServiceInfo serviceInfo, int userData) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.uce.common.StatusCode _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(presenceServiceHdl);
          _data.writeString(featureTag);
          if ((serviceInfo!=null)) {
            _data.writeInt(1);
            serviceInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userData);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setNewFeatureTag, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setNewFeatureTag(presenceServiceHdl, featureTag, serviceInfo, userData);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = com.android.ims.internal.uce.common.StatusCode.CREATOR.createFromParcel(_reply);
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
      public static com.android.ims.internal.uce.presence.IPresenceService sDefaultImpl;
    }
    static final int TRANSACTION_getVersion = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_addListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_removeListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_reenableService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_publishMyCap = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getContactCap = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_getContactListCap = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_setNewFeatureTag = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    public static boolean setDefaultImpl(com.android.ims.internal.uce.presence.IPresenceService impl) {
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
    public static com.android.ims.internal.uce.presence.IPresenceService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Gets the version of the Presence service implementation.
       * The verion information is received in getVersionCb callback.
       * @param presenceServiceHdl returned in createPresenceService().
       * @return StatusCode, status of the request placed.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/presence/IPresenceService.aidl:36:1:36:25")
  public com.android.ims.internal.uce.common.StatusCode getVersion(int presenceServiceHdl) throws android.os.RemoteException;
  /**
       * Adds a listener to the Presence service.
       * @param presenceServiceHdl returned in createPresenceService().
       * @param presenceServiceListener IPresenceListener Object.
       * @param presenceServiceListenerHdl wrapper for client's listener handle to be stored.
       *
       * The service will fill UceLong.mUceLong with presenceListenerHandle.
       *
       * @return StatusCode, status of the request placed
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/presence/IPresenceService.aidl:49:1:49:25")
  public com.android.ims.internal.uce.common.StatusCode addListener(int presenceServiceHdl, com.android.ims.internal.uce.presence.IPresenceListener presenceServiceListener, com.android.ims.internal.uce.common.UceLong presenceServiceListenerHdl) throws android.os.RemoteException;
  /**
       * Removes a listener from the Presence service.
       * @param presenceServiceHdl returned in createPresenceService().
       * @param presenceServiceListenerHdl provided in createPresenceService() or Addlistener().
       * @return StatusCode, status of the request placed.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/presence/IPresenceService.aidl:59:1:59:25")
  public com.android.ims.internal.uce.common.StatusCode removeListener(int presenceServiceHdl, com.android.ims.internal.uce.common.UceLong presenceServiceListenerHdl) throws android.os.RemoteException;
  /**
       * Re-enables the Presence service if it is in the Blocked state due to receiving a SIP
       * response 489 Bad event.
       * The application must call this API before calling any presence API after receiving a SIP
       * response 489 Bad event.
       * The status of this request is notified in cmdStatus callback.
       *
       * @param presenceServiceHdl returned in createPresenceService().
       * @param userData, userData provided by client to identify the request/API call, it
       *                  is returned in the cmdStatus() callback for client to match response
       *                  with original request.
       * @return StatusCode, status of the request placed.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/presence/IPresenceService.aidl:75:1:75:25")
  public com.android.ims.internal.uce.common.StatusCode reenableService(int presenceServiceHdl, int userData) throws android.os.RemoteException;
  /**
       * Sends a request to publish current device capabilities.
       * The network response is notifed in sipResponseReceived() callback.
       * @param presenceServiceHdl returned in createPresenceService().
       * @param myCapInfo PresCapInfo object.
       * @param userData, userData provided by client to identify the request/API call, it
       *                  is returned in the cmdStatus() callback for client to match response
       *                  with original request.
       * @return StatusCode, status of the request placed.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/presence/IPresenceService.aidl:88:1:88:25")
  public com.android.ims.internal.uce.common.StatusCode publishMyCap(int presenceServiceHdl, com.android.ims.internal.uce.presence.PresCapInfo myCapInfo, int userData) throws android.os.RemoteException;
  /**
       * Retrieves the capability information for a single contact. Clients receive the requested
       * information via the listener callback function capInfoReceived() callback.
       *
       * @param presenceServiceHdl returned in createPresenceService().
       * @param remoteUri remote contact URI
       * @param userData, userData provided by client to identify the request/API call, it
       *                  is returned in the cmdStatus() callback for client to match response
       *                  with original request.
       * @return StatusCode, status of the request placed.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/presence/IPresenceService.aidl:102:1:102:25")
  public com.android.ims.internal.uce.common.StatusCode getContactCap(int presenceServiceHdl, java.lang.String remoteUri, int userData) throws android.os.RemoteException;
  /**
       * Retrieves the capability information for a list of contacts. Clients receive the requested
       * information via the listener callback function listCapInfoReceived() callback.
       *
       * @param presenceServiceHdl returned in createPresenceService().
       * @param remoteUriList list of remote contact URI's.
       * @param userData, userData provided by client to identify the request/API call, it
       *                  is returned in the cmdStatus() callback for client to match response
       *                  with original request.
       * @return StatusCode, status of the request placed.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/presence/IPresenceService.aidl:116:1:116:25")
  public com.android.ims.internal.uce.common.StatusCode getContactListCap(int presenceServiceHdl, java.lang.String[] remoteUriList, int userData) throws android.os.RemoteException;
  /**
       * Sets the mapping between a new feature tag and the corresponding service tuple information
       * to be included in the published document.
       * The staus of this call is received in cmdStatus callback.
       *
       * @param presenceServiceHdl returned in createPresenceService().
       * @param featureTag to be supported
       * @param PresServiceInfo service information describing the featureTag.
       * @param userData, userData provided by client to identify the request/API call, it
       *                  is returned in the cmdStatus() callback for client to match response
       *                  with original request.
       * @return StatusCode, status of the request placed.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/presence/IPresenceService.aidl:132:1:132:25")
  public com.android.ims.internal.uce.common.StatusCode setNewFeatureTag(int presenceServiceHdl, java.lang.String featureTag, com.android.ims.internal.uce.presence.PresServiceInfo serviceInfo, int userData) throws android.os.RemoteException;
}
