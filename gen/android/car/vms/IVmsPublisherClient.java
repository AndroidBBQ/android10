/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.vms;
/**
 * @hide
 */
public interface IVmsPublisherClient extends android.os.IInterface
{
  /** Default implementation for IVmsPublisherClient. */
  public static class Default implements android.car.vms.IVmsPublisherClient
  {
    /**
        * Once the VmsPublisherService is bound to the client, this callback is used to set the
        * binder that the client can use to invoke publisher services. This also gives the client
        * the token it should use when calling the service.
        */
    @Override public void setVmsPublisherService(android.os.IBinder token, android.car.vms.IVmsPublisherService service) throws android.os.RemoteException
    {
    }
    /**
         * The VmsPublisherService uses this callback to notify about subscription changes.
         * @param subscriptionState all the layers that have subscribers and a sequence number,
         *                          clients should ignore any packet with a sequence number that is less
         *                          than the highest sequence number they have seen thus far.
         */
    @Override public void onVmsSubscriptionChange(android.car.vms.VmsSubscriptionState subscriptionState) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.vms.IVmsPublisherClient
  {
    private static final java.lang.String DESCRIPTOR = "android.car.vms.IVmsPublisherClient";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.vms.IVmsPublisherClient interface,
     * generating a proxy if needed.
     */
    public static android.car.vms.IVmsPublisherClient asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.vms.IVmsPublisherClient))) {
        return ((android.car.vms.IVmsPublisherClient)iin);
      }
      return new android.car.vms.IVmsPublisherClient.Stub.Proxy(obj);
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
        case TRANSACTION_setVmsPublisherService:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.car.vms.IVmsPublisherService _arg1;
          _arg1 = android.car.vms.IVmsPublisherService.Stub.asInterface(data.readStrongBinder());
          this.setVmsPublisherService(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onVmsSubscriptionChange:
        {
          data.enforceInterface(descriptor);
          android.car.vms.VmsSubscriptionState _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.car.vms.VmsSubscriptionState.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onVmsSubscriptionChange(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.vms.IVmsPublisherClient
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
          * Once the VmsPublisherService is bound to the client, this callback is used to set the
          * binder that the client can use to invoke publisher services. This also gives the client
          * the token it should use when calling the service.
          */
      @Override public void setVmsPublisherService(android.os.IBinder token, android.car.vms.IVmsPublisherService service) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          _data.writeStrongBinder((((service!=null))?(service.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setVmsPublisherService, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setVmsPublisherService(token, service);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * The VmsPublisherService uses this callback to notify about subscription changes.
           * @param subscriptionState all the layers that have subscribers and a sequence number,
           *                          clients should ignore any packet with a sequence number that is less
           *                          than the highest sequence number they have seen thus far.
           */
      @Override public void onVmsSubscriptionChange(android.car.vms.VmsSubscriptionState subscriptionState) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((subscriptionState!=null)) {
            _data.writeInt(1);
            subscriptionState.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onVmsSubscriptionChange, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onVmsSubscriptionChange(subscriptionState);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.car.vms.IVmsPublisherClient sDefaultImpl;
    }
    static final int TRANSACTION_setVmsPublisherService = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onVmsSubscriptionChange = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.car.vms.IVmsPublisherClient impl) {
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
    public static android.car.vms.IVmsPublisherClient getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
      * Once the VmsPublisherService is bound to the client, this callback is used to set the
      * binder that the client can use to invoke publisher services. This also gives the client
      * the token it should use when calling the service.
      */
  public void setVmsPublisherService(android.os.IBinder token, android.car.vms.IVmsPublisherService service) throws android.os.RemoteException;
  /**
       * The VmsPublisherService uses this callback to notify about subscription changes.
       * @param subscriptionState all the layers that have subscribers and a sequence number,
       *                          clients should ignore any packet with a sequence number that is less
       *                          than the highest sequence number they have seen thus far.
       */
  public void onVmsSubscriptionChange(android.car.vms.VmsSubscriptionState subscriptionState) throws android.os.RemoteException;
}
