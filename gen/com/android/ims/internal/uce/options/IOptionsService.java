/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.ims.internal.uce.options;
/** {@hide} */
public interface IOptionsService extends android.os.IInterface
{
  /** Default implementation for IOptionsService. */
  public static class Default implements com.android.ims.internal.uce.options.IOptionsService
  {
    /**
         * Gets the version of the Options service implementation.
         * the result of this Call is received in getVersionCb
         * @param optionsServiceHandle, received in serviceCreated() of IOptionsListener.
         * @return StatusCode, status of the request placed.
         * @hide
         */
    @Override public com.android.ims.internal.uce.common.StatusCode getVersion(int optionsServiceHandle) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Adds a listener to the Options service.
         * @param optionsServiceHandle, this returned in serviceCreated() of IOptionsListener.
         * @param optionsListener, IOptionsListener object.
         * @param optionsServiceListenerHdl wrapper for client's listener handle to be stored.
         *
         * The service will fill UceLong.mUceLong with optionsServiceListenerHdl
         * @return StatusCode, status of the request placed.
         */
    @Override public com.android.ims.internal.uce.common.StatusCode addListener(int optionsServiceHandle, com.android.ims.internal.uce.options.IOptionsListener optionsListener, com.android.ims.internal.uce.common.UceLong optionsServiceListenerHdl) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Removes a listener from the Options service.
         * @param optionsServiceHandle, received in serviceCreated() of IOptionsListener.
         * @param optionsListenerHandle, received in serviceCreated() of IOptionsListener.
         * @param optionsServiceListenerHdl provided in createOptionsService() or Addlistener().
         * @return StatusCode, status of the request placed.
         */
    @Override public com.android.ims.internal.uce.common.StatusCode removeListener(int optionsServiceHandle, com.android.ims.internal.uce.common.UceLong optionsServiceListenerHdl) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Sets the capabilities information of the self device.
         * The status of the call is received in cmdStatus callback
         * @param optionsServiceHandle, this returned in serviceCreated() of IOptionsListener.
         * @param capInfo, capability information to store.
         * @param reqUserData, userData provided by client to identify the request/API call, it
         *                  is returned in the cmdStatus() callback for client to match response
         *                  with original request.
         * @return StatusCode, status of the request placed.
         */
    @Override public com.android.ims.internal.uce.common.StatusCode setMyInfo(int optionsServiceHandle, com.android.ims.internal.uce.common.CapInfo capInfo, int reqUserData) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Gets the capabilities information of remote device.
         * The Capability information is received in cmdStatus callback
         * @param optionsServiceHandle, this returned in serviceCreated() of IOptionsListener.
         * @param reqUserData, userData provided by client to identify the request/API call, it
         *                  is returned in the cmdStatus() callback for client to match response
         *                  with original request.
         * @return StatusCode, status of the request placed.
         */
    @Override public com.android.ims.internal.uce.common.StatusCode getMyInfo(int optionsServiceHandle, int reqUserdata) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Requests the capabilities information of a remote URI.
         * the remote party capability is received in sipResponseReceived() callback.
         * @param optionsServiceHandle, this returned in serviceCreated() of IOptionsListener.
         * @param remoteURI, URI of the remote contact.
         * @param reqUserData, userData provided by client to identify the request/API call, it
         *                  is returned in the cmdStatus() callback for client to match response
         *                  with original request.
         * @return StatusCode, status of the request placed.
         */
    @Override public com.android.ims.internal.uce.common.StatusCode getContactCap(int optionsServiceHandle, java.lang.String remoteURI, int reqUserData) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Requests the capabilities information of specified contacts.
         * For each remote party capability is received in sipResponseReceived() callback
         * @param optionsServiceHandle, this returned in serviceCreated() of IOptionsListener.
         * @param remoteURIList, list of remote contact URI's.
         * @param reqUserData, userData provided by client to identify the request/API call, it
         *                  is returned in the cmdStatus() callback for client to match response
         *                  with original request.
         * @return StatusCode, status of the request placed.
         */
    @Override public com.android.ims.internal.uce.common.StatusCode getContactListCap(int optionsServiceHandle, java.lang.String[] remoteURIList, int reqUserData) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Requests the capabilities information of specified contacts.
         * The incoming Options request is received in incomingOptions() callback.
         *
         * @param optionsServiceHandle, this returned in serviceCreated() of IOptionsListener.
         * @param tId, transaction ID received in incomingOptions() call of IOptionsListener.
         * @param sipResponseCode, SIP response code the UE needs to share to network.
         * @param reasonPhrase, response phrase corresponding to the response code.
         * @param capInfo, capabilities to share in the resonse to network.
         * @param bContactInBL, true if the contact is blacklisted, else false.
         * @return StatusCode, status of the request placed.
         */
    @Override public com.android.ims.internal.uce.common.StatusCode responseIncomingOptions(int optionsServiceHandle, int tId, int sipResponseCode, java.lang.String reasonPhrase, com.android.ims.internal.uce.options.OptionsCapInfo capInfo, boolean bContactInBL) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.ims.internal.uce.options.IOptionsService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.ims.internal.uce.options.IOptionsService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.ims.internal.uce.options.IOptionsService interface,
     * generating a proxy if needed.
     */
    public static com.android.ims.internal.uce.options.IOptionsService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.ims.internal.uce.options.IOptionsService))) {
        return ((com.android.ims.internal.uce.options.IOptionsService)iin);
      }
      return new com.android.ims.internal.uce.options.IOptionsService.Stub.Proxy(obj);
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
          com.android.ims.internal.uce.options.IOptionsListener _arg1;
          _arg1 = com.android.ims.internal.uce.options.IOptionsListener.Stub.asInterface(data.readStrongBinder());
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
        case TRANSACTION_setMyInfo:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          com.android.ims.internal.uce.common.CapInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = com.android.ims.internal.uce.common.CapInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          com.android.ims.internal.uce.common.StatusCode _result = this.setMyInfo(_arg0, _arg1, _arg2);
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
        case TRANSACTION_getMyInfo:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          com.android.ims.internal.uce.common.StatusCode _result = this.getMyInfo(_arg0, _arg1);
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
        case TRANSACTION_responseIncomingOptions:
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
          com.android.ims.internal.uce.options.OptionsCapInfo _arg4;
          if ((0!=data.readInt())) {
            _arg4 = com.android.ims.internal.uce.options.OptionsCapInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          boolean _arg5;
          _arg5 = (0!=data.readInt());
          com.android.ims.internal.uce.common.StatusCode _result = this.responseIncomingOptions(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
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
    private static class Proxy implements com.android.ims.internal.uce.options.IOptionsService
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
           * Gets the version of the Options service implementation.
           * the result of this Call is received in getVersionCb
           * @param optionsServiceHandle, received in serviceCreated() of IOptionsListener.
           * @return StatusCode, status of the request placed.
           * @hide
           */
      @Override public com.android.ims.internal.uce.common.StatusCode getVersion(int optionsServiceHandle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.uce.common.StatusCode _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(optionsServiceHandle);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getVersion, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getVersion(optionsServiceHandle);
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
           * Adds a listener to the Options service.
           * @param optionsServiceHandle, this returned in serviceCreated() of IOptionsListener.
           * @param optionsListener, IOptionsListener object.
           * @param optionsServiceListenerHdl wrapper for client's listener handle to be stored.
           *
           * The service will fill UceLong.mUceLong with optionsServiceListenerHdl
           * @return StatusCode, status of the request placed.
           */
      @Override public com.android.ims.internal.uce.common.StatusCode addListener(int optionsServiceHandle, com.android.ims.internal.uce.options.IOptionsListener optionsListener, com.android.ims.internal.uce.common.UceLong optionsServiceListenerHdl) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.uce.common.StatusCode _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(optionsServiceHandle);
          _data.writeStrongBinder((((optionsListener!=null))?(optionsListener.asBinder()):(null)));
          if ((optionsServiceListenerHdl!=null)) {
            _data.writeInt(1);
            optionsServiceListenerHdl.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_addListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addListener(optionsServiceHandle, optionsListener, optionsServiceListenerHdl);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = com.android.ims.internal.uce.common.StatusCode.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
          if ((0!=_reply.readInt())) {
            optionsServiceListenerHdl.readFromParcel(_reply);
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Removes a listener from the Options service.
           * @param optionsServiceHandle, received in serviceCreated() of IOptionsListener.
           * @param optionsListenerHandle, received in serviceCreated() of IOptionsListener.
           * @param optionsServiceListenerHdl provided in createOptionsService() or Addlistener().
           * @return StatusCode, status of the request placed.
           */
      @Override public com.android.ims.internal.uce.common.StatusCode removeListener(int optionsServiceHandle, com.android.ims.internal.uce.common.UceLong optionsServiceListenerHdl) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.uce.common.StatusCode _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(optionsServiceHandle);
          if ((optionsServiceListenerHdl!=null)) {
            _data.writeInt(1);
            optionsServiceListenerHdl.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().removeListener(optionsServiceHandle, optionsServiceListenerHdl);
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
           * Sets the capabilities information of the self device.
           * The status of the call is received in cmdStatus callback
           * @param optionsServiceHandle, this returned in serviceCreated() of IOptionsListener.
           * @param capInfo, capability information to store.
           * @param reqUserData, userData provided by client to identify the request/API call, it
           *                  is returned in the cmdStatus() callback for client to match response
           *                  with original request.
           * @return StatusCode, status of the request placed.
           */
      @Override public com.android.ims.internal.uce.common.StatusCode setMyInfo(int optionsServiceHandle, com.android.ims.internal.uce.common.CapInfo capInfo, int reqUserData) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.uce.common.StatusCode _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(optionsServiceHandle);
          if ((capInfo!=null)) {
            _data.writeInt(1);
            capInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(reqUserData);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMyInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setMyInfo(optionsServiceHandle, capInfo, reqUserData);
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
           * Gets the capabilities information of remote device.
           * The Capability information is received in cmdStatus callback
           * @param optionsServiceHandle, this returned in serviceCreated() of IOptionsListener.
           * @param reqUserData, userData provided by client to identify the request/API call, it
           *                  is returned in the cmdStatus() callback for client to match response
           *                  with original request.
           * @return StatusCode, status of the request placed.
           */
      @Override public com.android.ims.internal.uce.common.StatusCode getMyInfo(int optionsServiceHandle, int reqUserdata) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.uce.common.StatusCode _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(optionsServiceHandle);
          _data.writeInt(reqUserdata);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMyInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMyInfo(optionsServiceHandle, reqUserdata);
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
           * Requests the capabilities information of a remote URI.
           * the remote party capability is received in sipResponseReceived() callback.
           * @param optionsServiceHandle, this returned in serviceCreated() of IOptionsListener.
           * @param remoteURI, URI of the remote contact.
           * @param reqUserData, userData provided by client to identify the request/API call, it
           *                  is returned in the cmdStatus() callback for client to match response
           *                  with original request.
           * @return StatusCode, status of the request placed.
           */
      @Override public com.android.ims.internal.uce.common.StatusCode getContactCap(int optionsServiceHandle, java.lang.String remoteURI, int reqUserData) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.uce.common.StatusCode _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(optionsServiceHandle);
          _data.writeString(remoteURI);
          _data.writeInt(reqUserData);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getContactCap, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getContactCap(optionsServiceHandle, remoteURI, reqUserData);
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
           * Requests the capabilities information of specified contacts.
           * For each remote party capability is received in sipResponseReceived() callback
           * @param optionsServiceHandle, this returned in serviceCreated() of IOptionsListener.
           * @param remoteURIList, list of remote contact URI's.
           * @param reqUserData, userData provided by client to identify the request/API call, it
           *                  is returned in the cmdStatus() callback for client to match response
           *                  with original request.
           * @return StatusCode, status of the request placed.
           */
      @Override public com.android.ims.internal.uce.common.StatusCode getContactListCap(int optionsServiceHandle, java.lang.String[] remoteURIList, int reqUserData) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.uce.common.StatusCode _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(optionsServiceHandle);
          _data.writeStringArray(remoteURIList);
          _data.writeInt(reqUserData);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getContactListCap, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getContactListCap(optionsServiceHandle, remoteURIList, reqUserData);
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
           * Requests the capabilities information of specified contacts.
           * The incoming Options request is received in incomingOptions() callback.
           *
           * @param optionsServiceHandle, this returned in serviceCreated() of IOptionsListener.
           * @param tId, transaction ID received in incomingOptions() call of IOptionsListener.
           * @param sipResponseCode, SIP response code the UE needs to share to network.
           * @param reasonPhrase, response phrase corresponding to the response code.
           * @param capInfo, capabilities to share in the resonse to network.
           * @param bContactInBL, true if the contact is blacklisted, else false.
           * @return StatusCode, status of the request placed.
           */
      @Override public com.android.ims.internal.uce.common.StatusCode responseIncomingOptions(int optionsServiceHandle, int tId, int sipResponseCode, java.lang.String reasonPhrase, com.android.ims.internal.uce.options.OptionsCapInfo capInfo, boolean bContactInBL) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.uce.common.StatusCode _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(optionsServiceHandle);
          _data.writeInt(tId);
          _data.writeInt(sipResponseCode);
          _data.writeString(reasonPhrase);
          if ((capInfo!=null)) {
            _data.writeInt(1);
            capInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((bContactInBL)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_responseIncomingOptions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().responseIncomingOptions(optionsServiceHandle, tId, sipResponseCode, reasonPhrase, capInfo, bContactInBL);
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
      public static com.android.ims.internal.uce.options.IOptionsService sDefaultImpl;
    }
    static final int TRANSACTION_getVersion = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_addListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_removeListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_setMyInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getMyInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getContactCap = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_getContactListCap = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_responseIncomingOptions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    public static boolean setDefaultImpl(com.android.ims.internal.uce.options.IOptionsService impl) {
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
    public static com.android.ims.internal.uce.options.IOptionsService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Gets the version of the Options service implementation.
       * the result of this Call is received in getVersionCb
       * @param optionsServiceHandle, received in serviceCreated() of IOptionsListener.
       * @return StatusCode, status of the request placed.
       * @hide
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/options/IOptionsService.aidl:36:1:36:25")
  public com.android.ims.internal.uce.common.StatusCode getVersion(int optionsServiceHandle) throws android.os.RemoteException;
  /**
       * Adds a listener to the Options service.
       * @param optionsServiceHandle, this returned in serviceCreated() of IOptionsListener.
       * @param optionsListener, IOptionsListener object.
       * @param optionsServiceListenerHdl wrapper for client's listener handle to be stored.
       *
       * The service will fill UceLong.mUceLong with optionsServiceListenerHdl
       * @return StatusCode, status of the request placed.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/options/IOptionsService.aidl:48:1:48:25")
  public com.android.ims.internal.uce.common.StatusCode addListener(int optionsServiceHandle, com.android.ims.internal.uce.options.IOptionsListener optionsListener, com.android.ims.internal.uce.common.UceLong optionsServiceListenerHdl) throws android.os.RemoteException;
  /**
       * Removes a listener from the Options service.
       * @param optionsServiceHandle, received in serviceCreated() of IOptionsListener.
       * @param optionsListenerHandle, received in serviceCreated() of IOptionsListener.
       * @param optionsServiceListenerHdl provided in createOptionsService() or Addlistener().
       * @return StatusCode, status of the request placed.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/options/IOptionsService.aidl:59:1:59:25")
  public com.android.ims.internal.uce.common.StatusCode removeListener(int optionsServiceHandle, com.android.ims.internal.uce.common.UceLong optionsServiceListenerHdl) throws android.os.RemoteException;
  /**
       * Sets the capabilities information of the self device.
       * The status of the call is received in cmdStatus callback
       * @param optionsServiceHandle, this returned in serviceCreated() of IOptionsListener.
       * @param capInfo, capability information to store.
       * @param reqUserData, userData provided by client to identify the request/API call, it
       *                  is returned in the cmdStatus() callback for client to match response
       *                  with original request.
       * @return StatusCode, status of the request placed.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/options/IOptionsService.aidl:72:1:72:25")
  public com.android.ims.internal.uce.common.StatusCode setMyInfo(int optionsServiceHandle, com.android.ims.internal.uce.common.CapInfo capInfo, int reqUserData) throws android.os.RemoteException;
  /**
       * Gets the capabilities information of remote device.
       * The Capability information is received in cmdStatus callback
       * @param optionsServiceHandle, this returned in serviceCreated() of IOptionsListener.
       * @param reqUserData, userData provided by client to identify the request/API call, it
       *                  is returned in the cmdStatus() callback for client to match response
       *                  with original request.
       * @return StatusCode, status of the request placed.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/options/IOptionsService.aidl:85:1:85:25")
  public com.android.ims.internal.uce.common.StatusCode getMyInfo(int optionsServiceHandle, int reqUserdata) throws android.os.RemoteException;
  /**
       * Requests the capabilities information of a remote URI.
       * the remote party capability is received in sipResponseReceived() callback.
       * @param optionsServiceHandle, this returned in serviceCreated() of IOptionsListener.
       * @param remoteURI, URI of the remote contact.
       * @param reqUserData, userData provided by client to identify the request/API call, it
       *                  is returned in the cmdStatus() callback for client to match response
       *                  with original request.
       * @return StatusCode, status of the request placed.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/options/IOptionsService.aidl:98:1:98:25")
  public com.android.ims.internal.uce.common.StatusCode getContactCap(int optionsServiceHandle, java.lang.String remoteURI, int reqUserData) throws android.os.RemoteException;
  /**
       * Requests the capabilities information of specified contacts.
       * For each remote party capability is received in sipResponseReceived() callback
       * @param optionsServiceHandle, this returned in serviceCreated() of IOptionsListener.
       * @param remoteURIList, list of remote contact URI's.
       * @param reqUserData, userData provided by client to identify the request/API call, it
       *                  is returned in the cmdStatus() callback for client to match response
       *                  with original request.
       * @return StatusCode, status of the request placed.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/options/IOptionsService.aidl:112:1:112:25")
  public com.android.ims.internal.uce.common.StatusCode getContactListCap(int optionsServiceHandle, java.lang.String[] remoteURIList, int reqUserData) throws android.os.RemoteException;
  /**
       * Requests the capabilities information of specified contacts.
       * The incoming Options request is received in incomingOptions() callback.
       *
       * @param optionsServiceHandle, this returned in serviceCreated() of IOptionsListener.
       * @param tId, transaction ID received in incomingOptions() call of IOptionsListener.
       * @param sipResponseCode, SIP response code the UE needs to share to network.
       * @param reasonPhrase, response phrase corresponding to the response code.
       * @param capInfo, capabilities to share in the resonse to network.
       * @param bContactInBL, true if the contact is blacklisted, else false.
       * @return StatusCode, status of the request placed.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/options/IOptionsService.aidl:129:1:129:25")
  public com.android.ims.internal.uce.common.StatusCode responseIncomingOptions(int optionsServiceHandle, int tId, int sipResponseCode, java.lang.String reasonPhrase, com.android.ims.internal.uce.options.OptionsCapInfo capInfo, boolean bContactInBL) throws android.os.RemoteException;
}
