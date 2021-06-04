/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.display;
/** @hide */
public interface IVirtualDisplayCallback extends android.os.IInterface
{
  /** Default implementation for IVirtualDisplayCallback. */
  public static class Default implements android.hardware.display.IVirtualDisplayCallback
  {
    /**
         * Called when the virtual display video projection has been
         * paused by the system or when the surface has been detached
         * by the application by calling setSurface(null).
         * The surface will not receive any more buffers while paused.
         */
    @Override public void onPaused() throws android.os.RemoteException
    {
    }
    /**
         * Called when the virtual display video projection has been
         * resumed after having been paused.
         */
    @Override public void onResumed() throws android.os.RemoteException
    {
    }
    /**
         * Called when the virtual display video projection has been
         * stopped by the system.  It will no longer receive frames
         * and it will never be resumed.  It is still the responsibility
         * of the application to release() the virtual display.
         */
    @Override public void onStopped() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.display.IVirtualDisplayCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.display.IVirtualDisplayCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.display.IVirtualDisplayCallback interface,
     * generating a proxy if needed.
     */
    public static android.hardware.display.IVirtualDisplayCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.display.IVirtualDisplayCallback))) {
        return ((android.hardware.display.IVirtualDisplayCallback)iin);
      }
      return new android.hardware.display.IVirtualDisplayCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onPaused:
        {
          data.enforceInterface(descriptor);
          this.onPaused();
          return true;
        }
        case TRANSACTION_onResumed:
        {
          data.enforceInterface(descriptor);
          this.onResumed();
          return true;
        }
        case TRANSACTION_onStopped:
        {
          data.enforceInterface(descriptor);
          this.onStopped();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.display.IVirtualDisplayCallback
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
           * Called when the virtual display video projection has been
           * paused by the system or when the surface has been detached
           * by the application by calling setSurface(null).
           * The surface will not receive any more buffers while paused.
           */
      @Override public void onPaused() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onPaused, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onPaused();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when the virtual display video projection has been
           * resumed after having been paused.
           */
      @Override public void onResumed() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onResumed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onResumed();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when the virtual display video projection has been
           * stopped by the system.  It will no longer receive frames
           * and it will never be resumed.  It is still the responsibility
           * of the application to release() the virtual display.
           */
      @Override public void onStopped() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onStopped, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onStopped();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.hardware.display.IVirtualDisplayCallback sDefaultImpl;
    }
    static final int TRANSACTION_onPaused = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onResumed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onStopped = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.hardware.display.IVirtualDisplayCallback impl) {
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
    public static android.hardware.display.IVirtualDisplayCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Called when the virtual display video projection has been
       * paused by the system or when the surface has been detached
       * by the application by calling setSurface(null).
       * The surface will not receive any more buffers while paused.
       */
  public void onPaused() throws android.os.RemoteException;
  /**
       * Called when the virtual display video projection has been
       * resumed after having been paused.
       */
  public void onResumed() throws android.os.RemoteException;
  /**
       * Called when the virtual display video projection has been
       * stopped by the system.  It will no longer receive frames
       * and it will never be resumed.  It is still the responsibility
       * of the application to release() the virtual display.
       */
  public void onStopped() throws android.os.RemoteException;
}
