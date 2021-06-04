/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view;
/**
 * An interface to the PinnedStackController to update it of state changes, and to query
 * information based on the current state.
 *
 * @hide
 */
public interface IPinnedStackController extends android.os.IInterface
{
  /** Default implementation for IPinnedStackController. */
  public static class Default implements android.view.IPinnedStackController
  {
    /**
         * Notifies the controller that the PiP is currently minimized.
         */
    @Override public void setIsMinimized(boolean isMinimized) throws android.os.RemoteException
    {
    }
    /**
         * Notifies the controller of the current min edge size, this is needed to allow the system to
         * properly calculate the aspect ratio of the expanded PIP.  The given {@param minEdgeSize} is
         * always bounded to be larger than the default minEdgeSize, so the caller can call this method
         * with 0 to reset to the default size.
         */
    @Override public void setMinEdgeSize(int minEdgeSize) throws android.os.RemoteException
    {
    }
    /**
         * @return what WM considers to be the current device rotation.
         */
    @Override public int getDisplayRotation() throws android.os.RemoteException
    {
      return 0;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.IPinnedStackController
  {
    private static final java.lang.String DESCRIPTOR = "android.view.IPinnedStackController";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.IPinnedStackController interface,
     * generating a proxy if needed.
     */
    public static android.view.IPinnedStackController asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.IPinnedStackController))) {
        return ((android.view.IPinnedStackController)iin);
      }
      return new android.view.IPinnedStackController.Stub.Proxy(obj);
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
        case TRANSACTION_setIsMinimized:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.setIsMinimized(_arg0);
          return true;
        }
        case TRANSACTION_setMinEdgeSize:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setMinEdgeSize(_arg0);
          return true;
        }
        case TRANSACTION_getDisplayRotation:
        {
          data.enforceInterface(descriptor);
          int _result = this.getDisplayRotation();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.IPinnedStackController
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
           * Notifies the controller that the PiP is currently minimized.
           */
      @Override public void setIsMinimized(boolean isMinimized) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((isMinimized)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setIsMinimized, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setIsMinimized(isMinimized);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the controller of the current min edge size, this is needed to allow the system to
           * properly calculate the aspect ratio of the expanded PIP.  The given {@param minEdgeSize} is
           * always bounded to be larger than the default minEdgeSize, so the caller can call this method
           * with 0 to reset to the default size.
           */
      @Override public void setMinEdgeSize(int minEdgeSize) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(minEdgeSize);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMinEdgeSize, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setMinEdgeSize(minEdgeSize);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * @return what WM considers to be the current device rotation.
           */
      @Override public int getDisplayRotation() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDisplayRotation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getDisplayRotation();
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.view.IPinnedStackController sDefaultImpl;
    }
    static final int TRANSACTION_setIsMinimized = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_setMinEdgeSize = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getDisplayRotation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.view.IPinnedStackController impl) {
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
    public static android.view.IPinnedStackController getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Notifies the controller that the PiP is currently minimized.
       */
  public void setIsMinimized(boolean isMinimized) throws android.os.RemoteException;
  /**
       * Notifies the controller of the current min edge size, this is needed to allow the system to
       * properly calculate the aspect ratio of the expanded PIP.  The given {@param minEdgeSize} is
       * always bounded to be larger than the default minEdgeSize, so the caller can call this method
       * with 0 to reset to the default size.
       */
  public void setMinEdgeSize(int minEdgeSize) throws android.os.RemoteException;
  /**
       * @return what WM considers to be the current device rotation.
       */
  public int getDisplayRotation() throws android.os.RemoteException;
}
