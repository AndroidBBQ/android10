/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.storagemonitoring;
/** @hide */
public interface ICarStorageMonitoring extends android.os.IInterface
{
  /** Default implementation for ICarStorageMonitoring. */
  public static class Default implements android.car.storagemonitoring.ICarStorageMonitoring
  {
    /**
       * Returns the value of the PRE_EOL register.
       */
    @Override public int getPreEolIndicatorStatus() throws android.os.RemoteException
    {
      return 0;
    }
    /**
       * Returns the current wear estimate indicators.
       */
    @Override public android.car.storagemonitoring.WearEstimate getWearEstimate() throws android.os.RemoteException
    {
      return null;
    }
    /**
       * Returns the list of all observed wear estimate changes.
       */
    @Override public java.util.List<android.car.storagemonitoring.WearEstimateChange> getWearEstimateHistory() throws android.os.RemoteException
    {
      return null;
    }
    /**
       * Returns I/O stats as collected at service boot time.
       */
    @Override public java.util.List<android.car.storagemonitoring.IoStatsEntry> getBootIoStats() throws android.os.RemoteException
    {
      return null;
    }
    /**
       * Returns total I/O stats as collected from kernel start until the last snapshot.
       */
    @Override public java.util.List<android.car.storagemonitoring.IoStatsEntry> getAggregateIoStats() throws android.os.RemoteException
    {
      return null;
    }
    /**
       * Return the I/O stats deltas currently known to the service.
       */
    @Override public java.util.List<android.car.storagemonitoring.IoStats> getIoStatsDeltas() throws android.os.RemoteException
    {
      return null;
    }
    /**
       * Register a new listener to receive new I/O activity deltas as they are generated.
       */
    @Override public void registerListener(android.car.storagemonitoring.IIoStatsListener listener) throws android.os.RemoteException
    {
    }
    /**
       * Remove a listener registration, terminating delivery of I/O activity deltas to it.
       */
    @Override public void unregisterListener(android.car.storagemonitoring.IIoStatsListener listener) throws android.os.RemoteException
    {
    }
    /**
       * Returns the approximate amount of bytes written to disk during the previous shutdown.
       */
    @Override public long getShutdownDiskWriteAmount() throws android.os.RemoteException
    {
      return 0L;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.storagemonitoring.ICarStorageMonitoring
  {
    private static final java.lang.String DESCRIPTOR = "android.car.storagemonitoring.ICarStorageMonitoring";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.storagemonitoring.ICarStorageMonitoring interface,
     * generating a proxy if needed.
     */
    public static android.car.storagemonitoring.ICarStorageMonitoring asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.storagemonitoring.ICarStorageMonitoring))) {
        return ((android.car.storagemonitoring.ICarStorageMonitoring)iin);
      }
      return new android.car.storagemonitoring.ICarStorageMonitoring.Stub.Proxy(obj);
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
        case TRANSACTION_getPreEolIndicatorStatus:
        {
          data.enforceInterface(descriptor);
          int _result = this.getPreEolIndicatorStatus();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getWearEstimate:
        {
          data.enforceInterface(descriptor);
          android.car.storagemonitoring.WearEstimate _result = this.getWearEstimate();
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
        case TRANSACTION_getWearEstimateHistory:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.car.storagemonitoring.WearEstimateChange> _result = this.getWearEstimateHistory();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getBootIoStats:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.car.storagemonitoring.IoStatsEntry> _result = this.getBootIoStats();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getAggregateIoStats:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.car.storagemonitoring.IoStatsEntry> _result = this.getAggregateIoStats();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getIoStatsDeltas:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.car.storagemonitoring.IoStats> _result = this.getIoStatsDeltas();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_registerListener:
        {
          data.enforceInterface(descriptor);
          android.car.storagemonitoring.IIoStatsListener _arg0;
          _arg0 = android.car.storagemonitoring.IIoStatsListener.Stub.asInterface(data.readStrongBinder());
          this.registerListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unregisterListener:
        {
          data.enforceInterface(descriptor);
          android.car.storagemonitoring.IIoStatsListener _arg0;
          _arg0 = android.car.storagemonitoring.IIoStatsListener.Stub.asInterface(data.readStrongBinder());
          this.unregisterListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getShutdownDiskWriteAmount:
        {
          data.enforceInterface(descriptor);
          long _result = this.getShutdownDiskWriteAmount();
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.storagemonitoring.ICarStorageMonitoring
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
         * Returns the value of the PRE_EOL register.
         */
      @Override public int getPreEolIndicatorStatus() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPreEolIndicatorStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPreEolIndicatorStatus();
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
      /**
         * Returns the current wear estimate indicators.
         */
      @Override public android.car.storagemonitoring.WearEstimate getWearEstimate() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.car.storagemonitoring.WearEstimate _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getWearEstimate, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getWearEstimate();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.car.storagemonitoring.WearEstimate.CREATOR.createFromParcel(_reply);
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
      /**
         * Returns the list of all observed wear estimate changes.
         */
      @Override public java.util.List<android.car.storagemonitoring.WearEstimateChange> getWearEstimateHistory() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.car.storagemonitoring.WearEstimateChange> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getWearEstimateHistory, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getWearEstimateHistory();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.car.storagemonitoring.WearEstimateChange.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
         * Returns I/O stats as collected at service boot time.
         */
      @Override public java.util.List<android.car.storagemonitoring.IoStatsEntry> getBootIoStats() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.car.storagemonitoring.IoStatsEntry> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getBootIoStats, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getBootIoStats();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.car.storagemonitoring.IoStatsEntry.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
         * Returns total I/O stats as collected from kernel start until the last snapshot.
         */
      @Override public java.util.List<android.car.storagemonitoring.IoStatsEntry> getAggregateIoStats() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.car.storagemonitoring.IoStatsEntry> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAggregateIoStats, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAggregateIoStats();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.car.storagemonitoring.IoStatsEntry.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
         * Return the I/O stats deltas currently known to the service.
         */
      @Override public java.util.List<android.car.storagemonitoring.IoStats> getIoStatsDeltas() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.car.storagemonitoring.IoStats> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getIoStatsDeltas, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getIoStatsDeltas();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.car.storagemonitoring.IoStats.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
         * Register a new listener to receive new I/O activity deltas as they are generated.
         */
      @Override public void registerListener(android.car.storagemonitoring.IIoStatsListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().registerListener(listener);
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
         * Remove a listener registration, terminating delivery of I/O activity deltas to it.
         */
      @Override public void unregisterListener(android.car.storagemonitoring.IIoStatsListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterListener(listener);
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
         * Returns the approximate amount of bytes written to disk during the previous shutdown.
         */
      @Override public long getShutdownDiskWriteAmount() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getShutdownDiskWriteAmount, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getShutdownDiskWriteAmount();
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
      public static android.car.storagemonitoring.ICarStorageMonitoring sDefaultImpl;
    }
    static final int TRANSACTION_getPreEolIndicatorStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getWearEstimate = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getWearEstimateHistory = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_getBootIoStats = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getAggregateIoStats = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_getIoStatsDeltas = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_registerListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_unregisterListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getShutdownDiskWriteAmount = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    public static boolean setDefaultImpl(android.car.storagemonitoring.ICarStorageMonitoring impl) {
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
    public static android.car.storagemonitoring.ICarStorageMonitoring getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
     * Returns the value of the PRE_EOL register.
     */
  public int getPreEolIndicatorStatus() throws android.os.RemoteException;
  /**
     * Returns the current wear estimate indicators.
     */
  public android.car.storagemonitoring.WearEstimate getWearEstimate() throws android.os.RemoteException;
  /**
     * Returns the list of all observed wear estimate changes.
     */
  public java.util.List<android.car.storagemonitoring.WearEstimateChange> getWearEstimateHistory() throws android.os.RemoteException;
  /**
     * Returns I/O stats as collected at service boot time.
     */
  public java.util.List<android.car.storagemonitoring.IoStatsEntry> getBootIoStats() throws android.os.RemoteException;
  /**
     * Returns total I/O stats as collected from kernel start until the last snapshot.
     */
  public java.util.List<android.car.storagemonitoring.IoStatsEntry> getAggregateIoStats() throws android.os.RemoteException;
  /**
     * Return the I/O stats deltas currently known to the service.
     */
  public java.util.List<android.car.storagemonitoring.IoStats> getIoStatsDeltas() throws android.os.RemoteException;
  /**
     * Register a new listener to receive new I/O activity deltas as they are generated.
     */
  public void registerListener(android.car.storagemonitoring.IIoStatsListener listener) throws android.os.RemoteException;
  /**
     * Remove a listener registration, terminating delivery of I/O activity deltas to it.
     */
  public void unregisterListener(android.car.storagemonitoring.IIoStatsListener listener) throws android.os.RemoteException;
  /**
     * Returns the approximate amount of bytes written to disk during the previous shutdown.
     */
  public long getShutdownDiskWriteAmount() throws android.os.RemoteException;
}
