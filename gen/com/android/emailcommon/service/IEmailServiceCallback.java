/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.emailcommon.service;
public interface IEmailServiceCallback extends android.os.IInterface
{
  /** Default implementation for IEmailServiceCallback. */
  public static class Default implements com.android.emailcommon.service.IEmailServiceCallback
  {
    /*
         * Ordinary results:
         *   statuscode = 1, progress = 0:      "starting"
         *   statuscode = 0, progress = n/a:    "finished"
         *
         * If there is an error, it must be reported as follows:
         *   statuscode = err, progress = n/a:  "stopping due to error"
         *
         * *Optionally* a callback can also include intermediate values from 1..99 e.g.
         *   statuscode = 1, progress = 0:      "starting"
         *   statuscode = 1, progress = 30:     "working"
         *   statuscode = 1, progress = 60:     "working"
         *   statuscode = 0, progress = n/a:    "finished"
         *//**
         * Callback to indicate that a particular attachment is being synced
         * messageId = the message that owns the attachment
         * attachmentId = the attachment being synced
         * statusCode = 0 for OK, 1 for progress, other codes for error
         * progress = 0 for "start", 1..100 for optional progress reports
         */
    @Override public void loadAttachmentStatus(long messageId, long attachmentId, int statusCode, int progress) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.emailcommon.service.IEmailServiceCallback
  {
    private static final java.lang.String DESCRIPTOR = "com.android.emailcommon.service.IEmailServiceCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.emailcommon.service.IEmailServiceCallback interface,
     * generating a proxy if needed.
     */
    public static com.android.emailcommon.service.IEmailServiceCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.emailcommon.service.IEmailServiceCallback))) {
        return ((com.android.emailcommon.service.IEmailServiceCallback)iin);
      }
      return new com.android.emailcommon.service.IEmailServiceCallback.Stub.Proxy(obj);
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
        case TRANSACTION_loadAttachmentStatus:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          long _arg1;
          _arg1 = data.readLong();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          this.loadAttachmentStatus(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.emailcommon.service.IEmailServiceCallback
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
      /*
           * Ordinary results:
           *   statuscode = 1, progress = 0:      "starting"
           *   statuscode = 0, progress = n/a:    "finished"
           *
           * If there is an error, it must be reported as follows:
           *   statuscode = err, progress = n/a:  "stopping due to error"
           *
           * *Optionally* a callback can also include intermediate values from 1..99 e.g.
           *   statuscode = 1, progress = 0:      "starting"
           *   statuscode = 1, progress = 30:     "working"
           *   statuscode = 1, progress = 60:     "working"
           *   statuscode = 0, progress = n/a:    "finished"
           *//**
           * Callback to indicate that a particular attachment is being synced
           * messageId = the message that owns the attachment
           * attachmentId = the attachment being synced
           * statusCode = 0 for OK, 1 for progress, other codes for error
           * progress = 0 for "start", 1..100 for optional progress reports
           */
      @Override public void loadAttachmentStatus(long messageId, long attachmentId, int statusCode, int progress) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(messageId);
          _data.writeLong(attachmentId);
          _data.writeInt(statusCode);
          _data.writeInt(progress);
          boolean _status = mRemote.transact(Stub.TRANSACTION_loadAttachmentStatus, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().loadAttachmentStatus(messageId, attachmentId, statusCode, progress);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.emailcommon.service.IEmailServiceCallback sDefaultImpl;
    }
    static final int TRANSACTION_loadAttachmentStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(com.android.emailcommon.service.IEmailServiceCallback impl) {
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
    public static com.android.emailcommon.service.IEmailServiceCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /*
       * Ordinary results:
       *   statuscode = 1, progress = 0:      "starting"
       *   statuscode = 0, progress = n/a:    "finished"
       *
       * If there is an error, it must be reported as follows:
       *   statuscode = err, progress = n/a:  "stopping due to error"
       *
       * *Optionally* a callback can also include intermediate values from 1..99 e.g.
       *   statuscode = 1, progress = 0:      "starting"
       *   statuscode = 1, progress = 30:     "working"
       *   statuscode = 1, progress = 60:     "working"
       *   statuscode = 0, progress = n/a:    "finished"
       *//**
       * Callback to indicate that a particular attachment is being synced
       * messageId = the message that owns the attachment
       * attachmentId = the attachment being synced
       * statusCode = 0 for OK, 1 for progress, other codes for error
       * progress = 0 for "start", 1..100 for optional progress reports
       */
  public void loadAttachmentStatus(long messageId, long attachmentId, int statusCode, int progress) throws android.os.RemoteException;
}
