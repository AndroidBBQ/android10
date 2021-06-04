/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os;
/**
  * Binder interface to communicate with the Java-based statistics service helper.
  * {@hide}
  */
public interface IStatsCompanionService extends android.os.IInterface
{
  /** Default implementation for IStatsCompanionService. */
  public static class Default implements android.os.IStatsCompanionService
  {
    /**
         * Tell statscompanion that stastd is up and running.
         */
    @Override public void statsdReady() throws android.os.RemoteException
    {
    }
    /**
        * Register an alarm for anomaly detection to fire at the given timestamp (ms since epoch).
        * If anomaly alarm had already been registered, it will be replaced with the new timestamp.
        * Uses AlarmManager.set API, so  if the timestamp is in the past, alarm fires immediately, and
        * alarm is inexact.
        */
    @Override public void setAnomalyAlarm(long timestampMs) throws android.os.RemoteException
    {
    }
    /** Cancel any anomaly detection alarm. */
    @Override public void cancelAnomalyAlarm() throws android.os.RemoteException
    {
    }
    /**
          * Register a repeating alarm for pulling to fire at the given timestamp and every
          * intervalMs thereafter (in ms since epoch).
          * If polling alarm had already been registered, it will be replaced by new one.
          * Uses AlarmManager.setRepeating API, so if the timestamp is in past, alarm fires immediately,
          * and alarm is inexact.
          */
    @Override public void setPullingAlarm(long nextPullTimeMs) throws android.os.RemoteException
    {
    }
    /** Cancel any repeating pulling alarm. */
    @Override public void cancelPullingAlarm() throws android.os.RemoteException
    {
    }
    /**
          * Register an alarm when we want to trigger subscribers at the given
          * timestamp (in ms since epoch).
          * If an alarm had already been registered, it will be replaced by new one.
          */
    @Override public void setAlarmForSubscriberTriggering(long timestampMs) throws android.os.RemoteException
    {
    }
    /** Cancel any alarm for the purpose of subscriber triggering. */
    @Override public void cancelAlarmForSubscriberTriggering() throws android.os.RemoteException
    {
    }
    /** Pull the specified data. Results will be sent to statsd when complete. */
    @Override public android.os.StatsLogEventWrapper[] pullData(int pullCode) throws android.os.RemoteException
    {
      return null;
    }
    /** Send a broadcast to the specified PendingIntent's as IBinder that it should getData now. */
    @Override public void sendDataBroadcast(android.os.IBinder intentSender, long lastReportTimeNs) throws android.os.RemoteException
    {
    }
    /**
         * Send a broadcast to the specified PendingIntent's as IBinder notifying it that the list
         * of active configs has changed.
         */
    @Override public void sendActiveConfigsChangedBroadcast(android.os.IBinder intentSender, long[] configIds) throws android.os.RemoteException
    {
    }
    /**
         * Requests StatsCompanionService to send a broadcast using the given intentSender
         * (which should cast to an IIntentSender), along with the other information specified.
         */
    @Override public void sendSubscriberBroadcast(android.os.IBinder intentSender, long configUid, long configId, long subscriptionId, long subscriptionRuleId, java.lang.String[] cookies, android.os.StatsDimensionsValue dimensionsValue) throws android.os.RemoteException
    {
    }
    /** Tells StatsCompaionService to grab the uid map snapshot and send it to statsd. */
    @Override public void triggerUidSnapshot() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.IStatsCompanionService
  {
    private static final java.lang.String DESCRIPTOR = "android.os.IStatsCompanionService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.IStatsCompanionService interface,
     * generating a proxy if needed.
     */
    public static android.os.IStatsCompanionService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.IStatsCompanionService))) {
        return ((android.os.IStatsCompanionService)iin);
      }
      return new android.os.IStatsCompanionService.Stub.Proxy(obj);
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
        case TRANSACTION_statsdReady:
        {
          data.enforceInterface(descriptor);
          this.statsdReady();
          return true;
        }
        case TRANSACTION_setAnomalyAlarm:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.setAnomalyAlarm(_arg0);
          return true;
        }
        case TRANSACTION_cancelAnomalyAlarm:
        {
          data.enforceInterface(descriptor);
          this.cancelAnomalyAlarm();
          return true;
        }
        case TRANSACTION_setPullingAlarm:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.setPullingAlarm(_arg0);
          return true;
        }
        case TRANSACTION_cancelPullingAlarm:
        {
          data.enforceInterface(descriptor);
          this.cancelPullingAlarm();
          return true;
        }
        case TRANSACTION_setAlarmForSubscriberTriggering:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.setAlarmForSubscriberTriggering(_arg0);
          return true;
        }
        case TRANSACTION_cancelAlarmForSubscriberTriggering:
        {
          data.enforceInterface(descriptor);
          this.cancelAlarmForSubscriberTriggering();
          return true;
        }
        case TRANSACTION_pullData:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.StatsLogEventWrapper[] _result = this.pullData(_arg0);
          reply.writeNoException();
          reply.writeTypedArray(_result, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          return true;
        }
        case TRANSACTION_sendDataBroadcast:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          long _arg1;
          _arg1 = data.readLong();
          this.sendDataBroadcast(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_sendActiveConfigsChangedBroadcast:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          long[] _arg1;
          _arg1 = data.createLongArray();
          this.sendActiveConfigsChangedBroadcast(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_sendSubscriberBroadcast:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          long _arg1;
          _arg1 = data.readLong();
          long _arg2;
          _arg2 = data.readLong();
          long _arg3;
          _arg3 = data.readLong();
          long _arg4;
          _arg4 = data.readLong();
          java.lang.String[] _arg5;
          _arg5 = data.createStringArray();
          android.os.StatsDimensionsValue _arg6;
          if ((0!=data.readInt())) {
            _arg6 = android.os.StatsDimensionsValue.CREATOR.createFromParcel(data);
          }
          else {
            _arg6 = null;
          }
          this.sendSubscriberBroadcast(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6);
          return true;
        }
        case TRANSACTION_triggerUidSnapshot:
        {
          data.enforceInterface(descriptor);
          this.triggerUidSnapshot();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.os.IStatsCompanionService
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
           * Tell statscompanion that stastd is up and running.
           */
      @Override public void statsdReady() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_statsdReady, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().statsdReady();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
          * Register an alarm for anomaly detection to fire at the given timestamp (ms since epoch).
          * If anomaly alarm had already been registered, it will be replaced with the new timestamp.
          * Uses AlarmManager.set API, so  if the timestamp is in the past, alarm fires immediately, and
          * alarm is inexact.
          */
      @Override public void setAnomalyAlarm(long timestampMs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(timestampMs);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAnomalyAlarm, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAnomalyAlarm(timestampMs);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /** Cancel any anomaly detection alarm. */
      @Override public void cancelAnomalyAlarm() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelAnomalyAlarm, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelAnomalyAlarm();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
            * Register a repeating alarm for pulling to fire at the given timestamp and every
            * intervalMs thereafter (in ms since epoch).
            * If polling alarm had already been registered, it will be replaced by new one.
            * Uses AlarmManager.setRepeating API, so if the timestamp is in past, alarm fires immediately,
            * and alarm is inexact.
            */
      @Override public void setPullingAlarm(long nextPullTimeMs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(nextPullTimeMs);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPullingAlarm, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPullingAlarm(nextPullTimeMs);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /** Cancel any repeating pulling alarm. */
      @Override public void cancelPullingAlarm() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelPullingAlarm, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelPullingAlarm();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
            * Register an alarm when we want to trigger subscribers at the given
            * timestamp (in ms since epoch).
            * If an alarm had already been registered, it will be replaced by new one.
            */
      @Override public void setAlarmForSubscriberTriggering(long timestampMs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(timestampMs);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAlarmForSubscriberTriggering, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAlarmForSubscriberTriggering(timestampMs);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /** Cancel any alarm for the purpose of subscriber triggering. */
      @Override public void cancelAlarmForSubscriberTriggering() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelAlarmForSubscriberTriggering, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelAlarmForSubscriberTriggering();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /** Pull the specified data. Results will be sent to statsd when complete. */
      @Override public android.os.StatsLogEventWrapper[] pullData(int pullCode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.StatsLogEventWrapper[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(pullCode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_pullData, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().pullData(pullCode);
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.os.StatsLogEventWrapper.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /** Send a broadcast to the specified PendingIntent's as IBinder that it should getData now. */
      @Override public void sendDataBroadcast(android.os.IBinder intentSender, long lastReportTimeNs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(intentSender);
          _data.writeLong(lastReportTimeNs);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendDataBroadcast, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendDataBroadcast(intentSender, lastReportTimeNs);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Send a broadcast to the specified PendingIntent's as IBinder notifying it that the list
           * of active configs has changed.
           */
      @Override public void sendActiveConfigsChangedBroadcast(android.os.IBinder intentSender, long[] configIds) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(intentSender);
          _data.writeLongArray(configIds);
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendActiveConfigsChangedBroadcast, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendActiveConfigsChangedBroadcast(intentSender, configIds);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Requests StatsCompanionService to send a broadcast using the given intentSender
           * (which should cast to an IIntentSender), along with the other information specified.
           */
      @Override public void sendSubscriberBroadcast(android.os.IBinder intentSender, long configUid, long configId, long subscriptionId, long subscriptionRuleId, java.lang.String[] cookies, android.os.StatsDimensionsValue dimensionsValue) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(intentSender);
          _data.writeLong(configUid);
          _data.writeLong(configId);
          _data.writeLong(subscriptionId);
          _data.writeLong(subscriptionRuleId);
          _data.writeStringArray(cookies);
          if ((dimensionsValue!=null)) {
            _data.writeInt(1);
            dimensionsValue.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendSubscriberBroadcast, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sendSubscriberBroadcast(intentSender, configUid, configId, subscriptionId, subscriptionRuleId, cookies, dimensionsValue);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /** Tells StatsCompaionService to grab the uid map snapshot and send it to statsd. */
      @Override public void triggerUidSnapshot() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_triggerUidSnapshot, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().triggerUidSnapshot();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.os.IStatsCompanionService sDefaultImpl;
    }
    static final int TRANSACTION_statsdReady = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_setAnomalyAlarm = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_cancelAnomalyAlarm = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_setPullingAlarm = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_cancelPullingAlarm = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_setAlarmForSubscriberTriggering = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_cancelAlarmForSubscriberTriggering = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_pullData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_sendDataBroadcast = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_sendActiveConfigsChangedBroadcast = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_sendSubscriberBroadcast = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_triggerUidSnapshot = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    public static boolean setDefaultImpl(android.os.IStatsCompanionService impl) {
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
    public static android.os.IStatsCompanionService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Tell statscompanion that stastd is up and running.
       */
  public void statsdReady() throws android.os.RemoteException;
  /**
      * Register an alarm for anomaly detection to fire at the given timestamp (ms since epoch).
      * If anomaly alarm had already been registered, it will be replaced with the new timestamp.
      * Uses AlarmManager.set API, so  if the timestamp is in the past, alarm fires immediately, and
      * alarm is inexact.
      */
  public void setAnomalyAlarm(long timestampMs) throws android.os.RemoteException;
  /** Cancel any anomaly detection alarm. */
  public void cancelAnomalyAlarm() throws android.os.RemoteException;
  /**
        * Register a repeating alarm for pulling to fire at the given timestamp and every
        * intervalMs thereafter (in ms since epoch).
        * If polling alarm had already been registered, it will be replaced by new one.
        * Uses AlarmManager.setRepeating API, so if the timestamp is in past, alarm fires immediately,
        * and alarm is inexact.
        */
  public void setPullingAlarm(long nextPullTimeMs) throws android.os.RemoteException;
  /** Cancel any repeating pulling alarm. */
  public void cancelPullingAlarm() throws android.os.RemoteException;
  /**
        * Register an alarm when we want to trigger subscribers at the given
        * timestamp (in ms since epoch).
        * If an alarm had already been registered, it will be replaced by new one.
        */
  public void setAlarmForSubscriberTriggering(long timestampMs) throws android.os.RemoteException;
  /** Cancel any alarm for the purpose of subscriber triggering. */
  public void cancelAlarmForSubscriberTriggering() throws android.os.RemoteException;
  /** Pull the specified data. Results will be sent to statsd when complete. */
  public android.os.StatsLogEventWrapper[] pullData(int pullCode) throws android.os.RemoteException;
  /** Send a broadcast to the specified PendingIntent's as IBinder that it should getData now. */
  public void sendDataBroadcast(android.os.IBinder intentSender, long lastReportTimeNs) throws android.os.RemoteException;
  /**
       * Send a broadcast to the specified PendingIntent's as IBinder notifying it that the list
       * of active configs has changed.
       */
  public void sendActiveConfigsChangedBroadcast(android.os.IBinder intentSender, long[] configIds) throws android.os.RemoteException;
  /**
       * Requests StatsCompanionService to send a broadcast using the given intentSender
       * (which should cast to an IIntentSender), along with the other information specified.
       */
  public void sendSubscriberBroadcast(android.os.IBinder intentSender, long configUid, long configId, long subscriptionId, long subscriptionRuleId, java.lang.String[] cookies, android.os.StatsDimensionsValue dimensionsValue) throws android.os.RemoteException;
  /** Tells StatsCompaionService to grab the uid map snapshot and send it to statsd. */
  public void triggerUidSnapshot() throws android.os.RemoteException;
}
