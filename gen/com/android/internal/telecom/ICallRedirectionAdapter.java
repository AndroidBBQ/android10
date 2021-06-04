/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.telecom;
/**
 * Internal remote callback interface for call redirection services.
 *
 * @see android.telecom.CallRedirectionService
 *
 * {@hide}
 */
public interface ICallRedirectionAdapter extends android.os.IInterface
{
  /** Default implementation for ICallRedirectionAdapter. */
  public static class Default implements com.android.internal.telecom.ICallRedirectionAdapter
  {
    @Override public void cancelCall() throws android.os.RemoteException
    {
    }
    @Override public void placeCallUnmodified() throws android.os.RemoteException
    {
    }
    @Override public void redirectCall(android.net.Uri handle, android.telecom.PhoneAccountHandle targetPhoneAccount, boolean confirmFirst) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.telecom.ICallRedirectionAdapter
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.telecom.ICallRedirectionAdapter";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.telecom.ICallRedirectionAdapter interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.telecom.ICallRedirectionAdapter asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.telecom.ICallRedirectionAdapter))) {
        return ((com.android.internal.telecom.ICallRedirectionAdapter)iin);
      }
      return new com.android.internal.telecom.ICallRedirectionAdapter.Stub.Proxy(obj);
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
        case TRANSACTION_cancelCall:
        {
          data.enforceInterface(descriptor);
          this.cancelCall();
          return true;
        }
        case TRANSACTION_placeCallUnmodified:
        {
          data.enforceInterface(descriptor);
          this.placeCallUnmodified();
          return true;
        }
        case TRANSACTION_redirectCall:
        {
          data.enforceInterface(descriptor);
          android.net.Uri _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.Uri.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.telecom.PhoneAccountHandle _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.telecom.PhoneAccountHandle.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.redirectCall(_arg0, _arg1, _arg2);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.telecom.ICallRedirectionAdapter
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
      @Override public void cancelCall() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_cancelCall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().cancelCall();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void placeCallUnmodified() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_placeCallUnmodified, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().placeCallUnmodified();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void redirectCall(android.net.Uri handle, android.telecom.PhoneAccountHandle targetPhoneAccount, boolean confirmFirst) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((handle!=null)) {
            _data.writeInt(1);
            handle.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((targetPhoneAccount!=null)) {
            _data.writeInt(1);
            targetPhoneAccount.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((confirmFirst)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_redirectCall, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().redirectCall(handle, targetPhoneAccount, confirmFirst);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.telecom.ICallRedirectionAdapter sDefaultImpl;
    }
    static final int TRANSACTION_cancelCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_placeCallUnmodified = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_redirectCall = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(com.android.internal.telecom.ICallRedirectionAdapter impl) {
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
    public static com.android.internal.telecom.ICallRedirectionAdapter getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void cancelCall() throws android.os.RemoteException;
  public void placeCallUnmodified() throws android.os.RemoteException;
  public void redirectCall(android.net.Uri handle, android.telecom.PhoneAccountHandle targetPhoneAccount, boolean confirmFirst) throws android.os.RemoteException;
}
