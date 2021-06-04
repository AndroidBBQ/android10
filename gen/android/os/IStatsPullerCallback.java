/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.os;
/**
  * Binder interface to pull atoms for the stats service.
  * {@hide}
  */
public interface IStatsPullerCallback extends android.os.IInterface
{
  /** Default implementation for IStatsPullerCallback. */
  public static class Default implements android.os.IStatsPullerCallback
  {
    /**
         * Pull data for the specified atom tag. Returns an array of StatsLogEventWrapper containing
         * the data.
         *
         * Note: These pulled atoms should not have uid/attribution chain. Additionally, the event
         * timestamps will be truncated to the nearest 5 minutes.
         */
    @Override public android.os.StatsLogEventWrapper[] pullData(int atomTag, long elapsedNanos, long wallClocknanos) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.os.IStatsPullerCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.os.IStatsPullerCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.os.IStatsPullerCallback interface,
     * generating a proxy if needed.
     */
    public static android.os.IStatsPullerCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.os.IStatsPullerCallback))) {
        return ((android.os.IStatsPullerCallback)iin);
      }
      return new android.os.IStatsPullerCallback.Stub.Proxy(obj);
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
          long _arg1;
          _arg1 = data.readLong();
          long _arg2;
          _arg2 = data.readLong();
          android.os.StatsLogEventWrapper[] _result = this.pullData(_arg0, _arg1, _arg2);
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
    private static class Proxy implements android.os.IStatsPullerCallback
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
           * Pull data for the specified atom tag. Returns an array of StatsLogEventWrapper containing
           * the data.
           *
           * Note: These pulled atoms should not have uid/attribution chain. Additionally, the event
           * timestamps will be truncated to the nearest 5 minutes.
           */
      @Override public android.os.StatsLogEventWrapper[] pullData(int atomTag, long elapsedNanos, long wallClocknanos) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.StatsLogEventWrapper[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(atomTag);
          _data.writeLong(elapsedNanos);
          _data.writeLong(wallClocknanos);
          boolean _status = mRemote.transact(Stub.TRANSACTION_pullData, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().pullData(atomTag, elapsedNanos, wallClocknanos);
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
      public static android.os.IStatsPullerCallback sDefaultImpl;
    }
    static final int TRANSACTION_pullData = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.os.IStatsPullerCallback impl) {
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
    public static android.os.IStatsPullerCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Pull data for the specified atom tag. Returns an array of StatsLogEventWrapper containing
       * the data.
       *
       * Note: These pulled atoms should not have uid/attribution chain. Additionally, the event
       * timestamps will be truncated to the nearest 5 minutes.
       */
  public android.os.StatsLogEventWrapper[] pullData(int atomTag, long elapsedNanos, long wallClocknanos) throws android.os.RemoteException;
}
