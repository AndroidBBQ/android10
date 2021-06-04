/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package org.codeaurora.ims.internal;
/**
 * Interface through which APP and vendor communicates.
 * {@hide}
 */
public interface IQtiImsExt extends android.os.IInterface
{
  /** Default implementation for IQtiImsExt. */
  public static class Default implements org.codeaurora.ims.internal.IQtiImsExt
  {
    /**
         * setCallForwardingUncondTimerOptions
         * sets a call forwarding unconditional Timer option.
         *
         * @param startHour indicates starting hour
         * @param startMinute indicates starting minute
         * @param endHour indicates ending hour
         * @param endMinute indicates ending minute
         * @param action is one of the valid call forwarding
         *        CF_ACTIONS, as defined in
         *        <code>com.android.internal.telephony.CommandsInterface.</code>
         * @param reason is one of the valid call forwarding
         *        CF_REASONS, as defined in
         *        <code>com.android.internal.telephony.CommandsInterface.</code>
         * @param serviceClass is service class, that is used to set CFT
         *        SERVICE_CLASS, as defined in
         *        <code>com.android.internal.telephony.CommandsInterface.</code>
         * @param dialingNumber is the target phone number to forward calls to
         * @param QtiImsExtListener listener to request
         * @return void
         */
    @Override public void setCallForwardUncondTimer(int startHour, int startMinute, int endHour, int endMinute, int action, int reason, int serviceClass, java.lang.String dialingNumber, org.codeaurora.ims.internal.IQtiImsExtListener listener) throws android.os.RemoteException
    {
    }
    /**
         * getCallForwardingUncondTimerOptions
         * gets a call forwarding option.
         *
         * @param reason is one of the valid call forwarding
         *        CF_REASONS, as defined in
         *        <code>com.android.internal.telephony.CommandsInterface.</code>
         * @param serviceClass is service class, that is used to get CFT
         *        SERVICE_CLASS, as defined in
         *        <code>com.android.internal.telephony.CommandsInterface.</code>
         * @param QtiImsExtListener listener to request
         * @return void
         */
    @Override public void getCallForwardUncondTimer(int reason, int serviceClass, org.codeaurora.ims.internal.IQtiImsExtListener listener) throws android.os.RemoteException
    {
    }
    /**
          * Total number of packets sent or received
          *
          * @param listener, provided if caller needs to be notified for get result.
          * @return void
          *
          * @throws RemoteException if calling the IMS service results in an error.
          */
    @Override public void getPacketCount(org.codeaurora.ims.internal.IQtiImsExtListener listener) throws android.os.RemoteException
    {
    }
    /**
         * Total number of packet errors encountered
         *
         * @param listener, provided if caller needs to be notified for get result.
         * @return void
         *
         * @throws RemoteException if calling the IMS service results in an error.
         */
    @Override public void getPacketErrorCount(org.codeaurora.ims.internal.IQtiImsExtListener listener) throws android.os.RemoteException
    {
    }
    /**
         * sendCallDeflectRequest
         * Deflects a incoming call to given number
         *
         * @param phoneId indicates the phone instance which triggered the request
         * @param deflectNumber indicates the target number to deflect
         * @param listener an IQtiImsExtListener instance to indicate the response
         * @return void
         */
    @Override public void sendCallDeflectRequest(int phoneId, java.lang.String deflectNumber, org.codeaurora.ims.internal.IQtiImsExtListener listener) throws android.os.RemoteException
    {
    }
    /**
         * sendCallTransferRequest
         * Transfer an established call to given number or call id
         *
         * @param phoneId indicates the phone instance which triggered the request
         * @param type is one of the values QTI_IMS_TRANSFER_TYPE_*, as defined in
         *        <code>org.codeaurora.ims.qtiims.QtiImsInterfaceUtils.</code>
         * @param number indicates the target number to transfer
         * @param listener an IQtiImsInterfaceListener instance to indicate the response
         * @return void
         */
    @Override public void sendCallTransferRequest(int phoneId, int type, java.lang.String number, org.codeaurora.ims.internal.IQtiImsExtListener listener) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements org.codeaurora.ims.internal.IQtiImsExt
  {
    private static final java.lang.String DESCRIPTOR = "org.codeaurora.ims.internal.IQtiImsExt";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an org.codeaurora.ims.internal.IQtiImsExt interface,
     * generating a proxy if needed.
     */
    public static org.codeaurora.ims.internal.IQtiImsExt asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof org.codeaurora.ims.internal.IQtiImsExt))) {
        return ((org.codeaurora.ims.internal.IQtiImsExt)iin);
      }
      return new org.codeaurora.ims.internal.IQtiImsExt.Stub.Proxy(obj);
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
        case TRANSACTION_setCallForwardUncondTimer:
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
          int _arg6;
          _arg6 = data.readInt();
          java.lang.String _arg7;
          _arg7 = data.readString();
          org.codeaurora.ims.internal.IQtiImsExtListener _arg8;
          _arg8 = org.codeaurora.ims.internal.IQtiImsExtListener.Stub.asInterface(data.readStrongBinder());
          this.setCallForwardUncondTimer(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8);
          return true;
        }
        case TRANSACTION_getCallForwardUncondTimer:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          org.codeaurora.ims.internal.IQtiImsExtListener _arg2;
          _arg2 = org.codeaurora.ims.internal.IQtiImsExtListener.Stub.asInterface(data.readStrongBinder());
          this.getCallForwardUncondTimer(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_getPacketCount:
        {
          data.enforceInterface(descriptor);
          org.codeaurora.ims.internal.IQtiImsExtListener _arg0;
          _arg0 = org.codeaurora.ims.internal.IQtiImsExtListener.Stub.asInterface(data.readStrongBinder());
          this.getPacketCount(_arg0);
          return true;
        }
        case TRANSACTION_getPacketErrorCount:
        {
          data.enforceInterface(descriptor);
          org.codeaurora.ims.internal.IQtiImsExtListener _arg0;
          _arg0 = org.codeaurora.ims.internal.IQtiImsExtListener.Stub.asInterface(data.readStrongBinder());
          this.getPacketErrorCount(_arg0);
          return true;
        }
        case TRANSACTION_sendCallDeflectRequest:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          org.codeaurora.ims.internal.IQtiImsExtListener _arg2;
          _arg2 = org.codeaurora.ims.internal.IQtiImsExtListener.Stub.asInterface(data.readStrongBinder());
          this.sendCallDeflectRequest(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_sendCallTransferRequest:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          org.codeaurora.ims.internal.IQtiImsExtListener _arg3;
          _arg3 = org.codeaurora.ims.internal.IQtiImsExtListener.Stub.asInterface(data.readStrongBinder());
          this.sendCallTransferRequest(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements org.codeaurora.ims.internal.IQtiImsExt
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
           * setCallForwardingUncondTimerOptions
           * sets a call forwarding unconditional Timer option.
           *
           * @param startHour indicates starting hour
           * @param startMinute indicates starting minute
           * @param endHour indicates ending hour
           * @param endMinute indicates ending minute
           * @param action is one of the valid call forwarding
           *        CF_ACTIONS, as defined in
           *        <code>com.android.internal.telephony.CommandsInterface.</code>
           * @param reason is one of the valid call forwarding
           *        CF_REASONS, as defined in
           *        <code>com.android.internal.telephony.CommandsInterface.</code>
           * @param serviceClass is service class, that is used to set CFT
           *        SERVICE_CLASS, as defined in
           *        <code>com.android.internal.telephony.CommandsInterface.</code>
           * @param dialingNumber is the target phone number to forward calls to
           * @param QtiImsExtListener listener to request
           * @return void
           */
      @Override public void setCallForwardUncondTimer(int startHour, int startMinute, int endHour, int endMinute, int action, int reason, int serviceClass, java.lang.String dialingNumber, org.codeaurora.ims.internal.IQtiImsExtListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(startHour);
          _data.writeInt(startMinute);
          _data.writeInt(endHour);
          _data.writeInt(endMinute);
          _data.writeInt(action);
          _data.writeInt(reason);
          _data.writeInt(serviceClass);
          _data.writeString(dialingNumber);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setCallForwardUncondTimer, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setCallForwardUncondTimer(startHour, startMinute, endHour, endMinute, action, reason, serviceClass, dialingNumber, listener);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * getCallForwardingUncondTimerOptions
           * gets a call forwarding option.
           *
           * @param reason is one of the valid call forwarding
           *        CF_REASONS, as defined in
           *        <code>com.android.internal.telephony.CommandsInterface.</code>
           * @param serviceClass is service class, that is used to get CFT
           *        SERVICE_CLASS, as defined in
           *        <code>com.android.internal.telephony.CommandsInterface.</code>
           * @param QtiImsExtListener listener to request
           * @return void
           */
      @Override public void getCallForwardUncondTimer(int reason, int serviceClass, org.codeaurora.ims.internal.IQtiImsExtListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(reason);
          _data.writeInt(serviceClass);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCallForwardUncondTimer, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getCallForwardUncondTimer(reason, serviceClass, listener);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
            * Total number of packets sent or received
            *
            * @param listener, provided if caller needs to be notified for get result.
            * @return void
            *
            * @throws RemoteException if calling the IMS service results in an error.
            */
      @Override public void getPacketCount(org.codeaurora.ims.internal.IQtiImsExtListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPacketCount, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getPacketCount(listener);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Total number of packet errors encountered
           *
           * @param listener, provided if caller needs to be notified for get result.
           * @return void
           *
           * @throws RemoteException if calling the IMS service results in an error.
           */
      @Override public void getPacketErrorCount(org.codeaurora.ims.internal.IQtiImsExtListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPacketErrorCount, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getPacketErrorCount(listener);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * sendCallDeflectRequest
           * Deflects a incoming call to given number
           *
           * @param phoneId indicates the phone instance which triggered the request
           * @param deflectNumber indicates the target number to deflect
           * @param listener an IQtiImsExtListener instance to indicate the response
           * @return void
           */
      @Override public void sendCallDeflectRequest(int phoneId, java.lang.String deflectNumber, org.codeaurora.ims.internal.IQtiImsExtListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(phoneId);
          _data.writeString(deflectNumber);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendCallDeflectRequest, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendCallDeflectRequest(phoneId, deflectNumber, listener);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * sendCallTransferRequest
           * Transfer an established call to given number or call id
           *
           * @param phoneId indicates the phone instance which triggered the request
           * @param type is one of the values QTI_IMS_TRANSFER_TYPE_*, as defined in
           *        <code>org.codeaurora.ims.qtiims.QtiImsInterfaceUtils.</code>
           * @param number indicates the target number to transfer
           * @param listener an IQtiImsInterfaceListener instance to indicate the response
           * @return void
           */
      @Override public void sendCallTransferRequest(int phoneId, int type, java.lang.String number, org.codeaurora.ims.internal.IQtiImsExtListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(phoneId);
          _data.writeInt(type);
          _data.writeString(number);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendCallTransferRequest, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendCallTransferRequest(phoneId, type, number, listener);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static org.codeaurora.ims.internal.IQtiImsExt sDefaultImpl;
    }
    static final int TRANSACTION_setCallForwardUncondTimer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getCallForwardUncondTimer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getPacketCount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getPacketErrorCount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_sendCallDeflectRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_sendCallTransferRequest = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(org.codeaurora.ims.internal.IQtiImsExt impl) {
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
    public static org.codeaurora.ims.internal.IQtiImsExt getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * setCallForwardingUncondTimerOptions
       * sets a call forwarding unconditional Timer option.
       *
       * @param startHour indicates starting hour
       * @param startMinute indicates starting minute
       * @param endHour indicates ending hour
       * @param endMinute indicates ending minute
       * @param action is one of the valid call forwarding
       *        CF_ACTIONS, as defined in
       *        <code>com.android.internal.telephony.CommandsInterface.</code>
       * @param reason is one of the valid call forwarding
       *        CF_REASONS, as defined in
       *        <code>com.android.internal.telephony.CommandsInterface.</code>
       * @param serviceClass is service class, that is used to set CFT
       *        SERVICE_CLASS, as defined in
       *        <code>com.android.internal.telephony.CommandsInterface.</code>
       * @param dialingNumber is the target phone number to forward calls to
       * @param QtiImsExtListener listener to request
       * @return void
       */
  public void setCallForwardUncondTimer(int startHour, int startMinute, int endHour, int endMinute, int action, int reason, int serviceClass, java.lang.String dialingNumber, org.codeaurora.ims.internal.IQtiImsExtListener listener) throws android.os.RemoteException;
  /**
       * getCallForwardingUncondTimerOptions
       * gets a call forwarding option.
       *
       * @param reason is one of the valid call forwarding
       *        CF_REASONS, as defined in
       *        <code>com.android.internal.telephony.CommandsInterface.</code>
       * @param serviceClass is service class, that is used to get CFT
       *        SERVICE_CLASS, as defined in
       *        <code>com.android.internal.telephony.CommandsInterface.</code>
       * @param QtiImsExtListener listener to request
       * @return void
       */
  public void getCallForwardUncondTimer(int reason, int serviceClass, org.codeaurora.ims.internal.IQtiImsExtListener listener) throws android.os.RemoteException;
  /**
        * Total number of packets sent or received
        *
        * @param listener, provided if caller needs to be notified for get result.
        * @return void
        *
        * @throws RemoteException if calling the IMS service results in an error.
        */
  public void getPacketCount(org.codeaurora.ims.internal.IQtiImsExtListener listener) throws android.os.RemoteException;
  /**
       * Total number of packet errors encountered
       *
       * @param listener, provided if caller needs to be notified for get result.
       * @return void
       *
       * @throws RemoteException if calling the IMS service results in an error.
       */
  public void getPacketErrorCount(org.codeaurora.ims.internal.IQtiImsExtListener listener) throws android.os.RemoteException;
  /**
       * sendCallDeflectRequest
       * Deflects a incoming call to given number
       *
       * @param phoneId indicates the phone instance which triggered the request
       * @param deflectNumber indicates the target number to deflect
       * @param listener an IQtiImsExtListener instance to indicate the response
       * @return void
       */
  public void sendCallDeflectRequest(int phoneId, java.lang.String deflectNumber, org.codeaurora.ims.internal.IQtiImsExtListener listener) throws android.os.RemoteException;
  /**
       * sendCallTransferRequest
       * Transfer an established call to given number or call id
       *
       * @param phoneId indicates the phone instance which triggered the request
       * @param type is one of the values QTI_IMS_TRANSFER_TYPE_*, as defined in
       *        <code>org.codeaurora.ims.qtiims.QtiImsInterfaceUtils.</code>
       * @param number indicates the target number to transfer
       * @param listener an IQtiImsInterfaceListener instance to indicate the response
       * @return void
       */
  public void sendCallTransferRequest(int phoneId, int type, java.lang.String number, org.codeaurora.ims.internal.IQtiImsExtListener listener) throws android.os.RemoteException;
}
