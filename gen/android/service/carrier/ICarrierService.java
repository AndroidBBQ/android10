/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.carrier;
/**
 * Service used to expose carrier-specific functionality to the system.
 *
 * @see android.service.carrier.CarrierService
 * @hide
 */
public interface ICarrierService extends android.os.IInterface
{
  /** Default implementation for ICarrierService. */
  public static class Default implements android.service.carrier.ICarrierService
  {
    /** @see android.service.carrier.CarrierService#onLoadConfig */
    @Override public void getCarrierConfig(android.service.carrier.CarrierIdentifier id, android.os.ResultReceiver result) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.carrier.ICarrierService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.carrier.ICarrierService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.carrier.ICarrierService interface,
     * generating a proxy if needed.
     */
    public static android.service.carrier.ICarrierService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.carrier.ICarrierService))) {
        return ((android.service.carrier.ICarrierService)iin);
      }
      return new android.service.carrier.ICarrierService.Stub.Proxy(obj);
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
        case TRANSACTION_getCarrierConfig:
        {
          data.enforceInterface(descriptor);
          android.service.carrier.CarrierIdentifier _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.service.carrier.CarrierIdentifier.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.ResultReceiver _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.os.ResultReceiver.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.getCarrierConfig(_arg0, _arg1);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.carrier.ICarrierService
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
      /** @see android.service.carrier.CarrierService#onLoadConfig */
      @Override public void getCarrierConfig(android.service.carrier.CarrierIdentifier id, android.os.ResultReceiver result) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((id!=null)) {
            _data.writeInt(1);
            id.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((result!=null)) {
            _data.writeInt(1);
            result.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCarrierConfig, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getCarrierConfig(id, result);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.carrier.ICarrierService sDefaultImpl;
    }
    static final int TRANSACTION_getCarrierConfig = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.service.carrier.ICarrierService impl) {
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
    public static android.service.carrier.ICarrierService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /** @see android.service.carrier.CarrierService#onLoadConfig */
  public void getCarrierConfig(android.service.carrier.CarrierIdentifier id, android.os.ResultReceiver result) throws android.os.RemoteException;
}
