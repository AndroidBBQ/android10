/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.internal.view;
/**
 * Helper interface for IInputMethod to allow the input method to notify the client when a new
 * session has been created.
 * {@hide}
 */
public interface IInputSessionCallback extends android.os.IInterface
{
  /** Default implementation for IInputSessionCallback. */
  public static class Default implements com.android.internal.view.IInputSessionCallback
  {
    @Override public void sessionCreated(com.android.internal.view.IInputMethodSession session) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.internal.view.IInputSessionCallback
  {
    private static final java.lang.String DESCRIPTOR = "com.android.internal.view.IInputSessionCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.internal.view.IInputSessionCallback interface,
     * generating a proxy if needed.
     */
    public static com.android.internal.view.IInputSessionCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.internal.view.IInputSessionCallback))) {
        return ((com.android.internal.view.IInputSessionCallback)iin);
      }
      return new com.android.internal.view.IInputSessionCallback.Stub.Proxy(obj);
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
        case TRANSACTION_sessionCreated:
        {
          data.enforceInterface(descriptor);
          com.android.internal.view.IInputMethodSession _arg0;
          _arg0 = com.android.internal.view.IInputMethodSession.Stub.asInterface(data.readStrongBinder());
          this.sessionCreated(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.internal.view.IInputSessionCallback
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
      @Override public void sessionCreated(com.android.internal.view.IInputMethodSession session) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((session!=null))?(session.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_sessionCreated, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().sessionCreated(session);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.internal.view.IInputSessionCallback sDefaultImpl;
    }
    static final int TRANSACTION_sessionCreated = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(com.android.internal.view.IInputSessionCallback impl) {
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
    public static com.android.internal.view.IInputSessionCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void sessionCreated(com.android.internal.view.IInputMethodSession session) throws android.os.RemoteException;
}
