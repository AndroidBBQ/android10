/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package org.codeaurora.ims.internal;
/**
 * Used by client application to get the result from lower layer by
 * communicating with vendor.
 * {@hide}
 */
public interface IQtiImsExtListener extends android.os.IInterface
{
  /** Default implementation for IQtiImsExtListener. */
  public static class Default implements org.codeaurora.ims.internal.IQtiImsExtListener
  {
    /**
         * Notifies client the value of the set operation result.
         *
         * @param status To return status of request.
         * @return void.
         */
    @Override public void onSetCallForwardUncondTimer(int status) throws android.os.RemoteException
    {
    }
    /**
         * Notifies client the value of the get operation result.
         *
         * @param startHour indicates starting hour
         * @param startMinute indicates starting minute
         * @param endHour indicates ending hour
         * @param endMinute indicates ending minute
         * @param reason is one of the valid call forwarding
         *        CF_REASONS, as defined in
         *        <code>com.android.internal.telephony.CommandsInterface.</code>
         * @param status indicates status of CF service.
         * @param number is the target phone number to forward calls to
         * @param serviceClass indicates serviceClass type that is supported.
         * @return void.
         */
    @Override public void onGetCallForwardUncondTimer(int startHour, int endHour, int startMinute, int endMinute, int reason, int status, java.lang.String number, int serviceClass) throws android.os.RemoteException
    {
    }
    /**
         * Notifies client with any UT failure result.
         *
         * @param errCode contains error code
         * @param errString contains error string if any.
         * @return void.
         */
    @Override public void onUTReqFailed(int errCode, java.lang.String errString) throws android.os.RemoteException
    {
    }
    /**
         * Notifies client the value of the get operation result on get packet count item.
         *
         * @param status. as defined in com.android.ims.ImsConfig#OperationStatusConstants.
         * @param packetCount. total number of packets sent or received
         * @return void
         */
    @Override public void onGetPacketCount(int status, long packetCount) throws android.os.RemoteException
    {
    }
    /**
         * Notifies client the value of the get operation result on get packet error count item.
         *
         * @param status. as defined in com.android.ims.ImsConfig#OperationStatusConstants.
         * @param packetErrorCount. total number of packet errors encountered
         * @return void
         */
    @Override public void onGetPacketErrorCount(int status, long packetErrorCount) throws android.os.RemoteException
    {
    }
    /**
         * Notifies client the result of call deflect request
         *
         * @param <result> is one of the values QTIIMS_REQUEST_*, as defined in
         *        <code>org.codeaurora.ims.utils.QtiImsExtUtils.</code>
         * @return void.
         */
    @Override public void receiveCallDeflectResponse(int result) throws android.os.RemoteException
    {
    }
    /**
         * Notifies client the result of call transfer request
         *
         * @param <result> is one of the values QTI_IMS_REQUEST_*, as defined in
         *        <code>org.codeaurora.ims.qtiims.QtiImsInterfaceUtils.</code>
         * @return void.
         */
    @Override public void receiveCallTransferResponse(int result) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements org.codeaurora.ims.internal.IQtiImsExtListener
  {
    private static final java.lang.String DESCRIPTOR = "org.codeaurora.ims.internal.IQtiImsExtListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an org.codeaurora.ims.internal.IQtiImsExtListener interface,
     * generating a proxy if needed.
     */
    public static org.codeaurora.ims.internal.IQtiImsExtListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof org.codeaurora.ims.internal.IQtiImsExtListener))) {
        return ((org.codeaurora.ims.internal.IQtiImsExtListener)iin);
      }
      return new org.codeaurora.ims.internal.IQtiImsExtListener.Stub.Proxy(obj);
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
        case TRANSACTION_onSetCallForwardUncondTimer:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onSetCallForwardUncondTimer(_arg0);
          return true;
        }
        case TRANSACTION_onGetCallForwardUncondTimer:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          int _arg5;
          _arg5 = data.readInt();
          java.lang.String _arg6;
          _arg6 = data.readString();
          int _arg7;
          _arg7 = data.readInt();
          this.onGetCallForwardUncondTimer(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7);
          return true;
        }
        case TRANSACTION_onUTReqFailed:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.onUTReqFailed(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onGetPacketCount:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          this.onGetPacketCount(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onGetPacketErrorCount:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          this.onGetPacketErrorCount(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_receiveCallDeflectResponse:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.receiveCallDeflectResponse(_arg0);
          return true;
        }
        case TRANSACTION_receiveCallTransferResponse:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.receiveCallTransferResponse(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements org.codeaurora.ims.internal.IQtiImsExtListener
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
           * Notifies client the value of the set operation result.
           *
           * @param status To return status of request.
           * @return void.
           */
      @Override public void onSetCallForwardUncondTimer(int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSetCallForwardUncondTimer, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSetCallForwardUncondTimer(status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies client the value of the get operation result.
           *
           * @param startHour indicates starting hour
           * @param startMinute indicates starting minute
           * @param endHour indicates ending hour
           * @param endMinute indicates ending minute
           * @param reason is one of the valid call forwarding
           *        CF_REASONS, as defined in
           *        <code>com.android.internal.telephony.CommandsInterface.</code>
           * @param status indicates status of CF service.
           * @param number is the target phone number to forward calls to
           * @param serviceClass indicates serviceClass type that is supported.
           * @return void.
           */
      @Override public void onGetCallForwardUncondTimer(int startHour, int endHour, int startMinute, int endMinute, int reason, int status, java.lang.String number, int serviceClass) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(startHour);
          _data.writeInt(endHour);
          _data.writeInt(startMinute);
          _data.writeInt(endMinute);
          _data.writeInt(reason);
          _data.writeInt(status);
          _data.writeString(number);
          _data.writeInt(serviceClass);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGetCallForwardUncondTimer, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGetCallForwardUncondTimer(startHour, endHour, startMinute, endMinute, reason, status, number, serviceClass);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies client with any UT failure result.
           *
           * @param errCode contains error code
           * @param errString contains error string if any.
           * @return void.
           */
      @Override public void onUTReqFailed(int errCode, java.lang.String errString) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(errCode);
          _data.writeString(errString);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onUTReqFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onUTReqFailed(errCode, errString);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies client the value of the get operation result on get packet count item.
           *
           * @param status. as defined in com.android.ims.ImsConfig#OperationStatusConstants.
           * @param packetCount. total number of packets sent or received
           * @return void
           */
      @Override public void onGetPacketCount(int status, long packetCount) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(status);
          _data.writeLong(packetCount);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGetPacketCount, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGetPacketCount(status, packetCount);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies client the value of the get operation result on get packet error count item.
           *
           * @param status. as defined in com.android.ims.ImsConfig#OperationStatusConstants.
           * @param packetErrorCount. total number of packet errors encountered
           * @return void
           */
      @Override public void onGetPacketErrorCount(int status, long packetErrorCount) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(status);
          _data.writeLong(packetErrorCount);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGetPacketErrorCount, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGetPacketErrorCount(status, packetErrorCount);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies client the result of call deflect request
           *
           * @param <result> is one of the values QTIIMS_REQUEST_*, as defined in
           *        <code>org.codeaurora.ims.utils.QtiImsExtUtils.</code>
           * @return void.
           */
      @Override public void receiveCallDeflectResponse(int result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(result);
          boolean _status = mRemote.transact(Stub.TRANSACTION_receiveCallDeflectResponse, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().receiveCallDeflectResponse(result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies client the result of call transfer request
           *
           * @param <result> is one of the values QTI_IMS_REQUEST_*, as defined in
           *        <code>org.codeaurora.ims.qtiims.QtiImsInterfaceUtils.</code>
           * @return void.
           */
      @Override public void receiveCallTransferResponse(int result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(result);
          boolean _status = mRemote.transact(Stub.TRANSACTION_receiveCallTransferResponse, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().receiveCallTransferResponse(result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static org.codeaurora.ims.internal.IQtiImsExtListener sDefaultImpl;
    }
    static final int TRANSACTION_onSetCallForwardUncondTimer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onGetCallForwardUncondTimer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onUTReqFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onGetPacketCount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onGetPacketErrorCount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_receiveCallDeflectResponse = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_receiveCallTransferResponse = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    public static boolean setDefaultImpl(org.codeaurora.ims.internal.IQtiImsExtListener impl) {
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
    public static org.codeaurora.ims.internal.IQtiImsExtListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Notifies client the value of the set operation result.
       *
       * @param status To return status of request.
       * @return void.
       */
  public void onSetCallForwardUncondTimer(int status) throws android.os.RemoteException;
  /**
       * Notifies client the value of the get operation result.
       *
       * @param startHour indicates starting hour
       * @param startMinute indicates starting minute
       * @param endHour indicates ending hour
       * @param endMinute indicates ending minute
       * @param reason is one of the valid call forwarding
       *        CF_REASONS, as defined in
       *        <code>com.android.internal.telephony.CommandsInterface.</code>
       * @param status indicates status of CF service.
       * @param number is the target phone number to forward calls to
       * @param serviceClass indicates serviceClass type that is supported.
       * @return void.
       */
  public void onGetCallForwardUncondTimer(int startHour, int endHour, int startMinute, int endMinute, int reason, int status, java.lang.String number, int serviceClass) throws android.os.RemoteException;
  /**
       * Notifies client with any UT failure result.
       *
       * @param errCode contains error code
       * @param errString contains error string if any.
       * @return void.
       */
  public void onUTReqFailed(int errCode, java.lang.String errString) throws android.os.RemoteException;
  /**
       * Notifies client the value of the get operation result on get packet count item.
       *
       * @param status. as defined in com.android.ims.ImsConfig#OperationStatusConstants.
       * @param packetCount. total number of packets sent or received
       * @return void
       */
  public void onGetPacketCount(int status, long packetCount) throws android.os.RemoteException;
  /**
       * Notifies client the value of the get operation result on get packet error count item.
       *
       * @param status. as defined in com.android.ims.ImsConfig#OperationStatusConstants.
       * @param packetErrorCount. total number of packet errors encountered
       * @return void
       */
  public void onGetPacketErrorCount(int status, long packetErrorCount) throws android.os.RemoteException;
  /**
       * Notifies client the result of call deflect request
       *
       * @param <result> is one of the values QTIIMS_REQUEST_*, as defined in
       *        <code>org.codeaurora.ims.utils.QtiImsExtUtils.</code>
       * @return void.
       */
  public void receiveCallDeflectResponse(int result) throws android.os.RemoteException;
  /**
       * Notifies client the result of call transfer request
       *
       * @param <result> is one of the values QTI_IMS_REQUEST_*, as defined in
       *        <code>org.codeaurora.ims.qtiims.QtiImsInterfaceUtils.</code>
       * @return void.
       */
  public void receiveCallTransferResponse(int result) throws android.os.RemoteException;
}
