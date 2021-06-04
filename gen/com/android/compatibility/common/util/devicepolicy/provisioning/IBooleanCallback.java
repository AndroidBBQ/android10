/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.compatibility.common.util.devicepolicy.provisioning;
public interface IBooleanCallback extends android.os.IInterface
{
  /** Default implementation for IBooleanCallback. */
  public static class Default implements com.android.compatibility.common.util.devicepolicy.provisioning.IBooleanCallback
  {
    @Override public void onResult(boolean result) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.compatibility.common.util.devicepolicy.provisioning.IBooleanCallback
  {
    private static final java.lang.String DESCRIPTOR = "com.android.compatibility.common.util.devicepolicy.provisioning.IBooleanCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.compatibility.common.util.devicepolicy.provisioning.IBooleanCallback interface,
     * generating a proxy if needed.
     */
    public static com.android.compatibility.common.util.devicepolicy.provisioning.IBooleanCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.compatibility.common.util.devicepolicy.provisioning.IBooleanCallback))) {
        return ((com.android.compatibility.common.util.devicepolicy.provisioning.IBooleanCallback)iin);
      }
      return new com.android.compatibility.common.util.devicepolicy.provisioning.IBooleanCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onResult:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onResult(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.compatibility.common.util.devicepolicy.provisioning.IBooleanCallback
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
      @Override public void onResult(boolean result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((result)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onResult(result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.compatibility.common.util.devicepolicy.provisioning.IBooleanCallback sDefaultImpl;
    }
    static final int TRANSACTION_onResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(com.android.compatibility.common.util.devicepolicy.provisioning.IBooleanCallback impl) {
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
    public static com.android.compatibility.common.util.devicepolicy.provisioning.IBooleanCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onResult(boolean result) throws android.os.RemoteException;
}
