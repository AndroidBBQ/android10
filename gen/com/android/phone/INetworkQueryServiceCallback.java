/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.phone;
/**
 * Service interface to handle callbacks into the activity from the
 * NetworkQueryService.  These objects are used to notify that a
 * query is complete and that the results are ready to process.
 */
public interface INetworkQueryServiceCallback extends android.os.IInterface
{
  /** Default implementation for INetworkQueryServiceCallback. */
  public static class Default implements com.android.phone.INetworkQueryServiceCallback
  {
    /**
         * Returns the scan results to the user, this callback will be
         * called at least one time.
         */
    @Override public void onResults(java.util.List<android.telephony.CellInfo> results) throws android.os.RemoteException
    {
    }
    /**
         * Informs the user that the scan has stopped.
         *
         * This callback will be called when the scan is finished or cancelled by the user.
         * The related NetworkScanRequest will be deleted after this callback.
         */
    @Override public void onComplete() throws android.os.RemoteException
    {
    }
    /**
         * Informs the user that there is some error about the scan.
         *
         * This callback will be called whenever there is any error about the scan,
         * and the scan will be terminated. onComplete() will NOT be called.
         */
    @Override public void onError(int error) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.phone.INetworkQueryServiceCallback
  {
    private static final java.lang.String DESCRIPTOR = "com.android.phone.INetworkQueryServiceCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.phone.INetworkQueryServiceCallback interface,
     * generating a proxy if needed.
     */
    public static com.android.phone.INetworkQueryServiceCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.phone.INetworkQueryServiceCallback))) {
        return ((com.android.phone.INetworkQueryServiceCallback)iin);
      }
      return new com.android.phone.INetworkQueryServiceCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onResults:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.telephony.CellInfo> _arg0;
          _arg0 = data.createTypedArrayList(android.telephony.CellInfo.CREATOR);
          this.onResults(_arg0);
          return true;
        }
        case TRANSACTION_onComplete:
        {
          data.enforceInterface(descriptor);
          this.onComplete();
          return true;
        }
        case TRANSACTION_onError:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onError(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.phone.INetworkQueryServiceCallback
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
           * Returns the scan results to the user, this callback will be
           * called at least one time.
           */
      @Override public void onResults(java.util.List<android.telephony.CellInfo> results) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedList(results);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onResults, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onResults(results);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Informs the user that the scan has stopped.
           *
           * This callback will be called when the scan is finished or cancelled by the user.
           * The related NetworkScanRequest will be deleted after this callback.
           */
      @Override public void onComplete() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onComplete();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Informs the user that there is some error about the scan.
           *
           * This callback will be called whenever there is any error about the scan,
           * and the scan will be terminated. onComplete() will NOT be called.
           */
      @Override public void onError(int error) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(error);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onError, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onError(error);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.phone.INetworkQueryServiceCallback sDefaultImpl;
    }
    static final int TRANSACTION_onResults = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onError = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(com.android.phone.INetworkQueryServiceCallback impl) {
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
    public static com.android.phone.INetworkQueryServiceCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Returns the scan results to the user, this callback will be
       * called at least one time.
       */
  public void onResults(java.util.List<android.telephony.CellInfo> results) throws android.os.RemoteException;
  /**
       * Informs the user that the scan has stopped.
       *
       * This callback will be called when the scan is finished or cancelled by the user.
       * The related NetworkScanRequest will be deleted after this callback.
       */
  public void onComplete() throws android.os.RemoteException;
  /**
       * Informs the user that there is some error about the scan.
       *
       * This callback will be called whenever there is any error about the scan,
       * and the scan will be terminated. onComplete() will NOT be called.
       */
  public void onError(int error) throws android.os.RemoteException;
}
