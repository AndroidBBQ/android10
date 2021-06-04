/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.car.procfsinspector;
public interface IProcfsInspector extends android.os.IInterface
{
  /** Default implementation for IProcfsInspector. */
  public static class Default implements com.android.car.procfsinspector.IProcfsInspector
  {
    @Override public java.util.List<com.android.car.procfsinspector.ProcessInfo> readProcessTable() throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.car.procfsinspector.IProcfsInspector
  {
    private static final java.lang.String DESCRIPTOR = "com.android.car.procfsinspector.IProcfsInspector";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.car.procfsinspector.IProcfsInspector interface,
     * generating a proxy if needed.
     */
    public static com.android.car.procfsinspector.IProcfsInspector asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.car.procfsinspector.IProcfsInspector))) {
        return ((com.android.car.procfsinspector.IProcfsInspector)iin);
      }
      return new com.android.car.procfsinspector.IProcfsInspector.Stub.Proxy(obj);
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
        case TRANSACTION_readProcessTable:
        {
          data.enforceInterface(descriptor);
          java.util.List<com.android.car.procfsinspector.ProcessInfo> _result = this.readProcessTable();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.car.procfsinspector.IProcfsInspector
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
      @Override public java.util.List<com.android.car.procfsinspector.ProcessInfo> readProcessTable() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<com.android.car.procfsinspector.ProcessInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_readProcessTable, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().readProcessTable();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(com.android.car.procfsinspector.ProcessInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static com.android.car.procfsinspector.IProcfsInspector sDefaultImpl;
    }
    static final int TRANSACTION_readProcessTable = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(com.android.car.procfsinspector.IProcfsInspector impl) {
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
    public static com.android.car.procfsinspector.IProcfsInspector getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public java.util.List<com.android.car.procfsinspector.ProcessInfo> readProcessTable() throws android.os.RemoteException;
}
