/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.wifi;
/**
 * Interface for Soft AP callback.
 *
 * @hide
 */
public interface ISoftApCallback extends android.os.IInterface
{
  /** Default implementation for ISoftApCallback. */
  public static class Default implements android.net.wifi.ISoftApCallback
  {
    /**
         * Service to manager callback providing current soft AP state. The possible
         * parameter values listed are defined in WifiManager.java
         *
         * @param state new AP state. One of WIFI_AP_STATE_DISABLED,
         *        WIFI_AP_STATE_DISABLING, WIFI_AP_STATE_ENABLED,
         *        WIFI_AP_STATE_ENABLING, WIFI_AP_STATE_FAILED
         * @param failureReason reason when in failed state. One of
         *        SAP_START_FAILURE_GENERAL, SAP_START_FAILURE_NO_CHANNEL
         */
    @Override public void onStateChanged(int state, int failureReason) throws android.os.RemoteException
    {
    }
    /**
         * Service to manager callback providing number of connected clients.
         *
         * @param numClients number of connected clients
         */
    @Override public void onNumClientsChanged(int numClients) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.wifi.ISoftApCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.net.wifi.ISoftApCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.wifi.ISoftApCallback interface,
     * generating a proxy if needed.
     */
    public static android.net.wifi.ISoftApCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.wifi.ISoftApCallback))) {
        return ((android.net.wifi.ISoftApCallback)iin);
      }
      return new android.net.wifi.ISoftApCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onStateChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          this.onStateChanged(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onNumClientsChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onNumClientsChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.wifi.ISoftApCallback
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
           * Service to manager callback providing current soft AP state. The possible
           * parameter values listed are defined in WifiManager.java
           *
           * @param state new AP state. One of WIFI_AP_STATE_DISABLED,
           *        WIFI_AP_STATE_DISABLING, WIFI_AP_STATE_ENABLED,
           *        WIFI_AP_STATE_ENABLING, WIFI_AP_STATE_FAILED
           * @param failureReason reason when in failed state. One of
           *        SAP_START_FAILURE_GENERAL, SAP_START_FAILURE_NO_CHANNEL
           */
      @Override public void onStateChanged(int state, int failureReason) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(state);
          _data.writeInt(failureReason);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onStateChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onStateChanged(state, failureReason);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Service to manager callback providing number of connected clients.
           *
           * @param numClients number of connected clients
           */
      @Override public void onNumClientsChanged(int numClients) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(numClients);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onNumClientsChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onNumClientsChanged(numClients);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.wifi.ISoftApCallback sDefaultImpl;
    }
    static final int TRANSACTION_onStateChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onNumClientsChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.net.wifi.ISoftApCallback impl) {
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
    public static android.net.wifi.ISoftApCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Service to manager callback providing current soft AP state. The possible
       * parameter values listed are defined in WifiManager.java
       *
       * @param state new AP state. One of WIFI_AP_STATE_DISABLED,
       *        WIFI_AP_STATE_DISABLING, WIFI_AP_STATE_ENABLED,
       *        WIFI_AP_STATE_ENABLING, WIFI_AP_STATE_FAILED
       * @param failureReason reason when in failed state. One of
       *        SAP_START_FAILURE_GENERAL, SAP_START_FAILURE_NO_CHANNEL
       */
  public void onStateChanged(int state, int failureReason) throws android.os.RemoteException;
  /**
       * Service to manager callback providing number of connected clients.
       *
       * @param numClients number of connected clients
       */
  public void onNumClientsChanged(int numClients) throws android.os.RemoteException;
}
