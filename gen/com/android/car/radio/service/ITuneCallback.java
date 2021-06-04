/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.car.radio.service;
/**
 * Asynchronous result for tune/seek operation.
 */
public interface ITuneCallback extends android.os.IInterface
{
  /** Default implementation for ITuneCallback. */
  public static class Default implements com.android.car.radio.service.ITuneCallback
  {
    /**
         * Called when tune operation has finished.
         *
         * @param succeeded States whether operation has succeeded or not.
         */
    @Override public void onFinished(boolean succeeded) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.car.radio.service.ITuneCallback
  {
    private static final java.lang.String DESCRIPTOR = "com.android.car.radio.service.ITuneCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.car.radio.service.ITuneCallback interface,
     * generating a proxy if needed.
     */
    public static com.android.car.radio.service.ITuneCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.car.radio.service.ITuneCallback))) {
        return ((com.android.car.radio.service.ITuneCallback)iin);
      }
      return new com.android.car.radio.service.ITuneCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onFinished:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          this.onFinished(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.car.radio.service.ITuneCallback
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
           * Called when tune operation has finished.
           *
           * @param succeeded States whether operation has succeeded or not.
           */
      @Override public void onFinished(boolean succeeded) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((succeeded)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_onFinished, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onFinished(succeeded);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.car.radio.service.ITuneCallback sDefaultImpl;
    }
    static final int TRANSACTION_onFinished = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(com.android.car.radio.service.ITuneCallback impl) {
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
    public static com.android.car.radio.service.ITuneCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Called when tune operation has finished.
       *
       * @param succeeded States whether operation has succeeded or not.
       */
  public void onFinished(boolean succeeded) throws android.os.RemoteException;
}
