/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view;
/**
 * Listener to be invoked when wallpaper visibility changes.
 * {@hide}
 */
public interface IWallpaperVisibilityListener extends android.os.IInterface
{
  /** Default implementation for IWallpaperVisibilityListener. */
  public static class Default implements android.view.IWallpaperVisibilityListener
  {
    /**
         * Method that will be invoked when wallpaper becomes visible or hidden.
         * @param visible True if wallpaper is being displayed; false otherwise.
         * @param displayId The id of the display where wallpaper visibility changed.
         */
    @Override public void onWallpaperVisibilityChanged(boolean visible, int displayId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.IWallpaperVisibilityListener
  {
    private static final java.lang.String DESCRIPTOR = "android.view.IWallpaperVisibilityListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.IWallpaperVisibilityListener interface,
     * generating a proxy if needed.
     */
    public static android.view.IWallpaperVisibilityListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.IWallpaperVisibilityListener))) {
        return ((android.view.IWallpaperVisibilityListener)iin);
      }
      return new android.view.IWallpaperVisibilityListener.Stub.Proxy(obj);
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
        case TRANSACTION_onWallpaperVisibilityChanged:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          this.onWallpaperVisibilityChanged(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.IWallpaperVisibilityListener
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
           * Method that will be invoked when wallpaper becomes visible or hidden.
           * @param visible True if wallpaper is being displayed; false otherwise.
           * @param displayId The id of the display where wallpaper visibility changed.
           */
      @Override public void onWallpaperVisibilityChanged(boolean visible, int displayId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((visible)?(1):(0)));
          _data.writeInt(displayId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onWallpaperVisibilityChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onWallpaperVisibilityChanged(visible, displayId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.view.IWallpaperVisibilityListener sDefaultImpl;
    }
    static final int TRANSACTION_onWallpaperVisibilityChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.view.IWallpaperVisibilityListener impl) {
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
    public static android.view.IWallpaperVisibilityListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Method that will be invoked when wallpaper becomes visible or hidden.
       * @param visible True if wallpaper is being displayed; false otherwise.
       * @param displayId The id of the display where wallpaper visibility changed.
       */
  public void onWallpaperVisibilityChanged(boolean visible, int displayId) throws android.os.RemoteException;
}
