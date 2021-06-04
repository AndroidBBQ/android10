/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.wifi.hotspot2;
/**
 * Interface for Provisioning callback.
 *
 * @hide
 */
public interface IProvisioningCallback extends android.os.IInterface
{
  /** Default implementation for IProvisioningCallback. */
  public static class Default implements android.net.wifi.hotspot2.IProvisioningCallback
  {
    /**
         * Service to manager callback providing failure notification
         */
    @Override public void onProvisioningFailure(int status) throws android.os.RemoteException
    {
    }
    /**
         * Service to manager callback providing Provisioning status
         */
    @Override public void onProvisioningStatus(int status) throws android.os.RemoteException
    {
    }
    /**
         * Service to manager callback providing completion of Provisioning/Remediation flow
         */
    @Override public void onProvisioningComplete() throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.wifi.hotspot2.IProvisioningCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.net.wifi.hotspot2.IProvisioningCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.wifi.hotspot2.IProvisioningCallback interface,
     * generating a proxy if needed.
     */
    public static android.net.wifi.hotspot2.IProvisioningCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.wifi.hotspot2.IProvisioningCallback))) {
        return ((android.net.wifi.hotspot2.IProvisioningCallback)iin);
      }
      return new android.net.wifi.hotspot2.IProvisioningCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onProvisioningFailure:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onProvisioningFailure(_arg0);
          return true;
        }
        case TRANSACTION_onProvisioningStatus:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onProvisioningStatus(_arg0);
          return true;
        }
        case TRANSACTION_onProvisioningComplete:
        {
          data.enforceInterface(descriptor);
          this.onProvisioningComplete();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.wifi.hotspot2.IProvisioningCallback
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
           * Service to manager callback providing failure notification
           */
      @Override public void onProvisioningFailure(int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onProvisioningFailure, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onProvisioningFailure(status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Service to manager callback providing Provisioning status
           */
      @Override public void onProvisioningStatus(int status) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(status);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onProvisioningStatus, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onProvisioningStatus(status);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Service to manager callback providing completion of Provisioning/Remediation flow
           */
      @Override public void onProvisioningComplete() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onProvisioningComplete, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onProvisioningComplete();
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.net.wifi.hotspot2.IProvisioningCallback sDefaultImpl;
    }
    static final int TRANSACTION_onProvisioningFailure = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onProvisioningStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onProvisioningComplete = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    public static boolean setDefaultImpl(android.net.wifi.hotspot2.IProvisioningCallback impl) {
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
    public static android.net.wifi.hotspot2.IProvisioningCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Service to manager callback providing failure notification
       */
  public void onProvisioningFailure(int status) throws android.os.RemoteException;
  /**
       * Service to manager callback providing Provisioning status
       */
  public void onProvisioningStatus(int status) throws android.os.RemoteException;
  /**
       * Service to manager callback providing completion of Provisioning/Remediation flow
       */
  public void onProvisioningComplete() throws android.os.RemoteException;
}
