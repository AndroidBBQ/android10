/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.car;
/**
 * Interface for pulling statsd atoms from automotive devices.
 *
 * @hide
 */
public interface ICarStatsService extends android.os.IInterface
{
  /** Default implementation for ICarStatsService. */
  public static class Default implements com.android.internal.car.ICarStatsService
  {
    /**
         * Pull the specified atom. Results will be sent to statsd when complete.
         */
    @Override public android.os.StatsLogEventWrapper[] pullData(int atomId) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.car.ICarStatsService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.car.ICarStatsService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.car.ICarStatsService interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.car.ICarStatsService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.car.ICarStatsService))) {
        return ((com.android.internal.car.ICarStatsService)iin);
      }
      return new com.android.internal.car.ICarStatsService.Stub.Proxy(obj);
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
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.car.ICarStatsService
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
           * Pull the specified atom. Results will be sent to statsd when complete.
           */
      @Override public android.os.StatsLogEventWrapper[] pullData(int atomId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.StatsLogEventWrapper[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(atomId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_pullData, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().pullData(atomId);
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
      public static com.android.internal.car.ICarStatsService sDefaultImpl;
    }
    static final int TRANSACTION_pullData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(com.android.internal.car.ICarStatsService impl) {
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
    public static com.android.internal.car.ICarStatsService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Pull the specified atom. Results will be sent to statsd when complete.
       */
  public android.os.StatsLogEventWrapper[] pullData(int atomId) throws android.os.RemoteException;
}
