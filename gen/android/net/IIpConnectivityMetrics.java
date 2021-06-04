/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net;
/** {@hide} */
public interface IIpConnectivityMetrics extends android.os.IInterface
{
  /** Default implementation for IIpConnectivityMetrics. */
  public static class Default implements android.net.IIpConnectivityMetrics
  {
    /**
         * @return the number of remaining available slots in buffer,
         * or -1 if the event was dropped due to rate limiting.
         */
    @Override public int logEvent(android.net.ConnectivityMetricsEvent event) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Callback can be registered by DevicePolicyManager or NetworkWatchlistService only.
         * @return status {@code true} if registering/unregistering of the callback was successful,
         *         {@code false} otherwise (might happen if IIpConnectivityMetrics is not available,
         *         if it happens make sure you call it when the service is up in the caller)
         */
    @Override public boolean addNetdEventCallback(int callerType, android.net.INetdEventCallback callback) throws android.os.RemoteException
    {
      return false;
    }
    @Override public boolean removeNetdEventCallback(int callerType) throws android.os.RemoteException
    {
      return false;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.IIpConnectivityMetrics
  {
    private static final java.lang.String DESCRIPTOR = "android.net.IIpConnectivityMetrics";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.IIpConnectivityMetrics interface,
     * generating a proxy if needed.
     */
    public static android.net.IIpConnectivityMetrics asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.IIpConnectivityMetrics))) {
        return ((android.net.IIpConnectivityMetrics)iin);
      }
      return new android.net.IIpConnectivityMetrics.Stub.Proxy(obj);
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
        case TRANSACTION_logEvent:
        {
          data.enforceInterface(descriptor);
          android.net.ConnectivityMetricsEvent _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.ConnectivityMetricsEvent.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _result = this.logEvent(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_addNetdEventCallback:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.net.INetdEventCallback _arg1;
          _arg1 = android.net.INetdEventCallback.Stub.asInterface(data.readStrongBinder());
          boolean _result = this.addNetdEventCallback(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_removeNetdEventCallback:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          boolean _result = this.removeNetdEventCallback(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.IIpConnectivityMetrics
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
           * @return the number of remaining available slots in buffer,
           * or -1 if the event was dropped due to rate limiting.
           */
      @Override public int logEvent(android.net.ConnectivityMetricsEvent event) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((event!=null)) {
            _data.writeInt(1);
            event.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_logEvent, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().logEvent(event);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /**
           * Callback can be registered by DevicePolicyManager or NetworkWatchlistService only.
           * @return status {@code true} if registering/unregistering of the callback was successful,
           *         {@code false} otherwise (might happen if IIpConnectivityMetrics is not available,
           *         if it happens make sure you call it when the service is up in the caller)
           */
      @Override public boolean addNetdEventCallback(int callerType, android.net.INetdEventCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(callerType);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addNetdEventCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().addNetdEventCallback(callerType, callback);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      @Override public boolean removeNetdEventCallback(int callerType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(callerType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeNetdEventCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().removeNetdEventCallback(callerType);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.net.IIpConnectivityMetrics sDefaultImpl;
    }
    static final int TRANSACTION_logEvent = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_addNetdEventCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_removeNetdEventCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.net.IIpConnectivityMetrics impl) {
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
    public static android.net.IIpConnectivityMetrics getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * @return the number of remaining available slots in buffer,
       * or -1 if the event was dropped due to rate limiting.
       */
  public int logEvent(android.net.ConnectivityMetricsEvent event) throws android.os.RemoteException;
  /**
       * Callback can be registered by DevicePolicyManager or NetworkWatchlistService only.
       * @return status {@code true} if registering/unregistering of the callback was successful,
       *         {@code false} otherwise (might happen if IIpConnectivityMetrics is not available,
       *         if it happens make sure you call it when the service is up in the caller)
       */
  public boolean addNetdEventCallback(int callerType, android.net.INetdEventCallback callback) throws android.os.RemoteException;
  public boolean removeNetdEventCallback(int callerType) throws android.os.RemoteException;
}
