/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app.backup;
/**
 * Observer of a full backup or restore process.  The observer is told "interesting"
 * information about an ongoing full backup or restore action.
 *
 * {@hide}
 */
public interface IFullBackupRestoreObserver extends android.os.IInterface
{
  /** Default implementation for IFullBackupRestoreObserver. */
  public static class Default implements android.app.backup.IFullBackupRestoreObserver
  {
    /**
         * Notification: a full backup operation has begun.
         */
    @Override public void onStartBackup() throws android.os.RemoteException
    {
    }
    /**
         * Notification: the system has begun backing up the given package.
         *
         * @param name The name of the application being saved.  This will typically be a
         *     user-meaningful name such as "Browser" rather than a package name such as
         *     "com.android.browser", though this is not guaranteed.
         */
    @Override public void onBackupPackage(java.lang.String name) throws android.os.RemoteException
    {
    }
    /**
         * Notification: the full backup operation has ended.
         */
    @Override public void onEndBackup() throws android.os.RemoteException
    {
    }
    /**
         * Notification: a restore-from-full-backup operation has begun.
         */
    @Override public void onStartRestore() throws android.os.RemoteException
    {
    }
    /**
         * Notification: the system has begun restore of the given package.
         *
         * @param name The name of the application being saved.  This will typically be a
         *     user-meaningful name such as "Browser" rather than a package name such as
         *     "com.android.browser", though this is not guaranteed.
         */
    @Override public void onRestorePackage(java.lang.String name) throws android.os.RemoteException
    {
    }
    /**
         * Notification: the restore-from-full-backup operation has ended.
         */
    @Override public void onEndRestore() throws android.os.RemoteException
    {
    }
    /**
         * The user's window of opportunity for confirming the operation has timed out.
         */
    @Override public void onTimeout() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.backup.IFullBackupRestoreObserver
  {
    private static final java.lang.String DESCRIPTOR = "android.app.backup.IFullBackupRestoreObserver";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.backup.IFullBackupRestoreObserver interface,
     * generating a proxy if needed.
     */
    public static android.app.backup.IFullBackupRestoreObserver asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.backup.IFullBackupRestoreObserver))) {
        return ((android.app.backup.IFullBackupRestoreObserver)iin);
      }
      return new android.app.backup.IFullBackupRestoreObserver.Stub.Proxy(obj);
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
        case TRANSACTION_onStartBackup:
        {
          data.enforceInterface(descriptor);
          this.onStartBackup();
          return true;
        }
        case TRANSACTION_onBackupPackage:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.onBackupPackage(_arg0);
          return true;
        }
        case TRANSACTION_onEndBackup:
        {
          data.enforceInterface(descriptor);
          this.onEndBackup();
          return true;
        }
        case TRANSACTION_onStartRestore:
        {
          data.enforceInterface(descriptor);
          this.onStartRestore();
          return true;
        }
        case TRANSACTION_onRestorePackage:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.onRestorePackage(_arg0);
          return true;
        }
        case TRANSACTION_onEndRestore:
        {
          data.enforceInterface(descriptor);
          this.onEndRestore();
          return true;
        }
        case TRANSACTION_onTimeout:
        {
          data.enforceInterface(descriptor);
          this.onTimeout();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.backup.IFullBackupRestoreObserver
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
           * Notification: a full backup operation has begun.
           */
      @Override public void onStartBackup() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onStartBackup, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onStartBackup();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notification: the system has begun backing up the given package.
           *
           * @param name The name of the application being saved.  This will typically be a
           *     user-meaningful name such as "Browser" rather than a package name such as
           *     "com.android.browser", though this is not guaranteed.
           */
      @Override public void onBackupPackage(java.lang.String name) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(name);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onBackupPackage, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onBackupPackage(name);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notification: the full backup operation has ended.
           */
      @Override public void onEndBackup() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEndBackup, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEndBackup();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notification: a restore-from-full-backup operation has begun.
           */
      @Override public void onStartRestore() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onStartRestore, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onStartRestore();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notification: the system has begun restore of the given package.
           *
           * @param name The name of the application being saved.  This will typically be a
           *     user-meaningful name such as "Browser" rather than a package name such as
           *     "com.android.browser", though this is not guaranteed.
           */
      @Override public void onRestorePackage(java.lang.String name) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(name);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onRestorePackage, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onRestorePackage(name);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notification: the restore-from-full-backup operation has ended.
           */
      @Override public void onEndRestore() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEndRestore, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEndRestore();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * The user's window of opportunity for confirming the operation has timed out.
           */
      @Override public void onTimeout() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onTimeout, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onTimeout();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.app.backup.IFullBackupRestoreObserver sDefaultImpl;
    }
    static final int TRANSACTION_onStartBackup = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onBackupPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onEndBackup = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onStartRestore = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_onRestorePackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_onEndRestore = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_onTimeout = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    public static boolean setDefaultImpl(android.app.backup.IFullBackupRestoreObserver impl) {
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
    public static android.app.backup.IFullBackupRestoreObserver getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Notification: a full backup operation has begun.
       */
  public void onStartBackup() throws android.os.RemoteException;
  /**
       * Notification: the system has begun backing up the given package.
       *
       * @param name The name of the application being saved.  This will typically be a
       *     user-meaningful name such as "Browser" rather than a package name such as
       *     "com.android.browser", though this is not guaranteed.
       */
  public void onBackupPackage(java.lang.String name) throws android.os.RemoteException;
  /**
       * Notification: the full backup operation has ended.
       */
  public void onEndBackup() throws android.os.RemoteException;
  /**
       * Notification: a restore-from-full-backup operation has begun.
       */
  public void onStartRestore() throws android.os.RemoteException;
  /**
       * Notification: the system has begun restore of the given package.
       *
       * @param name The name of the application being saved.  This will typically be a
       *     user-meaningful name such as "Browser" rather than a package name such as
       *     "com.android.browser", though this is not guaranteed.
       */
  public void onRestorePackage(java.lang.String name) throws android.os.RemoteException;
  /**
       * Notification: the restore-from-full-backup operation has ended.
       */
  public void onEndRestore() throws android.os.RemoteException;
  /**
       * The user's window of opportunity for confirming the operation has timed out.
       */
  public void onTimeout() throws android.os.RemoteException;
}
