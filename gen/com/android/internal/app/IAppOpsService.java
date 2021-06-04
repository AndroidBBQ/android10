/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.app;
public interface IAppOpsService extends android.os.IInterface
{
  /** Default implementation for IAppOpsService. */
  public static class Default implements com.android.internal.app.IAppOpsService
  {
    // These methods are also called by native code, so must
    // be kept in sync with frameworks/native/libs/binder/include/binder/IAppOpsService.h
    // and not be reordered

    @Override public int checkOperation(int code, int uid, java.lang.String packageName) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int noteOperation(int code, int uid, java.lang.String packageName) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int startOperation(android.os.IBinder token, int code, int uid, java.lang.String packageName, boolean startIfModeDefault) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void finishOperation(android.os.IBinder token, int code, int uid, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    @Override public void startWatchingMode(int op, java.lang.String packageName, com.android.internal.app.IAppOpsCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void stopWatchingMode(com.android.internal.app.IAppOpsCallback callback) throws android.os.RemoteException
    {
    }
    @Override public android.os.IBinder getToken(android.os.IBinder clientToken) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int permissionToOpCode(java.lang.String permission) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public int checkAudioOperation(int code, int usage, int uid, java.lang.String packageName) throws android.os.RemoteException
    {
      return 0;
    }
    // End of methods also called by native code.
    // Any new method exposed to native must be added after the last one, do not reorder

    @Override public int noteProxyOperation(int code, int proxyUid, java.lang.String proxyPackageName, int callingUid, java.lang.String callingPackageName) throws android.os.RemoteException
    {
      return 0;
    }
    // Remaining methods are only used in Java.

    @Override public int checkPackage(int uid, java.lang.String packageName) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public java.util.List<android.app.AppOpsManager.PackageOps> getPackagesForOps(int[] ops) throws android.os.RemoteException
    {
      return null;
    }
    @Override public java.util.List<android.app.AppOpsManager.PackageOps> getOpsForPackage(int uid, java.lang.String packageName, int[] ops) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void getHistoricalOps(int uid, java.lang.String packageName, java.util.List<java.lang.String> ops, long beginTimeMillis, long endTimeMillis, int flags, android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void getHistoricalOpsFromDiskRaw(int uid, java.lang.String packageName, java.util.List<java.lang.String> ops, long beginTimeMillis, long endTimeMillis, int flags, android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void offsetHistory(long duration) throws android.os.RemoteException
    {
    }
    @Override public void setHistoryParameters(int mode, long baseSnapshotInterval, int compressionStep) throws android.os.RemoteException
    {
    }
    @Override public void addHistoricalOps(android.app.AppOpsManager.HistoricalOps ops) throws android.os.RemoteException
    {
    }
    @Override public void resetHistoryParameters() throws android.os.RemoteException
    {
    }
    @Override public void clearHistory() throws android.os.RemoteException
    {
    }
    @Override public java.util.List<android.app.AppOpsManager.PackageOps> getUidOps(int uid, int[] ops) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setUidMode(int code, int uid, int mode) throws android.os.RemoteException
    {
    }
    @Override public void setMode(int code, int uid, java.lang.String packageName, int mode) throws android.os.RemoteException
    {
    }
    @Override public void resetAllModes(int reqUserId, java.lang.String reqPackageName) throws android.os.RemoteException
    {
    }
    @Override public void setAudioRestriction(int code, int usage, int uid, int mode, java.lang.String[] exceptionPackages) throws android.os.RemoteException
    {
    }
    @Override public void setUserRestrictions(android.os.Bundle restrictions, android.os.IBinder token, int userHandle) throws android.os.RemoteException
    {
    }
    @Override public void setUserRestriction(int code, boolean restricted, android.os.IBinder token, int userHandle, java.lang.String[] exceptionPackages) throws android.os.RemoteException
    {
    }
    @Override public void removeUser(int userHandle) throws android.os.RemoteException
    {
    }
    @Override public void startWatchingActive(int[] ops, com.android.internal.app.IAppOpsActiveCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void stopWatchingActive(com.android.internal.app.IAppOpsActiveCallback callback) throws android.os.RemoteException
    {
    }
    @Override public boolean isOperationActive(int code, int uid, java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void startWatchingModeWithFlags(int op, java.lang.String packageName, int flags, com.android.internal.app.IAppOpsCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void startWatchingNoted(int[] ops, com.android.internal.app.IAppOpsNotedCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void stopWatchingNoted(com.android.internal.app.IAppOpsNotedCallback callback) throws android.os.RemoteException
    {
    }
    @Override public int checkOperationRaw(int code, int uid, java.lang.String packageName) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void reloadNonHistoricalState() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.app.IAppOpsService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.app.IAppOpsService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.app.IAppOpsService interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.app.IAppOpsService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.app.IAppOpsService))) {
        return ((com.android.internal.app.IAppOpsService)iin);
      }
      return new com.android.internal.app.IAppOpsService.Stub.Proxy(obj);
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
        case TRANSACTION_checkOperation:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _result = this.checkOperation(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_noteOperation:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _result = this.noteOperation(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_startOperation:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          boolean _arg4;
          _arg4 = (0!=data.readInt());
          int _result = this.startOperation(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_finishOperation:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.finishOperation(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startWatchingMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          com.android.internal.app.IAppOpsCallback _arg2;
          _arg2 = com.android.internal.app.IAppOpsCallback.Stub.asInterface(data.readStrongBinder());
          this.startWatchingMode(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopWatchingMode:
        {
          data.enforceInterface(descriptor);
          com.android.internal.app.IAppOpsCallback _arg0;
          _arg0 = com.android.internal.app.IAppOpsCallback.Stub.asInterface(data.readStrongBinder());
          this.stopWatchingMode(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getToken:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.os.IBinder _result = this.getToken(_arg0);
          reply.writeNoException();
          reply.writeStrongBinder(_result);
          return true;
        }
        case TRANSACTION_permissionToOpCode:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _result = this.permissionToOpCode(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_checkAudioOperation:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          int _result = this.checkAudioOperation(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_noteProxyOperation:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          java.lang.String _arg4;
          _arg4 = data.readString();
          int _result = this.noteProxyOperation(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_checkPackage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.checkPackage(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getPackagesForOps:
        {
          data.enforceInterface(descriptor);
          int[] _arg0;
          _arg0 = data.createIntArray();
          java.util.List<android.app.AppOpsManager.PackageOps> _result = this.getPackagesForOps(_arg0);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getOpsForPackage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int[] _arg2;
          _arg2 = data.createIntArray();
          java.util.List<android.app.AppOpsManager.PackageOps> _result = this.getOpsForPackage(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getHistoricalOps:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.util.List<java.lang.String> _arg2;
          _arg2 = data.createStringArrayList();
          long _arg3;
          _arg3 = data.readLong();
          long _arg4;
          _arg4 = data.readLong();
          int _arg5;
          _arg5 = data.readInt();
          android.os.RemoteCallback _arg6;
          if ((0!=data.readInt())) {
            _arg6 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg6 = null;
          }
          this.getHistoricalOps(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getHistoricalOpsFromDiskRaw:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.util.List<java.lang.String> _arg2;
          _arg2 = data.createStringArrayList();
          long _arg3;
          _arg3 = data.readLong();
          long _arg4;
          _arg4 = data.readLong();
          int _arg5;
          _arg5 = data.readInt();
          android.os.RemoteCallback _arg6;
          if ((0!=data.readInt())) {
            _arg6 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg6 = null;
          }
          this.getHistoricalOpsFromDiskRaw(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_offsetHistory:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.offsetHistory(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setHistoryParameters:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          int _arg2;
          _arg2 = data.readInt();
          this.setHistoryParameters(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addHistoricalOps:
        {
          data.enforceInterface(descriptor);
          android.app.AppOpsManager.HistoricalOps _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.AppOpsManager.HistoricalOps.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.addHistoricalOps(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_resetHistoryParameters:
        {
          data.enforceInterface(descriptor);
          this.resetHistoryParameters();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_clearHistory:
        {
          data.enforceInterface(descriptor);
          this.clearHistory();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getUidOps:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int[] _arg1;
          _arg1 = data.createIntArray();
          java.util.List<android.app.AppOpsManager.PackageOps> _result = this.getUidOps(_arg0, _arg1);
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_setUidMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.setUidMode(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setMode:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          this.setMode(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_resetAllModes:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.resetAllModes(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setAudioRestriction:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          java.lang.String[] _arg4;
          _arg4 = data.createStringArray();
          this.setAudioRestriction(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setUserRestrictions:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          int _arg2;
          _arg2 = data.readInt();
          this.setUserRestrictions(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setUserRestriction:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          android.os.IBinder _arg2;
          _arg2 = data.readStrongBinder();
          int _arg3;
          _arg3 = data.readInt();
          java.lang.String[] _arg4;
          _arg4 = data.createStringArray();
          this.setUserRestriction(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeUser:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.removeUser(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startWatchingActive:
        {
          data.enforceInterface(descriptor);
          int[] _arg0;
          _arg0 = data.createIntArray();
          com.android.internal.app.IAppOpsActiveCallback _arg1;
          _arg1 = com.android.internal.app.IAppOpsActiveCallback.Stub.asInterface(data.readStrongBinder());
          this.startWatchingActive(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopWatchingActive:
        {
          data.enforceInterface(descriptor);
          com.android.internal.app.IAppOpsActiveCallback _arg0;
          _arg0 = com.android.internal.app.IAppOpsActiveCallback.Stub.asInterface(data.readStrongBinder());
          this.stopWatchingActive(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isOperationActive:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          boolean _result = this.isOperationActive(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_startWatchingModeWithFlags:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          com.android.internal.app.IAppOpsCallback _arg3;
          _arg3 = com.android.internal.app.IAppOpsCallback.Stub.asInterface(data.readStrongBinder());
          this.startWatchingModeWithFlags(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_startWatchingNoted:
        {
          data.enforceInterface(descriptor);
          int[] _arg0;
          _arg0 = data.createIntArray();
          com.android.internal.app.IAppOpsNotedCallback _arg1;
          _arg1 = com.android.internal.app.IAppOpsNotedCallback.Stub.asInterface(data.readStrongBinder());
          this.startWatchingNoted(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_stopWatchingNoted:
        {
          data.enforceInterface(descriptor);
          com.android.internal.app.IAppOpsNotedCallback _arg0;
          _arg0 = com.android.internal.app.IAppOpsNotedCallback.Stub.asInterface(data.readStrongBinder());
          this.stopWatchingNoted(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_checkOperationRaw:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _result = this.checkOperationRaw(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_reloadNonHistoricalState:
        {
          data.enforceInterface(descriptor);
          this.reloadNonHistoricalState();
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.app.IAppOpsService
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
      // These methods are also called by native code, so must
      // be kept in sync with frameworks/native/libs/binder/include/binder/IAppOpsService.h
      // and not be reordered

      @Override public int checkOperation(int code, int uid, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(code);
          _data.writeInt(uid);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_checkOperation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().checkOperation(code, uid, packageName);
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
      @Override public int noteOperation(int code, int uid, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(code);
          _data.writeInt(uid);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_noteOperation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().noteOperation(code, uid, packageName);
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
      @Override public int startOperation(android.os.IBinder token, int code, int uid, java.lang.String packageName, boolean startIfModeDefault) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(code);
          _data.writeInt(uid);
          _data.writeString(packageName);
          _data.writeInt(((startIfModeDefault)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startOperation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().startOperation(token, code, uid, packageName, startIfModeDefault);
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
      @Override public void finishOperation(android.os.IBinder token, int code, int uid, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeInt(code);
          _data.writeInt(uid);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_finishOperation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().finishOperation(token, code, uid, packageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void startWatchingMode(int op, java.lang.String packageName, com.android.internal.app.IAppOpsCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(op);
          _data.writeString(packageName);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startWatchingMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startWatchingMode(op, packageName, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stopWatchingMode(com.android.internal.app.IAppOpsCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopWatchingMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopWatchingMode(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.os.IBinder getToken(android.os.IBinder clientToken) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.IBinder _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(clientToken);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getToken, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getToken(clientToken);
          }
          _reply.readException();
          _result = _reply.readStrongBinder();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public int permissionToOpCode(java.lang.String permission) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(permission);
          boolean _status = mRemote.transact(Stub.TRANSACTION_permissionToOpCode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().permissionToOpCode(permission);
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
      @Override public int checkAudioOperation(int code, int usage, int uid, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(code);
          _data.writeInt(usage);
          _data.writeInt(uid);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_checkAudioOperation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().checkAudioOperation(code, usage, uid, packageName);
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
      // End of methods also called by native code.
      // Any new method exposed to native must be added after the last one, do not reorder

      @Override public int noteProxyOperation(int code, int proxyUid, java.lang.String proxyPackageName, int callingUid, java.lang.String callingPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(code);
          _data.writeInt(proxyUid);
          _data.writeString(proxyPackageName);
          _data.writeInt(callingUid);
          _data.writeString(callingPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_noteProxyOperation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().noteProxyOperation(code, proxyUid, proxyPackageName, callingUid, callingPackageName);
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
      // Remaining methods are only used in Java.

      @Override public int checkPackage(int uid, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_checkPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().checkPackage(uid, packageName);
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
      @Override public java.util.List<android.app.AppOpsManager.PackageOps> getPackagesForOps(int[] ops) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.app.AppOpsManager.PackageOps> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeIntArray(ops);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPackagesForOps, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPackagesForOps(ops);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.app.AppOpsManager.PackageOps.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public java.util.List<android.app.AppOpsManager.PackageOps> getOpsForPackage(int uid, java.lang.String packageName, int[] ops) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.app.AppOpsManager.PackageOps> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeString(packageName);
          _data.writeIntArray(ops);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getOpsForPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getOpsForPackage(uid, packageName, ops);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.app.AppOpsManager.PackageOps.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void getHistoricalOps(int uid, java.lang.String packageName, java.util.List<java.lang.String> ops, long beginTimeMillis, long endTimeMillis, int flags, android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeString(packageName);
          _data.writeStringList(ops);
          _data.writeLong(beginTimeMillis);
          _data.writeLong(endTimeMillis);
          _data.writeInt(flags);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getHistoricalOps, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getHistoricalOps(uid, packageName, ops, beginTimeMillis, endTimeMillis, flags, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void getHistoricalOpsFromDiskRaw(int uid, java.lang.String packageName, java.util.List<java.lang.String> ops, long beginTimeMillis, long endTimeMillis, int flags, android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeString(packageName);
          _data.writeStringList(ops);
          _data.writeLong(beginTimeMillis);
          _data.writeLong(endTimeMillis);
          _data.writeInt(flags);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getHistoricalOpsFromDiskRaw, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getHistoricalOpsFromDiskRaw(uid, packageName, ops, beginTimeMillis, endTimeMillis, flags, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void offsetHistory(long duration) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(duration);
          boolean _status = mRemote.transact(Stub.TRANSACTION_offsetHistory, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().offsetHistory(duration);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setHistoryParameters(int mode, long baseSnapshotInterval, int compressionStep) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(mode);
          _data.writeLong(baseSnapshotInterval);
          _data.writeInt(compressionStep);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setHistoryParameters, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setHistoryParameters(mode, baseSnapshotInterval, compressionStep);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void addHistoricalOps(android.app.AppOpsManager.HistoricalOps ops) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((ops!=null)) {
            _data.writeInt(1);
            ops.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_addHistoricalOps, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addHistoricalOps(ops);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void resetHistoryParameters() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_resetHistoryParameters, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().resetHistoryParameters();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void clearHistory() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearHistory, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearHistory();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public java.util.List<android.app.AppOpsManager.PackageOps> getUidOps(int uid, int[] ops) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.app.AppOpsManager.PackageOps> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(uid);
          _data.writeIntArray(ops);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getUidOps, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getUidOps(uid, ops);
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.app.AppOpsManager.PackageOps.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public void setUidMode(int code, int uid, int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(code);
          _data.writeInt(uid);
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setUidMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setUidMode(code, uid, mode);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setMode(int code, int uid, java.lang.String packageName, int mode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(code);
          _data.writeInt(uid);
          _data.writeString(packageName);
          _data.writeInt(mode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setMode(code, uid, packageName, mode);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void resetAllModes(int reqUserId, java.lang.String reqPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(reqUserId);
          _data.writeString(reqPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_resetAllModes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().resetAllModes(reqUserId, reqPackageName);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setAudioRestriction(int code, int usage, int uid, int mode, java.lang.String[] exceptionPackages) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(code);
          _data.writeInt(usage);
          _data.writeInt(uid);
          _data.writeInt(mode);
          _data.writeStringArray(exceptionPackages);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAudioRestriction, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAudioRestriction(code, usage, uid, mode, exceptionPackages);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setUserRestrictions(android.os.Bundle restrictions, android.os.IBinder token, int userHandle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((restrictions!=null)) {
            _data.writeInt(1);
            restrictions.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder(token);
          _data.writeInt(userHandle);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setUserRestrictions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setUserRestrictions(restrictions, token, userHandle);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void setUserRestriction(int code, boolean restricted, android.os.IBinder token, int userHandle, java.lang.String[] exceptionPackages) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(code);
          _data.writeInt(((restricted)?(1):(0)));
          _data.writeStrongBinder(token);
          _data.writeInt(userHandle);
          _data.writeStringArray(exceptionPackages);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setUserRestriction, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setUserRestriction(code, restricted, token, userHandle, exceptionPackages);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeUser(int userHandle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(userHandle);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeUser(userHandle);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void startWatchingActive(int[] ops, com.android.internal.app.IAppOpsActiveCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeIntArray(ops);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startWatchingActive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startWatchingActive(ops, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stopWatchingActive(com.android.internal.app.IAppOpsActiveCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopWatchingActive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopWatchingActive(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isOperationActive(int code, int uid, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(code);
          _data.writeInt(uid);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isOperationActive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isOperationActive(code, uid, packageName);
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
      @Override public void startWatchingModeWithFlags(int op, java.lang.String packageName, int flags, com.android.internal.app.IAppOpsCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(op);
          _data.writeString(packageName);
          _data.writeInt(flags);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startWatchingModeWithFlags, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startWatchingModeWithFlags(op, packageName, flags, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void startWatchingNoted(int[] ops, com.android.internal.app.IAppOpsNotedCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeIntArray(ops);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startWatchingNoted, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startWatchingNoted(ops, callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void stopWatchingNoted(com.android.internal.app.IAppOpsNotedCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopWatchingNoted, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopWatchingNoted(callback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int checkOperationRaw(int code, int uid, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(code);
          _data.writeInt(uid);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_checkOperationRaw, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().checkOperationRaw(code, uid, packageName);
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
      @Override public void reloadNonHistoricalState() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reloadNonHistoricalState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reloadNonHistoricalState();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.internal.app.IAppOpsService sDefaultImpl;
    }
    static final int TRANSACTION_checkOperation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_noteOperation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_startOperation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_finishOperation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_startWatchingMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_stopWatchingMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_getToken = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_permissionToOpCode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_checkAudioOperation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_noteProxyOperation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_checkPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_getPackagesForOps = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_getOpsForPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_getHistoricalOps = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_getHistoricalOpsFromDiskRaw = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_offsetHistory = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_setHistoryParameters = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_addHistoricalOps = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_resetHistoryParameters = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_clearHistory = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_getUidOps = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_setUidMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_setMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_resetAllModes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_setAudioRestriction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_setUserRestrictions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_setUserRestriction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    static final int TRANSACTION_removeUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 27);
    static final int TRANSACTION_startWatchingActive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 28);
    static final int TRANSACTION_stopWatchingActive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 29);
    static final int TRANSACTION_isOperationActive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 30);
    static final int TRANSACTION_startWatchingModeWithFlags = (android.os.IBinder.FIRST_CALL_TRANSACTION + 31);
    static final int TRANSACTION_startWatchingNoted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 32);
    static final int TRANSACTION_stopWatchingNoted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 33);
    static final int TRANSACTION_checkOperationRaw = (android.os.IBinder.FIRST_CALL_TRANSACTION + 34);
    static final int TRANSACTION_reloadNonHistoricalState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 35);
    public static boolean setDefaultImpl(com.android.internal.app.IAppOpsService impl) {
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
    public static com.android.internal.app.IAppOpsService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // These methods are also called by native code, so must
  // be kept in sync with frameworks/native/libs/binder/include/binder/IAppOpsService.h
  // and not be reordered

  public int checkOperation(int code, int uid, java.lang.String packageName) throws android.os.RemoteException;
  public int noteOperation(int code, int uid, java.lang.String packageName) throws android.os.RemoteException;
  public int startOperation(android.os.IBinder token, int code, int uid, java.lang.String packageName, boolean startIfModeDefault) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/app/IAppOpsService.aidl:36:1:36:25")
  public void finishOperation(android.os.IBinder token, int code, int uid, java.lang.String packageName) throws android.os.RemoteException;
  public void startWatchingMode(int op, java.lang.String packageName, com.android.internal.app.IAppOpsCallback callback) throws android.os.RemoteException;
  public void stopWatchingMode(com.android.internal.app.IAppOpsCallback callback) throws android.os.RemoteException;
  public android.os.IBinder getToken(android.os.IBinder clientToken) throws android.os.RemoteException;
  public int permissionToOpCode(java.lang.String permission) throws android.os.RemoteException;
  public int checkAudioOperation(int code, int usage, int uid, java.lang.String packageName) throws android.os.RemoteException;
  // End of methods also called by native code.
  // Any new method exposed to native must be added after the last one, do not reorder

  public int noteProxyOperation(int code, int proxyUid, java.lang.String proxyPackageName, int callingUid, java.lang.String callingPackageName) throws android.os.RemoteException;
  // Remaining methods are only used in Java.

  public int checkPackage(int uid, java.lang.String packageName) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/app/IAppOpsService.aidl:51:1:51:25")
  public java.util.List<android.app.AppOpsManager.PackageOps> getPackagesForOps(int[] ops) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/app/IAppOpsService.aidl:53:1:53:25")
  public java.util.List<android.app.AppOpsManager.PackageOps> getOpsForPackage(int uid, java.lang.String packageName, int[] ops) throws android.os.RemoteException;
  public void getHistoricalOps(int uid, java.lang.String packageName, java.util.List<java.lang.String> ops, long beginTimeMillis, long endTimeMillis, int flags, android.os.RemoteCallback callback) throws android.os.RemoteException;
  public void getHistoricalOpsFromDiskRaw(int uid, java.lang.String packageName, java.util.List<java.lang.String> ops, long beginTimeMillis, long endTimeMillis, int flags, android.os.RemoteCallback callback) throws android.os.RemoteException;
  public void offsetHistory(long duration) throws android.os.RemoteException;
  public void setHistoryParameters(int mode, long baseSnapshotInterval, int compressionStep) throws android.os.RemoteException;
  public void addHistoricalOps(android.app.AppOpsManager.HistoricalOps ops) throws android.os.RemoteException;
  public void resetHistoryParameters() throws android.os.RemoteException;
  public void clearHistory() throws android.os.RemoteException;
  public java.util.List<android.app.AppOpsManager.PackageOps> getUidOps(int uid, int[] ops) throws android.os.RemoteException;
  public void setUidMode(int code, int uid, int mode) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/app/IAppOpsService.aidl:66:1:66:25")
  public void setMode(int code, int uid, java.lang.String packageName, int mode) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/com/android/internal/app/IAppOpsService.aidl:68:1:68:25")
  public void resetAllModes(int reqUserId, java.lang.String reqPackageName) throws android.os.RemoteException;
  public void setAudioRestriction(int code, int usage, int uid, int mode, java.lang.String[] exceptionPackages) throws android.os.RemoteException;
  public void setUserRestrictions(android.os.Bundle restrictions, android.os.IBinder token, int userHandle) throws android.os.RemoteException;
  public void setUserRestriction(int code, boolean restricted, android.os.IBinder token, int userHandle, java.lang.String[] exceptionPackages) throws android.os.RemoteException;
  public void removeUser(int userHandle) throws android.os.RemoteException;
  public void startWatchingActive(int[] ops, com.android.internal.app.IAppOpsActiveCallback callback) throws android.os.RemoteException;
  public void stopWatchingActive(com.android.internal.app.IAppOpsActiveCallback callback) throws android.os.RemoteException;
  public boolean isOperationActive(int code, int uid, java.lang.String packageName) throws android.os.RemoteException;
  public void startWatchingModeWithFlags(int op, java.lang.String packageName, int flags, com.android.internal.app.IAppOpsCallback callback) throws android.os.RemoteException;
  public void startWatchingNoted(int[] ops, com.android.internal.app.IAppOpsNotedCallback callback) throws android.os.RemoteException;
  public void stopWatchingNoted(com.android.internal.app.IAppOpsNotedCallback callback) throws android.os.RemoteException;
  public int checkOperationRaw(int code, int uid, java.lang.String packageName) throws android.os.RemoteException;
  public void reloadNonHistoricalState() throws android.os.RemoteException;
}
