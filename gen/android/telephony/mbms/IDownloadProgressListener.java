/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony.mbms;
/**
 * The optional interface used by download clients to track progress.
 * @hide
 */
public interface IDownloadProgressListener extends android.os.IInterface
{
  /** Default implementation for IDownloadProgressListener. */
  public static class Default implements android.telephony.mbms.IDownloadProgressListener
  {
    /**
         * Gives progress callbacks for a given DownloadRequest.  Includes a FileInfo
         * as the list of files may not have been known at request-time.
         */
    @Override public void onProgressUpdated(android.telephony.mbms.DownloadRequest request, android.telephony.mbms.FileInfo fileInfo, int currentDownloadSize, int fullDownloadSize, int currentDecodedSize, int fullDecodedSize) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.mbms.IDownloadProgressListener
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.mbms.IDownloadProgressListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.mbms.IDownloadProgressListener interface,
     * generating a proxy if needed.
     */
    public static android.telephony.mbms.IDownloadProgressListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.mbms.IDownloadProgressListener))) {
        return ((android.telephony.mbms.IDownloadProgressListener)iin);
      }
      return new android.telephony.mbms.IDownloadProgressListener.Stub.Proxy(obj);
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
        case TRANSACTION_onProgressUpdated:
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
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          int _arg5;
          _arg5 = data.readInt();
          this.onProgressUpdated(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.mbms.IDownloadProgressListener
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
           * Gives progress callbacks for a given DownloadRequest.  Includes a FileInfo
           * as the list of files may not have been known at request-time.
           */
      @Override public void onProgressUpdated(android.telephony.mbms.DownloadRequest request, android.telephony.mbms.FileInfo fileInfo, int currentDownloadSize, int fullDownloadSize, int currentDecodedSize, int fullDecodedSize) throws android.os.RemoteException
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
          _data.writeInt(currentDownloadSize);
          _data.writeInt(fullDownloadSize);
          _data.writeInt(currentDecodedSize);
          _data.writeInt(fullDecodedSize);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onProgressUpdated, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onProgressUpdated(request, fileInfo, currentDownloadSize, fullDownloadSize, currentDecodedSize, fullDecodedSize);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.telephony.mbms.IDownloadProgressListener sDefaultImpl;
    }
    static final int TRANSACTION_onProgressUpdated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.telephony.mbms.IDownloadProgressListener impl) {
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
    public static android.telephony.mbms.IDownloadProgressListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Gives progress callbacks for a given DownloadRequest.  Includes a FileInfo
       * as the list of files may not have been known at request-time.
       */
  public void onProgressUpdated(android.telephony.mbms.DownloadRequest request, android.telephony.mbms.FileInfo fileInfo, int currentDownloadSize, int fullDownloadSize, int currentDecodedSize, int fullDecodedSize) throws android.os.RemoteException;
}
