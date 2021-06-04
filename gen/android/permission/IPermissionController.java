/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.permission;
/**
 * Interface for system apps to communication with the permission controller.
 *
 * @hide
 */
public interface IPermissionController extends android.os.IInterface
{
  /** Default implementation for IPermissionController. */
  public static class Default implements android.permission.IPermissionController
  {
    @Override public void revokeRuntimePermissions(android.os.Bundle request, boolean doDryRun, int reason, java.lang.String callerPackageName, android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void getRuntimePermissionBackup(android.os.UserHandle user, android.os.ParcelFileDescriptor pipe) throws android.os.RemoteException
    {
    }
    @Override public void restoreRuntimePermissionBackup(android.os.UserHandle user, android.os.ParcelFileDescriptor pipe) throws android.os.RemoteException
    {
    }
    @Override public void restoreDelayedRuntimePermissionBackup(java.lang.String packageName, android.os.UserHandle user, android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void getAppPermissions(java.lang.String packageName, android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void revokeRuntimePermission(java.lang.String packageName, java.lang.String permissionName) throws android.os.RemoteException
    {
    }
    @Override public void countPermissionApps(java.util.List<java.lang.String> permissionNames, int flags, android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void getPermissionUsages(boolean countSystem, long numMillis, android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void setRuntimePermissionGrantStateByDeviceAdmin(java.lang.String callerPackageName, java.lang.String packageName, java.lang.String permission, int grantState, android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override public void grantOrUpgradeDefaultRuntimePermissions(android.os.RemoteCallback callback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.permission.IPermissionController
  {
    private static final java.lang.String DESCRIPTOR = "android.permission.IPermissionController";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.permission.IPermissionController interface,
     * generating a proxy if needed.
     */
    public static android.permission.IPermissionController asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.permission.IPermissionController))) {
        return ((android.permission.IPermissionController)iin);
      }
      return new android.permission.IPermissionController.Stub.Proxy(obj);
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
        case TRANSACTION_revokeRuntimePermissions:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          android.os.RemoteCallback _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          this.revokeRuntimePermissions(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_getRuntimePermissionBackup:
        {
          data.enforceInterface(descriptor);
          android.os.UserHandle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.UserHandle.CREATOR.createFromParcel(data);
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
          this.getRuntimePermissionBackup(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_restoreRuntimePermissionBackup:
        {
          data.enforceInterface(descriptor);
          android.os.UserHandle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.UserHandle.CREATOR.createFromParcel(data);
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
          this.restoreRuntimePermissionBackup(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_restoreDelayedRuntimePermissionBackup:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.UserHandle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.UserHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.os.RemoteCallback _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.restoreDelayedRuntimePermissionBackup(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_getAppPermissions:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.os.RemoteCallback _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.getAppPermissions(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_revokeRuntimePermission:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.revokeRuntimePermission(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_countPermissionApps:
        {
          data.enforceInterface(descriptor);
          java.util.List<java.lang.String> _arg0;
          _arg0 = data.createStringArrayList();
          int _arg1;
          _arg1 = data.readInt();
          android.os.RemoteCallback _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.countPermissionApps(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_getPermissionUsages:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          long _arg1;
          _arg1 = data.readLong();
          android.os.RemoteCallback _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.getPermissionUsages(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_setRuntimePermissionGrantStateByDeviceAdmin:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          android.os.RemoteCallback _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          this.setRuntimePermissionGrantStateByDeviceAdmin(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_grantOrUpgradeDefaultRuntimePermissions:
        {
          data.enforceInterface(descriptor);
          android.os.RemoteCallback _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.grantOrUpgradeDefaultRuntimePermissions(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.permission.IPermissionController
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
      @Override public void revokeRuntimePermissions(android.os.Bundle request, boolean doDryRun, int reason, java.lang.String callerPackageName, android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((request!=null)) {
            _data.writeInt(1);
            request.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((doDryRun)?(1):(0)));
          _data.writeInt(reason);
          _data.writeString(callerPackageName);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_revokeRuntimePermissions, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().revokeRuntimePermissions(request, doDryRun, reason, callerPackageName, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getRuntimePermissionBackup(android.os.UserHandle user, android.os.ParcelFileDescriptor pipe) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((pipe!=null)) {
            _data.writeInt(1);
            pipe.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRuntimePermissionBackup, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getRuntimePermissionBackup(user, pipe);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void restoreRuntimePermissionBackup(android.os.UserHandle user, android.os.ParcelFileDescriptor pipe) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((pipe!=null)) {
            _data.writeInt(1);
            pipe.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_restoreRuntimePermissionBackup, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().restoreRuntimePermissionBackup(user, pipe);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void restoreDelayedRuntimePermissionBackup(java.lang.String packageName, android.os.UserHandle user, android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          if ((user!=null)) {
            _data.writeInt(1);
            user.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_restoreDelayedRuntimePermissionBackup, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().restoreDelayedRuntimePermissionBackup(packageName, user, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getAppPermissions(java.lang.String packageName, android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAppPermissions, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getAppPermissions(packageName, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void revokeRuntimePermission(java.lang.String packageName, java.lang.String permissionName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeString(permissionName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_revokeRuntimePermission, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().revokeRuntimePermission(packageName, permissionName);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void countPermissionApps(java.util.List<java.lang.String> permissionNames, int flags, android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStringList(permissionNames);
          _data.writeInt(flags);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_countPermissionApps, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().countPermissionApps(permissionNames, flags, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void getPermissionUsages(boolean countSystem, long numMillis, android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((countSystem)?(1):(0)));
          _data.writeLong(numMillis);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPermissionUsages, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getPermissionUsages(countSystem, numMillis, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void setRuntimePermissionGrantStateByDeviceAdmin(java.lang.String callerPackageName, java.lang.String packageName, java.lang.String permission, int grantState, android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callerPackageName);
          _data.writeString(packageName);
          _data.writeString(permission);
          _data.writeInt(grantState);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setRuntimePermissionGrantStateByDeviceAdmin, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setRuntimePermissionGrantStateByDeviceAdmin(callerPackageName, packageName, permission, grantState, callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void grantOrUpgradeDefaultRuntimePermissions(android.os.RemoteCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_grantOrUpgradeDefaultRuntimePermissions, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().grantOrUpgradeDefaultRuntimePermissions(callback);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.permission.IPermissionController sDefaultImpl;
    }
    static final int TRANSACTION_revokeRuntimePermissions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getRuntimePermissionBackup = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_restoreRuntimePermissionBackup = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_restoreDelayedRuntimePermissionBackup = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getAppPermissions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_revokeRuntimePermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_countPermissionApps = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_getPermissionUsages = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_setRuntimePermissionGrantStateByDeviceAdmin = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_grantOrUpgradeDefaultRuntimePermissions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    public static boolean setDefaultImpl(android.permission.IPermissionController impl) {
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
    public static android.permission.IPermissionController getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void revokeRuntimePermissions(android.os.Bundle request, boolean doDryRun, int reason, java.lang.String callerPackageName, android.os.RemoteCallback callback) throws android.os.RemoteException;
  public void getRuntimePermissionBackup(android.os.UserHandle user, android.os.ParcelFileDescriptor pipe) throws android.os.RemoteException;
  public void restoreRuntimePermissionBackup(android.os.UserHandle user, android.os.ParcelFileDescriptor pipe) throws android.os.RemoteException;
  public void restoreDelayedRuntimePermissionBackup(java.lang.String packageName, android.os.UserHandle user, android.os.RemoteCallback callback) throws android.os.RemoteException;
  public void getAppPermissions(java.lang.String packageName, android.os.RemoteCallback callback) throws android.os.RemoteException;
  public void revokeRuntimePermission(java.lang.String packageName, java.lang.String permissionName) throws android.os.RemoteException;
  public void countPermissionApps(java.util.List<java.lang.String> permissionNames, int flags, android.os.RemoteCallback callback) throws android.os.RemoteException;
  public void getPermissionUsages(boolean countSystem, long numMillis, android.os.RemoteCallback callback) throws android.os.RemoteException;
  public void setRuntimePermissionGrantStateByDeviceAdmin(java.lang.String callerPackageName, java.lang.String packageName, java.lang.String permission, int grantState, android.os.RemoteCallback callback) throws android.os.RemoteException;
  public void grantOrUpgradeDefaultRuntimePermissions(android.os.RemoteCallback callback) throws android.os.RemoteException;
}
