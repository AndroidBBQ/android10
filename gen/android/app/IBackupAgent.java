/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app;
/**
 * Interface presented by applications being asked to participate in the
 * backup & restore mechanism.  End user code will not typically implement
 * this interface directly; they subclass BackupAgent instead.
 *
 * {@hide}
 */
public interface IBackupAgent extends android.os.IInterface
{
  /** Default implementation for IBackupAgent. */
  public static class Default implements android.app.IBackupAgent
  {
    /**
         * Request that the app perform an incremental backup.
         *
         * @param oldState Read-only file containing the description blob of the
         *        app's data state as of the last backup operation's completion.
         *        This file is empty or invalid when a full backup is being
         *        requested.
         *
         * @param data Read-write file, empty when onBackup() is called, that
         *        is the data destination for this backup pass's incrementals.
         *
         * @param newState Read-write file, empty when onBackup() is called,
         *        where the new state blob is to be recorded.
         *
         * @param quota Quota reported by the transport for this backup operation (in bytes).
         *
         * @param token Opaque token identifying this transaction.  This must
         *        be echoed back to the backup service binder once the new
         *        data has been written to the data and newState files.
         *
         * @param callbackBinder Binder on which to indicate operation completion.
         *
         * @param transportFlags Flags with additional information about the transport.
         */
    @Override public void doBackup(android.os.ParcelFileDescriptor oldState, android.os.ParcelFileDescriptor data, android.os.ParcelFileDescriptor newState, long quotaBytes, android.app.backup.IBackupCallback callbackBinder, int transportFlags) throws android.os.RemoteException
    {
    }
    /**
         * Restore an entire data snapshot to the application.
         *
         * @param data Read-only file containing the full data snapshot of the
         *        app's backup.  This is to be a <i>replacement</i> of the app's
         *        current data, not to be merged into it.
         *
         * @param appVersionCode The android:versionCode attribute of the application
         *        that created this data set.  This can help the agent distinguish among
         *        various historical backup content possibilities.
         *
         * @param newState Read-write file, empty when onRestore() is called,
         *        that is to be written with the state description that holds after
         *        the restore has been completed.
         *
         * @param token Opaque token identifying this transaction.  This must
         *        be echoed back to the backup service binder once the agent is
         *        finished restoring the application based on the restore data
         *        contents.
         *
         * @param callbackBinder Binder on which to indicate operation completion,
         *        passed here as a convenience to the agent.
         */
    @Override public void doRestore(android.os.ParcelFileDescriptor data, long appVersionCode, android.os.ParcelFileDescriptor newState, int token, android.app.backup.IBackupManager callbackBinder) throws android.os.RemoteException
    {
    }
    /**
         * Perform a "full" backup to the given file descriptor.  The output file is presumed
         * to be a socket or other non-seekable, write-only data sink.  When this method is
         * called, the app should write all of its files to the output.
         *
         * @param data Write-only file to receive the backed-up file content stream.
         *        The data must be formatted correctly for the resulting archive to be
         *        legitimate, so that will be tightly controlled by the available API.
         *
         * @param quota Quota reported by the transport for this backup operation (in bytes).
         *
         * @param token Opaque token identifying this transaction.  This must
         *        be echoed back to the backup service binder once the agent is
         *        finished restoring the application based on the restore data
         *        contents.
         *
         * @param callbackBinder Binder on which to indicate operation completion,
         *        passed here as a convenience to the agent.
         *
         * @param transportFlags Flags with additional information about transport.
         */
    @Override public void doFullBackup(android.os.ParcelFileDescriptor data, long quotaBytes, int token, android.app.backup.IBackupManager callbackBinder, int transportFlags) throws android.os.RemoteException
    {
    }
    /**
         * Estimate how much data a full backup will deliver
         */
    @Override public void doMeasureFullBackup(long quotaBytes, int token, android.app.backup.IBackupManager callbackBinder, int transportFlags) throws android.os.RemoteException
    {
    }
    /**
         * Tells the application agent that the backup data size exceeded current transport quota.
         * Later calls to {@link #onBackup(ParcelFileDescriptor, BackupDataOutput, ParcelFileDescriptor)}
         * and {@link #onFullBackup(FullBackupDataOutput)} could use this information
         * to reduce backup size under the limit.
         * However, the quota can change, so do not assume that the value passed in here is absolute,
         * similarly all subsequent backups should not be restricted to this size.
         * This callback will be invoked before data has been put onto the wire in a preflight check,
         * so it is relatively inexpensive to hit your quota.
         * Apps that hit quota repeatedly without dealing with it can be subject to having their backup
         * schedule reduced.
         * The {@code quotaBytes} is a loose guideline b/c of metadata added by the backupmanager
         * so apps should be more aggressive in trimming their backup set.
         *
         * @param backupDataBytes Expected or already processed amount of data.
         *                        Could be less than total backup size if backup process was interrupted
         *                        before finish of processing all backup data.
         * @param quotaBytes Current amount of backup data that is allowed for the app.
         * @param callbackBinder Binder on which to indicate operation completion.
         */
    @Override public void doQuotaExceeded(long backupDataBytes, long quotaBytes, android.app.backup.IBackupCallback callbackBinder) throws android.os.RemoteException
    {
    }
    /**
         * Restore a single "file" to the application.  The file was typically obtained from
         * a full-backup dataset.  The agent reads 'size' bytes of file content
         * from the provided file descriptor.
         *
         * @param data Read-only pipe delivering the file content itself.
         *
         * @param size Size of the file being restored.
         * @param type Type of file system entity, e.g. FullBackup.TYPE_DIRECTORY.
         * @param domain Name of the file's semantic domain to which the 'path' argument is a
         *        relative path.  e.g. FullBackup.DATABASE_TREE_TOKEN.
         * @param path Relative path of the file within its semantic domain.
         * @param mode Access mode of the file system entity, e.g. 0660.
         * @param mtime Last modification time of the file system entity.
         * @param token Opaque token identifying this transaction.  This must
         *        be echoed back to the backup service binder once the agent is
         *        finished restoring the application based on the restore data
         *        contents.
         * @param callbackBinder Binder on which to indicate operation completion,
         *        passed here as a convenience to the agent.
         */
    @Override public void doRestoreFile(android.os.ParcelFileDescriptor data, long size, int type, java.lang.String domain, java.lang.String path, long mode, long mtime, int token, android.app.backup.IBackupManager callbackBinder) throws android.os.RemoteException
    {
    }
    /**
         * Provide the app with a canonical "all data has been delivered" end-of-restore
         * callback so that it can do any postprocessing of the restored data that might
         * be appropriate.  This is issued after both key/value and full data restore
         * operations have completed.
         *
         * @param token Opaque token identifying this transaction.  This must
         *        be echoed back to the backup service binder once the agent is
         *        finished restoring the application based on the restore data
         *        contents.
         * @param callbackBinder Binder on which to indicate operation completion,
         *        passed here as a convenience to the agent.
         */
    @Override public void doRestoreFinished(int token, android.app.backup.IBackupManager callbackBinder) throws android.os.RemoteException
    {
    }
    /**
         * Out of band: instruct the agent to crash within the client process.  This is used
         * when the backup infrastructure detects a semantic error post-hoc and needs to
         * pass the problem back to the app.
         *
         * @param message The message to be passed to the agent's application in an exception.
         */
    @Override public void fail(java.lang.String message) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.IBackupAgent
  {
    private static final java.lang.String DESCRIPTOR = "android.app.IBackupAgent";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.IBackupAgent interface,
     * generating a proxy if needed.
     */
    public static android.app.IBackupAgent asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.IBackupAgent))) {
        return ((android.app.IBackupAgent)iin);
      }
      return new android.app.IBackupAgent.Stub.Proxy(obj);
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
        case TRANSACTION_doBackup:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.ParcelFileDescriptor _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.os.ParcelFileDescriptor _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          long _arg3;
          _arg3 = data.readLong();
          android.app.backup.IBackupCallback _arg4;
          _arg4 = android.app.backup.IBackupCallback.Stub.asInterface(data.readStrongBinder());
          int _arg5;
          _arg5 = data.readInt();
          this.doBackup(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          return true;
        }
        case TRANSACTION_doRestore:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          long _arg1;
          _arg1 = data.readLong();
          android.os.ParcelFileDescriptor _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _arg3;
          _arg3 = data.readInt();
          android.app.backup.IBackupManager _arg4;
          _arg4 = android.app.backup.IBackupManager.Stub.asInterface(data.readStrongBinder());
          this.doRestore(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_doFullBackup:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          long _arg1;
          _arg1 = data.readLong();
          int _arg2;
          _arg2 = data.readInt();
          android.app.backup.IBackupManager _arg3;
          _arg3 = android.app.backup.IBackupManager.Stub.asInterface(data.readStrongBinder());
          int _arg4;
          _arg4 = data.readInt();
          this.doFullBackup(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_doMeasureFullBackup:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          android.app.backup.IBackupManager _arg2;
          _arg2 = android.app.backup.IBackupManager.Stub.asInterface(data.readStrongBinder());
          int _arg3;
          _arg3 = data.readInt();
          this.doMeasureFullBackup(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_doQuotaExceeded:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          long _arg1;
          _arg1 = data.readLong();
          android.app.backup.IBackupCallback _arg2;
          _arg2 = android.app.backup.IBackupCallback.Stub.asInterface(data.readStrongBinder());
          this.doQuotaExceeded(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_doRestoreFile:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          long _arg1;
          _arg1 = data.readLong();
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          java.lang.String _arg4;
          _arg4 = data.readString();
          long _arg5;
          _arg5 = data.readLong();
          long _arg6;
          _arg6 = data.readLong();
          int _arg7;
          _arg7 = data.readInt();
          android.app.backup.IBackupManager _arg8;
          _arg8 = android.app.backup.IBackupManager.Stub.asInterface(data.readStrongBinder());
          this.doRestoreFile(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8);
          return true;
        }
        case TRANSACTION_doRestoreFinished:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.app.backup.IBackupManager _arg1;
          _arg1 = android.app.backup.IBackupManager.Stub.asInterface(data.readStrongBinder());
          this.doRestoreFinished(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_fail:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.fail(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.IBackupAgent
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
           * Request that the app perform an incremental backup.
           *
           * @param oldState Read-only file containing the description blob of the
           *        app's data state as of the last backup operation's completion.
           *        This file is empty or invalid when a full backup is being
           *        requested.
           *
           * @param data Read-write file, empty when onBackup() is called, that
           *        is the data destination for this backup pass's incrementals.
           *
           * @param newState Read-write file, empty when onBackup() is called,
           *        where the new state blob is to be recorded.
           *
           * @param quota Quota reported by the transport for this backup operation (in bytes).
           *
           * @param token Opaque token identifying this transaction.  This must
           *        be echoed back to the backup service binder once the new
           *        data has been written to the data and newState files.
           *
           * @param callbackBinder Binder on which to indicate operation completion.
           *
           * @param transportFlags Flags with additional information about the transport.
           */
      @Override public void doBackup(android.os.ParcelFileDescriptor oldState, android.os.ParcelFileDescriptor data, android.os.ParcelFileDescriptor newState, long quotaBytes, android.app.backup.IBackupCallback callbackBinder, int transportFlags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((oldState!=null)) {
            _data.writeInt(1);
            oldState.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((data!=null)) {
            _data.writeInt(1);
            data.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((newState!=null)) {
            _data.writeInt(1);
            newState.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeLong(quotaBytes);
          _data.writeStrongBinder((((callbackBinder!=null))?(callbackBinder.asBinder()):(null)));
          _data.writeInt(transportFlags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_doBackup, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().doBackup(oldState, data, newState, quotaBytes, callbackBinder, transportFlags);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Restore an entire data snapshot to the application.
           *
           * @param data Read-only file containing the full data snapshot of the
           *        app's backup.  This is to be a <i>replacement</i> of the app's
           *        current data, not to be merged into it.
           *
           * @param appVersionCode The android:versionCode attribute of the application
           *        that created this data set.  This can help the agent distinguish among
           *        various historical backup content possibilities.
           *
           * @param newState Read-write file, empty when onRestore() is called,
           *        that is to be written with the state description that holds after
           *        the restore has been completed.
           *
           * @param token Opaque token identifying this transaction.  This must
           *        be echoed back to the backup service binder once the agent is
           *        finished restoring the application based on the restore data
           *        contents.
           *
           * @param callbackBinder Binder on which to indicate operation completion,
           *        passed here as a convenience to the agent.
           */
      @Override public void doRestore(android.os.ParcelFileDescriptor data, long appVersionCode, android.os.ParcelFileDescriptor newState, int token, android.app.backup.IBackupManager callbackBinder) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((data!=null)) {
            _data.writeInt(1);
            data.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeLong(appVersionCode);
          if ((newState!=null)) {
            _data.writeInt(1);
            newState.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(token);
          _data.writeStrongBinder((((callbackBinder!=null))?(callbackBinder.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_doRestore, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().doRestore(data, appVersionCode, newState, token, callbackBinder);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Perform a "full" backup to the given file descriptor.  The output file is presumed
           * to be a socket or other non-seekable, write-only data sink.  When this method is
           * called, the app should write all of its files to the output.
           *
           * @param data Write-only file to receive the backed-up file content stream.
           *        The data must be formatted correctly for the resulting archive to be
           *        legitimate, so that will be tightly controlled by the available API.
           *
           * @param quota Quota reported by the transport for this backup operation (in bytes).
           *
           * @param token Opaque token identifying this transaction.  This must
           *        be echoed back to the backup service binder once the agent is
           *        finished restoring the application based on the restore data
           *        contents.
           *
           * @param callbackBinder Binder on which to indicate operation completion,
           *        passed here as a convenience to the agent.
           *
           * @param transportFlags Flags with additional information about transport.
           */
      @Override public void doFullBackup(android.os.ParcelFileDescriptor data, long quotaBytes, int token, android.app.backup.IBackupManager callbackBinder, int transportFlags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((data!=null)) {
            _data.writeInt(1);
            data.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeLong(quotaBytes);
          _data.writeInt(token);
          _data.writeStrongBinder((((callbackBinder!=null))?(callbackBinder.asBinder()):(null)));
          _data.writeInt(transportFlags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_doFullBackup, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().doFullBackup(data, quotaBytes, token, callbackBinder, transportFlags);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Estimate how much data a full backup will deliver
           */
      @Override public void doMeasureFullBackup(long quotaBytes, int token, android.app.backup.IBackupManager callbackBinder, int transportFlags) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(quotaBytes);
          _data.writeInt(token);
          _data.writeStrongBinder((((callbackBinder!=null))?(callbackBinder.asBinder()):(null)));
          _data.writeInt(transportFlags);
          boolean _status = mRemote.transact(Stub.TRANSACTION_doMeasureFullBackup, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().doMeasureFullBackup(quotaBytes, token, callbackBinder, transportFlags);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Tells the application agent that the backup data size exceeded current transport quota.
           * Later calls to {@link #onBackup(ParcelFileDescriptor, BackupDataOutput, ParcelFileDescriptor)}
           * and {@link #onFullBackup(FullBackupDataOutput)} could use this information
           * to reduce backup size under the limit.
           * However, the quota can change, so do not assume that the value passed in here is absolute,
           * similarly all subsequent backups should not be restricted to this size.
           * This callback will be invoked before data has been put onto the wire in a preflight check,
           * so it is relatively inexpensive to hit your quota.
           * Apps that hit quota repeatedly without dealing with it can be subject to having their backup
           * schedule reduced.
           * The {@code quotaBytes} is a loose guideline b/c of metadata added by the backupmanager
           * so apps should be more aggressive in trimming their backup set.
           *
           * @param backupDataBytes Expected or already processed amount of data.
           *                        Could be less than total backup size if backup process was interrupted
           *                        before finish of processing all backup data.
           * @param quotaBytes Current amount of backup data that is allowed for the app.
           * @param callbackBinder Binder on which to indicate operation completion.
           */
      @Override public void doQuotaExceeded(long backupDataBytes, long quotaBytes, android.app.backup.IBackupCallback callbackBinder) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(backupDataBytes);
          _data.writeLong(quotaBytes);
          _data.writeStrongBinder((((callbackBinder!=null))?(callbackBinder.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_doQuotaExceeded, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().doQuotaExceeded(backupDataBytes, quotaBytes, callbackBinder);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Restore a single "file" to the application.  The file was typically obtained from
           * a full-backup dataset.  The agent reads 'size' bytes of file content
           * from the provided file descriptor.
           *
           * @param data Read-only pipe delivering the file content itself.
           *
           * @param size Size of the file being restored.
           * @param type Type of file system entity, e.g. FullBackup.TYPE_DIRECTORY.
           * @param domain Name of the file's semantic domain to which the 'path' argument is a
           *        relative path.  e.g. FullBackup.DATABASE_TREE_TOKEN.
           * @param path Relative path of the file within its semantic domain.
           * @param mode Access mode of the file system entity, e.g. 0660.
           * @param mtime Last modification time of the file system entity.
           * @param token Opaque token identifying this transaction.  This must
           *        be echoed back to the backup service binder once the agent is
           *        finished restoring the application based on the restore data
           *        contents.
           * @param callbackBinder Binder on which to indicate operation completion,
           *        passed here as a convenience to the agent.
           */
      @Override public void doRestoreFile(android.os.ParcelFileDescriptor data, long size, int type, java.lang.String domain, java.lang.String path, long mode, long mtime, int token, android.app.backup.IBackupManager callbackBinder) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((data!=null)) {
            _data.writeInt(1);
            data.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeLong(size);
          _data.writeInt(type);
          _data.writeString(domain);
          _data.writeString(path);
          _data.writeLong(mode);
          _data.writeLong(mtime);
          _data.writeInt(token);
          _data.writeStrongBinder((((callbackBinder!=null))?(callbackBinder.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_doRestoreFile, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().doRestoreFile(data, size, type, domain, path, mode, mtime, token, callbackBinder);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Provide the app with a canonical "all data has been delivered" end-of-restore
           * callback so that it can do any postprocessing of the restored data that might
           * be appropriate.  This is issued after both key/value and full data restore
           * operations have completed.
           *
           * @param token Opaque token identifying this transaction.  This must
           *        be echoed back to the backup service binder once the agent is
           *        finished restoring the application based on the restore data
           *        contents.
           * @param callbackBinder Binder on which to indicate operation completion,
           *        passed here as a convenience to the agent.
           */
      @Override public void doRestoreFinished(int token, android.app.backup.IBackupManager callbackBinder) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(token);
          _data.writeStrongBinder((((callbackBinder!=null))?(callbackBinder.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_doRestoreFinished, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().doRestoreFinished(token, callbackBinder);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Out of band: instruct the agent to crash within the client process.  This is used
           * when the backup infrastructure detects a semantic error post-hoc and needs to
           * pass the problem back to the app.
           *
           * @param message The message to be passed to the agent's application in an exception.
           */
      @Override public void fail(java.lang.String message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(message);
          boolean _status = mRemote.transact(Stub.TRANSACTION_fail, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().fail(message);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.app.IBackupAgent sDefaultImpl;
    }
    static final int TRANSACTION_doBackup = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_doRestore = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_doFullBackup = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_doMeasureFullBackup = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_doQuotaExceeded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_doRestoreFile = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_doRestoreFinished = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_fail = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    public static boolean setDefaultImpl(android.app.IBackupAgent impl) {
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
    public static android.app.IBackupAgent getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Request that the app perform an incremental backup.
       *
       * @param oldState Read-only file containing the description blob of the
       *        app's data state as of the last backup operation's completion.
       *        This file is empty or invalid when a full backup is being
       *        requested.
       *
       * @param data Read-write file, empty when onBackup() is called, that
       *        is the data destination for this backup pass's incrementals.
       *
       * @param newState Read-write file, empty when onBackup() is called,
       *        where the new state blob is to be recorded.
       *
       * @param quota Quota reported by the transport for this backup operation (in bytes).
       *
       * @param token Opaque token identifying this transaction.  This must
       *        be echoed back to the backup service binder once the new
       *        data has been written to the data and newState files.
       *
       * @param callbackBinder Binder on which to indicate operation completion.
       *
       * @param transportFlags Flags with additional information about the transport.
       */
  public void doBackup(android.os.ParcelFileDescriptor oldState, android.os.ParcelFileDescriptor data, android.os.ParcelFileDescriptor newState, long quotaBytes, android.app.backup.IBackupCallback callbackBinder, int transportFlags) throws android.os.RemoteException;
  /**
       * Restore an entire data snapshot to the application.
       *
       * @param data Read-only file containing the full data snapshot of the
       *        app's backup.  This is to be a <i>replacement</i> of the app's
       *        current data, not to be merged into it.
       *
       * @param appVersionCode The android:versionCode attribute of the application
       *        that created this data set.  This can help the agent distinguish among
       *        various historical backup content possibilities.
       *
       * @param newState Read-write file, empty when onRestore() is called,
       *        that is to be written with the state description that holds after
       *        the restore has been completed.
       *
       * @param token Opaque token identifying this transaction.  This must
       *        be echoed back to the backup service binder once the agent is
       *        finished restoring the application based on the restore data
       *        contents.
       *
       * @param callbackBinder Binder on which to indicate operation completion,
       *        passed here as a convenience to the agent.
       */
  public void doRestore(android.os.ParcelFileDescriptor data, long appVersionCode, android.os.ParcelFileDescriptor newState, int token, android.app.backup.IBackupManager callbackBinder) throws android.os.RemoteException;
  /**
       * Perform a "full" backup to the given file descriptor.  The output file is presumed
       * to be a socket or other non-seekable, write-only data sink.  When this method is
       * called, the app should write all of its files to the output.
       *
       * @param data Write-only file to receive the backed-up file content stream.
       *        The data must be formatted correctly for the resulting archive to be
       *        legitimate, so that will be tightly controlled by the available API.
       *
       * @param quota Quota reported by the transport for this backup operation (in bytes).
       *
       * @param token Opaque token identifying this transaction.  This must
       *        be echoed back to the backup service binder once the agent is
       *        finished restoring the application based on the restore data
       *        contents.
       *
       * @param callbackBinder Binder on which to indicate operation completion,
       *        passed here as a convenience to the agent.
       *
       * @param transportFlags Flags with additional information about transport.
       */
  public void doFullBackup(android.os.ParcelFileDescriptor data, long quotaBytes, int token, android.app.backup.IBackupManager callbackBinder, int transportFlags) throws android.os.RemoteException;
  /**
       * Estimate how much data a full backup will deliver
       */
  public void doMeasureFullBackup(long quotaBytes, int token, android.app.backup.IBackupManager callbackBinder, int transportFlags) throws android.os.RemoteException;
  /**
       * Tells the application agent that the backup data size exceeded current transport quota.
       * Later calls to {@link #onBackup(ParcelFileDescriptor, BackupDataOutput, ParcelFileDescriptor)}
       * and {@link #onFullBackup(FullBackupDataOutput)} could use this information
       * to reduce backup size under the limit.
       * However, the quota can change, so do not assume that the value passed in here is absolute,
       * similarly all subsequent backups should not be restricted to this size.
       * This callback will be invoked before data has been put onto the wire in a preflight check,
       * so it is relatively inexpensive to hit your quota.
       * Apps that hit quota repeatedly without dealing with it can be subject to having their backup
       * schedule reduced.
       * The {@code quotaBytes} is a loose guideline b/c of metadata added by the backupmanager
       * so apps should be more aggressive in trimming their backup set.
       *
       * @param backupDataBytes Expected or already processed amount of data.
       *                        Could be less than total backup size if backup process was interrupted
       *                        before finish of processing all backup data.
       * @param quotaBytes Current amount of backup data that is allowed for the app.
       * @param callbackBinder Binder on which to indicate operation completion.
       */
  public void doQuotaExceeded(long backupDataBytes, long quotaBytes, android.app.backup.IBackupCallback callbackBinder) throws android.os.RemoteException;
  /**
       * Restore a single "file" to the application.  The file was typically obtained from
       * a full-backup dataset.  The agent reads 'size' bytes of file content
       * from the provided file descriptor.
       *
       * @param data Read-only pipe delivering the file content itself.
       *
       * @param size Size of the file being restored.
       * @param type Type of file system entity, e.g. FullBackup.TYPE_DIRECTORY.
       * @param domain Name of the file's semantic domain to which the 'path' argument is a
       *        relative path.  e.g. FullBackup.DATABASE_TREE_TOKEN.
       * @param path Relative path of the file within its semantic domain.
       * @param mode Access mode of the file system entity, e.g. 0660.
       * @param mtime Last modification time of the file system entity.
       * @param token Opaque token identifying this transaction.  This must
       *        be echoed back to the backup service binder once the agent is
       *        finished restoring the application based on the restore data
       *        contents.
       * @param callbackBinder Binder on which to indicate operation completion,
       *        passed here as a convenience to the agent.
       */
  public void doRestoreFile(android.os.ParcelFileDescriptor data, long size, int type, java.lang.String domain, java.lang.String path, long mode, long mtime, int token, android.app.backup.IBackupManager callbackBinder) throws android.os.RemoteException;
  /**
       * Provide the app with a canonical "all data has been delivered" end-of-restore
       * callback so that it can do any postprocessing of the restored data that might
       * be appropriate.  This is issued after both key/value and full data restore
       * operations have completed.
       *
       * @param token Opaque token identifying this transaction.  This must
       *        be echoed back to the backup service binder once the agent is
       *        finished restoring the application based on the restore data
       *        contents.
       * @param callbackBinder Binder on which to indicate operation completion,
       *        passed here as a convenience to the agent.
       */
  public void doRestoreFinished(int token, android.app.backup.IBackupManager callbackBinder) throws android.os.RemoteException;
  /**
       * Out of band: instruct the agent to crash within the client process.  This is used
       * when the backup infrastructure detects a semantic error post-hoc and needs to
       * pass the problem back to the app.
       *
       * @param message The message to be passed to the agent's application in an exception.
       */
  public void fail(java.lang.String message) throws android.os.RemoteException;
}
