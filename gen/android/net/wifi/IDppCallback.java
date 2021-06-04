/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.wifi;
/**
 * Interface for DPP callback.
 *
 * @hide
 */
public interface IDppCallback extends android.os.IInterface
{
  /** Default implementation for IDppCallback. */
  public static class Default implements android.net.wifi.IDppCallback
  {
    /**
         * Called when local DPP Enrollee successfully receives a new Wi-Fi configuratrion from the
         * peer DPP configurator.
         */
    @Override public void onSuccessConfigReceived(int newNetworkId) throws android.os.RemoteException
    {
    }
    /**
         * Called when DPP success events take place, except for when configuration is received from
         * an external Configurator. The callback onSuccessConfigReceived will be used in this case.
         */
    @Override public void onSuccess(int status) throws android.os.RemoteException
    {
    }
    /**
         * Called when DPP Failure events take place.
         */
    @Override public void onFailure(int status) throws android.os.RemoteException
    {
    }
    /**
         * Called when DPP events that indicate progress take place. Can be used by UI elements
         * to show progress.
         */
    @Override public void onProgress(int status) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.wifi.IDppCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.net.wifi.IDppCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.wifi.IDppCallback interface,
     * generating a proxy if needed.
     */
    public static android.net.wifi.IDppCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.wifi.IDppCallback))) {
        return ((android.net.wifi.IDppCallback)iin);
      }
      return new android.net.wifi.IDppCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onSuccessConfigReceived:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onSuccessConfigReceived(_arg0);
          return true;
        }
        case TRANSACTION_onSuccess:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onSuccess(_arg0);
          return true;
        }
        case TRANSACTION_onFailure:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onFailure(_arg0);
          return true;
        }
        case TRANSACTION_onProgress:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onProgress(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.wifi.IDppCallback
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
           * Called when local DPP Enrollee successfully receives a new Wi-Fi configuratrion from the
           * peer DPP configurator.
           */
      @Override public void onSuccessConfigReceived(int newNetworkId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(newNetworkId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSuccessConfigReceived, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSuccessConfigReceived(newNetworkId);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when DPP success events take place, except for when configuration is received from
           * an external Configurator. The callback onSuccessConfigReceived will be used in this case.
           */
      @Override public void onSuccess(int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSuccess, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSuccess(status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when DPP Failure events take place.
           */
      @Override public void onFailure(int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onFailure, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onFailure(status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Called when DPP events that indicate progress take place. Can be used by UI elements
           * to show progress.
           */
      @Override public void onProgress(int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onProgress, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onProgress(status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.wifi.IDppCallback sDefaultImpl;
    }
    static final int TRANSACTION_onSuccessConfigReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onSuccess = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onFailure = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onProgress = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.net.wifi.IDppCallback impl) {
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
    public static android.net.wifi.IDppCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Called when local DPP Enrollee successfully receives a new Wi-Fi configuratrion from the
       * peer DPP configurator.
       */
  public void onSuccessConfigReceived(int newNetworkId) throws android.os.RemoteException;
  /**
       * Called when DPP success events take place, except for when configuration is received from
       * an external Configurator. The callback onSuccessConfigReceived will be used in this case.
       */
  public void onSuccess(int status) throws android.os.RemoteException;
  /**
       * Called when DPP Failure events take place.
       */
  public void onFailure(int status) throws android.os.RemoteException;
  /**
       * Called when DPP events that indicate progress take place. Can be used by UI elements
       * to show progress.
       */
  public void onProgress(int status) throws android.os.RemoteException;
}
