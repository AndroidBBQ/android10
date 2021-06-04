/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.telecom;
/**
 * Internal remote interface for a call redirection service.
 *
 * @see android.telecom.CallRedirectionService
 *
 * @hide
 */
public interface ICallRedirectionService extends android.os.IInterface
{
  /** Default implementation for ICallRedirectionService. */
  public static class Default implements com.android.internal.telecom.ICallRedirectionService
  {
    @Override public void placeCall(com.android.internal.telecom.ICallRedirectionAdapter adapter, android.net.Uri handle, android.telecom.PhoneAccountHandle initialPhoneAccount, boolean allowInteractiveResponse) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.telecom.ICallRedirectionService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.telecom.ICallRedirectionService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.telecom.ICallRedirectionService interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.telecom.ICallRedirectionService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.telecom.ICallRedirectionService))) {
        return ((com.android.internal.telecom.ICallRedirectionService)iin);
      }
      return new com.android.internal.telecom.ICallRedirectionService.Stub.Proxy(obj);
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
        case TRANSACTION_placeCall:
        {
          data.enforceInterface(descriptor);
          com.android.internal.telecom.ICallRedirectionAdapter _arg0;
          _arg0 = com.android.internal.telecom.ICallRedirectionAdapter.Stub.asInterface(data.readStrongBinder());
          android.net.Uri _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.telecom.PhoneAccountHandle _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          boolean _arg3;
          _arg3 = (0!=data.readInt());
          this.placeCall(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.telecom.ICallRedirectionService
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
      @Override public void placeCall(com.android.internal.telecom.ICallRedirectionAdapter adapter, android.net.Uri handle, android.telecom.PhoneAccountHandle initialPhoneAccount, boolean allowInteractiveResponse) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((adapter!=null))?(adapter.asBinder()):(null)));
          if ((handle!=null)) {
            _data.writeInt(1);
            handle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((initialPhoneAccount!=null)) {
            _data.writeInt(1);
            initialPhoneAccount.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((allowInteractiveResponse)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_placeCall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().placeCall(adapter, handle, initialPhoneAccount, allowInteractiveResponse);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.telecom.ICallRedirectionService sDefaultImpl;
    }
    static final int TRANSACTION_placeCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(com.android.internal.telecom.ICallRedirectionService impl) {
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
    public static com.android.internal.telecom.ICallRedirectionService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void placeCall(com.android.internal.telecom.ICallRedirectionAdapter adapter, android.net.Uri handle, android.telecom.PhoneAccountHandle initialPhoneAccount, boolean allowInteractiveResponse) throws android.os.RemoteException;
}
