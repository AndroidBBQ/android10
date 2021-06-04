/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.carrier;
/**
 * <p class="note"><strong>Note:</strong>
 * This service can only be implemented by a carrier privileged app.
 * @hide
 */
public interface ICarrierMessagingService extends android.os.IInterface
{
  /** Default implementation for ICarrierMessagingService. */
  public static class Default implements android.service.carrier.ICarrierMessagingService
  {
    /**
         * Request filtering an incoming SMS message.
         * The service will call callback.onFilterComplete with the filtering result.
         *
         * @param pdu the PDUs of the message
         * @param format the format of the PDUs, typically "3gpp" or "3gpp2"
         * @param destPort the destination port of a data SMS. It will be -1 for text SMS
         * @param subId SMS subscription ID of the SIM
         * @param callback the callback to notify upon completion
         */
    @Override public void filterSms(android.service.carrier.MessagePdu pdu, java.lang.String format, int destPort, int subId, android.service.carrier.ICarrierMessagingCallback callback) throws android.os.RemoteException
    {
    }
    /**
         * Request sending a new text SMS from the device.
         * The service will call {@link ICarrierMessagingCallback#onSendSmsComplete} with the send
         * status.
         *
         * @param text the text to send
         * @param subId SMS subscription ID of the SIM
         * @param destAddress phone number of the recipient of the message
         * @param sendSmsFlag flag for sending SMS
         * @param callback the callback to notify upon completion
         */
    @Override public void sendTextSms(java.lang.String text, int subId, java.lang.String destAddress, int sendSmsFlag, android.service.carrier.ICarrierMessagingCallback callback) throws android.os.RemoteException
    {
    }
    /**
         * Request sending a new data SMS from the device.
         * The service will call {@link ICarrierMessagingCallback#onSendSmsComplete} with the send
         * status.
         *
         * @param data the data to send
         * @param subId SMS subscription ID of the SIM
         * @param destAddress phone number of the recipient of the message
         * @param destPort port number of the recipient of the message
         * @param sendSmsFlag flag for sending SMS
         * @param callback the callback to notify upon completion
         */
    @Override public void sendDataSms(byte[] data, int subId, java.lang.String destAddress, int destPort, int sendSmsFlag, android.service.carrier.ICarrierMessagingCallback callback) throws android.os.RemoteException
    {
    }
    /**
         * Request sending a new multi-part text SMS from the device.
         * The service will call {@link ICarrierMessagingCallback#onSendMultipartSmsComplete}
         * with the send status.
         *
         * @param parts the parts of the multi-part text SMS to send
         * @param subId SMS subscription ID of the SIM
         * @param destAddress phone number of the recipient of the message
         * @param sendSmsFlag flag for sending SMS
         * @param callback the callback to notify upon completion
         */
    @Override public void sendMultipartTextSms(java.util.List<java.lang.String> parts, int subId, java.lang.String destAddress, int sendSmsFlag, android.service.carrier.ICarrierMessagingCallback callback) throws android.os.RemoteException
    {
    }
    /**
         * Request sending a new MMS PDU from the device.
         * The service will call {@link ICarrierMessagingCallback#onSendMmsComplete} with the send
         * status.
         *
         * @param pduUri the content provider URI of the PDU to send
         * @param subId SMS subscription ID of the SIM
         * @param location the optional URI to send this MMS PDU. If this is {code null},
         *        the PDU should be sent to the default MMSC URL.
         * @param callback the callback to notify upon completion
         */
    @Override public void sendMms(android.net.Uri pduUri, int subId, android.net.Uri location, android.service.carrier.ICarrierMessagingCallback callback) throws android.os.RemoteException
    {
    }
    /**
         * Request downloading a new MMS.
         * The service will call {@link ICarrierMessagingCallback#onDownloadMmsComplete} with the
         * download status.
         *
         * @param pduUri the content provider URI of the PDU to be downloaded.
         * @param subId SMS subscription ID of the SIM
         * @param location the URI of the message to be downloaded.
         * @param callback the callback to notify upon completion
         */
    @Override public void downloadMms(android.net.Uri pduUri, int subId, android.net.Uri location, android.service.carrier.ICarrierMessagingCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.carrier.ICarrierMessagingService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.carrier.ICarrierMessagingService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.carrier.ICarrierMessagingService interface,
     * generating a proxy if needed.
     */
    public static android.service.carrier.ICarrierMessagingService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.carrier.ICarrierMessagingService))) {
        return ((android.service.carrier.ICarrierMessagingService)iin);
      }
      return new android.service.carrier.ICarrierMessagingService.Stub.Proxy(obj);
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
        case TRANSACTION_filterSms:
        {
          data.enforceInterface(descriptor);
          android.service.carrier.MessagePdu _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.service.carrier.MessagePdu.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          android.service.carrier.ICarrierMessagingCallback _arg4;
          _arg4 = android.service.carrier.ICarrierMessagingCallback.Stub.asInterface(data.readStrongBinder());
          this.filterSms(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_sendTextSms:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          android.service.carrier.ICarrierMessagingCallback _arg4;
          _arg4 = android.service.carrier.ICarrierMessagingCallback.Stub.asInterface(data.readStrongBinder());
          this.sendTextSms(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_sendDataSms:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          android.service.carrier.ICarrierMessagingCallback _arg5;
          _arg5 = android.service.carrier.ICarrierMessagingCallback.Stub.asInterface(data.readStrongBinder());
          this.sendDataSms(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          return true;
        }
        case TRANSACTION_sendMultipartTextSms:
        {
          data.enforceInterface(descriptor);
          java.util.List<java.lang.String> _arg0;
          _arg0 = data.createStringArrayList();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          android.service.carrier.ICarrierMessagingCallback _arg4;
          _arg4 = android.service.carrier.ICarrierMessagingCallback.Stub.asInterface(data.readStrongBinder());
          this.sendMultipartTextSms(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_sendMms:
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
          android.net.Uri _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.service.carrier.ICarrierMessagingCallback _arg3;
          _arg3 = android.service.carrier.ICarrierMessagingCallback.Stub.asInterface(data.readStrongBinder());
          this.sendMms(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_downloadMms:
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
          android.net.Uri _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.service.carrier.ICarrierMessagingCallback _arg3;
          _arg3 = android.service.carrier.ICarrierMessagingCallback.Stub.asInterface(data.readStrongBinder());
          this.downloadMms(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.carrier.ICarrierMessagingService
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
           * Request filtering an incoming SMS message.
           * The service will call callback.onFilterComplete with the filtering result.
           *
           * @param pdu the PDUs of the message
           * @param format the format of the PDUs, typically "3gpp" or "3gpp2"
           * @param destPort the destination port of a data SMS. It will be -1 for text SMS
           * @param subId SMS subscription ID of the SIM
           * @param callback the callback to notify upon completion
           */
      @Override public void filterSms(android.service.carrier.MessagePdu pdu, java.lang.String format, int destPort, int subId, android.service.carrier.ICarrierMessagingCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((pdu!=null)) {
            _data.writeInt(1);
            pdu.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(format);
          _data.writeInt(destPort);
          _data.writeInt(subId);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_filterSms, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().filterSms(pdu, format, destPort, subId, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Request sending a new text SMS from the device.
           * The service will call {@link ICarrierMessagingCallback#onSendSmsComplete} with the send
           * status.
           *
           * @param text the text to send
           * @param subId SMS subscription ID of the SIM
           * @param destAddress phone number of the recipient of the message
           * @param sendSmsFlag flag for sending SMS
           * @param callback the callback to notify upon completion
           */
      @Override public void sendTextSms(java.lang.String text, int subId, java.lang.String destAddress, int sendSmsFlag, android.service.carrier.ICarrierMessagingCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(text);
          _data.writeInt(subId);
          _data.writeString(destAddress);
          _data.writeInt(sendSmsFlag);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendTextSms, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendTextSms(text, subId, destAddress, sendSmsFlag, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Request sending a new data SMS from the device.
           * The service will call {@link ICarrierMessagingCallback#onSendSmsComplete} with the send
           * status.
           *
           * @param data the data to send
           * @param subId SMS subscription ID of the SIM
           * @param destAddress phone number of the recipient of the message
           * @param destPort port number of the recipient of the message
           * @param sendSmsFlag flag for sending SMS
           * @param callback the callback to notify upon completion
           */
      @Override public void sendDataSms(byte[] data, int subId, java.lang.String destAddress, int destPort, int sendSmsFlag, android.service.carrier.ICarrierMessagingCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(data);
          _data.writeInt(subId);
          _data.writeString(destAddress);
          _data.writeInt(destPort);
          _data.writeInt(sendSmsFlag);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendDataSms, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendDataSms(data, subId, destAddress, destPort, sendSmsFlag, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Request sending a new multi-part text SMS from the device.
           * The service will call {@link ICarrierMessagingCallback#onSendMultipartSmsComplete}
           * with the send status.
           *
           * @param parts the parts of the multi-part text SMS to send
           * @param subId SMS subscription ID of the SIM
           * @param destAddress phone number of the recipient of the message
           * @param sendSmsFlag flag for sending SMS
           * @param callback the callback to notify upon completion
           */
      @Override public void sendMultipartTextSms(java.util.List<java.lang.String> parts, int subId, java.lang.String destAddress, int sendSmsFlag, android.service.carrier.ICarrierMessagingCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStringList(parts);
          _data.writeInt(subId);
          _data.writeString(destAddress);
          _data.writeInt(sendSmsFlag);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendMultipartTextSms, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendMultipartTextSms(parts, subId, destAddress, sendSmsFlag, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Request sending a new MMS PDU from the device.
           * The service will call {@link ICarrierMessagingCallback#onSendMmsComplete} with the send
           * status.
           *
           * @param pduUri the content provider URI of the PDU to send
           * @param subId SMS subscription ID of the SIM
           * @param location the optional URI to send this MMS PDU. If this is {code null},
           *        the PDU should be sent to the default MMSC URL.
           * @param callback the callback to notify upon completion
           */
      @Override public void sendMms(android.net.Uri pduUri, int subId, android.net.Uri location, android.service.carrier.ICarrierMessagingCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((pduUri!=null)) {
            _data.writeInt(1);
            pduUri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(subId);
          if ((location!=null)) {
            _data.writeInt(1);
            location.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendMms, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendMms(pduUri, subId, location, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Request downloading a new MMS.
           * The service will call {@link ICarrierMessagingCallback#onDownloadMmsComplete} with the
           * download status.
           *
           * @param pduUri the content provider URI of the PDU to be downloaded.
           * @param subId SMS subscription ID of the SIM
           * @param location the URI of the message to be downloaded.
           * @param callback the callback to notify upon completion
           */
      @Override public void downloadMms(android.net.Uri pduUri, int subId, android.net.Uri location, android.service.carrier.ICarrierMessagingCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((pduUri!=null)) {
            _data.writeInt(1);
            pduUri.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(subId);
          if ((location!=null)) {
            _data.writeInt(1);
            location.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_downloadMms, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().downloadMms(pduUri, subId, location, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.carrier.ICarrierMessagingService sDefaultImpl;
    }
    static final int TRANSACTION_filterSms = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_sendTextSms = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_sendDataSms = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_sendMultipartTextSms = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_sendMms = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_downloadMms = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    public static boolean setDefaultImpl(android.service.carrier.ICarrierMessagingService impl) {
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
    public static android.service.carrier.ICarrierMessagingService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Request filtering an incoming SMS message.
       * The service will call callback.onFilterComplete with the filtering result.
       *
       * @param pdu the PDUs of the message
       * @param format the format of the PDUs, typically "3gpp" or "3gpp2"
       * @param destPort the destination port of a data SMS. It will be -1 for text SMS
       * @param subId SMS subscription ID of the SIM
       * @param callback the callback to notify upon completion
       */
  public void filterSms(android.service.carrier.MessagePdu pdu, java.lang.String format, int destPort, int subId, android.service.carrier.ICarrierMessagingCallback callback) throws android.os.RemoteException;
  /**
       * Request sending a new text SMS from the device.
       * The service will call {@link ICarrierMessagingCallback#onSendSmsComplete} with the send
       * status.
       *
       * @param text the text to send
       * @param subId SMS subscription ID of the SIM
       * @param destAddress phone number of the recipient of the message
       * @param sendSmsFlag flag for sending SMS
       * @param callback the callback to notify upon completion
       */
  public void sendTextSms(java.lang.String text, int subId, java.lang.String destAddress, int sendSmsFlag, android.service.carrier.ICarrierMessagingCallback callback) throws android.os.RemoteException;
  /**
       * Request sending a new data SMS from the device.
       * The service will call {@link ICarrierMessagingCallback#onSendSmsComplete} with the send
       * status.
       *
       * @param data the data to send
       * @param subId SMS subscription ID of the SIM
       * @param destAddress phone number of the recipient of the message
       * @param destPort port number of the recipient of the message
       * @param sendSmsFlag flag for sending SMS
       * @param callback the callback to notify upon completion
       */
  public void sendDataSms(byte[] data, int subId, java.lang.String destAddress, int destPort, int sendSmsFlag, android.service.carrier.ICarrierMessagingCallback callback) throws android.os.RemoteException;
  /**
       * Request sending a new multi-part text SMS from the device.
       * The service will call {@link ICarrierMessagingCallback#onSendMultipartSmsComplete}
       * with the send status.
       *
       * @param parts the parts of the multi-part text SMS to send
       * @param subId SMS subscription ID of the SIM
       * @param destAddress phone number of the recipient of the message
       * @param sendSmsFlag flag for sending SMS
       * @param callback the callback to notify upon completion
       */
  public void sendMultipartTextSms(java.util.List<java.lang.String> parts, int subId, java.lang.String destAddress, int sendSmsFlag, android.service.carrier.ICarrierMessagingCallback callback) throws android.os.RemoteException;
  /**
       * Request sending a new MMS PDU from the device.
       * The service will call {@link ICarrierMessagingCallback#onSendMmsComplete} with the send
       * status.
       *
       * @param pduUri the content provider URI of the PDU to send
       * @param subId SMS subscription ID of the SIM
       * @param location the optional URI to send this MMS PDU. If this is {code null},
       *        the PDU should be sent to the default MMSC URL.
       * @param callback the callback to notify upon completion
       */
  public void sendMms(android.net.Uri pduUri, int subId, android.net.Uri location, android.service.carrier.ICarrierMessagingCallback callback) throws android.os.RemoteException;
  /**
       * Request downloading a new MMS.
       * The service will call {@link ICarrierMessagingCallback#onDownloadMmsComplete} with the
       * download status.
       *
       * @param pduUri the content provider URI of the PDU to be downloaded.
       * @param subId SMS subscription ID of the SIM
       * @param location the URI of the message to be downloaded.
       * @param callback the callback to notify upon completion
       */
  public void downloadMms(android.net.Uri pduUri, int subId, android.net.Uri location, android.service.carrier.ICarrierMessagingCallback callback) throws android.os.RemoteException;
}
