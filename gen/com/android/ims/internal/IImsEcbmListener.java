/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.ims.internal;
/**
 * A listener type for receiving notifications about the changes to
 * Emergency Callback Mode through IMS.
 *
 * {@hide}
 */
public interface IImsEcbmListener extends android.os.IInterface
{
  /** Default implementation for IImsEcbmListener. */
  public static class Default implements com.android.ims.internal.IImsEcbmListener
  {
    /**
         * Notifies the application when the device enters Emergency Callback Mode.
         */
    @Override public void enteredECBM() throws android.os.RemoteException
    {
    }
    /**
         * Notifies the application when the device exits Emergency Callback Mode.
         */
    @Override public void exitedECBM() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.ims.internal.IImsEcbmListener
  {
    private static final java.lang.String DESCRIPTOR = "com.android.ims.internal.IImsEcbmListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.ims.internal.IImsEcbmListener interface,
     * generating a proxy if needed.
     */
    public static com.android.ims.internal.IImsEcbmListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.ims.internal.IImsEcbmListener))) {
        return ((com.android.ims.internal.IImsEcbmListener)iin);
      }
      return new com.android.ims.internal.IImsEcbmListener.Stub.Proxy(obj);
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
        case TRANSACTION_enteredECBM:
        {
          data.enforceInterface(descriptor);
          this.enteredECBM();
          return true;
        }
        case TRANSACTION_exitedECBM:
        {
          data.enforceInterface(descriptor);
          this.exitedECBM();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.ims.internal.IImsEcbmListener
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
           * Notifies the application when the device enters Emergency Callback Mode.
           */
      @Override public void enteredECBM() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enteredECBM, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enteredECBM();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Notifies the application when the device exits Emergency Callback Mode.
           */
      @Override public void exitedECBM() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_exitedECBM, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().exitedECBM();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.ims.internal.IImsEcbmListener sDefaultImpl;
    }
    static final int TRANSACTION_enteredECBM = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_exitedECBM = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(com.android.ims.internal.IImsEcbmListener impl) {
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
    public static com.android.ims.internal.IImsEcbmListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Notifies the application when the device enters Emergency Callback Mode.
       */
  public void enteredECBM() throws android.os.RemoteException;
  /**
       * Notifies the application when the device exits Emergency Callback Mode.
       */
  public void exitedECBM() throws android.os.RemoteException;
}
