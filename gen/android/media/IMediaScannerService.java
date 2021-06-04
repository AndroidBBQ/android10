/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.media;
/**
 * {@hide}
 */
public interface IMediaScannerService extends android.os.IInterface
{
  /** Default implementation for IMediaScannerService. */
  public static class Default implements android.media.IMediaScannerService
  {
    /**
         * Requests the media scanner to scan a file.
         * @param path the path to the file to be scanned.
         * @param mimeType  an optional mimeType for the file.
         * If mimeType is null, then the mimeType will be inferred from the file extension.
         * @param listener an optional IMediaScannerListener. 
         * If specified, the caller will be notified when scanning is complete via the listener.
         */
    @Override public void requestScanFile(java.lang.String path, java.lang.String mimeType, android.media.IMediaScannerListener listener) throws android.os.RemoteException
    {
    }
    /**
         * Older API, left in for backward compatibility.
         * Requests the media scanner to scan a file.
         * @param path the path to the file to be scanned.
         * @param mimeType  an optional mimeType for the file.
         * If mimeType is null, then the mimeType will be inferred from the file extension.
         */
    @Override public void scanFile(java.lang.String path, java.lang.String mimeType) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.media.IMediaScannerService
  {
    private static final java.lang.String DESCRIPTOR = "android.media.IMediaScannerService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.media.IMediaScannerService interface,
     * generating a proxy if needed.
     */
    public static android.media.IMediaScannerService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.media.IMediaScannerService))) {
        return ((android.media.IMediaScannerService)iin);
      }
      return new android.media.IMediaScannerService.Stub.Proxy(obj);
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
        case TRANSACTION_requestScanFile:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.media.IMediaScannerListener _arg2;
          _arg2 = android.media.IMediaScannerListener.Stub.asInterface(data.readStrongBinder());
          this.requestScanFile(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_scanFile:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.scanFile(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.media.IMediaScannerService
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
           * Requests the media scanner to scan a file.
           * @param path the path to the file to be scanned.
           * @param mimeType  an optional mimeType for the file.
           * If mimeType is null, then the mimeType will be inferred from the file extension.
           * @param listener an optional IMediaScannerListener. 
           * If specified, the caller will be notified when scanning is complete via the listener.
           */
      @Override public void requestScanFile(java.lang.String path, java.lang.String mimeType, android.media.IMediaScannerListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(path);
          _data.writeString(mimeType);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestScanFile, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestScanFile(path, mimeType, listener);
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
           * Older API, left in for backward compatibility.
           * Requests the media scanner to scan a file.
           * @param path the path to the file to be scanned.
           * @param mimeType  an optional mimeType for the file.
           * If mimeType is null, then the mimeType will be inferred from the file extension.
           */
      @Override public void scanFile(java.lang.String path, java.lang.String mimeType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(path);
          _data.writeString(mimeType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_scanFile, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().scanFile(path, mimeType);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.media.IMediaScannerService sDefaultImpl;
    }
    static final int TRANSACTION_requestScanFile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_scanFile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.media.IMediaScannerService impl) {
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
    public static android.media.IMediaScannerService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Requests the media scanner to scan a file.
       * @param path the path to the file to be scanned.
       * @param mimeType  an optional mimeType for the file.
       * If mimeType is null, then the mimeType will be inferred from the file extension.
       * @param listener an optional IMediaScannerListener. 
       * If specified, the caller will be notified when scanning is complete via the listener.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/media/java/android/media/IMediaScannerService.aidl:34:1:34:25")
  public void requestScanFile(java.lang.String path, java.lang.String mimeType, android.media.IMediaScannerListener listener) throws android.os.RemoteException;
  /**
       * Older API, left in for backward compatibility.
       * Requests the media scanner to scan a file.
       * @param path the path to the file to be scanned.
       * @param mimeType  an optional mimeType for the file.
       * If mimeType is null, then the mimeType will be inferred from the file extension.
       */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/media/java/android/media/IMediaScannerService.aidl:44:1:44:25")
  public void scanFile(java.lang.String path, java.lang.String mimeType) throws android.os.RemoteException;
}
