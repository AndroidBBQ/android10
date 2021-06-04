/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.phone;
/**
 * Service interface to handle queries for available networks.  The
 * Phone application lets this service interface handle carrier 
 * availability queries instead of making direct calls to the Phone layer.
 */
public interface INetworkQueryService extends android.os.IInterface
{
  /** Default implementation for INetworkQueryService. */
  public static class Default implements com.android.phone.INetworkQueryService
  {
    /**
         * Starts a network query if it has not been started yet, and
         * request a callback through the INetworkQueryServiceCallback
         * object on query completion.  If there is an existing request,
         * then just add the callback to the list of notifications
         * that will be sent upon query completion.
         *
         * It will send the network query with the use of
         * <code>TelephonyManager.requestNetworkScan()</code> if the
         * isIncrementalResult is true. And if the isIncrementalResult
         * is set as false, it will try to send network query through
         * <code>Phone.getAvailableNetworks()</code>.
         */
    @Override public void startNetworkQuery(com.android.phone.INetworkQueryServiceCallback cb, int phoneId, boolean isIncrementalResult) throws android.os.RemoteException
    {
    }
    /**
         * Tells the service that the requested query is to be ignored.
         * This may not do anything for the Query request in the
         * underlying RIL, but it ensures that the callback is removed
         * from the list of notifications.
         */
    @Override public void stopNetworkQuery() throws android.os.RemoteException
    {
    }
    /**
         * Tells the service to unregister the network query callback.
         * Will not attempt to stop an ongoing network query.
         * Functionally may be the same as stopNetworkQuery since that
         * function also does not stop a query request in the underlying
         * RIL.
         */
    @Override public void unregisterCallback(com.android.phone.INetworkQueryServiceCallback cb) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.phone.INetworkQueryService
  {
    private static final java.lang.String DESCRIPTOR = "com.android.phone.INetworkQueryService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.phone.INetworkQueryService interface,
     * generating a proxy if needed.
     */
    public static com.android.phone.INetworkQueryService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.phone.INetworkQueryService))) {
        return ((com.android.phone.INetworkQueryService)iin);
      }
      return new com.android.phone.INetworkQueryService.Stub.Proxy(obj);
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
        case TRANSACTION_startNetworkQuery:
        {
          data.enforceInterface(descriptor);
          com.android.phone.INetworkQueryServiceCallback _arg0;
          _arg0 = com.android.phone.INetworkQueryServiceCallback.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          boolean _arg2;
          _arg2 = (0!=data.readInt());
          this.startNetworkQuery(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_stopNetworkQuery:
        {
          data.enforceInterface(descriptor);
          this.stopNetworkQuery();
          return true;
        }
        case TRANSACTION_unregisterCallback:
        {
          data.enforceInterface(descriptor);
          com.android.phone.INetworkQueryServiceCallback _arg0;
          _arg0 = com.android.phone.INetworkQueryServiceCallback.Stub.asInterface(data.readStrongBinder());
          this.unregisterCallback(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.phone.INetworkQueryService
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
           * Starts a network query if it has not been started yet, and
           * request a callback through the INetworkQueryServiceCallback
           * object on query completion.  If there is an existing request,
           * then just add the callback to the list of notifications
           * that will be sent upon query completion.
           *
           * It will send the network query with the use of
           * <code>TelephonyManager.requestNetworkScan()</code> if the
           * isIncrementalResult is true. And if the isIncrementalResult
           * is set as false, it will try to send network query through
           * <code>Phone.getAvailableNetworks()</code>.
           */
      @Override public void startNetworkQuery(com.android.phone.INetworkQueryServiceCallback cb, int phoneId, boolean isIncrementalResult) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((cb!=null))?(cb.asBinder()):(null)));
          _data.writeInt(phoneId);
          _data.writeInt(((isIncrementalResult)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_startNetworkQuery, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startNetworkQuery(cb, phoneId, isIncrementalResult);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Tells the service that the requested query is to be ignored.
           * This may not do anything for the Query request in the
           * underlying RIL, but it ensures that the callback is removed
           * from the list of notifications.
           */
      @Override public void stopNetworkQuery() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopNetworkQuery, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopNetworkQuery();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Tells the service to unregister the network query callback.
           * Will not attempt to stop an ongoing network query.
           * Functionally may be the same as stopNetworkQuery since that
           * function also does not stop a query request in the underlying
           * RIL.
           */
      @Override public void unregisterCallback(com.android.phone.INetworkQueryServiceCallback cb) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((cb!=null))?(cb.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_unregisterCallback, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unregisterCallback(cb);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static com.android.phone.INetworkQueryService sDefaultImpl;
    }
    static final int TRANSACTION_startNetworkQuery = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_stopNetworkQuery = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_unregisterCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(com.android.phone.INetworkQueryService impl) {
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
    public static com.android.phone.INetworkQueryService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Starts a network query if it has not been started yet, and
       * request a callback through the INetworkQueryServiceCallback
       * object on query completion.  If there is an existing request,
       * then just add the callback to the list of notifications
       * that will be sent upon query completion.
       *
       * It will send the network query with the use of
       * <code>TelephonyManager.requestNetworkScan()</code> if the
       * isIncrementalResult is true. And if the isIncrementalResult
       * is set as false, it will try to send network query through
       * <code>Phone.getAvailableNetworks()</code>.
       */
  public void startNetworkQuery(com.android.phone.INetworkQueryServiceCallback cb, int phoneId, boolean isIncrementalResult) throws android.os.RemoteException;
  /**
       * Tells the service that the requested query is to be ignored.
       * This may not do anything for the Query request in the
       * underlying RIL, but it ensures that the callback is removed
       * from the list of notifications.
       */
  public void stopNetworkQuery() throws android.os.RemoteException;
  /**
       * Tells the service to unregister the network query callback.
       * Will not attempt to stop an ongoing network query.
       * Functionally may be the same as stopNetworkQuery since that
       * function also does not stop a query request in the underlying
       * RIL.
       */
  public void unregisterCallback(com.android.phone.INetworkQueryServiceCallback cb) throws android.os.RemoteException;
}
