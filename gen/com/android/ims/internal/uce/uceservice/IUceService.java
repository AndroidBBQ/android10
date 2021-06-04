/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.ims.internal.uce.uceservice;
/** IUceService
 *  UCE service interface class.
 * {@hide} */
public interface IUceService extends android.os.IInterface
{
  /** Default implementation for IUceService. */
  public static class Default implements com.android.ims.internal.uce.uceservice.IUceService
  {
    /**
         * Starts the Uce service.
         * @param uceListener IUceListener object
         * @return boolean true if the service stop start is processed successfully, FALSE otherwise.
         *
         * Service status is returned in setStatus callback in IUceListener.
         * @hide
         */
    @Override public boolean startService(com.android.ims.internal.uce.uceservice.IUceListener uceListener) throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Stops the UCE service.
         * @return boolean true if the service stop request is processed successfully, FALSE otherwise.
         * @hide
         */
    @Override public boolean stopService() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Requests the UCE service start status.
         * @return boolean true if service started else false.
         * @hide
         */
    @Override public boolean isServiceStarted() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Creates a options service for Capability Discovery.
         * @param optionsListener IOptionsListener object.
         * @param optionsServiceListenerHdl wrapper for client's listener handle to be stored.
         *
         * The service will fill UceLong.mUceLong with presenceListenerHandle allocated and
         * used to validate callbacks received in IPresenceListener are indeed from the
         * service the client created.
         *
         * @return  optionsServiceHandle
         *
         * @hide
         *
         * @deprecated This is replaced with new API createOptionsServiceForSubscription()
         */
    @Override public int createOptionsService(com.android.ims.internal.uce.options.IOptionsListener optionsListener, com.android.ims.internal.uce.common.UceLong optionsServiceListenerHdl) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Creates a options service for Capability Discovery.
         * @param optionsListener IOptionsListener object.
         * @param optionsServiceListenerHdl wrapper for client's listener handle to be stored.
         * @param iccId the ICC-ID derived from SubscriptionInfo for the Service requested
         *
         * The service will fill UceLong.mUceLong with presenceListenerHandle allocated and
         * used to validate callbacks received in IPresenceListener are indeed from the
         * service the client created.
         *
         * @return  optionsServiceHandle
         *
         * @hide
         */
    @Override public int createOptionsServiceForSubscription(com.android.ims.internal.uce.options.IOptionsListener optionsListener, com.android.ims.internal.uce.common.UceLong optionsServiceListenerHdl, java.lang.String iccId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Destroys a Options service.
         * @param optionsServiceHandle this is received in serviceCreated() callback
         *        in IOptionsListener
         * @hide
         */
    @Override public void destroyOptionsService(int optionsServiceHandle) throws android.os.RemoteException
    {
    }
    /**
         * Creates a presence service.
         * @param presenceServiceListener IPresenceListener object
         * @param presenceServiceListenerHdl wrapper for client's listener handle to be stored.
         *
         * The service will fill UceLong.mUceLong with presenceListenerHandle allocated and
         * used to validate callbacks received in IPresenceListener are indeed from the
         * service the client created.
         *
         * @return  presenceServiceHdl
         *
         * @hide
         *
         * @deprecated This is replaced with new API createPresenceServiceForSubscription()
         */
    @Override public int createPresenceService(com.android.ims.internal.uce.presence.IPresenceListener presenceServiceListener, com.android.ims.internal.uce.common.UceLong presenceServiceListenerHdl) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Creates a presence service.
         * @param presenceServiceListener IPresenceListener object
         * @param presenceServiceListenerHdl wrapper for client's listener handle to be stored.
         * @param iccId the ICC-ID derived from SubscriptionInfo for the Service requested
         *
         * The service will fill UceLong.mUceLong with presenceListenerHandle allocated and
         * used to validate callbacks received in IPresenceListener are indeed from the
         * service the client created.
         *
         * @return  presenceServiceHdl
         *
         * @hide
         */
    @Override public int createPresenceServiceForSubscription(com.android.ims.internal.uce.presence.IPresenceListener presenceServiceListener, com.android.ims.internal.uce.common.UceLong presenceServiceListenerHdl, java.lang.String iccId) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Destroys a presence service.
         *
         * @param presenceServiceHdl handle returned during createPresenceService()
         *
         * @hide
         */
    @Override public void destroyPresenceService(int presenceServiceHdl) throws android.os.RemoteException
    {
    }
    /**
         * Query the UCE Service for information to know whether the is registered.
         *
         * @return boolean, true if Registered to for network events else false.
         *
         * @hide
         */
    @Override public boolean getServiceStatus() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Query the UCE Service for presence Service.
         *
         * @return IPresenceService object.
         *
         * @hide
         *
         * @deprecated use API getPresenceServiceForSubscription()
         */
    @Override public com.android.ims.internal.uce.presence.IPresenceService getPresenceService() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Query the UCE Service for presence Service.
         *
         * @param iccId the ICC-ID derived from SubscriptionInfo for the Service requested
         *
         * @return IPresenceService object.
         *
         * @hide
         */
    @Override public com.android.ims.internal.uce.presence.IPresenceService getPresenceServiceForSubscription(java.lang.String iccId) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Query the UCE Service for options service object.
         *
         * @return IOptionsService object.
         *
         * @deprecated use API getOptionsServiceForSubscription()
         *
         * @hide
         */
    @Override public com.android.ims.internal.uce.options.IOptionsService getOptionsService() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Query the UCE Service for options service object.
         *
         * @param iccId the ICC-ID derived from SubscriptionInfo for the Service requested
         *
         * @return IOptionsService object.
         *
         * @hide
         */
    @Override public com.android.ims.internal.uce.options.IOptionsService getOptionsServiceForSubscription(java.lang.String iccId) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.ims.internal.uce.uceservice.IUceService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.ims.internal.uce.uceservice.IUceService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.ims.internal.uce.uceservice.IUceService interface,
     * generating a proxy if needed.
     */
    public static com.android.ims.internal.uce.uceservice.IUceService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.ims.internal.uce.uceservice.IUceService))) {
        return ((com.android.ims.internal.uce.uceservice.IUceService)iin);
      }
      return new com.android.ims.internal.uce.uceservice.IUceService.Stub.Proxy(obj);
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
        case TRANSACTION_startService:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.uce.uceservice.IUceListener _arg0;
          _arg0 = com.android.ims.internal.uce.uceservice.IUceListener.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.startService(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_stopService:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.stopService();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isServiceStarted:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isServiceStarted();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_createOptionsService:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.uce.options.IOptionsListener _arg0;
          _arg0 = com.android.ims.internal.uce.options.IOptionsListener.Stub.asInterface(data.readStrongBinder());
          com.android.ims.internal.uce.common.UceLong _arg1;
          if ((0!=data.readInt())) {
            _arg1 = com.android.ims.internal.uce.common.UceLong.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _result = this.createOptionsService(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          if ((_arg1!=null)) {
            reply.writeInt(1);
            _arg1.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_createOptionsServiceForSubscription:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.uce.options.IOptionsListener _arg0;
          _arg0 = com.android.ims.internal.uce.options.IOptionsListener.Stub.asInterface(data.readStrongBinder());
          com.android.ims.internal.uce.common.UceLong _arg1;
          if ((0!=data.readInt())) {
            _arg1 = com.android.ims.internal.uce.common.UceLong.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _result = this.createOptionsServiceForSubscription(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          if ((_arg1!=null)) {
            reply.writeInt(1);
            _arg1.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_destroyOptionsService:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.destroyOptionsService(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_createPresenceService:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.uce.presence.IPresenceListener _arg0;
          _arg0 = com.android.ims.internal.uce.presence.IPresenceListener.Stub.asInterface(data.readStrongBinder());
          com.android.ims.internal.uce.common.UceLong _arg1;
          if ((0!=data.readInt())) {
            _arg1 = com.android.ims.internal.uce.common.UceLong.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _result = this.createPresenceService(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          if ((_arg1!=null)) {
            reply.writeInt(1);
            _arg1.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_createPresenceServiceForSubscription:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.uce.presence.IPresenceListener _arg0;
          _arg0 = com.android.ims.internal.uce.presence.IPresenceListener.Stub.asInterface(data.readStrongBinder());
          com.android.ims.internal.uce.common.UceLong _arg1;
          if ((0!=data.readInt())) {
            _arg1 = com.android.ims.internal.uce.common.UceLong.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _result = this.createPresenceServiceForSubscription(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          if ((_arg1!=null)) {
            reply.writeInt(1);
            _arg1.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_destroyPresenceService:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.destroyPresenceService(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getServiceStatus:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.getServiceStatus();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getPresenceService:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.uce.presence.IPresenceService _result = this.getPresenceService();
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_getPresenceServiceForSubscription:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          com.android.ims.internal.uce.presence.IPresenceService _result = this.getPresenceServiceForSubscription(_arg0);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_getOptionsService:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.uce.options.IOptionsService _result = this.getOptionsService();
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_getOptionsServiceForSubscription:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          com.android.ims.internal.uce.options.IOptionsService _result = this.getOptionsServiceForSubscription(_arg0);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.ims.internal.uce.uceservice.IUceService
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
           * Starts the Uce service.
           * @param uceListener IUceListener object
           * @return boolean true if the service stop start is processed successfully, FALSE otherwise.
           *
           * Service status is returned in setStatus callback in IUceListener.
           * @hide
           */
      @Override public boolean startService(com.android.ims.internal.uce.uceservice.IUceListener uceListener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((uceListener!=null))?(uceListener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startService(uceListener);
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
           * Stops the UCE service.
           * @return boolean true if the service stop request is processed successfully, FALSE otherwise.
           * @hide
           */
      @Override public boolean stopService() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().stopService();
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
           * Requests the UCE service start status.
           * @return boolean true if service started else false.
           * @hide
           */
      @Override public boolean isServiceStarted() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isServiceStarted, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isServiceStarted();
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
           * Creates a options service for Capability Discovery.
           * @param optionsListener IOptionsListener object.
           * @param optionsServiceListenerHdl wrapper for client's listener handle to be stored.
           *
           * The service will fill UceLong.mUceLong with presenceListenerHandle allocated and
           * used to validate callbacks received in IPresenceListener are indeed from the
           * service the client created.
           *
           * @return  optionsServiceHandle
           *
           * @hide
           *
           * @deprecated This is replaced with new API createOptionsServiceForSubscription()
           */
      @Override public int createOptionsService(com.android.ims.internal.uce.options.IOptionsListener optionsListener, com.android.ims.internal.uce.common.UceLong optionsServiceListenerHdl) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((optionsListener!=null))?(optionsListener.asBinder()):(null)));
          if ((optionsServiceListenerHdl!=null)) {
            _data.writeInt(1);
            optionsServiceListenerHdl.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_createOptionsService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createOptionsService(optionsListener, optionsServiceListenerHdl);
          }
          _reply.readException();
          _result = _reply.readInt();
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
           * Creates a options service for Capability Discovery.
           * @param optionsListener IOptionsListener object.
           * @param optionsServiceListenerHdl wrapper for client's listener handle to be stored.
           * @param iccId the ICC-ID derived from SubscriptionInfo for the Service requested
           *
           * The service will fill UceLong.mUceLong with presenceListenerHandle allocated and
           * used to validate callbacks received in IPresenceListener are indeed from the
           * service the client created.
           *
           * @return  optionsServiceHandle
           *
           * @hide
           */
      @Override public int createOptionsServiceForSubscription(com.android.ims.internal.uce.options.IOptionsListener optionsListener, com.android.ims.internal.uce.common.UceLong optionsServiceListenerHdl, java.lang.String iccId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((optionsListener!=null))?(optionsListener.asBinder()):(null)));
          if ((optionsServiceListenerHdl!=null)) {
            _data.writeInt(1);
            optionsServiceListenerHdl.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(iccId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createOptionsServiceForSubscription, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createOptionsServiceForSubscription(optionsListener, optionsServiceListenerHdl, iccId);
          }
          _reply.readException();
          _result = _reply.readInt();
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
           * Destroys a Options service.
           * @param optionsServiceHandle this is received in serviceCreated() callback
           *        in IOptionsListener
           * @hide
           */
      @Override public void destroyOptionsService(int optionsServiceHandle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(optionsServiceHandle);
          boolean _status = mRemote.transact(Stub.TRANSACTION_destroyOptionsService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().destroyOptionsService(optionsServiceHandle);
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
           * Creates a presence service.
           * @param presenceServiceListener IPresenceListener object
           * @param presenceServiceListenerHdl wrapper for client's listener handle to be stored.
           *
           * The service will fill UceLong.mUceLong with presenceListenerHandle allocated and
           * used to validate callbacks received in IPresenceListener are indeed from the
           * service the client created.
           *
           * @return  presenceServiceHdl
           *
           * @hide
           *
           * @deprecated This is replaced with new API createPresenceServiceForSubscription()
           */
      @Override public int createPresenceService(com.android.ims.internal.uce.presence.IPresenceListener presenceServiceListener, com.android.ims.internal.uce.common.UceLong presenceServiceListenerHdl) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((presenceServiceListener!=null))?(presenceServiceListener.asBinder()):(null)));
          if ((presenceServiceListenerHdl!=null)) {
            _data.writeInt(1);
            presenceServiceListenerHdl.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_createPresenceService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createPresenceService(presenceServiceListener, presenceServiceListenerHdl);
          }
          _reply.readException();
          _result = _reply.readInt();
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
           * Creates a presence service.
           * @param presenceServiceListener IPresenceListener object
           * @param presenceServiceListenerHdl wrapper for client's listener handle to be stored.
           * @param iccId the ICC-ID derived from SubscriptionInfo for the Service requested
           *
           * The service will fill UceLong.mUceLong with presenceListenerHandle allocated and
           * used to validate callbacks received in IPresenceListener are indeed from the
           * service the client created.
           *
           * @return  presenceServiceHdl
           *
           * @hide
           */
      @Override public int createPresenceServiceForSubscription(com.android.ims.internal.uce.presence.IPresenceListener presenceServiceListener, com.android.ims.internal.uce.common.UceLong presenceServiceListenerHdl, java.lang.String iccId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((presenceServiceListener!=null))?(presenceServiceListener.asBinder()):(null)));
          if ((presenceServiceListenerHdl!=null)) {
            _data.writeInt(1);
            presenceServiceListenerHdl.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(iccId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createPresenceServiceForSubscription, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createPresenceServiceForSubscription(presenceServiceListener, presenceServiceListenerHdl, iccId);
          }
          _reply.readException();
          _result = _reply.readInt();
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
           * Destroys a presence service.
           *
           * @param presenceServiceHdl handle returned during createPresenceService()
           *
           * @hide
           */
      @Override public void destroyPresenceService(int presenceServiceHdl) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(presenceServiceHdl);
          boolean _status = mRemote.transact(Stub.TRANSACTION_destroyPresenceService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().destroyPresenceService(presenceServiceHdl);
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
           * Query the UCE Service for information to know whether the is registered.
           *
           * @return boolean, true if Registered to for network events else false.
           *
           * @hide
           */
      @Override public boolean getServiceStatus() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getServiceStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getServiceStatus();
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
           * Query the UCE Service for presence Service.
           *
           * @return IPresenceService object.
           *
           * @hide
           *
           * @deprecated use API getPresenceServiceForSubscription()
           */
      @Override public com.android.ims.internal.uce.presence.IPresenceService getPresenceService() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.uce.presence.IPresenceService _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPresenceService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPresenceService();
          }
          _reply.readException();
          _result = com.android.ims.internal.uce.presence.IPresenceService.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Query the UCE Service for presence Service.
           *
           * @param iccId the ICC-ID derived from SubscriptionInfo for the Service requested
           *
           * @return IPresenceService object.
           *
           * @hide
           */
      @Override public com.android.ims.internal.uce.presence.IPresenceService getPresenceServiceForSubscription(java.lang.String iccId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.uce.presence.IPresenceService _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iccId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPresenceServiceForSubscription, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPresenceServiceForSubscription(iccId);
          }
          _reply.readException();
          _result = com.android.ims.internal.uce.presence.IPresenceService.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Query the UCE Service for options service object.
           *
           * @return IOptionsService object.
           *
           * @deprecated use API getOptionsServiceForSubscription()
           *
           * @hide
           */
      @Override public com.android.ims.internal.uce.options.IOptionsService getOptionsService() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.uce.options.IOptionsService _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getOptionsService, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getOptionsService();
          }
          _reply.readException();
          _result = com.android.ims.internal.uce.options.IOptionsService.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Query the UCE Service for options service object.
           *
           * @param iccId the ICC-ID derived from SubscriptionInfo for the Service requested
           *
           * @return IOptionsService object.
           *
           * @hide
           */
      @Override public com.android.ims.internal.uce.options.IOptionsService getOptionsServiceForSubscription(java.lang.String iccId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        com.android.ims.internal.uce.options.IOptionsService _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(iccId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getOptionsServiceForSubscription, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getOptionsServiceForSubscription(iccId);
          }
          _reply.readException();
          _result = com.android.ims.internal.uce.options.IOptionsService.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static com.android.ims.internal.uce.uceservice.IUceService sDefaultImpl;
    }
    static final int TRANSACTION_startService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_stopService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_isServiceStarted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_createOptionsService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_createOptionsServiceForSubscription = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_destroyOptionsService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_createPresenceService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_createPresenceServiceForSubscription = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_destroyPresenceService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getServiceStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_getPresenceService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_getPresenceServiceForSubscription = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_getOptionsService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_getOptionsServiceForSubscription = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    public static boolean setDefaultImpl(com.android.ims.internal.uce.uceservice.IUceService impl) {
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
    public static com.android.ims.internal.uce.uceservice.IUceService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Starts the Uce service.
       * @param uceListener IUceListener object
       * @return boolean true if the service stop start is processed successfully, FALSE otherwise.
       *
       * Service status is returned in setStatus callback in IUceListener.
       * @hide
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/uceservice/IUceService.aidl:41:1:41:25")
  public boolean startService(com.android.ims.internal.uce.uceservice.IUceListener uceListener) throws android.os.RemoteException;
  /**
       * Stops the UCE service.
       * @return boolean true if the service stop request is processed successfully, FALSE otherwise.
       * @hide
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/uceservice/IUceService.aidl:49:1:49:25")
  public boolean stopService() throws android.os.RemoteException;
  /**
       * Requests the UCE service start status.
       * @return boolean true if service started else false.
       * @hide
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/uceservice/IUceService.aidl:59:1:59:25")
  public boolean isServiceStarted() throws android.os.RemoteException;
  /**
       * Creates a options service for Capability Discovery.
       * @param optionsListener IOptionsListener object.
       * @param optionsServiceListenerHdl wrapper for client's listener handle to be stored.
       *
       * The service will fill UceLong.mUceLong with presenceListenerHandle allocated and
       * used to validate callbacks received in IPresenceListener are indeed from the
       * service the client created.
       *
       * @return  optionsServiceHandle
       *
       * @hide
       *
       * @deprecated This is replaced with new API createOptionsServiceForSubscription()
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/uceservice/IUceService.aidl:77:1:77:25")
  public int createOptionsService(com.android.ims.internal.uce.options.IOptionsListener optionsListener, com.android.ims.internal.uce.common.UceLong optionsServiceListenerHdl) throws android.os.RemoteException;
  /**
       * Creates a options service for Capability Discovery.
       * @param optionsListener IOptionsListener object.
       * @param optionsServiceListenerHdl wrapper for client's listener handle to be stored.
       * @param iccId the ICC-ID derived from SubscriptionInfo for the Service requested
       *
       * The service will fill UceLong.mUceLong with presenceListenerHandle allocated and
       * used to validate callbacks received in IPresenceListener are indeed from the
       * service the client created.
       *
       * @return  optionsServiceHandle
       *
       * @hide
       */
  public int createOptionsServiceForSubscription(com.android.ims.internal.uce.options.IOptionsListener optionsListener, com.android.ims.internal.uce.common.UceLong optionsServiceListenerHdl, java.lang.String iccId) throws android.os.RemoteException;
  /**
       * Destroys a Options service.
       * @param optionsServiceHandle this is received in serviceCreated() callback
       *        in IOptionsListener
       * @hide
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/uceservice/IUceService.aidl:104:1:104:25")
  public void destroyOptionsService(int optionsServiceHandle) throws android.os.RemoteException;
  /**
       * Creates a presence service.
       * @param presenceServiceListener IPresenceListener object
       * @param presenceServiceListenerHdl wrapper for client's listener handle to be stored.
       *
       * The service will fill UceLong.mUceLong with presenceListenerHandle allocated and
       * used to validate callbacks received in IPresenceListener are indeed from the
       * service the client created.
       *
       * @return  presenceServiceHdl
       *
       * @hide
       *
       * @deprecated This is replaced with new API createPresenceServiceForSubscription()
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/uceservice/IUceService.aidl:122:1:122:25")
  public int createPresenceService(com.android.ims.internal.uce.presence.IPresenceListener presenceServiceListener, com.android.ims.internal.uce.common.UceLong presenceServiceListenerHdl) throws android.os.RemoteException;
  /**
       * Creates a presence service.
       * @param presenceServiceListener IPresenceListener object
       * @param presenceServiceListenerHdl wrapper for client's listener handle to be stored.
       * @param iccId the ICC-ID derived from SubscriptionInfo for the Service requested
       *
       * The service will fill UceLong.mUceLong with presenceListenerHandle allocated and
       * used to validate callbacks received in IPresenceListener are indeed from the
       * service the client created.
       *
       * @return  presenceServiceHdl
       *
       * @hide
       */
  public int createPresenceServiceForSubscription(com.android.ims.internal.uce.presence.IPresenceListener presenceServiceListener, com.android.ims.internal.uce.common.UceLong presenceServiceListenerHdl, java.lang.String iccId) throws android.os.RemoteException;
  /**
       * Destroys a presence service.
       *
       * @param presenceServiceHdl handle returned during createPresenceService()
       *
       * @hide
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/uceservice/IUceService.aidl:150:1:150:25")
  public void destroyPresenceService(int presenceServiceHdl) throws android.os.RemoteException;
  /**
       * Query the UCE Service for information to know whether the is registered.
       *
       * @return boolean, true if Registered to for network events else false.
       *
       * @hide
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/uceservice/IUceService.aidl:162:1:162:25")
  public boolean getServiceStatus() throws android.os.RemoteException;
  /**
       * Query the UCE Service for presence Service.
       *
       * @return IPresenceService object.
       *
       * @hide
       *
       * @deprecated use API getPresenceServiceForSubscription()
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/uceservice/IUceService.aidl:174:1:174:25")
  public com.android.ims.internal.uce.presence.IPresenceService getPresenceService() throws android.os.RemoteException;
  /**
       * Query the UCE Service for presence Service.
       *
       * @param iccId the ICC-ID derived from SubscriptionInfo for the Service requested
       *
       * @return IPresenceService object.
       *
       * @hide
       */
  public com.android.ims.internal.uce.presence.IPresenceService getPresenceServiceForSubscription(java.lang.String iccId) throws android.os.RemoteException;
  /**
       * Query the UCE Service for options service object.
       *
       * @return IOptionsService object.
       *
       * @deprecated use API getOptionsServiceForSubscription()
       *
       * @hide
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/uceservice/IUceService.aidl:197:1:197:25")
  public com.android.ims.internal.uce.options.IOptionsService getOptionsService() throws android.os.RemoteException;
  /**
       * Query the UCE Service for options service object.
       *
       * @param iccId the ICC-ID derived from SubscriptionInfo for the Service requested
       *
       * @return IOptionsService object.
       *
       * @hide
       */
  public com.android.ims.internal.uce.options.IOptionsService getOptionsServiceForSubscription(java.lang.String iccId) throws android.os.RemoteException;
}
