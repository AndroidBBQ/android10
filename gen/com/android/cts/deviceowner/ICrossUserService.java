/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.cts.deviceowner;
public interface ICrossUserService extends android.os.IInterface
{
  /** Default implementation for ICrossUserService. */
  public static class Default implements com.android.cts.deviceowner.ICrossUserService
  {
    @Override public void onEnabledCalled(java.lang.String error) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.cts.deviceowner.ICrossUserService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.cts.deviceowner.ICrossUserService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.cts.deviceowner.ICrossUserService interface,
     * generating a proxy if needed.
     */
    public static com.android.cts.deviceowner.ICrossUserService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.cts.deviceowner.ICrossUserService))) {
        return ((com.android.cts.deviceowner.ICrossUserService)iin);
      }
      return new com.android.cts.deviceowner.ICrossUserService.Stub.Proxy(obj);
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
        case TRANSACTION_onEnabledCalled:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.onEnabledCalled(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.cts.deviceowner.ICrossUserService
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
      @Override public void onEnabledCalled(java.lang.String error) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(error);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onEnabledCalled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onEnabledCalled(error);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.cts.deviceowner.ICrossUserService sDefaultImpl;
    }
    static final int TRANSACTION_onEnabledCalled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(com.android.cts.deviceowner.ICrossUserService impl) {
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
    public static com.android.cts.deviceowner.ICrossUserService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onEnabledCalled(java.lang.String error) throws android.os.RemoteException;
}
