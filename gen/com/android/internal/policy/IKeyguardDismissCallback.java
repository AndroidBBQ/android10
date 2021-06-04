/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.policy;
public interface IKeyguardDismissCallback extends android.os.IInterface
{
  /** Default implementation for IKeyguardDismissCallback. */
  public static class Default implements com.android.internal.policy.IKeyguardDismissCallback
  {
    @Override public void onDismissError() throws android.os.RemoteException
    {
    }
    @Override public void onDismissSucceeded() throws android.os.RemoteException
    {
    }
    @Override public void onDismissCancelled() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.policy.IKeyguardDismissCallback
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.policy.IKeyguardDismissCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.policy.IKeyguardDismissCallback interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.policy.IKeyguardDismissCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.policy.IKeyguardDismissCallback))) {
        return ((com.android.internal.policy.IKeyguardDismissCallback)iin);
      }
      return new com.android.internal.policy.IKeyguardDismissCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onDismissError:
        {
          data.enforceInterface(descriptor);
          this.onDismissError();
          return true;
        }
        case TRANSACTION_onDismissSucceeded:
        {
          data.enforceInterface(descriptor);
          this.onDismissSucceeded();
          return true;
        }
        case TRANSACTION_onDismissCancelled:
        {
          data.enforceInterface(descriptor);
          this.onDismissCancelled();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.policy.IKeyguardDismissCallback
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
      @Override public void onDismissError() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDismissError, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDismissError();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDismissSucceeded() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDismissSucceeded, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDismissSucceeded();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onDismissCancelled() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onDismissCancelled, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onDismissCancelled();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.policy.IKeyguardDismissCallback sDefaultImpl;
    }
    static final int TRANSACTION_onDismissError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onDismissSucceeded = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onDismissCancelled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(com.android.internal.policy.IKeyguardDismissCallback impl) {
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
    public static com.android.internal.policy.IKeyguardDismissCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onDismissError() throws android.os.RemoteException;
  public void onDismissSucceeded() throws android.os.RemoteException;
  public void onDismissCancelled() throws android.os.RemoteException;
}
