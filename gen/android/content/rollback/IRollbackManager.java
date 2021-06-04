/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.content.rollback;
/** {@hide} */
public interface IRollbackManager extends android.os.IInterface
{
  /** Default implementation for IRollbackManager. */
  public static class Default implements android.content.rollback.IRollbackManager
  {
    @Override public android.content.pm.ParceledListSlice getAvailableRollbacks() throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.content.pm.ParceledListSlice getRecentlyExecutedRollbacks() throws android.os.RemoteException
    {
      return null;
    }
    @Override public void commitRollback(int rollbackId, android.content.pm.ParceledListSlice causePackages, java.lang.String callerPackageName, android.content.IntentSender statusReceiver) throws android.os.RemoteException
    {
    }
    // Exposed for use from the system server only. Callback from the package
    // manager during the install flow when user data can be backed up and restored for a given
    // package.

    @Override public void snapshotAndRestoreUserData(java.lang.String packageName, int[] userIds, int appId, long ceDataInode, java.lang.String seInfo, int token) throws android.os.RemoteException
    {
    }
    // Exposed for test purposes only.

    @Override public void reloadPersistedData() throws android.os.RemoteException
    {
    }
    // Exposed for test purposes only.

    @Override public void expireRollbackForPackage(java.lang.String packageName) throws android.os.RemoteException
    {
    }
    // Used by the staging manager to notify the RollbackManager that a session is
    // being staged. In the case of multi-package sessions, the specified sessionId
    // is that of the parent session.
    //
    // NOTE: This call is synchronous.

    @Override public boolean notifyStagedSession(int sessionId) throws android.os.RemoteException
    {
      return false;
    }
    // Used by the staging manager to notify the RollbackManager of the apk
    // session for a staged session.

    @Override public void notifyStagedApkSession(int originalSessionId, int apkSessionId) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.content.rollback.IRollbackManager
  {
    private static final java.lang.String DESCRIPTOR = "android.content.rollback.IRollbackManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.content.rollback.IRollbackManager interface,
     * generating a proxy if needed.
     */
    public static android.content.rollback.IRollbackManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.content.rollback.IRollbackManager))) {
        return ((android.content.rollback.IRollbackManager)iin);
      }
      return new android.content.rollback.IRollbackManager.Stub.Proxy(obj);
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
        case TRANSACTION_getAvailableRollbacks:
        {
          data.enforceInterface(descriptor);
          android.content.pm.ParceledListSlice _result = this.getAvailableRollbacks();
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_getRecentlyExecutedRollbacks:
        {
          data.enforceInterface(descriptor);
          android.content.pm.ParceledListSlice _result = this.getRecentlyExecutedRollbacks();
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_commitRollback:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.content.pm.ParceledListSlice _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.content.IntentSender _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.content.IntentSender.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          this.commitRollback(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_snapshotAndRestoreUserData:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int[] _arg1;
          _arg1 = data.createIntArray();
          int _arg2;
          _arg2 = data.readInt();
          long _arg3;
          _arg3 = data.readLong();
          java.lang.String _arg4;
          _arg4 = data.readString();
          int _arg5;
          _arg5 = data.readInt();
          this.snapshotAndRestoreUserData(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_reloadPersistedData:
        {
          data.enforceInterface(descriptor);
          this.reloadPersistedData();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_expireRollbackForPackage:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.expireRollbackForPackage(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_notifyStagedSession:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.notifyStagedSession(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_notifyStagedApkSession:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.notifyStagedApkSession(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.content.rollback.IRollbackManager
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
      @Override public android.content.pm.ParceledListSlice getAvailableRollbacks() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAvailableRollbacks, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAvailableRollbacks();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.content.pm.ParceledListSlice getRecentlyExecutedRollbacks() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRecentlyExecutedRollbacks, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRecentlyExecutedRollbacks();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void commitRollback(int rollbackId, android.content.pm.ParceledListSlice causePackages, java.lang.String callerPackageName, android.content.IntentSender statusReceiver) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rollbackId);
          if ((causePackages!=null)) {
            _data.writeInt(1);
            causePackages.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callerPackageName);
          if ((statusReceiver!=null)) {
            _data.writeInt(1);
            statusReceiver.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_commitRollback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().commitRollback(rollbackId, causePackages, callerPackageName, statusReceiver);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Exposed for use from the system server only. Callback from the package
      // manager during the install flow when user data can be backed up and restored for a given
      // package.

      @Override public void snapshotAndRestoreUserData(java.lang.String packageName, int[] userIds, int appId, long ceDataInode, java.lang.String seInfo, int token) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeIntArray(userIds);
          _data.writeInt(appId);
          _data.writeLong(ceDataInode);
          _data.writeString(seInfo);
          _data.writeInt(token);
          boolean _status = mRemote.transact(Stub.TRANSACTION_snapshotAndRestoreUserData, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().snapshotAndRestoreUserData(packageName, userIds, appId, ceDataInode, seInfo, token);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Exposed for test purposes only.

      @Override public void reloadPersistedData() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reloadPersistedData, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reloadPersistedData();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Exposed for test purposes only.

      @Override public void expireRollbackForPackage(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_expireRollbackForPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().expireRollbackForPackage(packageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Used by the staging manager to notify the RollbackManager that a session is
      // being staged. In the case of multi-package sessions, the specified sessionId
      // is that of the parent session.
      //
      // NOTE: This call is synchronous.

      @Override public boolean notifyStagedSession(int sessionId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sessionId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyStagedSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().notifyStagedSession(sessionId);
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
      // Used by the staging manager to notify the RollbackManager of the apk
      // session for a staged session.

      @Override public void notifyStagedApkSession(int originalSessionId, int apkSessionId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(originalSessionId);
          _data.writeInt(apkSessionId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_notifyStagedApkSession, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().notifyStagedApkSession(originalSessionId, apkSessionId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.content.rollback.IRollbackManager sDefaultImpl;
    }
    static final int TRANSACTION_getAvailableRollbacks = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getRecentlyExecutedRollbacks = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_commitRollback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_snapshotAndRestoreUserData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_reloadPersistedData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_expireRollbackForPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_notifyStagedSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_notifyStagedApkSession = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    public static boolean setDefaultImpl(android.content.rollback.IRollbackManager impl) {
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
    public static android.content.rollback.IRollbackManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public android.content.pm.ParceledListSlice getAvailableRollbacks() throws android.os.RemoteException;
  public android.content.pm.ParceledListSlice getRecentlyExecutedRollbacks() throws android.os.RemoteException;
  public void commitRollback(int rollbackId, android.content.pm.ParceledListSlice causePackages, java.lang.String callerPackageName, android.content.IntentSender statusReceiver) throws android.os.RemoteException;
  // Exposed for use from the system server only. Callback from the package
  // manager during the install flow when user data can be backed up and restored for a given
  // package.

  public void snapshotAndRestoreUserData(java.lang.String packageName, int[] userIds, int appId, long ceDataInode, java.lang.String seInfo, int token) throws android.os.RemoteException;
  // Exposed for test purposes only.

  public void reloadPersistedData() throws android.os.RemoteException;
  // Exposed for test purposes only.

  public void expireRollbackForPackage(java.lang.String packageName) throws android.os.RemoteException;
  // Used by the staging manager to notify the RollbackManager that a session is
  // being staged. In the case of multi-package sessions, the specified sessionId
  // is that of the parent session.
  //
  // NOTE: This call is synchronous.

  public boolean notifyStagedSession(int sessionId) throws android.os.RemoteException;
  // Used by the staging manager to notify the RollbackManager of the apk
  // session for a staged session.

  public void notifyStagedApkSession(int originalSessionId, int apkSessionId) throws android.os.RemoteException;
}
