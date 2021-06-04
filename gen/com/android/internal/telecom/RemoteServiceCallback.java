/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.telecom;
/**
 * Simple response callback object.
 *
 * {@hide}
 */
public interface RemoteServiceCallback extends android.os.IInterface
{
  /** Default implementation for RemoteServiceCallback. */
  public static class Default implements com.android.internal.telecom.RemoteServiceCallback
  {
    @Override public void onError() throws android.os.RemoteException
    {
    }
    @Override public void onResult(java.util.List<android.content.ComponentName> components, java.util.List<android.os.IBinder> callServices) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.telecom.RemoteServiceCallback
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.telecom.RemoteServiceCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.telecom.RemoteServiceCallback interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.telecom.RemoteServiceCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.telecom.RemoteServiceCallback))) {
        return ((com.android.internal.telecom.RemoteServiceCallback)iin);
      }
      return new com.android.internal.telecom.RemoteServiceCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onError:
        {
          data.enforceInterface(descriptor);
          this.onError();
          return true;
        }
        case TRANSACTION_onResult:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.content.ComponentName> _arg0;
          _arg0 = data.createTypedArrayList(android.content.ComponentName.CREATOR);
          java.util.List<android.os.IBinder> _arg1;
          _arg1 = data.createBinderArrayList();
          this.onResult(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.telecom.RemoteServiceCallback
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
      @Override public void onError() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onError, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onError();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onResult(java.util.List<android.content.ComponentName> components, java.util.List<android.os.IBinder> callServices) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(components);
          _data.writeBinderList(callServices);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onResult, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onResult(components, callServices);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.telecom.RemoteServiceCallback sDefaultImpl;
    }
    static final int TRANSACTION_onError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onResult = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(com.android.internal.telecom.RemoteServiceCallback impl) {
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
    public static com.android.internal.telecom.RemoteServiceCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onError() throws android.os.RemoteException;
  public void onResult(java.util.List<android.content.ComponentName> components, java.util.List<android.os.IBinder> callServices) throws android.os.RemoteException;
}
