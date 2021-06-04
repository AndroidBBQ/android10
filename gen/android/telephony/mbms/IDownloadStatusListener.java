/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.mbms;
/**
 * The optional interface used by download clients to track download status.
 * @hide
 */
public interface IDownloadStatusListener extends android.os.IInterface
{
  /** Default implementation for IDownloadStatusListener. */
  public static class Default implements android.telephony.mbms.IDownloadStatusListener
  {
    @Override public void onStatusUpdated(android.telephony.mbms.DownloadRequest request, android.telephony.mbms.FileInfo fileInfo, int status) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.mbms.IDownloadStatusListener
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.mbms.IDownloadStatusListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.mbms.IDownloadStatusListener interface,
     * generating a proxy if needed.
     */
    public static android.telephony.mbms.IDownloadStatusListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.mbms.IDownloadStatusListener))) {
        return ((android.telephony.mbms.IDownloadStatusListener)iin);
      }
      return new android.telephony.mbms.IDownloadStatusListener.Stub.Proxy(obj);
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
        case TRANSACTION_onStatusUpdated:
        {
          data.enforceInterface(descriptor);
          android.telephony.mbms.DownloadRequest _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.telephony.mbms.DownloadRequest.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.telephony.mbms.FileInfo _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telephony.mbms.FileInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          this.onStatusUpdated(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.mbms.IDownloadStatusListener
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
      @Override public void onStatusUpdated(android.telephony.mbms.DownloadRequest request, android.telephony.mbms.FileInfo fileInfo, int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((fileInfo!=null)) {
            _data.writeInt(1);
            fileInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onStatusUpdated, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onStatusUpdated(request, fileInfo, status);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.telephony.mbms.IDownloadStatusListener sDefaultImpl;
    }
    static final int TRANSACTION_onStatusUpdated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.telephony.mbms.IDownloadStatusListener impl) {
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
    public static android.telephony.mbms.IDownloadStatusListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onStatusUpdated(android.telephony.mbms.DownloadRequest request, android.telephony.mbms.FileInfo fileInfo, int status) throws android.os.RemoteException;
}
