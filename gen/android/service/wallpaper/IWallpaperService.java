/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.wallpaper;
/**
 * @hide
 */
public interface IWallpaperService extends android.os.IInterface
{
  /** Default implementation for IWallpaperService. */
  public static class Default implements android.service.wallpaper.IWallpaperService
  {
    @Override public void attach(android.service.wallpaper.IWallpaperConnection connection, android.os.IBinder windowToken, int windowType, boolean isPreview, int reqWidth, int reqHeight, android.graphics.Rect padding, int displayId) throws android.os.RemoteException
    {
    }
    @Override public void detach() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.wallpaper.IWallpaperService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.wallpaper.IWallpaperService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.wallpaper.IWallpaperService interface,
     * generating a proxy if needed.
     */
    public static android.service.wallpaper.IWallpaperService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.wallpaper.IWallpaperService))) {
        return ((android.service.wallpaper.IWallpaperService)iin);
      }
      return new android.service.wallpaper.IWallpaperService.Stub.Proxy(obj);
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
        case TRANSACTION_attach:
        {
          data.enforceInterface(descriptor);
          android.service.wallpaper.IWallpaperConnection _arg0;
          _arg0 = android.service.wallpaper.IWallpaperConnection.Stub.asInterface(data.readStrongBinder());
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          int _arg2;
          _arg2 = data.readInt();
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          int _arg4;
          _arg4 = data.readInt();
          int _arg5;
          _arg5 = data.readInt();
          android.graphics.Rect _arg6;
          if ((0!=data.readInt())) {
            _arg6 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg6 = null;
          }
          int _arg7;
          _arg7 = data.readInt();
          this.attach(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7);
          return true;
        }
        case TRANSACTION_detach:
        {
          data.enforceInterface(descriptor);
          this.detach();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.wallpaper.IWallpaperService
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
      @Override public void attach(android.service.wallpaper.IWallpaperConnection connection, android.os.IBinder windowToken, int windowType, boolean isPreview, int reqWidth, int reqHeight, android.graphics.Rect padding, int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((connection!=null))?(connection.asBinder()):(null)));
          _data.writeStrongBinder(windowToken);
          _data.writeInt(windowType);
          _data.writeInt(((isPreview)?(1):(0)));
          _data.writeInt(reqWidth);
          _data.writeInt(reqHeight);
          if ((padding!=null)) {
            _data.writeInt(1);
            padding.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_attach, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().attach(connection, windowToken, windowType, isPreview, reqWidth, reqHeight, padding, displayId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void detach() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_detach, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().detach();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.wallpaper.IWallpaperService sDefaultImpl;
    }
    static final int TRANSACTION_attach = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_detach = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.service.wallpaper.IWallpaperService impl) {
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
    public static android.service.wallpaper.IWallpaperService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void attach(android.service.wallpaper.IWallpaperConnection connection, android.os.IBinder windowToken, int windowType, boolean isPreview, int reqWidth, int reqHeight, android.graphics.Rect padding, int displayId) throws android.os.RemoteException;
  public void detach() throws android.os.RemoteException;
}
