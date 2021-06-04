/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.telephony;
public interface INumberVerificationCallback extends android.os.IInterface
{
  /** Default implementation for INumberVerificationCallback. */
  public static class Default implements com.android.internal.telephony.INumberVerificationCallback
  {
    @Override public void onCallReceived(java.lang.String phoneNumber) throws android.os.RemoteException
    {
    }
    @Override public void onVerificationFailed(int reason) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.telephony.INumberVerificationCallback
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.telephony.INumberVerificationCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.telephony.INumberVerificationCallback interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.telephony.INumberVerificationCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.telephony.INumberVerificationCallback))) {
        return ((com.android.internal.telephony.INumberVerificationCallback)iin);
      }
      return new com.android.internal.telephony.INumberVerificationCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onCallReceived:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          this.onCallReceived(_arg0);
          return true;
        }
        case TRANSACTION_onVerificationFailed:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onVerificationFailed(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.telephony.INumberVerificationCallback
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
      @Override public void onCallReceived(java.lang.String phoneNumber) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(phoneNumber);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onCallReceived, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onCallReceived(phoneNumber);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onVerificationFailed(int reason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(reason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onVerificationFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onVerificationFailed(reason);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.telephony.INumberVerificationCallback sDefaultImpl;
    }
    static final int TRANSACTION_onCallReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onVerificationFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(com.android.internal.telephony.INumberVerificationCallback impl) {
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
    public static com.android.internal.telephony.INumberVerificationCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onCallReceived(java.lang.String phoneNumber) throws android.os.RemoteException;
  public void onVerificationFailed(int reason) throws android.os.RemoteException;
}
