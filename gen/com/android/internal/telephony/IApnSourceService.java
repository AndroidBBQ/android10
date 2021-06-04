/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.telephony;
public interface IApnSourceService extends android.os.IInterface
{
  /** Default implementation for IApnSourceService. */
  public static class Default implements com.android.internal.telephony.IApnSourceService
  {
    /** Retreive APNs. */
    @Override public android.content.ContentValues[] getApns(int subId) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.telephony.IApnSourceService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.telephony.IApnSourceService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.telephony.IApnSourceService interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.telephony.IApnSourceService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.telephony.IApnSourceService))) {
        return ((com.android.internal.telephony.IApnSourceService)iin);
      }
      return new com.android.internal.telephony.IApnSourceService.Stub.Proxy(obj);
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
        case TRANSACTION_getApns:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.content.ContentValues[] _result = this.getApns(_arg0);
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
    private static class Proxy implements com.android.internal.telephony.IApnSourceService
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
      /** Retreive APNs. */
      @Override public android.content.ContentValues[] getApns(int subId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.content.ContentValues[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(subId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getApns, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getApns(subId);
          }
          _reply.readException();
          _result = _reply.createTypedArray(android.content.ContentValues.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static com.android.internal.telephony.IApnSourceService sDefaultImpl;
    }
    static final int TRANSACTION_getApns = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(com.android.internal.telephony.IApnSourceService impl) {
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
    public static com.android.internal.telephony.IApnSourceService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /** Retreive APNs. */
  public android.content.ContentValues[] getApns(int subId) throws android.os.RemoteException;
}
