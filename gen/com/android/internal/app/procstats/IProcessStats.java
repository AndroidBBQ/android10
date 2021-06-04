/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.app.procstats;
public interface IProcessStats extends android.os.IInterface
{
  /** Default implementation for IProcessStats. */
  public static class Default implements com.android.internal.app.procstats.IProcessStats
  {
    @Override public byte[] getCurrentStats(java.util.List<android.os.ParcelFileDescriptor> historic) throws android.os.RemoteException
    {
      return null;
    }
    @Override public android.os.ParcelFileDescriptor getStatsOverTime(long minTime) throws android.os.RemoteException
    {
      return null;
    }
    @Override public int getCurrentMemoryState() throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Get stats committed after highWaterMarkMs
         * @param highWaterMarkMs Report stats committed after this time.
         * @param section Integer mask to indicate which sections to include in the stats.
         * @param doAggregate Whether to aggregate the stats or keep them separated.
         * @param List of Files of individual commits in protobuf binary or one that is merged from them.
         */
    @Override public long getCommittedStats(long highWaterMarkMs, int section, boolean doAggregate, java.util.List<android.os.ParcelFileDescriptor> committedStats) throws android.os.RemoteException
    {
      return 0L;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.app.procstats.IProcessStats
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.app.procstats.IProcessStats";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.app.procstats.IProcessStats interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.app.procstats.IProcessStats asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.app.procstats.IProcessStats))) {
        return ((com.android.internal.app.procstats.IProcessStats)iin);
      }
      return new com.android.internal.app.procstats.IProcessStats.Stub.Proxy(obj);
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
        case TRANSACTION_getCurrentStats:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.os.ParcelFileDescriptor> _arg0;
          _arg0 = new java.util.ArrayList<android.os.ParcelFileDescriptor>();
          byte[] _result = this.getCurrentStats(_arg0);
          reply.writeNoException();
          reply.writeByteArray(_result);
          return true;
        }
        case TRANSACTION_getStatsOverTime:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          android.os.ParcelFileDescriptor _result = this.getStatsOverTime(_arg0);
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
        case TRANSACTION_getCurrentMemoryState:
        {
          data.enforceInterface(descriptor);
          int _result = this.getCurrentMemoryState();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getCommittedStats:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          java.util.List<android.os.ParcelFileDescriptor> _arg3;
          _arg3 = new java.util.ArrayList<android.os.ParcelFileDescriptor>();
          long _result = this.getCommittedStats(_arg0, _arg1, _arg2, _arg3);
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
    private static class Proxy implements com.android.internal.app.procstats.IProcessStats
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
      @Override public byte[] getCurrentStats(java.util.List<android.os.ParcelFileDescriptor> historic) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        byte[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentStats, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentStats(historic);
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
      @Override public android.os.ParcelFileDescriptor getStatsOverTime(long minTime) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.ParcelFileDescriptor _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(minTime);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getStatsOverTime, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getStatsOverTime(minTime);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(_reply);
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
      @Override public int getCurrentMemoryState() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentMemoryState, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentMemoryState();
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
           * Get stats committed after highWaterMarkMs
           * @param highWaterMarkMs Report stats committed after this time.
           * @param section Integer mask to indicate which sections to include in the stats.
           * @param doAggregate Whether to aggregate the stats or keep them separated.
           * @param List of Files of individual commits in protobuf binary or one that is merged from them.
           */
      @Override public long getCommittedStats(long highWaterMarkMs, int section, boolean doAggregate, java.util.List<android.os.ParcelFileDescriptor> committedStats) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(highWaterMarkMs);
          _data.writeInt(section);
          _data.writeInt(((doAggregate)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCommittedStats, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCommittedStats(highWaterMarkMs, section, doAggregate, committedStats);
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
      public static com.android.internal.app.procstats.IProcessStats sDefaultImpl;
    }
    static final int TRANSACTION_getCurrentStats = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getStatsOverTime = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getCurrentMemoryState = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getCommittedStats = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(com.android.internal.app.procstats.IProcessStats impl) {
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
    public static com.android.internal.app.procstats.IProcessStats getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public byte[] getCurrentStats(java.util.List<android.os.ParcelFileDescriptor> historic) throws android.os.RemoteException;
  public android.os.ParcelFileDescriptor getStatsOverTime(long minTime) throws android.os.RemoteException;
  public int getCurrentMemoryState() throws android.os.RemoteException;
  /**
       * Get stats committed after highWaterMarkMs
       * @param highWaterMarkMs Report stats committed after this time.
       * @param section Integer mask to indicate which sections to include in the stats.
       * @param doAggregate Whether to aggregate the stats or keep them separated.
       * @param List of Files of individual commits in protobuf binary or one that is merged from them.
       */
  public long getCommittedStats(long highWaterMarkMs, int section, boolean doAggregate, java.util.List<android.os.ParcelFileDescriptor> committedStats) throws android.os.RemoteException;
}
