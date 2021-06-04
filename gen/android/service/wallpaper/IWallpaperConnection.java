/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.wallpaper;
/**
 * @hide
 */
public interface IWallpaperConnection extends android.os.IInterface
{
  /** Default implementation for IWallpaperConnection. */
  public static class Default implements android.service.wallpaper.IWallpaperConnection
  {
    @Override public void attachEngine(android.service.wallpaper.IWallpaperEngine engine, int displayId) throws android.os.RemoteException
    {
    }
    @Override public void engineShown(android.service.wallpaper.IWallpaperEngine engine) throws android.os.RemoteException
    {
    }
    @Override public android.os.ParcelFileDescriptor setWallpaper(java.lang.String name) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void onWallpaperColorsChanged(android.app.WallpaperColors colors, int displayId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.wallpaper.IWallpaperConnection
  {
    private static final java.lang.String DESCRIPTOR = "android.service.wallpaper.IWallpaperConnection";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.wallpaper.IWallpaperConnection interface,
     * generating a proxy if needed.
     */
    public static android.service.wallpaper.IWallpaperConnection asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.wallpaper.IWallpaperConnection))) {
        return ((android.service.wallpaper.IWallpaperConnection)iin);
      }
      return new android.service.wallpaper.IWallpaperConnection.Stub.Proxy(obj);
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
        case TRANSACTION_attachEngine:
        {
          data.enforceInterface(descriptor);
          android.service.wallpaper.IWallpaperEngine _arg0;
          _arg0 = android.service.wallpaper.IWallpaperEngine.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.attachEngine(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_engineShown:
        {
          data.enforceInterface(descriptor);
          android.service.wallpaper.IWallpaperEngine _arg0;
          _arg0 = android.service.wallpaper.IWallpaperEngine.Stub.asInterface(data.readStrongBinder());
          this.engineShown(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setWallpaper:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.ParcelFileDescriptor _result = this.setWallpaper(_arg0);
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
        case TRANSACTION_onWallpaperColorsChanged:
        {
          data.enforceInterface(descriptor);
          android.app.WallpaperColors _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.WallpaperColors.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.onWallpaperColorsChanged(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.wallpaper.IWallpaperConnection
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
      @Override public void attachEngine(android.service.wallpaper.IWallpaperEngine engine, int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((engine!=null))?(engine.asBinder()):(null)));
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_attachEngine, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().attachEngine(engine, displayId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void engineShown(android.service.wallpaper.IWallpaperEngine engine) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((engine!=null))?(engine.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_engineShown, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().engineShown(engine);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.os.ParcelFileDescriptor setWallpaper(java.lang.String name) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.ParcelFileDescriptor _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(name);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setWallpaper, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setWallpaper(name);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(_reply);
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
      @Override public void onWallpaperColorsChanged(android.app.WallpaperColors colors, int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((colors!=null)) {
            _data.writeInt(1);
            colors.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onWallpaperColorsChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onWallpaperColorsChanged(colors, displayId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.service.wallpaper.IWallpaperConnection sDefaultImpl;
    }
    static final int TRANSACTION_attachEngine = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_engineShown = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_setWallpaper = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onWallpaperColorsChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.service.wallpaper.IWallpaperConnection impl) {
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
    public static android.service.wallpaper.IWallpaperConnection getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void attachEngine(android.service.wallpaper.IWallpaperEngine engine, int displayId) throws android.os.RemoteException;
  public void engineShown(android.service.wallpaper.IWallpaperEngine engine) throws android.os.RemoteException;
  public android.os.ParcelFileDescriptor setWallpaper(java.lang.String name) throws android.os.RemoteException;
  public void onWallpaperColorsChanged(android.app.WallpaperColors colors, int displayId) throws android.os.RemoteException;
}
