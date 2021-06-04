/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.telecom;
/**
 * Internal remote interface for a call screening service.
 * @see android.telecom.CallScreeningService
 * @hide
 */
public interface ICallScreeningService extends android.os.IInterface
{
  /** Default implementation for ICallScreeningService. */
  public static class Default implements com.android.internal.telecom.ICallScreeningService
  {
    @Override public void screenCall(com.android.internal.telecom.ICallScreeningAdapter adapter, android.telecom.ParcelableCall call) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.telecom.ICallScreeningService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.telecom.ICallScreeningService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.telecom.ICallScreeningService interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.telecom.ICallScreeningService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.telecom.ICallScreeningService))) {
        return ((com.android.internal.telecom.ICallScreeningService)iin);
      }
      return new com.android.internal.telecom.ICallScreeningService.Stub.Proxy(obj);
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
        case TRANSACTION_screenCall:
        {
          data.enforceInterface(descriptor);
          com.android.internal.telecom.ICallScreeningAdapter _arg0;
          _arg0 = com.android.internal.telecom.ICallScreeningAdapter.Stub.asInterface(data.readStrongBinder());
          android.telecom.ParcelableCall _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.ParcelableCall.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.screenCall(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.telecom.ICallScreeningService
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
      @Override public void screenCall(com.android.internal.telecom.ICallScreeningAdapter adapter, android.telecom.ParcelableCall call) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((adapter!=null))?(adapter.asBinder()):(null)));
          if ((call!=null)) {
            _data.writeInt(1);
            call.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_screenCall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().screenCall(adapter, call);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.telecom.ICallScreeningService sDefaultImpl;
    }
    static final int TRANSACTION_screenCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(com.android.internal.telecom.ICallScreeningService impl) {
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
    public static com.android.internal.telecom.ICallScreeningService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void screenCall(com.android.internal.telecom.ICallScreeningAdapter adapter, android.telecom.ParcelableCall call) throws android.os.RemoteException;
}
