/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os;
/**
  * Binder interface to communicate with the statistics management service.
  * {@hide}
  */
public interface IStatsManager extends android.os.IInterface
{
  /** Default implementation for IStatsManager. */
  public static class Default implements android.os.IStatsManager
  {
    /**
         * Tell the stats daemon that the android system server is up and running.
         */
    @Override public void systemRunning() throws android.os.RemoteException
    {
    }
    /**
         * Tell the stats daemon that the StatsCompanionService is up and running.
         * Two-way binder call so that caller knows message received.
         */
    @Override public void statsCompanionReady() throws android.os.RemoteException
    {
    }
    /**
         * Tells statsd that an anomaly may have occurred, so statsd can check whether this is so and
         * act accordingly.
         * Two-way binder call so that caller's method (and corresponding wakelocks) will linger.
         */
    @Override public void informAnomalyAlarmFired() throws android.os.RemoteException
    {
    }
    /**
         * Tells statsd that it is time to poll some stats. Statsd will be responsible for determing
         * what stats to poll and initiating the polling.
         * Two-way binder call so that caller's method (and corresponding wakelocks) will linger.
         */
    @Override public void informPollAlarmFired() throws android.os.RemoteException
    {
    }
    /**
         * Tells statsd that it is time to handle periodic alarms. Statsd will be responsible for
         * determing what alarm subscriber to trigger.
         * Two-way binder call so that caller's method (and corresponding wakelocks) will linger.
         */
    @Override public void informAlarmForSubscriberTriggeringFired() throws android.os.RemoteException
    {
    }
    /**
         * Tells statsd that the device is about to shutdown.
         */
    @Override public void informDeviceShutdown() throws android.os.RemoteException
    {
    }
    /**
         * Inform statsd about a file descriptor for a pipe through which we will pipe version
         * and package information for each uid.
         * Versions and package information are supplied via UidData proto where info for each app
         * is captured in its own element of a repeated ApplicationInfo message.
         */
    @Override public void informAllUidData(android.os.ParcelFileDescriptor fd) throws android.os.RemoteException
    {
    }
    /**
         * Inform statsd what the uid, version, version_string, and installer are for one app that was
         * updated.
         */
    @Override public void informOnePackage(java.lang.String app, int uid, long version, java.lang.String version_string, java.lang.String installer) throws android.os.RemoteException
    {
    }
    /**
         * Inform stats that an app was removed.
         */
    @Override public void informOnePackageRemoved(java.lang.String app, int uid) throws android.os.RemoteException
    {
    }
    /**
         * Fetches data for the specified configuration key. Returns a byte array representing proto
         * wire-encoded of ConfigMetricsReportList.
         *
         * Requires Manifest.permission.DUMP.
         */
    @Override public byte[] getData(long key, java.lang.String packageName) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Fetches metadata across statsd. Returns byte array representing wire-encoded proto.
         *
         * Requires Manifest.permission.DUMP.
         */
    @Override public byte[] getMetadata(java.lang.String packageName) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Sets a configuration with the specified config key and subscribes to updates for this
         * configuration key. Broadcasts will be sent if this configuration needs to be collected.
         * The configuration must be a wire-encoded StatsdConfig. The receiver for this data is
         * registered in a separate function.
         *
         * Requires Manifest.permission.DUMP.
         */
    @Override public void addConfiguration(long configKey, byte[] config, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    /**
         * Registers the given pending intent for this config key. This intent is invoked when the
         * memory consumed by the metrics for this configuration approach the pre-defined limits. There
         * can be at most one listener per config key.
         *
         * Requires Manifest.permission.DUMP.
         */
    @Override public void setDataFetchOperation(long configKey, android.os.IBinder intentSender, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    /**
         * Removes the data fetch operation for the specified configuration.
         *
         * Requires Manifest.permission.DUMP.
         */
    @Override public void removeDataFetchOperation(long configKey, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    /**
         * Registers the given pending intent for this packagename. This intent is invoked when the
         * active status of any of the configs sent by this package changes and will contain a list of
         * config ids that are currently active. It also returns the list of configs that are currently
         * active. There can be at most one active configs changed listener per package.
         *
         * Requires Manifest.permission.DUMP and Manifest.permission.PACKAGE_USAGE_STATS.
         */
    @Override public long[] setActiveConfigsChangedOperation(android.os.IBinder intentSender, java.lang.String packageName) throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Removes the active configs changed operation for the specified package name.
         *
         * Requires Manifest.permission.DUMP and Manifest.permission.PACKAGE_USAGE_STATS.
         */
    @Override public void removeActiveConfigsChangedOperation(java.lang.String packageName) throws android.os.RemoteException
    {
    }
    /**
         * Removes the configuration with the matching config key. No-op if this config key does not
         * exist.
         *
         * Requires Manifest.permission.DUMP.
         */
    @Override public void removeConfiguration(long configKey, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    /**
         * Set the IIntentSender (i.e. PendingIntent) to be used when broadcasting subscriber
         * information to the given subscriberId within the given config.
         *
         * Suppose that the calling uid has added a config with key configKey, and that in this config
         * it is specified that when a particular anomaly is detected, a broadcast should be sent to
         * a BroadcastSubscriber with id subscriberId. This function links the given intentSender with
         * that subscriberId (for that config), so that this intentSender is used to send the broadcast
         * when the anomaly is detected.
         *
         * This function can only be called by the owner (uid) of the config. It must be called each
         * time statsd starts. Later calls overwrite previous calls; only one intentSender is stored.
         *
         * intentSender must be convertible into an IntentSender using IntentSender(IBinder)
         * and cannot be null.
         *
         * Requires Manifest.permission.DUMP.
         */
    @Override public void setBroadcastSubscriber(long configKey, long subscriberId, android.os.IBinder intentSender, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    /**
         * Undoes setBroadcastSubscriber() for the (configKey, subscriberId) pair.
         * Any broadcasts associated with subscriberId will henceforth not be sent.
         * No-op if this (configKey, subsriberId) pair was not associated with an IntentSender.
         *
         * Requires Manifest.permission.DUMP.
         */
    @Override public void unsetBroadcastSubscriber(long configKey, long subscriberId, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    /**
         * Apps can send an atom via this application breadcrumb with the specified label and state for
         * this label. This allows building custom metrics and predicates.
         */
    @Override public void sendAppBreadcrumbAtom(int label, int state) throws android.os.RemoteException
    {
    }
    /**
         * Registers a puller callback function that, when invoked, pulls the data
         * for the specified vendor atom tag.
         *
         * Requires Manifest.permission.DUMP and Manifest.permission.PACKAGE_USAGE_STATS
         */
    @Override public void registerPullerCallback(int atomTag, android.os.IStatsPullerCallback pullerCallback, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    /**
        * Unregisters a puller callback function for the given vendor atom.
        *
        * Requires Manifest.permission.DUMP and Manifest.permission.PACKAGE_USAGE_STATS
        */
    @Override public void unregisterPullerCallback(int atomTag, java.lang.String packageName) throws android.os.RemoteException
    {
    }
    /**
         * Logs an event for binary push for module updates.
         */
    @Override public void sendBinaryPushStateChangedAtom(java.lang.String trainName, long trainVersionCode, int options, int state, long[] experimentId) throws android.os.RemoteException
    {
    }
    /**
         * Logs an event for watchdog rollbacks.
         */
    @Override public void sendWatchdogRollbackOccurredAtom(int rollbackType, java.lang.String packageName, long packageVersionCode, int rollbackReason, java.lang.String failingPackageName) throws android.os.RemoteException
    {
    }
    /**
         * Returns the most recently registered experiment IDs.
         */
    @Override public long[] getRegisteredExperimentIds() throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.IStatsManager
  {
    private static final java.lang.String DESCRIPTOR = "android.os.IStatsManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.IStatsManager interface,
     * generating a proxy if needed.
     */
    public static android.os.IStatsManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.IStatsManager))) {
        return ((android.os.IStatsManager)iin);
      }
      return new android.os.IStatsManager.Stub.Proxy(obj);
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
        case TRANSACTION_systemRunning:
        {
          data.enforceInterface(descriptor);
          this.systemRunning();
          return true;
        }
        case TRANSACTION_statsCompanionReady:
        {
          data.enforceInterface(descriptor);
          this.statsCompanionReady();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_informAnomalyAlarmFired:
        {
          data.enforceInterface(descriptor);
          this.informAnomalyAlarmFired();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_informPollAlarmFired:
        {
          data.enforceInterface(descriptor);
          this.informPollAlarmFired();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_informAlarmForSubscriberTriggeringFired:
        {
          data.enforceInterface(descriptor);
          this.informAlarmForSubscriberTriggeringFired();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_informDeviceShutdown:
        {
          data.enforceInterface(descriptor);
          this.informDeviceShutdown();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_informAllUidData:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.informAllUidData(_arg0);
          return true;
        }
        case TRANSACTION_informOnePackage:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          long _arg2;
          _arg2 = data.readLong();
          java.lang.String _arg3;
          _arg3 = data.readString();
          java.lang.String _arg4;
          _arg4 = data.readString();
          this.informOnePackage(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_informOnePackageRemoved:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.informOnePackageRemoved(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_getData:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          java.lang.String _arg1;
          _arg1 = data.readString();
          byte[] _result = this.getData(_arg0, _arg1);
          reply.writeNoException();
          reply.writeByteArray(_result);
          return true;
        }
        case TRANSACTION_getMetadata:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          byte[] _result = this.getMetadata(_arg0);
          reply.writeNoException();
          reply.writeByteArray(_result);
          return true;
        }
        case TRANSACTION_addConfiguration:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          byte[] _arg1;
          _arg1 = data.createByteArray();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.addConfiguration(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setDataFetchOperation:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.setDataFetchOperation(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeDataFetchOperation:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.removeDataFetchOperation(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setActiveConfigsChangedOperation:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          java.lang.String _arg1;
          _arg1 = data.readString();
          long[] _result = this.setActiveConfigsChangedOperation(_arg0, _arg1);
          reply.writeNoException();
          reply.writeLongArray(_result);
          return true;
        }
        case TRANSACTION_removeActiveConfigsChangedOperation:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.removeActiveConfigsChangedOperation(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeConfiguration:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.removeConfiguration(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setBroadcastSubscriber:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          long _arg1;
          _arg1 = data.readLong();
          android.os.IBinder _arg2;
          _arg2 = data.readStrongBinder();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.setBroadcastSubscriber(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unsetBroadcastSubscriber:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          long _arg1;
          _arg1 = data.readLong();
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.unsetBroadcastSubscriber(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_sendAppBreadcrumbAtom:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.sendAppBreadcrumbAtom(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_registerPullerCallback:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.IStatsPullerCallback _arg1;
          _arg1 = android.os.IStatsPullerCallback.Stub.asInterface(data.readStrongBinder());
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.registerPullerCallback(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_unregisterPullerCallback:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.unregisterPullerCallback(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_sendBinaryPushStateChangedAtom:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          long _arg1;
          _arg1 = data.readLong();
          int _arg2;
          _arg2 = data.readInt();
          int _arg3;
          _arg3 = data.readInt();
          long[] _arg4;
          _arg4 = data.createLongArray();
          this.sendBinaryPushStateChangedAtom(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_sendWatchdogRollbackOccurredAtom:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          long _arg2;
          _arg2 = data.readLong();
          int _arg3;
          _arg3 = data.readInt();
          java.lang.String _arg4;
          _arg4 = data.readString();
          this.sendWatchdogRollbackOccurredAtom(_arg0, _arg1, _arg2, _arg3, _arg4);
          return true;
        }
        case TRANSACTION_getRegisteredExperimentIds:
        {
          data.enforceInterface(descriptor);
          long[] _result = this.getRegisteredExperimentIds();
          reply.writeNoException();
          reply.writeLongArray(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.os.IStatsManager
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
           * Tell the stats daemon that the android system server is up and running.
           */
      @Override public void systemRunning() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_systemRunning, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().systemRunning();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Tell the stats daemon that the StatsCompanionService is up and running.
           * Two-way binder call so that caller knows message received.
           */
      @Override public void statsCompanionReady() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_statsCompanionReady, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().statsCompanionReady();
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
           * Tells statsd that an anomaly may have occurred, so statsd can check whether this is so and
           * act accordingly.
           * Two-way binder call so that caller's method (and corresponding wakelocks) will linger.
           */
      @Override public void informAnomalyAlarmFired() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_informAnomalyAlarmFired, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().informAnomalyAlarmFired();
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
           * Tells statsd that it is time to poll some stats. Statsd will be responsible for determing
           * what stats to poll and initiating the polling.
           * Two-way binder call so that caller's method (and corresponding wakelocks) will linger.
           */
      @Override public void informPollAlarmFired() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_informPollAlarmFired, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().informPollAlarmFired();
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
           * Tells statsd that it is time to handle periodic alarms. Statsd will be responsible for
           * determing what alarm subscriber to trigger.
           * Two-way binder call so that caller's method (and corresponding wakelocks) will linger.
           */
      @Override public void informAlarmForSubscriberTriggeringFired() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_informAlarmForSubscriberTriggeringFired, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().informAlarmForSubscriberTriggeringFired();
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
           * Tells statsd that the device is about to shutdown.
           */
      @Override public void informDeviceShutdown() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_informDeviceShutdown, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().informDeviceShutdown();
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
           * Inform statsd about a file descriptor for a pipe through which we will pipe version
           * and package information for each uid.
           * Versions and package information are supplied via UidData proto where info for each app
           * is captured in its own element of a repeated ApplicationInfo message.
           */
      @Override public void informAllUidData(android.os.ParcelFileDescriptor fd) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((fd!=null)) {
            _data.writeInt(1);
            fd.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_informAllUidData, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().informAllUidData(fd);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Inform statsd what the uid, version, version_string, and installer are for one app that was
           * updated.
           */
      @Override public void informOnePackage(java.lang.String app, int uid, long version, java.lang.String version_string, java.lang.String installer) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(app);
          _data.writeInt(uid);
          _data.writeLong(version);
          _data.writeString(version_string);
          _data.writeString(installer);
          boolean _status = mRemote.transact(Stub.TRANSACTION_informOnePackage, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().informOnePackage(app, uid, version, version_string, installer);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Inform stats that an app was removed.
           */
      @Override public void informOnePackageRemoved(java.lang.String app, int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(app);
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_informOnePackageRemoved, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().informOnePackageRemoved(app, uid);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Fetches data for the specified configuration key. Returns a byte array representing proto
           * wire-encoded of ConfigMetricsReportList.
           *
           * Requires Manifest.permission.DUMP.
           */
      @Override public byte[] getData(long key, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        byte[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(key);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getData, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getData(key, packageName);
          }
          _reply.readException();
          _result = _reply.createByteArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Fetches metadata across statsd. Returns byte array representing wire-encoded proto.
           *
           * Requires Manifest.permission.DUMP.
           */
      @Override public byte[] getMetadata(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        byte[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getMetadata, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getMetadata(packageName);
          }
          _reply.readException();
          _result = _reply.createByteArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Sets a configuration with the specified config key and subscribes to updates for this
           * configuration key. Broadcasts will be sent if this configuration needs to be collected.
           * The configuration must be a wire-encoded StatsdConfig. The receiver for this data is
           * registered in a separate function.
           *
           * Requires Manifest.permission.DUMP.
           */
      @Override public void addConfiguration(long configKey, byte[] config, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(configKey);
          _data.writeByteArray(config);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addConfiguration, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addConfiguration(configKey, config, packageName);
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
           * Registers the given pending intent for this config key. This intent is invoked when the
           * memory consumed by the metrics for this configuration approach the pre-defined limits. There
           * can be at most one listener per config key.
           *
           * Requires Manifest.permission.DUMP.
           */
      @Override public void setDataFetchOperation(long configKey, android.os.IBinder intentSender, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(configKey);
          _data.writeStrongBinder(intentSender);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDataFetchOperation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDataFetchOperation(configKey, intentSender, packageName);
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
           * Removes the data fetch operation for the specified configuration.
           *
           * Requires Manifest.permission.DUMP.
           */
      @Override public void removeDataFetchOperation(long configKey, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(configKey);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeDataFetchOperation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeDataFetchOperation(configKey, packageName);
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
           * Registers the given pending intent for this packagename. This intent is invoked when the
           * active status of any of the configs sent by this package changes and will contain a list of
           * config ids that are currently active. It also returns the list of configs that are currently
           * active. There can be at most one active configs changed listener per package.
           *
           * Requires Manifest.permission.DUMP and Manifest.permission.PACKAGE_USAGE_STATS.
           */
      @Override public long[] setActiveConfigsChangedOperation(android.os.IBinder intentSender, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(intentSender);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setActiveConfigsChangedOperation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().setActiveConfigsChangedOperation(intentSender, packageName);
          }
          _reply.readException();
          _result = _reply.createLongArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Removes the active configs changed operation for the specified package name.
           *
           * Requires Manifest.permission.DUMP and Manifest.permission.PACKAGE_USAGE_STATS.
           */
      @Override public void removeActiveConfigsChangedOperation(java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeActiveConfigsChangedOperation, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeActiveConfigsChangedOperation(packageName);
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
           * Removes the configuration with the matching config key. No-op if this config key does not
           * exist.
           *
           * Requires Manifest.permission.DUMP.
           */
      @Override public void removeConfiguration(long configKey, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(configKey);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeConfiguration, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeConfiguration(configKey, packageName);
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
           * Set the IIntentSender (i.e. PendingIntent) to be used when broadcasting subscriber
           * information to the given subscriberId within the given config.
           *
           * Suppose that the calling uid has added a config with key configKey, and that in this config
           * it is specified that when a particular anomaly is detected, a broadcast should be sent to
           * a BroadcastSubscriber with id subscriberId. This function links the given intentSender with
           * that subscriberId (for that config), so that this intentSender is used to send the broadcast
           * when the anomaly is detected.
           *
           * This function can only be called by the owner (uid) of the config. It must be called each
           * time statsd starts. Later calls overwrite previous calls; only one intentSender is stored.
           *
           * intentSender must be convertible into an IntentSender using IntentSender(IBinder)
           * and cannot be null.
           *
           * Requires Manifest.permission.DUMP.
           */
      @Override public void setBroadcastSubscriber(long configKey, long subscriberId, android.os.IBinder intentSender, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(configKey);
          _data.writeLong(subscriberId);
          _data.writeStrongBinder(intentSender);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setBroadcastSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setBroadcastSubscriber(configKey, subscriberId, intentSender, packageName);
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
           * Undoes setBroadcastSubscriber() for the (configKey, subscriberId) pair.
           * Any broadcasts associated with subscriberId will henceforth not be sent.
           * No-op if this (configKey, subsriberId) pair was not associated with an IntentSender.
           *
           * Requires Manifest.permission.DUMP.
           */
      @Override public void unsetBroadcastSubscriber(long configKey, long subscriberId, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(configKey);
          _data.writeLong(subscriberId);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unsetBroadcastSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unsetBroadcastSubscriber(configKey, subscriberId, packageName);
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
           * Apps can send an atom via this application breadcrumb with the specified label and state for
           * this label. This allows building custom metrics and predicates.
           */
      @Override public void sendAppBreadcrumbAtom(int label, int state) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(label);
          _data.writeInt(state);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendAppBreadcrumbAtom, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendAppBreadcrumbAtom(label, state);
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
           * Registers a puller callback function that, when invoked, pulls the data
           * for the specified vendor atom tag.
           *
           * Requires Manifest.permission.DUMP and Manifest.permission.PACKAGE_USAGE_STATS
           */
      @Override public void registerPullerCallback(int atomTag, android.os.IStatsPullerCallback pullerCallback, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(atomTag);
          _data.writeStrongBinder((((pullerCallback!=null))?(pullerCallback.asBinder()):(null)));
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerPullerCallback, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerPullerCallback(atomTag, pullerCallback, packageName);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
          * Unregisters a puller callback function for the given vendor atom.
          *
          * Requires Manifest.permission.DUMP and Manifest.permission.PACKAGE_USAGE_STATS
          */
      @Override public void unregisterPullerCallback(int atomTag, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(atomTag);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterPullerCallback, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterPullerCallback(atomTag, packageName);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Logs an event for binary push for module updates.
           */
      @Override public void sendBinaryPushStateChangedAtom(java.lang.String trainName, long trainVersionCode, int options, int state, long[] experimentId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(trainName);
          _data.writeLong(trainVersionCode);
          _data.writeInt(options);
          _data.writeInt(state);
          _data.writeLongArray(experimentId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendBinaryPushStateChangedAtom, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendBinaryPushStateChangedAtom(trainName, trainVersionCode, options, state, experimentId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Logs an event for watchdog rollbacks.
           */
      @Override public void sendWatchdogRollbackOccurredAtom(int rollbackType, java.lang.String packageName, long packageVersionCode, int rollbackReason, java.lang.String failingPackageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(rollbackType);
          _data.writeString(packageName);
          _data.writeLong(packageVersionCode);
          _data.writeInt(rollbackReason);
          _data.writeString(failingPackageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendWatchdogRollbackOccurredAtom, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendWatchdogRollbackOccurredAtom(rollbackType, packageName, packageVersionCode, rollbackReason, failingPackageName);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Returns the most recently registered experiment IDs.
           */
      @Override public long[] getRegisteredExperimentIds() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getRegisteredExperimentIds, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getRegisteredExperimentIds();
          }
          _reply.readException();
          _result = _reply.createLongArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.os.IStatsManager sDefaultImpl;
    }
    static final int TRANSACTION_systemRunning = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_statsCompanionReady = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_informAnomalyAlarmFired = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_informPollAlarmFired = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_informAlarmForSubscriberTriggeringFired = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_informDeviceShutdown = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_informAllUidData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_informOnePackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_informOnePackageRemoved = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_getMetadata = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_addConfiguration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_setDataFetchOperation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_removeDataFetchOperation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_setActiveConfigsChangedOperation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_removeActiveConfigsChangedOperation = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_removeConfiguration = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_setBroadcastSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_unsetBroadcastSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_sendAppBreadcrumbAtom = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_registerPullerCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_unregisterPullerCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_sendBinaryPushStateChangedAtom = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_sendWatchdogRollbackOccurredAtom = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_getRegisteredExperimentIds = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    public static boolean setDefaultImpl(android.os.IStatsManager impl) {
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
    public static android.os.IStatsManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * The install requires staging.
       */
  public static final int FLAG_REQUIRE_STAGING = 1;
  /**
       * Rollback is enabled with this install.
       */
  public static final int FLAG_ROLLBACK_ENABLED = 2;
  /**
       * Requires low latency monitoring.
       */
  public static final int FLAG_REQUIRE_LOW_LATENCY_MONITOR = 4;
  /**
       * Tell the stats daemon that the android system server is up and running.
       */
  public void systemRunning() throws android.os.RemoteException;
  /**
       * Tell the stats daemon that the StatsCompanionService is up and running.
       * Two-way binder call so that caller knows message received.
       */
  public void statsCompanionReady() throws android.os.RemoteException;
  /**
       * Tells statsd that an anomaly may have occurred, so statsd can check whether this is so and
       * act accordingly.
       * Two-way binder call so that caller's method (and corresponding wakelocks) will linger.
       */
  public void informAnomalyAlarmFired() throws android.os.RemoteException;
  /**
       * Tells statsd that it is time to poll some stats. Statsd will be responsible for determing
       * what stats to poll and initiating the polling.
       * Two-way binder call so that caller's method (and corresponding wakelocks) will linger.
       */
  public void informPollAlarmFired() throws android.os.RemoteException;
  /**
       * Tells statsd that it is time to handle periodic alarms. Statsd will be responsible for
       * determing what alarm subscriber to trigger.
       * Two-way binder call so that caller's method (and corresponding wakelocks) will linger.
       */
  public void informAlarmForSubscriberTriggeringFired() throws android.os.RemoteException;
  /**
       * Tells statsd that the device is about to shutdown.
       */
  public void informDeviceShutdown() throws android.os.RemoteException;
  /**
       * Inform statsd about a file descriptor for a pipe through which we will pipe version
       * and package information for each uid.
       * Versions and package information are supplied via UidData proto where info for each app
       * is captured in its own element of a repeated ApplicationInfo message.
       */
  public void informAllUidData(android.os.ParcelFileDescriptor fd) throws android.os.RemoteException;
  /**
       * Inform statsd what the uid, version, version_string, and installer are for one app that was
       * updated.
       */
  public void informOnePackage(java.lang.String app, int uid, long version, java.lang.String version_string, java.lang.String installer) throws android.os.RemoteException;
  /**
       * Inform stats that an app was removed.
       */
  public void informOnePackageRemoved(java.lang.String app, int uid) throws android.os.RemoteException;
  /**
       * Fetches data for the specified configuration key. Returns a byte array representing proto
       * wire-encoded of ConfigMetricsReportList.
       *
       * Requires Manifest.permission.DUMP.
       */
  public byte[] getData(long key, java.lang.String packageName) throws android.os.RemoteException;
  /**
       * Fetches metadata across statsd. Returns byte array representing wire-encoded proto.
       *
       * Requires Manifest.permission.DUMP.
       */
  public byte[] getMetadata(java.lang.String packageName) throws android.os.RemoteException;
  /**
       * Sets a configuration with the specified config key and subscribes to updates for this
       * configuration key. Broadcasts will be sent if this configuration needs to be collected.
       * The configuration must be a wire-encoded StatsdConfig. The receiver for this data is
       * registered in a separate function.
       *
       * Requires Manifest.permission.DUMP.
       */
  public void addConfiguration(long configKey, byte[] config, java.lang.String packageName) throws android.os.RemoteException;
  /**
       * Registers the given pending intent for this config key. This intent is invoked when the
       * memory consumed by the metrics for this configuration approach the pre-defined limits. There
       * can be at most one listener per config key.
       *
       * Requires Manifest.permission.DUMP.
       */
  public void setDataFetchOperation(long configKey, android.os.IBinder intentSender, java.lang.String packageName) throws android.os.RemoteException;
  /**
       * Removes the data fetch operation for the specified configuration.
       *
       * Requires Manifest.permission.DUMP.
       */
  public void removeDataFetchOperation(long configKey, java.lang.String packageName) throws android.os.RemoteException;
  /**
       * Registers the given pending intent for this packagename. This intent is invoked when the
       * active status of any of the configs sent by this package changes and will contain a list of
       * config ids that are currently active. It also returns the list of configs that are currently
       * active. There can be at most one active configs changed listener per package.
       *
       * Requires Manifest.permission.DUMP and Manifest.permission.PACKAGE_USAGE_STATS.
       */
  public long[] setActiveConfigsChangedOperation(android.os.IBinder intentSender, java.lang.String packageName) throws android.os.RemoteException;
  /**
       * Removes the active configs changed operation for the specified package name.
       *
       * Requires Manifest.permission.DUMP and Manifest.permission.PACKAGE_USAGE_STATS.
       */
  public void removeActiveConfigsChangedOperation(java.lang.String packageName) throws android.os.RemoteException;
  /**
       * Removes the configuration with the matching config key. No-op if this config key does not
       * exist.
       *
       * Requires Manifest.permission.DUMP.
       */
  public void removeConfiguration(long configKey, java.lang.String packageName) throws android.os.RemoteException;
  /**
       * Set the IIntentSender (i.e. PendingIntent) to be used when broadcasting subscriber
       * information to the given subscriberId within the given config.
       *
       * Suppose that the calling uid has added a config with key configKey, and that in this config
       * it is specified that when a particular anomaly is detected, a broadcast should be sent to
       * a BroadcastSubscriber with id subscriberId. This function links the given intentSender with
       * that subscriberId (for that config), so that this intentSender is used to send the broadcast
       * when the anomaly is detected.
       *
       * This function can only be called by the owner (uid) of the config. It must be called each
       * time statsd starts. Later calls overwrite previous calls; only one intentSender is stored.
       *
       * intentSender must be convertible into an IntentSender using IntentSender(IBinder)
       * and cannot be null.
       *
       * Requires Manifest.permission.DUMP.
       */
  public void setBroadcastSubscriber(long configKey, long subscriberId, android.os.IBinder intentSender, java.lang.String packageName) throws android.os.RemoteException;
  /**
       * Undoes setBroadcastSubscriber() for the (configKey, subscriberId) pair.
       * Any broadcasts associated with subscriberId will henceforth not be sent.
       * No-op if this (configKey, subsriberId) pair was not associated with an IntentSender.
       *
       * Requires Manifest.permission.DUMP.
       */
  public void unsetBroadcastSubscriber(long configKey, long subscriberId, java.lang.String packageName) throws android.os.RemoteException;
  /**
       * Apps can send an atom via this application breadcrumb with the specified label and state for
       * this label. This allows building custom metrics and predicates.
       */
  public void sendAppBreadcrumbAtom(int label, int state) throws android.os.RemoteException;
  /**
       * Registers a puller callback function that, when invoked, pulls the data
       * for the specified vendor atom tag.
       *
       * Requires Manifest.permission.DUMP and Manifest.permission.PACKAGE_USAGE_STATS
       */
  public void registerPullerCallback(int atomTag, android.os.IStatsPullerCallback pullerCallback, java.lang.String packageName) throws android.os.RemoteException;
  /**
      * Unregisters a puller callback function for the given vendor atom.
      *
      * Requires Manifest.permission.DUMP and Manifest.permission.PACKAGE_USAGE_STATS
      */
  public void unregisterPullerCallback(int atomTag, java.lang.String packageName) throws android.os.RemoteException;
  /**
       * Logs an event for binary push for module updates.
       */
  public void sendBinaryPushStateChangedAtom(java.lang.String trainName, long trainVersionCode, int options, int state, long[] experimentId) throws android.os.RemoteException;
  /**
       * Logs an event for watchdog rollbacks.
       */
  public void sendWatchdogRollbackOccurredAtom(int rollbackType, java.lang.String packageName, long packageVersionCode, int rollbackReason, java.lang.String failingPackageName) throws android.os.RemoteException;
  /**
       * Returns the most recently registered experiment IDs.
       */
  public long[] getRegisteredExperimentIds() throws android.os.RemoteException;
}
