/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app;
/**
 * Interface used to control special UI modes.
 * @hide
 */
public interface IUiModeManager extends android.os.IInterface
{
  /** Default implementation for IUiModeManager. */
  public static class Default implements android.app.IUiModeManager
  {
    /**
         * Enables the car mode. Only the system can do this.
         * @hide
         */
    @Override public void enableCarMode(int flags) throws android.os.RemoteException
    {
    }
    /**
         * Disables the car mode.
         */
    @Override public void disableCarMode(int flags) throws android.os.RemoteException
    {
    }
    /**
         * Return the current running mode.
         */
    @Override public int getCurrentModeType() throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Sets the night mode.
         * The mode can be one of:
         *   1 - notnight mode
         *   2 - night mode
         *   3 - automatic mode switching
         */
    @Override public void setNightMode(int mode) throws android.os.RemoteException
    {
    }
    /**
         * Gets the currently configured night mode.  Return 1 for notnight,
         * 2 for night, and 3 for automatic mode switching.
         */
    @Override public int getNightMode() throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Tells if UI mode is locked or not.
         */
    @Override public boolean isUiModeLocked() throws android.os.RemoteException
    {
      return false;
    }
    /**
         * Tells if Night mode is locked or not.
         */
    @Override public boolean isNightModeLocked() throws android.os.RemoteException
    {
      return false;
    }
    /**
        * @hide
        */
    @Override public boolean setNightModeActivated(boolean active) throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.IUiModeManager
  {
    private static final java.lang.String DESCRIPTOR = "android.app.IUiModeManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.IUiModeManager interface,
     * generating a proxy if needed.
     */
    public static android.app.IUiModeManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.IUiModeManager))) {
        return ((android.app.IUiModeManager)iin);
      }
      return new android.app.IUiModeManager.Stub.Proxy(obj);
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
        case TRANSACTION_enableCarMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.enableCarMode(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_disableCarMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.disableCarMode(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getCurrentModeType:
        {
          data.enforceInterface(descriptor);
          int _result = this.getCurrentModeType();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setNightMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.setNightMode(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getNightMode:
        {
          data.enforceInterface(descriptor);
          int _result = this.getNightMode();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_isUiModeLocked:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isUiModeLocked();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isNightModeLocked:
        {
          data.enforceInterface(descriptor);
          boolean _result = this.isNightModeLocked();
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setNightModeActivated:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          boolean _result = this.setNightModeActivated(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.IUiModeManager
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
           * Enables the car mode. Only the system can do this.
           * @hide
           */
      @Override public void enableCarMode(int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableCarMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enableCarMode(flags);
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
           * Disables the car mode.
           */
      @Override public void disableCarMode(int flags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(flags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableCarMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableCarMode(flags);
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
           * Return the current running mode.
           */
      @Override public int getCurrentModeType() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentModeType, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentModeType();
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
      /**
           * Sets the night mode.
           * The mode can be one of:
           *   1 - notnight mode
           *   2 - night mode
           *   3 - automatic mode switching
           */
      @Override public void setNightMode(int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setNightMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setNightMode(mode);
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
           * Gets the currently configured night mode.  Return 1 for notnight,
           * 2 for night, and 3 for automatic mode switching.
           */
      @Override public int getNightMode() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNightMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNightMode();
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
      /**
           * Tells if UI mode is locked or not.
           */
      @Override public boolean isUiModeLocked() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isUiModeLocked, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isUiModeLocked();
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Tells if Night mode is locked or not.
           */
      @Override public boolean isNightModeLocked() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isNightModeLocked, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isNightModeLocked();
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
          * @hide
          */
      @Override public boolean setNightModeActivated(boolean active) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((active)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setNightModeActivated, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setNightModeActivated(active);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.app.IUiModeManager sDefaultImpl;
    }
    static final int TRANSACTION_enableCarMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_disableCarMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getCurrentModeType = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_setNightMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getNightMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_isUiModeLocked = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_isNightModeLocked = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_setNightModeActivated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    public static boolean setDefaultImpl(android.app.IUiModeManager impl) {
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
    public static android.app.IUiModeManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Enables the car mode. Only the system can do this.
       * @hide
       */
  public void enableCarMode(int flags) throws android.os.RemoteException;
  /**
       * Disables the car mode.
       */
  public void disableCarMode(int flags) throws android.os.RemoteException;
  /**
       * Return the current running mode.
       */
  public int getCurrentModeType() throws android.os.RemoteException;
  /**
       * Sets the night mode.
       * The mode can be one of:
       *   1 - notnight mode
       *   2 - night mode
       *   3 - automatic mode switching
       */
  public void setNightMode(int mode) throws android.os.RemoteException;
  /**
       * Gets the currently configured night mode.  Return 1 for notnight,
       * 2 for night, and 3 for automatic mode switching.
       */
  public int getNightMode() throws android.os.RemoteException;
  /**
       * Tells if UI mode is locked or not.
       */
  public boolean isUiModeLocked() throws android.os.RemoteException;
  /**
       * Tells if Night mode is locked or not.
       */
  public boolean isNightModeLocked() throws android.os.RemoteException;
  /**
      * @hide
      */
  public boolean setNightModeActivated(boolean active) throws android.os.RemoteException;
}
