/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app.usage;
/**
 * System private API for talking with the UsageStatsManagerService.
 *
 * {@hide}
 */
public interface IUsageStatsManager extends android.os.IInterface
{
  /** Default implementation for IUsageStatsManager. */
  public static class Default implements android.app.usage.IUsageStatsManager
  {
    @Override public android.content.pm.ParceledListSlice queryUsageStats(int bucketType, long beginTime, long endTime, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.content.pm.ParceledListSlice queryConfigurationStats(int bucketType, long beginTime, long endTime, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.content.pm.ParceledListSlice queryEventStats(int bucketType, long beginTime, long endTime, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.app.usage.UsageEvents queryEvents(long beginTime, long endTime, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.app.usage.UsageEvents queryEventsForPackage(long beginTime, long endTime, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.app.usage.UsageEvents queryEventsForUser(long beginTime, long endTime, int userId, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.app.usage.UsageEvents queryEventsForPackageForUser(long beginTime, long endTime, int userId, java.lang.String pkg, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setAppInactive(java.lang.String packageName, boolean inactive, int userId) throws android.os.RemoteException
    {
    }
    @Override public boolean isAppInactive(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
      return false;
    }
    @Override public void whitelistAppTemporarily(java.lang.String packageName, long duration, int userId) throws android.os.RemoteException
    {
    }
    @Override public void onCarrierPrivilegedAppsChanged() throws android.os.RemoteException
    {
    }
    @Override public void reportChooserSelection(java.lang.String packageName, int userId, java.lang.String contentType, java.lang.String[] annotations, java.lang.String action) throws android.os.RemoteException
    {
    }
    @Override public int getAppStandbyBucket(java.lang.String packageName, java.lang.String callingPackage, int userId) throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void setAppStandbyBucket(java.lang.String packageName, int bucket, int userId) throws android.os.RemoteException
    {
    }
    @Override public android.content.pm.ParceledListSlice getAppStandbyBuckets(java.lang.String callingPackage, int userId) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void setAppStandbyBuckets(android.content.pm.ParceledListSlice appBuckets, int userId) throws android.os.RemoteException
    {
    }
    @Override public void registerAppUsageObserver(int observerId, java.lang.String[] packages, long timeLimitMs, android.app.PendingIntent callback, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public void unregisterAppUsageObserver(int observerId, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public void registerUsageSessionObserver(int sessionObserverId, java.lang.String[] observed, long timeLimitMs, long sessionThresholdTimeMs, android.app.PendingIntent limitReachedCallbackIntent, android.app.PendingIntent sessionEndCallbackIntent, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public void unregisterUsageSessionObserver(int sessionObserverId, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public void registerAppUsageLimitObserver(int observerId, java.lang.String[] packages, long timeLimitMs, long timeUsedMs, android.app.PendingIntent callback, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public void unregisterAppUsageLimitObserver(int observerId, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public void reportUsageStart(android.os.IBinder activity, java.lang.String token, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public void reportPastUsageStart(android.os.IBinder activity, java.lang.String token, long timeAgoMs, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public void reportUsageStop(android.os.IBinder activity, java.lang.String token, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public int getUsageSource() throws android.os.RemoteException
    {
      return 0;
    }
    @Override public void forceUsageSourceSettingRead() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.usage.IUsageStatsManager
  {
    private static final java.lang.String DESCRIPTOR = "android.app.usage.IUsageStatsManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.usage.IUsageStatsManager interface,
     * generating a proxy if needed.
     */
    public static android.app.usage.IUsageStatsManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.usage.IUsageStatsManager))) {
        return ((android.app.usage.IUsageStatsManager)iin);
      }
      return new android.app.usage.IUsageStatsManager.Stub.Proxy(obj);
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
        case TRANSACTION_queryUsageStats:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          long _arg2;
          _arg2 = data.readLong();
          java.lang.String _arg3;
          _arg3 = data.readString();
          android.content.pm.ParceledListSlice _result = this.queryUsageStats(_arg0, _arg1, _arg2, _arg3);
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
        case TRANSACTION_queryConfigurationStats:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          long _arg2;
          _arg2 = data.readLong();
          java.lang.String _arg3;
          _arg3 = data.readString();
          android.content.pm.ParceledListSlice _result = this.queryConfigurationStats(_arg0, _arg1, _arg2, _arg3);
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
        case TRANSACTION_queryEventStats:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          long _arg1;
          _arg1 = data.readLong();
          long _arg2;
          _arg2 = data.readLong();
          java.lang.String _arg3;
          _arg3 = data.readString();
          android.content.pm.ParceledListSlice _result = this.queryEventStats(_arg0, _arg1, _arg2, _arg3);
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
        case TRANSACTION_queryEvents:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          long _arg1;
          _arg1 = data.readLong();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.app.usage.UsageEvents _result = this.queryEvents(_arg0, _arg1, _arg2);
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
        case TRANSACTION_queryEventsForPackage:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          long _arg1;
          _arg1 = data.readLong();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.app.usage.UsageEvents _result = this.queryEventsForPackage(_arg0, _arg1, _arg2);
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
        case TRANSACTION_queryEventsForUser:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          long _arg1;
          _arg1 = data.readLong();
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          android.app.usage.UsageEvents _result = this.queryEventsForUser(_arg0, _arg1, _arg2, _arg3);
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
        case TRANSACTION_queryEventsForPackageForUser:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          long _arg1;
          _arg1 = data.readLong();
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          java.lang.String _arg4;
          _arg4 = data.readString();
          android.app.usage.UsageEvents _result = this.queryEventsForPackageForUser(_arg0, _arg1, _arg2, _arg3, _arg4);
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
        case TRANSACTION_setAppInactive:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          int _arg2;
          _arg2 = data.readInt();
          this.setAppInactive(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isAppInactive:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.isAppInactive(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_whitelistAppTemporarily:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          long _arg1;
          _arg1 = data.readLong();
          int _arg2;
          _arg2 = data.readInt();
          this.whitelistAppTemporarily(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_onCarrierPrivilegedAppsChanged:
        {
          data.enforceInterface(descriptor);
          this.onCarrierPrivilegedAppsChanged();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_reportChooserSelection:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          java.lang.String[] _arg3;
          _arg3 = data.createStringArray();
          java.lang.String _arg4;
          _arg4 = data.readString();
          this.reportChooserSelection(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getAppStandbyBucket:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          int _result = this.getAppStandbyBucket(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setAppStandbyBucket:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.setAppStandbyBucket(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getAppStandbyBuckets:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.content.pm.ParceledListSlice _result = this.getAppStandbyBuckets(_arg0, _arg1);
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
        case TRANSACTION_setAppStandbyBuckets:
        {
          data.enforceInterface(descriptor);
          android.content.pm.ParceledListSlice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.pm.ParceledListSlice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.setAppStandbyBuckets(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerAppUsageObserver:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String[] _arg1;
          _arg1 = data.createStringArray();
          long _arg2;
          _arg2 = data.readLong();
          android.app.PendingIntent _arg3;
          if ((0!=data.readInt())) {
            _arg3 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg3 = null;
          }
          java.lang.String _arg4;
          _arg4 = data.readString();
          this.registerAppUsageObserver(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterAppUsageObserver:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.unregisterAppUsageObserver(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerUsageSessionObserver:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String[] _arg1;
          _arg1 = data.createStringArray();
          long _arg2;
          _arg2 = data.readLong();
          long _arg3;
          _arg3 = data.readLong();
          android.app.PendingIntent _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          android.app.PendingIntent _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          java.lang.String _arg6;
          _arg6 = data.readString();
          this.registerUsageSessionObserver(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterUsageSessionObserver:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.unregisterUsageSessionObserver(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerAppUsageLimitObserver:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String[] _arg1;
          _arg1 = data.createStringArray();
          long _arg2;
          _arg2 = data.readLong();
          long _arg3;
          _arg3 = data.readLong();
          android.app.PendingIntent _arg4;
          if ((0!=data.readInt())) {
            _arg4 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg4 = null;
          }
          java.lang.String _arg5;
          _arg5 = data.readString();
          this.registerAppUsageLimitObserver(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterAppUsageLimitObserver:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.unregisterAppUsageLimitObserver(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_reportUsageStart:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.reportUsageStart(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_reportPastUsageStart:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _arg1;
          _arg1 = data.readString();
          long _arg2;
          _arg2 = data.readLong();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.reportPastUsageStart(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_reportUsageStop:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _arg1;
          _arg1 = data.readString();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.reportUsageStop(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getUsageSource:
        {
          data.enforceInterface(descriptor);
          int _result = this.getUsageSource();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_forceUsageSourceSettingRead:
        {
          data.enforceInterface(descriptor);
          this.forceUsageSourceSettingRead();
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.usage.IUsageStatsManager
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
      @Override public android.content.pm.ParceledListSlice queryUsageStats(int bucketType, long beginTime, long endTime, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(bucketType);
          _data.writeLong(beginTime);
          _data.writeLong(endTime);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryUsageStats, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().queryUsageStats(bucketType, beginTime, endTime, callingPackage);
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
      @Override public android.content.pm.ParceledListSlice queryConfigurationStats(int bucketType, long beginTime, long endTime, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(bucketType);
          _data.writeLong(beginTime);
          _data.writeLong(endTime);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryConfigurationStats, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().queryConfigurationStats(bucketType, beginTime, endTime, callingPackage);
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
      @Override public android.content.pm.ParceledListSlice queryEventStats(int bucketType, long beginTime, long endTime, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(bucketType);
          _data.writeLong(beginTime);
          _data.writeLong(endTime);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryEventStats, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().queryEventStats(bucketType, beginTime, endTime, callingPackage);
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
      @Override public android.app.usage.UsageEvents queryEvents(long beginTime, long endTime, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.usage.UsageEvents _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(beginTime);
          _data.writeLong(endTime);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryEvents, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().queryEvents(beginTime, endTime, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.app.usage.UsageEvents.CREATOR.createFromParcel(_reply);
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
      @Override public android.app.usage.UsageEvents queryEventsForPackage(long beginTime, long endTime, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.usage.UsageEvents _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(beginTime);
          _data.writeLong(endTime);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryEventsForPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().queryEventsForPackage(beginTime, endTime, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.app.usage.UsageEvents.CREATOR.createFromParcel(_reply);
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
      @Override public android.app.usage.UsageEvents queryEventsForUser(long beginTime, long endTime, int userId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.usage.UsageEvents _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(beginTime);
          _data.writeLong(endTime);
          _data.writeInt(userId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryEventsForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().queryEventsForUser(beginTime, endTime, userId, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.app.usage.UsageEvents.CREATOR.createFromParcel(_reply);
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
      @Override public android.app.usage.UsageEvents queryEventsForPackageForUser(long beginTime, long endTime, int userId, java.lang.String pkg, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.app.usage.UsageEvents _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(beginTime);
          _data.writeLong(endTime);
          _data.writeInt(userId);
          _data.writeString(pkg);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryEventsForPackageForUser, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().queryEventsForPackageForUser(beginTime, endTime, userId, pkg, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.app.usage.UsageEvents.CREATOR.createFromParcel(_reply);
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
      @Override public void setAppInactive(java.lang.String packageName, boolean inactive, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(((inactive)?(1):(0)));
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAppInactive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAppInactive(packageName, inactive, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public boolean isAppInactive(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isAppInactive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isAppInactive(packageName, userId);
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
      @Override public void whitelistAppTemporarily(java.lang.String packageName, long duration, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeLong(duration);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_whitelistAppTemporarily, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().whitelistAppTemporarily(packageName, duration, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void onCarrierPrivilegedAppsChanged() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCarrierPrivilegedAppsChanged, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCarrierPrivilegedAppsChanged();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void reportChooserSelection(java.lang.String packageName, int userId, java.lang.String contentType, java.lang.String[] annotations, java.lang.String action) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          _data.writeString(contentType);
          _data.writeStringArray(annotations);
          _data.writeString(action);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportChooserSelection, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportChooserSelection(packageName, userId, contentType, annotations, action);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getAppStandbyBucket(java.lang.String packageName, java.lang.String callingPackage, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeString(callingPackage);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAppStandbyBucket, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAppStandbyBucket(packageName, callingPackage, userId);
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
      @Override public void setAppStandbyBucket(java.lang.String packageName, int bucket, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(bucket);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAppStandbyBucket, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAppStandbyBucket(packageName, bucket, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.content.pm.ParceledListSlice getAppStandbyBuckets(java.lang.String callingPackage, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.pm.ParceledListSlice _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(callingPackage);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAppStandbyBuckets, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAppStandbyBuckets(callingPackage, userId);
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
      @Override public void setAppStandbyBuckets(android.content.pm.ParceledListSlice appBuckets, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((appBuckets!=null)) {
            _data.writeInt(1);
            appBuckets.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAppStandbyBuckets, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAppStandbyBuckets(appBuckets, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void registerAppUsageObserver(int observerId, java.lang.String[] packages, long timeLimitMs, android.app.PendingIntent callback, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(observerId);
          _data.writeStringArray(packages);
          _data.writeLong(timeLimitMs);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerAppUsageObserver, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerAppUsageObserver(observerId, packages, timeLimitMs, callback, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterAppUsageObserver(int observerId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(observerId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterAppUsageObserver, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterAppUsageObserver(observerId, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void registerUsageSessionObserver(int sessionObserverId, java.lang.String[] observed, long timeLimitMs, long sessionThresholdTimeMs, android.app.PendingIntent limitReachedCallbackIntent, android.app.PendingIntent sessionEndCallbackIntent, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sessionObserverId);
          _data.writeStringArray(observed);
          _data.writeLong(timeLimitMs);
          _data.writeLong(sessionThresholdTimeMs);
          if ((limitReachedCallbackIntent!=null)) {
            _data.writeInt(1);
            limitReachedCallbackIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((sessionEndCallbackIntent!=null)) {
            _data.writeInt(1);
            sessionEndCallbackIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerUsageSessionObserver, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerUsageSessionObserver(sessionObserverId, observed, timeLimitMs, sessionThresholdTimeMs, limitReachedCallbackIntent, sessionEndCallbackIntent, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterUsageSessionObserver(int sessionObserverId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sessionObserverId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterUsageSessionObserver, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterUsageSessionObserver(sessionObserverId, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void registerAppUsageLimitObserver(int observerId, java.lang.String[] packages, long timeLimitMs, long timeUsedMs, android.app.PendingIntent callback, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(observerId);
          _data.writeStringArray(packages);
          _data.writeLong(timeLimitMs);
          _data.writeLong(timeUsedMs);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerAppUsageLimitObserver, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerAppUsageLimitObserver(observerId, packages, timeLimitMs, timeUsedMs, callback, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void unregisterAppUsageLimitObserver(int observerId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(observerId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterAppUsageLimitObserver, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterAppUsageLimitObserver(observerId, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void reportUsageStart(android.os.IBinder activity, java.lang.String token, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(activity);
          _data.writeString(token);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportUsageStart, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportUsageStart(activity, token, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void reportPastUsageStart(android.os.IBinder activity, java.lang.String token, long timeAgoMs, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(activity);
          _data.writeString(token);
          _data.writeLong(timeAgoMs);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportPastUsageStart, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportPastUsageStart(activity, token, timeAgoMs, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void reportUsageStop(android.os.IBinder activity, java.lang.String token, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(activity);
          _data.writeString(token);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_reportUsageStop, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().reportUsageStop(activity, token, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public int getUsageSource() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getUsageSource, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getUsageSource();
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
      @Override public void forceUsageSourceSettingRead() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_forceUsageSourceSettingRead, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().forceUsageSourceSettingRead();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.app.usage.IUsageStatsManager sDefaultImpl;
    }
    static final int TRANSACTION_queryUsageStats = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_queryConfigurationStats = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_queryEventStats = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_queryEvents = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_queryEventsForPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_queryEventsForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_queryEventsForPackageForUser = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_setAppInactive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_isAppInactive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_whitelistAppTemporarily = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_onCarrierPrivilegedAppsChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_reportChooserSelection = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_getAppStandbyBucket = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_setAppStandbyBucket = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_getAppStandbyBuckets = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_setAppStandbyBuckets = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_registerAppUsageObserver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_unregisterAppUsageObserver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_registerUsageSessionObserver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_unregisterUsageSessionObserver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_registerAppUsageLimitObserver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_unregisterAppUsageLimitObserver = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_reportUsageStart = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_reportPastUsageStart = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_reportUsageStop = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_getUsageSource = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    static final int TRANSACTION_forceUsageSourceSettingRead = (android.os.IBinder.FIRST_CALL_TRANSACTION + 26);
    public static boolean setDefaultImpl(android.app.usage.IUsageStatsManager impl) {
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
    public static android.app.usage.IUsageStatsManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/usage/IUsageStatsManager.aidl:31:1:31:25")
  public android.content.pm.ParceledListSlice queryUsageStats(int bucketType, long beginTime, long endTime, java.lang.String callingPackage) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/usage/IUsageStatsManager.aidl:34:1:34:25")
  public android.content.pm.ParceledListSlice queryConfigurationStats(int bucketType, long beginTime, long endTime, java.lang.String callingPackage) throws android.os.RemoteException;
  public android.content.pm.ParceledListSlice queryEventStats(int bucketType, long beginTime, long endTime, java.lang.String callingPackage) throws android.os.RemoteException;
  public android.app.usage.UsageEvents queryEvents(long beginTime, long endTime, java.lang.String callingPackage) throws android.os.RemoteException;
  public android.app.usage.UsageEvents queryEventsForPackage(long beginTime, long endTime, java.lang.String callingPackage) throws android.os.RemoteException;
  public android.app.usage.UsageEvents queryEventsForUser(long beginTime, long endTime, int userId, java.lang.String callingPackage) throws android.os.RemoteException;
  public android.app.usage.UsageEvents queryEventsForPackageForUser(long beginTime, long endTime, int userId, java.lang.String pkg, java.lang.String callingPackage) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/usage/IUsageStatsManager.aidl:43:1:43:25")
  public void setAppInactive(java.lang.String packageName, boolean inactive, int userId) throws android.os.RemoteException;
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/usage/IUsageStatsManager.aidl:45:1:45:25")
  public boolean isAppInactive(java.lang.String packageName, int userId) throws android.os.RemoteException;
  public void whitelistAppTemporarily(java.lang.String packageName, long duration, int userId) throws android.os.RemoteException;
  public void onCarrierPrivilegedAppsChanged() throws android.os.RemoteException;
  public void reportChooserSelection(java.lang.String packageName, int userId, java.lang.String contentType, java.lang.String[] annotations, java.lang.String action) throws android.os.RemoteException;
  public int getAppStandbyBucket(java.lang.String packageName, java.lang.String callingPackage, int userId) throws android.os.RemoteException;
  public void setAppStandbyBucket(java.lang.String packageName, int bucket, int userId) throws android.os.RemoteException;
  public android.content.pm.ParceledListSlice getAppStandbyBuckets(java.lang.String callingPackage, int userId) throws android.os.RemoteException;
  public void setAppStandbyBuckets(android.content.pm.ParceledListSlice appBuckets, int userId) throws android.os.RemoteException;
  public void registerAppUsageObserver(int observerId, java.lang.String[] packages, long timeLimitMs, android.app.PendingIntent callback, java.lang.String callingPackage) throws android.os.RemoteException;
  public void unregisterAppUsageObserver(int observerId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void registerUsageSessionObserver(int sessionObserverId, java.lang.String[] observed, long timeLimitMs, long sessionThresholdTimeMs, android.app.PendingIntent limitReachedCallbackIntent, android.app.PendingIntent sessionEndCallbackIntent, java.lang.String callingPackage) throws android.os.RemoteException;
  public void unregisterUsageSessionObserver(int sessionObserverId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void registerAppUsageLimitObserver(int observerId, java.lang.String[] packages, long timeLimitMs, long timeUsedMs, android.app.PendingIntent callback, java.lang.String callingPackage) throws android.os.RemoteException;
  public void unregisterAppUsageLimitObserver(int observerId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void reportUsageStart(android.os.IBinder activity, java.lang.String token, java.lang.String callingPackage) throws android.os.RemoteException;
  public void reportPastUsageStart(android.os.IBinder activity, java.lang.String token, long timeAgoMs, java.lang.String callingPackage) throws android.os.RemoteException;
  public void reportUsageStop(android.os.IBinder activity, java.lang.String token, java.lang.String callingPackage) throws android.os.RemoteException;
  public int getUsageSource() throws android.os.RemoteException;
  public void forceUsageSourceSettingRead() throws android.os.RemoteException;
}
