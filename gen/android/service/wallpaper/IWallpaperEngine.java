/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.wallpaper;
/**
 * @hide
 */
public interface IWallpaperEngine extends android.os.IInterface
{
  /** Default implementation for IWallpaperEngine. */
  public static class Default implements android.service.wallpaper.IWallpaperEngine
  {
    @Override public void setDesiredSize(int width, int height) throws android.os.RemoteException
    {
    }
    @Override public void setDisplayPadding(android.graphics.Rect padding) throws android.os.RemoteException
    {
    }
    @Override public void setVisibility(boolean visible) throws android.os.RemoteException
    {
    }
    @Override public void setInAmbientMode(boolean inAmbientDisplay, long animationDuration) throws android.os.RemoteException
    {
    }
    @Override public void dispatchPointer(android.view.MotionEvent event) throws android.os.RemoteException
    {
    }
    @Override public void dispatchWallpaperCommand(java.lang.String action, int x, int y, int z, android.os.Bundle extras) throws android.os.RemoteException
    {
    }
    @Override public void requestWallpaperColors() throws android.os.RemoteException
    {
    }
    @Override public void destroy() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.wallpaper.IWallpaperEngine
  {
    private static final java.lang.String DESCRIPTOR = "android.service.wallpaper.IWallpaperEngine";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.wallpaper.IWallpaperEngine interface,
     * generating a proxy if needed.
     */
    public static android.service.wallpaper.IWallpaperEngine asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.wallpaper.IWallpaperEngine))) {
        return ((android.service.wallpaper.IWallpaperEngine)iin);
      }
      return new android.service.wallpaper.IWallpaperEngine.Stub.Proxy(obj);
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
        case TRANSACTION_setDesiredSize:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.setDesiredSize(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_setDisplayPadding:
        {
          data.enforceInterface(descriptor);
          android.graphics.Rect _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.graphics.Rect.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setDisplayPadding(_arg0);
          return true;
        }
        case TRANSACTION_setVisibility:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setVisibility(_arg0);
          return true;
        }
        case TRANSACTION_setInAmbientMode:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          long _arg1;
          _arg1 = data.readLong();
          this.setInAmbientMode(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_dispatchPointer:
        {
          data.enforceInterface(descriptor);
          android.view.MotionEvent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.view.MotionEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.dispatchPointer(_arg0);
          return true;
        }
        case TRANSACTION_dispatchWallpaperCommand:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          android.os.Bundle _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          this.dispatchWallpaperCommand(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_requestWallpaperColors:
        {
          data.enforceInterface(descriptor);
          this.requestWallpaperColors();
          return true;
        }
        case TRANSACTION_destroy:
        {
          data.enforceInterface(descriptor);
          this.destroy();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.wallpaper.IWallpaperEngine
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
      @Override public void setDesiredSize(int width, int height) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(width);
          _data.writeInt(height);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDesiredSize, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDesiredSize(width, height);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setDisplayPadding(android.graphics.Rect padding) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((padding!=null)) {
            _data.writeInt(1);
            padding.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDisplayPadding, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDisplayPadding(padding);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setVisibility(boolean visible) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((visible)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVisibility, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVisibility(visible);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setInAmbientMode(boolean inAmbientDisplay, long animationDuration) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((inAmbientDisplay)?(1):(0)));
          _data.writeLong(animationDuration);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setInAmbientMode, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setInAmbientMode(inAmbientDisplay, animationDuration);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void dispatchPointer(android.view.MotionEvent event) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((event!=null)) {
            _data.writeInt(1);
            event.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchPointer, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispatchPointer(event);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void dispatchWallpaperCommand(java.lang.String action, int x, int y, int z, android.os.Bundle extras) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(action);
          _data.writeInt(x);
          _data.writeInt(y);
          _data.writeInt(z);
          if ((extras!=null)) {
            _data.writeInt(1);
            extras.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_dispatchWallpaperCommand, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().dispatchWallpaperCommand(action, x, y, z, extras);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void requestWallpaperColors() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestWallpaperColors, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestWallpaperColors();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void destroy() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_destroy, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().destroy();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.wallpaper.IWallpaperEngine sDefaultImpl;
    }
    static final int TRANSACTION_setDesiredSize = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_setDisplayPadding = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_setVisibility = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_setInAmbientMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_dispatchPointer = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_dispatchWallpaperCommand = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_requestWallpaperColors = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_destroy = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    public static boolean setDefaultImpl(android.service.wallpaper.IWallpaperEngine impl) {
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
    public static android.service.wallpaper.IWallpaperEngine getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void setDesiredSize(int width, int height) throws android.os.RemoteException;
  public void setDisplayPadding(android.graphics.Rect padding) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/service/wallpaper/IWallpaperEngine.aidl:29:1:29:25")
  public void setVisibility(boolean visible) throws android.os.RemoteException;
  public void setInAmbientMode(boolean inAmbientDisplay, long animationDuration) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/service/wallpaper/IWallpaperEngine.aidl:32:1:32:25")
  public void dispatchPointer(android.view.MotionEvent event) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/service/wallpaper/IWallpaperEngine.aidl:34:1:34:25")
  public void dispatchWallpaperCommand(java.lang.String action, int x, int y, int z, android.os.Bundle extras) throws android.os.RemoteException;
  public void requestWallpaperColors() throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/service/wallpaper/IWallpaperEngine.aidl:38:1:38:25")
  public void destroy() throws android.os.RemoteException;
}
