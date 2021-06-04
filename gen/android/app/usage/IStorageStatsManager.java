/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app.usage;
/** {@hide} */
public interface IStorageStatsManager extends android.os.IInterface
{
  /** Default implementation for IStorageStatsManager. */
  public static class Default implements android.app.usage.IStorageStatsManager
  {
    @Override public boolean isQuotaSupported(java.lang.String volumeUuid, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean isReservedSupported(java.lang.String volumeUuid, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return false;
    }
    @Override public long getTotalBytes(java.lang.String volumeUuid, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0L;
    }
    @Override public long getFreeBytes(java.lang.String volumeUuid, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0L;
    }
    @Override public long getCacheBytes(java.lang.String volumeUuid, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0L;
    }
    @Override public long getCacheQuotaBytes(java.lang.String volumeUuid, int uid, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return 0L;
    }
    @Override public android.app.usage.StorageStats queryStatsForPackage(java.lang.String volumeUuid, java.lang.String packageName, int userId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.app.usage.StorageStats queryStatsForUid(java.lang.String volumeUuid, int uid, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.app.usage.StorageStats queryStatsForUser(java.lang.String volumeUuid, int userId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.app.usage.ExternalStorageStats queryExternalStatsForUser(java.lang.String volumeUuid, int userId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.usage.IStorageStatsManager
  {
    private static final java.lang.String DESCRIPTOR = "android.app.usage.IStorageStatsManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.usage.IStorageStatsManager interface,
     * generating a proxy if needed.
     */
    public static android.app.usage.IStorageStatsManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.usage.IStorageStatsManager))) {
        return ((android.app.usage.IStorageStatsManager)iin);
      }
      return new android.app.usage.IStorageStatsManager.Stub.Proxy(obj);
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
        case TRANSACTION_isQuotaSupported:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.isQuotaSupported(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_isReservedSupported:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.isReservedSupported(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_getTotalBytes:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          long _result = this.getTotalBytes(_arg0, _arg1);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_getFreeBytes:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          long _result = this.getFreeBytes(_arg0, _arg1);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_getCacheBytes:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          long _result = this.getCacheBytes(_arg0, _arg1);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_getCacheQuotaBytes:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          long _result = this.getCacheQuotaBytes(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_queryStatsForPackage:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          android.app.usage.StorageStats _result = this.queryStatsForPackage(_arg0, _arg1, _arg2, _arg3);
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
        case TRANSACTION_queryStatsForUid:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.app.usage.StorageStats _result = this.queryStatsForUid(_arg0, _arg1, _arg2);
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
        case TRANSACTION_queryStatsForUser:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.app.usage.StorageStats _result = this.queryStatsForUser(_arg0, _arg1, _arg2);
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
        case TRANSACTION_queryExternalStatsForUser:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.app.usage.ExternalStorageStats _result = this.queryExternalStatsForUser(_arg0, _arg1, _arg2);
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
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.usage.IStorageStatsManager
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
      @Override public boolean isQuotaSupported(java.lang.String volumeUuid, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(volumeUuid);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isQuotaSupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isQuotaSupported(volumeUuid, callingPackage);
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
      @Override public boolean isReservedSupported(java.lang.String volumeUuid, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(volumeUuid);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isReservedSupported, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isReservedSupported(volumeUuid, callingPackage);
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
      @Override public long getTotalBytes(java.lang.String volumeUuid, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(volumeUuid);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getTotalBytes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getTotalBytes(volumeUuid, callingPackage);
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public long getFreeBytes(java.lang.String volumeUuid, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(volumeUuid);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getFreeBytes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getFreeBytes(volumeUuid, callingPackage);
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public long getCacheBytes(java.lang.String volumeUuid, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(volumeUuid);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCacheBytes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCacheBytes(volumeUuid, callingPackage);
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public long getCacheQuotaBytes(java.lang.String volumeUuid, int uid, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(volumeUuid);
          _data.writeInt(uid);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCacheQuotaBytes, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCacheQuotaBytes(volumeUuid, uid, callingPackage);
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public android.app.usage.StorageStats queryStatsForPackage(java.lang.String volumeUuid, java.lang.String packageName, int userId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.usage.StorageStats _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(volumeUuid);
          _data.writeString(packageName);
          _data.writeInt(userId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryStatsForPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().queryStatsForPackage(volumeUuid, packageName, userId, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.app.usage.StorageStats.CREATOR.createFromParcel(_reply);
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
      @Override public android.app.usage.StorageStats queryStatsForUid(java.lang.String volumeUuid, int uid, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.usage.StorageStats _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(volumeUuid);
          _data.writeInt(uid);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryStatsForUid, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().queryStatsForUid(volumeUuid, uid, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.app.usage.StorageStats.CREATOR.createFromParcel(_reply);
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
      @Override public android.app.usage.StorageStats queryStatsForUser(java.lang.String volumeUuid, int userId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.usage.StorageStats _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(volumeUuid);
          _data.writeInt(userId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryStatsForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().queryStatsForUser(volumeUuid, userId, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.app.usage.StorageStats.CREATOR.createFromParcel(_reply);
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
      @Override public android.app.usage.ExternalStorageStats queryExternalStatsForUser(java.lang.String volumeUuid, int userId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.usage.ExternalStorageStats _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(volumeUuid);
          _data.writeInt(userId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryExternalStatsForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().queryExternalStatsForUser(volumeUuid, userId, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.app.usage.ExternalStorageStats.CREATOR.createFromParcel(_reply);
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
      public static android.app.usage.IStorageStatsManager sDefaultImpl;
    }
    static final int TRANSACTION_isQuotaSupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_isReservedSupported = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getTotalBytes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getFreeBytes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getCacheBytes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getCacheQuotaBytes = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_queryStatsForPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_queryStatsForUid = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_queryStatsForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_queryExternalStatsForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    public static boolean setDefaultImpl(android.app.usage.IStorageStatsManager impl) {
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
    public static android.app.usage.IStorageStatsManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public boolean isQuotaSupported(java.lang.String volumeUuid, java.lang.String callingPackage) throws android.os.RemoteException;
  public boolean isReservedSupported(java.lang.String volumeUuid, java.lang.String callingPackage) throws android.os.RemoteException;
  public long getTotalBytes(java.lang.String volumeUuid, java.lang.String callingPackage) throws android.os.RemoteException;
  public long getFreeBytes(java.lang.String volumeUuid, java.lang.String callingPackage) throws android.os.RemoteException;
  public long getCacheBytes(java.lang.String volumeUuid, java.lang.String callingPackage) throws android.os.RemoteException;
  public long getCacheQuotaBytes(java.lang.String volumeUuid, int uid, java.lang.String callingPackage) throws android.os.RemoteException;
  public android.app.usage.StorageStats queryStatsForPackage(java.lang.String volumeUuid, java.lang.String packageName, int userId, java.lang.String callingPackage) throws android.os.RemoteException;
  public android.app.usage.StorageStats queryStatsForUid(java.lang.String volumeUuid, int uid, java.lang.String callingPackage) throws android.os.RemoteException;
  public android.app.usage.StorageStats queryStatsForUser(java.lang.String volumeUuid, int userId, java.lang.String callingPackage) throws android.os.RemoteException;
  public android.app.usage.ExternalStorageStats queryExternalStatsForUser(java.lang.String volumeUuid, int userId, java.lang.String callingPackage) throws android.os.RemoteException;
}
