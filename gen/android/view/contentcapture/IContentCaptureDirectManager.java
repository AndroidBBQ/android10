/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view.contentcapture;
/**
  * Interface between an app (ContentCaptureManager / ContentCaptureSession) and the app providing
  * the ContentCaptureService implementation.
  *
  * @hide
  */
public interface IContentCaptureDirectManager extends android.os.IInterface
{
  /** Default implementation for IContentCaptureDirectManager. */
  public static class Default implements android.view.contentcapture.IContentCaptureDirectManager
  {
    // reason and options are used only for metrics logging.

    @Override public void sendEvents(android.content.pm.ParceledListSlice events, int reason, android.content.ContentCaptureOptions options) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.contentcapture.IContentCaptureDirectManager
  {
    private static final java.lang.String DESCRIPTOR = "android.view.contentcapture.IContentCaptureDirectManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.contentcapture.IContentCaptureDirectManager interface,
     * generating a proxy if needed.
     */
    public static android.view.contentcapture.IContentCaptureDirectManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.contentcapture.IContentCaptureDirectManager))) {
        return ((android.view.contentcapture.IContentCaptureDirectManager)iin);
      }
      return new android.view.contentcapture.IContentCaptureDirectManager.Stub.Proxy(obj);
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
        case TRANSACTION_sendEvents:
        {
          data.enforceInterface(descriptor);
          android.content.pm.ParceledListSlice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          android.content.ContentCaptureOptions _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.content.ContentCaptureOptions.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.sendEvents(_arg0, _arg1, _arg2);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.contentcapture.IContentCaptureDirectManager
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
      // reason and options are used only for metrics logging.

      @Override public void sendEvents(android.content.pm.ParceledListSlice events, int reason, android.content.ContentCaptureOptions options) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((events!=null)) {
            _data.writeInt(1);
            events.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(reason);
          if ((options!=null)) {
            _data.writeInt(1);
            options.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendEvents, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendEvents(events, reason, options);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.view.contentcapture.IContentCaptureDirectManager sDefaultImpl;
    }
    static final int TRANSACTION_sendEvents = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.view.contentcapture.IContentCaptureDirectManager impl) {
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
    public static android.view.contentcapture.IContentCaptureDirectManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // reason and options are used only for metrics logging.

  public void sendEvents(android.content.pm.ParceledListSlice events, int reason, android.content.ContentCaptureOptions options) throws android.os.RemoteException;
}
