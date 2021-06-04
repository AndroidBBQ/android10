/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app.backup;
/**
 * Callback class for receiving success or failure callbacks on selecting a backup transport. These
 * methods will all be called on your application's main thread.
 *
 * @hide
 */
public interface ISelectBackupTransportCallback extends android.os.IInterface
{
  /** Default implementation for ISelectBackupTransportCallback. */
  public static class Default implements android.app.backup.ISelectBackupTransportCallback
  {
    /**
         * Called when BackupManager has successfully bound to the requested transport.
         *
         * @param transportName Name of the selected transport. This is the String returned by
         *        {@link BackupTransport#name()}.
         */
    @Override public void onSuccess(java.lang.String transportName) throws android.os.RemoteException
    {
    }
    /**
         * Called when BackupManager fails to bind to the requested transport.
         *
         * @param reason Error code denoting reason for failure.
         */
    @Override public void onFailure(int reason) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.backup.ISelectBackupTransportCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.app.backup.ISelectBackupTransportCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.backup.ISelectBackupTransportCallback interface,
     * generating a proxy if needed.
     */
    public static android.app.backup.ISelectBackupTransportCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.backup.ISelectBackupTransportCallback))) {
        return ((android.app.backup.ISelectBackupTransportCallback)iin);
      }
      return new android.app.backup.ISelectBackupTransportCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onSuccess:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.onSuccess(_arg0);
          return true;
        }
        case TRANSACTION_onFailure:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onFailure(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.backup.ISelectBackupTransportCallback
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
           * Called when BackupManager has successfully bound to the requested transport.
           *
           * @param transportName Name of the selected transport. This is the String returned by
           *        {@link BackupTransport#name()}.
           */
      @Override public void onSuccess(java.lang.String transportName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(transportName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSuccess, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSuccess(transportName);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when BackupManager fails to bind to the requested transport.
           *
           * @param reason Error code denoting reason for failure.
           */
      @Override public void onFailure(int reason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(reason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onFailure, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onFailure(reason);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.app.backup.ISelectBackupTransportCallback sDefaultImpl;
    }
    static final int TRANSACTION_onSuccess = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onFailure = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.app.backup.ISelectBackupTransportCallback impl) {
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
    public static android.app.backup.ISelectBackupTransportCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Called when BackupManager has successfully bound to the requested transport.
       *
       * @param transportName Name of the selected transport. This is the String returned by
       *        {@link BackupTransport#name()}.
       */
  public void onSuccess(java.lang.String transportName) throws android.os.RemoteException;
  /**
       * Called when BackupManager fails to bind to the requested transport.
       *
       * @param reason Error code denoting reason for failure.
       */
  public void onFailure(int reason) throws android.os.RemoteException;
}
