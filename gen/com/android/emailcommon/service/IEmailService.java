/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.emailcommon.service;
public interface IEmailService extends android.os.IInterface
{
  /** Default implementation for IEmailService. */
  public static class Default implements com.android.emailcommon.service.IEmailService
  {
    // Core email operations.
    // Many of these functions return status codes. The valid status codes are defined in
    // EmailServiceStatus.java

    @Override public void loadAttachment(com.android.emailcommon.service.IEmailServiceCallback cb, long accountId, long attachmentId, boolean background) throws android.os.RemoteException
    {
    }
    @Override public void updateFolderList(long accountId) throws android.os.RemoteException
    {
    }
    // TODO: For Eas, sync() will also sync the outbox. We should make IMAP and POP work the same
    // way and get rid of sendMail().

    @Override public void sendMail(long accountId) throws android.os.RemoteException
    {
    }
    @Override public int sync(long accountId, android.os.Bundle syncExtras) throws android.os.RemoteException
    {
      return 0;
    }
    // Push-related functionality.
    // Notify the service that the push configuration has changed for an account.

    @Override public void pushModify(long accountId) throws android.os.RemoteException
    {
    }
    // Other email operations.

    @Override public android.os.Bundle validate(com.android.emailcommon.service.HostAuthCompat hostauth) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int searchMessages(long accountId, com.android.emailcommon.service.SearchParams params, long destMailboxId) throws android.os.RemoteException
    {
      return 0;
    }
    // PIM functionality (not strictly EAS specific).

    @Override public void sendMeetingResponse(long messageId, int response) throws android.os.RemoteException
    {
    }
    // Specific to EAS protocol.
    // TODO: this passes a HostAuth back in the bundle. We should be using a HostAuthCom for that.

    @Override public android.os.Bundle autoDiscover(java.lang.String userName, java.lang.String password) throws android.os.RemoteException
    {
      return null;
    }
    // Service control operations (i.e. does not generate a client-server message).
    // TODO: We should store the logging flags in the contentProvider, and this call should just
    // trigger the service to reload the flags.

    @Override public void setLogging(int flags) throws android.os.RemoteException
    {
    }
    @Override public void deleteExternalAccountPIMData(java.lang.String emailAddress) throws android.os.RemoteException
    {
    }
    @Override public int getApiVersion() throws android.os.RemoteException
    {
      return 0;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.emailcommon.service.IEmailService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.emailcommon.service.IEmailService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.emailcommon.service.IEmailService interface,
     * generating a proxy if needed.
     */
    public static com.android.emailcommon.service.IEmailService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.emailcommon.service.IEmailService))) {
        return ((com.android.emailcommon.service.IEmailService)iin);
      }
      return new com.android.emailcommon.service.IEmailService.Stub.Proxy(obj);
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
        case TRANSACTION_loadAttachment:
        {
          data.enforceInterface(descriptor);
          com.android.emailcommon.service.IEmailServiceCallback _arg0;
          _arg0 = com.android.emailcommon.service.IEmailServiceCallback.Stub.asInterface(data.readStrongBinder());
          long _arg1;
          _arg1 = data.readLong();
          long _arg2;
          _arg2 = data.readLong();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          this.loadAttachment(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_updateFolderList:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.updateFolderList(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_sendMail:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.sendMail(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_sync:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          android.os.Bundle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _result = this.sync(_arg0, _arg1);
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
        case TRANSACTION_pushModify:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.pushModify(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_validate:
        {
          data.enforceInterface(descriptor);
          com.android.emailcommon.service.HostAuthCompat _arg0;
          if ((0!=data.readInt())) {
            _arg0 = com.android.emailcommon.service.HostAuthCompat.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.Bundle _result = this.validate(_arg0);
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
        case TRANSACTION_searchMessages:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          com.android.emailcommon.service.SearchParams _arg1;
          if ((0!=data.readInt())) {
            _arg1 = com.android.emailcommon.service.SearchParams.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          long _arg2;
          _arg2 = data.readLong();
          int _result = this.searchMessages(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_sendMeetingResponse:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          this.sendMeetingResponse(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_autoDiscover:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.Bundle _result = this.autoDiscover(_arg0, _arg1);
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
        case TRANSACTION_setLogging:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setLogging(_arg0);
          return true;
        }
        case TRANSACTION_deleteExternalAccountPIMData:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.deleteExternalAccountPIMData(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getApiVersion:
        {
          data.enforceInterface(descriptor);
          int _result = this.getApiVersion();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.emailcommon.service.IEmailService
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
      // Core email operations.
      // Many of these functions return status codes. The valid status codes are defined in
      // EmailServiceStatus.java

      @Override public void loadAttachment(com.android.emailcommon.service.IEmailServiceCallback cb, long accountId, long attachmentId, boolean background) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((cb!=null))?(cb.asBinder()):(null)));
          _data.writeLong(accountId);
          _data.writeLong(attachmentId);
          _data.writeInt(((background)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_loadAttachment, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().loadAttachment(cb, accountId, attachmentId, background);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void updateFolderList(long accountId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(accountId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateFolderList, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().updateFolderList(accountId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // TODO: For Eas, sync() will also sync the outbox. We should make IMAP and POP work the same
      // way and get rid of sendMail().

      @Override public void sendMail(long accountId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(accountId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendMail, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendMail(accountId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int sync(long accountId, android.os.Bundle syncExtras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(accountId);
          if ((syncExtras!=null)) {
            _data.writeInt(1);
            syncExtras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_sync, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().sync(accountId, syncExtras);
          }
          _reply.readException();
          _result = _reply.readInt();
          if ((0!=_reply.readInt())) {
            syncExtras.readFromParcel(_reply);
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // Push-related functionality.
      // Notify the service that the push configuration has changed for an account.

      @Override public void pushModify(long accountId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(accountId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_pushModify, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().pushModify(accountId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Other email operations.

      @Override public android.os.Bundle validate(com.android.emailcommon.service.HostAuthCompat hostauth) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.Bundle _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((hostauth!=null)) {
            _data.writeInt(1);
            hostauth.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_validate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().validate(hostauth);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.Bundle.CREATOR.createFromParcel(_reply);
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
      @Override public int searchMessages(long accountId, com.android.emailcommon.service.SearchParams params, long destMailboxId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(accountId);
          if ((params!=null)) {
            _data.writeInt(1);
            params.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeLong(destMailboxId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_searchMessages, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().searchMessages(accountId, params, destMailboxId);
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
      // PIM functionality (not strictly EAS specific).

      @Override public void sendMeetingResponse(long messageId, int response) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(messageId);
          _data.writeInt(response);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendMeetingResponse, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendMeetingResponse(messageId, response);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Specific to EAS protocol.
      // TODO: this passes a HostAuth back in the bundle. We should be using a HostAuthCom for that.

      @Override public android.os.Bundle autoDiscover(java.lang.String userName, java.lang.String password) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.Bundle _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(userName);
          _data.writeString(password);
          boolean _status = mRemote.transact(Stub.TRANSACTION_autoDiscover, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().autoDiscover(userName, password);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.Bundle.CREATOR.createFromParcel(_reply);
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
      // Service control operations (i.e. does not generate a client-server message).
      // TODO: We should store the logging flags in the contentProvider, and this call should just
      // trigger the service to reload the flags.

      @Override public void setLogging(int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setLogging, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setLogging(flags);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void deleteExternalAccountPIMData(java.lang.String emailAddress) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(emailAddress);
          boolean _status = mRemote.transact(Stub.TRANSACTION_deleteExternalAccountPIMData, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deleteExternalAccountPIMData(emailAddress);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getApiVersion() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getApiVersion, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getApiVersion();
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
      public static com.android.emailcommon.service.IEmailService sDefaultImpl;
    }
    static final int TRANSACTION_loadAttachment = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_updateFolderList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_sendMail = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_sync = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_pushModify = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_validate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_searchMessages = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_sendMeetingResponse = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_autoDiscover = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_setLogging = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_deleteExternalAccountPIMData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_getApiVersion = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    public static boolean setDefaultImpl(com.android.emailcommon.service.IEmailService impl) {
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
    public static com.android.emailcommon.service.IEmailService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // Core email operations.
  // Many of these functions return status codes. The valid status codes are defined in
  // EmailServiceStatus.java

  public void loadAttachment(com.android.emailcommon.service.IEmailServiceCallback cb, long accountId, long attachmentId, boolean background) throws android.os.RemoteException;
  public void updateFolderList(long accountId) throws android.os.RemoteException;
  // TODO: For Eas, sync() will also sync the outbox. We should make IMAP and POP work the same
  // way and get rid of sendMail().

  public void sendMail(long accountId) throws android.os.RemoteException;
  public int sync(long accountId, android.os.Bundle syncExtras) throws android.os.RemoteException;
  // Push-related functionality.
  // Notify the service that the push configuration has changed for an account.

  public void pushModify(long accountId) throws android.os.RemoteException;
  // Other email operations.

  public android.os.Bundle validate(com.android.emailcommon.service.HostAuthCompat hostauth) throws android.os.RemoteException;
  public int searchMessages(long accountId, com.android.emailcommon.service.SearchParams params, long destMailboxId) throws android.os.RemoteException;
  // PIM functionality (not strictly EAS specific).

  public void sendMeetingResponse(long messageId, int response) throws android.os.RemoteException;
  // Specific to EAS protocol.
  // TODO: this passes a HostAuth back in the bundle. We should be using a HostAuthCom for that.

  public android.os.Bundle autoDiscover(java.lang.String userName, java.lang.String password) throws android.os.RemoteException;
  // Service control operations (i.e. does not generate a client-server message).
  // TODO: We should store the logging flags in the contentProvider, and this call should just
  // trigger the service to reload the flags.

  public void setLogging(int flags) throws android.os.RemoteException;
  public void deleteExternalAccountPIMData(java.lang.String emailAddress) throws android.os.RemoteException;
  public int getApiVersion() throws android.os.RemoteException;
}
