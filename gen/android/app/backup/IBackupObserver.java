/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app.backup;
/**
 * Callback class for receiving progress reports during a backup operation.  These
 * methods will all be called on your application's main thread.
 *
 * @hide
 */
public interface IBackupObserver extends android.os.IInterface
{
  /** Default implementation for IBackupObserver. */
  public static class Default implements android.app.backup.IBackupObserver
  {
    /**
         * This method could be called several times for packages with full data backup.
         * It will tell how much of backup data is already saved and how much is expected.
         *
         * @param currentBackupPackage The name of the package that now being backed up.
         * @param backupProgress Current progress of backup for the package.
         */
    @Override public void onUpdate(java.lang.String currentPackage, android.app.backup.BackupProgress backupProgress) throws android.os.RemoteException
    {
    }
    /**
         * Backup of one package or initialization of one transport has completed.  This
         * method will be called at most one time for each package or transport, and might not
         * be not called if the operation fails before backupFinished(); for example, if the
         * requested package/transport does not exist.
         *
         * @param target The name of the package that was backed up, or of the transport
         *                  that was initialized
         * @param status Zero on success; a nonzero error code if the backup operation failed.
         */
    @Override public void onResult(java.lang.String target, int status) throws android.os.RemoteException
    {
    }
    /**
         * The backup process has completed.  This method will always be called,
         * even if no individual package backup operations were attempted.
         *
         * @param status Zero on success; a nonzero error code if the backup operation
         *   as a whole failed with a transport error.
         */
    @Override public void backupFinished(int status) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.backup.IBackupObserver
  {
    private static final java.lang.String DESCRIPTOR = "android.app.backup.IBackupObserver";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.backup.IBackupObserver interface,
     * generating a proxy if needed.
     */
    public static android.app.backup.IBackupObserver asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.backup.IBackupObserver))) {
        return ((android.app.backup.IBackupObserver)iin);
      }
      return new android.app.backup.IBackupObserver.Stub.Proxy(obj);
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
        case TRANSACTION_onUpdate:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.app.backup.BackupProgress _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.app.backup.BackupProgress.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.onUpdate(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onResult:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.onResult(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_backupFinished:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.backupFinished(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.backup.IBackupObserver
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
           * This method could be called several times for packages with full data backup.
           * It will tell how much of backup data is already saved and how much is expected.
           *
           * @param currentBackupPackage The name of the package that now being backed up.
           * @param backupProgress Current progress of backup for the package.
           */
      @Override public void onUpdate(java.lang.String currentPackage, android.app.backup.BackupProgress backupProgress) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(currentPackage);
          if ((backupProgress!=null)) {
            _data.writeInt(1);
            backupProgress.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onUpdate, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onUpdate(currentPackage, backupProgress);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Backup of one package or initialization of one transport has completed.  This
           * method will be called at most one time for each package or transport, and might not
           * be not called if the operation fails before backupFinished(); for example, if the
           * requested package/transport does not exist.
           *
           * @param target The name of the package that was backed up, or of the transport
           *                  that was initialized
           * @param status Zero on success; a nonzero error code if the backup operation failed.
           */
      @Override public void onResult(java.lang.String target, int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(target);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onResult(target, status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * The backup process has completed.  This method will always be called,
           * even if no individual package backup operations were attempted.
           *
           * @param status Zero on success; a nonzero error code if the backup operation
           *   as a whole failed with a transport error.
           */
      @Override public void backupFinished(int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_backupFinished, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().backupFinished(status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.app.backup.IBackupObserver sDefaultImpl;
    }
    static final int TRANSACTION_onUpdate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_backupFinished = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.app.backup.IBackupObserver impl) {
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
    public static android.app.backup.IBackupObserver getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * This method could be called several times for packages with full data backup.
       * It will tell how much of backup data is already saved and how much is expected.
       *
       * @param currentBackupPackage The name of the package that now being backed up.
       * @param backupProgress Current progress of backup for the package.
       */
  public void onUpdate(java.lang.String currentPackage, android.app.backup.BackupProgress backupProgress) throws android.os.RemoteException;
  /**
       * Backup of one package or initialization of one transport has completed.  This
       * method will be called at most one time for each package or transport, and might not
       * be not called if the operation fails before backupFinished(); for example, if the
       * requested package/transport does not exist.
       *
       * @param target The name of the package that was backed up, or of the transport
       *                  that was initialized
       * @param status Zero on success; a nonzero error code if the backup operation failed.
       */
  public void onResult(java.lang.String target, int status) throws android.os.RemoteException;
  /**
       * The backup process has completed.  This method will always be called,
       * even if no individual package backup operations were attempted.
       *
       * @param status Zero on success; a nonzero error code if the backup operation
       *   as a whole failed with a transport error.
       */
  public void backupFinished(int status) throws android.os.RemoteException;
}
