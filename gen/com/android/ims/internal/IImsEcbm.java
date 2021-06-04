/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.ims.internal;
/**
 * Provides the ECBM interface
 *
 * {@hide}
 */
public interface IImsEcbm extends android.os.IInterface
{
  /** Default implementation for IImsEcbm. */
  public static class Default implements com.android.ims.internal.IImsEcbm
  {
    /**
         * Sets the listener.
         */
    @Override public void setListener(com.android.ims.internal.IImsEcbmListener listener) throws android.os.RemoteException
    {
    }
    /**
         * Requests Modem to come out of ECBM mode
         */
    @Override public void exitEmergencyCallbackMode() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.ims.internal.IImsEcbm
  {
    private static final java.lang.String DESCRIPTOR = "com.android.ims.internal.IImsEcbm";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.ims.internal.IImsEcbm interface,
     * generating a proxy if needed.
     */
    public static com.android.ims.internal.IImsEcbm asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.ims.internal.IImsEcbm))) {
        return ((com.android.ims.internal.IImsEcbm)iin);
      }
      return new com.android.ims.internal.IImsEcbm.Stub.Proxy(obj);
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
        case TRANSACTION_setListener:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.IImsEcbmListener _arg0;
          _arg0 = com.android.ims.internal.IImsEcbmListener.Stub.asInterface(data.readStrongBinder());
          this.setListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_exitEmergencyCallbackMode:
        {
          data.enforceInterface(descriptor);
          this.exitEmergencyCallbackMode();
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.ims.internal.IImsEcbm
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
      /**
           * Sets the listener.
           */
      @Override public void setListener(com.android.ims.internal.IImsEcbmListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Requests Modem to come out of ECBM mode
           */
      @Override public void exitEmergencyCallbackMode() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_exitEmergencyCallbackMode, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().exitEmergencyCallbackMode();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static com.android.ims.internal.IImsEcbm sDefaultImpl;
    }
    static final int TRANSACTION_setListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_exitEmergencyCallbackMode = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(com.android.ims.internal.IImsEcbm impl) {
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
    public static com.android.ims.internal.IImsEcbm getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Sets the listener.
       */
  public void setListener(com.android.ims.internal.IImsEcbmListener listener) throws android.os.RemoteException;
  /**
       * Requests Modem to come out of ECBM mode
       */
  public void exitEmergencyCallbackMode() throws android.os.RemoteException;
}
