/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.contentcapture;
/**
 * Interface from the Content Capture service to the system.
 *
 * @hide
 */
public interface IContentCaptureServiceCallback extends android.os.IInterface
{
  /** Default implementation for IContentCaptureServiceCallback. */
  public static class Default implements android.service.contentcapture.IContentCaptureServiceCallback
  {
    @Override public void setContentCaptureWhitelist(java.util.List<java.lang.String> packages, java.util.List<android.content.ComponentName> activities) throws android.os.RemoteException
    {
    }
    @Override public void setContentCaptureConditions(java.lang.String packageName, java.util.List<android.view.contentcapture.ContentCaptureCondition> conditions) throws android.os.RemoteException
    {
    }
    @Override public void disableSelf() throws android.os.RemoteException
    {
    }
    // Logs aggregated content capture flush metrics to Statsd

    @Override public void writeSessionFlush(int sessionId, android.content.ComponentName app, android.service.contentcapture.FlushMetrics flushMetrics, android.content.ContentCaptureOptions options, int flushReason) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.contentcapture.IContentCaptureServiceCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.service.contentcapture.IContentCaptureServiceCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.contentcapture.IContentCaptureServiceCallback interface,
     * generating a proxy if needed.
     */
    public static android.service.contentcapture.IContentCaptureServiceCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.contentcapture.IContentCaptureServiceCallback))) {
        return ((android.service.contentcapture.IContentCaptureServiceCallback)iin);
      }
      return new android.service.contentcapture.IContentCaptureServiceCallback.Stub.Proxy(obj);
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
        case TRANSACTION_setContentCaptureWhitelist:
        {
          data.enforceInterface(descriptor);
          java.util.List<java.lang.String> _arg0;
          _arg0 = data.createStringArrayList();
          java.util.List<android.content.ComponentName> _arg1;
          _arg1 = data.createTypedArrayList(android.content.ComponentName.CREATOR);
          this.setContentCaptureWhitelist(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setContentCaptureConditions:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.util.List<android.view.contentcapture.ContentCaptureCondition> _arg1;
          _arg1 = data.createTypedArrayList(android.view.contentcapture.ContentCaptureCondition.CREATOR);
          this.setContentCaptureConditions(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_disableSelf:
        {
          data.enforceInterface(descriptor);
          this.disableSelf();
          return true;
        }
        case TRANSACTION_writeSessionFlush:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.content.ComponentName _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.service.contentcapture.FlushMetrics _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.service.contentcapture.FlushMetrics.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.content.ContentCaptureOptions _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.content.ContentCaptureOptions.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          int _arg4;
          _arg4 = data.readInt();
          this.writeSessionFlush(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.contentcapture.IContentCaptureServiceCallback
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
      @Override public void setContentCaptureWhitelist(java.util.List<java.lang.String> packages, java.util.List<android.content.ComponentName> activities) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStringList(packages);
          _data.writeTypedList(activities);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setContentCaptureWhitelist, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setContentCaptureWhitelist(packages, activities);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setContentCaptureConditions(java.lang.String packageName, java.util.List<android.view.contentcapture.ContentCaptureCondition> conditions) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeTypedList(conditions);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setContentCaptureConditions, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setContentCaptureConditions(packageName, conditions);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void disableSelf() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableSelf, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableSelf();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      // Logs aggregated content capture flush metrics to Statsd

      @Override public void writeSessionFlush(int sessionId, android.content.ComponentName app, android.service.contentcapture.FlushMetrics flushMetrics, android.content.ContentCaptureOptions options, int flushReason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sessionId);
          if ((app!=null)) {
            _data.writeInt(1);
            app.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((flushMetrics!=null)) {
            _data.writeInt(1);
            flushMetrics.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(flushReason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_writeSessionFlush, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().writeSessionFlush(sessionId, app, flushMetrics, options, flushReason);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.contentcapture.IContentCaptureServiceCallback sDefaultImpl;
    }
    static final int TRANSACTION_setContentCaptureWhitelist = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_setContentCaptureConditions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_disableSelf = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_writeSessionFlush = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.service.contentcapture.IContentCaptureServiceCallback impl) {
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
    public static android.service.contentcapture.IContentCaptureServiceCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void setContentCaptureWhitelist(java.util.List<java.lang.String> packages, java.util.List<android.content.ComponentName> activities) throws android.os.RemoteException;
  public void setContentCaptureConditions(java.lang.String packageName, java.util.List<android.view.contentcapture.ContentCaptureCondition> conditions) throws android.os.RemoteException;
  public void disableSelf() throws android.os.RemoteException;
  // Logs aggregated content capture flush metrics to Statsd

  public void writeSessionFlush(int sessionId, android.content.ComponentName app, android.service.contentcapture.FlushMetrics flushMetrics, android.content.ContentCaptureOptions options, int flushReason) throws android.os.RemoteException;
}
