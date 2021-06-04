/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.debug;
/**
 * Interface to communicate remotely with the {@code AdbService} in the system server.
 *
 * @hide
 */
public interface IAdbManager extends android.os.IInterface
{
  /** Default implementation for IAdbManager. */
  public static class Default implements android.debug.IAdbManager
  {
    /**
         * Allow ADB debugging from the attached host. If {@code alwaysAllow} is
         * {@code true}, add {@code publicKey} to list of host keys that the
         * user has approved.
         *
         * @param alwaysAllow if true, add permanently to list of allowed keys
         * @param publicKey RSA key in mincrypt format and Base64-encoded
         */
    @Override public void allowDebugging(boolean alwaysAllow, java.lang.String publicKey) throws android.os.RemoteException
    {
    }
    /**
         * Deny ADB debugging from the attached host.
         */
    @Override public void denyDebugging() throws android.os.RemoteException
    {
    }
    /**
         * Clear all public keys installed for secure ADB debugging.
         */
    @Override public void clearDebuggingKeys() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.debug.IAdbManager
  {
    private static final java.lang.String DESCRIPTOR = "android.debug.IAdbManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.debug.IAdbManager interface,
     * generating a proxy if needed.
     */
    public static android.debug.IAdbManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.debug.IAdbManager))) {
        return ((android.debug.IAdbManager)iin);
      }
      return new android.debug.IAdbManager.Stub.Proxy(obj);
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
        case TRANSACTION_allowDebugging:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.allowDebugging(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_denyDebugging:
        {
          data.enforceInterface(descriptor);
          this.denyDebugging();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_clearDebuggingKeys:
        {
          data.enforceInterface(descriptor);
          this.clearDebuggingKeys();
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.debug.IAdbManager
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
           * Allow ADB debugging from the attached host. If {@code alwaysAllow} is
           * {@code true}, add {@code publicKey} to list of host keys that the
           * user has approved.
           *
           * @param alwaysAllow if true, add permanently to list of allowed keys
           * @param publicKey RSA key in mincrypt format and Base64-encoded
           */
      @Override public void allowDebugging(boolean alwaysAllow, java.lang.String publicKey) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((alwaysAllow)?(1):(0)));
          _data.writeString(publicKey);
          boolean _status = mRemote.transact(Stub.TRANSACTION_allowDebugging, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().allowDebugging(alwaysAllow, publicKey);
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
           * Deny ADB debugging from the attached host.
           */
      @Override public void denyDebugging() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_denyDebugging, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().denyDebugging();
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
           * Clear all public keys installed for secure ADB debugging.
           */
      @Override public void clearDebuggingKeys() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearDebuggingKeys, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearDebuggingKeys();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.debug.IAdbManager sDefaultImpl;
    }
    static final int TRANSACTION_allowDebugging = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_denyDebugging = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_clearDebuggingKeys = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.debug.IAdbManager impl) {
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
    public static android.debug.IAdbManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Allow ADB debugging from the attached host. If {@code alwaysAllow} is
       * {@code true}, add {@code publicKey} to list of host keys that the
       * user has approved.
       *
       * @param alwaysAllow if true, add permanently to list of allowed keys
       * @param publicKey RSA key in mincrypt format and Base64-encoded
       */
  public void allowDebugging(boolean alwaysAllow, java.lang.String publicKey) throws android.os.RemoteException;
  /**
       * Deny ADB debugging from the attached host.
       */
  public void denyDebugging() throws android.os.RemoteException;
  /**
       * Clear all public keys installed for secure ADB debugging.
       */
  public void clearDebuggingKeys() throws android.os.RemoteException;
}
