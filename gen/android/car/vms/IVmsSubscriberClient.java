/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.vms;
/**
 * @hide
 */
public interface IVmsSubscriberClient extends android.os.IInterface
{
  /** Default implementation for IVmsSubscriberClient. */
  public static class Default implements android.car.vms.IVmsSubscriberClient
  {
    /**
         * A VmsService uses this callback to pass messages to subscribers.
         */
    @Override public void onVmsMessageReceived(android.car.vms.VmsLayer layer, byte[] payload) throws android.os.RemoteException
    {
    }
    @Override public void onLayersAvailabilityChanged(android.car.vms.VmsAvailableLayers availableLayers) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.vms.IVmsSubscriberClient
  {
    private static final java.lang.String DESCRIPTOR = "android.car.vms.IVmsSubscriberClient";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.vms.IVmsSubscriberClient interface,
     * generating a proxy if needed.
     */
    public static android.car.vms.IVmsSubscriberClient asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.vms.IVmsSubscriberClient))) {
        return ((android.car.vms.IVmsSubscriberClient)iin);
      }
      return new android.car.vms.IVmsSubscriberClient.Stub.Proxy(obj);
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
        case TRANSACTION_onVmsMessageReceived:
        {
          data.enforceInterface(descriptor);
          android.car.vms.VmsLayer _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.car.vms.VmsLayer.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          byte[] _arg1;
          _arg1 = data.createByteArray();
          this.onVmsMessageReceived(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onLayersAvailabilityChanged:
        {
          data.enforceInterface(descriptor);
          android.car.vms.VmsAvailableLayers _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.car.vms.VmsAvailableLayers.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onLayersAvailabilityChanged(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.vms.IVmsSubscriberClient
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
           * A VmsService uses this callback to pass messages to subscribers.
           */
      @Override public void onVmsMessageReceived(android.car.vms.VmsLayer layer, byte[] payload) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((layer!=null)) {
            _data.writeInt(1);
            layer.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeByteArray(payload);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onVmsMessageReceived, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onVmsMessageReceived(layer, payload);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onLayersAvailabilityChanged(android.car.vms.VmsAvailableLayers availableLayers) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((availableLayers!=null)) {
            _data.writeInt(1);
            availableLayers.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onLayersAvailabilityChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onLayersAvailabilityChanged(availableLayers);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.car.vms.IVmsSubscriberClient sDefaultImpl;
    }
    static final int TRANSACTION_onVmsMessageReceived = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onLayersAvailabilityChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.car.vms.IVmsSubscriberClient impl) {
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
    public static android.car.vms.IVmsSubscriberClient getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * A VmsService uses this callback to pass messages to subscribers.
       */
  public void onVmsMessageReceived(android.car.vms.VmsLayer layer, byte[] payload) throws android.os.RemoteException;
  public void onLayersAvailabilityChanged(android.car.vms.VmsAvailableLayers availableLayers) throws android.os.RemoteException;
}
