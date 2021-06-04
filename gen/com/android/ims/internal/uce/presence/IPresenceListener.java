/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.ims.internal.uce.presence;
/**
 * IPresenceListener
 * {@hide} */
public interface IPresenceListener extends android.os.IInterface
{
  /** Default implementation for IPresenceListener. */
  public static class Default implements com.android.ims.internal.uce.presence.IPresenceListener
  {
    /**
         * Gets the version of the presence listener implementation.
         * @param version, version information.
         */
    @Override public void getVersionCb(java.lang.String version) throws android.os.RemoteException
    {
    }
    /**
         * Callback function to be invoked by the Presence service to notify the listener of service
         * availability.
         * @param statusCode, UCE_SUCCESS as service availability.
         */
    @Override public void serviceAvailable(com.android.ims.internal.uce.common.StatusCode statusCode) throws android.os.RemoteException
    {
    }
    /**
         * Callback function to be invoked by the Presence service to notify the listener of service
         * unavailability.
         * @param statusCode, UCE_SUCCESS as service unAvailability.
         */
    @Override public void serviceUnAvailable(com.android.ims.internal.uce.common.StatusCode statusCode) throws android.os.RemoteException
    {
    }
    /**
         * Callback function to be invoked by the Presence service to notify the listener to send a
         * publish request.
         * @param publishTrigger, Publish trigger for the network being supported.
         */
    @Override public void publishTriggering(com.android.ims.internal.uce.presence.PresPublishTriggerType publishTrigger) throws android.os.RemoteException
    {
    }
    /**
         * Callback function to be invoked to inform the client of the status of an asynchronous call.
         * @param cmdStatus, command status of the request placed.
         */
    @Override public void cmdStatus(com.android.ims.internal.uce.presence.PresCmdStatus cmdStatus) throws android.os.RemoteException
    {
    }
    /**
         * Callback function to be invoked to inform the client when the response for a SIP message,
         * such as PUBLISH or SUBSCRIBE, has been received.
         * @param sipResponse, network response received for the request placed.
         */
    @Override public void sipResponseReceived(com.android.ims.internal.uce.presence.PresSipResponse sipResponse) throws android.os.RemoteException
    {
    }
    /**
         * Callback function to be invoked to inform the client when the NOTIFY message carrying a
         * single contact's capabilities information is received.
         * @param presentityURI, URI of the remote entity the request was placed.
         * @param tupleInfo, array of capability information remote entity supports.
         */
    @Override public void capInfoReceived(java.lang.String presentityURI, com.android.ims.internal.uce.presence.PresTupleInfo[] tupleInfo) throws android.os.RemoteException
    {
    }
    /**
         * Callback function to be invoked to inform the client when the NOTIFY message carrying
         * contact's capabilities information is received.
         * @param rlmiInfo, resource infomation received from network.
         * @param resInfo, array of capabilities received from network for the list of  remore URI.
         */
    @Override public void listCapInfoReceived(com.android.ims.internal.uce.presence.PresRlmiInfo rlmiInfo, com.android.ims.internal.uce.presence.PresResInfo[] resInfo) throws android.os.RemoteException
    {
    }
    /**
         * Callback function to be invoked to inform the client when Unpublish message
         * is sent to network.
         */
    @Override public void unpublishMessageSent() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.ims.internal.uce.presence.IPresenceListener
  {
    private static final java.lang.String DESCRIPTOR = "com.android.ims.internal.uce.presence.IPresenceListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.ims.internal.uce.presence.IPresenceListener interface,
     * generating a proxy if needed.
     */
    public static com.android.ims.internal.uce.presence.IPresenceListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.ims.internal.uce.presence.IPresenceListener))) {
        return ((com.android.ims.internal.uce.presence.IPresenceListener)iin);
      }
      return new com.android.ims.internal.uce.presence.IPresenceListener.Stub.Proxy(obj);
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
        case TRANSACTION_getVersionCb:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.getVersionCb(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_serviceAvailable:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.uce.common.StatusCode _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.ims.internal.uce.common.StatusCode.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.serviceAvailable(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_serviceUnAvailable:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.uce.common.StatusCode _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.ims.internal.uce.common.StatusCode.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.serviceUnAvailable(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_publishTriggering:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.uce.presence.PresPublishTriggerType _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.ims.internal.uce.presence.PresPublishTriggerType.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.publishTriggering(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_cmdStatus:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.uce.presence.PresCmdStatus _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.ims.internal.uce.presence.PresCmdStatus.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.cmdStatus(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_sipResponseReceived:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.uce.presence.PresSipResponse _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.ims.internal.uce.presence.PresSipResponse.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.sipResponseReceived(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_capInfoReceived:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          com.android.ims.internal.uce.presence.PresTupleInfo[] _arg1;
          _arg1 = data.createTypedArray(com.android.ims.internal.uce.presence.PresTupleInfo.CREATOR);
          this.capInfoReceived(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_listCapInfoReceived:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.uce.presence.PresRlmiInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.ims.internal.uce.presence.PresRlmiInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          com.android.ims.internal.uce.presence.PresResInfo[] _arg1;
          _arg1 = data.createTypedArray(com.android.ims.internal.uce.presence.PresResInfo.CREATOR);
          this.listCapInfoReceived(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unpublishMessageSent:
        {
          data.enforceInterface(descriptor);
          this.unpublishMessageSent();
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.ims.internal.uce.presence.IPresenceListener
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
           * Gets the version of the presence listener implementation.
           * @param version, version information.
           */
      @Override public void getVersionCb(java.lang.String version) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(version);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getVersionCb, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getVersionCb(version);
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
           * Callback function to be invoked by the Presence service to notify the listener of service
           * availability.
           * @param statusCode, UCE_SUCCESS as service availability.
           */
      @Override public void serviceAvailable(com.android.ims.internal.uce.common.StatusCode statusCode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((statusCode!=null)) {
            _data.writeInt(1);
            statusCode.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_serviceAvailable, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().serviceAvailable(statusCode);
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
           * Callback function to be invoked by the Presence service to notify the listener of service
           * unavailability.
           * @param statusCode, UCE_SUCCESS as service unAvailability.
           */
      @Override public void serviceUnAvailable(com.android.ims.internal.uce.common.StatusCode statusCode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((statusCode!=null)) {
            _data.writeInt(1);
            statusCode.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_serviceUnAvailable, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().serviceUnAvailable(statusCode);
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
           * Callback function to be invoked by the Presence service to notify the listener to send a
           * publish request.
           * @param publishTrigger, Publish trigger for the network being supported.
           */
      @Override public void publishTriggering(com.android.ims.internal.uce.presence.PresPublishTriggerType publishTrigger) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((publishTrigger!=null)) {
            _data.writeInt(1);
            publishTrigger.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_publishTriggering, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().publishTriggering(publishTrigger);
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
           * Callback function to be invoked to inform the client of the status of an asynchronous call.
           * @param cmdStatus, command status of the request placed.
           */
      @Override public void cmdStatus(com.android.ims.internal.uce.presence.PresCmdStatus cmdStatus) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((cmdStatus!=null)) {
            _data.writeInt(1);
            cmdStatus.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_cmdStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cmdStatus(cmdStatus);
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
           * Callback function to be invoked to inform the client when the response for a SIP message,
           * such as PUBLISH or SUBSCRIBE, has been received.
           * @param sipResponse, network response received for the request placed.
           */
      @Override public void sipResponseReceived(com.android.ims.internal.uce.presence.PresSipResponse sipResponse) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((sipResponse!=null)) {
            _data.writeInt(1);
            sipResponse.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_sipResponseReceived, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sipResponseReceived(sipResponse);
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
           * Callback function to be invoked to inform the client when the NOTIFY message carrying a
           * single contact's capabilities information is received.
           * @param presentityURI, URI of the remote entity the request was placed.
           * @param tupleInfo, array of capability information remote entity supports.
           */
      @Override public void capInfoReceived(java.lang.String presentityURI, com.android.ims.internal.uce.presence.PresTupleInfo[] tupleInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(presentityURI);
          _data.writeTypedArray(tupleInfo, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_capInfoReceived, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().capInfoReceived(presentityURI, tupleInfo);
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
           * Callback function to be invoked to inform the client when the NOTIFY message carrying
           * contact's capabilities information is received.
           * @param rlmiInfo, resource infomation received from network.
           * @param resInfo, array of capabilities received from network for the list of  remore URI.
           */
      @Override public void listCapInfoReceived(com.android.ims.internal.uce.presence.PresRlmiInfo rlmiInfo, com.android.ims.internal.uce.presence.PresResInfo[] resInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((rlmiInfo!=null)) {
            _data.writeInt(1);
            rlmiInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeTypedArray(resInfo, 0);
          boolean _status = mRemote.transact(Stub.TRANSACTION_listCapInfoReceived, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().listCapInfoReceived(rlmiInfo, resInfo);
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
           * Callback function to be invoked to inform the client when Unpublish message
           * is sent to network.
           */
      @Override public void unpublishMessageSent() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unpublishMessageSent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unpublishMessageSent();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.ims.internal.uce.presence.IPresenceListener sDefaultImpl;
    }
    static final int TRANSACTION_getVersionCb = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_serviceAvailable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_serviceUnAvailable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_publishTriggering = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_cmdStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_sipResponseReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_capInfoReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_listCapInfoReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_unpublishMessageSent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    public static boolean setDefaultImpl(com.android.ims.internal.uce.presence.IPresenceListener impl) {
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
    public static com.android.ims.internal.uce.presence.IPresenceListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Gets the version of the presence listener implementation.
       * @param version, version information.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/presence/IPresenceListener.aidl:39:1:39:25")
  public void getVersionCb(java.lang.String version) throws android.os.RemoteException;
  /**
       * Callback function to be invoked by the Presence service to notify the listener of service
       * availability.
       * @param statusCode, UCE_SUCCESS as service availability.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/presence/IPresenceListener.aidl:47:1:47:25")
  public void serviceAvailable(com.android.ims.internal.uce.common.StatusCode statusCode) throws android.os.RemoteException;
  /**
       * Callback function to be invoked by the Presence service to notify the listener of service
       * unavailability.
       * @param statusCode, UCE_SUCCESS as service unAvailability.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/presence/IPresenceListener.aidl:55:1:55:25")
  public void serviceUnAvailable(com.android.ims.internal.uce.common.StatusCode statusCode) throws android.os.RemoteException;
  /**
       * Callback function to be invoked by the Presence service to notify the listener to send a
       * publish request.
       * @param publishTrigger, Publish trigger for the network being supported.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/presence/IPresenceListener.aidl:63:1:63:25")
  public void publishTriggering(com.android.ims.internal.uce.presence.PresPublishTriggerType publishTrigger) throws android.os.RemoteException;
  /**
       * Callback function to be invoked to inform the client of the status of an asynchronous call.
       * @param cmdStatus, command status of the request placed.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/presence/IPresenceListener.aidl:70:1:70:25")
  public void cmdStatus(com.android.ims.internal.uce.presence.PresCmdStatus cmdStatus) throws android.os.RemoteException;
  /**
       * Callback function to be invoked to inform the client when the response for a SIP message,
       * such as PUBLISH or SUBSCRIBE, has been received.
       * @param sipResponse, network response received for the request placed.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/presence/IPresenceListener.aidl:78:1:78:25")
  public void sipResponseReceived(com.android.ims.internal.uce.presence.PresSipResponse sipResponse) throws android.os.RemoteException;
  /**
       * Callback function to be invoked to inform the client when the NOTIFY message carrying a
       * single contact's capabilities information is received.
       * @param presentityURI, URI of the remote entity the request was placed.
       * @param tupleInfo, array of capability information remote entity supports.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/presence/IPresenceListener.aidl:87:1:87:25")
  public void capInfoReceived(java.lang.String presentityURI, com.android.ims.internal.uce.presence.PresTupleInfo[] tupleInfo) throws android.os.RemoteException;
  /**
       * Callback function to be invoked to inform the client when the NOTIFY message carrying
       * contact's capabilities information is received.
       * @param rlmiInfo, resource infomation received from network.
       * @param resInfo, array of capabilities received from network for the list of  remore URI.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/presence/IPresenceListener.aidl:97:1:97:25")
  public void listCapInfoReceived(com.android.ims.internal.uce.presence.PresRlmiInfo rlmiInfo, com.android.ims.internal.uce.presence.PresResInfo[] resInfo) throws android.os.RemoteException;
  /**
       * Callback function to be invoked to inform the client when Unpublish message
       * is sent to network.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/telephony/java/com/android/ims/internal/uce/presence/IPresenceListener.aidl:105:1:105:25")
  public void unpublishMessageSent() throws android.os.RemoteException;
}
