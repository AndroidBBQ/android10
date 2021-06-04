/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.car;
/**
 * Helper API for car service. Only for itneraction between system server and car service.
 * @hide
 */
public interface ICarServiceHelper extends android.os.IInterface
{
  /** Default implementation for ICarServiceHelper. */
  public static class Default implements com.android.internal.car.ICarServiceHelper
  {
    @Override public int forceSuspend(int timeoutMs) throws android.os.RemoteException
    {
      return 0;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.car.ICarServiceHelper
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.car.ICarServiceHelper";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.car.ICarServiceHelper interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.car.ICarServiceHelper asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.car.ICarServiceHelper))) {
        return ((com.android.internal.car.ICarServiceHelper)iin);
      }
      return new com.android.internal.car.ICarServiceHelper.Stub.Proxy(obj);
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
        case TRANSACTION_forceSuspend:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.forceSuspend(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.car.ICarServiceHelper
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
      @Override public int forceSuspend(int timeoutMs) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(timeoutMs);
          boolean _status = mRemote.transact(Stub.TRANSACTION_forceSuspend, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().forceSuspend(timeoutMs);
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
      public static com.android.internal.car.ICarServiceHelper sDefaultImpl;
    }
    static final int TRANSACTION_forceSuspend = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(com.android.internal.car.ICarServiceHelper impl) {
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
    public static com.android.internal.car.ICarServiceHelper getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public int forceSuspend(int timeoutMs) throws android.os.RemoteException;
}
